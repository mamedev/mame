// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Laser Memory Expansions

***************************************************************************/

#include "emu.h"
#include "memory.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_LASER110_16K, vtech_laser110_16k_device, "vtech_laser110_16k", "Laser 110/200/VZ-200 16k Memory")
DEFINE_DEVICE_TYPE(VTECH_LASER210_16K, vtech_laser210_16k_device, "vtech_laser210_16k", "Laser 210/VZ-200 16k Memory")
DEFINE_DEVICE_TYPE(VTECH_LASER310_16K, vtech_laser310_16k_device, "vtech_laser310_16k", "Laser 310/VZ-300 16k Memory")
DEFINE_DEVICE_TYPE(VTECH_LASER_64K,    vtech_laser_64k_device,    "vtech_laser_64k",    "Laser/VZ 64k Memory")


//**************************************************************************
//  LASER 110 16K DEVICE
//**************************************************************************

//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_laser110_16k_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x8000, 0xbfff).ram();
}

//-------------------------------------------------
//  laser110_16k_device - constructor
//-------------------------------------------------

vtech_laser110_16k_device::vtech_laser110_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_LASER110_16K, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_laser110_16k_device::device_start()
{
	vtech_memexp_device::device_start();
}


//**************************************************************************
//  LASER 210 16K DEVICE
//**************************************************************************

//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_laser210_16k_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x9000, 0xcfff).ram();
}

//-------------------------------------------------
//  vtech_laser210_16k_device - constructor
//-------------------------------------------------

vtech_laser210_16k_device::vtech_laser210_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_LASER210_16K, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_laser210_16k_device::device_start()
{
	vtech_memexp_device::device_start();
}


//**************************************************************************
//  VZ300 16K DEVICE
//**************************************************************************

//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_laser310_16k_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0xb800, 0xf7ff).ram();
}

//-------------------------------------------------
//  vtech_laser310_16k_device - constructor
//-------------------------------------------------

vtech_laser310_16k_device::vtech_laser310_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_LASER310_16K, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_laser310_16k_device::device_start()
{
	vtech_memexp_device::device_start();
}


//**************************************************************************
//  VZ300 64K DEVICE
//**************************************************************************

//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_laser_64k_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x8000, 0xbfff).bankrw(m_fixed_bank);
	map(0xc000, 0xffff).bankrw(m_bank);
}

//-------------------------------------------------
//  io_map - memory space address map
//-------------------------------------------------

void vtech_laser_64k_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x70, 0x70).mirror(0x0f).lw8(NAME([this] (uint8_t data) { m_bank->set_entry(data & 0x03); }));
}

//-------------------------------------------------
//  vtech_laser_64k_device - constructor
//-------------------------------------------------

vtech_laser_64k_device::vtech_laser_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_LASER_64K, tag, owner, clock),
	m_fixed_bank(*this, "fixed_bank"),
	m_bank(*this, "bank")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_laser_64k_device::device_start()
{
	vtech_memexp_device::device_start();

	// init ram
	m_ram = std::make_unique<uint8_t[]>(0x10000);

	// configure banking
	m_fixed_bank->set_base(m_ram.get());

	m_bank->configure_entries(0, 4, m_ram.get(), 0x4000);
	m_bank->set_entry(1);

	// register for savestates
	save_pointer(NAME(m_ram), 0x10000);
}
