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
#define WD1770_TAG      "u11"



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


//-------------------------------------------------
//  ADDRESS_MAP( c1571cr_mem )
//-------------------------------------------------

void c1571cr_device::c1571cr_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).ram().share("ram");
	map(0x1800, 0x180f).mirror(0x03f0).m(m_via0, FUNC(via6522_device::map));
	map(0x1c00, 0x1c0f).mirror(0x03f0).rw(FUNC(c1571cr_device::via1_r), FUNC(c1571cr_device::via1_w));
	map(0x2000, 0x2007).mirror(0x1fe0).rw(m_5710, FUNC(mos5710_device::fdc_r), FUNC(mos5710_device::fdc_w));
	map(0x4000, 0x400f).mirror(0x1fe0).rw(m_5710, FUNC(mos5710_device::cia_r), FUNC(mos5710_device::cia_w));
	map(0x4010, 0x4017).mirror(0x1be8).rw(m_5710, FUNC(mos5710_device::fdc2_r), FUNC(mos5710_device::fdc2_w));
	map(0x6000, 0x67ff).mirror(0x1800).ram().share("ram");
	map(0x8000, 0xffff).rom().region(M6502_TAG, 0);
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
		m_5710->set_unscaled_clock(clock);
		m_via0->set_unscaled_clock(clock);
		m_via1->set_unscaled_clock(clock);
		m_ga->accl_w(clock_1_2);

		m_1_2mhz = clock_1_2;
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1571cr_device::device_add_mconfig(machine_config &config)
{
	add_base_mconfig(config);
	config.device_remove(WD1770_TAG);

	m_maincpu->set_addrmap(AS_PROGRAM, &c1571cr_device::c1571cr_mem);

	m_via0->writepa_handler().set(FUNC(c1571cr_device::via0_pa_w));

	MOS5710(config, m_5710, 16_MHz_XTAL / 16);
	m_5710->irq_wr_callback().set("irqs", FUNC(input_merger_device::in_w<2>));
	m_5710->sp_wr_callback().set(FUNC(c1571cr_device::cia_sp_w));
	m_5710->cnt_wr_callback().set(FUNC(c1571cr_device::cia_cnt_w));
}


//-------------------------------------------------
//  c1571cr_device - constructor
//-------------------------------------------------

c1571cr_device::c1571cr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c1571_device(mconfig, C1571CR, tag, owner, clock),
	m_5710(*this, M5710_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1571cr_device::device_start()
{
	c1571_device::device_start();

	m_5710->set_floppy(m_floppy);
}


//-------------------------------------------------
//  update_iec -
//-------------------------------------------------

void c1571cr_device::update_iec()
{
	m_5710->cnt_w(m_bus->srq_r());
	m_5710->sp_w(m_bus->data_r());

	bool atn = m_bus->atn_r();
	m_via0->write_ca1(!atn);
	m_ga->atni_w(!atn);

	m_bus->clk_w(this, m_iec_clk);
	m_bus->data_w(this, !m_data_out && !m_ga->atn_r() && m_sp_out);
	m_bus->srq_w(this, m_cnt_out);
}
