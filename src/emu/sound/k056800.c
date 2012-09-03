/***********************************************************************

    Konami K056800 (MIRAC)
    Sound interface


***********************************************************************/

#include "emu.h"
#include "sound/k056800.h"

typedef struct _k056800_state k056800_state;
struct _k056800_state
{
	UINT8                host_reg[8];
	UINT8                sound_reg[8];

	emu_timer            *sound_cpu_timer;
	UINT8                sound_cpu_irq1_enable;
	k056800_irq_cb       irq_cb;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k056800_state *k056800_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == K056800);

	return (k056800_state *)downcast<k056800_device *>(device)->token();
}

INLINE const k056800_interface *k056800_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == K056800));
	return (const k056800_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/


static UINT8 k056800_host_reg_r( device_t *device, int reg )
{
	k056800_state *k056800 = k056800_get_safe_token(device);
	UINT8 value = k056800->host_reg[reg];
	if (reg == 2)
		value &= ~3; // suppress VOLWR busy flags

	return value;
}

static void k056800_host_reg_w( device_t *device, int reg, UINT8 data )
{
	k056800_state *k056800 = k056800_get_safe_token(device);

	k056800->sound_reg[reg] = data;

	if (reg == 7)
		k056800->irq_cb(device->machine(), 1);
}

static UINT8 k056800_sound_reg_r( device_t *device, int reg )
{
	k056800_state *k056800 = k056800_get_safe_token(device);
	return k056800->sound_reg[reg];
}

static void k056800_sound_reg_w( device_t *device, int reg, UINT8 data )
{
	k056800_state *k056800 = k056800_get_safe_token(device);

	if (reg == 4)
		k056800->sound_cpu_irq1_enable = data & 0x01;

	k056800->host_reg[reg] = data;
}

static TIMER_CALLBACK( k056800_sound_cpu_timer_tick )
{
	k056800_state *k056800 = (k056800_state *)ptr;

	if (k056800->sound_cpu_irq1_enable)
		k056800->irq_cb(machine, 0);
}

READ32_DEVICE_HANDLER( k056800_host_r )
{
	UINT32 r = 0;

	if (ACCESSING_BITS_24_31)
		r |= k056800_host_reg_r(device, (offset * 4) + 0) << 24;

	if (ACCESSING_BITS_16_23)
		r |= k056800_host_reg_r(device, (offset * 4) + 1) << 16;

	if (ACCESSING_BITS_8_15)
		r |= k056800_host_reg_r(device, (offset * 4) + 2) << 8;

	if (ACCESSING_BITS_0_7)
		r |= k056800_host_reg_r(device, (offset * 4) + 3) << 0;


	return r;
}

WRITE32_DEVICE_HANDLER( k056800_host_w )
{
	if (ACCESSING_BITS_24_31)
		k056800_host_reg_w(device, (offset * 4) + 0, (data >> 24) & 0xff);

	if (ACCESSING_BITS_16_23)
		k056800_host_reg_w(device, (offset * 4) + 1, (data >> 16) & 0xff);

	if (ACCESSING_BITS_8_15)
		k056800_host_reg_w(device, (offset * 4) + 2, (data >> 8) & 0xff);

	if (ACCESSING_BITS_0_7)
		k056800_host_reg_w(device, (offset * 4) + 3, (data >> 0) & 0xff);

}

READ16_DEVICE_HANDLER( k056800_sound_r )
{
	return k056800_sound_reg_r(device, offset);
}

WRITE16_DEVICE_HANDLER( k056800_sound_w )
{
	k056800_sound_reg_w(device, offset, data);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k056800 )
{
	k056800_state *k056800 = k056800_get_safe_token(device);
	const k056800_interface *intf = k056800_get_interface(device);
	attotime timer_period = attotime::from_hz(44100) * 128;	// roughly 2.9us

	k056800->irq_cb = intf->irq_cb;

	k056800->sound_cpu_timer = device->machine().scheduler().timer_alloc(FUNC(k056800_sound_cpu_timer_tick), k056800);
	k056800->sound_cpu_timer->adjust(timer_period, 0, timer_period);

	device->save_item(NAME(k056800->host_reg));
	device->save_item(NAME(k056800->sound_reg));
	device->save_item(NAME(k056800->sound_cpu_irq1_enable));
}

static DEVICE_RESET( k056800 )
{
	k056800_state *k056800 = k056800_get_safe_token(device);

	memset(k056800->host_reg, 0, sizeof(k056800->host_reg));
	memset(k056800->sound_reg, 0, sizeof(k056800->sound_reg));

	k056800->sound_cpu_irq1_enable = 0;
}

const device_type K056800 = &device_creator<k056800_device>;

k056800_device::k056800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K056800, "Konami 056800 MIRAC", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(k056800_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k056800_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k056800_device::device_start()
{
	DEVICE_START_NAME( k056800 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k056800_device::device_reset()
{
	DEVICE_RESET_NAME( k056800 )(this);
}


