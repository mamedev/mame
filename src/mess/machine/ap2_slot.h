/*********************************************************************

    ap2_slot.h

    Implementation of Apple II device slots

*********************************************************************/

#ifndef __AP2_SLOT__
#define __AP2_SLOT__

#include "emu.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(APPLE2_SLOT, apple2_slot);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _apple2_slot_config apple2_slot_config;
struct _apple2_slot_config
{
	const char *tag;
	int slotnum;			// slot number

	read8_device_func rh;		// C0nX I/O space (DEVSEL)
	write8_device_func wh;

	read8_device_func rhcnxx;	// CnXX ROM space (IOSEL)
	write8_device_func whcnxx;

	read8_device_func rhc800;	// C800 ROM extension space (IOSTB)
	write8_device_func whc800;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_APPLE2_SLOT_ADD(_slot_number, _slot_device_tag, _rh, _wh, _rhc800, _whc800, _rhcnxx, _whcnxx)		\
	MCFG_DEVICE_ADD("slot_" #_slot_number, APPLE2_SLOT, 0)			\
	MCFG_DEVICE_CONFIG_DATAPTR(apple2_slot_config, tag, _slot_device_tag)	\
	MCFG_DEVICE_CONFIG_DATA32(apple2_slot_config, slotnum, _slot_number)		\
	MCFG_DEVICE_CONFIG_DATAPTR(apple2_slot_config, rh, _rh)		\
	MCFG_DEVICE_CONFIG_DATAPTR(apple2_slot_config, wh, _wh)		\
	MCFG_DEVICE_CONFIG_DATAPTR(apple2_slot_config, rhc800, _rhc800)		\
	MCFG_DEVICE_CONFIG_DATAPTR(apple2_slot_config, whc800, _whc800)		\
	MCFG_DEVICE_CONFIG_DATAPTR(apple2_slot_config, rhcnxx, _rhcnxx)		\
	MCFG_DEVICE_CONFIG_DATAPTR(apple2_slot_config, whcnxx, _whcnxx)		\



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
/* slot read function */
READ8_DEVICE_HANDLER(apple2_slot_r);

/* slot write function */
WRITE8_DEVICE_HANDLER(apple2_slot_w);

/* slot ROM read function */
READ8_DEVICE_HANDLER(apple2_slot_ROM_r);

/* slot ROM write function */
WRITE8_DEVICE_HANDLER(apple2_slot_ROM_w);

/* slot extended ROM read function */
READ8_DEVICE_HANDLER(apple2_c800_slot_r);

/* slot extended ROM write function */
WRITE8_DEVICE_HANDLER(apple2_c800_slot_w);

/* slot device lookup */
device_t *apple2_slot(running_machine &machine, int slotnum);

#endif /* __AP2_SLOT__ */
