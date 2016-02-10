// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

Wyvern F-0 (1985, Taito)

driver by Luca Elia

Typical Taito mid-80s hardware but with dual video output.

Sound board:    Z80, 2 x YM2149, OKI M5232
CPU board:      Z80, ROM and RAM, 68705P5 MCU (protected)
OBJ board:      ROMs and RAM
Video board:    ROMs and RAM, 4 x Fujitsu MB112S146 (also used on arkanoid, lkage)

The rest is just common logic, there are no custom chips.

The cabinet uses a half-silvered mirror to mix the images from two screens for a pseudo-3D effect:

http://www.higenekodo.jp/untiku/wy.htm

Backgrounds and enemies on the ground are displayed in the lower screen, while
player ship and enemies in the air are displayed in the upper screen.
And the cabinet also has two speakers. The sound of enemies on the ground is heard
from the bottom speaker and the sound of enemies in the air is heard from the top speaker.

Actual game video: http://www.nicozon.net/watch/sm10823430

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"

class wyvernf0_state : public driver_device
{
public:
	wyvernf0_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgram(*this,"bgram"),
		m_fgram(*this,"fgram"),
		m_scrollram(*this,"scrollram"),
		m_spriteram(*this,"spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	// memory pointers
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_fgram;
	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_spriteram;

	// video-related
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	std::unique_ptr<UINT8[]>      m_objram;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	DECLARE_WRITE8_MEMBER(bgram_w);
	DECLARE_WRITE8_MEMBER(fgram_w);
	DECLARE_VIDEO_START(wyvernf0);
	UINT32 screen_update_wyvernf0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, bool is_foreground );

	// misc
	int         m_sound_nmi_enable;
	int         m_pending_nmi;
	UINT8       m_rombank;
	UINT8       m_rambank;

	DECLARE_WRITE8_MEMBER(rambank_w);
	DECLARE_WRITE8_MEMBER(rombank_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_MACHINE_START(wyvernf0);
	DECLARE_MACHINE_RESET(wyvernf0);
	TIMER_CALLBACK_MEMBER(nmi_callback);

	// MCU
	UINT8       m_mcu_val, m_mcu_ready;

	DECLARE_READ8_MEMBER(fake_mcu_r);
	DECLARE_WRITE8_MEMBER(fake_mcu_w);
	DECLARE_READ8_MEMBER(fake_status_r);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
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

WRITE8_MEMBER(wyvernf0_state::bgram_w)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(wyvernf0_state::fgram_w)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(wyvernf0_state::get_bg_tile_info)
{
	int offs = tile_index * 2;
	int code = m_bgram[offs] + (m_bgram[offs+1] << 8);
	int color = 0 + ((code & 0x3000) >> 12);

	SET_TILE_INFO_MEMBER(1, code, color, TILE_FLIPXY(code >> 14));
}
TILE_GET_INFO_MEMBER(wyvernf0_state::get_fg_tile_info)
{
	int offs = tile_index * 2;
	int code = m_fgram[offs] + (m_fgram[offs+1] << 8);
	int color = 8 + ((code & 0x3000) >> 12);

	SET_TILE_INFO_MEMBER(1, code, color, TILE_FLIPXY(code >> 14));
}

VIDEO_START_MEMBER(wyvernf0_state,wyvernf0)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wyvernf0_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wyvernf0_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

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
	UINT8 *sprram = &m_spriteram[ is_foreground ? m_spriteram.bytes()/2 : 0 ];

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

UINT32 wyvernf0_state::screen_update_wyvernf0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

READ8_MEMBER(wyvernf0_state::fake_mcu_r)
{
	int result = 0;

	if ((m_mcu_val & 0x73) == 0x73)
		result = 0x42;  // at boot

	return result;
}

WRITE8_MEMBER(wyvernf0_state::fake_mcu_w)
{
	m_mcu_val = data;
}

READ8_MEMBER(wyvernf0_state::fake_status_r)
{
	// bit 0 = ok to write
	// bit 1 = ok to read
	return 0x03;
}


/***************************************************************************

    Memory Maps

***************************************************************************/

// D100
WRITE8_MEMBER(wyvernf0_state::rambank_w)
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
WRITE8_MEMBER(wyvernf0_state::rombank_w)
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
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	else
		m_pending_nmi = 1;
}

WRITE8_MEMBER(wyvernf0_state::sound_command_w)
{
	soundlatch_byte_w(space, 0, data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(wyvernf0_state::nmi_callback),this), data);
}

WRITE8_MEMBER(wyvernf0_state::nmi_disable_w)
{
	m_sound_nmi_enable = 0;
}

WRITE8_MEMBER(wyvernf0_state::nmi_enable_w)
{
	m_sound_nmi_enable = 1;
	if (m_pending_nmi)
	{
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_pending_nmi = 0;
	}
}

static ADDRESS_MAP_START( wyvernf0_map, AS_PROGRAM, 8, wyvernf0_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM

	AM_RANGE(0x9000, 0x9fff) AM_RAMBANK("rambank")

	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("rombank")

	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(fgram_w) AM_SHARE("fgram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(bgram_w) AM_SHARE("bgram")

	AM_RANGE(0xd000, 0xd000) AM_WRITENOP // d000 write (02)
	AM_RANGE(0xd100, 0xd100) AM_WRITE(rambank_w)
	AM_RANGE(0xd200, 0xd200) AM_WRITE(rombank_w)

	AM_RANGE(0xd300, 0xd303) AM_RAM AM_SHARE("scrollram")

	AM_RANGE(0xd400, 0xd400) AM_READWRITE(fake_mcu_r, fake_mcu_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(fake_status_r)

	AM_RANGE(0xd500, 0xd5ff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0xd600, 0xd600) AM_READ_PORT("DSW1")
	AM_RANGE(0xd601, 0xd601) AM_READ_PORT("DSW2")
	AM_RANGE(0xd602, 0xd602) AM_READ_PORT("DSW3")
	AM_RANGE(0xd603, 0xd603) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd604, 0xd604) AM_READ_PORT("JOY1")
	AM_RANGE(0xd605, 0xd605) AM_READ_PORT("FIRE1")
	AM_RANGE(0xd606, 0xd606) AM_READ_PORT("JOY2")
	AM_RANGE(0xd607, 0xd607) AM_READ_PORT("FIRE2")

	AM_RANGE(0xd610, 0xd610) AM_READWRITE(soundlatch_byte_r, sound_command_w)
	// d613 write (FF -> 00 at boot)

	AM_RANGE(0xd800, 0xdbff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0xdc00, 0xdc00) AM_WRITENOP    // irq ack?
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, wyvernf0_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc801) AM_DEVWRITE("ay1", ym2149_device, address_data_w)
	AM_RANGE(0xc802, 0xc803) AM_DEVWRITE("ay2", ym2149_device, address_data_w)
	AM_RANGE(0xc900, 0xc90d) AM_DEVWRITE("msm", msm5232_device, write)
	// ca00 write
	// cb00 write
	// cc00 write
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(soundlatch_byte_r, soundlatch_byte_w)
	AM_RANGE(0xd200, 0xd200) AM_WRITE(nmi_enable_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(nmi_disable_w)
	AM_RANGE(0xd600, 0xd600) AM_RAM // VOL/BAL?
	AM_RANGE(0xe000, 0xefff) AM_ROM // space for diagnostics ROM
ADDRESS_MAP_END


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

static GFXDECODE_START( wyvernf0 )
	GFXDECODE_ENTRY( "sprites", 0, layout_8x8x4, 0, 32 ) // [0] sprites
	GFXDECODE_ENTRY( "tiles",   0, layout_8x8x4, 0, 32 ) // [1] tilemaps
GFXDECODE_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

MACHINE_START_MEMBER(wyvernf0_state,wyvernf0)
{
	UINT8 *ROM = memregion("rombank")->base();
	membank("rombank")->configure_entries(0, 8, ROM, 0x2000);

	// sprite codes lookup in banked RAM
	m_objram = std::make_unique<UINT8[]>(0x1000 * 2);
	save_pointer(NAME(m_objram.get()), 0x1000 * 2);
	membank("rambank")->configure_entries(0, 2, m_objram.get(), 0x1000);

	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_pending_nmi));
	save_item(NAME(m_rombank));
	save_item(NAME(m_rambank));
	save_item(NAME(m_mcu_val));
	save_item(NAME(m_mcu_ready));
}

MACHINE_RESET_MEMBER(wyvernf0_state,wyvernf0)
{
	m_sound_nmi_enable = 0;
	m_pending_nmi = 0;
	m_rombank = 0;
	m_rambank = 0;
	m_mcu_val = 0;
	m_mcu_ready = 0;
}

static MACHINE_CONFIG_START( wyvernf0, wyvernf0_state )

	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 6000000) // ?
	MCFG_CPU_PROGRAM_MAP(wyvernf0_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", wyvernf0_state, irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) // ?
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(wyvernf0_state, irq0_line_hold, 60*2)  // IRQ generated by ??? (drives music tempo), NMI by main cpu

//  MCFG_CPU_ADD("mcu", M68705, 4000000) // ?
//  MCFG_CPU_PROGRAM_MAP(mcu_map)

//  MCFG_QUANTUM_TIME(attotime::from_hz(6000)) // 100 CPU slices per second to synchronize between the MCU and the main CPU

	MCFG_MACHINE_START_OVERRIDE(wyvernf0_state,wyvernf0)
	MCFG_MACHINE_RESET_OVERRIDE(wyvernf0_state,wyvernf0)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(wyvernf0_state, screen_update_wyvernf0)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wyvernf0)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	MCFG_VIDEO_START_OVERRIDE(wyvernf0_state,wyvernf0)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	// coin, fire, lift-off
	MCFG_SOUND_ADD("ay1", YM2149, 3000000) // YM2149 clock ??, pin 26 ??
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	// lift-off, explosion (saucers), boss alarm
	MCFG_SOUND_ADD("ay2", YM2149, 3000000) // YM2149 clock ??, pin 26 ??
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	// music
	MCFG_SOUND_ADD("msm", MSM5232, 2000000) // ?
	MCFG_MSM5232_SET_CAPACITORS(0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6) /* default 0.39 uF capacitors (not verified) */
	MCFG_SOUND_ROUTE(0, "mono", 1.0)    // pin 28  2'-1
	MCFG_SOUND_ROUTE(1, "mono", 1.0)    // pin 29  4'-1
	MCFG_SOUND_ROUTE(2, "mono", 1.0)    // pin 30  8'-1
	MCFG_SOUND_ROUTE(3, "mono", 1.0)    // pin 31 16'-1
	MCFG_SOUND_ROUTE(4, "mono", 1.0)    // pin 36  2'-2
	MCFG_SOUND_ROUTE(5, "mono", 1.0)    // pin 35  4'-2
	MCFG_SOUND_ROUTE(6, "mono", 1.0)    // pin 34  8'-2
	MCFG_SOUND_ROUTE(7, "mono", 1.0)    // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped
MACHINE_CONFIG_END

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

	ROM_REGION( 0x0800, "mcu", 0 )  // protected 68705P5 MCU
	ROM_LOAD( "a39_mcu.icxx", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x10000, "sprites", 0 ) // sprites
	ROM_LOAD( "a39_11.ic99", 0x0000, 0x4000, CRC(af70e1dc) SHA1(98dba673750cdfdf25c119c24da10428eff6591b) )
	ROM_LOAD( "a39_10.ic78", 0x4000, 0x4000, CRC(a84380fb) SHA1(ed77892c1a789040fdfecd5903a23b8cbc1df1da) )
	ROM_LOAD( "a39_09.ic96", 0x8000, 0x4000, CRC(c0cee243) SHA1(97f66dde552c7a011ecc7ca8da0e62bc83ef8102) )
	ROM_LOAD( "a39_08.ic75", 0xc000, 0x4000, CRC(0ad69501) SHA1(29037c60bed9435568e997689d193f161f6a4f5b) )

	ROM_REGION( 0x8000, "tiles", 0 ) // tilemaps
	ROM_LOAD( "a39_14.ic99",  0x0000, 0x2000, CRC(90a66147) SHA1(8515c43980b7fa55933ca74fb23172e8c832a830) ) // wrong name?
	ROM_LOAD( "a39_14.ic73",  0x2000, 0x2000, CRC(a31f3507) SHA1(f72e089dbd700639d64e418812d4b6f4dc1dff75) )
	ROM_LOAD( "a39_13.ic100", 0x4000, 0x2000, CRC(be708238) SHA1(f12d433af7bf6010dea9454a1b3bb2990a42a372) )
	ROM_LOAD( "a39_12.ic74",  0x6000, 0x2000, CRC(1cc389de) SHA1(4213484d3a82688f312811e7a5c4d128e40584c3) )
ROM_END

GAME( 1985, wyvernf0, 0, wyvernf0, wyvernf0, driver_device, 0, ROT270, "Taito", "Wyvern F-0", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND)
