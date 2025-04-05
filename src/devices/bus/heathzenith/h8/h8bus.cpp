// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  h8bus.cpp - Heath/Zenith H-8 bus

  Also known as the "Benton Harbor Bus" BHBus based on city where the
  corporate headquarters were located.

***************************************************************************/

#include "emu.h"
#include "h8bus.h"

device_h8bus_card_interface::device_h8bus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "h8bus"),
	m_h8bus(nullptr)
{
}

device_h8bus_card_interface::~device_h8bus_card_interface()
{
}

void device_h8bus_card_interface::interface_pre_start()
{
	if (!m_h8bus)
	{
		fatalerror("Can't find H-8 bus device\n");
	}
}


DEFINE_DEVICE_TYPE(H8BUS_SLOT, h8bus_slot_device, "h8bus_slot", "Heath H-8 slot")

h8bus_slot_device::h8bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8bus_slot_device(mconfig, H8BUS_SLOT, tag, owner, clock)
{
}

h8bus_slot_device::h8bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_h8bus(*this, finder_base::DUMMY_TAG),
	m_h8bus_slottag(nullptr)
{
}

void h8bus_slot_device::device_start()
{
}

void h8bus_slot_device::device_resolve_objects()
{
	device_h8bus_card_interface *dev = get_card_device();

	if (dev)
	{
		dev->set_h8bus_tag(m_h8bus.target(), m_h8bus_slottag);
		m_h8bus->add_h8bus_card(*dev);
	}
}


DEFINE_DEVICE_TYPE(H8BUS, h8bus_device, "h8bus", "Heathkit H8 bus (Benton Harbor Bus)")

h8bus_device::h8bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8bus_device(mconfig, H8BUS, tag, owner, clock)
{
}

h8bus_device::h8bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_program_space(*this, finder_base::DUMMY_TAG, -1),
	m_io_space(*this, finder_base::DUMMY_TAG, -1),
	m_out_int0_cb(*this),
	m_out_int1_cb(*this),
	m_out_int2_cb(*this),
	m_out_int3_cb(*this),
	m_out_int4_cb(*this),
	m_out_int5_cb(*this),
	m_out_int6_cb(*this),
	m_out_int7_cb(*this),
	m_out_reset_cb(*this),
	m_out_hold_cb(*this),
	m_out_rom_disable_cb(*this)
{
}

h8bus_device::~h8bus_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void h8bus_device::device_start()
{
}

void h8bus_device::device_reset()
{
}

void h8bus_device::install_mem_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler)
{
	m_program_space->install_readwrite_handler(start, end, rhandler, whandler);
}

void h8bus_device::install_io_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler)
{
	m_io_space->install_readwrite_handler(start, end, rhandler, whandler);
}

void h8bus_device::install_io_device(offs_t start, offs_t end, read8smo_delegate rhandler, write8smo_delegate whandler)
{
	m_io_space->install_readwrite_handler(start, end, rhandler, whandler);
}

void h8bus_device::install_io_device_w(offs_t start, offs_t end, write8smo_delegate whandler)
{
	m_io_space->install_write_handler(start, end, whandler);
}

void h8bus_device::add_h8bus_card(device_h8bus_card_interface &card)
{
	m_device_list.emplace_back(card);
}

void h8bus_device::set_int0_line(int state)
{
	m_out_int0_cb(state);
}

void h8bus_device::set_int1_line(int state)
{
	m_out_int1_cb(state);
}

void h8bus_device::set_int2_line(int state)
{
	m_out_int2_cb(state);
}

void h8bus_device::set_int3_line(int state)
{
	m_out_int3_cb(state);
}

void h8bus_device::set_int4_line(int state)
{
	m_out_int4_cb(state);
}

void h8bus_device::set_int5_line(int state)
{
	m_out_int5_cb(state);
}

void h8bus_device::set_int6_line(int state)
{
	m_out_int6_cb(state);
}

void h8bus_device::set_int7_line(int state)
{
	m_out_int7_cb(state);
}

void h8bus_device::set_reset_line(int state)
{
	m_out_reset_cb(state);
}

void h8bus_device::set_hold_line(int state)
{
	m_out_hold_cb(state);
}

void h8bus_device::set_disable_rom_line(int state)
{
	m_out_rom_disable_cb(state);
}

void h8bus_device::set_hold_ack(u8 val)
{
	// TODO implement
}
