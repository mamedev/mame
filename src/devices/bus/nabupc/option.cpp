// license:BSD-3-Clause
// copyright-holders:Brian Johnson

/*******************************************************************
 *
 * NABU PC Expansion Emulation
 *
 *******************************************************************/
#include "emu.h"
#include "option.h"

#include "fdc.h"
#include "hdd.h"
#include "rs232.h"

DEFINE_DEVICE_TYPE(NABUPC_OPTION_BUS_SLOT, bus::nabupc::option_slot_device, "nabupc_option_slot", "NABU PC Option slot")
DEFINE_DEVICE_TYPE(NABUPC_OPTION_BUS, bus::nabupc::option_bus_device, "nabupc_option_bus", "NABU PC Option Bus")

namespace bus::nabupc {
//**************************************************************************
//  NABU SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  option_bus_slot_device - constructor
//-------------------------------------------------
option_slot_device::option_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NABUPC_OPTION_BUS_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_option_expansion_interface>(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG),
	m_slot(-1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void option_slot_device::device_start()
{
	device_option_expansion_interface *const intf(get_card_device());
	if (intf) {
		intf->set_option_bus(*m_bus, m_slot);
	}
	m_bus->add_slot(*this);
}


uint8_t option_slot_device::io_read(offs_t offset)
{
	device_option_expansion_interface *const intf(get_card_device());
	if (intf) {
		return intf->read(offset);
	}
	return 0xFF;
}

void option_slot_device::io_write(offs_t offset, uint8_t data)
{
	device_option_expansion_interface *const intf(get_card_device());
	if (intf) {
		intf->write(offset, data);
	}
}

void option_slot_device::int_w(int state) {  (*m_bus).set_int_line(state, m_slot); }


//**************************************************************************
//  NABU OPTION BUS DEVICE
//**************************************************************************

//-------------------------------------------------
//  option_bus_device - constructor
//-------------------------------------------------
option_bus_device::option_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NABUPC_OPTION_BUS, tag, owner, clock),
	m_int_cb(*this)
{
}

void option_bus_device::add_slot(option_slot_device &slot)
{
	m_slot_list.push_back(&slot);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void option_bus_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------
void option_bus_device::device_reset()
{
}


//**************************************************************************
//  NABU OPTION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  option_device_expansion_interface - constructor
//-------------------------------------------------
device_option_expansion_interface::device_option_expansion_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "nabupc_option_bus"),
	m_bus(nullptr),
	m_slot(-1)
{
}

//---------------------------------------------------
//  interface_pre_start - device-specific pre startup
//---------------------------------------------------
void device_option_expansion_interface::interface_pre_start()
{
	if (!m_bus || m_slot == -1)
		throw device_missing_dependencies();
}

//-------------------------------------------------
//  SLOT_INTERFACE( option_bus_devices )
//-------------------------------------------------

void option_bus_devices(device_slot_interface &device)
{
	device.option_add("fdc", NABUPC_OPTION_FDC);
	device.option_add("hdd", NABUPC_OPTION_HDD);
	device.option_add("rs232", NABUPC_OPTION_RS232);
}

}  // namespace bus::nabupc
