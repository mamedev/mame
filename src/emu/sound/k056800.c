/***********************************************************************

    Konami K056800 (MIRAC)
    Sound interface


***********************************************************************/

#include "emu.h"
#include "sound/k056800.h"


const device_type K056800 = &device_creator<k056800_device>;

k056800_device::k056800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, K056800, "Konami 056800 MIRAC", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k056800_device::device_config_complete()
{
	// inherit a copy of the static data
	const k056800_interface *intf = reinterpret_cast<const k056800_interface *>(static_config());
	if (intf != NULL)
		*static_cast<k056800_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_irq_cb = NULL;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k056800_device::device_start()
{
	attotime timer_period = attotime::from_hz(clock()) * 14700 * 3;

	m_irq_cb_func = m_irq_cb;

	m_sound_cpu_timer = timer_alloc(TIMER_TICK_SOUND_CPU);
	m_sound_cpu_timer->adjust(timer_period, 0, timer_period);

	save_item(NAME(m_host_reg));
	save_item(NAME(m_sound_reg));
	save_item(NAME(m_sound_cpu_irq1_enable));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k056800_device::device_reset()
{
	memset(m_host_reg, 0, sizeof(m_host_reg));
	memset(m_sound_reg, 0, sizeof(m_sound_reg));

	m_sound_cpu_irq1_enable = 0;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/


UINT8 k056800_device::host_reg_r( int reg )
{
	UINT8 value = m_host_reg[reg];
	if (reg == 2)
		value &= ~3; // suppress VOLWR busy flags

	return value;
}

void k056800_device::host_reg_w( int reg, UINT8 data )
{
	m_sound_reg[reg] = data;

	if (reg == 7)
		m_irq_cb(machine(), 1);
}

UINT8 k056800_device::sound_reg_r( int reg )
{
	return m_sound_reg[reg];
}

void k056800_device::sound_reg_w( int reg, UINT8 data )
{
	if (reg == 4)
		m_sound_cpu_irq1_enable = data & 0x01;

	m_host_reg[reg] = data;
}

void k056800_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
 	{
		case TIMER_TICK_SOUND_CPU:
			if (m_sound_cpu_irq1_enable)
				m_irq_cb_func(machine(), 0);
			break;
			
			default:
			assert_always(FALSE, "Unknown id in k056800_device::device_timer");
	}
}

READ32_MEMBER( k056800_device::host_r )
{
	UINT32 r = 0;

	if (ACCESSING_BITS_24_31)
		r |= host_reg_r((offset * 4) + 0) << 24;

	if (ACCESSING_BITS_16_23)
		r |= host_reg_r((offset * 4) + 1) << 16;

	if (ACCESSING_BITS_8_15)
		r |= host_reg_r((offset * 4) + 2) << 8;

	if (ACCESSING_BITS_0_7)
		r |= host_reg_r((offset * 4) + 3) << 0;


	return r;
}

WRITE32_MEMBER( k056800_device::host_w )
{
	if (ACCESSING_BITS_24_31)
		host_reg_w((offset * 4) + 0, (data >> 24) & 0xff);

	if (ACCESSING_BITS_16_23)
		host_reg_w((offset * 4) + 1, (data >> 16) & 0xff);

	if (ACCESSING_BITS_8_15)
		host_reg_w((offset * 4) + 2, (data >> 8) & 0xff);

	if (ACCESSING_BITS_0_7)
		host_reg_w((offset * 4) + 3, (data >> 0) & 0xff);

}

READ16_MEMBER( k056800_device::sound_r )
{
	return sound_reg_r(offset);
}

WRITE16_MEMBER( k056800_device::sound_w )
{
	sound_reg_w(offset, data);
}
