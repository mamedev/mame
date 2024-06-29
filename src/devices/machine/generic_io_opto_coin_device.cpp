// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

 Generic Dual Opto Coin In input device
 
 Originally developed by Angelo Salese for TAITO_O driver.
 
 Rewritten as generic device by Roberto Fresca & Grull Osgo (Just renamed class, some objects and tags).
 
 All credits to Angelo Salese.

**************************************************************************************************/

#include "emu.h"
#include "generic_io_opto_coin_device.h"


DEFINE_DEVICE_TYPE(GENERIC_IO_OPTO_COIN_DEVICE, generic_io_opto_coin_device, "generic_io_opto_coin_device", "Generic I/O Opto coin device")



generic_io_opto_coin_device::generic_io_opto_coin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GENERIC_IO_OPTO_COIN_DEVICE, tag, owner, clock)
	, m_opto_timer(nullptr)
	, m_coin_sense(false)
	, m_opto_h(false)
	, m_opto_l(false)
	, m_opto_start(attotime::never)
	, m_opto_end(attotime::never)
{
}

void generic_io_opto_coin_device::device_start()
{
	m_opto_timer = timer_alloc(FUNC(generic_io_opto_coin_device::opto_clear_cb), this);

	save_item(NAME(m_coin_sense));
	save_item(NAME(m_opto_h));
	save_item(NAME(m_opto_l));
	save_item(NAME(m_opto_start));
	save_item(NAME(m_opto_end));
}

// detect 0 -> 1 / 1 -> 0 transitions
void generic_io_opto_coin_device::coin_sense_w(int state)
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

TIMER_CALLBACK_MEMBER(generic_io_opto_coin_device::opto_clear_cb)
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
