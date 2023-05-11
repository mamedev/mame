// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "deviceinformationwindow.h"

#include "util/xmlfile.h"

#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>


namespace osd::debugger::qt {

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
	primaryFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	QGridLayout *gl1 = new QGridLayout(primaryFrame);
	gl1->addWidget(new QLabel(QString("Tag"), primaryFrame), 0, 0);
	gl1->addWidget(new QLabel(QString(m_device->tag()), primaryFrame), 0, 1);
	gl1->addWidget(new QLabel(QString("Name"), primaryFrame), 1, 0);
	gl1->addWidget(new QLabel(QString(m_device->name()), primaryFrame), 1, 1);
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
		QFrame *f = new QFrame(mainWindowFrame);
		f->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		QVBoxLayout *vb = new QVBoxLayout(f);
		bool first = true;
		for (int i=0; i<d_memory->max_space_count(); i++)
			if (d_memory->has_space(i))
			{
				QFrame *ff = new QFrame(f);
				QHBoxLayout *hb = new QHBoxLayout(ff);
				if (first)
				{
					hb->addWidget(new QLabel("Memory maps"));
					first = false;
				}
				hb->addStretch();
				hb->addWidget(new QLabel(d_memory->space_config(i)->name()));
				vb->addWidget(ff);
			}
		vLayout->addWidget(f);
	}

	vLayout->addStretch();

	setCentralWidget(mainWindowFrame);
}

void DeviceInformationWindow::set_device(const char *tag)
{
	m_device = m_machine.root_device().subdevice(tag);
	if (!m_device)
		m_device = &m_machine.root_device();
	fill_device_information();
}

} // namespace osd::debugger::qt
