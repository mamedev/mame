// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "deviceswindow.h"
#include "deviceinformationwindow.h"

DevicesWindowModel::DevicesWindowModel(running_machine *machine, QObject *parent)
{
	m_machine = machine;
}

DevicesWindowModel::~DevicesWindowModel()
{
}

QVariant DevicesWindowModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	device_t *dev = static_cast<device_t *>(index.internalPointer());
	switch(index.column()) {
	case 0: return dev == &m_machine->root_device() ? QString("<root>") : QString(dev->basetag());
	case 1: return QString(dev->name());
	}

	return QVariant();
}

Qt::ItemFlags DevicesWindowModel::flags(const QModelIndex &index) const
{
	if(!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

QVariant DevicesWindowModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role != Qt::DisplayRole || section < 0 || section >= 2)
		return QVariant();
	return QString(section ? "Name" : "Tag");
}

QModelIndex DevicesWindowModel::index(int row, int column, const QModelIndex &parent) const
{
	if(!hasIndex(row, column, parent))
		return QModelIndex();

	device_t *target = NULL;

	if(!parent.isValid()) {
		if(row == 0)
			target = &m_machine->root_device();

	} else {
		device_t *dparent = static_cast<device_t *>(parent.internalPointer());
		int count = row;
		for(target = dparent->subdevices().first(); count && target; target = target->next())
			count--;
	}

	if(target)
		return createIndex(row, column, target);

	return QModelIndex();
}

QModelIndex DevicesWindowModel::parent(const QModelIndex &index) const
{
	if(!index.isValid())
		return QModelIndex();

	device_t *dchild = static_cast<device_t *>(index.internalPointer());
	device_t *dparent = dchild->owner();

	if(!dparent)
		return QModelIndex();

	device_t *dpp = dparent->owner();
	int row = 0;
	if(dpp) {
		for(device_t *child = dpp->subdevices().first(); child && child != dparent; child = child->next())
			row++;
	}
	return createIndex(row, 0, dparent);
}

int DevicesWindowModel::rowCount(const QModelIndex &parent) const
{
	if(!parent.isValid())
		return 1;

	device_t *dparent = static_cast<device_t *>(parent.internalPointer());
	int count = 0;
	for (device_t &child : dparent->subdevices()) 
	{
		(void)child;
		count++;
	}

	return count;
}

int DevicesWindowModel::columnCount(const QModelIndex &parent) const
{
	return 2;
}



DevicesWindow::DevicesWindow(running_machine* machine, QWidget* parent) :
	WindowQt(machine, NULL),
	m_devices_model(machine)
{
	m_selected_device = NULL;

	setWindowTitle("Debug: All Devices");

	if (parent != NULL)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 600, 400);
	}

	//
	// The tree widget
	//
	m_devices_view = new QTreeView(this);
	m_devices_view->setModel(&m_devices_model);
	m_devices_view->expandAll();
	m_devices_view->resizeColumnToContents(0);
	connect(m_devices_view->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &DevicesWindow::currentRowChanged);
	connect(m_devices_view, &QTreeView::activated, this, &DevicesWindow::activated);
	setCentralWidget(m_devices_view);
}


DevicesWindow::~DevicesWindow()
{
}


void DevicesWindow::currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
	m_selected_device = static_cast<device_t *>(current.internalPointer());
}


void DevicesWindow::activated(const QModelIndex &index)
{
	device_t *dev = static_cast<device_t *>(index.internalPointer());
	(new DeviceInformationWindow(m_machine, dev, this))->show();
}



//=========================================================================
//  DevicesWindowQtConfig
//=========================================================================
void DevicesWindowQtConfig::buildFromQWidget(QWidget* widget)
{
	WindowQtConfig::buildFromQWidget(widget);
	//  DevicesWindow* window = dynamic_cast<DevicesWindow*>(widget);
}


void DevicesWindowQtConfig::applyToQWidget(QWidget* widget)
{
	WindowQtConfig::applyToQWidget(widget);
	//  DevicesWindow* window = dynamic_cast<DevicesWindow*>(widget);
}


void DevicesWindowQtConfig::addToXmlDataNode(xml_data_node* node) const
{
	WindowQtConfig::addToXmlDataNode(node);
}


void DevicesWindowQtConfig::recoverFromXmlNode(xml_data_node* node)
{
	WindowQtConfig::recoverFromXmlNode(node);
}
