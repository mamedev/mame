/*********************************************************************

    ap2_lang.c

    Implementation of the Apple II Language Card

*********************************************************************/

#include "emu.h"
#include "includes/apple2.h"
#include "ap2_lang.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_LANGCARD	0



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    apple2_langcard_touch - device read callback
-------------------------------------------------*/

static void apple2_langcard_touch(device_t *device, offs_t offset)
{
	UINT32 val, mask;

	if (LOG_LANGCARD)
		logerror("language card bankswitch read, offset: $c08%0x\n", offset);

	/* determine which flags to change */
	mask = VAR_LCWRITE | VAR_LCRAM | VAR_LCRAM2;
	val = 0;

	if (offset & 0x01)
		val |= VAR_LCWRITE;

	switch(offset & 0x03)
	{
		case 0x03:
		case 0x00:
			val |= VAR_LCRAM;
			break;
	}

	if ((offset & 0x08) == 0)
		val |= VAR_LCRAM2;

	/* change the flags */
	apple2_setvar(device->machine(), val, mask);
}



/*-------------------------------------------------
    apple2_langcard_r - device read callback
-------------------------------------------------*/

READ8_DEVICE_HANDLER(apple2_langcard_r)
{
	apple2_langcard_touch(device, offset);
	return 0x00;
}



/*-------------------------------------------------
    apple2_langcard_w - device read callback
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER(apple2_langcard_w)
{
	apple2_langcard_touch(device, offset);
}



/*-------------------------------------------------
    DEVICE_START(apple2_langcard) - device start
    function
-------------------------------------------------*/

static DEVICE_START(apple2_langcard)
{
	/* nothing to do */
}



/*-------------------------------------------------
    DEVICE_GET_INFO(apple2_langcard) - device get info
    function
-------------------------------------------------*/

DEVICE_GET_INFO(apple2_langcard)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = 1;								break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(apple2_langcard);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Apple II Language Card");			break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Apple II Language Card");			break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
	}
}

DEFINE_LEGACY_DEVICE(APPLE2_LANGCARD, apple2_langcard);
