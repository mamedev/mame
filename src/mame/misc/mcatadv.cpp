// license:BSD-3-Clause
// copyright-holders: Paul Priest, David Haywood

/******************************************************************************

  'Face' LINDA board
 driver by Paul Priest + David Haywood

*******************************************************************************

 Games on this Hardware

 Magical Cat Adventure (c)1993 Wintechno
 Nostradamus (c)1993 Face

*******************************************************************************

 Hardware Overview:

Magical Cat (C) 1993 Wintechno
Board Name: LINDA5

 Main CPU: 68000-16
 Sound CPU: Z0840004PSC
 Sound Chip: YMF286-K & Y3016-F

 Custom: FACE FX1037 x1
         038 x2 (As in Cave)


Nostradamus (C) 1993 FACE
Board Name: LINDA25

  Main CPU: MC68000P12F 16MHz
 Sound CPU: Z8400B PS (Goldstar)
Sound chip: YMF286-K & Y3016-F

Graphics chips:
176 Pin PQFP 038 9330EX705
176 Pin PQFP 038 9320EX702
176 Pin PQFP FX1037 FACE FA01-2075 (Face Custom)

OSC 28.000 MHz - SEOAN SX0-T100
OSC 16.000 MHz - Sunny SC0-010T

8 Way DIP Switch x 2
Push Button Test Switch

Roms:
NOS-PO-U 2740000PC-15 (68k Program) U29 - Odd
NOS-PE-U 2740000PC-15 (68k Program) U30 - Even
NOS-PS     D27C020-15 (Z80 program) U9

As labelled on PCB, with location:
NOS-SO-00.U83-
NOS-SO-01.U85 \
NOS-SO-02.U87  | Sprites Odd/Even (These are 27C8001)
NOS-SE-00.U82  |
NOS-SE-01.U84 /
NOS-SE-02.U86-
U92 & U93 are unpopulated

NOS-SN-00.U53 Sound samples (Near the YMF286-K)

NOS-B0-00.U58-
NOS-B0-01.U59 \ Background (separate for each 038 chip?)
NOS-B1-00.U60 /
NOS-B1-01.U61-

YMF286-K is compatible to YM2610 - see psikyo/psikyo.cpp driver
038 9320EX702 / 038 9330EX705    - see misc/cave.cpp driver

Note # = Pin #1    PCB Layout:

+----------------------------------------------------------------------------+
| ___________                                                                |_
|| NOS-B1-00 |                                                                J|
|#___________|                ________   ________                             A|
| ___________   __________   |NOS-PO-U| |NOS-PE-U|                            M|
|| NOS-B1-01 | |          |  #________| #________|                            M|
|#___________| | 038      |   ___________________    _______                  A|
|              | 9330EX705|  |   MC68000P12F     |  |NOS-PS |                  |
|              |__________#  |   16MHz           |  #_______|                 C|
|                            #___________________|  ___________               o|
| ___________   __________                         | Z8400B PS |              n|
|| NOS-B0-00 | |          |                        #___________|              n|
|#___________| | 038      |                        ______________             e|
| ___________  | 9320EX702|                   SW1 |   YMF286-K   |            c|
|| NOS-B0-01 | |__________#     _________         #______________|            t|
|#___________|                 |FX1037   #  SW2                    _______    i|
|                              |(C) Face |         ___________    |Y3016-F#   o|
|                              |FA01-2075|        | NOS-SN-00 |   |_______|   n|
|                              |_________|        #___________|               _|
| ______                   ___  ___  ___       ___  ___  ___                 |
||OSC 28|                 # N |# N |# N |  E  # N |# N |# N | E              |
|#______|                 | O || O || O |  m  | O || O || O | m              |
|                         | S || S || S |  p  | S || S || S | p              |
| Empty                   | | || | || | |  t  | | || | || | | t              |
|  OSC                    | S || S || S |  y  | S || S || S | y              |
| ______                  | E || E || E |     | O || O || O |                |
||OSC 16|                 | | || | || | |  S  | | || | || | | S              |
|#______|                 | 0 || 0 || 0 |  C  | 0 || 0 || 0 | C              |
|                         | 0 || 1 || 2 |  K  | 0 || 1 || 2 | K              |
|    PUSHBTN              |___||___||___|  T  |___||___||___| T              |
+----------------------------------------------------------------------------+

*******************************************************************************

Stephh's notes (based on the games M68000 code and some tests) :

1) "mcatadv*'

  - Player 1 Button 3 is only used in the "test" mode :
      * to select "OBJECT ROM CHECK"
      * in "BG ROM", to change the background number

  - Do NOT trust the "NORMAL TESTMODE" for the system inputs !

  - The Japan version has extra GFX/anims and it's harder than the other set.

2) 'nost*'

  - When entering the "test mode", you need to press SERVICE1 to cycle through
    the different screens.

*******************************************************************************

 TODO:

 Fix Sprites & Rowscroll/Select for Cocktail

*******************************************************************************

 trivia:

 Magical Cat Adventure tests for 'MASICAL CAT ADVENTURE' in RAM on start-up
 and will write it there if not found, expecting a reset, great engrish ;-)

******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"
#include "video/tmap038.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>


// configurable logging
#define LOG_SPRITEBANK     (1U << 1)
#define LOG_ROWSCROLL      (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_SPRITEBANK | LOG_ROWSCROLL)

#include "logmacro.h"

#define LOGSPRITEBANK(...)     LOGMASKED(LOG_SPRITEBANK,     __VA_ARGS__)
#define LOGROWSCROLL(...)      LOGMASKED(LOG_ROWSCROLL,      __VA_ARGS__)


namespace {

class mcatadv_state : public driver_device
{
public:
	mcatadv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_sprdata(*this, "sprdata")
		, m_soundbank(*this, "soundbank")
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_watchdog(*this, "watchdog")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_tilemap(*this, "tilemap_%u", 0U)
		, m_spriteram(*this, "spriteram")
		, m_vidregs(*this, "vidregs")
	{ }

	void nost(machine_config &config);
	void mcatadv(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_region_ptr<u8> m_sprdata;
	required_memory_bank m_soundbank;

	// video-related
	u8 m_palette_bank[2] = {};

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<tilemap038_device, 2> m_tilemap;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_vidregs;

	u16 mcatadv_wd_r(); // mcatadv only
	void sound_banking_w(u8 data);
	template<int Chip> void get_banked_color(bool tiledim, u32 &color, u32 &pri, u32 &code);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tilemap_part(screen_device &screen, int layer, int i, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void mcatadv_sound_io_map(address_map &map) ATTR_COLD;
	void mcatadv_sound_map(address_map &map) ATTR_COLD;
	void nost_sound_io_map(address_map &map) ATTR_COLD;
	void nost_sound_map(address_map &map) ATTR_COLD;
};


void mcatadv_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 *source = (m_spriteram->buffer() + (m_spriteram->bytes() / 2) /2);
	source -= 4;
	u16 *finish = m_spriteram->buffer();
	int const global_x = m_vidregs->live()[0] - 0x184;
	int const global_y = m_vidregs->live()[1] - 0x1f1;

	u32 const sprmask = m_sprdata.bytes() - 1;

	int xstart, xend, xinc;
	int ystart, yend, yinc;

	if (m_vidregs->buffer()[2] == 0x0001) // double buffered
	{
		source += (m_spriteram->bytes() / 2) / 2;
		finish += (m_spriteram->bytes() / 2) / 2;
	}
	else if (m_vidregs->buffer()[2]) // I suppose it's possible that there is 4 banks, haven't seen it used though
	{
		LOGSPRITEBANK("Spritebank != 0/1\n");
	}

	while (source >= finish)
	{
		u32 const pen = (source[0] & 0x3f00) >> 8;
		u32 const tileno = source[1] & 0xffff;
		u8 pri = (source[0] & 0xc000) >> 14;

		pri |= 0x8;

		int x = source[2] & 0x3ff;
		int y = source[3] & 0x3ff;
		int flipy = source[0] & 0x0040;
		int flipx = source[0] & 0x0080;

		int const height = ((source[3] & 0xf000) >> 12) * 16;
		int const width = ((source[2] & 0xf000) >> 12) * 16;
		u32 offset = tileno * 256;

		if (x & 0x200) x -= 0x400;
		if (y & 0x200) y -= 0x400;

#if 0 // For Flipscreen/Cocktail
		if (m_vidregs->live()[0] & 0x8000)
		{
			flipx = !flipx;
		}
		if (m_vidregs->live()[1] & 0x8000)
		{
			flipy = !flipy;
		}
#endif

		if (source[3] != source[0]) // 'hack' don't draw sprites while it's testing the RAM!
		{
			if (!flipx) { xstart = 0;        xend = width;  xinc = 1; }
			else        { xstart = width-1;  xend = -1;     xinc = -1; }
			if (!flipy) { ystart = 0;        yend = height; yinc = 1; }
			else        { ystart = height-1; yend = -1;     yinc = -1; }

			for (int ycnt = ystart; ycnt != yend; ycnt += yinc)
			{
				const int drawypos = y + ycnt - global_y;

				if ((drawypos >= cliprect.min_y) && (drawypos <= cliprect.max_y))
				{
					u16 *const destline = &bitmap.pix(drawypos);
					u8 *const priline = &screen.priority().pix(drawypos);

					for (int xcnt = xstart; xcnt != xend; xcnt += xinc)
					{
						const int drawxpos = x + xcnt - global_x;

						if ((drawxpos >= cliprect.min_x) && (drawxpos <= cliprect.max_x))
						{
							const int pridata = priline[drawxpos];

							if (!(pridata & 0x10)) // if we haven't already drawn a sprite pixel here (sprite masking)
							{
								u8 pix = m_sprdata[(offset / 2)&sprmask];

								if (offset & 1)
									pix = pix >> 4;
								pix &= 0x0f;

								if (pix)
								{
									if ((priline[drawxpos] < pri))
										destline[drawxpos] = (pix + (pen << 4));

									priline[drawxpos] |= 0x10;
								}

							}
						}
						offset++;
					}
				}
				else
				{
					offset += width;
				}
			}
		}
		source -= 4;
	}
}

void mcatadv_state::draw_tilemap_part(screen_device &screen, int layer, int i, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_tilemap[layer]->enable())
		return;

	rectangle clip;

	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;

	for (u32 drawline = cliprect.min_y; drawline <= cliprect.max_y; drawline++)
	{
		clip.min_y = drawline;
		clip.max_y = drawline;

		int scrollx = (m_tilemap[layer]->scrollx() & 0x1ff) - 0x194;
		int scrolly = (m_tilemap[layer]->scrolly() & 0x1ff) - 0x1df;

		if (m_tilemap[layer]->rowselect_en())
		{
			const int rowselect = m_tilemap[layer]->rowselect(drawline + scrolly);
			scrolly = rowselect - drawline;
		}

		if (m_tilemap[layer]->rowscroll_en())
		{
			const int rowscroll = m_tilemap[layer]->rowscroll(drawline + scrolly);
			scrollx += rowscroll;
		}

		// global flip
		if (m_tilemap[layer]->flipx()) scrollx -= 0x19;
		if (m_tilemap[layer]->flipy()) scrolly -= 0x141;
		int flip = (m_tilemap[layer]->flipx() ? TILEMAP_FLIPX : 0) | (m_tilemap[layer]->flipy() ? TILEMAP_FLIPY : 0);

		m_tilemap[layer]->set_scrollx(0, scrollx);
		m_tilemap[layer]->set_scrolly(0, scrolly);
		m_tilemap[layer]->set_flip(flip);

		m_tilemap[layer]->draw(screen, bitmap, clip, i, i);
	}
}

u32 mcatadv_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x3f0, cliprect);
	screen.priority().fill(0, cliprect);

	for (int i = 0; i < 2; i++)
	{
		m_tilemap[i]->prepare();
		if (m_tilemap[i]->external() != m_palette_bank[i])
		{
			m_palette_bank[i] = m_tilemap[i]->external() & 0xf;
			m_tilemap[i]->mark_all_dirty();
		}
	}


	LOGROWSCROLL("%02x %02x %02x %02x",
		m_tilemap[0]->rowscroll_en(),
		m_tilemap[0]->rowselect_en(),
		m_tilemap[1]->rowscroll_en(),
		m_tilemap[1]->rowselect_en());


	for (int i = 0; i <= 3; i++)
	{
	#ifdef MAME_DEBUG
			if (!machine().input().code_pressed(KEYCODE_Q))
	#endif
				draw_tilemap_part(screen, 0, i | 0x8, bitmap, cliprect);

	#ifdef MAME_DEBUG
			if (!machine().input().code_pressed(KEYCODE_W))
	#endif
				draw_tilemap_part(screen, 1, i | 0x8, bitmap, cliprect);
	}

	auto profile = g_profiler.start(PROFILER_USER1);
#ifdef MAME_DEBUG
	if (!machine().input().code_pressed(KEYCODE_E))
#endif
		draw_sprites(screen, bitmap, cliprect);
	return 0;
}

void mcatadv_state::video_start()
{
	m_palette_bank[0] = m_palette_bank[1] = 0;
}


template<int Chip>
void mcatadv_state::get_banked_color(bool tiledim, u32 &color, u32 &pri, u32 &code)
{
	pri |= 0x8;
	color += m_palette_bank[Chip] * 0x40;
}

/*** Main CPU ***/

#if 0 // mcat only.. install read handler?
void mcatadv_state::mcat_coin_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x10);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x40);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x80);
}
#endif

u16 mcatadv_state::mcatadv_wd_r()
{
	if (!machine().side_effects_disabled())
		m_watchdog->watchdog_reset();
	return 0xc00;
}


void mcatadv_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();

//  map(0x180018, 0x18001f).nopr(); // ?

	map(0x200000, 0x200005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w));
	map(0x300000, 0x300005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w));

	map(0x400000, 0x401fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_16x16_map));
	map(0x500000, 0x501fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_16x16_map));

	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x602000, 0x602fff).ram(); // Bigger than needs to be?

	map(0x700000, 0x707fff).ram().share("spriteram"); // Sprites, two halves for double buffering
	map(0x708000, 0x70ffff).ram(); // Tests more than is needed?

	map(0x800000, 0x800001).portr("P1");
	map(0x800002, 0x800003).portr("P2");
//  map(0x900000, 0x900000).w(FUNC(mcatadv_state::mcat_coin_w)); // Lockout / Counter MCAT Only
	map(0xa00000, 0xa00001).portr("DSW1");
	map(0xa00002, 0xa00003).portr("DSW2");

	map(0xb00000, 0xb0000f).ram().share("vidregs");

	map(0xb00018, 0xb00019).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w)); // NOST Only
	map(0xb0001e, 0xb0001f).r(FUNC(mcatadv_state::mcatadv_wd_r)); // MCAT Only
	map(0xc00001, 0xc00001).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0xc00000, 0xc00001).w("soundlatch", FUNC(generic_latch_8_device::write)).umask16(0x00ff).cswidth(16);
}

/*** Sound ***/

void mcatadv_state::sound_banking_w(u8 data)
{
	m_soundbank->set_entry(data);
}


void mcatadv_state::mcatadv_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xbfff).bankr(m_soundbank);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xf000, 0xf000).w(FUNC(mcatadv_state::sound_banking_w));
}

void mcatadv_state::mcatadv_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read)).w("soundlatch2", FUNC(generic_latch_8_device::write));
}


void mcatadv_state::nost_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xc000, 0xdfff).ram();
}

void mcatadv_state::nost_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).w("ymsnd", FUNC(ym2610_device::write));
	map(0x04, 0x07).r("ymsnd", FUNC(ym2610_device::read));
	map(0x40, 0x40).w(FUNC(mcatadv_state::sound_banking_w));
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read)).w("soundlatch2", FUNC(generic_latch_8_device::write));
}

/*** Inputs ***/

static INPUT_PORTS_START( mcatadv )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // "Fire"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // "Jump"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)   // See notes
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)   // "Fire"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   // "Jump"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(   0x0400, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0800, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x3000, 0x3000, "Energy" )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x4000, "Upright 1 Player" )
	PORT_DIPSETTING(      0xc000, "Upright 2 Players" )
	PORT_DIPSETTING(      0x0000, "Upright 2 Players" )       // duplicated setting (NEVER tested)
	PORT_DIPSETTING(      0x8000, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nost )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // Button 2 in "test mode"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // Button 3 in "test mode"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // "test" 3 in "test mode"
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )                 // Must be LOW or startup freezes !
	PORT_BIT( 0xf400, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // Button 2 in "test mode"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // Button 3 in "test mode"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "500k 1000k" )
	PORT_DIPSETTING(      0xc000, "800k 1500k" )
	PORT_DIPSETTING(      0x4000, "1000k 2000k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )        // Listed as "Unused"
	PORT_SERVICE_DIPLOC(   0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

/*** GFX Decode ***/

static GFXDECODE_START( gfx_mcatadv )
	GFXDECODE_ENTRY( "bg0", 0, gfx_8x8x4_packed_msb, 0, 0x200 )
	GFXDECODE_ENTRY( "bg1", 0, gfx_8x8x4_packed_msb, 0, 0x200 )
GFXDECODE_END


void mcatadv_state::machine_start()
{
	const u32 max = memregion("soundcpu")->bytes() / 0x4000;

	m_soundbank->configure_entries(0, max, memregion("soundcpu")->base(), 0x4000);
	m_soundbank->set_entry(1);

	save_item(NAME(m_palette_bank));
}

void mcatadv_state::mcatadv(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000)); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &mcatadv_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(mcatadv_state::irq1_line_hold));

	Z80(config, m_soundcpu, XTAL(16'000'000) / 4); // verified on PCB
	m_soundcpu->set_addrmap(AS_PROGRAM, &mcatadv_state::mcatadv_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &mcatadv_state::mcatadv_sound_io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 256);
	screen.set_visarea(0, 320-1, 0, 224-1);
	screen.set_screen_update(FUNC(mcatadv_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.screen_vblank().append(m_vidregs, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mcatadv);
	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 0x2000 / 2);

	TMAP038(config, m_tilemap[0]);
	m_tilemap[0]->set_gfxdecode_tag(m_gfxdecode);
	m_tilemap[0]->set_gfx(0);
	m_tilemap[0]->set_tile_callback(FUNC(mcatadv_state::get_banked_color<0>));

	TMAP038(config, m_tilemap[1]);
	m_tilemap[1]->set_gfxdecode_tag(m_gfxdecode);
	m_tilemap[1]->set_gfx(1);
	m_tilemap[1]->set_tile_callback(FUNC(mcatadv_state::get_banked_color<1>));

	BUFFERED_SPRITERAM16(config, m_spriteram);

	BUFFERED_SPRITERAM16(config, m_vidregs);

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_seconds(3));  // a guess, and certainly wrong

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, "soundlatch2");

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(16'000'000) / 2)); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(0, "mono", 0.32);
	ymsnd.add_route(1, "mono", 0.5);
	ymsnd.add_route(2, "mono", 0.5);
}

void mcatadv_state::nost(machine_config &config)
{
	mcatadv(config);

	m_soundcpu->set_addrmap(AS_PROGRAM, &mcatadv_state::nost_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &mcatadv_state::nost_sound_io_map);

	ym2610_device &ymsnd(YM2610(config.replace(), "ymsnd", XTAL(16'000'000) / 2)); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(0, "mono", 0.2);
	ymsnd.add_route(1, "mono", 0.5);
	ymsnd.add_route(2, "mono", 0.5);
}


ROM_START( mcatadv )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M68000
	ROM_LOAD16_BYTE( "mca-u30e", 0x00000, 0x80000, CRC(c62fbb65) SHA1(39a30a165d4811141db8687a4849626bef8e778e) )
	ROM_LOAD16_BYTE( "mca-u29e", 0x00001, 0x80000, CRC(cf21227c) SHA1(4012811ebfe3c709ab49946f8138bc4bad881ef7) )

	ROM_REGION( 0x020000, "soundcpu", 0 ) // Z80-A
	ROM_LOAD( "u9.bin", 0x00000, 0x20000, CRC(fda05171) SHA1(2c69292573ec35034572fa824c0cae2839d23919) )

	ROM_REGION( 0x800000, "sprdata", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mca-u82.bin", 0x000000, 0x100000, CRC(5f01d746) SHA1(11b241456e15299912ee365eedb8f9d5e5ca875d) )
	ROM_LOAD16_BYTE( "mca-u83.bin", 0x000001, 0x100000, CRC(4e1be5a6) SHA1(cb19aad42dba54d6a4a33859f27254c2a3271e8c) )
	ROM_LOAD16_BYTE( "mca-u84.bin", 0x200000, 0x080000, CRC(df202790) SHA1(f6ae54e799af195860ed0ab3c85138cf2f10efa6) )
	ROM_LOAD16_BYTE( "mca-u85.bin", 0x200001, 0x080000, CRC(a85771d2) SHA1(a1817cd72f5bf0a4f24a37c782dc63ecec3b8e68) )
	ROM_LOAD16_BYTE( "mca-u86e",    0x400000, 0x080000, CRC(017bf1da) SHA1(f6446a7219275c0eff62129f59fdfa3a6a3e06c8) )
	ROM_LOAD16_BYTE( "mca-u87e",    0x400001, 0x080000, CRC(bc9dc9b9) SHA1(f525c9f994d5107752aa4d3a499ee376ec75f42b) )

	ROM_REGION( 0x080000, "bg0", 0 )
	ROM_LOAD( "mca-u58.bin", 0x000000, 0x080000, CRC(3a8186e2) SHA1(129c220d72608a8839f779ce1a6cfec8646dbf23) )

	ROM_REGION( 0x280000, "bg1", 0 )
	ROM_LOAD( "mca-u60.bin", 0x000000, 0x100000, CRC(c8942614) SHA1(244fccb9abbb04e33839dd2cd0e2de430819a18c) )
	ROM_LOAD( "mca-u61.bin", 0x100000, 0x100000, CRC(51af66c9) SHA1(1055cf78ea286f02003b0d1bf08c2d7829b36f90) )
	ROM_LOAD( "mca-u100",    0x200000, 0x080000, CRC(b273f1b0) SHA1(39318fe2aaf2792b85426ec6791b3360ac964de3) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "mca-u53.bin", 0x00000, 0x80000, CRC(64c76e05) SHA1(379cef5e0cba78d0e886c9cede41985850a3afb7) )
ROM_END

ROM_START( mcatadvj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M68000
	ROM_LOAD16_BYTE( "u30.bin", 0x00000, 0x80000, CRC(05762f42) SHA1(3675fb606bf9d7be9462324e68263f4a6c2fea1c) )
	ROM_LOAD16_BYTE( "u29.bin", 0x00001, 0x80000, CRC(4c59d648) SHA1(2ab77ea254f2c11fc016078cedcab2fffbe5ee1b) )

	ROM_REGION( 0x020000, "soundcpu", 0 ) // Z80-A
	ROM_LOAD( "u9.bin", 0x00000, 0x20000, CRC(fda05171) SHA1(2c69292573ec35034572fa824c0cae2839d23919) )

	ROM_REGION( 0x800000, "sprdata", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mca-u82.bin", 0x000000, 0x100000, CRC(5f01d746) SHA1(11b241456e15299912ee365eedb8f9d5e5ca875d) )
	ROM_LOAD16_BYTE( "mca-u83.bin", 0x000001, 0x100000, CRC(4e1be5a6) SHA1(cb19aad42dba54d6a4a33859f27254c2a3271e8c) )
	ROM_LOAD16_BYTE( "mca-u84.bin", 0x200000, 0x080000, CRC(df202790) SHA1(f6ae54e799af195860ed0ab3c85138cf2f10efa6) )
	ROM_LOAD16_BYTE( "mca-u85.bin", 0x200001, 0x080000, CRC(a85771d2) SHA1(a1817cd72f5bf0a4f24a37c782dc63ecec3b8e68) )
	ROM_LOAD16_BYTE( "u86.bin",     0x400000, 0x080000, CRC(2d3725ed) SHA1(8b4c0f280eb901113d842848ffc26371be7b6067) )
	ROM_LOAD16_BYTE( "u87.bin",     0x400001, 0x080000, CRC(4ddefe08) SHA1(5ade0a694d73f4f3891c1ab7757e37a88afcbf54) )

	ROM_REGION( 0x080000, "bg0", 0 )
	ROM_LOAD( "mca-u58.bin", 0x000000, 0x080000, CRC(3a8186e2) SHA1(129c220d72608a8839f779ce1a6cfec8646dbf23) )

	ROM_REGION( 0x280000, "bg1", 0 )
	ROM_LOAD( "mca-u60.bin", 0x000000, 0x100000, CRC(c8942614) SHA1(244fccb9abbb04e33839dd2cd0e2de430819a18c) )
	ROM_LOAD( "mca-u61.bin", 0x100000, 0x100000, CRC(51af66c9) SHA1(1055cf78ea286f02003b0d1bf08c2d7829b36f90) )
	ROM_LOAD( "u100.bin",    0x200000, 0x080000, CRC(e2c311da) SHA1(cc3217484524de94704869eaa9ce1b90393039d8) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "mca-u53.bin", 0x00000, 0x80000, CRC(64c76e05) SHA1(379cef5e0cba78d0e886c9cede41985850a3afb7) )
ROM_END

ROM_START( catt )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M68000
	ROM_LOAD16_BYTE( "catt-u30.bin",  0x00000, 0x80000, CRC(8c921e1e) SHA1(2fdaa9b743e1731f3cfe9d8334f1b759cf46855d) )
	ROM_LOAD16_BYTE( "catt-u29.bin",  0x00001, 0x80000, CRC(e725af6d) SHA1(78c08fa5744a6a953e13c0ff39736ccd4875fb72) )

	ROM_REGION( 0x020000, "soundcpu", 0 ) // Z80-A
	ROM_LOAD( "u9.bin", 0x00000, 0x20000, CRC(fda05171) SHA1(2c69292573ec35034572fa824c0cae2839d23919) )

	ROM_REGION( 0x800000, "sprdata", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mca-u82.bin", 0x000000, 0x100000, CRC(5f01d746) SHA1(11b241456e15299912ee365eedb8f9d5e5ca875d) )
	ROM_LOAD16_BYTE( "mca-u83.bin", 0x000001, 0x100000, CRC(4e1be5a6) SHA1(cb19aad42dba54d6a4a33859f27254c2a3271e8c) )
	ROM_LOAD16_BYTE( "u84.bin",     0x200000, 0x100000, CRC(843fd624) SHA1(2e16d8a909fe9447da37a87428bff0734af59a00) )
	ROM_LOAD16_BYTE( "u85.bin",     0x200001, 0x100000, CRC(5ee7b628) SHA1(feedc212ed4893d784dc6b3361930b9199c6876d) )
	ROM_LOAD16_BYTE( "mca-u86e",    0x400000, 0x080000, CRC(017bf1da) SHA1(f6446a7219275c0eff62129f59fdfa3a6a3e06c8) )
	ROM_LOAD16_BYTE( "mca-u87e",    0x400001, 0x080000, CRC(bc9dc9b9) SHA1(f525c9f994d5107752aa4d3a499ee376ec75f42b) )

	ROM_REGION( 0x100000, "bg0", 0 )
	ROM_LOAD( "u58.bin",     0x00000, 0x100000, CRC(73c9343a) SHA1(9efdddbad6244c1ed267bd954563ab43a1017c96) )

	ROM_REGION( 0x280000, "bg1", 0 )
	ROM_LOAD( "mca-u60.bin", 0x000000, 0x100000, CRC(c8942614) SHA1(244fccb9abbb04e33839dd2cd0e2de430819a18c) )
	ROM_LOAD( "mca-u61.bin", 0x100000, 0x100000, CRC(51af66c9) SHA1(1055cf78ea286f02003b0d1bf08c2d7829b36f90) )
	ROM_LOAD( "mca-u100",    0x200000, 0x080000, CRC(b273f1b0) SHA1(39318fe2aaf2792b85426ec6791b3360ac964de3) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "u53.bin",     0x00000, 0x100000, CRC(99f2a624) SHA1(799e8e40e8bdcc8fa4cd763a366cc32473038a49) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "peel18cv8.u1", 0x0000, 0x0155, NO_DUMP )
	ROM_LOAD( "gal16v8a.u10", 0x0200, 0x0117, NO_DUMP )
ROM_END

ROM_START( nost )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M68000
	ROM_LOAD16_BYTE( "nos-pe-u.bin", 0x00000, 0x80000, CRC(4b080149) SHA1(e1dbbe5bf554c7c5731cc3079850f257417e3caa) )
	ROM_LOAD16_BYTE( "nos-po-u.bin", 0x00001, 0x80000, CRC(9e3cd6d9) SHA1(db5351ff9a05f602eceae62c0051c16ae0e4ead9) )

	ROM_REGION( 0x040000, "soundcpu", 0 ) // Z80-A
	ROM_LOAD( "nos-ps.u9", 0x00000, 0x40000, CRC(832551e9) SHA1(86fc481b1849f378c88593594129197c69ea1359) )

	ROM_REGION( 0x800000, "sprdata", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nos-se-0.u82", 0x000000, 0x100000, CRC(9d99108d) SHA1(466540989d7b1b7f6dc7acbae74f6a8201973d45) )
	ROM_LOAD16_BYTE( "nos-so-0.u83", 0x000001, 0x100000, CRC(7df0fc7e) SHA1(2e064cb5367b2839d736d339c4f1a44785b4eedf) )
	ROM_LOAD16_BYTE( "nos-se-1.u84", 0x200000, 0x100000, CRC(aad07607) SHA1(89c51a9cb6b8d8ed3a357f5d8ac8399ff1c7ad46) )
	ROM_LOAD16_BYTE( "nos-so-1.u85", 0x200001, 0x100000, CRC(83d0012c) SHA1(831d36521693891f44e7adcc2ba63fef5d493821) )
	ROM_LOAD16_BYTE( "nos-se-2.u86", 0x400000, 0x080000, CRC(d99e6005) SHA1(49aae72111334ff5cd0fd86500882f559ff921f9) )
	ROM_LOAD16_BYTE( "nos-so-2.u87", 0x400001, 0x080000, CRC(f60e8ef3) SHA1(4f7472b5a465e6cc6a5df520ebfe6a544739dd28) )

	ROM_REGION( 0x180000, "bg0", 0 )
	ROM_LOAD( "nos-b0-0.u58", 0x000000, 0x100000, CRC(0214b0f2) SHA1(678fa3dc739323bda6d7bbb1c7a573c976d69356) )
	ROM_LOAD( "nos-b0-1.u59", 0x100000, 0x080000, CRC(3f8b6b34) SHA1(94c48614782ce6405965bcf6029e3bcc24a6d84f) )

	ROM_REGION( 0x180000, "bg1", 0 )
	ROM_LOAD( "nos-b1-0.u60", 0x000000, 0x100000, CRC(ba6fd0c7) SHA1(516d6e0c4dc6fb12ec9f30877ea1c582e7440a19) )
	ROM_LOAD( "nos-b1-1.u61", 0x100000, 0x080000, CRC(dabd8009) SHA1(1862645b8d6216c3ec2b8dbf74816b8e29dea14f) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "nossn-00.u53", 0x00000, 0x100000, CRC(3bd1bcbc) SHA1(1bcad43792e985402db4eca122676c2c555f3313) )
ROM_END

ROM_START( nostj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M68000
	ROM_LOAD16_BYTE( "nos-pe-j.u30", 0x00000, 0x80000, CRC(4b080149) SHA1(e1dbbe5bf554c7c5731cc3079850f257417e3caa) )
	ROM_LOAD16_BYTE( "nos-po-j.u29", 0x00001, 0x80000, CRC(7fe241de) SHA1(aa4ffd81cb73efc59690c2038ae9375021a775a4) )

	ROM_REGION( 0x040000, "soundcpu", 0 ) // Z80-A
	ROM_LOAD( "nos-ps.u9", 0x00000, 0x40000, CRC(832551e9) SHA1(86fc481b1849f378c88593594129197c69ea1359) )

	ROM_REGION( 0x800000, "sprdata", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nos-se-0.u82", 0x000000, 0x100000, CRC(9d99108d) SHA1(466540989d7b1b7f6dc7acbae74f6a8201973d45) )
	ROM_LOAD16_BYTE( "nos-so-0.u83", 0x000001, 0x100000, CRC(7df0fc7e) SHA1(2e064cb5367b2839d736d339c4f1a44785b4eedf) )
	ROM_LOAD16_BYTE( "nos-se-1.u84", 0x200000, 0x100000, CRC(aad07607) SHA1(89c51a9cb6b8d8ed3a357f5d8ac8399ff1c7ad46) )
	ROM_LOAD16_BYTE( "nos-so-1.u85", 0x200001, 0x100000, CRC(83d0012c) SHA1(831d36521693891f44e7adcc2ba63fef5d493821) )
	ROM_LOAD16_BYTE( "nos-se-2.u86", 0x400000, 0x080000, CRC(d99e6005) SHA1(49aae72111334ff5cd0fd86500882f559ff921f9) )
	ROM_LOAD16_BYTE( "nos-so-2.u87", 0x400001, 0x080000, CRC(f60e8ef3) SHA1(4f7472b5a465e6cc6a5df520ebfe6a544739dd28) )

	ROM_REGION( 0x180000, "bg0", 0 )
	ROM_LOAD( "nos-b0-0.u58", 0x000000, 0x100000, CRC(0214b0f2) SHA1(678fa3dc739323bda6d7bbb1c7a573c976d69356) )
	ROM_LOAD( "nos-b0-1.u59", 0x100000, 0x080000, CRC(3f8b6b34) SHA1(94c48614782ce6405965bcf6029e3bcc24a6d84f) )

	ROM_REGION( 0x180000, "bg1", 0 )
	ROM_LOAD( "nos-b1-0.u60", 0x000000, 0x100000, CRC(ba6fd0c7) SHA1(516d6e0c4dc6fb12ec9f30877ea1c582e7440a19) )
	ROM_LOAD( "nos-b1-1.u61", 0x100000, 0x080000, CRC(dabd8009) SHA1(1862645b8d6216c3ec2b8dbf74816b8e29dea14f) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "nossn-00.u53", 0x00000, 0x100000, CRC(3bd1bcbc) SHA1(1bcad43792e985402db4eca122676c2c555f3313) )
ROM_END

ROM_START( nostk )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M68000
	ROM_LOAD16_BYTE( "nos-pe-t.u30", 0x00000, 0x80000, CRC(bee5fbc8) SHA1(a8361fa004bb31471f973ece51a9a87b9f3438ab) )
	ROM_LOAD16_BYTE( "nos-po-t.u29", 0x00001, 0x80000, CRC(f4736331) SHA1(7a6db2db1a4dbf105c22e15deff6f6032e04609c) )

	ROM_REGION( 0x040000, "soundcpu", 0 ) // Z80-A
	ROM_LOAD( "nos-ps.u9", 0x00000, 0x40000, CRC(832551e9) SHA1(86fc481b1849f378c88593594129197c69ea1359) )

	ROM_REGION( 0x800000, "sprdata", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nos-se-0.u82", 0x000000, 0x100000, CRC(9d99108d) SHA1(466540989d7b1b7f6dc7acbae74f6a8201973d45) )
	ROM_LOAD16_BYTE( "nos-so-0.u83", 0x000001, 0x100000, CRC(7df0fc7e) SHA1(2e064cb5367b2839d736d339c4f1a44785b4eedf) )
	ROM_LOAD16_BYTE( "nos-se-1.u84", 0x200000, 0x100000, CRC(aad07607) SHA1(89c51a9cb6b8d8ed3a357f5d8ac8399ff1c7ad46) )
	ROM_LOAD16_BYTE( "nos-so-1.u85", 0x200001, 0x100000, CRC(83d0012c) SHA1(831d36521693891f44e7adcc2ba63fef5d493821) )
	ROM_LOAD16_BYTE( "nos-se-2.u86", 0x400000, 0x080000, CRC(d99e6005) SHA1(49aae72111334ff5cd0fd86500882f559ff921f9) )
	ROM_LOAD16_BYTE( "nos-so-2.u87", 0x400001, 0x080000, CRC(f60e8ef3) SHA1(4f7472b5a465e6cc6a5df520ebfe6a544739dd28) )

	ROM_REGION( 0x180000, "bg0", 0 )
	ROM_LOAD( "nos-b0-0.u58", 0x000000, 0x100000, CRC(0214b0f2) SHA1(678fa3dc739323bda6d7bbb1c7a573c976d69356) )
	ROM_LOAD( "nos-b0-1.u59", 0x100000, 0x080000, CRC(3f8b6b34) SHA1(94c48614782ce6405965bcf6029e3bcc24a6d84f) )

	ROM_REGION( 0x180000, "bg1", 0 )
	ROM_LOAD( "nos-b1-0.u60", 0x000000, 0x100000, CRC(ba6fd0c7) SHA1(516d6e0c4dc6fb12ec9f30877ea1c582e7440a19) )
	ROM_LOAD( "nos-b1-1.u61", 0x100000, 0x080000, CRC(dabd8009) SHA1(1862645b8d6216c3ec2b8dbf74816b8e29dea14f) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "nossn-00.u53", 0x00000, 0x100000, CRC(3bd1bcbc) SHA1(1bcad43792e985402db4eca122676c2c555f3313) )
ROM_END

} // anonymous namespace


GAME( 1993, mcatadv,  0,       mcatadv, mcatadv, mcatadv_state, empty_init, ROT0,   "Wintechno", "Magical Cat Adventure",         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, mcatadvj, mcatadv, mcatadv, mcatadv, mcatadv_state, empty_init, ROT0,   "Wintechno", "Magical Cat Adventure (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, catt,     mcatadv, mcatadv, mcatadv, mcatadv_state, empty_init, ROT0,   "Wintechno", "Catt (Japan)",                  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, nost,     0,       nost,    nost,    mcatadv_state, empty_init, ROT270, "Face",      "Nostradamus",                   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, nostj,    nost,    nost,    nost,    mcatadv_state, empty_init, ROT270, "Face",      "Nostradamus (Japan)",           MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, nostk,    nost,    nost,    nost,    mcatadv_state, empty_init, ROT270, "Face",      "Nostradamus (Korea)",           MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
