// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    uthernet.cpp

    Apple II Uthernet Card

*********************************************************************/

#include "emu.h"
#include "uthernet.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_UTHERNET, a2bus_uthernet_device, "a2uthernet", "a2RetroSystems Uthernet")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_uthernet_device::a2bus_uthernet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_uthernet_device(mconfig, A2BUS_UTHERNET, tag, owner, clock)
{
}

a2bus_uthernet_device::a2bus_uthernet_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_netinf(*this, "cs8900a")
{
}

void a2bus_uthernet_device::device_add_mconfig(machine_config &config)
{
	CS8900A(config, m_netinf, 20_MHz_XTAL); // see CS8900A data sheet sec 3.13
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_uthernet_device::device_start()
{
}

void a2bus_uthernet_device::device_reset()
{
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_uthernet_device::read_c0nx(uint8_t offset)
{
	return m_netinf->read(offset);
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_uthernet_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_netinf->write(offset,data);
}

