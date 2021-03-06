#include <QtXml>

#include "mameopt.h"
#include "prototype.h"
#include "utils.h"
#include "mainwindow.h"
#include "dialogs.h"

#ifdef USE_SDL
#undef main
#include "SDL.h"
#endif /* USE_SDL */

/* global */
OptionUtils *optUtils;

//collection of all MameOptions
QHash<QString, MameOption*> mameOpts;
QByteArray option_column_state;
QByteArray option_geometry;
QString mameIniPath = "";

bool isSDLPort = false;
bool hasLanguage = false;
bool hasIPS = false;
bool hasDevices = false;

/* internal */
OptionDelegate optDelegate(win);

enum
{
	USERROLE_TITLE = 0,
	USERROLE_KEY,
	USERROLE_TYPE,
	USERROLE_MIN,
	USERROLE_MAX,
	USERROLE_DEFAULT,
	USERROLE_VALLIST,
	USERROLE_GUIVALLIST
};

OptionsUI::OptionsUI(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);

	if (optCtrls.isEmpty())
		optCtrls << lvGuiOpt
				 << lvGlobalOpt
				 << lvSourceOpt
				 << lvBiosOpt
				 << lvCloneofOpt
				 << lvCurrOpt;

	connect(buttonBoxDlg, SIGNAL(accepted()), this, SLOT(accept()));
}

void OptionsUI::init(int optLevel, int lstRow)
{
	//init ctlrs
	for (int i = OPTLEVEL_GUI; i < OPTLEVEL_LAST; i++)
		optUtils->chainLoadOptions(NULL, i);

	//select option tab
	tabOptions->setCurrentIndex(optLevel);

	//select row in list on the left
	if (lstRow > -1)
		optCtrls[optLevel]->setCurrentRow(lstRow);
}

void OptionsUI::showEvent(QShowEvent *e)
{
	restoreGeometry(option_geometry);
	e->accept();
}

void OptionsUI::closeEvent(QCloseEvent *event)
{
	option_geometry = saveGeometry();
	event->accept();
}

CsvCfgUI::CsvCfgUI(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
}

void CsvCfgUI::init(QString title, QMap<QString, bool> items)
{
	groupBoxTitle->setTitle(title);

	QStringList keys = items.keys();
	QStringList names;
	foreach (QString key, keys)
	{
		names.append(QString(key).left(2));
	}

	//fixme: remove widgets

	for (int i = 0; i < keys.size(); i++)
	{
		const QString key = keys[i];
//		const QString name = names[i];
		const QString desc = QString(key).remove(0, 3);
		QCheckBox *checkBox = groupBoxTitle->findChild<QCheckBox *>(desc);
		if (checkBox == NULL)
		{
			checkBox = new QCheckBox(groupBoxTitle);
			checkBox->setObjectName(desc);
		}

		checkBox->setText(desc);
		checkBox->setChecked(items[key]);
		layContainer->addWidget(checkBox);
	}
}

QString CsvCfgUI::getCSV()
{
	QString csv = "";

	for (int i = 0; i < layContainer->count(); i++)
	{
		QCheckBox *checkBox = (QCheckBox *)layContainer->itemAt(i)->widget();

		if (checkBox->checkState() == Qt::Checked)
		{
			csv.append(checkBox->objectName());
			csv.append(",");
		}
	}

	if (csv.endsWith(','))
		csv.chop(1);

	return csv;
}


OptInfo::OptInfo(QListWidget *cat, QTreeView *opt, QObject *parent) :
	QObject(parent)
{
	optCatView = cat;
	optView = opt;
	optModel = NULL;
}

ResetWidget::ResetWidget(/*QtProperty *property, */ QWidget *parent) :
	QWidget(parent),
	//m_property(property),
	_textLabel(new QLabel(this)),
	_iconLabel(new QLabel(this)),
	_btnReset(new QToolButton(this)),
	ctrlSpacing(-1),
	optType(0),
	sliderOffset(0)
{
	_textLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
	_iconLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	_btnReset->setToolButtonStyle(Qt::ToolButtonIconOnly);
	_btnReset->setIcon(QIcon(":/res/reset_property.png"));
	_btnReset->setMaximumWidth(24);
	_btnReset->setMinimumWidth(24);
	_btnReset->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
	_btnReset->setToolTip("Reset to default");
	connect(_btnReset, SIGNAL(clicked()), &optDelegate, SLOT(sync()));

	QLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(ctrlSpacing);
	layout->addWidget(_iconLabel);
	layout->addWidget(_textLabel);
	layout->addWidget(_btnReset);
	setFocusProxy(_textLabel);
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

void ResetWidget::setSpacing(int spacing)
{
	ctrlSpacing = spacing;
	layout()->setSpacing(ctrlSpacing);
}

//insert some widgets before reset button
void ResetWidget::setWidget(QWidget *widget, QWidget *widget2, int optType, int sliderOffset)
{
	this->optType = optType;
	this->sliderOffset = sliderOffset;

	//delete the ctrls
	if (_textLabel) {
		delete _textLabel;
		_textLabel = 0;
	}
	if (_iconLabel) {
		delete _iconLabel;
		_iconLabel = 0;
	}
	delete layout();

	QLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	layout->addWidget(widget);
	subWidget = widget;

	//fix save change bug	
	//win->log(QString("optType: %1").arg(optType));
	//QCheckBox *_checkBox;
	//QComboBox *_comboBox
	switch (optType)
	{
		case MAMEOPT_TYPE_BOOL:
			widget->disconnect(SIGNAL(toggled(bool)));
			connect(widget, SIGNAL(toggled(bool)), &optDelegate, SLOT(saveChange(bool)));
			win->log("listen checkbox");
			break;

		case MAMEOPT_TYPE_STRING:
		case MAMEOPT_TYPE_STRING_EDITABLE:
			widget->disconnect(SIGNAL(activated(int)));
			connect(widget, SIGNAL(activated(int)), &optDelegate, SLOT(saveChange(int)));
			win->log("listen combobox");
			break;

		case MAMEOPT_TYPE_INT:
		case MAMEOPT_TYPE_FLOAT:
			widget->disconnect(SIGNAL(sliderMoved(int)));
			connect(widget, SIGNAL(sliderMoved(int)), &optDelegate, SLOT(saveChange(int)));
			win->log("listen combobox");
			break;

		default:
			
			widget->disconnect(SIGNAL(textChanged(const QString&)));
			connect(widget, SIGNAL(textChanged(const QString&)), &optDelegate, SLOT(saveChange(const QString&)));
			win->log("listen unknown");
			break;
	}

	if (widget2)
	{
		layout->addWidget(widget2);
		subWidget2 = widget2;

		switch (optType)
		{
		case MAMEOPT_TYPE_INT:
		case MAMEOPT_TYPE_FLOAT:
			_slider = static_cast<QSlider*>(subWidget);
			_sliderLabel = static_cast<QLabel*>(subWidget2);

			disconnect(_slider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderLabel(int)));
			connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderLabel(int)));
			break;

		case MAMEOPT_TYPE_FILE:
		case MAMEOPT_TYPE_DATFILE:
		case MAMEOPT_TYPE_CFGFILE:
		case MAMEOPT_TYPE_EXEFILE:
		case MAMEOPT_TYPE_DIR:
		case MAMEOPT_TYPE_DIRS:
			_btnSetDlg = static_cast<QToolButton*>(subWidget2);
			_btnSetDlg->disconnect(SIGNAL(clicked()));
			if (optType == MAMEOPT_TYPE_DIRS)
				connect(_btnSetDlg, SIGNAL(clicked()), &optDelegate, SLOT(setDirectories()));
			else if (optType == MAMEOPT_TYPE_DIR)
				connect(_btnSetDlg, SIGNAL(clicked()), &optDelegate, SLOT(setDirectory()));
			else if (optType == MAMEOPT_TYPE_FILE)
				connect(_btnSetDlg, SIGNAL(clicked()), &optDelegate, SLOT(setFile()));
			else if (optType == MAMEOPT_TYPE_DATFILE)
				connect(_btnSetDlg, SIGNAL(clicked()), &optDelegate, SLOT(setDatFile()));
			else if (optType == MAMEOPT_TYPE_CFGFILE)
				connect(_btnSetDlg, SIGNAL(clicked()), &optDelegate, SLOT(setCfgFile()));
			else if (optType == MAMEOPT_TYPE_EXEFILE)
				connect(_btnSetDlg, SIGNAL(clicked()), &optDelegate, SLOT(setExeFile()));
			break;

		case MAMEOPT_TYPE_CSV:
			_btnSetDlg = static_cast<QToolButton*>(subWidget2);
			_btnSetDlg->disconnect(SIGNAL(clicked()));
			connect(_btnSetDlg, SIGNAL(clicked()), &optDelegate, SLOT(setCSV()));

			break;
		}
	}
	layout->addWidget(_btnReset);

	setFocusProxy(widget);
}

void ResetWidget::setResetEnabled(bool enabled)
{
	_btnReset->setEnabled(enabled);
}


/*
void ResetWidget::setValueText(const QString &text)
{
	if (m_textLabel)
		m_textLabel->setText(text);
}

void ResetWidget::setValueIcon(const QIcon &icon)
{
	QPixmap pix = icon.pixmap(QSize(16, 16));
	if (m_iconLabel) {
		m_iconLabel->setVisible(!pix.isNull());
		m_iconLabel->setPixmap(pix);
	}
}
*/

void ResetWidget::slotClicked()
{
	//emit resetProperty(m_property);

//	emit commitData(w);
}

void ResetWidget::updateSliderLabel(int value)
{
	QLabel *ctrl2 = static_cast<QLabel*>(subWidget2);

	double multiplier = 1.0;
	QString format = "%.0f";
	if (optType == MAMEOPT_TYPE_FLOAT)
	{
		multiplier = 100.0;
		format = "%.2f";
	}

	ctrl2->setMaximumWidth(36);
	ctrl2->setMinimumWidth(36);
	ctrl2->setAlignment(Qt::AlignRight);
	ctrl2->setText(QString().sprintf(qPrintable(format), (value - sliderOffset) / multiplier));
}

OptionDelegate::OptionDelegate(QObject *parent)
: QItemDelegate(parent)
{
	isReset = false;
	csvBuf = "";
}

QSize OptionDelegate::sizeHint ( const QStyleOptionViewItem & option,
								const QModelIndex & index ) const
{
	return QSize(60,18);
}

void OptionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt,
						   const QModelIndex &index ) const
{
	QStyleOptionViewItem option = opt;
	QBrush brush;

	if (!optUtils->isTitle(index))
	{
		brush = QBrush(optUtils->inheritColor(index));
		//render option name
		if (optUtils->isChanged(index))
			option.font.setBold(true);

		//render option value
		if (index.column() == 1)
		{
			int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
			switch (optType)
			{
			case MAMEOPT_TYPE_BOOL:
			{
				bool value = index.model()->data(index, Qt::EditRole).toBool();
				QStyleOptionButton ctrl;
				ctrl.state = QStyle::State_Enabled | (value ? QStyle::State_On : QStyle::State_Off);
				ctrl.direction = QApplication::layoutDirection();
				ctrl.rect = option.rect;
				ctrl.fontMetrics = QApplication::fontMetrics();

				if (index.column() == 1)
					option.state &= ~QStyle::State_Selected;

				// fixme: selection background is unwanted
				if (option.state & QStyle::State_Selected)
					painter->fillRect(option.rect, option.palette.highlight());
				else
					painter->fillRect(option.rect, brush);

				QApplication::style()->drawControl(QStyle::CE_CheckBox, &ctrl, painter);
				return;
			}

//fixme: mac doesn't draw the bar very well
#ifndef Q_OS_MAC
			case MAMEOPT_TYPE_INT:
			case MAMEOPT_TYPE_FLOAT:
			{
				double min, max, value;
				min = optUtils->getField(index, USERROLE_MIN).toDouble();
				max = optUtils->getField(index, USERROLE_MAX).toDouble();
				value = index.model()->data(index, Qt::EditRole).toDouble();

				QStyleOptionProgressBar ctrl;

				ctrl.state = QStyle::State_Enabled;
				ctrl.direction = QApplication::layoutDirection();

				QRect rc = option.rect;

				int sliderWidth = rc.width() - 36 - 24; /*reset button+label*/
				rc.setWidth(sliderWidth);
				rc.setY(rc.y() + (int)(rc.height() * 0.1));
				rc.setHeight((int)(rc.height() * 0.8));	//don't occupy full height

				int multiplier = 1;
				int offset = 0;
				QString format = "%.0f";

				if (optType == MAMEOPT_TYPE_FLOAT)
				{
					multiplier = 100;
					format = "%.2f";
				}
				else if (min < 0)
				{
					offset = (int)(0 - min);
					max += offset;
					value += offset;
					min = 0;
				}

				ctrl.rect = rc;
				ctrl.fontMetrics = QApplication::fontMetrics();
				ctrl.minimum = (int)(multiplier * min);
				ctrl.maximum = (int)(multiplier * max);
				ctrl.textAlignment = Qt::AlignRight;
				ctrl.textVisible = false;

				ctrl.progress = (int)(multiplier * value);
				ctrl.text = QString().sprintf(qPrintable(format), value - offset);

				// Draw the progress bar
				QApplication::style()->drawControl(QStyle::CE_ProgressBar, &ctrl, painter);

				rc = option.rect;
				rc.setWidth(rc.width() - 24);
				rc.setY(rc.y() + (int)(rc.height() * 0.1));
				rc.setHeight((int)(rc.height() * 0.8));	//don't occupy full height
				QApplication::style()->drawItemText(painter, rc, Qt::AlignRight | Qt::AlignVCenter, option.palette, true,
					ctrl.text, QPalette::Text);

				return;
			}
#endif /* Q_OS_MAC */
			}
		}
	}
	else
	{
//		option.palette.setColor(QPalette::Text, option.palette.color(QPalette::BrightText));
		option.palette.setColor(QPalette::Text, QColor(0, 21, 110, 255));
		option.font.setBold(true);
//		brush = option.palette.dark();
		brush = QBrush(QColor(221, 231, 238, 255));
	}

//	if (index.column() == 1)
//		option.state &= ~QStyle::State_Selected;

	if (option.state & QStyle::State_Selected)
		painter->fillRect(option.rect, option.palette.highlight());
	else
		painter->fillRect(option.rect, brush);

	QItemDelegate::paint(painter, option, index);
	return;
}

QWidget *OptionDelegate::createEditor(QWidget *parent,
									  const QStyleOptionViewItem & option,
									  const QModelIndex & index) const
{
	if (index.column() == 0 || optUtils->isTitle(index))
		return 0;

	ResetWidget *resetWidget = new ResetWidget(parent);

	int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
	switch (optType)
	{
	case MAMEOPT_TYPE_BOOL:
	{
		QCheckBox *ctrl = new QCheckBox(parent);

		resetWidget->setWidget(ctrl, NULL, optType);
		return resetWidget;
	}

	case MAMEOPT_TYPE_INT:
	case MAMEOPT_TYPE_FLOAT:
	{
		double min, max;
		min = optUtils->getField(index, USERROLE_MIN).toDouble();
		max = optUtils->getField(index, USERROLE_MAX).toDouble();

		int multiplier = 1;
		int offset = 0;
		if (optType == MAMEOPT_TYPE_FLOAT)
			multiplier = 100;
		else if (min < 0)
		{
			offset = (int)(0 - min);
			max += offset;
			min = 0;
		}

		QSlider *ctrl = new QSlider(Qt::Horizontal, parent);
		ctrl->setStyleSheet("background-color: white;");//fixme: hack
		ctrl->setMinimum((int)(min * multiplier));
		ctrl->setMaximum((int)(max * multiplier));

		QLabel *ctrl2 = new QLabel(parent);
		ctrl2->setAlignment(Qt::AlignHCenter);
		ctrl2->setStyleSheet("background-color: white;");//fixme: hack

		resetWidget->setWidget(ctrl, ctrl2, optType, offset);
		return resetWidget;
	}

	case MAMEOPT_TYPE_FILE:
	case MAMEOPT_TYPE_DATFILE:
	case MAMEOPT_TYPE_CFGFILE:
	case MAMEOPT_TYPE_EXEFILE:
	case MAMEOPT_TYPE_DIR:
	case MAMEOPT_TYPE_DIRS:
	{
		QLineEdit *ctrl = new QLineEdit(resetWidget);

		QToolButton *ctrl2 = new QToolButton(resetWidget);
		ctrl2->setText("...");

		resetWidget->setWidget(ctrl, ctrl2, optType);
		return resetWidget;
	}

	case MAMEOPT_TYPE_CSV:
	{
		QLineEdit *ctrl = new QLineEdit(resetWidget);
		ctrl->setReadOnly(true);

		QToolButton *ctrl2 = new QToolButton(resetWidget);
		ctrl2->setText("...");

		resetWidget->setWidget(ctrl, ctrl2, optType);
		return resetWidget;
	}

	case MAMEOPT_TYPE_STRING:
	case MAMEOPT_TYPE_STRING_EDITABLE:
	{
		QList<QVariant> guivalues = optUtils->getField(index, USERROLE_GUIVALLIST).toList();
		if (guivalues.size() > 0)
		{
			QComboBox *ctrl = new QComboBox(resetWidget);
			if (optType == MAMEOPT_TYPE_STRING_EDITABLE)
				ctrl->setEditable(true);
			//	ctrl->installEventFilter(const_cast<OptionDelegate*>(this));
			foreach (QVariant guivalue, guivalues)
				ctrl->addItem(guivalue.toString());

			resetWidget->setWidget(ctrl, NULL, optType);
			return resetWidget;
		}
		//fall to default
	}
	default:
	{
		QLineEdit *ctrl = new QLineEdit(resetWidget);
		resetWidget->setWidget(ctrl);
		return resetWidget;
	}
	}
}

void OptionDelegate::setEditorData(QWidget *editor,
								   const QModelIndex &index) const
{
	if (index.column() == 0)
		return;

	ResetWidget *resetWidget = static_cast<ResetWidget*>(editor);
	if (optUtils->isChanged(index))
		resetWidget->setResetEnabled(true);
	else
		resetWidget->setResetEnabled(false);

	int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
	switch (optType)
	{
	case MAMEOPT_TYPE_BOOL:
	{
		bool value = index.model()->data(index, Qt::EditRole).toBool();
		QCheckBox *ctrl = static_cast<QCheckBox*>(resetWidget->subWidget);
		ctrl->setCheckState(value ? Qt::Checked : Qt::Unchecked);
		break;
	}

	case MAMEOPT_TYPE_INT:
	case MAMEOPT_TYPE_FLOAT:
	{
		double min, value;
		min = optUtils->getField(index, USERROLE_MIN).toDouble();
		value = index.model()->data(index, Qt::EditRole).toDouble();

		int multiplier = 1;
		int offset = 0;
		if (optType == MAMEOPT_TYPE_FLOAT)
			multiplier = 100;
		else if (min < 0)
		{
			offset = (int)(0 - min);
			value += offset;
		}

		QSlider *ctrl = static_cast<QSlider*>(resetWidget->subWidget);
		ctrl->setPageStep(2);
		int intVal = ((int)((value + 0.001) * multiplier));	//hack: precision fix
		ctrl->setValue(intVal);
//		win->log(QString("val: %1").arg(intVal));
		resetWidget->updateSliderLabel(intVal);
		break;
	}

	case MAMEOPT_TYPE_CSV:
	{
		QString guivalue = index.model()->data(index, Qt::DisplayRole).toString();

		QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);
		ctrl->setText(guivalue);
		break;
	}

	case MAMEOPT_TYPE_STRING:
	case MAMEOPT_TYPE_STRING_EDITABLE:
	{
		QString guivalue = index.model()->data(index, Qt::DisplayRole).toString();

		QList<QVariant> guivalues = optUtils->getField(index, USERROLE_GUIVALLIST).toList();
		if (guivalues.size() > 0)
		{
			QComboBox *ctrl = static_cast<QComboBox*>(resetWidget->subWidget);
			ctrl->setCurrentIndex(guivalues.indexOf(guivalue));
			break;
		}
	}

	//file, dir, dirs case also here
	default:
	{
		QString guivalue = index.model()->data(index, Qt::DisplayRole).toString();

		QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);
		ctrl->setText(guivalue);
//			QItemDelegate::setEditorData(editor, index);
		break;
	}
	}
}

// override setModelData()
void OptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
								  const QModelIndex &index) const
{
	if (index.column() == 0)
		return;

	ResetWidget *resetWidget = static_cast<ResetWidget*>(editor);
	int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
	int optLevel = index.model()->objectName().remove("optModel").toInt();
	QString optName = optUtils->getField(index, USERROLE_KEY).toString();
	MameOption *pMameOpt = mameOpts[optName];

	/* update control's display value */
	QVariant dispValue;

	//reset to default value
	if (isReset)
	{
		switch (optLevel)
		{
		case OPTLEVEL_GUI:
		case OPTLEVEL_GLOBAL:
			dispValue = pMameOpt->defvalue;
			break;

		case OPTLEVEL_SRC:
			dispValue = pMameOpt->globalvalue;
			break;

		case OPTLEVEL_BIOS:
			dispValue = pMameOpt->srcvalue;
			break;

		case OPTLEVEL_CLONEOF:
			dispValue = pMameOpt->biosvalue;
			break;

		case OPTLEVEL_CURR:
		default:
			dispValue = pMameOpt->cloneofvalue;
		}

		dispValue = optUtils->getLongValue(optName, dispValue.toString());
	}
	//set new value
	else
	{
		switch (optType)
		{
		case MAMEOPT_TYPE_BOOL:
		{
			QCheckBox *ctrl = static_cast<QCheckBox*>(resetWidget->subWidget);
			dispValue = (ctrl->checkState() == Qt::Checked) ? true : false;
			break;
		}
		case MAMEOPT_TYPE_INT:
		case MAMEOPT_TYPE_FLOAT:
		{
			double min, value;
			min = optUtils->getField(index, USERROLE_MIN).toDouble();
			value = index.model()->data(index, Qt::EditRole).toDouble();

			double multiplier = 1.0;
			int offset = 0;
			if (optType == MAMEOPT_TYPE_FLOAT)
				multiplier = 100.0;
			else if (min < 0)
			{
				offset = (int)(0 - min);
				value += offset;
			}

			QSlider *ctrl = static_cast<QSlider*>(resetWidget->subWidget);
			// ensure .00 display for MAMEOPT_TYPE_FLOAT

			dispValue = optUtils->getLongValue(optName, QString::number((ctrl->value() - offset) / multiplier));
			break;
		}
		case MAMEOPT_TYPE_STRING:
		case MAMEOPT_TYPE_STRING_EDITABLE:
		{
			QList<QVariant> guiValues = optUtils->getField(index, USERROLE_GUIVALLIST).toList();
			if (guiValues.size() > 0)
			{
				QComboBox *ctrl = static_cast<QComboBox*>(resetWidget->subWidget);
				dispValue = ctrl->currentText();
				break;
			}
		}
		default:
		{
			QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);

			// set file dialog result
			if (!csvBuf.isEmpty())
				dispValue = csvBuf;
			else
				dispValue = ctrl->text();
//			QItemDelegate::setModelData(editor, model, index);
		}
		}
	}

	model->setData(index, dispValue);

	// process inherited value overrides
	QString prevVal;
	QAbstractItemModel *itemModel;
	QString iniFileName;
	GameInfo *gameInfo = pMameDat->games[currentGame];

	switch (optLevel)
	{
	case OPTLEVEL_GUI:
	case OPTLEVEL_GLOBAL:
		iniFileName = mameIniPath + (isMESS ? "mess" INI_EXT : (isUME ? "ume" INI_EXT : "mame" INI_EXT));

		//save old value
		prevVal = pMameOpt->globalvalue;

		//assign new value
		pMameOpt->globalvalue = optUtils->getShortValue(optName, dispValue.toString());

		//special case for console dirs
		if (pMameOpt->guivisible && optName.endsWith("_extra_software"))
			mameOpts[optName]->globalvalue = pMameOpt->globalvalue;

		if (optLevel == OPTLEVEL_GUI)
			itemModel = win->optionsUI->treeGuiOpt->model();
		else
			itemModel = win->optionsUI->treeGlobalOpt->model();

		itemModel->setData(itemModel->index(index.row(), 1), dispValue);
		//fall to next case

	case OPTLEVEL_SRC:
		if (iniFileName.isNull())
		{
			iniFileName = gameInfo->sourcefile;
			iniFileName.replace(".c", INI_EXT);
			iniFileName = mameIniPath + "ini/source/" + iniFileName;
		}

		// prevent overwrite prevVal from prev case
		if (optLevel == OPTLEVEL_SRC)
			prevVal = pMameOpt->srcvalue;

		if (pMameOpt->srcvalue == prevVal)
		{
			pMameOpt->srcvalue = optUtils->getShortValue(optName, dispValue.toString());

			itemModel = win->optionsUI->treeSourceOpt->model();
			itemModel->setData(itemModel->index(index.row(), 1), dispValue);
			// fall to next case
		}
		else
			break;

	case OPTLEVEL_BIOS:
		if (iniFileName.isNull())
			iniFileName = mameIniPath + "ini/" + gameInfo->biosof() + INI_EXT;

		if (optLevel == OPTLEVEL_BIOS)
			prevVal = pMameOpt->biosvalue;

		if (pMameOpt->biosvalue == prevVal)
		{
			pMameOpt->biosvalue = optUtils->getShortValue(optName, dispValue.toString());

			itemModel = win->optionsUI->treeBiosOpt->model();
			itemModel->setData(itemModel->index(index.row(), 1), dispValue);
			// fall to next case
		}
		else
			break;

	case OPTLEVEL_CLONEOF:
		if (iniFileName.isNull())
			iniFileName = mameIniPath + "ini/" + gameInfo->cloneof + INI_EXT;

		if (optLevel == OPTLEVEL_CLONEOF)
			prevVal = pMameOpt->cloneofvalue;

		if (pMameOpt->cloneofvalue == prevVal)
		{
			pMameOpt->cloneofvalue = optUtils->getShortValue(optName, dispValue.toString());

			itemModel = win->optionsUI->treeCloneofOpt->model();
			itemModel->setData(itemModel->index(index.row(), 1), dispValue);
			// fall to next case
		}
		else
			break;

	case OPTLEVEL_CURR:
		if (iniFileName.isNull())
		{
			// special case for consoles
			if (gameInfo->isExtRom)
				iniFileName = mameIniPath + "ini/" + gameInfo->romof + INI_EXT;
			else
				iniFileName = mameIniPath + "ini/" + currentGame + INI_EXT;
		}

		if (optLevel == OPTLEVEL_CURR)
			prevVal = pMameOpt->currvalue;

		if (pMameOpt->currvalue == prevVal)
		{
			pMameOpt->currvalue = optUtils->getShortValue(optName, dispValue.toString());

			itemModel = win->optionsUI->treeCurrOpt->model();
			itemModel->setData(itemModel->index(index.row(), 1), dispValue);
		}
		break;
	}

	//special case for screen, also updates resolution
	if (optName.startsWith("screen"))
	{
		QString optName2 = optName;
		optName2.replace("screen", "resolution");
		optUtils->updateSelectableItems(optName2);
	}

	optUtils->saveIniFile(optLevel, iniFileName);

	if (optLevel == OPTLEVEL_GUI)
		win->saveSettings();

	//special case for driver_config
	if (pMameOpt->globalvalue != prevVal)
	{
		bool needReload = false;

		if (optName == "driver_config")
			needReload = true;
		else if (optName == "mame_binary")
		{
			QString _mame_binary = mameOpts["mame_binary"]->globalvalue;

			if (mame_binary != _mame_binary)
			{
				mame_binary = _mame_binary;
				needReload = true;
			}
		}

		if (needReload)
		{
			//avoid accepted() SIGNAL
			win->optionsUI->hide();
			gameList->disableCtrls();

			pTempDat = pMameDat;
			pMameDat = new MameDat(0, 1);
		}
	}
}

void OptionDelegate::updateEditorGeometry(QWidget *editor,
										  const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

void OptionDelegate::sync()
{
	ResetWidget *w = qobject_cast<ResetWidget*>(sender()->parent());
	if (w == NULL)
		return;
	isReset = true;
	emit commitData(w);
	emit closeEditor(w, QAbstractItemDelegate::EditNextItem);
	isReset = false;
}

void OptionDelegate::saveChange(bool b)
{
	ResetWidget *w = qobject_cast<ResetWidget*>(sender()->parent());
	win->log("saveChange");
	if (w == NULL)
		return;
	emit commitData(w);
}

void OptionDelegate::saveChange(int index)
{
	ResetWidget *w = qobject_cast<ResetWidget*>(sender()->parent());
	win->log("saveChange");
	if (w == NULL)
		return;
	emit commitData(w);
}

void OptionDelegate::saveChange(const QString &)
{
	ResetWidget *w = qobject_cast<ResetWidget*>(sender()->parent());
	win->log("saveChange");
	if (w == NULL)
		return;
	emit commitData(w);
}

//save options when dialog is closed
void OptionDelegate::setChangesAccepted()
{
	emit commitData(rWidget);
}

void OptionDelegate::setCSV()
{
	rWidget = qobject_cast<ResetWidget*>(sender()->parent());
	if (rWidget == NULL)
		return;

	disconnect(win->csvCfgUI, SIGNAL(accepted()), this, SLOT(setCSVAccepted()));
	connect(win->csvCfgUI, SIGNAL(accepted()), this, SLOT(setCSVAccepted()));

	QMap<QString, bool> items;

	//fixme: the following is not generic
	if (!mameOpts.contains("driver_config"))
		return;

	QStringList driverStrings = mameOpts["driver_config"]->globalvalue.split(',');

	items.insert("c0_mame", driverStrings.contains("mame"));
	items.insert("c1_plus", driverStrings.contains("plus"));
	items.insert("c2_homebrew", driverStrings.contains("homebrew"));
	items.insert("c3_decrypted", driverStrings.contains("decrypted"));
	items.insert("c4_console", driverStrings.contains("console"));

	win->csvCfgUI->init(tr("Driver Config"), items);
	win->csvCfgUI->exec();
}

void OptionDelegate::setCSVAccepted()
{
	csvBuf = win->csvCfgUI->getCSV();
	emit commitData(rWidget);
	csvBuf.clear();
}

void OptionDelegate::setDirectories()
{
	rWidget = qobject_cast<ResetWidget*>(sender()->parent());
	if (rWidget == NULL)
		return;

	QLineEdit *ctrl = static_cast<QLineEdit*>(rWidget->subWidget);

	disconnect(win->dirsUI, SIGNAL(accepted()), this, SLOT(setDirectoriesAccepted()));
	connect(win->dirsUI, SIGNAL(accepted()), this, SLOT(setDirectoriesAccepted()));

	//take existing dir
	win->dirsUI->init(ctrl->text());
	win->dirsUI->exec();
}

void OptionDelegate::setDirectoriesAccepted()
{
	csvBuf = win->dirsUI->getDirs();
	emit commitData(rWidget);
	csvBuf.clear();
}

//todo: merge with setFile
void OptionDelegate::setDirectory()
{
	ResetWidget *resetWidget = qobject_cast<ResetWidget*>(sender()->parent());
	if (resetWidget == NULL)
		return;

	QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);

	//take existing dir
	QString initPath = ctrl->text();
	QFileInfo fi(initPath);
	if (!fi.exists())
	{
		//take mame_binary dir
		fi.setFile(mame_binary);
		initPath = fi.absolutePath();
	}

	QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
	QString directory = QFileDialog::getExistingDirectory(resetWidget,
								tr("Directory name:"),
								initPath,
								options);
	if (!directory.isEmpty())
	{
		csvBuf = directory;
		emit commitData(resetWidget);
		csvBuf.clear();
	}
}

void OptionDelegate::setFile(QString filter, ResetWidget *resetWidget)
{
//	win->log(filter);

	if (resetWidget == NULL)
		resetWidget = qobject_cast<ResetWidget*>(sender()->parent());

	if (resetWidget == NULL)
		return;

	QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);

	//take existing dir
	QString initPath = ctrl->text();
	QFileInfo fi(initPath);
	if (!fi.exists())
	{
		//take mame_binary dir
		fi.setFile(mame_binary);
	}
	initPath = fi.absolutePath();

	if (!filter.isEmpty())
		filter.append(";;");
	filter.append(tr("All Files") + " (*)");
//	win->log(filter);

	QString fileName = QFileDialog::getOpenFileName
		(resetWidget, tr("File name:"), initPath, filter);

	if (!fileName.isEmpty())
	{
		csvBuf = fileName;
		emit commitData(resetWidget);
		csvBuf.clear();
	}
}

void OptionDelegate::setDatFile()
{
	ResetWidget *resetWidget = qobject_cast<ResetWidget*>(sender()->parent());
	setFile(tr("Dat files") + " (*.dat)", resetWidget);
}

void OptionDelegate::setExeFile()
{
	ResetWidget *resetWidget = qobject_cast<ResetWidget*>(sender()->parent());
	setFile(tr("Executable files") + " (*" EXEC_EXT ")", resetWidget);
}

void OptionDelegate::setCfgFile()
{
	ResetWidget *resetWidget = qobject_cast<ResetWidget*>(sender()->parent());
	setFile(tr("Config files") + " (*.cfg)", resetWidget);
}


// parse listxml and init default mame opts
class OptionXMLHandler : public QXmlDefaultHandler
{
private:
	MameOption *pMameOpt;
	QString currentText;
	QString OSType;
	bool metRootTag;

public:
	OptionXMLHandler(int d = 0)
	{
		pMameOpt = NULL;
		metRootTag = false;
	}

	bool startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &attributes)
	{
		if (!metRootTag && qName != "optiontemplate")
			return false;

		if (qName == "optiontemplate")
		{
			metRootTag = true;
		}
		else if (qName == "option")
		{
			QString optName = attributes.value("name");

			//add GUI options to mameOpts
			bool guivisible = attributes.value("guivisible") == "1";
			if (guivisible)
			{
				pMameOpt = new MameOption(0);	//fixme parent
				pMameOpt->defvalue = attributes.value("default");
				mameOpts[optName] = pMameOpt;
			}

			//add more info to exisiting core mameOpts
			if (mameOpts.contains(optName))
			{
				pMameOpt = mameOpts[optName];

				pMameOpt->guiname = attributes.value("guiname");
				pMameOpt->max = attributes.value("max");
				pMameOpt->min = attributes.value("min");

				QString type = attributes.value("type");
				if (type == "string")
					pMameOpt->type = MAMEOPT_TYPE_STRING;
				else if (type == "stringeditable")
					pMameOpt->type = MAMEOPT_TYPE_STRING_EDITABLE;
				else if (type == "file")
					pMameOpt->type = MAMEOPT_TYPE_FILE;
				else if (type == "datfile")
					pMameOpt->type = MAMEOPT_TYPE_DATFILE;
				else if (type == "exefile")
					pMameOpt->type = MAMEOPT_TYPE_EXEFILE;
				else if (type == "cfgfile")
					pMameOpt->type = MAMEOPT_TYPE_CFGFILE;
				else if (type == "dir")
					pMameOpt->type = MAMEOPT_TYPE_DIR;
				else if (type == "dirs")
					pMameOpt->type = MAMEOPT_TYPE_DIRS;
				else if (type == "int")
					pMameOpt->type = MAMEOPT_TYPE_INT;
				else if (type == "float")
					pMameOpt->type = MAMEOPT_TYPE_FLOAT;
				else if (type == "bool")
					pMameOpt->type = MAMEOPT_TYPE_BOOL;
				else if (type == "csv")
					pMameOpt->type = MAMEOPT_TYPE_CSV;
				else
					pMameOpt->type = MAMEOPT_TYPE_UNKNOWN;

				pMameOpt->guivisible = guivisible;
				pMameOpt->globalvisible = attributes.value("globalvisible") != "0";
				pMameOpt->srcvisible = attributes.value("srcvisible") != "0";
				pMameOpt->biosvisible = attributes.value("biosvisible") != "0";
				pMameOpt->cloneofvisible = attributes.value("cloneofvisible") != "0";
				pMameOpt->gamevisible = attributes.value("gamevisible") != "0";

				mameOpts[attributes.value("name")] = pMameOpt;
			}
			else
				pMameOpt = NULL;
		}
		else if (qName == "value")
		{
			OSType = attributes.value("os");
			if (pMameOpt != NULL && (OSType.isEmpty() ||
				OSType == (isSDLPort ? "sdl" : "win")))
				pMameOpt->guivalues << attributes.value("guivalue");
		}

		currentText.clear();
		return true;
	}

	bool endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName)
	{
		if (pMameOpt != NULL)
		{
			if (qName == "description")
				pMameOpt->description = currentText;
			else if (qName == "value")
			{
				if (OSType.isEmpty() || OSType == (isSDLPort ? "sdl" : "win"))
					pMameOpt->values << currentText;
			}
		}
		return true;
	}

	bool characters(const QString &str)
	{
		currentText += str;
		return true;
	}
};


MameOption::MameOption(QObject *parent)
: QObject(parent)
{
	guivisible = false;
	globalvisible = srcvisible = biosvisible = cloneofvisible = gamevisible = true;
}


OptionUtils::OptionUtils(QObject *parent)
: QObject(parent)
{
}

void OptionUtils::init()
{
	//assign unique ctlrs for each level of options
	optInfos = (QList <OptInfo *>()
			<< new OptInfo(win->optionsUI->lvGuiOpt, win->optionsUI->treeGuiOpt, win->optionsUI)
			<< new OptInfo(win->optionsUI->lvGlobalOpt, win->optionsUI->treeGlobalOpt, win->optionsUI)
			<< new OptInfo(win->optionsUI->lvSourceOpt, win->optionsUI->treeSourceOpt, win->optionsUI)
			<< new OptInfo(win->optionsUI->lvBiosOpt, win->optionsUI->treeBiosOpt, win->optionsUI)
			<< new OptInfo(win->optionsUI->lvCloneofOpt, win->optionsUI->treeCloneofOpt, win->optionsUI)
			<< new OptInfo(win->optionsUI->lvCurrOpt, win->optionsUI->treeCurrOpt, win->optionsUI));

	//init category list ctlrs for each level of options
	for (int optLevel = OPTLEVEL_GUI; optLevel < OPTLEVEL_LAST; optLevel++)
	{
		QListWidget *optCatView = optInfos[optLevel]->optCatView;

		if (optLevel == OPTLEVEL_GUI)
		{
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/folder.png"), tr("GUI Paths"), optCatView));
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/folder.png"), tr("MAME Paths"), optCatView));
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/folder.png"), tr("MESS Paths"), optCatView));
		}
		else
		{
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-display.png"), tr("Core Video"), optCatView));
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-osd.png"), tr("OSD Video"), optCatView));
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-display-blue.png"), tr("Screen"), optCatView));
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/audio-x-generic.png"), tr("Audio"), optCatView));
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/input-gaming.png"), tr("Control"), optCatView));
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-vector.png"), tr("Vector"), optCatView));
			optCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/applications-system.png"), tr("Misc"), optCatView));
		}
		const QSize szItem(1, 32);
		for (int i = 0; i < optCatView->count(); i ++)
			optCatView->item(i)->setSizeHint(szItem);

		optCatView->setIconSize(QSize(24, 24));
		optCatView->setMaximumWidth(130);

		connect(optInfos[optLevel]->optView->header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(updateHeaderSize(int, int, int)));
	}

	for (int i = OPTLEVEL_GUI; i < OPTLEVEL_LAST; i++)
		connect(win->optionsUI->optCtrls[i], SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(chainLoadOptions(QListWidgetItem *)));

	connect(win->optionsUI, SIGNAL(accepted()), &optDelegate, SLOT(setChangesAccepted()));
	connect(win->optionsUI->tabOptions, SIGNAL(currentChanged(int)), this, SLOT(chainLoadOptions()));
}

QVariant OptionUtils::getField(const QModelIndex &index, int field)
{
	QModelIndex i = index.sibling(index.row(), 0);
	QString optname = index.model()->data(i, Qt::UserRole + USERROLE_KEY).toString();

	if (mameOpts.contains(optname))
	{
		switch (field)
		{
		case USERROLE_KEY:
			return optname;

		case USERROLE_TYPE:
			return mameOpts[optname]->type;

		case USERROLE_MIN:
			return mameOpts[optname]->min;

		case USERROLE_MAX:
			return mameOpts[optname]->max;

		case USERROLE_DEFAULT:
			return mameOpts[optname]->defvalue;

		case USERROLE_VALLIST:
			return QVariant(mameOpts[optname]->values);

		case USERROLE_GUIVALLIST:
			return QVariant(mameOpts[optname]->guivalues);
		}
	}

	return QVariant();
}

const QString OptionUtils::getLongName(QString optname)
{
	const QString longname = mameOpts[optname]->guiname;
	if (longname.isEmpty())
		return utils->capitalizeStr(optname);

	return longname;
}

const QString OptionUtils::getLongValue(const QString &optname, const QString &optval)
{
	const MameOption *pMameOpt = mameOpts[optname];

	if (pMameOpt->type == MAMEOPT_TYPE_BOOL)
		return (optval=="0") ? "false" : "true";

	else if (pMameOpt->type == MAMEOPT_TYPE_FLOAT)
		return QString().sprintf("%.2f", optval.toFloat());

	else if (pMameOpt->type == MAMEOPT_TYPE_STRING
		  || pMameOpt->type == MAMEOPT_TYPE_STRING_EDITABLE)
	{
		//fill value or GUI value desc
		int i = pMameOpt->values.indexOf(optval);
		if ( i > -1)
			return pMameOpt->guivalues[i];
	}

	return optval;
}

const QString OptionUtils::getShortValue(const QString &optName, const QString &optval)
{
	const MameOption *pMameOpt = mameOpts[optName];

	if (pMameOpt->type == MAMEOPT_TYPE_BOOL)
		return (optval=="true") ? "1" : "0";

	else if (pMameOpt->type == MAMEOPT_TYPE_STRING
		  || pMameOpt->type == MAMEOPT_TYPE_STRING_EDITABLE)
	{
		//fill value or GUI value desc
		int i = pMameOpt->guivalues.indexOf(optval);
		if ( i > -1)
			return pMameOpt->values[i];
	}

	return optval;
}

QColor OptionUtils::inheritColor(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 1);
	QString optName = getField(index, USERROLE_KEY).toString();
	QString dispVal = index.model()->data(i).toString();

	QString compVal;
	MameOption *pMameOpt = mameOpts[optName];

	// return a different color if value is not default
	if (dispVal == getLongValue(optName, pMameOpt->defvalue))
		return Qt::transparent;
	else
		return QColor(255, 255, 0, 64);
}

bool OptionUtils::isChanged(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 1);
	int optLevel = index.model()->objectName().remove("optModel").toInt();
	QString optName = getField(index, USERROLE_KEY).toString();
	QString dispVal = index.model()->data(i).toString();

	QString compVal;
	MameOption *pMameOpt = mameOpts[optName];
	switch (optLevel)
	{
	case OPTLEVEL_GUI:
	case OPTLEVEL_GLOBAL:
		compVal = pMameOpt->defvalue;
		break;

	case OPTLEVEL_SRC:
		compVal = pMameOpt->globalvalue;
		break;

	case OPTLEVEL_BIOS:
		compVal = pMameOpt->srcvalue;
		break;

	case OPTLEVEL_CLONEOF:
		compVal = pMameOpt->biosvalue;
		break;

	case OPTLEVEL_CURR:
		compVal = pMameOpt->cloneofvalue;
		break;
	}

	compVal = getLongValue(optName, compVal);

	if (dispVal != compVal)
		return true;
	else
		return false;
}

bool OptionUtils::isTitle(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 0);
	return index.model()->data(i, Qt::UserRole + USERROLE_TITLE).toString() == "OPTIONTITLE";
}

void OptionUtils::loadDefault(QString text)
{
//the following is for Qt translation
#if 0
	QStringList optList = (QStringList()
		<< QT_TR_NOOP("cabinet directory")
		<< QT_TR_NOOP("control panel directory")
		<< QT_TR_NOOP("flyer directory")
		<< QT_TR_NOOP("marquee directory")
		<< QT_TR_NOOP("pcb directory")
		<< QT_TR_NOOP("title directory")
		<< QT_TR_NOOP("icons directory")
		<< QT_TR_NOOP("background directory")
		<< QT_TR_NOOP("external folder list")

		<< QT_TR_NOOP("m1 directory")
		<< QT_TR_NOOP("history file")
		<< QT_TR_NOOP("story file")
		<< QT_TR_NOOP("mameinfo file")
		<< QT_TR_NOOP("mame binary")

		<< QT_TR_NOOP("driver config")

		<< QT_TR_NOOP("readconfig")

		<< QT_TR_NOOP("romsets directory")
		<< QT_TR_NOOP("hash files directory")
		<< QT_TR_NOOP("samplesets directory")
		<< QT_TR_NOOP("artwork files directory")
		<< QT_TR_NOOP("controller definitions directory")
		<< QT_TR_NOOP("ini files directory")
		<< QT_TR_NOOP("font files directory")
		<< QT_TR_NOOP("cheat files directory")
		<< QT_TR_NOOP("crosshair files directory")
		<< QT_TR_NOOP("language files directory")
		<< QT_TR_NOOP("localized directory")
		<< QT_TR_NOOP("ips files directory")

		<< QT_TR_NOOP("cfg directory")
		<< QT_TR_NOOP("nvram directory")
		<< QT_TR_NOOP("memcard directory")
		<< QT_TR_NOOP("input directory")
		<< QT_TR_NOOP("state directory")
		<< QT_TR_NOOP("snapshot directory")
		<< QT_TR_NOOP("diff directory")
		<< QT_TR_NOOP("comment directory")
		<< QT_TR_NOOP("hiscore directory")

		<< QT_TR_NOOP("command file")
		<< QT_TR_NOOP("hiscore file")

		<< QT_TR_NOOP("auto restore and save")
		<< QT_TR_NOOP("snapshot/movie pattern")
		<< QT_TR_NOOP("snapshot/movie resolution")
		<< QT_TR_NOOP("snapshot/movie view")
		<< QT_TR_NOOP("create burn-in snapshots")

		<< QT_TR_NOOP("auto frame skipping")
		<< QT_TR_NOOP("frame skipping")
		<< QT_TR_NOOP("seconds to run")
		<< QT_TR_NOOP("throttle")
		<< QT_TR_NOOP("sleep when possible")
		<< QT_TR_NOOP("gameplay speed")
		<< QT_TR_NOOP("auto refresh speed")

		<< QT_TR_NOOP("rotate")
		<< QT_TR_NOOP("rotate clockwise")
		<< QT_TR_NOOP("rotate anti-clockwise")
		<< QT_TR_NOOP("auto rotate clockwise")
		<< QT_TR_NOOP("auto rotate anti-clockwise")
		<< QT_TR_NOOP("flip screen left-right")
		<< QT_TR_NOOP("flip screen upside-down")

		<< QT_TR_NOOP("crop artwork")
		<< QT_TR_NOOP("use backdrops")
		<< QT_TR_NOOP("use overlays")
		<< QT_TR_NOOP("use bezels")

		<< QT_TR_NOOP("brightness correction")
		<< QT_TR_NOOP("contrast correction")
		<< QT_TR_NOOP("gamma correction")
		<< QT_TR_NOOP("pause brightness")
		<< QT_TR_NOOP("image enhancement")

		<< QT_TR_NOOP("draw antialiased vectors")
		<< QT_TR_NOOP("beam width")
		<< QT_TR_NOOP("flicker")

		<< QT_TR_NOOP("sound output method")
		<< QT_TR_NOOP("sample rate")

		<< QT_TR_NOOP("use samples")
		<< QT_TR_NOOP("volume attenuation")
		<< QT_TR_NOOP("use volume auto adjust")

		<< QT_TR_NOOP("coin lockout")
		<< QT_TR_NOOP("default input layout")
		<< QT_TR_NOOP("enable mouse input")
		<< QT_TR_NOOP("enable joystick input")
		<< QT_TR_NOOP("enable lightgun input")
		<< QT_TR_NOOP("enable multiple keyboards")
		<< QT_TR_NOOP("enable multiple mice")
		<< QT_TR_NOOP("enable steadykey support")
		<< QT_TR_NOOP("offscreen shots reload")
		<< QT_TR_NOOP("joystick map")
		<< QT_TR_NOOP("joystick deadzone")
		<< QT_TR_NOOP("joystick saturation")

		<< QT_TR_NOOP("paddle device")
		<< QT_TR_NOOP("adstick device")
		<< QT_TR_NOOP("pedal device")
		<< QT_TR_NOOP("dial device")
		<< QT_TR_NOOP("trackball device")
		<< QT_TR_NOOP("lightgun device")
		<< QT_TR_NOOP("positional device")
		<< QT_TR_NOOP("mouse device")

		<< QT_TR_NOOP("log")
		<< QT_TR_NOOP("verbose")
		<< QT_TR_NOOP("update in pause")

		<< QT_TR_NOOP("bios")
		<< QT_TR_NOOP("enable game cheats")
		<< QT_TR_NOOP("skip game info")
		<< QT_TR_NOOP("ips")
		<< QT_TR_NOOP("quit game with confirmation")
		<< QT_TR_NOOP("auto pause when playback is finished")

		<< QT_TR_NOOP("transparent in-game ui")
		<< QT_TR_NOOP("in-game ui transparency")

		<< QT_TR_NOOP("font blank")
		<< QT_TR_NOOP("font normal")
		<< QT_TR_NOOP("font special")
		<< QT_TR_NOOP("system background")
		<< QT_TR_NOOP("button red")
		<< QT_TR_NOOP("button yellow")
		<< QT_TR_NOOP("button green")
		<< QT_TR_NOOP("button blue")
		<< QT_TR_NOOP("button purple")
		<< QT_TR_NOOP("button pink")
		<< QT_TR_NOOP("button aqua")
		<< QT_TR_NOOP("button silver")
		<< QT_TR_NOOP("button navy")
		<< QT_TR_NOOP("button lime")
		<< QT_TR_NOOP("cursor")

		<< QT_TR_NOOP("language")
		<< QT_TR_NOOP("use lang list")

		<< QT_TR_NOOP("oslog")
		<< QT_TR_NOOP("watchdog")

		<< QT_TR_NOOP("thread priority")
		<< QT_TR_NOOP("enable multi-threading")
		<< QT_TR_NOOP("number of processors")

		<< QT_TR_NOOP("show sdl video performance")

		<< QT_TR_NOOP("video output method")

		<< QT_TR_NOOP("number of screens to create")
		<< QT_TR_NOOP("run in a window")
		<< QT_TR_NOOP("start out maximized")
		<< QT_TR_NOOP("enforce aspect ratio")
		<< QT_TR_NOOP("scale screen")
		<< QT_TR_NOOP("non-integer stretching")
		<< QT_TR_NOOP("visual effects")
		<< QT_TR_NOOP("center horizontally")
		<< QT_TR_NOOP("center vertically")
		<< QT_TR_NOOP("wait for vertical sync")
		<< QT_TR_NOOP("sync to monitor refresh")
		<< QT_TR_NOOP("scale mode")

		<< QT_TR_NOOP("hardware stretching")

		<< QT_TR_NOOP("d3d version")
		<< QT_TR_NOOP("bilinear filtering")

		<< QT_TR_NOOP("force power of 2 textures")
		<< QT_TR_NOOP("no gl arb texture rectangle")
		<< QT_TR_NOOP("enable opengl vbo")
		<< QT_TR_NOOP("enable opengl pbo")
		<< QT_TR_NOOP("enable opengl glsl")
		<< QT_TR_NOOP("opengl glsl filtering")
		<< QT_TR_NOOP("opengl glsl video attributes")

		<< QT_TR_NOOP("all screens: physical monitor")
		<< QT_TR_NOOP("all screens: aspect ratio")
		<< QT_TR_NOOP("all screens: resolution")
		<< QT_TR_NOOP("all screens: view")

		<< QT_TR_NOOP("screen 1: physical monitor")
		<< QT_TR_NOOP("screen 1: aspect ratio")
		<< QT_TR_NOOP("screen 1: resolution")
		<< QT_TR_NOOP("screen 1: view")

		<< QT_TR_NOOP("screen 2: physical monitor")
		<< QT_TR_NOOP("screen 2: aspect ratio")
		<< QT_TR_NOOP("screen 2: resolution")
		<< QT_TR_NOOP("screen 2: view")

		<< QT_TR_NOOP("screen 3: physical monitor")
		<< QT_TR_NOOP("screen 3: aspect ratio")
		<< QT_TR_NOOP("screen 3: resolution")
		<< QT_TR_NOOP("screen 3: view")

		<< QT_TR_NOOP("screen 4: physical monitor")
		<< QT_TR_NOOP("screen 4: aspect ratio")
		<< QT_TR_NOOP("screen 4: resolution")
		<< QT_TR_NOOP("screen 4: view")

		<< QT_TR_NOOP("triple buffering")
		<< QT_TR_NOOP("switch resolutions to fit")
		<< QT_TR_NOOP("full screen brightness")
		<< QT_TR_NOOP("full screen contrast")
		<< QT_TR_NOOP("full screen gamma")

		<< QT_TR_NOOP("audio latency")
		<< QT_TR_NOOP("audio sync")

		<< QT_TR_NOOP("dual lightgun")
		<< QT_TR_NOOP("joyid1")
		<< QT_TR_NOOP("joyid2")
		<< QT_TR_NOOP("joyid3")
		<< QT_TR_NOOP("joyid4")
		<< QT_TR_NOOP("joyid5")
		<< QT_TR_NOOP("joyid6")
		<< QT_TR_NOOP("joyid7")
		<< QT_TR_NOOP("joyid8")

		<< QT_TR_NOOP("ramsize")
		<< QT_TR_NOOP("writeconfig")
		<< QT_TR_NOOP("skip warnings")
		<< QT_TR_NOOP("use new mess ui")
		<< QT_TR_NOOP("use natural keyboard")

		<< QT_TR_NOOP("enable keymap")
		<< QT_TR_NOOP("keymap filename")

		<< QT_TR_NOOP("enable joystick mapping")
		<< QT_TR_NOOP("joymap filename")
		<< QT_TR_NOOP("ps3 sixaxis controllers")

		<< QT_TR_NOOP("sdl audio driver")
		<< QT_TR_NOOP("sdl video driver")
		<< QT_TR_NOOP("sdl render driver")

		<< QT_TR_NOOP("alternative libGL.so")
	);
#endif

	//parse default mame.ini
	QTextStream in(&text);
	QHash<QString, QString> iniSettings = parseIni(in, true);

	//init mameOpts with default values
	foreach (QString key, iniSettings.keys())
		{
				MameOption *pMameOpt = new MameOption(0);	//fixme parent
		pMameOpt->defvalue = iniSettings.value(key);
		mameOpts[key] = pMameOpt;
					}

	//take the first value of mame's default inipath csv as our inipath
	QStringList inipaths = mameOpts["inipath"]->defvalue.split(";");
	mameIniPath = utils->getPath(inipaths.first());

	//test ini readable/writable
	QString warnings = "";
	QFile iniFile(mameIniPath + (isMESS ? "mess" INI_EXT : (isUME ? "ume" INI_EXT : "mame" INI_EXT)));
	if (!iniFile.open(QIODevice::ReadWrite | QFile::Text))
		warnings.append(QFileInfo(iniFile).absoluteFilePath());
	iniFile.close();
	if (iniFile.size() == 0)
		iniFile.remove();

	// mkdir for individual game settings
	QDir().mkpath(mameIniPath + "ini/source");

	iniFile.setFileName(mameIniPath + "ini/puckman" INI_EXT);
	if (!iniFile.open(QIODevice::ReadWrite | QFile::Text))
		warnings.append("\n" + QFileInfo(mameIniPath + "ini").absoluteFilePath());
	iniFile.close();
	if (iniFile.size() == 0)
		iniFile.remove();

	if (warnings.size() > 0)
		win->poplog("Current user has no sufficient privilege to read/write:\n" + warnings + "\n\nCouldn't save MAME settings.");

	//patch inipath for unofficial mame
	QString inipath = mameOpts["inipath"]->defvalue;
	if (inipath != ".;ini")
	{
//		win->log("unofficial mame inipath");
		inipath.append(";");
		inipath.append(mameIniPath + "ini");
		inipath.remove("./");
		mameOpts["inipath"]->defvalue = inipath;
	}

	//mameOpts constructed, test MAME derivative now
	if (mameOpts.contains("sdlvideofps") || mameOpts.contains("videodriver"))
		isSDLPort = true;

	if (mameOpts.contains("langpath"))
		hasLanguage = true;

	if (mameOpts.contains("ips"))
		hasIPS = true;

	loadTemplate();

	// append GUI MESS extra software paths to mameOpts
	foreach (QString gameName, pMameDat->games.keys())
	{
		GameInfo *gameInfo = pMameDat->games[gameName];

		if (!gameInfo->devices.isEmpty() && !gameInfo->isExtRom)
		{
			MameOption *pMameOpt = new MameOption(0);	//fixme parent
			pMameOpt->guivisible = true;
			mameOpts[gameName + "_extra_software"] = pMameOpt;
			hasDevices = true;
		}
	}

	//append GUI and MESS settings to optCatMap
	QStringList optNames = mameOpts.keys();
	qSort(optNames);
	foreach (QString optName, optNames)
	{
		MameOption *pMameOpt = mameOpts[optName];

		if (!pMameOpt->guivisible)
			continue;

		if (optName.endsWith("_extra_software"))
		{
			pMameOpt->type = MAMEOPT_TYPE_DIR;
			pMameOpt->defvalue = "";
			optCatMap["03_MESS Paths_00_" + QString(QT_TR_NOOP("MESS software directory"))] << optName;
		}
		else
			optCatMap["01_GUI Paths_00_" + QString(QT_TR_NOOP("GUI paths"))] << optName;
	}
}

// assign option type, defvalue, min, max, etc. from template
void OptionUtils::loadTemplate()
{
	QFile file(":/res/optiontemplate.xml");

	QXmlInputSource xmlInputSource(&file);
	OptionXMLHandler handler(0);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);
	reader.parse(xmlInputSource);
}

//update mameOpts from .ini file, must be called in a proper order
void OptionUtils::loadIni(int optLevel, const QString &iniFileName)
{
	//ignore GUI settings, they are not handled by mame's .ini
	if (optLevel == OPTLEVEL_GUI)
		return;

	//get opt values from ini
	QHash<QString, QString> iniSettings;
	QFile inFile(iniFileName);
	if (inFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&inFile);
		iniSettings = parseIni(in, false);
		inFile.close();
	}

	//iterate every option
	foreach (QString optName, mameOpts.keys())
	{
		MameOption *pMameOpt = mameOpts[optName];

		//ignore GUI settings, they are not handled by mame's .ini
		//fixme:remove pGuiSettings?
		if (pGuiSettings->contains(optName))
		{
			pMameOpt->globalvalue = pMameOpt->currvalue
								  = pGuiSettings->value(optName).toString();
			continue;
		}

		//ips settings doesnt inherit
		if (optName == "ips")
		{
			//reset all to default
			pMameOpt->currvalue	= pMameOpt->cloneofvalue
								= pMameOpt->biosvalue
								= pMameOpt->srcvalue
								= pMameOpt->globalvalue
								= pMameOpt->defvalue;

			if (iniSettings.contains(optName))
				pMameOpt->currvalue = iniSettings[optName];

			continue;
		}

		//take ini override when available in loaded value
		if (iniSettings.contains(optName))
		{
			//assign ini value to the current optLevel in mameOpts
			pMameOpt->currvalue = iniSettings[optName];

			//assign ini value to the specified optLevel in mameOpts
			switch (optLevel)
			{
			case OPTLEVEL_GUI:
			case OPTLEVEL_GLOBAL:
				pMameOpt->globalvalue = pMameOpt->currvalue;
				break;

			case OPTLEVEL_SRC:
				pMameOpt->srcvalue = pMameOpt->currvalue;
				break;

			case OPTLEVEL_BIOS:
				pMameOpt->biosvalue = pMameOpt->currvalue;
				break;

			case OPTLEVEL_CLONEOF:
				pMameOpt->cloneofvalue = pMameOpt->currvalue;
				break;
			}
		}
		//inherit from higher level value, assign it to current and level value
		else
		{
			switch (optLevel)
			{
			case OPTLEVEL_GUI:
			case OPTLEVEL_GLOBAL:
				pMameOpt->currvalue = pMameOpt->globalvalue = pMameOpt->defvalue;
				break;

			case OPTLEVEL_SRC:
				pMameOpt->currvalue = pMameOpt->srcvalue = pMameOpt->globalvalue;
				break;

			case OPTLEVEL_BIOS:
				pMameOpt->currvalue = pMameOpt->biosvalue = pMameOpt->srcvalue;
				break;

			case OPTLEVEL_CLONEOF:
				pMameOpt->currvalue = pMameOpt->cloneofvalue= pMameOpt->biosvalue;
				break;

			case OPTLEVEL_CURR:
				pMameOpt->currvalue = pMameOpt->cloneofvalue;
				break;
			}
		}
	}
}

void OptionUtils::saveIniFile(int optLevel, const QString &iniFileName)
{
	//saveIniFile MAME Options
	QFile outFile(iniFileName);
	QString line;
	QStringList headers;
	QString mameIni;

	if (outFile.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream in(&pMameDat->defaultIni);
		QTextStream outBuf(&mameIni);
		QTextStream out(&outFile);
		in.setCodec("UTF-8");
		outBuf.setCodec("UTF-8");
		out.setCodec("UTF-8");
		//mame.ini always uses a BOM, doesnt work otherwise
		out.setGenerateByteOrderMark(true);
		bool isHeader = false, isChanged = false;
		QString optName;

		do
		{
			QString currVal, defVal;

			line = in.readLine();

			// ignore <UNADORNED>
			if (line.startsWith("<"))
				continue;

			// process # headers
			if (line.startsWith("#"))
			{
				headers << line;
				isHeader = true;
			}
			// process empty lines
			else if (line.isEmpty())
				outBuf << endl;
			// process option entry
			else
			{
				int sep = line.indexOf(utils->rxSpace);
				optName = line.left(sep);

				MameOption *pMameOpt = mameOpts[optName];

				switch (optLevel)
				{
				case OPTLEVEL_GUI:
				case OPTLEVEL_GLOBAL:
					currVal = pMameOpt->globalvalue;
					defVal = pMameOpt->defvalue;
					//ignore global bios setting
					if (optName == "bios")
						currVal = pMameOpt->defvalue;
					break;

				case OPTLEVEL_SRC:
					currVal = pMameOpt->srcvalue;
					defVal = pMameOpt->globalvalue;
					//ignore src bios setting
					if (optName == "bios")
						currVal = pMameOpt->defvalue;
					break;

				case OPTLEVEL_BIOS:
					currVal = pMameOpt->biosvalue;
					defVal = pMameOpt->srcvalue;
					break;

				case OPTLEVEL_CLONEOF:
					currVal = pMameOpt->cloneofvalue;
					defVal = pMameOpt->biosvalue;
					break;

				case OPTLEVEL_CURR:
					currVal = pMameOpt->currvalue;
					//do not inherit ips value
					if (optName == "ips")
						defVal = pMameOpt->defvalue;
					else
						defVal = pMameOpt->cloneofvalue;
				}

				if (getLongValue(optName, currVal) == getLongValue(optName, defVal))
					isChanged = false;
				else
					isChanged = true;

				isHeader = false;
			}

			if (!isHeader && !line.isEmpty())
			{
				// write header and clear header buffer
				if (!headers.isEmpty())
				{
					foreach (QString header, headers)
						outBuf << header << endl;

					headers.clear();
				}

				// write changed option entry to .ini or the complete mame.ini
				if (isChanged || optLevel == OPTLEVEL_GLOBAL || optLevel == OPTLEVEL_GUI)
				{
//					win->log(QString("curr: %1, def: %2").arg(currVal).arg(defVal));
					outBuf.setFieldWidth(26);
					outBuf.setFieldAlignment(QTextStream::AlignLeft);
					outBuf << optName;
					outBuf.setFieldWidth(0);
					//quote value if needed
					if (currVal.indexOf(utils->rxSpace) > 0 && !(currVal.startsWith('"') && currVal.endsWith('"')))
						outBuf << "\"" << currVal << "\"" << endl;
					else
						outBuf << currVal << endl;
				}
			}
		}
		while (!line.isNull());

		/* postprocess */
		QStringList bufs = mameIni.split(QRegExp("[\\r\\n]+"));

		// remove language setting, it's set at runtime
		for (int i = 0; i < bufs.size(); i ++)
		{
			if (bufs[i].startsWith("language"))
				bufs.removeAt(i);
		}

		// read in reverse order to eat empty headers
		for (int i = bufs.size() - 1; i >= 0; i --)
		{
			static int c = 0;
			line = bufs[i];
			if (line.startsWith("#"))
				c --;
			else if (!line.isEmpty())
				c = 3;	// each header has a maximum of 3 '#' lines

			if (c < 0)
				bufs.removeAt(i);
		}

		// remove trailing blank lines
		for (int i = bufs.size() - 1; i >= 0; i --)
		{
			if (bufs[i].isEmpty())
				bufs.removeAt(i);
			else
				break;
		}

		for (int i = 0; i < bufs.size(); i ++)
		{
			static bool isEntry = false;

			// make an empty line between sections
			if (bufs[i].startsWith("#"))
			{
				// add it when prev line is an entry and curr line is a header
				if (isEntry && i > 3)	//skip first entry
					out << endl;

				isEntry = false;
			}
			else
				isEntry = true;

			// don't output readconfig
			if (!bufs[i].contains("readconfig"))
				out << bufs[i] << endl;
		}
	}

	// delete ini if all options are default
	if (outFile.size() == 0)
		outFile.remove();
}

//returns a Hash containing key/value pairs of mame.ini settings, also inits optCatMap
QHash<QString, QString> OptionUtils::parseIni(QTextStream &in, bool isInitOptCatMap)
{
	// underscore separated category list, prefixed by a sorting number, init from .ini for core only
	static const QStringList optCatList = (QStringList()
		<< "00_Global Misc_00_" + QString(QT_TR_NOOP("core configuration"))
		<< "00_Global Misc_01_" + QString(QT_TR_NOOP("core palette"))
		<< "00_Global Misc_02_" + QString(QT_TR_NOOP("core language"))

		<< "02_MAME Paths_00_" + QString(QT_TR_NOOP("core search path"))
		<< "02_MAME Paths_01_" + QString(QT_TR_NOOP("core output directory"))
		<< "02_MAME Paths_02_" + QString(QT_TR_NOOP("core filename"))

		<< "04_Core Video_00_" + QString(QT_TR_NOOP("core rotation"))
		<< "04_Core Video_01_" + QString(QT_TR_NOOP("core screen"))
		<< "04_Core Video_02_" + QString(QT_TR_NOOP("core performance"))

		<< "05_OSD Video_00_" + QString(QT_TR_NOOP("OSD video"))
		<< "05_OSD Video_01_" + QString(QT_TR_NOOP("Windows video"))
		<< "05_OSD Video_02_" + QString(QT_TR_NOOP("OSD full screen"))
		<< "05_OSD Video_03_" + QString(QT_TR_NOOP("full screen"))
		<< "05_OSD Video_04_" + QString(QT_TR_NOOP("DirectDraw-specific"))
		<< "05_OSD Video_05_" + QString(QT_TR_NOOP("Direct3D-specific"))
		<< "05_OSD Video_06_" + QString(QT_TR_NOOP("Direct3D post-processing"))
		<< "05_OSD Video_07_" + QString(QT_TR_NOOP("OSD performance"))
		<< "05_OSD Video_08_" + QString(QT_TR_NOOP("OpenGL-specific"))
		<< "05_OSD Video_09_" + QString(QT_TR_NOOP("NTSC post-processing"))
		<< "05_OSD Video_10_" + QString(QT_TR_NOOP("Bloom post-processing"))

		<< "06_Screen_00_" + QString(QT_TR_NOOP("OSD per-window video"))

		<< "07_Audio_00_" + QString(QT_TR_NOOP("OSD sound"))
		<< "07_Audio_01_" + QString(QT_TR_NOOP("core sound"))

		<< "08_Control_00_" + QString(QT_TR_NOOP("core input"))
		<< "08_Control_01_" + QString(QT_TR_NOOP("core input automatic enable"))
		<< "08_Control_02_" + QString(QT_TR_NOOP("input device"))
		<< "08_Control_03_" + QString(QT_TR_NOOP("SDL keyboard mapping"))
		<< "08_Control_04_" + QString(QT_TR_NOOP("SDL joystick mapping"))

		<< "09_Vector_00_" + QString(QT_TR_NOOP("core vector"))
		<< "09_Vector_01_" + QString(QT_TR_NOOP("Vector post-processing"))

		<< "10_Misc_01_" + QString(QT_TR_NOOP("core misc"))
		<< "10_Misc_02_" + QString(QT_TR_NOOP("core artwork"))
		<< "10_Misc_03_" + QString(QT_TR_NOOP("core state/playback"))
		<< "10_Misc_04_" + QString(QT_TR_NOOP("SDL lowlevel driver"))
		<< "10_Misc_05_" + QString(QT_TR_NOOP("MESS specific"))
		<< "10_Misc_06_" + QString(QT_TR_NOOP("Windows MESS specific"))
		<< "10_Misc_07_" + QString(QT_TR_NOOP("Windows performance"))
		<< "10_Misc_08_" + QString(QT_TR_NOOP("core debugging"))
		<< "10_Misc_09_" + QString(QT_TR_NOOP("OSD debugging"))
		<< "10_Misc_10_" + QString(QT_TR_NOOP("Windows debugging"))
		);

	QHash<QString, QString> settings;
	QString optCat = "99_ERROR_00_MAGIC";
	in.setCodec("UTF-8");

	if (isInitOptCatMap)
		optCatMap.clear();

	while(true)
	{
		QString key, value;
		const QString line = in.readLine().trimmed();
		if (line.isNull())
			break;

		//init option headers
		if (line.startsWith("#"))
		{
			//line contains only # is ignored
			if (line.size() > 2 && isInitOptCatMap)
		{
				QString category(line);
				category.remove("#");
				category.remove("OPTIONS");
				category = category.trimmed();

				// assign the header to a known category
				int c = optCatList.indexOf (QRegExp(".*" + category + ".*", Qt::CaseInsensitive));
				if (c > 0)
					optCat = optCatList[c];
				// assign the header to Misc
				else
					optCat = "06_Misc_99_" + category;
				//we should have a valid optHeader by now
			}
		}
		// ignore <UNADORNED>
		else if (!line.startsWith("<") && line.size() > 0)
			{
			QStringList strs = utils->split2Str(line, " ");

			//add option name to optCatMap
			if (isInitOptCatMap)
				optCatMap[optCat] << strs.first();

			//option has a value from ini
			if (strs.size() > 1)
				{
				key = strs.first();
				value = strs.last();

					//remove quoted value if needed
					if (value.startsWith('"') && value.endsWith('"'))
					{
						value.remove(0, 1);
						value.chop(1);
					}
				}
			//option has empty value
				else
				{
				key = strs.first();
					value = "";
				}
			}

		if (!key.isEmpty())
			settings[key] = value;
		}

	return settings;
}

//fixme: use signal to separate GUI and core logic
void OptionUtils::chainLoadOptions(QListWidgetItem *currrentOptCatItem, int optLevel, QString gameName, int method)
{
	if (gameName.isEmpty())
		gameName = currentGame;

	//get optLevel from selected tab
	//fixme: not selected?
	if (optLevel == -1)
		optLevel = win->optionsUI->tabOptions->currentIndex();

	//get optSubCat from list ctlr
	QString optSubCat;
	if (optLevel > -1 && method == 0)
	{
		//assign to the selected option category if not assigned
		if (currrentOptCatItem == NULL)
			currrentOptCatItem = win->optionsUI->optCtrls[optLevel]->currentItem();

		//assign to option category in the first row if none selected
		if (currrentOptCatItem == NULL)
			currrentOptCatItem = win->optionsUI->optCtrls[optLevel]->item(0);

		if (currrentOptCatItem != NULL)
			optSubCat = currrentOptCatItem->text();
	}

	/* update mameOpts by loading .ini files of different levels */
	GameInfo *gameInfo = pMameDat->games[gameName];
	QString iniFileName;
	static const QString STR_OPTS_ = tr("Options") + " - ";

	loadIni(OPTLEVEL_GLOBAL, mameIniPath + (isMESS ? "mess" INI_EXT : (isUME ? "ume" INI_EXT : "mame" INI_EXT)));
	//GUI
	if (optLevel == OPTLEVEL_GUI)
	{
		if (method == 0)
		{
			updateModel(optSubCat, optLevel);
			win->optionsUI->setWindowTitle(STR_OPTS_ + tr("GUI"));
		}
		return;
	}

	//global
	if (optLevel == OPTLEVEL_GLOBAL)
	{
		if (method == 0)
		{
			updateModel(optSubCat, optLevel);
			win->optionsUI->setWindowTitle(STR_OPTS_ + tr("Global"));
		}
		return;
	}

	//source
	iniFileName = gameInfo->sourcefile;
	iniFileName.replace(".c", INI_EXT);
	loadIni(OPTLEVEL_SRC, mameIniPath + "ini/source/" + iniFileName);

	if (optLevel == OPTLEVEL_SRC)
	{
		if (method == 0)
		{
			updateModel(optSubCat, optLevel);
			win->optionsUI->setWindowTitle(STR_OPTS_ + gameInfo->sourcefile);
		}
		return;
	}

	//bios
	iniFileName = gameInfo->biosof();
	loadIni(OPTLEVEL_BIOS, mameIniPath + "ini/" + iniFileName + INI_EXT);

	if (optLevel == OPTLEVEL_BIOS)
	{
		if (method == 0)
		{
			updateModel(optSubCat, optLevel);
			win->optionsUI->setWindowTitle(STR_OPTS_ + iniFileName);
			win->optionsUI->tabOptions->widget(OPTLEVEL_BIOS)->setEnabled(iniFileName.isEmpty() ? false : true);
		}
		return;
	}

	//cloneof
	iniFileName = gameInfo->cloneof;
	loadIni(OPTLEVEL_CLONEOF, mameIniPath + "ini/" + iniFileName + INI_EXT);

	if (optLevel == OPTLEVEL_CLONEOF)
	{
		if (method == 0)
		{
			updateModel(optSubCat, optLevel);
			win->optionsUI->setWindowTitle(STR_OPTS_ + iniFileName);
			win->optionsUI->tabOptions->widget(OPTLEVEL_CLONEOF)->setEnabled(iniFileName.isEmpty() ? false : true);
		}
		return;
	}

	//current game
	//special case for consoles
	if (gameInfo->isExtRom)
		iniFileName = gameInfo->romof;
	else
		iniFileName = gameName;
	loadIni(OPTLEVEL_CURR, mameIniPath + "ini/" + iniFileName + INI_EXT);

	if (optLevel == OPTLEVEL_CURR)
	{
		if (method == 0)
		{
			updateModel(optSubCat, optLevel);
			win->optionsUI->setWindowTitle(STR_OPTS_ + iniFileName);
		}
		return;
	}
}

//update view in the option dialog
void OptionUtils::updateModel(const QString &optSubCat, int optLevel)
{
	QStandardItemModel *optModel0, *optModel;
	QTreeView *optView = optInfos[optLevel]->optView;

	//init option listview
	//hack: save model and delete it later, so that scroll bar position keeps the same
	optModel0 = optInfos[optLevel]->optModel;
	//construct a new model
	optModel = optInfos[optLevel]->optModel = new QStandardItemModel(win);
	//setup columns
	optModel->setObjectName(QString("optModel%1").arg(optLevel));
	optModel->setColumnCount(2);
	optModel->setHeaderData(0, Qt::Horizontal, tr("Option"));
	optModel->setHeaderData(1, Qt::Horizontal, tr("Value"));

	//apply new model
	optView->setModel(optModel);
	optView->setItemDelegate(&optDelegate);
	optView->header()->restoreState(option_column_state);

	foreach (QString optCat, optCatMap.keys())
	{
		//get all optNames belongs to optCat
		const QStringList optNames = optCatMap[optCat];
		//split optHeader str into 4 sections
		const QStringList optCatStrs = optCat.split('_');

		// if optHeader's category is in the current view
		if (optSubCat == tr(qPrintable(optCatStrs[1])))
		{
			//add section title
			addModelItemTitle(optModel, optCatStrs.last());

			//add option items for each optName
			foreach (QString optName, optNames)
			{
				MameOption *pMameOpt = mameOpts[optName];

				//filter non-applicable options
				if ((optLevel == OPTLEVEL_GLOBAL && !pMameOpt->globalvisible) ||
					(optLevel == OPTLEVEL_SRC && !pMameOpt->srcvisible) ||
					(optLevel == OPTLEVEL_BIOS && !pMameOpt->biosvisible) ||
					(optLevel == OPTLEVEL_CLONEOF && !pMameOpt->cloneofvisible) ||
					(optLevel == OPTLEVEL_CURR && !pMameOpt->gamevisible))
					continue;

				addModelItem(optModel, optName);
			}
		}
	}

	if (optModel0 != NULL)
		delete optModel0;
}

void OptionUtils::addModelItemTitle(QStandardItemModel *optModel, QString title)
{
	//fill title
	QStandardItem *item = new QStandardItem(tr(qPrintable(title)).toUpper());
	item->setData("OPTIONTITLE", Qt::UserRole + USERROLE_TITLE);
	optModel->appendRow(item);
}

void OptionUtils::addModelItem(QStandardItemModel *optModel, QString optName)
{
	updateSelectableItems(optName);

	MameOption *pMameOpt = mameOpts[optName];
	QString key = utils->capitalizeStr(tr(qPrintable(getLongName(optName).toLower())));

	//fixme: inaccurate
	//fix incorrectly lower cased
	if (language == "en_US" && !mameOpts[optName]->guiname.isEmpty())
		key = utils->capitalizeStr(mameOpts[optName]->guiname);

	QStandardItem *itemKey = new QStandardItem(key);
	itemKey->setData(optName, Qt::UserRole + USERROLE_KEY);

	//hack: a blank icon for padding
	static const QIcon icon(":/res/16x16/blank.png");
	itemKey->setIcon(icon);

	/* fill value */
	QStandardItem *itemVal = new QStandardItem(getLongValue(optName, pMameOpt->currvalue));

	optModel->appendRow(QList<QStandardItem *>() << itemKey << itemVal);
}

void OptionUtils::updateSelectableItems(QString optName)
{
	MameOption *pMameOpt = mameOpts[optName];

	// init BIOS values
	if (optName == "bios")
	{
		GameInfo *gameInfo = pMameDat->games[currentGame];
		QString biosof = gameInfo->biosof();
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		//MESS doesnt use "isbios" attrib, so all MESS entries with "biosset" is a bios
		if ((isMESS || gameInfo->isBios || !biosof.isEmpty()))
		{
			if (!isMESS && !gameInfo->isBios)
				gameInfo = pMameDat->games[biosof];

			QStringList biosSets = gameInfo->biosSets.keys();
			biosSets.sort();
			foreach (QString name, biosSets)
			{
				BiosSet *biosSet = gameInfo->biosSets[name];

				pMameOpt->values.append(name);
				pMameOpt->guivalues.append(biosSet->description);
			}
		}
	}

	// init ctrlr values
	else if (optName == "ctrlr")
	{
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		// iterate files
		QDir dir(mameOpts["ctrlrpath"]->currvalue);

		QStringList nameFilter;
		nameFilter << "*.cfg";

		QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
		files.sort();
		for (int i = 0; i < files.size(); i++)
		{
			QFileInfo fi(files[i]);
			QString ctrlr = fi.fileName();
			ctrlr.remove(".cfg");
			pMameOpt->values.append(ctrlr);
			pMameOpt->guivalues.append(ctrlr);
		}
	}

	//init effect values
	else if (optName == "effect")
	{
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		pMameOpt->values.append("none");
		pMameOpt->guivalues.append(tr("None"));

		// iterate files
		QDir dir(mameOpts["artpath"]->currvalue);

		QStringList nameFilter;
		nameFilter << "*" PNG_EXT;

		QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
		files.sort();
		for (int i = 0; i < files.size(); i++)
		{
			QFileInfo fi(files[i]);
			QString ctrlr = fi.fileName();
			ctrlr.remove(PNG_EXT);
			pMameOpt->values.append(ctrlr);
			pMameOpt->guivalues.append(ctrlr);
		}
	}

#ifdef USE_SDL
	else if (optName.startsWith("screen"))
	{
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		pMameOpt->values.append("auto");
		pMameOpt->guivalues.append(tr("Auto"));

		if (sdlInited)
		{
			for (int d = 0; d < SDL_GetNumVideoDisplays(); ++d)
			{
				QString displayName = QString("\\\\.\\DISPLAY%1").arg(d + 1);
				pMameOpt->values.append(displayName);
				pMameOpt->guivalues.append(displayName);
			}
		}
	}

	else if (optName.startsWith("resolution"))
	{
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		pMameOpt->values.append("auto");
		pMameOpt->guivalues.append(tr("Auto"));

		if (sdlInited)
		{
			//select respective screen
			QString optName2 = optName;
			optName2.replace("resolution", "screen");
			QString displayName = mameOpts[optName2]->currvalue;
			bool ok;
			int d = displayName.remove("\\\\.\\DISPLAY").toInt(&ok);
			SDL_SelectVideoDisplay(ok ? d - 1 : 0);

			// emun available fullscreen video modes
			int nmodes = SDL_GetNumDisplayModes();
			if (nmodes == 0)
				win->log("SDL: No available fullscreen video modes");
			else
			{
				SDL_DisplayMode mode;
				int bpp;
				Uint32 Rmask, Gmask, Bmask, Amask;

				for (int m = 0; m < nmodes; ++ m)
				{
					SDL_GetDisplayMode(m, &mode);
					SDL_PixelFormatEnumToMasks(mode.format, &bpp,
						&Rmask, &Gmask, &Bmask, &Amask);

					QString modeName = QString("%1x%2@%3")
						.arg(mode.w)
						.arg(mode.h)
						.arg(mode.refresh_rate);

					if (pMameOpt->values.contains(modeName))
						continue; // skip reduplicate resolution

					pMameOpt->values.append(modeName);
					pMameOpt->guivalues.append(modeName + "Hz");
				}
			}
		}
	}
#endif /* USE_SDL */

	// prepare GUI value desc
	for (int i = 0; i < pMameOpt->guivalues.size(); i++)
	{
		if (pMameOpt->guivalues[i].isEmpty())
			pMameOpt->guivalues[i] = utils->capitalizeStr(pMameOpt->values[i]);
	}
}

void OptionUtils::updateHeaderSize(int logicalIndex, int oldSize, int newSize)
{
	if (logicalIndex > 0)
		return;

	int optLevel = win->optionsUI->tabOptions->currentIndex();
	option_column_state = optInfos[optLevel]->optView->header()->saveState();

//	win->log(QString("header%3: %1 to %2").arg(optInfos[optLevel]->optView->header()->sectionSize(0)).arg(newSize).arg(logicalIndex));
}
