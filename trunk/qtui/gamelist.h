#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QtGui>
#include "audit.h"
#include "utils.h"

enum
{
	GAMELIST_INIT_FULL = 0,
	GAMELIST_INIT_AUDIT,
	GAMELIST_INIT_DIR
};

enum
{
	DOCK_SNAP,
	DOCK_FLYER,
	DOCK_CABINET,
	DOCK_MARQUEE,
	DOCK_TITLE,
	DOCK_CPANEL,
	DOCK_PCB,
	
	DOCK_HISTORY,
	DOCK_MAMEINFO,
	DOCK_STORY,
	DOCK_COMMAND,
	DOCK_LAST
};

class UpdateSelectionThread : public QThread
{
	Q_OBJECT

public:
	MyQueue myqueue;
	QString historyText;
	QString mameinfoText;
	QString storyText;
	QString commandText;

	QByteArray pmSnapData[DOCK_LAST];

	UpdateSelectionThread(QObject *parent = 0);
	~UpdateSelectionThread();

	void update();

signals:
	void snapUpdated(int);

protected:
	void run();

private:
	QMutex mutex;
	bool abort;

	QByteArray getScreenshot(const QString &, const QString &, int);
};

class TreeItem
{
public:
	TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
	~TreeItem();

	void appendChild(TreeItem *child);

	TreeItem *child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	int row() const;
	TreeItem *parent();

private:
	QList<TreeItem*> childItems;
	QList<QVariant> itemData;
	TreeItem *parentItem;
};

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TreeModel(QObject *parent = 0, bool isGroup = true);
	~TreeModel();

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(int column, TreeItem *childItem) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	void updateRow(const QModelIndex &index);

private:
	TreeItem *rootItem;

	TreeItem *getItem(const QModelIndex &index) const;
	TreeItem * buildItem(TreeItem *, QString, bool);
};

class GamelistDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	GamelistDelegate(QObject *parent = 0);

	QSize sizeHint ( const QStyleOptionViewItem & option, 
		const QModelIndex & index ) const;

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index ) const;
};

/*
class XTreeView : public QTreeView
{
Q_OBJECT

public:
    XTreeView(QWidget *parent = 0);
 
protected:
	void paintEvent(QPaintEvent *);
};
//*/

class Gamelist : public QObject
{
	Q_OBJECT

public:
	bool hasInitd;
	QProcess *loadProc;
	QString mameOutputBuf;
	QStringList xmlLines;
	QTime loadTimer;
	int numTotalGames;
	QString mameVersion;
	QMenu *menuContext;
	QMenu *headerMenu;
	QString listMode;

	QStringList folderList;

	// interactive threads used by the game list
	RomAuditor auditor;
	UpdateSelectionThread selectionThread;

	MergedRomAuditor *mAuditor;

	Gamelist(QObject *parent = 0);
	~Gamelist();

	void loadIcon();

public slots:
	void init(bool, int = GAMELIST_INIT_AUDIT);	//the default init value is a hack, for connect slots

	void showContextMenu(const QPoint &);
	void updateContextMenu();
	void showHeaderContextMenu(const QPoint &);
	void updateHeaderContextMenu();
	void restoreGameSelection();

	void loadDefaultIni();
	void runMame(bool = false);
	QString getViewString(const QModelIndex &index, int column) const;
	void updateProgress(int progress);
	void switchProgress(int max, QString title);
	void updateSelection();
	void updateSelection(const QModelIndex & current, const QModelIndex & previous);
	void setupSnap(int);
	void toggleDelegate(bool);

	// external process management
	void loadListXmlReadyReadStandardOutput();
	void loadListXmlFinished(int, QProcess::ExitStatus);
	void loadDefaultIniReadyReadStandardOutput();
	void loadDefaultIniFinished(int, QProcess::ExitStatus);
	void extractMerged(QString, QString);
	void extractMergedFinished(int, QProcess::ExitStatus);
	void runMergedFinished(int, QProcess::ExitStatus);

	void filterSearchCleared();
	void filterSearchChanged();
	void filterFolderChanged(QTreeWidgetItem * = NULL, QTreeWidgetItem * = NULL);

private:
	QString currentTempROM;
	QFutureWatcher<void> loadIconWatcher;
	int loadIconStatus;
	QAbstractItemDelegate *defaultGameListDelegate;
	QStringList extFolders;

	void initFolders();
	void initExtFolders(const QString&, const QString&);
	void initMenus();
	void updateDeviceMenu(QMenu *);
	void loadMMO(int);
	void loadIconWorkder();
	void parse();
	void restoreFolderSelection();

private slots:
	void postLoadIcon();
};

class BiosSet : public QObject
{
public:
	QString description;
	bool isDefault;

	BiosSet(QObject *parent = 0);
};

class RomInfo : public QObject
{
public:
	QString name;
	QString bios;
	quint64 size;
	//quint32 crc is the key
	//md5
	//sha1
	QString merge;
	QString region;
	//offset
	QString status;
	//dispose

	/* internal */
	bool available;

	RomInfo(QObject *parent = 0);
};

class DiskInfo : public QObject
{
public:
	QString name;
	//md5
	//QString sha1 is the key
	QString merge;
	QString region;
	quint8 index;
	QString status;
	//dispose

	/* internal */
	bool available;

	DiskInfo(QObject *parent = 0);
};

class ChipInfo : public QObject
{
public:
	QString name;
	QString tag;
	QString type;
	quint32 clock;

	ChipInfo(QObject *parent = 0);
};

class DisplayInfo : public QObject
{
public:
	QString type;
	QString rotate;
	bool flipx;
	quint16 width;
	quint16 height;
	QString refresh;
//	int pixclock;
	quint16 htotal;
	quint16 hbend;
	quint16 hbstart;
	quint16 vtotal;
	quint16 vbend;
	quint16 vbstart;

	DisplayInfo(QObject *parent = 0);
};

class ControlInfo : public QObject
{
public:
	QString type;
	quint16 minimum;
	quint16 maximum;
	quint16 sensitivity;
	quint16 keydelta;
	bool reverse;

	ControlInfo(QObject *parent = 0);
};

class DeviceInfo : public QObject
{
public:
	QString type;
	QString tag;
	bool mandatory;

//	QString instanceName is the key
	QStringList extensionNames;

	DeviceInfo(QObject *parent = 0);
};

class GameInfo : public QObject
{
public:
	/* game */
	QString sourcefile;
	bool isBios;
//	bool runnable;
	QString cloneof;
	QString romof;
	QString sampleof;
	QString description;
	QString year;
	QString manufacturer;

	/* biosset */
	QHash<QString /*name*/, BiosSet *> biosSets;

	/* rom */
	QHash<quint32 /*crc*/, RomInfo *> roms;

	/* disk */
	QHash<QString /*sha1*/, DiskInfo *> disks;

	/* sample */
	QStringList samples;

	/* chip */
	QList<ChipInfo *> chips;

	/* display */
	QList<DisplayInfo *> displays;

	/* sound */
	quint8 channels;

	/* input */
	bool service;
	bool tilt;
	quint8 players;
	quint8 buttons;
	quint8 coins;
	QList<ControlInfo *> controls;

	//dipswitch 

	/* driver, impossible for a game to have multiple drivers */
	quint8 status;
	quint8 emulation;
	quint8 color;
	quint8 sound;
	quint8 graphic;
	quint8 cocktail;
	quint8 protection;
	quint8 savestate;
	quint32 palettesize;

	/* device */
	QMap<QString /* instanceName */, DeviceInfo *> devices;

	/*ramoption */
	QList<quint32> ramOptions;
	quint32 defaultRamOption;

	/* internal */
	QString lcDesc;
	QString lcMftr;
	QString reading;

	bool isExtRom;
	bool isCloneAvailable;

	qint8 available;
	QByteArray icondata;
	TreeItem *pModItem;
	QSet<QString> clones;

	GameInfo(QObject *parent = 0);
	~GameInfo();

	QString biosof();
	DeviceInfo *getDevice(QString type, int = 0);
	QString getDeviceInstanceName(QString type, int = 0);
};

class MameGame : public QObject
{
Q_OBJECT

public:
	QString mameVersion;
	QString mameDefaultIni;
	QHash<QString, GameInfo *> games;

	MameGame(QObject *parent = 0);
	~MameGame();

	void s11n();
	int des11n();
	void completeData();
};

class GameListSortFilterProxyModel : public QSortFilterProxyModel
{
Q_OBJECT

public:
	QString searchText, filterText;
	QStringList filterList;

	GameListSortFilterProxyModel(QObject *parent = 0);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
//	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

extern MameGame *mameGame;
extern Gamelist *gameList;
extern QString currentGame, currentFolder;
extern QStringList consoleGamesL;

#endif
