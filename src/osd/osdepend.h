/***************************************************************************

    osdepend.h

    OS-dependent code interface.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    The prototypes in this file describe the interfaces that the MAME core
    relies upon to interact with the outside world. They are broken out into
    several categories.

    The general flow for an OSD port of MAME is as follows:

        - parse the command line or display the frontend
        - call run_game (mame.c) with the index in the driver list of
            the game selected
            - osd_init() is called shortly afterwards; at this time, you are
                expected to set up the display system and create render_targets
            - the input system will call osd_get_code_list()
            - the input port system will call osd_customize_inputport_list()
            - the sound system will call osd_start_audio_stream()
            - while the game runs, osd_update() will be called periodically
            - when the game exits, we return from run_game()
        - the OSD layer is now in control again

    This process is expected to be in flux over the next several versions
    (this was written during 0.109u2 development) as some of the OSD
    responsibilities are pushed into the core.

*******************************************************************c********/

#pragma once

#ifndef __OSDEPEND_H__
#define __OSDEPEND_H__

#include "mamecore.h"
#include "osdcore.h"
#include "timer.h"


/*-----------------------------------------------------------------------------
    osd_init: initialize the OSD system.

    Parameters:

        machine - pointer to a structure that contains parameters for the
            current "machine"

    Return value:

        None

    Notes:

        This function is responsible for initializing the OSD-specific
        video and input functionality, and registering that functionality
        with the MAME core.

        In terms of video, this function is expected to create one or more
        render_targets that will be used by the MAME core to provide graphics
        data to the system. Although it is possible to do this later, the
        assumption in the MAME core is that the user interface will be
        visible starting at osd_init() time, so you will have some work to
        do to avoid these assumptions.

        In terms of input, this function is expected to enumerate all input
        devices available and describe them to the MAME core by adding
        input devices and their attached items (buttons/axes) via the input
        system.

        Beyond these core responsibilities, osd_init() should also initialize
        any other OSD systems that require information about the current
        running_machine.

        This callback is also the last opportunity to adjust the options
        before they are consumed by the rest of the core.

        Note that there is no corresponding osd_exit(). Rather, like most
        systems in MAME, you can register an exit callback via the
        add_exit_callback() function in mame.c.

        Also note that there is no return value. If you need to report a
        fatal error, use the fatalerror() function with a friendly message
        to the user.

    Future work/changes:

        Audio initialization may eventually move into here as well,
        instead of relying on independent callbacks from each system.
-----------------------------------------------------------------------------*/
void osd_init(running_machine *machine);


void osd_wait_for_debugger(void);



/******************************************************************************

    Display

******************************************************************************/

void osd_update(int skip_redraw);




/******************************************************************************

    Sound

******************************************************************************/

void osd_update_audio_stream(INT16 *buffer, int samples_this_frame);

/*
  control master volume. attenuation is the attenuation in dB (a negative
  number). To convert from dB to a linear volume scale do the following:
    volume = MAX_VOLUME;
    while (attenuation++ < 0)
        volume /= 1.122018454;      //  = (10 ^ (1/20)) = 1dB
*/
void osd_set_mastervolume(int attenuation);



/******************************************************************************

    Controls

******************************************************************************/

/*
  inptport.c defines some general purpose defaults for key and joystick bindings.
  They may be further adjusted by the OS dependent code to better match the
  available keyboard, e.g. one could map pause to the Pause key instead of P, or
  snapshot to PrtScr instead of F12. Of course the user can further change the
  settings to anything he/she likes.
  This function is called on startup, before reading the configuration from disk.
  Scan the list, and change the keys/joysticks you want.
*/
void osd_customize_inputport_list(input_port_default_entry *defaults);



#endif	/* __OSDEPEND_H__ */
