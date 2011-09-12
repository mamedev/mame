/***************************************************************************

	HD63484 ACRTC (rewrite in progress)

***************************************************************************/

#include "emu.h"
#include "video/h63484.h"

typedef struct _h63484_state h63484_state;
struct _h63484_state
{
	h63484_display_pixels_func display_func;
	screen_device *screen;	/* screen */

};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE h63484_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == H63484);

	return (h63484_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const h63484_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == H63484);
	return (const h63484_interface *) device->static_config();
}

/*-------------------------------------------------
    ROM( h63484 )
-------------------------------------------------*/

ROM_START( h63484 )
	ROM_REGION( 0x100, "h63484", 0 )
	ROM_LOAD( "h63484.bin", 0x000, 0x100, NO_DUMP ) /* internal control ROM */
ROM_END

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

READ16_DEVICE_HANDLER( h63484_status_r )
{
	//h63484_state *h63484 = get_safe_token(device);

	return 0;
}

READ16_DEVICE_HANDLER( h63484_data_r )
{
	//h63484_state *h63484 = get_safe_token(device);
	int res;

	res = 0xffff;

	return res;
}

WRITE16_DEVICE_HANDLER( h63484_address_w )
{
	//h63484_state *h63484 = get_safe_token(device);
}

WRITE16_DEVICE_HANDLER( h63484_data_w )
{
	//h63484_state *h63484 = get_safe_token(device);
}

static DEVICE_START( h63484 )
{
	h63484_state *h63484 = get_safe_token(device);
	const h63484_interface *intf = get_interface(device);

	h63484->display_func = intf->display_func;

	h63484->screen = device->machine().device<screen_device>(intf->screen_tag);
	assert(h63484->screen != NULL);
}

static DEVICE_RESET( h63484 )
{
	//h63484_state *h63484 = get_safe_token(device);

	//h63484->fifo_counter = 0;
}

DEVICE_GET_INFO( h63484 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(h63484_state);					break;
		case DEVINFO_INT_DATABUS_WIDTH_0:		info->i = 8;									break;
		case DEVINFO_INT_ADDRBUS_WIDTH_0:		info->i = 19;									break;
		case DEVINFO_INT_ADDRBUS_SHIFT_0:		info->i = -1;									break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = ROM_NAME(h63484);				break;

		/* --- the following bits of info are returned as pointers to data --- */
		case DEVINFO_PTR_DEFAULT_MEMORY_MAP_0:	info->default_map8 = NULL; 						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(h63484);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(h63484);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Hitachi 63484");				break;
		case DEVINFO_STR_SHORTNAME:				strcpy(info->s, "h63484");						break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Hitachi 63484 ACRTC");			break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEFINE_LEGACY_MEMORY_DEVICE(H63484, h63484);
