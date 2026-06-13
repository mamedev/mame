/* pmwin.c -- PortMidi os-dependent code */

/* This file only needs to implement:
       pm_init(), which calls various routines to register the 
           available midi devices,
       Pm_GetDefaultInputDeviceID(), and
       Pm_GetDefaultOutputDeviceID().
   This file must
   be separate from the main portmidi.c file because it is system
   dependent, and it is separate from, say, pmwinmm.c, because it
   might need to register devices for winmm, directx, and others.

 */

#include "stdlib.h"
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"
#include "pmwinmm.h"
#ifdef DEBUG
#include "stdio.h"
#endif
#include <windows.h>

/* pm_exit is called when the program exits.
   It calls pm_term to make sure PortMidi is properly closed.
   If DEBUG is on, we prompt for input to avoid losing error messages.
 */
static void pm_exit(void) {
    pm_term();
}


static BOOL WINAPI ctrl_c_handler(DWORD fdwCtrlType)
{
    exit(1);  /* invokes pm_exit() */
    ExitProcess(1);  /* probably never called */
    return TRUE;
}

/* pm_init is the windows-dependent initialization.*/
void pm_init(void)
{
    atexit(pm_exit);
    SetConsoleCtrlHandler(ctrl_c_handler, TRUE);
#ifdef DEBUG
    printf("registered pm_exit with atexit()\n");
#endif
    pm_winmm_init();
    /* initialize other APIs (DirectX?) here */
}


void pm_term(void) {
    pm_winmm_term();
}


static PmDeviceID pm_get_default_device_id(int is_input, char *key) {
#define PATTERN_MAX 256
    /* Find first input or device -- this is the default. */
    PmDeviceID id = pmNoDevice;
    int i;
    Pm_Initialize(); /* make sure descriptors exist! */
    for (i = 0; i < pm_descriptor_len; i++) {
        if (pm_descriptors[i].pub.input == is_input) {
            id = i;
            break;
        }
    }
    return id;
}


PmDeviceID Pm_GetDefaultInputDeviceID(void) {
    return pm_get_default_device_id(TRUE, 
           "/P/M_/R/E/C/O/M/M/E/N/D/E/D_/I/N/P/U/T_/D/E/V/I/C/E");
}


PmDeviceID Pm_GetDefaultOutputDeviceID(void) {
  return pm_get_default_device_id(FALSE,
          "/P/M_/R/E/C/O/M/M/E/N/D/E/D_/O/U/T/P/U/T_/D/E/V/I/C/E");
}


#include "stdio.h" 

void *pm_alloc(size_t s) {
    return malloc(s); 
}


void pm_free(void *ptr) { 
    free(ptr); 
}


