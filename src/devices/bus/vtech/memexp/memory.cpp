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
//  laser110_16k_device - constructor
//-------------------------------------------------

vtech_laser110_16k_device::vtech_laser110_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_LASER110_16K, tag, owner, clock),
	device_vtech_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_laser110_16k_device::device_start()
{
	m_ram.resize(16 * 1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vtech_laser110_16k_device::device_reset()
{
	program_space().install_ram(0x8000, 0xbfff, &m_ram[0]);
}


//**************************************************************************
//  LASER 210 16K DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_laser210_16k_device - constructor
//-------------------------------------------------

vtech_laser210_16k_device::vtech_laser210_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_LASER210_16K, tag, owner, clock),
	device_vtech_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_laser210_16k_device::device_start()
{
	m_ram.resize(16 * 1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vtech_laser210_16k_device::device_reset()
{
	program_space().install_ram(0x9000, 0xcfff, &m_ram[0]);
}


//**************************************************************************
//  VZ300 16K DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_laser310_16k_device - constructor
//-------------------------------------------------

vtech_laser310_16k_device::vtech_laser310_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_LASER310_16K, tag, owner, clock),
	device_vtech_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_laser310_16k_device::device_start()
{
	m_ram.resize(16 * 1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vtech_laser310_16k_device::device_reset()
{
	program_space().install_ram(0xb800, 0xf7ff, &m_ram[0]);
}


//**************************************************************************
//  VZ300 64K DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_laser_64k_device - constructor
//-------------------------------------------------

vtech_laser_64k_device::vtech_laser_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_LASER_64K, tag, owner, clock),
	device_vtech_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_laser_64k_device::device_start()
{
	m_ram.resize(64 * 1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vtech_laser_64k_device::device_reset()
{
	// fixed first bank
	program_space().install_ram(0x8000, 0xbfff, &m_ram[0]);

	// other banks
	program_space().install_readwrite_bank(0xc000, 0xffff, tag());

	membank(tag())->configure_entries(0, 4, &m_ram[0], 0x4000);
	membank(tag())->set_entry(1);

	// bank switch
	io_space().install_write_handler(0x70, 0x7f, write8_delegate(FUNC(vtech_laser_64k_device::bankswitch_w), this));
}

WRITE8_MEMBER( vtech_laser_64k_device::bankswitch_w )
{
	membank(tag())->set_entry(data & 0x03);
}
