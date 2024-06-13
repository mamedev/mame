// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***********************************************************************

    Bomb Jack

    driver by Mirko Buffoni, Z80 code research and notes by Stepph
    original Calorie Kun driver by David Haywood/Pierpaolo Prazzoli

    NOTE:
        * The hardware is a later iteration of the designs
          sprung by Tehkan from the Crazy Climber family.

        * bombjack2 is an older version. The lucky message
          has a typo where "LUCKY" is misspelled as "LUCY."

        * The "Bonus Life" DIP switch NEVER gives an extra
          life because of patched code at 0x5a07: There is
          a 'ret' opcode at 0x5a07 that should be 'push af'
          instead. However, it still affects the high score
          table due to the preceding code at 0x0945:

               Value     Bonus Life        High Score
               0x00      none                   10000
               0x01      every 100k            100000
               0x02      every 30k              30000
               0x03      50k only               50000
               0x04      100k only             100000
               0x05      50k and 100k           50000
               0x06      100k and 300k         100000
               0x07      50k, 100k, and 300k    50000

        * The POST sequence appears upside down when reset
          during cocktail mode as player two. This is only
          temporary as the /FLIP line is reset afterwards.

        * PCB references:
            - Bomb Jack: https://youtu.be/YoazYfkkFJY&t=624
            - Calorie Kun: https://youtu.be/Hd-qZ9bS5NU

    TODO:
        * The audio is filtered through a series of LM324
          amplifiers and CD4066 switches. The filters are
          set by each of the three AY-3-8910 sound chips.

        * Convert the audio and video hardware to devices.

***********************************************************************/
#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/segacrp2_device.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class bombjack_state : public driver_device
{
public:
	bombjack_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainram(*this, "mainram")
		, m_watchdog(*this, "watchdog")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_speaker(*this, "speaker")
		, m_ay8910(*this, "psg%u", 1U)
		, m_screen(*this, "screen")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_spriteram(*this, "spriteram")
		, m_spritectrl(*this, "spritectrl")
		, m_bgmaps(*this, "bgmaps")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void bombjack(machine_config &config);

protected:
	// constants
	static XTAL constexpr CLOCK_X1 = 4_MHz_XTAL,
	                      CLOCK_X2 = 12_MHz_XTAL;
	static u16 constexpr HTOTAL = 384, HBSTART = 256, HBEND = 0,
	                     VTOTAL = 264, VBSTART = 240, VBEND = 16;

	// initialization
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	// main CPU
	void program_map(address_map &map);
	void bombjack_map(address_map &map);

	// audio CPU
	u8 soundlatch_r();
	void bombjack_audio_map(address_map &map);
	void bombjack_audio_portmap(address_map &map);

	// video hardware
	void videoram_w(offs_t offset, u8 data);
	void colorram_w(offs_t offset, u8 data);

	void spritectrl_w(offs_t offset, u8 data);

	void background_w(u8 data);

	void flip_w(u8 data);

	void nmi_on_w(u8 data);
	void vblank_nmi(int state);
	void watchdog_w(u8 data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void virtual set_bg_tile_info(u8 const attr, u16 &code, u8 &color, bool &flipx, bool &flipy);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void virtual set_fg_tile_info(u8 const attr, u16 &code, u8 &color, bool &flipx, bool &flipy);

	void draw_sprites(bitmap_ind16 &bitmap, rectangle const &cliprect);
	bool virtual large_sprite(int const index, u8 const attr);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect);

	// devices and memory pointers
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_mainram;
	optional_device<watchdog_timer_device> m_watchdog;

	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<speaker_device> m_speaker;
	required_device_array<ay8910_device, 3> m_ay8910;

	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	optional_shared_ptr<u8> m_spriteram;
	optional_shared_ptr<u8> m_spritectrl;
	required_region_ptr<u8> m_bgmaps;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

private:
	// internal state
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	u8 m_bg_image = 0U;

	bool m_flip = false;

	bool m_nmi_on = false;
};

class calorie_state : public bombjack_state
{
public:
	calorie_state(machine_config const &mconfig,
		device_type type, char const *tag)
		: bombjack_state(mconfig, type, tag)
		, m_opcodes(*this, "opcodes")
	{ }

	void calorie(machine_config &config, bool encrypted);
	void calorieb(machine_config &config);

	void init_calorieb();

protected:
	// main CPU
	void calorie_map(address_map &map);
	void calorie_opcodes_map(address_map &map);

	// audio CPU
	void calorie_audio_map(address_map &map);

	// video hardware
	void set_bg_tile_info(u8 const attr, u16 &code, u8 &color, bool &flipx, bool &flipy) override;
	void set_fg_tile_info(u8 const attr, u16 &code, u8 &color, bool &flipx, bool &flipy) override;

	bool large_sprite(int const index, u8 const attr) override;

	// devices and memory pointers
	optional_shared_ptr<u8> m_opcodes;
};

/*************************************
 *
 *  Audio hardware
 *
 *************************************/
u8 bombjack_state::soundlatch_r()
{
	// An extra flip-flop is used to clear the LS273 after
	// reading it through a LS245 (this flip-flop is then
	// cleared in sync with /1H leading to the audio clock
	// TODO: This is affected by both /MREQ access to the
	// soundlatch and /SRD (Z80 /RD connected to an LS32)
	u8 const res = m_soundlatch->read();
	if (!machine().side_effects_disabled())
		m_soundlatch->clear_w();
	return res;
}

/*************************************
 *
 *  Video hardware
 *
 *************************************/
void bombjack_state::video_start()
{
	save_item(NAME(m_bg_image));
	save_item(NAME(m_flip));

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bombjack_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bombjack_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
}

void bombjack_state::videoram_w(offs_t offset, u8 data)
{
	if (m_videoram[offset] != data)
	{
		m_videoram[offset] = data;
		m_fg_tilemap->mark_tile_dirty(offset);
	}
}

void bombjack_state::colorram_w(offs_t offset, u8 data)
{
	if (m_colorram[offset] != data)
	{
		m_colorram[offset] = data;
		m_fg_tilemap->mark_tile_dirty(offset);
	}
}

void bombjack_state::spritectrl_w(offs_t offset, u8 data)
{
	data = BIT(data, 0, 4); // four bits, addresses 16 sprites
	m_spritectrl[offset] = data;
}

void bombjack_state::background_w(u8 data)
{
	data = BIT(data, 0, 5); // four address bits + a "KILL" bit
	if (m_bg_image != data)
	{
		m_bg_image = data;
		m_bg_tilemap->mark_all_dirty();
	}
}

void bombjack_state::flip_w(u8 data)
{
	data = BIT(data, 0);
	m_flip = data;

	m_bg_tilemap->set_flip(m_flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_fg_tilemap->set_flip(m_flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

void bombjack_state::nmi_on_w(u8 data)
{
	data = BIT(data, 0);
	if (!data)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_nmi_on = data;
}

void bombjack_state::vblank_nmi(int state)
{
	if (state && m_nmi_on)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void bombjack_state::watchdog_w(u8 data)
{
	data = BIT(data, 0);
	m_watchdog->watchdog_enable(data);
}

void bombjack_state::set_bg_tile_info(u8 const attr, u16 &code, u8 &color, bool &flipx, bool &flipy)
{
	color = BIT(attr, 0, 4);
	flipy = BIT(attr, 7);
}

void calorie_state::set_bg_tile_info(u8 const attr, u16 &code, u8 &color, bool &flipx, bool &flipy)
{
	bombjack_state::set_bg_tile_info(attr, code, color, flipx, flipy);

	code |= BIT(attr, 4) << 8;
	flipx = BIT(attr, 6);
}

TILE_GET_INFO_MEMBER(bombjack_state::get_bg_tile_info)
{
	tile_index |= BIT(m_bg_image, 0, 4) << 9;

	u8 const attr = m_bgmaps[tile_index + 0x100];
	u16 code = m_bgmaps[tile_index];
	u8 color = 0;
	bool flipx = false, flipy = false;

	set_bg_tile_info(attr, code, color, flipx, flipy);
	tileinfo.set(1, code, color, (flipx ? TILE_FLIPX : 0) |
	                             (flipy ? TILE_FLIPY : 0));
}

void bombjack_state::set_fg_tile_info(u8 const attr, u16 &code, u8 &color, bool &flipx, bool &flipy)
{
	code |= BIT(attr, 4) << 8;
	color = BIT(attr, 0, 4);
}

void calorie_state::set_fg_tile_info(u8 const attr, u16 &code, u8 &color, bool &flipx, bool &flipy)
{
	bombjack_state::set_fg_tile_info(attr, code, color, flipx, flipy);

	code |= BIT(attr, 5) << 9;
	flipx = BIT(attr, 6);
	flipy = BIT(attr, 7);
}

TILE_GET_INFO_MEMBER(bombjack_state::get_fg_tile_info)
{
	u8 const attr = m_colorram[tile_index];
	u16 code = m_videoram[tile_index];
	u8 color = 0;
	bool flipx = false, flipy = false;

	set_fg_tile_info(attr, code, color, flipx, flipy);
	tileinfo.set(0, code, color, (flipx ? TILE_FLIPX : 0) |
	                             (flipy ? TILE_FLIPY : 0));
}

bool bombjack_state::large_sprite(int const index, u8 const attr)
{
	bool const reverse = m_spritectrl[0] > m_spritectrl[1];
	return (index >  m_spritectrl[reverse ? 1 : 0]) &&
	       (index <= m_spritectrl[reverse ? 0 : 1]);
}

bool calorie_state::large_sprite(int const index, u8 const attr)
{
	return BIT(attr, 4);
}

void bombjack_state::draw_sprites(bitmap_ind16 &bitmap, rectangle const &cliprect)
{
	int const max = 31, fill = HTOTAL / 16;
	for (int sprite = max; sprite > max - fill; sprite--)
	{
		int const offs = sprite * 4;
		int code = m_spriteram[offs];
		int const attr = m_spriteram[offs + 1];
		int const color = BIT(attr, 0, 4);
		// BIT(attr, 4) - internal tag for bonus objects
		// BIT(attr, 5) - internal tag for large objects
		bool flipx = BIT(attr, 6);
		bool flipy = BIT(attr, 7);
		int ypos = m_spriteram[offs + 2];
		int xpos = m_spriteram[offs + 3];

		bool const large = large_sprite(sprite >> 1, attr);
		if (large)
		{
			if (BIT(sprite, 0))
				continue;
			else
				code |= (1 << 6);
		}

		int const vpos = large ? VBSTART - 16 : VBSTART;
		ypos = (vpos + 1) - ypos;
		if (m_flip)
		{
			xpos = vpos - xpos;
			ypos = vpos - ypos;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(large ? 3 : 2)->transpen(bitmap, cliprect,
			code, color, flipx, flipy, xpos, ypos, 0);
	}
}

u32 bombjack_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect)
{
	bitmap.fill(0, cliprect);

	if (BIT(m_bg_image, 4))
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	return 0;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/
void bombjack_state::program_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share(m_mainram);
	map(0x1000, 0x13ff).ram().w(FUNC(bombjack_state::videoram_w)).share(m_videoram);
	map(0x1400, 0x17ff).ram().w(FUNC(bombjack_state::colorram_w)).share(m_colorram);
	map(0x1800, 0x187f).mirror(0x0180).writeonly().share(m_spriteram);
	map(0x1a00, 0x1a01).mirror(0x01fe).writeonly().w(FUNC(bombjack_state::spritectrl_w)).share(m_spritectrl);
	map(0x1c00, 0x1cff).mirror(0x0100).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x1e00, 0x1e00).mirror(0x01ff).writeonly().w(FUNC(bombjack_state::background_w));
	map(0x2000, 0x2fff).noprw(); // decoded but unused
	map(0x3000, 0x3000).mirror(0x07f8).portr("P1").w(FUNC(bombjack_state::nmi_on_w));
	map(0x3001, 0x3001).mirror(0x07f8).portr("P2").nopw();
	map(0x3002, 0x3002).mirror(0x07f8).portr("SYSTEM").nopw();
	map(0x3003, 0x3003).mirror(0x07f8).unmaprw(); // watchdog
	map(0x3004, 0x3004).mirror(0x07f8).portr("SW1").w(FUNC(bombjack_state::flip_w));
	map(0x3005, 0x3005).mirror(0x07f8).portr("SW2").nopw();
	map(0x3006, 0x3007).mirror(0x07f8).noprw(); // decoded but unused
	map(0x3800, 0x3800).mirror(0x07ff).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void bombjack_state::bombjack_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).m(FUNC(bombjack_state::program_map));
	map(0xb003, 0xb003).mirror(0x07f8).r(m_watchdog, FUNC(watchdog_timer_device::reset_r)).w(FUNC(bombjack_state::watchdog_w));
	map(0xc000, 0xdfff).rom();
}

void calorie_state::calorie_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xffff).m(FUNC(calorie_state::program_map));
	map(0xe000, 0xe001).mirror(0x01fe).nopw(); // no sprite latch
	map(0xf000, 0xf000).mirror(0x07f8).nopw(); // no interrupt control
	map(0xf003, 0xf003).mirror(0x07f8).noprw(); // no watchdog
}

void calorie_state::calorie_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_opcodes);
	map(0x8000, 0xbfff).rom().region("maincpu", 0x8000);
	map(0xc000, 0xcfff).ram().share(m_mainram); // TODO: check
}

void bombjack_state::bombjack_audio_map(address_map &map)
{
	map.global_mask(0x7fff);

	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).mirror(0x1800).ram();
	map(0x6000, 0x6000).mirror(0x1fff).r(FUNC(bombjack_state::soundlatch_r));
}

void calorie_state::calorie_audio_map(address_map &map)
{
	// TODO: verify mirrors
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0x87ff).mirror(0x3800).ram();
	map(0xc000, 0xc000).mirror(0x3fff).r(FUNC(calorie_state::soundlatch_r));
}

void bombjack_state::bombjack_audio_portmap(address_map &map)
{
	map.global_mask(0xff);

	map(0x01, 0x01).mirror(0x6e).r(m_ay8910[0], FUNC(ay8910_device::data_r));
	map(0x00, 0x01).mirror(0x6e).w(m_ay8910[0], FUNC(ay8910_device::address_data_w));
	map(0x11, 0x11).mirror(0x6e).r(m_ay8910[1], FUNC(ay8910_device::data_r));
	map(0x10, 0x11).mirror(0x6e).w(m_ay8910[1], FUNC(ay8910_device::address_data_w));
	map(0x81, 0x81).mirror(0x6e).r(m_ay8910[2], FUNC(ay8910_device::data_r));
	map(0x80, 0x81).mirror(0x6e).w(m_ay8910[2], FUNC(ay8910_device::address_data_w));
}

/*************************************
 *
 *  Input ports
 *  (Bomb Jack)
 *
 *************************************/
static INPUT_PORTS_START( bombjack )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW2")
	// Manual states SW2 bits 0-2 are unused and have to be left on OFF (0x00)
	PORT_DIPNAME( 0x07, 0x00, "Bonus Life (Unused)" ) PORT_DIPLOCATION("SW2:!1,!2,!3") // see notes
	PORT_DIPSETTING(    0x02, "Every 30k" )
	PORT_DIPSETTING(    0x01, "Every 100k" )
	PORT_DIPSETTING(    0x07, "50k, 100k and 300k" )
	PORT_DIPSETTING(    0x05, "50k and 100k" )
	PORT_DIPSETTING(    0x03, "50k only" )
	PORT_DIPSETTING(    0x06, "100k and 300k" )
	PORT_DIPSETTING(    0x04, "100k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x18, 0x10, "Bird Speed" ) PORT_DIPLOCATION("SW2:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x40, "Enemies Number & Speed" ) PORT_DIPLOCATION("SW2:!6,!7")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, "Special Coin" ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
INPUT_PORTS_END

/*************************************
 *
 *  Input ports
 *  (Calorie Kun)
 *
 *************************************/
static INPUT_PORTS_START( calorie )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "20,000 Only" )
	PORT_DIPSETTING(    0x03, "20,000 and 60,000" ) // No listed value for 0x02 (Off / On)
	PORT_DIPNAME( 0x04, 0x00, "Number of Bombs" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, "Difficulty - Mogura Nian" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x00, "Difficulty - Select of Mogura" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, "Infinite Lives" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/
static gfx_layout const layout_8x8 =
{
	8, 8,
	RGN_FRAC(1, 3),
	3,
	{ RGN_FRAC(0, 3), RGN_FRAC(1, 3), RGN_FRAC(2, 3) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	8 * 8
};

static gfx_layout const layout_16x16 =
{
	16, 16,
	RGN_FRAC(1, 3),
	3,
	{ RGN_FRAC(0, 3), RGN_FRAC(1, 3), RGN_FRAC(2, 3) },
	{ STEP8(0, 1), STEP8(64, 1) },
	{ STEP8(0, 8), STEP8(128, 8) },
	32 * 8
};

static gfx_layout const layout_32x32 =
{
	32, 32,
	RGN_FRAC(1, 3),
	3,
	{ RGN_FRAC(0, 3), RGN_FRAC(1, 3), RGN_FRAC(2, 3) },
	{ STEP8(0, 1), STEP8(64, 1), STEP8(256, 1), STEP8(320, 1) },
	{ STEP8(0, 8), STEP8(128, 8), STEP8(512, 8), STEP8(640, 8) },
	128 * 8
};

static GFXDECODE_START( gfx_bombjack )
	GFXDECODE_ENTRY( "fgtiles", 0x0000, layout_8x8,   0, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0x0000, layout_16x16, 0, 16 )
	GFXDECODE_ENTRY( "sprites", 0x0000, layout_16x16, 0, 16 )
	GFXDECODE_ENTRY( "sprites", 0x0000, layout_32x32, 0, 16 )
GFXDECODE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/
void bombjack_state::machine_start()
{
	save_item(NAME(m_nmi_on));
}

void bombjack_state::machine_reset()
{
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	// TODO: CLEAR line should be toggled when both the sound
	// latch is actively being read and the RESET line is on
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void bombjack_state::bombjack(machine_config &config)
{
	// NOTE: X2 is 12MHz, but schematics specify 12.096MHz
	Z80(config, m_maincpu, CLOCK_X1);
	m_maincpu->set_addrmap(AS_PROGRAM, &bombjack_state::bombjack_map);

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 8);

	Z80(config, m_audiocpu, CLOCK_X2 / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &bombjack_state::bombjack_audio_map);
	m_audiocpu->set_addrmap(AS_IO, &bombjack_state::bombjack_audio_portmap);

	GENERIC_LATCH_8(config, m_soundlatch);

	SPEAKER(config, m_speaker).front_center();

	AY8910(config, m_ay8910[0], CLOCK_X2 / 8).add_route(ALL_OUTPUTS, m_speaker, 0.13);
	m_ay8910[0]->port_a_write_callback().set_nop(); // see todo
	m_ay8910[0]->port_b_write_callback().set_nop();
	AY8910(config, m_ay8910[1], CLOCK_X2 / 8).add_route(ALL_OUTPUTS, m_speaker, 0.13);
	m_ay8910[1]->port_a_write_callback().set_nop(); // see todo
	m_ay8910[1]->port_b_write_callback().set_nop();
	AY8910(config, m_ay8910[2], CLOCK_X2 / 8).add_route(ALL_OUTPUTS, m_speaker, 0.13);
	m_ay8910[2]->port_a_write_callback().set_nop(); // see todo
	m_ay8910[2]->port_b_write_callback().set_nop();

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(CLOCK_X2 / 2, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(bombjack_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(bombjack_state::vblank_nmi));
	m_screen->screen_vblank().append_inputline(m_audiocpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bombjack);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 128);
}

void calorie_state::calorie(machine_config &config, bool encrypted = true)
{
	bombjack(config);

	if (encrypted)
	{
		sega_317_0004_device &maincpu(SEGA_317_0004(config.replace(), m_maincpu, CLOCK_X1));
		maincpu.set_addrmap(AS_OPCODES, &calorie_state::calorie_opcodes_map);
		maincpu.set_decrypted_tag(m_opcodes);
	}
	m_maincpu->set_addrmap(AS_PROGRAM, &calorie_state::calorie_map);

	config.device_remove("watchdog");

	m_audiocpu->set_addrmap(AS_PROGRAM, &calorie_state::calorie_audio_map);

	// NOTE: never seems to use filters
	YM2149(config.replace(), m_ay8910[0], CLOCK_X2 / 8).add_route(ALL_OUTPUTS, m_speaker, 0.13);
	YM2149(config.replace(), m_ay8910[1], CLOCK_X2 / 8).add_route(ALL_OUTPUTS, m_speaker, 0.13);
	YM2149(config.replace(), m_ay8910[2], CLOCK_X2 / 8).add_route(ALL_OUTPUTS, m_speaker, 0.13);

	// TODO: proper IRQ pulse timings
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0, HOLD_LINE);
	m_screen->screen_vblank().append_inputline(m_audiocpu, INPUT_LINE_IRQ0, HOLD_LINE);
}

void calorie_state::calorieb(machine_config &config)
{
	calorie(config, false);

	// not encrypted, but still needs a lookup table for /M1
	m_maincpu->set_addrmap(AS_OPCODES, &calorie_state::calorie_opcodes_map);
}

void calorie_state::init_calorieb()
{
	memcpy(m_opcodes, memregion("maincpu")->base() + 0x10000, 0x8000);
}

/*************************************
 *
 *  ROM definitions
 *  (Bomb Jack)
 *
 *************************************/
ROM_START( bombjack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "09_j01b.bin", 0x0000, 0x2000, CRC(c668dc30) SHA1(51dd6a2688b42e9f28f0882bd76f75be7ec3222a) )
	ROM_LOAD( "10_l01b.bin", 0x2000, 0x2000, CRC(52a1e5fb) SHA1(e1cdc4b4efbc6c7a1e4fa65019486617f2acba1b) )
	ROM_LOAD( "11_m01b.bin", 0x4000, 0x2000, CRC(b68a062a) SHA1(43bae56494ac0202aaa8f1ed5c1ed1bff775b2b8) )
	ROM_LOAD( "12_n01b.bin", 0x6000, 0x2000, CRC(1d3ecee5) SHA1(8b3c49e21ea4952cae7042890d1be2115f7d6fda) )
	ROM_LOAD( "13.1r",       0xc000, 0x2000, CRC(70e0244d) SHA1(67654155e42821ea78a655f869fb81c8d6387f63) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "01_h03t.3h", 0x0000, 0x2000, CRC(8407917d) SHA1(318face9f7a7ab6c7eeac773995040425e780aaf) )
	// IC 3J not populated

	ROM_REGION( 0x3000, "fgtiles", 0 )
	ROM_LOAD( "03_e08t.bin", 0x0000, 0x1000, CRC(9f0470d5) SHA1(94ef52ef47b4399a03528fe3efeac9c1d6983446) )
	ROM_LOAD( "04_h08t.bin", 0x1000, 0x1000, CRC(81ec12e6) SHA1(e29ba193f21aa898499187603b25d2e226a07c7b) )
	ROM_LOAD( "05_k08t.bin", 0x2000, 0x1000, CRC(e87ec8b1) SHA1(a66808ef2d62fca2854396898b86bac9be5f17a3) )

	ROM_REGION( 0x6000, "bgtiles", 0 )
	ROM_LOAD( "06_l08t.bin", 0x0000, 0x2000, CRC(51eebd89) SHA1(515128a3971fcb97b60c5b6bdd2b03026aec1921) )
	ROM_LOAD( "07_n08t.bin", 0x2000, 0x2000, CRC(9dd98e9d) SHA1(6db6006a6e20ff7c243d88293ca53681c4703ea5) )
	ROM_LOAD( "08_r08t.bin", 0x4000, 0x2000, CRC(3155ee7d) SHA1(e7897dca4c145f10b7d975b8ef0e4d8aa9354c25) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "16_m07b.bin", 0x0000, 0x2000, CRC(94694097) SHA1(de71bcd67f97d05527f2504fc8430be333fb9ec2) )
	ROM_LOAD( "15_l07b.bin", 0x2000, 0x2000, CRC(013f58f2) SHA1(20c64593ab9fcb04cefbce0cd5d17ce3ff26441b) )
	ROM_LOAD( "14_j07b.bin", 0x4000, 0x2000, CRC(101c858d) SHA1(ed1746c15cdb04fae888601d940183d5c7702282) )

	ROM_REGION( 0x2000, "bgmaps", 0 ) // schematics specify 2764, final board has a smaller ROM in place
	ROM_LOAD( "02_p04t.bin", 0x0000, 0x1000, CRC(398d4a02) SHA1(ac18a8219f99ba9178b96c9564de3978e39c59fd) )
	ROM_RELOAD(              0x1000, 0x1000)
ROM_END

ROM_START( bombjack2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "09_j01b.bin", 0x0000, 0x2000, CRC(c668dc30) SHA1(51dd6a2688b42e9f28f0882bd76f75be7ec3222a) )
	ROM_LOAD( "10_l01b.bin", 0x2000, 0x2000, CRC(52a1e5fb) SHA1(e1cdc4b4efbc6c7a1e4fa65019486617f2acba1b) )
	ROM_LOAD( "11_m01b.bin", 0x4000, 0x2000, CRC(b68a062a) SHA1(43bae56494ac0202aaa8f1ed5c1ed1bff775b2b8) )
	ROM_LOAD( "12_n01b.bin", 0x6000, 0x2000, CRC(1d3ecee5) SHA1(8b3c49e21ea4952cae7042890d1be2115f7d6fda) )
	ROM_LOAD( "13_r01b.bin", 0xc000, 0x2000, CRC(bcafdd29) SHA1(d243eb1249e885aa75fc910fce6e7744770d6e82) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "01_h03t.bin", 0x0000, 0x2000, CRC(8407917d) SHA1(318face9f7a7ab6c7eeac773995040425e780aaf) )
	// IC 3J not populated

	ROM_REGION( 0x3000, "fgtiles", 0 )
	ROM_LOAD( "03_e08t.bin", 0x0000, 0x1000, CRC(9f0470d5) SHA1(94ef52ef47b4399a03528fe3efeac9c1d6983446) )
	ROM_LOAD( "04_h08t.bin", 0x1000, 0x1000, CRC(81ec12e6) SHA1(e29ba193f21aa898499187603b25d2e226a07c7b) )
	ROM_LOAD( "05_k08t.bin", 0x2000, 0x1000, CRC(e87ec8b1) SHA1(a66808ef2d62fca2854396898b86bac9be5f17a3) )

	ROM_REGION( 0x6000, "bgtiles", 0 )
	ROM_LOAD( "06_l08t.bin", 0x0000, 0x2000, CRC(51eebd89) SHA1(515128a3971fcb97b60c5b6bdd2b03026aec1921) )
	ROM_LOAD( "07_n08t.bin", 0x2000, 0x2000, CRC(9dd98e9d) SHA1(6db6006a6e20ff7c243d88293ca53681c4703ea5) )
	ROM_LOAD( "08_r08t.bin", 0x4000, 0x2000, CRC(3155ee7d) SHA1(e7897dca4c145f10b7d975b8ef0e4d8aa9354c25) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "16_m07b.bin", 0x0000, 0x2000, CRC(94694097) SHA1(de71bcd67f97d05527f2504fc8430be333fb9ec2) )
	ROM_LOAD( "15_l07b.bin", 0x2000, 0x2000, CRC(013f58f2) SHA1(20c64593ab9fcb04cefbce0cd5d17ce3ff26441b) )
	ROM_LOAD( "14_j07b.bin", 0x4000, 0x2000, CRC(101c858d) SHA1(ed1746c15cdb04fae888601d940183d5c7702282) )

	ROM_REGION( 0x2000, "bgmaps", 0 ) // schematics specify 2764, final board has a smaller ROM in place
	ROM_LOAD( "02_p04t.bin", 0x0000, 0x1000, CRC(398d4a02) SHA1(ac18a8219f99ba9178b96c9564de3978e39c59fd) )
    ROM_RELOAD(              0x1000, 0x1000)
ROM_END

ROM_START( bombjackt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9.1j",  0x0000, 0x4000, CRC(4b59a3bb) SHA1(dae45985d2575821c86757ead14b8313e922570d) ) // == 09_j01b.bin + 10_l01b.bin
	ROM_LOAD( "12.1n", 0x4000, 0x4000, CRC(0a32506a) SHA1(2fb3ce695caebbae3ca7dd9f3d34ac5b734d77ed) ) // == 11_m01b.bin + (97.229004%) 12_n01b.bin
	ROM_LOAD( "13.1r", 0xc000, 0x2000, CRC(964ac5c5) SHA1(8d235ae91aea1ae86411671c5aa050c146a52026) ) // sldh - (99.877930%) 13_r01b.bin

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "1.6h",  0x0000, 0x2000, CRC(8407917d) SHA1(318face9f7a7ab6c7eeac773995040425e780aaf) )
	// IC not populated

	ROM_REGION( 0x6000, "fgtiles", 0 ) // uses double size ROMs here (content duplicated in each half)
	ROM_LOAD( "3.1e",  0x0000, 0x2000, CRC(54e1dac1) SHA1(3c5d8b932b2a87acf42e0b4632195776689c1154) )
	ROM_LOAD( "4.1h",  0x2000, 0x2000, CRC(05e428ab) SHA1(0b2cae76aba8372482a4e315a9f49fd15cb94625) )
	ROM_LOAD( "5.1k",  0x4000, 0x2000, CRC(f282f29a) SHA1(521a110213d6ecdf54be0f50f41c3c266d65d84c) )

	ROM_REGION( 0x6000, "bgtiles", 0 ) // ok
	ROM_LOAD( "6.1l",  0x0000, 0x2000, CRC(51eebd89) SHA1(515128a3971fcb97b60c5b6bdd2b03026aec1921) )
	ROM_LOAD( "7.1n",  0x2000, 0x2000, CRC(9dd98e9d) SHA1(6db6006a6e20ff7c243d88293ca53681c4703ea5) )
	ROM_LOAD( "8.1r",  0x4000, 0x2000, CRC(3155ee7d) SHA1(e7897dca4c145f10b7d975b8ef0e4d8aa9354c25) )

	ROM_REGION( 0x2000, "bgmaps", 0 ) // 1xxxxxxxxxxxx = 0xFF (double size, second half empty, otherwise the same)
	ROM_LOAD( "2.5n",  0x0000, 0x2000, CRC(de796158) SHA1(e004f10ada5c282f3b4208031e274190a54bf94f) )

	ROM_REGION( 0x6000, "sprites", 0 ) // ok
	ROM_LOAD( "16.7m", 0x0000, 0x2000, CRC(94694097) SHA1(de71bcd67f97d05527f2504fc8430be333fb9ec2) )
	ROM_LOAD( "15.7k", 0x2000, 0x2000, CRC(013f58f2) SHA1(20c64593ab9fcb04cefbce0cd5d17ce3ff26441b) )
	ROM_LOAD( "14.7j", 0x4000, 0x2000, CRC(101c858d) SHA1(ed1746c15cdb04fae888601d940183d5c7702282) )
ROM_END

// Only 12 and 13 differ slightly (bumped up year + changes in some data tables.
// According to the dumper the game is harder, with more enemies shown from the start.
ROM_START( bombjackbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "09.bin", 0x0000, 0x2000, CRC(c668dc30) SHA1(51dd6a2688b42e9f28f0882bd76f75be7ec3222a) )
	ROM_LOAD( "10.bin", 0x2000, 0x2000, CRC(52a1e5fb) SHA1(e1cdc4b4efbc6c7a1e4fa65019486617f2acba1b) )
	ROM_LOAD( "11.bin", 0x4000, 0x2000, CRC(b68a062a) SHA1(43bae56494ac0202aaa8f1ed5c1ed1bff775b2b8) )
	ROM_LOAD( "12.bin", 0x6000, 0x2000, CRC(0f4d0726) SHA1(282215fa50c6fd6a63ce0879f1fb9fb88dbd1888) )
	ROM_LOAD( "13.bin", 0xc000, 0x2000, CRC(9740f99b) SHA1(c0f992c07b17caccb83bee70bd65c554b1a66dc0) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "01.bin", 0x0000, 0x2000, CRC(8407917d) SHA1(318face9f7a7ab6c7eeac773995040425e780aaf) )
	// IC not populated

	ROM_REGION( 0x3000, "fgtiles", 0 )
	ROM_LOAD( "03.bin", 0x0000, 0x1000, CRC(9f0470d5) SHA1(94ef52ef47b4399a03528fe3efeac9c1d6983446) )
	ROM_LOAD( "04.bin", 0x1000, 0x1000, CRC(81ec12e6) SHA1(e29ba193f21aa898499187603b25d2e226a07c7b) )
	ROM_LOAD( "05.bin", 0x2000, 0x1000, CRC(e87ec8b1) SHA1(a66808ef2d62fca2854396898b86bac9be5f17a3) )

	ROM_REGION( 0x6000, "bgtiles", 0 )
	ROM_LOAD( "06.bin", 0x0000, 0x2000, CRC(51eebd89) SHA1(515128a3971fcb97b60c5b6bdd2b03026aec1921) )
	ROM_LOAD( "07.bin", 0x2000, 0x2000, CRC(9dd98e9d) SHA1(6db6006a6e20ff7c243d88293ca53681c4703ea5) )
	ROM_LOAD( "08.bin", 0x4000, 0x2000, CRC(3155ee7d) SHA1(e7897dca4c145f10b7d975b8ef0e4d8aa9354c25) )

	ROM_REGION( 0x2000, "bgmaps", 0 ) // schematics specify 2764, final board has a smaller ROM in place
	ROM_LOAD( "02.bin", 0x0000, 0x1000, CRC(398d4a02) SHA1(ac18a8219f99ba9178b96c9564de3978e39c59fd) )
	ROM_RELOAD(         0x1000, 0x1000)

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "16.bin", 0x0000, 0x2000, CRC(94694097) SHA1(de71bcd67f97d05527f2504fc8430be333fb9ec2) )
	ROM_LOAD( "15.bin", 0x2000, 0x2000, CRC(013f58f2) SHA1(20c64593ab9fcb04cefbce0cd5d17ce3ff26441b) )
	ROM_LOAD( "14.bin", 0x4000, 0x2000, CRC(101c858d) SHA1(ed1746c15cdb04fae888601d940183d5c7702282) )
ROM_END

/*************************************
 *
 *  ROM definitions
 *  (Calorie Kun)
 *
 *************************************/
/*
Calorie Kun
Sega, 1986

PCB Layout
----------

Top PCB

837-6077  171-5381
|--------------------------------------------|
|                    Z80             YM2149  |
|                                            |
|                    2016            YM2149  |
|                                            |
|10079    10077      10075           YM2149  |
|    10078    10076                          |
|                                           -|
|                                          |
|                    2114                   -|
|                                           1|
|                    2114                   8|
|                                           W|
|                    2114                   A|
|                                           Y|
|                                           -|
|                                          |
|                                    DSW2(8)-|
|           10082   10081   10080            |
|2016                                DSW1(8) |
|--------------------------------------------|

Notes:
      Z80 clock   : 3.000MHz
      YM2149 clock: 1.500MHz
      VSync       : 60Hz
      2016        : 2K x8 SRAM
      2114        : 1K x4 SRAM

Bottom PCB

837-6076  171-5380
|--------------------------------------------|
|                            12MHz           |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
| 10074                               10071  |
|               NEC                          |
| 10073      D317-0004   4MHz         10070  |
|                                            |
| 10072                               10069  |
|                                            |
| 2016         2114      6148                |
|                                            |
| 2016         2114      6148                |
|                                            |
|                        6148                |
|                        6148                |
|--------------------------------------------|
Notes:
      317-0004 clock: 4.000MHz
      2016          : 2K x8 SRAM
      2114          : 1K x4 SRAM
      6148          : 1K x4 SRAM
*/
ROM_START( calorie )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr10072.1j", 0x00000, 0x4000, CRC(ade792c1) SHA1(6ea5afb00a87037d502c17adda7e4060d12680d7) )
	ROM_LOAD( "epr10073.1k", 0x04000, 0x4000, CRC(b53e109f) SHA1(a41c5affe917232e7adf40d7c15cff778b197e90) )
	ROM_LOAD( "epr10074.1m", 0x08000, 0x4000, CRC(a08da685) SHA1(69f9cfebc771312dbb1726350c2d9e9e8c46353f) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "epr10075.4d", 0x0000, 0x4000, CRC(ca547036) SHA1(584a65482f2b92a4c08c37560450d6db68a56c7b) )

	ROM_REGION( 0x6000, "fgtiles", 0 )
	ROM_LOAD( "epr10082.5r", 0x0000, 0x2000, CRC(5984ea44) SHA1(010011b5b8dfa593c6fc7d2366f8cf82fcc8c978) )
	ROM_LOAD( "epr10081.4r", 0x2000, 0x2000, CRC(e2d45dd8) SHA1(5e11089680b574ea4cbf64510e51b0a945f79174) )
	ROM_LOAD( "epr10080.3r", 0x4000, 0x2000, CRC(42edfcfe) SHA1(feba7b1daffcad24d4c24f55ab5466f8cebf31ad) )

	ROM_REGION( 0xc000, "bgtiles", 0 )
	ROM_LOAD( "epr10078.7d", 0x0000, 0x4000, CRC(5b8eecce) SHA1(e7eee82081939b361edcbb9587b072b4b9a162f9) )
	ROM_LOAD( "epr10077.6d", 0x4000, 0x4000, CRC(01bcb609) SHA1(5d01fa75f214d34483284aaaef985ab92a606505) )
	ROM_LOAD( "epr10076.5d", 0x8000, 0x4000, CRC(b1529782) SHA1(8e0e92aae4c8dd8720414372aa767054cc316a0f) )

	ROM_REGION( 0x2000, "bgmaps", 0 )
	ROM_LOAD( "epr10079.8d", 0x0000, 0x2000, CRC(3c61a42c) SHA1(68ea6b5d2f3c6a9e5308c08dde20424f20021a73) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "epr10071.7m", 0x0000, 0x4000, CRC(5f55527a) SHA1(ec1ba8f95ac47a0c783e117ef4af6fe0ab5925b5) )
	ROM_LOAD( "epr10070.7k", 0x4000, 0x4000, CRC(97f35a23) SHA1(869553a334e1b3ba900a8b9c9eaf25fbc6ab31dd) )
	ROM_LOAD( "epr10069.7j", 0x8000, 0x4000, CRC(c0c3deaf) SHA1(8bf2e2146b794a330a079dd080f0586500964b1a) )
ROM_END

ROM_START( calorieb )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "12.bin",      0x10000, 0x4000, CRC(cf5fa69e) SHA1(520d5652e93a672a1fc147295fbd63b873967885) )
	ROM_CONTINUE(            0x00000, 0x4000 )
	ROM_LOAD( "13.bin",      0x14000, 0x4000, CRC(52e7263f) SHA1(4d684c9e3f08ddb18b0b3b982aba82d3c809a633) )
	ROM_CONTINUE(            0x04000, 0x4000 )
	ROM_LOAD( "epr10074.1m", 0x08000, 0x4000, CRC(a08da685) SHA1(69f9cfebc771312dbb1726350c2d9e9e8c46353f) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "epr10075.4d", 0x0000, 0x4000, CRC(ca547036) SHA1(584a65482f2b92a4c08c37560450d6db68a56c7b) )

	ROM_REGION( 0x6000, "fgtiles", 0 )
	ROM_LOAD( "epr10082.5r", 0x0000, 0x2000, CRC(5984ea44) SHA1(010011b5b8dfa593c6fc7d2366f8cf82fcc8c978) )
	ROM_LOAD( "epr10081.4r", 0x2000, 0x2000, CRC(e2d45dd8) SHA1(5e11089680b574ea4cbf64510e51b0a945f79174) )
	ROM_LOAD( "epr10080.3r", 0x4000, 0x2000, CRC(42edfcfe) SHA1(feba7b1daffcad24d4c24f55ab5466f8cebf31ad) )

	ROM_REGION( 0xc000, "bgtiles", 0 )
	ROM_LOAD( "epr10078.7d", 0x0000, 0x4000, CRC(5b8eecce) SHA1(e7eee82081939b361edcbb9587b072b4b9a162f9) )
	ROM_LOAD( "epr10077.6d", 0x4000, 0x4000, CRC(01bcb609) SHA1(5d01fa75f214d34483284aaaef985ab92a606505) )
	ROM_LOAD( "epr10076.5d", 0x8000, 0x4000, CRC(b1529782) SHA1(8e0e92aae4c8dd8720414372aa767054cc316a0f) )

	ROM_REGION( 0x2000, "bgmaps", 0 )
	ROM_LOAD( "epr10079.8d", 0x0000, 0x2000, CRC(3c61a42c) SHA1(68ea6b5d2f3c6a9e5308c08dde20424f20021a73) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "epr10071.7m", 0x0000, 0x4000, CRC(5f55527a) SHA1(ec1ba8f95ac47a0c783e117ef4af6fe0ab5925b5) )
	ROM_LOAD( "epr10070.7k", 0x4000, 0x4000, CRC(97f35a23) SHA1(869553a334e1b3ba900a8b9c9eaf25fbc6ab31dd) )
	ROM_LOAD( "epr10069.7j", 0x8000, 0x4000, CRC(c0c3deaf) SHA1(8bf2e2146b794a330a079dd080f0586500964b1a) )
ROM_END

} // anonymous namespace

/*************************************
 *
 *  Game drivers
 *  (Bomb Jack)
 *
 *************************************/
GAME( 1984, bombjack,   0,        bombjack, bombjack, bombjack_state, empty_init, ROT90, "Tehkan",                  "Bomb Jack",                 MACHINE_SUPPORTS_SAVE )
GAME( 1984, bombjack2,  bombjack, bombjack, bombjack, bombjack_state, empty_init, ROT90, "Tehkan",                  "Bomb Jack (earlier)",       MACHINE_SUPPORTS_SAVE )
GAME( 1984, bombjackt,  bombjack, bombjack, bombjack, bombjack_state, empty_init, ROT90, "Tehkan (Tecfri license)", "Bomb Jack (Tecfri, Spain)", MACHINE_SUPPORTS_SAVE ) // official license
GAME( 1985, bombjackbl, bombjack, bombjack, bombjack, bombjack_state, empty_init, ROT90, "bootleg",                 "Bomb Jack (bootleg)",       MACHINE_SUPPORTS_SAVE )

/*************************************
 *
 *  Game drivers
 *  (Calorie Kun)
 *
 *************************************/
GAME( 1986, calorie,  0,       calorie,  calorie, calorie_state, empty_init,    ROT0, "Sega",    "Calorie Kun vs Moguranian",           MACHINE_SUPPORTS_SAVE )
GAME( 1986, calorieb, calorie, calorieb, calorie, calorie_state, init_calorieb, ROT0, "bootleg", "Calorie Kun vs Moguranian (bootleg)", MACHINE_SUPPORTS_SAVE )
