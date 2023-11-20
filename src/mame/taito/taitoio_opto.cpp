// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Taito Opto(meter?) coin input device

Detects coin weight tied to a non-JAMMA harness.
Parent Jack pinout has two input pins, labeled "in medal sen(sor?)" and "coin drop".

Used by Taito gambling/medal games:
- taito/cchance.cpp
- taito/pkspirit.cpp
- taito/sbmjb.cpp
- taito/taito_o.cpp

**************************************************************************************************/

#include "emu.h"
#include "taitoio_opto.h"


DEFINE_DEVICE_TYPE(TAITOIO_OPTO, taitoio_opto_device, "taitoio_opto", "Taito I/O Opto coin slot")


taitoio_opto_device::taitoio_opto_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TAITOIO_OPTO, tag, owner, clock)
	, m_opto_timer(nullptr)
	, m_coin_sense(false)
	, m_opto_h(false)
	, m_opto_l(false)
	, m_opto_start(attotime::never)
	, m_opto_end(attotime::never)
{
}

void taitoio_opto_device::device_start()
{
	m_opto_timer = timer_alloc(FUNC(taitoio_opto_device::opto_clear_cb), this);

	save_item(NAME(m_coin_sense));
	save_item(NAME(m_opto_h));
	save_item(NAME(m_opto_l));
	save_item(NAME(m_opto_start));
	save_item(NAME(m_opto_end));
}

// detect 0 -> 1 / 1 -> 0 transitions
void taitoio_opto_device::coin_sense_w(int state)
{
	if (!m_coin_sense && state)
	{
		m_opto_start = machine().time();
		m_coin_sense = true;
		m_opto_h = true;
		m_opto_l = false;
	}
	else if (m_coin_sense && !state)
	{
		// arbitrary, just make it sure whatever gets in has even phases.
		// Sensor must behave so that -- -> H- -> HL -> -L -> --
		m_opto_end = machine().time() - m_opto_start;
		m_opto_timer->adjust(m_opto_end, 1);
		m_coin_sense = false;
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
	{
		m_opto_l = false;
	}
}
