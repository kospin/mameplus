#include <QtPlugin>

#include "mamepguimain.h"

#include "mameopt.h"
#include "utils.h"
#include "dialogs.h"
#include "ips.h"
#include "m1.h"

#ifdef Q_OS_WIN
#include "SDL.h"
#undef main
#endif /* Q_OS_WIN */

//static qt works with windows version
Q_IMPORT_PLUGIN(qico)
Q_IMPORT_PLUGIN(qjpeg)
//Q_IMPORT_PLUGIN(qmng)

/* global */
MainWindow *win;
QSettings guiSettings(CFG_PREFIX + "mamepgui.ini", QSettings::IniFormat);
QSettings defSettings(":/res/mamepgui.ini", QSettings::IniFormat);
QString mame_binary;
QString language;
bool local_game_list;
bool isDarkBg = false;
bool sdlInited = false;
bool isMamePlus = false;
QStringList validGuiSettings;

/* internal */
QDockWidget *dwHistory = NULL;

void MainWindow::log(QString message, char logOrigin)
{
	QString timeString = QTime::currentTime().toString("hh:mm:ss.zzz");

	QString msg = timeString + ": " + message;

	tbGUILog->append(msg);
	tbGUILog->horizontalScrollBar()->setValue(0);
}

void MainWindow::poplog(QString message)
{
	QMessageBox::critical(this, "Debug", message); 
}

void MainWindow::logStatus(QString message)
{
	labelProgress->setText(message);
}

void MainWindow::logStatus(GameInfo *gameInfo)
{
	//if everything is ok, only show 1 icon
	//show all 6 icons otherwise
	static const QString prefix = ":/res/16x16/";
	static const QString suffix = ".png";
	QString statusBuffer = "";

	if (gameInfo->isExtRom)
		gameInfo = mameGame->games[gameInfo->romof];

	QString statusText = utils->getStatusString (gameInfo->status);
	QString statusName = QT_TR_NOOP("status");
	labelStatus->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->emulation);
	statusName = QT_TR_NOOP("emulation");
	labelEmulation->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->color);
	statusName = QT_TR_NOOP("color");
	labelColor->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->sound);
	statusName = QT_TR_NOOP("sound");
	labelSound->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->graphic);
	statusName = QT_TR_NOOP("graphic");
	labelGraphic->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->savestate, true);
	statusName = QT_TR_NOOP("savestate");
	labelSavestate->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	//show 2 more icons if applicable, hide otherwise
	labelCocktail->hide();
	wStatus->layout()->removeWidget(labelCocktail);
	if (gameInfo->cocktail != 64)
	{
		wStatus->layout()->addWidget(labelCocktail);
		labelCocktail->show();
		statusText = utils->getStatusString (gameInfo->cocktail);
		statusName = QT_TR_NOOP("cocktail");
		labelCocktail->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
		statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";
	}

	labelProtection->hide();
	wStatus->layout()->removeWidget(labelProtection);
	if (gameInfo->protection != 64)
	{
		wStatus->layout()->addWidget(labelProtection);
		labelProtection->show();
		statusText = utils->getStatusString (gameInfo->protection);
		statusName = QT_TR_NOOP("protection");
		labelProtection->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
		statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";
	}

	statusBuffer.chop(1);
	win->wStatus->setToolTip(statusBuffer);

	
//	setText(QString("E: %1").arg(status));
}

MainWindow::MainWindow(QWidget *parent) : 
QMainWindow(parent)
{
	dockCtrlNames = (QStringList() 
	   << QT_TR_NOOP("Snapshot")
	   << QT_TR_NOOP("Flyer")
	   << QT_TR_NOOP("Cabinet")
	   << QT_TR_NOOP("Marquee")
	   << QT_TR_NOOP("Title")
	   << QT_TR_NOOP("Control Panel")
	   << QT_TR_NOOP("PCB")
	
	   << QT_TR_NOOP("History")
	   << QT_TR_NOOP("MAMEInfo")
	   << QT_TR_NOOP("Story")
	   << QT_TR_NOOP("Command"));

	setupUi(this);

	//setExclusive(true) for some actions
    QActionGroup *viewActions = new QActionGroup(this);
    viewActions->addAction(actionDetails);
    viewActions->addAction(actionGrouped);
    viewActions->addAction(actionLargeIcons);

	QActionGroup *langActions = new QActionGroup(this);
	langActions->addAction(actionEnglish);
	langActions->addAction(actionChinese_PRC);
	langActions->addAction(actionChinese_Taiwan);
	langActions->addAction(actionJapanese);
	langActions->addAction(actionBrazilian);

	QActionGroup *bgStretchActions = new QActionGroup(this);
	bgStretchActions->addAction(actionBgStretch);
	bgStretchActions->addAction(actionBgTile);

	// init controls
    tvGameList = new QTreeView(centralwidget);
    tvGameList->setRootIsDecorated(false);
    tvGameList->setItemsExpandable(false);
	tvGameList->hide();

	lvGameList = new QListView(centralwidget);
	lvGameList->setMovement(QListView::Static);
	lvGameList->setResizeMode(QListView::Adjust);
	lvGameList->setViewMode(QListView::IconMode);
//	lvGameList->setGridSize(QSize(96, 64));
	lvGameList->setUniformItemSizes(true);
	lvGameList->setWordWrap(true);
//	lvGameList->setTextElideMode(Qt::TextDontClip | Qt::TextWordWrap);
	lvGameList->hide();

	lineEditSearch = new QLineEdit(centralwidget);
	QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(lineEditSearch->sizePolicy().hasHeightForWidth());
	lineEditSearch->setSizePolicy(sizePolicy);
	lineEditSearch->setMinimumWidth(240);
	lineEditSearch->setEnabled(false);
	toolBarSearch->addWidget(lineEditSearch);

	btnSearch = new QToolButton(centralwidget);
	btnSearch->setIcon(QIcon(":/res/16x16/system-search.png"));
	btnSearch->setFixedWidth(24);
	btnSearch->setToolTip(tr("Search"));
	toolBarSearch->addWidget(btnSearch);
	
	btnClearSearch = new QToolButton(centralwidget);
	btnClearSearch->setIcon(QIcon(":/res/16x16/status_cross.png"));
	btnClearSearch->setFixedWidth(24);
	btnClearSearch->setToolTip(tr("Clear"));
	toolBarSearch->addWidget(btnClearSearch);

	labelProgress = new QLabel(statusbar);
	statusbar->addWidget(labelProgress);
	
	wStatus = new QWidget(statusbar);
	wStatus->setMaximumHeight(18);
	QHBoxLayout *layout = new QHBoxLayout(wStatus);
	layout->setMargin(0);
	layout->setSpacing(1);
	wStatus->setLayout(layout);
	statusbar->addPermanentWidget(wStatus);

	labelStatus = new QLabel(statusbar);
	layout->addWidget(labelStatus);
	labelEmulation = new QLabel(statusbar);
	layout->addWidget(labelEmulation);
	labelColor = new QLabel(statusbar);
	layout->addWidget(labelColor);
	labelSound = new QLabel(statusbar);
	layout->addWidget(labelSound);
	labelGraphic = new QLabel(statusbar);
	layout->addWidget(labelGraphic);
	labelSavestate = new QLabel(statusbar);
	layout->addWidget(labelSavestate);
	labelCocktail = new QLabel(statusbar);
	layout->addWidget(labelCocktail);
	labelProtection = new QLabel(statusbar);
	layout->addWidget(labelProtection);

	labelGameCount = new QLabel(statusbar);
	statusbar->addPermanentWidget(labelGameCount);

	progressBarGamelist = new QProgressBar(centralwidget);
	progressBarGamelist->setMaximumHeight(16);
	progressBarGamelist->hide();

	QAction *actionFolderList = dwFolderList->toggleViewAction();
	actionFolderList->setIcon(QIcon(":/res/mame32-show-tree.png"));
	menuView->insertAction(actionVerticalTabs, actionFolderList);
	toolBar->insertAction(actionLargeIcons, actionFolderList);

	gameList = new Gamelist(this);
	optUtils = new OptionUtils(this);
	dirsUI = new Dirs(this);
	optionsUI = new Options(this);
	aboutUI = new About(this);
#ifdef Q_OS_WIN
	ipsUI = new IPS(this);
	m1UI = new M1UI(this);
#endif /* Q_OS_WIN */

	QTimer::singleShot(0, this, SLOT(init()));
}

MainWindow::~MainWindow()
{
#ifdef Q_OS_WIN
	SDL_Quit();
#endif
}

void MainWindow::initHistory(int snapType)
{
	QString title = dockCtrlNames[snapType];
	QDockWidget *dockWidget = new QDockWidget(this);

	dockWidget->setObjectName("dockWidget_" + title);
	dockWidget->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::NoDockWidgetFeatures);
	QWidget *dockWidgetContents = new QWidget(dockWidget);
	dockWidgetContents->setObjectName("dockWidgetContents_" + title);
	QGridLayout *gridLayout = new QGridLayout(dockWidgetContents);
	gridLayout->setObjectName("gridLayout_" + title);
	gridLayout->setContentsMargins(0, 0, 0, 0);

	QTextBrowser * tb;
	if (snapType == DOCK_HISTORY)
	{
		tb = tbHistory = new QTextBrowser(dockWidgetContents);
		tbHistory->setOpenExternalLinks(true);
	}
	else if (snapType == DOCK_MAMEINFO)
		tb = tbMameinfo = new QTextBrowser(dockWidgetContents);
	else if (snapType == DOCK_STORY)
		tb = tbStory = new QTextBrowser(dockWidgetContents);
	else
		tb = tbCommand = new QTextBrowser(dockWidgetContents);
	
	tb->setObjectName("textBrowser_" + title);

	gridLayout->addWidget(tb);

	dockWidget->setWidget(dockWidgetContents);
	dockWidget->setWindowTitle(tr(qPrintable(title)));
	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::RightDockWidgetArea), dockWidget);

	// create tabbed history widgets
	if (dwHistory)
		tabifyDockWidget(dwHistory, dockWidget);
	else
		dwHistory = dockWidget;

	menuDocuments->addAction(dockWidget->toggleViewAction());
	dockCtrls[snapType] = dockWidget;
}

void MainWindow::initSnap(int snapType)
{
	static Screenshot *dockWidget0 = NULL;
	
	Screenshot *dockWidget = new Screenshot(dockCtrlNames[snapType], this);

	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::RightDockWidgetArea), dockWidget);

	// create tabbed history widgets
	if (dockWidget0)
		tabifyDockWidget(dockWidget0, dockWidget);
	else
		dockWidget0 = dockWidget;

	menuPictures->addAction(dockWidget->toggleViewAction());
	dockCtrls[snapType] = dockWidget;
}

void MainWindow::init()
{
	for (int i = DOCK_SNAP; i <= DOCK_PCB; i ++)
		initSnap(i);

	for (int i = DOCK_HISTORY; i <= DOCK_COMMAND; i ++)
		initHistory(i);

	//init SDL
#ifdef Q_OS_WIN
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			win->log("SDL_INIT_VIDEO failed.");
		else
			sdlInited = true;

	ipsUI->init();

	//rearrange docks
	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::LeftDockWidgetArea), m1UI);
	tabifyDockWidget(dwFolderList, m1UI);
#endif /* Q_OS_WIN */

	//rearrange docks
	tabifyDockWidget(dwHistory, dwGUILog);

	//hide non popular docks by default
	dwGUILog->hide();

	for (int i = DOCK_SNAP; i < DOCK_LAST; i ++)
	{
		if (i != DOCK_SNAP
		 && i != DOCK_TITLE
		 && i != DOCK_HISTORY
		 && i != DOCK_MAMEINFO)
			dockCtrls[i]->hide();
	}

	dwFolderList->raise();
	dockCtrls[DOCK_SNAP]->raise();
	dockCtrls[DOCK_HISTORY]->raise();

	/* test ini readable/writable */
	// mkdir for individual game settings
	QDir().mkpath(CFG_PREFIX);

	QString warnings = "";
	QFile iniFile(CFG_PREFIX + "mamepgui.ini");
	if (!iniFile.open(QIODevice::ReadWrite | QFile::Text))
		warnings.append(QFileInfo(iniFile).absoluteFilePath());
	iniFile.close();

	if (warnings.size() > 0)
	{
		win->poplog("Current user has no sufficient privilege to read/write:\n" + warnings + "\n\ncouldn't save GUI settings.");
		//quit the program
		close();
		return;
	}

	initSettings();
	loadSettings();

	// validate mame_binary
	mame_binary = guiSettings.value("mame_binary").toString();
	QFile mamebin(mame_binary);

	// if no valid exec was found, popup a dialog
	if (!mamebin.exists() || mame_binary.endsWith("mamepgui" EXEC_EXT))
	{
		QString filter = "";
#ifdef Q_WS_WIN
		filter.append(tr("Executable files (*" EXEC_EXT ")"));
		filter.append(";;");
#endif
		filter.append(tr("All Files (*)"));
	
		mame_binary = QFileDialog::getOpenFileName(this,
									tr("MAME executable:"),
									QCoreApplication::applicationDirPath(),
									filter);

		if (mame_binary.isEmpty() || mame_binary.endsWith("mamepgui" EXEC_EXT))
		{
			win->poplog(QString("Could not find MAME."));
			mame_binary = "";
			//quit the program
			close();
			return;
		}
	}

	//save the new mame_binary value now, it will be accessed later in option module
	guiSettings.setValue("mame_binary", mame_binary);

	// must optUtils->initOption() after win, before show()
	optUtils->initOption();

	QIcon mamepIcon(":/res/16x16/mamep.png");
	qApp->setWindowIcon(mamepIcon);
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setIcon(mamepIcon);

	// must init app style before background
	if (gui_style.isEmpty())
		gui_style = guiSettings.value("gui_style").toString();

	QStringList styles = QStyleFactory::keys();
	QActionGroup *styleActions = new QActionGroup(this);
	foreach (QString style, styles)
	{
		QAction* act = win->menuGUIStyle->addAction(style);
		act->setCheckable(true);
		if (gui_style == style)
			act->setChecked(true);
		styleActions->addAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(setGuiStyle()));
	}

	if (!gui_style.isEmpty())
		setGuiStyle(gui_style);

	// init background menu
	QString _dirpath = utils->getPath(guiSettings.value("background_directory", "bkground").toString());
	QDir dir(_dirpath);
	
	if (background_file.isEmpty())
		background_file = guiSettings.value("background_file").toString();

	QActionGroup *bgActions = new QActionGroup(this);
	if (dir.exists())
	{
		QString dirpath = utils->getPath(_dirpath);
		
		QStringList nameFilter;
		nameFilter << "*.png";
		nameFilter << "*.jpg";
	
		// iterate all files in the path
		QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
		foreach (QString fileName, files)
		{
			QAction* act = win->menuBackground->addAction(fileName);
			act->setCheckable(true);
			if (background_file == fileName)
				act->setChecked(true);
			bgActions->addAction(act);
			connect(act, SIGNAL(triggered()), this, SLOT(setBgPixmap()));
		}
	}

	//show UI
	loadLayout();
	setDockOptions();

	//after loadLayout(), so we can check background stretch option
	if (!background_file.isEmpty())
		setBgPixmap(background_file);

	gameList->init(true, GAMELIST_INIT_FULL);
	show();

	// connect misc signal and slots

	// Docked snapshots
	QList<QTabBar *> tabBars = getSSTabBars();
	foreach (QTabBar *tabBar, tabBars)
		connect(tabBar, SIGNAL(currentChanged(int)), gameList, SLOT(updateSelection()));

	connect(actionBgStretch, SIGNAL(triggered()), this, SLOT(setBgTile()));
	connect(actionBgTile, SIGNAL(triggered()), this, SLOT(setBgTile()));

	// Actions
	connect(actionVerticalTabs, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));
	connect(actionLargeIcons, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));
	connect(actionDetails, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));
	connect(actionGrouped, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));

	connect(actionRowDelegate, SIGNAL(toggled(bool)), gameList, SLOT(toggleDelegate(bool)));
	//fixme:
	actionRowDelegate->setEnabled(false);
	connect(actionStretchSshot, SIGNAL(toggled(bool)), gameList, SLOT(updateSelection()));
	connect(actionEnforceAspect, SIGNAL(toggled(bool)), gameList, SLOT(updateSelection()));

	// Auditor
	connect(&gameList->auditor, SIGNAL(progressSwitched(int, QString)), gameList, SLOT(switchProgress(int, QString)));
	connect(&gameList->auditor, SIGNAL(progressUpdated(int)), gameList, SLOT(updateProgress(int)));
	connect(&gameList->auditor, SIGNAL(finished()), gameList->mAuditor, SLOT(init()));

	// Game List
	connect(lineEditSearch, SIGNAL(returnPressed()), gameList, SLOT(filterSearchChanged()));
	connect(btnSearch, SIGNAL(clicked()), gameList, SLOT(filterSearchChanged()));
	connect(btnClearSearch, SIGNAL(clicked()), gameList, SLOT(filterSearchCleared()));

	// Options
	for (int i = 1; i < optCtrls.count(); i++)
	{
		connect(optCtrls[i], SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), 
				optUtils, SLOT(updateModel(QListWidgetItem *)));
	}
	connect(optionsUI->tabOptions, SIGNAL(currentChanged(int)), optUtils, SLOT(updateModel()));

	// Tray Icon
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(on_trayIconActivated(QSystemTrayIcon::ActivationReason)));

#ifdef Q_OS_WIN
	//init M1 in a background thread
	m1 = new M1(this);
	m1->init();
#endif /* Q_OS_WIN */

	gameList->restoreGameSelection();
	gameList->updateSelection();
}

void MainWindow::setVersion()
{
	//set version info

	QString m1Ver = "";
	QString m1VerString = "";
	QString sdlVerString = "";

#ifdef Q_OS_WIN
	if (m1 != NULL && m1->available)
	{
		m1Ver = m1->version;
		m1VerString = QString("<a href=\"http://rbelmont.mameworld.info/?page_id=223\">M1</a> %1 multi-platform arcade music emulator &copy; R. Belmont")
						.arg(m1Ver);
	}

	sdlVerString = QString("<a href=\"http://www.libsdl.org\">SDL</a> %1.%2.%3 - Simple DirectMedia Layer<br>")
					.arg(SDL_MAJOR_VERSION)
					.arg(SDL_MINOR_VERSION)
					.arg(SDL_PATCHLEVEL);
#endif /* Q_OS_WIN */

	QString strVersion = QString(
		"<html>"
		"<head>"
		"<style type=\"text/css\">"
		"a {color: #006d9f;text-decoration: underline;}"
		"</style>"
		"</head>"
		"<body>"
		"<strong>MAME Plus! GUI</strong> %1 &copy; 2008 <a href=\"http://mameicons.free.fr/mame32p/\">MAME Plus!</a> Team<br>"
		"A Qt implementation of the famous <a href=\"http://mameui.classicgaming.gamespy.com\">MameUI</a>"
		"<hr>"
		"<a href=\"http://mamedev.org\">M.A.M.E.</a> %2 - Multiple Arcade Machine Emulator &copy; Nicola Salmoria and the MAME Team<br>"
		"<a href=\"http://trolltech.com\">Qt</a> %3 &copy; Nokia Corporation<br>"
		"%4"
		"%5"
		"</body>"
		"</html>")
		.arg("1.3 beta 10")
		.arg(mameGame->mameVersion)
		.arg(QT_VERSION_STR)
		.arg(sdlVerString)
		.arg(m1VerString);

	aboutUI->tbVersion->setHtml(strVersion);
#ifdef Q_OS_WIN
	m1UI->setWindowTitle("M1 - " + m1Ver);
#endif /* Q_OS_WIN */

	QFileInfo fi(mame_binary);

	win->setWindowTitle(QString("%1 - %2 %3")
		.arg(win->windowTitle())
		.arg(fi.baseName().toUpper())
		.arg(mameGame->mameVersion));
}

void MainWindow::on_actionPlay_activated()
{
	gameList->runMame();
}

void MainWindow::on_actionConfigIPS_activated()
{
#ifdef Q_OS_WIN
	ipsUI->updateList();
	ipsUI->exec();
#endif /* Q_OS_WIN */
}

void MainWindow::on_actionRefresh_activated()
{
	gameList->auditor.audit();
}

void MainWindow::on_actionProperties_activated()
{
	showOptionsDialog(OPTLEVEL_CURR, 0);
}

void MainWindow::on_actionSrcProperties_activated()
{
	showOptionsDialog(OPTLEVEL_SRC, 0);
}

void MainWindow::on_actionDefaultOptions_activated()
{
	showOptionsDialog(OPTLEVEL_GLOBAL, 1);
}

void MainWindow::on_actionDirectories_activated()
{
	showOptionsDialog(OPTLEVEL_GLOBAL, 0);
}

void MainWindow::showOptionsDialog(int optLevel, int lstRow)
{
	//prevent crash when list is empty
	if (currentGame.isEmpty())
		return;

	//init ctlrs, 
	for (int i = OPTLEVEL_GLOBAL; i < OPTLEVEL_LAST; i++)
		optUtils->updateModel(NULL, i);

	optionsUI->tabOptions->setCurrentIndex(optLevel);

	if (lstRow > -1)
		optCtrls[optLevel]->setCurrentRow(lstRow);

	optionsUI->exec();
}

void MainWindow::on_actionExitStop_activated()
{
	close();
}

void MainWindow::on_actionReadme_activated()
{
	QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/showflat.php?Cat=&Number=158710&view=collapsed"));
}

void MainWindow::on_actionFAQ_activated()
{
	QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/showflat.php?Cat=&Number=164054&view=collapsed"));
}

void MainWindow::on_actionBoard_activated()
{
	QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/postlist.php?Cat=&Board=mameplus&view=collapsed"));
}

void MainWindow::on_actionAbout_activated()
{
	aboutUI->exec();
}

void MainWindow::toggleGameListColumn(int logicalIndex)
{
	if (win->tvGameList->header()->isSectionHidden(logicalIndex))
		win->tvGameList->header()->setSectionHidden (logicalIndex, false);
	else
		win->tvGameList->header()->setSectionHidden (logicalIndex, true);
}

void MainWindow::on_actionColDescription_activated()
{
	toggleGameListColumn(0);
}

void MainWindow::on_actionColName_activated()
{
	toggleGameListColumn(1);
}

void MainWindow::on_actionColROMs_activated()
{
	toggleGameListColumn(2);
}

void MainWindow::on_actionColManufacturer_activated()
{
	toggleGameListColumn(3);
}

void MainWindow::on_actionColDriver_activated()
{
	toggleGameListColumn(4);
}

void MainWindow::on_actionColYear_activated()
{
	toggleGameListColumn(5);
}

void MainWindow::on_actionColCloneOf_activated()
{
	toggleGameListColumn(6);
}

void MainWindow::on_actionEnglish_activated()
{
	language = "en_US";
	showRestartDialog();
}

void MainWindow::on_actionChinese_PRC_activated()
{
	language = "zh_CN";
	showRestartDialog();
}

void MainWindow::on_actionChinese_Taiwan_activated()
{
	language = "zh_TW";
	showRestartDialog();
}

void MainWindow::on_actionJapanese_activated()
{
	language = "ja_JP";
	showRestartDialog();
}

void MainWindow::on_actionBrazilian_activated()
{
	language = "pt_BR";
	showRestartDialog();
}

void MainWindow::on_actionLocalGameList_activated()
{
	local_game_list = actionLocalGameList->isChecked();
}

void MainWindow::on_trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
		if (win->isVisible())
			hide();
		else
			show();
		break;
	default:
		;
	}
}

void MainWindow::showRestartDialog()
{
	if (QMessageBox::Yes == QMessageBox::question(this, 
								tr("Restart"), 
								tr("Changing this option requires a restart to take effect.\nDo you wish to continue?"), 
								QMessageBox::Yes | QMessageBox::No))
		close();
}

void MainWindow::initSettings()
{
    guiSettings.setFallbacksEnabled(false);

	//remove invalid settings
	validGuiSettings = (QStringList() 
		<< "snapshot_directory"	//fixme: core
		<< "flyer_directory"
		<< "cabinet_directory"
		<< "marquee_directory"
		<< "title_directory"
		<< "cpanel_directory"
		<< "pcb_directory"
		
		<< "history_file"
		<< "mameinfo_file"
		<< "story_file"
		<< "command_file"	//fixme: core

		<< "icons_directory"
		<< "background_directory"
		<< "folder_directory"
		<< "background_file"
		<< "m1_directory"
		<< "m1_language"
		<< "ips_language"
		<< "ips_relationship"
		<< "gui_style"
		<< "background_stretch"
		<< "mame_binary"
		<< "option_geometry"
		<< "sort_column"
		<< "sort_reverse"
		<< "default_game"
		<< "folder_current"
		<< "vertical_tabs"
		<< "stretch_screenshot_larger"
		<< "enforce_aspect"
		<< "game_list_delegate"
		<< "local_game_list"
		<< "list_mode"
		<< "option_column_state"
		<< "window_geometry"
		<< "window_state"
		<< "column_state"
		<< "language");

	QStringList keys = guiSettings.allKeys();
	foreach(QString key, keys)
	{
		if (key.contains("_extra_software") || validGuiSettings.contains(key))
			continue;

		guiSettings.remove(key);
	}

}

void MainWindow::loadLayout()
{
	if (guiSettings.value("window_geometry").isValid())
		restoreGeometry(guiSettings.value("window_geometry").toByteArray());

	if (guiSettings.value("window_state").isValid())
		restoreState(guiSettings.value("window_state").toByteArray());
	
	option_geometry = guiSettings.value("option_geometry").toByteArray();

	actionVerticalTabs->setChecked(guiSettings.value("vertical_tabs", "1").toInt() == 1);
	actionRowDelegate->setChecked(guiSettings.value("game_list_delegate", "1").toInt() == 1);

	gameList->listMode = guiSettings.value("list_mode").toString();
	if (gameList->listMode == win->actionDetails->objectName().remove("action"))
		actionDetails->setChecked(true);
	else if (gameList->listMode == win->actionLargeIcons->objectName().remove("action"))
		actionLargeIcons->setChecked(true);
	else
		actionGrouped->setChecked(true);

	//fixme: use built-in locale name string?
	if (language == "zh_CN")
		actionChinese_PRC->setChecked(true);
	else if (language == "zh_TW")
		actionChinese_Taiwan->setChecked(true);
	else if (language == "ja_JP")
		actionJapanese->setChecked(true);
	else if (language == "pt_BR")
		actionBrazilian->setChecked(true);
	else
		actionEnglish->setChecked(true);

	actionStretchSshot->setChecked(guiSettings.value("stretch_screenshot_larger", "0").toInt() == 1);
	actionEnforceAspect->setChecked(guiSettings.value("enforce_aspect", "1").toInt() == 1);

	local_game_list = guiSettings.value("local_game_list", "1").toInt() == 1;
	actionLocalGameList->setChecked(local_game_list);

	if (guiSettings.value("background_stretch", "1").toInt() == 0)
		actionBgTile->setChecked(true);
	else
		actionBgStretch->setChecked(true);
	
}

void MainWindow::loadSettings()
{
	currentGame = guiSettings.value("default_game", "pacman").toString();

	if (defSettings.value("option_column_state").isValid())
		option_column_state = defSettings.value("option_column_state").toByteArray();
	else
		option_column_state = guiSettings.value("option_column_state").toByteArray();
}

void MainWindow::saveSettings()
{
	//some guiSettings uses mameOpts mapping for dialog view
	guiSettings.setValue("cabinet_directory", mameOpts["cabinet_directory"]->globalvalue);
	guiSettings.setValue("cpanel_directory", mameOpts["cpanel_directory"]->globalvalue);
	guiSettings.setValue("flyer_directory", mameOpts["flyer_directory"]->globalvalue);
	guiSettings.setValue("marquee_directory", mameOpts["marquee_directory"]->globalvalue);
	guiSettings.setValue("pcb_directory", mameOpts["pcb_directory"]->globalvalue);
	guiSettings.setValue("title_directory", mameOpts["title_directory"]->globalvalue);
	guiSettings.setValue("icons_directory", mameOpts["icons_directory"]->globalvalue);
	guiSettings.setValue("background_directory", mameOpts["background_directory"]->globalvalue);
	guiSettings.setValue("folder_directory", mameOpts["folder_directory"]->globalvalue);

	guiSettings.setValue("background_file", background_file);
	guiSettings.setValue("m1_directory", mameOpts["m1_directory"]->globalvalue);
#ifdef Q_OS_WIN
	guiSettings.setValue("m1_language", m1UI->cmbLang->currentText());
	guiSettings.setValue("ips_language", ipsUI->cmbLang->currentText());
	guiSettings.setValue("ips_relationship", ipsUI->chkRelation->isChecked() ? 1 : 0);
#endif /* Q_OS_WIN */
	guiSettings.setValue("gui_style", gui_style);
	guiSettings.setValue("language", language);

	guiSettings.setValue("history_file", mameOpts["history_file"]->globalvalue);
	guiSettings.setValue("story_file", mameOpts["story_file"]->globalvalue);
	guiSettings.setValue("mameinfo_file", mameOpts["mameinfo_file"]->globalvalue);
	if (mameOpts["mame_binary"]->globalvalue != mameOpts["mame_binary"]->defvalue)
		guiSettings.setValue("mame_binary", mameOpts["mame_binary"]->globalvalue);
	else
		guiSettings.setValue("mame_binary", mame_binary);

	//save console dirs
	foreach (QString optName, mameOpts.keys())
	{
		MameOption *pMameOpt = mameOpts[optName];

		if (pMameOpt->guivisible && optName.contains("_extra_software"))
		{
			QString v = mameOpts[optName]->globalvalue;
			if (!v.trimmed().isEmpty())
				guiSettings.setValue(optName, mameOpts[optName]->globalvalue);
		}
	}

	//save layout
	guiSettings.setValue("window_geometry", saveGeometry());
	guiSettings.setValue("window_state", saveState());
	guiSettings.setValue("option_geometry", option_geometry);
	guiSettings.setValue("option_column_state", option_column_state);
	guiSettings.setValue("column_state", tvGameList->header()->saveState());
	guiSettings.setValue("sort_column", tvGameList->header()->sortIndicatorSection());
	guiSettings.setValue("sort_reverse", (tvGameList->header()->sortIndicatorOrder() == Qt::AscendingOrder) ? 0 : 1);
	guiSettings.setValue("vertical_tabs", actionVerticalTabs->isChecked() ? 1 : 0);
	guiSettings.setValue("stretch_screenshot_larger", actionStretchSshot->isChecked() ? 1 : 0);
	guiSettings.setValue("enforce_aspect", actionEnforceAspect->isChecked() ? 1 : 0);
	guiSettings.setValue("game_list_delegate", actionRowDelegate->isChecked() ? 1 : 0);
	guiSettings.setValue("local_game_list", actionLocalGameList->isChecked() ? 1 : 0);
	guiSettings.setValue("background_stretch", actionBgTile->isChecked() ? 0 : 1);
	guiSettings.setValue("default_game", currentGame);
	guiSettings.setValue("folder_current", currentFolder);//fixme: rename
	guiSettings.setValue("list_mode", gameList->listMode);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	hide();
	if (!mame_binary.isEmpty())
	{
		saveSettings();
		mameGame->s11n();
	}
	event->accept();
}

void MainWindow::setDockOptions()
{
	DockOptions opts = dockOptions();
    if (actionVerticalTabs->isChecked())
	    opts |= VerticalTabs;
    else
	    opts &= ~VerticalTabs;
	QMainWindow::setDockOptions(opts);
}

void MainWindow::setTransparentBg(QWidget * w)
{
	QColor color, bgColor;
	QPalette palette;

	if (isDarkBg)
	{
		bgColor = QColor(0, 0, 0, 128);
		color = QColor(Qt::white);
	}
	else
	{
		bgColor = QColor(255, 255, 255, 128);
		color = QColor(Qt::black);
	}

	palette.setColor(QPalette::Active, QPalette::Text, color);
	palette.setColor(QPalette::Inactive, QPalette::Text, color);
	palette.setColor(QPalette::Disabled, QPalette::Text, color);
	palette.setColor(QPalette::Active, QPalette::Base, bgColor);
	palette.setColor(QPalette::Inactive, QPalette::Base, bgColor);
	palette.setColor(QPalette::Disabled, QPalette::Base, bgColor);

	w->setPalette(palette);
}

void MainWindow::setTransparentStyle(QWidget * w)
{
	QString style;
	if (isDarkBg)
		style =
			"color: rgba(255, 255, 255, 196);"
			"background-color: rgba(0, 0, 0, 128);"
			;
	else
		style =
			"color: rgba(0, 0, 0, 196);"
			"background-color: rgba(255, 255, 255, 128);"
			;
	
	w->setStyleSheet(style);
}

void MainWindow::resizeEvent(QResizeEvent * /* event */)
{
//	setBgPixmap(background_file);
}

void MainWindow::setGuiStyle(QString style)
{
	if (style.isEmpty())
		style = ((QAction *)sender())->text();

	gui_style = style;

	QApplication::setStyle(QStyleFactory::create(gui_style));

	//have to chain setBgPixmap() otherwise bgcolor isn't set properly
	if (!background_file.isEmpty())
		setBgPixmap(background_file);
}

void MainWindow::setBgTile()
{
	setBgPixmap(background_file);
}

void MainWindow::setBgPixmap(QString fileName)
{
	if (fileName.isEmpty())
		fileName = ((QAction *)sender())->text();

	background_file = fileName;

	QString _dirpath = utils->getPath(guiSettings.value("background_directory", "bkground").toString());
	QDir dir(_dirpath);
	QString dirpath = utils->getPath(_dirpath);

	QImage bkgroundImg(_dirpath + fileName);

	// setup mainwindow background
	if (!bkgroundImg.isNull())
	{
		if (actionBgStretch->isChecked())
			bkgroundImg = bkgroundImg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

		//fixme: must init before stylesheet applied
		QPalette palette = win->palette();
		palette.setBrush(backgroundRole(), QBrush(bkgroundImg));
//		win->setPalette(palette);

		//get the color tone of bg image
		bkgroundImg = bkgroundImg.scaled(1, 1, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		int grayVal = qGray(bkgroundImg.pixel(0, 0));
		if (grayVal < 128)
			isDarkBg = true;
		else
			isDarkBg = false;

		QString TEXT_COLOR = QString("color:") + (isDarkBg ? "white" : "black") + "; ";
		QString TEXT_BGCOLOR = isDarkBg ? "background-color: rgba(0, 0, 0, 128)" : "background-color: rgba(255, 255, 255, 128)";
		static const QString STATIC_STYLE = 
			" QToolBar{background-color:palette(window);}"
			" QStatusBar::item, QDockWidget::title {border-width:1px; border-style:solid; border-color:palette(dark);}"
			" QStatusBar QLabel {padding:0 1px;}"
			" QDockWidget::title {padding:1px 2px; margin:2px 0;}"
			;

		qApp->setStyleSheet( STATIC_STYLE
			+ " QDockWidget, QStatusBar QLabel {" + TEXT_COLOR + "}"
			+ " QStatusBar::item, QDockWidget::title {" + TEXT_BGCOLOR + "}"
			);
	
		setPalette(palette);

		// setup background alpha
		setTransparentBg(treeFolders);
		setTransparentBg(tvGameList);
		setTransparentBg(lvGameList);
		setTransparentBg(tbHistory);
		setTransparentBg(tbMameinfo);
		setTransparentBg(tbStory);
		setTransparentBg(tbCommand);
#ifdef Q_OS_WIN
		setTransparentBg(m1UI->twSongList);
		setTransparentStyle(m1UI->groupBox);
		setTransparentStyle(m1UI->lcdNumber);
#endif /* Q_OS_WIN */
	}
}

void MainWindow::toggleTrayIcon(int, QProcess::ExitStatus, bool isTrayIconVisible)
{
	win->trayIcon->setVisible(isTrayIconVisible);
	if (isTrayIconVisible)
		win->trayIcon->setToolTip(win->windowTitle());

	win->setVisible(!isTrayIconVisible);
}

QList<QTabBar *> MainWindow::getSSTabBars()
{
	//there's no API in Qt to access docked widget tabbar, we have to do it on our own
	QList<QTabBar *> tabBars = findChildren<QTabBar *>();
	QList<QTabBar *> tabBars2;

	//iterate all tab ctrls
	foreach (QTabBar *tabBar, tabBars)
	{
		bool isSSDocked = false;

		//iterate known screenshot/history dock names
		for (int i = 0; i < dockCtrlNames.count(); i++)
		{
			//iterate tabs in a tab bar
			for (int t = 0; t < tabBar->count(); t ++)
			{
				//if the tab contains any known screenshot/history dock names
				if (tr(qPrintable(dockCtrlNames[i])) == tabBar->tabText(t))
				{
					isSSDocked = true;
					break;
				}
			}
			if (isSSDocked)
				break;
		}

		if (isSSDocked)
			tabBars2.append(tabBar);
	}

	return tabBars2;
}

bool MainWindow::isDockTabVisible(QString objName)
{
	bool isSSTabbed = false;
	QList<QTabBar *> tabBars = getSSTabBars();
	objName = tr(qPrintable(objName));

	//iterate all tab ctrls
	foreach (QTabBar *tabBar, tabBars)
	{
		//tab bar's current index matches $objName
		if (objName == tabBar->tabText(tabBar->currentIndex()))
			return true;

		//iterate tabs in a tab bar
		for (int t = 0; t < tabBar->count(); t ++)
		{
			//if $objName is tabified in any of the tab ctrls
			if (objName == tabBar->tabText(t))
			{
				isSSTabbed = true;
				break;
			}
		}
	}

	// if the dock area contains only one SS widget
	if (!isSSTabbed)
		return true;

	return false;
}

Screenshot::Screenshot(QString title, QWidget *parent)
: QDockWidget(parent),
forceAspect(false)
{
	setObjectName("dockWidget_" + title);
	setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::NoDockWidgetFeatures);
	dockWidgetContents = new QWidget(this);
	dockWidgetContents->setObjectName("dockWidgetContents_" + title);
	mainLayout = new QGridLayout(dockWidgetContents);
	mainLayout->setObjectName("mainLayout_" + title);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	screenshotLabel = new QPushButton(dockWidgetContents);
	screenshotLabel->setObjectName("label_" + title);
	screenshotLabel->setCursor(QCursor(Qt::PointingHandCursor));
	screenshotLabel->setFlat(true);

	//so that we can shrink image
	screenshotLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	
	mainLayout->addWidget(screenshotLabel);

	setWidget(dockWidgetContents);
	setWindowTitle(MainWindow::tr(qPrintable(title)));

	connect(screenshotLabel, SIGNAL(clicked()), this, SLOT(rotateImage()));
}

void Screenshot::resizeEvent(QResizeEvent * /* event */)
{
    QSize scaledSize = originalPixmap.size();
	scaledSize.scale(screenshotLabel->size(), Qt::KeepAspectRatio);

	updateScreenshotLabel();
}

void Screenshot::setPixmap(QPixmap pm)
{
	originalPixmap = pm;
	forceAspect = false;
	updateScreenshotLabel();
}

void Screenshot::setPixmap(const QByteArray &pmdata, bool _forceAspect)
{
	QPixmap pm;
	pm.loadFromData(pmdata);
	originalPixmap = pm;

	forceAspect = _forceAspect;
	updateScreenshotLabel();
}

//click screenshot area to rotate dockwidgets
void Screenshot::rotateImage()
{
	QString objName = ((QWidget* )sender())->objectName();
	objName.remove(0, 6);	//remove "label_"

	//there's no API in Qt to access docked widget tabbar
	QList<QTabBar *> tabs = win->findChildren<QTabBar *>();
	foreach (QTabBar *tab, tabs)
	{
		bool isDock = false;

		// if the dock widget contains any of screenshot/history widgets
		for (int i = 0; i < win->dockCtrlNames.count(); i++)
		{
			if (MainWindow::tr(qPrintable(win->dockCtrlNames[i])) == tab->tabText(0))
			{
				isDock = true;
				break;
			}
		}

		// select the next tab
		if (isDock && MainWindow::tr(qPrintable(objName)) == tab->tabText(tab->currentIndex()))
		{
			int i = tab->currentIndex();
			if (++i > tab->count() - 1)
				i = 0;
			tab->setCurrentIndex(i);
			break;
		}
	}
}

void Screenshot::updateScreenshotLabel()
{
    QSize scaledSize = utils->getScaledSize(originalPixmap.size(), screenshotLabel->size(), forceAspect);

	screenshotLabel->setIconSize(scaledSize);
	screenshotLabel->setIcon(originalPixmap.scaled(scaledSize,
			Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

int main(int argc, char *argv[])
{
	QApplication myApp(argc, argv);

	QTranslator appTranslator;

	language = guiSettings.value("language").toString();
	if (language.isEmpty())
		language = QLocale::system().name();
	appTranslator.load(":/lang/mamepgui_" + language);

	myApp.installTranslator(&appTranslator);

	if (language.startsWith("zh_") || language.startsWith("ja_"))
	{
		QFont font;
		font.setPointSize(9);
		myApp.setFont(font);
	}
	
	procMan = new ProcessManager(0);	
	utils = new Utils(0);
	win = new MainWindow(0);

	return myApp.exec();
}

