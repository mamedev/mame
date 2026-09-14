// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1571CR Single Disk Drive emulation

**********************************************************************/

#include "emu.h"
#include "c1571cr.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "u1"
#define M5710_TAG       "u107"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1571CR, c1571cr_device, "c1571cr", "Commodore 1571CR Disk Drive")


//-------------------------------------------------
//  ROM( c1571cr )
//-------------------------------------------------

ROM_START( c1571cr )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("cbm")
	ROM_SYSTEM_BIOS( 0, "cbm", "Commodore" )
	ROMX_LOAD( "318047-01.u102", 0x0000, 0x8000, CRC(f24efcc4) SHA1(14ee7a0fb7e1c59c51fbf781f944387037daa3ee), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1571d.u102", 0x0000, 0x8000, CRC(9cba146d) SHA1(823b178561302b403e6bfd8dd741d757efef3958), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1571cr_device::device_rom_region() const
{
	return ROM_NAME( c1571cr );
}


void c1571cr_device::via0_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2     SIDE
	    PA3
	    PA4
	    PA5     _1/2 MHZ
	    PA6
	    PA7

	*/

	// side select
	m_floppy->ss_w(BIT(data, 2));

	// 1/2 MHz
	int clock_1_2 = BIT(data, 5);

	if (m_1_2mhz != clock_1_2)
	{
		const XTAL clock = 16_MHz_XTAL / (clock_1_2 ? 8 : 16);

		m_maincpu->set_unscaled_clock(clock);
		m_cia->set_unscaled_clock(clock);
		m_via0->set_unscaled_clock(clock);
		m_via1->set_unscaled_clock(clock);
		m_ga->accl_w(clock_1_2);

		m_1_2mhz = clock_1_2;
	}
}

void c1571cr_device::via0_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0
	    PB1     DATA OUT
	    PB2
	    PB3     CLK OUT
	    PB4     ATNI
	    PB5
	    PB6
	    PB7

	*/

	// data out
	m_data_out = BIT(data, 1);

	// clock out
	m_iec_clk = !BIT(data, 3);

	// attention in
	m_ga->atni_w(BIT(data, 4));

	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1571cr_device::device_add_mconfig(machine_config &config)
{
	add_base_mconfig(config);

	m_via0->writepa_handler().set(FUNC(c1571cr_device::via0_pa_w));
	m_via0->writepb_handler().set(FUNC(c1571cr_device::via0_pb_w));

	MOS5710(config, M5710_TAG, XTAL(16'000'000)/16);
}


//-------------------------------------------------
//  c1571cr_device - constructor
//-------------------------------------------------

c1571cr_device::c1571cr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c1571_device(mconfig, C1571CR, tag, owner, clock) { }
