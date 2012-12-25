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
#include "debugger.h"

#include "debugqtmainwindow.h"


//============================================================
//  "Global" variables to make QT happy
//============================================================

int qtArgc = 0;
char** qtArgv = NULL;

bool oneShot = true;
MainWindow* mainQtWindow = NULL;


//============================================================
//  Core functionality
//============================================================

void sdl_osd_interface::init_debugger()
{
    // QT is a magical thing
    new QApplication(qtArgc, qtArgv);
}


//============================================================
//  Core functionality
//============================================================

void sdl_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
    if (oneShot)
    {
        mainQtWindow = new MainWindow(&device, &machine());
        mainQtWindow->show();
        oneShot = false;
    }

    // Make sure the main window displays the proper cpu
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

        // Exit if the machine has been instructed to do so
        if (machine().exit_pending())
        {
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
