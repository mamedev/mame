/***************************************************************************

    Atari vector hardware

***************************************************************************/

#include "emu.h"
#include "atari_vg.h"

#define EAROM_SIZE	0x40


typedef struct _atari_vg_earom_state atari_vg_earom_state;
struct _atari_vg_earom_state
{
	running_device *device;

	int offset;
	int data;
	char rom[EAROM_SIZE];

};


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - convert a device's token
    into a atari_vg_earom_state
-------------------------------------------------*/

INLINE atari_vg_earom_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == ATARIVGEAROM);
	return (atari_vg_earom_state *)device->token;
}




READ8_DEVICE_HANDLER( atari_vg_earom_r )
{
	atari_vg_earom_state *earom = get_safe_token(device);

	logerror("read earom: %02x(%02x):%02x\n", earom->offset, offset, earom->data);
	return (earom->data);
}


WRITE8_DEVICE_HANDLER( atari_vg_earom_w )
{
	atari_vg_earom_state *earom = get_safe_token(device);

	logerror("write earom: %02x:%02x\n", offset, data);
	earom->offset = offset;
	earom->data = data;
}


/* 0,8 and 14 get written to this location, too.
 * Don't know what they do exactly
 */
WRITE8_DEVICE_HANDLER( atari_vg_earom_ctrl_w )
{
	atari_vg_earom_state *earom = get_safe_token(device);

	logerror("earom ctrl: %02x:%02x\n",offset, data);
	/*
        0x01 = clock
        0x02 = set data latch? - writes only (not always)
        0x04 = write mode? - writes only
        0x08 = set addr latch?
    */
	if (data & 0x01)
		earom->data = earom->rom[earom->offset];
	if ((data & 0x0c) == 0x0c)
	{
		earom->rom[earom->offset]=earom->data;
		logerror("    written %02x:%02x\n", earom->offset, earom->data);
	}
}


static DEVICE_NVRAM( atari_vg_earom )
{
	atari_vg_earom_state *earom = get_safe_token(device);

	if (read_or_write)
		mame_fwrite(file,earom->rom,EAROM_SIZE);
	else if (file)
		mame_fread(file,earom->rom,EAROM_SIZE);
	else
		memset(earom,0,EAROM_SIZE);
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static DEVICE_START( atari_vg_earom )
{
	atari_vg_earom_state *earom = get_safe_token(device);

	/* validate arguments */
	assert(device != NULL);

	/* set static values */
	earom->device = device;

	/* register for save states */
	state_save_register_device_item(device, 0, earom->offset);
	state_save_register_device_item(device, 0, earom->data);
}


static DEVICE_RESET( atari_vg_earom )
{
	atari_vg_earom_state *earom = get_safe_token(device);

	earom->data = 0;
	earom->offset = 0;
}


DEVICE_GET_INFO( atari_vg_earom )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(atari_vg_earom_state);	break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(atari_vg_earom);break;
		case DEVINFO_FCT_NVRAM:							info->nvram = DEVICE_NVRAM_NAME(atari_vg_earom); break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(atari_vg_earom);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "atari_vg_earom");		break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "EEPROM");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
