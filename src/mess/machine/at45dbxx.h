/*

    Atmel Serial DataFlash

    (c) 2001-2007 Tim Schuerewegen

    AT45DB041 -  528 KByte
    AT45DB081 - 1056 KByte
    AT45DB161 - 2112 KByte

*/

#ifndef _AT45DBXX_H_
#define _AT45DBXX_H_

#include "emu.h"


/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(AT45DB041, at45db041);

#define MCFG_AT45DB041_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB041, 0) \

DECLARE_LEGACY_DEVICE(AT45DB081, at45db081);

#define MCFG_AT45DB081_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB081, 0) \

DECLARE_LEGACY_DEVICE(AT45DB161, at45db161);

#define MCFG_AT45DB161_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB161, 0) \


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
// pins
void at45dbxx_pin_cs(device_t *device, int data);
void at45dbxx_pin_sck(device_t *device,  int data);
void at45dbxx_pin_si(device_t *device,  int data);
int  at45dbxx_pin_so(device_t *device);

// load/save
void at45dbxx_load(device_t *device, emu_file *file);
void at45dbxx_save(device_t *device, emu_file *file);

// non-volatile ram handler
//NVRAM_HANDLER( at45dbxx );

#endif
