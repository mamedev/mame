// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

#include "deviceinformationwindow.h"


DeviceInformationWindow::DeviceInformationWindow(running_machine* machine, device_t* device, QWidget* parent) :
	WindowQt(machine, NULL)
{
	m_device = device;

	if (parent != NULL)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 600, 400);
	}

	if(m_device)
		fill_device_information();
}


DeviceInformationWindow::~DeviceInformationWindow()
{
}

void DeviceInformationWindow::fill_device_information()
{
	char title[4069];
	sprintf(title, "Debug: Device %s", m_device->tag());
	setWindowTitle(title);


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
	if(intf) {
		gl1->addWidget(new QLabel(QString("Interfaces"), primaryFrame), cpos, 0);
		while(intf) {
			gl1->addWidget(new QLabel(QString(intf->interface_type()), primaryFrame), cpos, 1);
			cpos++;
			intf = intf->interface_next();
		}
	}

	vLayout->addWidget(primaryFrame);

	device_memory_interface *d_memory;
	if(m_device->interface(d_memory)) {
		QFrame *f = new QFrame(mainWindowFrame);
		f->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		QVBoxLayout *vb = new QVBoxLayout(f);
		bool first = true;
		for(address_spacenum i=AS_0; i<ADDRESS_SPACES; i++)
			if(d_memory->has_space(i)) {
				QFrame *ff = new QFrame(f);
				QHBoxLayout *hb = new QHBoxLayout(ff);
				if(first) {
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
	m_device = m_machine->device(tag);
	if(!m_device)
		m_device = &m_machine->root_device();
	fill_device_information();
}

const char *DeviceInformationWindow::device_tag() const
{
	return m_device->tag();
}


//=========================================================================
//  DeviceInformationWindowQtConfig
//=========================================================================
void DeviceInformationWindowQtConfig::buildFromQWidget(QWidget* widget)
{
	WindowQtConfig::buildFromQWidget(widget);
	DeviceInformationWindow* window = dynamic_cast<DeviceInformationWindow*>(widget);
	m_device_tag = window->device_tag();
}


void DeviceInformationWindowQtConfig::applyToQWidget(QWidget* widget)
{
	WindowQtConfig::applyToQWidget(widget);
	DeviceInformationWindow* window = dynamic_cast<DeviceInformationWindow*>(widget);
	window->set_device(m_device_tag.c_str());
}


void DeviceInformationWindowQtConfig::addToXmlDataNode(xml_data_node* node) const
{
	WindowQtConfig::addToXmlDataNode(node);
	xml_set_attribute(node, "device-tag", m_device_tag.c_str());
}


void DeviceInformationWindowQtConfig::recoverFromXmlNode(xml_data_node* node)
{
	WindowQtConfig::recoverFromXmlNode(node);
	m_device_tag = xml_get_attribute_string(node, "device-tag", ":");
}
