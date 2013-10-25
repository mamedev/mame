//============================================================
//
//  debugqt.c - SDL/QT debug window handling
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#if !defined(NO_DEBUGGER)

#define NO_MEM_TRACKING

#include <vector>

#include <QtGui/QtGui>
#include <QtGui/QApplication>

#include "emu.h"
#if defined(WIN32) && !defined(SDLMAME_WIN32)
#include "winmain.h"
#define xxx_osd_interface windows_osd_interface
#else
#include "osdsdl.h"
#define xxx_osd_interface sdl_osd_interface
#endif
#include "config.h"
#include "debugger.h"

#include "debugqtlogwindow.h"
#include "debugqtmainwindow.h"
#include "debugqtdasmwindow.h"
#include "debugqtmemorywindow.h"
#include "debugqtbreakpointswindow.h"

#include "debugqt.h"


//============================================================
//  "Global" variables to make QT happy
//============================================================

int qtArgc = 0;
char** qtArgv = NULL;

bool oneShot = true;
MainWindow* mainQtWindow = NULL;


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
			case WindowQtConfig::WIN_TYPE_MAIN:         xmlConfigurations.push_back(new MainWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_MEMORY:       xmlConfigurations.push_back(new MemoryWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_DASM:         xmlConfigurations.push_back(new DasmWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_LOG:          xmlConfigurations.push_back(new LogWindowQtConfig()); break;
			case WindowQtConfig::WIN_TYPE_BREAK_POINTS: xmlConfigurations.push_back(new BreakpointsWindowQtConfig()); break;
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

void xxx_osd_interface::init_debugger()
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

	// Setup the configuration XML saving and loading
	config_register(machine(),
					"debugger",
					config_saveload_delegate(FUNC(xml_configuration_load), &machine()),
					config_saveload_delegate(FUNC(xml_configuration_save), &machine()));
}


//============================================================
//  Core functionality
//============================================================

#if defined(SDLMAME_UNIX) || defined(SDLMAME_WIN32)
extern int sdl_entered_debugger;
#elif defined(WIN32)
void winwindow_update_cursor_state(running_machine &machine);
#endif

void xxx_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
#if defined(SDLMAME_UNIX) || defined(SDLMAME_WIN32)
	sdl_entered_debugger = 1;
#endif

	// Dialog initialization
	if (oneShot)
	{
		mainQtWindow = new MainWindow(&machine());
		load_and_clear_main_window_config(xmlConfigurations);
		setup_additional_startup_windows(machine(), xmlConfigurations);
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
	bring_main_window_to_front();

	// Set the main window to display the proper cpu
	mainQtWindow->setProcessor(&device);

	// Run our own QT event loop
	while (debug_cpu_is_stopped(machine()))
	{
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
		if (machine().scheduled_event_pending())
		{
			// Keep a list of windows we want to save.
			// We need to do this here because by the time xml_configuration_save gets called
			// all the QT windows are already gone.
			gather_save_configurations();
			break;
		}
#if defined(WIN32) && !defined(SDLMAME_WIN32)
		winwindow_update_cursor_state(machine()); // make sure the cursor isn't hidden while in debugger
#endif
	}
}


//============================================================
//  Available for video.*
//============================================================

void debugwin_update_during_game(running_machine &machine)
{
	qApp->processEvents(QEventLoop::AllEvents, 1);
}



#else



#include "sdlinc.h"

#include "emu.h"
#include "osdepend.h"
#include "osdsdl.h"

// win32 stubs for linking
void sdl_osd_interface::init_debugger()
{
}

void sdl_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
}

// win32 stubs for linking
void debugwin_update_during_game(running_machine &machine)
{
}

#endif
