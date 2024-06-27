// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :    68000 + 65C02
Custom :    X1-001A, X1-002A (SDIP64)   Sprites
            X1-001
            X1-002
            X1-003 or X1-007 (SDIP42)   Video blanking (feeds RGB DACs)
            X1-004           (SDIP52)   Inputs
            X1-005 or X1-009 (DIP48)    NVRAM/simple protection
            X1-006           (SDIP64)   Palette
            X1-010           (QFP80)    Sound: 16 Bit PCM
            X1-011           (QFP80)    Graphics mixing
            X1-012           (QFP100)   Tilemaps
            X1-014                      Sprites?

-------------------------------------------------------------------------------
Ordered by Board        Year + Game                             Licensed To
-------------------------------------------------------------------------------
P0-029-A (M6100287A)    88 Thundercade / Twin Formation (1)     Taito
P0-034A  (M6100326A)    88 Twin Eagle                           Taito
P0-044B                 88 Caliber 50                           Taito / RomStar
P0-045A  (M6100429A)    89 DownTown                             Taito / RomStar
P0-045A                 89 Arbalester                           Taito / RomStar
P0-046A  (M6100430A)    89 U.S. Classic(2)                      Taito / RomStar
P1-036-A + P0-045-A +
P1-049A                 89 Meta Fox                             Taito / RomStar
-------------------------------------------------------------------------------
(1) YM2203 + YM3812 instead of X1-010
(2) wrong colors

Notes:
- The NEC D4701 used by Caliber 50 is a mouse interface IC (uPD4701c).
  Of course it's used to control the spinners. U.S. Classic also uses one to control both trackballs.
- DownTown receives the joystick rotation counts through a pair of JST13 connectors (CN1, CN2).
  On Meta Fox, these connectors are present but unused; they are not populated on Arbalester.

TODO:
- metafox test grid not aligned when screen flipped
- tndrcade: lots of flickering sprites


***************************************************************************/

/***************************************************************************

Thundercade / Twin Formation
Taito America Corp./Romstar USA/Seta, 1987
Hardware info by Guru

PCB Layout
---------
M6100287A
P0-029-A
|------------------------------------------------------------------|
|M    L    K     J     H     G     F     E     D     C     B     A1|
|A UA0-4  UA0-3  4364  UA0-2  UA0-1  4364           16MHz          |
|                                         X1-001           X1-002 2|
|  68000                                                           |
|                                                                  |
|                                                                 3|
|                                                                  |
|                                                                  |
|                       4364 4364   UA0-9  UA0-8  UA0-7  UA0-6    4|
|                                                                  |
|                                                                  |
|                                                                 5|
|                            4584                                  |
|                                                                  |
|                        RESET_SW   UA0-13 UA0-12 UA0-11 UA0-10   6|
|                        4050  TL7705                              |
|                                                                  |
|                                                                 7|
|      X0-006                                                      |
|                                                                  |
|                            SW1                                  8|
| UA10-5 2016 YM3812 YM2203                                        |
|            YM3014          SW2                   X1-006         9|
|VOL   4558  YM3014 M54528            X1-004                       |
|                                                                10|
|                         X2-003 X2-003 X2-003     X1-003          |
|                                                                11|
|            |--|              JAMMA               |--|            |
|------------|  |----------------------------------|  |------------|
Notes:
      68000  - clock 8.000MHz [16/2]
      YM3812 - clock 4.000MHz [16/4]
      YM2203 - clock 4.000MHz [16/4]
      4364   - 8kx8 SRAM
      2016   - 2kx8 SRAM
      VSync  - 59.1845Hz
      HSync  - 15.21kHz

      Custom Chips -
                    X1-001 (SDIP64)
                    X1-002 (SDIP64)
                    X1-006 (SDIP64)
                    X0-006 (SDIP64), also marked 'RP5A10-0001'. This is a Ricoh 65c02 with ROM, RAM and
                                     logic acting as a protection chip.
                                     clocks - pin1 16MHz, pin2 2MHz, pin3 59.1845Hz [VSYNC),
                                     pin63 2MHz, pin62 2MHz
                    X1-003 (SDIP42)
                    X1-004 (SDIP52)
                    X2-003 (DIP16) - Resistor network IC for inputs

      ROMs -
             UA0-6 to AU0-13 DIP42 maskROM, read as MX27C2100 (2 Meg)
             UA0-2 DIP32 27C1000 EPROM
             All other ROMs DIP32 read as 27C1000
             All ROMs have IC locations, but the PCB also has IC locations so components
             can be referenced with IC# or location. See above diagram for info.

***************************************************************************/

/***************************************************************************

                                Twin Eagle

M6100326A   Taito (Seta)

ua2-4              68000
ua2-3
ua2-6
ua2-5
ua2-8
ua2-10
ua2-7               ua2-1
ua2-9
ua2-12
ua2-11              ua2-2

***************************************************************************/

/***************************************************************************

                                U.S. Classic

M6100430A (Taito 1989)

       u7 119  u6 118   u5 117   u4 116
                                         68000-8
u13  120                                 000
u19  121                                 001
u21  122                                 002
u29  123                                 003
u33  124
u40  125
u44  126
u51  127
u58  128
u60  129                                 65c02
u68  130
u75  131                                 u61 004

                                         u83 132

***************************************************************************/
/***************************************************************************

Caliber 50 (Athena / Seta, 1989)
Hardware info by Guru


PCB Layout
----------

P0-044B
|--------------------------------------------------------------|
| VOL  3404   2063 UH-001-013.12M  UH-001-010.8M UH-001-007.4M |N
|MB3730 3403        UH-001-012.11M  UH-001-009.6M UH-001-006.2M|M
|                      UH-001-011.9M  UH-001-008.5M            |L
|            |------|                     |--------|           |K
|            |X1-010|                     |X1-002A |           |
|            |------|                     |--------|           |J
|                                         |--------|           |H
|J                  |------| |------|     |X1-001A |        SW2|
|A                  |X1-011| |X1-012|     |--------|           |G
|M                  |------| |------|                          |F
|M        UH-001-005.17E                                    SW1|E
|A     UPD4701    65C02             8464     8464              |
|   4584                                8464      8464         |
|                       16MHz                           |----| |D
|                                                       | 6  | |
|                                                       | 8  | |C
| X2-005(X5)        X1-006                UH-002-001.3B | 0  | |
|               X1-007    UH-002-004.11B       51832    | 0  | |B
| SW3     X1-004             LH5116  UH_001_002.7B      | 0  | |
|                         BAT    UH_001_003.9B 51832    |----| |A
|--------------------------------------------------------------|
21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1

Notes:
         68000 - Toshiba TMP68000N-8 in SDIP64 package. Clock 8.000MHz [16/2]
         65C02 - Rockwell R65C02P2. Clock 2.000MHz [16/8]
          8464 - Fujitsu MB8464 8kBx8-bit SRAM
          2063 - Toshiba TMM2063 8kBx8-bit SRAM
         51832 - Toshiba TC51832 32kBx8-bit Psuedo SRAM
        LH5116 - Sharp LH5116 2kBx8-bit SRAM (battery-backed)
       UPD4701 - NEC uPD4701 Schmitt-Triggered 12-bit Binary X-Y 2-axis Incremental Encoder Counter.
                 Used for controlling the shooting direction similar to how rotary joysticks work.
                 The original control is called a 'Loop24 Joystick' and is very specialised.
                 The joystick name 'Loop24' suggests there are 24 positions.
                 The joystick has 4 additional wires coming from the bottom; +5V, GND, Loop1, Loop2.
                 Loop1 connects to JAMMA pin 25 and Loop2 connects to JAMMA pin 26. This is basically a spinner or half of a trackball encoder.
                 The game uses only 1 axis per player. Player 1 uses inputs Xa, Xb and player 2 uses inputs Ya, Yb
          4584 - Toshiba TC4584 Hex Schmitt Trigger 4000-series logic chip. This is wired to the JAMMA pins 25 and 26
                 via a 10k resistor array and is used for inputting the shooting direction into the uPD4701.
           BAT - CR2032 3V Lithium Battery with solder tags
         HSync - 15.6250kHz. Measured on X1-007 pin 22
         VSync - 57.4449Hz. Measured on X1-007 pin 23
         SW1/2 - 8 position DIP Switch. Note SW1#1 and SW2#7 are hard-wired to ground and do nothing.
                 The PCB could be modified to enable them but by default the region changing does not work on the real PCB.
                 In MAME it changes the region when those switches are changed. On the real PCB the region is fixed to
                 Japan with Seta as the manufacturer. This effectively means SW1#1 and SW2#7 are always ON.
                 But even though they work in MAME, on the test screen they don't show as 'on' even when toggled in MAME
                 and the test screen just ignores it.
        MB3730 - Fujitsu MB3730 Audio Power Amp
        X1-010 - 16-bit PCM sound chip. Clock input 16.000MHz
        X2-005 - Custom resistor array used for inputs
        X1-004 - Seta custom chip marked 'X1-004' in SDIP52 package used for I/O
        X1-006 - Seta custom chip marked 'X1-006' in SDIP64 package used for palette and pixel mixing functions
        X1-007 - Seta custom chip marked 'X1-007' in SDIP42 package
                 RGB and H/V Sync on the JAMMA connector are tied to this chip so likely this is an RGB DAC
       X1-001A - Seta custom graphics chip \
       X1-002A - Seta custom graphics chip / these work together to create sprites
        X1-011 - Seta custom chip used for graphics mixing
        X1-012 - Seta custom chip used for tilemaps
          3404 - JRC3404 Dual Operational Amplifier
          3403 - JRC3403 Quad Operational Amplifier
           SW3 - Push button switch for reset
UH-001-005.17E - 23C2001 32 pin 2Mbit mask ROM (65C02 sound program)
UH-001-012.11M \ 23C4001 32 pin 4Mbit mask ROM (X1-010 samples)
UH-001-013.12M /
 UH-002-001.3B \ 23C2000 40 pin 2Mbit mask ROM (main program)
UH-002-004.11B /
 UH_001_002.7B \ 27C512 EPROM (main program)
 UH_001_003.9B /
 UH-001-006.2M \
 UH-001-007.4M |
 UH-001-008.5M | 23C4001 32 pin 4Mbit mask ROM (sprites)
 UH-001-009.6M /
 UH-001-010.8M \ 23C4001 32 pin 4Mbit mask ROM (background tiles)
 UH-001-011.9M /

Note not all ROMs have IC locations but regardless, the locations that are there are under chips
and can't be seen unless the chip is removed. Therefore all ROMs are named with x,y locations.

***************************************************************************/
/***************************************************************************

                                    Meta Fox

(Seta 1990)

P0-045A

P1-006-163                    8464   68000-8
P1-007-164    X1-002A X1-001A 8464
P1-008-165                    8464
P1-009-166                    8464     256K-12
                                       256K-12

                 X1-012
                 X1-011


   2063    X1-010     X1-006     X0-006
                      X1-007
                      X1-004     X1-004

----------------------
P1-036-A

UP-001-010
UP-001-011
UP-001-012
UP-001-013


UP-001-014
UP-001-015

-----------------------
P1-049-A

              UP-001-001
              UP-001-002
              P1-003-161
              P1-004-162


              UP-001-005
              x

***************************************************************************/


#include "emu.h"
#include "x1_012.h"

#include "cpu/m6502/m65c02.h"
#include "cpu/m68000/m68000.h"
#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/upd4701.h"
#include "machine/watchdog.h"
#include "sound/x1_010.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "video/x1_001.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#define LOG_PROT     (1U << 1)

#define LOG_ALL      (LOG_PROT)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGPROT(...) LOGMASKED(LOG_PROT, __VA_ARGS__)

namespace {

class tndrcade_state : public driver_device
{
public:
	tndrcade_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_screen(*this, "screen"),
		m_spritegen(*this, "spritegen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch%u", 1U),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_dsw(*this, "DSW"),
		m_coins(*this, "COINS"),
		m_paletteram(*this, "paletteram"),
		m_sharedram(*this, "sharedram"),
		m_subbank(*this, "subbank")
	{ }

	void tndrcade(machine_config &config);

protected:
	virtual void machine_start() override;

	u16 ipl1_ack_r();
	void ipl1_ack_w(u16 data = 0);

	void seta_coin_lockout_w(u8 data);
	X1_001_SPRITE_GFXBANK_CB_MEMBER(setac_gfxbank_callback);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 sharedram_68000_r(offs_t offset);
	void sharedram_68000_w(offs_t offset, u8 data);
	void sub_ctrl_w(offs_t offset, u8 data);
	void sub_bankswitch_w(u8 data);
	void sub_bankswitch_lockout_w(u8 data);
	u8 ff_r();
	u8 dsw1_r();
	u8 dsw2_r();

	TIMER_DEVICE_CALLBACK_MEMBER(tndrcade_sub_interrupt);

	void tndrcade_map(address_map &map);
	void tndrcade_sub_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<x1_001_device> m_spritegen;
	required_device<palette_device> m_palette;
	optional_device_array<generic_latch_8_device, 2> m_soundlatch;

	optional_ioport m_p1;
	optional_ioport m_p2;
	required_ioport m_dsw;
	required_ioport m_coins;

	optional_shared_ptr<u16> m_paletteram;
	optional_shared_ptr<u8> m_sharedram;

	required_memory_bank m_subbank;

	u8 m_sub_ctrl_data = 0;
};

class downtown_state : public tndrcade_state
{
public:
	downtown_state(const machine_config &mconfig, device_type type, const char *tag) :
		tndrcade_state(mconfig, type, tag),
		m_tiles(*this, "tiles"),
		m_x1snd(*this, "x1snd"),
		m_rot(*this, "ROT%u", 1)
	{ }

	void calibr50(machine_config &config);
	void downtown(machine_config &config);
	void metafox(machine_config &config);
	void arbalest(machine_config &config);
	void twineagl(machine_config &config);

	void init_downtown();
	void init_twineagl();
	void init_metafox();
	void init_arbalest();

protected:
	u16 ipl2_ack_r();
	void ipl2_ack_w(u16 data = 0);

	u32 screen_update_downtown(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vram_layer0_vctrl_raster_trampoline_w(offs_t offset, u16 data, u16 mem_mask);

	u16 metafox_protection_r(offs_t offset);
	void twineagl_tilebank_w(offs_t offset, u8 data);
	u8 downtown_ip_r(offs_t offset);
	void calibr50_sub_bankswitch_w(u8 data);
	void calibr50_soundlatch2_w(u8 data);
	void twineagl_ctrl_w(u8 data);
	u16 twineagl_debug_r();
	u16 twineagl_200100_r(offs_t offset);
	void twineagl_200100_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 downtown_protection_r(offs_t offset);
	void downtown_protection_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 arbalest_debug_r();

	DECLARE_MACHINE_RESET(calibr50);
	u16 twineagl_tile_offset(u16 code);

	TIMER_DEVICE_CALLBACK_MEMBER(seta_sub_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(calibr50_interrupt);

	void calibr50_map(address_map &map);
	void calibr50_sub_map(address_map &map);
	void downtown_map(address_map &map);
	void downtown_sub_map(address_map &map);
	void metafox_sub_map(address_map &map);
	void twineagl_sub_map(address_map &map);

	required_device<x1_012_device> m_tiles;
	required_device<x1_010_device> m_x1snd;

	optional_ioport_array<2> m_rot;

	u8 m_twineagl_xram[8] = { };
	u8 m_twineagl_tilebank[4] = { };

	std::unique_ptr<u8[]> m_downtown_protection;
};

class usclssic_state : public downtown_state
{
public:
	usclssic_state(const machine_config &mconfig, device_type type, const char *tag) :
		downtown_state(mconfig, type, tag),
		m_upd4701(*this, "upd4701"),
		m_buttonmux(*this, "buttonmux"),
		m_track_x(*this, "TRACK%u_X", 1U),
		m_track_y(*this, "TRACK%u_Y", 1U)
	{ }

	void usclssic(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(trackball_x_r);
	DECLARE_CUSTOM_INPUT_MEMBER(trackball_y_r);

protected:
	virtual void machine_start() override;

private:
	u16 dsw_r(offs_t offset);
	void lockout_w(u8 data);

	void usclssic_palette(palette_device &palette) const;

	u16 tile_offset(u16 code);
	u32 screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void usclssic_set_pens();

	void usclssic_map(address_map &map);

	required_device<upd4701_device> m_upd4701;
	required_device<hc157_device> m_buttonmux;
	required_ioport_array<2> m_track_x;
	required_ioport_array<2> m_track_y;

	u8 m_port_select = 0;
	u16 m_tiles_offset = 0;
};


void downtown_state::twineagl_tilebank_w(offs_t offset, u8 data)
{
	if (m_twineagl_tilebank[offset] != data)
	{
		m_twineagl_tilebank[offset] = data;
		m_tiles->mark_all_dirty();
	}
}

u16 downtown_state::twineagl_tile_offset(u16 code)
{
	if ((code & 0x3e00) == 0x3e00)
		return (code & 0x007f) | ((m_twineagl_tilebank[(code & 0x0180) >> 7] >> 1) << 7);
	else
		return code;
}

u16 usclssic_state::tile_offset(u16 code)
{
	return m_tiles_offset + code;
}

X1_001_SPRITE_GFXBANK_CB_MEMBER(tndrcade_state::setac_gfxbank_callback)
{
	const int bank = (color & 0x06) >> 1;
	code = (code & 0x3fff) + (bank * 0x4000);

	return code;
}


void downtown_state::vram_layer0_vctrl_raster_trampoline_w(offs_t offset, u16 data, u16 mem_mask)
{
	// Used by calibr50 as VIDEO_UPDATE_SCANLINE is problematic due to devices/video/x1_001.cpp not being optimized
	// for scanline drawing, so instead we use this trampoline on tilemap register writes. Also see notes in x1_012.cpp
	// for why we can't just do this in vctrl_w.
	m_screen->update_partial(m_screen->vpos());
	m_tiles->vctrl_w(offset, data, mem_mask);
}


/***************************************************************************

                            Palette Init Functions

***************************************************************************/


void usclssic_state::usclssic_palette(palette_device &palette) const
{
	const u8 *const color_prom = memregion("proms")->base();

	// decode PROM
	for (int x = 0; x < 0x200; x++)
	{
		const u16 data = (color_prom[x*2] << 8) | color_prom[x*2 + 1];
		const rgb_t color(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (x >= 0x100)
			palette.set_indirect_color(x + 0x000, color);
		else
			palette.set_indirect_color(x + 0x300, color);
	}

	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x200 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0xa00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}

void usclssic_state::usclssic_set_pens()
{
	for (int i = 0; i < 0x200; i++)
	{
		const u16 data = m_paletteram[i];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (i >= 0x100)
			m_palette->set_indirect_color(i - 0x100, color);
		else
			m_palette->set_indirect_color(i + 0x200, color);
	}
}


/***************************************************************************

                                Screen Drawing

***************************************************************************/

u32 tndrcade_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, 0x1000);

	return 0;
}

u32 downtown_state::screen_update_downtown(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = screen.visible_area();
	const int vis_dimy = visarea.max_y - visarea.min_y + 1;

	const int flip = m_spritegen->is_flipped();
	m_tiles->set_flip(flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

	// the hardware wants different scroll values when flipped

	/*  bg x scroll      flip
	    metafox     0000 025d = 0, $400-$1a3 = $400 - $190 - $13
	    eightfrc    ffe8 0272
	                fff0 0260 = -$10, $400-$190 -$10
	                ffe8 0272 = -$18, $400-$190 -$18 + $1a      */

	m_tiles->update_scroll(vis_dimy, flip);

	m_tiles->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	m_spritegen->draw_sprites(screen, bitmap, cliprect, 0x1000);

	return 0;
}

u32 usclssic_state::screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	usclssic_set_pens();
	return screen_update_downtown(screen, bitmap, cliprect);
}


/***************************************************************************

                                Common Routines

***************************************************************************/

/*

 Shared RAM:

 The 65c02 sees a linear array of bytes that is mapped, for the 68000,
 to a linear array of words whose low order bytes hold the data

*/

u8 tndrcade_state::sharedram_68000_r(offs_t offset)
{
	return m_sharedram[offset];
}

void tndrcade_state::sharedram_68000_w(offs_t offset, u8 data)
{
	m_sharedram[offset] = data & 0xff;
}


/*

 Sub CPU Control

*/

void tndrcade_state::sub_ctrl_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0/2:   // bit 0: reset sub cpu?
			if (!(m_sub_ctrl_data & 1) && (data & 1))
				m_subcpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			m_sub_ctrl_data = data;
			break;

		case 2/2:   // ?
			break;

		case 4/2:   // not sure
			if (m_soundlatch[0].found()) m_soundlatch[0]->write(data);
			break;

		case 6/2:   // not sure
			if (m_soundlatch[1].found()) m_soundlatch[1]->write(data);
			break;
	}

}

// DSW reading for 8 bit CPUs

u8 tndrcade_state::dsw1_r()
{
	return (m_dsw->read() >> 8) & 0xff;
}

u8 tndrcade_state::dsw2_r()
{
	return (m_dsw->read() >> 0) & 0xff;
}


void tndrcade_state::seta_coin_lockout_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(1, !BIT(data, 3));
}


/***************************************************************************

                                    Main CPU

***************************************************************************/


u16 tndrcade_state::ipl1_ack_r()
{
	if (!machine().side_effects_disabled())
		ipl1_ack_w();
	return 0;
}

void tndrcade_state::ipl1_ack_w(u16 data)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}

u16 downtown_state::ipl2_ack_r()
{
	if (!machine().side_effects_disabled())
		ipl2_ack_w();
	return 0;
}

void downtown_state::ipl2_ack_w(u16 data)
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
}


/***************************************************************************
                                Thundercade
***************************************************************************/

/* Mirror RAM seems necessary since the e00000-e03fff area is not cleared
   on startup. Level 2 int uses $e0000a as a counter that controls when
   to write a value to the sub cpu, and when to read the result back.
   If the check fails "error x0-006" is displayed. Hence if the counter
   is not cleared at startup the game could check for the result before
   writing to sharedram! */


void tndrcade_state::tndrcade_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // ROM
	map(0x200000, 0x200001).w(FUNC(tndrcade_state::ipl1_ack_w));
	map(0x280000, 0x280001).nopw();                        // ? 0 / 1 (sub cpu related?)
	map(0x300000, 0x300001).nopw();                        // ? 0 / 1
	map(0x380000, 0x3803ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // Palette
	map(0x400000, 0x400000).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));
	map(0x600000, 0x6005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0x600600, 0x600607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));

	map(0x800000, 0x800007).w(FUNC(tndrcade_state::sub_ctrl_w)).umask16(0x00ff);               // Sub CPU Control?
	map(0xa00000, 0xa00fff).rw(FUNC(tndrcade_state::sharedram_68000_r), FUNC(tndrcade_state::sharedram_68000_w)).umask16(0x00ff);  // Shared RAM
	map(0xc00000, 0xc03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xe00000, 0xe03fff).ram().share("mainram");                  // RAM (Mirrored?)
	map(0xffc000, 0xffffff).ram().share("mainram");                  // RAM (Mirrored?)
}


/***************************************************************************
                Twin Eagle, DownTown, Arbalester, Meta Fox
        (with slight variations, and Meta Fox protection hooked in)
***************************************************************************/

void downtown_state::twineagl_ctrl_w(u8 data)
{
	if ((data & 0x30) == 0)
	{
		m_maincpu->set_input_line(1, CLEAR_LINE);
		m_maincpu->set_input_line(3, CLEAR_LINE);
	}
}

void downtown_state::downtown_map(address_map &map)
{
	map(0x000000, 0x09ffff).rom();                             // ROM
	map(0x100000, 0x103fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0x200000, 0x200001).noprw();                             // watchdog? (twineagl)
	map(0x300000, 0x300001).w(FUNC(downtown_state::ipl1_ack_w));
	map(0x400000, 0x400007).w(FUNC(downtown_state::twineagl_tilebank_w)).umask16(0x00ff);      // special tile banking to animate water in twineagl
	map(0x500001, 0x500001).w(FUNC(downtown_state::twineagl_ctrl_w));
	map(0x600001, 0x600001).r(FUNC(downtown_state::dsw1_r));
	map(0x600003, 0x600003).r(FUNC(downtown_state::dsw2_r));
	map(0x700000, 0x7003ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");  // Palette
	map(0x800000, 0x800005).w(m_tiles, FUNC(x1_012_device::vctrl_w));// VRAM Ctrl
	map(0x900000, 0x903fff).ram().w(m_tiles, FUNC(x1_012_device::vram_w)).share("tiles"); // VRAM
	map(0xa00000, 0xa00007).w(FUNC(downtown_state::sub_ctrl_w)).umask16(0x00ff);               // Sub CPU Control?
	map(0xb00000, 0xb00fff).rw(FUNC(downtown_state::sharedram_68000_r), FUNC(downtown_state::sharedram_68000_w)).umask16(0x00ff);  // Shared RAM
	map(0xc00000, 0xc00000).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));
	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xf00000, 0xffffff).ram();                             // RAM
}


/***************************************************************************
                                Caliber 50
***************************************************************************/

void downtown_state::calibr50_map(address_map &map)
{
	map(0x000000, 0x09ffff).rom();                             // ROM
	map(0x100000, 0x100001).r(FUNC(downtown_state::ipl2_ack_r));
	map(0x200000, 0x200fff).ram().share("nvram");              // NVRAM (battery backed)
	map(0x300000, 0x300001).rw(FUNC(downtown_state::ipl1_ack_r), FUNC(downtown_state::ipl1_ack_w));
	map(0x400000, 0x400001).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0x500000, 0x500001).nopw();                        // ?
	map(0x600001, 0x600001).r(FUNC(downtown_state::dsw1_r));
	map(0x600003, 0x600003).r(FUNC(downtown_state::dsw2_r));
	map(0x700000, 0x7003ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");  // Palette
	map(0x800000, 0x800005).w(FUNC(downtown_state::vram_layer0_vctrl_raster_trampoline_w));// VRAM Ctrl
	map(0x900000, 0x903fff).ram().w(m_tiles, FUNC(x1_012_device::vram_w)).share("tiles"); // VRAM
	map(0x904000, 0x904fff).ram();                             //
	map(0xa00000, 0xa00001).portr("P1");                 // X1-004
	map(0xa00002, 0xa00003).portr("P2");                 // X1-004
	map(0xa00008, 0xa00009).portr("COINS");              // X1-004
	map(0xa00010, 0xa00017).r("upd4701", FUNC(upd4701_device::read_xy)).umask16(0x00ff);
	map(0xa00019, 0xa00019).r("upd4701", FUNC(upd4701_device::reset_xy_r));
	map(0xb00001, 0xb00001).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)); // From Sub CPU
	map(0xb00001, 0xb00001).w(m_soundlatch[0], FUNC(generic_latch_8_device::write)); // To Sub CPU
	map(0xc00000, 0xc00000).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));
	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xff0000, 0xffffff).ram();                             // RAM
}


/***************************************************************************
                                U.S. Classic
***************************************************************************/

u16 usclssic_state::dsw_r(offs_t offset)
{
	switch (offset)
	{
		case 0/2:   return (m_dsw->read() >>  8) & 0xf;
		case 2/2:   return (m_dsw->read() >> 12) & 0xf;
		case 4/2:   return (m_dsw->read() >>  0) & 0xf;
		case 6/2:   return (m_dsw->read() >>  4) & 0xf;
	}
	return 0;
}

CUSTOM_INPUT_MEMBER(usclssic_state::trackball_x_r)
{
	return m_track_x[m_port_select ? 1 : 0]->read();
}

CUSTOM_INPUT_MEMBER(usclssic_state::trackball_y_r)
{
	return m_track_y[m_port_select ? 1 : 0]->read();
}


void usclssic_state::lockout_w(u8 data)
{
	u16 tiles_offset = BIT(data, 4) ? 0x4000 : 0;

	if (m_port_select != BIT(data, 6))
	{
		m_upd4701->update();
		m_port_select = BIT(data, 6);
		m_buttonmux->select_w(m_port_select);
		m_upd4701->recalibrate();
	}

	m_upd4701->resetx_w(BIT(data, 7));
	m_upd4701->resety_w(BIT(data, 7));

	if (tiles_offset != m_tiles_offset)
		m_tiles->mark_all_dirty();
	m_tiles_offset = tiles_offset;

	seta_coin_lockout_w(data);
}


void usclssic_state::usclssic_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                 // ROM
	map(0x800000, 0x8005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); // Sprites Y
	map(0x800600, 0x800607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0x900000, 0x900000).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));
	map(0xa00000, 0xa00005).rw(m_tiles, FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));         // VRAM Ctrl
	map(0xb00000, 0xb003ff).ram().share(m_paletteram);  // Palette
	map(0xb40000, 0xb40007).r(m_upd4701, FUNC(upd4701_device::read_xy)).umask16(0x00ff);
	map(0xb40001, 0xb40001).w(FUNC(usclssic_state::lockout_w));  // Coin Lockout + Tiles Banking
	map(0xb4000a, 0xb4000b).w(FUNC(usclssic_state::ipl1_ack_w));
	map(0xb40010, 0xb40011).portr("COINS");                  // Coins
	map(0xb40011, 0xb40011).w(m_soundlatch[0], FUNC(generic_latch_8_device::write)); // To Sub CPU
	map(0xb40018, 0xb4001f).r(FUNC(usclssic_state::dsw_r));                // 2 DSWs
	map(0xb40018, 0xb40019).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0xb80000, 0xb80001).r(FUNC(usclssic_state::ipl2_ack_r));
	map(0xc00000, 0xc03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));         // Sprites Code + X + Attr
	map(0xd00000, 0xd03fff).ram().w(m_tiles, FUNC(x1_012_device::vram_w)).share("tiles"); // VRAM
	map(0xd04000, 0xd04fff).ram();                                 //
	map(0xe00000, 0xe00fff).ram();                                 // NVRAM? (odd bytes)
	map(0xff0000, 0xffffff).ram();                                 // RAM
}


/***************************************************************************

                                Sub / Sound CPU

***************************************************************************/

void tndrcade_state::sub_bankswitch_w(u8 data)
{
	m_subbank->set_entry(data >> 4);
}

void tndrcade_state::sub_bankswitch_lockout_w(u8 data)
{
	sub_bankswitch_w(data);
	seta_coin_lockout_w(data);

	// 65C02 code doesn't seem to do anything to explicitly acknowledge IRQ; implicitly acknowledging it here seems most likely
	m_subcpu->set_input_line(m65c02_device::IRQ_LINE, CLEAR_LINE);
}


/***************************************************************************
                                Thundercade
***************************************************************************/

u8 tndrcade_state::ff_r(){return 0xff;}

void tndrcade_state::tndrcade_sub_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();                             // RAM
	map(0x0800, 0x0800).r(FUNC(tndrcade_state::ff_r));                      // ? (bits 0/1/2/3: 1 -> do test 0-ff/100-1e0/5001-57ff/banked rom)
	//map(0x0800, 0x0800).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));             //
	//map(0x0801, 0x0801).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));            //
	map(0x1000, 0x1000).portr("P1");                 // P1
	map(0x1000, 0x1000).w(FUNC(tndrcade_state::sub_bankswitch_lockout_w)); // ROM Bank + Coin Lockout
	map(0x1001, 0x1001).portr("P2");                 // P2
	map(0x1002, 0x1002).portr("COINS");              // Coins
	map(0x2000, 0x2001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x3000, 0x3001).w("ym2", FUNC(ym3812_device::write));
	map(0x5000, 0x57ff).ram().share(m_sharedram);       // Shared RAM
	map(0x6000, 0x7fff).rom();                             // ROM
	map(0x8000, 0xbfff).bankr(m_subbank);                        // Banked ROM
	map(0xc000, 0xffff).rom();                             // ROM
}


/***************************************************************************
                                Twin Eagle
***************************************************************************/

void downtown_state::twineagl_sub_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();                         // RAM
	map(0x0800, 0x0800).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));         //
	map(0x0801, 0x0801).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));            //
	map(0x1000, 0x1000).portr("P1");             // P1
	map(0x1000, 0x1000).w(FUNC(downtown_state::sub_bankswitch_lockout_w)); // ROM Bank + Coin Lockout
	map(0x1001, 0x1001).portr("P2");             // P2
	map(0x1002, 0x1002).portr("COINS");          // Coins
	map(0x5000, 0x57ff).ram().share(m_sharedram);       // Shared RAM
	map(0x7000, 0x7fff).rom();                         // ROM
	map(0x8000, 0xbfff).bankr(m_subbank);                    // Banked ROM
	map(0xc000, 0xffff).rom();                         // ROM
}


/***************************************************************************
                                DownTown
***************************************************************************/

u8 downtown_state::downtown_ip_r(offs_t offset)
{
	int dir1 = m_rot[0]->read();  // analog port
	int dir2 = m_rot[1]->read();  // analog port

	dir1 = (~ (0x800 >> dir1)) & 0xfff;
	dir2 = (~ (0x800 >> dir2)) & 0xfff;

	switch (offset)
	{
		case 0: return (m_coins->read() & 0xf0) + (dir1 >> 8);  // upper 4 bits of p1 rotation + coins
		case 1: return (dir1 & 0xff);                   // lower 8 bits of p1 rotation
		case 2: return m_p1->read();    // p1
		case 3: return 0xff;                            // ?
		case 4: return (dir2 >> 8);                     // upper 4 bits of p2 rotation + ?
		case 5: return (dir2 & 0xff);                   // lower 8 bits of p2 rotation
		case 6: return m_p2->read();    // p2
		case 7: return 0xff;                            // ?
	}

	return 0;
}

void downtown_state::downtown_sub_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();                         // RAM
	map(0x0800, 0x0800).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));         //
	map(0x0801, 0x0801).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));            //
	map(0x1000, 0x1007).r(FUNC(downtown_state::downtown_ip_r));         // Input Ports
	map(0x1000, 0x1000).w(FUNC(downtown_state::sub_bankswitch_lockout_w)); // ROM Bank + Coin Lockout
	map(0x5000, 0x57ff).ram().share(m_sharedram);       // Shared RAM
	map(0x7000, 0x7fff).rom();                         // ROM
	map(0x8000, 0xbfff).bankr(m_subbank);                    // Banked ROM
	map(0xc000, 0xffff).rom();                         // ROM
}


/***************************************************************************
                        Caliber 50 / U.S. Classic
***************************************************************************/

MACHINE_RESET_MEMBER(downtown_state,calibr50)
{
	calibr50_sub_bankswitch_w(0);
}

void downtown_state::calibr50_sub_bankswitch_w(u8 data)
{
	// Bits 7-4: BK3-BK0
	sub_bankswitch_w(data);

	// Bit 3: NMICLR
	if (!BIT(data, 3))
		m_soundlatch[0]->acknowledge_w();

	// Bit 2: IRQCLR
	if (!BIT(data, 2))
		m_subcpu->set_input_line(m65c02_device::IRQ_LINE, CLEAR_LINE);

	// Bit 1: /PCMMUTE
	m_x1snd->set_output_gain(ALL_OUTPUTS, BIT(data, 1) ? 1.0f : 0.0f);
}

void downtown_state::calibr50_soundlatch2_w(u8 data)
{
	m_soundlatch[1]->write(data);
	m_subcpu->spin_until_time(attotime::from_usec(50));  // Allow the other cpu to reply
}

void downtown_state::calibr50_sub_map(address_map &map)
{
	map(0x0000, 0x1fff).lrw8(
								 NAME([this](offs_t offset) { return m_x1snd->read(offset ^ 0x1000); }),
								 NAME([this](offs_t offset, u8 data) { m_x1snd->write(offset ^ 0x1000, data); })); // Sound
	map(0x4000, 0x4000).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));             // From Main CPU
	map(0x4000, 0x4000).w(FUNC(downtown_state::calibr50_sub_bankswitch_w));        // Bankswitching
	map(0x8000, 0xbfff).bankr(m_subbank);                        // Banked ROM
	map(0xc000, 0xffff).rom();                             // ROM
	map(0xc000, 0xc000).w(FUNC(downtown_state::calibr50_soundlatch2_w));   // To Main CPU
}


/***************************************************************************
                                Meta Fox
***************************************************************************/

void downtown_state::metafox_sub_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();                         // RAM
	map(0x0800, 0x0800).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));         //
	map(0x0801, 0x0801).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));            //
	map(0x1000, 0x1000).portr("COINS");          // Coins
	map(0x1000, 0x1000).w(FUNC(downtown_state::sub_bankswitch_lockout_w)); // ROM Bank + Coin Lockout
	map(0x1002, 0x1002).portr("P1");             // P1
	//map(0x1004, 0x1004).nopr();                // ?
	map(0x1006, 0x1006).portr("P2");             // P2
	map(0x5000, 0x57ff).ram().share(m_sharedram);       // Shared RAM
	map(0x7000, 0x7fff).rom();                         // ROM
	map(0x8000, 0xbfff).bankr(m_subbank);                    // Banked ROM
	map(0xc000, 0xffff).rom();                         // ROM
}


/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( common_type1 )
	PORT_START("P1") //Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1         )

	PORT_START("P2") //Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2         )

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END

static INPUT_PORTS_START( common_type2 )
	PORT_START("P1")    // Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1         )

	PORT_START("P2")    // Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2         )

	PORT_START("COINS") // Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
INPUT_PORTS_END



/***************************************************************************
                                Arbalester
***************************************************************************/

static INPUT_PORTS_START( arbalest )
	PORT_INCLUDE(common_type2)

	PORT_START("DSW")   // 2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, "Licensed To" )       PORT_DIPLOCATION("SW1:1") // Manual states "Don't Touch"
	PORT_DIPSETTING(      0x0000, "Taito" )             PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x4000)
	PORT_DIPSETTING(      0x0001, "Jordan" )            PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x4000)
	PORT_DIPSETTING(      0x0000, "Taito" )             PORT_CONDITION("DSW",0x4000,EQUALS,0x4000)
	PORT_DIPSETTING(      0x0001, "Romstar" )           PORT_CONDITION("DSW",0x4000,EQUALS,0x4000) // Manual shows DSW1-1=Off & DSW2-7=Off
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4") // Manual states "Don't Touch"
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "Never" )
	PORT_DIPSETTING(      0x0800, "300k Only" )
	PORT_DIPSETTING(      0x0400, "600k Only" )
	PORT_DIPSETTING(      0x0000, "300k & 600k" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x1000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0x4000, 0x4000, "Licensor Option" )   PORT_DIPLOCATION("SW2:7") // Manual states "Don't Touch"
	PORT_DIPSETTING(      0x4000, "Option 1" )
	PORT_DIPSETTING(      0x0000, "Option 2" )
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" )      PORT_DIPLOCATION("SW2:8") // Manual states "Don't Touch"
	PORT_DIPSETTING(      0x8000, "Coin Mode 1" )
	PORT_DIPSETTING(      0x0000, "Coin Mode 2" )
INPUT_PORTS_END


/***************************************************************************
                                Caliber 50
***************************************************************************/

static INPUT_PORTS_START( calibr50 )
	PORT_INCLUDE(common_type2)

	PORT_START("DSW")   //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, "Copyright / License" )       PORT_DIPLOCATION("SW1:1") // Romstar's Manual states "Don't Touch"
	PORT_DIPSETTING(      0x0000, "Seta (Japan only)" )     PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x4000)
	PORT_DIPSETTING(      0x0001, "Seta USA / Romstar" )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x4000)
	PORT_DIPSETTING(      0x0000, "Seta / Taito" )          PORT_CONDITION("DSW",0x4000,EQUALS,0x4000)
	PORT_DIPSETTING(      0x0001, "Seta USA / Taito America" )  PORT_CONDITION("DSW",0x4000,EQUALS,0x4000) // Romstar's Manual shows DSW1-1=Off & DSW2-7=Off
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Score Digits" )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "7" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Display Score" )     PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Erase Backup Ram" )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Licensor Option" )   PORT_DIPLOCATION("SW2:7") // Romstar's Manual states "Don't Touch"
	PORT_DIPSETTING(      0x4000, "Option 1" )
	PORT_DIPSETTING(      0x0000, "Option 2" )
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" )      PORT_DIPLOCATION("SW2:8") // Romstar's Manual states "Don't Touch"
	PORT_DIPSETTING(      0x8000, "Coin Mode 1" )
	PORT_DIPSETTING(      0x0000, "Coin Mode 2" )

	PORT_START("ROT1")  // Rotation Player 1
	PORT_BIT( 0xfff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)

	PORT_START("ROT2")  // Rotation Player 2
	PORT_BIT( 0xfff, 0x00, IPT_DIAL ) PORT_PLAYER(2) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M)
INPUT_PORTS_END

/***************************************************************************
                                DownTown
***************************************************************************/

static INPUT_PORTS_START( downtown )
	PORT_INCLUDE(common_type2)

	PORT_START("DSW")   //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0000, "Sales" ) PORT_DIPLOCATION("SW1:1")       // Manual for USA version says "Always Off"
	PORT_DIPSETTING(      0x0001, "Japan Only" )
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	// default "Normal" on Romstar manual, and matches ALL OFF scheme
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "Never" )
	PORT_DIPSETTING(      0x0800, "50K Only" )
	PORT_DIPSETTING(      0x0400, "100K Only" )
	PORT_DIPSETTING(      0x0000, "50K, Every 150K" )
	// TODO: defaults to 2 lives in the Romstar manual, contradicts with DIPSW ALL OFF convention.
	// Is it a regional difference? Verify with a Mokugeki Jp manual.
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x1000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0x4000, 0x0000, "World License" ) PORT_DIPLOCATION("SW2:7") // Manual for USA version says "Unused"
	PORT_DIPSETTING(      0x4000, "Romstar" )
	PORT_DIPSETTING(      0x0000, "Taito" )
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" ) PORT_DIPLOCATION("SW2:8") // Manual for USA version says "Unused"
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_START("ROT1")  //Rotation Player 1
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_FULL_TURN_COUNT(12)

	PORT_START("ROT2")  //Rotation Player 2
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END



/***************************************************************************
                                Meta Fox
***************************************************************************/

static INPUT_PORTS_START( metafox )
	PORT_INCLUDE(common_type2)

	PORT_START("DSW") //$600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, "Copyright / License" )   PORT_DIPLOCATION("SW1:1") // Romstar's Manual states "Don't Touch"
	PORT_DIPSETTING(      0x0000, "Seta USA / Taito America" )  PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x4000)
	PORT_DIPSETTING(      0x0001, "Seta / Jordan I.S." )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x4000)
	PORT_DIPSETTING(      0x0000, "Seta / Taito" )          PORT_CONDITION("DSW",0x4000,EQUALS,0x4000)
	PORT_DIPSETTING(      0x0001, "Seta USA / Romstar" )        PORT_CONDITION("DSW",0x4000,EQUALS,0x4000) // Romstar's Manual shows DSW1-1=Off & DSW2-7=Off
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0800, "600K Only" )
	PORT_DIPSETTING(      0x0000, "600k & 900k" )
	PORT_DIPSETTING(      0x0400, "900K Only" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x1000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0x4000, 0x4000, "Licensor Option" )   PORT_DIPLOCATION("SW2:7") // Romstar's Manual states "Don't Touch"
	PORT_DIPSETTING(      0x4000, "Option 1" )
	PORT_DIPSETTING(      0x0000, "Option 2" )
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" )      PORT_DIPLOCATION("SW2:8") // Romstar's Manual states "Don't Touch"
	PORT_DIPSETTING(      0x8000, "Coin Mode 1" )
	PORT_DIPSETTING(      0x0000, "Coin Mode 2" )
INPUT_PORTS_END



/***************************************************************************
                                Thundercade (US)
***************************************************************************/

static INPUT_PORTS_START( tndrcade )
	PORT_INCLUDE(common_type1)

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x000c, "50K  Only" )
	PORT_DIPSETTING(      0x0004, "50K, Every 150K" )
	PORT_DIPSETTING(      0x0000, "70K, Every 200K" )
	PORT_DIPSETTING(      0x0008, "100K Only" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Licensed To" )           PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Taito America Corp." )
	PORT_DIPSETTING(      0x0000, "Taito Corp. Japan" )

	PORT_DIPNAME( 0x0100, 0x0100, "Title" )             PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, "Thundercade" )
	PORT_DIPSETTING(      0x0000, "Twin Formation" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0400, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )            PORT_CONDITION("DSW",0x0080,EQUALS,0x0080)
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )            PORT_CONDITION("DSW",0x0080,EQUALS,0x0080)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )            PORT_CONDITION("DSW",0x0080,EQUALS,0x0080)
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )            PORT_CONDITION("DSW",0x0080,EQUALS,0x0080)
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )            PORT_CONDITION("DSW",0x0080,NOTEQUALS,0x0080)
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )            PORT_CONDITION("DSW",0x0080,NOTEQUALS,0x0080)
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )            PORT_CONDITION("DSW",0x0080,NOTEQUALS,0x0080)
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )            PORT_CONDITION("DSW",0x0080,NOTEQUALS,0x0080)
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )            PORT_CONDITION("DSW",0x0080,EQUALS,0x0080)
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )            PORT_CONDITION("DSW",0x0080,EQUALS,0x0080)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )            PORT_CONDITION("DSW",0x0080,EQUALS,0x0080)
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )            PORT_CONDITION("DSW",0x0080,EQUALS,0x0080)
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )            PORT_CONDITION("DSW",0x0080,NOTEQUALS,0x0080)
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )            PORT_CONDITION("DSW",0x0080,NOTEQUALS,0x0080)
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_4C ) )            PORT_CONDITION("DSW",0x0080,NOTEQUALS,0x0080)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )            PORT_CONDITION("DSW",0x0080,NOTEQUALS,0x0080)
INPUT_PORTS_END


/***************************************************************************
                                Thundercade (Japan)
***************************************************************************/

static INPUT_PORTS_START( tndrcadj )
	PORT_INCLUDE(tndrcade)

	PORT_MODIFY("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )
INPUT_PORTS_END



/***************************************************************************
                                Twin Eagle
***************************************************************************/

static INPUT_PORTS_START( twineagl )
	PORT_INCLUDE(common_type1)

	// default taken off original Japanese DIP sheet
	// Cabinet is user choice
	// We use Licensor Option 2 instead of 1 just to avoid the "For use in Japan"
	// TODO: the copyright dips actually don't have any effect on a real (European) PCB
	// Most likely these are overwritten by the lead JPs that the board has
	// (4x3 near SW2 at J2, 2x3 near two empty ROM sockets U31/U37 at C10).
	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, "Copyright / License" )   PORT_DIPLOCATION("SW1:1") // Always "Seta" if sim. players = 1
	PORT_DIPSETTING(      0x0000, "Taito America / Romstar" )   PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x4000)
	PORT_DIPSETTING(      0x0001, "Taito Corp Japan" )      PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x4000)   // "World" Copyright
	PORT_DIPSETTING(      0x0000, "Taito America" )         PORT_CONDITION("DSW",0x4000,EQUALS,0x4000)
	PORT_DIPSETTING(      0x0001, "Seta / Taito" )          PORT_CONDITION("DSW",0x4000,EQUALS,0x4000)  // Japan Only Notice
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:4") // Only if simultaneous players = 1
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,EQUALS,0x8000)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )        PORT_CONDITION("DSW",0x8000,NOTEQUALS,0x8000)
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "Never" )
	PORT_DIPSETTING(      0x0800, "500K Only" )
	PORT_DIPSETTING(      0x0400, "1000K Only" )
	PORT_DIPSETTING(      0x0000, "500K, Every 1500K" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x1000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0x4000, 0x0000, "Licensor Option" )   PORT_DIPLOCATION("SW2:7") // Manual states "Don't Touch"
	PORT_DIPSETTING(      0x4000, "Option 1" )
	PORT_DIPSETTING(      0x0000, "Option 2" )
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" )      PORT_DIPLOCATION("SW2:8") // Manual states "Don't Touch"
	PORT_DIPSETTING(      0x8000, "Coin Mode 1" )
	PORT_DIPSETTING(      0x0000, "Coin Mode 2" )
INPUT_PORTS_END



/***************************************************************************
                                U.S. Classic
***************************************************************************/

static INPUT_PORTS_START( usclssic )
	PORT_START("TRACKX")
	PORT_BIT( 0xfff, 0x000, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(usclssic_state, trackball_x_r)

	PORT_START("TRACKY")
	PORT_BIT( 0xfff, 0x000, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(usclssic_state, trackball_y_r)

	PORT_START("TRACK1_X")     // muxed port 0
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("TRACK1_Y")     // muxed port 0
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("TRACK2_X")     // muxed port 1
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_COCKTAIL

	PORT_START("TRACK2_Y")     // muxed port 1
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_COCKTAIL

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER("buttonmux", hc157_device, a0_w)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_WRITE_LINE_DEVICE_MEMBER("buttonmux", hc157_device, a1_w)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_WRITE_LINE_DEVICE_MEMBER("buttonmux", hc157_device, a2_w)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER("buttonmux", hc157_device, b0_w)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_WRITE_LINE_DEVICE_MEMBER("buttonmux", hc157_device, b1_w)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_WRITE_LINE_DEVICE_MEMBER("buttonmux", hc157_device, b2_w)

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_UNKNOWN  )    // tested (sound related?)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_TILT     )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, "Credits For 9-Hole" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0002, 0x0002, "Game Type" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, "Domestic" )
	PORT_DIPSETTING(      0x0000, "Foreign" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0004, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )

	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0400, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x3800, 0x3800, "Flight Distance" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x3800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x3000, "-30 Yards" )
	PORT_DIPSETTING(      0x2800, "+10 Yards" )
	PORT_DIPSETTING(      0x2000, "+20 Yards" )
	PORT_DIPSETTING(      0x1800, "+30 Yards" )
	PORT_DIPSETTING(      0x1000, "+40 Yards" )
	PORT_DIPSETTING(      0x0800, "+50 Yards" )
	PORT_DIPSETTING(      0x0000, "+60 Yards" )
	PORT_DIPNAME( 0xc000, 0xc000, "Licensed To" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, "Romstar" )
	PORT_DIPSETTING(      0x8000, "None (Japan)" )
	PORT_DIPSETTING(      0x4000, "Taito" )
	PORT_DIPSETTING(      0x0000, "Taito America" )
INPUT_PORTS_END



/***************************************************************************

                                Graphics Layouts

Sprites and layers use 16x16 tile, made of four 8x8 tiles. They can be 4
or 6 planes deep and are stored in a wealth of formats.

***************************************************************************/

						// First the 4 bit tiles


// The tilemap bitplanes are packed togheter
static const gfx_layout layout_tilemap =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,4) },
	{ STEP4(4*4*8*3,1), STEP4(4*4*8*2,1), STEP4(4*4*8,1), STEP4(0,1) },
	{ STEP8(0,4*4), STEP8(4*4*8*4,4*4) },
	16*16*4
};


// The sprite bitplanes are separated (but there are 2 per rom)
static const gfx_layout layout_sprites =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 8, RGN_FRAC(1,2) + 0, 8, 0 },
	{ STEP8(0,1), STEP8(8*2*8,1) },
	{ STEP8(0,8*2), STEP8(8*2*8*2,8*2) },
	16*16*2
};


						// Then the 6 bit tiles


// The tilemap bitplanes are packed together
static const gfx_layout layout_tilemap_6bpp =
{
	16,16,
	RGN_FRAC(1,1),
	6,
	{ STEP4(0,4), STEP2(4*4,4) },
	{ STEP4(6*4*8*3,1), STEP4(6*4*8*2,1), STEP4(6*4*8,1), STEP4(0,1) },
	{ STEP8(0,6*4), STEP8(6*4*8*4,6*4) },
	16*16*6
};


/***************************************************************************
                                DownTown
***************************************************************************/

static GFXDECODE_START( gfx_downtown )
	GFXDECODE_ENTRY( "tiles", 0, layout_tilemap, 0, 32 ) // [0] Layer 1
GFXDECODE_END

/***************************************************************************
                                Thundercade
***************************************************************************/

static GFXDECODE_START( gfx_sprites )
	GFXDECODE_ENTRY( "sprites", 0, layout_sprites, 0, 32 ) // Sprites
GFXDECODE_END



/***************************************************************************
                                U.S. Classic
***************************************************************************/

static GFXDECODE_START( gfx_usclssic )
	GFXDECODE_ENTRY( "tiles", 0, layout_tilemap_6bpp, 512+64*32*0, 32 ) // [0] Layer 1
	GFXDECODE_ENTRY( "tiles", 0, layout_tilemap_6bpp, 512+64*32*1, 32 ) // [1] Layer 1
GFXDECODE_END


/***************************************************************************

                                Machine drivers

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(downtown_state::seta_sub_interrupt)
{
	int scanline = param;

	if (scanline == 240)
		m_subcpu->pulse_input_line(m65c02_device::NMI_LINE, attotime::zero);

	if (scanline == 112)
		m_subcpu->set_input_line(m65c02_device::IRQ_LINE, ASSERT_LINE);
}


/***************************************************************************
                                Thundercade
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(tndrcade_state::tndrcade_sub_interrupt)
{
	int scanline = param;

	if (scanline == 240)
		m_subcpu->pulse_input_line(m65c02_device::NMI_LINE, attotime::zero);

	if ((scanline % 16) == 0)
		m_subcpu->set_input_line(m65c02_device::IRQ_LINE, ASSERT_LINE);
}

void tndrcade_state::tndrcade(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &tndrcade_state::tndrcade_map);

	M65C02(config, m_subcpu, 16_MHz_XTAL / 8); // 2 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &tndrcade_state::tndrcade_sub_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(tndrcade_state::tndrcade_sub_interrupt), "screen", 0, 1);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(tndrcade_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (start grid, wall at beginning of game)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tndrcade_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 512);    // sprites only

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ym1(YM2203(config, "ym1", 16_MHz_XTAL / 4));   // 4 MHz
	ym1.port_a_read_callback().set(FUNC(tndrcade_state::dsw1_r));     // input A: DSW 1
	ym1.port_b_read_callback().set(FUNC(tndrcade_state::dsw2_r));     // input B: DSW 2
	ym1.add_route(ALL_OUTPUTS, "mono", 0.35);

	ym3812_device &ym2(YM3812(config, "ym2", 16_MHz_XTAL / 4));   // 4 MHz
	ym2.add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                Twin Eagle
***************************************************************************/

/* Just like metafox, but:
   the sub cpu reads the ip at different locations,
   the visible area seems different. */

// twineagl lev 3 = lev 2 + lev 1 !

void downtown_state::twineagl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &downtown_state::downtown_map);

	M65C02(config, m_subcpu, 16_MHz_XTAL / 8); // 2 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &downtown_state::twineagl_sub_map);
	TIMER(config, "s_scantimer").configure_scanline(FUNC(downtown_state::seta_sub_interrupt), "screen", 0, 1);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(downtown_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.42); // Possibly lower than 60Hz, Correct?
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(downtown_state::screen_update_downtown));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 3, ASSERT_LINE);

	X1_012(config, m_tiles, m_palette, gfx_downtown);
	m_tiles->set_screen(m_screen);
	m_tiles->set_tile_offset_callback(FUNC(downtown_state::twineagl_tile_offset));
	m_tiles->set_xoffsets(-3, 0);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                DownTown
***************************************************************************/

// downtown lev 3 = lev 2 + lev 1 !

void downtown_state::downtown(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &downtown_state::downtown_map);

	M65C02(config, m_subcpu, 16_MHz_XTAL / 8); // verified on pcb
	m_subcpu->set_addrmap(AS_PROGRAM, &downtown_state::downtown_sub_map);
	TIMER(config, "s_scantimer").configure_scanline(FUNC(downtown_state::seta_sub_interrupt), "screen", 0, 1);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(downtown_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 1); // sprites correct (test grid), tilemap unknown but at least -1 non-flipped to fix glitches later in the game
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.42); // verified on pcb
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(downtown_state::screen_update_downtown));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	X1_012(config, m_tiles, m_palette, gfx_downtown);
	m_tiles->set_screen(m_screen);
	m_tiles->set_tile_offset_callback(FUNC(downtown_state::twineagl_tile_offset));
	m_tiles->set_xoffsets(0, -1);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***************************************************************************
                                U.S. Classic
***************************************************************************/


/*  usclssic lev 6 = lev 2+4 !
    Test mode shows a 16ms and 4ms counters. I wonder if every game has
    5 ints per frame
*/

TIMER_DEVICE_CALLBACK_MEMBER(downtown_state::calibr50_interrupt)
{
	int scanline = param;

	if ((scanline % 64) == 0)
		m_maincpu->set_input_line(4, ASSERT_LINE);

	if (scanline == 248)
		m_maincpu->set_input_line(2, ASSERT_LINE);
}


void usclssic_state::machine_start()
{
	downtown_state::machine_start();

	m_buttonmux->ab_w(0xff);

	save_item(NAME(m_port_select));
	save_item(NAME(m_tiles_offset));
}


void usclssic_state::usclssic(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &usclssic_state::usclssic_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(usclssic_state::calibr50_interrupt), "screen", 0, 1);

	WATCHDOG_TIMER(config, "watchdog");

	M65C02(config, m_subcpu, 16_MHz_XTAL / 8); // 2 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &usclssic_state::calibr50_sub_map);

	UPD4701A(config, m_upd4701);
	m_upd4701->set_portx_tag("TRACKX");
	m_upd4701->set_porty_tag("TRACKY");

	HC157(config, m_buttonmux, 0);
	m_buttonmux->out_callback().set(m_upd4701, FUNC(upd4701_device::middle_w)).bit(0);
	m_buttonmux->out_callback().append(m_upd4701, FUNC(upd4701_device::right_w)).bit(1);
	m_buttonmux->out_callback().append(m_upd4701, FUNC(upd4701_device::left_w)).bit(2);

	MCFG_MACHINE_RESET_OVERRIDE(usclssic_state,calibr50)

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(usclssic_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(2, 1); // correct (test grid and bg)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(usclssic_state::screen_update_usclssic));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_subcpu, m65c02_device::IRQ_LINE, ASSERT_LINE);

	X1_012(config, m_tiles, m_palette, gfx_usclssic);
	m_tiles->set_screen(m_screen);
	m_tiles->set_tile_offset_callback(FUNC(usclssic_state::tile_offset));
	m_tiles->set_xoffsets(-1, 0);

	PALETTE(config, m_palette, FUNC(usclssic_state::usclssic_palette), 16*32 + 64*32*2, 0x400); // sprites, layer - layer is 6 planes deep

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_subcpu, m65c02_device::NMI_LINE);
	m_soundlatch[0]->set_separate_acknowledge(true);

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                Caliber 50
***************************************************************************/

/*  calibr50 lev 6 = lev 2 + lev 4 !
             lev 3 = lev 2 + lev 1 !
    Test mode shows a 16ms and 4ms counters. I wonder if every game has
    5 ints per frame */

void downtown_state::calibr50(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &downtown_state::calibr50_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(downtown_state::calibr50_interrupt), "screen", 0, 1);

	WATCHDOG_TIMER(config, "watchdog");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	M65C02(config, m_subcpu, 16_MHz_XTAL / 8); // verified on pcb
	m_subcpu->set_addrmap(AS_PROGRAM, &downtown_state::calibr50_sub_map);
	m_subcpu->set_periodic_int(FUNC(downtown_state::irq0_line_assert), attotime::from_hz(4*60));  // IRQ: 4/frame

	upd4701_device &upd4701(UPD4701A(config, "upd4701"));
	upd4701.set_portx_tag("ROT1");
	upd4701.set_porty_tag("ROT2");

	MCFG_MACHINE_RESET_OVERRIDE(downtown_state,calibr50)

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(downtown_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(2, -1); // correct (test grid and roof in animation at beginning of game)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.42); // verified on pcb
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(downtown_state::screen_update_downtown));
	screen.set_palette(m_palette);
	//screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);

	X1_012(config, m_tiles, m_palette, gfx_downtown);
	m_tiles->set_screen(m_screen);
	m_tiles->set_xoffsets(-2, -3);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_subcpu, m65c02_device::NMI_LINE);
	m_soundlatch[0]->set_separate_acknowledge(true);

	GENERIC_LATCH_8(config, m_soundlatch[1]);

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***************************************************************************
                                Meta Fox
***************************************************************************/

// metafox lev 3 = lev 2 + lev 1 !

void downtown_state::metafox(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000/2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &downtown_state::downtown_map);

	M65C02(config, m_subcpu, 16000000/8); // 2 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &downtown_state::metafox_sub_map);
	TIMER(config, "s_scantimer").configure_scanline(FUNC(downtown_state::seta_sub_interrupt), "screen", 0, 1);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(downtown_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // sprites unknown, tilemap correct (test grid)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(downtown_state::screen_update_downtown));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 3, ASSERT_LINE);

	X1_012(config, m_tiles, m_palette, gfx_downtown);
	m_tiles->set_screen(m_screen);
	m_tiles->set_tile_offset_callback(FUNC(downtown_state::twineagl_tile_offset));
	m_tiles->set_xoffsets(-19, 16);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void downtown_state::arbalest(machine_config &config)
{
	metafox(config);

	m_spritegen->set_fg_xoffsets(1, 0); // correct (test grid and landing pad at beginning of game)
	m_tiles->set_xoffsets(-1, -2);
}


/***************************************************************************

                                ROMs Loading

***************************************************************************/

// used for 6bpp gfxs
#define ROM_LOAD24_BYTE(name, offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(2))

ROM_START( tndrcade )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ua0-4.u19", 0x000000, 0x020000, CRC(73bd63eb) SHA1(5d410d2a77f1c3c4c37a9fe1e56019335891fe67) )
	ROM_LOAD16_BYTE( "ua0-2.u17", 0x000001, 0x020000, CRC(e96194b1) SHA1(c5084d06a2e4f7ba3112be1ccc314f7d712bb45e) )
	ROM_LOAD16_BYTE( "ua0-3.u18", 0x040000, 0x020000, CRC(0a7b1c41) SHA1(ede14ac08d7e63972c21fd2d0717276e73153f18) )
	ROM_LOAD16_BYTE( "ua0-1.u16", 0x040001, 0x020000, CRC(fa906626) SHA1(a1d28328afa8dda98dd20f3f5a19c0dbf2ebaf36) )

	ROM_REGION( 0x02c000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "ua10-5.u24", 0x004000, 0x020000, CRC(8eff6122) SHA1(1adc1643018e612df85643014b78525106478889) )  // $1fffd=2 (country code)
	ROM_RELOAD(             0x00c000, 0x020000  )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "ua0-10.u12", 0x000000, 0x040000, CRC(aa7b6757) SHA1(9157cc930760c846cce95e18bf38e7ea241f7a8e) )
	ROM_LOAD( "ua0-11.u13", 0x040000, 0x040000, CRC(11eaf931) SHA1(ba1dfc4b0f87b1bbdc6c2e36deaecda2b4655d57) )
	ROM_LOAD( "ua0-12.u14", 0x080000, 0x040000, CRC(00b5381c) SHA1(6fc3138dd0e2b3f99872b1f0d177094df5bed39d) )
	ROM_LOAD( "ua0-13.u15", 0x0c0000, 0x040000, CRC(8f9a0ed3) SHA1(61315312fdb2fe090cd8e99a1ce3ecba46b466e9) )
	ROM_LOAD( "ua0-6.u8",   0x100000, 0x040000, CRC(14ecc7bb) SHA1(920983f5086462f8f06dc9cf7bebffeeb7187977) )
	ROM_LOAD( "ua0-7.u9",   0x140000, 0x040000, CRC(ff1a4e68) SHA1(d732df7d139995814969a603c9c4e9f8b068b1a3) )
	ROM_LOAD( "ua0-8.u10",  0x180000, 0x040000, CRC(936e1884) SHA1(9ad495b88e124d08a7141611ed1897b6e2abd412) )
	ROM_LOAD( "ua0-9.u11",  0x1c0000, 0x040000, CRC(e812371c) SHA1(b0e1e0c143da743bf9f7b48d657594e76f4970ed) )
ROM_END

ROM_START( tndrcadej )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ua0-4.u19", 0x000000, 0x020000, CRC(73bd63eb) SHA1(5d410d2a77f1c3c4c37a9fe1e56019335891fe67) )
	ROM_LOAD16_BYTE( "ua0-2.u17", 0x000001, 0x020000, CRC(e96194b1) SHA1(c5084d06a2e4f7ba3112be1ccc314f7d712bb45e) )
	ROM_LOAD16_BYTE( "ua0-3.u18", 0x040000, 0x020000, CRC(0a7b1c41) SHA1(ede14ac08d7e63972c21fd2d0717276e73153f18) )
	ROM_LOAD16_BYTE( "ua0-1.u16", 0x040001, 0x020000, CRC(fa906626) SHA1(a1d28328afa8dda98dd20f3f5a19c0dbf2ebaf36) )

	ROM_REGION( 0x02c000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "thcade5.u24", 0x004000, 0x020000, CRC(8cb9df7b) SHA1(5b504657f4cc1ea265913ff670aed108ceddba46) ) // $1fffd=1 (country code jp)
	ROM_RELOAD(              0x00c000, 0x020000  )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "ua0-10.u12", 0x000000, 0x040000, CRC(aa7b6757) SHA1(9157cc930760c846cce95e18bf38e7ea241f7a8e) )
	ROM_LOAD( "ua0-11.u13", 0x040000, 0x040000, CRC(11eaf931) SHA1(ba1dfc4b0f87b1bbdc6c2e36deaecda2b4655d57) )
	ROM_LOAD( "ua0-12.u14", 0x080000, 0x040000, CRC(00b5381c) SHA1(6fc3138dd0e2b3f99872b1f0d177094df5bed39d) )
	ROM_LOAD( "ua0-13.u15", 0x0c0000, 0x040000, CRC(8f9a0ed3) SHA1(61315312fdb2fe090cd8e99a1ce3ecba46b466e9) )
	ROM_LOAD( "ua0-6.u8",   0x100000, 0x040000, CRC(14ecc7bb) SHA1(920983f5086462f8f06dc9cf7bebffeeb7187977) )
	ROM_LOAD( "ua0-7.u9",   0x140000, 0x040000, CRC(ff1a4e68) SHA1(d732df7d139995814969a603c9c4e9f8b068b1a3) )
	ROM_LOAD( "ua0-8.u10",  0x180000, 0x040000, CRC(936e1884) SHA1(9ad495b88e124d08a7141611ed1897b6e2abd412) )
	ROM_LOAD( "ua0-9.u11",  0x1c0000, 0x040000, CRC(e812371c) SHA1(b0e1e0c143da743bf9f7b48d657594e76f4970ed) )
ROM_END

ROM_START( twineagl )
	ROM_REGION( 0x0a0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD( "ua2-1", 0x000000, 0x080000, CRC(5c3fe531) SHA1(e484dad25cda906fb1b0606fb10ae50056c64e6a) )

	ROM_REGION( 0x010000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "ua2-2", 0x006000, 0x002000, CRC(783ca84e) SHA1(21e19f74812de50e98b755dd1f68c187dd1e7e81) )
	ROM_RELOAD(        0x008000, 0x002000  )
	ROM_RELOAD(        0x00a000, 0x002000  )
	ROM_RELOAD(        0x00c000, 0x002000  )
	ROM_RELOAD(        0x00e000, 0x002000  )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ua2-4",  0x000000, 0x040000, CRC(8b7532d6) SHA1(ec42d21bc44f004282f822b3da36b5442eabd87a) )
	ROM_LOAD16_BYTE( "ua2-3",  0x000001, 0x040000, CRC(1124417a) SHA1(c908f51b943188946486c639a0cb9712114b5437) )
	ROM_LOAD16_BYTE( "ua2-6",  0x080000, 0x040000, CRC(99d8dbba) SHA1(ac2a3c5cad23e0207eba52935c72e23203c8e0af) )
	ROM_LOAD16_BYTE( "ua2-5",  0x080001, 0x040000, CRC(6e450d28) SHA1(d0050afcc3f425ac70768271c9d2d55ab7fba622) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "ua2-7",  0x000001, 0x080000, CRC(fce56907) SHA1(5d0d2d6dfdbadb21f1d61d84b8992ec0e527e18d) )
	ROM_LOAD16_BYTE( "ua2-8",  0x000000, 0x080000, CRC(7d3a8d73) SHA1(d6a0bea124d7d228818f8ea8c804ad2ba8cead4b) )
	ROM_LOAD16_BYTE( "ua2-9",  0x100001, 0x080000, CRC(a451eae9) SHA1(c236c92d9ecf56f8d8f4a5ee493e3791be0d3db4) )
	ROM_LOAD16_BYTE( "ua2-10", 0x100000, 0x080000, CRC(5bbe1f56) SHA1(309bc43884816dafeb0f47e71ff5272d4d7cac54) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ua2-11", 0x000000, 0x080000, CRC(624e6057) SHA1(0e8e4d4b6bc5febf5ca83eea92e3ed06f16e7df0) )
	ROM_LOAD( "ua2-12", 0x080000, 0x080000, CRC(3068ff64) SHA1(7c06a48a99ebb9e7f3709f25bd0caa4c9d7a2688) )
ROM_END

ROM_START( downtown )
	ROM_REGION( 0x0a0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ud2-001-000.3c",  0x000000, 0x040000, CRC(f1965260) SHA1(c0560342238d75f9b81ae9f3408cacfbcd331529) ) // 40 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-003.11c", 0x000001, 0x040000, CRC(e7d5fa5f) SHA1(48612514598711aa73bf75243c842f0aca72f3d0) ) // 40 pin MASK rom
	ROM_LOAD16_BYTE( "ud2001002.9b", 0x080000, 0x010000, CRC(a300e3ac) SHA1(958cb121787444cdc6938fc5aad1e92238e39c13) )
	ROM_LOAD16_BYTE( "ud2001001.8b", 0x080001, 0x010000, CRC(d2918094) SHA1(c135939ad12e3cf0688db148c49f99e757ad7b0d) )

	ROM_REGION( 0x04c000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "ud2-002-004.17c", 0x004000, 0x040000, CRC(bbd538b1) SHA1(de4c43bfc4004a14f9f66b5e8ff192b00c45c003) ) // 40 pin MASK rom
	ROM_RELOAD(                  0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ud2-001-005-t01.2n", 0x000000, 0x080000, CRC(77e6d249) SHA1(cdf67211cd447858293188511e826640fe24078b) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-006-t02.3n", 0x000001, 0x080000, CRC(6e381bf2) SHA1(ba46e019d2991dec539444ef7376fe0e9a6a8b75) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-007-t03.5n", 0x100000, 0x080000, CRC(737b4971) SHA1(2a034011b0ac03d532a89b544f4eec497ac7ee80) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-008-t04.6n", 0x100001, 0x080000, CRC(99b9d757) SHA1(c3a763993305110ec2a0b231d75fbef4c385d21b) ) // 32 pin MASK rom

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "ud2-001-009-t05.8n", 0x000000, 0x080000, CRC(aee6c581) SHA1(5b2150a308ca12eea8148d0bbff663b3baf0c831) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-010-t06.9n", 0x000001, 0x080000, CRC(3d399d54) SHA1(7d9036e73fbf0e9c3b976336e3e4786b17b2f4fc) ) // 32 pin MASK rom

	ROM_REGION( 0x080000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ud2-001-011-t07.14n", 0x000000, 0x080000, CRC(9c9ff69f) SHA1(3840b654f4f709bc4c03dfe4ee79369d5c70dd62) ) // 32 pin MASK rom
ROM_END

ROM_START( downtown2 )
	ROM_REGION( 0x0a0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ud2-001-000.3c",  0x000000, 0x040000, CRC(f1965260) SHA1(c0560342238d75f9b81ae9f3408cacfbcd331529) ) // 40 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-003.11c", 0x000001, 0x040000, CRC(e7d5fa5f) SHA1(48612514598711aa73bf75243c842f0aca72f3d0) ) // 40 pin MASK rom
	ROM_LOAD16_BYTE( "ud2000002.9b",    0x080000, 0x010000, CRC(ca976b24) SHA1(3b2e362f414b0103dd02c9af6a5d480ec2cf9ca3) )
	ROM_LOAD16_BYTE( "ud2000001.8b",    0x080001, 0x010000, CRC(1708aebd) SHA1(337a9e8d5da5b13a7ea4ee728de6b82fe92e16c5) )

	ROM_REGION( 0x04c000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "ud2-002-004.17c", 0x004000, 0x040000, CRC(bbd538b1) SHA1(de4c43bfc4004a14f9f66b5e8ff192b00c45c003) ) // 40 pin MASK rom
	ROM_RELOAD(                  0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ud2-001-005-t01.2n", 0x000000, 0x080000, CRC(77e6d249) SHA1(cdf67211cd447858293188511e826640fe24078b) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-006-t02.3n", 0x000001, 0x080000, CRC(6e381bf2) SHA1(ba46e019d2991dec539444ef7376fe0e9a6a8b75) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-007-t03.5n", 0x100000, 0x080000, CRC(737b4971) SHA1(2a034011b0ac03d532a89b544f4eec497ac7ee80) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-008-t04.6n", 0x100001, 0x080000, CRC(99b9d757) SHA1(c3a763993305110ec2a0b231d75fbef4c385d21b) ) // 32 pin MASK rom

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "ud2-001-009-t05.8n", 0x000000, 0x080000, CRC(aee6c581) SHA1(5b2150a308ca12eea8148d0bbff663b3baf0c831) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-010-t06.9n", 0x000001, 0x080000, CRC(3d399d54) SHA1(7d9036e73fbf0e9c3b976336e3e4786b17b2f4fc) ) // 32 pin MASK rom

	ROM_REGION( 0x080000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ud2-001-011-t07.14n", 0x000000, 0x080000, CRC(9c9ff69f) SHA1(3840b654f4f709bc4c03dfe4ee79369d5c70dd62) ) // 32 pin MASK rom
ROM_END

ROM_START( downtownj )
	ROM_REGION( 0x0a0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ud2-001-000.3c",  0x000000, 0x040000, CRC(f1965260) SHA1(c0560342238d75f9b81ae9f3408cacfbcd331529) ) // 40 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-003.11c", 0x000001, 0x040000, CRC(e7d5fa5f) SHA1(48612514598711aa73bf75243c842f0aca72f3d0) ) // 40 pin MASK rom
	ROM_LOAD16_BYTE( "u37.9b",          0x080000, 0x010000, CRC(73047657) SHA1(731663101d809170aad3cd39e901ef494494c5a1) )
	ROM_LOAD16_BYTE( "u31.8b",          0x080001, 0x010000, CRC(6a050240) SHA1(6a1a305b7d32bb2ad17842b4eeabc891fce02160) )

	ROM_REGION( 0x04c000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "ud2-002-004.17c", 0x004000, 0x040000, CRC(bbd538b1) SHA1(de4c43bfc4004a14f9f66b5e8ff192b00c45c003) ) // 40 pin MASK rom
	ROM_RELOAD(                  0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ud2-001-005-t01.2n", 0x000000, 0x080000, CRC(77e6d249) SHA1(cdf67211cd447858293188511e826640fe24078b) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-006-t02.3n", 0x000001, 0x080000, CRC(6e381bf2) SHA1(ba46e019d2991dec539444ef7376fe0e9a6a8b75) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-007-t03.5n", 0x100000, 0x080000, CRC(737b4971) SHA1(2a034011b0ac03d532a89b544f4eec497ac7ee80) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-008-t04.6n", 0x100001, 0x080000, CRC(99b9d757) SHA1(c3a763993305110ec2a0b231d75fbef4c385d21b) ) // 32 pin MASK rom

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "ud2-001-009-t05.8n", 0x000000, 0x080000, CRC(aee6c581) SHA1(5b2150a308ca12eea8148d0bbff663b3baf0c831) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-010-t06.9n", 0x000001, 0x080000, CRC(3d399d54) SHA1(7d9036e73fbf0e9c3b976336e3e4786b17b2f4fc) ) // 32 pin MASK rom

	ROM_REGION( 0x080000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ud2-001-011-t07.14n", 0x000000, 0x080000, CRC(9c9ff69f) SHA1(3840b654f4f709bc4c03dfe4ee79369d5c70dd62) ) // 32 pin MASK rom
ROM_END

ROM_START( downtownp )
	ROM_REGION( 0x0a0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ud2-001-000.3c",   0x000000, 0x040000, CRC(f1965260) SHA1(c0560342238d75f9b81ae9f3408cacfbcd331529) ) // 40 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-003.11c",  0x000001, 0x040000, CRC(e7d5fa5f) SHA1(48612514598711aa73bf75243c842f0aca72f3d0) ) // 40 pin MASK rom
	ROM_LOAD16_BYTE( "ud2_even_v061.9b", 0x080000, 0x010000, CRC(251d6552) SHA1(0f78bf142db826e956f670ba81102804e88fa2ed) ) // handwritten label UD2 EVEN V0.61 2/13
	ROM_LOAD16_BYTE( "ud2_odd_v061.8b",  0x080001, 0x010000, CRC(6394a7c0) SHA1(9f5099b32b3c3e100441f6c0ccbe88c19b01a9e5) ) // handwritten label UD2 ODD V0.61 2/13

	ROM_REGION( 0x04c000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "ud2-002-004.17c", 0x004000, 0x040000, CRC(bbd538b1) SHA1(de4c43bfc4004a14f9f66b5e8ff192b00c45c003) ) // 40 pin MASK rom
	ROM_RELOAD(                  0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ud2-001-005-t01.2n", 0x000000, 0x080000, CRC(77e6d249) SHA1(cdf67211cd447858293188511e826640fe24078b) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-006-t02.3n", 0x000001, 0x080000, CRC(6e381bf2) SHA1(ba46e019d2991dec539444ef7376fe0e9a6a8b75) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-007-t03.5n", 0x100000, 0x080000, CRC(737b4971) SHA1(2a034011b0ac03d532a89b544f4eec497ac7ee80) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-008-t04.6n", 0x100001, 0x080000, CRC(99b9d757) SHA1(c3a763993305110ec2a0b231d75fbef4c385d21b) ) // 32 pin MASK rom

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "ud2-001-009-t05.8n", 0x000000, 0x080000, CRC(aee6c581) SHA1(5b2150a308ca12eea8148d0bbff663b3baf0c831) ) // 32 pin MASK rom
	ROM_LOAD16_BYTE( "ud2-001-010-t06.9n", 0x000001, 0x080000, CRC(3d399d54) SHA1(7d9036e73fbf0e9c3b976336e3e4786b17b2f4fc) ) // 32 pin MASK rom

	ROM_REGION( 0x080000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ud2-001-011-t07.14n", 0x000000, 0x080000, CRC(9c9ff69f) SHA1(3840b654f4f709bc4c03dfe4ee79369d5c70dd62) ) // 32 pin MASK rom
ROM_END

ROM_START( usclssic )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ue2001.u20", 0x000000, 0x020000, CRC(18b41421) SHA1(74e96071d46eda152aaa82cf87d09203f225b504) )
	ROM_LOAD16_BYTE( "ue2000.u14", 0x000001, 0x020000, CRC(69454bc2) SHA1(19a3b6ca65770353401544c50e04d895e391612c) )
	ROM_LOAD16_BYTE( "ue2002.u22", 0x040000, 0x020000, CRC(a7bbe248) SHA1(8f7ffeffb8b6ef0e1ab5e7fbba31a1b97bbd7f8c) )
	ROM_LOAD16_BYTE( "ue2003.u30", 0x040001, 0x020000, CRC(29601906) SHA1(9cdf2d80a72317a4eb7a335aaaae381822da24b1) )

	ROM_REGION( 0x04c000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "ue002u61.004", 0x004000, 0x040000, CRC(476e9f60) SHA1(940c09eb472652a88d5d34130270ff55a5f5ba27) )
	ROM_RELOAD(               0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ue001009.119", 0x000000, 0x080000, CRC(dc065204) SHA1(0478b8126cd3ce3dee64cb7de2b30b509636eb1a) )
	ROM_LOAD16_BYTE( "ue001008.118", 0x000001, 0x080000, CRC(5947d9b5) SHA1(de3a63c55b558451bbbe98bf8d71561ba32c5e60) )
	ROM_LOAD16_BYTE( "ue001007.117", 0x100000, 0x080000, CRC(b48a885c) SHA1(8c0d458d6967c2ff4bdcf37aaa8025341fe90bbc) )
	ROM_LOAD16_BYTE( "ue001006.116", 0x100001, 0x080000, CRC(a6ab6ef4) SHA1(9f54f116d1d8e54d64ba541195baa66c5ca960bd) )

	ROM_REGION( 0x600000, "tiles", 0 )
	ROM_LOAD24_BYTE( "ue001010.120", 0x000000, 0x080000, CRC(dd683031) SHA1(06ed38a243666c1acaf8eb3fdba51d18fc2a70bc) )    // planes 01
	ROM_LOAD24_BYTE( "ue001011.121", 0x180000, 0x080000, CRC(0e27bc49) SHA1(f9ec4f4c15c86f608607a5ec916f5182a8e265fa) )
	ROM_LOAD24_BYTE( "ue001012.122", 0x300000, 0x080000, CRC(961dfcdc) SHA1(9de95692860abd4206db22ad7ade9f02f0c03506) )
	ROM_LOAD24_BYTE( "ue001013.123", 0x480000, 0x080000, CRC(03e9eb79) SHA1(e7cabfd73b73c7df8d79c113db5eca110bf2f05e) )

	ROM_LOAD24_BYTE( "ue001014.124", 0x000001, 0x080000, CRC(9576ace7) SHA1(a5350934533241daf63c561a88d952bb6976b81b) )    // planes 23
	ROM_LOAD24_BYTE( "ue001015.125", 0x180001, 0x080000, CRC(631d6eb1) SHA1(df342c20e2b3c29eab3c72440c11be60d14d3557) )
	ROM_LOAD24_BYTE( "ue001016.126", 0x300001, 0x080000, CRC(f44a8686) SHA1(649f6f95cc67fa2f4551af19a2b607c811318820) )
	ROM_LOAD24_BYTE( "ue001017.127", 0x480001, 0x080000, CRC(7f568258) SHA1(ac36e87386f9d5c68c66a9469e1b30ee66c4cb7f) )

	ROM_LOAD24_BYTE( "ue001018.128", 0x000002, 0x080000, CRC(4bd98f23) SHA1(be6483253a5ea1efe7c7f6b4432fe819b906894c) )    // planes 45
	ROM_LOAD24_BYTE( "ue001019.129", 0x180002, 0x080000, CRC(6d9f5a33) SHA1(8d300adf2b3299df78e274c4c7f2ee2d8e1e2575) )
	ROM_LOAD24_BYTE( "ue001020.130", 0x300002, 0x080000, CRC(bc07403f) SHA1(f994b6d1dee23f5dabdb328f955f4380a8ca9d52) )
	ROM_LOAD24_BYTE( "ue001021.131", 0x480002, 0x080000, CRC(98c03efd) SHA1(761c51d5573e6f35c48b8b9ee5d88cbde02e92a7) )

	ROM_REGION( 0x400, "proms", 0 ) // Extra Colours
	ROM_LOAD16_BYTE( "ue1-022.prm", 0x000, 0x200, CRC(1a23129e) SHA1(110eb54ab83ecb8375164a5c96f522b2737c379c) )
	ROM_LOAD16_BYTE( "ue1-023.prm", 0x001, 0x200, CRC(a13192a4) SHA1(86e312e0f7400b7fa08fbe8fced1eb95a32502ca) )

	ROM_REGION( 0x080000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ue001005.132", 0x000000, 0x080000, CRC(c5fea37c) SHA1(af4f09dd36af06e50262f607ff14eedc33beffd2) )
ROM_END

ROM_START( calibr50 )
	ROM_REGION( 0x0a0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "uh-002-001.3b",  0x000000, 0x040000, CRC(eb92e7ed) SHA1(2aee8a7bce549ef7d7b35d1c248ebbdbc906e38d) )
	ROM_LOAD16_BYTE( "uh-002-004.11b", 0x000001, 0x040000, CRC(5a0ed31e) SHA1(d6ee7654354ac9f1dc7add1ef9f68a147b6f2953) )
	ROM_LOAD16_BYTE( "uh_001_003.9b",  0x080000, 0x010000, CRC(0d30d09f) SHA1(8a48511b628e85b72fda0968d813f4faebd0c418) )
	ROM_LOAD16_BYTE( "uh_001_002.7b",  0x080001, 0x010000, CRC(7aecc3f9) SHA1(2454d9c758fa623d4d81a9230871b67d31d16cef) )

	ROM_REGION( 0x04c000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "uh-001-005.17e", 0x004000, 0x040000, CRC(4a54c085) SHA1(f53ff257ce3d95f945a6befcfb61f1b570f0eafe) )
	ROM_RELOAD(                 0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "uh-001-006.2m", 0x000000, 0x080000, CRC(fff52f91) SHA1(fd7807e9a8dd5a88df1fcd13746b44a33adbc0fa) )
	ROM_LOAD16_BYTE( "uh-001-007.4m", 0x000001, 0x080000, CRC(b6c19f71) SHA1(eb8bbaeaf4af07e178100ff16b228b537aa36272) )
	ROM_LOAD16_BYTE( "uh-001-008.5m", 0x100000, 0x080000, CRC(7aae07ef) SHA1(1db666db20efce1efe5b5769b8e3c78bbf508466) )
	ROM_LOAD16_BYTE( "uh-001-009.6m", 0x100001, 0x080000, CRC(f85da2c5) SHA1(d090e49b3a897729c7fb05f9386939448fe1d3d9) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "uh-001-010.8m", 0x000000, 0x080000, CRC(f986577a) SHA1(8f6c2fca271fed21a1c04e93c3f50dc41348ae30) )
	ROM_LOAD16_BYTE( "uh-001-011.9m", 0x000001, 0x080000, CRC(08620052) SHA1(e2ab49dbabc139e6b276401340085ccab1ae3892) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "uh-001-013.12m", 0x000000, 0x080000, CRC(09ec0df6) SHA1(57c68d05074ea4a1e133be2ce6e25c594f04a712) )
	ROM_LOAD( "uh-001-012.11m", 0x080000, 0x080000, CRC(bb996547) SHA1(0c8f570ef4454b10a023e0c463001c22a8cf99cd) )
ROM_END

ROM_START( arbalest )
	ROM_REGION( 0x0a0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "uk-001-003",  0x000000, 0x040000, CRC(ee878a2c) SHA1(f7d5817015382ce6af317c02746b473ec798bb4f) ) // Mask ROM on P1-037A sub PCB
	ROM_LOAD16_BYTE( "uk-001-004",  0x000001, 0x040000, CRC(902bb4e3) SHA1(e37a361a7c03aee2d6ac8c96c2dd6c1e411b46fb) ) // Mask ROM on P1-037A sub PCB

	ROM_REGION( 0x010000, "sub", 0 )        // 65c02 Code
	// Label is correct, 1st & 2nd halves identical is correct. Chip is a 27128 - Verified on 2 different PCBs
	ROM_LOAD( "uk6005", 0x006000, 0x004000, CRC(48c73a4a) SHA1(1284ae7236a82a5898a57ec0451b7dcc4d409099) ) // EPROM on P1-037A sub PCB
	ROM_RELOAD(         0x00a000, 0x004000  )
	ROM_RELOAD(         0x00e000, 0x002000  )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "uk001.06", 0x000000, 0x040000, CRC(11c75746) SHA1(7faf9a26534397d21211d5ef25ca53c4eb286474) )
	ROM_LOAD16_BYTE( "uk001.07", 0x000001, 0x040000, CRC(01b166c7) SHA1(d1b5b73a55025a264a22dd950ea79ba8172c4bed) )
	ROM_LOAD16_BYTE( "uk001.08", 0x080000, 0x040000, CRC(78d60ba3) SHA1(c4fa546e4ca637d67ecc1b085b91c753606ccdb3) )
	ROM_LOAD16_BYTE( "uk001.09", 0x080001, 0x040000, CRC(b4748ae0) SHA1(a71e671754ed5bba737f0b5f7be510a23d5e925c) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "uk-001-010-t26", 0x000000, 0x080000, CRC(c1e2f823) SHA1(892473351e7b590c59c578047a67fc235bd31e02) ) // Mask ROM on P1-036A sub PCB
	ROM_LOAD16_BYTE( "uk-001-011-t27", 0x100000, 0x080000, CRC(09dfe56a) SHA1(077627627d3cb8f79ffdd83e46157bd3c473c4a1) ) // Mask ROM on P1-036A sub PCB
	ROM_LOAD16_BYTE( "uk-001-012-t28", 0x000001, 0x080000, CRC(818a4085) SHA1(fd8b5658fc7f5fa6d3daebb4be17aeabd60c9028) ) // Mask ROM on P1-036A sub PCB
	ROM_LOAD16_BYTE( "uk-001-013-t29", 0x100001, 0x080000, CRC(771fa164) SHA1(a91214318808f991846a828f0e309c0ff430245e) ) // Mask ROM on P1-036A sub PCB

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "uk-001-015-t31", 0x000000, 0x080000, CRC(ce9df5dd) SHA1(91d879b774b5b367adb5bd511fda827bc0bae0a9) ) // Mask ROM on P1-036A sub PCB
	ROM_LOAD( "uk-001-014-t30", 0x080000, 0x080000, CRC(016b844a) SHA1(1fe091233746ced358292014393896af730f5940) ) // Mask ROM on P1-036A sub PCB
ROM_END

ROM_START( metafox )
	ROM_REGION( 0x0a0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "p1003161", 0x000000, 0x040000, CRC(4fd6e6a1) SHA1(11a830d76ef737bcfac73d0958425fe4329f0dcd) )
	ROM_LOAD16_BYTE( "p1004162", 0x000001, 0x040000, CRC(b6356c9a) SHA1(182a1ea9f0643b05b14ad2a2cd820f5ca2086c4c) )
	ROM_LOAD16_BYTE( "up001002", 0x080000, 0x010000, CRC(ce91c987) SHA1(63546fa1342371a7080ac3cf59b41a01ac313c8c) )
	ROM_LOAD16_BYTE( "up001001", 0x080001, 0x010000, CRC(0db7a505) SHA1(d593da2f7d8b54724cae017cbabc3c0909893da1) )

	ROM_REGION( 0x010000, "sub", 0 )        // 65c02 Code
	ROM_LOAD( "up001005", 0x006000, 0x002000, CRC(2ac5e3e3) SHA1(b794554cd25bdd48a21a0a2861daf8369e798ce8) )
	ROM_RELOAD(           0x008000, 0x002000  )
	ROM_RELOAD(           0x00a000, 0x002000  )
	ROM_RELOAD(           0x00c000, 0x002000  )
	ROM_RELOAD(           0x00e000, 0x002000  )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "p1006163", 0x000000, 0x040000, CRC(80f69c7c) SHA1(df323e801ebec6316ba17fe0371f7c87fad19295) )
	ROM_LOAD16_BYTE( "p1007164", 0x000001, 0x040000, CRC(d137e1a3) SHA1(0e0234f1d0adb7db6d0508263e3b0b31fe7071b9) )
	ROM_LOAD16_BYTE( "p1008165", 0x080000, 0x040000, CRC(57494f2b) SHA1(28d620254e81d7e63dfed07b29b252b975a81248) )
	ROM_LOAD16_BYTE( "p1009166", 0x080001, 0x040000, CRC(8344afd2) SHA1(7348b423405ad00b9240d152b119cf5341754815) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "up001010", 0x000000, 0x080000, CRC(bfbab472) SHA1(d3e7b20d14de48134c4fbe3da31feb928c1c655b) )
	ROM_LOAD16_BYTE( "up001011", 0x100000, 0x080000, CRC(26cea381) SHA1(b4bfd2a13ef6051376fe3ed57e2331a072970f86) )
	ROM_LOAD16_BYTE( "up001012", 0x000001, 0x080000, CRC(fed2c5f9) SHA1(81f0f19a500b665c937f5431000ebde7abd97c30) )
	ROM_LOAD16_BYTE( "up001013", 0x100001, 0x080000, CRC(adabf9ea) SHA1(db28e4e565e567a97a6b05a4803a55a403e24a0e) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "up001015", 0x000000, 0x080000, CRC(2e20e39f) SHA1(6f8bd4a76ed5c2150015698e7a98044d060157be) )
	ROM_LOAD( "up001014", 0x080000, 0x080000, CRC(fca6315e) SHA1(cef2385ec43f8b7a2d655b42c18ef44e46ff7364) )
ROM_END

u16 downtown_state::twineagl_debug_r()
{
	/*  At several points in the code, the program checks if four
	    consecutive bytes in this range are equal to a string, and if they
	    are, it fetches an address from the following 4 bytes and jumps there.
	    They are probably hooks for debug routines to be found in an extra ROM.

	    0000 "MT00" + jump address
	    0008 "MT01" + jump address
	    0010 "MT02" + jump address
	    0018 "MT03" + jump address
	    0020 "MT04" + jump address
	    0028 "MT05" + jump address
	    0030 "MT06" + jump address
	    0038 "MT07" + jump address
	    0040 "WZ08" + jump address
	    0080 "KB00" + jump address
	    00C0 "MT18" + jump address
	    00C8 "MT19" + jump address
	    00D0 "MT1a" + jump address
	    00D8 "MT1b" + jump address
	    00E0 "MT1c" + jump address
	    00E8 "MT1d" + jump address
	    00F0 "MT1e" + jump address
	    00F8 "MT1f" + jump address
	*/

	return 0;
}

void tndrcade_state::machine_start()
{
	u8 *rom = memregion("sub")->base();
	const u32 max = (memregion("sub")->bytes() - 0xc000) / 0x4000;

	if (max > 1) // if 6502 ROM is bankswitched(size is larger than 0x4000)
	{
		m_subbank->configure_entries(0, max, &rom[0xc000], 0x4000);
		if (max < 16)
			m_subbank->configure_entries(max, 16-max, &rom[0xc000], 0x4000); // Unverified : Bankswitching is Mirrored?
	}
	else
		m_subbank->configure_entries(0, 16, &rom[0xc000], 0); // Not bankswitched : for avoid crashing when accessing bank functions
}

// Extra RAM ? Check code at 0x00ba90
// 2000F8 = A3 enables it, 2000F8 = 00 disables? see downtown too
u16 downtown_state::twineagl_200100_r(offs_t offset)
{
	// protection check at boot
	if (!machine().side_effects_disabled())
		LOGPROT("%04x: twineagl_200100_r %d\n",m_maincpu->pc(), offset);
	return m_twineagl_xram[offset];
}
void downtown_state::twineagl_200100_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGPROT("%04x: twineagl_200100_w %d = %02x\n",m_maincpu->pc(), offset,data);

	if (ACCESSING_BITS_0_7)
	{
		m_twineagl_xram[offset] = data & 0xff;
	}
}

void downtown_state::init_twineagl()
{
	// debug?
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x800000, 0x8000ff, read16smo_delegate(*this, FUNC(downtown_state::twineagl_debug_r)));

	// This allows 2 simultaneous players and the use of the "Copyright" Dip Switch.
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200100, 0x20010f, read16sm_delegate(*this, FUNC(downtown_state::twineagl_200100_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x200100, 0x20010f, write16s_delegate(*this, FUNC(downtown_state::twineagl_200100_w)));
}


// Protection? NVRAM is handled writing commands here
u16 downtown_state::downtown_protection_r(offs_t offset)
{
	const int job = m_downtown_protection[0xf8/2] & 0xff;

	switch (job)
	{
		case 0xa3:
		{
			static const u8 word[] = "WALTZ0";
			if (offset >= 0x100/2 && offset <= 0x10a/2)
				return word[offset - 0x100/2];

			// definitely wants to read-back hi-score table from 0x110-0x15f
			break;
		}
	}

	return m_downtown_protection[offset] & 0xff;
}

void downtown_state::downtown_protection_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_downtown_protection[offset]);
}

void downtown_state::init_downtown()
{
	m_downtown_protection = make_unique_clear<u8[]>(0x100);
	// TODO: protection RAM area is user clearable from service mode, most likely has some sort of battery backed RAM
	// initializing with 0xff is enough for host CPU to catch up and default it with sensible defaults.
	for (int i = 0; i < 0x100; i++)
		m_downtown_protection[i] = 0xff;
	save_pointer(NAME(m_downtown_protection), 0x100);

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000, 0x2001ff, read16sm_delegate(*this, FUNC(downtown_state::downtown_protection_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x200000, 0x2001ff, write16s_delegate(*this, FUNC(downtown_state::downtown_protection_w)));
}


u16 downtown_state::arbalest_debug_r()
{
	/*  At some points in the code, the program checks if four
	    consecutive bytes in this range are equal to a string, and if they
	    are, it fetches an address from the following 4 bytes and jumps there.
	    They are probably hooks for debug routines to be found in an extra ROM.

	    0000 "CHK1" + jump address
	    0008 "CHK2" + jump address
	*/

	return 0;
}

void downtown_state::init_arbalest()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80000, 0x8000f, read16smo_delegate(*this, FUNC(downtown_state::arbalest_debug_r)));
}


u16 downtown_state::metafox_protection_r(offs_t offset)
{
	// very simplified protection simulation
	// 21c000-21c3ff, 21d000-21d3ff, and 21e000-21e3ff are tested as 8 bit reads/writes
	// the first address in each range is special and returns data written elsewhere in that range
	// 21fde0-21fdff appears to be control bytes?

	// Protection device (likely X1-017) is a surface-scratched SDIP64 on the P1-049-A sub PCB

	switch (offset)
	{
		case 0x0001/2:
			return 0x3d;

		case 0x1001/2:
			return 0x76;

		case 0x2001/2:
			return 0x10;
	}

	return offset * 0x1f;
}

void downtown_state::init_metafox()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x21c000, 0x21ffff,read16sm_delegate(*this, FUNC(downtown_state::metafox_protection_r)));
}

} // anonymous namespace


/***************************************************************************

                                Game Drivers

***************************************************************************/

GAME( 1987, tndrcade,  0,        tndrcade,  tndrcade,  tndrcade_state, empty_init,     ROT270, "Seta (Taito license)",      "Thundercade / Twin Formation" , 0) // Title/License: DSW
GAME( 1987, tndrcadej, tndrcade, tndrcade,  tndrcadj,  tndrcade_state, empty_init,     ROT270, "Seta (Taito license)",      "Tokusyu Butai U.A.G. (Japan)" , 0) // License: DSW

GAME( 1988, twineagl,  0,        twineagl,  twineagl,  downtown_state, init_twineagl,  ROT270, "Seta (Taito license)",      "Twin Eagle - Revenge Joe's Brother" , 0) // Country/License: DSW

GAME( 1989, downtown,  0,        downtown,  downtown,  downtown_state, init_downtown,  ROT270, "Seta",                      "DownTown / Mokugeki (set 1)" , 0) // Country/License: DSW
GAME( 1989, downtown2, downtown, downtown,  downtown,  downtown_state, init_downtown,  ROT270, "Seta",                      "DownTown / Mokugeki (set 2)" , 0) // Country/License: DSW
GAME( 1989, downtownj, downtown, downtown,  downtown,  downtown_state, init_downtown,  ROT270, "Seta",                      "DownTown / Mokugeki (joystick hack)" , 0) // Country/License: DSW
GAME( 1989, downtownp, downtown, downtown,  downtown,  downtown_state, init_downtown,  ROT270, "Seta",                      "DownTown / Mokugeki (prototype)" , 0) // Country/License: DSW

GAME( 1989, usclssic,  0,        usclssic,  usclssic,  usclssic_state, empty_init,     ROT270, "Seta",                      "U.S. Classic" , 0) // Country/License: DSW

GAME( 1989, calibr50,  0,        calibr50,  calibr50,  downtown_state, empty_init,     ROT270, "Athena / Seta",             "Caliber 50 (Ver. 1.01)" , 0) // Country/License: DSW

GAME( 1989, arbalest,  0,        arbalest,  arbalest,  downtown_state, init_arbalest,  ROT270, "Jordan I.S. / Seta",        "Arbalester" , 0) // Developed by Jordan for Seta, Country/License: DSW

GAME( 1989, metafox,   0,        metafox,   metafox,   downtown_state, init_metafox,   ROT270, "Jordan I.S. / Seta",        "Meta Fox" , 0) // Developed by Jordan for Seta, Country/License: DSW
