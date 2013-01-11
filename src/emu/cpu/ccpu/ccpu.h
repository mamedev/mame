/***************************************************************************

    ccpu.h
    Core implementation for the portable Cinematronics CPU emulator.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#pragma once

#ifndef __CCPU_H__
#define __CCPU_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	CCPU_PC=1,
	CCPU_FLAGS,
	CCPU_A,
	CCPU_B,
	CCPU_I,
	CCPU_J,
	CCPU_P,
	CCPU_X,
	CCPU_Y,
	CCPU_T
};



/***************************************************************************
    CONFIG STRUCTURE
***************************************************************************/

typedef UINT8 (*ccpu_input_func)(device_t *device);
typedef void (*ccpu_vector_func)(device_t *device, INT16 sx, INT16 sy, INT16 ex, INT16 ey, UINT8 shift);

struct ccpu_config
{
	ccpu_input_func     external_input;     /* if NULL, assume JMI jumper is present */
	ccpu_vector_func    vector_callback;
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(CCPU, ccpu);

void ccpu_wdt_timer_trigger(device_t *device);

CPU_DISASSEMBLE( ccpu );

#endif /* __CCPU_H__ */
