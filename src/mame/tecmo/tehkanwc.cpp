// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca
/***************************************************************************

Tehkan World Cup - (c) Tehkan 1985

Ernesto Corvi
ernesto@imagina.com

Roberto Juan Fresca
robbiex@rocketmail.com

TODO:
- dip switches and input ports for Gridiron and Tee'd Off
- Check MEMORY_* definitions (too many M?A_NOP areas)
- Check sound in all games (too many messages like this in the .log file :
  'Warning: sound latch 2 written before being read')
- Figure out the controls in 'tehkanwc' (they are told to be better in MAME 0.34)
- Figure out the controls in 'teedoff'
- Confirm "Difficulty" Dip Switch in 'teedoff'
- What does the Z80 I register do here? (HW can latch it during refresh cycles.)


Additional notes (Steph 2002.01.14)

Even if there is NO "screen flipping" for 'tehkanwc' and 'gridiron', there are writes
to 0xfe60 and 0xfe70 of the main CPU with 00 ...

About 'teedoff' :

The main problem with that game is that it should sometimes jumps into shared memory
(see 'init_teedoff' function below) depending on a value that is supposed to be
in the palette RAM ! (maybe palette RAM is write only, and this is an open bus read
or was used for debugging during the game development?)

Palette RAM is reset here (main CPU) :

5D15: ED 57       ld   a,i
5D17: CB FF       set  7,a
5D19: ED 47       ld   i,a
5D1B: AF          xor  a
5D1C: 21 00 D8    ld   hl,$D800
5D1F: 01 80 0C    ld   bc,$0C80
5D22: 77          ld   (hl),a
5D23: 23          inc  hl
5D24: 0D          dec  c
5D25: 20 FB       jr   nz,$5D22
5D27: 0E 80       ld   c,$80
5D29: 10 F7       djnz $5D22
....

Then it is filled here (main CPU) :

5D50: 21 C4 70    ld   hl,$70C4
5D53: 11 00 D8    ld   de,$D800
5D56: 01 00 02    ld   bc,$0200
5D59: ED B0       ldir
5D5B: 21 C4 72    ld   hl,$72C4
5D5E: 01 00 01    ld   bc,$0100
5D61: ED B0       ldir
5D63: C9          ret

0x72c4 is in ROM and it's ALWAYS 00 !

Another place where the palette is filled is here (sub CPU) :

16AC: 21 06 1D    ld   hl,$1D06
16AF: 11 00 DA    ld   de,$DA00
16B2: 01 C0 00    ld   bc,$00C0
16B5: ED B0       ldir

But here again, 0x1d06 is in ROM and it's ALWAYS 00 !

So the "jp z" instruction at 0x0238 of the main CPU will ALWAYS jump
in shared memory when NO code seems to be written !

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "gridiron.lh"

namespace {

class tehkanwc_state : public driver_device
{
public:
	tehkanwc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_adpcm_rom(*this, "adpcm"),
		m_track_p1(*this, "P1%c", 'X'),
		m_track_p2(*this, "P2%c", 'X'),
		m_digits(*this, "digit%u", 0U)
	{ }

	void tehkanwcb(machine_config &config);
	void tehkanwc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_adpcm_rom;

	required_ioport_array<2> m_track_p1;
	required_ioport_array<2> m_track_p2;
	output_finder<2> m_digits;

	int m_track_p1_reset[2]{};
	int m_track_p2_reset[2]{};
	int m_msm_data_offs = 0;
	int m_toggle = 0;
	uint8_t m_scroll_x[2]{};
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void sub_cpu_reset_w(uint8_t data);
	void sound_cpu_reset_w(uint8_t data);
	uint8_t track_0_r(offs_t offset);
	uint8_t track_1_r(offs_t offset);
	void track_0_reset_w(offs_t offset, uint8_t data);
	void track_1_reset_w(offs_t offset, uint8_t data);
	void sound_command_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void videoram2_w(offs_t offset, uint8_t data);
	void scroll_x_w(offs_t offset, uint8_t data);
	void scroll_y_w(uint8_t data);
	void flipscreen_x_w(uint8_t data);
	void flipscreen_y_w(uint8_t data);
	void gridiron_led0_w(uint8_t data);
	void gridiron_led1_w(uint8_t data);
	uint8_t teedoff_unk_r();
	uint8_t portA_r();
	uint8_t portB_r();
	void portA_w(uint8_t data);
	void portB_w(uint8_t data);
	void msm_reset_w(uint8_t data);
	void adpcm_int(int state);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void shared_mem(address_map &map) ATTR_COLD;
	void main_mem(address_map &map) ATTR_COLD;
	void sound_mem(address_map &map) ATTR_COLD;
	void sound_port(address_map &map) ATTR_COLD;
	void sub_mem(address_map &map) ATTR_COLD;
};

void tehkanwc_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_track_p1_reset));
	save_item(NAME(m_track_p2_reset));
	save_item(NAME(m_msm_data_offs));
	save_item(NAME(m_toggle));
	save_item(NAME(m_scroll_x));

	m_digits.resolve();
}

void tehkanwc_state::sub_cpu_reset_w(uint8_t data)
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
}

void tehkanwc_state::sound_cpu_reset_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
	msm_reset_w(data);
}

void tehkanwc_state::sound_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


uint8_t tehkanwc_state::track_0_r(offs_t offset)
{
	return m_track_p1[offset]->read() - m_track_p1_reset[offset];
}

uint8_t tehkanwc_state::track_1_r(offs_t offset)
{
	return m_track_p2[offset]->read() - m_track_p2_reset[offset];
}

void tehkanwc_state::track_0_reset_w(offs_t offset, uint8_t data)
{
	// reset the trackball counters
	m_track_p1_reset[offset] = m_track_p1[offset]->read() + data;
}

void tehkanwc_state::track_1_reset_w(offs_t offset, uint8_t data)
{
	// reset the trackball counters
	m_track_p2_reset[offset] = m_track_p2[offset]->read() + data;
}


uint8_t tehkanwc_state::teedoff_unk_r()
{
	logerror("%s: teedoff_unk_r\n", machine().describe_context());
	return 0x80;
}


/* Video emulation */

void tehkanwc_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void tehkanwc_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void tehkanwc_state::videoram2_w(offs_t offset, uint8_t data)
{
	m_videoram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void tehkanwc_state::scroll_x_w(offs_t offset, uint8_t data)
{
	m_scroll_x[offset] = data;
}

void tehkanwc_state::scroll_y_w(uint8_t data)
{
	m_bg_tilemap->set_scrolly(0, data);
}

void tehkanwc_state::flipscreen_x_w(uint8_t data)
{
	flip_screen_x_set(data & 0x40);
}

void tehkanwc_state::flipscreen_y_w(uint8_t data)
{
	flip_screen_y_set(data & 0x40);
}


TILE_GET_INFO_MEMBER(tehkanwc_state::get_bg_tile_info)
{
	int offs = tile_index * 2;
	int attr = m_videoram2[offs + 1];
	int code = m_videoram2[offs] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo.set(2, code, color, flags);
}

TILE_GET_INFO_MEMBER(tehkanwc_state::get_fg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0x10) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo.category = (attr & 0x20) ? 0 : 1;

	tileinfo.set(0, code, color, flags);
}

void tehkanwc_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tehkanwc_state::get_bg_tile_info)),
			TILEMAP_SCAN_ROWS, 16, 8, 32, 32);

	m_fg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tehkanwc_state::get_fg_tile_info)),
			TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void tehkanwc_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0;offs < m_spriteram.bytes();offs += 4)
	{
		int attr = m_spriteram[offs + 1];
		int code = m_spriteram[offs] + ((attr & 0x08) << 5);
		int color = attr & 0x07;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = m_spriteram[offs + 2] + ((attr & 0x20) << 3) - 128;
		int sy = m_spriteram[offs + 3];

		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);
	}
}

uint32_t tehkanwc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scroll_x[0] + 256 * m_scroll_x[1]);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	return 0;
}


/*
   Gridiron Fight has a LED display on the control panel, to let each player
   choose the formation without letting the other know.
*/

void tehkanwc_state::gridiron_led0_w(uint8_t data)
{
	m_digits[0] = (data & 0x80) ? (data & 0x7f) : 0;
}

void tehkanwc_state::gridiron_led1_w(uint8_t data)
{
	m_digits[1] = (data & 0x80) ? (data & 0x7f) : 0;
}


/* Emulate MSM sound samples with counters */

uint8_t tehkanwc_state::portA_r()
{
	return m_msm_data_offs & 0xff;
}

uint8_t tehkanwc_state::portB_r()
{
	return (m_msm_data_offs >> 8) & 0xff;
}

void tehkanwc_state::portA_w(uint8_t data)
{
	m_msm_data_offs = (m_msm_data_offs & 0xff00) | data;
}

void tehkanwc_state::portB_w(uint8_t data)
{
	m_msm_data_offs = (m_msm_data_offs & 0x00ff) | (data << 8);
}

void tehkanwc_state::msm_reset_w(uint8_t data)
{
	m_msm->reset_w(data ? 0 : 1);
}

void tehkanwc_state::adpcm_int(int state)
{
	uint8_t msm_data = m_adpcm_rom[m_msm_data_offs & 0x7fff];

	if (m_toggle == 0)
		m_msm->data_w((msm_data >> 4) & 0x0f);
	else
	{
		m_msm->data_w(msm_data & 0x0f);
		m_msm_data_offs++;
	}

	m_toggle ^= 1;
}


void tehkanwc_state::shared_mem(address_map &map)
{
	map(0xc800, 0xcfff).ram().share("shareram");
	map(0xd000, 0xd3ff).ram().w(FUNC(tehkanwc_state::videoram_w)).share("videoram");
	map(0xd400, 0xd7ff).ram().w(FUNC(tehkanwc_state::colorram_w)).share("colorram");
	map(0xd800, 0xddff).writeonly().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xde00, 0xdfff).nopw(); // unused part of the palette RAM, I think? Gridiron writes here
	map(0xe000, 0xe7ff).ram().w(FUNC(tehkanwc_state::videoram2_w)).share("videoram2");
	map(0xe800, 0xebff).ram().share("spriteram");
	map(0xec00, 0xec01).w(FUNC(tehkanwc_state::scroll_x_w));
	map(0xec02, 0xec02).w(FUNC(tehkanwc_state::scroll_y_w));
}

void tehkanwc_state::main_mem(address_map &map)
{
	shared_mem(map);
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xda00, 0xda00).r(FUNC(tehkanwc_state::teedoff_unk_r));
	map(0xf800, 0xf801).rw(FUNC(tehkanwc_state::track_0_r), FUNC(tehkanwc_state::track_0_reset_w)); // track 0 x/y
	map(0xf802, 0xf802).portr("SYSTEM").w(FUNC(tehkanwc_state::gridiron_led0_w));
	map(0xf803, 0xf803).portr("P1BUT");
	map(0xf806, 0xf806).portr("SYSTEM");
	map(0xf810, 0xf811).rw(FUNC(tehkanwc_state::track_1_r), FUNC(tehkanwc_state::track_1_reset_w)); // track 1 x/y
	map(0xf812, 0xf812).w(FUNC(tehkanwc_state::gridiron_led1_w));
	map(0xf813, 0xf813).portr("P2BUT");
	map(0xf820, 0xf820).r(m_soundlatch2, FUNC(generic_latch_8_device::read)).w(FUNC(tehkanwc_state::sound_command_w)); // answer from the sound CPU
	map(0xf840, 0xf840).portr("DSW2").w(FUNC(tehkanwc_state::sub_cpu_reset_w));
	map(0xf850, 0xf850).portr("DSW3").w(FUNC(tehkanwc_state::sound_cpu_reset_w));
	map(0xf860, 0xf860).r("watchdog", FUNC(watchdog_timer_device::reset_r)).w(FUNC(tehkanwc_state::flipscreen_x_w));
	map(0xf870, 0xf870).portr("DSW1").w(FUNC(tehkanwc_state::flipscreen_y_w));
}

void tehkanwc_state::sub_mem(address_map &map)
{
	shared_mem(map);
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xc7ff).ram();
	map(0xf860, 0xf860).r("watchdog", FUNC(watchdog_timer_device::reset_r));
}

void tehkanwc_state::sound_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8001, 0x8001).w(FUNC(tehkanwc_state::msm_reset_w)); // MSM51xx reset
	map(0x8002, 0x8002).nopw(); // ?? written in the IRQ handler
	map(0x8003, 0x8003).nopw(); // ?? written in the NMI handler
	map(0xc000, 0xc000).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(m_soundlatch2, FUNC(generic_latch_8_device::write));
}

void tehkanwc_state::sound_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("ay1", FUNC(ay8910_device::data_r));
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x02, 0x02).r("ay2", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("ay2", FUNC(ay8910_device::data_address_w));
}



static INPUT_PORTS_START( tehkanwc )
	PORT_START("DSW2") // DSW2 - Active LOW
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING (   0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING (   0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (   0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING (   0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING (   0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING (   0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING (   0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (   0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING (   0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING (   0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Start Credits (P1&P2)/Extra" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING (   0x80, "1&1/200%" )
	PORT_DIPSETTING (   0xc0, "1&2/100%" )
	PORT_DIPSETTING (   0x40, "2&2/100%" )
	PORT_DIPSETTING (   0x00, "2&3/67%" )

	PORT_START("DSW3") // DSW3 - Active LOW
	PORT_DIPNAME( 0x03, 0x03, "1P Game Time" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING (   0x00, "2:30" )
	PORT_DIPSETTING (   0x01, "2:00" )
	PORT_DIPSETTING (   0x03, "1:30" )
	PORT_DIPSETTING (   0x02, "1:00" )
	PORT_DIPNAME( 0x7c, 0x7c, "2P Game Time" ) PORT_DIPLOCATION("SW3:3,4,5,6,7")
	PORT_DIPSETTING (   0x00, "5:00/3:00 Extra" )
	PORT_DIPSETTING (   0x60, "5:00/2:45 Extra" )
	PORT_DIPSETTING (   0x20, "5:00/2:35 Extra" )
	PORT_DIPSETTING (   0x40, "5:00/2:30 Extra" )
	PORT_DIPSETTING (   0x04, "4:00/2:30 Extra" )
	PORT_DIPSETTING (   0x64, "4:00/2:15 Extra" )
	PORT_DIPSETTING (   0x24, "4:00/2:05 Extra" )
	PORT_DIPSETTING (   0x44, "4:00/2:00 Extra" )
	PORT_DIPSETTING (   0x1c, "3:30/2:15 Extra" )
	PORT_DIPSETTING (   0x7c, "3:30/2:00 Extra" )
	PORT_DIPSETTING (   0x3c, "3:30/1:50 Extra" )
	PORT_DIPSETTING (   0x5c, "3:30/1:45 Extra" )
	PORT_DIPSETTING (   0x08, "3:00/2:00 Extra" )
	PORT_DIPSETTING (   0x68, "3:00/1:45 Extra" )
	PORT_DIPSETTING (   0x28, "3:00/1:35 Extra" )
	PORT_DIPSETTING (   0x48, "3:00/1:30 Extra" )
	PORT_DIPSETTING (   0x0c, "2:30/1:45 Extra" )
	PORT_DIPSETTING (   0x6c, "2:30/1:30 Extra" )
	PORT_DIPSETTING (   0x2c, "2:30/1:20 Extra" )
	PORT_DIPSETTING (   0x4c, "2:30/1:15 Extra" )
	PORT_DIPSETTING (   0x10, "2:00/1:30 Extra" )
	PORT_DIPSETTING (   0x70, "2:00/1:15 Extra" )
	PORT_DIPSETTING (   0x30, "2:00/1:05 Extra" )
	PORT_DIPSETTING (   0x50, "2:00/1:00 Extra" )
	PORT_DIPSETTING (   0x14, "1:30/1:15 Extra" )
	PORT_DIPSETTING (   0x74, "1:30/1:00 Extra" )
	PORT_DIPSETTING (   0x34, "1:30/0:50 Extra" )
	PORT_DIPSETTING (   0x54, "1:30/0:45 Extra" )
	PORT_DIPSETTING (   0x18, "1:00/1:00 Extra" )
	PORT_DIPSETTING (   0x78, "1:00/0:45 Extra" )
	PORT_DIPSETTING (   0x38, "1:00/0:35 Extra" )
	PORT_DIPSETTING (   0x58, "1:00/0:30 Extra" )
	PORT_DIPNAME( 0x80, 0x80, "Game Type" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING (   0x80, "Timer In" )
	PORT_DIPSETTING (   0x00, "Credit In" )

	PORT_START("DSW1") // DSW1 - Active LOW
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING (   0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING (   0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "Timer Speed" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING (   0x04, "60/60" )
	PORT_DIPSETTING (   0x00, "55/60" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x08, DEF_STR( On ) )

	PORT_START("P1X")    // IN0 - X AXIS
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("P1Y")    // IN0 - Y AXIS
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("P1BUT")  // IN0 - BUTTON
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P2X")    // IN1 - X AXIS
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)

	PORT_START("P2Y")    // IN1 - Y AXIS
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)

	PORT_START("P2BUT")  // IN1 - BUTTON
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("SYSTEM") // IN2 - Active LOW
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( tehkanwcd )
	PORT_INCLUDE( tehkanwc )

	PORT_MODIFY("DSW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) // DSW1 doesn't exist on this PCB?

	PORT_MODIFY("P1BUT") // IN0 - BUTTON
	// DSW4 in test mode
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW4:1,2")
	PORT_DIPSETTING (   0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING (   0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "Timer Speed" ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING (   0x04, "60/60" )
	PORT_DIPSETTING (   0x00, "55/60" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x08, DEF_STR( On ) )

	PORT_MODIFY("P2BUT") // IN1 - BUTTON
	// DSW5 in test mode
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW5:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW5:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW5:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW5:4" )
INPUT_PORTS_END

static INPUT_PORTS_START( gridiron )
	PORT_INCLUDE( tehkanwc )

	PORT_MODIFY("DSW2") // DSW2 - Active LOW
	PORT_DIPNAME( 0x03, 0x03, "Start Credits (P1&P2)/Extra" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING (   0x01, "1&1/200%" )
	PORT_DIPSETTING (   0x03, "1&2/100%" )
	PORT_DIPSETTING (   0x00, "2&1/200% (duplicate)" )
	PORT_DIPSETTING (   0x02, "2&2/100%" )
	/* This Dip Switch only has an effect in a 2 players game.
	   If offense player selects his formation before defense player,
	   defense formation time will be set to 3, 5 or 7 seconds.
	   Check code at 0x3ed9 and table at 0x3f89. */
	PORT_DIPNAME( 0x0c, 0x0c, "Formation Time (Defense)" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING (   0x0c, "Same as Offense" )
	PORT_DIPSETTING (   0x00, "7" )
	PORT_DIPSETTING (   0x08, "5" )
	PORT_DIPSETTING (   0x04, "3" )
	PORT_DIPNAME( 0x30, 0x30, "Timer Speed" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING (   0x30, "60/60" )
	PORT_DIPSETTING (   0x00, "57/60" )
	PORT_DIPSETTING (   0x10, "54/60" )
	PORT_DIPSETTING (   0x20, "50/60" )
	PORT_DIPNAME( 0xc0, 0xc0, "Formation Time (Offense)" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING (   0x00, "25" )
	PORT_DIPSETTING (   0x40, "20" )
	PORT_DIPSETTING (   0xc0, "15" )
	PORT_DIPSETTING (   0x80, "10" )

	PORT_MODIFY("DSW3") // DSW3 - Active LOW
	PORT_DIPNAME( 0x03, 0x03, "1P Game Time" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING (   0x00, "2:30" )
	PORT_DIPSETTING (   0x01, "2:00" )
	PORT_DIPSETTING (   0x03, "1:30" )
	PORT_DIPSETTING (   0x02, "1:00" )
	PORT_DIPNAME( 0x7c, 0x7c, "2P Game Time" ) PORT_DIPLOCATION("SW3:3,4,5,6,7")
	PORT_DIPSETTING (   0x60, "5:00/3:00 Extra" )
	PORT_DIPSETTING (   0x00, "5:00/2:45 Extra" )
	PORT_DIPSETTING (   0x20, "5:00/2:35 Extra" )
	PORT_DIPSETTING (   0x40, "5:00/2:30 Extra" )
	PORT_DIPSETTING (   0x64, "4:00/2:30 Extra" )
	PORT_DIPSETTING (   0x04, "4:00/2:15 Extra" )
	PORT_DIPSETTING (   0x24, "4:00/2:05 Extra" )
	PORT_DIPSETTING (   0x44, "4:00/2:00 Extra" )
	PORT_DIPSETTING (   0x68, "3:30/2:15 Extra" )
	PORT_DIPSETTING (   0x08, "3:30/2:00 Extra" )
	PORT_DIPSETTING (   0x28, "3:30/1:50 Extra" )
	PORT_DIPSETTING (   0x48, "3:30/1:45 Extra" )
	PORT_DIPSETTING (   0x6c, "3:00/2:00 Extra" )
	PORT_DIPSETTING (   0x0c, "3:00/1:45 Extra" )
	PORT_DIPSETTING (   0x2c, "3:00/1:35 Extra" )
	PORT_DIPSETTING (   0x4c, "3:00/1:30 Extra" )
	PORT_DIPSETTING (   0x7c, "2:30/1:45 Extra" )
	PORT_DIPSETTING (   0x1c, "2:30/1:30 Extra" )
	PORT_DIPSETTING (   0x3c, "2:30/1:20 Extra" )
	PORT_DIPSETTING (   0x5c, "2:30/1:15 Extra" )
	PORT_DIPSETTING (   0x70, "2:00/1:30 Extra" )
	PORT_DIPSETTING (   0x10, "2:00/1:15 Extra" )
	PORT_DIPSETTING (   0x30, "2:00/1:05 Extra" )
	PORT_DIPSETTING (   0x50, "2:00/1:00 Extra" )
	PORT_DIPSETTING (   0x74, "1:30/1:15 Extra" )
	PORT_DIPSETTING (   0x14, "1:30/1:00 Extra" )
	PORT_DIPSETTING (   0x34, "1:30/0:50 Extra" )
	PORT_DIPSETTING (   0x54, "1:30/0:45 Extra" )
	PORT_DIPSETTING (   0x78, "1:00/1:00 Extra" )
	PORT_DIPSETTING (   0x18, "1:00/0:45 Extra" )
	PORT_DIPSETTING (   0x38, "1:00/0:35 Extra" )
	PORT_DIPSETTING (   0x58, "1:00/0:30 Extra" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW3:8")      // Check code at 0x14b4
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x80, DEF_STR( On ) )

	PORT_MODIFY("DSW1") // no DSW1
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( teedoff )
	PORT_INCLUDE( gridiron )

	PORT_MODIFY("DSW2") // DSW2 - Active LOW
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING (   0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING (   0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x30, "Balls" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING (   0x30, "5" )
	PORT_DIPSETTING (   0x20, "6" )
	PORT_DIPSETTING (   0x10, "7" )
	PORT_DIPSETTING (   0x00, "8" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")         // Check code at 0x0c5c
	PORT_DIPSETTING (   0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING (   0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")     // Check code at 0x5dd0
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x80, DEF_STR( On ) )

	PORT_MODIFY("DSW3") // DSW3 - Active LOW
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPNAME( 0x18, 0x18, "Penalty (Over Par)" ) PORT_DIPLOCATION("SW3:4,5") // Check table at 0x2d67
	PORT_DIPSETTING (   0x10, "1/1/2/3/4" ) // +1 / +2 / +3 / +4 / +5 or +6
	PORT_DIPSETTING (   0x18, "1/2/3/3/4" )
	PORT_DIPSETTING (   0x08, "1/2/3/4/4" )
	PORT_DIPSETTING (   0x00, "2/3/3/4/4" )
	PORT_DIPNAME( 0x20, 0x20, "Bonus Balls (Multiple coins)" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING (   0x20, DEF_STR( None ) )
	PORT_DIPSETTING (   0x00, "+1" )
	PORT_DIPNAME( 0xc0, 0xc0, "Difficulty?" )  PORT_DIPLOCATION("SW3:7,8") // Check table at 0x5df9
	PORT_DIPSETTING (   0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING (   0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Hardest ) )

	PORT_MODIFY("SYSTEM") // IN2 - Active LOW
	// "Coin"  buttons are read from address 0xf802
	// "Start" buttons are read from address 0xf806
	// coin input must be active between 2 and 15 frames to be consistently recognized
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	512,    /* 512 sprites */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			8*32+1*4, 8*32+0*4, 8*32+3*4, 8*32+2*4, 8*32+5*4, 8*32+4*4, 8*32+7*4, 8*32+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,8,   /* 16*8 characters */
	1024,   /* 1024 characters */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
		32*8+1*4, 32*8+0*4, 32*8+3*4, 32*8+2*4, 32*8+5*4, 32*8+4*4, 32*8+7*4, 32*8+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	64*8    /* every char takes 64 consecutive bytes */
};

static GFXDECODE_START( gfx_tehkanwc )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 ) /* Colors 0 - 255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256,  8 ) /* Colors 256 - 383 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,   512, 16 ) /* Colors 512 - 767 */
GFXDECODE_END


void tehkanwc_state::tehkanwc(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 18432000/4);     /* 18.432000 / 4 */
	m_maincpu->set_addrmap(AS_PROGRAM, &tehkanwc_state::main_mem);
	m_maincpu->set_vblank_int("screen", FUNC(tehkanwc_state::irq0_line_hold));

	Z80(config, m_subcpu, 18432000/4);
	m_subcpu->set_addrmap(AS_PROGRAM, &tehkanwc_state::sub_mem);
	m_subcpu->set_vblank_int("screen", FUNC(tehkanwc_state::irq0_line_hold));

	Z80(config, m_audiocpu, 18432000/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &tehkanwc_state::sound_mem);
	m_audiocpu->set_addrmap(AS_IO, &tehkanwc_state::sound_port);
	m_audiocpu->set_vblank_int("screen", FUNC(tehkanwc_state::irq0_line_hold));

	config.set_maximum_quantum(attotime::from_hz(600));  /* 10 CPU slices per frame - seems enough to keep the CPUs in sync */

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tehkanwc_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tehkanwc);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 768).set_endianness(ENDIANNESS_BIG);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym2149_device &ay1(YM2149(config, "ay1", 18432000/12));
	ay1.port_a_write_callback().set(FUNC(tehkanwc_state::portA_w));
	ay1.port_b_write_callback().set(FUNC(tehkanwc_state::portB_w));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ym2149_device &ay2(YM2149(config, "ay2", 18432000/12));
	ay2.port_a_read_callback().set(FUNC(tehkanwc_state::portA_r));
	ay2.port_b_read_callback().set(FUNC(tehkanwc_state::portB_r));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(tehkanwc_state::adpcm_int));  /* interrupt function */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8KHz */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.45);
}

void tehkanwc_state::tehkanwcb(machine_config &config)
{
	tehkanwc(config);
	ay8910_device &ay1(AY8910(config.replace(), "ay1", 18432000/12));
	ay1.port_a_write_callback().set(FUNC(tehkanwc_state::portA_w));
	ay1.port_b_write_callback().set(FUNC(tehkanwc_state::portB_w));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config.replace(), "ay2", 18432000/12));
	ay2.port_a_read_callback().set(FUNC(tehkanwc_state::portA_r));
	ay2.port_b_read_callback().set(FUNC(tehkanwc_state::portB_r));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tehkanwc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twc-1.bin",    0x0000, 0x4000, CRC(34d6d5ff) SHA1(72f4d408b8a7766d348f6a229d395e0c98215c40) )
	ROM_LOAD( "twc-2.bin",    0x4000, 0x4000, CRC(7017a221) SHA1(4b4700af0a6ff64f976db369ba4b9d97cee1fd5f) )
	ROM_LOAD( "twc-3.bin",    0x8000, 0x4000, CRC(8b662902) SHA1(13bcd4bf23e34dd7193545561e05bb2cb2c95f9b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "twc-4.bin",    0x0000, 0x8000, CRC(70a9f883) SHA1(ace04359265271eb37512a89eb0217eb013aecb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "twc-6.bin",    0x0000, 0x4000, CRC(e3112be2) SHA1(7859e51b4312dc5df01c88e1d97cf608abc7ca72) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "twc-12.bin",   0x0000, 0x4000, CRC(a9e274f8) SHA1(02b46e1b149a856f0be74a23faaeb792935b66c7) )   /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "twc-8.bin",    0x0000, 0x8000, CRC(055a5264) SHA1(fe294ba57c2c858952e2fab0be1b8859730846cb) )   /* sprites */
	ROM_LOAD( "twc-7.bin",    0x8000, 0x8000, CRC(59faebe7) SHA1(85dad90928369601e039467d575750539410fcf6) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "twc-11.bin",   0x0000, 0x8000, CRC(669389fc) SHA1(a93e8455060ce5242cb65f78e47b4840aa13ab13) )   /* bg tiles */
	ROM_LOAD( "twc-9.bin",    0x8000, 0x8000, CRC(347ef108) SHA1(bb9c2f51d65f28655404e10c3be44d7ade98711b) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "twc-5.bin",    0x0000, 0x4000, CRC(444b5544) SHA1(0786d6d9ada7fe49c8ab9751b049095474d2e598) )
ROM_END

/* from a bootleg board, but clearly a different revision of the game code too,
   it still displays the Tehkan copyright etc. so might actually be a legitimate alt revision */

/*
CPUs

    on main board:
        3x Z8400A-Z80ACPU (main, sound)
        2x YM-3-8910 (sound)
        1x OKI M5205 (sound)

    on roms board:
        1x oscillator 18.000MHz
        1x oscillator 4.00000MHz

ROMs:

    on main board:
        5x TMM27128D (1,2,3,5,6)
        1x HN27256G (4)
        1x PAL16L8A (on a small piggyback, not dumped)

    on roms board:
        1x TMM27128D (12)
        4x HN27256G (7,8,9,11)

Notes:

    on main board:
        2x 18x2 edge connectors
        2x 50 pins flat cable connectors to roms board
        1x trimmer (volume)
        2x red LEDs
        2x 8x2 switches dip

    on roms board:
        2x 50 pins flat cable connectors to roms board

*/

ROM_START( tehkanwcb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e-1.3-18.ic32",   0x0000, 0x4000, CRC(ac9d851b) SHA1(38a799cec4f29a88ed22c7a1e35fd2287cee869a) )
	ROM_LOAD( "e-2.3-17.ic31",   0x4000, 0x4000, CRC(65b53d99) SHA1(ea172b2540763d64dc4a238700421cea27138fae) )
	ROM_LOAD( "e-3.3-15.ic30",   0x8000, 0x4000, CRC(12064bfc) SHA1(954b56a548c697927d58b9cb2ecfe32b4db8d769) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "e-4.9-17.ic100",  0x0000, 0x8000, CRC(70a9f883) SHA1(ace04359265271eb37512a89eb0217eb013aecb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "e-6.8-3.ic83",    0x0000, 0x4000, CRC(e3112be2) SHA1(7859e51b4312dc5df01c88e1d97cf608abc7ca72) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "e-12.8c.ic233",   0x0000, 0x4000, CRC(a9e274f8) SHA1(02b46e1b149a856f0be74a23faaeb792935b66c7) )    /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "e-8.5n.ic191",    0x0000, 0x8000, CRC(055a5264) SHA1(fe294ba57c2c858952e2fab0be1b8859730846cb) )    /* sprites */
	ROM_LOAD( "e-7.5r.ic193",    0x8000, 0x8000, CRC(59faebe7) SHA1(85dad90928369601e039467d575750539410fcf6) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "e-11.8k.ic238",   0x0000, 0x8000, CRC(669389fc) SHA1(a93e8455060ce5242cb65f78e47b4840aa13ab13) )    /* bg tiles */
	ROM_LOAD( "e-9.8n.ic240",    0x8000, 0x8000, CRC(347ef108) SHA1(bb9c2f51d65f28655404e10c3be44d7ade98711b) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "e-5.4-3.ic35",    0x0000, 0x4000, CRC(444b5544) SHA1(0786d6d9ada7fe49c8ab9751b049095474d2e598) )
ROM_END

/* only rom e1 is changed from above bootleg */
ROM_START( tehkanwcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e1.bin",    0x0000, 0x4000, CRC(7aaaddef) SHA1(10f1f8c86504e5b13a6358b633789f9a27be85e3) )
	ROM_LOAD( "e2.bin",    0x4000, 0x4000, CRC(65b53d99) SHA1(ea172b2540763d64dc4a238700421cea27138fae) )
	ROM_LOAD( "e3.bin",    0x8000, 0x4000, CRC(12064bfc) SHA1(954b56a548c697927d58b9cb2ecfe32b4db8d769) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "e4.bin",    0x0000, 0x8000, CRC(70a9f883) SHA1(ace04359265271eb37512a89eb0217eb013aecb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "e6.bin",    0x0000, 0x4000, CRC(e3112be2) SHA1(7859e51b4312dc5df01c88e1d97cf608abc7ca72) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "e12.bin",   0x0000, 0x4000, CRC(a9e274f8) SHA1(02b46e1b149a856f0be74a23faaeb792935b66c7) )  /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "e8.bin",    0x0000, 0x8000, CRC(055a5264) SHA1(fe294ba57c2c858952e2fab0be1b8859730846cb) )  /* sprites */
	ROM_LOAD( "e7.bin",    0x8000, 0x8000, CRC(59faebe7) SHA1(85dad90928369601e039467d575750539410fcf6) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "e11.bin",   0x0000, 0x8000, CRC(669389fc) SHA1(a93e8455060ce5242cb65f78e47b4840aa13ab13) )  /* bg tiles */
	ROM_LOAD( "e9.bin",    0x8000, 0x8000, CRC(347ef108) SHA1(bb9c2f51d65f28655404e10c3be44d7ade98711b) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "e5.bin",    0x0000, 0x4000, CRC(444b5544) SHA1(0786d6d9ada7fe49c8ab9751b049095474d2e598) )
ROM_END

ROM_START( tehkanwcd ) // from a 2-PCB set labeled "A-32302 Tehkan" and "B-32302 Tehkan"
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",    0x0000, 0x4000, CRC(2218d00f) SHA1(9f417246f685a15cec8d737a02840df099b60d77) )
	ROM_LOAD( "2.bin",    0x4000, 0x4000, CRC(dbb39858) SHA1(8d60be2245004e0669ee7c639a8e9904cea6f0e2) )
	ROM_LOAD( "3.bin",    0x8000, 0x4000, CRC(9c69c64a) SHA1(dc4e61fa626461474705de388c31ce253d0cfe94) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "4.bin",    0x0000, 0x8000, CRC(19533319) SHA1(e91c830db55abf7d77c8fd63e32b22f8d5a03372) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "twc-6.bin",    0x0000, 0x4000, CRC(e3112be2) SHA1(7859e51b4312dc5df01c88e1d97cf608abc7ca72) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "twc-12.bin",   0x0000, 0x4000, CRC(a9e274f8) SHA1(02b46e1b149a856f0be74a23faaeb792935b66c7) )   /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "twc-8.bin",    0x0000, 0x8000, CRC(055a5264) SHA1(fe294ba57c2c858952e2fab0be1b8859730846cb) )   /* sprites */
	ROM_LOAD( "twc-7.bin",    0x8000, 0x8000, CRC(59faebe7) SHA1(85dad90928369601e039467d575750539410fcf6) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "twc-11.bin",   0x0000, 0x8000, CRC(669389fc) SHA1(a93e8455060ce5242cb65f78e47b4840aa13ab13) )   /* bg tiles */
	ROM_LOAD( "twc-9.bin",    0x8000, 0x8000, CRC(347ef108) SHA1(bb9c2f51d65f28655404e10c3be44d7ade98711b) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "twc-5.bin",    0x0000, 0x4000, CRC(444b5544) SHA1(0786d6d9ada7fe49c8ab9751b049095474d2e598) )
ROM_END

/* Just a year hack to put "1986" plus some other small changes, but this set has been found on different bootleg TWC PCBs. */
ROM_START( tehkanwch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "worldcup_3.bin",   0x0000, 0x4000, CRC(dd3f789b) SHA1(8e616a64d96f62797485c78e9c3f36fa90486e3f) ) // 27128
	ROM_LOAD( "worldcup_2.bin",   0x4000, 0x4000, CRC(7017a221) SHA1(4b4700af0a6ff64f976db369ba4b9d97cee1fd5f) ) // 27128
	ROM_LOAD( "worldcup_1.bin",   0x8000, 0x4000, CRC(8b662902) SHA1(13bcd4bf23e34dd7193545561e05bb2cb2c95f9b) ) // 27128

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "worldcup_6.bin",   0x0000, 0x8000, CRC(70a9f883) SHA1(ace04359265271eb37512a89eb0217eb013aecb7) ) // 24256

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "worldcup_5.bin",   0x0000, 0x4000, CRC(e3112be2) SHA1(7859e51b4312dc5df01c88e1d97cf608abc7ca72) ) // 27128

	ROM_REGION( 0x4000, "gfx1", 0 ) /* fg tiles */
	ROM_LOAD( "worldcup_9.bin",   0x0000, 0x4000, CRC(a9e274f8) SHA1(02b46e1b149a856f0be74a23faaeb792935b66c7) ) // 27128

	ROM_REGION( 0x10000, "gfx2", 0 ) /* sprites */
	ROM_LOAD( "worldcup_7.bin",   0x0000, 0x8000, CRC(055a5264) SHA1(fe294ba57c2c858952e2fab0be1b8859730846cb) ) // 24256
	ROM_LOAD( "worldcup_8.bin",   0x8000, 0x8000, CRC(59faebe7) SHA1(85dad90928369601e039467d575750539410fcf6) ) // 24256

	ROM_REGION( 0x10000, "gfx3", 0 ) /* bg tiles */
	ROM_LOAD( "worldcup_10.bin",  0x0000, 0x8000, CRC(669389fc) SHA1(a93e8455060ce5242cb65f78e47b4840aa13ab13) ) // 24256
	ROM_LOAD( "worldcup_11.bin",  0x8000, 0x8000, CRC(4ea7586f) SHA1(fd852c1d5ff09270e398137a7687f68d7256c8a6) ) // 24256

	ROM_REGION( 0x8000, "adpcm", 0 ) /* ADPCM samples */
	ROM_LOAD( "worldcup_4.bin",   0x0000,  0x4000, CRC(444b5544) SHA1(0786d6d9ada7fe49c8ab9751b049095474d2e598) ) // 27128
ROM_END

ROM_START( gridiron )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gfight1.bin",  0x0000, 0x4000, CRC(51612741) SHA1(a0417a35f0ce51ba7fc81f27b356852a97f52a58) )
	ROM_LOAD( "gfight2.bin",  0x4000, 0x4000, CRC(a678db48) SHA1(5ddcb93b3ed52cec6ba04bb19832ae239b7d2287) )
	ROM_LOAD( "gfight3.bin",  0x8000, 0x4000, CRC(8c227c33) SHA1(c0b58dbebc159ee681aed33c858f5e0172edd75a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gfight4.bin",  0x0000, 0x4000, CRC(8821415f) SHA1(772ce0770ed869ebf625d210bc2b9c381b14b7ea) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gfight5.bin",  0x0000, 0x4000, CRC(92ca3c07) SHA1(580077ca8cf01996b29497187e41a54242de7f50) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "gfight7.bin",  0x0000, 0x4000, CRC(04390cca) SHA1(ff010c0c18ddd1f793b581f0a70bc1b98ef7d21d) )   /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "gfight8.bin",  0x0000, 0x4000, CRC(5de6a70f) SHA1(416aba9de59d46861671c49f8ca33489db1b8634) )   /* sprites */
	ROM_LOAD( "gfight9.bin",  0x4000, 0x4000, CRC(eac9dc16) SHA1(8b3cf87ede8aba45752cc2651a471a5942570037) )
	ROM_LOAD( "gfight10.bin", 0x8000, 0x4000, CRC(61d0690f) SHA1(cd7c81b0e5356bc865380cae5582d6c6b017dfa1) )
	/* 0c000-0ffff empty */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "gfight11.bin", 0x0000, 0x4000, CRC(80b09c03) SHA1(41627bb6d0f163430c1709a449a42f0f216da852) )   /* bg tiles */
	ROM_LOAD( "gfight12.bin", 0x4000, 0x4000, CRC(1b615eae) SHA1(edfdb4311c5cc314806c8f017f190f7b94f8cd98) )
	/* 08000-0ffff empty */

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "gfight6.bin",  0x0000, 0x4000, CRC(d05d463d) SHA1(30f2bce0ad75c4a7d8344cff16bce27f5e3a3f5d) )
ROM_END

ROM_START( teedoff )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_m5m27c128_dip28.4a",  0x0000, 0x4000, CRC(0e18f6ee) SHA1(7e78b97ca343b6bdc7ee24e99063fbe9bc86e7a2) )
	ROM_LOAD( "2_m5m27c128_dip28.4b",  0x4000, 0x4000, CRC(70635a77) SHA1(301794ef4761ed417ae211bd570d0cbc6a75bcc5) )
	ROM_LOAD( "3_m5m27c128_dip28.4d",  0x8000, 0x4000, CRC(2c765def) SHA1(bf256104d8b89713b69dde3d84d03638241ba6af) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "4_hn27256g@dip28.9c",   0x0000, 0x8000, CRC(a21315bf) SHA1(a9dd2c1fea3a184ec5d40fa3246fa24c4d720bb3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6_m5m27c128_dip28.8r",  0x0000, 0x4000, CRC(f87a43f5) SHA1(268da812846b9ec24cfeb8d89869e39fa01a6676) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "12_m5m27c128_dip28.8u", 0x0000, 0x4000, CRC(4f44622c) SHA1(161c3646a3ec2274bffc957240d47d55a35a8416) )   /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "8_hn27256g_dip28.5j",   0x0000, 0x8000, CRC(363bd1ba) SHA1(c5b7d56b0595712b18351403a9e3325a03de1676) )   /* sprites */
	ROM_LOAD( "7_hn27256g_dip28.5e",   0x8000, 0x8000, CRC(6583fa5b) SHA1(1041181887350d860c517c0a031ab064a20f5cee) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "11_hn27256g_dip28.8m",  0x0000, 0x8000, CRC(1ec00cb5) SHA1(0e61eed3d6fc44ff89d8b9e4f558f0989eb8094f) )   /* bg tiles */
	ROM_LOAD( "9_hn27256g_dip28.8j",   0x8000, 0x8000, CRC(a14347f0) SHA1(00a34ed56ec32336bb524424fcb007d8160163ec) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "5_m5m27c256k_dip28.4r", 0x0000, 0x8000, CRC(90141093) SHA1(e8983d8c47e47481c2a8ee2a0bac6df3b17f8e70) )
ROM_END

ROM_START( teedoffj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "to-1.4a",     0x0000, 0x4000, CRC(cc2aebc5) SHA1(358e77e53b35dd89fcfdb3b2484b8c4fbc34c1be) )
	ROM_LOAD( "to-2.4b",     0x4000, 0x4000, CRC(f7c9f138) SHA1(2fe56059ef67387b5938bb4751aa2f74a58b04fb) )
	ROM_LOAD( "to-3.4d",     0x8000, 0x4000, CRC(a0f0a6da) SHA1(72390c8dc5519d90e39a660e6ec18861fdbadcc8) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "to-4.9c",     0x0000, 0x8000, CRC(e922cbd2) SHA1(922c030be70150efb760fa81bda0bc54f2ec681a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "to-6.8r",     0x0000, 0x4000, CRC(d8dfe1c8) SHA1(d00a71ad89b530339990780334588f5738c60f25) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "12_m5m27c128_dip28.8u",   0x0000, 0x4000, CRC(4f44622c) SHA1(161c3646a3ec2274bffc957240d47d55a35a8416) )   /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "8_hn27256g_dip28.5j",     0x0000, 0x8000, CRC(363bd1ba) SHA1(c5b7d56b0595712b18351403a9e3325a03de1676) )   /* sprites */
	ROM_LOAD( "7_hn27256g_dip28.5e",     0x8000, 0x8000, CRC(6583fa5b) SHA1(1041181887350d860c517c0a031ab064a20f5cee) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "11_hn27256g_dip28.8m",    0x0000, 0x8000, CRC(1ec00cb5) SHA1(0e61eed3d6fc44ff89d8b9e4f558f0989eb8094f) )   /* bg tiles */
	ROM_LOAD( "9_hn27256g_dip28.8j",     0x8000, 0x8000, CRC(a14347f0) SHA1(00a34ed56ec32336bb524424fcb007d8160163ec) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "to-5.4r",     0x0000, 0x8000, CRC(e5e4246b) SHA1(b2fe2e68fa86163ebe1ef00ecce73fb62cef6b19) )
ROM_END

} // anonymous namespace


GAME( 1985, tehkanwc,  0,        tehkanwc, tehkanwc, tehkanwc_state, empty_init,   ROT0,  "Tehkan",  "Tehkan World Cup (set 1)",           MACHINE_SUPPORTS_SAVE )
GAME( 1985, tehkanwcb, tehkanwc, tehkanwcb,tehkanwc, tehkanwc_state, empty_init,   ROT0,  "Tehkan",  "Tehkan World Cup (set 2, bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tehkanwcc, tehkanwc, tehkanwcb,tehkanwc, tehkanwc_state, empty_init,   ROT0,  "bootleg", "Tehkan World Cup (set 3, bootleg)",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // aka 'World Cup 85', different inputs?
GAME( 1985, tehkanwcd, tehkanwc, tehkanwc, tehkanwcd,tehkanwc_state, empty_init,   ROT0,  "Tehkan",  "Tehkan World Cup (set 4, earlier)",  MACHINE_SUPPORTS_SAVE )
GAME( 1986, tehkanwch, tehkanwc, tehkanwc, tehkanwcd,tehkanwc_state, empty_init,   ROT0,  "hack",    "Tehkan World Cup (1986 year hack)",  MACHINE_SUPPORTS_SAVE )

GAMEL(1985, gridiron,  0,        tehkanwc, gridiron, tehkanwc_state, empty_init,   ROT0,  "Tehkan",  "Gridiron Fight",                     MACHINE_SUPPORTS_SAVE, layout_gridiron )

GAME( 1987, teedoff,   0,        tehkanwc, teedoff,  tehkanwc_state, empty_init,   ROT90, "Tecmo",   "Tee'd Off (World)",                  MACHINE_SUPPORTS_SAVE ) // found in US, but no region warning
GAME( 1986, teedoffj,  teedoff,  tehkanwc, teedoff,  tehkanwc_state, empty_init,   ROT90, "Tecmo",   "Tee'd Off (Japan)",                  MACHINE_SUPPORTS_SAVE )
