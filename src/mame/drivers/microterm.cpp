// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Micro-Term terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/mc2661.h"
#include "machine/mc68681.h"
#include "video/scn2674.h"
#include "screen.h"

class microterm_state : public driver_device
{
public:
	microterm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	void mt420(machine_config &config);
	void mt5510(machine_config &config);

private:
	u32 mt5510_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(c000_r);
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void mt420_io_map(address_map &map);
	void mt420_mem_map(address_map &map);
	void mt420_vram_map(address_map &map);
	void mt5510_io_map(address_map &map);
	void mt5510_mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	optional_region_ptr<u8> m_p_chargen;
};

u32 microterm_state::mt5510_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ8_MEMBER(microterm_state::c000_r)
{
	return machine().rand() & 0x80;
}

void microterm_state::mt420_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x9000, 0x9000).nopw();
	map(0xc000, 0xc000).r(FUNC(microterm_state::c000_r)).nopw();
	map(0xe000, 0xefff).ram();
	map(0xeff8, 0xefff).rw("avdc", FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0xf000, 0xf7ff).ram();
}

void microterm_state::mt420_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xef).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xf0, 0xf3).rw("aci", FUNC(mc2661_device::read), FUNC(mc2661_device::write));
}

SCN2674_DRAW_CHARACTER_MEMBER(microterm_state::draw_character)
{
}

void microterm_state::mt420_vram_map(address_map &map)
{
	map(0x0000, 0x3fff).noprw();
}

void microterm_state::mt5510_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0).nopw();
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xffff).ram();
}

void microterm_state::mt5510_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x6f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
}

static INPUT_PORTS_START( microterm )
INPUT_PORTS_END

void microterm_state::mt420(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &microterm_state::mt420_mem_map);
	m_maincpu->set_addrmap(AS_IO, &microterm_state::mt420_io_map);

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL)); // MC2681
	duart.irq_cb().set_inputline(m_maincpu, 0);
	duart.outport_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(5);
	duart.outport_cb().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	duart.outport_cb().append("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(3);

	MC2661(config, "aci", 3.6864_MHz_XTAL); // SCN2641

	EEPROM_93C46_16BIT(config, "eeprom").do_callback().set("duart", FUNC(scn2681_device::ip6_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(9.87768_MHz_XTAL, 612, 0, 480, 269, 0, 250);
	//screen.set_raw(15.30072_MHz_XTAL, 948, 0, 792, 269, 0, 250);
	screen.set_screen_update("avdc", FUNC(scn2674_device::screen_update));

	scn2674_device &avdc(SCN2674(config, "avdc", 9.87768_MHz_XTAL / 6));
	//avdc.set_clock(15.30072_MHz_XTAL / 6);
	avdc.set_character_width(6);
	avdc.set_display_callback(FUNC(microterm_state::draw_character));
	avdc.set_addrmap(0, &microterm_state::mt420_vram_map);
	avdc.set_screen("screen");
}

void microterm_state::mt5510(machine_config &config)
{
	Z80(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &microterm_state::mt5510_mem_map);
	m_maincpu->set_addrmap(AS_IO, &microterm_state::mt5510_io_map);

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set_inputline(m_maincpu, 0);
	duart.outport_cb().set("eeprom1", FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	duart.outport_cb().append("eeprom2", FUNC(eeprom_serial_93cxx_device::di_write)).bit(5);
	duart.outport_cb().append("eeprom1", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	duart.outport_cb().append("eeprom2", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	duart.outport_cb().append("eeprom1", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(3);
	duart.outport_cb().append("eeprom2", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(3);

	EEPROM_93C46_16BIT(config, "eeprom1").do_callback().set("duart", FUNC(scn2681_device::ip6_w));

	EEPROM_93C46_16BIT(config, "eeprom2").do_callback().set("duart", FUNC(scn2681_device::ip5_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(45.8304_MHz_XTAL / 2, 1120, 0, 960, 341, 0, 300); // wild guess at resolution
	screen.set_screen_update(FUNC(microterm_state::mt5510_update));
}


/**************************************************************************************************************

Micro-Term Model 420.
Chips: Z80, MC2681P, SCN2674, 2x CDM6264E3, TMM2016BP-12, SCN2641, NMC9345N. Undumped PAL10L8NC at U18 and PROM (N82S129N) at U41.
Crystals: 3.6864, 15.30072 (hard to read), 9.87768

***************************************************************************************************************/

ROM_START( mt420 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "1910_m.p._r1.9.u8",   0x0000, 0x8000, CRC(e79154e9) SHA1(7c3f22097b931986c921bf731de98a1d0536aec9) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "mt420cg_rev2.1.u44",  0x0000, 0x0fe0, CRC(7950e485) SHA1(1f03525958464bbe861d2e78f07cc5264e17c0e8) ) // incomplete?
ROM_END



/**************************************************************************************************************

Micro-Term 5510.
Chips: Z80, SCN2681, S8842C4/SCX6244UNT, 4x CXK5864BP-70L, 2x NMC9346N
Crystals: 6.000, 3.68640, 45.8304

***************************************************************************************************************/

ROM_START( mt5510 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "2500_m.p._r1.9.u11", 0x00000, 0x10000, CRC(71f19a53) SHA1(91df26d46a93359cd033d7137f1676bcfa58223b) )
ROM_END




COMP( 1986, mt420,  0, 0, mt420,  microterm, microterm_state, empty_init, "Micro-Term", "Micro-Term 420", MACHINE_IS_SKELETON )
COMP( 1988, mt5510, 0, 0, mt5510, microterm, microterm_state, empty_init, "Micro-Term", "Micro-Term 5510", MACHINE_IS_SKELETON )
