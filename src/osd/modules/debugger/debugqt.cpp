// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
//============================================================
//
//  debugqt.c - SDL/QT debug window handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "debug_module.h"
#include "modules/osdmodule.h"

#if (USE_QTDEBUG)

#include <vector>

#include <QtWidgets/QApplication>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QAbstractNativeEventFilter>

#include "emu.h"
#include "config.h"
#include "debugger.h"
#include "modules/lib/osdobj_common.h"

#include "qt/logwindow.h"
#include "qt/mainwindow.h"
#include "qt/dasmwindow.h"
#include "qt/memorywindow.h"
#include "qt/breakpointswindow.h"
#include "qt/deviceswindow.h"
#include "qt/deviceinformationwindow.h"

#include "util/xmlfile.h"


#if defined(SDLMAME_UNIX) || defined(SDLMAME_WIN32)
extern int sdl_entered_debugger;
#elif defined(_WIN32)
void winwindow_update_cursor_state(running_machine &machine);
bool winwindow_qt_filter(void *message);
#endif


namespace osd {

namespace {

class debug_qt : public osd_module, public debug_module, protected debugger::qt::DebuggerQt
#if defined(_WIN32) && !defined(SDLMAME_WIN32)
, public QAbstractNativeEventFilter
#endif
{
public:
	debug_qt() :
		osd_module(OSD_DEBUG_PROVIDER, "qt"),
		debug_module(),
		m_machine(nullptr),
		m_mainwindow(nullptr)
	{
	}

	virtual ~debug_qt() { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override;

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;
#if defined(_WIN32) && !defined(SDLMAME_WIN32)
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override
	{
		winwindow_qt_filter(message);
		return false;
	}
#endif

	virtual running_machine &machine() const override { return *m_machine; }

private:
	void configuration_load(config_type which_type, config_level level, util::xml::data_node const *parentnode);
	void configuration_save(config_type which_type, util::xml::data_node *parentnode);
	void load_window_configurations(util::xml::data_node const &parentnode);

	running_machine *m_machine;
	debugger::qt::MainWindow *m_mainwindow;
	util::xml::file::ptr m_config;
};


//============================================================
//  "Global" variables to make QT happy
//============================================================

int qtArgc = 1;
char qtArg0[] = "mame";
char *qtArgv[] = { qtArg0, nullptr };


//============================================================
//  Core functionality
//============================================================

void debug_qt::exit()
{
	// If you've done a hard reset, clear out existing widgets
	emit exitDebugger();
	m_mainwindow = nullptr;
}


void debug_qt::init_debugger(running_machine &machine)
{
	if (!qApp)
	{
		// If you're starting from scratch, create a new qApp
		new QApplication(qtArgc, qtArgv);
#if defined(_WIN32) && !defined(SDLMAME_WIN32)
		QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
#endif
	}

	m_machine = &machine;

	// Setup the configuration XML saving and loading
	machine.configuration().config_register("debugger",
			configuration_manager::load_delegate(&debug_qt::configuration_load, this),
			configuration_manager::save_delegate(&debug_qt::configuration_save, this));
}


//============================================================
//  Core functionality
//============================================================

void debug_qt::wait_for_debugger(device_t &device, bool firststop)
{
#if defined(SDLMAME_UNIX) || defined(SDLMAME_WIN32)
	sdl_entered_debugger = 1;
#endif

	// Dialog initialization
	if (!m_mainwindow)
	{
		m_mainwindow = new debugger::qt::MainWindow(*this);
		if (m_config)
		{
			load_window_configurations(*m_config->get_first_child());
			m_config.reset();
		}
		m_mainwindow->show();
	}

	// Ensure all top level widgets are visible & bring main window to front
	emit showAllWindows();

	if (firststop)
	{
		m_mainwindow->activateWindow();
		m_mainwindow->raise();
	}

	// Set the main window to display the proper CPU
	m_mainwindow->setProcessor(&device);

	// Run our own QT event loop
	osd_sleep(osd_ticks_per_second() / 1000 * 50);
	qApp->processEvents(QEventLoop::AllEvents, 1);

#if defined(_WIN32) && !defined(SDLMAME_WIN32)
	winwindow_update_cursor_state(*m_machine); // make sure the cursor isn't hidden while in debugger
#endif
}


//============================================================
//  Available for video.*
//============================================================

void debug_qt::debugger_update()
{
	qApp->processEvents(QEventLoop::AllEvents);
}


void debug_qt::configuration_load(config_type which_type, config_level level, util::xml::data_node const *parentnode)
{
	// We only care about system configuration files for now
	if ((config_type::SYSTEM == which_type) && parentnode)
	{
		if (m_mainwindow)
		{
			load_window_configurations(*parentnode);
		}
		else
		{
			m_config = util::xml::file::create();
			parentnode->copy_into(*m_config);
		}
	}
}


void debug_qt::configuration_save(config_type which_type, util::xml::data_node *parentnode)
{
	// We only save system configuration for now
	if ((config_type::SYSTEM == which_type) && parentnode)
		emit saveConfiguration(*parentnode);
}


void debug_qt::load_window_configurations(util::xml::data_node const &parentnode)
{
	for (util::xml::data_node const *wnode = parentnode.get_child(debugger::NODE_WINDOW); wnode; wnode = wnode->get_next_sibling(debugger::NODE_WINDOW))
	{
		debugger::qt::WindowQt *win = nullptr;
		switch (wnode->get_attribute_int(debugger::ATTR_WINDOW_TYPE, -1))
		{
		case debugger::WINDOW_TYPE_CONSOLE:
			win = m_mainwindow;
			break;
		case debugger::WINDOW_TYPE_MEMORY_VIEWER:
			win = new debugger::qt::MemoryWindow(*this);
			break;
		case debugger::WINDOW_TYPE_DISASSEMBLY_VIEWER:
			win = new debugger::qt::DasmWindow(*this);
			break;
		case debugger::WINDOW_TYPE_ERROR_LOG_VIEWER:
			win = new debugger::qt::LogWindow(*this);
			break;
		case debugger::WINDOW_TYPE_POINTS_VIEWER:
			win = new debugger::qt::BreakpointsWindow(*this);
			break;
		case debugger::WINDOW_TYPE_DEVICES_VIEWER:
			win = new debugger::qt::DevicesWindow(*this);
			break;
		case debugger::WINDOW_TYPE_DEVICE_INFO_VIEWER:
			win = new debugger::qt::DeviceInformationWindow(*this);
			break;
		}
		if (win)
			win->restoreConfiguration(*wnode);
	}
}

} // anonymous namespace

} // namespace osd

#else // USE_QTDEBUG

namespace osd { namespace { MODULE_NOT_SUPPORTED(debug_qt, OSD_DEBUG_PROVIDER, "qt") } }

#endif

MODULE_DEFINITION(DEBUG_QT, osd::debug_qt)
