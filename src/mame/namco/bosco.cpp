// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  Functions to emulate the video and sound hardware of the machine.

  Bullet vs tilemap offsets are correct when compared with PCB videos
  (both playfield area, and radar area). Bullet vs sprite offsets are also
  correct.

  The radar area is offset by 3 pixels, also confirmed with PCB video when
  it does the VRAM check.


Custom ICs:
----------
Bosconian:
---------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
50XX     player score control (protection)
51XX     I/O
54XX     explosion sound generator

Video board:
03XX(x2) ?
05XX     starfield generator
06XX     interface to custom 5xXX
07XX     clock divider
50XX     player score control (only used as protection check)
52XX     sample player


Memory maps:
-----------
Bosconian:
---------
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
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3H    program ROM
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
01101-----00xxxx   W ----xxxx RAM 2A    \ sound control registers
01101-----01xxxx   W ----xxxx RAM 2B    /
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
01111xxxxxxxxxxx R/W xxxxxxxx RAM 2N    work RAM (not present in Galaga)
10000xxxxxxxxxxx R/W xxxxxxxx DHRAM     tilemap RAM (tile code) [1]
10001xxxxxxxxxxx R/W xxxxxxxx VCRAM     tilemap RAM (tile attr) [1]
10010--0-------- R/W xxxxxxxx EXCS      custom 06XX #2 data
10010--1-------- R/W xxxxxxxx EXCS      custom 06XX #2 control
10011----000xxxx   W ----xxxx SOWR      bullets shape and X pos msb [2]
10011----001----   W xxxxxxxx POSI X    playfield X scroll
10011----010----   W xxxxxxxx POSI Y    playfield Y scroll
10011----011----   W -----xxx STAR      to 05XX: starfield X scroll speed
10011----011----   W --xxx--- STAR      to 05XX: starfield Y scroll speed
10011----100----   W -------- STARCLR   to 05XX: unknown
10011----101----   W          n.c.
10011----110----   W          n.c.
10011----111-000   W -------x FLIP      flip screen
10011----111-001   W -------x n.c.
10011----111-010   W -------x n.c.
10011----111-011   W -------x n.c.
10011----111-100   W -------x BLK 0     \ to 05XX: starfield blink
10011----111-101   W -------x BLK 1     /          (select active subset)
10011----111-110   W -------x n.c.
10011----111-111   W -------x RESET     reset 5xXX chips on video board
10100-----------              n.c.
10101-----------              n.c.
10110-----------              n.c.
10111-----------              n.c.

[1] 1st half is radar + sprite registers, 2nd half is scrolling playfield
[2] SO = Small Objects? Only locations 4-F are used.


Easter eggs:
-----------
- Bosconian:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    5xU 6xR 1xD 4xL
  (c) 1981 NAMCO LTD. will be added at the bottom of the screen.


Notes:
-----
- The Cabinet Type "DIP switch" actually comes from the edge connector, but is mapped
  in memory in place of dip switch #8. DIP switch #8 selects single/dual coin counters
  and is entirely handled by hardware.

- bosco: there appears to be a bug in the code at 0BB1, which handles communication
  with the 06XX custom chip. First it saves in A' the command to write, then if a
  transfer is still in progress it jumps to 0BC1, does other things, then restores
  the command from A' and stores it in RAM. At that point (0BE1) it checks again if
  a transfer is in progress. If the transfer has terminated, it jumps to 0BEB, which
  restores the command from RAM, and jumps back to 0BBA to send the command. However,
  the instruction at 0BBA is ex af,af', so the command is overwritten with garbage.
  There's also an exx at 0BBB which seems unnecessary but that's harmless.
  Anyway, what this bug means is that we must make sure that the 06XX generates NMIs
  quickly enough to ensure that 0BB1 is usually not called with a transfer still is
  progress. It doesn't seem possible to prevent it altogether though, so we can only
  hope that the transfer doesn't terminate in the middle of the function.

- bosco: we have two dumps of the sound shape ROM, "prom.1d" and "bosco.spr". Music
  changes a lot from one version to the other.
  I'm using the former because it is more similar to the other Namco games. The latter,
  after masking off the unused top 4 bits and inverting bit 3, matches the Galaga one,
  so it might have come from a (bootleg?) conversion.

- bosco & galaga: the Midway arcade cabinet had an optional rapid fire board, using
  a 556 to generate autofire while the button was held. That really makes little
  sense in Galaga! For Bosconian, I guess it was for the boscomdo set I, because the
  other sets have autofire built-in.

- the bosconian video system is (apart from the starfield) almost identical functionally
  to Rally-X, but the hardware is quite different: Rally-X has no custom ICs.


TODO:
----
- bosco: is the screen horizontal resolution maybe 285? PCB videos do show a slightly
  larger right border though

***************************************************************************/

#include "emu.h"
#include "galaga.h"

#include "namco06.h"
#include "namco50.h"
#include "namco51.h"
#include "namco52.h"
#include "namco54.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"

#include "tilemap.h"
#include "video/resnet.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK                (XTAL(18'432'000)) // same as galaga.cpp

#define STARFIELD_Y_OFFSET_BOSCO    (16)
#define STARFIELD_X_LIMIT_BOSCO     (224)


class bosco_state : public galaga_state_base
{
public:
	bosco_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaga_state_base(mconfig, type, tag)
		, m_rom_52xx(*this, "52xx")
		, m_videoram(*this, "videoram")
		, m_radarattr(*this, "bosco_radarattr")
		, m_starcontrol(*this, "starcontrol")
		, m_videolatch(*this, "videolatch")
		, m_starfield(*this, "starfield")
	{
	}

	void bosco(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan);
	template <int RamOffs> TILE_GET_INFO_MEMBER(get_tile_info);
	void bosco_palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_bosco(int state);

	uint8_t namco_52xx_rom_r(offs_t offset);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void bosco_videoram_w(offs_t offset, uint8_t data);
	void scrollx_w(uint8_t data);
	void scrolly_w(uint8_t data);
	void starclr_w(uint8_t data);
	void bosco_map(address_map &map) ATTR_COLD;

	required_region_ptr<uint8_t> m_rom_52xx;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_radarattr;
	required_shared_ptr<uint8_t> m_starcontrol;
	required_device<ls259_device> m_videolatch;
	required_device<starfield_05xx_device> m_starfield;

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t *m_radarx = nullptr;
	uint8_t *m_radary = nullptr;

	uint8_t *m_spriteram = nullptr;
	uint8_t *m_spriteram2 = nullptr;
	uint32_t m_spriteram_size = 0U;

	uint8_t m_starclr = 0U;
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

static const gfx_layout spritelayout_bosco =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};

static const gfx_layout dotlayout =
{
	4,4,
	8,
	3,  /* 2 bits color + 1 bit transparency */
	{ 5, 6, 7 },
	{ STEP4(0,8) },
	{ STEP4(0,32) },
	16*8
};

static GFXDECODE_START( gfx_bosco )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_2bpp,       0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_bosco, 64*4, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, dotlayout,     64*4+64*4,  1 )
GFXDECODE_END


/* the same memory map is used by all three CPUs; all RAM areas are shared */
void bosco_state::bosco_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().nopw();         /* the only area different for each CPU */
	map(0x6800, 0x6807).r(FUNC(bosco_state::bosco_dsw_r));
	map(0x6800, 0x681f).w(m_namco_sound, FUNC(namco_wsg_device::pacman_sound_w));
	map(0x6820, 0x6827).w("misclatch", FUNC(ls259_device::write_d0));
	map(0x6830, 0x6830).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x7000, 0x70ff).rw("06xx_0", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x7100, 0x7100).rw("06xx_0", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
	map(0x7800, 0x7fff).ram().share("share1");
	map(0x8000, 0x8fff).ram().w(FUNC(bosco_state::bosco_videoram_w)).share(m_videoram);/* + sprite registers */
	map(0x9000, 0x90ff).rw("06xx_1", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x9100, 0x9100).rw("06xx_1", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
	map(0x9800, 0x980f).writeonly().share(m_radarattr);
	map(0x9810, 0x9810).w(FUNC(bosco_state::scrollx_w));
	map(0x9820, 0x9820).w(FUNC(bosco_state::scrolly_w));
	map(0x9830, 0x9830).writeonly().share(m_starcontrol);
	map(0x9840, 0x9840).w(FUNC(bosco_state::starclr_w));
	map(0x9870, 0x9877).w(m_videolatch, FUNC(ls259_device::write_d0));
}


void bosco_state::bosco(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &bosco_state::bosco_map);

	Z80(config, m_subcpu, MASTER_CLOCK/6);    /* 3.072 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &bosco_state::bosco_map);

	Z80(config, m_subcpu2, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_subcpu2->set_addrmap(AS_PROGRAM, &bosco_state::bosco_map);

	ls259_device &misclatch(LS259(config, "misclatch")); // 3C on CPU board
	misclatch.q_out_cb<0>().set(FUNC(bosco_state::irq1_clear_w));
	misclatch.q_out_cb<1>().set(FUNC(bosco_state::irq2_clear_w));
	misclatch.q_out_cb<2>().set(FUNC(bosco_state::nmion_w));
	misclatch.q_out_cb<3>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append_inputline(m_subcpu2, INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append("50xx_1", FUNC(namco_50xx_device::reset));
	misclatch.q_out_cb<3>().append("51xx", FUNC(namco_51xx_device::reset));
	misclatch.q_out_cb<3>().append("54xx", FUNC(namco_54xx_device::reset));

	NAMCO_50XX(config, "50xx_1", MASTER_CLOCK/6/2); /* 1.536 MHz */
	NAMCO_50XX(config, "50xx_2", MASTER_CLOCK/6/2); /* 1.536 MHz */

	namco_51xx_device &n51xx(NAMCO_51XX(config, "51xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n51xx.input_callback<0>().set_ioport("IN0").mask(0x0f);
	n51xx.input_callback<1>().set_ioport("IN0").rshift(4);
	n51xx.input_callback<2>().set_ioport("IN1").mask(0x0f);
	n51xx.input_callback<3>().set_ioport("IN1").rshift(4);
	n51xx.output_callback().set(FUNC(bosco_state::out));
	n51xx.lockout_callback().set(FUNC(bosco_state::lockout));

	namco_52xx_device &n52xx(NAMCO_52XX(config, "52xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n52xx.set_discrete("discrete");
	n52xx.set_basenote(NODE_04);
	n52xx.set_extclock(ATTOSECONDS_IN_NSEC(PERIOD_OF_555_ASTABLE_NSEC(RES_K(33), RES_K(10), CAP_U(0.0047))));
	n52xx.romread_callback().set(FUNC(bosco_state::namco_52xx_rom_r));
	n52xx.si_callback().set_constant(0); // pulled to GND

	namco_54xx_device &n54xx(NAMCO_54XX(config, "54xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n54xx.set_discrete("discrete");
	n54xx.set_basenote(NODE_01);

	namco_06xx_device &n06xx_0(NAMCO_06XX(config, "06xx_0", MASTER_CLOCK/6/64));
	n06xx_0.set_maincpu(m_maincpu);
	n06xx_0.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx_0.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx_0.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx_0.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));
	n06xx_0.chip_select_callback<2>().set("50xx_1", FUNC(namco_50xx_device::chip_select));
	n06xx_0.rw_callback<2>().set("50xx_1", FUNC(namco_50xx_device::rw));
	n06xx_0.read_callback<2>().set("50xx_1", FUNC(namco_50xx_device::read));
	n06xx_0.write_callback<2>().set("50xx_1", FUNC(namco_50xx_device::write));
	n06xx_0.chip_select_callback<3>().set("54xx", FUNC(namco_54xx_device::chip_select));
	n06xx_0.write_callback<3>().set("54xx", FUNC(namco_54xx_device::write));

	// The clock should be hblank, but approx with 512.
	namco_06xx_device &n06xx_1(NAMCO_06XX(config, "06xx_1", MASTER_CLOCK/6/512));
	n06xx_1.set_maincpu(m_subcpu);
	n06xx_1.read_callback<0>().set("50xx_2", FUNC(namco_50xx_device::read));
	n06xx_1.chip_select_callback<0>().set("50xx_2", FUNC(namco_50xx_device::chip_select));
	n06xx_1.rw_callback<0>().set("50xx_2", FUNC(namco_50xx_device::rw));
	n06xx_1.write_callback<0>().set("50xx_2", FUNC(namco_50xx_device::write));
	n06xx_1.write_callback<1>().set("52xx", FUNC(namco_52xx_device::write));
	n06xx_1.chip_select_callback<1>().set("52xx", FUNC(namco_52xx_device::chip_select));

	LS259(config, m_videolatch); // 1B on video board
	m_videolatch->q_out_cb<0>().set(FUNC(bosco_state::flip_screen_set)).invert();
	// Q4-Q5 to 05XX for starfield blink
	m_videolatch->q_out_cb<7>().set("50xx_2", FUNC(namco_50xx_device::reset));
	m_videolatch->q_out_cb<7>().append("52xx", FUNC(namco_52xx_device::reset));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 8);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_raw(MASTER_CLOCK/3, 384, 0, 288, 264, 16, 224+16);
	m_screen->set_screen_update(FUNC(bosco_state::screen_update));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE); // starfield lfsr
	m_screen->screen_vblank().set(FUNC(bosco_state::screen_vblank_bosco));
	m_screen->screen_vblank().append(FUNC(bosco_state::vblank_irq));
	m_screen->screen_vblank().append("51xx", FUNC(namco_51xx_device::vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bosco);
	PALETTE(config, m_palette, FUNC(bosco_state::bosco_palette), 64*4 + 64*4 + 4 + 64, 32+64);

	STARFIELD_05XX(config, m_starfield);
	m_starfield->set_starfield_config(0, STARFIELD_Y_OFFSET_BOSCO, STARFIELD_X_LIMIT_BOSCO);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NAMCO_WSG(config, m_namco_sound, MASTER_CLOCK/6/32);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0);

	/* discrete circuit on the 54XX outputs */
	DISCRETE(config, "discrete", bosco_discrete).add_route(ALL_OUTPUTS, "mono", 0.90);
}


uint8_t bosco_state::namco_52xx_rom_r(offs_t offset)
{
	//logerror("ROM read %04X\n", offset);
	if (!(offset & 0x1000))
		offset = (offset & 0xfff) | 0x0000;
	else if (!(offset & 0x2000))
		offset = (offset & 0xfff) | 0x1000;
	else if (!(offset & 0x4000))
		offset = (offset & 0xfff) | 0x2000;
	else if (!(offset & 0x8000))
		offset = (offset & 0xfff) | 0x3000;
	return (offset < m_rom_52xx.length()) ? m_rom_52xx[offset] : 0xff;
}


void bosco_state::bosco_palette(palette_device &palette) const
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

	// characters / sprites
	for (int i = 0; i < 64*4; i++)
	{
		palette.set_pen_indirect(i, (color_prom[i] & 0x0f) | 0x10); // chars
		palette.set_pen_indirect(i + 64*4, color_prom[i] & 0x0f); // sprites
	}

	// bullets lookup table
	// they use colors 28-31, I think - PAL 5A controls it
	for (int i = 0; i < 4; i++)
		palette.set_pen_indirect(64*4+64*4+i, 31 - i);

	// now the stars
	for (int i = 0; i < 64; i++)
		palette.set_pen_indirect(64*4+64*4+4+i, 32 + i);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* the video RAM has space for 32x32 tiles and is only partially used for the radar */
TILEMAP_MAPPER_MEMBER(bosco_state::fg_tilemap_scan )
{
	return col + (row << 5);
}


template <int RamOffs>
TILE_GET_INFO_MEMBER(bosco_state::get_tile_info)
{
	uint8_t const attr = m_videoram[RamOffs + tile_index + 0x800];
	tileinfo.group = attr & 0x3f;
	tileinfo.set(0,
			m_videoram[RamOffs + tile_index],
			attr & 0x3f,
			TILE_FLIPYX(attr >> 6) ^ TILE_FLIPX);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void bosco_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bosco_state::get_tile_info<0x400>)), TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bosco_state::get_tile_info<0x000>)), tilemap_mapper_delegate(*this, FUNC(bosco_state::fg_tilemap_scan)), 8,8, 8,32);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);
	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);

	m_spriteram = &m_videoram[0x03d4];
	m_spriteram_size = 0x0c;
	m_spriteram2 = m_spriteram + 0x0800;
	m_radarx = &m_videoram[0x03f0];
	m_radary = m_radarx + 0x0800;

	m_starclr = 1;

	save_item(NAME(m_starclr));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void bosco_state::bosco_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	if (offset & 0x400)
		m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
	else
		m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void bosco_state::scrollx_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0,data);
}

void bosco_state::scrolly_w(uint8_t data)
{
	m_bg_tilemap->set_scrolly(0,data);
}

void bosco_state::starclr_w(uint8_t data)
{
	// On any write to $9840, turn on starfield
	m_starclr = 0;
}



/***************************************************************************

  Display refresh

***************************************************************************/

void bosco_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	uint8_t *spriteram = m_spriteram;
	uint8_t *spriteram_2 = m_spriteram2;

	for (int offs = 0;offs < m_spriteram_size;offs += 2)
	{
		int sx = spriteram[offs + 1] - 2;
		int sy = 240 - spriteram_2[offs];
		int flipx = spriteram[offs] & 1;
		int flipy = spriteram[offs] & 2;
		int color = spriteram_2[offs + 1] & 0x3f;

		if (flip) sx += 32-1;

		m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
				(spriteram[offs] & 0xfc) >> 2,
				color,
				flipx,flipy,
				sx,sy,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x0f));
	}
}


void bosco_state::draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	for (int offs = 4; offs < 0x10;offs++)
	{
		int x = m_radarx[offs] + ((~m_radarattr[offs] & 0x01) << 8) - 2;
		int y = 251 - m_radary[offs];

		if (flip)
		{
			x -= 1;
			y += 2;
		}

		m_gfxdecode->gfx(2)->transmask(bitmap,cliprect,
				((m_radarattr[offs] & 0x0e) >> 1) ^ 0x07,
				0,
				!flip,!flip,
				x,y,0xf0);
	}
}



uint32_t bosco_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* the radar tilemap is just 8x32. We rely on the tilemap code to repeat it across
	   the screen, and clip it to only the position where it is supposed to be shown */
	rectangle fg_clip = cliprect;
	rectangle bg_clip = cliprect;
	int flip = flip_screen();
	if (flip)
	{
		bg_clip.min_x = 8*8;
		fg_clip.max_x = 8*8-1;
	}
	else
	{
		bg_clip.max_x = 28*8-1;
		fg_clip.min_x = 28*8;
	}

	bg_clip &= cliprect;
	fg_clip &= cliprect;

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_starfield->draw_starfield(bitmap, bg_clip, flip);

	draw_sprites(bitmap, bg_clip, flip);

	m_bg_tilemap->draw(screen, bitmap, bg_clip);
	m_fg_tilemap->draw(screen, bitmap, fg_clip);

	draw_bullets(bitmap, cliprect, flip);

	/* It looks like H offsets 221-223 are skipped over, moving the radar tilemap
	   (including the 'bullets' on it) 3 pixels to the left */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		if (flip)
		{
			for (int x = 63; x >= 0; x--)
			{
				bitmap.pix(y, x + 3) = bitmap.pix(y, x);
				bitmap.pix(y, x) = m_palette->black_pen();
			}
		}
		else
		{
			for (int x = 224; x < 288; x++)
			{
				bitmap.pix(y, x - 3) = bitmap.pix(y, x);
				bitmap.pix(y, x) = m_palette->black_pen();
			}
		}
	}

	return 0;
}


void bosco_state::screen_vblank_bosco(int state)
{
	// falling edge
	if (!state)
	{
		// Bosconian scrolls in X and Y directions
		const uint8_t speed_index_X = m_starcontrol[0] & 0x07;
		const uint8_t speed_index_Y = (m_starcontrol[0] & 0x38) >> 3;
		m_starfield->set_scroll_speed(speed_index_X,speed_index_Y);

		m_starfield->set_active_starfield_sets(m_videolatch->q4_r(), m_videolatch->q5_r() | 2);

		// _STARCLR signal enables/disables starfield
		m_starfield->enable_starfield(!m_starclr);
	}
}


static INPUT_PORTS_START( bosco )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

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
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) ) // factory default = "Yes"
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze" )                    PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	/* bonus scores are different for 5 lives */
	PORT_DIPNAME( 0x38, 0x20, "Bonus Fighter" )         PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x30, "15K and 50K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) /* Began with 1, 2 or 3 fighters */
	PORT_DIPSETTING(    0x38, "20K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "15K, 50K, Every 50K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "15K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) // factory default = "20K, 70K, Every70K"
	PORT_DIPSETTING(    0x28, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0) /* Began with 5 fighters */
	PORT_DIPSETTING(    0x38, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "15K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "20K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "20K and 100K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "30K and 120K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K, 80K, Every 80K" )   PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( boscomd )
	PORT_INCLUDE( bosco )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits Game" )            PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x01, "2 Players" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )                    PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/**********************************************************************************************
  Bosconian & clones
**********************************************************************************************/
/*

Bosconian
Namco/Midway, 1981

*/

ROM_START( bosco ) // 23209611 (23209631) main PCB + 23169612 (23169632) sub PCB
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos5_1.3p",    0x0000, 0x1000, CRC(b1482ad1) SHA1(32d0402fc4882cae2c3655f24087f9f1911f99c1) )
	ROM_LOAD( "bos5_2.3m",    0x1000, 0x1000, CRC(e0828ef8) SHA1(2633c7518bf0918f33dac8fbed7aa7053a4793f0) )
	ROM_LOAD( "bos5_3.2m",    0x2000, 0x1000, CRC(229edd51) SHA1(6e34837b1d18637b94b3e772ff1b61ffe70e8fcc) )
	ROM_LOAD( "bos5_4.2l",   0x3000, 0x1000, CRC(928a39a0) SHA1(dfc5a7ff62a0eabc900b43ef6b8e86e4d46125fe) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos5_5.3f",   0x0000, 0x1000, CRC(84f7c1ea) SHA1(53a1242490575938fca9e738546c93edf82ad7d3) )
	ROM_LOAD( "bos5_6.3j",   0x1000, 0x1000, CRC(7fa34d5e) SHA1(c99feb051ab62ef3f7278a411938cd09e731fc19) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.2c",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

ROM_START( bosco3 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos3_1.3n",    0x0000, 0x1000, CRC(96021267) SHA1(bd49b0caabcccf9df45a272d767456a4fc8a7c07) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4b.3k",   0x3000, 0x1000, CRC(a3f7f4ab) SHA1(eb26184311bae0767c7a5593926e6eadcbcb680e) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5c.3j",   0x0000, 0x1000, CRC(a7c8e432) SHA1(3607be75daa10f1f98dbfd9e600c5ba513130d44) )
	ROM_LOAD( "bos3_6.3h",    0x1000, 0x1000, CRC(4543cf82) SHA1(50ad7d1ab6694eb8fab88d0fa79ee04f6984f3ca) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

ROM_START( bosco1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos1_1.3n",    0x0000, 0x1000, CRC(0d9920e7) SHA1(e7633233f603ccb5b7a970ed5b58ef361ef2c94e) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4b.3k",   0x3000, 0x1000, CRC(a3f7f4ab) SHA1(eb26184311bae0767c7a5593926e6eadcbcb680e) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5c.3j",   0x0000, 0x1000, CRC(a7c8e432) SHA1(3607be75daa10f1f98dbfd9e600c5ba513130d44) )
	ROM_LOAD( "bos1_6.3h",    0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

ROM_START( bosco1o )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos1_1.3n",    0x0000, 0x1000, CRC(0d9920e7) SHA1(e7633233f603ccb5b7a970ed5b58ef361ef2c94e) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4.3k",    0x3000, 0x1000, CRC(7ebea2b8) SHA1(92fc66526ed77f3efd947b7d321b255aba4a0140) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5b.3j",   0x0000, 0x1000, CRC(3d6955a8) SHA1(f89860d74865da5ced2f5b2196bdaa8eeb5e2322) )
	ROM_LOAD( "bos1_6.3h",    0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

/*
    Bosconian - Midway Version

    CPU/Sound Board: A084-91412-B550
    Video Board:     A084-91413-B550
*/

ROM_START( boscomd )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "3n",       0x0000, 0x1000, CRC(441b501a) SHA1(7b4921ff40b3c56950fd32aa0ec5563b02a00929) )
	ROM_LOAD( "3m",       0x1000, 0x1000, CRC(a3c5c7ef) SHA1(70a095a8dbca857245a70404f803916f519e0cbc) )
	ROM_LOAD( "3l",       0x2000, 0x1000, CRC(6ca9a0cf) SHA1(8f70e29beae921e63cd65689a618ca678dd14614) )
	ROM_LOAD( "3k",       0x3000, 0x1000, CRC(d83bacc5) SHA1(cf2fbfa81dabb9b6bcf436d61992e705723776fb) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "3j",       0x0000, 0x1000, CRC(4374e39a) SHA1(7571fd5961f49a0e9ba4301ddd0aca52e94e2f8b) )
	ROM_LOAD( "3h",       0x1000, 0x1000, CRC(04e9fcef) SHA1(2115a9718d511854848704e2693f9efa1c80a307) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "2900.3e",      0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5300.5d",      0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "5200.5e",      0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "prom.2d",      0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bosco.6b",     0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bosco.4m",     0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "prom.2r",      0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "prom.7h",      0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom.1d",      0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "prom.5c",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "4900.5n",      0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "5000.5m",      0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "5100.5l",      0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )

	ROM_REGION( 0x0001, "pal_vidbd", 0 ) /* PAL located on the video board */
	ROM_LOAD( "0066-005xx-xxqx.5a", 0x00000, 0x00001, NO_DUMP ) /* According to the manual it's a PAL. What type is unknown. */
ROM_END

ROM_START( boscomdo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "2300.3n",      0x0000, 0x1000, CRC(db6128b0) SHA1(ddd285f7e00d5e58ab9b15838528e0020d47fcd2) )
	ROM_LOAD( "2400.3m",      0x1000, 0x1000, CRC(86907614) SHA1(3295ab6c5171a069875c2239b3325296c1df6031) )
	ROM_LOAD( "2500.3l",      0x2000, 0x1000, CRC(a21fae11) SHA1(dff38d90ee30558274d2d399edc3281c2ef5cb69) )
	ROM_LOAD( "2600.3k",      0x3000, 0x1000, CRC(11d6ae23) SHA1(f2f72f5c777b684f7ffd53b9c034560211113499) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "2700.3j",      0x0000, 0x1000, CRC(7254e65e) SHA1(c2ee29fcb5173e8d46a80a8a1b931a53dbdeae66) )
	ROM_LOAD( "2800.3h",      0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "2900.3e",      0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5300.5d",      0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "5200.5e",      0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "prom.2d",      0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bosco.6b",     0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bosco.4m",     0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "prom.2r",      0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "prom.7h",      0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom.1d",      0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "prom.5c",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "4900.5n",      0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "5000.5m",      0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "5100.5l",      0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )

	ROM_REGION( 0x0001, "pal_vidbd", 0 ) /* PAL located on the video board */
	ROM_LOAD( "0066-005xx-xxqx.5a", 0x00000, 0x00001, NO_DUMP ) /* According to the manual it's a PAL. What type is unknown. */
ROM_END

} // anonymous namespace


/* Original Namco hardware, with Namco Customs */

//    YEAR, NAME,      PARENT,   MACHINE, INPUT,    STATE,         INIT,         MONITOR,COMPANY,                  FULLNAME,FLAGS
GAME( 1981, bosco,     0,        bosco,   bosco,    bosco_state,   empty_init,   ROT0,   "Namco",                  "Bosconian - Star Destroyer (version 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, bosco3,    bosco,    bosco,   bosco,    bosco_state,   empty_init,   ROT0,   "Namco",                  "Bosconian - Star Destroyer (version 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, bosco1,    bosco,    bosco,   bosco,    bosco_state,   empty_init,   ROT0,   "Namco",                  "Bosconian - Star Destroyer (version 1, newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, bosco1o,   bosco,    bosco,   bosco,    bosco_state,   empty_init,   ROT0,   "Namco",                  "Bosconian - Star Destroyer (version 1, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, boscomd,   bosco,    bosco,   boscomd,  bosco_state,   empty_init,   ROT0,   "Namco (Midway license)", "Bosconian - Star Destroyer (Midway, new version)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, boscomdo,  bosco,    bosco,   boscomd,  bosco_state,   empty_init,   ROT0,   "Namco (Midway license)", "Bosconian - Star Destroyer (Midway, old version)", MACHINE_SUPPORTS_SAVE )
