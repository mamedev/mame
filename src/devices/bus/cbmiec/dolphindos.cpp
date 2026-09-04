// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Evesham Micros Dolphin-DOS

**********************************************************************/

#include "emu.h"
#include "dolphindos.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"
#define M6522_0_TAG     "uab1"
#define M6522_1_TAG     "ucd4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_DOLPHIN_DOS_V2, c1541_dolphin_dos_v2_device, "c1541dd2", "Evesham Micros Dolphin-DOS v2")
DEFINE_DEVICE_TYPE(C1571_DOLPHIN_DOS_V3, c1571_dolphin_dos_v3_device, "c1571dd3", "Evesham Micros Dolphin-DOS v3")


//-------------------------------------------------
//  ROM( c1541dd )
//-------------------------------------------------

ROM_START( c1541dd )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "dd20.bin", 0x0000, 0x8000, CRC(94c7fe19) SHA1(e4d5b9ad6b719dd988276214aa4536d3525d313c) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_dolphin_dos_v2_device::device_rom_region() const
{
	return ROM_NAME( c1541dd );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1541dd_mem )
//-------------------------------------------------

void c1541_dolphin_dos_v2_device::c1541dd_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x6000).ram();
	map(0x1800, 0x180f).mirror(0x63f0).m(M6522_0_TAG, FUNC(via6522_device::map));
	map(0x1c00, 0x1c0f).mirror(0x63f0).m(M6522_1_TAG, FUNC(via6522_device::map));
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xffff).rom().region(M6502_TAG, 0x2000);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_dolphin_dos_v2_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &c1541_dolphin_dos_v2_device::c1541dd_mem);
	
	m_via0->readpa_handler().set(FUNC(c1541_dolphin_dos_v2_device::via0_pa_r));
	m_via0->writepa_handler().set(FUNC(c1541_dolphin_dos_v2_device::via0_pa_w));
	m_via0->ca2_handler().set(FUNC(c1541_dolphin_dos_v2_device::via0_ca2_w));
}
	


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_dolphin_dos_v2_device - constructor
//-------------------------------------------------

c1541_dolphin_dos_v2_device::c1541_dolphin_dos_v2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541_DOLPHIN_DOS_V2, tag, owner, clock),
	  device_c64_floppy_parallel_interface(mconfig, *this) {  }


//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void c1541_dolphin_dos_v2_device::parallel_data_w(u8 data)
{
	m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c1541_dolphin_dos_v2_device::parallel_strobe_w(int state)
{
	m_via0->write_cb1(state);
}

uint8_t c1541_dolphin_dos_v2_device::via0_pa_r()
{
	return m_parallel_data;
}

void c1541_dolphin_dos_v2_device::via0_pa_w(uint8_t data)
{
	if (m_other != nullptr)
	{
		m_other->parallel_data_w(data);
	}
}

void c1541_dolphin_dos_v2_device::via0_ca2_w(int state)
{
	if (m_other != nullptr)
	{
		m_other->parallel_strobe_w(state);
	}
}


//-------------------------------------------------
//  c1571_dolphin_dos_v3_device - constructor
//-------------------------------------------------

c1571_dolphin_dos_v3_device::c1571_dolphin_dos_v3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1571_device(mconfig, C1571_DOLPHIN_DOS_V3, tag, owner, clock),
	  device_c64_floppy_parallel_interface(mconfig, *this) {  }


//-------------------------------------------------
//  ROM( c1571dd3 )
//-------------------------------------------------

ROM_START( c1571dd3 )
	ROM_REGION( 0x8000, "u1", 0 )
	ROM_LOAD( "1571-dolphin-dos-3.bin", 0x0000, 0x8000, CRC(59d857f7) SHA1(651f12c5629d44c6f8e86130aef9624659c1eea6) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1571_dolphin_dos_v3_device::device_rom_region() const
{
	return ROM_NAME( c1571dd3 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

uint8_t c1571_dolphin_dos_v3_device::cia_pb_r()
{
	return m_parallel_data;
}

void c1571_dolphin_dos_v3_device::cia_pb_w(uint8_t data)
{
	if (m_other != nullptr)
	{
		m_other->parallel_data_w(data);
	}
}

void c1571_dolphin_dos_v3_device::cia_pc_w(int state)
{
	if (m_other != nullptr)
	{
		m_other->parallel_strobe_w(state);
	}
}

void c1571_dolphin_dos_v3_device::device_add_mconfig(machine_config &config)
{
	c1571_device::device_add_mconfig(config);

	m_cia->pb_rd_callback().set(FUNC(c1571_dolphin_dos_v3_device::cia_pb_r));
	m_cia->pb_wr_callback().set(FUNC(c1571_dolphin_dos_v3_device::cia_pb_w));
	m_cia->pc_wr_callback().set(FUNC(c1571_dolphin_dos_v3_device::cia_pc_w));
}

//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void c1571_dolphin_dos_v3_device::parallel_data_w(uint8_t data)
{
	m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c1571_dolphin_dos_v3_device::parallel_strobe_w(int state)
{
	m_cia->flag_w(state);
}
