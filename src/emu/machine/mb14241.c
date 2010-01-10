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

INLINE mb14241_state *get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == MB14241);

	return (mb14241_state *)device->token;
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

	state_save_register_device_item(device, 0, mb14241->shift_data);
	state_save_register_device_item(device, 0, mb14241->shift_count);
}

static DEVICE_RESET( mb14241 )
{
	mb14241_state *mb14241 = get_safe_token(device);

	mb14241->shift_data = 0;
	mb14241->shift_count = 0;
}

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID( p, s )	p##mb14241##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"MB14241"
#define DEVTEMPLATE_FAMILY		"MB14241 Shifter IC"
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_VIDEO
#include "devtempl.h"
