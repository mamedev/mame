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
#include "machine/x2212.h"
#include "video/crt9007.h"
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
		m_crtc(*this, "crtc"),
		m_screen(*this, "screen"),
		m_duart(*this, "duart"),
		m_nvr(*this, "nvr"),
		m_chargen(*this, "chargen"),
		m_screen_ram(*this, "screen_ram"),
		m_attr_ram(*this, "attr_ram")
	{ }

	void vt220(machine_config &config);
	void vt220a(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_vt220a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vt220a_palette(palette_device &palette) const;
	required_device<i8051_device> m_maincpu;
	required_device<ram_device> m_ram;
	optional_device<crt9007_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<scn2681_device> m_duart;
	optional_device<x2212_device> m_nvr;
	required_region_ptr<u8> m_chargen;
	optional_shared_ptr<u8> m_screen_ram;
	optional_shared_ptr<u8> m_attr_ram;
	u8 m_p1 = 0xff;
	u8 m_char_addr = 0;
	u8 m_atr_latch = 0;
	u8 m_duart_op = 0xff;
	u8 m_video_lba = 0;
	int m_row_y = 0;
	bool m_wben = false;
	bool m_cblank = true;
	u8 m_conf = 0;
	u8 m_lba = 0;
	u8 m_lbf_bank = 0;
	bool m_132col = false;
	std::unique_ptr<u8[]> m_softchar;
	std::unique_ptr<u8[]> m_lbf;
	void p1_w(u8 data);
	void conf_w(u8 data);
	void duart_out_w(u8 data);
	void set_columns();
	void char_addr_w(u8 data);
	offs_t lbf_addr(bool chr, u8 lba) const;
	u8 char_code() const;
	u8 dot(int col, int dot, int line, bool &bold, bool &blink) const;
	u8 duart_r(offs_t offset);
	u8 vram_r(offs_t offset);
	void wben_w(int state);
	void cblank_w(int state);
	u8 chargen_r(offs_t offset);
	void chargen_w(offs_t offset, u8 data);
	void vt220_data(address_map &map) ATTR_COLD;
	void vt220_mem(address_map &map) ATTR_COLD;
	void vt220a_data(address_map &map) ATTR_COLD;
	void vt220a_mem(address_map &map) ATTR_COLD;
	void vt220a_vram(address_map &map) ATTR_COLD;
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
	map(0x3000, 0x300f).mirror(0xc7f0).r(FUNC(vt220_state::duart_r)).w(m_duart, FUNC(scn2681_device::write)); // B7F0-B7FF
	map(0x8000, 0x8fff).mirror(0x4000).ram().share("attr_ram"); // attribute RAM 0/1
	map(0x0800, 0x0800).mirror(0x47ff).w(FUNC(vt220_state::char_addr_w)); // 0FFF
	map(0x0000, 0x0000).mirror(0x47ff).w(FUNC(vt220_state::conf_w)); // 07FF
	map(0x1000, 0x103f).mirror(0x47c0).rw(m_crtc, FUNC(crt9007_device::read), FUNC(crt9007_device::write)); // 1700-17FF
	map(0x9000, 0x90ff).mirror(0x4700).rw(m_nvr, FUNC(x2212_device::read), FUNC(x2212_device::write)); // 9700-97FF
	map(0x9800, 0x980f).mirror(0x47f0).rw(FUNC(vt220_state::chargen_r), FUNC(vt220_state::chargen_w)); // 9FF0-9FFF
}

offs_t vt220_state::lbf_addr(bool chr, u8 lba) const
{
	// E14 (2016): A10/A9 BNKS1/BNKS0, A8 ATR/CHR (high for character), A7-A0 LBA
	return (m_lbf_bank << 9) | (chr ? 0x100 : 0) | lba;
}

u8 vt220_state::char_code() const
{
	// CHRG LATCH2 (E20) takes the character address latch (E19) while LBF DISABLE H (P1.3) is high,
	// otherwise the line buffer output
	return BIT(m_p1, 3) ? m_char_addr : m_lbf[lbf_addr(true, m_lba)];
}

void vt220_state::vt220a_vram(address_map &map)
{
	// CRT9007 VA0-VA13 drive the CPU address bus during DMA
	map(0x0000, 0x3fff).r(FUNC(vt220_state::vram_r));
}

u8 vt220_state::vram_r(offs_t offset)
{
	// screen and attribute RAM see the same address; row data (not the row table) goes to the line buffer
	const u8 chr = m_screen_ram[offset & 0x0fff];
	if (m_wben && !BIT(m_conf, 2) && !BIT(m_p1, 3))
	{
		m_lbf[lbf_addr(true, m_video_lba)] = chr;
		m_lbf[lbf_addr(false, m_video_lba)] = m_attr_ram[offset & 0x0fff];
		m_video_lba++;
	}
	return chr;
}

void vt220_state::wben_w(int state)
{
	if (state && !m_wben)
	{
		// the row loaded now is displayed from the next scan line
		m_screen->update_partial(m_screen->vpos());
		m_video_lba = 0;
		m_row_y = m_screen->vpos() + 1;
	}
	m_wben = state;
}

void vt220_state::cblank_w(int state)
{
	m_cblank = state;
}

u8 vt220_state::dot(int col, int x, int line, bool &bold, bool &blink) const
{
	// one dot of the video stream: E30 gives the CPU the line buffer address and ALT SEL while
	// EN CHR GEN RW H is set, otherwise the video side walks the row
	const bool cpu = BIT(m_conf, 2);
	const u8 lba = cpu ? m_lba : u8(col);
	const u8 chr = BIT(m_p1, 3) ? m_char_addr : m_lbf[lbf_addr(true, lba)];
	const u8 atr = BIT(m_p1, 3) ? m_atr_latch : m_lbf[lbf_addr(false, lba)];
	const bool alt = cpu ? BIT(m_conf, 3) : BIT(atr, 3);

	// AD7 BLINK (E38 NAND with DUART BLINK H), AD6 BOLD, AD5 REV (low for reverse), AD4 UL, AD3 ALT SEL
	bold = BIT(atr, 6);
	blink = !(BIT(m_duart_op, 3) && BIT(atr, 7));

	if (line >= 10)
		return 0;
	const u8 bits = alt ? m_softchar[((chr & 0x7f) << 4) | line] : m_chargen[(BIT(m_p1, 5) << 12) | (chr << 4) | line];

	// MSB first; each lit dot is stretched into the next (TM 6.2.4), so a lit dot 7 lights the first fill dot
	bool on = (x < 8 && BIT(bits, 7 - x)) || (x > 0 && x <= 8 && BIT(bits, 8 - x));
	if (BIT(atr, 4) && line >= 8) // UL ATR gated with SC3: scan lines 9 and 10
		on = true;
	// reverse: NORM FIELD H XOR REV ATR (E46)
	return on ^ BIT(m_conf, 0) ^ !BIT(atr, 5);
}

u8 vt220_state::duart_r(offs_t offset)
{
	if (offset == 0x0d && !machine().side_effects_disabled())
	{
		// self-test status taps on the video output (sheet 5): TBLK on IP4 is low while blanked
		// (CBLANK or DIS VIDEO), TVB H on IP3 is E45 NOR of the video and the two intensity lines
		const int x = m_screen->hpos(), y = m_screen->vpos();
		const int width = m_132col ? 9 : 10;
		const bool blank = m_cblank || BIT(m_duart_op, 7); // OP7 DIS VIDEO H
		bool bold = false, blink = false, video = false;
		if (!blank)
			video = dot(x / width, x % width, y - m_row_y, bold, blink);
		m_duart->ip3_w(!(video || bold || blink));
		m_duart->ip4_w(!blank);
	}
	return m_duart->read(offset);
}

void vt220_state::p1_w(u8 data)
{
	// DMA L (P1.0) falling edge loads the character address latch into the line buffer
	if (BIT(m_p1, 0) && !BIT(data, 0) && !BIT(data, 3))
	{
		m_lbf[lbf_addr(true, m_lba)] = m_char_addr;
		m_lbf[lbf_addr(false, m_lba)] = m_atr_latch;
	}
	// NVR RECALL L (P1.1) and NVR STORE L (P1.2); the X2212 core takes them active high
	if (m_nvr)
	{
		m_nvr->recall(!BIT(data, 1));
		m_nvr->store(!BIT(data, 2));
	}
	// RESET CRT L (P1.6)
	if (m_crtc && BIT(m_p1, 6) && !BIT(data, 6))
		m_crtc->reset();
	m_p1 = data;
}

void vt220_state::conf_w(u8 data)
{
	// E26 (LS174) latches BAD0-5, bit order per firmware self-test (reverse of the sheet 2 Q labels):
	// 0 NORM FIELD, 1 LBA CLRS, 2 EN CHR GEN RW, 3 ALT SEL, 4 RESET CMM, 5 LBA CLKS
	// E32 F/F latches BAD6: set for ENA 20mA, reset for ENA EIA
	// with EN CHR GEN RW H set, E30 gives the CPU the line buffer address counter (E7, LS393)
	if (BIT(data, 2))
	{
		if (BIT(data, 1))
			m_lba = 0;
		else if (BIT(m_conf, 5) && !BIT(data, 5))
			m_lba++;
	}
	if (m_crtc)
		m_screen->update_partial(m_screen->vpos());
	m_conf = data;
}

void vt220_state::duart_out_w(u8 data)
{
	if (m_crtc)
		m_screen->update_partial(m_screen->vpos());
	m_duart_op = data;
	m_lbf_bank = (data >> 5) & 3; // OP5/OP6 BNK S0/S1
	if (m_crtc && BIT(data, 4) != m_132col)
	{
		m_132col = BIT(data, 4); // OP4 132 COL H
		set_columns();
	}
}

void vt220_state::set_columns()
{
	// dot clock 22.896 MHz (Y4) at 9 dots per character for 132 columns, 15.288 MHz (Y3) at 10 for 80;
	// horizontal totals are as the firmware programs the CRT9007
	const XTAL dot = m_132col ? 22.896_MHz_XTAL : 15.288_MHz_XTAL;
	const int width = m_132col ? 9 : 10;
	const int htotal = (m_132col ? 164 : 99) * width;
	const int hvis = (m_132col ? 132 : 80) * width;
	m_screen->configure(htotal, 260, rectangle(0, hvis - 1, 0, 240 - 1), attotime::from_ticks(htotal * 260, dot));
	m_crtc->set_unscaled_clock(dot / width);
	m_crtc->set_character_width(width);
}

void vt220_state::char_addr_w(u8 data)
{
	// E19 latches the character address from BAD0-7 and E21 the attribute from AD3-7
	m_char_addr = data;
	m_atr_latch = data & 0xf8;
}

u8 vt220_state::chargen_r(offs_t offset)
{
	// ALT SEL H selects the soft character RAM; otherwise CHRG SEL H (P1.5) is the ROM's MSB
	if (BIT(m_conf, 3))
		return m_softchar[((char_code() & 0x7f) << 4) | offset];
	return m_chargen[(BIT(m_p1, 5) << 12) | (char_code() << 4) | offset];
}

void vt220_state::chargen_w(offs_t offset, u8 data)
{
	if (BIT(m_conf, 3) && BIT(m_conf, 2))
		m_softchar[((char_code() & 0x7f) << 4) | offset] = data;
}

/* Input ports */
static INPUT_PORTS_START( vt220 )
INPUT_PORTS_END

void vt220_state::machine_start()
{
	m_softchar = make_unique_clear<u8[]>(0x800);
	m_lbf = make_unique_clear<u8[]>(0x800);

	save_item(NAME(m_p1));
	save_item(NAME(m_char_addr));
	save_item(NAME(m_conf));
	save_item(NAME(m_lba));
	save_item(NAME(m_lbf_bank));
	save_item(NAME(m_132col));
	save_item(NAME(m_atr_latch));
	save_item(NAME(m_duart_op));
	save_item(NAME(m_video_lba));
	save_item(NAME(m_row_y));
	save_item(NAME(m_wben));
	save_item(NAME(m_cblank));
	if (m_crtc)
		machine().save().register_postload(save_prepost_delegate(FUNC(vt220_state::set_columns), this));
	save_pointer(NAME(m_softchar), 0x800);
	save_pointer(NAME(m_lbf), 0x800);
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

uint32_t vt220_state::screen_update_vt220a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int width = m_132col ? 9 : 10;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 *const pix = &bitmap.pix(y);
		const int line = y - m_row_y;
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bool bold, blink;
			if (line < 0 || BIT(m_duart_op, 7) || !dot(x / width, x % width, line, bold, blink))
				pix[x] = 0;
			else
				pix[x] = 1 + blink + (bold << 1);
		}
	}
	return 0;
}

void vt220_state::vt220a_palette(palette_device &palette) const
{
	// E49 open-collector gates into R17 1.8K (video), R15 2.7K and R16 2K (blink and bold lines;
	// which is which is unverified); intensity taken as the summed conductance
	const double g[4] = { 1 / 1.8, 1 / 1.8 + 1 / 2.7, 1 / 1.8 + 1 / 2.0, 1 / 1.8 + 1 / 2.7 + 1 / 2.0 };
	palette.set_pen_color(0, rgb_t::black());
	for (int i = 0; i < 4; i++)
	{
		const u8 v = u8(255 * g[i] / g[3]);
		palette.set_pen_color(1 + i, rgb_t(v, v, v));
	}
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
	duart.outport_cb().set(FUNC(vt220_state::duart_out_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, m_screen));
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

	// T5 character clock: 15.288 MHz (Y3) / 10 dots at 80 columns; 22.896 MHz (Y4) / 9 at 132 not yet switched
	CRT9007(config, m_crtc, 15.288_MHz_XTAL / 10);
	m_crtc->set_addrmap(0, &vt220_state::vt220a_vram);
	m_crtc->set_character_width(10);
	m_crtc->set_screen(m_screen);
	m_screen->set_raw(15.288_MHz_XTAL, 990, 0, 800, 260, 0, 240);
	m_crtc->vs_callback().set_inputline(m_maincpu, MCS51_T0_LINE);
	m_crtc->hs_callback().set_inputline(m_maincpu, MCS51_T1_LINE);
	m_crtc->drb_callback().set_inputline(m_maincpu, MCS51_INT0_LINE).invert(); // DRB is active low, as is DMA REQ L
	X2212(config, m_nvr); // E17

	m_crtc->wben_callback().set(FUNC(vt220_state::wben_w));
	m_crtc->cblank_callback().set(FUNC(vt220_state::cblank_w));
	m_screen->set_screen_update(FUNC(vt220_state::screen_update_vt220a));
	m_screen->set_palette("palette_a");
	PALETTE(config, "palette_a", FUNC(vt220_state::vt220a_palette), 5);
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
