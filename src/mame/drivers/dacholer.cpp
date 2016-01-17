// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/******************************************************************************

    Dacholer    (c) 1983 Nichibutsu
    Kick Boy    (c) 1983 Nichibutsu

    Driver by Pierpaolo Prazzoli

    TODO:
      - is the background color pen correct for both games? (Dacholer probably
        just use a different color prom data)

    Mods by Tomasz Slanina (2008.06.12):
      - fixed sound cpu interrupts (mode 2 (two vectors)+ nmi)
      - added sound and music.
      - ay/msm clocks are arbitrary
      - just a guess - upper nibble of byte from port 3 _probably_
        contains sound command (sound cpu writes it to port c)

    Itazura Tenshi (Japan Ver.)
    (c)1984 Nichibutsu / Alice



    --- Team Japump!!! ---
    Dumped by Chack'n
    Driver written by Hau

    based on driver from drivers/dacholer.c by Pierpaolo Prazzoli
    note:
    Sound test does not work.
******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/msm5205.h"
#include "sound/ay8910.h"
#include "video/resnet.h"

class dacholer_state : public driver_device
{
public:
	dacholer_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this,"audiocpu"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	/* memory pointers */
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_spriteram;

	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	int      m_bg_bank;
	UINT8    m_scroll_x;
	UINT8    m_scroll_y;

	/* sound-related */
	int m_msm_data;
	int m_msm_toggle;
	UINT8 m_snd_interrupt_enable;
	UINT8 m_music_interrupt_enable;
	UINT8 m_snd_ack;

	DECLARE_WRITE8_MEMBER(bg_scroll_x_w);
	DECLARE_WRITE8_MEMBER(bg_scroll_y_w);
	DECLARE_WRITE8_MEMBER(background_w);
	DECLARE_WRITE8_MEMBER(foreground_w);
	DECLARE_WRITE8_MEMBER(bg_bank_w);
	DECLARE_WRITE8_MEMBER(coins_w);
	DECLARE_WRITE8_MEMBER(snd_w);
	DECLARE_WRITE8_MEMBER(main_irq_ack_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);
	DECLARE_WRITE8_MEMBER(snd_ack_w);
	DECLARE_WRITE8_MEMBER(snd_irq_w);
	DECLARE_WRITE8_MEMBER(music_irq_w);
	DECLARE_CUSTOM_INPUT_MEMBER(snd_ack_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(dacholer);
	UINT32 screen_update_dacholer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
};

TILE_GET_INFO_MEMBER(dacholer_state::get_bg_tile_info)
{
	SET_TILE_INFO_MEMBER(1, m_bgvideoram[tile_index] + m_bg_bank * 0x100, 0, 0);
}

TILE_GET_INFO_MEMBER(dacholer_state::get_fg_tile_info)
{
	SET_TILE_INFO_MEMBER(0, m_fgvideoram[tile_index], 0, 0);
}

void dacholer_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dacholer_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dacholer_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

WRITE8_MEMBER(dacholer_state::bg_scroll_x_w)
{
	m_scroll_x = data;
}

WRITE8_MEMBER(dacholer_state::bg_scroll_y_w)
{
	m_scroll_y = data;
}

void dacholer_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs, code, attr, sx, sy, flipx, flipy;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		code = m_spriteram[offs + 1];
		attr = m_spriteram[offs + 2];

		flipx = attr & 0x10;
		flipy = attr & 0x20;

		sx = (m_spriteram[offs + 3] - 128) + 256 * (attr & 0x01);
		sy = 255 - m_spriteram[offs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,
				0,
				flipx,flipy,
				sx,sy,0);
	}
}

UINT32 dacholer_state::screen_update_dacholer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (flip_screen())
	{
		m_bg_tilemap->set_scrollx(0, 256 - m_scroll_x);
		m_bg_tilemap->set_scrolly(0, 256 - m_scroll_y);
	}
	else
	{
		m_bg_tilemap->set_scrollx(0, m_scroll_x);
		m_bg_tilemap->set_scrolly(0, m_scroll_y);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

WRITE8_MEMBER(dacholer_state::background_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(dacholer_state::foreground_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(dacholer_state::bg_bank_w)
{
	if ((data & 3) != m_bg_bank)
	{
		m_bg_bank = data & 3;
		m_bg_tilemap->mark_all_dirty();
	}

	flip_screen_set(data & 0xc); // probably one bit for flipx and one for flipy

}

WRITE8_MEMBER(dacholer_state::coins_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);

	output().set_led_value(0, data & 4);
	output().set_led_value(1, data & 8);
}

WRITE8_MEMBER(dacholer_state::snd_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(dacholer_state::main_irq_ack_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, dacholer_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8800, 0x97ff) AM_RAM
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x400) AM_RAM_WRITE(background_w) AM_SHARE("bgvideoram")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(foreground_w) AM_SHARE("fgvideoram")
	AM_RANGE(0xe000, 0xe0ff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( itaten_main_map, AS_PROGRAM, 8, dacholer_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xb7ff) AM_RAM
	AM_IMPORT_FROM( main_map )
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, dacholer_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSWA")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSWB")
	AM_RANGE(0x05, 0x05) AM_READNOP // watchdog in itaten
	AM_RANGE(0x20, 0x20) AM_WRITE(coins_w)
	AM_RANGE(0x21, 0x21) AM_WRITE(bg_bank_w)
	AM_RANGE(0x22, 0x22) AM_WRITE(bg_scroll_x_w)
	AM_RANGE(0x23, 0x23) AM_WRITE(bg_scroll_y_w)
	AM_RANGE(0x24, 0x24) AM_WRITE(main_irq_ack_w)
	AM_RANGE(0x27, 0x27) AM_WRITE(snd_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( snd_map, AS_PROGRAM, 8, dacholer_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0xd000, 0xe7ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( itaten_snd_map, AS_PROGRAM, 8, dacholer_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END


WRITE8_MEMBER(dacholer_state::adpcm_w)
{
	m_msm_data = data;
	m_msm_toggle = 0;
}

WRITE8_MEMBER(dacholer_state::snd_ack_w)
{
	m_snd_ack = data;
}

CUSTOM_INPUT_MEMBER(dacholer_state::snd_ack_r)
{
	return m_snd_ack;       //guess ...
}

WRITE8_MEMBER(dacholer_state::snd_irq_w)
{
	m_snd_interrupt_enable = data;
}

WRITE8_MEMBER(dacholer_state::music_irq_w)
{
	m_music_interrupt_enable = data;
}

static ADDRESS_MAP_START( snd_io_map, AS_IO, 8, dacholer_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(soundlatch_byte_r, soundlatch_clear_byte_w )
	AM_RANGE(0x04, 0x04) AM_WRITE(music_irq_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(snd_irq_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(snd_ack_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(adpcm_w)
	AM_RANGE(0x86, 0x87) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x8a, 0x8b) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0x8e, 0x8f) AM_DEVWRITE("ay3", ay8910_device, data_address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( itaten_snd_io_map, AS_IO, 8, dacholer_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(soundlatch_byte_r, soundlatch_clear_byte_w )
	AM_RANGE(0x86, 0x87) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x8a, 0x8b) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0x8e, 0x8f) AM_DEVWRITE("ay3", ay8910_device, data_address_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( dacholer )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           /* table at 0x0a8c */
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           /* table at 0x0a94 */
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, dacholer_state,snd_ack_r, NULL)

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            /* table at 0x0a9c */
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
//  PORT_DIPNAME( 0x0c, 0x0c, "1st Bonus Life" )            /* table at 0x0aa0 */
//  PORT_DIPSETTING(    0x0c, "20k" )
//  PORT_DIPSETTING(    0x08, "30k" )
//  PORT_DIPSETTING(    0x04, "40k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
//  PORT_DIPNAME( 0x30, 0x30, "Next Bonus Lifes" )          /* table at 0x0aa8 */
//  PORT_DIPSETTING(    0x30, "+50k" )
//  PORT_DIPSETTING(    0x20, "+70k" )
//  PORT_DIPSETTING(    0x10, "+100k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3c, 0x3c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x3c, "20k 70k then every 50k" )
	PORT_DIPSETTING(    0x38, "30k 80k then every 50k" )
	PORT_DIPSETTING(    0x34, "40k 90k then every 50k" )
//  PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x2c, "20k 90k then every 70k" )
	PORT_DIPSETTING(    0x28, "30k 100k then every 70k" )
	PORT_DIPSETTING(    0x24, "40k 110k then every 70k" )
//  PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	PORT_DIPSETTING(    0x1c, "20k 120k then every 100k" )
	PORT_DIPSETTING(    0x18, "30k 130k then every 100k" )
	PORT_DIPSETTING(    0x14, "40k 140k then every 100k" )
//  PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0c, "20k only" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPSETTING(    0x04, "40k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kickboy )
	PORT_INCLUDE(dacholer)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           /* table at 0x0f71 - same as in 'dacholer' */
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           /* table at 0x0f79 */
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )            /* table at 0x0f81 */
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)" )
//  PORT_DIPNAME( 0x0c, 0x0c, "1st Bonus Life" )            /* table at 0x0f85 - same as in 'dacholer' */
//  PORT_DIPSETTING(    0x0c, "20k" )
//  PORT_DIPSETTING(    0x08, "30k" )
//  PORT_DIPSETTING(    0x04, "40k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
//  PORT_DIPNAME( 0x30, 0x30, "Next Bonus Lifes" )          /* table at 0x0f8d */
//  PORT_DIPSETTING(    0x30, "+30k" )
//  PORT_DIPSETTING(    0x20, "+50k" )
//  PORT_DIPSETTING(    0x10, "+70k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3c, 0x3c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x3c, "20k 50k then every 30k" )
	PORT_DIPSETTING(    0x38, "30k 60k then every 30k" )
	PORT_DIPSETTING(    0x34, "40k 70k then every 30k" )
//  PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x2c, "20k 70k then every 50k" )
	PORT_DIPSETTING(    0x28, "30k 80k then every 50k" )
	PORT_DIPSETTING(    0x24, "40k 90k then every 50k" )
//  PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	PORT_DIPSETTING(    0x1c, "20k 90k then every 70k" )
	PORT_DIPSETTING(    0x18, "30k 100k then every 70k" )
	PORT_DIPSETTING(    0x14, "40k 110k then every 70k" )
//  PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0c, "20k only" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPSETTING(    0x04, "40k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )       /* stored at 0x920f */
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( itaten )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "30k then every 50k" )
	PORT_DIPSETTING(    0x20, "60k then every 50k" )
	PORT_DIPSETTING(    0x10, "30k then every 90k" )
	PORT_DIPSETTING(    0x00, "60k then every 90k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*16*4
};

static GFXDECODE_START( dacholer )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   0x10, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x10, 1 )
GFXDECODE_END

static GFXDECODE_START( itaten )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x10, 1 )
GFXDECODE_END


INTERRUPT_GEN_MEMBER(dacholer_state::sound_irq)
{
	if (m_music_interrupt_enable == 1)
	{
		device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x30);
	}
}

WRITE_LINE_MEMBER(dacholer_state::adpcm_int)
{
	if (m_snd_interrupt_enable == 1 || (m_snd_interrupt_enable == 0 && m_msm_toggle == 1))
	{
		m_msm->data_w(m_msm_data >> 4);
		m_msm_data <<= 4;
		m_msm_toggle ^= 1;
		if (m_msm_toggle == 0)
		{
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0x38);
		}
	}
}

void dacholer_state::machine_start()
{
	save_item(NAME(m_bg_bank));
	save_item(NAME(m_msm_data));
	save_item(NAME(m_msm_toggle));
	save_item(NAME(m_snd_interrupt_enable));
	save_item(NAME(m_music_interrupt_enable));
	save_item(NAME(m_snd_ack));
}

void dacholer_state::machine_reset()
{
	m_msm_data = 0;
	m_msm_toggle = 0;

	m_bg_bank = 0;
	m_snd_interrupt_enable = 0;
	m_music_interrupt_enable = 0;
	m_snd_ack = 0;
}

/* guess: use the same resistor values as Crazy Climber (needs checking on the real HW) */
PALETTE_INIT_MEMBER(dacholer_state, dacholer)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_rg[3], weights_b[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, nullptr, nullptr, 0, 0);

	for (i = 0;i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

/* note: clocks are taken from itaten sound reference recording */
static MACHINE_CONFIG_START( dacholer, dacholer_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/4)  /* ? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dacholer_state,  irq0_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_19_968MHz/8) /* ? */
	MCFG_CPU_PROGRAM_MAP(snd_map)
	MCFG_CPU_IO_MAP(snd_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dacholer_state, sound_irq)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-1-16)
	MCFG_SCREEN_UPDATE_DRIVER(dacholer_state, screen_update_dacholer)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(dacholer_state, dacholer)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dacholer)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_19_968MHz/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_19_968MHz/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay3", AY8910, XTAL_19_968MHz/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz)
	MCFG_MSM5205_VCLK_CB(WRITELINE(dacholer_state, adpcm_int))          /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)  /* 1 / 96 = 3906.25Hz playback  - guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( itaten, dacholer )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(itaten_main_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(itaten_snd_map)
	MCFG_CPU_IO_MAP(itaten_snd_io_map)
	MCFG_CPU_VBLANK_INT_REMOVE()

	MCFG_GFXDECODE_MODIFY("gfxdecode", itaten)

	MCFG_DEVICE_REMOVE("msm")
MACHINE_CONFIG_END

ROM_START( dacholer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dacholer8.rom",  0x0000, 0x2000, CRC(8b73a441) SHA1(6de9e4845b9063af8df42aa82ad536737190582c) )
	ROM_LOAD( "dacholer9.rom",  0x2000, 0x2000, CRC(9499289f) SHA1(bcfe554eb1f8e686d193050c18278b6bf93f179f) )
	ROM_LOAD( "dacholer10.rom", 0x4000, 0x2000, CRC(39d37281) SHA1(daaf84079dd18dd854946e066e2dcde994bcbba4) )
	ROM_LOAD( "dacholer11.rom", 0x6000, 0x2000, CRC(bb781ea4) SHA1(170966c4bcd0246968850d908a69f81ea1e136d5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dacholer12.rom", 0x0000, 0x2000, CRC(cc3a4b68) SHA1(29344dc10c5d236f9a452196b3809565b4101327) )
	ROM_LOAD( "dacholer13.rom", 0x2000, 0x2000, CRC(aa18e126) SHA1(e6af334188d0edbc37a7fb4a00a325b2039172b7) )
	ROM_LOAD( "dacholer14.rom", 0x4000, 0x2000, CRC(3b0131c7) SHA1(338ca2c2c7480e1cd0bb15ee6b90d683ce06f0fd) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dacholer7.rom", 0x0000, 0x2000, CRC(fd649d36) SHA1(77d78eef44f348b635dbc0711e662a5236c00d51) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "dacholer1.rom", 0x0000, 0x2000, CRC(9cca0fd2) SHA1(3ca1b4cca9611232df1195ae6ac172a79c8368c3) )
	ROM_LOAD( "dacholer2.rom", 0x2000, 0x2000, CRC(c1322b27) SHA1(8022f59b8ae10a7a911563b01bffc2d5646108a5) )
	ROM_LOAD( "dacholer3.rom", 0x4000, 0x2000, CRC(9e1e7198) SHA1(7a75da1ae09f6cf095976b48f462ede42625b244) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "dacholer5.rom", 0x0000, 0x2000, CRC(dd4818f0) SHA1(718236932248512f8779f640e0367b5d92e6497e) )
	ROM_LOAD( "dacholer4.rom", 0x2000, 0x2000, CRC(7f338ae0) SHA1(9206ed044feb44c55990803cdf608dd899e976ff) )
	ROM_LOAD( "dacholer6.rom", 0x4000, 0x2000, CRC(0a6d4ec4) SHA1(419ea1f6ead3afb2de98432d9f8ead7447842a1e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "k.13d", 0x0000, 0x0020, BAD_DUMP CRC(82f87a36) SHA1(5dc2059eb5b6cd541b014347c36198b8838d98fa) ) //taken from Kick Boy
ROM_END

ROM_START( kickboy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "k_1.5k", 0x0000, 0x2000, CRC(525746f1) SHA1(4044f880f271f77b56b2d8964ab97d34fb507c7a) )
	ROM_LOAD( "k_2.5l", 0x2000, 0x2000, CRC(9d091725) SHA1(827cea1c371094720b47fda271945cee20c9d956) )
	ROM_LOAD( "k_3.5m", 0x4000, 0x2000, CRC(d61b6ff6) SHA1(071ab4c05ed54526144f2ba751c111e8c4bdc61a) )
	ROM_LOAD( "k_4.5n", 0x6000, 0x2000, CRC(a8985bfe) SHA1(a8e466a7df381dfc8dd2e3483eba0215bfec7551) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "k_1.6g", 0x0000, 0x2000, CRC(cc3a4b68) SHA1(29344dc10c5d236f9a452196b3809565b4101327) )
	ROM_LOAD( "k_2.6h", 0x2000, 0x2000, CRC(aa18e126) SHA1(e6af334188d0edbc37a7fb4a00a325b2039172b7) )
	ROM_LOAD( "k_3.6j", 0x4000, 0x2000, CRC(3b0131c7) SHA1(338ca2c2c7480e1cd0bb15ee6b90d683ce06f0fd) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "k_7.12j", 0x0000, 0x2000, CRC(22be46e8) SHA1(d92b3913d8eba881c69acd1d85ca73ee58489fae) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "k_3.13a",  0x0000, 0x2000, CRC(7eac2a64) SHA1(b4a44770bbded59cd572ac5d0ae178affc8cdab8) )
	ROM_LOAD( "k_2.12a",  0x2000, 0x2000, CRC(b8829572) SHA1(01009ec63449c809608923fd9dcecd82b29c5d6d) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "k_5.2d", 0x0000, 0x2000, CRC(4b769a1c) SHA1(fde17dcd4b7cda9cc54572e81bc2f0e48c19277d) )
	ROM_LOAD( "k_4.1d", 0x2000, 0x2000, CRC(45199750) SHA1(a04b4d6d0defa613d269625b089d28dc68d5b73a) )
	ROM_LOAD( "k_6.3d", 0x4000, 0x2000, CRC(d1795506) SHA1(e0f7a64e301cf43c4739031461dba16aa44100a1) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "k.13d", 0x0000, 0x0020, CRC(82f87a36) SHA1(5dc2059eb5b6cd541b014347c36198b8838d98fa) )
ROM_END

/*
--------------------------------
IT A-1
CPU  :LH0080 Z80,LH0080A Z80A
Sound:AY-3-8910 x3
OSC  :16.000MHz
--------------------------------
1.5K         [84c8a010] 2764
2.5L         [19946038]  |
3.5M         [4f9e26fd] /

6.6G         [dfcb1a3e] 2764
7.6H         [844e78d6] 2732

AF-1.3N      [5638e485] 82S123


--------------------------------
ITA-EXP
--------------------------------
4.1F         [35f85aeb] 2764
5.1E         [6cf30924] /


--------------------------------
IT A-2
OSC  :19.968 ?
--------------------------------
8.10A        [c32b0859] 2764
9.11A        [919cac5e]  |
10.12A       [d2b60e5d]  |
11.13A       [ed3279d5] /

12.1D        [f0f64636] 2764
13.2D        [d32559f5]  |
14.3D        [8c532c74]  |
15.4D        [d119b483] /

16.12J       [8af2bfb8] 2764

AF-2.1H      [e1cac297] 82S123
AF-3.13D     [875429ba] /
--------------------------------
*/


ROM_START( itaten )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5k",  0x0000, 0x2000, CRC(84c8a010) SHA1(52d78ac70b3d5e905a11efd76acd99810c56e467) )
	ROM_LOAD( "2.5l",  0x2000, 0x2000, CRC(19946038) SHA1(74f76096e676535ead4386755fce853caac7673b) )
	ROM_LOAD( "3.5m",  0x4000, 0x2000, CRC(4f9e26fd) SHA1(33062724c46108611c9db16fdbf2cb9feed7e213) )
	ROM_LOAD( "4.1f",  0x6000, 0x2000, CRC(35f85aeb) SHA1(ecd8f62e304d1277332a5a2b7ec6aace9f77d8ad) )
	ROM_LOAD( "5.1e",  0x8000, 0x2000, CRC(6cf30924) SHA1(5e82e9aa0811ec1b853d300368c5ceec44938363) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6.6g",  0x0000, 0x2000, CRC(dfcb1a3e) SHA1(cee0906cfbddd0254a947737da1cfbe47c445c32) )
	ROM_LOAD( "7.6h",  0x2000, 0x1000, CRC(844e78d6) SHA1(11b48af650809f8504b56e9a8e53c9f043782c5f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "16.12j", 0x0000, 0x2000, CRC(8af2bfb8) SHA1(6744db0deb4fda7920fcfddf7f9c1ed6681d3622) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "11.13a", 0x0000, 0x2000, CRC(ed3279d5) SHA1(e4bcae8038739c588f896ff35fa95288979fa683) )
	ROM_LOAD( "10.12a", 0x2000, 0x2000, CRC(d2b60e5d) SHA1(c833ac6e5d4d0a244ace600f5f02c6f43d3bd34b) )
	ROM_LOAD( "9.11a",  0x4000, 0x2000, CRC(919cac5e) SHA1(9602ad7618b5e4d93fa22676aa855256d4690dd1) )
	ROM_LOAD( "8.10a",  0x6000, 0x2000, CRC(c32b0859) SHA1(1bb00de55742a1f2cbbf3970b43b122114b0911b) )

	ROM_REGION( 0x8000, "gfx3", 0 )
	ROM_LOAD( "13.2d",  0x0000, 0x2000, CRC(d32559f5) SHA1(a4f05b1c8c48aad367ff675c29a9a1828c71b693) )
	ROM_LOAD( "12.1d",  0x2000, 0x2000, CRC(f0f64636) SHA1(a3354be74460e45453fea62c8dd910f98b5d2fb5) )
	ROM_LOAD( "14.3d",  0x4000, 0x2000, CRC(8c532c74) SHA1(c95786c81f82f7211f4411a9f39fd4ba4def9073) )
	ROM_LOAD( "15.4d",  0x6000, 0x2000, CRC(d119b483) SHA1(c1e403369bfbda0233ec5764fc703522e9f312a7) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "af-3.13d", 0x0000, 0x0020, CRC(875429ba) SHA1(7186e1fb15806d60fd3e704be8db94c7b6c8c058) )
	ROM_LOAD( "af-2.1h",  0x0020, 0x0020, CRC(e1cac297) SHA1(f15326d04d006d9d029a6565aebf9daf3657bc2a) )
	ROM_LOAD( "af-1.3n",  0x0040, 0x0020, CRC(5638e485) SHA1(5d892111936a8eb7646c03a17300069be9a2b442) )
ROM_END


GAME( 1983, dacholer, 0, dacholer, dacholer, driver_device, 0, ROT0, "Nichibutsu",         "Dacholer",               MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1983, kickboy,  0, dacholer, kickboy, driver_device,  0, ROT0, "Nichibutsu",         "Kick Boy",               MACHINE_SUPPORTS_SAVE )
GAME( 1984, itaten,   0, itaten,   itaten, driver_device,   0, ROT0, "Nichibutsu / Alice", "Itazura Tenshi (Japan)", MACHINE_SUPPORTS_SAVE )
