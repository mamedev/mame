// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

2017-11-05 Skeleton

Ampex Dialogue 80 terminal

Chips: CRT-5037, COM8017, SMC (COM)5016-5, MK3880N (Z80), SN74LS424N (TIM8224)
Crystals: 4.9152, 23.814
Other: Beeper, 5x 10sw-dips.

The program code seems to have been designed with a 8080 CPU in mind, using no
Z80-specific opcodes. This impression is reinforced by the IC types present on
the PCB, which go so far as to include the standard 8224 clock generator.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "machine/com8116.h"
#include "video/tms9927.h"
#include "screen.h"

#define CHAR_WIDTH 7

class ampex_state : public driver_device
{
public:
	ampex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_dbrg(*this, "dbrg")
		, m_p_chargen(*this, "chargen")
	{ }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(read_5840);
	DECLARE_WRITE8_MEMBER(write_5840);
	DECLARE_READ8_MEMBER(read_5841);
	DECLARE_WRITE8_MEMBER(write_5841);
	DECLARE_READ8_MEMBER(read_5842);
	DECLARE_WRITE8_MEMBER(write_5842);
	DECLARE_READ8_MEMBER(read_5843);
	DECLARE_WRITE8_MEMBER(write_5843);
	DECLARE_READ8_MEMBER(read_5846);
	DECLARE_READ8_MEMBER(read_5847);

	DECLARE_READ8_MEMBER(page_r);
	DECLARE_WRITE8_MEMBER(page_w);

	DECLARE_WRITE_LINE_MEMBER(vsyn_w);
	DECLARE_WRITE_LINE_MEMBER(dav_w);

	void ampex(machine_config &config);
	void mem_map(address_map &map);
private:
	virtual void machine_start() override;

	u8 m_page;
	u8 m_attr;
	bool m_attr_readback;
	std::unique_ptr<u16[]> m_paged_ram;

	required_device<cpu_device> m_maincpu;
	required_device<ay31015_device> m_uart;
	required_device<com8116_device> m_dbrg;
	required_region_ptr<u8> m_p_chargen;
};

u32 ampex_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ8_MEMBER(ampex_state::read_5840)
{
	logerror("%s: Read from 5840\n", machine().describe_context());
	return 0;
}

WRITE8_MEMBER(ampex_state::write_5840)
{
	m_page = (data & 0x30) >> 4;

	logerror("%s: Write %02X to 5840\n", machine().describe_context(), data);
}

READ8_MEMBER(ampex_state::read_5841)
{
	u8 result = m_uart->get_output_pin(AY31015_DAV) << 3;
	result |= m_uart->get_output_pin(AY31015_OR) << 4;
	result |= m_uart->get_output_pin(AY31015_PE) << 5;
	result |= m_uart->get_output_pin(AY31015_FE) << 6;
	return result;
}

WRITE8_MEMBER(ampex_state::write_5841)
{
	// bit 7 set for UART loopback test

	m_attr_readback = BIT(data, 5);
}

READ8_MEMBER(ampex_state::read_5842)
{
	//logerror("%s: Read from 5842\n", machine().describe_context());
	return 0;
}

WRITE8_MEMBER(ampex_state::write_5842)
{
	m_uart->set_transmit_data(data);
}

READ8_MEMBER(ampex_state::read_5843)
{
	m_uart->set_input_pin(AY31015_RDAV, 0);
	u8 data = m_uart->get_received_data();
	m_uart->set_input_pin(AY31015_RDAV, 1);
	return data;
}

WRITE8_MEMBER(ampex_state::write_5843)
{
	logerror("%s: Write %02X to 5843\n", machine().describe_context(), data);
	m_attr = (data & 0x78) >> 3;
}

READ8_MEMBER(ampex_state::read_5846)
{
	// probably acknowledges RST 6 interrupt (value not used)
	return 0;
}

READ8_MEMBER(ampex_state::read_5847)
{
	// acknowledges RST 4/5 interrupt (value not used)
	return 0;
}

READ8_MEMBER(ampex_state::page_r)
{
	if (m_attr_readback)
		return 0x87 | m_paged_ram[m_page * 0x1800 + offset] >> 5;
	else
		return 0xff & m_paged_ram[m_page * 0x1800 + offset];
}

WRITE8_MEMBER(ampex_state::page_w)
{
	m_paged_ram[m_page * 0x1800 + offset] = data | m_attr << 8;
}

WRITE_LINE_MEMBER(ampex_state::vsyn_w)
{
	// should generate RST 6 interrupt
}

WRITE_LINE_MEMBER(ampex_state::dav_w)
{
	// DAV should generate RST 7
}

ADDRESS_MAP_START(ampex_state::mem_map)
	AM_RANGE(0x0000, 0x2fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x4000, 0x43ff) AM_RAM // main RAM
	AM_RANGE(0x4400, 0x57ff) AM_RAM // expansion RAM
	AM_RANGE(0x5840, 0x5840) AM_READWRITE(read_5840, write_5840)
	AM_RANGE(0x5841, 0x5841) AM_READWRITE(read_5841, write_5841)
	AM_RANGE(0x5842, 0x5842) AM_READWRITE(read_5842, write_5842)
	AM_RANGE(0x5843, 0x5843) AM_READWRITE(read_5843, write_5843)
	AM_RANGE(0x5846, 0x5846) AM_READ(read_5846)
	AM_RANGE(0x5847, 0x5847) AM_READ(read_5847)
	AM_RANGE(0x5c00, 0x5c0f) AM_DEVREADWRITE("vtac", crt5037_device, read, write)
	AM_RANGE(0x8000, 0x97ff) AM_READWRITE(page_r, page_w)
	AM_RANGE(0xc000, 0xcfff) AM_RAM // video RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( ampex )
INPUT_PORTS_END

void ampex_state::machine_start()
{
	m_page = 0;
	m_attr = 0;
	m_attr_readback = false;
	m_paged_ram = std::make_unique<u16[]>(0x1800 * 4);

	m_uart->set_input_pin(AY31015_SWE, 0);

	save_item(NAME(m_page));
	save_item(NAME(m_attr));
	save_item(NAME(m_attr_readback));
	save_pointer(NAME(m_paged_ram.get()), 0x1800 * 4);
}

MACHINE_CONFIG_START(ampex_state::ampex)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(23'814'000) / 9) // clocked by 8224?
	MCFG_CPU_PROGRAM_MAP(mem_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(23'814'000) / 2, 105 * CHAR_WIDTH, 0, 80 * CHAR_WIDTH, 270, 0, 250)
	MCFG_SCREEN_UPDATE_DRIVER(ampex_state, screen_update)

	// FIXME: dot clock should be divided by char width
	MCFG_DEVICE_ADD("vtac", CRT5037, XTAL(23'814'000) / 2)
	MCFG_TMS9927_CHAR_WIDTH(CHAR_WIDTH)
	MCFG_TMS9927_VSYN_CALLBACK(WRITELINE(ampex_state, vsyn_w))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("uart", AY31015, 0) // COM8017, actually
	MCFG_AY31015_WRITE_DAV_CB(WRITELINE(ampex_state, dav_w))

	MCFG_DEVICE_ADD("dbrg", COM5016_5, XTAL(4'915'200))
MACHINE_CONFIG_END

ROM_START( dialog80 )
	ROM_REGION( 0x3000, "roms", 0 )
	ROM_LOAD( "3505240-01.u102", 0x0000, 0x0800, CRC(c5315780) SHA1(f2a8924f277d04bf4407f9b71b8d2788df0b1dc2) )
	ROM_LOAD( "3505240-02.u104", 0x0800, 0x0800, CRC(3fefa114) SHA1(d83c00605ae6c02d3aac7b572eb2bf615f0d4f3a) )
	ROM_LOAD( "3505240-03.u103", 0x1000, 0x0800, CRC(03abbcb2) SHA1(e5d382eefc3baff8f3e4d6b13219cb5eb1ca32f2) )
	ROM_LOAD( "3505240-04.u105", 0x1800, 0x0800, CRC(c051e15f) SHA1(16a066c39743ddf9a7da54bb8c03e2090d461862) )
	ROM_LOAD( "3505240-05.u100", 0x2000, 0x0800, CRC(6db6365b) SHA1(a68c83e554c2493645287e369749a07474723452) )
	ROM_LOAD( "3505240-06.u101", 0x2800, 0x0800, CRC(8f9a4969) SHA1(f9cd434f8d287c584cda429b45ca2537fdfb436b) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "3505240-07.u69",  0x0000, 0x0800, CRC(838a16cb) SHA1(4301324b9fe9453c2d277972f9464c4214c6793d) )

	ROM_REGION( 0x0200, "proms", 0 ) // unknown TI 16-pin DIPs
	ROM_LOAD( "417129-010.u16",  0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "417129-010.u87",  0x0100, 0x0100, NO_DUMP )
ROM_END

COMP( 1980, dialog80, 0, 0, ampex, ampex, ampex_state, 0, "Ampex", "Dialogue 80", MACHINE_IS_SKELETON )
