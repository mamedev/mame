// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/**************************************************************************

Based on drivers from Juno First emulator by Chris Hardy (chris@junofirst.freeserve.co.uk)

To enter service mode, keep 1&2 pressed on reset


 Info provided with these alt sets

        MEGA ZONE   CHIP PLACEMENT

USES 69A09EP, Z80 CPU'S & AY-3-8910 SOUND CHIP W/8039 CPU

THERE ARE AT LEAST THREE VERSIONS OF MEGA ZONE, ALL THE ROMS ARE THE
SAME EXCEPT POSITION 6,7,8,9,11H IN SETS 1,2
ALL ROMS ARE 2764 EXCEPT H01 (E01) IS A 2732

CHIP #     POSITION                         VERS 3
-----------------------------------------------------
VER-1                VER-2
-----------------------------------------------------
319-E08    2D        E08      REAR BOARD    8  SAME
319-E09    2E        E09       "            9  SAME
319-E10    3D        E10       "           10  SAME
319-E11    3E        E11       "           11  SAME
319-G12    8C        G12       "           12
319-G13    10C       G13       "           13  SAME
319-E02    6D        E02      CONN BOARD    2  SAME
319-H03    6H        J03       "            3
319-H04    7H        J04       "            4
319-H05    8H        J05       "            5
319-H06    9H        J06       "            6
319-H07    11H       J07       "            7
319-H01    3A        E01       "            1  SAME
Z80        7E                                        IC#
AY-3-8910  8B                               PROM     98     TBP18S030 (82S123)
AO72       12F   KONAMI                     PROM     48       "
K824-501   8D    KONAMI                     PROM     42       "
8039       4B                               PROM     63     TBP24S10  (823126)
                                            PROM     33       "
                                            PAL16L8  63
                                            PAL16L8A 67

VERSION 3 IS ON THE SAME SIZE CONNECTOR BOARD, BUT THE BOTTOM
BOARD IS ABOUT 1 1/4" LONGER AND WIDER

THE CHIPS THAT HAVE THE DESIGNATION SCRATCHED OFF ON THE ORIGINAL
BOARDS ARE      NAME          CHIP TYPE
            ---------------------------
CONN BOARD      IC3             TMP8039P-6
 "              IC6             AY3-8910
 "              IC26            Z-80
 "              IC39            MC68A09EP (CUSTOM ON ORIGINAL)
 "              IC27            N/U       (CUSTOM ON ORIGINAL)
REAR BOARD      1C026           N/U       (CUSTOM ON ORIGINAL)


**************************************************************************

Mega Zone, Konami 1983
Hardware info by Guru
Note: Revision documented here is the Konami/Interlogic/Kosuka
version with hand-written labels and Program Code H software.


Bottom Board
------------
KONAMI GX319
PWB(A)2000079B
    |--------------|                |--------------|
|---|--------------|----------------|--------------|---|
|                                                      |
|                                                      |
|319B18.IC091                                          |
|         18.432kHz                                    |
|                                                      |
|                                2114   2114           |
|                                                      |
|                                2114   2114           |
|                                                      |
|                                                      |
|                                                      |
|319B17.IC063                                          |
|                                                      |
|               319G13.IC058                           |
|                                                      |
|               319G12.IC046    319B15.IC048  8128     |
|                                                      |
|                               319B14.IC042  8128     |
|               319B16.IC033                           |
| 2148   2148   |-------|                              |
|               |  083  |                              |
|               |-------|                              |
|                                                      |
|                                                      |
|                               319E11.IC015           |
|                     319E10.IC014                     |
|                               319E09.IC005           |
|                     319E08.IC004              CN001  |
|------------------------------------------------------|
Notes:
        2148 - Fujitsu MBM2148 1kBx4-bit SRAM
        2114 - Toshiba TMM2114 1kBx4-bit SRAM
        8128 - Fujitsu MB8128 2kBx8-bit SRAM (compatible with 6116)
       CN001 - 2 pin connector for 5V/GND coming from top board
         083 - Konami custom chip 083 (contains only logic). Can be replaced with Konami daughterboard 'PWB 400322'
319G13.IC058 \ 2764 8kBx8-bit EPROM (background tiles)
319G12.IC046 /
319E08.IC004 \
319E09.IC005  \ 2764 8kBx8-bit EPROM (sprites)
319E10.IC014  /
319E11.IC015 /
319B18.IC091 - Texas Instruments TBP18S030 32x8-bit bipolar PROM (palette)
319B17.IC063 - MMI 6301 256x4-bit bipolar PROM (character lookup table)
319B16.IC033 - MMI 6301 256x4-bit bipolar PROM (sprite lookup table)
319B15.IC048 - Texas Instruments TBP18S030 32x8-bit bipolar PROM (timing)
319B14.IC042 - Texas Instruments TBP18S030 32x8-bit bipolar PROM (timing)


Top Board
---------
KONAMI GX319
PWB(B)2000082A
    |--------------|                |--------------|
|---|--------------|----------------|--------------|---|
|                      MC1455                          |
|                                                      |
|                                                      |
|                                                      |
|                              |--------------|        |
|                              |   KONAMI-1   |        |
|                              |--------------|        |
|  14.31818MHz                                  DIP28  |
|                                                      |
|  |--------------|  |-------|              319H07.IC59|
|  |  AY-3-8910   |  |  501  |                         |
|  |--------------|  |-------|              319H06.IC58|
|                                       MB8128         |
|                     |--------------|      319H05.IC57|
|            M5224    |     Z80      |                 |
|                     |--------------|      319H04.IC56|
|                     319E02.IC25                      |
|                                           319H03.IC55|
|  |--------------|                                    |
|  |    8039      |                                    |
|  |--------------|                                    |
|                                              DSW2    |
|                VOL                                   |
| 319E01.IC2                                           |
|          LA4460                              DSW1    |
|                                                      |
|                                                      |
|                      18-WAY                   CN5    |
|------------------------------------------------------|
Notes:
        8128 - Fujitsu MB8128 2kBx8-bit SRAM (compatible with 6116)
         CN5 - 2 pin connector for 5V/GND tied to bottom board
         501 - Konami custom chip 501 (contains only logic). Can be replaced with Konami daughterboard 'PWB 402034'
  319E01.IC2 - 2732 4kBx8-bit EPROM (8039 program)
 319E02.IC25 - 2764 8kBx8-bit EPROM (Z80 sound program)
 319H03.IC55 \
 319H04.IC56  \
 319H05.IC57   \ 2764 8kBx8-bit EPROM (main program for custom KONAMI-1 CPU)
 319H06.IC58  /
 319H07.IC59 /
       DIP28 - Empty DIP28 socket
      LA4460 - Sanyo LA4460 12W Audio Power Amplifier
      MC1455 - Motorola MC1455 Timer (compatible with NE555)
       M5224 - Mitsubishi M5224 Quad Operational Amplifier (compatible with LM324)
         Z80 - Z80 CPU. Clock input 3.072MHz [18.432/6]
        8039 - Intel 8039 ROM-less microcontroller with 128 bytes internal RAM. Clock input 7.15909MHz [14.31818/2]
   AY-3-8910 - General Instrument AY-3-8910 Programmable Sound Generator (PSG). Clock input 1.7897725MHz [14.31818/8]
    KONAMI-1 - Konami custom encrypted 6809 CPU. Clock input 1.536MHz on pins 15 & 17 [18.432/12]
       HSync - 15.4566kHz  \
       VSync - 60.606059Hz / Actual PCB measurements

***************************************************************************/


#include "emu.h"

#include "konamipt.h"
#include "konami1.h"

#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "video/resnet.h"


namespace {

class megazone_state : public driver_device
{
public:
	megazone_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_scrolly(*this, "scrolly"),
		m_scrollx(*this, "scrollx"),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_spriteram(*this, "spriteram"),
		m_share1(*this, "share1"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_daccpu(*this, "daccpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_filter(*this, "filter.0.%u", 0U)
	{ }

	void megazone(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_scrolly;
	required_shared_ptr<uint8_t> m_scrollx;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_share1;

	// video-related
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	bool m_flipscreen = 0;

	// misc
	uint8_t m_i8039_status = 0;
	bool m_irq_mask = false;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8039_device> m_daccpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<filter_rc_device, 3> m_filter;

	void i8039_irq_w(uint8_t data);
	void i8039_irqen_and_status_w(uint8_t data);
	void coin_counter_1_w(int state);
	void coin_counter_2_w(int state);
	void irq_mask_w(int state);
	void flipscreen_w(int state);
	uint8_t port_a_r();
	void port_b_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void i8039_io_map(address_map &map) ATTR_COLD;
	void i8039_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************
Based on driver from MAME 0.55
Changes by Martin M. (pfloyd@gmx.net) 14.10.2001:

 - Added support for screen flip in cocktail mode (tricky!) */


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Megazone has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
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

void megazone_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 1000, 0,
			3, &resistances_rg[0], gweights, 1000, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		// red component
		int bit0 = BIT(color_prom[i], 0);
		int bit1 = BIT(color_prom[i], 1);
		int bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// sprites
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	// characters
	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void megazone_state::flipscreen_w(int state)
{
	m_flipscreen = state;
}

void megazone_state::video_start()
{
	m_tmpbitmap = std::make_unique<bitmap_ind16>(256, 256);

	save_item(NAME(*m_tmpbitmap));
}


uint32_t megazone_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// for every character in the Video RAM
	for (int offs = m_videoram[0].bytes() - 1; offs >= 0; offs--)
	{
		int sx = offs % 32;
		int sy = offs / 32;
		int flipx = m_colorram[0][offs] & (1 << 6);
		int flipy = m_colorram[0][offs] & (1 << 5);

		if (m_flipscreen)
		{
			sx = 31 - sx;
			sy = 31 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->opaque(*m_tmpbitmap, m_tmpbitmap->cliprect(),
				((int)m_videoram[0][offs]) + ((m_colorram[0][offs] & (1 << 7) ? 256 : 0) ),
				(m_colorram[0][offs] & 0x0f) + 0x10,
				flipx, flipy,
				8 * sx, 8 * sy);
	}

	// copy the temporary bitmap to the screen
	{
		int scrollx;
		int scrolly;

		if (m_flipscreen)
		{
			scrollx = *m_scrolly;
			scrolly = *m_scrollx;
		}
		else
		{
			scrollx = - *m_scrolly + 4 * 8; // leave space for credit&score overlay
			scrolly = - *m_scrollx;
		}

		copyscrollbitmap(bitmap, *m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
	}


	// Draw the sprites.
	{
		for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
		{
			int sx = m_spriteram[offs + 3];
			int sy = 255 - ((m_spriteram[offs + 1] + 16) & 0xff);
			int color =  m_spriteram[offs + 0] & 0x0f;
			int flipx = ~m_spriteram[offs + 0] & 0x40;
			int flipy =  m_spriteram[offs + 0] & 0x80;

			if (m_flipscreen)
			{
				sx = sx - 11;
				sy = sy + 2;
			}
			else
				sx = sx + 32;

			m_gfxdecode->gfx(0)->transmask(bitmap, cliprect,
					m_spriteram[offs + 2],
					color,
					flipx, flipy,
					sx, sy,
					m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));
		}
	}

	for (int y = 0; y < 32;y++)
	{
		int offs = y * 32;
		for (int x = 0; x < 6; x++)
		{
			int sx = x;
			int sy = y;

			int flipx = m_colorram[1][offs] & (1 << 6);
			int flipy = m_colorram[1][offs] & (1 << 5);

			if (m_flipscreen)
			{
				sx = 35 - sx;
				sy = 31 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(1)->opaque(bitmap, cliprect,
					((int)m_videoram[1][offs]) + ((m_colorram[1][offs] & (1 << 7) ? 256 : 0) ),
					(m_colorram[1][offs] & 0x0f) + 0x10,
					flipx, flipy,
					8 * sx, 8 * sy);
			offs++;
		}
	}
	return 0;
}


uint8_t megazone_state::port_a_r()
{
	// main xtal 14.318MHz, divided by 8 to get the AY-3-8910 clock, further divided by 1024 to get this timer
	// The base clock for the CPU and 8910 is NOT the same, so we have to compensate.
	// (divide by (1024/2), and not 1024, because the CPU cycle counter is incremented every other state change of the clock)

	int clock = m_audiocpu->total_cycles() * 7159 / 12288;    // = (14318 / 8) / (18432 / 6)
	int timer = (clock / (1024 / 2)) & 0x0f;

	// low three bits come from the 8039
	return (timer << 4) | m_i8039_status;
}

void megazone_state::port_b_w(uint8_t data)
{
	for (int i = 0; i < 3; i++)
	{
		int C = 0;
		if (data & 1)
			C +=  10000;    //  10000pF = 0.01uF
		if (data & 2)
			C += 220000;    // 220000pF = 0.22uF

		data >>= 2;
		m_filter[i]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 200, CAP_P(C));
	}
}

void megazone_state::i8039_irq_w(uint8_t data)
{
	m_daccpu->set_input_line(0, ASSERT_LINE);
}

void megazone_state::i8039_irqen_and_status_w(uint8_t data)
{
	if ((data & 0x80) == 0)
		m_daccpu->set_input_line(0, CLEAR_LINE);
	m_i8039_status = (data & 0x70) >> 4;
}

void megazone_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void megazone_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

void megazone_state::irq_mask_w(int state)
{
	m_irq_mask = state;
}


void megazone_state::main_map(address_map &map)
{
	map(0x0000, 0x0007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x0800, 0x0800).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1000, 0x1000).writeonly().share(m_scrolly);
	map(0x1800, 0x1800).writeonly().share(m_scrollx);
	map(0x2000, 0x23ff).ram().share(m_videoram[0]);
	map(0x2400, 0x27ff).ram().share(m_videoram[1]);
	map(0x2800, 0x2bff).ram().share(m_colorram[0]);
	map(0x2c00, 0x2fff).ram().share(m_colorram[1]);
	map(0x3000, 0x33ff).ram().share(m_spriteram);
	map(0x3800, 0x3fff).lrw8([this](offs_t off) { return m_share1[off]; }, "share_r", [this](offs_t off, u8 data) { m_share1[off] = data; }, "share_w");
	map(0x4000, 0xffff).rom();     // 4000->5FFF is a debug ROM
}

void megazone_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2000).w(FUNC(megazone_state::i8039_irq_w)); // START line. Interrupts 8039
	map(0x4000, 0x4000).w("soundlatch", FUNC(generic_latch_8_device::write));            // CODE  line. Command Interrupts 8039
	map(0x6000, 0x6000).portr("IN0");            // IO Coin
	map(0x6001, 0x6001).portr("IN1");            // P1 IO
	map(0x6002, 0x6002).portr("IN2");            // P2 IO
	map(0x8000, 0x8000).portr("DSW2");
	map(0x8001, 0x8001).portr("DSW1");
	map(0xa000, 0xa000).nopw();                    // INTMAIN - Interrupts main CPU (unused)
	map(0xc000, 0xc000).nopw();                    // INT (Actually is NMI) enable/disable (unused)
	map(0xc001, 0xc001).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xe000, 0xe7ff).ram().share(m_share1);
}

void megazone_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x00, 0x02).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x02).w("aysnd", FUNC(ay8910_device::data_w));
}

void megazone_state::i8039_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void megazone_state::i8039_io_map(address_map &map)
{
	map(0x00, 0xff).r("soundlatch", FUNC(generic_latch_8_device::read));
}

static INPUT_PORTS_START( megazone )
	// 0x6000 -> 0xe320 (CPU1) = 0x3b20 (CPU0)
	PORT_START("IN0")
	KONAMI8_SYSTEM_UNK

	// 0x6001 -> 0xe31e (CPU1) = 0x3b1e (CPU0)
	PORT_START("IN1")
	KONAMI8_MONO_B1_UNK

	// 0x6002 -> 0xe31e (CPU1) = 0x3b1e (CPU0) or 0xe31f (CPU1) = 0x3b1f (CPU0) in "test mode"
	PORT_START("IN2")
	KONAMI8_COCKTAIL_B1_UNK

	// 0x8001 -> 0xe021 (CPU1) = 0x3821 (CPU0)
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	// 0x8000 -> 0xe020 (CPU1) = 0x3820 (CPU0)
	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k 70k 70k+" )
	PORT_DIPSETTING(    0x10, "20k 80k 80k+" )
	PORT_DIPSETTING(    0x08, "30k 90k 90k+" )
	PORT_DIPSETTING(    0x00, "30k 100k 100k+" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( megazona )
	PORT_INCLUDE( megazone )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	512,    // 512 characters
	4,      // 4 bits per pixel
	{ 0, 1, 2, 3 }, // the four bitplanes are packed in one nibble
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    // every char takes 8 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	256,    // 256 sprites
	4,  // 4 bits per pixel
	{ 0x4000*8+4, 0x4000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    // every sprite takes 64 consecutive bytes
};

static GFXDECODE_START( gfx_megazone )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   16*16, 16 )
GFXDECODE_END

void megazone_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_i8039_status));
	save_item(NAME(m_irq_mask));
}

void megazone_state::machine_reset()
{
	m_i8039_status = 0;
}

void megazone_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(0, HOLD_LINE);

	if (state)
		m_audiocpu->set_input_line(0, HOLD_LINE);
}


void megazone_state::megazone(machine_config &config)
{
	// basic machine hardware
	KONAMI1(config, m_maincpu, XTAL(18'432'000) / 12);        // 1.536 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &megazone_state::main_map);

	Z80(config, m_audiocpu, XTAL(18'432'000) / 6);     // Z80 Clock is derived from the H1 signal
	m_audiocpu->set_addrmap(AS_PROGRAM, &megazone_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &megazone_state::sound_io_map);

	I8039(config, m_daccpu, XTAL(14'318'181) / 2);    // 7.15909 MHz
	m_daccpu->set_addrmap(AS_PROGRAM, &megazone_state::i8039_map);
	m_daccpu->set_addrmap(AS_IO, &megazone_state::i8039_io_map);
	m_daccpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_daccpu->p2_out_cb().set(FUNC(megazone_state::i8039_irqen_and_status_w));

	config.set_maximum_quantum(attotime::from_hz(900));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 13A
	mainlatch.q_out_cb<0>().set(FUNC(megazone_state::coin_counter_2_w));
	mainlatch.q_out_cb<1>().set(FUNC(megazone_state::coin_counter_1_w));
	mainlatch.q_out_cb<5>().set(FUNC(megazone_state::flipscreen_w));
	mainlatch.q_out_cb<7>().set(FUNC(megazone_state::irq_mask_w));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.606060);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(36*8, 32*8);
	screen.set_visarea(0*8, 36*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(megazone_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(megazone_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_megazone);
	PALETTE(config, m_palette, FUNC(megazone_state::palette), 16*16+16*16, 32);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(14'318'181) / 8));
	aysnd.port_a_read_callback().set(FUNC(megazone_state::port_a_r));
	aysnd.port_b_write_callback().set(FUNC(megazone_state::port_b_w));
	aysnd.add_route(0, "filter.0.0", 0.30);
	aysnd.add_route(1, "filter.0.1", 0.30);
	aysnd.add_route(2, "filter.0.2", 0.30);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // R-2R ladder network

	FILTER_RC(config, m_filter[0]).add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_RC(config, m_filter[1]).add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_RC(config, m_filter[2]).add_route(ALL_OUTPUTS, "speaker", 1.0);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( megazone )
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "319_l07.11h",  0x6000, 0x2000, CRC(73b616ca) SHA1(ecdcdd085020f8ffe87a574832f7cedb9bcacef9) )
	ROM_LOAD( "319_l06.9h",   0x8000, 0x2000, CRC(0ced03f9) SHA1(4c8688b7bde5ee1adfe6a0db7178cca046eca7f4) )
	ROM_LOAD( "319_l05.8h",   0xa000, 0x2000, CRC(9dc3b5a1) SHA1(d6373f5be06cd61b6d3ffbe36c160167ba9852f3) )
	ROM_LOAD( "319_l04.7h",   0xc000, 0x2000, CRC(785b983d) SHA1(389e2f5494284089d39249e91293f3998c2b22c0) )
	ROM_LOAD( "319_l03.6h",   0xe000, 0x2000, CRC(a5318686) SHA1(8c8fbb76a36108f4a0b3a50e8a9b1781508490f1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     // 4k for the 8039 DAC CPU
	ROM_LOAD( "319e01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_g12.8c",   0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) // palette
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) // sprite lookup table
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) // character lookup table
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) // timing (not used)
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) // timing (not used)
ROM_END

ROM_START( megazonej ) // Interlogic + Kosuka license set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319_j07.11h",  0x6000, 0x2000,  CRC(5161a523) SHA1(90b456c30bccaaca96c75c2f421af3a2875b0b6b) )
	ROM_LOAD( "319_j06.9h",   0x8000, 0x2000,  CRC(7344c3de) SHA1(d3867738d4828afa50c8b43116d68cc6074d6cb5) )
	ROM_LOAD( "319_j05.8h",   0xa000, 0x2000,  CRC(affa492b) SHA1(ee6789f293902716d65d08a89ae12dd96c75c885) )
	ROM_LOAD( "319_j04.7h",   0xc000, 0x2000,  CRC(03544ab3) SHA1(efa034cc6976b47915601cf215758df23e308878) )
	ROM_LOAD( "319_j03.6h",   0xe000, 0x2000,  CRC(0d95cc0a) SHA1(9aadadf09a4826da451ee35c89ee0254ec552d80) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     // 4k for the 8039 DAC CPU
	ROM_LOAD( "319e01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_g12.8c",   0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) // palette
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) // sprite lookup table
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) // character lookup table
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) // timing (not used)
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) // timing (not used)
ROM_END

ROM_START( megazonei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319_i07.11h",  0x6000, 0x2000, CRC(94b22ea8) SHA1(dc3ed2a0d1a12df51e46561324d78b7d655be313) )
	ROM_LOAD( "319_i06.9h",   0x8000, 0x2000, CRC(0468b619) SHA1(a6755728fab37674749f9b77cb53f6f228102f2f) )
	ROM_LOAD( "319_i05.8h",   0xa000, 0x2000, CRC(ac59000c) SHA1(c7568589f6b0e1706e996fdfed9c16755541951e) )
	ROM_LOAD( "319_i04.7h",   0xc000, 0x2000, CRC(1e968603) SHA1(fd818f678a3dc8d48a30f9f7670bfcb42a3009a2) )
	ROM_LOAD( "319_i03.6h",   0xe000, 0x2000, CRC(0888b803) SHA1(37249bfb14c6c3ce40ad68be457ab1f66fd7ea70) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     // 4k for the 8039 DAC CPU
	ROM_LOAD( "319e01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_e12.8c",   0x0000, 0x2000, CRC(e0fb7835) SHA1(44ccaaf92bdb83323f45e08dbe118697720e9105) )
	ROM_LOAD( "319_e13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) )

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) // palette
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) // sprite lookup table
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) // character lookup table
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) // timing (not used)
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) // timing (not used)
ROM_END

ROM_START( megazoneh ) // Kosuka license set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319_h07.11h",  0x6000, 0x2000,  CRC(8ca47f64) SHA1(1a20db5ac504b9b004116cfa6992d63a86a04cc5) )
	ROM_LOAD( "319_h06.9h",   0x8000, 0x2000,  CRC(ed35b12e) SHA1(69e88c4801c838a24aba0a867af205a7169ad089) )
	ROM_LOAD( "319_h05.8h",   0xa000, 0x2000,  CRC(c3655ccd) SHA1(b86b58a12c6ced9a7e0a6d0cdb3881a28220a650) )
	ROM_LOAD( "319_h04.7h",   0xc000, 0x2000,  CRC(9e221177) SHA1(0c6fffd657d66090362108578aa78eb36bdcce6b) )
	ROM_LOAD( "319_h03.6h",   0xe000, 0x2000,  CRC(9048955b) SHA1(d8a7b46d984832f566298d3b417b3a34c9fea6c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     // 4k for the 8039 DAC CPU
	ROM_LOAD( "319h01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) ) // same as e01

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_g12.8c",   0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) // palette
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) // sprite lookup table
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) // character lookup table
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) // timing (not used)
	ROM_LOAD( "prom.48",     0x0240, 0x020, CRC(796dea94) SHA1(bab3c2a5466e1c07ec27cccf7b1a21e9de4ed982) ) // timing (not used)
ROM_END

ROM_START( megazonea ) // Interlogic + Kosuka license set.
// A second dump was made from a PCB with all hand written labels on both top and bottom boards. The labels for the second PCB are noted in the comments. Curiously, they seem to point at this being version H, but they differ from megazoneh
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "ic59_cpu.bin",  0x6000, 0x2000, CRC(f41922a0) SHA1(9f54509da18721a76593921c6e52085e62e6ea6b) ) // need to correct program / region code ID // 2nd PCB: 319h07.ic59
	ROM_LOAD( "ic58_cpu.bin",  0x8000, 0x2000, CRC(7fd7277b) SHA1(e773247e0c9419cae49e04962ea362a2976c2db2) ) // 2nd PCB: 319h06.ic58
	ROM_LOAD( "ic57_cpu.bin",  0xa000, 0x2000, CRC(a4b33b51) SHA1(12bb4da0319a7fe355e5ea4945759c8709aed5fe) ) // 2nd PCB: 319h05.ic57
	ROM_LOAD( "ic56_cpu.bin",  0xc000, 0x2000, CRC(2aabcfbf) SHA1(f0054af98bd68158eab3328f8cf2a04b35e812c7) ) // 2nd PCB: 319h04.ic56
	ROM_LOAD( "ic55_cpu.bin",  0xe000, 0x2000, CRC(b33a3c37) SHA1(2f1fdf1b9f18fcc9bd97cc9adeecc4ce77dd30c9) ) // 2nd PCB: 319h03.ic55

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) ) // 2nd PCB: 319e02.ic25

	ROM_REGION( 0x1000, "daccpu", 0 )     // 4k for the 8039 DAC CPU
	ROM_LOAD( "319e01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) ) // 2nd PCB: 319e01.ic2

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) ) // 2nd PCB: 319e11.ic015
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) ) // 2nd PCB: 319e09.ic005
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) ) // 2nd PCB: 319e10.ic014
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) ) // 2nd PCB: 319e08.ic004

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_g12.8c",   0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) ) // 2nd PCB: 319g12.ic046
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13 // 2nd PCB: 319g13.ic058

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) // palette
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) // sprite lookup table
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) // character lookup table
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) // timing (not used)
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) // timing (not used)
ROM_END

ROM_START( megazoneb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7.11h",  0x6000, 0x2000, CRC(d42d67bf) SHA1(adac80d183ad26a9b1ec25a2da7ebbb33b441b63) ) // need to correct program / region code ID
	ROM_LOAD( "6.9h",   0x8000, 0x2000, CRC(692398eb) SHA1(518001d738c2fb9417e52edfe9a7b74a074af3b0) )
	ROM_LOAD( "5.8h",   0xa000, 0x2000, CRC(620ffec3) SHA1(e047beb29e0cda72126e8dcdd0b7504a202efba2) )
	ROM_LOAD( "4.7h",   0xc000, 0x2000, CRC(28650971) SHA1(25e405fb9f648b7118e3c7c7b3ba59a7b7c29c42) )
	ROM_LOAD( "3.6h",   0xe000, 0x2000, CRC(f264018f) SHA1(6ca0f7e26311799b0650a6c58567405bc2a7f922) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319h02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) ) // same as e02

	ROM_REGION( 0x1000, "daccpu", 0 )     // 4k for the 8039 DAC CPU
	ROM_LOAD( "319h01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) ) // same as e01

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_e12.8c",   0x0000, 0x2000, CRC(e0fb7835) SHA1(44ccaaf92bdb83323f45e08dbe118697720e9105) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) // palette
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) // sprite lookup table
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) // character lookup table
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) // timing (not used)
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) // timing (not used)
ROM_END

} // anonymous namespace


GAME( 1983, megazone, 0,         megazone, megazone, megazone_state, empty_init, ROT90, "Konami",                               "Mega Zone (program code L)",         MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazonej, megazone, megazone, megazone, megazone_state, empty_init, ROT90, "Konami (Interlogic / Kosuka license)", "Mega Zone (program code J)",         MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazonei, megazone, megazone, megazone, megazone_state, empty_init, ROT90, "Konami",                               "Mega Zone (program code I)",         MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazoneh, megazone, megazone, megazone, megazone_state, empty_init, ROT90, "Konami (Kosuka license)",              "Mega Zone (program code H)",         MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazonea, megazone, megazone, megazone, megazone_state, empty_init, ROT90, "Konami (Interlogic / Kosuka license)", "Mega Zone (unknown program code 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazoneb, megazone, megazone, megazona, megazone_state, empty_init, ROT90, "Konami",                               "Mega Zone (unknown program code 2)", MACHINE_SUPPORTS_SAVE )
