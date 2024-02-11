// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

        Iskra Delta Partner Bus

**********************************************************************/

#include "emu.h"
#include "bus.h"

DEFINE_DEVICE_TYPE(IDPARTNER_BUS, bus::idpartner::bus_device, "idpartner_bus_device", "Iskra Delta Partner Bus")
DEFINE_DEVICE_TYPE(IDPARTNER_BUS_CONNECTOR, bus::idpartner::bus_connector_device, "idpartner_bus_connector", "Iskra Delta Partner Bus Connector")

namespace bus::idpartner {

/***********************************************************************
    CARD INTERFACE
***********************************************************************/

device_exp_card_interface::device_exp_card_interface(const machine_config &mconfig, device_t &device)
    : device_interface(device, "idpartner_card")
    , m_bus(nullptr)
{
}

/***********************************************************************
    SLOT DEVICE
***********************************************************************/

bus_connector_device::bus_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : device_t(mconfig, IDPARTNER_BUS_CONNECTOR, tag, owner, clock)
    , device_single_card_slot_interface<device_exp_card_interface>(mconfig, *this)
    , m_bus(*this, finder_base::DUMMY_TAG)
{
}

/*----------------------------------
  device_t implementation
----------------------------------*/

void bus_connector_device::device_resolve_objects()
{
    device_exp_card_interface *const exp_card = get_card_device();
    if (exp_card)
        exp_card->set_bus(m_bus);
}

void bus_connector_device::device_start()
{
}

/***********************************************************************
    BUS DEVICE
***********************************************************************/

bus_device::bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : device_t(mconfig, IDPARTNER_BUS, tag, owner, clock)
    , m_io(*this, finder_base::DUMMY_TAG, -1)
    , m_int_handler(*this)
    , m_nmi_handler(*this)
{
}

/*----------------------------------
  device_t implementation
----------------------------------*/

void bus_device::device_start()
{
}

void bus_device::device_reset()
{
}

} // namespace bus::idpartner

#include "gdp.h"

void idpartner_exp_devices(device_slot_interface &device)
{
    device.option_add("gdp", IDPARTNER_GDP);
}
