// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

Wyvern F-0 (1985, Taito)

driver by Luca Elia

Typical Taito mid-80s hardware but with dual video output.

Sound board:    Z80, 2 x YM2149, OKI M5232
CPU board:      Z80, ROM and RAM, 68705P5 MCU (protected)
OBJ board:      48MHz OSC, ROMs and RAM
Video board:    ROMs and RAM, 4 x Fujitsu MB112S146 (also used on arkanoid, lkage)

The rest is just common logic, there are no custom chips.

The cabinet uses a half-silvered mirror to mix the images from two screens for a pseudo-3D effect:

http://www.higenekodo.jp/untiku/wy.htm

Backgrounds and enemies on the ground are displayed in the lower screen, while
player ship and enemies in the air are displayed in the upper screen.
And the cabinet also has two speakers. The sound of enemies on the ground is heard
from the bottom speaker and the sound of enemies in the air is heard from the top speaker.

Actual game video: http://www.nicozon.net/watch/sm10823430

TODO:
-  TA7630;

***************************************************************************/

#include "emu.h"

#include "taito68705.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class wyvernf0_state : public driver_device
{
public:
	wyvernf0_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgram(*this,"bgram"),
		m_fgram(*this,"fgram"),
		m_scrollram(*this,"scrollram"),
		m_spriteram(*this,"spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bmcu(*this, "bmcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void wyvernf0(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;

	// video-related
	tilemap_t  *m_bg_tilemap = nullptr;
	tilemap_t  *m_fg_tilemap = nullptr;
	std::unique_ptr<uint8_t[]>    m_objram{};

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void bgram_w(offs_t offset, uint8_t data);
	void fgram_w(offs_t offset, uint8_t data);
	uint32_t screen_update_wyvernf0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, bool is_foreground );

	// misc
	int         m_sound_nmi_enable = 0;
	int         m_pending_nmi = 0;
	uint8_t       m_rombank = 0U;
	uint8_t       m_rambank = 0U;

	void rambank_w(uint8_t data);
	void rombank_w(uint8_t data);
	void sound_command_w(uint8_t data);
	void nmi_disable_w(uint8_t data);
	void nmi_enable_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(nmi_callback);

	uint8_t mcu_status_r();

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<taito68705_mcu_device> m_bmcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_map(address_map &map) ATTR_COLD;
	void wyvernf0_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

    Video

    Note:   if MAME_DEBUG is defined, pressing Z with:

                    Q       Shows the background tilemap
                    W       Shows the foreground tilemap
                    A       Shows the background sprites
                    S       Shows the foreground sprites

            Keys can be used together!

***************************************************************************/

void wyvernf0_state::bgram_w(offs_t offset, uint8_t data)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void wyvernf0_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(wyvernf0_state::get_bg_tile_info)
{
	int offs = tile_index * 2;
	int code = m_bgram[offs] + (m_bgram[offs+1] << 8);
	int color = 0 + ((code & 0x3000) >> 12);

	tileinfo.set(1, code, color, TILE_FLIPXY(code >> 14));
}
TILE_GET_INFO_MEMBER(wyvernf0_state::get_fg_tile_info)
{
	int offs = tile_index * 2;
	int code = m_fgram[offs] + (m_fgram[offs+1] << 8);
	int color = 8 + ((code & 0x3000) >> 12);

	tileinfo.set(1, code, color, TILE_FLIPXY(code >> 14));
}

void wyvernf0_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wyvernf0_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wyvernf0_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(0x12, 0xf4);
	m_bg_tilemap->set_scrolldy(   0,    0);

	m_fg_tilemap->set_scrolldx(0x10, 0xf6);
	m_fg_tilemap->set_scrolldy(   0,    0);
}

void wyvernf0_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, bool is_foreground )
{
	int offs;
/*
1st boss:
YY       XX
71 1D 02 60 <- head
81 73 02 80 <- left neck
81 71 02 A0 <- left body
61 74 02 80 <- right neck
61 72 02 A0 <- right body
71 6C 02 C0 <- tail
41 E5 02 60 <- right arm
41 E4 02 80 <- right shoulder
41 6B 02 A0 <- right wing
A1 65 02 60 <- left arm
A1 EB 02 A0 <- left wing
A1 64 02 80 <- left shoulder

player+target:
YY       XX
71 11 04 a8 <- target
71 01 0f 50 <- player

yyyyyyyy fccccccc x???pppp xxxxxxxx

*/
	uint8_t *sprram = &m_spriteram[ is_foreground ? m_spriteram.bytes()/2 : 0 ];

	// sy = 0 -> on the left
	for (offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int sx, sy, code, color;

		sx = sprram[offs + 3] - ((sprram[offs + 2] & 0x80) << 1);
		sy = 256 - 8 - sprram[offs + 0] - 23;   // center player sprite: 256 - 8 - 0x71 + dy = 256/2-32/2 -> dy = -23

//      int flipx = sprram[offs + 2] & 0x40;    // nope
		int flipx = 0;
		int flipy = sprram[offs + 1] & 0x80;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 256 - 8 - sx - 3*8;
		}
		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 256 - 8 - sy - 3*8;
		}

		code = sprram[offs + 1] & 0x7f;
		color = (sprram[offs + 2] & 0x0f);

		if (is_foreground)
		{
			code  += 0x80;
			color += 0x10;
		}

		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				int objoffs = code * 0x20 + (x + y * 4) * 2;

				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						(m_objram[objoffs + 1] << 8) + m_objram[objoffs],
						color,
						flipx, flipy,
						sx + (flipx ? 3-x : x) * 8, sy + (flipy ? 3-y : y) * 8, 0);
			}
		}
	}
}

uint32_t wyvernf0_state::screen_update_wyvernf0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))   msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))   msk |= 2;
	if (machine().input().code_pressed(KEYCODE_A))   msk |= 4;
	if (machine().input().code_pressed(KEYCODE_S))   msk |= 8;
	if (msk != 0) layers_ctrl &= msk;

	popmessage("fg:%02x %02x bg:%02x %02x ROM:%02x RAM:%02x",
		m_scrollram[0],m_scrollram[1],m_scrollram[2],m_scrollram[3],
		m_rombank, m_rambank
	);
}
#endif

	m_fg_tilemap->set_scrollx(0, m_scrollram[0]);
	m_fg_tilemap->set_scrolly(0, m_scrollram[1]);

	m_bg_tilemap->set_scrollx(0, m_scrollram[2]);
	m_bg_tilemap->set_scrolly(0, m_scrollram[3]);

	bitmap.fill(0, cliprect);

	// background monitor
	if (layers_ctrl & 1)    m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl & 4)    draw_sprites(bitmap, cliprect, false);

	// foreground monitor
	if (layers_ctrl & 8)    draw_sprites(bitmap, cliprect, true);
	if (layers_ctrl & 2)    m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/***************************************************************************

    MCU

***************************************************************************/

uint8_t wyvernf0_state::mcu_status_r()
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}


/***************************************************************************

    Memory Maps

***************************************************************************/

// D100
void wyvernf0_state::rambank_w(uint8_t data)
{
	// bit 0 Flip X/Y
	// bit 1 Flip X/Y
	// bit 5 ??? set, except at boot
	// bit 6 Coin lockout
	// bit 7 RAM bank
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

	machine().bookkeeping().coin_lockout_w(0, !(data & 0x40));
	machine().bookkeeping().coin_lockout_w(1, !(data & 0x40));

	m_rambank = data;
	membank("rambank")->set_entry((data & 0x80) ? 1 : 0);

	if (data & ~0xe3)
		logerror("%s: unknown rambank bits %02x\n", machine().describe_context(), data);
}

// D200
void wyvernf0_state::rombank_w(uint8_t data)
{
	// bit 0-2 ROM bank
	m_rombank = data;
	membank("rombank")->set_entry(data & 0x07);

	if (data & ~0x07)
		logerror("%s: unknown rombank bits %02x\n", machine().describe_context(), data);
}

TIMER_CALLBACK_MEMBER(wyvernf0_state::nmi_callback)
{
	if (m_sound_nmi_enable)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	else
		m_pending_nmi = 1;
}

void wyvernf0_state::sound_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(wyvernf0_state::nmi_callback),this), data);
}

void wyvernf0_state::nmi_disable_w(uint8_t data)
{
	m_sound_nmi_enable = 0;
}

void wyvernf0_state::nmi_enable_w(uint8_t data)
{
	m_sound_nmi_enable = 1;
	if (m_pending_nmi)
	{
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		m_pending_nmi = 0;
	}
}

void wyvernf0_state::wyvernf0_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();

	map(0x9000, 0x9fff).bankrw("rambank");

	map(0xa000, 0xbfff).bankr("rombank");

	map(0xc000, 0xc7ff).ram().w(FUNC(wyvernf0_state::fgram_w)).share("fgram");
	map(0xc800, 0xcfff).ram().w(FUNC(wyvernf0_state::bgram_w)).share("bgram");

	map(0xd000, 0xd000).nopw(); // d000 write (02)
	map(0xd100, 0xd100).w(FUNC(wyvernf0_state::rambank_w));
	map(0xd200, 0xd200).w(FUNC(wyvernf0_state::rombank_w));

	map(0xd300, 0xd303).ram().share("scrollram");

	map(0xd400, 0xd400).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0xd401, 0xd401).r(FUNC(wyvernf0_state::mcu_status_r));

	map(0xd500, 0xd5ff).ram().share("spriteram");

	map(0xd600, 0xd600).portr("DSW1");
	map(0xd601, 0xd601).portr("DSW2");
	map(0xd602, 0xd602).portr("DSW3");
	map(0xd603, 0xd603).portr("SYSTEM");
	map(0xd604, 0xd604).portr("JOY1");
	map(0xd605, 0xd605).portr("FIRE1");
	map(0xd606, 0xd606).portr("JOY2");
	map(0xd607, 0xd607).portr("FIRE2");

	map(0xd610, 0xd610).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(FUNC(wyvernf0_state::sound_command_w));
	// d613 write (FF -> 00 at boot)

	map(0xd800, 0xdbff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0xdc00, 0xdc00).nopw();    // irq ack?
}

void wyvernf0_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc801).w("ay1", FUNC(ym2149_device::address_data_w));
	map(0xc802, 0xc803).w("ay2", FUNC(ym2149_device::address_data_w));
	map(0xc900, 0xc90d).w("msm", FUNC(msm5232_device::write));
	// ca00 write
	// cb00 write
	// cc00 write
	map(0xd000, 0xd000).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));
	map(0xd200, 0xd200).w(FUNC(wyvernf0_state::nmi_enable_w));
	map(0xd400, 0xd400).w(FUNC(wyvernf0_state::nmi_disable_w));
	map(0xd600, 0xd600).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe000, 0xefff).rom(); // space for diagnostics ROM
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( wyvernf0 )
	PORT_START("DSW1")  // d600 -> 800c
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "?? 0" )
	PORT_DIPSETTING(    0x01, "?? 1" )
	PORT_DIPSETTING(    0x02, "?? 2" )
	PORT_DIPSETTING(    0x03, "?? 3" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")  // d601 -> 800d
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

	PORT_START("DSW3")  // d602 -> 800e
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPUNKNOWN( 0x04, 0x04 )   // *
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) /* Music at every other title screen */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Copyright" )
	PORT_DIPSETTING(    0x00, "Taito Corporation" )
	PORT_DIPSETTING(    0x20, "Taito Corp. 1985" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("SYSTEM")    // d603 -> 800f / 8023
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1         )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2         )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1       )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT           )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1          ) // must be 0 at boot
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2          ) // ""
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN         )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN         )

	PORT_START("JOY1")  // d604 -> 8010 / 8024
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN        )

	PORT_START("FIRE1") // d605 -> 8011 / 8025
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3        ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN        )

	PORT_START("JOY2")  // d606 -> 8012 / 8026
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN        )

	PORT_START("FIRE2") // d607 -> 8013 / 8027
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3        ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN        )
INPUT_PORTS_END

/***************************************************************************

    Graphics Layouts

***************************************************************************/

// Sprites use 2 x 2 tiles and a tile code lookup

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ STEP8(7,-1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_wyvernf0 )
	GFXDECODE_ENTRY( "sprites", 0, layout_8x8x4, 0, 32 ) // [0] sprites
	GFXDECODE_ENTRY( "tiles",   0, layout_8x8x4, 0, 32 ) // [1] tilemaps
GFXDECODE_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

void wyvernf0_state::machine_start()
{
	uint8_t *ROM = memregion("rombank")->base();
	membank("rombank")->configure_entries(0, 8, ROM, 0x2000);

	// sprite codes lookup in banked RAM
	m_objram = std::make_unique<uint8_t[]>(0x1000 * 2);
	save_pointer(NAME(m_objram), 0x1000 * 2);
	membank("rambank")->configure_entries(0, 2, m_objram.get(), 0x1000);

	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_pending_nmi));
	save_item(NAME(m_rombank));
	save_item(NAME(m_rambank));
}

void wyvernf0_state::machine_reset()
{
	m_sound_nmi_enable = 0;
	m_pending_nmi = 0;
	m_rombank = 0;
	m_rambank = 0;
}

void wyvernf0_state::wyvernf0(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 48_MHz_XTAL/8); // 6MHz D780C-2 - Clock verified
	m_maincpu->set_addrmap(AS_PROGRAM, &wyvernf0_state::wyvernf0_map);
	m_maincpu->set_vblank_int("screen", FUNC(wyvernf0_state::irq0_line_hold));

	// OSC on sound board is a custom/strange 6-pin part that outputs 8MHz, 4MHz, 2MHz (no external divider)
	Z80(config, m_audiocpu, 4_MHz_XTAL); // 4MHz - Clock verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &wyvernf0_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(wyvernf0_state::irq0_line_hold), attotime::from_hz(60*2)); // IRQ generated by ??? (drives music tempo), NMI by main cpu

	TAITO68705_MCU(config, m_bmcu, 48_MHz_XTAL/16); // 3MHz - Clock verified

	/* 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs */
	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(wyvernf0_state::screen_update_wyvernf0));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wyvernf0);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 512);
	m_palette->set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	// coin, fire, lift-off
	YM2149(config, "ay1", 2_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.25); // YM2149 2MHz clock verified, pin 26 ??

	// lift-off, explosion (saucers), boss alarm
	YM2149(config, "ay2", 2_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.25); // YM2149 2MHz clock verified, pin 26 ??

	// music
	msm5232_device &msm(MSM5232(config, "msm", 2_MHz_XTAL)); // 2MHz - Clock verified
	msm.set_capacitors(1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6); /* default 1 uF capacitors (not verified) */
	msm.add_route(0, "mono", 0.5);   // pin 28  2'-1
	msm.add_route(1, "mono", 0.5);   // pin 29  4'-1
	msm.add_route(2, "mono", 0.5);   // pin 30  8'-1
	msm.add_route(3, "mono", 0.5);   // pin 31 16'-1
	msm.add_route(4, "mono", 0.5);   // pin 36  2'-2
	msm.add_route(5, "mono", 0.5);   // pin 35  4'-2
	msm.add_route(6, "mono", 0.5);   // pin 34  8'-2
	msm.add_route(7, "mono", 0.5);   // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.25); // unknown DAC
}

/***************************************************************************

    ROMs

***************************************************************************/

ROM_START( wyvernf0 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a39_01-1.ic37", 0x0000, 0x4000, CRC(a94887ec) SHA1(0b4406290810494e88442dcec7a750c7d3cf316a) )
	ROM_LOAD( "a39_02-1.ic36", 0x4000, 0x4000, CRC(171cfdbe) SHA1(41d922df00c869b8f1f6a026dbe102afffc42cc6) )

	ROM_REGION( 0x10000, "rombank", 0 )
	ROM_LOAD( "a39_03.ic35", 0x0000, 0x4000, CRC(50314281) SHA1(0f4805f06b92c170469b7bc2c0342db919107a91) )
	ROM_LOAD( "a39_04.ic34", 0x4000, 0x4000, CRC(7a225bf9) SHA1(4f0c287051e27f5bc936736225003a685cdf8ad3) )
	ROM_LOAD( "a39_05.ic33", 0x8000, 0x4000, CRC(41f21a67) SHA1(bee4a692259c727baf5fc4f47e09efb953b1c94e) )
	ROM_LOAD( "a39_06.ic32", 0xc000, 0x4000, CRC(deb2d850) SHA1(1d1f265e320fb2a48507c3133fd8a080f7bc4846) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a39_16.ic26", 0x0000, 0x4000, CRC(5a681fb4) SHA1(e31e751a54fa9853acb462ce22dd2ff5286808f0) )
	ROM_FILL(                0xe000, 0x2000, 0xff ) // diagnostics ROM

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  // 68705P5 MCU
	ROM_LOAD( "a39_mc68705p5s.ic23", 0x0000, 0x0800, CRC(14bff574) SHA1(c91446540e7628b3e62135e2f560a118f7e0dad4) ) /* from other set, appears to be correct */

	ROM_REGION( 0x10000, "sprites", 0 ) // sprites
	ROM_LOAD( "a39_11.ic99", 0x0000, 0x4000, CRC(af70e1dc) SHA1(98dba673750cdfdf25c119c24da10428eff6591b) )
	ROM_LOAD( "a39_10.ic78", 0x4000, 0x4000, CRC(a84380fb) SHA1(ed77892c1a789040fdfecd5903a23b8cbc1df1da) )
	ROM_LOAD( "a39_09.ic96", 0x8000, 0x4000, CRC(c0cee243) SHA1(97f66dde552c7a011ecc7ca8da0e62bc83ef8102) )
	ROM_LOAD( "a39_08.ic75", 0xc000, 0x4000, CRC(0ad69501) SHA1(29037c60bed9435568e997689d193f161f6a4f5b) )

	ROM_REGION( 0x8000, "tiles", 0 ) // tilemaps
	ROM_LOAD( "a39_15.ic99",  0x0000, 0x2000, CRC(90a66147) SHA1(8515c43980b7fa55933ca74fb23172e8c832a830) ) // was listed as a39_14,ic99 but changed to a39_15.ic99
	ROM_LOAD( "a39_14.ic73",  0x2000, 0x2000, CRC(a31f3507) SHA1(f72e089dbd700639d64e418812d4b6f4dc1dff75) )
	ROM_LOAD( "a39_13.ic100", 0x4000, 0x2000, CRC(be708238) SHA1(f12d433af7bf6010dea9454a1b3bb2990a42a372) )
	ROM_LOAD( "a39_12.ic74",  0x6000, 0x2000, CRC(1cc389de) SHA1(4213484d3a82688f312811e7a5c4d128e40584c3) )
ROM_END

ROM_START( wyvernf0a ) /* Possibly the first version or even an earlier development version as A39 06 above isn't labeled as A39 06-1 */
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "soft1_c2a0.ic37", 0x0000, 0x4000, CRC(15f0beb8) SHA1(4105f7064bf94460a020aecca8795553870e3fdc) ) /* Hand written label SOFT1 C2A0 */
	ROM_LOAD( "soft2_7b60.ic36", 0x4000, 0x4000, CRC(569a40c4) SHA1(5391b6cdc854277e63e4658f79889da4a941ee42) ) /* Hand written label SOFT2 7B60 */

	ROM_REGION( 0x10000, "rombank", 0 ) /* Only EXT 4 label was hand written, the others were printed */
	ROM_LOAD( "ext1.ic35",      0x0000, 0x4000, CRC(50314281) SHA1(0f4805f06b92c170469b7bc2c0342db919107a91) ) /* == a39_03.ic35 */
	ROM_LOAD( "ext2.ic34",      0x4000, 0x4000, CRC(7a225bf9) SHA1(4f0c287051e27f5bc936736225003a685cdf8ad3) ) /* == a39_04.ic34 */
	ROM_LOAD( "ext3.ic33",      0x8000, 0x4000, CRC(41f21a67) SHA1(bee4a692259c727baf5fc4f47e09efb953b1c94e) ) /* == a39_05.ic33 */
	ROM_LOAD( "ext4_8ca8.ic32", 0xc000, 0x4000, CRC(793e36de) SHA1(2a316d832ce524250c36602ca910bb4c8befa15d) ) /* Hand written label EXT 4 8CA8 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) // ROM had hand written label
	ROM_LOAD( "sound_4182.ic26", 0x0000, 0x4000, CRC(5a681fb4) SHA1(e31e751a54fa9853acb462ce22dd2ff5286808f0) ) /* == a39_16.ic26 */
	ROM_FILL(                    0xe000, 0x2000, 0xff ) // diagnostics ROM

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  // 68705P5 MCU
	ROM_LOAD( "a39_mc68705p5s.ic23", 0x0000, 0x0800, CRC(14bff574) SHA1(c91446540e7628b3e62135e2f560a118f7e0dad4) ) /* hand written label P5 5/1 - part was unprotected */

	ROM_REGION( 0x10000, "sprites", 0 ) // sprites - These 4 ROMs had hand written labels
	ROM_LOAD( "obj4_d779.ic99", 0x0000, 0x4000, CRC(af70e1dc) SHA1(98dba673750cdfdf25c119c24da10428eff6591b) ) /* == a39_11.ic99 */
	ROM_LOAD( "obj3_5852.ic78", 0x4000, 0x4000, CRC(a84380fb) SHA1(ed77892c1a789040fdfecd5903a23b8cbc1df1da) ) /* == a39_10.ic78 */
	ROM_LOAD( "obj2_50fd.ic96", 0x8000, 0x4000, CRC(c0cee243) SHA1(97f66dde552c7a011ecc7ca8da0e62bc83ef8102) ) /* == a39_09.ic96 */
	ROM_LOAD( "obj1_bd50.ic75", 0xc000, 0x4000, CRC(0ad69501) SHA1(29037c60bed9435568e997689d193f161f6a4f5b) ) /* == a39_08.ic75 */

	ROM_REGION( 0x8000, "tiles", 0 ) // tilemaps
	ROM_LOAD( "sch_4.ic99",  0x0000, 0x2000, CRC(90a66147) SHA1(8515c43980b7fa55933ca74fb23172e8c832a830) ) /* == a39_15.ic99  */
	ROM_LOAD( "sch_3.ic73",  0x2000, 0x2000, CRC(a31f3507) SHA1(f72e089dbd700639d64e418812d4b6f4dc1dff75) ) /* == a39_14.ic73  */
	ROM_LOAD( "sch_2.ic100", 0x4000, 0x2000, CRC(be708238) SHA1(f12d433af7bf6010dea9454a1b3bb2990a42a372) ) /* == a39_13.ic100 */
	ROM_LOAD( "sch_1.ic74",  0x6000, 0x2000, CRC(1cc389de) SHA1(4213484d3a82688f312811e7a5c4d128e40584c3) ) /* == a39_12.ic74  */
ROM_END

} // anonymous namespace

GAME( 1985, wyvernf0,  0,        wyvernf0, wyvernf0, wyvernf0_state, empty_init, ROT270, "Taito Corporation", "Wyvern F-0 (Rev 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND)
GAME( 1985, wyvernf0a, wyvernf0, wyvernf0, wyvernf0, wyvernf0_state, empty_init, ROT270, "Taito Corporation", "Wyvern F-0",         MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND) // First version or earlier dev version?
