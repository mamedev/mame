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

INLINE k056800_state *k056800_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K056800);

	return (k056800_state *)device->token;
}

INLINE const k056800_interface *k056800_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K056800));
	return (const k056800_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/


static UINT8 k056800_host_reg_r( const device_config *device, int reg )
{
	k056800_state *k056800 = k056800_get_safe_token(device);
	UINT8 value = k056800->host_reg[reg];
	if (reg == 2)
		value &= ~3; // suppress VOLWR busy flags

	return value;
}

static void k056800_host_reg_w( const device_config *device, int reg, UINT8 data )
{
	k056800_state *k056800 = k056800_get_safe_token(device);

	k056800->sound_reg[reg] = data;

	if (reg == 7)
		k056800->irq_cb(device->machine, 1);
}

static UINT8 k056800_sound_reg_r( const device_config *device, int reg )
{
	k056800_state *k056800 = k056800_get_safe_token(device);
	return k056800->sound_reg[reg];
}

static void k056800_sound_reg_w( const device_config *device, int reg, UINT8 data )
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
	attotime timer_period = attotime_mul(ATTOTIME_IN_HZ(44100), 128);	// roughly 2.9us

	k056800->irq_cb = intf->irq_cb;

	k056800->sound_cpu_timer = timer_alloc(device->machine, k056800_sound_cpu_timer_tick, k056800);
	timer_adjust_periodic(k056800->sound_cpu_timer, timer_period, 0, timer_period);

	state_save_register_device_item_array(device, 0, k056800->host_reg);
	state_save_register_device_item_array(device, 0, k056800->sound_reg);
	state_save_register_device_item(device, 0, k056800->sound_cpu_irq1_enable);
}

static DEVICE_RESET( k056800 )
{
	k056800_state *k056800 = k056800_get_safe_token(device);

	memset(k056800->host_reg, 0, sizeof(k056800->host_reg));
	memset(k056800->sound_reg, 0, sizeof(k056800->sound_reg));

	k056800->sound_cpu_irq1_enable = 0;
}


static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID( p, s )	p##k056800##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"Konami 056800 MIRAC"
#define DEVTEMPLATE_FAMILY		"Konami custom"
#include "devtempl.h"
