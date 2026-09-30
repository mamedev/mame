// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Bosconian  (c) 1981 Namco
Galaga     (c) 1981 Namco
Xevious    (c) 1982 Namco
Dig Dug    (c) 1982 Namco

driver by Nicola Salmoria
based on previous work by Martin Scragg, Mirko Buffoni, Aaron Giles


All these games are based on the same 3xZ80, shared memory, CPU design.
Bosconian and Galaga use the same CPU board, with minor differences
(Galaga has one missing RAM and no 50XX custom)
Xevious is physically different, but logically identical.
Dig Dug is the only one a bit different, because it reads the DIP switches
through a custom chip instead of having them mapped in memory.

The video board, on the other hand, is completely different for all the games,
that's why they use separate video/ source files.


Custom ICs:
----------
Galaga:
------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
51XX     I/O
54XX     explosion sound generator

Video board:
00XX     tilemap address generator with scrolling capability (only Super Pacman)
02XX     gfx data shifter and mixer (16-bit in, 4-bit out)
04XX     sprite address generator
05XX     starfield generator
07XX     clock divider


Memory maps:
-----------
Galaga:
------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3N    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3M    program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 3L    program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM 3K    program ROM
the rest of the memory map is common to the other CPUs

SUB CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3J    program ROM
0001------------              n.c.
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3E    program ROM
0001------------              n.c.
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

COMMON:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
01000-----------              n.c.
01001-----------              n.c.
01010-----------              n.c.
01011-----------              n.c.
01100-----------              n.c.
01101-----00----   W ----xxxx RAM 2A    \ sound control registers
01101-----01----   W ----xxxx RAM 2B    /
01101-----10-000   W -------x IRQ1      main CPU irq enable/acknowledge
01101-----10-001   W -------x IRQ2      motion CPU irq enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU nmi enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x MOD 0     unused?
01101-----10-110   W -------x MOD 1     unused?
01101-----10-111   W -------x MOD 2     unused?
01101-----11----   W -------- WDR       watchdog reset
01101-----00-xxx R   -------x DIP SW    dip switch B
01101-----00-xxx R   ------x- DIP SW    dip switch A
01101-----01---- R            n.c.
01101-----10---- R            n.c.
01101-----11---- R            n.c.
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
10000xxxxxxxxxxx R/W xxxxxxxx RAM 1K    tilemap RAM
10001-xxxxxxxxxx R/W xxxxxxxx RAM 3E/3F work RAM
10001-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10010-xxxxxxxxxx R/W xxxxxxxx RAM 3K/3L work RAM
10010-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10011-xxxxxxxxxx R/W xxxxxxxx RAM 3H/3J work RAM
10011-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10100--------000   W -------x           \
10100--------001   W -------x            > to 05XX: starfield X scroll speed
10100--------010   W -------x           /
10100--------011   W -------x           \ to 05XX: starfield blink
10100--------100   W -------x           /          (select active subset)
10100--------101   W -------x           to 05XX: unknown. It is the same as STARCLR in Bosconian
10100--------110   W -------x n.c.
10100--------111   W -------x FLIP      flip screen
10101-----------              n.c.
10110-----------              n.c.
10111-----------              n.c.


Namco vs Midway ROM names and locations
---------------------------------------
Location  ID         Location  ID
--------  ----       --------  -----
CPU 3P    GG1-1      CPU 3N    3200A
CPU 3M    GG1-2      CPU 3M    3300B
CPU 2M    GG1-3      CPU 3L    3400C
CPU 2L    GG1-4      CPU 3K    3500D
CPU 3F    GG1-5      CPU 3J    3600E
CPU 2C    GG1-7      CPU 3E    3700G
CPU 1D    GG1-1[bpr] CPU 1D
CPU 5C    GG1-2[bpr] CPU 5C

VID 4L    GG1-9      VID 4L    2600J
VID 4F    GG1-10     VID 4F    2700K
VID 4D    GG1-11     VID 4D    2800L
VID 1C    GG1-3[bpr] VID 1C
VID 2N    GG1-4[bpr] VID 2N
VID 5N    GG1-5[bpr] VID 5N



Gatsbee (Galaga mod/bootleg)
----------------------------
This game runs on modified bootleg Galaga hardware (blue board with PCB numbers DG-09-02 and DG-07-02)

ROM8: is a 2764. pins 1, 26, 27, 28 tied together.
      pin2 out of socket, has wire that is tied to pin 4 of a LS259 that sits on top of the main Z80
      CPU located at 5B/6B

Z80: There are 2 logic chips sitting on top of it which are wired up to the Z80 and to each other.
     Looks like this....
     |-------------------|
     |  LS32   LS259     <
     |-------------------|

Bend all the legs outwards.
Line up the LS259 so pin 16 is in line with Z80 pin 11
Line up the LS32 so pin 7 is in line with Z80 pin 29
Atach the 2 chips to the top of the Z80 with some glue
Connect like this....

LS32 pin 1 tied to Z80 pin 22 (WR)
LS32 pin 2 tied to Z80 pin 19 (MREQ)
LS32 pin 3,4 tied together
LS32 pin 5 tied to Z80 pin 4 (A14)
LS32 pin 6 tied to pin 10 LS32
LS32 pin 7 tied to Z80 pin 29 (GND)
LS32 pin 8 tied to LS259 pin 14
LS32 pin 9 tied to Z80 pin 5 (A15)
LS32 pins 11, 12, 13 have NC
LS32 pin 14 tied to Z80 pin 11 (+5V)

LS259 pin 1 tied to Z80 pin 30 (A0)
LS259 pin 2 tied to Z80 pin 31 (A1)
LS259 pin 3 tied to Z80 pin 32 (A2)
LS259 pin 4 to ROM 8 (as above)
LS259 pins 5, 6, 7 have NC
LS259 pin 8 tied to Z80 pin 29 (GND)
LS259 pins 9, 10, 11, 12 have NC
LS259 pin 13 tied to Z80 pin 14 (D0)
LS259 pin 15 tied to Z80 pin 26 (RESET)
LS259 pin 16 tied to Z80 pin 11 (+5V)



Easter eggs:
-----------
- Galaga:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    5xR 6xL 3xR 7xL
  (c) 1981 NAMCO LTD. will appear on the screen.


Notes:
-----
- The Cabinet Type "DIP switch" actually comes from the edge connector, but is mapped
  in memory in place of dip switch #8. DIP switch #8 selects single/dual coin counters
  and is entirely handled by hardware.

- galaga: there is a bug in the sound CPU program. During initialization, it enables
  NMI before clearing RAM, but the NMI handler doesn't save the registers, so it cannot
  interrupt program execution. If the NMI happens before the LDIR that clears RAM has
  finished, the program will crash.

- galaga: there were "fast shoot" hacks available, which are not supported.
  Their effects can be replicated with this line in cheat.dat:
  galaga:1:070D:0D:100:Fast Shoot

- bosco & galaga: the Midway arcade cabinet had an optional rapid fire board, using
  a 556 to generate autofire while the button was held. That really makes little
  sense in Galaga! For Bosconian, I guess it was for the boscomdo set I, because the
  other sets have autofire built-in.

- gallag is identical to galagao, apart from the title changed to "GALLAG" and the
  copyright notice changed from "(c) 1981 NAMCO LTD" to "1 9 8 2" (and the Namco logo
  removed from the gfx). The only interesting thing about it is the 4th Z80, used to
  simulate the custom 5xXX chips of the original.
  It also has different explosion and starfield circuitries, to do without the Namco
  custom chips.


TODO:
----
- gallag/gatsbee: explosions are not emulated since the bootleg board doesn't have
  the 54XX custom. Should probably use samples like Battles?

***************************************************************************/

#include "emu.h"
#include "galaga.h"

#include "namco06.h"
#include "namco51.h"
#include "namco54.h"

#include "cpu/mb88xx/mb88xx.h"
#include "cpu/z80/z80.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"

#include "speaker.h"
#include "video/resnet.h"


#define MASTER_CLOCK                (XTAL(18'432'000))

#define STARFIELD_X_OFFSET_GALAGA   (16)
#define STARFIELD_X_LIMIT_GALAGA    (256 + STARFIELD_X_OFFSET_GALAGA)


uint8_t galaga_state_base::bosco_dsw_r(offs_t offset)
{
	uint8_t const bit0 = BIT(m_io_dsw[1]->read(), offset);
	uint8_t const bit1 = BIT(m_io_dsw[0]->read(), offset);

	return bit0 | (bit1 << 1);
}

void galaga_state_base::irq1_clear_w(int state)
{
	m_main_irq_mask = state;
	if (!m_main_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void galaga_state_base::irq2_clear_w(int state)
{
	m_sub_irq_mask = state;
	if (!m_sub_irq_mask)
		m_subcpu->set_input_line(0, CLEAR_LINE);
}

void galaga_state_base::nmion_w(int state)
{
	m_sub2_nmi_mask = !state;
}

void galaga_state_base::out(uint8_t data)
{
	m_leds[1] = BIT(data, 0);
	m_leds[0] = BIT(data, 1);
	machine().bookkeeping().coin_counter_w(1, BIT(~data, 2));
	machine().bookkeeping().coin_counter_w(0, BIT(~data, 3));
}

void galaga_state_base::vblank_irq(int state)
{
	if (state && m_main_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	if (state && m_sub_irq_mask)
		m_subcpu->set_input_line(0, ASSERT_LINE);
}


void galaga_state_base::lockout(int state)
{
	machine().bookkeeping().coin_lockout_global_w(state);
}

TIMER_CALLBACK_MEMBER(galaga_state_base::cpu3_interrupt_callback)
{
	int scanline = param;

	if(m_sub2_nmi_mask)
		m_subcpu2->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	scanline = scanline + 128;
	if (scanline >= 272)
		scanline = 64;

	/* the vertical synch chain is clocked by H256 -- this is probably not important, but oh well */
	m_cpu3_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


void galaga_state_base::machine_start()
{
	// create the interrupt timer
	m_cpu3_interrupt_timer = timer_alloc(FUNC(galaga_state::cpu3_interrupt_callback), this);

	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_sub_irq_mask));
	save_item(NAME(m_sub2_nmi_mask));
}

void galaga_state_base::machine_reset()
{
	m_cpu3_interrupt_timer->adjust(m_screen->time_until_pos(64), 64);
}



/***************************************************************************

  Convert the color PROMs.

  Galaga has one 32x8 palette PROM and two 256x4 color lookup table PROMs
  (one for characters, one for sprites). Only the first 128 bytes of the
  lookup tables seem to be used.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void galaga_state::galaga_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances[3] = { 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	// core palette
	for (int i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(*color_prom, 6);
		bit1 = BIT(*color_prom, 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
		color_prom++;
	}

	// r/g low bit is n/c and effectively becomes a pulldown
	double rsweights[2], gsweights[2], bsweights[2];
	compute_resistor_weights(0, 255, -1.0,
			2, &resistances[1], rsweights, resistances[0], 0,
			2, &resistances[1], gsweights, resistances[0], 0,
			2, &resistances[1], bsweights, 0, 0);

	// palette for the stars
	for (int i = 0; i < 64; i++)
	{
		int const r = combine_weights(rsweights, BIT(i, 0), BIT(i, 1));
		int const g = combine_weights(gsweights, BIT(i, 2), BIT(i, 3));
		int const b = combine_weights(bsweights, BIT(i, 4), BIT(i, 5));

		palette.set_indirect_color(32 + i, rgb_t(r, g, b));
	}

	// characters
	for (int i = 0; i < 64*4; i++)
		palette.set_pen_indirect(i, (*color_prom++ & 0x0f) | 0x10);

	// sprites
	for (int i = 0; i < 64*4; i++)
		palette.set_pen_indirect(64*4 + i, *color_prom++ & 0x0f);

	// now the stars
	for (int i = 0; i < 64; i++)
		palette.set_pen_indirect(64*4 + 64*4 + i, 32 + i);
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(galaga_state::tilemap_scan)
{
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	else
		return col + (row << 5);
}


TILE_GET_INFO_MEMBER(galaga_state::get_tile_info)
{
	/* the hardware has two character sets, one normal and one x-flipped. When
	   screen is flipped, character y flip is done by the hardware inverting the
	   timing signals, while x flip is done by selecting the 2nd character set.
	   We reproduce this here, but since the tilemap system automatically flips
	   characters when screen is flipped, we have to flip them back. */
	int color = m_videoram[tile_index + 0x400] & 0x3f;
	tileinfo.set(0,
			(m_videoram[tile_index] & 0x7f) | (flip_screen() ? 0x80 : 0) | (m_gfxbank << 8),
			color,
			flip_screen() ? TILE_FLIPX : 0);
	tileinfo.group = color;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void galaga_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaga_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(galaga_state::tilemap_scan)), 8,8,36,28);
	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);

	m_gfxbank = 0;

	save_item(NAME(m_gfxbank));
}



/***************************************************************************

  Memory handlers

***************************************************************************/


void galaga_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void galaga_state::gatsbee_bank_w(int state)
{
	m_gfxbank = state;
	m_fg_tilemap->mark_all_dirty();
}



/***************************************************************************

  Display refresh

***************************************************************************/

void galaga_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *spriteram = &m_galaga_ram1[0x380];
	uint8_t *spriteram_2 = &m_galaga_ram2[0x380];
	uint8_t *spriteram_3 = &m_galaga_ram3[0x380];

	for (int offs = 0; offs < 0x80; offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		const int sprite = spriteram[offs] & 0x7f;
		const int color = spriteram[offs + 1] & 0x3f;
		int sx = spriteram_2[offs + 1] - 40 + 0x100*(spriteram_3[offs + 1] & 3);
		int sy = 256 - spriteram_2[offs] + 1;   // sprites are buffered and delayed by one scanline
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		const int sizex = (spriteram_3[offs] & 0x04) >> 2;
		const int sizey = (spriteram_3[offs] & 0x08) >> 3;

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;  // fix wraparound

		if (flip_screen())
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		for (int y = 0; y <= sizey; y++)
		{
			for (int x = 0; x <= sizex; x++)
			{
				m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x, sy + 16*y,
					m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x0f));
			}
		}
	}
}



uint32_t galaga_state::screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_starfield->draw_starfield(bitmap,cliprect, 0);
	draw_sprites(bitmap,cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect);
	return 0;
}



void galaga_state::screen_vblank_galaga(int state)
{
	// falling edge
	if (!state)
	{
		// Galaga only scrolls in X direction - the SCROLL_Y pins
		// of the 05XX chip are tied to ground.
		const uint8_t speed_index_X = (m_videolatch->q2_r()<<2) | (m_videolatch->q1_r()<<1) | (m_videolatch->q0_r()<<0);
		const uint8_t speed_index_Y = 0;
		m_starfield->set_scroll_speed(speed_index_X,speed_index_Y);

		m_starfield->set_active_starfield_sets(m_videolatch->q3_r(), m_videolatch->q4_r() | 2);

		// _STARCLR signal enables/disables starfield
		m_starfield->enable_starfield(m_videolatch->q5_r());
	}
}


void galaga_state::galaga_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().nopw();         /* the only area different for each CPU */
	map(0x6800, 0x6807).r(FUNC(galaga_state::bosco_dsw_r));
	map(0x6800, 0x681f).w(m_namco_sound, FUNC(namco_wsg_device::pacman_sound_w));
	map(0x6820, 0x6827).w("misclatch", FUNC(ls259_device::write_d0));
	map(0x6830, 0x6830).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x7000, 0x70ff).rw("06xx", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x7100, 0x7100).rw("06xx", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
	map(0x8000, 0x87ff).ram().w(FUNC(galaga_state::videoram_w)).share(m_videoram);
	map(0x8800, 0x8bff).ram().share(m_galaga_ram1);
	map(0x9000, 0x93ff).ram().share(m_galaga_ram2);
	map(0x9800, 0x9bff).ram().share(m_galaga_ram3);
	map(0xa000, 0xa007).w(m_videolatch, FUNC(ls259_device::write_d0));
}

void galaga_state::gatsbee_main_map(address_map &map)
{
	galaga_map(map);

	map(0x0000, 0x0007).mirror(0x3ff8).w("extralatch", FUNC(ls259_device::write_d0));
}



/* bootleg 4th CPU replacing the 5xXX chips */
void galaga_state::galaga_mem4(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x107f).ram();
}


static INPUT_PORTS_START( galaga )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze" )                PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Rack Test" )             PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x20, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) /* Began with 2, 3 or 4 fighters */
	PORT_DIPSETTING(    0x18, "20K and 60K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) // factory default = "20K, 70K, Every70K"
	PORT_DIPSETTING(    0x30, "20K, 80K, Every 80K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x38, "30K and 80K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0) /* Began with 5 fighters */
	PORT_DIPSETTING(    0x18, "30K and 150K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "30K, 150K, Every 150K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x38, "30K Only" )              PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "30K and 100K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K and 120K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( galagamw )
	PORT_INCLUDE( galaga )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits Game" )        PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x01, "2 Players" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
INPUT_PORTS_END

/* the same as galaga but with vertical movement */
static INPUT_PORTS_START( gatsbee )
	PORT_INCLUDE( galaga )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
INPUT_PORTS_END



static const gfx_layout charlayout_2bpp =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8) },
	16*8
};

const gfx_layout spritelayout_galaga =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0*8,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};

static GFXDECODE_START( gfx_galaga )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_2bpp,        0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_galaga, 64*4, 64 )
GFXDECODE_END


void galaga_state::galaga(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &galaga_state::galaga_map);

	Z80(config, m_subcpu, MASTER_CLOCK/6);    /* 3.072 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &galaga_state::galaga_map);

	Z80(config, m_subcpu2, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_subcpu2->set_addrmap(AS_PROGRAM, &galaga_state::galaga_map);

	ls259_device &misclatch(LS259(config, "misclatch")); // 3C on CPU board
	misclatch.q_out_cb<0>().set(FUNC(galaga_state::irq1_clear_w));
	misclatch.q_out_cb<1>().set(FUNC(galaga_state::irq2_clear_w));
	misclatch.q_out_cb<2>().set(FUNC(galaga_state::nmion_w));
	misclatch.q_out_cb<3>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append_inputline(m_subcpu2, INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append("51xx", FUNC(namco_51xx_device::reset));
	misclatch.q_out_cb<3>().append("54xx", FUNC(namco_54xx_device::reset));

	namco_51xx_device &n51xx(NAMCO_51XX(config, "51xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n51xx.input_callback<0>().set_ioport("IN0").mask(0x0f);
	n51xx.input_callback<1>().set_ioport("IN0").rshift(4);
	n51xx.input_callback<2>().set_ioport("IN1").mask(0x0f);
	n51xx.input_callback<3>().set_ioport("IN1").rshift(4);
	n51xx.output_callback().set(FUNC(galaga_state::out));
	n51xx.lockout_callback().set(FUNC(galaga_state::lockout));

	namco_54xx_device &n54xx(NAMCO_54XX(config, "54xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n54xx.set_discrete("discrete");
	n54xx.set_basenote(NODE_01);

	namco_06xx_device &n06xx(NAMCO_06XX(config, "06xx", MASTER_CLOCK/6/64));
	n06xx.set_maincpu(m_maincpu);
	n06xx.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));
	n06xx.write_callback<3>().set("54xx", FUNC(namco_54xx_device::write));
	n06xx.chip_select_callback<3>().set("54xx", FUNC(namco_54xx_device::chip_select));

	LS259(config, m_videolatch); // 5K on video board
	// Q0-Q5 to 05XX for starfield control
	m_videolatch->q_out_cb<7>().set(FUNC(galaga_state::flip_screen_set));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 8);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_raw(MASTER_CLOCK/3, 384, 0, 288, 264, 0, 224);
	m_screen->set_screen_update(FUNC(galaga_state::screen_update_galaga));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE); // starfield lfsr
	m_screen->screen_vblank().set(FUNC(galaga_state::screen_vblank_galaga));
	m_screen->screen_vblank().append(FUNC(galaga_state::vblank_irq));
	m_screen->screen_vblank().append("51xx", FUNC(namco_51xx_device::vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galaga);
	PALETTE(config, m_palette, FUNC(galaga_state::galaga_palette), 64*4 + 64*4 + 4 + 64, 32+64);

	STARFIELD_05XX(config, m_starfield);
	m_starfield->set_starfield_config(STARFIELD_X_OFFSET_GALAGA, 0, STARFIELD_X_LIMIT_GALAGA);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NAMCO_WSG(config, m_namco_sound, MASTER_CLOCK/6/32);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0);

	/* discrete circuit on the 54XX outputs */
	DISCRETE(config, "discrete", galaga_discrete).add_route(ALL_OUTPUTS, "mono", 0.90);
}

void galaga_state::galagab(machine_config &config)
{
	galaga(config);

	/* basic machine hardware */
	config.device_remove("06xx");
	config.device_remove("54xx");
	ls259_device* misclatch = reinterpret_cast<ls259_device*>(config.device("misclatch"));
	// galaga has the custom chips on this line, so just set the resets this board has
	misclatch->q_out_cb<3>().set_inputline("sub", INPUT_LINE_RESET).invert();
	misclatch->q_out_cb<3>().append_inputline("sub2", INPUT_LINE_RESET).invert();
	misclatch->q_out_cb<3>().append_inputline("sub3", INPUT_LINE_RESET).invert();

	/* FIXME: bootlegs should not have any Namco custom chip. However, this workaround is needed atm */
	namco_06xx_device &n06xx(NAMCO_06XX(config, "06xx", MASTER_CLOCK/6/64));
	n06xx.set_maincpu(m_maincpu);
	n06xx.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));

	z80_device &sub3(Z80(config, "sub3", MASTER_CLOCK/6));   /* 3.072 MHz */
	sub3.set_addrmap(AS_PROGRAM, &galaga_state::galaga_mem4);

	/* sound hardware */
	config.device_remove("discrete");
}

void galaga_state::gatsbee(machine_config &config)
{
	galaga(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &galaga_state::gatsbee_main_map);

	ls259_device &extralatch(LS259(config, "extralatch"));
	extralatch.q_out_cb<0>().set(FUNC(galaga_state::gatsbee_bank_w));
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

/**********************************************************************************************
  Galaga & clones
**********************************************************************************************/
/*

Galaga
Namco/Midway, 1982

PCB Layout
----------

Top board

23149611 (23149631
|------------------------------------------|
|                         LM324            |
|          04M_G01.3N                      |
|                       Z80        5400    |
|          04K_G02.3M                      |
|                                          |
|   0600   04J_G03.3L                    |-|
|                                 DSW1   |
|          04H_G04.3K             DSW2   |-|
|                                          |
|   0801   04E_G05.3J   Z80               4|
|                                         4|
|                                         W|
|          *            5100              A|
|   0801                                  Y|
|                                TD62064   |
|          04D_G06.3E   Z80                |
|                                        |-|
|   0801                                 |
|          *                             |-|
|                       0702     VOL       |
|                                    MB3730|
|GG1-1.1D                                  |
|                       GG1-2.5C           |
|       3101                               |
|       3101  4066                18.432MHz|
|------------------------------------------|
Notes:
      GG1-1.1D & GG1-2.5C are PROMs, type MB7052 (equivalent to TBP24S10 and 82S129).
      All other ROMs are 2732 EPROMs (i.e. 04*.*).
      *: Unpopulated sockets

      VSync           : 60.606060Hz
      Z80 clocks (all): 1.536MHz
      5400 clock      : 1.536MHz
      5100 clock      : 1.536MHz

      3101   : 16bytes x4 bit Bipolar SRAM, compatible with 7489, MB461 & AM31L01 (trivia - This was Intel's first product, released in 1969!)
      MB3730 : Sound AMP
      TD62064: Darlington transistor for driving coin counters.
      4066   : Quad Bilateral Switch logic IC, used to mix several sound sources to one output.

      NAMCO customs:
                    0600 (DIP28): Bus Interface IC
                    0801 (DIP28): Multi CPU Bus Controller IC
                    5100 (DIP42): Controls player input, coins, DSW's (custom 4 bit I/O Microcontroller)
                    0702 (DIP28): Sync Generator/Clock Divider IC
                    5400 (DIP28): MUX 4-channel Audio Generator IC. Generates 'death bang'.
                                  This is not a Z80 with swapped pins as many sites have reported.

      Pinouts:
      Galaga PCB edge connector pinouts

      Parts Side    Pin   Pin Solder Side
      ----------------------------------
      Logic Ground   A     1  Logic Ground
      Speaker +      B     2  Speaker -
                     C     3  Coin Counter 1
      P1 Start Lamp  D     4  P2 Start Lamp
      +12            E     5  +12
      +5             F     6  +5
      Ground         H     7  Ground
      Service Credit J     8  Test
      Coin 1         K     9  Coin 2
      Player 1 Start L     10 Player 2 Start
      P1 Fire        M     11 P2 Fire
      P1 Left        N     12 P2 Left
                     P     13
      P1 Right       R     14 P2 Right
                     S     15
                     T     16
                     U     17
                     V     18
                     W     19
                     X     20
      Coin Counter 2 Y     21 Cocktail Mode
      Ground         Z     22 Ground

      Pin21: Ground this pin for cocktail mode


Bottom board

23149612 (23149632
|------------------------------------------|
| 0700     GG1-4.2N           GG1-5.5N     |
|                        *             RGBS|
| 0015                                     |
|                                          |
|              2114                        |
| 6116                               8147  |
|              2114    07M_G08.4L          |
|                                    8147  |
|              2114                        |
| 0400                               8147  |
|              2114                        |
|                                    8147  |
|              2114    0200                |
|                                          |
|              2114                        |
|                      07H_G09.4F    8147  |
|                                          |
|                                    8147  |
|                                          |
|                      07E_G10.4D    8147  |
|                                          |
|                                    8147  |
| GG1-3.1C                                 |
|                                          |
|                                          |
|------------------------------------------|
Notes:
      RGBS: Video output socket (Red, Green Blue, Sync to monitor)
      GG1*  are PROMs, type MB7052 (equivalent to TBP24S10 and 82S129).
      All other ROMs are 2732 EPROMs.
      *: Unpopulated socket

      2114    : 1K x4 SRAM
      6116    : 2K x8 SRAM
      8147    : 4K x1 SRAM (Note - you can remove the eight 8147 RAMs and install two 2148s (1K x 4) in their place at positions 6H and 6B.

      Bootup RAM Errors
      Error Code    Meaning
      RAM OK        All RAMs are good
      RAM 0L        RAM located on Video PC board at position 1K is bad
      RAM 0H        RAM located on Video PC board at position 1K is bad
      RAM 1L        RAM located on Video PC board at position 1K is bad
      RAM 1H        RAM located on Video PC board at position 1K is bad
      RAM 2L        RAM located on Video PC board at position 3E is bad
      RAM 2H        RAM located on Video PC board at position 3F is bad
      RAM 3L        RAM located on Video PC board at position 3K is bad
      RAM 3H        RAM located on Video PC board at position 3L is bad
      RAM 4L        RAM located on Video PC board at position 3H is bad
      RAM 4H        RAM located on Video PC board at position 3J is bad

      Bootup ROM Errors
      Error Code   Meaning
      ROM OK       All ROMs are good
      ROM 01       ROM located on CPU PC board at position 3N is bad
      ROM 02       ROM located on CPU PC board at position 3M is bad
      ROM 03       ROM located on CPU PC board at position 3L is bad
      ROM 04       ROM located on CPU PC board at position 3K is bad
      ROM 11       ROM located on CPU PC board at position 3J is bad
      ROM 21       ROM located on CPU PC board at position 3E is bad

      NAMCO customs:
                    0015 (DIP28): Video RAM addresser IC
                    0200 (DIP28): Graphics ROM Data Custom Shift Register IC
                    0400 (DIP28): Motion Object and Scratch RAM to CPU Bus Interface IC
                    0702 (DIP28): Sync Generator/Clock Divider IC

There's also a Sidam version of Galaga (Sidam PCB 11200, official license for the Italian market),
but, unlike Sidam's Dig Dug, this one does not use any unique ROM, but a mix between the two Namco sets:
     bottom-10_2532.4d  = gg1_11.4d  galaga   Galaga (Namco rev. B)
     bottom-8_2532.4h   = gg1_9.4l   galaga   Galaga (Namco rev. B)
     bottom-9_2532.4f   = gg1_10.4f  galaga   Galaga (Namco rev. B)
     top-0_2532.3n      = gg1-1.3p   galagao  Galaga (Namco)
     top-1_2532.3m      = gg1-2.3m   galagao  Galaga (Namco)
     top-2_2532.3l      = gg1_3.2m   galaga   Galaga (Namco rev. B)
     top-3_2532.3k      = gg1-4.2l   galagao  Galaga (Namco)
     top-4_2532.3j      = gg1_5b.3f  galaga   Galaga (Namco rev. B)
     top-6_2532.3e      = gg1_7b.2c  galaga   Galaga (Namco rev. B)
*/

ROM_START( galaga )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gg1_1b.3p",    0x0000, 0x1000, CRC(ab036c9f) SHA1(ca7f5da42d4e76fd89bb0b35198a23c01462fbfe) )
	ROM_LOAD( "gg1_2b.3m",    0x1000, 0x1000, CRC(d9232240) SHA1(ab202aa259c3d332ef13dfb8fc8580ce2a5a253d) )
	ROM_LOAD( "gg1_3.2m",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gg1_4b.2l",    0x3000, 0x1000, CRC(499fcc76) SHA1(ddb8b121903646c320939c7d13f4aa4ebb130378) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1_5b.3f",    0x0000, 0x1000, CRC(bb5caae3) SHA1(e957a581463caac27bc37ca2e2a90f27e4f62b6f) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1_7b.2c",    0x0000, 0x1000, CRC(d016686b) SHA1(44c1a04fba3c7c826ff484185cb881b4b22e6657) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1_9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1_11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1_10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagao )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gg1-1.3p",     0x0000, 0x1000, CRC(a3a0f743) SHA1(6907773db7c002ecde5e41853603d53387c5c7cd) )
	ROM_LOAD( "gg1-2.3m",     0x1000, 0x1000, CRC(43bb0d5c) SHA1(666975aed5ce84f09794c54b550d64d95ab311f0) )
	ROM_LOAD( "gg1-3.2m",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gg1-4.2l",     0x3000, 0x1000, CRC(83874442) SHA1(366cb0dbd31b787e64f88d182108b670d03b393e) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7.2c",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1-9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1-11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1-10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamw )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "3200a.bin",    0x0000, 0x1000, CRC(3ef0b053) SHA1(0c04a362b737998c0952a753fb3fd8c8a17e9b46) )
	ROM_LOAD( "3300b.bin",    0x1000, 0x1000, CRC(1b280831) SHA1(f7ea12e61929717ebe43a4198a97f109845a2c62) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "3500d.bin",    0x3000, 0x1000, CRC(0aaf5c23) SHA1(3f4b0bb960bf002261e9c1278c88f594c6aa8ab6) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "3600e.bin",    0x0000, 0x1000, CRC(bc556e76) SHA1(0d3d68243c4571d985b4d8f7e0ea9f6fcffa2116) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "3700g.bin",    0x0000, 0x1000, CRC(b07f0aa4) SHA1(7528644a8480d0be2d0d37069515ed319e94778f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2600j.bin",    0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2800l.bin",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "2700k.bin",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamf )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "3200a.bin",    0x0000, 0x1000, CRC(3ef0b053) SHA1(0c04a362b737998c0952a753fb3fd8c8a17e9b46) )
	ROM_LOAD( "3300b.bin",    0x1000, 0x1000, CRC(1b280831) SHA1(f7ea12e61929717ebe43a4198a97f109845a2c62) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "3500d.bin",    0x3000, 0x1000, CRC(0aaf5c23) SHA1(3f4b0bb960bf002261e9c1278c88f594c6aa8ab6) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "3600fast.bin", 0x0000, 0x1000, CRC(23d586e5) SHA1(43346c69385e9091e64cff6c027ac2689cafcbb9) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "3700g.bin",    0x0000, 0x1000, CRC(b07f0aa4) SHA1(7528644a8480d0be2d0d37069515ed319e94778f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2600j.bin",    0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2800l.bin",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "2700k.bin",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamk )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "mk2-1",        0x0000, 0x1000, CRC(23cea1e2) SHA1(18db33ade0ca6e47cc48aa151d2ccbb4646e3ae3) )
	ROM_LOAD( "mk2-2",        0x1000, 0x1000, CRC(89695b1a) SHA1(fda5557018884e903f855bf3b69a25d75ed8a767) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "mk2-4",        0x3000, 0x1000, CRC(24b767f5) SHA1(d4c03e2ed582cfa7f8168ac352f790ef7af54cb8) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7b.2c",    0x0000, 0x1000, CRC(d016686b) SHA1(44c1a04fba3c7c826ff484185cb881b4b22e6657) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1-9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1-11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1-10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( gallag )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gallag.1",     0x0000, 0x1000, CRC(a3a0f743) SHA1(6907773db7c002ecde5e41853603d53387c5c7cd) )
	ROM_LOAD( "gallag.2",     0x1000, 0x1000, CRC(5eda60a7) SHA1(853d7b974dd04abd7af3a8ba2681dfabce4dce18) )
	ROM_LOAD( "gallag.3",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gallag.4",     0x3000, 0x1000, CRC(83874442) SHA1(366cb0dbd31b787e64f88d182108b670d03b393e) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gallag.5",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gallag.7",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "gallag.6",     0x0000, 0x1000, CRC(001b70bc) SHA1(b465eee91e75257b7b049d49c0064ab5fd66c576) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gallag.8",     0x0000, 0x1000, CRC(169a98a4) SHA1(edbeb11076061e744ea88d9899dbdfe0964c7e78) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gallag.a",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gallag.9",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( gatsbee )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "1.4b",         0x0000, 0x1000, CRC(9fb8e28b) SHA1(7171e3fb37b0d6cc8f7a023c1775080d5986de99) )
	ROM_LOAD( "2.4c",         0x1000, 0x1000, CRC(bf6cb840) SHA1(5763140d32d35a38cdcb49e6de1fd5b07a9e8cc2) )
	ROM_LOAD( "3.4d",         0x2000, 0x1000, CRC(3604e2dd) SHA1(1736cf8497f7ac28e92ca94fa137c144353dc192) )
	ROM_LOAD( "4.4e",         0x3000, 0x1000, CRC(bf9f613b) SHA1(41c852fc77f0f35bf48a5b81a19234ed99871c89) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )    // 5.4j

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7.2c",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )    // 7.4k

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "gallag.6",     0x0000, 0x1000, CRC(001b70bc) SHA1(b465eee91e75257b7b049d49c0064ab5fd66c576) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "8.5r",  0x0000, 0x2000, CRC(b324f650) SHA1(7bcb254f7cf03bd84291b9fdc27b8962b3e12aa4) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "9.6a",         0x0000, 0x1000, CRC(22e339d5) SHA1(9ac2887ede802d28daa4ad0a0a54bcf7b1155a2e) )
	ROM_LOAD( "10.7a",        0x1000, 0x1000, CRC(60dcf940) SHA1(6530aa5b4afef4a8422ece76a93d0c5b1d93355e) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( nebulbee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nebulbee.01",  0x0000, 0x1000, CRC(f405f2c4) SHA1(9249afeffd8df0f24539ea9b4f88c23a6ad58d8c) )
	ROM_LOAD( "nebulbee.02",  0x1000, 0x1000, CRC(31022b60) SHA1(90e64afb4128c6dfeeee89635ea9f97a34f70f5f) )
	ROM_LOAD( "gg1_3.2m",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "nebulbee.04",  0x3000, 0x1000, CRC(d76788a5) SHA1(adcb83cf64951d86c701a99b410e9230912f8a48) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gg1-5",        0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "gg1-7",        0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "nebulbee.07",     0x0000, 0x1000, CRC(035e300c) SHA1(cfda2467e71c27381b7150ff8fc7b69d61df123a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1_9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1_11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1_10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "prom-5.5n",       0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )
	ROM_LOAD( "2n.bin",       0x0020, 0x0100, CRC(a547d33b) SHA1(7323084320bb61ae1530d916f5edd8835d4d2461) )
	ROM_LOAD( "1c.bin",       0x0120, 0x0100, CRC(b6f585fb) SHA1(dd10147c4f05fede7ae6e7a760681700a660e87e) )
	ROM_LOAD( "5c.bin",       0x0220, 0x0100, CRC(8bd565f6) SHA1(bedba65816abfc2ebeacac6ee335ca6f136e3e3d) )

	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "1d.bin",       0x0000, 0x0100, CRC(86d92b24) SHA1(6bef9102b97c83025a2cf84e89d95f2d44c3d2ed) )
ROM_END

void galaga_state::init_galaga()
{
	// TODO: handle in drawing routines not here
	/* swap bytes for flipped character so we can decode them together with normal characters */
	uint8_t *rom = memregion("gfx1")->base();
	int len = memregion("gfx1")->bytes();

	for (int i = 0; i < len; i++)
	{
		if ((i & 0x0808) == 0x0800)
		{
			int t = rom[i];
			rom[i] = rom[i+8];
			rom[i+8] = t;
		}
	}
}


/* Original Namco hardware, with Namco Customs */

//    YEAR, NAME,      PARENT,   MACHINE, INPUT,    STATE,         INIT,         MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1981, galaga,    0,        galaga,  galaga,   galaga_state,  init_galaga,  ROT90,  "Namco", "Galaga (Namco rev. B)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, galagao,   galaga,   galaga,  galaga,   galaga_state,  init_galaga,  ROT90,  "Namco", "Galaga (Namco)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, galagamw,  galaga,   galaga,  galagamw, galaga_state,  init_galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, galagamk,  galaga,   galaga,  galaga,   galaga_state,  init_galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, galagamf,  galaga,   galaga,  galaga,   galaga_state,  init_galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 1 with fast shoot hack)", MACHINE_SUPPORTS_SAVE )

/* Bootlegs with replacement I/O chips */

GAME( 1982, gallag,    galaga,   galagab, galaga,   galaga_state,  init_galaga,  ROT90,  "bootleg", "Gallag", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, gatsbee,   galaga,   gatsbee, gatsbee,  galaga_state,  init_galaga,  ROT90,  "hack (Uchida)", "Gatsbee", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1981, nebulbee,  galaga,   galagab, galaga,   galaga_state,  init_galaga,  ROT90,  "bootleg", "Nebulous Bee", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
