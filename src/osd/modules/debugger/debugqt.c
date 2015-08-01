// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
//============================================================
//
//  debugqt.c - SDL/QT debug window handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#if (!defined(NO_MEM_TRACKING))
#define NO_MEM_TRACKING
#endif

#include "debug_module.h"
#include "modules/osdmodule.h"

#if (USE_QTDEBUG)

#include <vector>

#include <QtGui/QtGui>
#include <QtGui/QApplication>

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

class debug_qt : public osd_module, public debug_module
{
public:
	debug_qt()
	: osd_module(OSD_DEBUG_PROVIDER, "qt"), debug_module(),
		m_machine(NULL)
	{
	}

	virtual ~debug_qt() { }

	virtual int init(const osd_options &options) { return 0; }
	virtual void exit() { }

	virtual void init_debugger(running_machine &machine);
	virtual void wait_for_debugger(device_t &device, bool firststop);
	virtual void debugger_update();

private:
	running_machine *m_machine;
};

//============================================================
//  "Global" variables to make QT happy
//============================================================

int qtArgc = 0;
char** qtArgv = NULL;

bool oneShot = true;
static MainWindow* mainQtWindow = NULL;

//============================================================
//  XML configuration save/load
//============================================================

// Global variable used to feed the xml configuration callbacks
std::vector<WindowQtConfig*> xmlConfigurations;


static void xml_configuration_load(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	// We only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// Might not have any data
	if (parentnode == NULL)
		return;

	for (int i = 0; i < xmlConfigurations.size(); i++)
		delete xmlConfigurations[i];
	xmlConfigurations.clear();

	// Configuration load
	xml_data_node* wnode = NULL;
	for (wnode = xml_get_sibling(parentnode->child, "window"); wnode != NULL; wnode = xml_get_sibling(wnode->next, "window"))
	{
		WindowQtConfig::WindowType type = (WindowQtConfig::WindowType)xml_get_attribute_int(wnode, "type", WindowQtConfig::WIN_TYPE_UNKNOWN);
		switch (type)
		{
			case WindowQtConfig::WIN_TYPE_MAIN:               xmlConfigurations.push_back(new MainWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_MEMORY:             xmlConfigurations.push_back(new MemoryWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_DASM:               xmlConfigurations.push_back(new DasmWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_LOG:                xmlConfigurations.push_back(new LogWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_BREAK_POINTS:       xmlConfigurations.push_back(new BreakpointsWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_DEVICES:            xmlConfigurations.push_back(new DevicesWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_DEVICE_INFORMATION: xmlConfigurations.push_back(new DeviceInformationWindowQtConfig()); break;
			default: continue;
		}
		xmlConfigurations.back()->recoverFromXmlNode(wnode);
	}
}


static void xml_configuration_save(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	// We only write to game configurations
	if (config_type != CONFIG_TYPE_GAME)
		return;

	for (int i = 0; i < xmlConfigurations.size(); i++)
	{
		WindowQtConfig* config = xmlConfigurations[i];

		// Create an xml node
		xml_data_node *debugger_node;
		debugger_node = xml_add_child(parentnode, "window", NULL);
		if (debugger_node == NULL)
			continue;

		// Insert the appropriate information
		config->addToXmlDataNode(debugger_node);
	}
}


static void gather_save_configurations()
{
	for (int i = 0; i < xmlConfigurations.size(); i++)
		delete xmlConfigurations[i];
	xmlConfigurations.clear();

	// Loop over all the open windows
	foreach (QWidget* widget, QApplication::topLevelWidgets())
	{
		if (!widget->isVisible())
			continue;

		if (!widget->isWindow() || widget->windowType() != Qt::Window)
			continue;

		// Figure out its type
		if (dynamic_cast<MainWindow*>(widget))
			xmlConfigurations.push_back(new MainWindowQtConfig());
		else if (dynamic_cast<MemoryWindow*>(widget))
			xmlConfigurations.push_back(new MemoryWindowQtConfig());
		else if (dynamic_cast<DasmWindow*>(widget))
			xmlConfigurations.push_back(new DasmWindowQtConfig());
		else if (dynamic_cast<LogWindow*>(widget))
			xmlConfigurations.push_back(new LogWindowQtConfig());
		else if (dynamic_cast<BreakpointsWindow*>(widget))
			xmlConfigurations.push_back(new BreakpointsWindowQtConfig());
		else if (dynamic_cast<DevicesWindow*>(widget))
			xmlConfigurations.push_back(new DevicesWindowQtConfig());
		else if (dynamic_cast<DeviceInformationWindow*>(widget))
			xmlConfigurations.push_back(new DeviceInformationWindowQtConfig());

		xmlConfigurations.back()->buildFromQWidget(widget);
	}
}


//============================================================
//  Utilities
//============================================================

static void load_and_clear_main_window_config(std::vector<WindowQtConfig*>& configList)
{
	for (int i = 0; i < configList.size(); i++)
	{
		WindowQtConfig* config = configList[i];
		if (config->m_type == WindowQtConfig::WIN_TYPE_MAIN)
		{
			config->applyToQWidget(mainQtWindow);
			configList.erase(configList.begin()+i);
			break;
		}
	}
}


static void setup_additional_startup_windows(running_machine& machine, std::vector<WindowQtConfig*>& configList)
{
	for (int i = 0; i < configList.size(); i++)
	{
		WindowQtConfig* config = configList[i];

		WindowQt* foo = NULL;
		switch (config->m_type)
		{
			case WindowQtConfig::WIN_TYPE_MEMORY:
				foo = new MemoryWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_DASM:
				foo = new DasmWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_LOG:
				foo = new LogWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_BREAK_POINTS:
				foo = new BreakpointsWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_DEVICES:
				foo = new DevicesWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_DEVICE_INFORMATION:
				foo = new DeviceInformationWindow(&machine); break;
			default: break;
		}
		config->applyToQWidget(foo);
		foo->show();
	}
}


static void bring_main_window_to_front()
{
	foreach (QWidget* widget, QApplication::topLevelWidgets())
	{
		if (!dynamic_cast<MainWindow*>(widget))
			continue;
		widget->activateWindow();
		widget->raise();
	}
}


//============================================================
//  Core functionality
//============================================================

#if defined(WIN32) && !defined(SDLMAME_WIN32)
bool winwindow_qt_filter(void *message);
#endif

void debug_qt::init_debugger(running_machine &machine)
{
	if (qApp == NULL)
	{
		// If you're starting from scratch, create a new qApp
		new QApplication(qtArgc, qtArgv);
#if defined(WIN32) && !defined(SDLMAME_WIN32)
		QAbstractEventDispatcher::instance()->setEventFilter((QAbstractEventDispatcher::EventFilter)&winwindow_qt_filter);
#endif
	}
	else
	{
		// If you've done a hard reset, clear out existing widgets & get ready for re-init
		foreach (QWidget* widget, QApplication::topLevelWidgets())
		{
			if (!widget->isWindow() || widget->windowType() != Qt::Window)
				continue;
			delete widget;
		}
		oneShot = true;
	}

	m_machine = &machine;
	// Setup the configuration XML saving and loading
	config_register(machine,
					"debugger",
					config_saveload_delegate(FUNC(xml_configuration_load), &machine),
					config_saveload_delegate(FUNC(xml_configuration_save), &machine));
}


//============================================================
//  Core functionality
//============================================================

#if defined(SDLMAME_UNIX) || defined(SDLMAME_WIN32)
extern int sdl_entered_debugger;
#elif defined(WIN32)
void winwindow_update_cursor_state(running_machine &machine);
#endif

void debug_qt::wait_for_debugger(device_t &device, bool firststop)
{
#if defined(SDLMAME_UNIX) || defined(SDLMAME_WIN32)
	sdl_entered_debugger = 1;
#endif

	// Dialog initialization
	if (oneShot)
	{
		mainQtWindow = new MainWindow(m_machine);
		load_and_clear_main_window_config(xmlConfigurations);
		setup_additional_startup_windows(*m_machine, xmlConfigurations);
		mainQtWindow->show();
		oneShot = false;
	}

	// Insure all top level widgets are visible & bring main window to front
	foreach (QWidget* widget, QApplication::topLevelWidgets())
	{
		if (!widget->isWindow() || widget->windowType() != Qt::Window)
			continue;
		widget->show();
	}

	if (firststop)
	{
		bring_main_window_to_front();
	}

	// Set the main window to display the proper cpu
	mainQtWindow->setProcessor(&device);

	// Run our own QT event loop
	osd_sleep(50000);
	qApp->processEvents(QEventLoop::AllEvents, 1);

	// Refresh everyone if requested
	if (mainQtWindow->wantsRefresh())
	{
		QWidgetList allWidgets = qApp->allWidgets();
		for (int i = 0; i < allWidgets.length(); i++)
			allWidgets[i]->update();
		mainQtWindow->clearRefreshFlag();
	}

	// Hide all top level widgets if requested
	if (mainQtWindow->wantsHide())
	{
		foreach (QWidget* widget, QApplication::topLevelWidgets())
		{
			if (!widget->isWindow() || widget->windowType() != Qt::Window)
				continue;
			widget->hide();
		}
		mainQtWindow->clearHideFlag();
	}

	// Exit if the machine has been instructed to do so (scheduled event == exit || hard_reset)
	if (m_machine->scheduled_event_pending())
	{
		// Keep a list of windows we want to save.
		// We need to do this here because by the time xml_configuration_save gets called
		// all the QT windows are already gone.
		gather_save_configurations();
	}
#if defined(WIN32) && !defined(SDLMAME_WIN32)
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

#else /* SDLMAME_UNIX */
	MODULE_NOT_SUPPORTED(debug_qt, OSD_DEBUG_PROVIDER, "qt")
#endif

MODULE_DEFINITION(DEBUG_QT, debug_qt)
