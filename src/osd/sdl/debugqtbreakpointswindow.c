#define NO_MEM_TRACKING

#include "debugqtbreakpointswindow.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/dvdisasm.h"


BreakpointsWindow::BreakpointsWindow(running_machine* machine, QWidget* parent) :
	WindowQt(machine, NULL)
{
	setWindowTitle("Debug: All Breakpoints");

	if (parent != NULL)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and breakpoints widgets
	//
	QFrame* mainWindowFrame = new QFrame(this);

	// The main breakpoints view
	m_breakpointsView = new DebuggerView(DVT_BREAK_POINTS, m_machine, this);

	// Layout
	QVBoxLayout* vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);
	vLayout->addWidget(m_breakpointsView);

	setCentralWidget(mainWindowFrame);
}


BreakpointsWindow::~BreakpointsWindow()
{
}


//=========================================================================
//  BreakpointsWindowQtConfig
//=========================================================================
void BreakpointsWindowQtConfig::buildFromQWidget(QWidget* widget)
{
	WindowQtConfig::buildFromQWidget(widget);
}


void BreakpointsWindowQtConfig::applyToQWidget(QWidget* widget)
{
	WindowQtConfig::applyToQWidget(widget);
}


void BreakpointsWindowQtConfig::addToXmlDataNode(xml_data_node* node) const
{
	WindowQtConfig::addToXmlDataNode(node);
}


void BreakpointsWindowQtConfig::recoverFromXmlNode(xml_data_node* node)
{
	WindowQtConfig::recoverFromXmlNode(node);
}
