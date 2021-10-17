// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "breakpointswindow.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QVBoxLayout>


BreakpointsWindow::BreakpointsWindow(running_machine &machine, QWidget *parent) :
	WindowQt(machine, nullptr)
{
	setWindowTitle("Debug: All Breakpoints");

	if (parent)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and breakpoints widgets
	//
	QFrame *mainWindowFrame = new QFrame(this);

	// The main breakpoints view
	m_breakpointsView = new DebuggerView(DVT_BREAK_POINTS, m_machine, this);

	// Layout
	QVBoxLayout *vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setObjectName("vlayout");
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);
	vLayout->addWidget(m_breakpointsView);

	setCentralWidget(mainWindowFrame);

	//
	// Menu bars
	//
	QActionGroup *typeGroup = new QActionGroup(this);
	typeGroup->setObjectName("typegroup");

	QAction *typeBreak = new QAction("Breakpoints", this);
	typeBreak->setObjectName("typebreak");
	typeBreak->setCheckable(true);
	typeBreak->setActionGroup(typeGroup);
	typeBreak->setShortcut(QKeySequence("Ctrl+1"));

	QAction *typeWatch = new QAction("Watchpoints", this);
	typeWatch->setObjectName("typewatch");
	typeWatch->setCheckable(true);
	typeWatch->setActionGroup(typeGroup);
	typeWatch->setShortcut(QKeySequence("Ctrl+2"));

	QAction *typeRegister = new QAction("Registerpoints", this);
	typeRegister->setObjectName("typeregister");
	typeRegister->setCheckable(true);
	typeRegister->setActionGroup(typeGroup);
	typeRegister->setShortcut(QKeySequence("Ctrl+3"));

	typeBreak->setChecked(true);
	connect(typeGroup, &QActionGroup::triggered, this, &BreakpointsWindow::typeChanged);

	// Assemble the options menu
	QMenu *optionsMenu = menuBar()->addMenu("&Options");
	optionsMenu->addActions(typeGroup->actions());
}


BreakpointsWindow::~BreakpointsWindow()
{
}


void BreakpointsWindow::typeChanged(QAction* changedTo)
{
	// Clean
	delete m_breakpointsView;
	m_breakpointsView = nullptr;

	// Create
	if (changedTo->text() == "Breakpoints")
	{
		m_breakpointsView = new DebuggerView(DVT_BREAK_POINTS, m_machine, this);
		setWindowTitle("Debug: All Breakpoints");
	}
	else if (changedTo->text() == "Watchpoints")
	{
		m_breakpointsView = new DebuggerView(DVT_WATCH_POINTS, m_machine, this);
		setWindowTitle("Debug: All Watchpoints");
	}
	else if (changedTo->text() == "Registerpoints")
	{
		m_breakpointsView = new DebuggerView(DVT_REGISTER_POINTS, m_machine, this);
		setWindowTitle("Debug: All Registerpoints");
	}

	// Re-register
	QVBoxLayout *layout = findChild<QVBoxLayout *>("vlayout");
	layout->addWidget(m_breakpointsView);
}



//=========================================================================
//  BreakpointsWindowQtConfig
//=========================================================================
void BreakpointsWindowQtConfig::buildFromQWidget(QWidget *widget)
{
	WindowQtConfig::buildFromQWidget(widget);
	BreakpointsWindow *window = dynamic_cast<BreakpointsWindow *>(widget);

	QActionGroup* typeGroup = window->findChild<QActionGroup*>("typegroup");
	if (typeGroup->checkedAction()->text() == "Breakpoints")
		m_bwType = 0;
	else if (typeGroup->checkedAction()->text() == "Watchpoints")
		m_bwType = 1;
	else if (typeGroup->checkedAction()->text() == "Registerpoints")
		m_bwType = 2;
}


void BreakpointsWindowQtConfig::applyToQWidget(QWidget* widget)
{
	WindowQtConfig::applyToQWidget(widget);
	BreakpointsWindow *window = dynamic_cast<BreakpointsWindow *>(widget);

	QActionGroup *typeGroup = window->findChild<QActionGroup *>("typegroup");
	typeGroup->actions()[m_bwType]->trigger();
}


void BreakpointsWindowQtConfig::addToXmlDataNode(util::xml::data_node &node) const
{
	WindowQtConfig::addToXmlDataNode(node);
	node.set_attribute_int("bwtype", m_bwType);
}


void BreakpointsWindowQtConfig::recoverFromXmlNode(util::xml::data_node const &node)
{
	WindowQtConfig::recoverFromXmlNode(node);
	m_bwType = node.get_attribute_int("bwtype", m_bwType);
}
