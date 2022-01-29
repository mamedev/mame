// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansion Slot

    44-pin slot

***************************************************************************/

#include "emu.h"
#include "memexp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_MEMEXP_SLOT, vtech_memexp_slot_device, "vtech_memexp_slot", "Laser/VZ Memory Expansion Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_memexp_slot_device - constructor
//-------------------------------------------------

vtech_memexp_slot_device::vtech_memexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_MEMEXP_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_vtech_memexp_interface>(mconfig, *this),
	m_memspace(*this, finder_base::DUMMY_TAG, -1),
	m_iospace(*this, finder_base::DUMMY_TAG, -1),
	m_int_handler(*this),
	m_nmi_handler(*this),
	m_reset_handler(*this)
{
}

//-------------------------------------------------
//  vtech_memexp_slot_device - destructor
//-------------------------------------------------

vtech_memexp_slot_device::~vtech_memexp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_memexp_slot_device::device_start()
{
	// get inserted module
	m_module = get_card_device();

	// install memory access taps
	m_memspace->install_readwrite_tap
	(
		0x0000, 0xffff, "mem_tap",
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask)
		{
			if (m_module)
				data &= m_module->mreq_r(offset);
		},
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask)
		{
			if (m_module)
				m_module->mreq_w(offset, data);
		}
	);

	// install io access taps
	m_iospace->install_readwrite_tap
	(
		0x00, 0xff, "io_tap",
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask)
		{
			if (m_module)
				data &= m_module->iorq_r(offset);
		},
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask)
		{
			if (m_module)
				m_module->iorq_w(offset, data);
		}
	);

	// resolve callbacks
	m_int_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
	m_reset_handler.resolve_safe();
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vtech_memexp_interface - constructor
//-------------------------------------------------

device_vtech_memexp_interface::device_vtech_memexp_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vtechmemexp")
{
	m_slot = dynamic_cast<vtech_memexp_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_vtech_memexp_interface - destructor
//-------------------------------------------------

device_vtech_memexp_interface::~device_vtech_memexp_interface()
{
}


//**************************************************************************
//  BASE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_memexp_device - constructor
//-------------------------------------------------

vtech_memexp_device::vtech_memexp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vtech_memexp_interface(mconfig, *this),
	m_mem(*this, "memspace"),
	m_io(*this, "iospace")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vtech_memexp_device::device_add_mconfig(machine_config &config)
{
	ADDRESS_MAP_BANK(config, m_mem);
	m_mem->set_addrmap(AS_PROGRAM, &vtech_memexp_device::mem_map);
	m_mem->set_data_width(8);
	m_mem->set_addr_width(16);

	ADDRESS_MAP_BANK(config, m_io);
	m_io->set_addrmap(AS_PROGRAM, &vtech_memexp_device::io_map);
	m_io->set_data_width(8);
	m_io->set_addr_width(16);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_memexp_device::device_start()
{
	// silence unmapped warnings, or we'll get them for everything the
	// expansion device doesn't handle
	m_mem->set_log_unmap(false);
	m_io->set_log_unmap(false);
}

uint8_t vtech_memexp_device::mreq_r(offs_t offset)
{
	return m_mem->read8(offset);
}

void vtech_memexp_device::mreq_w(offs_t offset, uint8_t data)
{
	m_mem->write8(offset, data);
}

uint8_t vtech_memexp_device::iorq_r(offs_t offset)
{
	return m_io->read8(offset);
}

void vtech_memexp_device::iorq_w(offs_t offset, uint8_t data)
{
	m_io->write8(offset, data);
}
