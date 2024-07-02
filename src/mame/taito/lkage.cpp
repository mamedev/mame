// license:BSD-3-Clause
// copyright-holders: Phil Stroffolino

/***************************************************************************

Legend of Kage
(C)1985 Taito
CPU: Z80 (x2), MC68705
Sound: YM2203 (x2)

Phil Stroffolino
ptroffo@yahoo.com


Stephh's notes (based on the games Z80 code and some tests) :

1) 'lkage'

  - There is an ingame bug about the way difficulty is handled : if you look at code
    at 0xa05a, you'll notice that DSW3 bit 3 is tested as well as DSW3 bit 4.
    But DSW3 bit 4 also determines if coinage info shall be displayed (code at 0x1295) !
    In fact, DSW3 bit 4 is tested only when DSW3 bit 3 is ON ("Normal" difficulty),
    so you have 3 different "levels" of difficulty :

      bit 3   bit 4    Difficulty
       OFF     OFF       Easy
       OFF     ON        Easy
       ON      OFF       Normal
       ON      ON        Hard

    Flame length/color (and perhaps some other stuff) is also affected by DSW3 bit 3.
  - DSW3 bit 7 is supposed to determine how many coin slots there are (again, check
    the coinage display routine and code at 0x1295), but if you look at coinage insertion
    routines (0x091b for COIN1 and 0x0991 for COIN2), you'll notice that DSW3 bit 7
    is NOT tested !

2) 'lkageo'

  - Bugs are the same as in 'lkage'. Some routines addresses vary though :
      * difficulty : 0x9f50
      * coinage display : 0x128f
  - This set does more tests/things when TILT is pressed before jumping to 0x0000
    (see numerous instructions/calls at 0x0318 - most of them related to MCU).

3) 'lkageb*'

  - The difficulty bug is fixed : see code at 0x9e42 which reads DSW3 bits 2 and 3.
    So you have 4 different "levels" of difficulty :

      bit 2   bit 3    Difficulty
       OFF     OFF       Easy
       OFF     ON        Normal
       ON      OFF       Hard
       ON      ON        Hardest

    However DSW3 bit 3 isn't tested anywhere else, so flame length/color is constant.
  - The coin slots bug is not fixed as coinage insertion routines are unchanged
    (coinage display routine is now at 0x13f6).
  - These bootlegs are based on 'lkageo' (call to 0x0318 when TILT is pressed).
  - Hi-scores, scores, and as a consequence bonus lives, have been divided by 10,
    but it's only a cosmetical effect as data from 0xe200 to 0xe22f is unchanged.

4) 'bygone'

  - is it prototype ?
  - MCU (same code as lkage) is not used
  - No music ? (could be emulation bug) (the 2nd YM is not used...)


TODO:

  - The high score display uses a video attribute flag whose purpose isn't known.

  - purpose of the 0x200 byte prom, "a54-10.2" is unknown. It contains values in
    range 0x0..0xf.

  - SOUND: lots of unknown writes to the YM2203 I/O ports. Does it have some sort
    of volume filtering, and if so, only on the SSG channels maybe?

  - Note that all the bootlegs are derived from a different version of the
    original which hasn't been found yet.

  - lkage is verified to be an original set, but it seems to work regardless of what
    the MCU does. Moreover, the MCU returns a checksum which is different from what
    is expected - the MCU computes 0x89, but the main CPU expects 0x5d.
    The game works anyway, it never gives the usual Taito "BAD HW" message
    (because there is no test at 0x033b after call at routine at 0xde1d).

  - Sprite x alignment may be off by 1? An easy way to see it is on the lkage
    title screen, and look at the left tree ninja stars. There's pcb footage that
    looks like MAME (holes visible). And there's also pcb footage with the sprites
    positioned 1 pixel to the left (like MAME with flip-screen).
    Also see bygone pink energy orbs, alignment looks good on MAME on both normal
    and flip-screen, but is no pcb reference AFAIK.

***************************************************************************/

#include "emu.h"

#include "taito68705.h"

#include "cpu/m6805/m6805.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "sound/ta7630.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class lkage_state : public driver_device
{
public:
	lkage_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vreg(*this, "vreg"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bmcu(*this, "bmcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundnmi(*this, "soundnmi")
	{ }

	void lkagebl(machine_config &config);
	void lkage(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void sh_nmi_disable_w(uint8_t data);
	void sh_nmi_enable_w(uint8_t data);
	uint8_t sound_status_r();
	uint8_t mcu_status_r();
	uint8_t fake_mcu_r();
	void fake_mcu_w(uint8_t data);
	uint8_t fake_status_r();

	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void program_map(address_map &map);
	virtual void io_map(address_map &map);
	void bootleg_program_map(address_map &map);
	void mcu_map(address_map &map);
	virtual void sound_map(address_map &map);

	required_shared_ptr<uint8_t> m_vreg;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<input_merger_device> m_soundnmi;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	uint8_t m_bg_tile_bank = 0U;
	uint8_t m_fg_tile_bank = 0U;
	uint8_t m_tx_tile_bank = 0U;
	int8_t m_sprite_dx[2] = { };

	// lkagebl fake MCU
	uint8_t m_mcu_val = 0U;
	uint8_t m_mcu_ready = 0; // CPU data/MCU ready status
};

class lkagem_state : public lkage_state
{
public:
	lkagem_state(const machine_config &mconfig, device_type type, const char *tag) :
		lkage_state(mconfig, type, tag),
		m_ta7630(*this, "ta7630"),
		m_msm(*this, "msm"),
		m_ay(*this, "ay%u", 1U),
		m_exrom(*this, "data")
	{ }

	void lkagem(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	uint8_t exrom_data_r();
	void exrom_offset_w(offs_t offset, uint8_t data);
	void msm_volume_w(uint8_t data);
	template <uint8_t N> void ay_volume_w(uint8_t data);

	virtual void program_map(address_map &map) override;
	virtual void io_map(address_map &map) override { }
	virtual void sound_map(address_map &map) override;

	required_device<ta7630_device> m_ta7630;
	required_device<msm5232_device> m_msm;
	required_device_array<ay8910_device, 2> m_ay;
	required_region_ptr<uint8_t> m_exrom;

	uint8_t m_exrom_offs[2] = { };
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void lkage_state::machine_start()
{
	save_item(NAME(m_bg_tile_bank));
	save_item(NAME(m_fg_tile_bank));
	save_item(NAME(m_tx_tile_bank));

	save_item(NAME(m_mcu_val));
}

void lkagem_state::machine_start()
{
	lkage_state::machine_start();
	save_item(NAME(m_exrom_offs));
}

void lkage_state::machine_reset()
{
	m_bg_tile_bank = m_fg_tile_bank = m_tx_tile_bank = 0;

	m_mcu_ready = 3;
	m_mcu_val = 0;

	m_soundnmi->in_w<1>(0);
}



/*******************************************************************************
    Video hardware
--------------------------------------------------------------------------------

    m_scroll[0x00]: text layer horizontal scroll
    m_scroll[0x01]: text layer vertical scroll
    m_scroll[0x02]: foreground layer horizontal scroll
    m_scroll[0x03]: foreground layer vertical scroll
    m_scroll[0x04]: background layer horizontal scroll
    m_scroll[0x05]: background layer vertical scroll

    m_vreg:
        04 7d f3 : title screen 101
        0c 7d f3 : high score   101
        04 06 f3 : attract#1    110
        04 1e f3 : attract#2    110
        04 1e f3 : attract#3    110
        00 4e f3 : attract#4    110

    m_vreg[0]: 0x00,0x04
        0x02: tx tile bank select (bygone only?)
        0x04: fg tile bank select
        0x08: ?

    m_vreg[1]: 0x7d
        0xf0: tile palette select
        0x08: bg tile bank select
        0x07: priority config?

    m_vreg[2]: 0xf3
        0x03: flip screen x/y
        0xf0: normally 1111, but 1001 and 0001 inbetween stages (while the
        backgrounds are are being redrawn). These bits are used to enable
        individual layers. 0x10 is the text layer, 0x80 should be the sprites,
        the other 2 bits uncertain.

*******************************************************************************/

void lkage_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;

	switch (offset / 0x400)
	{
	case 0:
		m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
		break;

	case 1:
		m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
		break;

	case 2:
		m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
		break;

	default:
		break;
	}
}

TILE_GET_INFO_MEMBER(lkage_state::get_bg_tile_info)
{
	int const code = m_videoram[tile_index + 0x800] + 256 * (m_bg_tile_bank ? 5 : 1);
	tileinfo.set(0, code, 0, 0);
}

TILE_GET_INFO_MEMBER(lkage_state::get_fg_tile_info)
{
	int const code = m_videoram[tile_index + 0x400] + 256 * (m_fg_tile_bank ? 1 : 0);
	tileinfo.set(0, code, 0, 0);
}

TILE_GET_INFO_MEMBER(lkage_state::get_tx_tile_info)
{
	int const code = m_videoram[tile_index] + 256 * (m_tx_tile_bank ? 4 : 0);
	tileinfo.set(0, code, 0, 0);
}

void lkage_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lkage_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lkage_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lkage_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-5, -5 + 32);
	m_fg_tilemap->set_scrolldx(-3, -3 + 28);
	m_tx_tilemap->set_scrolldx(-1, -1 + 24);
	m_sprite_dx[0] = -14; m_sprite_dx[1] = -14;

	for (int i = 0; i < m_videoram.bytes(); i++)
		m_videoram[i] = 0xff;
}

void lkagem_state::video_start()
{
	lkage_state::video_start();
	m_sprite_dx[0] = 8; m_sprite_dx[1] = -4;
}

void lkage_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *source = m_spriteram;
	uint8_t const *const finish = source + m_spriteram.bytes();

	for ( ; source < finish; source += 4)
	{
		int const attributes = source[2];
		/* 0x01: horizontal flip
		 * 0x02: vertical flip
		 * 0x04: bank select
		 * 0x08: sprite size
		 * 0x70: color
		 * 0x80: priority
		 */
		int const color = (attributes >> 4) & 7;
		int flipx = attributes & 0x01;
		int flipy = attributes & 0x02;
		int const height = (attributes & 0x08) ? 2 : 1;
		int sx = source[0] + m_sprite_dx[0];
		int sy = 255 - (16 * height) - source[1];
		int sprite_number = source[3] + ((attributes & 0x04) << 6);

		int const priority_mask = (attributes & 0x80) ? (0xf0 | 0xcc) : 0xf0;

		if (flip_screen_x())
		{
			sx = 256 - source[0] + m_sprite_dx[1];
			flipx = !flipx;
		}

		if (flip_screen_y())
		{
			sy = 254 - (16 * height) - sy;
			flipy = !flipy;
		}
		if (height == 2 && !flipy)
		{
			sprite_number ^= 1;
		}

		for (int y = 0; y < height; y++)
		{
			m_gfxdecode->gfx(1)->prio_transpen(
					bitmap,
					cliprect,
					sprite_number ^ y,
					color,
					flipx, flipy,
					sx & 0xff,
					sy + 16 * y,
					screen.priority(),
					priority_mask, 0);
		}
	}
}

uint32_t lkage_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	flip_screen_x_set(~m_vreg[2] & 0x01);
	flip_screen_y_set(~m_vreg[2] & 0x02);

	int const bg_bank = m_vreg[1] & 0x08;
	if (m_bg_tile_bank != bg_bank)
	{
		m_bg_tile_bank = bg_bank;
		m_bg_tilemap->mark_all_dirty();
	}

	int const fg_bank = m_vreg[0] & 0x04;
	if (m_fg_tile_bank != fg_bank)
	{
		m_fg_tile_bank = fg_bank;
		m_fg_tilemap->mark_all_dirty();
	}

	int const tx_bank = m_vreg[0] & 0x02;
	if (m_tx_tile_bank != tx_bank)
	{
		m_tx_tile_bank = tx_bank;
		m_tx_tilemap->mark_all_dirty();
	}

	m_bg_tilemap->set_palette_offset(0x300 + (m_vreg[1] & 0xf0));
	m_fg_tilemap->set_palette_offset(0x200 + (m_vreg[1] & 0xf0));
	m_tx_tilemap->set_palette_offset(0x100 + (m_vreg[1] & 0xf0));

	m_tx_tilemap->set_scrollx(0, m_scroll[0]);
	m_tx_tilemap->set_scrolly(0, m_scroll[1]);

	m_fg_tilemap->set_scrollx(0, m_scroll[2]);
	m_fg_tilemap->set_scrolly(0, m_scroll[3]);

	m_bg_tilemap->set_scrollx(0, m_scroll[4]);
	m_bg_tilemap->set_scrolly(0, m_scroll[5]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_bg_tilemap->palette_offset(), cliprect);

	if (m_vreg[2] & 0x40)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);

	if (m_vreg[2] & 0x20)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, (m_vreg[1] & 2) ? 2 : 4);

	if (m_vreg[2] & 0x10)
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	if (m_vreg[2] & 0x80)
		draw_sprites(screen, bitmap, cliprect);

	return 0;
}



/*******************************************************************************
    Common handlers
*******************************************************************************/

void lkage_state::sh_nmi_disable_w(uint8_t data)
{
	m_soundnmi->in_w<1>(0);
}

void lkage_state::sh_nmi_enable_w(uint8_t data)
{
	m_soundnmi->in_w<1>(1);
}

uint8_t lkage_state::sound_status_r()
{
	return 0xff;
}

uint8_t lkage_state::mcu_status_r()
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}

// address maps

void lkage_state::program_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram(); // work RAM
	map(0xe800, 0xefff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf003).ram().share(m_vreg);
	map(0xf060, 0xf060).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf061, 0xf061).nopw().r(FUNC(lkage_state::sound_status_r));
	map(0xf063, 0xf063).nopw(); // pulsed; NMI on sound CPU?
	map(0xf080, 0xf080).portr("DSW1");
	map(0xf081, 0xf081).portr("DSW2");
	map(0xf082, 0xf082).portr("DSW3");
	map(0xf083, 0xf083).portr("SYSTEM");
	map(0xf084, 0xf084).portr("P1");
	map(0xf086, 0xf086).portr("P2");
	map(0xf0a3, 0xf0a3).rw("watchdog", FUNC(watchdog_timer_device::reset_r), FUNC(watchdog_timer_device::reset_w));
	map(0xf0c0, 0xf0c5).ram().share(m_scroll);
	map(0xf0e1, 0xf0e1).nopw(); // pulsed
	map(0xf100, 0xf15f).writeonly().share(m_spriteram);
	map(0xf160, 0xf1ff).nopw(); // bygone
	map(0xf400, 0xffff).ram().w(FUNC(lkage_state::videoram_w)).share(m_videoram);
}

void lkage_state::mcu_map(address_map &map)
{
	program_map(map);
	map(0xf062, 0xf062).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0xf087, 0xf087).r(FUNC(lkage_state::mcu_status_r));
}

void lkage_state::io_map(address_map &map)
{
	map(0x4000, 0x7fff).rom().region("data", 0);
}

// sound hardware is almost identical to Bubble Bobble, YM2203 instead of YM3526

void lkage_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa001).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xb000, 0xb000).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw(); // write?
	map(0xb001, 0xb001).nopr().w(FUNC(lkage_state::sh_nmi_enable_w)); // read?
	map(0xb002, 0xb002).w(FUNC(lkage_state::sh_nmi_disable_w));
	map(0xb003, 0xb003).nopw();
	map(0xe000, 0xefff).rom(); // space for diagnostic ROM?
}



/*******************************************************************************
    MSM5232 version handlers
*******************************************************************************/

uint8_t lkagem_state::exrom_data_r()
{
	return m_exrom[((m_exrom_offs[1] & 0x3f) << 8) | m_exrom_offs[0]];
}

void lkagem_state::exrom_offset_w(offs_t offset, uint8_t data)
{
	m_exrom_offs[offset] = data;
}

void lkagem_state::msm_volume_w(uint8_t data)
{
	for (int i = 0; i < 4; i++)
	{
		m_ta7630->set_channel_volume(m_msm, i, data & 0xf);
		m_ta7630->set_channel_volume(m_msm, i + 4, data >> 4);
	}
}

template <uint8_t N>
void lkagem_state::ay_volume_w(uint8_t data)
{
	m_ta7630->set_device_volume(m_ay[N], data >> 4);
}

// address maps

void lkagem_state::program_map(address_map &map)
{
	lkage_state::program_map(map);
	map(0xf0a0, 0xf0a0).r(FUNC(lkagem_state::exrom_data_r));
	map(0xf0a0, 0xf0a1).w(FUNC(lkagem_state::exrom_offset_w));
	map(0xf0e0, 0xf0e0).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0xf0e1, 0xf0e1).r(FUNC(lkagem_state::mcu_status_r));
	map(0xf140, 0xf15f).unmapw();
	map(0xf160, 0xf17f).lw8(NAME([this](offs_t offset, uint8_t data) { m_spriteram[offset | 0x40] = data; }));
}

// lkagem sound hardware is more similar to Buggy Challenge and Metal Soldier Isaac II

void lkagem_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8001).w(m_ay[0], FUNC(ym2149_device::address_data_w));
	map(0x8002, 0x8003).w(m_ay[1], FUNC(ym2149_device::address_data_w));
	map(0x8010, 0x801d).w(m_msm, FUNC(msm5232_device::write));
	map(0x8020, 0x8020).w(FUNC(lkagem_state::msm_volume_w));
	map(0xc000, 0xc000).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw(); // write?
	map(0xc001, 0xc001).nopr().w(FUNC(lkagem_state::sh_nmi_enable_w)); // read?
	map(0xc002, 0xc002).w(FUNC(lkagem_state::sh_nmi_disable_w));
	map(0xc003, 0xc003).nopw();
	map(0xe000, 0xefff).rom(); // space for diagnostic ROM?
}



/*******************************************************************************
    Bootleg handlers
*******************************************************************************/

// Note: This probably uses another MCU program, which is undumped.

uint8_t lkage_state::fake_mcu_r()
{
	int result = 0;

	switch (m_mcu_val)
	{
		// These are for the attract mode
		case 0x01:
			result = m_mcu_val - 1;
			break;

		case 0x90:
			result = m_mcu_val + 0x43;
			break;

		// Gameplay Protection, checked in this order at a start of a play
		case 0xa6:
			result = m_mcu_val + 0x27;
			break;

		case 0x34:
			result = m_mcu_val + 0x7f;
			break;

		case 0x48:
			result = m_mcu_val + 0xb7;
			break;

		default:
			result = m_mcu_val;
			break;
	}
	return result;
}

void lkage_state::fake_mcu_w(uint8_t data)
{
	m_mcu_val = data;
}

uint8_t lkage_state::fake_status_r()
{
	return m_mcu_ready;
}

// address maps

void lkage_state::bootleg_program_map(address_map &map)
{
	program_map(map);
	map(0xf062, 0xf062).rw(FUNC(lkage_state::fake_mcu_r), FUNC(lkage_state::fake_mcu_w));
	map(0xf087, 0xf087).r(FUNC(lkage_state::fake_status_r));
}



/*******************************************************************************
    Input ports
*******************************************************************************/

static INPUT_PORTS_START( lkage )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       // table at 0x04b8
	PORT_DIPSETTING(    0x03, "200k 700k 500k+" )
	PORT_DIPSETTING(    0x02, "200k 900k 700k+" )
	PORT_DIPSETTING(    0x01, "300k 1000k 700k+" )
	PORT_DIPSETTING(    0x00, "300k 1300k 1000k+" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPUNUSED( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPUNUSED( 0x01, 0x01 )
	PORT_DIPNAME( 0x02, 0x02, "Initial Season" )
	PORT_DIPSETTING(    0x02, "Spring" )
	PORT_DIPSETTING(    0x00, "Winter" )                    // same as if you saved the princess twice ("HOWEVER ...")
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )       // see notes
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )           // see notes
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, "1985" )
	PORT_DIPSETTING(    0x20, "MCMLXXXIV" )                 // 1984(!)
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( lkagebl )
	PORT_INCLUDE( lkage )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       // table at 0x04b8
	PORT_DIPSETTING(    0x03, "20k 70k 50k+" )
	PORT_DIPSETTING(    0x02, "20k 90k 70k+" )
	PORT_DIPSETTING(    0x01, "30k 100k 70k+" )
	PORT_DIPSETTING(    0x00, "30k 130k 100k+" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )       // see notes
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bygone )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze After Game Over")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*******************************************************************************
    GFX layouts
*******************************************************************************/

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4),RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4),RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			64+7, 64+6, 64+5, 64+4, 64+3, 64+2, 64+1, 64+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			128+0*8, 128+1*8, 128+2*8, 128+3*8, 128+4*8, 128+5*8, 128+6*8, 128+7*8 },
	32*8
};

static GFXDECODE_START( gfx_lkage )
	GFXDECODE_ENTRY( "gfx", 0x0000, tile_layout,    0, 64 )
	GFXDECODE_ENTRY( "gfx", 0x0000, sprite_layout,  0, 16 )
GFXDECODE_END



/*******************************************************************************
    Machine configs
*******************************************************************************/

void lkage_state::lkage(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &lkage_state::mcu_map);
	m_maincpu->set_addrmap(AS_IO, &lkage_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(lkage_state::irq0_line_hold));

	Z80(config, m_audiocpu, 8_MHz_XTAL / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lkage_state::sound_map); // IRQs are triggered by the YM2203

	TAITO68705_MCU(config, m_bmcu, 12_MHz_XTAL / 4);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 128);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(2*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(lkage_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lkage);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set(m_soundnmi, FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi).output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 8_MHz_XTAL / 2));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.40);

	ym2203_device &ym2(YM2203(config, "ym2", 8_MHz_XTAL / 2));
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.40);
}

void lkagem_state::lkagem(machine_config &config)
{
	lkage(config);

	m_audiocpu->set_periodic_int(FUNC(lkagem_state::irq0_line_hold), attotime::from_hz(60));

	config.device_remove("ym1");
	config.device_remove("ym2");

	YM2149(config, m_ay[0], 8_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.20);
	m_ay[0]->port_a_write_callback().set(FUNC(lkagem_state::ay_volume_w<0>));
	m_ay[0]->port_b_write_callback().set_nop(); // also TA7630?

	YM2149(config, m_ay[1], 8_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.20);
	m_ay[1]->port_a_write_callback().set(FUNC(lkagem_state::ay_volume_w<1>));
	m_ay[1]->port_b_write_callback().set_nop(); // also TA7630?

	MSM5232(config, m_msm, 8_MHz_XTAL / 4);
	m_msm->set_capacitors(1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6);
	for (int i = 0; i < 8; i++)
		m_msm->add_route(i, "mono", 1.00);

	TA7630(config, m_ta7630);
}

void lkage_state::lkagebl(machine_config &config)
{
	lkage(config);

	config.device_remove("bmcu");

	m_maincpu->set_addrmap(AS_PROGRAM, &lkage_state::bootleg_program_map);
}



/*******************************************************************************
    ROM definitions
*******************************************************************************/

ROM_START( lkage )
	ROM_REGION( 0x14000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "a54-01-2.37", 0x0000, 0x8000, CRC(60fd9734) SHA1(33b444b887d80acb3a63ca4534db65c4d8147712) )
	ROM_LOAD( "a54-02-2.38", 0x8000, 0x8000, CRC(878a25ce) SHA1(6228a12774e116e333c3563ee6e20c0c70db514b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x00800, "bmcu:mcu", 0 ) // 68705 code
	ROM_LOAD( "a54-09.53",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) )

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "a54-05-1.84", 0x0000, 0x4000, CRC(0033c06a) SHA1(89964503fc338817c6511fd15942741996b7037a) )
	ROM_LOAD( "a54-06-1.85", 0x4000, 0x4000, CRC(9f04d9ad) SHA1(3b9a4d30348fd02e5c8ae94655548bd4a02dd65d) )
	ROM_LOAD( "a54-07-1.86", 0x8000, 0x4000, CRC(b20561a4) SHA1(0d6d83dfae79ea133e37704ca47426b4c978fb36) )
	ROM_LOAD( "a54-08-1.87", 0xc000, 0x4000, CRC(3ff3b230) SHA1(ffcd964efb0af32b5d7a70305dfda615ea95acbe) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) // unknown

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8-a54-11.34",  0x0000, 0x0104, CRC(56232113) SHA1(4cdc6732aa3e7fbe8df51966a1295253711ecc8f) )
	ROM_LOAD( "pal16l8-a54-12.76",  0x0200, 0x0104, CRC(e57c3c89) SHA1(a23f91da254055bb990e8bb730564c40b5725f78) )
	ROM_LOAD( "pal16l8a-a54-13.27", 0x0400, 0x0104, CRC(c9b1938e) SHA1(2fd1adc4bde8f07cf4b6314d56b48bb3d7144cc3) )
	ROM_LOAD( "pal16l8a-a54-14.35", 0x0600, 0x0104, CRC(a89c644e) SHA1(b41a077d1d070d9563f924c776930c33a4ff27d0) )
ROM_END

ROM_START( lkagea )
	ROM_REGION( 0x14000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "a54-01-1.37", 0x0000, 0x8000, CRC(973da9c5) SHA1(ad3b5d6a329b784e47be563c6f8dc628f32ba0a5) )
	ROM_LOAD( "a54-02-1.38", 0x8000, 0x8000, CRC(27b509da) SHA1(c623950bd7dd2b5699ca948e3731455964106b89) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x00800, "bmcu:mcu", 0 ) // 68705 code
	ROM_LOAD( "a54-09.53",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) )

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "a54-05-1.84", 0x0000, 0x4000, CRC(0033c06a) SHA1(89964503fc338817c6511fd15942741996b7037a) )
	ROM_LOAD( "a54-06-1.85", 0x4000, 0x4000, CRC(9f04d9ad) SHA1(3b9a4d30348fd02e5c8ae94655548bd4a02dd65d) )
	ROM_LOAD( "a54-07-1.86", 0x8000, 0x4000, CRC(b20561a4) SHA1(0d6d83dfae79ea133e37704ca47426b4c978fb36) )
	ROM_LOAD( "a54-08-1.87", 0xc000, 0x4000, CRC(3ff3b230) SHA1(ffcd964efb0af32b5d7a70305dfda615ea95acbe) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) // unknown

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8-a54-11.34",  0x0000, 0x0104, CRC(56232113) SHA1(4cdc6732aa3e7fbe8df51966a1295253711ecc8f) )
	ROM_LOAD( "pal16l8-a54-12.76",  0x0200, 0x0104, CRC(e57c3c89) SHA1(a23f91da254055bb990e8bb730564c40b5725f78) )
	ROM_LOAD( "pal16l8a-a54-13.27", 0x0400, 0x0104, CRC(c9b1938e) SHA1(2fd1adc4bde8f07cf4b6314d56b48bb3d7144cc3) )
	ROM_LOAD( "pal16l8a-a54-14.35", 0x0600, 0x0104, CRC(a89c644e) SHA1(b41a077d1d070d9563f924c776930c33a4ff27d0) )
ROM_END

ROM_START( lkageb )
	ROM_REGION( 0x14000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "a54-01.37", 0x0000, 0x8000, CRC(34eab2c5) SHA1(25bf2dc80d21aa68c3af5debf10b24c75d83a738) )
	ROM_LOAD( "a54-02.38", 0x8000, 0x8000, CRC(ea471d8a) SHA1(1ffc7f78e3e983e16a23e97019f7030f9846569b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x00800, "bmcu:mcu", 0 ) // 68705 code
	ROM_LOAD( "a54-09.53",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) )

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "a54-05.84", 0x0000, 0x4000, CRC(76753e52) SHA1(13f61969d59b055a5ab40237148e091d7cabe190) )
	ROM_LOAD( "a54-06.85", 0x4000, 0x4000, CRC(f33c015c) SHA1(756326daab255d3a36d97e51ee141b9f7157f12e) )
	ROM_LOAD( "a54-07.86", 0x8000, 0x4000, CRC(0e02c2e8) SHA1(1d8a817ba66cf26a4fe51ae00874c0fe6e7cebe3) )
	ROM_LOAD( "a54-08.87", 0xc000, 0x4000, CRC(4ef5f073) SHA1(dfd234542b28cff74692a1c381772da01e8bb4a7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) // unknown

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8-a54-11.34",  0x0000, 0x0104, CRC(56232113) SHA1(4cdc6732aa3e7fbe8df51966a1295253711ecc8f) )
	ROM_LOAD( "pal16l8-a54-12.76",  0x0200, 0x0104, CRC(e57c3c89) SHA1(a23f91da254055bb990e8bb730564c40b5725f78) )
	ROM_LOAD( "pal16l8a-a54-13.27", 0x0400, 0x0104, CRC(c9b1938e) SHA1(2fd1adc4bde8f07cf4b6314d56b48bb3d7144cc3) )
	ROM_LOAD( "pal16l8a-a54-14.35", 0x0600, 0x0104, CRC(a89c644e) SHA1(b41a077d1d070d9563f924c776930c33a4ff27d0) )
ROM_END

ROM_START( lkagem )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF ) // Z80 code
	ROM_LOAD( "a51_11-2", 0x0000, 0x4000, CRC(540fdb1f) SHA1(11d2a5b56d6d72458816aaf7687e490126b468cc) )
	ROM_LOAD( "a51_12-2", 0x4000, 0x4000, CRC(a625a4b8) SHA1(417e7590f98eadc71cbed749350d6d3a1c1fd413) )
	ROM_LOAD( "a51_13-2", 0x8000, 0x4000, CRC(aba8c6a3) SHA1(6723138f54a06d3e4719a43e8e3f11b3cabfb6f7) )
	ROM_LOAD( "a51_10-2", 0xc000, 0x2000, CRC(f6243d5c) SHA1(7b2810afe9c128a290f15c6ea1a6a1c2757ddf55) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "a51_01-1", 0x0000, 0x4000, CRC(03c818ba) SHA1(80604726c647495ab76870806cd1fb448cffe34d) )

	ROM_REGION( 0x00800, "bmcu:mcu", 0 ) // 68705 code
	ROM_LOAD( "a51-09",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) ) // taken from lkage

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "a51_03-1", 0x0000, 0x2000, CRC(99847f0a) SHA1(34ea492e82845d0366bd755ddf1cad7f574d867a) ) // tile
	ROM_LOAD( "a51_07-1", 0x2000, 0x2000, CRC(c9d01e5b) SHA1(16d689ccc9c3cb16e6b4d85f8e50386c78c439e5) ) // spr
	ROM_LOAD( "a51_02-1", 0x4000, 0x2000, CRC(28bbf964) SHA1(67fb767549d7326133c630f424703abe2b14273d) ) // tile
	ROM_LOAD( "a51_06-1", 0x6000, 0x2000, CRC(d16c7c95) SHA1(f3cfc995cc072311b3bd831b69ccb229e2734f53) ) // spr
	ROM_LOAD( "a51_05-1", 0x8000, 0x2000, CRC(38bb3ad0) SHA1(9c24d705e55acaaa99fbb39e06486ca932bda796) ) // tile
	ROM_LOAD( "a51_09-1", 0xa000, 0x2000, CRC(40fd3d86) SHA1(f92156e5e44483b2683457166cb5b9cfc7fbbf14) ) // spr
	ROM_LOAD( "a51_04-1", 0xc000, 0x2000, CRC(8e132cc6) SHA1(c7b196e6b8c3b6841a1f4ca0904597085a53cc25) ) // tile
	ROM_LOAD( "a51_08-1", 0xe000, 0x2000, CRC(2ab68af8) SHA1(6dd91311f2344936590577440898da5f26e35880) ) // spr

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a51_14",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )
ROM_END

ROM_START( lkagebl1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "ic37_1",      0x0000, 0x8000, CRC(05694f7b) SHA1(08a3796d6cf04d64db52ed8208a51084c420e10a) )
	ROM_LOAD( "ic38_2",      0x8000, 0x8000, CRC(22efe29e) SHA1(f7a29d54081ca7509e822ad8823ec977bccc4a40) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "ic93_5",      0x0000, 0x4000, CRC(76753e52) SHA1(13f61969d59b055a5ab40237148e091d7cabe190) )
	ROM_LOAD( "ic94_6",      0x4000, 0x4000, CRC(f33c015c) SHA1(756326daab255d3a36d97e51ee141b9f7157f12e) )
	ROM_LOAD( "ic95_7",      0x8000, 0x4000, CRC(0e02c2e8) SHA1(1d8a817ba66cf26a4fe51ae00874c0fe6e7cebe3) )
	ROM_LOAD( "ic96_8",      0xc000, 0x4000, CRC(4ef5f073) SHA1(dfd234542b28cff74692a1c381772da01e8bb4a7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) // unknown
ROM_END

ROM_START( lkagebl2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "lok.a",       0x0000, 0x8000, CRC(866df793) SHA1(44a9a773d7bbfc5f9d53f56682438ef8b23ecbd6) )
	ROM_LOAD( "lok.b",       0x8000, 0x8000, CRC(fba9400f) SHA1(fedcb9b717feaeec31afda098f0ac2744df6c7be) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "ic93_5",      0x0000, 0x4000, CRC(76753e52) SHA1(13f61969d59b055a5ab40237148e091d7cabe190) )
	ROM_LOAD( "ic94_6",      0x4000, 0x4000, CRC(f33c015c) SHA1(756326daab255d3a36d97e51ee141b9f7157f12e) )
	ROM_LOAD( "ic95_7",      0x8000, 0x4000, CRC(0e02c2e8) SHA1(1d8a817ba66cf26a4fe51ae00874c0fe6e7cebe3) )
	ROM_LOAD( "ic96_8",      0xc000, 0x4000, CRC(4ef5f073) SHA1(dfd234542b28cff74692a1c381772da01e8bb4a7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) // unknown
ROM_END

ROM_START( lkagebl3 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "z1.bin",      0x0000, 0x8000, CRC(60cac488) SHA1(b61df14159f37143b1faed22d77fc7be31602022) )
	ROM_LOAD( "z2.bin",      0x8000, 0x8000, CRC(22c95f17) SHA1(8ca438d508a36918778651adf599cf45a7c4a5d7) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "a54-04.54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "ic93_5",      0x0000, 0x4000, CRC(76753e52) SHA1(13f61969d59b055a5ab40237148e091d7cabe190) )
	ROM_LOAD( "ic94_6",      0x4000, 0x4000, CRC(f33c015c) SHA1(756326daab255d3a36d97e51ee141b9f7157f12e) )
	ROM_LOAD( "ic95_7",      0x8000, 0x4000, CRC(0e02c2e8) SHA1(1d8a817ba66cf26a4fe51ae00874c0fe6e7cebe3) )
	ROM_LOAD( "ic96_8",      0xc000, 0x4000, CRC(4ef5f073) SHA1(dfd234542b28cff74692a1c381772da01e8bb4a7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a54-10.2",    0x0000, 0x0200, CRC(17dfbd14) SHA1(f8f0b6dfedd4ba108dad43ccc7697ef4ab9cbf86) ) // unknown
ROM_END

ROM_START( lkagebl4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.ic37",      0x0000, 0x8000, CRC(fa20e863) SHA1(0edba6014e8d7cdd7f6ad1bb5eb65338a3a91243) )
	ROM_LOAD( "2.ic38",      0x8000, 0x8000, CRC(a5bdd3b4) SHA1(4f691ea8b75fae0dd92f827998e97e40791d24b2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4.ic54",   0x0000, 0x8000, CRC(541faf9a) SHA1(b142ff3bd198f700697ec06ea92db3109ab5818e) )

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a54-03.51",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "5.ic84", 0x0000, 0x4000, CRC(0033c06a) SHA1(89964503fc338817c6511fd15942741996b7037a) )
	ROM_LOAD( "6.ic85", 0x4000, 0x4000, CRC(9f04d9ad) SHA1(3b9a4d30348fd02e5c8ae94655548bd4a02dd65d) )
	ROM_LOAD( "7.ic86", 0x8000, 0x4000, CRC(b20561a4) SHA1(0d6d83dfae79ea133e37704ca47426b4c978fb36) )
	ROM_LOAD( "8.ic87", 0xc000, 0x4000, CRC(3ff3b230) SHA1(ffcd964efb0af32b5d7a70305dfda615ea95acbe) )
ROM_END


/*
Bygone
Taito, 1985?

This is a rare prototype platform game conversion on a Legend Of Kage PCB.
There are some wire mods on the video board.


PCB Layouts
-----------

K1100135A
J1100057A
CPU PCB
M4300040A (sticker, also matches The Legend Of Kage)
|------------------------------------------------------|
|        VOL  LM324   TL074                    PC040DA |
|             PC010SA YM3014    YM2203         PC040DA |
|     MB3731                                   PC040DA |
|             PC010SA YM3014    YM2203      MB2148     |
|    PC030CM                                MB2148     |
|                      8MHz                 MB2148     |
|          6116                                       |-|
|2                                                    | |
|2         A53_07.IC54   Z80A                         | |
|W                                                    | |
|A         A51_09.IC53                                | |
|Y                                                    | |
|                                                     | |
|                                                     | |
|                                     A54-14.IC27     |-|
|          A53_08.IC51     A53_06.IC38                 |
|                                                      |
|                                                      |
|                                                      |
|          6264            A53_05.IC37   Z80B          |
|             DSW3 DSW2 DSW1    TL7700                 |
|------------------------------------------------------|
Notes:
      Z80A   - clock 4.000MHz [8/2]
      Z80B   - clock 6.000MHz [12/2]
      YM2203 - clock 4.000MHz [8/2]
      A51_09 - MC68705P5 microcontroller, clock 3.000MHz [12/4].
               It seems to be from Taito game A51, which is unknown?
               It was not protected ^_^
      A54-14 - PAL16L8
      A53*   - 27C128 and 27C256 EPROMs
      6264   - 8k x8 SRAM
      6116   - 2k x8 SRAM
      2148   - 1k x4 SRAM
      DIPs have 8 switches each


K1100136A
J1100058A
VIDEO PCB
|------------------------------------------------------|
|    12MHz                                             |
|                                                      |
|                                                      |
|                                                      |
|                                            6116      |
|                         6116                         |
|                                            6116     |-|
|1                                                    | |
|8                                                    | |
|W                                                    | |
|A        A54-11.IC76                                 | |
|Y                                    93422  93422    | |
|                                                     | |
|                                     93422  93422    | |
|  A53_04.IC87  MB112S146 MB112S146  A54-13.IC35      |-|
|                                                      |
|  A53_03.IC86  MB112S146 MB112S146  A54-12.IC34       |
|                                                      |
|  A53_02.IC85  MB112S146 MB112S146                    |
|                                                      |
|  A53_01.IC84  MB112S146 MB112S146          A54-10.IC2|
|------------------------------------------------------|
Notes:
      A54-10 - MB7122 PROM
      A54-11 - PAL16L8
      A54-12 - PAL16L8
      A54-13 - PAL16L8
      6116   - 2k x8 SRAM
      93422  - 256b x4 SRAM
      A53*   - 27C128 EPROMs

*/

ROM_START( bygone )
	ROM_REGION( 0x14000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "a53_05.ic37", 0x0000, 0x8000, CRC(63a3f08b) SHA1(781539077cb1d3b8eecc8bd3717330c0f281833d) )
	ROM_LOAD( "a53_06.ic38", 0x8000, 0x8000, CRC(cb0dcb08) SHA1(6b12b018b983b8225b5f33fb9fcd8004a00fd8ff) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "a53_07.ic54",   0x0000, 0x8000, CRC(72f69a77) SHA1(dfc1050a4123b3c83ae733ece1b6fe2836beb901) )

	ROM_REGION( 0x00800, "bmcu:mcu", 0 ) // 68705 code
	ROM_LOAD( "a51_09.ic53",   0x0000, 0x0800, CRC(0e8b8846) SHA1(a4a105462b0127229bb7edfadd2e581c7e40f1cc) ) // the same as lkage

	ROM_REGION( 0x4000, "data", 0 )
	ROM_LOAD( "a53_08.ic51",   0x0000, 0x4000, CRC(f85139f9) SHA1(7e089d1dd5c5fa8abb396b44aa15aabcf8677940) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "a53_01.ic84", 0x0000, 0x4000, CRC(38cf7fb2) SHA1(424efabe2386fa5f1c22444a53952d85c05c2d64) )
	ROM_LOAD( "a53_02.ic85", 0x4000, 0x4000, CRC(dca7adfe) SHA1(83b159e92dab96d9e20fa3a9d1ce7a7d2e83b313) )
	ROM_LOAD( "a53_03.ic86", 0x8000, 0x4000, CRC(af3eb997) SHA1(ba66ffb9d83f91c98446ac38bb0e712ec0800625) )
	ROM_LOAD( "a53_04.ic87", 0xc000, 0x4000, CRC(65af72d3) SHA1(759a1dd7548075630ddb9c692bdb32ad4712c579) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "a54-10.ic2",  0x0000, 0x0400, CRC(369722d9) SHA1(2df9932ad8ce87c0a9d2c89222a4cec12c29046d) ) // unknown
ROM_END

} // anonymous namespace



/*******************************************************************************
    Game drivers
*******************************************************************************/

//    YEAR  NAME      PARENT MACHINE  INPUT    CLASS         INIT        SCREEN COMPANY              FULLNAME                              FLAGS
GAME( 1984, lkage,    0,     lkage,   lkage,   lkage_state,  empty_init, ROT0,  "Taito Corporation", "The Legend of Kage (rev 2)",                MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkagea,   lkage, lkage,   lkage,   lkage_state,  empty_init, ROT0,  "Taito Corporation", "The Legend of Kage (rev 1)",                MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkageb,   lkage, lkage,   lkage,   lkage_state,  empty_init, ROT0,  "Taito Corporation", "The Legend of Kage",                        MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkagem,   lkage, lkagem,  lkage,   lkagem_state, empty_init, ROT0,  "Taito Corporation", "The Legend of Kage (rev 2, MSM5232 sound)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkagebl1, lkage, lkagebl, lkagebl, lkage_state,  empty_init, ROT0,  "bootleg",           "The Legend of Kage (bootleg set 1)",        MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkagebl2, lkage, lkagebl, lkagebl, lkage_state,  empty_init, ROT0,  "bootleg",           "The Legend of Kage (bootleg set 2)",        MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkagebl3, lkage, lkagebl, lkagebl, lkage_state,  empty_init, ROT0,  "bootleg",           "The Legend of Kage (bootleg set 3)",        MACHINE_SUPPORTS_SAVE )
GAME( 1984, lkagebl4, lkage, lkagebl, lkagebl, lkage_state,  empty_init, ROT0,  "bootleg",           "The Legend of Kage (bootleg set 4)",        MACHINE_SUPPORTS_SAVE )

GAME( 1985, bygone,   0,     lkage,   bygone,  lkage_state,  empty_init, ROT0,  "Taito Corporation", "Bygone (prototype)",                        MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
