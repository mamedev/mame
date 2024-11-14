// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "logwindow.h"

#include "util/xmlfile.h"

#include <QtWidgets/QVBoxLayout>


namespace osd::debugger::qt {

LogWindow::LogWindow(DebuggerQt &debugger, QWidget *parent) :
	WindowQt(debugger, nullptr)
{
	setWindowTitle("Debug: Machine Log");

	if (parent)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and log widgets
	//
	QFrame *mainWindowFrame = new QFrame(this);

	// The main log view
	m_logView = new DebuggerView(DVT_LOG, m_machine, this);

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


void LogWindow::restoreConfiguration(util::xml::data_node const &node)
{
	WindowQt::restoreConfiguration(node);

	m_logView->restoreConfigurationFromNode(node);
}


void LogWindow::saveConfigurationToNode(util::xml::data_node &node)
{
	WindowQt::saveConfigurationToNode(node);

	node.set_attribute_int(ATTR_WINDOW_TYPE, WINDOW_TYPE_ERROR_LOG_VIEWER);

	m_logView->saveConfigurationToNode(node);
}

} // namespace osd::debugger::qt
