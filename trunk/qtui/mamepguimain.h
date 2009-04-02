#ifndef _MAMEPGUIMAIN_H_
#define _MAMEPGUIMAIN_H_

//common
#include "quazip.h"
#include "quazipfile.h"

#include <QtGui>
#include <QApplication>
#include <QtXml>

#include "ui_mamepguimain.h"
#include "gamelist.h"

class Screenshot : public QDockWidget
{
    Q_OBJECT

public:
    Screenshot(QString, QWidget *parent = 0);
	void setPixmap(QPixmap pm);
	void setPixmap(const QByteArray &, bool);

protected:
    void resizeEvent(QResizeEvent *);


private slots:
	void rotateImage();

private:
    void updateScreenshotLabel();

	QPushButton *screenshotLabel;
    QPixmap originalPixmap;
	QGridLayout *mainLayout;
	QWidget *dockWidgetContents;
	bool forceAspect;

};

class MainWindow : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

	bool isDockTabVisible(QString);
	void setVersion();
	void enableCtrls(bool);

	RomAuditor romAuditor;
	MergedRomAuditor mergedAuditor;
	MameExeRomAuditor *mameAuditor;

	QTreeView *tvGameList;
	QListView *lvGameList;
	
	QLineEdit *lineEditSearch;
	QToolButton *btnSearch, *btnClearSearch;
	QLabel *labelProgress, *labelGameCount, 
		*labelStatus, *labelEmulation, *labelColor, *labelSound, 
		*labelGraphic, *labelCocktail, *labelProtection, *labelSavestate;
	QWidget *wStatus;
	QProgressBar *progressBarGamelist;
	QSystemTrayIcon *trayIcon;
	
	QStringList dockCtrlNames;
	QDockWidget* dockCtrls[DOCK_LAST];
	QTextBrowser *tbHistory, *tbMameinfo, *tbStory, *tbCommand;

public slots:
	void on_actionExitStop_activated();
	void on_actionRefresh_activated();
	void on_actionEnglish_activated();
	void on_actionChinese_PRC_activated();
	void on_actionChinese_Taiwan_activated();
	void on_actionJapanese_activated();
	void on_actionHungarian_activated();
	void on_actionKorean_activated();
	void on_actionBrazilian_activated();
	void on_actionLocalGameList_activated();
	void on_actionReadme_activated();
	void on_actionFAQ_activated();
	void on_actionBoard_activated();
	void on_actionAbout_activated();

	void on_actionPlay_activated();
	void on_actionSavestate_activated();
	void on_actionPlayback_activated();
	void on_actionRecord_activated();
	void on_actionMNG_activated();
	void on_actionAVI_activated();
	void on_actionWave_activated();
	void on_actionConfigIPS_activated();
	void on_actionAudit_activated();
	void on_actionProperties_activated();
	void on_actionSrcProperties_activated();
	void on_actionDefaultOptions_activated();
	void on_actionDirectories_activated();

	void on_actionColDescription_activated();
	void on_actionColName_activated();
	void on_actionColROMs_activated();
	void on_actionColManufacturer_activated();
	void on_actionColDriver_activated();
	void on_actionColYear_activated();
	void on_actionColCloneOf_activated();
	void on_trayIconActivated(QSystemTrayIcon::ActivationReason);

    void log(QString, char logOrigin = 1);
	void poplog(QString);
	void logStatus(QString);
	void logStatus(GameInfo *);
	void init();
	void initSettings();
	void loadLayout();
	void loadSettings();
	void saveSettings();
	void setDockOptions();
	void setGuiStyle(QString = NULL);
	void setBgTile();
	void setBgPixmap(QString = NULL);
	void toggleTrayIcon(int, QProcess::ExitStatus, bool = false);
	
protected:
	void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);

private:
	QString background_file;
	QString gui_style;
	
	void toggleGameListColumn(int);
	void initHistory(int);
	void initSnap(int);
	void showRestartDialog();
	void setTransparentBg(QWidget *);
	void setTransparentStyle(QWidget * w);
	QList<QTabBar *> getSSTabBars();
};

#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 11

// global vars
#define ZIP_EXT ".zip"
#define ICO_EXT ".ico"
#define INI_EXT ".ini"
#define STA_EXT ".sta"
#define INP_EXT ".inp"
#define PNG_EXT ".png"
#define MNG_EXT ".mng"
#define AVI_EXT ".avi"
#define WAV_EXT ".wav"
#define SZIP_EXT ".7z"
#ifdef Q_WS_WIN
#define EXEC_EXT ".exe"
#else
#define EXEC_EXT ""
#endif

extern const QString CFG_PREFIX;

extern MainWindow *win;
extern QSettings guiSettings, defSettings;
extern QString mame_binary;
extern QString language;
extern bool local_game_list;
extern bool isDarkBg;
extern bool sdlInited;

extern bool isSDLPort;
extern bool isMESS;
extern bool hasIPS;
extern bool hasDevices;
extern bool hasLanguage;
extern bool hasDriverCfg;

extern QStringList validGuiSettings;

#endif
