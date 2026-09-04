// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "deviceinformationwindow.h"

#include "memorywindow.h"

#include "util/xmlfile.h"

#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>


namespace osd::debugger::qt {

AddressSpacesListModel::AddressSpacesListModel(device_memory_interface &memory, QObject *parent) :
	QAbstractTableModel(parent),
	m_memory(memory)
{
	for (int i = 0; i < m_memory.max_space_count(); i++)
	{
		if (m_memory.has_space(i))
			m_indices.emplace_back(i);
	}
}


AddressSpacesListModel::~AddressSpacesListModel()
{
}


QVariant AddressSpacesListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || (role != Qt::DisplayRole))
		return QVariant();

	address_space &s = space(index);
	switch (index.column())
	{
	case 0: return QString(s.name());
	case 1: return s.data_width();
	case 2: return s.addr_width();
	case 3: return s.addr_shift();
	case 4: return s.alignment();
	case 5: return QString((s.endianness() == std::endian::big) ? "big" : "little");
	}

	return QVariant();
}


Qt::ItemFlags AddressSpacesListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return QAbstractItemModel::flags(index);
}


QVariant AddressSpacesListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0: return QString("Name");
		case 1: return QString("Data width");
		case 2: return QString("Address width");
		case 3: return QString("Address shift");
		case 4: return QString("Alignment");
		case 5: return QString("Endianness");
		}
	}
	else if (orientation == Qt::Vertical)
	{
		return m_memory.space(m_indices[section]).spacenum();
	}

	return QVariant();
}


int AddressSpacesListModel::rowCount(const QModelIndex &parent) const
{
	return m_indices.size();
}


int AddressSpacesListModel::columnCount(const QModelIndex &parent) const
{
	return 6;
}


address_space &AddressSpacesListModel::space(const QModelIndex &index) const
{
	return m_memory.space(m_indices[index.row()]);
}



DeviceInformationWindow::DeviceInformationWindow(DebuggerQt &debugger, device_t *device, QWidget *parent) :
	WindowQt(debugger, nullptr),
	m_device(device)
{
	if (parent)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 600, 400);
	}

	if (m_device)
		fill_device_information();
}


DeviceInformationWindow::~DeviceInformationWindow()
{
}


void DeviceInformationWindow::set_device(const char *tag)
{
	m_device = m_machine.root_device().subdevice(tag);
	if (!m_device)
		m_device = &m_machine.root_device();
	fill_device_information();
}


void DeviceInformationWindow::openAddessSpace(const QModelIndex &index)
{
	MemoryWindow *foo = new MemoryWindow(m_debugger, this);
	foo->selectSpace(m_addrspaces_model->space(index));
}


void DeviceInformationWindow::restoreConfiguration(util::xml::data_node const &node)
{
	WindowQt::restoreConfiguration(node);

	auto const tag = node.get_attribute_string(ATTR_WINDOW_DEVICE_TAG, ":");
	set_device(tag);
}


void DeviceInformationWindow::saveConfigurationToNode(util::xml::data_node &node)
{
	WindowQt::saveConfigurationToNode(node);

	node.set_attribute_int(ATTR_WINDOW_TYPE, WINDOW_TYPE_DEVICE_INFO_VIEWER);

	node.set_attribute(ATTR_WINDOW_DEVICE_TAG, m_device->tag());
}


void DeviceInformationWindow::fill_device_information()
{
	setWindowTitle(util::string_format("Debug: Device %s", m_device->tag()).c_str());

	QFrame *mainWindowFrame = new QFrame(this);
	QVBoxLayout *vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setObjectName("vlayout");
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);

	QFrame *primaryFrame = new QFrame(mainWindowFrame);
	primaryFrame->setFrameStyle(0 | QFrame::StyledPanel | QFrame::Sunken); // zero because C++20 doesn't allow arithmetic between different enums
	QGridLayout *gl1 = new QGridLayout(primaryFrame);
	gl1->addWidget(new QLabel(QString("Tag"), primaryFrame), 0, 0);
	gl1->addWidget(new QLabel(QString(m_device->tag()), primaryFrame), 0, 1);
	gl1->addWidget(new QLabel(QString("Name"), primaryFrame), 1, 0);
	gl1->addWidget(new QLabel(QString::fromUtf8(m_device->name()), primaryFrame), 1, 1);
	gl1->addWidget(new QLabel(QString("Shortname"), primaryFrame), 2, 0);
	gl1->addWidget(new QLabel(QString(m_device->shortname()), primaryFrame), 2, 1);

	int cpos = 3;
	device_interface *intf = m_device->interfaces().first();
	if (intf)
	{
		gl1->addWidget(new QLabel(QString("Interfaces"), primaryFrame), cpos, 0);
		while(intf)
		{
			gl1->addWidget(new QLabel(QString(intf->interface_type()), primaryFrame), cpos, 1);
			cpos++;
			intf = intf->interface_next();
		}
	}

	vLayout->addWidget(primaryFrame);

	device_memory_interface *d_memory;
	if (m_device->interface(d_memory))
	{
		for (int i = 0; i < d_memory->max_space_count(); i++)
		{
			if (d_memory->has_space(i))
			{
				m_addrspaces_model.emplace(*d_memory);
				QFrame *f = new QFrame(mainWindowFrame);
				f->setFrameStyle(0 | QFrame::StyledPanel | QFrame::Sunken); // zero because C++20 doesn't allow arithmetic between different enums
				QVBoxLayout *vb = new QVBoxLayout(f);
				vb->addWidget(new QLabel("Address spaces"));
				QTableView *t = new QTableView;
				t->setModel(&*m_addrspaces_model);
				connect(t, &QTableView::doubleClicked, this, &DeviceInformationWindow::openAddessSpace);
				vb->addWidget(t);
				vLayout->addWidget(f);
				break;
			}
		}
	}

	vLayout->addStretch();

	setCentralWidget(mainWindowFrame);
}

} // namespace osd::debugger::qt
