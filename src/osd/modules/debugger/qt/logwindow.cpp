// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#define NO_MEM_TRACKING

#include "logwindow.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/dvdisasm.h"


LogWindow::LogWindow(running_machine* machine, QWidget* parent) :
	WindowQt(machine, NULL)
{
	setWindowTitle("Debug: Machine Log");

	if (parent != NULL)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and log widgets
	//
	QFrame* mainWindowFrame = new QFrame(this);

	// The main log view
	m_logView = new DebuggerView(DVT_LOG,
									m_machine,
									this);

	// Layout
	QVBoxLayout* vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);
	vLayout->addWidget(m_logView);

	setCentralWidget(mainWindowFrame);
}


LogWindow::~LogWindow()
{
}


//=========================================================================
//  LogWindowQtConfig
//=========================================================================
void LogWindowQtConfig::buildFromQWidget(QWidget* widget)
{
	WindowQtConfig::buildFromQWidget(widget);
}


void LogWindowQtConfig::applyToQWidget(QWidget* widget)
{
	WindowQtConfig::applyToQWidget(widget);
}


void LogWindowQtConfig::addToXmlDataNode(xml_data_node* node) const
{
	WindowQtConfig::addToXmlDataNode(node);
}


void LogWindowQtConfig::recoverFromXmlNode(xml_data_node* node)
{
	WindowQtConfig::recoverFromXmlNode(node);
}
