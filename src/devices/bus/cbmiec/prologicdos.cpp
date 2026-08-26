// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    REX Data Technology ProLogic-DOS Classic

**********************************************************************/

#include "emu.h"
#include "prologicdos.h"

#include "bus/centronics/ctronics.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"
#define MC6821_TAG      "pia"
#define CENTRONICS_TAG  "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_PROLOGIC_DOS_CLASSIC, c1541_prologic_dos_classic_device, "c1541pdc", "REX Data Technology ProLogic-DOS Classic")


//-------------------------------------------------
//  ROM( c1541pdc )
//-------------------------------------------------

ROM_START( c1541pdc )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "325302-01.uab4", 0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11) )
	ROM_LOAD( "901229-06 aa.uab5", 0x2000, 0x2000, CRC(3a235039) SHA1(c7f94f4f51d6de4cdc21ecbb7e57bb209f0530c0) )
	ROM_LOAD( "kernal.bin", 0x4000, 0x4000, CRC(79032ed5) SHA1(0ca4d5ef41c7e3d18d8945476d1481573af3e27c) )

	ROM_REGION( 0x2000, "mmu", 0 )
	ROM_LOAD( "mmu.bin", 0x0000, 0x2000, CRC(4c41392c) SHA1(78846af2ee6a56fceee44f9246659685ab2cbb7e) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_prologic_dos_classic_device::device_rom_region() const
{
	return ROM_NAME( c1541pdc );
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t c1541_prologic_dos_classic_device::read()
{
	return 0;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void c1541_prologic_dos_classic_device::write(uint8_t data)
{
}


//-------------------------------------------------
//  ADDRESS_MAP( c1541pdc_mem )
//-------------------------------------------------

void c1541_prologic_dos_classic_device::c1541pdc_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c1541_prologic_dos_classic_device::read), FUNC(c1541_prologic_dos_classic_device::write));
/*  map(0x0000, 0x07ff).mirror(0x6000).ram().share("share1");
    map(0x1800, 0x180f).mirror(0x63f0).rw(M6522_0_TAG, FUNC(via6522_device::read), FUNC(via6522_device::write));
    map(0x1c00, 0x1c0f).mirror(0x63f0).rw(M6522_1_TAG, FUNC(via6522_device::read), FUNC(via6522_device::write));
    map(0x8000, 0x87ff).ram().share("share1");
    map(0x8800, 0x9fff).ram();
    map(0xa000, 0xb7ff).rom().region(M6502_TAG, 0x0000);
    map(0xb800, 0xb80f).rw(FUNC(c1541_prologic_dos_classic_device::pia_r), FUNC(c1541_prologic_dos_classic_device::pia_w));
    map(0xf000, 0xffff).rom().region(M6502_TAG, 0x2000);*/
}


uint8_t c1541_prologic_dos_classic_device::pia_r(offs_t offset)
{
	return m_pia->read((offset >> 2) & 0x03);
}

void c1541_prologic_dos_classic_device::pia_w(offs_t offset, uint8_t data)
{
	m_pia->write((offset >> 2) & 0x03, data);
}

void c1541_prologic_dos_classic_device::pia_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       1/2 MHz
	    1
	    2
	    3       35/40 tracks
	    4
	    5
	    6
	    7       Hi

	*/
}

uint8_t c1541_prologic_dos_classic_device::pia_pb_r()
{
	return m_parallel_data;
}

void c1541_prologic_dos_classic_device::pia_pb_w(uint8_t data)
{
	m_parallel_data = data;

	m_cent_data_out->write(data);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_prologic_dos_classic_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &c1541_prologic_dos_classic_device::c1541pdc_mem);

	PIA6821(config, m_pia);
	m_pia->readpb_handler().set(FUNC(c1541_prologic_dos_classic_device::pia_pb_r));
	m_pia->writepa_handler().set(FUNC(c1541_prologic_dos_classic_device::pia_pa_w));
	m_pia->writepb_handler().set(FUNC(c1541_prologic_dos_classic_device::pia_pb_w));
	m_pia->ca2_handler().set(CENTRONICS_TAG, FUNC(centronics_device::write_strobe));

	centronics_device &centronics(CENTRONICS(config, CENTRONICS_TAG, centronics_devices, "printer"));
	centronics.ack_handler().set(MC6821_TAG, FUNC(pia6821_device::ca1_w));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_prologic_dos_classic_device - constructor
//-------------------------------------------------

c1541_prologic_dos_classic_device::c1541_prologic_dos_classic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541_PROLOGIC_DOS_CLASSIC, tag, owner, clock),
		m_pia(*this, MC6821_TAG),
		m_cent_data_out(*this, "cent_data_out"),
		m_mmu_rom(*this, "mmu")
{
}
