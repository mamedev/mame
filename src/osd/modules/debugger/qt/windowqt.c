// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#define NO_MEM_TRACKING

#include "windowqt.h"
#include "logwindow.h"
#include "dasmwindow.h"
#include "memorywindow.h"
#include "breakpointswindow.h"
#include "deviceswindow.h"

bool WindowQt::s_refreshAll = false;
bool WindowQt::s_hideAll = false;


// Since all debug windows are intended to be top-level, this inherited
// constructor is always called with a NULL parent.  The passed-in parent widget,
// however, is often used to place each child window & the code to do this can
// be found in most of the inherited classes.

WindowQt::WindowQt(running_machine* machine, QWidget* parent) :
	QMainWindow(parent),
	m_machine(machine)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	// The Debug menu bar
	QAction* debugActOpenMemory = new QAction("New &Memory Window", this);
	debugActOpenMemory->setShortcut(QKeySequence("Ctrl+M"));
	connect(debugActOpenMemory, SIGNAL(triggered()), this, SLOT(debugActOpenMemory()));

	QAction* debugActOpenDasm = new QAction("New &Dasm Window", this);
	debugActOpenDasm->setShortcut(QKeySequence("Ctrl+D"));
	connect(debugActOpenDasm, SIGNAL(triggered()), this, SLOT(debugActOpenDasm()));

	QAction* debugActOpenLog = new QAction("New &Log Window", this);
	debugActOpenLog->setShortcut(QKeySequence("Ctrl+L"));
	connect(debugActOpenLog, SIGNAL(triggered()), this, SLOT(debugActOpenLog()));

	QAction* debugActOpenPoints = new QAction("New &Break|Watchpoints Window", this);
	debugActOpenPoints->setShortcut(QKeySequence("Ctrl+B"));
	connect(debugActOpenPoints, SIGNAL(triggered()), this, SLOT(debugActOpenPoints()));

	QAction* debugActOpenDevices = new QAction("New D&evices Window", this);
	debugActOpenDevices->setShortcut(QKeySequence("Shift+Ctrl+D"));
	connect(debugActOpenDevices, SIGNAL(triggered()), this, SLOT(debugActOpenDevices()));

	QAction* dbgActRun = new QAction("Run", this);
	dbgActRun->setShortcut(Qt::Key_F5);
	connect(dbgActRun, SIGNAL(triggered()), this, SLOT(debugActRun()));

	QAction* dbgActRunAndHide = new QAction("Run And Hide Debugger", this);
	dbgActRunAndHide->setShortcut(Qt::Key_F12);
	connect(dbgActRunAndHide, SIGNAL(triggered()), this, SLOT(debugActRunAndHide()));

	QAction* dbgActRunToNextCpu = new QAction("Run to Next CPU", this);
	dbgActRunToNextCpu->setShortcut(Qt::Key_F6);
	connect(dbgActRunToNextCpu, SIGNAL(triggered()), this, SLOT(debugActRunToNextCpu()));

	QAction* dbgActRunNextInt = new QAction("Run to Next Interrupt on This CPU", this);
	dbgActRunNextInt->setShortcut(Qt::Key_F7);
	connect(dbgActRunNextInt, SIGNAL(triggered()), this, SLOT(debugActRunNextInt()));

	QAction* dbgActRunNextVBlank = new QAction("Run to Next VBlank", this);
	dbgActRunNextVBlank->setShortcut(Qt::Key_F8);
	connect(dbgActRunNextVBlank, SIGNAL(triggered()), this, SLOT(debugActRunNextVBlank()));

	QAction* dbgActStepInto = new QAction("Step Into", this);
	dbgActStepInto->setShortcut(Qt::Key_F11);
	connect(dbgActStepInto, SIGNAL(triggered()), this, SLOT(debugActStepInto()));

	QAction* dbgActStepOver = new QAction("Step Over", this);
	dbgActStepOver->setShortcut(Qt::Key_F10);
	connect(dbgActStepOver, SIGNAL(triggered()), this, SLOT(debugActStepOver()));

	QAction* dbgActStepOut = new QAction("Step Out", this);
	dbgActStepOut->setShortcut(QKeySequence("Shift+F11"));
	connect(dbgActStepOut, SIGNAL(triggered()), this, SLOT(debugActStepOut()));

	QAction* dbgActSoftReset = new QAction("Soft Reset", this);
	dbgActSoftReset->setShortcut(Qt::Key_F3);
	connect(dbgActSoftReset, SIGNAL(triggered()), this, SLOT(debugActSoftReset()));

	QAction* dbgActHardReset = new QAction("Hard Reset", this);
	dbgActHardReset->setShortcut(QKeySequence("Shift+F3"));
	connect(dbgActHardReset, SIGNAL(triggered()), this, SLOT(debugActHardReset()));

	QAction* dbgActClose = new QAction("Close &Window", this);
	dbgActClose->setShortcut(QKeySequence::Close);
	connect(dbgActClose, SIGNAL(triggered()), this, SLOT(debugActClose()));

	QAction* dbgActQuit = new QAction("&Quit", this);
	dbgActQuit->setShortcut(QKeySequence::Quit);
	connect(dbgActQuit, SIGNAL(triggered()), this, SLOT(debugActQuit()));

	// Construct the menu
	QMenu* debugMenu = menuBar()->addMenu("&Debug");
	debugMenu->addAction(debugActOpenMemory);
	debugMenu->addAction(debugActOpenDasm);
	debugMenu->addAction(debugActOpenLog);
	debugMenu->addAction(debugActOpenPoints);
	debugMenu->addAction(debugActOpenDevices);
	debugMenu->addSeparator();
	debugMenu->addAction(dbgActRun);
	debugMenu->addAction(dbgActRunAndHide);
	debugMenu->addAction(dbgActRunToNextCpu);
	debugMenu->addAction(dbgActRunNextInt);
	debugMenu->addAction(dbgActRunNextVBlank);
	debugMenu->addSeparator();
	debugMenu->addAction(dbgActStepInto);
	debugMenu->addAction(dbgActStepOver);
	debugMenu->addAction(dbgActStepOut);
	debugMenu->addSeparator();
	debugMenu->addAction(dbgActSoftReset);
	debugMenu->addAction(dbgActHardReset);
	debugMenu->addSeparator();
	debugMenu->addAction(dbgActClose);
	debugMenu->addAction(dbgActQuit);
}


WindowQt::~WindowQt()
{
}

void WindowQt::debugActOpenMemory()
{
	MemoryWindow* foo = new MemoryWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActOpenDasm()
{
	DasmWindow* foo = new DasmWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActOpenLog()
{
	LogWindow* foo = new LogWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActOpenPoints()
{
	BreakpointsWindow* foo = new BreakpointsWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActOpenDevices()
{
	DevicesWindow* foo = new DevicesWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActRun()
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->go();
}

void WindowQt::debugActRunAndHide()
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->go();
	hideAll();
}

void WindowQt::debugActRunToNextCpu()
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->go_next_device();
}

void WindowQt::debugActRunNextInt()
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->go_interrupt();
}

void WindowQt::debugActRunNextVBlank()
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->go_vblank();
}

void WindowQt::debugActStepInto()
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step();
}

void WindowQt::debugActStepOver()
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step_over();
}

void WindowQt::debugActStepOut()
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step_out();
}

void WindowQt::debugActSoftReset()
{
	m_machine->schedule_soft_reset();
	debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step();
}

void WindowQt::debugActHardReset()
{
	m_machine->schedule_hard_reset();
}

void WindowQt::debugActClose()
{
	close();
}

void WindowQt::debugActQuit()
{
	m_machine->schedule_exit();
}


//=========================================================================
//  WindowQtConfig
//=========================================================================
void WindowQtConfig::buildFromQWidget(QWidget* widget)
{
	m_position.setX(widget->geometry().topLeft().x());
	m_position.setY(widget->geometry().topLeft().y());
	m_size.setX(widget->size().width());
	m_size.setY(widget->size().height());
}


void WindowQtConfig::applyToQWidget(QWidget* widget)
{
	widget->setGeometry(m_position.x(), m_position.y(), m_size.x(), m_size.y());
}


void WindowQtConfig::addToXmlDataNode(xml_data_node* node) const
{
	xml_set_attribute_int(node, "type", m_type);
	xml_set_attribute_int(node, "position_x", m_position.x());
	xml_set_attribute_int(node, "position_y", m_position.y());
	xml_set_attribute_int(node, "size_x", m_size.x());
	xml_set_attribute_int(node, "size_y", m_size.y());
}


void WindowQtConfig::recoverFromXmlNode(xml_data_node* node)
{
	m_size.setX(xml_get_attribute_int(node, "size_x", m_size.x()));
	m_size.setY(xml_get_attribute_int(node, "size_y", m_size.y()));
	m_position.setX(xml_get_attribute_int(node, "position_x", m_position.x()));
	m_position.setY(xml_get_attribute_int(node, "position_y", m_position.y()));
	m_type = (WindowQtConfig::WindowType)xml_get_attribute_int(node, "type", m_type);
}
