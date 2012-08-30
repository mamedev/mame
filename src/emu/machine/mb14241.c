/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "emu.h"
#include "machine/mb14241.h"

typedef struct _mb14241_state  mb14241_state;
struct _mb14241_state
{
	UINT16 shift_data;	/* 15 bits only */
	UINT8 shift_count;	/* 3 bits */
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE mb14241_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == MB14241);

	return (mb14241_state *)downcast<legacy_device_base *>(device)->token();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

WRITE8_DEVICE_HANDLER( mb14241_shift_count_w )
{
	mb14241_state *mb14241 = get_safe_token(device);
	mb14241->shift_count = ~data & 0x07;
}

WRITE8_DEVICE_HANDLER( mb14241_shift_data_w )
{
	mb14241_state *mb14241 = get_safe_token(device);
	mb14241->shift_data = (mb14241->shift_data >> 8) | ((UINT16)data << 7);
}

READ8_DEVICE_HANDLER( mb14241_shift_result_r )
{
	mb14241_state *mb14241 = get_safe_token(device);
	return mb14241->shift_data >> mb14241->shift_count;
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( mb14241 )
{
	mb14241_state *mb14241 = get_safe_token(device);

	device->save_item(NAME(mb14241->shift_data));
	device->save_item(NAME(mb14241->shift_count));
}

static DEVICE_RESET( mb14241 )
{
	mb14241_state *mb14241 = get_safe_token(device);

	mb14241->shift_data = 0;
	mb14241->shift_count = 0;
}

DEVICE_GET_INFO(mb14241)
{
 switch (state)
 {
  case DEVINFO_INT_TOKEN_BYTES: info->i = sizeof(mb14241_state); break;

  case DEVINFO_FCT_START: info->start = DEVICE_START_NAME(mb14241); break;

  case DEVINFO_FCT_RESET: info->reset = DEVICE_RESET_NAME(mb14241); break;

  case DEVINFO_STR_NAME: strcpy(info->s, "MB14241"); break;
 }
}

DEFINE_LEGACY_DEVICE(MB14241, mb14241);
