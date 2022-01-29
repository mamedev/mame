// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    VTech Laser/VZ I/O Expansion Slot

    30-pin slot

***************************************************************************/

#include "emu.h"
#include "ioexp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_IOEXP_SLOT, vtech_ioexp_slot_device, "vtech_ioexp_slot", "Laser/VZ I/O Expansion Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_ioexp_slot_device - constructor
//-------------------------------------------------

vtech_ioexp_slot_device::vtech_ioexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_IOEXP_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_vtech_ioexp_interface>(mconfig, *this),
	m_iospace(*this, finder_base::DUMMY_TAG, -1)
{
}

//-------------------------------------------------
//  vtech_ioexp_slot_device - destructor
//-------------------------------------------------

vtech_ioexp_slot_device::~vtech_ioexp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_ioexp_slot_device::device_start()
{
	// get inserted module
	m_module = get_card_device();

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
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vtech_ioexp_interface - constructor
//-------------------------------------------------

device_vtech_ioexp_interface::device_vtech_ioexp_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vtechioexp")
{
	m_slot = dynamic_cast<vtech_ioexp_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_vtech_ioexp_interface - destructor
//-------------------------------------------------

device_vtech_ioexp_interface::~device_vtech_ioexp_interface()
{
}


//**************************************************************************
//  BASE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_ioexp_device - constructor
//-------------------------------------------------

vtech_ioexp_device::vtech_ioexp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vtech_ioexp_interface(mconfig, *this),
	m_io(*this, "iospace")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vtech_ioexp_device::device_add_mconfig(machine_config &config)
{
	ADDRESS_MAP_BANK(config, m_io);
	m_io->set_addrmap(AS_PROGRAM, &vtech_ioexp_device::io_map);
	m_io->set_data_width(8);
	m_io->set_addr_width(16);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_ioexp_device::device_start()
{
	// silence unmapped warnings, or we'll get them for everything the
	// expansion device doesn't handle
	m_io->set_log_unmap(false);
}

uint8_t vtech_ioexp_device::iorq_r(offs_t offset)
{
	return m_io->read8(offset);
}

void vtech_ioexp_device::iorq_w(offs_t offset, uint8_t data)
{
	m_io->write8(offset, data);
}
