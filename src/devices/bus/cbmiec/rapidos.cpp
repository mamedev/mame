// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Chip Level Design RapiDOS Professional

**********************************************************************/

#include "emu.h"
#include "rapidos.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"
#define M6522_0_TAG     "uab1"
#define M6522_1_TAG     "ucd4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_RAPIDOS_PROFESSIONAL, c1541_rapidos_professional_device, "c1541rp", "Chip Level Design RapiDOS Professional")


//-------------------------------------------------
//  ROM( c1541rp )
//-------------------------------------------------

ROM_START( c1541rp )
	ROM_REGION( 0x2000, M6502_TAG, 0 )
	ROM_LOAD( "325302-01.uab4", 0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11) )

	ROM_REGION( 0x8000, "eprom", 0 )
	ROM_LOAD( "rapidos_pro_20_drive.bin", 0x0000, 0x8000, CRC(6dbe0668) SHA1(06242e082a36f6751dca3d4c8c386737627a494c) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_rapidos_professional_device::device_rom_region() const
{
	return ROM_NAME( c1541rp );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1541rp_mem )
//-------------------------------------------------

void c1541_rapidos_professional_device::c1541rp_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x2000).ram();
	map(0x1800, 0x180f).mirror(0x23f0).r(FUNC(c1541_rapidos_professional_device::via0_r)).w(M6522_0_TAG, FUNC(via6522_device::write));
	map(0x1c00, 0x1c0f).mirror(0x23f0).m(M6522_1_TAG, FUNC(via6522_device::map));
	map(0x4000, 0x5fff).ram();
	map(0x6000, 0x6fff).bankr(m_bank[0]);
	map(0x6800, 0x6fff).w(FUNC(c1541_rapidos_professional_device::clock_w));
	map(0x7000, 0x7fff).r(FUNC(c1541_rapidos_professional_device::decoder_r)).nopw();
	map(0x8000, 0x9fff).mirror(0x4000).rom().region(M6502_TAG, 0);
	map(0xa000, 0xbfff).mirror(0x4000).bankr(m_bank[1]);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_rapidos_professional_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &c1541_rapidos_professional_device::c1541rp_mem);

	m_via0->readpa_handler().set(FUNC(c1541_rapidos_professional_device::via0_pa_r));
	m_via0->writepa_handler().set(FUNC(c1541_rapidos_professional_device::via0_pa_w));
	m_via0->ca2_handler().set(FUNC(c1541_rapidos_professional_device::via0_ca2_w));
}


//-------------------------------------------------
//  INPUT_PORTS( c1541rp )
//-------------------------------------------------

static INPUT_PORTS_START( c1541rp )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )

	PORT_START("SWITCH")
	PORT_CONFNAME( 0x01, 0x00, "DOS Select" )
	PORT_CONFSETTING(    0x00, "RapiDOS" )
	PORT_CONFSETTING(    0x01, "CBM DOS" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1541_rapidos_professional_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1541rp );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_rapidos_professional_device - constructor
//-------------------------------------------------

c1541_rapidos_professional_device::c1541_rapidos_professional_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	c1541_device_base(mconfig, C1541_RAPIDOS_PROFESSIONAL, tag, owner, clock),
	device_c64_floppy_parallel_interface(mconfig, *this),
	m_rom(*this, "eprom"),
	m_bank(*this, "bank%u", 0U),
	m_switch(*this, "SWITCH")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1541_rapidos_professional_device::device_start()
{
	c1541_device_base::device_start();

	m_bank[0]->configure_entries(0, 2, &m_rom[0x0000], 0x4000);
	m_bank[1]->configure_entries(0, 2, &m_rom[0x2000], 0x4000);

	save_item(NAME(m_parallel_data));
	save_item(NAME(m_latch));
	save_item(NAME(m_a14));
	save_item(NAME(m_slow));
}


//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void c1541_rapidos_professional_device::device_post_load()
{
	update_clock();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1541_rapidos_professional_device::device_reset()
{
	c1541_device_base::device_reset();

	int const a14 = BIT(m_switch->read(), 0);

	m_a14 = a14 << 14;
	m_bank[0]->set_entry(a14);
	m_bank[1]->set_entry(a14);
	m_latch = 0;

	clock_w(2, 0);
}


u8 c1541_rapidos_professional_device::decoder_r(offs_t offset)
{
	offs_t const addr = (offset & 0xf00) | (m_latch << 4) | (BIT(offset, 11) ? BIT(offset, 4, 4) : BIT(offset, 0, 4));

	if (!machine().side_effects_disabled())
		m_latch = addr & 0x0f;

	return m_rom[m_a14 | 0x1000 | addr];
}


void c1541_rapidos_professional_device::clock_w(offs_t offset, u8 data)
{
	m_slow = BIT(offset, 1);

	update_clock();
}

void c1541_rapidos_professional_device::update_clock()
{
	XTAL const clock = 16_MHz_XTAL / (m_slow ? 16 : 8);

	m_maincpu->set_unscaled_clock(clock);
	m_via0->set_unscaled_clock(clock);
	m_via1->set_unscaled_clock(clock);
}


//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void c1541_rapidos_professional_device::parallel_data_w(u8 data)
{
	m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c1541_rapidos_professional_device::parallel_strobe_w(int state)
{
	m_via0->write_cb1(state);
}


u8 c1541_rapidos_professional_device::via0_pa_r()
{
	return m_parallel_data;
}

void c1541_rapidos_professional_device::via0_pa_w(u8 data)
{
	if (m_other != nullptr)
	{
		m_other->parallel_data_w(data);
	}
}

void c1541_rapidos_professional_device::via0_ca2_w(int state)
{
	if (m_other != nullptr)
	{
		m_other->parallel_strobe_w(state);
	}
}
