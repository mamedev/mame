// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Custom ICs:
----------
Dig Dug:
-------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
51XX     I/O
53XX     I/O

Video board:
00XX     tilemap address generator
02XX     gfx data shifter and mixer (16-bit in, 4-bit out)
04XX     sprite address generator
07XX     clock divider


Memory maps:
-----------
Dig Dug:
-------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 0     program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 1     program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 2     program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM 3     program ROM
the rest of the memory map is common to the other CPUs

SUB CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 4     program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 5     program ROM
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 6     program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 7     program ROM (optional, not used)
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
01101-----00----   W ----xxxx AUDIO 0   \ sound control registers
01101-----01----   W ----xxxx AUDIO 1   /
01101-----10-000   W -------x IRQ1      main CPU IRQ enable/acknowledge
01101-----10-001   W -------x IRQ2      sub CPU IRQ enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU NMI enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x MOD 0     \
01101-----10-110   W -------x MOD 1     | to custom 53XX
01101-----10-111   W -------x MOD 2     /
01101-----11----   W -------- WDDIS     watchdog reset
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
01111-----------              n.c.
10000xxxxxxxxxxx R/W xxxxxxxx RAM 0     tilemap RAM + work RAM
10001-xxxxxxxxxx R/W xxxxxxxx OBJRAM    work RAM
10001-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (sprite number and color)
10010-xxxxxxxxxx R/W xxxxxxxx POSRAM    work RAM
10010-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (x and y)
10011-xxxxxxxxxx R/W xxxxxxxx FLPRAM    work RAM
10011-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (flip)
10100--------000   W -------x           \ background ROM (114) bank select
10100--------001   W -------x           /
10100--------010   W -------x           tilemap color select (low or high 4 bits of tilemap RAM)
10100--------011   W -------x           background enable
10100--------100   W -------x           \ background color lookup PROM (112) bank select
10100--------101   W -------x           /
10100--------110   W -------x n.c.
10100--------111   W -------x FLIP      flip screen
10101-----------              n.c.
10110-----------              n.c.
10111----0xxxxxx   W xxxxxxxx EAROM     non volatile memory address latch and data write
10111----0------ R   xxxxxxxx EAROM     non volatile memory read
10111----1------   W ----xxxx EAROM     non volatile memory control



Namco vs Atari ROM names and locations
--------------------------------------
The Namco version is composed of two boards, while the Atari version is
single board. There are two revisions of the Atari version.

Location  ID        Location  Location  ID
                    (type 1)  (type 2)
--------  ----      --------  --------  ----------
CPU 3P    DD1-1     6L        2C/D      136007-101
CPU 3M    DD1-2     6M        2E        136007-102
CPU 2M    DD1-3     6N/P      2B/C      136007-103
CPU 2L    DD1-4     6R        2A        136007-104
CPU 3F    DD1-5     6C        2P        136007-105
CPU 3J    DD1-6     6D        2N        136007-106
CPU 2C    DD1-7     5L        2K/L      136007-107
CPU 5N    [bpr]     2K/L      10A       136007-109
CPU 7N    [bpr]     2P        11A       136007-110

VID 2C    DD1-9     8R        5K        136007-108
VID 1C    [bpr]     4G        8F        136007-111
VID 2N    [bpr]     10K/L     4N        136007-112
VID 5N    [bpr]     1R        8L        136007-113
VID 2D    DD1-10    9N        4J        136007-114
VID 5C    DD1-11    10C/D     4F        136007-115
VID 5F    DD1-12    7A/B      5B        136007-119
VID 5H    DD1-13    8A/B      5A        136007-118
VID 5J    DD1-14    7C        5C        136007-117
VID 5K    DD1-15    8C        5D        136007-116



Easter eggs:
-----------
- Dig Dug:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    6xU 3xR 4xD 8xL
  (c) 1982 NAMCO LTD. will appear on the screen.


Notes:
-----
- The Cabinet Type "DIP switch" actually comes from the edge connector, but is mapped
  in memory in place of dip switch #8. DIP switch #8 selects single/dual coin counters
  and is entirely handled by hardware.

- digdug: if you enter service mode and press press service coin something like
  the following is written at the bottom of the screen:
  99.9999.9999.9999.9999.
  This is explained in the manual: it is the number of games played, of points, etc.
  The counters start from 999 and count backwards.

- differences between versions of digdug:
  - the background graphics are slightly different in the Atari versions; the earth is
    less regular.

  - "digduga1" is identical to "digdugb", apart from the gfx and copyright notices
    changed from "NAMCO LTD." to "ATARI INC.".

  - "digdug" fixes two bugs that were present in "digdugb":  First, as monster speed
    increased in later rounds it could eventually roll over to 0, causing the monsters
    to stop moving altogether.  Second, "double-killing" a monster by bursting it and
    immediately dropping a rock on the corpse could result in the round not ending
    even after all monsters were killed.
    This set also has the code to save high scores to EEPROM rewritten, though the
    reason for the changes is unclear.

  - "digdugat" is almost identical to "digdug" (apart from the Atari gfx/copyright
    changes), but there are three added instructions in the CPU0 program that change
    the code alignment.  The change eliminates the "kill screen" at round 256 by
    making the round number roll over to 156, and hides the rollover from the player
    by only ever displaying the lower two digits of the round number.  Interestingly,
    "digdug" actually contains all the code to implement the rollover (at $0018-$0026)
    but just doesn't call it, implying that Namco deliberately chose to keep the kill
    screen in this version.

  - "digsid" is intermediate between "digdugb" and "digdug"; it has the changed EEPROM
    handling, but not the gameplay bug fixes.  It has some unique changes as well:
    the initial high scores are 25000 instead of 10000, and the game begins on the
    screen that is round 4 in the other sets, skipping the first three screens.
    The latter change seems likely to have been done by Namco themselves and not by
    Sidam, as it involves insertion of code right in the middle of the CPU0 program
    and realignment of all the code after the insertion.

  - "dzigzag" and "digdugb" are identical, apart from the hacked gfx and the copyright
    notices changed from "NAMCO LTD." to "1 9 8 2".  It's a bootleg of "digdugb", and
    not of "digduga1", because the hidden "NAMCO" string at offset 0x1eea of CPU2 is
    still present, while it is replaced by "ATARI" in digduga1.
    The only interesting thing about the bootleg is the 4th Z80, used to simulate
    the custom 5xXX chips of the original.


TODO:
----
- dzigzag: emulate the 4th CPU (should be similar to battles)

***************************************************************************/
#include "emu.h"
#include "galaga.h"

#include "namco06.h"
#include "namco51.h"
#include "namco53.h"

#include "cpu/z80/z80.h"
#include "machine/er2055.h"
#include "machine/watchdog.h"
#include "video/resnet.h"

#include "tilemap.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK (XTAL(18'432'000)) // same as galaga.cpp


class digdug_state : public galaga_state
{
public:
	digdug_state(const machine_config &mconfig, device_type type, const char *tag) :
		galaga_state(mconfig, type, tag),
		m_earom(*this, "earom"),
		m_digdug_objram(*this, "digdug_objram"),
		m_digdug_posram(*this, "digdug_posram"),
		m_digdug_flpram(*this, "digdug_flpram")
	{ }

	void dzigzag(machine_config &config) ATTR_COLD;
	void digdug(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	void digdug_palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update_digdug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void digdug_videoram_w(offs_t offset, uint8_t data);
	void bg_select_w(uint8_t data);
	void tx_color_mode_w(int state);
	void bg_disable_w(int state);

	uint8_t earom_read();
	void earom_write(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);

	void digdug_map(address_map &map) ATTR_COLD;
	void dzigzag_mem4(address_map &map) ATTR_COLD;

	required_device<er2055_device> m_earom;
	required_shared_ptr<uint8_t> m_digdug_objram;
	required_shared_ptr<uint8_t> m_digdug_posram;
	required_shared_ptr<uint8_t> m_digdug_flpram;

	uint8_t m_bg_select = 0U;
	uint8_t m_tx_color_mode = 0U;
	uint8_t m_bg_disable = 0U;
	uint8_t m_bg_color_bank = 0U;
};


static const gfx_layout charlayout_2bpp = // same as galaga.cpp
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8) },
	16*8
};

static const gfx_layout charlayout_digdug =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(7,-1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_digdug )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_digdug,         0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_galaga,    16*2, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout_2bpp, 64*4 + 16*2, 64 )
GFXDECODE_END


void digdug_state::digdug_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().nopw();         /* the only area different for each CPU */
	map(0x6800, 0x681f).w(m_namco_sound, FUNC(namco_wsg_device::pacman_sound_w));
	map(0x6820, 0x6827).w("misclatch", FUNC(ls259_device::write_d0));
	map(0x6830, 0x6830).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x7000, 0x70ff).rw("06xx", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x7100, 0x7100).rw("06xx", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
	map(0x8000, 0x83ff).ram().w(FUNC(digdug_state::digdug_videoram_w)).share("videoram"); /* tilemap RAM (bottom half of RAM 0 */
	map(0x8400, 0x87ff).ram().share("share1");                          /* work RAM (top half for RAM 0 */
	map(0x8800, 0x8bff).ram().share(m_digdug_objram);   /* work RAM + sprite registers */
	map(0x9000, 0x93ff).ram().share(m_digdug_posram);   /* work RAM + sprite registers */
	map(0x9800, 0x9bff).ram().share(m_digdug_flpram);   /* work RAM + sprite registers */
	map(0xa000, 0xa007).nopr().w(m_videolatch, FUNC(ls259_device::write_d0));   /* video latches (spurious reads when setting latch bits) */
	map(0xb800, 0xb83f).rw(FUNC(digdug_state::earom_read), FUNC(digdug_state::earom_write));   /* non volatile memory data */
	map(0xb840, 0xb840).w(FUNC(digdug_state::earom_control_w));                    /* non volatile memory control */
}

/* bootleg 4th CPU replacing the 5xXX chips */
void digdug_state::dzigzag_mem4(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x107f).ram();
	map(0x4000, 0x4007).readonly();    // DIP switches? bits 0 & 1 used
}


void digdug_state::digdug(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &digdug_state::digdug_map);

	Z80(config, m_subcpu, MASTER_CLOCK/6);    /* 3.072 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &digdug_state::digdug_map);

	Z80(config, m_subcpu2, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_subcpu2->set_addrmap(AS_PROGRAM, &digdug_state::digdug_map);

	ls259_device &misclatch(LS259(config, "misclatch")); // 8R
	misclatch.q_out_cb<0>().set(FUNC(digdug_state::irq1_clear_w));
	misclatch.q_out_cb<1>().set(FUNC(digdug_state::irq2_clear_w));
	misclatch.q_out_cb<2>().set(FUNC(digdug_state::nmion_w));
	misclatch.q_out_cb<3>().set_inputline("sub", INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append_inputline("sub2", INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append("51xx", FUNC(namco_51xx_device::reset));
	misclatch.q_out_cb<3>().append("53xx", FUNC(namco_53xx_device::reset));
	// Q5-Q7 also used (see below)

	namco_51xx_device &n51xx(NAMCO_51XX(config, "51xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n51xx.input_callback<0>().set_ioport("IN0").mask(0x0f);
	n51xx.input_callback<1>().set_ioport("IN0").rshift(4);
	n51xx.input_callback<2>().set_ioport("IN1").mask(0x0f);
	n51xx.input_callback<3>().set_ioport("IN1").rshift(4);
	n51xx.output_callback().set(FUNC(digdug_state::out));
	n51xx.lockout_callback().set(FUNC(digdug_state::lockout));

	namco_53xx_device &n53xx(NAMCO_53XX(config, "53xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n53xx.k_port_callback().set("misclatch", FUNC(ls259_device::q7_r)).lshift(3); // MOD 2 = K3
	n53xx.k_port_callback().append("misclatch", FUNC(ls259_device::q6_r)).lshift(2); // MOD 1 = K2
	n53xx.k_port_callback().append("misclatch", FUNC(ls259_device::q5_r)).lshift(1); // MOD 0 = K1
	// K0 is left unconnected
	n53xx.input_callback<0>().set_ioport("DSWA").mask(0x0f);
	n53xx.input_callback<1>().set_ioport("DSWA").rshift(4);
	n53xx.input_callback<2>().set_ioport("DSWB").mask(0x0f);
	n53xx.input_callback<3>().set_ioport("DSWB").rshift(4);

	namco_06xx_device &n06xx(NAMCO_06XX(config, "06xx", MASTER_CLOCK/6/64));
	n06xx.set_maincpu(m_maincpu);
	n06xx.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));
	n06xx.chip_select_callback<1>().set("53xx", FUNC(namco_53xx_device::chip_select));
	n06xx.read_callback<1>().set("53xx", FUNC(namco_53xx_device::read));

	LS259(config, m_videolatch); // 5R
	m_videolatch->parallel_out_cb().set(FUNC(digdug_state::bg_select_w)).mask(0x33);
	m_videolatch->q_out_cb<2>().set(FUNC(digdug_state::tx_color_mode_w));
	m_videolatch->q_out_cb<3>().set(FUNC(digdug_state::bg_disable_w));
	m_videolatch->q_out_cb<7>().set(FUNC(digdug_state::flip_screen_set));

	ER2055(config, m_earom);

	WATCHDOG_TIMER(config, "watchdog");

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_raw(MASTER_CLOCK/3, 384, 0, 288, 264, 0, 224);
	m_screen->set_screen_update(FUNC(digdug_state::screen_update_digdug));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(digdug_state::vblank_irq));
	m_screen->screen_vblank().append("51xx", FUNC(namco_51xx_device::vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_digdug);
	PALETTE(config, m_palette, FUNC(digdug_state::digdug_palette), 16*2 + 64*4 + 64*4, 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NAMCO_WSG(config, m_namco_sound, MASTER_CLOCK/6/32);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0);
}

void digdug_state::dzigzag(machine_config &config)
{
	digdug(config);

	/* basic machine hardware */
	z80_device &sub3(Z80(config, "sub3", MASTER_CLOCK/6));   /* 3.072 MHz */
	sub3.set_addrmap(AS_PROGRAM, &digdug_state::dzigzag_mem4);
}


uint8_t digdug_state::earom_read()
{
	return m_earom->data();
}

void digdug_state::earom_write(offs_t offset, uint8_t data)
{
	m_earom->set_address(offset & 0x3f);
	m_earom->set_data(data);
}

void digdug_state::earom_control_w(uint8_t data)
{
	// CK = DB0, C1 = /DB1, C2 = DB2, CS1 = DB3, /CS2 = GND
	m_earom->set_control(BIT(data, 3), 1, !BIT(data, 1), BIT(data, 2));
	m_earom->set_clk(BIT(data, 0));
}


void digdug_state::machine_start()
{
	galaga_state::machine_start();

	earom_control_w(0);
}


/***************************************************************************

  Convert the color PROMs.

  digdug has one 32x8 palette PROM and two 256x4 color lookup table PROMs
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

void digdug_state::digdug_palette(palette_device &palette) const
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

	// characters - direct mapping
	for (int i = 0; i < 16; i++)
	{
		palette.set_pen_indirect((i << 1) | 0, 0);
		palette.set_pen_indirect((i << 1) | 1, i);
	}

	// sprites
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(16*2 + i, (*color_prom++ & 0x0f) | 0x10);

	// bg_select
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(16*2 + 256 + i, *color_prom++ & 0x0f);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(digdug_state::tilemap_scan)
{
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	else
		return col + (row << 5);
}


TILE_GET_INFO_MEMBER(digdug_state::bg_get_tile_info)
{
	uint8_t *rom = memregion("gfx4")->base();

	int code = rom[tile_index | (m_bg_select << 10)];
	/* when the background is "disabled", it is actually still drawn, but using
	   a color code that makes all pixels black. There are pullups setting the
	   code to 0xf, but also solder pads that optionally connect the lines with
	   tilemap RAM, therefore allowing to pick some bits of the color code from
	   the top 4 bits of alpha code. This feature is not used by Dig Dug. */
	int color = m_bg_disable ? 0xf : (code >> 4);
	tileinfo.set(2,
			code,
			color | m_bg_color_bank,
			0);
}

TILE_GET_INFO_MEMBER(digdug_state::tx_get_tile_info)
{
	uint8_t code = m_videoram[tile_index];
	int color;

	/* the hardware has two ways to pick the color, either straight from the
	   bottom 4 bits of the character code, or from the top 4 bits through a
	   formula. The former method isnot used by Dig Dug and seems kind of
	   useless (I don't know what use they were thinking of when they added
	   it), anyway here it is reproduced faithfully. */
	if (m_tx_color_mode)
		color = code & 0x0f;
	else
		color = ((code >> 4) & 0x0e) | ((code >> 3) & 2);

	/* the hardware has two character sets, one normal and one x-flipped. When
	   screen is flipped, character y flip is done by the hardware inverting the
	   timing signals, while x flip is done by selecting the 2nd character set.
	   We reproduce this here, but since the tilemap system automatically flips
	   characters when screen is flipped, we have to flip them back. */
	tileinfo.set(0,
			(code & 0x7f) | (flip_screen() ? 0x80 : 0),
			color,
			flip_screen() ? TILE_FLIPX : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void digdug_state::video_start()
{
	m_bg_select = 0;
	m_tx_color_mode = 0;
	m_bg_disable = 0;
	m_bg_color_bank = 0;

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(digdug_state::bg_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(digdug_state::tilemap_scan)), 8, 8, 36, 28);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(digdug_state::tx_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(digdug_state::tilemap_scan)), 8, 8, 36, 28);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_bg_select));
	save_item(NAME(m_tx_color_mode));
	save_item(NAME(m_bg_disable));
	save_item(NAME(m_bg_color_bank));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void digdug_state::digdug_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void digdug_state::bg_select_w(uint8_t data)
{
	// select background picture
	if (m_bg_select != (data & 0x03))
	{
		m_bg_select = data & 0x03;
		m_bg_tilemap->mark_all_dirty();
	}

	// background color bank
	if (m_bg_color_bank != (data & 0x30))
	{
		m_bg_color_bank = data & 0x30;
		m_bg_tilemap->mark_all_dirty();
	}
}

void digdug_state::tx_color_mode_w(int state)
{
	// select alpha layer color mode (see tx_get_tile_info)
	m_tx_color_mode = state;
	m_fg_tilemap->mark_all_dirty();
}

void digdug_state::bg_disable_w(int state)
{
	// "disable" background (see bg_get_tile_info)
	m_bg_disable = state;
	m_bg_tilemap->mark_all_dirty();
}



/***************************************************************************

  Display refresh

***************************************************************************/

void digdug_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *spriteram = m_digdug_objram + 0x380;
	uint8_t *spriteram_2 = m_digdug_posram + 0x380;
	uint8_t *spriteram_3 = m_digdug_flpram + 0x380;
	int offs;

	// mask upper and lower columns
	rectangle visarea = cliprect;
	visarea.min_x = 2*8;
	visarea.max_x = 34*8-1;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs];
		int color = spriteram[offs+1] & 0x3f;
		int sx = spriteram_2[offs+1] - 40+1;
		int sy = 256 - spriteram_2[offs] + 1;   // sprites are buffered and delayed by one scanline
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int size  = (sprite & 0x80) >> 7;
		int x,y;

		if (size)
			sprite = (sprite & 0xc0) | ((sprite & ~0xc0) << 2);

		sy -= 16 * size;
		sy = (sy & 0xff) - 32;  // fix wraparound

		if (flip_screen())
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		for (y = 0;y <= size;y++)
		{
			for (x = 0;x <= size;x++)
			{
				uint32_t transmask =  m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x1f);
				m_gfxdecode->gfx(1)->transmask(bitmap,visarea,
					sprite + gfx_offs[y ^ (size * flipy)][x ^ (size * flipx)],
					color,
					flipx,flipy,
					((sx + 16*x) & 0xff), sy + 16*y,transmask);
				/* wraparound */
				m_gfxdecode->gfx(1)->transmask(bitmap,visarea,
					sprite + gfx_offs[y ^ (size * flipy)][x ^ (size * flipx)],
					color,
					flipx,flipy,
					((sx + 16*x) & 0xff) + 0x100, sy + 16*y,transmask);
			}
		}
	}
}


uint32_t digdug_state::screen_update_digdug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	return 0;
}


static INPUT_PORTS_START( digdug )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL

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
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x20, "10K, 40K, Every 40K" )   PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0) // Atari factory default = "10K, 40K, Every40K"
	PORT_DIPSETTING(    0x10, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "10K and 40K Only" )      PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "20K and 60K Only" )      PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0) // Namco factory default = "20K, 60K"
	PORT_DIPSETTING(    0x38, "10K Only" )              PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "30K, 80K, Every 80K" )   PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "20K and 50K Only" )      PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "20K and 60K Only" )      PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K and 70K Only" )      PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "20K Only" )              PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x38, "30K Only" )              PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSWB") // reverse order against SWA
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )                    PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) ) // factory default = "No"
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
INPUT_PORTS_END

/*
static INPUT_PORTS_START( digdugja ) // Namco older?
    PORT_INCLUDE( digdug )

    PORT_MODIFY("DSWB") // same order as SWA
    PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:2,1")
    PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
    PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
    PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
    PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:3")
    PORT_DIPSETTING(    0x04, DEF_STR( No ) ) // Namco factory default = "No"
    PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
    PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x10, "Freeze" )                    PORT_DIPLOCATION("SWB:5")
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x60, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWB:7,6")
    PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
    PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( digdugus ) // Atari older?
    PORT_INCLUDE( digdug )

    PORT_MODIFY("DSWB") // reverse order against SWA
    PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWB:1,2")
    PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
    PORT_DIPNAME( 0x20, 0x20, "Freeze" )                    PORT_DIPLOCATION("SWB:3")
    PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:5")
    PORT_DIPSETTING(    0x08, DEF_STR( No ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Yes ) ) // Atari factory default = "Yes"
    PORT_DIPNAME( 0x06, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:6,7")
    PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
    PORT_DIPSETTING(    0x06, DEF_STR( Hardest ) )
    PORT_DIPNAME( 0x01, 0x01, "Number Of Coin Counter(s)" ) PORT_DIPLOCATION("SWB:8")
    PORT_DIPSETTING(    0x01, "Two Coin Counters" )
    PORT_DIPSETTING(    0x00, "One Coin Counter" )
INPUT_PORTS_END
*/


/**********************************************************************************************
  Dig Dug & clones
**********************************************************************************************/

ROM_START( digdug )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "dd1a.1",       0x0000, 0x1000, CRC(a80ec984) SHA1(86689980410b9429cd7582c7a76342721c87d030) )
	ROM_LOAD( "dd1a.2",       0x1000, 0x1000, CRC(559f00bd) SHA1(fde17785df21956d6fd06bcfe675c392dadb1524) )
	ROM_LOAD( "dd1a.3",       0x2000, 0x1000, CRC(8cbc6fe1) SHA1(57b8a5777f8bb9773caf0cafe5408c8b9768cb25) )
	ROM_LOAD( "dd1a.4",       0x3000, 0x1000, CRC(d066f830) SHA1(b0a615fe4a5c8742c1e4ef234ef34c369d2723b9) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "dd1a.5",       0x0000, 0x1000, CRC(6687933b) SHA1(c16144de7633595ddc1450ddce379f48e7b2195a) )
	ROM_LOAD( "dd1a.6",       0x1000, 0x1000, CRC(843d857f) SHA1(89b2ead7e478e119d33bfd67376cdf28f83de67a) )

	ROM_REGION( 0x10000, "sub2", 0 ) /* 64k for the third CPU  */
	ROM_LOAD( "dd1.7",        0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "dd1.9",        0x0000, 0x0800, CRC(f14a6fe1) SHA1(0aa63300c2cb887196de590aceb98f3cf06fead4) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "dd1.15",       0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "dd1.14",       0x1000, 0x1000, CRC(2829ec99) SHA1(3e435c1afb2e44487cd7ba28a93ada2e5ccbb86d) )
	ROM_LOAD( "dd1.13",       0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "dd1.12",       0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "dd1.11",       0x0000, 0x1000, CRC(7b383983) SHA1(57f1e8f5171d13f9f76bd091d81b4423b59f6b42) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "dd1.10b",      0x0000, 0x1000, CRC(2cf399c2) SHA1(317c48818992f757b1bd0e3997fa99937f81b52c) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( digdug1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "dd1.1",        0x0000, 0x1000, CRC(b9198079) SHA1(1d3fe04020f584ed250e32fdc6f6a3b769342884) )
	ROM_LOAD( "dd1.2",        0x1000, 0x1000, CRC(b2acbe49) SHA1(c8f713e8cfa70d3bc64d3002ff7bffc65ee138e2) )
	ROM_LOAD( "dd1.3",        0x2000, 0x1000, CRC(d6407b49) SHA1(0e71a8f02778286488865e20439776dbb2a8ec78) )
	ROM_LOAD( "dd1.4b",       0x3000, 0x1000, CRC(f4cebc16) SHA1(19b568f92069a1cfe1c07287408efe3b0e253375) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "dd1.5b",       0x0000, 0x1000, CRC(370ef9b4) SHA1(746b1fa15f5f2cfd69d8b5a7d6fb8c770abc3b4d) )
	ROM_LOAD( "dd1.6b",       0x1000, 0x1000, CRC(361eeb71) SHA1(372c97c666411c3590d790213ae6fa1ccb5ffa1c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "dd1.7",        0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "dd1.9",        0x0000, 0x0800, CRC(f14a6fe1) SHA1(0aa63300c2cb887196de590aceb98f3cf06fead4) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "dd1.15",       0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "dd1.14",       0x1000, 0x1000, CRC(2829ec99) SHA1(3e435c1afb2e44487cd7ba28a93ada2e5ccbb86d) )
	ROM_LOAD( "dd1.13",       0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "dd1.12",       0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "dd1.11",       0x0000, 0x1000, CRC(7b383983) SHA1(57f1e8f5171d13f9f76bd091d81b4423b59f6b42) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "dd1.10b",      0x0000, 0x1000, CRC(2cf399c2) SHA1(317c48818992f757b1bd0e3997fa99937f81b52c) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
    Dig Dug - Atari Version

    There are two revisions of the board and the placement of the components
    are different between the two versions.

    Revision A:
        * The letter "A" is silkscreened in the A10 corner of the board.
        * The 1st, 2nd and 3rd edition TM-203 and SP-203 manuals cover this board.

    Revision B:
        * The letter "B" is silkscreened in the P12 corner (on right side of
          the edge connector).
        * Also, the three Z80's are located on the opposite side of the edge connector and
          they are stacked in a column.  (The Z80's are oriented vertically instead of
          horizontal as the other chips are.)
        * The 4th edition TM-203 and SP-203 manuals cover this board.
*/

ROM_START( digdugat )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "136007.201",   0x0000, 0x1000, CRC(23d0b1a4) SHA1(a118d55e03a9ccf069f37c7bac2c9044dccd1f5e) )
	ROM_LOAD( "136007.202",   0x1000, 0x1000, CRC(5453dc1f) SHA1(8be091dd53e9b44e80e1ac9b1751efbe832db78d) )
	ROM_LOAD( "136007.203",   0x2000, 0x1000, CRC(c9077dfa) SHA1(611b3e1b575a51639530917366557773534c80aa) )
	ROM_LOAD( "136007.204",   0x3000, 0x1000, CRC(a8fc8eac) SHA1(7a24197f4ec5989bc4d635b27b6578f4d62cb5f4) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "136007.205",   0x0000, 0x1000, CRC(5ba385c5) SHA1(f4577bddff74a14b13b212f5553fa13fe9ae4bcc) )
	ROM_LOAD( "136007.206",   0x1000, 0x1000, CRC(382b4011) SHA1(2b79ddcf48177c99b5fa1f957374f4baa2bec143) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "136007.107",   0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136007.108",   0x0000, 0x0800, CRC(3d24a3af) SHA1(857ae93e2a41258a129dcecbaed2df359540b735) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136007.116",   0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "136007.117",   0x1000, 0x1000, CRC(a3bbfd85) SHA1(2105455762e0de120f2d943f9010a7d06c6b6448) )
	ROM_LOAD( "136007.118",   0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "136007.119",   0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "136007.115",   0x0000, 0x1000, CRC(754539be) SHA1(466ae754eb4721df8814d4d33a31d867507d45b3) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "136007.114",   0x0000, 0x1000, CRC(d6822397) SHA1(055ca6514141323f1e6dfcf91451507c04114d41) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( digdugat1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "136007.101",   0x0000, 0x1000, CRC(b9198079) SHA1(1d3fe04020f584ed250e32fdc6f6a3b769342884) )
	ROM_LOAD( "136007.102",   0x1000, 0x1000, CRC(b2acbe49) SHA1(c8f713e8cfa70d3bc64d3002ff7bffc65ee138e2) )
	ROM_LOAD( "136007.103",   0x2000, 0x1000, CRC(d6407b49) SHA1(0e71a8f02778286488865e20439776dbb2a8ec78) )
	ROM_LOAD( "136007.104",   0x3000, 0x1000, CRC(b3ad42c3) SHA1(83ea80f0dd42ec1cb62e6ed45d5dda43ed21f567) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "136007.105",   0x0000, 0x1000, CRC(0a2aef4a) SHA1(ef40974fde8e8c305059e1dd03ea811a6aaca737) )
	ROM_LOAD( "136007.106",   0x1000, 0x1000, CRC(a2876d6e) SHA1(08e8ac50918ae32dd6fb34e65534652beb0395b2) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "136007.107",   0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136007.108",   0x0000, 0x0800, CRC(3d24a3af) SHA1(857ae93e2a41258a129dcecbaed2df359540b735) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136007.116",   0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "136007.117",   0x1000, 0x1000, CRC(a3bbfd85) SHA1(2105455762e0de120f2d943f9010a7d06c6b6448) )
	ROM_LOAD( "136007.118",   0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "136007.119",   0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "136007.115",   0x0000, 0x1000, CRC(754539be) SHA1(466ae754eb4721df8814d4d33a31d867507d45b3) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "136007.114",   0x0000, 0x1000, CRC(d6822397) SHA1(055ca6514141323f1e6dfcf91451507c04114d41) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
    Zig Zag (Dig Dug bootleg)
*/

ROM_START( dzigzag )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "136007.101",   0x0000, 0x1000, CRC(b9198079) SHA1(1d3fe04020f584ed250e32fdc6f6a3b769342884) )
	ROM_LOAD( "136007.102",   0x1000, 0x1000, CRC(b2acbe49) SHA1(c8f713e8cfa70d3bc64d3002ff7bffc65ee138e2) )
	ROM_LOAD( "136007.103",   0x2000, 0x1000, CRC(d6407b49) SHA1(0e71a8f02778286488865e20439776dbb2a8ec78) )
	ROM_LOAD( "zigzag4",      0x3000, 0x1000, CRC(da20d2f6) SHA1(4eafe5ee917060d01d9df92d678c455edbbf27a6) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "zigzag5",      0x0000, 0x2000, CRC(f803c748) SHA1(a4c7dde0b794366cbfd03f339de980a6575a42fc) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "136007.107",   0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "zigzag7",      0x0000, 0x1000, CRC(24c3510c) SHA1(3214a16f697f88d23f3441e58c56110930d7c341) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "zigzag8",      0x0000, 0x0800, CRC(86120541) SHA1(c974441ee0421a38c25bc7c3edbc6b510b7df473) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136007.116",   0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "zigzag12",     0x1000, 0x1000, CRC(386a0956) SHA1(79f5d6af1fdc467a503216a588cb03535c823a40) )
	ROM_LOAD( "zigzag13",     0x2000, 0x1000, CRC(69f6e395) SHA1(10a7518e963f2cecb494d77137e01a068116e20b) )
	ROM_LOAD( "136007.119",   0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "dd1.11",       0x0000, 0x1000, CRC(7b383983) SHA1(57f1e8f5171d13f9f76bd091d81b4423b59f6b42) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "dd1.10b",      0x0000, 0x1000, CRC(2cf399c2) SHA1(317c48818992f757b1bd0e3997fa99937f81b52c) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*

Year:  1982
Manufacturer:  Sidam

CPUs:

on main PCB (Sidam 11500):

3x MK3880-4IRL-Z80CPU (main)
1x LM324N (sound)
1x TDA2003 (sound)
1x custom 0640 (DIL28)(interface to custom 5303)
1x custom 0748 (DIL28)(clock divider)
3x custom 0883 (DIL28)(bus controller)
1x custom 5156 (DIL42)(I/O)
1x custom 5303 (DIL42)(I/O)
1x oscillator 18432

on bottom PCB (Sidam 11510):

1x custom 0037 (DIL28)(unknown)
1x custom 0228 (DIL28)(gfx data shifter and mixer(16-bit in, 4-bit out))
1x custom 0425 (DIL28)(sprite address generator)
1x custom 0764 (DIL28)(clock divider)
1x custom DD1-6 (DIL20 300mil)(unknown)

ROMs:

on main PCB (Sidam 11500):

7x TMS2531JL
2x TBP24S10N (11220, 11221)

on bottom PCB (Sidam 11510):

1x TMS2516JL (8)
6x D2732A
1x TBP24S10N (11523)
1x SN74S288N (11524)

Notes:

on main PCB (Sidam 11500):
1x 22x2 edge connector
1x 3 legs power connector
1x 50 pins flat cable connector to bottom
1x trimmer (volume)
2x 8x2 switches DIP

on bottom PCB (Sidam 11510):
1x 50 pins flat cable connector to main
1x 6 legs connector

*/

ROM_START( digsid )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "digdug0.0",   0x0000, 0x1000, CRC(602197f0) SHA1(bea3b98a3f0f89d3b9e87aa38550ddd6f7883921) )
	ROM_LOAD( "digdug1.1",   0x1000, 0x1000, CRC(c6c8306b) SHA1(53e63ccb7edfdeea75df961ac69ebe882d808920) )
	ROM_LOAD( "digdug2.2",   0x2000, 0x1000, CRC(b695ec17) SHA1(46811106dbb686df6dc73b29e9e7db97b8c0d412) )
	ROM_LOAD( "digdug3.3",   0x3000, 0x1000, CRC(17bbfa40) SHA1(d3c7bf986d1d2b1961cea0c5e548245e84d74924) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "digdug4.4",       0x0000, 0x1000, CRC(370ef9b4) SHA1(746b1fa15f5f2cfd69d8b5a7d6fb8c770abc3b4d) )
	ROM_LOAD( "digdug5.5",       0x1000, 0x1000, CRC(d751df5d) SHA1(b08becb0176849a0fd1a706d6fae862684ff00b9) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "digdug6.6",   0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "digdug8.8",        0x0000, 0x0800, CRC(f14a6fe1) SHA1(0aa63300c2cb887196de590aceb98f3cf06fead4) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "digdug14.14",   0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "digdug13.13",   0x1000, 0x1000, CRC(2829ec99) SHA1(3e435c1afb2e44487cd7ba28a93ada2e5ccbb86d) )
	ROM_LOAD( "digdug12.12",   0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "digdug11.11",   0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "digdug10.10",       0x0000, 0x1000, CRC(7b383983) SHA1(57f1e8f5171d13f9f76bd091d81b4423b59f6b42) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "digdug9.9",      0x0000, 0x1000, CRC(2cf399c2) SHA1(317c48818992f757b1bd0e3997fa99937f81b52c) )

	/* Proms were not dumped with this set */
	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

} // anonymous namespace


/* Original Namco hardware, with Namco Customs */

//    YEAR, NAME,      PARENT,   MACHINE, INPUT,    STATE,         INIT,         MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1982, digdug,    0,        digdug,  digdug,   digdug_state,  empty_init,   ROT90,  "Namco", "Dig Dug (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, digdug1,   digdug,   digdug,  digdug,   digdug_state,  empty_init,   ROT90,  "Namco", "Dig Dug (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, digdugat,  digdug,   digdug,  digdug,   digdug_state,  empty_init,   ROT90,  "Namco (Atari license)", "Dig Dug (Atari, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, digdugat1, digdug,   digdug,  digdug,   digdug_state,  empty_init,   ROT90,  "Namco (Atari license)", "Dig Dug (Atari, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, digsid,    digdug,   digdug,  digdug,   digdug_state,  empty_init,   ROT90,  "Namco (Sidam license)", "Dig Dug (manufactured by Sidam)", MACHINE_SUPPORTS_SAVE )

/* Bootlegs with replacement I/O chips */

GAME( 1982, dzigzag,   digdug,   dzigzag, digdug,   digdug_state,  empty_init,   ROT90,  "bootleg", "Zig Zag (Dig Dug hardware)", MACHINE_SUPPORTS_SAVE )
