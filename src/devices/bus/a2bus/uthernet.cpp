// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    uthernet.cpp

    Apple II Uthernet Card

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "uthernet.h"
#include "tfe/defc.h"
#include "tfe/tfe/tfesupp.h" 
#include "tfe/tfe/protos_tfe.h" 

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_UTHERNET, a2bus_uthernet_device, "uthernet", "Uthernet")

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
		m_started(false),
		m_interface(mconfig.options().value(OPTION_UTHERNET_INTF))
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_uthernet_device::device_start()
{
	if (!m_started) {
	        set_tfe_interface(m_interface.c_str()); //Connect the emulated ethernet device with the selected host adapter
		tfe_init();
		m_started = true;
	}
}

void a2bus_uthernet_device::device_reset()
{
	if (m_started) {
		tfe_reset();
	} else {
		device_start();
	}
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_uthernet_device::read_c0nx(uint8_t offset)
{
	return tfe_read((word16)offset);
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_uthernet_device::write_c0nx(uint8_t offset, uint8_t data)
{
	tfe_store((word16)offset, data);
}

