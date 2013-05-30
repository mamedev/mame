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

#include <QtGui/QtGui>
#include <QtGui/QApplication>

#include "emu.h"
#if defined(SDL)
#include "osdsdl.h"
#define xxx_osd_interface sdl_osd_interface
#elif defined(WIN32)
#include "winmain.h"
#define xxx_osd_interface windows_osd_interface
#endif
#include "config.h"
#include "debugger.h"

#include "debugqtlogwindow.h"
#include "debugqtmainwindow.h"
#include "debugqtdasmwindow.h"
#include "debugqtmemorywindow.h"


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
simple_list<WindowQtConfig> xmlConfigurations;


static void xml_configuration_load(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	// We only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// Might not have any data
	if (parentnode == NULL)
		return;

	xmlConfigurations.reset();

	// Configuration load
	xml_data_node* wnode = NULL;
	for (wnode = xml_get_sibling(parentnode->child, "window"); wnode != NULL; wnode = xml_get_sibling(wnode->next, "window"))
	{
		WindowQtConfig* config = NULL;
		WindowQtConfig::WindowType type = (WindowQtConfig::WindowType)xml_get_attribute_int(wnode, "type", WindowQtConfig::WIN_TYPE_UNKNOWN);
		switch (type)
		{
			case WindowQtConfig::WIN_TYPE_MAIN:   config = &xmlConfigurations.append(*global_alloc(MainWindowQtConfig)); break;
			case WindowQtConfig::WIN_TYPE_MEMORY: config = &xmlConfigurations.append(*global_alloc(MemoryWindowQtConfig)); break;
			case WindowQtConfig::WIN_TYPE_DASM:   config = &xmlConfigurations.append(*global_alloc(DasmWindowQtConfig)); break;
			case WindowQtConfig::WIN_TYPE_LOG:    config = &xmlConfigurations.append(*global_alloc(LogWindowQtConfig)); break;
			default: break;
		}
		config->recoverFromXmlNode(wnode);
	}
}


static void xml_configuration_save(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	// We only write to game configurations
	if (config_type != CONFIG_TYPE_GAME)
		return;

	for (WindowQtConfig* config = xmlConfigurations.first(); config != NULL; config = config->next())
	{
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
	xmlConfigurations.reset();

	// Loop over all the open windows
	foreach (QWidget* widget, QApplication::topLevelWidgets())
	{
		if (!widget->isVisible())
			continue;

		if (!widget->isWindow() || widget->windowType() != Qt::Window)
			continue;

		// Figure out its type
		WindowQtConfig* config = NULL;
		if (dynamic_cast<MainWindow*>(widget))
			config = &xmlConfigurations.append(*global_alloc(MainWindowQtConfig));
		else if (dynamic_cast<MemoryWindow*>(widget))
			config = &xmlConfigurations.append(*global_alloc(MemoryWindowQtConfig));
		else if (dynamic_cast<DasmWindow*>(widget))
			config = &xmlConfigurations.append(*global_alloc(DasmWindowQtConfig));
		else if (dynamic_cast<LogWindow*>(widget))
			config = &xmlConfigurations.append(*global_alloc(LogWindowQtConfig));

		config->buildFromQWidget(widget);
	}
}


//============================================================
//  Utilities
//============================================================

static void load_and_clear_main_window_config(simple_list<WindowQtConfig>& configList)
{
	for (WindowQtConfig* config = xmlConfigurations.first(); config != NULL; config = config->next())
	{
		if (config->m_type == WindowQtConfig::WIN_TYPE_MAIN)
		{
			config->applyToQWidget(mainQtWindow);
			configList.remove(*config);
			break;
		}
	}
}


static void setup_additional_startup_windows(running_machine& machine, simple_list<WindowQtConfig>& configList)
{
	for (WindowQtConfig* config = xmlConfigurations.first(); config != NULL; config = config->next())
	{
		WindowQt* foo = NULL;
		switch (config->m_type)
		{
			case WindowQtConfig::WIN_TYPE_MEMORY:
				foo = new MemoryWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_DASM:
				foo = new DasmWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_LOG:
				foo = new LogWindow(&machine); break;
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

void xxx_osd_interface::init_debugger()
{
	if (qApp == NULL)
	{
		// If you're starting from scratch, create a new qApp
		new QApplication(qtArgc, qtArgv);
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

#ifdef SDLMAME_UNIX
extern int sdl_entered_debugger;
#endif

void xxx_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
#if defined(SDL)
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
