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
#include "osdsdl.h"
#include "config.h"
#include "debugger.h"

#include "debugqtmainwindow.h"
#include "debugqtmemorywindow.h"
#include "debugqtdasmwindow.h"
#include "debugqtlogwindow.h"


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
std::vector<WindowQtConfig> xmlConfigurations;

static void xml_configuration_load(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *wnode;

	// We only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// Might not have any data
	if (parentnode == NULL)
		return;

	xmlConfigurations.clear();

	// Configuration load
	for (wnode = xml_get_sibling(parentnode->child, "window"); wnode != NULL; wnode = xml_get_sibling(wnode->next, "window"))
	{
		WindowQtConfig config;
		config.m_size.setX(xml_get_attribute_int(wnode, "size_x", config.m_size.x()));
		config.m_size.setY(xml_get_attribute_int(wnode, "size_y", config.m_size.y()));
		config.m_position.setX(xml_get_attribute_int(wnode, "position_x", config.m_position.x()));
		config.m_position.setY(xml_get_attribute_int(wnode, "position_y", config.m_position.y()));
		config.m_type = (WindowQtConfig::WindowType)xml_get_attribute_int(wnode, "type", config.m_type);
		xmlConfigurations.push_back(config);
	}
}


static void xml_configuration_save(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	// We only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	for (int i = 0; i < xmlConfigurations.size(); i++)
	{
		// Create an xml node
		xml_data_node *debugger_node;
		debugger_node = xml_add_child(parentnode, "window", NULL);
		if (debugger_node == NULL) 
			continue;

		xml_set_attribute_int(debugger_node, "type", xmlConfigurations[i].m_type);
		xml_set_attribute_int(debugger_node, "position_x", xmlConfigurations[i].m_position.x());
		xml_set_attribute_int(debugger_node, "position_y", xmlConfigurations[i].m_position.y());
		xml_set_attribute_int(debugger_node, "size_x", xmlConfigurations[i].m_size.x());
		xml_set_attribute_int(debugger_node, "size_y", xmlConfigurations[i].m_size.y());
	}
}


static void gather_save_configurations()
{
	xmlConfigurations.clear();

	// Loop over all the open windows
	foreach (QWidget* widget, QApplication::topLevelWidgets())
	{
		if (!widget->isVisible())
			continue;

		if (!widget->isWindow() || widget->windowType() != Qt::Window)
			continue;

		// Figure out its type
		WindowQtConfig::WindowType type = WindowQtConfig::WIN_TYPE_UNKNOWN;
		if (dynamic_cast<MainWindow*>(widget))
			type = WindowQtConfig::WIN_TYPE_MAIN;
		else if (dynamic_cast<MemoryWindow*>(widget))
			type = WindowQtConfig::WIN_TYPE_MEMORY;
		else if (dynamic_cast<DasmWindow*>(widget))
			type = WindowQtConfig::WIN_TYPE_DISASM;
		else if (dynamic_cast<LogWindow*>(widget))
			type = WindowQtConfig::WIN_TYPE_LOG;

		WindowQtConfig config;
		config.m_type = type;
		config.m_position.setX(widget->geometry().topLeft().x());
		config.m_position.setY(widget->geometry().topLeft().y());
		config.m_size.setX(widget->size().width());
		config.m_size.setY(widget->size().height());
		xmlConfigurations.push_back(config);
	}
}


//============================================================
//  Utilities
//============================================================

static void load_and_clear_main_window_config(std::vector<WindowQtConfig>& configs)
{
	if (configs.size() == 0)
		return;

	int i = 0;
	for (i = 0; i < configs.size(); i++)
	{
		if (configs[i].m_type == WindowQtConfig::WIN_TYPE_MAIN)
		{
			mainQtWindow->setGeometry(configs[i].m_position.x(), configs[i].m_position.y(),
									  configs[i].m_size.x(), configs[i].m_size.y());
			break;
		}
	}
	configs.erase(configs.begin()+i);
}


static void setup_additional_startup_windows(running_machine& machine, std::vector<WindowQtConfig>& configs)
{
	for (int i = 0; i < configs.size(); i++)
	{
		WindowQt* foo = NULL;
		switch (configs[i].m_type)
		{
			case WindowQtConfig::WIN_TYPE_MEMORY:
				foo = new MemoryWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_DISASM:
				foo = new DasmWindow(&machine); break;
			case WindowQtConfig::WIN_TYPE_LOG:
				foo = new LogWindow(&machine); break;
			default: break;
		}
		foo->setGeometry(configs[i].m_position.x(), configs[i].m_position.y(),
						 configs[i].m_size.x(), configs[i].m_size.y());
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

void sdl_osd_interface::init_debugger()
{
	if (qApp == NULL)
	{
		// If you're starting from scratch, create a new qApp
		new QApplication(qtArgc, qtArgv);
	}
	else
	{
		// If you're doing a hard reset, clear out existing widgets & get ready for re-init
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

void sdl_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
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
