// license:BSD-3-Clause
// copyright-holders:David Haywood
/*******************************************************************************
 Welltris (c)1991 Video System

********************************************************************************
 hardware is similar to aerofgt.cpp but with slightly different sprites, sound,
 and an additional 'pixel' layer used for the backdrops

 Driver by David Haywood, with help from Steph from The Ultimate Patchers
 Thanks to the authors of aerofgt.cpp and fromance.cpp on which most of this is
 based
********************************************************************************
OW-13 CPU

CPU  : MC68000P10
Sound: Z80 YM2610 YM3016
OSC  : 14.31818MHz (X1), 12.000MHz (X2),
       8.000MHz (X3), 20.0000MHz (OSC1)

ROMs:
j1.7 - Main programs (271000 compatible onetime)
j2.8 /

lh532j11.9  - Data
lh532j10.10 /

3.144 - Sound program (27c1000)

lh534j09.123 - Samples
lh534j10.124 |
lh534j11.126 /

lh534j12.77 - BG chr.

046.93 - OBJ chr.
048.94 /

PALs (16L8):
ow13-1.1
ow13-2.2
ow13-3.97
ow13-4.115

Custom chips:
V-SYSTEM C7-01 GGA
VS8905 6620 9039 ABBA
V-SYSTEM VS8904 GGB
V-SYSTEM VS8803 6082 9040 EBBB

********************************************************************************

 it's impossible to know what some of the video registers do due to lack of
 evidence (bg palette has a selector, but I'm not sure which ... test mode
 colours use different palette on rgb test

********************************************************************************

 Info from Steph (a lot of thanks to him for looking at this)
 ---------------


The main thing is that, as in 'ridleofp', there is some code that could be used
if there was no "brute hack" in the code to never use it ...

The "brute hack" is there :

00B91C: 0000 0030                ori.b   #$30, D0

Replace with 0x4e714e71 and you'll be able to test the "hidden features" .
* See #define in driver, maybe there are other versions of the game?..

Dip Switch 1 is read from $f00d and is stored at $8802, while Switch 2 is read from
$f00f and is stored at $8803 ...

"DIPSW 2-5" (bit 4 of DSW2) is tested at address 0x007a18 :

00A718: 0838 0004 8803           btst    #$4, $8803.w

I haven't been able to figure out what this Dip Switch does as I haven't found a way
to call the routine that seems to start at 0x00a710 8( If you find ANY infos, please
let me know ...

"DIPSW 2-6" (bit 5 of DSW2) is probably one of the most "important" Dip Switch, as
it sets the maximum player from 2 (when OFF) to 4 (when ON) ... Watch the "attract
mode" to see how the game looks like ...


Differences between 2 players and 4 players mode :

1) 2 players mode (DIPSW 2-5 is OFF)

There are 2 coin slots (each one has its own coinage), there are only 2 "Start" buttons
and 2 sets of controls :

  1  "P1 Start"
  2  "P2 Start"
  3  no effect
  4  no effect
  5  "Coin 1"  (reads "DIPSW 1-1" to "DIPSW 1-4")
  6  "Coin 2"  (reads "DIPSW 1-5" to "DIPSW 1-8")
  7  no effect (in fact, calls the routine (see below) but doesn't add credits)
  8  no effect
  0  "Service" (adds 1 credit)

  P1 and P2 controls are OK, while P3 and P4 controls have no effect (and you can't
  even test them !) ...

In a 2 players game, the players can move their pieces on the following walls :

  P1  West and South walls
  P2  East and North walls

"DIPSW 2-3" determines how many credits are needed for a 2 players game :

  ON   2 (this should be the default value)
  OFF  1

Note that in a 2 players game, "PLAYER 1" is displayed twice ...

2) 4 players mode  (DIPSW 2-5 is ON)

There are 4 coin slots (each one using the SAME coinage), there are 4 "Start" buttons
and 4 sets of controls :

  1  used only in "test mode"
  2  used only in "test mode"
  3  used only in "test mode"
  4  used only in "test mode"
  5  "Coin 1"  (reads "DIPSW 1-1" to "DIPSW 1-4")
  6  "Coin 2"  (reads "DIPSW 1-1" to "DIPSW 1-4")
  7  "Coin 3"  (reads "DIPSW 1-1" to "DIPSW 1-4")
  8  "Coin 4"  (reads "DIPSW 1-1" to "DIPSW 1-4")
  0  "Service" (adds 1 credit)

  P1 to P4 controls are OK ...

You can test ALL these inputs when you are in the "test mode" : even if you don't see
the ones for players 3 and 4 when you reboot, they will "appear" once you press the key !
However "DISPW 2-4" and ""DISPW 2-5" will still display "N.C." ("Not Connected" ?)
followed by OFF or ON ...

While in the "test mode", the controls will be shown with an arrow :

  MAME key    player 3 display    player 4 display

    Up              Right               Left
    Down            Left                Right
    Left            Up                  Down
    Right           Down                Up

Note that I haven't been able to find where the "Tilt" key was mapped ...

In a 4 players game, each player can move his pieces on ONE following wall :

  P1  North wall
  P2  East  wall
  P3  South wall
  P4  West  wall

When 1 credit is inserted, a timer appears to wait for players to enter the game, and
it's IMPOSSIBLE to start a DEF_STR( Normal ) 1 player game (with the 4 walls) ...

To select a player, press one of his 2 buttons ...

"DIPSW 2-3" is VERY important here :

  - When it's OFF, only player 1 can play, the number of credits is decremented when
    you press the player buttons ... If you wait until timer reaches 0 without selecting
    a player, 1 credit will be subtracted, and you'll start a game with player 1 ...
  - When it's ON, 1 credit will be automatically subtracted, then the 4 players can
    play by pressing one of their buttons ... If you wait until timer reaches 0 without
    selecting a player, you'll start a game with player 1 ...

"Unfortunately", it's an endless game, has you have NO penalty when a piece can't be
placed 8( Also note that there is NO possibility to "join in" once the game has started ...


Some useful addresses :

  0xff803a ($803a) : credits

Even if display is limited to 9, there doesn't seem to be any limit to the real number
of credits ...

  0xff803b ($803b) : credits that will be added to $803a when you press '6'
  0xff803c ($803c) : credits that will be added to $803a when you press '7'
  0xff803d ($803d) : credits that will be added to $803a when you press '8'
  0xff803e ($803e) : credits that will be added to $803a when you press '0'

These credits will be added by routine described below ...

  0xff8959 ($8959) : "handicap" for player 1 (in a 1 or 2 players game)
  0xff89d9 ($89d9) : "handicap" for player 2 (in a 2 players game only)

"Handicap" range is 0x00 (piece fall from the top of the well) to 0x0c (when the game
is over) ... To end a game quickly for a player, set his value to 0x0b and wait until
a piece can't be placed ...


Some useful routines :

  $b080 : initialisation

When this routine is called with A4 = $400 (at $2466), it determines which routine will
be called when a coin is inserted ... It stores 0x0490 at 0xff8026 ($8026), then it
calls routine at $414 which tests "DIPSW 2-5" ... If it's ON, it stores 0x04a2 ...

Then it stores addresses of routines to add credits according to "DIPSW 1-1" to
"DIPSW 1-4" (stored at 0xff802a ($802a)), and to "DIPSW 1-5" to "DIPSW 1-8" (stored at
0xff802e ($802e)) ...

  $490 : reads coins inputs ("2 players mode")

This routines reads "Coin 1" status, and adds credits according to routine stored at
0xff802a ($802a) ... Then it reads "Coin 2" status, and adds credits according to
routine stored at 0xff802e ($802e) ...

  $4a2 : reads coins inputs ("4 players mode")

This routines reads status of the 4 "Coin", and adds credits according to routine
stored at 0xff802a ($802a) ...

  $50c : read status of a "Coin" button

This routines checks if value is <> 0x00 ... If this is the case, the routine that adds
credits is called, and the value is reset to 0x00 ...

  $ba36 : determines "Coin" status

This routines splits the inputs into consecutive addresses : if the button is pressed,
0x01 will be added :

  - status of "Coin 1"  is stored at address 0xff8030 ($8030)
  - status of "Coin 2"  is stored at address 0xff8031 ($8031)
  - status of "Coin 3"  is stored at address 0xff8032 ($8032)
  - status of "Coin 4"  is stored at address 0xff8033 ($8033)
  - status of "Service" is stored at address 0xff8034 ($8034)

  $9002 : checks "1 Player Start" and "2 Players Start" buttons

  $9018 : "1 Player Start" button is pressed

If enough credits, the number of credits is decremented by 1 ...

  $9056 : "2 Player Start" button is pressed

If enough credits, the number of credits is decremented by 1 or 2, depending of
"DIPSW 2-3" ...

  $90ce : reads status of "DIPSW 2-3" Dip Switch

This routine determines how many credits (1 or 2) are needed for a 2 players game ...

  $b908 : stores Dip Switches in memory

  $b91c : "brute hack" to disable "4 players mode" features

  $b924 : reads status of "DIPSW 2-7" Dip Switch for "screen flipping" support ...

  $9962 : reads status of "DIPSW 2-4" Dip Switch for "demo sounds" support ...

  $cf70 : sound routine

There are read/writes on bit 7 of 0xfff009 ($f009) ...

*/
/*******************************************************************************

    Miyasu Nonki no Quiz 18-Kin (Japan)
    (c)1992 EIM

    Added by Takahiro Nogi 2003/08/15 -


Board:  OW-13 CPU
CPU:    68000-10
        Z80-B
Sound:  YM2610
OSC:    20.00000MHz
        14.31818MHz
        12.000MHz
        8.000MHz
Custom: C7-01 GGA
        VS8803
        VS8904
        VS8905


1-IC8.BIN    main prg.
2-IC7.BIN
IC10.BIN
IC9.BIN

3-IC144.BIN  sound prg.

IC123.BIN    samples
IC124.BIN
IC126.BIN

IC77.BIN     BG chr.
IC78.BIN
IC79.BIN

IC93.BIN     OBJ chr.
IC94.BIN


TODO:

- Couldn't figure out sprite table initialize routine, so I initialize it manually.

*******************************************************************************/

#include "emu.h"

#include "vsystem_gga.h"
#include "vsystem_spr2.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

#define WELLTRIS_4P_HACK 0

class welltris_state : public driver_device
{
public:
	welltris_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_soundlatch(*this, "soundlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram"),
		m_pixelram(*this, "pixelram"),
		m_charvideoram(*this, "charvideoram"),
		m_soundbank(*this, "soundbank")
	{ }

	void quiz18k(machine_config &config);
	void welltris(machine_config &config);

	void init_quiz18k();
	void init_welltris();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<vsystem_spr2_device> m_spr_old;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_pixelram;
	required_shared_ptr<uint16_t> m_charvideoram;
	required_memory_bank m_soundbank;

	tilemap_t *m_char_tilemap;
	uint8_t m_gfxbank[2];
	uint16_t m_charpalettebank;
	uint16_t m_spritepalettebank;
	uint16_t m_pixelpalettebank;
	int m_scrollx;
	int m_scrolly;

	void sound_bankswitch_w(uint8_t data);
	void soundlatch_pending_w(int state);
	void palette_bank_w(offs_t offset, uint8_t data);
	void gfxbank_w(offs_t offset, uint8_t data);
	void scrollreg_w(offs_t offset, uint16_t data);
	void charvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void setbank(int num, int bank);
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sound_port_map(address_map &map);
};


void welltris_state::setbank(int num, int bank)
{
	if (m_gfxbank[num] != bank)
	{
		m_gfxbank[num] = bank;
		m_char_tilemap->mark_all_dirty();
	}
}


// Not really enough evidence here

void welltris_state::palette_bank_w(offs_t offset, uint8_t data)
{
	if (m_charpalettebank != (data & 0x03))
	{
		m_charpalettebank = (data & 0x03);
		m_char_tilemap->mark_all_dirty();
	}

	flip_screen_set(data & 0x80);

	m_spritepalettebank = (data & 0x20) >> 5;
	m_pixelpalettebank = (data & 0x08) >> 3;
}

void welltris_state::gfxbank_w(offs_t offset, uint8_t data)
{
	setbank(0, (data & 0xf0) >> 4);
	setbank(1, data & 0x0f);
}

void welltris_state::scrollreg_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0: m_scrollx = data - 14; break;
		case 1: m_scrolly = data +  0; break;
	}
}

TILE_GET_INFO_MEMBER(welltris_state::get_tile_info)
{
	uint16_t code = m_charvideoram[tile_index];
	int bank = (code & 0x1000) >> 12;

	tileinfo.set(0,
			(code & 0x0fff) + (m_gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + (8 * m_charpalettebank),
			0);
}

void welltris_state::charvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_charvideoram[offset]);
	m_char_tilemap->mark_tile_dirty(offset);
}

void welltris_state::video_start()
{
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(welltris_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_char_tilemap->set_transparent_pen(15);

	save_item(NAME(m_gfxbank));
	save_item(NAME(m_charpalettebank));
	save_item(NAME(m_spritepalettebank));
	save_item(NAME(m_pixelpalettebank));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}

void welltris_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 512 / 2; x++) {
			int pixdata = m_pixelram[(x & 0xff) + (y & 0xff) * 256];

			bitmap.pix(y, (x * 2) + 0) = (pixdata >> 8) + (0x100 * m_pixelpalettebank) + 0x400;
			bitmap.pix(y, (x * 2) + 1) = (pixdata & 0xff) + (0x100 * m_pixelpalettebank) + 0x400;
		}
	}
}

uint32_t welltris_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_char_tilemap->set_scrollx(0, m_scrollx);
	m_char_tilemap->set_scrolly(0, m_scrolly);

	draw_background(bitmap, cliprect);
	m_char_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_spr_old->draw_sprites(m_spriteram, m_spriteram.bytes(), m_spritepalettebank, bitmap, cliprect, screen.priority(), 0);
	return 0;
}


void welltris_state::sound_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x03);
}

void welltris_state::soundlatch_pending_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);

	// sound comms is 2-way (see pending_r in "SYSTEM"),
	// NMI routine is very short, so briefly set perfect_quantum to make sure that the timing is right
	if (state)
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}


void welltris_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x17ffff).rom();
	map(0x800000, 0x81ffff).ram().share(m_pixelram);    // graph_1 & 2
	map(0xff8000, 0xffbfff).ram();                      // work
	map(0xffc000, 0xffc3ff).ram().share(m_spriteram);
	map(0xffd000, 0xffdfff).ram().w(FUNC(welltris_state::charvideoram_w)).share(m_charvideoram);
	map(0xffe000, 0xffefff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("P1");                 // bottom controls
	map(0xfff001, 0xfff001).w(FUNC(welltris_state::palette_bank_w));
	map(0xfff002, 0xfff003).portr("P2");                 // top controls
	map(0xfff003, 0xfff003).w(FUNC(welltris_state::gfxbank_w));
	map(0xfff004, 0xfff005).portr("P3");                 // left side controls
	map(0xfff004, 0xfff007).w(FUNC(welltris_state::scrollreg_w));
	map(0xfff006, 0xfff007).portr("P4");                 // right side controls
	map(0xfff008, 0xfff009).portr("SYSTEM");             // bit 5 tested at start of IRQ 1
	map(0xfff009, 0xfff009).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xfff00a, 0xfff00b).portr("EXTRA");              // P3+P4 coin + start buttons
	map(0xfff00c, 0xfff00d).portr("DSW1");
	map(0xfff00e, 0xfff00f).portr("DSW2");
	map(0xfff00c, 0xfff00f).w("gga", FUNC(vsystem_gga_device::write)).umask16(0x00ff);
}

void welltris_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void welltris_state::sound_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(welltris_state::sound_bankswitch_w));
	map(0x08, 0x0b).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x10, 0x10).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x18, 0x18).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}

static INPUT_PORTS_START( welltris )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // Test (used to go through tests in service mode)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )       // Tested at start of IRQ 1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )   // Service (adds a coin)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", generic_latch_8_device, pending_r) // pending sound command

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#if WELLTRIS_4P_HACK
	/* These can actually be read in the test mode even if they're not used by the game without patching the code
	   might be useful if a real 4 player version ever turns up if it was ever produced */
	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
#else
	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
#endif

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
#if WELLTRIS_4P_HACK
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
#else
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, "2-1, 4-2, 5-3, 6-4" )
	PORT_DIPSETTING(      0x0003, "2-1, 4-3" )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, "1-1, 2-2, 3-3, 4-5" )
	PORT_DIPSETTING(      0x0002, "1-1, 2-2, 3-3, 4-4, 5-6" )
	PORT_DIPSETTING(      0x0000, "1-1, 2-3" )
	PORT_DIPSETTING(      0x0005, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0040, "2-1, 4-2, 5-3, 6-4" )
	PORT_DIPSETTING(      0x0030, "2-1, 4-3" )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, "1-1, 2-2, 3-3, 4-5" )
	PORT_DIPSETTING(      0x0020, "1-1, 2-2, 3-3, 4-4, 5-6" )
	PORT_DIPSETTING(      0x0000, "1-1, 2-3" )
	PORT_DIPSETTING(      0x0050, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )          // "Super" in test mode
	PORT_DIPNAME( 0x0004, 0x0000, "Coin Mode" )
	PORT_DIPSETTING(      0x0004, "Mono Player" )
	PORT_DIPSETTING(      0x0000, "Many Player" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
#if WELLTRIS_4P_HACK
	// again might be handy if a real 4 player version shows up
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 2-5 (see notes)" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "4 Players Mode (see notes)" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#else
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 2-5 (unused)" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 2-6 (unused)" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#endif
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) // Flip screen not currently supported
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( quiz18k )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   // pending sound command

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT (0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT (0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT (0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) // Flip screen not currently supported
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Title Logo Type" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END




static GFXDECODE_START( gfx_welltris )
	GFXDECODE_ENTRY( "chars", 0, gfx_8x8x4_packed_lsb, 16* 0, 4*16 )
GFXDECODE_END

static GFXDECODE_START( gfx_welltris_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_lsb, 16*96, 2*16 )
GFXDECODE_END


void welltris_state::init_welltris()
{
#if WELLTRIS_4P_HACK
	// A Hack which shows 4 player mode in code which is disabled
	uint16_t *ram = (uint16_t *)memregion("maincpu")->base();
	ram[0xb91c / 2] = 0x4e71;
	ram[0xb91e / 2] = 0x4e71;
#endif
}

void welltris_state::machine_start()
{
	m_soundbank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x8000);
}

void welltris_state::welltris(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000 / 2);  // 10 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &welltris_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(welltris_state::irq1_line_hold));

	Z80(config, m_audiocpu, 8000000 / 2);     // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &welltris_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &welltris_state::sound_port_map); // IRQs are triggered by the YM2610

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(15, 367-1, 8, 248-1);
	screen.set_screen_update(FUNC(welltris_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_welltris);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 2048);

	VSYSTEM_GGA(config, "gga", XTAL(14'318'181) / 2); // divider not verified

	VSYSTEM_SPR2(config, m_spr_old, 0, "palette", gfx_welltris_spr);
	m_spr_old->set_pritype(-1);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(welltris_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.75);
	ymsnd.add_route(2, "mono", 0.75);
}

void welltris_state::quiz18k(machine_config &config)
{
	welltris(config);

	// basic machine hardware
	subdevice<screen_device>("screen")->set_visarea(15, 335-1, 0, 224-1);

	m_spr_old->set_offsets(6, 1);
}



ROM_START( welltris )
	ROM_REGION( 0x180000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "j2u.8", 0x000000, 0x20000, CRC(7488fe94) SHA1(41874366e2ab763cd827ff712b76ea2da0f9af6a) )
	ROM_LOAD16_BYTE( "j1u.7", 0x000001, 0x20000, CRC(571413ac) SHA1(5eb9387efb9c1597005abff4d79f4b32aa7c93b2) )
	// Space
	ROM_LOAD16_BYTE( "lh532j10.10", 0x100000, 0x40000, CRC(1187c665) SHA1(c6c55016e46805694348b386e521a3ef1a443621) )
	ROM_LOAD16_BYTE( "lh532j11.9",  0x100001, 0x40000, CRC(18eda9e5) SHA1(c01d1dc6bfde29797918490947c89440b58d5372) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "3.144", 0x00000, 0x20000, CRC(ae8f763e) SHA1(255419e02189c2e156c1fbcb0cd4aedd14ed8ffa) )

	ROM_REGION( 0x0a0000, "chars", 0 )
	ROM_LOAD( "lh534j12.77", 0x000000, 0x80000, CRC(b61a8b74) SHA1(e17f7355375bdc166ef8131f7de9dbda5453f570) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD32_WORD( "046.93", 0x000000, 0x40000, CRC(31d96d77) SHA1(5613ef9e9e38406b4e64fc8983ea50b57613923e) )
	ROM_LOAD32_WORD( "048.94", 0x000002, 0x40000, CRC(bb4643da) SHA1(38d54f8c3dba09b528df05d748ab5bdf5d028453) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "lh534j09.123", 0x00000, 0x80000, CRC(6c2ce9a5) SHA1(a4011ecfb505191c9934ba374933cd11b331d55a) )
	ROM_LOAD( "lh534j10.124", 0x80000, 0x80000, CRC(e3682221) SHA1(3e1cda07cf451955dc473eabe007854e5148ae27) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "lh534j11.126", 0x00000, 0x80000, CRC(bf85fb0d) SHA1(358f91bbff2d3260f83b5a0422c0d985d1735cef) )
ROM_END

ROM_START( welltrisj )
	ROM_REGION( 0x180000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "j2.8", 0x000000, 0x20000, CRC(68ec5691) SHA1(8615415c5c98aa9caa0878a8251da7985f050f94) )
	ROM_LOAD16_BYTE( "j1.7", 0x000001, 0x20000, CRC(1598ea2c) SHA1(e9150c3ab9b5c0eb9a5fee3e071358f92a005078) )
	// Space
	ROM_LOAD16_BYTE( "lh532j10.10", 0x100000, 0x40000, CRC(1187c665) SHA1(c6c55016e46805694348b386e521a3ef1a443621) )
	ROM_LOAD16_BYTE( "lh532j11.9",  0x100001, 0x40000, CRC(18eda9e5) SHA1(c01d1dc6bfde29797918490947c89440b58d5372) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "3.144", 0x00000, 0x20000, CRC(ae8f763e) SHA1(255419e02189c2e156c1fbcb0cd4aedd14ed8ffa) )

	ROM_REGION( 0x0a0000, "chars", 0 )
	ROM_LOAD( "lh534j12.77", 0x000000, 0x80000, CRC(b61a8b74) SHA1(e17f7355375bdc166ef8131f7de9dbda5453f570) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD32_WORD( "046.93", 0x000000, 0x40000, CRC(31d96d77) SHA1(5613ef9e9e38406b4e64fc8983ea50b57613923e) )
	ROM_LOAD32_WORD( "048.94", 0x000002, 0x40000, CRC(bb4643da) SHA1(38d54f8c3dba09b528df05d748ab5bdf5d028453) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "lh534j09.123", 0x00000, 0x80000, CRC(6c2ce9a5) SHA1(a4011ecfb505191c9934ba374933cd11b331d55a) )
	ROM_LOAD( "lh534j10.124", 0x80000, 0x80000, CRC(e3682221) SHA1(3e1cda07cf451955dc473eabe007854e5148ae27) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "lh534j11.126", 0x00000, 0x80000, CRC(bf85fb0d) SHA1(358f91bbff2d3260f83b5a0422c0d985d1735cef) )
ROM_END

ROM_START( quiz18k )
	ROM_REGION( 0x180000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "1-ic8.bin", 0x000000, 0x20000, CRC(10a64336) SHA1(d63c0752385e1d66b09a7197e267dcd0e5e93be8) )
	ROM_LOAD16_BYTE( "2-ic7.bin", 0x000001, 0x20000, CRC(8b21b431) SHA1(278238ab4a5d11577c5ab3c7462b429f510a1d50) )
	// Space
	ROM_LOAD16_BYTE( "ic10.bin", 0x100000, 0x40000, CRC(501453a3) SHA1(d127f417f1c52333e478ac397fbe8a2f223b1ce7) )
	ROM_LOAD16_BYTE( "ic9.bin",  0x100001, 0x40000, CRC(99b6840f) SHA1(8409a33c64729066bfed6e49dcd84f30906274cb) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "3-ic144.bin", 0x00000, 0x20000, CRC(72d372e3) SHA1(d077e34947de1050b68d76506cc8926b06a94a76) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "ic77.bin", 0x000000, 0x80000, CRC(af3b6fd1) SHA1(d22f7cf62a94ae3a2dcb0236630e9ac88d5e528b) )
	ROM_LOAD( "ic78.bin", 0x080000, 0x80000, CRC(44bbdef3) SHA1(cd91eaf98602ef3448f49c8287591aa845afb874) )
	ROM_LOAD( "ic79.bin", 0x100000, 0x80000, CRC(d721e169) SHA1(33ec819c4e7b4dbab41756af9eca857107d96c8b) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD32_WORD( "ic93.bin", 0x000000, 0x80000, CRC(4d387c5e) SHA1(e77aea06b9b2dc8ada5618aaf83bb80f63670363) )
	ROM_LOAD32_WORD( "ic94.bin", 0x000002, 0x80000, CRC(6be2f164) SHA1(6a3ca63d6238d587a50718d2a6c76f01932c76c3) )

	ROM_REGION( 0x140000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "ic123.bin", 0x00000, 0x80000, CRC(ee4995cf) SHA1(1b47938ddc87709f8d118b86fe62602972c77ced) )
	ROM_LOAD( "ic124.bin", 0x80000, 0x40000, CRC(076f58c3) SHA1(bd78f39b85b2697e733896705355e21b8d2a141d) )

	ROM_REGION( 0x040000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "ic126.bin", 0x00000, 0x40000, CRC(7a92fbc9) SHA1(c13be1e84fc8e74c85d25d3357e078bc9e264682) )
ROM_END

} // anonymous namespace


GAME( 1991, welltris,  0,        welltris, welltris, welltris_state, init_welltris, ROT0,   "Video System Co.", "Welltris (World?, 2 players)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, welltrisj, welltris, welltris, welltris, welltris_state, init_welltris, ROT0,   "Video System Co.", "Welltris (Japan, 2 players)",  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, quiz18k,   0,        quiz18k,  quiz18k,  welltris_state, empty_init,    ROT0,   "EIM",              "Miyasu Nonki no Quiz 18-Kin",  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
