// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for TeleVideo 965 video display terminal.

TeleVideo 9320 appears to run on similar hardware with a 2681 DUART replacing the ACIAs.

************************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/g65816/g65816.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class tv965_state : public driver_device
{
public:
	tv965_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_char_view(*this, "char-view")
		, m_charram(*this, "charram", 0x8000, ENDIANNESS_LITTLE)
		, m_attrram(*this, "attrram", 0x8000, ENDIANNESS_LITTLE)
		, m_chargen(*this, "chargen", 0x2000, ENDIANNESS_LITTLE)
		, m_bcharram(*this, "bcharram")
		, m_battrram(*this, "battrram")
	{ }

	void tv965(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<g65816_device> m_maincpu;
	required_device<scn2672_device> m_crtc;
	required_device<screen_device> m_screen;
	memory_view m_char_view;
	memory_share_creator<u8> m_charram;
	memory_share_creator<u8> m_attrram;
	memory_share_creator<u8> m_chargen;
	required_memory_bank m_bcharram;
	required_memory_bank m_battrram;

	u8 m_c1, m_c2, m_c3, m_char_latch, m_char_offset;

	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	u8 ga_hack_r();
	void c1_w(u8 data);
	void c2_w(u8 data);
	void c3_w(u8 data);
	void char_latch_w(u8 data);
	void char_ram_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
};

SCN2672_DRAW_CHARACTER_MEMBER(tv965_state::draw_character)
{
	u32 off = BIT(m_c1, 3) ? 0x4000 : 0;
	u8 ch = m_charram[(address & 0x3fff) | off];
	u8 at = m_attrram[(address & 0x3fff) | off];
	u8 v = m_chargen[(BIT(m_c1, 4) ? 0x1000 : 0) | (ch << 4) | linecount];
	if(at != 0)
		v = ~v;
	u32 *dest = &bitmap.pix(y, x);
	*dest++ = 0x000000;
	for(int xx=0; xx != 8; xx++)
		*dest++ = BIT(v, 7-xx) ? 0xffffff : 0x000000;
	*dest++ = 0;
	(void)at;
}

void tv965_state::machine_start()
{
	save_item(NAME(m_c1));
	save_item(NAME(m_c2));
	save_item(NAME(m_c3));
	save_item(NAME(m_char_latch));
	save_item(NAME(m_char_offset));
	m_bcharram->configure_entries(0, 2, m_charram, 0x4000);
	m_battrram->configure_entries(0, 2, m_attrram, 0x4000);
}

void tv965_state::machine_reset()
{
	m_c1 = 0;
	m_c2 = 0;
	m_c3 = 0;
	m_char_latch = 0;
	m_char_offset = 0;
	m_bcharram->set_entry(0);
	m_battrram->set_entry(0);
	m_char_view.select(0);
}

u8 tv965_state::ga_hack_r()
{
	return 0x08;
}

void tv965_state::c1_w(u8 data)
{
	m_c1 = data;
	m_bcharram->set_entry(BIT(m_c1, 3));
	m_battrram->set_entry(BIT(m_c1, 3));
	m_char_view.select(BIT(m_c1, 4) ? 1 : 0);
	logerror("c1 %s %s cgb=%d%s bank=%d protect=%cul%crv%chi\n",
			 BIT(m_c1, 7) ? "132" : "80",
			 BIT(m_c1, 6) ? "light" : "dark",
			 BIT(m_c1, 5) ? 512 : 0,
			 BIT(m_c1, 4) ? " fontload" : "",
			 BIT(m_c1, 3) ? 1 : 0,
			 BIT(m_c1, 2) ? '+' : '-',
			 BIT(m_c1, 1) ? '+' : '-',
			 BIT(m_c1, 0) ? '+' : '-');
}

void tv965_state::c2_w(u8 data)
{
	m_c2 = data;

	logerror("c2%s %s %s %s%s%s attr=%d\n",
			 BIT(m_c2, 7) ? " econ" : "",
			 BIT(m_c2, 6) ? "line" : "page",
			 BIT(m_c2, 5) ? "dec" : "tvi",
			 BIT(m_c2, 4) ? "emb" : "nemb",
			 BIT(m_c2, 3) ? " 920" : "",
			 BIT(m_c2, 2) ? " hi" : "",
			 BIT(m_c2, 0, 2));
}

void tv965_state::c3_w(u8 data)
{
	m_c3 = data;
	logerror("c3 br=%d%s%s%s smode=%s speaker=%s%s\n",
			 BIT(m_c3, 6, 2),
			 BIT(m_c3, 5) ? " hsync-delay" : "",
			 BIT(m_c3, 4) ? " blink" : "",
			 BIT(m_c3, 3) ? " txirq" : "",
			 BIT(m_c3, 2) ? "43+" : "42-",
			 BIT(m_c3, 1) ? "on" : "off",
			 BIT(m_c3, 0) ? " dtr" : "");
}

void tv965_state::char_latch_w(u8 data)
{
	m_char_latch = data;
	m_char_offset = 0;
}

void tv965_state::char_ram_w(u8 data)
{
	m_chargen[(BIT(m_c1, 5) ? 0x1000 : 0x0000) | (m_char_latch << 4) | m_char_offset] = data;
	m_char_offset = (m_char_offset + 1) & 0xf;
}


void tv965_state::mem_map(address_map &map)
{
	map(0x00000, 0x01fff).ram().share("nvram");
	map(0x02000, 0x02007).rw("crtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x04000, 0x04000).r(FUNC(tv965_state::ga_hack_r));
	map(0x04002, 0x04002).w(FUNC(tv965_state::c1_w));
	map(0x04003, 0x04003).w(FUNC(tv965_state::c2_w));
	map(0x04004, 0x04004).w(FUNC(tv965_state::c3_w));
	map(0x04006, 0x04006).w(FUNC(tv965_state::char_ram_w));
	map(0x06200, 0x06203).rw("acia1", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x06400, 0x06403).rw("acia2", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x08000, 0x0bfff).view(m_char_view);
	m_char_view[0](0x08000, 0x0bfff).bankrw(m_bcharram);
	m_char_view[1](0x08000, 0x0bfff).w(FUNC(tv965_state::char_latch_w));
	map(0x0c000, 0x0ffff).bankrw(m_battrram);
	map(0x10000, 0x1ffff).rom().region("eprom1", 0);
	map(0x30000, 0x37fff).rom().region("eprom2", 0).mirror(0x8000);
}

void tv965_state::program_map(address_map &map)
{
	map.global_mask(0x2ffff);
	map(0x00000, 0x0ffff).rom().region("eprom1", 0);
	map(0x20000, 0x27fff).rom().region("eprom2", 0).mirror(0x8000);
}

static INPUT_PORTS_START( tv965 )
INPUT_PORTS_END

void tv965_state::tv965(machine_config &config)
{
	G65816(config, m_maincpu, 44.4528_MHz_XTAL / 10);
	m_maincpu->set_addrmap(AS_DATA, &tv965_state::mem_map);
	m_maincpu->set_addrmap(AS_PROGRAM, &tv965_state::program_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CXK5864BP-10L + battery

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	//  m_screen->set_raw(26.9892_MHz_XTAL, 1020, 0, 800, 441, 0, 416);
	m_screen->set_raw(44.4528_MHz_XTAL, 1680, 0, 1320, 441, 0, 416);
	m_screen->set_screen_update("crtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_crtc, 44.4528_MHz_XTAL / 10);
	m_crtc->set_character_width(10);
	m_crtc->set_display_callback(FUNC(tv965_state::draw_character));
	m_crtc->intr_callback().set_inputline(m_maincpu, G65816_LINE_NMI);
	m_crtc->set_screen(m_screen);

	mos6551_device &acia1(MOS6551(config, "acia1", 0));
	acia1.set_xtal(3.6864_MHz_XTAL / 2); // divider not verified, possibly even programmable

	mos6551_device &acia2(MOS6551(config, "acia2", 0));
	acia2.set_xtal(3.6864_MHz_XTAL / 2); // divider not verified, possibly even programmable
}

/**************************************************************************************************************

Televideo TVI-965 (P/N 132970-00)
Chips: G65SC816P-5, SCN2672TC5N40, Silicon Logic 271582-00, 2x UM6551A, Beeper, 2x MK48H64LN-70, HY6264LP-10 (next to gate array), CXK5864BP-10L, DS1231, round battery
Crystals: 44.4528, 26.9892, 3.6864

***************************************************************************************************************/

ROM_START( tv965 )
	ROM_REGION(0x10000, "eprom1", 0)
	ROM_LOAD( "180003-30h.u8", 0x00000, 0x10000, CRC(c7b9ca39) SHA1(1d95a8b0a4ea5caf3fb628c44c7a3567700a0b59) )

	ROM_REGION(0x08000, "eprom2", ROMREGION_ERASE00)
	ROM_LOAD( "180003-38h.u9", 0x00000, 0x08000, CRC(30fae408) SHA1(f05bb2a9ce2df60b046733f746d8d8a1eb3ac8bc) )
ROM_END

} // anonymous namespace


COMP( 1989, tv965, 0, 0, tv965, tv965, tv965_state, empty_init, "TeleVideo Systems", "TeleVideo 965", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
