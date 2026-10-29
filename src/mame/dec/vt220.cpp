// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

    DEC VT220

    30/06/2009 Skeleton driver.

    The VT220 exists in two major hardware revisions. The Pocket Service
    Guide has separate lists of components for earlier and later models,
    with the following being those that differ:

                                    A, B, and C     D, E, and F
        Terminal controller         70-20814-02     70-23363-01
        Power supply/monitor board  70-20624-02     70-23361-01
        Transformer                 70-20772-02     70-23362-02
        CRT/bezel/yoke (white)      70-20662-04     70-23360-01
                       (green)      70-20662-05     70-23360-02
                       (amber)      70-20662-06     70-23360-03
        Danish keyboard             LK201-AD        LK201-ED
        British keyboard            LK201-AE        LK201-EE
        Norwegian keyboard          LK201-AN        LK201-EN

    The original VT220 Technical Manual (EK-VT220-TM) covers Models A, B,
    and C only. Though it was later reprinted with an addendum for Models
    D, E, and F, this has not been found. The available schematics are
    inapplicable to later models, which have an altogether different memory
    map and, evidently, use some different IC types: an ER5911 rather than
    X2212 as non-volatile memory and, in place of the CRT9007, some sort
    of custom video gate array (which might even be clocked differently).

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/i8051.h"
//#include "machine/eepromser.h"
#include "machine/mc68681.h"
#include "machine/ram.h"
//#include "machine/x2212.h"
//#include "video/crt9007.h"
#include "emupal.h"
#include "screen.h"


namespace {

class vt220_state : public driver_device
{
public:
	vt220_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_chargen(*this, "chargen")
	{ }

	void vt220(machine_config &config);
	void vt220a(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<i8051_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_region_ptr<u8> m_chargen;
	u8 m_p1 = 0xff;
	u8 m_char_addr = 0;
	u8 m_conf = 0;
	std::unique_ptr<u8[]> m_softchar;
	void p1_w(u8 data);
	void conf_w(u8 data);
	void char_addr_w(u8 data);
	u8 chargen_r(offs_t offset);
	void chargen_w(offs_t offset, u8 data);
	void vt220_data(address_map &map) ATTR_COLD;
	void vt220_mem(address_map &map) ATTR_COLD;
	void vt220a_data(address_map &map) ATTR_COLD;
	void vt220a_mem(address_map &map) ATTR_COLD;
};


void vt220_state::vt220_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

void vt220_state::vt220a_mem(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void vt220_state::vt220_data(address_map &map)
{
	map.unmap_value_high();
	map(0x2000, 0x2fff).mirror(0xc000).ram();
	map(0x3800, 0x380f).mirror(0xc7f0).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
}

void vt220_state::vt220a_data(address_map &map)
{
	map.unmap_value_high();
	map(0x2000, 0x2fff).mirror(0xc000).ram().share("screen_ram"); // RAM 0/1 (A000-AFFF in the technical manual)
	map(0x3000, 0x300f).mirror(0xc7f0).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)); // B7F0-B7FF
	map(0x8000, 0x8fff).mirror(0x4000).ram().share("attr_ram"); // attribute RAM 0/1
	map(0x0800, 0x0800).mirror(0x47ff).w(FUNC(vt220_state::char_addr_w)); // 0FFF
	map(0x0000, 0x0000).mirror(0x47ff).w(FUNC(vt220_state::conf_w)); // 07FF
	map(0x9800, 0x980f).mirror(0x47f0).rw(FUNC(vt220_state::chargen_r), FUNC(vt220_state::chargen_w)); // 9FF0-9FFF
}

void vt220_state::p1_w(u8 data)
{
	m_p1 = data;
}

void vt220_state::conf_w(u8 data)
{
	// E26 (LS174) latches BAD0-5, bit order per firmware self-test (reverse of the sheet 2 Q labels):
	// 0 NORM FIELD, 1 LBA CLRS, 2 EN CHR GEN RW, 3 ALT SEL, 4 RESET CMM, 5 LBA CLKS
	// E32 F/F latches BAD6: set for ENA 20mA, reset for ENA EIA
	m_conf = data;
}

void vt220_state::char_addr_w(u8 data)
{
	m_char_addr = data;
}

u8 vt220_state::chargen_r(offs_t offset)
{
	// ALT SEL H selects the soft character RAM; otherwise CHRG SEL H (P1.5) is the ROM's MSB
	if (BIT(m_conf, 3))
		return m_softchar[((m_char_addr & 0x7f) << 4) | offset];
	return m_chargen[(BIT(m_p1, 5) << 12) | (m_char_addr << 4) | offset];
}

void vt220_state::chargen_w(offs_t offset, u8 data)
{
	if (BIT(m_conf, 3) && BIT(m_conf, 2))
		m_softchar[((m_char_addr & 0x7f) << 4) | offset] = data;
}

/* Input ports */
static INPUT_PORTS_START( vt220 )
INPUT_PORTS_END

void vt220_state::machine_start()
{
	m_softchar = make_unique_clear<u8[]>(0x800);

	save_item(NAME(m_p1));
	save_item(NAME(m_char_addr));
	save_item(NAME(m_conf));
	save_pointer(NAME(m_softchar), 0x800);
}

void vt220_state::machine_reset()
{
	memset(m_ram->pointer(),0,16*1024);
}

void vt220_state::video_start()
{
}

uint32_t vt220_state::screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void vt220_state::vt220(machine_config &config)
{
	/* basic machine hardware */
	I8051(config, m_maincpu, XTAL(11'059'200)); // from schematic for earlier version
	m_maincpu->set_addrmap(AS_PROGRAM, &vt220_state::vt220_mem);
	m_maincpu->set_addrmap(AS_DATA, &vt220_state::vt220_data);
	m_maincpu->port_in_cb<1>().set_constant(0xff); // P1.4 PROD TEST low only from factory fixture on J1-1
	m_maincpu->port_out_cb<1>().set(FUNC(vt220_state::p1_w));

	scn2681_device &duart(SCN2681(config, "duart", XTAL(3'686'400)));
	duart.irq_cb().set_inputline("maincpu", MCS51_INT1_LINE);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(vt220_state::screen_update_vt220));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("16K");
}

void vt220_state::vt220a(machine_config &config)
{
	vt220(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt220_state::vt220a_mem);
	m_maincpu->set_addrmap(AS_DATA, &vt220_state::vt220a_data);
}

/* ROM definitions */
ROM_START(vt220)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("23-178e6.bin", 0x0000, 0x8000, CRC(cce5088c) SHA1(4638304729d1213658a96bb22c5211322b74d8fc) )

	ROM_REGION(0x4000, "chargen", ROMREGION_ERASEFF)
	ROM_LOAD("23-348e4.e13", 0x0000, 0x2000, CRC(994f3e37) SHA1(fe72a9fe9adb3a24743a6288d88ae07570cfea9a) )
ROM_END

ROM_START(vt220a)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
#if 0
	// these ROMs are listed in the schematics; it is unknown whether or not they were actually shipped
	ROM_LOAD("23-012e5.e3", 0x0000, 0x4000, NO_DUMP) // location e3 with jumper w7 present and w6 cut
	ROM_RELOAD(0x4000, 0x4000) // the rom also maps to 4000-7fff as a14 is unconnected
	ROM_LOAD("23-248e4.e4", 0x8000, 0x2000, NO_DUMP)
	// a000-bfff may be open bus or may be a mirror of above depending on whether the rom uses PGM/A13 as a secondary enable or not
	ROM_RELOAD(0xc000, 0x2000)
	// e000-ffff may be open bus or may be a mirror of above depending on whether the rom uses PGM/A13 as a secondary enable or not
#endif
	ROM_LOAD("23-183e5.e3", 0x8000, 0x4000, CRC(2848db40) SHA1(b3b029ef964c86ede68ad1b2854cfad766f20af0))
	ROM_RELOAD(0xc000, 0x4000)
	ROM_LOAD("23-182e5.e4", 0x0000, 0x4000, CRC(c759bf9f) SHA1(6fe21e8eb9576fbcda76d7909b07859db0793c4e))
	ROM_RELOAD(0x4000, 0x4000)
	ROM_LOAD("23-011m1.e1", 0x0000, 0x1000, CRC(6c4930a9) SHA1(1200a5dbf431017a11a51b5c4c9b4e7952b0a2bb)) // 8051 internal code

	ROM_REGION(0x4000, "chargen", ROMREGION_ERASEFF)
#if 0
	// this ROM is listed in the schematics
	ROM_LOAD("23-247e4.e13", 0x0000, 0x2000, NO_DUMP)
#endif
	ROM_LOAD("23-348e4.e13", 0x0000, 0x2000, CRC(994f3e37) SHA1(fe72a9fe9adb3a24743a6288d88ae07570cfea9a)) // this can maybe be read as well as a read/writable ram for custom characters which lives ?above? it in chargen address space by setting a bit in a config register. I haven't figured out where in 8051 address space it appears when readable nor where the ram appears.
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE        INIT        COMPANY                          FULLNAME               FLAGS */
COMP( 1983, vt220,  0,      0,      vt220,   vt220, vt220_state, empty_init, "Digital Equipment Corporation", "VT220 (Version 2.3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 1983, vt220a, vt220,  0,      vt220a,  vt220, vt220_state, empty_init, "Digital Equipment Corporation", "VT220 (Version 2.1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
