/*********************************************************************

    Philips PCF8593 CMOS clock/calendar circuit

    (c) 2001-2007 Tim Schuerewegen

*********************************************************************/

#ifndef __PCF8593_H__
#define __PCF8593_H__

#include "emu.h"


/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(PCF8593, pcf8593);

#define MCFG_PCF8593_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PCF8593, 0) \

#define MCFG_PCF8593_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
/* pins */
void pcf8593_pin_scl(device_t *device, int data);
void pcf8593_pin_sda_w(device_t *device, int data);
int  pcf8593_pin_sda_r(device_t *device);

/* load/save */
void pcf8593_load(device_t *device, emu_file *file);
void pcf8593_save(device_t *device, emu_file *file);

#endif /* __PCF8593_H__ */
