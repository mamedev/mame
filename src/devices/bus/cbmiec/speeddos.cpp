// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SpeedDOS-compatible 1541 parallel DOS emulation

**********************************************************************/

#include "emu.h"
#include "speeddos.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_SPEEDDOS, c1541_speeddos_device, "c1541sd", "Commodore 1541 SpeedDOS")


//-------------------------------------------------
//  ROM( c1541sd )
//-------------------------------------------------

ROM_START( c1541sd )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("speeddos")
	ROM_SYSTEM_BIOS( 0, "speeddos", "SpeedDOS-Plus+" )
	ROMX_LOAD( "speed-dosplus.uab5", 0x0000, 0x4000, CRC(f9db1eac) SHA1(95407e59a9c1d26a0e4bcf2c244cfe8942576e2c), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "rolo27", "Rolo DOS v2.7" )
	ROMX_LOAD( "325302-01.uab4", 0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11), ROM_BIOS(1) )
	ROMX_LOAD( "rolo27.uab5", 0x2000, 0x2000, CRC(1fc4793f) SHA1(fa28a7e318712b32959b2d39a56da68406b8a69a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "digidos", "DigiDOS" )
	ROMX_LOAD( "325302-01.uab4", 0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11), ROM_BIOS(2) )
	ROMX_LOAD( "digidos.uab5", 0x2000, 0x2000, CRC(b3f05ea3) SHA1(99d3d848344c68410b686cda812f3788b41fead3), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_speeddos_device::device_rom_region() const
{
	return ROM_NAME( c1541sd );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_speeddos_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);

	m_via0->readpa_handler().set(FUNC(c1541_speeddos_device::via0_pa_r));
	m_via0->writepa_handler().set(FUNC(c1541_speeddos_device::via0_pa_w));
	m_via0->ca2_handler().set(FUNC(c1541_speeddos_device::via0_ca2_w));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_speeddos_device - constructor
//-------------------------------------------------

c1541_speeddos_device::c1541_speeddos_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	c1541_device_base(mconfig, C1541_SPEEDDOS, tag, owner, clock),
	device_c64_floppy_parallel_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1541_speeddos_device::device_start()
{
	c1541_device_base::device_start();

	save_item(NAME(m_parallel_data));
}


//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void c1541_speeddos_device::parallel_data_w(u8 data)
{
	m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c1541_speeddos_device::parallel_strobe_w(int state)
{
	m_via0->write_cb1(state);
}


u8 c1541_speeddos_device::via0_pa_r()
{
	return m_parallel_data;
}

void c1541_speeddos_device::via0_pa_w(u8 data)
{
	if (m_other != nullptr)
	{
		m_other->parallel_data_w(data);
	}
}

void c1541_speeddos_device::via0_ca2_w(int state)
{
	if (m_other != nullptr)
	{
		m_other->parallel_strobe_w(state);
	}
}
