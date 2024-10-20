// license:BSD-3-Clause
// copyright-holders:

/*

This skeleton driver hosts 2 unidentified Chang Yu Electronic (CYE) games. The PCBs,
while sharing most components, differ, so they may very well go in different drivers when emulated.

unknown Chang Yu Electronic gambling game 1:

main components:

main PCB (FAN-21 sticker):
1 x R6502P
1 x 12 MHz XTAL
3 x 8-dip banks
1 x XILINK XC2064-33 (originally covered by a black box)
1 x HD46505RP-2
1 x AY-3-8910A
1 x UM5100

small sub PCB (HY-8902):
1 x D8751H
1 x 8 MHz XTAL
1 x TIBPAL16L8


unknown Chang Yu Electronic gambling game 2:

main components:

main PCB (marked 9101):
1 x R6502AP
1 x 12 MHz XTAL
1 x 3.579545 MHz XTAL (near UM3567)
3 x 8-dip banks (2x near AY8910)
1 x HD46505RP-2
1 x AY-3-8910A
1 x UM5100
1 x UM3567 (YM2413 clone)
1 x D8253C
1 x P87C51 MCU
*/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"
#include "sound/ymopl.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class changyu_state : public driver_device
{
public:
	changyu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_videoram(*this, "videoram")
	{
	}

	void changyu(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void common_map(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_bg_tilemap = nullptr;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void palette_init(palette_device &palette) const ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;

	void videoram_w(offs_t offset, u8 data);
};


class changyu2_state : public changyu_state
{
public:
	changyu2_state(const machine_config &mconfig, device_type type, const char *tag)
		: changyu_state(mconfig, type, tag)
		, m_mcu_response_latch(*this, "mcu_response")
	{
	}

	void changyu2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void main2_map(address_map &map) ATTR_COLD;
	void ext2_map(address_map &map) ATTR_COLD;

	void mcu_cmd_w(u8 data);
	void mcu_ctrl_w(u8 data);
	u8 mcu_status_r();
	u8 mcu_cmd_r();
	u8 mcu_p1_r();
	TIMER_CALLBACK_MEMBER(set_mcu_cmd);

	required_device<generic_latch_8_device> m_mcu_response_latch;

	u8 m_mcu_cmd = 0;
};

void changyu_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(changyu_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 72, 28);
}

void changyu2_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(changyu2_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}


void changyu_state::palette_init(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
	{
		int const r = color_prom[0x000 + i] & 0xf;
		int const g = color_prom[0x000 + i] >> 4;
		int const b = color_prom[0x100 + i] & 0xf;

		palette.set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

TILE_GET_INFO_MEMBER(changyu_state::get_bg_tile_info)
{
	int const attr  = m_videoram[tile_index | 0x800];
	int const code  = m_videoram[tile_index] | (attr & 0xf) << 8;
	int const color = attr >> 4;

	tileinfo.set(0, code, color, 0 );
}

uint32_t changyu_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void changyu_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void changyu2_state::mcu_cmd_w(u8 data)
{
	machine().scheduler().perfect_quantum(attotime::from_usec(50)); // enough time for the MCU to take the interrupt about to be triggered
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(changyu2_state::set_mcu_cmd), this), s32(u32(data)));
}

void changyu2_state::mcu_ctrl_w(u8 data)
{
	// other bits unknown
	m_mcu->set_input_line(MCS51_INT0_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
}

u8 changyu2_state::mcu_status_r()
{
	// other bits unknown
	return m_mcu_response_latch->pending_r() ? 0x01 : 0x00;
}

u8 changyu2_state::mcu_cmd_r()
{
	return m_mcu_cmd;
}

u8 changyu2_state::mcu_p1_r()
{
	return m_mcu_response_latch->pending_r() ? 0x7f : 0xff;
}

TIMER_CALLBACK_MEMBER(changyu2_state::set_mcu_cmd)
{
	m_mcu_cmd = u8(u32(param));
}


void changyu_state::common_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().w(FUNC(changyu_state::videoram_w)).share(m_videoram);
}

void changyu_state::main_map(address_map &map)
{
	common_map(map);
	map(0x0800, 0x0800).portr("IN0");
	map(0x0808, 0x0808).portr("IN1");
	map(0x0810, 0x0810).portr("IN2");
	map(0x0818, 0x0818).portr("IN3");

	map(0x0838, 0x0838).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x0839, 0x0839).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x1000, 0x1fff).ram().w(FUNC(changyu_state::videoram_w)).share("videoram");

	map(0x3000, 0x37ff).ram();

	map(0x8000, 0xffff).rom().region("boot_rom", 0x8000);
}

void changyu2_state::main2_map(address_map &map)
{
	common_map(map);

	map(0x2000, 0x2000).portr("IN0");
	map(0x2008, 0x2008).portr("IN1");
	map(0x2010, 0x2010).portr("IN2");
	map(0x2018, 0x2018).portr("IN3");

	map(0x2030, 0x2030).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x2031, 0x2031).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x2038, 0x2038).w(FUNC(changyu2_state::mcu_cmd_w));
	map(0x2800, 0x2801).w("ymsnd", FUNC(ym2413_device::write));
	map(0x2808, 0x2808).r(FUNC(changyu2_state::mcu_status_r));
	map(0x2020, 0x2020).r(m_mcu_response_latch, FUNC(generic_latch_8_device::read));
	map(0x3000, 0x3000).w(FUNC(changyu2_state::mcu_ctrl_w));

	map(0x6000, 0xffff).rom().region("boot_rom", 0x6000);
}

void changyu2_state::ext2_map(address_map &map)
{
	map(0x0300, 0x0300).r(FUNC(changyu2_state::mcu_cmd_r));
	map(0x0400, 0x0400).w(m_mcu_response_latch, FUNC(generic_latch_8_device::write));
	map(0x0502, 0x0503).w("ay", FUNC(ay8910_device::data_address_w));
}

static INPUT_PORTS_START( changyu )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // enables some kind of meters in-game
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) // resets on attract, no effect in gameplay?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_NAME("Clear Meters") // in bookkeeping menu
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Stand / Pass")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Bet All")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Hit")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Bet 3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Bet 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Bet 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )

	PORT_START("DSW0") // dips' listing available
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW0:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW0:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW0:8")

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END

// similar to above, with some shuffling.
static INPUT_PORTS_START( changyu2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Hit")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // freezes with no cards when soft reset if high
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_NAME("Clear Meters")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Stand / Pass")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Bet All")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Bet 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Bet 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Bet 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW0:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW0:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW0:8")

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END



void changyu_state::machine_start()
{
}

void changyu2_state::machine_start()
{
	changyu_state::machine_start();

	save_item(NAME(m_mcu_cmd));
}

static GFXDECODE_START( gfx_changyu )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_planar, 0, 16 )
GFXDECODE_END

void changyu_state::changyu(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(12'000'000) / 6); // R6502P, divisor not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &changyu_state::main_map);

	I8751(config, m_mcu, XTAL(8'000'000));
//  m_mcu->set_disable();

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(changyu_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(changyu_state::palette_init), 0x100);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_changyu);

	HD6845S(config, m_crtc, XTAL(12'000'000) / 8);  // HD46505RP-2, divisor not verified
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay", XTAL(12'000'000 / 6)).add_route(ALL_OUTPUTS, "mono", 0.9); // divisor not verified

	HC55516(config, "voice", XTAL(12'000'000 / 6)).add_route(ALL_OUTPUTS, "mono", 0.9); // UM5100 is a HC55536 with ROM hook-up, divisor not verified
}

void changyu2_state::changyu2(machine_config &config)
{
	changyu(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &changyu2_state::main2_map);

	auto &mcu(I87C51(config.replace(), m_mcu, XTAL(8'000'000)));
	mcu.set_addrmap(AS_IO, &changyu2_state::ext2_map);
	mcu.port_in_cb<0>().set(FUNC(changyu2_state::mcu_p1_r));

	GENERIC_LATCH_8(config, m_mcu_response_latch);

	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.9);
}

ROM_START( changyu )
	ROM_REGION(0x10000, "boot_rom", 0)
	ROM_LOAD( "23h.u29", 0x08000, 0x8000, CRC(df0a7417) SHA1(9be2be664ed688dc9d5a1803b7f4d9bc2a0b1fae) ) // 27C256

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "15a.sub", 0x00000, 0x1000, CRC(c7c24394) SHA1(e290f0b49f3bb536e1bc0cd06e04ed9e99f12e47) )

	ROM_REGION(0x20000, "gfx1", 0)
	ROM_LOAD( "5.u21",   0x00000, 0x08000, CRC(25b23b14) SHA1(cb42442c09475941ffcb9940d130f9b2188ce79e) ) // 27C256
	ROM_LOAD( "6.u20",   0x08000, 0x08000, CRC(35bcfdef) SHA1(5c4173ddf55a3bf4731d819be267f738dfe9fd29) ) // 27C256
	ROM_LOAD( "7.u19",   0x10000, 0x08000, CRC(ed69d69d) SHA1(adbcea3045bec61aef9ee2ee8425f429bf5e0fc8) ) // 27C256
	ROM_LOAD( "8.u42",   0x18000, 0x08000, CRC(4013b219) SHA1(735c64647595285fc0c79c617cd6833c473daa12) ) // 27C256

	ROM_REGION(0x58000, "unsorted", 0)
	ROM_LOAD( "1.u1",    0x00000, 0x08000, CRC(8d60cb76) SHA1(f33e14549ceb6511509be16dd1c238512e6ae758) ) // 27C256
	ROM_LOAD( "2.u2",    0x08000, 0x08000, CRC(b9d78664) SHA1(763876f075f2b5b07e96b36b4e670dc466808f08) ) // 27C256
	ROM_LOAD( "3.u3",    0x10000, 0x08000, CRC(17cc6716) SHA1(df8af0fbe93b8f92219721a35772ef93bca7adb5) ) // 27C256
	ROM_LOAD( "4.u4",    0x18000, 0x08000, CRC(31b76c13) SHA1(c46da02aff8f57c0277e493c82e01970c0acd4fb) ) // 27C256

	ROM_LOAD( "9a.u74",  0x20000, 0x10000, CRC(12f3fd7a) SHA1(e220694b8fa5cfc172bf23149fceaeeb6d0b6230) ) // 27C512
	ROM_LOAD( "10a.u61", 0x30000, 0x10000, CRC(8869968b) SHA1(fbab29436acde19d7d559160ef2394d43a6ebb87) ) // 27C512
	ROM_LOAD( "11a.u62", 0x40000, 0x10000, CRC(d05e6348) SHA1(5b8bd4c94631aed46cbf7cd4db749e4855d4516c) ) // 27C512

	ROM_LOAD( "14.u70",  0x50000, 0x08000, CRC(cdfdfe11) SHA1(b170f9a6e2c77ce3ae01aabc8a963a11eb7fe74e) ) // under the sub board and near an empty socket (u77), program for a removed second CPU? or for the first?

	// u9 and u63 not populated

	ROM_REGION(0x220, "proms", 0)
	ROM_LOAD( "63s281n.u48", 0x000, 0x100, CRC(eb75e89b) SHA1(d3d6843c2cb6fb94e39d51de92205863745efdc1) )
	ROM_LOAD( "63s281n.u49", 0x100, 0x100, CRC(137e2d9c) SHA1(4e498e4fb73cad869789b902fc74d31ee3aa259f) )
	ROM_LOAD( "82s123.u44",  0x200, 0x020, CRC(cbd7e5d4) SHA1(c7d96ee7f6fb0129630fdd4b079c4ed1eabda7c5) )

	ROM_REGION(0x104, "pals", 0)
	ROM_LOAD( "tibpal16l8-25cn.sub", 0x000, 0x104, NO_DUMP )
ROM_END

ROM_START( changyu2 ) // 999 ROM999 II BY HUANGYEH string
	ROM_REGION(0x10000, "boot_rom", 0)
	ROM_LOAD( "95.bin", 0x00000, 0x10000, CRC(c3a8061f) SHA1(8e2b2509de32b90c0ac5f3eabb8d256a1fbb393e) ) // 27C512

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "99c.bin", 0x00000, 0x1000, CRC(8d52dc7d) SHA1(84ae3d95696aec6a13401ea46f7b13410dc9c31b) ) // decapped

	ROM_REGION(0x20000, "gfx1", 0)
	ROM_LOAD( "91.bin",  0x00000, 0x08000, CRC(747c98e3) SHA1(30926ee500c6ee21b7e73424afc76f34d84cb896) ) // 27256
	ROM_LOAD( "92.bin",  0x08000, 0x08000, CRC(93ac967a) SHA1(41914ccdb6d07e7a74b4db3d5473f8949462ee1c) ) // 27256
	ROM_LOAD( "93.bin",  0x10000, 0x08000, CRC(1d2b75de) SHA1(89b201b75691ee6ac3fc71fb8a998dbf05a1b0b2) ) // 27256
	ROM_LOAD( "94.bin",  0x18000, 0x08000, CRC(f61a8410) SHA1(3c4df3e973322200aa72cf1d1df827c2ba69671b) ) // 27256

	ROM_REGION(0x30000, "unsorted", 0)
	ROM_LOAD( "96c.bin", 0x00000, 0x10000, CRC(06d11350) SHA1(3c65d1d71010a3f10b00c799ede2debc96f6f3cf) ) // 27C512
	ROM_LOAD( "97c.bin", 0x10000, 0x10000, CRC(e242ab79) SHA1(a7b14692556605eb039d1ef98fb3b8b007717c12) ) // 27C512
	ROM_LOAD( "98c.bin", 0x20000, 0x10000, CRC(c8879f76) SHA1(6bcc686720dc63f50509f3f003b1f62ff43fc6b1) ) // 27C512

	// Not provided in dump, seems to decode fine with changyu proms anyway
	ROM_REGION(0x220, "proms", 0)
	ROM_LOAD( "63s281n.u48", 0x000, 0x100, CRC(eb75e89b) SHA1(d3d6843c2cb6fb94e39d51de92205863745efdc1) )
	ROM_LOAD( "63s281n.u49", 0x100, 0x100, CRC(137e2d9c) SHA1(4e498e4fb73cad869789b902fc74d31ee3aa259f) )
	ROM_LOAD( "82s123.u44",  0x200, 0x020, CRC(cbd7e5d4) SHA1(c7d96ee7f6fb0129630fdd4b079c4ed1eabda7c5) )

	ROM_REGION(0x400, "pals", 0)
	ROM_LOAD( "9a", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "9b", 0x200, 0x104, NO_DUMP )
ROM_END

} // anonymous namespace


// No copyright for both, are these really bootlegs?
GAME( 1989, changyu,  0, changyu,  changyu, changyu_state,  empty_init, ROT0, "Chang Yu Electronic", "Mayo no 21", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Wing Co. in GFX1, year taken from start of maincpu ROM
GAME( 19??, changyu2, 0, changyu2, changyu2,changyu2_state, empty_init, ROT0, "Chang Yu Electronic", "999", MACHINE_NOT_WORKING ) // Wing Co. in GFX1
