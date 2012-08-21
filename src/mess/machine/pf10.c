/***************************************************************************

    Epson PF-10

    Serial floppy drive

    Skeleton driver, not working

***************************************************************************/

#include "emu.h"
#include "pf10.h"
#include "cpu/m6800/m6800.h"
#include "machine/upd765.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _pf10_state pf10_state;
struct _pf10_state
{
	UINT8 dummy;
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE pf10_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == PF10);

	return (pf10_state *)downcast<legacy_device_base *>(device)->token();
}


/*****************************************************************************
    ADDRESS MAPS
*****************************************************************************/

static ADDRESS_MAP_START( pf10_mem, AS_PROGRAM, 8, pf10_device )
	AM_RANGE(0x0040, 0x013f) AM_RAM /* internal ram */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* external 2k ram */
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("pf10", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pf10_io, AS_IO, 8, pf10_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/*****************************************************************************
    MACHINE CONFIG
*****************************************************************************/

static const upd765_interface pf10_upd765a_intf =
{
	DEVCB_NULL, /* interrupt line */
	DEVCB_NULL,
	NULL,
	UPD765_RDY_PIN_NOT_CONNECTED, /* ??? */
	{NULL, NULL, NULL, NULL}
};

static MACHINE_CONFIG_FRAGMENT( pf10 )
	MCFG_CPU_ADD("pf10", M6803, XTAL_2_4576MHz / 4 /* ??? */) /* HD63A03 */
	MCFG_CPU_PROGRAM_MAP(pf10_mem)
	MCFG_CPU_IO_MAP(pf10_io)

	MCFG_UPD765A_ADD("upd765a", pf10_upd765a_intf)
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( pf10 )
	ROM_REGION(0x2000, "pf10", 0)
	ROM_LOAD("k3pf1.bin", 0x0000, 0x2000, CRC(eef4593a) SHA1(bb176e4baf938fe58c2d32f7c46d7bb7b0627755))
ROM_END


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( pf10 )
{
	pf10_state *pf10 = get_safe_token(device);

	pf10->dummy = 0;
}

static DEVICE_RESET( pf10 )
{
}

DEVICE_GET_INFO( pf10 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(pf10_state);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = 0;									break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = MACHINE_CONFIG_NAME(pf10);	break;
		case DEVINFO_PTR_ROM_REGION:			info->romregion = ROM_NAME(pf10);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(pf10);			break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(pf10);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "PF-10");						break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Floppy drive");				break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MESS Team");			break;
	}
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* serial interface in (to the host computer) */
READ_LINE_DEVICE_HANDLER( pf10_txd1_r )
{
	logerror("%s: pf10_txd1_r\n", device->machine().describe_context());

	return 0;
}

WRITE_LINE_DEVICE_HANDLER( pf10_rxd1_w )
{
	logerror("%s: pf10_rxd1_w %u\n", device->machine().describe_context(), state);
}


/* serial interface out (to another floppy drive) */
READ_LINE_DEVICE_HANDLER( pf10_txd2_r )
{
	logerror("%s: pf10_txd2_r\n", device->machine().describe_context());

	return 0;
}

WRITE_LINE_DEVICE_HANDLER( pf10_rxd2_w )
{
	logerror("%s: pf10_rxd2_w %u\n", device->machine().describe_context(), state);
}

DEFINE_LEGACY_DEVICE(PF10, pf10);
