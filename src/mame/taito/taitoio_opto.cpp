// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Taito Opto(meter?) coin input device

Detects coin weight tied to a non-JAMMA harness.
Parent Jack pinout has two input pins, labeled "in medal sen(sor?)" and "coin drop".

Used by Taito gambling/medal games:
- taito/pkspirit.cpp
- taito/sbmjb.cpp
- taito/taito_o.cpp

**************************************************************************************************/

#include "emu.h"
#include "taitoio_opto.h"


DEFINE_DEVICE_TYPE(TAITOIO_OPTO, taitoio_opto_device, "taitoio_opto", "Taito I/O Opto coin slot")


taitoio_opto_device::taitoio_opto_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TAITOIO_OPTO, tag, owner, clock)
	, m_coin_in(*this, "COIN")
{
}

void taitoio_opto_device::device_start()
{
	m_opto_timer = timer_alloc(FUNC(taitoio_opto_device::opto_clear_cb), this);
	save_item(NAME(m_opto_h));
	save_item(NAME(m_opto_l));
}

void taitoio_opto_device::device_reset()
{
	m_opto_timer->adjust(attotime::never);
	m_opto_start = attotime::never;
	m_opto_end = attotime::never;
}

static INPUT_PORTS_START( opto_input )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, taitoio_opto_device, coin_sense_cb, 0)
INPUT_PORTS_END

ioport_constructor taitoio_opto_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( opto_input );
}

// detect 0 -> 1 / 1 -> 0 transitions
INPUT_CHANGED_MEMBER(taitoio_opto_device::coin_sense_cb)
{
	if (!oldval && newval)
	{
		m_opto_start = machine().time();
		m_opto_h = true;
		m_opto_l = false;
	}
	
	if (oldval && !newval)
	{
		// arbitrary, just make it sure whatever gets in has even phases.
		// Sensor must behave so that -- -> H- -> HL -> -L -> --
		m_opto_end = machine().time() - m_opto_start;
		m_opto_timer->adjust(m_opto_end, 1);
		m_opto_l = true;
	}
}

TIMER_CALLBACK_MEMBER(taitoio_opto_device::opto_clear_cb)
{
	if (param)
	{
		m_opto_h = false;
		m_opto_timer->adjust(m_opto_end, 0);
	}
	else
		m_opto_l = false;
}

int taitoio_opto_device::opto_h_r()
{
	return m_opto_h;
}

int taitoio_opto_device::opto_l_r()
{
	return m_opto_l;
}
