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


namespace {

class debug_qt : public osd_module, public debug_module
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

	virtual int init(const osd_options &options) { return 0; }
	virtual void exit();

	virtual void init_debugger(running_machine &machine);
	virtual void wait_for_debugger(device_t &device, bool firststop);
	virtual void debugger_update();
#if defined(_WIN32) && !defined(SDLMAME_WIN32)
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override
	{
		winwindow_qt_filter(message);
		return false;
	}
#endif

private:
	void configuration_load(config_type which_type, config_level level, util::xml::data_node const *parentnode);
	void configuration_save(config_type which_type, util::xml::data_node *parentnode);
	void load_window_configurations(util::xml::data_node const &parentnode);

	running_machine *m_machine;
	osd::debugger::qt::MainWindow *m_mainwindow;
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
	if (m_mainwindow)
	{
		m_mainwindow->setExiting();
		QApplication::closeAllWindows();
		qApp->processEvents(QEventLoop::AllEvents, 1);
		m_mainwindow = nullptr;
	}
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
		m_mainwindow = new osd::debugger::qt::MainWindow(*m_machine);
		if (m_config)
		{
			load_window_configurations(*m_config->get_first_child());
			m_config.reset();
		}
		m_mainwindow->show();
	}

	// Ensure all top level widgets are visible & bring main window to front
	foreach (QWidget *widget, QApplication::topLevelWidgets())
	{
		if (widget->isWindow() && (widget->windowType() == Qt::Window))
			widget->show();
	}

	if (firststop)
	{
		m_mainwindow->activateWindow();
		m_mainwindow->raise();
	}

	// Set the main window to display the proper cpu
	m_mainwindow->setProcessor(&device);

	// Run our own QT event loop
	osd_sleep(osd_ticks_per_second() / 1000 * 50);
	qApp->processEvents(QEventLoop::AllEvents, 1);

	// Refresh everyone if requested
	if (m_mainwindow->wantsRefresh())
	{
		QWidgetList allWidgets = qApp->allWidgets();
		for (int i = 0; i < allWidgets.length(); i++)
			allWidgets[i]->update();
		m_mainwindow->clearRefreshFlag();
	}

	// Hide all top level widgets if requested
	if (m_mainwindow->wantsHide())
	{
		foreach (QWidget *widget, QApplication::topLevelWidgets())
		{
			if (widget->isWindow() && (widget->windowType() == Qt::Window))
				widget->hide();
		}
		m_mainwindow->clearHideFlag();
	}

#if defined(_WIN32) && !defined(SDLMAME_WIN32)
	winwindow_update_cursor_state(*m_machine); // make sure the cursor isn't hidden while in debugger
#endif
}


//============================================================
//  Available for video.*
//============================================================

void debug_qt::debugger_update()
{
	qApp->processEvents(QEventLoop::AllEvents, 1);
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
	{
		// Loop over all the open windows
		for (QWidget *widget : QApplication::topLevelWidgets())
		{
			if (!widget->isWindow() || (widget->windowType() != Qt::Window))
				continue;

			osd::debugger::qt::WindowQt *const win = dynamic_cast<osd::debugger::qt::WindowQt *>(widget);
			if (win)
				win->saveConfiguration(*parentnode);
		}
	}
}


void debug_qt::load_window_configurations(util::xml::data_node const &parentnode)
{
	for (util::xml::data_node const *wnode = parentnode.get_child(osd::debugger::NODE_WINDOW); wnode; wnode = wnode->get_next_sibling(osd::debugger::NODE_WINDOW))
	{
		std::unique_ptr<osd::debugger::qt::WindowQtConfig> cfg;
		osd::debugger::qt::WindowQt *win = nullptr;
		switch (wnode->get_attribute_int(osd::debugger::ATTR_WINDOW_TYPE, -1))
		{
		case osd::debugger::WINDOW_TYPE_CONSOLE:
			cfg = std::make_unique<osd::debugger::qt::MainWindowQtConfig>();
			break;
		case osd::debugger::WINDOW_TYPE_MEMORY_VIEWER:
			cfg = std::make_unique<osd::debugger::qt::MemoryWindowQtConfig>();
			win = new osd::debugger::qt::MemoryWindow(*m_machine);
			break;
		case osd::debugger::WINDOW_TYPE_DISASSEMBLY_VIEWER:
			cfg = std::make_unique<osd::debugger::qt::DasmWindowQtConfig>();
			win = new osd::debugger::qt::DasmWindow(*m_machine);
			break;
		case osd::debugger::WINDOW_TYPE_ERROR_LOG_VIEWER:
			cfg = std::make_unique<osd::debugger::qt::LogWindowQtConfig>();
			win = new osd::debugger::qt::LogWindow(*m_machine);
			break;
		case osd::debugger::WINDOW_TYPE_POINTS_VIEWER:
			cfg = std::make_unique<osd::debugger::qt::BreakpointsWindowQtConfig>();
			win = new osd::debugger::qt::BreakpointsWindow(*m_machine);
			break;
		case osd::debugger::WINDOW_TYPE_DEVICES_VIEWER:
			cfg = std::make_unique<osd::debugger::qt::DevicesWindowQtConfig>();
			win = new osd::debugger::qt::DevicesWindow(*m_machine);
			break;
		case osd::debugger::WINDOW_TYPE_DEVICE_INFO_VIEWER:
			cfg = std::make_unique<osd::debugger::qt::DeviceInformationWindowQtConfig>();
			win = new osd::debugger::qt::DeviceInformationWindow(*m_machine);
			break;
		}
		if (cfg)
		{
			cfg->recoverFromXmlNode(*wnode);
			if (win)
				cfg->applyToQWidget(win);
			else if (osd::debugger::WINDOW_TYPE_CONSOLE == cfg->m_type)
				cfg->applyToQWidget(m_mainwindow);
		}
	}
}

} // anonymous namespace

#else // USE_QTDEBUG

MODULE_NOT_SUPPORTED(debug_qt, OSD_DEBUG_PROVIDER, "qt")

#endif

MODULE_DEFINITION(DEBUG_QT, debug_qt)
