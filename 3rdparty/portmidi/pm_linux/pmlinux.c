/* pmlinux.c -- PortMidi os-dependent code */

/* This file only needs to implement pm_init(), which calls various
   routines to register the available midi devices. This file must
   be separate from the main portmidi.c file because it is system
   dependent, and it is separate from, pmlinuxalsa.c, because it
   might need to register non-alsa devices as well.

   NOTE: if you add non-ALSA support, you need to fix :alsa_poll()
   in pmlinuxalsa.c, which assumes all input devices are ALSA.
 */

#include "stdlib.h"
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"

#ifdef PMALSA
  #include "pmlinuxalsa.h"
#endif

#ifdef PMNULL
  #include "pmlinuxnull.h"
#endif

#if !(defined(PMALSA) || defined(PMNULL))
#error One of PMALSA or PMNULL must be defined
#endif

void pm_init(void)
{
    /* Note: it is not an error for PMALSA to fail to initialize. 
     * It may be a design error that the client cannot query what subsystems
     * are working properly other than by looking at the list of available
     * devices.
     */
#ifdef PMALSA
    pm_linuxalsa_init();
#endif
#ifdef PMNULL
        pm_linuxnull_init();
#endif
}

void pm_term(void)
{
    #ifdef PMALSA
        pm_linuxalsa_term();
    #endif
    #ifdef PMNULL
        pm_linuxnull_term();
    #endif
}

PmDeviceID Pm_GetDefaultInputDeviceID(void) { 
    Pm_Initialize();
    return pm_default_input_device_id; 
}

PmDeviceID Pm_GetDefaultOutputDeviceID(void) { 
    Pm_Initialize();
    return pm_default_output_device_id; 
}

void *pm_alloc(size_t s) { return malloc(s); }

void pm_free(void *ptr) { free(ptr); }

