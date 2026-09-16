/*
 * pmlinuxnull.c -- system specific definitions
 * 
 * written by:
 *  Roger Dannenberg
 * 
 * If there is no ALSA, you can define PMNULL and build PortMidi. It will
 * not report any devices, so you will not be able to open any, but if
 * you wanted to disable MIDI from some application, this could be used.
 *    Mainly, this code shows the possibility of supporting multiple
 * interfaces, e.g., ALSA and Sndio on BSD, or ALSA and Jack on Linux.
 * But as of Dec, 2021, the only supported MIDI API for Linux is ALSA.
 */

#ifdef PMNULL

#include "portmidi.h"
#include "pmlinuxnull.h"


PmError pm_linuxnull_init(void)
{
    return pmNoError;
}


void pm_linuxnull_term(void)
{
}

#endif
