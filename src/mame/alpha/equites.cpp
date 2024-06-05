// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Nicola Salmoria
/*******************************************************************************

Equites           (c) 1984 Alpha Denshi Co./Sega
Bull Fighter      (c) 1984 Alpha Denshi Co./Sega
Gekisou           (c) 1985 Eastern Corp.
The Koukouyakyuh  (c) 1985 Alpha Denshi Co.
Bingo Time        (c) 1986 CLS

The following are not dumped yet:
Champion Croquet  (c) 1984 Alpha Denshi Co. (sports)
Violent Run       (c) 1985 Eastern Corp. (export version of Gekisou)
Tune Pit(?)       (c) 1985 Alpha Denshi Co.
Perfect Janputer  (c) 1984 Alpha Denshi Co. (4-player Mahjong)

Driver by Acho A. Tang, Nicola Salmoria
Many thanks to Corrado Tomaselli for precious hardware info.


Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - To enter sort of "test mode", COIN switches 0 and 1 need to be ON when the game is reset.


1) 'equites'

  - When in "test mode", press START1 to cycle through next sound, and press START2
    to directly test the inputs and the Dip Switches.
  - When the number of buttons is set to 2, you need to press BOTH BUTTON1 and
    BUTTON2 to have the same effect as BUTTON3.


2) 'bullfgtr'

  - When in "test mode", press START1 to cycle through next sound, and press START2
    to directly test the inputs and the Dip Switches.
  - I'm not sure I understand how the coinage is handled, and so it's hard to make
    a good description. Anyway, the values are correct.


3) 'kouyakyu'

  - When in "test mode", press START1 to cycle through next sound, and press START2
    to directly test the inputs and the Dip Switches.
  - Bit 1 of Dip Switch is only read in combination of bit 0 during P.O.S.T. to
    enter the "test mode", but it doesn't add any credit ! That's why I've patched
    the inputs, so you can enter the "test mode" by pressing COIN1 during P.O.S.T.


Notes:
-----
- The sound board in all games is identical, labelled SOUND BOARD NO.59 MC 07.
  There are three pots, labelled MUSIC, VOICE and FRQ. MUSIC and VOICE control
  the volume, while FRQ changes (a unique feature of this hardware) the input
  clock to the MSM5232. This affects the music pitch ONLY--tempo is unaffected.
  Clock speed apparently ranges from 6.144MHz (OSC value) to about 150kHz.

- equites hardware: even if there are 0x200 bytes of sprite RAM, which would give
  a total of 128 possible sprites, since all games only write to a limited part of
  that RAM it looks like the hardware can only display 55 sprites. This is confirmed
  by the POST test which only shows those 55 sprites. The strange thing is that the
  sprites are not consecutive in RAM. The "good" parts are
  100000-10005f
  1000e0-1000ff
  1001a4-1001ff
  Possibly the remaining RAM is used by the sprite hardware as buffer? This doesn't
  really explain the weird layout in memory, however.
  Also, the priority order is counterintuitive. It seems that the above blocks are
  in increasing priority order, however in each block the higher priority sprites
  are at the lower addresses. This gives good priorities in gekisou (the car and
  helicopter shadows would be wrong otherwise).

- gekisou doesn't have dip-switches but battery backed RAM.
  By pressing service button (F2) at any time after POST operator can access to
  a simple config menu with no setting OSD indication about what they do but just a
  laconic number for each, basically mimicking an actual dip bank.
  END arrows indicates where to exit and return back to title screen, basically
  giving the operator to either read or rewrite all the settings.
  Defaults are all ON.

  Settings:                1   2   3   4   5   6   7   8
  COIN 1  1 Coin / 1 Play  ON  ON
          1 Coin / 2 Play  ON  OFF
          1 Coin / 3 Play  OFF OFF
          2 Coin / 1 Play  OFF ON
  COIN 2  1 Coin / 1 Play          ON  ON
          1 Coin / 2 Play          ON  OFF
          1 Coin / 3 Play          OFF OFF
          2 Coin / 1 Play          OFF ON
  DIFFICULTY  Easy                         ON
              Hard                         OFF
  CABINET     Cocktail                         ON
              Upright                          OFF


TODO:
----
- on startup, the CPU continuously writes 5555 to 100000 in a tight loop and expects
  it to change to exit the loop. The value should obviously be modified by the
  sprite hardware but it's difficult to guess what should happen. A kludge read
  handler is used as a work around.

- the second interrupt source for main CPU is unknown.

- gekisou has some unknown device mapped to 580000/5a0000. The bit read from bit 7
  of 180000 must match the last write, otherwise the game will report a BOARD ERROR
  during boot.

- gekisou: there's a white line crossing the explosion sprite. This doesn't look
  like a bad ROM since the line is crossing several, not concecutive, explosion
  sprites, and no other sprites.

- gekisou: wrong sprite when hitting helicopter with missile?

- gekisou: there is a small glitch during the text intro at the beginning of player
  2 game in cocktail mode: a white line spills out from the text box as characters
  in the last line are written. This might well be a bug in the original.
  Update: background is actually misaligned one line, cfr. the blue buildings
  that are near the status bar (missing on the other side).

- Need to use different default volumes for various games, especially gekisou
  where the car noise is really unpleasant with the default settings.

- analog drums/cymbals missing.

- bassline imperfect. This is just the square wave output of the 5232 at the moment.
  It should go through analog stages.

- implement low-pass filters on the DAC output

- the purpose of the sound PROM is unclear. From the schematics, it seems it
  should influence the MSM5232 clock. However, even removing it from the board
  doesn't seem to affect the sound.

- dump the MCU ROM for kouyakyu, gekisou, though they currently work fine with
  the ROM from another chip.

* Special Thanks to:

  Jarek Burczynski for a superb MSM5232 emulation
  The Equites WIP webmasters for the vital screenshots and sound clips
  Yasuhiro Ogawa for the correct Equites ROM information


*******************************************************************************

Bull Fighter
PCB layout
2005-03-21
f205v

|----------------------|
| Fully boxed = socket |
|----------------------|


| separation = solder


-----------------------------------------------------------------------|
|ALPHA DENSHI CO, LTD. SOUND BOARD NO.59MC07                           |
|                                                                      |
|          |--------------------|                                      |
|   P | \/ |    JAPAN 84250C    | SN74LS232J | SN74LS74AN | DISSIPATOR |
|     | /\ |      M5l8085AP     | 8131BJ     | 8314A      |            |
|          |--------------------|                                      |
|                                                                      |
|   N                           | SN74LS74A                            |
|                               | 8122AG                               |
|                                                                      |
|     |------------|                                                   |
|   M | HV1VR OKI  | SN74LS138J | SN74LS74AJ | LM324N     | LM324N     |
|     | 2764-45    | 8044AG     | F8048      | J423A2     | 98509      |- (+12V)
|     |------------|                                                   |
|                                                                      |- (+5V)
|     |------------|                                                   |
|   L | HV2VR OKI  | T74LS14B1  | SN74LS08N                            |- (OUT)
|     | 2764-45    | 88442      | K8208                                |
|     |------------|                                                   |- (
|                                                                      |  (GND)
|     |------------|                                                   |- (
|   K | HV3VR OKI  | SN74LS08J  | SN74LS04N  | LM324N                  |
|     | 2764-45    | K8208      | I8313      | 98509                   |
|     |------------|                                                   |
|                                                                      |
|   J              | SN74LS74A  | SN74LS232J |            | LM324N     |
|                  | 8122AG     | 8131BG     |            | 436A       |
|                                                                      |
|     |------------|            |------------|                         |
|   H | HV4VR OKI  | SN74LS393N | TBP18S030N |            | LM324N     |
|     | 2764-45    | K8208      | J419X      |            | 436A       |
|     |------------|            |------------|                         |
|                                                                      |
|   F              | SN74LS138J | HCF40174BE |            | CD4066BCN  |
|                  | 8044AG     | MSM40174   |            | MM5666BN   |
|                                                                      |
|   E              | M74LS123P  | SN74LS138J | CD4066BCN  | CD4066BCN  |
|                  | 84A6C1     | 8044AG     | MM5666BN   | MM5666BN   |
|                                                           MUSIC      |
|                  |-------------------------|                         |
|   D | SN74LS373N |       JAPAN 841903      |            | CD4066BCN  |
|     | 2764-45    |         M5L8155P        |            | MM5666BN   |
|                  |-------------------------|              VOICE      |
|                                                                      |
|C                 |-------------------------|                         |
|O  C | 74LS173 PC |        MSM5232RS        |                         |
|N    | 8247       |        OKI 4342         |                         |
|N                 |-------------------------|                         |
|                                                                      |
|T  B                                                                  |
|O                                                                     |
|                  |-------------------------|                         |
|M  A | 74LS173 PC |     SOUND AY-3-8910     | CD4066BCN  | CD4066BCN  |
|A    | 8247       |        8046 CDA         | MM5666BN   | MM5666BN   |
|I                 |-------------------------|                         |
|N                                                                     |
|     | 1          | 2           | 3         | 4          | 5          |
-----------------------------------------------------------------------|


-----------------------------------------------------------------------------------|
|ALPHA DENSHI CO, LTD. MAIN CONNECTOR BOARD                                        |
|                                                                                  |
|                                                                                  |
|      | K     | J     | H     | F     | E     | D     | C     | B     | A         |
|                                                                              |   |
J    1 | LS08  | LS74  | LS109 | LS27  | LS32  |       | LS153 | LS153 | 24S10 |L  |
2                                                                              |S  |
     2 | LS74  | LS04  | LS21  | LS00  | LS174 | LS153 | LS153 | LS273 | 24S10 |2  |
T                                                                              |7  |
O                                      |-------|                               |3  |
     3 | LS32  | LS161 | LS161 | LS157 |H      | LS194 | LS194 | LS04  | 24S10 |   |
L                                      |TMS2732|                                   |
O                                      |-------|                                   |
W                                                              |---------------|   |
E    4 | LS00  | LS86  | LS86  | LS08          | LS157 | LS157 | ALPHA-8303    |   |
R                                                              | 44801B42-3M 3 |   |
                                                               |---------------|   |
B                                      |-------|                                   |
O    5 | LS393 | LS86  | LS86  | OSCLT |M58725P| LS157 | LS157                     |
A                                      |-------|                                   |
R                                                                      |-------|   |
D    6 | LS04  | LS161 | LS161         | LS273 | LS157 | LS157 | LS08  |M58725P|   |-C
|                                                                      |-------|   |-O
|                                              |-----------|-----------|           |-N
|    7 | LS74  | LS367 | LS74  | LS368 | LS373 |  M58725P  |  M58725P  | LS245     |-N
|                                              |-----------|-----------|           |-E
|                                                                                  |-C
|    8 | LS32  | LS74  | LS32  | LS138 | LS245                         | LS245     |-T
J                                              |-----------|-----------|           |-O
1    9 | LS08  | LS00  | LS74  | LS138         |   empty   |   empty   | LS368     |-R
                                               |-----------|-----------|           |-
T                                              |-----------|-----------|           |-N
O   10 | LS08  | LS32  | LS74  | LS138 | LS139 |HP3VR      |HP7VR      | LS368     |-O
                                               |M5L2764K   |HN482764G  |           |-N
L        |-                                    |-----------|-----------|           |-J
O   11   |C \  | LS02  | LS00  | LS138 | LS259                         | LS368     |-A
W        |O  \                                 |-----------|-----------|           |-M
E        |N S |                                |HP2VR      |HP6VR      | LS368     |-M
R   12   |N O |                                |HN482764G  |M5L2764K   |           |-A
         |  U |                                |-----------|-----------|           |
B        |F N |        |-----------------------|-----------|-----------|           |
O   13   |R D || LS74  |       HD68000-8       |HP1VR      |HP5VR      |           |
A        |O  /         |         4D3-R         |M5L2764K   |M5L2764K   |           |
R        |M /          |-----------------------|-----------|-----------|           |
D        |-                                                                        |
|   14         | LS05  | LS193 | LS214 | LS08                  | LS139 | DIP 6x    |
|                                                                                  |
|----------------------------------------------------------------------------------|


-------------------------------------------------------------------------------------|
|ALPHA DENSHI CO, LTD. LOWER BOARD                                                   |
|                                                                                    |
|                                                                                    |
J      | K     | J     | H        | F        | E     | D     | C     | B     | A     |
2                                                                                    |
     1 | LS245 | LS139 |----------|----------| LS174 | LS153 | 4416  | LS04  | LS194 |
T                      | HM6116-3 | HS4VR    |                                       |
O                      | M58725P  | M5L2764K |                                       |
     2         | LS245 |          |          | LS174 | LS153 | 4416  | LS373 | LS194 |
M                      |----------|----------|                                       |
A                      | HM6116-3 | HS3VR    |                                       |
I    3 | LS08  | LS04  | M58725P  | M5L2764K | LS283 | LS153 | 4416  | LS157 | LS194 |
N                      |          |          |                                       |
                       |----------|----------|                                       |
B    4 | LS86  | LS86  | HS6VR    | HS2VR    | LS283 | LS153 | 4416  | LS373 | LS194 |
O                      | M5L2764K | HN482764G|                                       |
A                      |          |          |-------|-------|                       |
R    5 | LS157 | LS157 |----------|----------| LS193 | LS193 | LS08  | LS32  | LS32  |
D                      | HS5VR    | HS1VR    |-------|-------|                       |
|                      | M5L2764K | M5L2764K |                       |-------|       |
|    6 | LS08  | LS86  |          |          | LS157 | LS157 | LS153 | 24S10 | LS257 |
|                      |----------|----------|                       |-------|       |
|                      | HP5VR    | HB1VR    |                       |-------|       |
|    7 | LS157 | LS157 | M5L2764K | M5L2764K | LS157 | LS273 | LS153 | 24S10 | LS257 |
|                      |          |          |                       |-------|       |
|                      |----------|----------|                                       |
|    8 | LS669 | LS669 | HP6VR    | HB2VR    | LS157 | LS273 | LS153 | LS153 | LS32  |
J                      | M5L2764K | M5L2764K |                                       |
1                      |          |          |                       |-------|       |
     9 | LS669 | LS669 |----------|----------| LS157 | LS273 | LS153 | 24S10 | LS257 |
T                      | HM6116-3 | HB3VR    |                       |-------|       |
O                      | M58725P  | M5L2764K |                       |-------|       |
    10 | LS273 | LS273 |          |          | LS194 | LS04  | LS153 | 24S10 | LS257 |
M                      |----------|----------|                       |-------|       |
A                      | HM6116-3 | HB4VR    |                                       |
I   11 | LS245 | LS245 | M58725P  | M5L2764K | LS157 | LS194 | LS86  | LS08  | LS02  |
N                      |          |          |                                       |
                       |----------|----------|                                       |
B   12 | LS373 | LS373                       | LS194 | LS157 | LS08  | LS74  | LS175 |
O                                                                                    |
A                                                                                    |
R   13 | LS373 | LS273 | LS174    | LS139    | LS08  | LS32  | LS74  | LS10  | LS00  |
D                                                                                    |
|------------------------------------------------------------------------------------|


*******************************************************************************/

#include "emu.h"

#include "ad_sound.h"
#include "alpha8201.h"

#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

namespace {

class equites_state : public driver_device
{
public:
	equites_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_alpha_8201(*this, "alpha_8201"),
		m_mainlatch(*this, "mainlatch"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram", 0x800, ENDIANNESS_BIG),
		m_spriteram(*this, "spriteram")
	{ }

	void equites(machine_config &config);
	void bngotime(machine_config &config);
	void init_equites();

protected:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<alpha_8201_device> m_alpha_8201;
	required_device<ls259_device> m_mainlatch;

	// memory pointers
	required_shared_ptr<uint16_t> m_bg_videoram;
	memory_share_creator<uint8_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	virtual void machine_start() override;
	virtual void video_start() override;

	uint16_t spriteram_kludge_r();
	uint8_t fg_videoram_r(offs_t offset);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bgcolor_w(offs_t offset, uint8_t data);
	void scrollreg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(fg_info);
	TILE_GET_INFO_MEMBER(bg_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);
	void draw_sprites_block(bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void unpack_block(const char *region, int offset, int size);
	void unpack_region(const char *region);

	void bngotime_map(address_map &map);
	void equites_map(address_map &map);
	void common_map(address_map &map);

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_bgcolor = 0U;
};


class gekisou_state : public equites_state
{
public:
	gekisou_state(const machine_config &mconfig, device_type type, const char *tag) :
		equites_state(mconfig, type, tag)
	{ }

	int unknown_bit_r();
	void gekisou(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void gekisou_map(address_map &map);
	void unknown_bit_w(offs_t offset, uint16_t data);

	int m_unknown_bit = 0;
};


/******************************************************************************/
// Palette handling

void equites_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
		palette.set_indirect_color(i, rgb_t(pal4bit(color_prom[i]), pal4bit(color_prom[i + 0x100]), pal4bit(color_prom[i + 0x200])));

	// point to the CLUT
	color_prom += 0x380;

	for (int i = 0; i < 256; i++)
		palette.set_pen_indirect(i, i);

	for (int i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i + 0x100, color_prom[i]);
}



/******************************************************************************/
// Callbacks for the tilemap code

TILE_GET_INFO_MEMBER(equites_state::fg_info)
{
	int tile = m_fg_videoram[2 * tile_index];
	int color = m_fg_videoram[2 * tile_index + 1] & 0x1f;

	tileinfo.set(0, tile, color, 0);
	if (color & 0x10)
		tileinfo.flags |= TILE_FORCE_LAYER0;
}

TILE_GET_INFO_MEMBER(equites_state::bg_info)
{
	int data = m_bg_videoram[tile_index];
	int tile = data & 0x1ff;
	int color = (data & 0xf000) >> 12;
	int fxy = (data & 0x0600) >> 9;

	tileinfo.set(1, tile, color, TILE_FLIPXY(fxy));
}



/******************************************************************************/
// Video system start

void equites_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(equites_state::fg_info)), TILEMAP_SCAN_COLS,  8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(equites_state::bg_info)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(0, -10);
}



/******************************************************************************/
// Video update

void equites_state::draw_sprites_block(bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end)
{
	for (int offs = end - 2; offs >= start; offs -= 2)
	{
		int attr = m_spriteram[offs + 1];
		if (!(attr & 0x800))    // disable or x MSB?
		{
			int tile = attr & 0x1ff;
			int fx = ~attr & 0x400;
			int fy = ~attr & 0x200;
			int color = (~attr & 0xf000) >> 12;
			int sx = (m_spriteram[offs] & 0xff00) >> 8;
			int sy = (m_spriteram[offs] & 0x00ff);
			int transmask = m_palette->transpen_mask(*m_gfxdecode->gfx(2), color, 0);

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				fx = !fx;
				fy = !fy;
			}

			// align
			sx -= 4;

			// sprites are 16x14 centered in a 16x16 square, so skip the first line
			sy += 1;

			m_gfxdecode->gfx(2)->transmask(bitmap,cliprect,
					tile,
					color,
					fx, fy,
					sx, sy, transmask);
		}
	}
}

void equites_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// note that we draw the sprites in three blocks; in each blocks, sprites at
	// a lower address have priority. This gives good priorities in gekisou.
	draw_sprites_block(bitmap, cliprect, 0x000/2, 0x060/2);
	draw_sprites_block(bitmap, cliprect, 0x0e0/2, 0x100/2);
	draw_sprites_block(bitmap, cliprect, 0x1a4/2, 0x200/2);
}


uint32_t equites_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



/******************************************************************************/
// Interrupt Handlers

TIMER_DEVICE_CALLBACK_MEMBER(equites_state::scanline_cb)
{
	int scanline = param;

    // all games but bullfgtr have both valid
    // bullfgtr definitely expects to vblank from 2, reversing will make it to run at half speed.
	if(scanline == 232) // vblank-in irq
		m_maincpu->set_input_line(2, HOLD_LINE);

	if(scanline == 24) // vblank-out irq or sprite DMA done
		m_maincpu->set_input_line(1, HOLD_LINE);
}



/******************************************************************************/
// CPU Handlers

uint8_t equites_state::fg_videoram_r(offs_t offset)
{
	// 8-bit
	return m_fg_videoram[offset];
}

void equites_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void equites_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_bg_videoram + offset);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void equites_state::bgcolor_w(offs_t offset, uint8_t data)
{
	m_bgcolor = data;
}

void equites_state::scrollreg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_bg_tilemap->set_scrolly(0, data & 0xff);

	if (ACCESSING_BITS_8_15)
		m_bg_tilemap->set_scrollx(0, data >> 8);
}

uint16_t equites_state::spriteram_kludge_r()
{
	if (m_spriteram[0] == 0x5555)
		return 0;
	else
		return m_spriteram[0];
}


int gekisou_state::unknown_bit_r()
{
	return m_unknown_bit;
}

void gekisou_state::unknown_bit_w(offs_t offset, uint16_t data)
{
	// data bit is A17 (offset)
	m_unknown_bit = (offset == 0) ? 0 : 1;
}


/******************************************************************************/
// CPU Memory Maps

void equites_state::common_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x00ffff).rom(); // ROM area is written several times (dev system?)
	map(0x080000, 0x080fff).rw(FUNC(equites_state::fg_videoram_r), FUNC(equites_state::fg_videoram_w)).umask16(0x00ff);
	map(0x0c0000, 0x0c01ff).ram().w(FUNC(equites_state::bg_videoram_w)).share("bg_videoram");
	map(0x0c0200, 0x0c0fff).ram();
	map(0x100000, 0x1001ff).ram().share("spriteram");
	map(0x100000, 0x100001).r(FUNC(equites_state::spriteram_kludge_r));
	map(0x140000, 0x1407ff).rw(m_alpha_8201, FUNC(alpha_8201_device::ext_ram_r), FUNC(alpha_8201_device::ext_ram_w)).umask16(0x00ff);
	map(0x180000, 0x180001).portr("IN1");
	map(0x180000, 0x180000).select(0x03c000).lw8(NAME([this] (offs_t offset, u8 data) { m_mainlatch->write_a3(offset >> 14); }));
	map(0x1c0000, 0x1c0001).portr("IN0").w(FUNC(equites_state::scrollreg_w));
	map(0x380000, 0x380000).w(FUNC(equites_state::bgcolor_w));
	map(0x780000, 0x780001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
}

void equites_state::equites_map(address_map &map)
{
	common_map(map);
	map(0x040000, 0x040fff).ram();
	map(0x180001, 0x180001).w("sound_board", FUNC(ad_59mc07_device::sound_command_w));
}

void gekisou_state::gekisou_map(address_map &map)
{
	common_map(map);
	map(0x040000, 0x040fff).ram().share("nvram"); // mainram is battery-backed
	map(0x180001, 0x180001).w("sound_board", FUNC(ad_59mc07_device::sound_command_w));
	map(0x580000, 0x580001).select(0x020000).w(FUNC(gekisou_state::unknown_bit_w));
}

void equites_state::bngotime_map(address_map &map)
{
	common_map(map);
	map(0x040000, 0x040fff).ram();
	map(0x180001, 0x180001).w("sound_board", FUNC(ad_60mc01_device::sound_command_w));
}



/******************************************************************************/
// Common Port Map

#define EQUITES_PLAYER_INPUT_LSB( button1, button2, button3, start ) \
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY \
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY \
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY \
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY \
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, button1 ) \
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, button2 ) \
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, button3 ) \
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, start )

#define EQUITES_PLAYER_INPUT_MSB( button1, button2, button3, start ) \
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, button1 ) PORT_COCKTAIL \
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, button2 ) PORT_COCKTAIL \
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, button3 ) PORT_COCKTAIL \
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, start )

#define FRQ_ADJUSTER_TAG    "FRQ"

/******************************************************************************/
// Equites Port Map

static INPUT_PORTS_START( equites )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:!6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Buttons" ) PORT_DIPLOCATION("SW:!5")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0800, "3" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR ( Lives ) ) PORT_DIPLOCATION("SW:!4")
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!2,!1")
	PORT_DIPSETTING(      0xc000, "A 2C/1C B 3C/1C" )
	PORT_DIPSETTING(      0x0000, "A 1C/1C B 2C/1C" )
	PORT_DIPSETTING(      0x8000, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(      0x4000, "A 1C/3C B 1C/6C" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(25, "MSM5232 Clock") // approximate factory setting
INPUT_PORTS_END

/******************************************************************************/
// Gekisou Port Map

static INPUT_PORTS_START( gekisou )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_SERVICE ) // settings
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(gekisou_state, unknown_bit_r)

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(24, "MSM5232 Clock") // approximate factory setting
INPUT_PORTS_END

/******************************************************************************/
// Bull Fighter Port Map

static INPUT_PORTS_START( bullfgtr )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SW:!6,!5")
	PORT_DIPSETTING(      0x0c00, "3:00" )
	PORT_DIPSETTING(      0x0800, "2:00" )
	PORT_DIPSETTING(      0x0000, "1:30" )
	PORT_DIPSETTING(      0x0400, "1:00" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x9000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!4,!1")
	PORT_DIPSETTING(      0x9000, "1C/1C (3C per player)" )
	PORT_DIPSETTING(      0x0000, "1C/1C (1C per player)" )
	PORT_DIPSETTING(      0x8000, "A 1C/1C B 1C/4C" )
	PORT_DIPSETTING(      0x1000, "A 1C/2C B 1C/3C" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(33, "MSM5232 Clock") // approximate factory setting
INPUT_PORTS_END

/******************************************************************************/
// Koukouyakyuh Port Map

static INPUT_PORTS_START( kouyakyu )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) // only used to access testmode!
	PORT_DIPNAME( 0x0c00, 0x0000, "Game Points" ) PORT_DIPLOCATION("SW:!6,!5")
	PORT_DIPSETTING(      0x0800, "3000" )
	PORT_DIPSETTING(      0x0400, "4000" )
	PORT_DIPSETTING(      0x0000, "5000" )
	PORT_DIPSETTING(      0x0c00, "7000" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x9000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!4,!1")
	PORT_DIPSETTING(      0x9000, "1C/1C (2C per player)" )
	PORT_DIPSETTING(      0x0000, "1C/1C (1C per player)" )
	PORT_DIPSETTING(      0x8000, "1C/1C (1C for 2 players)" )
	PORT_DIPSETTING(      0x1000, "1C/3C (1C per player)" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(33, "MSM5232 Clock") // approximate factory setting
INPUT_PORTS_END

/******************************************************************************/
// Bingo Time  Port Map

static INPUT_PORTS_START( bngotime ) // TODO: possibly still missing something? Couldn't find any use for the unknown inputs
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) // tilt up, only has effect when ball's in play
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) // tilt right, only has effect when ball's in play
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) // tilt left, only has effect when ball's in play
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // spring launcher and tilt up (doubled?)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 ) // starts game after betting, also changes background before launching first ball
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // buys extra ball after game over
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x8000, IP_ACTIVE_HIGH ) // settings // allows to set coinage, odds, 'rank', etc.
INPUT_PORTS_END



/******************************************************************************/
// Graphics Layouts

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8+3,-1), STEP4(0*8+3,-1) },
	{ STEP8(0*8,8) },
	16*8
};

static const gfx_layout tilelayout_3bpp =
{
	16, 16,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(16*8+3,-1), STEP4(32*8+3,-1), STEP4(48*8+3,-1), STEP4(0*8+3,-1) },
	{ STEP16(0,8) },
	64*8
};

static const gfx_layout spritelayout_16x14 =
{
	16, 14, // 16x14, very unusual
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(128*0+3,-1), STEP4(128*1+3,-1), STEP4(128*2+3,-1), STEP4(128*3+3,-1) },
	{ 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8 },
	64*8
};


static GFXDECODE_START( gfx_equites )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0x000, 0x80/4 ) // chars
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_3bpp,    0x080, 0x80/8 ) // tiles
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_16x14, 0x100, 0x80/8 ) // sprites
GFXDECODE_END



/******************************************************************************/

void equites_state::machine_start()
{
	save_item(NAME(m_bgcolor));
}

void gekisou_state::machine_start()
{
	equites_state::machine_start();

	save_item(NAME(m_unknown_bit));
}

void equites_state::equites(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL/4); // 68000P8 running at 3mhz! verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &equites_state::equites_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(equites_state::scanline_cb), "screen", 0, 1);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<1>().set(FUNC(equites_state::flip_screen_set));
	m_mainlatch->q_out_cb<2>().set(m_alpha_8201, FUNC(alpha_8201_device::mcu_start_w));
	m_mainlatch->q_out_cb<3>().set(m_alpha_8201, FUNC(alpha_8201_device::bus_dir_w)).invert();

	ALPHA_8201(config, m_alpha_8201, 4000000/8); // 8303 or 8304 (same device!)
	config.set_perfect_quantum("alpha_8201:mcu");

	AD_59MC07(config, "sound_board");

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 3*8, 29*8-1);
	m_screen->set_screen_update(FUNC(equites_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_equites);
	PALETTE(config, m_palette, FUNC(equites_state::palette), 0x180, 0x100);
}

void gekisou_state::gekisou(machine_config &config)
{
	equites(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gekisou_state::gekisou_map);

	// gekisou has battery-backed RAM to store settings
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void equites_state::bngotime(machine_config &config)
{
	equites(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &equites_state::bngotime_map);

	AD_60MC01(config.replace(), "sound_board");
}



/******************************************************************************/
// Equites ROM Map

/*
Equites by ALPHA DENSHI CO. (1984)

Note: CPU - Main PCB
      SND - Sound PCB    NO.59 MC07
      VID - Video PCB

Main processor   - 68000 2.988MHz

Protection processor  - ALPHA-8303 custom

Sound processor  - 8085 3.073MHz
                 - TMP8155 RIOTs    (RAM & I/O Timers)
                 - MSM5232R3
                 - AY-3-8910
*/
ROM_START( equites )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "ep1",         0x00001, 0x2000, CRC(6a4fe5f7) SHA1(5ff1594a2cee28cc7d59448eb57473088ac6f14b) )
	ROM_LOAD16_BYTE( "ep5",         0x00000, 0x2000, CRC(00faa3eb) SHA1(6b31d041ad4ca81eda36487f659997cc4030f23c) )
	ROM_LOAD16_BYTE( "epr-ep2.12d", 0x04001, 0x2000, CRC(0c1bc2e7) SHA1(4c3510dfeee2fb2f295a32e2fe2021c4c7f08e8a) )
	ROM_LOAD16_BYTE( "epr-ep6.12b", 0x04000, 0x2000, CRC(bbed3dcc) SHA1(46ef2c60ccfa76a187b19dc0b7e6c594050b183f) )
	ROM_LOAD16_BYTE( "epr-ep3.10d", 0x08001, 0x2000, CRC(5f2d059a) SHA1(03fe904a445cce89462788fecfd61ac53f4dd17f) )
	ROM_LOAD16_BYTE( "epr-ep7.10b", 0x08000, 0x2000, CRC(a8f6b8aa) SHA1(ee4edb54c147a95944482e998616b025642a268a) )
	ROM_LOAD16_BYTE( "ep4",         0x0c001, 0x2000, CRC(b636e086) SHA1(5fc23a86b6051ecf6ff3f95f810f0eb471a203b0) )
	ROM_LOAD16_BYTE( "ep8",         0x0c000, 0x2000, CRC(d7ee48b0) SHA1(d0398704d8e89f2b0a9ed05e18f7c644d1e047c0) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "ev1.1m", 0x00000, 0x2000, CRC(43faaf2e) SHA1(c9aaf298d673eb70399366776474f1b242549eb4) )
	ROM_LOAD( "ev2.1l", 0x02000, 0x2000, CRC(09e6702d) SHA1(896771f73a486e5035909eeed9ef48103d81d4ae) )
	ROM_LOAD( "ev3.1k", 0x04000, 0x2000, CRC(10ff140b) SHA1(7c28f988a9c8b2a702d007096199e67b447a183c) )
	ROM_LOAD( "ev4.1h", 0x06000, 0x2000, CRC(b7917264) SHA1(e58345fda088b171fd348959de15082f3cb42514) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "ep9",  0x00000, 0x1000, CRC(0325be11) SHA1(d95667b439e3d97b08efeaf08022348546a4f385) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "eb5.7h",  0x00000, 0x2000, CRC(cbef7da5) SHA1(c5fcd2341ce5b039a15116fbd85796bb5ddc4701) )
	// empty space to unpack previous ROM
	ROM_LOAD( "eb6.8h",  0x04000, 0x2000, CRC(1e5e5475) SHA1(80ebe9326c628685faafb259f956a98ac435c809) )
	// empty space to unpack previous ROM
	ROM_LOAD( "eb1.7f",  0x08000, 0x2000, CRC(9a236583) SHA1(fcc4da2efe904f0178bd83fdee25d4752b9cc5ce) )
	ROM_LOAD( "eb2.8f",  0x0a000, 0x2000, CRC(f0fb6355) SHA1(3c4c009f80e648d02767b29bb8d18f4de7b26d4e) )
	ROM_LOAD( "eb3.10f", 0x0c000, 0x2000, CRC(dbd0044b) SHA1(5611517bb0f54bfb0585eeca8af21fbfc2f65b2c) )
	ROM_LOAD( "eb4.11f", 0x0e000, 0x2000, CRC(f8f8e600) SHA1(c7c97e4dc1f7a73694c98b2b1a3d7fa9f3317a2a) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "es5.5h",  0x00000, 0x2000, CRC(d5b82e6a) SHA1(956a1413426e53f8a735260e660805b04016ca8d) )
	// empty space to unpack previous ROM
	ROM_LOAD( "es6.4h",  0x04000, 0x2000, CRC(cb4f5da9) SHA1(5af8f6aca0a3bb6417430e0179ec97c33d4014e3) )
	// empty space to unpack previous ROM
	ROM_LOAD( "es1.5f",  0x08000, 0x2000, CRC(cf81a2cd) SHA1(a1b45451cafeaceabe3dfe24eb073098a33ab22b) )
	ROM_LOAD( "es2.4f",  0x0a000, 0x2000, CRC(ae3111d8) SHA1(d63633b531339fa04af757f42e956b8eb1debc4e) )
	ROM_LOAD( "es3.2f",  0x0c000, 0x2000, CRC(3d44f815) SHA1(1835aef280a6915acbf7cad771d65bf1074f0f98) )
	ROM_LOAD( "es4.1f",  0x0e000, 0x2000, CRC(16e6d18a) SHA1(44f9045ad034808070cd6497a3b94c3d8cc93262) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "bprom.3a",  0x0000, 0x100, CRC(2fcdf217) SHA1(4acf67d37e844c2773028ecffe72a66754ed5bca) ) // R
	ROM_LOAD( "bprom.1a",  0x0100, 0x100, CRC(d7e6cd1f) SHA1(ce330e43ba8a97ab79040c053a25e46e8fe60bdb) ) // G
	ROM_LOAD( "bprom.2a",  0x0200, 0x100, CRC(e3d106e8) SHA1(6b153eb8140d36b4d194e26106a5ba5bffd1a851) ) // B
	ROM_LOAD( "bprom.6b",  0x0300, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) ) // CLUT(same PROM x 4)
	ROM_LOAD( "bprom.7b",  0x0400, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )
	ROM_LOAD( "bprom.9b",  0x0500, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )
	ROM_LOAD( "bprom.10b", 0x0600, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "bprom.3h",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/*
Equites
(c)1984 SEGA/ALPHA

CPU   : 68000 Z-80x2
SOUND : AY-3-8910 MSM5232RS
OSC.  : 12.000MHz 14.31818MHz ?MHz
*/
ROM_START( equitess )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "epr-ep1.13d", 0x00001, 0x2000, CRC(c6edf1cd) SHA1(21dba62e692f4fdc79155ce169a48ae827bd5994) )
	ROM_LOAD16_BYTE( "epr-ep5.13b", 0x00000, 0x2000, CRC(c11f0759) SHA1(5caf2b6b777b2fdabc26ea232225be2d789e87f3) )
	ROM_LOAD16_BYTE( "epr-ep2.12d", 0x04001, 0x2000, CRC(0c1bc2e7) SHA1(4c3510dfeee2fb2f295a32e2fe2021c4c7f08e8a) )
	ROM_LOAD16_BYTE( "epr-ep6.12b", 0x04000, 0x2000, CRC(bbed3dcc) SHA1(46ef2c60ccfa76a187b19dc0b7e6c594050b183f) )
	ROM_LOAD16_BYTE( "epr-ep3.10d", 0x08001, 0x2000, CRC(5f2d059a) SHA1(03fe904a445cce89462788fecfd61ac53f4dd17f) )
	ROM_LOAD16_BYTE( "epr-ep7.10b", 0x08000, 0x2000, CRC(a8f6b8aa) SHA1(ee4edb54c147a95944482e998616b025642a268a) )
	ROM_LOAD16_BYTE( "epr-ep4.9d",  0x0c001, 0x2000, CRC(956a06bd) SHA1(a716f9aaf0c32c522968f4ff13de904d6e8c7f98) )
	ROM_LOAD16_BYTE( "epr-ep8.9b",  0x0c000, 0x2000, CRC(4c78d60d) SHA1(207a82779e2fe3e9082f4fa09b87c713a51167e6) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "ev1.1m", 0x00000, 0x2000, CRC(43faaf2e) SHA1(c9aaf298d673eb70399366776474f1b242549eb4) )
	ROM_LOAD( "ev2.1l", 0x02000, 0x2000, CRC(09e6702d) SHA1(896771f73a486e5035909eeed9ef48103d81d4ae) )
	ROM_LOAD( "ev3.1k", 0x04000, 0x2000, CRC(10ff140b) SHA1(7c28f988a9c8b2a702d007096199e67b447a183c) )
	ROM_LOAD( "ev4.1h", 0x06000, 0x2000, CRC(b7917264) SHA1(e58345fda088b171fd348959de15082f3cb42514) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "epr-ep0.3e",  0x00000, 0x1000, CRC(3f5a81c3) SHA1(8fd5bc621f483bfa46be7e40e6480b25243bdf70) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "eb5.7h",  0x00000, 0x2000, CRC(cbef7da5) SHA1(c5fcd2341ce5b039a15116fbd85796bb5ddc4701) )
	// empty space to unpack previous ROM
	ROM_LOAD( "eb6.8h",  0x04000, 0x2000, CRC(1e5e5475) SHA1(80ebe9326c628685faafb259f956a98ac435c809) )
	// empty space to unpack previous ROM
	ROM_LOAD( "eb1.7f",  0x08000, 0x2000, CRC(9a236583) SHA1(fcc4da2efe904f0178bd83fdee25d4752b9cc5ce) )
	ROM_LOAD( "eb2.8f",  0x0a000, 0x2000, CRC(f0fb6355) SHA1(3c4c009f80e648d02767b29bb8d18f4de7b26d4e) )
	ROM_LOAD( "eb3.10f", 0x0c000, 0x2000, CRC(dbd0044b) SHA1(5611517bb0f54bfb0585eeca8af21fbfc2f65b2c) )
	ROM_LOAD( "eb4.11f", 0x0e000, 0x2000, CRC(f8f8e600) SHA1(c7c97e4dc1f7a73694c98b2b1a3d7fa9f3317a2a) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "es5.5h",  0x00000, 0x2000, CRC(d5b82e6a) SHA1(956a1413426e53f8a735260e660805b04016ca8d) )
	// empty space to unpack previous ROM
	ROM_LOAD( "es6.4h",  0x04000, 0x2000, CRC(cb4f5da9) SHA1(5af8f6aca0a3bb6417430e0179ec97c33d4014e3) )
	// empty space to unpack previous ROM
	ROM_LOAD( "es1.5f",  0x08000, 0x2000, CRC(cf81a2cd) SHA1(a1b45451cafeaceabe3dfe24eb073098a33ab22b) )
	ROM_LOAD( "es2.4f",  0x0a000, 0x2000, CRC(ae3111d8) SHA1(d63633b531339fa04af757f42e956b8eb1debc4e) )
	ROM_LOAD( "es3.2f",  0x0c000, 0x2000, CRC(3d44f815) SHA1(1835aef280a6915acbf7cad771d65bf1074f0f98) )
	ROM_LOAD( "es4.1f",  0x0e000, 0x2000, CRC(16e6d18a) SHA1(44f9045ad034808070cd6497a3b94c3d8cc93262) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "bprom.3a",  0x0000, 0x100, CRC(2fcdf217) SHA1(4acf67d37e844c2773028ecffe72a66754ed5bca) ) // R
	ROM_LOAD( "bprom.1a",  0x0100, 0x100, CRC(d7e6cd1f) SHA1(ce330e43ba8a97ab79040c053a25e46e8fe60bdb) ) // G
	ROM_LOAD( "bprom.2a",  0x0200, 0x100, CRC(e3d106e8) SHA1(6b153eb8140d36b4d194e26106a5ba5bffd1a851) ) // B
	ROM_LOAD( "bprom.6b",  0x0300, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) ) // CLUT(same PROM x 4)
	ROM_LOAD( "bprom.7b",  0x0400, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )
	ROM_LOAD( "bprom.9b",  0x0500, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )
	ROM_LOAD( "bprom.10b", 0x0600, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "bprom.3h",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/******************************************************************************/
// Bull Fighter ROM Map

ROM_START( bullfgtr )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "hp1vr.bin",  0x00001, 0x2000, CRC(e5887586) SHA1(c06883f5c4a2e777b011199787bd4d52f48ceb41) )
	ROM_LOAD16_BYTE( "hp5vr.bin",  0x00000, 0x2000, CRC(b49fa09f) SHA1(ee947f7b4fa96f887f9b14e7503f98b4d117d1c8) )
	ROM_LOAD16_BYTE( "hp2vr.bin",  0x04001, 0x2000, CRC(845bdf28) SHA1(4eac9ca034aaa6a7db4061ad11587189fc843ca0) )
	ROM_LOAD16_BYTE( "hp6vr.bin",  0x04000, 0x2000, CRC(3dfadcf4) SHA1(724d45df0be7073bbe2767f3c0d050c8b45c9d27) )
	ROM_LOAD16_BYTE( "hp3vr.bin",  0x08001, 0x2000, CRC(d3a21f8a) SHA1(2b3135aaae798eeee5850e616ed6ad8987fbc01b) )
	ROM_LOAD16_BYTE( "hp7vr.bin",  0x08000, 0x2000, CRC(665cc015) SHA1(17fe18c8f22808a102f48bc4cbc8e4a1f6f9eaf1) )
	ROM_FILL(                      0x0c000, 0x4000, 0x00 )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "hv1vr.bin", 0x00000, 0x2000, CRC(2a8e6fcf) SHA1(866903408e05938a982ffef4c9b849203c6cc060) )
	ROM_LOAD( "hv2vr.bin", 0x02000, 0x2000, CRC(026e1533) SHA1(6271869a3faaafacfac35262746e87a83c158b93) )
	ROM_LOAD( "hv3vr.bin", 0x04000, 0x2000, CRC(51ee751c) SHA1(60bf848dfdfe313ab05df5a5c05819b0fa87ca50) )
	ROM_LOAD( "hv4vr.bin", 0x06000, 0x2000, CRC(62c7a25b) SHA1(237d3cbdfbf45b33c2f65d30faba151380866a93) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "h.bin", 0x000000, 0x1000, CRC(c6894c9a) SHA1(0d5a55cded4fd833211bdc733a78c6c8423897de) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "hb5vr.bin",  0x00000, 0x2000, CRC(6d05e9f2) SHA1(4b5c92b72bf73a08a2359fe889a327a696a45e8a) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hb6vr.bin",  0x04000, 0x2000, CRC(016340ae) SHA1(f980d39337c711a15520388967ca4503e7970e18) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hb1vr.bin",  0x08000, 0x2000, CRC(4352d069) SHA1(bac687f050837b023da00cb53bb524b2a76310d4) )
	ROM_LOAD( "hb2vr.bin",  0x0a000, 0x2000, CRC(24edfd7d) SHA1(be8a40d8d5ccff06f37c1ab67341f56e41a5ea88) )
	ROM_LOAD( "hb3vr.bin",  0x0c000, 0x2000, CRC(4947114e) SHA1(822dc3f14b71dc9e5b69078aefbed6b438aa0690) )
	ROM_LOAD( "hb4vr.bin",  0x0e000, 0x2000, CRC(fa296cb3) SHA1(2ba864766655cb3dd2999a6cdf96dcefd6818135) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "hs5vr.bin",  0x00000, 0x2000, CRC(48394389) SHA1(a5c6021b60226a775b2052909e8d21b5f79d9ec5) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hs6vr.bin",  0x04000, 0x2000, CRC(141409ec) SHA1(3cc542fa34fdbd71e392c7c22da5d5120642be86) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hs1vr.bin",  0x08000, 0x2000, CRC(7c69b473) SHA1(abc181b4e5b3f48c667a0bb4814c3818dfc6e9e2) )
	ROM_LOAD( "hs2vr.bin",  0x0a000, 0x2000, CRC(c3dc713f) SHA1(c2072cc71ea61e0c718c339bda1460d93343469e) )
	ROM_LOAD( "hs3vr.bin",  0x0c000, 0x2000, CRC(883f93fd) SHA1(a96df701f82e62582522953830049d29bcb3d458) )
	ROM_LOAD( "hs4vr.bin",  0x0e000, 0x2000, CRC(578ace7b) SHA1(933e85d49db7b27fd85e4713f0984612bc29e134) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "24s10n.a3",  0x0000, 0x100, CRC(e8a9d159) SHA1(d015149a39adcb5fc6d12d9afe7820ecef480039) ) // R
	ROM_LOAD( "24s10n.a1",  0x0100, 0x100, CRC(3956af86) SHA1(ccbb69535ece5e228622907d17c959b195b97a0a) ) // G
	ROM_LOAD( "24s10n.a2",  0x0200, 0x100, CRC(f50f8ec5) SHA1(ec2d934618e25e3471153f9fe7b34f978b113a47) ) // B
	ROM_LOAD( "24s10n.b6",  0x0300, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) ) // CLUT(same PROM x 4)
	ROM_LOAD( "24s10n.b7",  0x0400, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )
	ROM_LOAD( "24s10n.b9",  0x0500, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )
	ROM_LOAD( "24s10n.b10", 0x0600, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "18s030.h3",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/*
Bull Fighter

Upper board (Sound)
-------------------
All roms 2764
1 prom TBP18S030
CPU 8085, AY-3-8910, 8155, 5232

Midle board
-----------
Chips named M_xxx

e3 = 2732
All other roms 2764
A1- A3 TBP24S030 PROM

CPU 68000
Special chip:
Alpha 8303 44801B42

Lower board
-----------
Chips named L_xxx
All roms 2764
All Proms TBP24S10
*/
ROM_START( bullfgtrs )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "m_d13.bin",  0x00001, 0x2000, CRC(7c35dd4b) SHA1(6bd604ee32c0c5db17f90e24aa254ec7072d27dd) )
	ROM_LOAD16_BYTE( "m_b13.bin",  0x00000, 0x2000, CRC(c4adddce) SHA1(48b6ddbad52a3941d3e651642b26d9adf70f71f5) )
	ROM_LOAD16_BYTE( "m_d12.bin",  0x04001, 0x2000, CRC(5d51be2b) SHA1(55d2718479cb71ceefefbaf40c14285e5603e526) )
	ROM_LOAD16_BYTE( "m_b12.bin",  0x04000, 0x2000, CRC(d98390ef) SHA1(17006503325627055c8b22052d7ed94e474f4ef7) )
	ROM_LOAD16_BYTE( "m_d10.bin",  0x08001, 0x2000, CRC(21875752) SHA1(016db4125b1a4584ae277af427370780d96a17c5) )
	ROM_LOAD16_BYTE( "m_b10.bin",  0x08000, 0x2000, CRC(9d84f678) SHA1(32584d54788cb570bd5210992836f28ba9c87aac) )
	ROM_FILL(                      0x0c000, 0x4000, 0x00 )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "hv1vr.bin", 0x00000, 0x2000, CRC(2a8e6fcf) SHA1(866903408e05938a982ffef4c9b849203c6cc060) )
	ROM_LOAD( "hv2vr.bin", 0x02000, 0x2000, CRC(026e1533) SHA1(6271869a3faaafacfac35262746e87a83c158b93) )
	ROM_LOAD( "hv3vr.bin", 0x04000, 0x2000, CRC(51ee751c) SHA1(60bf848dfdfe313ab05df5a5c05819b0fa87ca50) )
	ROM_LOAD( "hv4vr.bin", 0x06000, 0x2000, CRC(62c7a25b) SHA1(237d3cbdfbf45b33c2f65d30faba151380866a93) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "h.bin", 0x000000, 0x1000, CRC(c6894c9a) SHA1(0d5a55cded4fd833211bdc733a78c6c8423897de) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "hb5vr.bin",  0x00000, 0x2000, CRC(6d05e9f2) SHA1(4b5c92b72bf73a08a2359fe889a327a696a45e8a) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hb6vr.bin",  0x04000, 0x2000, CRC(016340ae) SHA1(f980d39337c711a15520388967ca4503e7970e18) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hb1vr.bin",  0x08000, 0x2000, CRC(4352d069) SHA1(bac687f050837b023da00cb53bb524b2a76310d4) )
	ROM_LOAD( "hb2vr.bin",  0x0a000, 0x2000, CRC(24edfd7d) SHA1(be8a40d8d5ccff06f37c1ab67341f56e41a5ea88) )
	ROM_LOAD( "hb3vr.bin",  0x0c000, 0x2000, CRC(4947114e) SHA1(822dc3f14b71dc9e5b69078aefbed6b438aa0690) )
	ROM_LOAD( "hb4vr.bin",  0x0e000, 0x2000, CRC(fa296cb3) SHA1(2ba864766655cb3dd2999a6cdf96dcefd6818135) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "hs5vr.bin",  0x00000, 0x2000, CRC(48394389) SHA1(a5c6021b60226a775b2052909e8d21b5f79d9ec5) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hs6vr.bin",  0x04000, 0x2000, CRC(141409ec) SHA1(3cc542fa34fdbd71e392c7c22da5d5120642be86) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hs1vr.bin",  0x08000, 0x2000, CRC(7c69b473) SHA1(abc181b4e5b3f48c667a0bb4814c3818dfc6e9e2) )
	ROM_LOAD( "hs2vr.bin",  0x0a000, 0x2000, CRC(c3dc713f) SHA1(c2072cc71ea61e0c718c339bda1460d93343469e) )
	ROM_LOAD( "hs3vr.bin",  0x0c000, 0x2000, CRC(883f93fd) SHA1(a96df701f82e62582522953830049d29bcb3d458) )
	ROM_LOAD( "hs4vr.bin",  0x0e000, 0x2000, CRC(578ace7b) SHA1(933e85d49db7b27fd85e4713f0984612bc29e134) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "24s10n.a3",  0x0000, 0x100, CRC(e8a9d159) SHA1(d015149a39adcb5fc6d12d9afe7820ecef480039) ) // R
	ROM_LOAD( "24s10n.a1",  0x0100, 0x100, CRC(3956af86) SHA1(ccbb69535ece5e228622907d17c959b195b97a0a) ) // G
	ROM_LOAD( "24s10n.a2",  0x0200, 0x100, CRC(f50f8ec5) SHA1(ec2d934618e25e3471153f9fe7b34f978b113a47) ) // B
	ROM_LOAD( "24s10n.b6",  0x0300, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) ) // CLUT(same PROM x 4)
	ROM_LOAD( "24s10n.b7",  0x0400, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )
	ROM_LOAD( "24s10n.b9",  0x0500, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )
	ROM_LOAD( "24s10n.b10", 0x0600, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "18s030.h3",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/******************************************************************************/
// Koukouyakyuh ROM Map

/*
The Koukouyakyuh (JPN Ver.)

(c)1985 Alpha denshi

CPU   :MAIN  68000
       SOUND 8085
Sound :AY-3-8910 M3M5232RS
OSC   :12.000MHz 6.??MHz 14.31818MHz
*/
ROM_START( kouyakyu )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 68000 ROMs
	ROM_LOAD16_BYTE( "epr-6704.bin", 0x00001, 0x2000, CRC(c7ac2292) SHA1(614bfb0949620d4c260768f14a116b076dd38438) )
	ROM_LOAD16_BYTE( "epr-6707.bin", 0x00000, 0x2000, CRC(9cb2962e) SHA1(bd1bcbc53a3346e22789f24a35ab3aa681317d02) )
	ROM_LOAD16_BYTE( "epr-6705.bin", 0x04001, 0x2000, CRC(985327cb) SHA1(86969fe763cbaa527d64de35844773b5ab1d7f83) )
	ROM_LOAD16_BYTE( "epr-6708.bin", 0x04000, 0x2000, CRC(f8863dc5) SHA1(bfdd294d51420dd70aa97942909a9b8a95ffc05c) )
	ROM_LOAD16_BYTE( "epr-6706.bin", 0x08001, 0x2000, BAD_DUMP CRC(79e94cd2) SHA1(f44c2292614b46116818fad9a7eb48cceeb3b819)  )  // was bad, manually patched
	ROM_LOAD16_BYTE( "epr-6709.bin", 0x08000, 0x2000, CRC(f41cb58c) SHA1(f0d1048e949d51432739755f985e4df65b8e918b) )
	ROM_FILL(                        0x0c000, 0x4000, 0x00 )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "epr-6703.bin", 0x00000, 0x2000, CRC(fbff3a86) SHA1(4ed2887b1e4509ded853a230f735d4d2aa475886) )
	ROM_LOAD( "epr-6702.bin", 0x02000, 0x2000, CRC(27ddf031) SHA1(2f11d3b693e46852762669ed1e35a667990edec7) )
	ROM_LOAD( "epr-6701.bin", 0x04000, 0x2000, CRC(3c83588a) SHA1(a84c813ba9d464cffc855397aaacbb9177c86fb4) )
	ROM_LOAD( "epr-6700.bin", 0x06000, 0x2000, CRC(ee579266) SHA1(94dfcf506049fc78db00084ff7031d19520d9a85) )
	ROM_LOAD( "epr-6699.bin", 0x08000, 0x2000, CRC(9bfa4a72) SHA1(8ac4d308dab0d67a26b4e3550c2e8064aaf36a74) )
	ROM_LOAD( "epr-6698.bin", 0x0a000, 0x2000, CRC(7adfd1ff) SHA1(b543dd6734a681a187dabf602bea390de663039c) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8505_44801c57.bin", 0x0000, 0x2000, BAD_DUMP CRC(1f5a1405) SHA1(23f2e23db402f88037a5cbdab2935ec1b9a05298) ) // 8304 is not dumped yet, using 8505 instead, works ok

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "epr-6710.bin", 0x00000, 0x1000, CRC(accda190) SHA1(265d2fd92574d65e7890e48d5f305bf903a67bc8) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "epr-6695.bin", 0x00000, 0x2000, CRC(22bea465) SHA1(4860d7ee3c386cdacc9c608ffe74ec8bfa58edcb) )
	// empty space to unpack previous ROM
	ROM_LOAD( "epr-6694.bin", 0x04000, 0x2000, CRC(51a7345e) SHA1(184c890559ed633e23cb459c313e6179cc3eb542) )
	// empty space to unpack previous ROM
	ROM_LOAD( "epr-6689.bin", 0x08000, 0x2000, CRC(53bf7587) SHA1(0046cd04d11ce789ff69e0807700a624af96eb36) )
	ROM_LOAD( "epr-6688.bin", 0x0a000, 0x2000, CRC(ceb76c5b) SHA1(81fa236871f10c77eb201e1c9771bd57406df15b) )
	ROM_LOAD( "epr-6687.bin", 0x0c000, 0x2000, CRC(9c1f49df) SHA1(1a5cf5278777f829d3654e838bd2bb9f4dbb57ba) )
	ROM_LOAD( "epr-6686.bin", 0x0e000, 0x2000, CRC(3d9e516f) SHA1(498614821f87dbcc39edb1756e1af6b536044e6a) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "epr-6696.bin", 0x00000, 0x2000, CRC(0625f48e) SHA1(bea09ccf37f38678fb53c55bd0a79557d6c81b3f) )
	// empty space to unpack previous ROM
	ROM_LOAD( "epr-6697.bin", 0x04000, 0x2000, CRC(f18afabe) SHA1(abd7f6c0bd0de145c423166a2f4e86ccdb12b1ce) )
	// empty space to unpack previous ROM
	ROM_LOAD( "epr-6690.bin", 0x08000, 0x2000, CRC(a142a11d) SHA1(209c7e0591622434ada4445f3f8789059c5f4f77) )
	ROM_LOAD( "epr-6691.bin", 0x0a000, 0x2000, CRC(b640568c) SHA1(8cef1387c469abec8b488621a94cc9575d6c5fcc) )
	ROM_LOAD( "epr-6692.bin", 0x0c000, 0x2000, CRC(b91d8172) SHA1(8d8f6ea78ebf652f295ce96abf19e628fe777d07) )
	ROM_LOAD( "epr-6693.bin", 0x0e000, 0x2000, CRC(874e3acc) SHA1(29438f196811fc2c8f54b6c47f1c175e4797dd4c) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "pr6627.bpr",  0x0000, 0x100, CRC(5ec5480d) SHA1(f966a277539a5d257f32692cdd92ce44b08599e8) ) // R
	ROM_LOAD( "pr6629.bpr",  0x0100, 0x100, CRC(29c7a393) SHA1(67cced39c0a80655c420aad668dfe836c1d7c643) ) // G
	ROM_LOAD( "pr6628.bpr",  0x0200, 0x100, CRC(8af247a4) SHA1(01702fbce53dd4875e4825f0487e7aed9cf212fa) ) // B
	ROM_LOAD( "pr6630a.bpr", 0x0300, 0x100, CRC(d6e202da) SHA1(500ebd5c95d2d2c33535d25cf7f8f649897dc224) ) // CLUT(same PROM x 4)
	ROM_LOAD( "pr6630b.bpr", 0x0400, 0x100, CRC(d6e202da) SHA1(500ebd5c95d2d2c33535d25cf7f8f649897dc224) )
	ROM_LOAD( "pr6630c.bpr", 0x0500, 0x100, CRC(d6e202da) SHA1(500ebd5c95d2d2c33535d25cf7f8f649897dc224) )
	ROM_LOAD( "pr6630d.bpr", 0x0600, 0x100, CRC(d6e202da) SHA1(500ebd5c95d2d2c33535d25cf7f8f649897dc224) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "pr.bpr",      0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/******************************************************************************/
/*
Gekisou (JPN Ver.)
(c)1985 Eastern

68K55-2
CPU:MC68000P8
OSC:12.000MHz

SOUND BOARD NO.59 MC 07
CPU  :TMP8085AP
Sound:AY-3-8910A,OKI M5232,D8155HC
OSC  :6.144MHz
*/
ROM_START( gekisou )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "1.15b", 0x00001, 0x4000, CRC(945fd546) SHA1(6045dbf11272fcec8320aacb2852d4223d0943a0) )
	ROM_LOAD16_BYTE( "2.15d", 0x00000, 0x4000, CRC(3c057150) SHA1(2b1ad7993addfd1c0eee99dfe5bb3476cd387f6a) )
	ROM_LOAD16_BYTE( "3.14b", 0x08001, 0x4000, CRC(7c1cf4d0) SHA1(a122d3a51d205123e04c694912809e0bb31155d5) )
	ROM_LOAD16_BYTE( "4.14d", 0x08000, 0x4000, CRC(c7282391) SHA1(144a34d74bb1e71e2f799913ab04927d00faec87) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "v1.1l", 0x00000, 0x4000, CRC(dc6af437) SHA1(77112fec51343d8e73765b2a342a888612813c3b) )
	ROM_LOAD( "v2.1h", 0x04000, 0x4000, CRC(cb12582e) SHA1(ef378232e2744540cc4c9187cfb36d780dadc962) )
	ROM_LOAD( "v3.1e", 0x08000, 0x4000, CRC(0ab5e777) SHA1(9177c42418f022a65d73c3302873b894c5a137a4) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8505_44801c57.bin", 0x0000, 0x2000, BAD_DUMP CRC(1f5a1405) SHA1(23f2e23db402f88037a5cbdab2935ec1b9a05298) ) // 8304 is not dumped yet, using 8505 instead, works ok

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "0.5c",  0x00000, 0x1000, CRC(7e8bf4d1) SHA1(8abb82be006e8d1df449a5f83d59637314405119) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "7.18r",   0x00000, 0x2000, CRC(a1918b6c) SHA1(6ffa4c845d23d311b59cc19411a68a782618b3fd) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(        0x04000, 0x2000)
	// empty space to unpack previous ROM
	ROM_LOAD( "5.16r",   0x08000, 0x2000, CRC(88ef550a) SHA1(b50e7b8257d1bb6923d289e7af885c14d089b394) )
	ROM_CONTINUE(        0x0c000, 0x2000)
	ROM_LOAD( "6.15r",   0x0a000, 0x2000, CRC(473e3fbf) SHA1(5039387b3627c19f592d630ba7bd010a3881adc5) )
	ROM_CONTINUE(        0x0e000, 0x2000)

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "10.9r",   0x00000, 0x2000, CRC(11d89c73) SHA1(8753f635d321c8e9b93b0ab767cf44aca1db7a0a) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(        0x04000, 0x2000)
	// empty space to unpack previous ROM
	ROM_LOAD( "8.8r",    0x08000, 0x2000, CRC(2e0c392c) SHA1(48542a24a34e3d5d00af418b29a2ee15557efc99) )
	ROM_CONTINUE(        0x0c000, 0x2000)
	ROM_LOAD( "9.6r",    0x0a000, 0x2000, CRC(56a03b08) SHA1(d90b246890fedfc437de85be8bcc6b60ff068be1) )
	ROM_CONTINUE(        0x0e000, 0x2000)

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "1b.bpr",  0x0000, 0x100, CRC(11a1c0aa) SHA1(d007d31f68bf802c89422ea2393747ac8de94d70) ) // R
	ROM_LOAD( "4b.bpr",  0x0100, 0x100, CRC(c7ebe52c) SHA1(19d2ee70d67fd5e1c57f66d030ec9a5b6af5a49e) ) // G
	ROM_LOAD( "2b.bpr",  0x0200, 0x100, CRC(4f5d4141) SHA1(965221c6af4a868760e6d168b55e037fc5f9fa52) ) // B
	ROM_LOAD( "2n.bpr",  0x0300, 0x100, CRC(c7333120) SHA1(ad590e8ece3dcf56b285c4a080f4ee8bb4c9d77c) ) // CLUT(same PROM x 4)
	ROM_LOAD( "3n.bpr",  0x0400, 0x100, CRC(c7333120) SHA1(ad590e8ece3dcf56b285c4a080f4ee8bb4c9d77c) )
	ROM_LOAD( "4n.bpr",  0x0500, 0x100, CRC(c7333120) SHA1(ad590e8ece3dcf56b285c4a080f4ee8bb4c9d77c) )
	ROM_LOAD( "5n.bpr",  0x0600, 0x100, CRC(c7333120) SHA1(ad590e8ece3dcf56b285c4a080f4ee8bb4c9d77c) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/******************************************************************************/
/*
Bingo Time
(c)1986 CLS

68K55-2
CPU:MC68000P8
OSC:12.000MHz

SOUND BOARD NO.60 MC 01
CPU  :Z80A
Sound:AY-3-8910A (unpopulated: another 8910 and a YM2203)
OSC  :6.000MHz
*/

ROM_START( bngotime )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "1.b15", 0x00001, 0x4000, CRC(34a27f5c) SHA1(d30ac37d8665ccc92f6a10f6b0f55783096df687) )
	ROM_LOAD16_BYTE( "0.d15", 0x00000, 0x4000, CRC(21c738ee) SHA1(8c14265fe1ea44945555b37cb13ff6b72c747053) )
	ROM_LOAD16_BYTE( "3.b14", 0x08001, 0x4000, CRC(e22555ab) SHA1(5c533b0b99ef600e2bc42c21b79a1a6914b1fc1e) )
	ROM_LOAD16_BYTE( "2.d14", 0x08000, 0x4000, CRC(0f328bde) SHA1(30a98924600fc2beec8227100adfa6dfbbce5d67) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 )
	ROM_LOAD( "11.sub", 0x00000, 0x2000, CRC(9b063c07) SHA1(c9fe7fe10bd204cb82066db7b576072df7787046) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8505_44801c57.bin", 0x0000, 0x2000, CRC(1f5a1405) SHA1(23f2e23db402f88037a5cbdab2935ec1b9a05298) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "9.d5",  0x00000, 0x1000, CRC(3c356e82) SHA1(55a58f1335206a0996caf8967b4ee962d2373db4) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "6.r18",   0x00000, 0x2000, CRC(e85790f2) SHA1(473d5074e506cfe9ccc8d2a86ee64328b6cefa5f) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(        0x04000, 0x2000)
	// empty space to unpack previous ROM
	ROM_LOAD( "4.r16",   0x08000, 0x2000, CRC(58479aaf) SHA1(916f6b193da7ed223831ca30d3ec8c57f5f1fa7f) )
	ROM_CONTINUE(        0x0c000, 0x2000)
	ROM_LOAD( "5.r15",   0x0a000, 0x2000, CRC(561dbbf6) SHA1(7a294b744ed96962e2d69bfd5d92b690c16b6371) )
	ROM_CONTINUE(        0x0e000, 0x2000)

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "8.r9",   0x00000, 0x2000, CRC(067fd3a1) SHA1(8aeeb5c9a79db4e624de6203ce3810d715cbb35c) )
	// empty space to unpack previous ROM
	ROM_LOAD( "7.r8",    0x08000, 0x2000, CRC(4f50006a) SHA1(2e501181678b904577f457129e6c5e00542e3996) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "82s129_br.b1",  0x0000, 0x100, CRC(fd98b98a) SHA1(754797272338adf36c951fa4cfc40dbcd3429c18) ) // R
	ROM_LOAD( "82s129_bg.b4",  0x0100, 0x100, CRC(68d61fca) SHA1(4143587c3e68157e488093efabb7d182cdece111) ) // G
	ROM_LOAD( "82s129_bb.b2",  0x0200, 0x100, CRC(839bc7a3) SHA1(54289fb75676a30640babf831edf659d84d1616d) ) // B
	ROM_LOAD( "82s129_bs.n2",  0x0300, 0x100, CRC(1ecbeb37) SHA1(c4a139bc81f31b668c80c2cf150ce44b9b181e8a) ) // CLUT(same PROM x 4)
	ROM_LOAD( "82s129_bs.n3",  0x0400, 0x100, CRC(1ecbeb37) SHA1(c4a139bc81f31b668c80c2cf150ce44b9b181e8a) )
	ROM_LOAD( "82s129_bs.n4",  0x0500, 0x100, CRC(1ecbeb37) SHA1(c4a139bc81f31b668c80c2cf150ce44b9b181e8a) )
	ROM_LOAD( "82s129_bs.n5",  0x0600, 0x100, CRC(1ecbeb37) SHA1(c4a139bc81f31b668c80c2cf150ce44b9b181e8a) )
ROM_END



/******************************************************************************/
// Initializations

void equites_state::unpack_block(const char *region, int offset, int size)
{
	uint8_t *rom = memregion(region)->base();

	for (int i = 0; i < size; i++)
	{
		rom[(offset + i + size)] = (rom[(offset + i)] >> 4);
		rom[(offset + i)] &= 0x0f;
	}
}

void equites_state::unpack_region(const char *region)
{
	unpack_block(region, 0x0000, 0x2000);
	unpack_block(region, 0x4000, 0x2000);
}


void equites_state::init_equites()
{
	unpack_region("gfx2");
	unpack_region("gfx3");
}

} // anonymous namespace



/******************************************************************************/
// Game Entries

GAME( 1984, equites,   0,        equites,  equites,  equites_state,  init_equites,  ROT90, "Alpha Denshi Co.", "Equites", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, equitess,  equites,  equites,  equites,  equites_state,  init_equites,  ROT90, "Alpha Denshi Co. (Sega license)", "Equites (Sega)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, bullfgtr,  0,        equites,  bullfgtr, equites_state,  init_equites,  ROT90, "Alpha Denshi Co.", "Bull Fighter", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, bullfgtrs, bullfgtr, equites,  bullfgtr, equites_state,  init_equites,  ROT90, "Alpha Denshi Co. (Sega license)", "Bull Fighter (Sega)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, kouyakyu,  0,        equites,  kouyakyu, equites_state,  init_equites,  ROT0,  "Alpha Denshi Co.", "The Koukou Yakyuu", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, gekisou,   0,        gekisou,  gekisou,  gekisou_state,  init_equites,  ROT90, "Eastern Corp.", "Gekisou (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, bngotime,  0,        bngotime, bngotime, equites_state,  init_equites,  ROT90, "CLS", "Bingo Time", MACHINE_SUPPORTS_SAVE ) // emulation of the sound board is imperfect (flag is in the audio device)
