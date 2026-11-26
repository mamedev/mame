// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rossmöller TurboTrans

**********************************************************************/

#include "emu.h"
#include "turbotrans.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_TURBOTRANS, c1541_turbotrans_device, "c1541tt", "Rossmöller TurboTrans")


//-------------------------------------------------
//  ROM( c1541tt )
//-------------------------------------------------

ROM_START( c1541tt )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("v34")
	ROM_SYSTEM_BIOS( 0, "v30", "v3.0" )
	ROMX_LOAD( "ttd30.uab5", 0x0000, 0x8000, CRC(32985d32) SHA1(0c5d51b65ec6a09cd738afcce668f49cc58c2a4e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v31", "v3.1" )
	ROMX_LOAD( "ttd31.uab5", 0x0000, 0x8000, CRC(7fc69491) SHA1(e0215024da1b014bd8b1d9a3844b8a04d50ccf1e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v34", "v3.4" )
	ROMX_LOAD( "ttd34.uab5", 0x0000, 0x8000, CRC(518d34a1) SHA1(4d6ffdce6ab122e9627b0a839861687bcd4e03ec), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_turbotrans_device::device_rom_region() const
{
	return ROM_NAME( c1541tt );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1541tt_mem )
//-------------------------------------------------

void c1541_turbotrans_device::c1541tt_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x6000).ram();
	map(0x0800, 0x0800).mirror(0x07ff).lw8(NAME([this] (u8 data) { m_ext.select(0); }));
	map(0x1000, 0x1000).mirror(0x07ff).lw8(NAME([this] (u8 data) { m_ext.select(1); }));
	map(0x1800, 0x180f).mirror(0x63f0).r(FUNC(c1541_turbotrans_device::via0_r)).w(m_via0, FUNC(via6522_device::write));
	map(0x1c00, 0x1c0f).mirror(0x63f0).m(m_via1, FUNC(via6522_device::map));
	map(0x2800, 0x2800).mirror(0x07ff).lw8(NAME([this] (u8 data) { m_dram_enable = true; }));
	map(0x3000, 0x3000).mirror(0x07ff).lw8(NAME([this] (u8 data) { m_dram_enable = false; }));
	map(0x4800, 0x4800).mirror(0x07ff).lw8(NAME([this] (u8 data) { m_counter = 0; }));
	map(0x5000, 0x5000).mirror(0x07ff).lw8(NAME([this] (u8 data) { m_counter++; }));
	map(0x6800, 0x6bff).mirror(0x0400).rw(FUNC(c1541_turbotrans_device::dram_r<0>), FUNC(c1541_turbotrans_device::dram_w<0>));
	map(0x7000, 0x73ff).mirror(0x0400).rw(FUNC(c1541_turbotrans_device::dram_r<1>), FUNC(c1541_turbotrans_device::dram_w<1>));
	map(0x8000, 0xbfff).view(m_ext);
	m_ext[0](0x8000, 0x9fff).rom().region(M6502_TAG, 0);
	m_ext[0](0xa000, 0xbfff).bankr(m_dos);
	m_ext[1](0x8000, 0x9fff).rom().region(M6502_TAG, 0x4000);
	m_ext[1](0xa000, 0xbfff).ram().share(m_sram);
	map(0xc000, 0xdfff).rom().region(M6502_TAG, 0);
	map(0xe000, 0xffff).bankr(m_dos);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_turbotrans_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &c1541_turbotrans_device::c1541tt_mem);

	m_via0->readpa_handler().set(FUNC(c1541_turbotrans_device::via0_pa_r));
	m_via0->writepa_handler().set(FUNC(c1541_turbotrans_device::via0_pa_w));
	m_via0->ca2_handler().set(FUNC(c1541_turbotrans_device::via0_ca2_w));
}


//-------------------------------------------------
//  INPUT_PORTS( c1541tt )
//-------------------------------------------------

static INPUT_PORTS_START( c1541tt )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "DRAM" )
	PORT_CONFSETTING(    0x00, "256 KB" )
	PORT_CONFSETTING(    0x01, "512 KB" )
	PORT_CONFNAME( 0x02, 0x02, "DOS" )
	PORT_CONFSETTING(    0x00, "Original" )
	PORT_CONFSETTING(    0x02, "TurboTrans" )

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("TurboTrans NMI") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c1541_turbotrans_device::nmi_w), 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1541_turbotrans_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1541tt );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_turbotrans_device - constructor
//-------------------------------------------------

c1541_turbotrans_device::c1541_turbotrans_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	c1541_device_base(mconfig, C1541_TURBOTRANS, tag, owner, clock),
	device_c64_floppy_parallel_interface(mconfig, *this),
	m_config(*this, "CONFIG"),
	m_ext(*this, "ext"),
	m_dos(*this, "dos"),
	m_sram(*this, "sram", 0x2000, ENDIANNESS_LITTLE),
	m_dram_enable(false),
	m_counter(0),
	m_dram_banks(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1541_turbotrans_device::device_start()
{
	c1541_device_base::device_start();

	m_dos->configure_entries(0, 2, memregion(M6502_TAG)->base() + 0x2000, 0x4000);

	m_dram = make_unique_clear<u8[]>(0x80000);

	save_pointer(NAME(m_dram), 0x80000);
	save_item(NAME(m_dram_enable));
	save_item(NAME(m_counter));
	save_item(NAME(m_parallel_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1541_turbotrans_device::device_reset()
{
	c1541_device_base::device_reset();

	u8 config = m_config->read();
	m_dram_banks = BIT(config, 0) ? 2 : 1;
	m_dos->set_entry(BIT(config, 1));

	m_ext.select(0);
	m_dram_enable = false;
}


//-------------------------------------------------
//  dram_r -
//-------------------------------------------------

template <int Bank> u8 c1541_turbotrans_device::dram_r(offs_t offset)
{
	if (!m_dram_enable || Bank >= m_dram_banks)
		return m_maincpu->space(AS_PROGRAM).unmap();

	return m_dram[(Bank << 18) | (offset << 8) | m_counter];
}


//-------------------------------------------------
//  dram_w -
//-------------------------------------------------

template <int Bank> void c1541_turbotrans_device::dram_w(offs_t offset, u8 data)
{
	if (!m_dram_enable || Bank >= m_dram_banks)
		return;

	m_dram[(Bank << 18) | (offset << 8) | m_counter] = data;
}


//-------------------------------------------------
//  nmi_w -
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c1541_turbotrans_device::nmi_w )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void c1541_turbotrans_device::parallel_data_w(u8 data)
{
	m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c1541_turbotrans_device::parallel_strobe_w(int state)
{
	m_via0->write_cb1(state);
}


//-------------------------------------------------
//  via0_pa_r -
//-------------------------------------------------

u8 c1541_turbotrans_device::via0_pa_r()
{
	return m_parallel_data;
}


//-------------------------------------------------
//  via0_pa_w -
//-------------------------------------------------

void c1541_turbotrans_device::via0_pa_w(u8 data)
{
	if (m_other)
		m_other->parallel_data_w(data);
}


//-------------------------------------------------
//  via0_ca2_w -
//-------------------------------------------------

void c1541_turbotrans_device::via0_ca2_w(int state)
{
	if (m_other)
		m_other->parallel_strobe_w(state);
}
