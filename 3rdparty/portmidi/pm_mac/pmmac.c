/* pmmac.c -- PortMidi os-dependent code */

/* This file only needs to implement:
pm_init(), which calls various routines to register the 
available midi devices,
Pm_GetDefaultInputDeviceID(), and
Pm_GetDefaultOutputDeviceID().
It is seperate from pmmacosxcm because we might want to register
non-CoreMIDI devices.
*/

#include "stdlib.h"
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"
#include "pmmacosxcm.h"

void pm_init(void)
{
    pm_macosxcm_init();
}


void pm_term(void)
{
    pm_macosxcm_term();
}

PmDeviceID Pm_GetDefaultInputDeviceID(void) 
{
    Pm_Initialize();
    return pm_default_input_device_id;
}

PmDeviceID Pm_GetDefaultOutputDeviceID(void) {
    Pm_Initialize();
    return pm_default_output_device_id;
}

void *pm_alloc(size_t s) { return malloc(s); }

void pm_free(void *ptr) { free(ptr); }


