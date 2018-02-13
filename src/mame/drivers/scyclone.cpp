// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    Space Cyclone

    3 board stack? - the PCB pictures make it very difficult to figure much out

    seems a bit like 8080bw hardware but with extra sprites and a z80 cpu?
    maybe an evolution of Polaris etc.?


    there's an MB14241 near the Sprite ROM

    1x 8DSW, 1x 4DSW

    there are 11 HM4716AP RAM chips near the main CPU (1 is unmarked, could be different?)
    HM4716AP is 16384 * 1-bit (2048 [0x800] bytes)

    1bpp of video needs 0x3800 bytes

    1bpp video would need 4 of these (with 0x400 bytes leftover)
    2bpp video would need 7 of these
    3bpp video would need 11 of these (with 0x400 bytes leftover)
    mainram is 0x400 bytes (so the leftover in above calc?)


    Notes:

    (to check on hardware)
    the stars scroll backwards when in cocktail mode, the direction changes when the screen
    is flipped but is still reversed compared to upright.  We would have no way of knowing
    if we're in cocktail mode without checking the dipswitch, so I think this is a game bug
    furthermore the game seems to set what I believe to be the 'flipscreen' bit for player 2
    even when in 'upright' mode, so it's possible this romset was only really made for a
    cocktail table?

    it looks like the stars should roughly align with the constellations during the
    intermissions
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/mb14241.h"
#include "sound/dac.h"
#include "sound/sn76477.h"
#include "sound/volt_reg.h"

#include "screen.h"
#include "speaker.h"


class scyclone_state : public driver_device
{
public:
	scyclone_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_stars(*this, "stars"),
		m_gfx1pal(*this, "gfx1pal"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	DECLARE_WRITE8_MEMBER(vidctrl_w);

	DECLARE_WRITE8_MEMBER(sprite_xpos_w);
	DECLARE_WRITE8_MEMBER(sprite_ypos_w);
	DECLARE_WRITE8_MEMBER(sprite_colour_w);
	DECLARE_WRITE8_MEMBER(sprite_tile_w);
	DECLARE_WRITE8_MEMBER(starscroll_w);
	DECLARE_WRITE8_MEMBER(port0e_w);
	DECLARE_WRITE8_MEMBER(port06_w);
	DECLARE_WRITE8_MEMBER(videomask1_w);
	DECLARE_WRITE8_MEMBER(videomask2_w);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(snd_3001_w);
//  DECLARE_WRITE8_MEMBER(snd_3003_w);
//  DECLARE_WRITE8_MEMBER(snd_3004_w);
	DECLARE_WRITE8_MEMBER(snd_3005_w);

	uint32_t screen_update_scyclone(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(irq);

	CUSTOM_INPUT_MEMBER(collision_r);

	void scyclone(machine_config &config);
	void scyclone_iomap(address_map &map);
	void scyclone_map(address_map &map);
	void scyclone_sub_iomap(address_map &map);
	void scyclone_sub_map(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_region_ptr<uint8_t> m_stars;
	required_region_ptr<uint8_t> m_gfx1pal;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	std::unique_ptr<uint8_t[]> m_vram;

	uint8_t m_videowritemask;
	uint8_t m_videowritemask2;

	uint8_t m_sprite_xpos;
	uint8_t m_sprite_ypos;
	uint8_t m_sprite_colour;
	uint8_t m_sprite_tile;
	uint8_t m_starscroll;
	uint8_t m_p0e;
	uint8_t m_vidctrl;

	uint8_t m_hascollided;

	/* video-related */
	const uint8_t get_sprite_pixel(int x, int y);
	const uint8_t get_bitmap_pixel(int x, int y);
	uint32_t draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t draw_bitmap_and_sprite(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void scyclone_state::video_start()
{
	m_vram = std::make_unique<uint8_t []>(0x3800*3);
	save_pointer(&m_vram[0], "m_vram", 0x3800*3);

	m_videowritemask = 0x00;
	m_videowritemask2 = 0x00;

	for (int i = 0; i < 8; i++)
		m_palette->set_pen_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2)));

	for (int i = 0; i < 16; i++)
	{
		uint8_t col = m_gfx1pal[i];
		m_palette->set_pen_color(i+8, rgb_t(pal1bit(col >> 2), pal1bit(col >> 1), pal1bit(col >> 0)));
	}
}

const uint8_t scyclone_state::get_sprite_pixel(int x, int y)
{
	int minx = 0xe0-m_sprite_xpos;
	int miny = 0xe0-m_sprite_ypos;
	int col = m_sprite_colour & 3;
	int code = m_sprite_tile & 7;

	int maxx = minx + 32;
	int maxy = miny + 32;

	/* if we're not even inside the sprite, return nothing */
	if (x < minx || x >= maxx)
		return 0;

	if (y < miny || y >= maxy)
		return 0;

	int sprx = x-minx;
	int spry = y-miny;

	const uint8_t* srcdata = m_gfxdecode->gfx(0)->get_data(code);

	uint8_t pix = srcdata[spry*32 + sprx] + col*4;

	return pix;
}


const uint8_t scyclone_state::get_bitmap_pixel(int x, int y)
{
	const uint8_t video_data0 = m_vram[(y*32+(x/8))];
	const uint8_t video_data1 = m_vram[(y*32+(x/8))+0x3800];
	const uint8_t video_data2 = m_vram[(y*32+(x/8))+0x3800+0x3800];

	int const bit(x & 0x07);

	const uint8_t plane0(BIT(video_data0, bit));
	const uint8_t plane1(BIT(video_data1, bit));
	const uint8_t plane2(BIT(video_data2, bit));

	const uint8_t pal = plane0 | (plane1<<1) | (plane2<<2);

	return pal;
}

uint32_t scyclone_state::draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();

	// this is not correct, but the first 0x80 bytes of the PROM are blank suggesting that this
	// part is for the non-visible 32 rows at least, so it should be something like this
	// blinking and colours are not understood at all



	for (int strip=0;strip<16;strip++)
	{
		for (int x=0;x<256;x++)
		{
			const uint8_t star = m_stars[((x+m_starscroll) & 0x3f)+(strip*0x40)];

			for (int y=0;y<16;y++)
			{
				const int ypos = (y+16*strip)-32;

				if (ypos>=0 && ypos<256)
				{
					int noclipped = 0;

					if (m_vidctrl & 0x08)
						noclipped = x < 64*3;
					else
						noclipped = x >= 64;


					if (y == star && star != 0 && noclipped)
						bitmap.pix32(ypos, x) = paldata[7];
					else
						bitmap.pix32(ypos, x) = paldata[0];
				}
			}
		}
	}

	return 0;
}

uint32_t scyclone_state::draw_bitmap_and_sprite(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();
	m_hascollided = 0;

	for (int y=0;y<256-32;y++)
	{
		for (int x=0;x<256/8;x++)
		{
			for (int i=0;i<8;i++)
			{
				int realx = 0;
				int realy = 0;

				if (m_vidctrl & 0x08)
				{
					realx = 255 - ((x*8)+i);
					realy = 255-32-y;
				}
				else
				{
					realx = (x*8)+i;
					realy = y;
				}


				uint8_t pal = get_bitmap_pixel(realx, realy);

				if (pal) bitmap.pix32(y, (x*8)+i) = paldata[pal];

				uint8_t pal2 = get_sprite_pixel(realx, realy);

				if (pal2 & 0x3)
				{
					bitmap.pix32(y, (x*8)+i) = paldata[8+pal2];
					if (pal == 0x7) m_hascollided = 1;
				}
			}
		}
	}

	return 0;
}


uint32_t scyclone_state::screen_update_scyclone(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	draw_starfield(screen,bitmap,cliprect);
	draw_bitmap_and_sprite(screen,bitmap,cliprect);

	//popmessage("%02x %02x %02x %02x %02x %02x", m_sprite_xpos, m_sprite_ypos, m_sprite_colour, m_sprite_tile, m_starscroll, m_p0e);

	return 0;
}



ADDRESS_MAP_START(scyclone_state::scyclone_map)
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x4400, 0x5fff) AM_READWRITE(vram_r,vram_w)
	AM_RANGE(0x6000, 0x60ff) AM_NOP // this just seems to be overflow from the VRAM writes, probably goes nowhere
ADDRESS_MAP_END


ADDRESS_MAP_START(scyclone_state::scyclone_iomap)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0") AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW0") AM_WRITE(vidctrl_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(sprite_xpos_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(sprite_ypos_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(port06_w) // possible watchdog, unlikely to be twinkle related.
	AM_RANGE(0x08, 0x08) AM_WRITE(sprite_colour_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(sprite_tile_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(starscroll_w)
	AM_RANGE(0x0e, 0x0e) AM_WRITE(port0e_w)
	AM_RANGE(0x0f, 0x0f) AM_DEVWRITE("soundlatch", generic_latch_8_device, write)
	AM_RANGE(0x40, 0x40) AM_WRITE(videomask1_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(videomask2_w)
ADDRESS_MAP_END


ADDRESS_MAP_START(scyclone_state::scyclone_sub_map)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM

	AM_RANGE(0x3000, 0x3000) AM_DEVREAD("soundlatch", generic_latch_8_device, read) AM_DEVWRITE("dac", dac_byte_interface, write) // music
	AM_RANGE(0x3001, 0x3001) AM_WRITE(snd_3001_w) // written at the same time, with the same data as 0x3005
	AM_RANGE(0x3002, 0x3002) AM_DEVWRITE("dac2", dac_byte_interface, write) // speech
//  AM_RANGE(0x3003, 0x3003) AM_WRITE(snd_3003_w) // writes 02 or 00
//  AM_RANGE(0x3004, 0x3004) AM_WRITE(snd_3004_w) // always writes 00?
	AM_RANGE(0x3005, 0x3005) AM_WRITE(snd_3005_w) // written at the same time, with the same data as 0x3001
ADDRESS_MAP_END

ADDRESS_MAP_START(scyclone_state::scyclone_sub_iomap)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


// appears to be when a white bitmap pixel (col 0x7) collides with a large sprite?
// if you simply set it to 1 and shoot in the left corner, the game gets stuck
// but if you have it set to 0 there are no collisions with large objects
CUSTOM_INPUT_MEMBER(scyclone_state::collision_r)
{
	return m_hascollided;
}


static INPUT_PORTS_START( scyclone )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, scyclone_state, collision_r, nullptr) // hw collision?
	// maybe these 4 are the 4xdsw bank?
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CONDITION("DSW0",0x04,EQUALS,0x00)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("DSW0",0x04,EQUALS,0x00)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("DSW0",0x04,EQUALS,0x00)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )   PORT_DIPLOCATION("DSW0:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("DSW0:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x08, 0x00, "Disable Collision (Buggy!)" ) PORT_DIPLOCATION("DSW0:4") // this causes the game to malfunction, likely leftover debug feature
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Show Copyright Date" ) PORT_DIPLOCATION("DSW0:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW0:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW0:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW0:8" )
INPUT_PORTS_END

READ8_MEMBER(scyclone_state::vram_r)
{
	uint8_t ret = 0;
	// not sure, collisions depend on readback
	ret |= m_vram[offset];
	ret |= m_vram[offset+0x3800];
	ret |= m_vram[offset+0x3800+0x3800];

	return ret;
}


WRITE8_MEMBER(scyclone_state::vram_w)
{
#if 0
	// halves are NOT equal in between level cutscenes
	// this is used when drawing the pointing stick, need closeup reference

	// 0x40 and 0x80 ports always seem equal tho
	if ((m_videowritemask & 0xf) != ((m_videowritemask>>4) & 0xf))
	{
		logerror("m_videowritemask halves not equal %02x\n", m_videowritemask);
	}

	if ((m_videowritemask2 & 0xf) != ((m_videowritemask2>>4) & 0xf))
	{
		logerror("m_videowritemask2 halves not equal %02x\n", m_videowritemask2);
	}

	if ((m_videowritemask) != (m_videowritemask2))
	{
		logerror("m_videowritemask != m_videowritemask2 %02x %02x\n", m_videowritemask, m_videowritemask2);
	}
#endif


	for (int i=0; i<8; i++)
	{
		const uint8_t databit = data & (1<<i);
		const uint8_t nodatabit = (1<<i);

		uint8_t videowritemask = 0;

		if (i>=6) videowritemask = (m_videowritemask>>4) & 0x0f;
		else if (i>=4) videowritemask = (m_videowritemask>>0) & 0x0f;
		else if (i>=2) videowritemask = (m_videowritemask2>>4) & 0x0f;
		else videowritemask = (m_videowritemask2>>0) & 0x0f;

		if (!(videowritemask & 1) && databit) m_vram[offset] |= databit;
		else m_vram[offset] &= ~nodatabit;

		if (!(videowritemask & 2) && databit) m_vram[offset+0x3800] |= databit;
		else m_vram[offset+0x3800] &= ~nodatabit;

		if (!(videowritemask & 4) && databit) m_vram[offset+0x3800+0x3800] |= databit;
		else m_vram[offset+0x3800+0x3800] &= ~nodatabit;
	}
}


WRITE8_MEMBER(scyclone_state::vidctrl_w)
{
	// ---- facu
	// f = flipscreen (always set during player 2 turn, even in upright mode?!)
	// a = alternates during attract mode, enabled during gameplay
	// u = unknown but used (set to 1 during gameplay)
	// c = coinlock
	m_vidctrl = data;

	if (data & 0xf0)
	{
		logerror("vidctrl_w %02x\n", data);
	}
}

WRITE8_MEMBER(scyclone_state::sprite_xpos_w)
{
	m_sprite_xpos = data;
}

WRITE8_MEMBER(scyclone_state::sprite_ypos_w)
{
	m_sprite_ypos = data;
}

WRITE8_MEMBER(scyclone_state::sprite_colour_w)
{
	m_sprite_colour = data;
}

WRITE8_MEMBER(scyclone_state::sprite_tile_w)
{
	m_sprite_tile = data;
}

WRITE8_MEMBER(scyclone_state::starscroll_w)
{
	m_starscroll = data;
}

WRITE8_MEMBER(scyclone_state::port0e_w)
{
	// could be related to basic sound effects?
	//logerror("port0e_w %02x\n",data);
	m_p0e = data;
}

WRITE8_MEMBER(scyclone_state::port06_w)
{
	// watchdog?
}

// these seem to select where the vram writes go
// 3 possible planes, probably one 4 bits per 2 pixels?
WRITE8_MEMBER(scyclone_state::videomask1_w)
{
	// format seems to be -xxx -xxx
	m_videowritemask = data;
}

WRITE8_MEMBER(scyclone_state::videomask2_w)
{
	// format seems to be -xxx -xxx
	m_videowritemask2 = data;
}

// Sound CPU handlers

WRITE8_MEMBER(scyclone_state::snd_3001_w)
{
	// need to clear the latch somewhere, the command value is written back here and at 3005
	// after acknowledging a command
	// might actually reset the DACs as there are (at least) 2 of them?
	m_soundlatch->clear_w(space, 0, data);
}

/*
WRITE8_MEMBER(scyclone_state::snd_3003_w)
{
//  m_soundlatch->clear_w(space, 0, data);
}

WRITE8_MEMBER(scyclone_state::snd_3004_w)
{
//  m_soundlatch->clear_w(space, 0, data);
}
*/

WRITE8_MEMBER(scyclone_state::snd_3005_w)
{
//  m_soundlatch->clear_w(space, 0, data);
}


static const gfx_layout tiles32x32_layout =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 256+0, 256+1, 256+2, 256+3, 512+0, 512+1, 512+2, 512+3, 768+0, 768+1, 768+2, 768+3, 1024+0, 1024+1, 1024+2, 1024+3, 1280+0, 1280+1, 1280+2, 1280+3, 1536+0, 1536+1, 1536+2, 1536+3, 1792+0, 1792+1, 1792+2, 1792+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8, 16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8, 24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8 },
	8*32*8
};

static GFXDECODE_START( scyclone )
	GFXDECODE_ENTRY( "gfx1", 0, tiles32x32_layout, 8, 4 )
GFXDECODE_END



void scyclone_state::machine_start()
{
	save_item(NAME(m_videowritemask));
	save_item(NAME(m_videowritemask2));
	save_item(NAME(m_sprite_xpos));
	save_item(NAME(m_sprite_ypos));
	save_item(NAME(m_sprite_colour));
	save_item(NAME(m_sprite_tile));
	save_item(NAME(m_starscroll));
	save_item(NAME(m_p0e));
	save_item(NAME(m_vidctrl));
	save_item(NAME(m_hascollided));
}

void scyclone_state::machine_reset()
{
	m_sprite_xpos = 0;
	m_sprite_ypos = 0;
	m_sprite_colour = 0;
	m_sprite_tile = 0;
	m_starscroll = 0;
	m_p0e = 0;

	m_hascollided = 0;
}



INTERRUPT_GEN_MEMBER(scyclone_state::irq)
{
	// CPU runs in IM0
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   /* RST 10h */
}


MACHINE_CONFIG_START(scyclone_state::scyclone)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 5000000/2) // MOSTEK Z80-CPU   ? MHz  (there's also a 9.987MHz XTAL)  intermissions seem driven directly by CPU speed for reference
	MCFG_CPU_PROGRAM_MAP(scyclone_map)
	MCFG_CPU_IO_MAP(scyclone_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", scyclone_state, irq)

	// sound ?
	MCFG_CPU_ADD("subcpu", Z80, 5000000/2) // LH0080 Z80-CPU SHARP  ? MHz   (5Mhz XTAL on this sub-pcb)
	MCFG_CPU_PROGRAM_MAP(scyclone_sub_map)
	MCFG_CPU_IO_MAP(scyclone_sub_iomap)
	// no idea, but it does wait on an irq in places, irq0 increases a register checked in the wait loop so without it sound dies after a while
	MCFG_CPU_PERIODIC_INT_DRIVER(scyclone_state, irq0_line_hold, 400*60)

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-32-1)
	MCFG_SCREEN_UPDATE_DRIVER(scyclone_state, screen_update_scyclone)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE) // due to hw collisions

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", scyclone)
	MCFG_PALETTE_ADD("palette", 8 + 4*4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")

	MCFG_SOUND_ADD("snsnd0", SN76477, 0)
	MCFG_SN76477_ENABLE(1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.2)

	MCFG_SOUND_ADD("snsnd1", SN76477, 0)
	MCFG_SN76477_ENABLE(1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.2)

	// this is just taken from route16.cpp

	MCFG_SOUND_ADD("dac", DAC_8BIT_R2R, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25) // unknown DAC

	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0)
	MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)

	MCFG_SOUND_ADD("dac2", DAC_8BIT_R2R, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25) // unknown DAC

	MCFG_DEVICE_ADD("vref2", VOLTAGE_REGULATOR, 0)
	MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac2", 1.0, DAC_VREF_POS_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac2", -1.0, DAC_VREF_NEG_INPUT)

MACHINE_CONFIG_END

ROM_START( scyclone )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "DE07.1H.2716", 0x0000, 0x0800, CRC(fa9c93a3) SHA1(701a3196ee67533d69f1264e14085d04dfb1b915) )
	ROM_LOAD( "DE08.3H.2716", 0x0800, 0x0800, CRC(bdff3e77) SHA1(e3fc6840348fa6df834cb1fb8b3832db05b5c585) )
	ROM_LOAD( "DE09.4H.2716", 0x1000, 0x0800, CRC(eed05875) SHA1(e00a1b2d2d723c2deedf5f5e3159936a5d27891d) )
	ROM_LOAD( "DE10.5H.2716", 0x1800, 0x0800, CRC(71301be6) SHA1(70c1983d256a6a9e18c7a15b93f5e3781d884fdb) )
	ROM_LOAD( "DE11.6H.2716", 0x2000, 0x0800, CRC(d241db51) SHA1(53c6489d715baf5f03032a7ac367a6af64bce040) )
	ROM_LOAD( "DE12.7H.2716", 0x2800, 0x0800, CRC(208e3e9a) SHA1(07f0e6c5417584eef171d8dcad83d741f88c9348) )

	ROM_REGION( 0x2000, "subcpu", 0 )
	ROM_LOAD( "DE18.IC1.2716",  0x0000, 0x0800, CRC(182a5c24) SHA1(0ba29b6b3e7ec97c0b0748a6e60bb1a737dc55fa) )
	ROM_LOAD( "DE04.IC9.2716",  0x0800, 0x0800, CRC(098269ac) SHA1(138af858bbbd73380086f971bcdd52ae47e2b667) )
	ROM_LOAD( "DE05.IC14.2716", 0x1000, 0x0800, CRC(878f68b2) SHA1(2c2fa1b9053664ec26452ab0b35e2ae550601cc9) )
	ROM_LOAD( "DE06.IC19.2716", 0x1800, 0x0800, CRC(0b1ead90) SHA1(38322b39f4420408c223cdbed3b75692a3d70746) )

	ROM_REGION( 0x0800, "gfx1", 0 ) // 32x32x2 sprites
	ROM_LOAD( "DE01.11C.2716", 0x0000, 0x0800, CRC(bf770730) SHA1(1d0f9235b0618e3f4dd6db47efbdf92e2b00f5f6) )

	ROM_REGION( 0x0400, "gfx1pal", 0 ) // only 16 bytes of this are actually used
	ROM_LOAD( "DE02.5B.82S137", 0x0000, 0x0400, CRC(fe4b278c) SHA1(c03080ab4d3fb84b8eec7087b925d1a1d8565fcc) )

	ROM_REGION( 0x0840, "stars", 0 ) // probably the starfield bitmap
	ROM_LOAD( "DE15.3A.82S137", 0x0000, 0x0400, CRC(c2816161) SHA1(2d0e7b4dbdb2af1481a4b7e45b1f9e3640f69aec) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "DE16.4A.82S123", 0x0000, 0x0020, CRC(5178e9c5) SHA1(dd2f81894069282f37feae21c5cfacf50f77dcd5) )
	ROM_LOAD( "DE17.2E.82S123", 0x0020, 0x0020, CRC(3c8572e4) SHA1(c908c4ed99828fff576c3d0963cd8b99edeb993b) )
ROM_END

GAME( 1980, scyclone,  0,    scyclone, scyclone, scyclone_state, 0, ROT270, "Taito Corporation", "Space Cyclone", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
