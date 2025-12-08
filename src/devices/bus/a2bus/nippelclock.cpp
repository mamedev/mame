// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    nippelclock.c

    Implementation of the Nippel Clock card for Agat.

    https://archive.org/details/Nippel_Clock_Agat

*********************************************************************/

#include "emu.h"
#include "nippelclock.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_NIPPELCLOCK, a2bus_nippelclock_device, "nclock", "Nippel Clock")

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_nippelclock_device::device_add_mconfig(machine_config &config)
{
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(FUNC(a2bus_nippelclock_device::irq_w));
	m_rtc->set_24hrs(true);
	m_rtc->set_binary(true);
}

void a2bus_nippelclock_device::irq_w(int state)
{
	if (state == ASSERT_LINE)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_nippelclock_device::a2bus_nippelclock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
{
}

a2bus_nippelclock_device::a2bus_nippelclock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_nippelclock_device(mconfig, A2BUS_NIPPELCLOCK, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_nippelclock_device::device_start()
{
}

void a2bus_nippelclock_device::device_reset()
{
}

void a2bus_nippelclock_device::reset_from_bus()
{
	m_rtc->reset();
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_nippelclock_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
	case 7:
		return m_rtc->data_r();
		break;
	}

	return 0;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_nippelclock_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
	case 6:
		m_rtc->address_w(data);
		break;
	case 7:
		m_rtc->data_w(data);
		break;
	}
}
