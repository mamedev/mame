// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

Mat Mania
Memetron, 1985
(copyright Taito, licensed by Technos, distributed by Memetron)

driver by Brad Oliver

MAIN BOARD:

0000-0fff RAM
1000-13ff Video RAM
1400-17ff Attribute RAM
1800-1fff ?? Only used in self-test ??
2000-21ff Background video RAM #1
2200-23ff Background attribute RAM #1
2400-25ff Background video RAM #2
2600-27ff Background attribute RAM #2
4000-ffff ROM


2008.04.04: Small note regarding DipSwitches. Locations and values have been
verified with the manual for both Mat Mania and Mania Challenge.
Exciting Hour DIPs confirmed with crazykong diplist, no manual available ATM.
Notice that the manual for Mat Mania lists DSW2:3,4 as Unused, but they
correctly affect the timer speed during the game. Also, default difficulty
in Mat Mania is Normal, while manual for Mania Challenge reports Easy.
The driver has been updated accordingly.

***************************************************************************/

#include "emu.h"

#include "taito68705.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class matmania_state : public driver_device
{
public:
	matmania_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_scroll(*this, "scroll"),
		m_pageselect(*this, "pageselect"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void matmania(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

	// memory pointers
	required_shared_ptr_array<uint8_t, 3> m_videoram;
	required_shared_ptr_array<uint8_t, 3> m_colorram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_pageselect;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// video-related
	std::unique_ptr<bitmap_ind16> m_tmpbitmap[2];

	void paletteram_w(offs_t offset, uint8_t data);
	void palette(palette_device &palette) const;

	void main_map(address_map &map) ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_map(address_map &map) ATTR_COLD;
};

class maniach_state : public matmania_state
{
public:
	maniach_state(const machine_config &mconfig, device_type type, const char *tag) :
		matmania_state(mconfig, type, tag),
		m_mcu(*this, "mcu")
	{ }

	void maniach(machine_config &config);

private:
	required_device<taito68705_mcu_device> m_mcu;

	uint8_t mcu_status_r();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

/***************************************************************************

    There are only a few differences between the video hardware of Mysterious
    Stones and Mat Mania. The tile bank select bit is different and the sprite
    selection seems to be different as well. Additionally, the palette is stored
    differently. I'm also not sure that the 2nd tile page is really used in
    Mysterious Stones.

***************************************************************************/

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mat Mania is unusual in that it has both PROMs and RAM to control the
  palette. PROMs are used for characters and background tiles, RAM for
  sprites.
  I don't know for sure how the PROMs are connected to the RGB output,
  but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

void matmania_state::palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	for (int i = 0; i < 64; i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[0], 4);
		bit1 = BIT(color_prom[0], 5);
		bit2 = BIT(color_prom[0], 6);
		bit3 = BIT(color_prom[0], 7);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[64], 0);
		bit1 = BIT(color_prom[64], 1);
		bit2 = BIT(color_prom[64], 2);
		bit3 = BIT(color_prom[64], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}



void matmania_state::paletteram_w(offs_t offset, uint8_t data)
{
	int bit0, bit1, bit2, bit3, val;

	m_paletteram[offset] = data;
	offset &= 0x0f;

	val = m_paletteram[offset];
	bit0 = BIT(val, 0);
	bit1 = BIT(val, 1);
	bit2 = BIT(val, 2);
	bit3 = BIT(val, 3);
	int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = m_paletteram[offset | 0x10];
	bit0 = BIT(val, 0);
	bit1 = BIT(val, 1);
	bit2 = BIT(val, 2);
	bit3 = BIT(val, 3);
	int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = m_paletteram[offset | 0x20];
	bit0 = BIT(val, 0);
	bit1 = BIT(val, 1);
	bit2 = BIT(val, 2);
	bit3 = BIT(val, 3);
	int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	m_palette->set_pen_color(offset + 64, rgb_t(r, g, b));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void matmania_state::video_start()
{
	int width = m_screen->width();
	int height = m_screen->height();

	// Mat Mania has a virtual screen twice as large as the visible screen
	m_tmpbitmap[0] = std::make_unique<bitmap_ind16>(width, 2 * height);
	m_tmpbitmap[1] = std::make_unique<bitmap_ind16>(width, 2 * height);
}



uint32_t matmania_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Update the tiles in the left tile RAM bank
	for (int offs = m_videoram[0].bytes() - 1; offs >= 0; offs--)
	{
		int const sx = 15 - offs / 32;
		int const sy = offs % 32;

		m_gfxdecode->gfx(1)->opaque(*m_tmpbitmap[0], m_tmpbitmap[0]->cliprect(),
				m_videoram[0][offs] + ((m_colorram[0][offs] & 0x08) << 5),
				(m_colorram[0][offs] & 0x30) >> 4,
				0, sy >= 16, // flip horizontally tiles on the right half of the bitmap
				16 * sx, 16 * sy);
	}

	// Update the tiles in the right tile RAM bank
	for (int offs = m_videoram[2].bytes() - 1; offs >= 0; offs--)
	{
		int const sx = 15 - offs / 32;
		int const sy = offs % 32;

		m_gfxdecode->gfx(1)->opaque(*m_tmpbitmap[1], m_tmpbitmap[1]->cliprect(),
				m_videoram[2][offs] + ((m_colorram[2][offs] & 0x08) << 5),
				(m_colorram[2][offs] & 0x30) >> 4,
				0, sy >= 16, // flip horizontally tiles on the right half of the bitmap
				16 * sx, 16 * sy);
	}

	// copy the temporary bitmap to the screen
	{
		int const scrolly = -*m_scroll;
		if (m_pageselect[0] & 0x01) // maniach sets 0x20 sometimes, which must have a different meaning
			copyscrollbitmap(bitmap, *m_tmpbitmap[1], 0, nullptr, 1, &scrolly, cliprect);
		else
			copyscrollbitmap(bitmap, *m_tmpbitmap[0], 0, nullptr, 1, &scrolly, cliprect);
	}


	// Draw the sprites
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		if (m_spriteram[offs] & 0x01)
		{
			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					m_spriteram[offs + 1] + ((m_spriteram[offs] & 0xf0) << 4),
					(m_spriteram[offs] & 0x08) >> 3,
					m_spriteram[offs] & 0x04, m_spriteram[offs] & 0x02,
					239 - m_spriteram[offs + 3], (240 - m_spriteram[offs + 2]) & 0xff, 0);
		}
	}


	// draw the frontmost playfield. They are characters, but draw them as sprites
	for (int offs = m_videoram[1].bytes() - 1; offs >= 0; offs--)
	{
		int const sx = 31 - offs / 32;
		int const sy = offs % 32;

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				m_videoram[1][offs] + 256 * (m_colorram[1][offs] & 0x07),
				(m_colorram[1][offs] & 0x30) >> 4,
				0, 0,
				8 * sx, 8 * sy, 0);
	}
	return 0;
}

uint32_t maniach_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Update the tiles in the left tile RAM bank
	for (int offs = m_videoram[0].bytes() - 1; offs >= 0; offs--)
	{
		int const sx = 15 - offs / 32;
		int const sy = offs % 32;

		m_gfxdecode->gfx(1)->opaque(*m_tmpbitmap[0], m_tmpbitmap[0]->cliprect(),
				m_videoram[0][offs] + ((m_colorram[0][offs] & 0x03) << 8),
				(m_colorram[0][offs] & 0x30) >> 4,
				0, sy >= 16, // flip horizontally tiles on the right half of the bitmap
				16 * sx, 16 * sy);
	}

	// Update the tiles in the right tile RAM bank
	for (int offs = m_videoram[2].bytes() - 1; offs >= 0; offs--)
	{
		int const sx = 15 - offs / 32;
		int const sy = offs % 32;

		m_gfxdecode->gfx(1)->opaque(*m_tmpbitmap[1], m_tmpbitmap[1]->cliprect(),
				m_videoram[2][offs] + ((m_colorram[2][offs] & 0x03) << 8),
				(m_colorram[2][offs] & 0x30) >> 4,
				0, sy >= 16, // flip horizontally tiles on the right half of the bitmap
				16 * sx, 16 * sy);
	}


	// copy the temporary bitmap to the screen
	{
		int const scrolly = -*m_scroll;

		if (m_pageselect[0] & 0x01) // this sets 0x20 sometimes, which must have a different meaning
			copyscrollbitmap(bitmap, *m_tmpbitmap[1], 0, nullptr, 1, &scrolly, cliprect);
		else
			copyscrollbitmap(bitmap, *m_tmpbitmap[0], 0, nullptr, 1, &scrolly, cliprect);
	}


	// Draw the sprites
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		if (m_spriteram[offs] & 0x01)
		{
			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					m_spriteram[offs + 1] + ((m_spriteram[offs] & 0xf0) << 4),
					(m_spriteram[offs] & 0x08) >> 3,
					m_spriteram[offs] & 0x04, m_spriteram[offs] & 0x02,
					239 - m_spriteram[offs + 3], (240 - m_spriteram[offs + 2]) & 0xff, 0);
		}
	}


	// draw the frontmost playfield. They are characters, but draw them as sprites
	for (int offs = m_videoram[1].bytes() - 1; offs >= 0; offs--)
	{
		int const sx = 31 - offs / 32;
		int const sy = offs % 32;

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				m_videoram[1][offs] + 256 * (m_colorram[1][offs] & 0x07),
				(m_colorram[1][offs] & 0x30) >> 4,
				0, 0,
				8 * sx, 8 * sy, 0);
	}
	return 0;
}


/*************************************
 *
 *  Misc Memory handlers
 *
 *************************************/

uint8_t maniach_state::mcu_status_r()
{
	return
			((CLEAR_LINE == m_mcu->mcu_semaphore_r()) ? 0x01 : 0x00) |
			((CLEAR_LINE == m_mcu->host_semaphore_r()) ? 0x02 : 0x00);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void matmania_state::main_map(address_map &map)
{
	map(0x0000, 0x077f).ram();
	map(0x0780, 0x07df).writeonly().share(m_spriteram);
	map(0x1000, 0x13ff).ram().share(m_videoram[1]);
	map(0x1400, 0x17ff).ram().share(m_colorram[1]);
	map(0x2000, 0x21ff).ram().share(m_videoram[0]);
	map(0x2200, 0x23ff).ram().share(m_colorram[0]);
	map(0x2400, 0x25ff).ram().share(m_videoram[2]);
	map(0x2600, 0x27ff).ram().share(m_colorram[2]);
	map(0x3000, 0x3000).portr("IN0").writeonly().share(m_pageselect);
	map(0x3010, 0x3010).portr("IN1").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x3020, 0x3020).portr("DSW2").writeonly().share(m_scroll);
	map(0x3030, 0x3030).portr("DSW1").nopw(); // ??
	map(0x3050, 0x307f).w(FUNC(matmania_state::paletteram_w)).share(m_paletteram);
	map(0x4000, 0xffff).rom();
}

void maniach_state::main_map(address_map &map)
{
	matmania_state::main_map(map);

	map(0x0780, 0x07df).ram().share(m_spriteram);
	map(0x3040, 0x3040).rw(m_mcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0x3041, 0x3041).r(FUNC(maniach_state::mcu_status_r));
}


void matmania_state::sound_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x2000, 0x2001).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x2002, 0x2003).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x2004, 0x2004).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x2007, 0x2007).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom();
}

void maniach_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x2000, 0x2001).w("ymsnd", FUNC(ym3526_device::write));
	map(0x2002, 0x2002).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x2004, 0x2004).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x4000, 0xffff).rom();
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( matmania )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME(0x03, 0x03, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME(0x10, 0x10, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x10, DEF_STR( On ) )
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Cabinet ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(   0x00, DEF_STR( Upright ) )       // The default setting should be cocktail.
	PORT_DIPSETTING(   0x20, DEF_STR( Cocktail ) )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")      // Listed as always ON among DIPs in the manual

	PORT_START("DSW2")
	PORT_DIPNAME(0x03, 0x02, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(   0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x02, DEF_STR( Medium ) )        // According to the manual, default is Medium
	PORT_DIPSETTING(   0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME(0x0c, 0x0c, "Tournament Time" )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(   0x00, "2:12" )                   // Tournament time is always 3:00, but time per 1 second is shorter.
	PORT_DIPSETTING(   0x04, "2:24" )
	PORT_DIPSETTING(   0x08, "2:30" )
	PORT_DIPSETTING(   0x0c, "2:36" )
	PORT_DIPUNUSED_DIPLOC(0x10, IP_ACTIVE_LOW, "SW2:5") // Listed as Unused
	PORT_DIPUNUSED_DIPLOC(0x20, IP_ACTIVE_LOW, "SW2:6") // Listed as Unused
	PORT_DIPUNUSED_DIPLOC(0x40, IP_ACTIVE_LOW, "SW2:7") // Listed as Unused
	PORT_DIPUNUSED_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8") // Listed as Unused
INPUT_PORTS_END

static INPUT_PORTS_START( maniach )
	PORT_INCLUDE( matmania )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x03, 0x03, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(   0x03, DEF_STR( Easy ) )          // According to the manual, default for this game is Easy
	PORT_DIPSETTING(   0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(   0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	1024,   // 1024 characters
	3,  // 3 bits per pixel
	{ 2*1024*8*8, 1024*8*8, 0 },    // the bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,  // 16*16 tiles
	512,    // 512 tiles
	3,  // 3 bits per pixel
	{ 2*512*16*16, 512*16*16, 0 },  // the bitplanes are separated
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every tile takes 16 consecutive bytes
};

static const gfx_layout matmania_spritelayout =
{
	16,16,  // 16*16 sprites
	3584,    // 3584 sprites
	3,  // 3 bits per pixel
	{ 2*3584*16*16, 3584*16*16, 0 },    // the bitplanes are separated
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every sprite takes 16 consecutive bytes
};

static const gfx_layout maniach_spritelayout =
{
	16,16,  // 16*16 sprites
	3584,    // 3584 sprites
	3,  // 3 bits per pixel
	{ 0, 3584*16*16, 2*3584*16*16 },    // the bitplanes are separated
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every sprite takes 16 consecutive bytes
};

static const gfx_layout maniach_tilelayout =
{
	16,16,  // 16*16 tiles
	1024,    /* 1024 tiles */
	3,  // 3 bits per pixel
	{ 2*1024*16*16, 1024*16*16, 0 },    // the bitplanes are separated
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every tile takes 16 consecutive bytes
};

static GFXDECODE_START( gfx_matmania )
	GFXDECODE_ENTRY( "chars",   0, charlayout,              0, 4 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,            4*8, 4 )
	GFXDECODE_ENTRY( "sprites", 0, matmania_spritelayout, 8*8, 2 )
GFXDECODE_END

static GFXDECODE_START( gfx_maniach )
	GFXDECODE_ENTRY( "chars",   0, charlayout,             0, 4 )
	GFXDECODE_ENTRY( "tiles",   0, maniach_tilelayout,   4*8, 4 )
	GFXDECODE_ENTRY( "sprites", 0, maniach_spritelayout, 8*8, 2 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void matmania_state::matmania(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 1'500'000);  // 1.5 MHz ????
	m_maincpu->set_addrmap(AS_PROGRAM, &matmania_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(matmania_state::irq0_line_hold));

	M6502(config, m_audiocpu, 1'200'000); // 1.2 MHz ????
	m_audiocpu->set_addrmap(AS_PROGRAM, &matmania_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(matmania_state::nmi_line_pulse), attotime::from_hz(15 * 60)); // ????

	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(matmania_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_matmania);
	PALETTE(config, m_palette, FUNC(matmania_state::palette), 64 + 16);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, M6502_IRQ_LINE);

	AY8910(config, "ay1", 1'500'000).add_route(ALL_OUTPUTS, "speaker", 0.3);
	AY8910(config, "ay2", 1'500'000).add_route(ALL_OUTPUTS, "speaker", 0.3);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.4); // unknown DAC
}

void maniach_state::maniach(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 1'500'000); // 1.5 MHz ????
	m_maincpu->set_addrmap(AS_PROGRAM, &maniach_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(maniach_state::irq0_line_hold));

	MC6809E(config, m_audiocpu, 1'500'000);    // 1.5 MHz ???? (HD68A09EP)
	m_audiocpu->set_addrmap(AS_PROGRAM, &maniach_state::sound_map);

	TAITO68705_MCU(config, m_mcu, 1'500'000 * 2);  // (don't know really how fast, but it doesn't need to even be this fast)

	config.set_maximum_quantum(attotime::from_hz(6000));  // 100 CPU slice per frame - high interleaving to sync main and MCU

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(maniach_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_maniach);
	PALETTE(config, m_palette, FUNC(maniach_state::palette), 64 + 16);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, M6809_IRQ_LINE);

	ym3526_device &ymsnd(YM3526(config, "ymsnd", 3'600'000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 1.0);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.4); // unknown DAC
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( matmania )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "k0-03",        0x4000, 0x4000, CRC(314ab8a4) SHA1(dc86b2f71a9af8524edad2317343b0d05fe5ef4a) )
	ROM_LOAD( "k1-03",        0x8000, 0x4000, CRC(3b3c3f08) SHA1(65f0c5dba0b8eeb5c2d42b050cac37c475e6a398) )
	ROM_LOAD( "k2-03",        0xc000, 0x4000, CRC(286c0917) SHA1(50d6133406e7db0694b02858c7d06725744cf243) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "k4-0",         0x8000, 0x4000, CRC(86dab489) SHA1(27f6eea29b0287e461e0e321fd7bfaada52c39dc) )
	ROM_LOAD( "k5-0",         0xc000, 0x4000, CRC(4c41cdba) SHA1(a0af0c019bd6d9456cbbe83ecdeee689bc5f1bea) )

	ROM_REGION( 0x06000, "chars", 0 ) // 1024 chars, 3 bpp
	ROM_LOAD( "ku-02",        0x00000, 0x2000, CRC(613c8698) SHA1(07acb2fe150a64029fd15d177c8b6481fcd9eb0b) )
	ROM_LOAD( "kv-02",        0x02000, 0x2000, CRC(274ce14b) SHA1(58ed8c8fe0cc157d642aae596e41f2099c1ea6b1) )
	ROM_LOAD( "kw-02",        0x04000, 0x2000, CRC(7588a9c4) SHA1(0c197a8fea1acb6c9a99071845be54c949ec83b1) )

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "kt-02",        0x00000, 0x4000, CRC(5d817c70) SHA1(f7759be40a8850d325440d336241ecd05b80c0bd) )
	ROM_LOAD( "ks-02",        0x04000, 0x4000, CRC(2e9f3ba0) SHA1(21d6686580de6ecfe57e458821fa92e966a42d95) )
	ROM_LOAD( "kr-02",        0x08000, 0x4000, CRC(b057d3e3) SHA1(24216b22a69c1ecc7eabd7ae10de381e1ff0afc1) )

	ROM_REGION( 0x54000, "sprites", 0 )
	ROM_LOAD( "k6-00",        0x00000, 0x4000, CRC(294d0878) SHA1(0aaae97e35d504dbf6c479ddf04b981847a23ea6) )
	ROM_LOAD( "k7-00",        0x04000, 0x4000, CRC(0908c2f5) SHA1(acc34c578f9a3521855ad4dd8fbd554e05c3f63c) )
	ROM_LOAD( "k8-00",        0x08000, 0x4000, CRC(ae8341e1) SHA1(ca198087b3aec320543a19921015861324ace8a2) )
	ROM_LOAD( "k9-00",        0x0c000, 0x4000, CRC(752ac2c6) SHA1(309fe4e396616b569b9b25654e3dc2751d7b1605) )
	ROM_LOAD( "ka-00",        0x10000, 0x4000, CRC(46a9cb16) SHA1(35e6bd4f33098c98bf2d0b1dfefec2f9d25444e7) )
	ROM_LOAD( "kb-00",        0x14000, 0x4000, CRC(bf016772) SHA1(c901fc2d553622b6dbfaaa9cd94759799d974c39) )
	ROM_LOAD( "kc-00",        0x18000, 0x4000, CRC(8d08bce7) SHA1(1433962c837f568cc1eb27464e243dc580a141de) )
	ROM_LOAD( "kd-00",        0x1c000, 0x4000, CRC(af1d6a60) SHA1(ae3131e3e1fcc9bb1d59db6b1668f6838849241d) )
	ROM_LOAD( "ke-00",        0x20000, 0x4000, CRC(614f19b0) SHA1(67e4687b9be36007c2e1fd504a2eb952fe098d53) )
	ROM_LOAD( "kf-00",        0x24000, 0x4000, CRC(bdf58c18) SHA1(a76c6984e4d4f88384e15d0b6b74093c3bc0fcda) )
	ROM_LOAD( "kg-00",        0x28000, 0x4000, CRC(2189f5cf) SHA1(48289263f7b9cc5b6d975742d45dd64ba45e38c8) )
	ROM_LOAD( "kh-00",        0x2c000, 0x4000, CRC(6b11ed1f) SHA1(8b5c52a14ac3f80ebf630fed8108df17106efd93) )
	ROM_LOAD( "ki-00",        0x30000, 0x4000, CRC(d7ac4ec5) SHA1(35b1503147cb521d2fcc756e6f90ef70d62e2d04) )
	ROM_LOAD( "kj-00",        0x34000, 0x4000, CRC(2caee05d) SHA1(51e0799312e4737bc6f6ae7b74d02f9e10f91c3b) )
	ROM_LOAD( "kk-00",        0x38000, 0x4000, CRC(eb54f010) SHA1(9ed8addd8a542299be2a8f0108447e68b9b33436) )
	ROM_LOAD( "kl-00",        0x3c000, 0x4000, CRC(fa4c7e0c) SHA1(365f5b60ac880928b49a254a5a49a9e9a766046d) )
	ROM_LOAD( "km-00",        0x40000, 0x4000, CRC(6d2369b6) SHA1(b3071cc27598045167681a00f41bf77b6d4bd5bd) )
	ROM_LOAD( "kn-00",        0x44000, 0x4000, CRC(c55733e2) SHA1(b550afd2ceb3b0159c11627ab31f49cc49785809) )
	ROM_LOAD( "ko-00",        0x48000, 0x4000, CRC(ed3c3476) SHA1(eb7bc7c72443d4e3bdfc535bfe460524c0f900d3) )
	ROM_LOAD( "kp-00",        0x4c000, 0x4000, CRC(9c84a969) SHA1(8492ba523e1c1ca94eeba1e53521dd74df854cb9) )
	ROM_LOAD( "kq-00",        0x50000, 0x4000, CRC(fa2f0003) SHA1(7327ce822be8aea360210bbd466a8129788a65c3) )

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "matmania.1",   0x0000, 0x0020, CRC(1b58f01f) SHA1(ffc098d85413777740a25c767096ba5b2aeaf5a8) ) // char palette red and green components
	ROM_LOAD( "matmania.5",   0x0020, 0x0020, CRC(2029f85f) SHA1(7825d42eed284ea0fe7fd60304b8a27a1b5a4075) ) // tile palette red and green components
	ROM_LOAD( "matmania.2",   0x0040, 0x0020, CRC(b6ac1fd5) SHA1(e312a8ff7317eb21320308400539a733c27e8fca) ) // char palette blue component
	ROM_LOAD( "matmania.16",  0x0060, 0x0020, CRC(09325dc2) SHA1(3d9ebdf73840a9603af2acc4bcc4339f3029d284) ) // tile palette blue component
ROM_END

ROM_START( excthour )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e29",          0x04000, 0x4000, CRC(c453e855) SHA1(52ce83042ec04e7ba5b45a61316d6281cb10170a) )
	ROM_LOAD( "e28",          0x08000, 0x4000, CRC(17b63708) SHA1(01c868b7ea32c4857f7187ce73a4cab5b4def246) )
	ROM_LOAD( "e27",          0x0c000, 0x4000, CRC(269ab3bc) SHA1(f2f307c5fc6d50167be8904bef8c7ef21209be50) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "k4-0",         0x8000, 0x4000, CRC(86dab489) SHA1(27f6eea29b0287e461e0e321fd7bfaada52c39dc) )
	ROM_LOAD( "k5-0",         0xc000, 0x4000, CRC(4c41cdba) SHA1(a0af0c019bd6d9456cbbe83ecdeee689bc5f1bea) )

	ROM_REGION( 0x06000, "chars", 0 ) // 1024 chars, 3 bpp
	ROM_LOAD( "e30",          0x00000, 0x2000, CRC(b2875329) SHA1(b37a8b95eb09f1ddc422cc981184b3ea40a5730d) )
	ROM_LOAD( "e31",          0x02000, 0x2000, CRC(c9506de8) SHA1(1036f9acd8b391c03e6408fe1db3406e105373d9) )
	ROM_LOAD( "e32",          0x04000, 0x2000, CRC(00d1635f) SHA1(3a7a20ff949d333ec4d3c0287d73e15dcfefdc71) )

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "e5",           0x00000, 0x4000, CRC(0604dc55) SHA1(dc4e36dac1a820d4e649132206a8b16603d08192) )
	ROM_LOAD( "ks-02",        0x04000, 0x4000, CRC(2e9f3ba0) SHA1(21d6686580de6ecfe57e458821fa92e966a42d95) )
	ROM_LOAD( "e3",           0x08000, 0x4000, CRC(ebd273c6) SHA1(415f68ee10499583f5557aae6a41b5499013b5d2) )

	ROM_REGION( 0x54000, "sprites", 0 )
	ROM_LOAD( "k6-00",        0x00000, 0x4000, CRC(294d0878) SHA1(0aaae97e35d504dbf6c479ddf04b981847a23ea6) )
	ROM_LOAD( "k7-00",        0x04000, 0x4000, CRC(0908c2f5) SHA1(acc34c578f9a3521855ad4dd8fbd554e05c3f63c) )
	ROM_LOAD( "k8-00",        0x08000, 0x4000, CRC(ae8341e1) SHA1(ca198087b3aec320543a19921015861324ace8a2) )
	ROM_LOAD( "k9-00",        0x0c000, 0x4000, CRC(752ac2c6) SHA1(309fe4e396616b569b9b25654e3dc2751d7b1605) )
	ROM_LOAD( "ka-00",        0x10000, 0x4000, CRC(46a9cb16) SHA1(35e6bd4f33098c98bf2d0b1dfefec2f9d25444e7) )
	ROM_LOAD( "kb-00",        0x14000, 0x4000, CRC(bf016772) SHA1(c901fc2d553622b6dbfaaa9cd94759799d974c39) )
	ROM_LOAD( "kc-00",        0x18000, 0x4000, CRC(8d08bce7) SHA1(1433962c837f568cc1eb27464e243dc580a141de) )
	ROM_LOAD( "kd-00",        0x1c000, 0x4000, CRC(af1d6a60) SHA1(ae3131e3e1fcc9bb1d59db6b1668f6838849241d) )
	ROM_LOAD( "ke-00",        0x20000, 0x4000, CRC(614f19b0) SHA1(67e4687b9be36007c2e1fd504a2eb952fe098d53) )
	ROM_LOAD( "kf-00",        0x24000, 0x4000, CRC(bdf58c18) SHA1(a76c6984e4d4f88384e15d0b6b74093c3bc0fcda) )
	ROM_LOAD( "kg-00",        0x28000, 0x4000, CRC(2189f5cf) SHA1(48289263f7b9cc5b6d975742d45dd64ba45e38c8) )
	ROM_LOAD( "kh-00",        0x2c000, 0x4000, CRC(6b11ed1f) SHA1(8b5c52a14ac3f80ebf630fed8108df17106efd93) )
	ROM_LOAD( "ki-00",        0x30000, 0x4000, CRC(d7ac4ec5) SHA1(35b1503147cb521d2fcc756e6f90ef70d62e2d04) )
	ROM_LOAD( "kj-00",        0x34000, 0x4000, CRC(2caee05d) SHA1(51e0799312e4737bc6f6ae7b74d02f9e10f91c3b) )
	ROM_LOAD( "kk-00",        0x38000, 0x4000, CRC(eb54f010) SHA1(9ed8addd8a542299be2a8f0108447e68b9b33436) )
	ROM_LOAD( "kl-00",        0x3c000, 0x4000, CRC(fa4c7e0c) SHA1(365f5b60ac880928b49a254a5a49a9e9a766046d) )
	ROM_LOAD( "km-00",        0x40000, 0x4000, CRC(6d2369b6) SHA1(b3071cc27598045167681a00f41bf77b6d4bd5bd) )
	ROM_LOAD( "kn-00",        0x44000, 0x4000, CRC(c55733e2) SHA1(b550afd2ceb3b0159c11627ab31f49cc49785809) )
	ROM_LOAD( "ko-00",        0x48000, 0x4000, CRC(ed3c3476) SHA1(eb7bc7c72443d4e3bdfc535bfe460524c0f900d3) )
	ROM_LOAD( "kp-00",        0x4c000, 0x4000, CRC(9c84a969) SHA1(8492ba523e1c1ca94eeba1e53521dd74df854cb9) )
	ROM_LOAD( "kq-00",        0x50000, 0x4000, CRC(fa2f0003) SHA1(7327ce822be8aea360210bbd466a8129788a65c3) )

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "matmania.1",   0x0000, 0x0020, CRC(1b58f01f) SHA1(ffc098d85413777740a25c767096ba5b2aeaf5a8) ) // char palette red and green components
	ROM_LOAD( "matmania.5",   0x0020, 0x0020, CRC(2029f85f) SHA1(7825d42eed284ea0fe7fd60304b8a27a1b5a4075) ) // tile palette red and green components
	ROM_LOAD( "matmania.2",   0x0040, 0x0020, CRC(b6ac1fd5) SHA1(e312a8ff7317eb21320308400539a733c27e8fca) ) // char palette blue component
	ROM_LOAD( "matmania.16",  0x0060, 0x0020, CRC(09325dc2) SHA1(3d9ebdf73840a9603af2acc4bcc4339f3029d284) ) // tile palette blue component
ROM_END

ROM_START( maniach )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc-mb2.bin",   0x04000, 0x4000, CRC(a6da1ba8) SHA1(d861c1c68b25272205939df43cdcca423ba7c937) )
	ROM_LOAD( "mc-ma2.bin",   0x08000, 0x4000, CRC(84583323) SHA1(f1512fec6f3e03dc633a96917a114b0b6369c577) )
	ROM_LOAD( "mc-m92.bin",   0x0c000, 0x4000, CRC(e209a500) SHA1(d1a3ab91ffbc321a51c99a2170aca3e217b22576) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mc-m50.bin",   0x4000, 0x4000, CRC(ba415d68) SHA1(484af7a1f109cc9546f17d19b53d284c934705db) )
	ROM_LOAD( "mc-m40.bin",   0x8000, 0x4000, CRC(2a217ed0) SHA1(b06f7c9a2c96ffe78a7065e5edadfdbf985305a5) )
	ROM_LOAD( "mc-m30.bin",   0xc000, 0x4000, CRC(95af1723) SHA1(691ca3f7400d10897e805ff691c904fb2d5bb53a) )

	ROM_REGION( 0x0800, "mcu:mcu", 0 )
	ROM_LOAD( "01",           0x0000, 0x0800, CRC(00c7f80c) SHA1(d2216f660eb8310b1530fa5dc844d26ba90c5e9c) )

	ROM_REGION( 0x06000, "chars", 0 ) // 1024 chars, 3 bpp
	ROM_LOAD( "mc-m60.bin",   0x00000, 0x2000, CRC(1cdbb117) SHA1(cce99c7380fa2a7ae070c7e2d64866866c976085) )
	ROM_LOAD( "mc-m70.bin",   0x02000, 0x2000, CRC(553f0780) SHA1(eacce92ae7b872a35f289f79b33383f5442082d5) )
	ROM_LOAD( "mc-m80.bin",   0x04000, 0x2000, CRC(9392ecb7) SHA1(fb4be39fc2f1c826b146bb5b4dd10eb56b23c300) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "mc-m01.bin",   0x00000, 0x8000, CRC(da558e4d) SHA1(0635f4cded061b0b3649ed1497f087ecd53d54a3) )
	ROM_LOAD( "mc-m10.bin",   0x08000, 0x8000, CRC(619a02f8) SHA1(18de76277c263c76b8d8d9093b3c1aebbf2b7ae4) )
	ROM_LOAD( "mc-m20.bin",   0x10000, 0x8000, CRC(a617c6c1) SHA1(dccae543daa9987f2778327145fc785472f41228) )

	ROM_REGION( 0x54000, "sprites", 0 )
	ROM_LOAD( "mc-mc0.bin",   0x00000, 0x4000, CRC(133d644f) SHA1(5378e0cb665c0aa65d7ad76c3f7c04a3bc301f64) )
	ROM_LOAD( "mc-md0.bin",   0x04000, 0x4000, CRC(e387b036) SHA1(828a42789d9ced9f9fcdfd08a43530008dcbbf2f) )
	ROM_LOAD( "mc-me0.bin",   0x08000, 0x4000, CRC(b36b1283) SHA1(9d12ea9f7a0f12aad532c0f2d3608cf4a86933a6) )
	ROM_LOAD( "mc-mf0.bin",   0x0c000, 0x4000, CRC(2584d8a9) SHA1(f24b4cb827421cd51cb35b581622c41646f3f4d8) )
	ROM_LOAD( "mc-mg0.bin",   0x10000, 0x4000, CRC(cf31a714) SHA1(9740f36e279fc4404112145abb4ff4d138f46474) )
	ROM_LOAD( "mc-mh0.bin",   0x14000, 0x4000, CRC(6292d589) SHA1(a0bf50ebc7712d9bea082834025d3bf816e5afa6) )
	ROM_LOAD( "mc-mi0.bin",   0x18000, 0x4000, CRC(ee2e06e3) SHA1(fbe0457322f5ae03e7eb46b27f044a46f3ee36b5) )
	ROM_LOAD( "mc-mj0.bin",   0x1c000, 0x4000, CRC(7e73895b) SHA1(76861f16f6845e53b80564eb869cfc6767040aa5) )
	ROM_LOAD( "mc-mk0.bin",   0x20000, 0x4000, CRC(66c8bf75) SHA1(071c8635c0264397363d6a33297fd1404f0531a9) )
	ROM_LOAD( "mc-ml0.bin",   0x24000, 0x4000, CRC(88138a1d) SHA1(a669620d51495734f0270fe49c75663cc54e2e50) )
	ROM_LOAD( "mc-mm0.bin",   0x28000, 0x4000, CRC(a1a4260d) SHA1(fbeeac9929f0b273deb1add39db228d3a6d74e76) )
	ROM_LOAD( "mc-mn0.bin",   0x2c000, 0x4000, CRC(6bc61b58) SHA1(dd5e1f8e7299358c6684cb99c4b47b1cf1dc64a7) )
	ROM_LOAD( "mc-mo0.bin",   0x30000, 0x4000, CRC(f96ef600) SHA1(f739ae6c45b2d46a587f8ce32d7626669225ad57) )
	ROM_LOAD( "mc-mp0.bin",   0x34000, 0x4000, CRC(1259618e) SHA1(9f3169675f7add038746edae2ab83fc0a7746db6) )
	ROM_LOAD( "mc-mq0.bin",   0x38000, 0x4000, CRC(102a1666) SHA1(ab052e76a3cef68dd199b5ecf01b73a8abaa32a7) )
	ROM_LOAD( "mc-mr0.bin",   0x3c000, 0x4000, CRC(1e854453) SHA1(41d4997361132c63fcd52dba23885a10ae34bf82) )
	ROM_LOAD( "mc-ms0.bin",   0x40000, 0x4000, CRC(7bc9d878) SHA1(72689ec3263e179f76c7139ed4a82684781d7bb6) )
	ROM_LOAD( "mc-mt0.bin",   0x44000, 0x4000, CRC(09cea985) SHA1(805c58bf73ea19329aa2c8a88a0c35cfaceca985) )
	ROM_LOAD( "mc-mu0.bin",   0x48000, 0x4000, CRC(5421769e) SHA1(c662c53711acf28754a60aedb4637d7d528dc5ea) )
	ROM_LOAD( "mc-mv0.bin",   0x4c000, 0x4000, CRC(36fc3e2d) SHA1(6cd358f29536ff6d5087570cb3e26fd9e971b888) )
	ROM_LOAD( "mc-mw0.bin",   0x50000, 0x4000, CRC(135dce4c) SHA1(3e64a52400137d87b60adf9c307656eadbfe709c) )

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "prom.2",       0x0000, 0x0020, CRC(32db2cf4) SHA1(854b3226a4843a6db94c01c6571294f17a469acf) ) // char palette red and green components
	ROM_LOAD( "prom.16",      0x0020, 0x0020, CRC(18836d26) SHA1(950e1ea5184355501b41548d40732b96c5516fd7) ) // tile palette red and green components
	ROM_LOAD( "prom.3",       0x0040, 0x0020, CRC(c7925311) SHA1(6b997803eb630b79886cebbe3bc49db1c1ab3fd9) ) // char palette blue component
	ROM_LOAD( "prom.17",      0x0060, 0x0020, CRC(41f51d49) SHA1(7cfaf308752cbfddf5a37a31140119afc3febaa7) ) // tile palette blue component

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal10l8.51",   0x0000, 0x002c, CRC(424547af) SHA1(d5e57729906ae0caa8606c52284622e26509e025) )
	ROM_LOAD( "pal10l8.56",   0x0100, 0x002c, CRC(5f6fdf22) SHA1(af6c285c4b23a15b9f1d9db2166681e1b518cc11) )
	ROM_LOAD( "pal16r4a.117", 0x0200, 0x0104, CRC(76640daa) SHA1(3a0be5925ae9a73ea4275d1d641ada2bdb506c31) )
	ROM_LOAD( "pal16r4a.118", 0x0400, 0x0104, CRC(bca7cae2) SHA1(5fad37626a166371c8dd59e55f7f98064621ec1b) )
ROM_END

ROM_START( maniach2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic40-mb1",     0x04000, 0x4000, CRC(b337a867) SHA1(a615283a70700028267e223df00e70bdd159ec09) )
	ROM_LOAD( "ic41-ma1",     0x08000, 0x4000, CRC(85ec8279) SHA1(dada5fa6981573a1fbb235becbc647e1e2d497e1) )
	ROM_LOAD( "ic42-m91",     0x0c000, 0x4000, CRC(a14b86dd) SHA1(73172dfeb34846beaa713c8886d56ed691139d06) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mc-m50.bin",   0x4000, 0x4000, CRC(ba415d68) SHA1(484af7a1f109cc9546f17d19b53d284c934705db) )
	ROM_LOAD( "mc-m40.bin",   0x8000, 0x4000, CRC(2a217ed0) SHA1(b06f7c9a2c96ffe78a7065e5edadfdbf985305a5) )
	ROM_LOAD( "mc-m30.bin",   0xc000, 0x4000, CRC(95af1723) SHA1(691ca3f7400d10897e805ff691c904fb2d5bb53a) )

	ROM_REGION( 0x0800, "mcu:mcu", 0 )
	ROM_LOAD( "01",           0x0000, 0x0800, CRC(00c7f80c) SHA1(d2216f660eb8310b1530fa5dc844d26ba90c5e9c) )

	ROM_REGION( 0x06000, "chars", 0 ) // 1024 chars, 3 bpp
	ROM_LOAD( "mc-m60.bin",   0x00000, 0x2000, CRC(1cdbb117) SHA1(cce99c7380fa2a7ae070c7e2d64866866c976085) )
	ROM_LOAD( "mc-m70.bin",   0x02000, 0x2000, CRC(553f0780) SHA1(eacce92ae7b872a35f289f79b33383f5442082d5) )
	ROM_LOAD( "mc-m80.bin",   0x04000, 0x2000, CRC(9392ecb7) SHA1(fb4be39fc2f1c826b146bb5b4dd10eb56b23c300) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "mc-m01.bin",   0x00000, 0x8000, CRC(da558e4d) SHA1(0635f4cded061b0b3649ed1497f087ecd53d54a3) )
	ROM_LOAD( "mc-m10.bin",   0x08000, 0x8000, CRC(619a02f8) SHA1(18de76277c263c76b8d8d9093b3c1aebbf2b7ae4) )
	ROM_LOAD( "mc-m20.bin",   0x10000, 0x8000, CRC(a617c6c1) SHA1(dccae543daa9987f2778327145fc785472f41228) )

	ROM_REGION( 0x54000, "sprites", 0 )
	ROM_LOAD( "mc-mc0.bin",   0x00000, 0x4000, CRC(133d644f) SHA1(5378e0cb665c0aa65d7ad76c3f7c04a3bc301f64) )
	ROM_LOAD( "mc-md0.bin",   0x04000, 0x4000, CRC(e387b036) SHA1(828a42789d9ced9f9fcdfd08a43530008dcbbf2f) )
	ROM_LOAD( "mc-me0.bin",   0x08000, 0x4000, CRC(b36b1283) SHA1(9d12ea9f7a0f12aad532c0f2d3608cf4a86933a6) )
	ROM_LOAD( "mc-mf0.bin",   0x0c000, 0x4000, CRC(2584d8a9) SHA1(f24b4cb827421cd51cb35b581622c41646f3f4d8) )
	ROM_LOAD( "mc-mg0.bin",   0x10000, 0x4000, CRC(cf31a714) SHA1(9740f36e279fc4404112145abb4ff4d138f46474) )
	ROM_LOAD( "mc-mh0.bin",   0x14000, 0x4000, CRC(6292d589) SHA1(a0bf50ebc7712d9bea082834025d3bf816e5afa6) )
	ROM_LOAD( "mc-mi0.bin",   0x18000, 0x4000, CRC(ee2e06e3) SHA1(fbe0457322f5ae03e7eb46b27f044a46f3ee36b5) )
	ROM_LOAD( "mc-mj0.bin",   0x1c000, 0x4000, CRC(7e73895b) SHA1(76861f16f6845e53b80564eb869cfc6767040aa5) )
	ROM_LOAD( "mc-mk0.bin",   0x20000, 0x4000, CRC(66c8bf75) SHA1(071c8635c0264397363d6a33297fd1404f0531a9) )
	ROM_LOAD( "mc-ml0.bin",   0x24000, 0x4000, CRC(88138a1d) SHA1(a669620d51495734f0270fe49c75663cc54e2e50) )
	ROM_LOAD( "mc-mm0.bin",   0x28000, 0x4000, CRC(a1a4260d) SHA1(fbeeac9929f0b273deb1add39db228d3a6d74e76) )
	ROM_LOAD( "mc-mn0.bin",   0x2c000, 0x4000, CRC(6bc61b58) SHA1(dd5e1f8e7299358c6684cb99c4b47b1cf1dc64a7) )
	ROM_LOAD( "mc-mo0.bin",   0x30000, 0x4000, CRC(f96ef600) SHA1(f739ae6c45b2d46a587f8ce32d7626669225ad57) )
	ROM_LOAD( "mc-mp0.bin",   0x34000, 0x4000, CRC(1259618e) SHA1(9f3169675f7add038746edae2ab83fc0a7746db6) )
	ROM_LOAD( "mc-mq0.bin",   0x38000, 0x4000, CRC(102a1666) SHA1(ab052e76a3cef68dd199b5ecf01b73a8abaa32a7) )
	ROM_LOAD( "mc-mr0.bin",   0x3c000, 0x4000, CRC(1e854453) SHA1(41d4997361132c63fcd52dba23885a10ae34bf82) )
	ROM_LOAD( "mc-ms0.bin",   0x40000, 0x4000, CRC(7bc9d878) SHA1(72689ec3263e179f76c7139ed4a82684781d7bb6) )
	ROM_LOAD( "mc-mt0.bin",   0x44000, 0x4000, CRC(09cea985) SHA1(805c58bf73ea19329aa2c8a88a0c35cfaceca985) )
	ROM_LOAD( "mc-mu0.bin",   0x48000, 0x4000, CRC(5421769e) SHA1(c662c53711acf28754a60aedb4637d7d528dc5ea) )
	ROM_LOAD( "mc-mv0.bin",   0x4c000, 0x4000, CRC(36fc3e2d) SHA1(6cd358f29536ff6d5087570cb3e26fd9e971b888) )
	ROM_LOAD( "mc-mw0.bin",   0x50000, 0x4000, CRC(135dce4c) SHA1(3e64a52400137d87b60adf9c307656eadbfe709c) )

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "prom.2",       0x0000, 0x0020, CRC(32db2cf4) SHA1(854b3226a4843a6db94c01c6571294f17a469acf) ) // char palette red and green components
	ROM_LOAD( "prom.16",      0x0020, 0x0020, CRC(18836d26) SHA1(950e1ea5184355501b41548d40732b96c5516fd7) ) // tile palette red and green components
	ROM_LOAD( "prom.3",       0x0040, 0x0020, CRC(c7925311) SHA1(6b997803eb630b79886cebbe3bc49db1c1ab3fd9) ) // char palette blue component
	ROM_LOAD( "prom.17",      0x0060, 0x0020, CRC(41f51d49) SHA1(7cfaf308752cbfddf5a37a31140119afc3febaa7) ) // tile palette blue component

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal10l8.51",   0x0000, 0x002c, CRC(424547af) SHA1(d5e57729906ae0caa8606c52284622e26509e025) )
	ROM_LOAD( "pal10l8.56",   0x0100, 0x002c, CRC(5f6fdf22) SHA1(af6c285c4b23a15b9f1d9db2166681e1b518cc11) )
	ROM_LOAD( "pal16r4a.117", 0x0200, 0x0104, CRC(76640daa) SHA1(3a0be5925ae9a73ea4275d1d641ada2bdb506c31) )
	ROM_LOAD( "pal16r4a.118", 0x0400, 0x0104, CRC(bca7cae2) SHA1(5fad37626a166371c8dd59e55f7f98064621ec1b) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1985, matmania, 0,        matmania, matmania, matmania_state, empty_init, ROT270, "Technos Japan (Taito America license)", "Mat Mania",               MACHINE_SUPPORTS_SAVE )
GAME( 1985, excthour, matmania, matmania, maniach,  matmania_state, empty_init, ROT270, "Technos Japan (Taito license)",         "Exciting Hour",           MACHINE_SUPPORTS_SAVE )
GAME( 1986, maniach,  0,        maniach,  maniach,  maniach_state,  empty_init, ROT270, "Technos Japan (Taito America license)", "Mania Challenge (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, maniach2, maniach,  maniach,  maniach,  maniach_state,  empty_init, ROT270, "Technos Japan (Taito America license)", "Mania Challenge (set 2)", MACHINE_SUPPORTS_SAVE ) // earlier version?
