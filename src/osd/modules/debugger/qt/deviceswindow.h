// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef __DEBUG_QT_DEVICES_WINDOW_H__
#define __DEBUG_QT_DEVICES_WINDOW_H__

#include <QtWidgets/QTreeView>

#include "windowqt.h"


//============================================================
//  The model for the treeview
//============================================================

class DevicesWindowModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit DevicesWindowModel(running_machine *machine, QObject *parent = 0);
	~DevicesWindowModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
						const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
	running_machine *m_machine;
};

//============================================================
//  The Devices Window.
//============================================================
class DevicesWindow : public WindowQt
{
	Q_OBJECT

public:
	DevicesWindow(running_machine* machine, QWidget* parent=NULL);
	virtual ~DevicesWindow();

public slots:
	void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);
	void activated(const QModelIndex &index);

private:
	QTreeView *m_devices_view;
	DevicesWindowModel m_devices_model;
	device_t *m_selected_device;
};




//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class DevicesWindowQtConfig : public WindowQtConfig
{
public:
	DevicesWindowQtConfig() :
		WindowQtConfig(WIN_TYPE_DEVICES)
	{
	}

	~DevicesWindowQtConfig() {}

	void buildFromQWidget(QWidget* widget);
	void applyToQWidget(QWidget* widget);
	void addToXmlDataNode(xml_data_node* node) const;
	void recoverFromXmlNode(xml_data_node* node);
};


#endif
