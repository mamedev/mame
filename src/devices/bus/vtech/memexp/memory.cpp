// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Laser Memory Expansions

***************************************************************************/

#include "memory.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LASER110_16K = &device_creator<laser110_16k_device>;
const device_type LASER210_16K = &device_creator<laser210_16k_device>;
const device_type LASER310_16K = &device_creator<laser310_16k_device>;
const device_type LASER_64K = &device_creator<laser_64k_device>;


//**************************************************************************
//  LASER 110 16K DEVICE
//**************************************************************************

//-------------------------------------------------
//  laser110_16k_device - constructor
//-------------------------------------------------

laser110_16k_device::laser110_16k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, LASER110_16K, "Laser 110/200/VZ-200 16k Memory", tag, owner, clock, "laser110_16k", __FILE__),
	device_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void laser110_16k_device::device_start()
{
	m_ram.resize(16 * 1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void laser110_16k_device::device_reset()
{
	m_slot->m_program->install_ram(0x8000, 0xbfff, &m_ram[0]);
}


//**************************************************************************
//  LASER 210 16K DEVICE
//**************************************************************************

//-------------------------------------------------
//  laser210_16k_device - constructor
//-------------------------------------------------

laser210_16k_device::laser210_16k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, LASER210_16K, "Laser 210/VZ-200 (DSE) 16k Memory", tag, owner, clock, "laser210_16k", __FILE__),
	device_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void laser210_16k_device::device_start()
{
	m_ram.resize(16 * 1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void laser210_16k_device::device_reset()
{
	m_slot->m_program->install_ram(0x9000, 0xcfff, &m_ram[0]);
}


//**************************************************************************
//  VZ300 16K DEVICE
//**************************************************************************

//-------------------------------------------------
//  laser310_16k_device - constructor
//-------------------------------------------------

laser310_16k_device::laser310_16k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, LASER310_16K, "Laser 310/VZ-300 16k Memory", tag, owner, clock, "laser310_16k", __FILE__),
	device_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void laser310_16k_device::device_start()
{
	m_ram.resize(16 * 1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void laser310_16k_device::device_reset()
{
	m_slot->m_program->install_ram(0xb800, 0xf7ff, &m_ram[0]);
}


//**************************************************************************
//  VZ300 64K DEVICE
//**************************************************************************

//-------------------------------------------------
//  laser_64k_device - constructor
//-------------------------------------------------

laser_64k_device::laser_64k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, LASER_64K, "Laser/VZ 64k Memory", tag, owner, clock, "laser_64k", __FILE__),
	device_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void laser_64k_device::device_start()
{
	m_ram.resize(64 * 1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void laser_64k_device::device_reset()
{
	// fixed first bank
	m_slot->m_program->install_ram(0x8000, 0xbfff, &m_ram[0]);

	// other banks
	m_slot->m_program->install_readwrite_bank(0xc000, 0xffff, tag());

	membank(tag())->configure_entries(0, 4, &m_ram[0], 0x4000);
	membank(tag())->set_entry(1);

	// bank switch
	m_slot->m_io->install_write_handler(0x70, 0x7f, write8_delegate(FUNC(laser_64k_device::bankswitch_w), this));
}

WRITE8_MEMBER( laser_64k_device::bankswitch_w )
{
	membank(tag())->set_entry(data & 0x03);
}
