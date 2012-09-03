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

	return (mb14241_state *)downcast<mb14241_device *>(device)->token();
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

const device_type MB14241 = &device_creator<mb14241_device>;

mb14241_device::mb14241_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MB14241, "MB14241", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(mb14241_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mb14241_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb14241_device::device_start()
{
	DEVICE_START_NAME( mb14241 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb14241_device::device_reset()
{
	DEVICE_RESET_NAME( mb14241 )(this);
}


