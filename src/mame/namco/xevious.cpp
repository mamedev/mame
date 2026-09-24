// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  xevious.cpp

  Functions to emulate the general aspects and video hardware of the machine.


Custom ICs:
----------
Xevious:
-------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
50XX     player score control (only used for a protection check on startup)
51XX     I/O
54XX     explosion sound generator

Video board:
03XX(x2) ?
04XX     sprite address generator
07XX     clock divider
11XX(x2) gfx data shifter and mixer (16-bit in, 4-bit out)
12XX     sprite generator
13XX     dual scrolling tilemap address generator


Memory maps:
-----------
Xevious:
-------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000xxxxxxxxxxxxx R   xxxxxxxx ROM 1     program ROM
001xxxxxxxxxxxxx R   xxxxxxxx ROM 2     program ROM
the rest of the memory map is common to the other CPUs

MOTION CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000xxxxxxxxxxxxx R   xxxxxxxx ROM 3     program ROM
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00-xxxxxxxxxxxxx R   xxxxxxxx ROM 4     program ROM
the rest of the memory map is common to the other CPUs

COMMON:
a small part of the decoding for the video board is done by a PAL so it is inferred by program behaviour

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
01000-----------              n.c.
01001-----------              n.c.
01010-----------              n.c.
01011-----------              n.c.
01100-----------              n.c.
01101-----00----   W ----xxxx SRAM 0    \ sound control registers
01101-----01----   W ----xxxx SRAM 1    /
01101-----10-000   W -------x IRQ1      main CPU irq enable/acknowledge
01101-----10-001   W -------x IRQ2      motion CPU irq enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU nmi enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x n.c.
01101-----10-110   W -------x n.c.
01101-----10-111   W -------x n.c.
01101-----11----   W -------- WDR       watchdog reset
01101-----00-xxx R   -------x DIP SW    dip switch B
01101-----00-xxx R   ------x- DIP SW    dip switch A
01101-----01---- R            n.c.
01101-----10---- R            n.c.
01101-----11---- R            n.c.
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
01111xxxxxxxxxxx R/W xxxxxxxx           work RAM
1000-xxxxxxxxxxx R/W xxxxxxxx           work RAM
1000-1111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (x, y)
1001-xxxxxxxxxxx R/W xxxxxxxx           work RAM
1001-1111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (flip, size)
1010-xxxxxxxxxxx R/W xxxxxxxx           work RAM
1010-1111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (sprite number & color)
10110xxxxxxxxxxx R/W xxxxxxxx PF0       fg tilemap RAM (tile attributes)
10111xxxxxxxxxxx R/W xxxxxxxx PF1       bg tilemap RAM (tile attributes)
11000xxxxxxxxxxx R/W xxxxxxxx PF2       fg tilemap RAM (tile code)
11001xxxxxxxxxxx R/W xxxxxxxx PF3       bg tilemap RAM (tile code)
1101-----000---x   W xxxxxxxx           bg X scroll (9-bit data: A0 is the msb)
1101-----001---x   W xxxxxxxx           fg X scroll (9-bit data: A0 is the msb)
1101-----010---x   W xxxxxxxx           bg Y scroll (9-bit data: A0 is the msb)
1101-----011---x   W xxxxxxxx           fg Y scroll (9-bit data: A0 is the msb)
1101-----111----   W -------x FLIP      flip screen
1110------------              n.c.
1111-----------0   W xxxxxxxx BS0       \ address to read from background data ROMs
1111-----------1   W xxxxxxxx BS1       / (see xevious_bb_r)
1111-----------0 R   xxxxxxxx BB0       \ read from background data ROMs
1111-----------1 R   xxxxxxxx BB1       /


Namco vs Atari ROM names and locations
--------------------------------------
Location  ID          Location  ID
--------  ----        --------  ----------
CPU 3P    XVI-1       CPU 1M    136018-118
CPU 3M    XVI-2        "   "      "     "
CPU 2M    XVI-3       CPU 1L    136018-119
CPU 2L    XVI-4        "   "      "     "
CPU 3F    XVI-5       CPU 4C    136018-120
CPU 3J    XVI-6        "   "      "     "
CPU 2C    XVI-7       CPU 2C    136018-127
CPU 5N    XVI-1[bpr]  CPU 6M    136018-028
CPU 7N    XVI-2[bpr]  CPU 8M    136018-029

VID 2A    XVI-9       VID 2A    136018-101
VID 2B    XVI-10      VID 2B    136018-102
VID 2C    XVI-11      VID 2C    136018-103
VID 3B    XVI-12      VID 3B    136018-104
VID 3C    XVI-13      VID 3C    136018-105
VID 3D    XVI-14      VID 3D    136018-106
VID 4M    XVI-15      VID 4M    136018-107
VID 4N    XVI-16      VID 4N    136018-108
VID 4P    XVI-17      VID 4P    136018-109
VID 4R    XVI-18      VID 4R    136018-110
VID 3L    XVI-4[bpr]  VID 3L    136018-011
VID 3M    XVI-5[bpr]  VID 3M    136018-012
VID 4F    XVI-6[bpr]  VID 4F    136018-013
VID 4H    XVI-7[bpr]  VID 4H    136018-014
VID 6A    XVI-8[bpr]  VID 6A    136018-015
VID 6D    XVI-9[bpr]  VID 6D    136018-016
VID 6E    XVI-10[bpr] VID 6E    136018-017


Easter eggs:
-----------
- Xevious:
  - start a game
  - go to the bottom right of the screen and keep B2 pressed
  NAMCO ORIGINAL
  program by EVEZOO
  will be written at the bottom of the screen
  In Super Xevious this is changed to
  special thanks for you
  by game designer EVEZOO


TODO:
----
- The Cabinet Type "DIP switch" actually comes from the edge connector, but is mapped
  in memory in place of dip switch #8. DIP switch #8 selects single/dual coin counters
  and is entirely handled by hardware.

- Xevios: emulate the 4th Z80 (ROM dump is complete)

- xevious: I haven't found any Easter egg in service mode. The main loop is very
  simple so there might just not be one, though this would be the only Namco game
  of that era to not have a service mode Easter egg. On the other hand, the service
  mode in this game is VERY spartan when compared to the other Namco games.

***************************************************************************/

#include "emu.h"
#include "galaga.h"

#include "namco06.h"
#include "namco50.h"
#include "namco51.h"
#include "namco54.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/samples.h"

#include "speaker.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK (XTAL(18'432'000)) // same as galaga.cpp


class xevious_state : public galaga_state
{
public:
	xevious_state(const machine_config &mconfig, device_type type, const char *tag) :
		galaga_state(mconfig, type, tag),
		m_xevious_sr1(*this, "xevious_sr1"),
		m_xevious_sr2(*this, "xevious_sr2"),
		m_xevious_sr3(*this, "xevious_sr3"),
		m_xevious_fg_colorram(*this, "fg_colorram"),
		m_xevious_bg_colorram(*this, "bg_colorram"),
		m_xevious_fg_videoram(*this, "fg_videoram"),
		m_xevious_bg_videoram(*this, "bg_videoram"),
		m_samples(*this, "samples")
	{ }

	void xevious(machine_config &config) ATTR_COLD;

	void init_xevious() ATTR_COLD;
	void init_xevios() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	void xevious_common(machine_config &config) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void xevious_palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update_xevious(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void xevious_fg_videoram_w(offs_t offset, uint8_t data);
	void xevious_fg_colorram_w(offs_t offset, uint8_t data);
	void xevious_bg_videoram_w(offs_t offset, uint8_t data);
	void xevious_bg_colorram_w(offs_t offset, uint8_t data);
	void xevious_vh_latch_w(offs_t offset, uint8_t data);
	void xevious_bs_w(offs_t offset, uint8_t data);
	uint8_t xevious_bb_r(offs_t offset);

	void common_map(address_map &map) ATTR_COLD;
	void xevious_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_xevious_sr1;
	required_shared_ptr<uint8_t> m_xevious_sr2;
	required_shared_ptr<uint8_t> m_xevious_sr3;
	required_shared_ptr<uint8_t> m_xevious_fg_colorram;
	required_shared_ptr<uint8_t> m_xevious_bg_colorram;
	required_shared_ptr<uint8_t> m_xevious_fg_videoram;
	required_shared_ptr<uint8_t> m_xevious_bg_videoram;
	optional_device<samples_device> m_samples;

	int32_t m_xevious_bs[2];
};

class battles_state : public xevious_state
{
public:
	battles_state(const machine_config &mconfig, device_type type, const char *tag) :
		xevious_state(mconfig, type, tag),
		m_subcpu3(*this, "sub3"),
		m_nmi_timer(*this, "nmi"),
		m_io_in(*this, "IN%u", 0U)
	{
	}

	void battles(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void interrupt_4(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_generate);

	void battles_map(address_map &map) ATTR_COLD;
	void battles_mem4(address_map &map) ATTR_COLD;

	// Custom I/O
	uint8_t customio0_r();
	uint8_t customio_data0_r(offs_t offset);
	uint8_t customio3_r();
	uint8_t customio_data3_r(offs_t offset);
	uint8_t input_port_r(offs_t offset);

	void customio0_w(uint8_t data);
	void customio_data0_w(offs_t offset, uint8_t data);
	void customio3_w(uint8_t data);
	void customio_data3_w(offs_t offset, uint8_t data);
	void cpu4_coin_w(uint8_t data);
	void noise_sound_w(offs_t offset, uint8_t data);

	required_device<cpu_device> m_subcpu3;
	required_device<timer_device> m_nmi_timer;
	required_ioport_array<2> m_io_in;

	uint8_t m_customio[16]{};
	char m_customio_command = 0;
	char m_customio_prev_command = 0;
	char m_customio_command_count = 0;
	char m_customio_data = 0;
	char m_sound_played = 0;
};


static const gfx_layout bgcharlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout spritelayout_xevious =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(0*8,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};

static GFXDECODE_START( gfx_xevious )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1,          128*4+64*8,  64 )
	GFXDECODE_ENTRY( "gfx2", 0, bgcharlayout,                0, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_xevious,    128*4,  64 )
GFXDECODE_END


/* The resistance path of the namco sound is 16k compared to
 * the 10k of the highest gain 54xx filter. Giving a 10/16 gain.
 */

static const char *const battles_sample_names[] =
{
	"*battles",
	"explo1",   /* ground target explosion */
	"explo2",   /* Solvalou explosion */
	nullptr   /* end of array */
};


void xevious_state::common_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().nopw();         /* the only area different for each CPU */
	map(0x6800, 0x6807).r(FUNC(xevious_state::bosco_dsw_r));
	map(0x6800, 0x681f).w(m_namco_sound, FUNC(namco_wsg_device::pacman_sound_w));
	map(0x6820, 0x6827).w("misclatch", FUNC(ls259_device::write_d0));
	map(0x6830, 0x6830).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	// either the Namco custom or the bootleg's replacement go here
	map(0x7800, 0x7fff).ram().share("share1");                          /* work RAM */
	map(0x8000, 0x87ff).ram().share(m_xevious_sr1); /* work RAM + sprite registers */
	map(0x9000, 0x97ff).ram().share(m_xevious_sr2); /* work RAM + sprite registers */
	map(0xa000, 0xa7ff).ram().share(m_xevious_sr3); /* work RAM + sprite registers */
	map(0xb000, 0xb7ff).ram().w(FUNC(xevious_state::xevious_fg_colorram_w)).share(m_xevious_fg_colorram);
	map(0xb800, 0xbfff).ram().w(FUNC(xevious_state::xevious_bg_colorram_w)).share(m_xevious_bg_colorram);
	map(0xc000, 0xc7ff).ram().w(FUNC(xevious_state::xevious_fg_videoram_w)).share(m_xevious_fg_videoram);
	map(0xc800, 0xcfff).ram().w(FUNC(xevious_state::xevious_bg_videoram_w)).share(m_xevious_bg_videoram);
	map(0xd000, 0xd07f).w(FUNC(xevious_state::xevious_vh_latch_w));
	map(0xf000, 0xffff).rw(FUNC(xevious_state::xevious_bb_r), FUNC(xevious_state::xevious_bs_w));
}

void xevious_state::xevious_map(address_map &map)
{
	common_map(map);

	map(0x7000, 0x70ff).rw("06xx", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x7100, 0x7100).rw("06xx", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
}

void battles_state::battles_map(address_map &map)
{
	common_map(map);

	// replace the Namco I/O handlers with interface to the 4th CPU
	map(0x7000, 0x700f).rw(FUNC(battles_state::customio_data0_r), FUNC(battles_state::customio_data0_w));
	map(0x7100, 0x7100).rw(FUNC(battles_state::customio0_r), FUNC(battles_state::customio0_w));
}

void battles_state::battles_mem4(address_map &map)
{
	map(0x0000, 0x0fff).rom();

	map(0x4000, 0x4003).r(FUNC(battles_state::input_port_r));
	map(0x4001, 0x4001).w(FUNC(battles_state::cpu4_coin_w));
	map(0x5000, 0x5000).w(FUNC(battles_state::noise_sound_w));
	map(0x6000, 0x6000).rw(FUNC(battles_state::customio3_r), FUNC(battles_state::customio3_w));
	map(0x7000, 0x7000).rw(FUNC(battles_state::customio_data3_r), FUNC(battles_state::customio_data3_w));
	map(0x8000, 0x80ff).ram();
}


void xevious_state::xevious_common(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);  /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &xevious_state::xevious_map);

	Z80(config, m_subcpu, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &xevious_state::xevious_map);

	Z80(config, m_subcpu2, MASTER_CLOCK/6);  /* 3.072 MHz */
	m_subcpu2->set_addrmap(AS_PROGRAM, &xevious_state::xevious_map);

	ls259_device &misclatch(LS259(config, "misclatch")); // 5K
	misclatch.q_out_cb<0>().set(FUNC(xevious_state::irq1_clear_w));
	misclatch.q_out_cb<1>().set(FUNC(xevious_state::irq2_clear_w));
	misclatch.q_out_cb<2>().set(FUNC(xevious_state::nmion_w));
	misclatch.q_out_cb<3>().set_inputline("sub", INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append_inputline("sub2", INPUT_LINE_RESET).invert();

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 8);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_raw(MASTER_CLOCK/3, 384, 0, 288, 264, 0, 224);
	m_screen->set_screen_update(FUNC(xevious_state::screen_update_xevious));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(xevious_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_xevious);
	PALETTE(config, m_palette, FUNC(xevious_state::xevious_palette), 128*4 + 64*8 + 64*2, 128+1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NAMCO_WSG(config, m_namco_sound, MASTER_CLOCK/6/32);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0);
}

void xevious_state::xevious(machine_config &config)
{
	xevious_common(config);

	/* basic machine hardware */
	auto &misclatch = downcast<ls259_device &>(*config.device("misclatch"));
	misclatch.q_out_cb<3>().append("50xx", FUNC(namco_50xx_device::reset));
	misclatch.q_out_cb<3>().append("51xx", FUNC(namco_51xx_device::reset));
	misclatch.q_out_cb<3>().append("54xx", FUNC(namco_54xx_device::reset));

	NAMCO_50XX(config, "50xx", MASTER_CLOCK/6/2);   /* 1.536 MHz */

	namco_51xx_device &n51xx(NAMCO_51XX(config, "51xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n51xx.input_callback<0>().set_ioport("IN0").mask(0x0f);
	n51xx.input_callback<1>().set_ioport("IN0").rshift(4);
	n51xx.input_callback<2>().set_ioport("IN1").mask(0x0f);
	n51xx.input_callback<3>().set_ioport("IN1").rshift(4);
	n51xx.output_callback().set(FUNC(xevious_state::out));
	n51xx.lockout_callback().set(FUNC(xevious_state::lockout));

	namco_54xx_device &n54xx(NAMCO_54XX(config, "54xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n54xx.set_discrete("discrete");
	n54xx.set_basenote(NODE_01);

	namco_06xx_device &n06xx(NAMCO_06XX(config, "06xx", MASTER_CLOCK/6/64));
	n06xx.set_maincpu(m_maincpu);
	n06xx.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));
	n06xx.chip_select_callback<2>().set("50xx", FUNC(namco_50xx_device::chip_select));
	n06xx.rw_callback<2>().set("50xx", FUNC(namco_50xx_device::rw));
	n06xx.read_callback<2>().set("50xx", FUNC(namco_50xx_device::read));
	n06xx.write_callback<2>().set("50xx", FUNC(namco_50xx_device::write));
	n06xx.write_callback<3>().set("54xx", FUNC(namco_54xx_device::write));
	n06xx.chip_select_callback<3>().set("54xx", FUNC(namco_54xx_device::chip_select));

	m_screen->screen_vblank().append("51xx", FUNC(namco_51xx_device::vblank));

	/* discrete circuit on the 54XX outputs */
	DISCRETE(config, "discrete", galaga_discrete).add_route(ALL_OUTPUTS, "mono", 0.90);
}

void battles_state::battles(machine_config &config)
{
	xevious_common(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &battles_state::battles_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &battles_state::common_map);
	m_subcpu2->set_addrmap(AS_PROGRAM, &battles_state::common_map);

	Z80(config, m_subcpu3, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_subcpu3->set_addrmap(AS_PROGRAM, &battles_state::battles_mem4);

	m_screen->screen_vblank().append(FUNC(battles_state::interrupt_4));

	TIMER(config, "nmi").configure_generic(FUNC(battles_state::nmi_generate));

	/* sound hardware */
	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(battles_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.80);
}


/***************************************************************************

 BATTLES CPU4 (custum I/O Emulation) I/O Handlers

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(battles_state::nmi_generate)
{
	m_customio_prev_command = m_customio_command;

	if (m_customio_command & 0x10)
	{
		if (m_customio_command_count == 0)
		{
			m_subcpu3->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
		else
		{
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			m_subcpu3->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
	}
	else
	{
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		m_subcpu3->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	m_customio_command_count++;
}


uint8_t battles_state::customio0_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: custom I/O Read = %02x\n", machine().describe_context(), m_customio_command);

	return m_customio_command;
}

uint8_t battles_state::customio3_r()
{
	uint8_t return_data;

	if (m_subcpu3->pc() == 0xae)
	{
		/* CPU4 0xAA - 0xB9 : waiting for MB8851 ? */
		return_data =   ( (m_customio_command & 0x10) << 3)
						| 0x00
						| (m_customio_command & 0x0f);
	}
	else
	{
		return_data =   ( (m_customio_prev_command & 0x10) << 3)
						| 0x60
						| (m_customio_prev_command & 0x0f);
	}
	logerror("%s: custom I/O Read = %02x\n", machine().describe_context(), return_data);

	return return_data;
}


void battles_state::customio0_w(uint8_t data)
{
	logerror("%s: custom I/O Write = %02x\n", machine().describe_context(), data);

	m_customio_command = data;
	m_customio_command_count = 0;

	switch (data)
	{
		case 0x10:
			m_nmi_timer->reset();
			return; /* nop */
	}
	m_nmi_timer->adjust(attotime::from_usec(166), 0, attotime::from_usec(166));
}

void battles_state::customio3_w(uint8_t data)
{
	logerror("%s: custom I/O Write = %02x\n", machine().describe_context(), data);

	m_customio_command = data;
}



uint8_t battles_state::customio_data0_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		logerror("%s: custom I/O parameter %02x Read = %02x\n", machine().describe_context(), offset, m_customio_data);

	return m_customio_data;
}

uint8_t battles_state::customio_data3_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		logerror("%s: custom I/O parameter %02x Read = %02x\n", machine().describe_context(), offset, m_customio_data);

	return m_customio_data;
}


void battles_state::customio_data0_w(offs_t offset, uint8_t data)
{
	logerror("%s: custom I/O parameter %02x Write = %02x\n", machine().describe_context(), offset, data);

	m_customio_data = data;
}

void battles_state::customio_data3_w(offs_t offset, uint8_t data)
{
	logerror("%s: custom I/O parameter %02x Write = %02x\n", machine().describe_context(), offset, data);

	m_customio_data = data;
}


void battles_state::cpu4_coin_w(uint8_t data)
{
	m_leds[0] = BIT(data, 1); // Start 1
	m_leds[1] = BIT(data, 0); // Start 2

	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 4));
	machine().bookkeeping().coin_lockout_global_w(BIT(~data, 2));
}


void battles_state::noise_sound_w(offs_t offset, uint8_t data)
{
	logerror("%s: 50%02x Write = %02x\n", machine().describe_context(), offset, data);
	if ((m_sound_played == 0) && (data == 0xff))
	{
		if (m_customio[0] == 0x40)
			m_samples->start(0, 0);
		else
			m_samples->start(0, 1);
	}
	m_sound_played = data;
}


uint8_t battles_state::input_port_r(offs_t offset)
{
	switch (offset)
	{
		default:
		case 0: return ~bitswap<4>(m_io_in[1]->read(), 6, 7, 5, 4);
		case 1: return ~m_io_in[0]->read() & 0x0f;
		case 2: return ~m_io_in[0]->read() >> 4;
		case 3: return ~m_io_in[1]->read() & 0x0f;
	}
}


void battles_state::interrupt_4(int state)
{
	if (state)
		m_subcpu3->set_input_line(0, HOLD_LINE);
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Xevious has three 256x4 palette PROMs (one per gun) and four 512x4 lookup
  table PROMs (two for sprites, two for background tiles; foreground
  characters map directly to a palette color without using a PROM).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
void xevious_state::xevious_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	auto const TOTAL_COLORS = [this] (int gfxn) { return m_gfxdecode->gfx(gfxn)->colors() * m_gfxdecode->gfx(gfxn)->granularity(); };

	for (int i = 0; i < 128; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = BIT(color_prom[256], 0);
		bit1 = BIT(color_prom[256], 1);
		bit2 = BIT(color_prom[256], 2);
		bit3 = BIT(color_prom[256], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = BIT(color_prom[2*256], 0);
		bit1 = BIT(color_prom[2*256], 1);
		bit2 = BIT(color_prom[2*256], 2);
		bit3 = BIT(color_prom[2*256], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r, g, b));
		color_prom++;
	}

	// color 0x80 is used by sprites to mark transparency
	palette.set_indirect_color(0x80, rgb_t(0, 0, 0));

	color_prom += 128;  // the bottom part of the PROM is unused
	color_prom += 2*256;
	// color_prom now points to the beginning of the lookup table

	// background tiles
	for (int i = 0; i < TOTAL_COLORS(1); i++)
	{
		palette.set_pen_indirect(
				m_gfxdecode->gfx(1)->colorbase() + i,
				(color_prom[0] & 0x0f) | ((color_prom[TOTAL_COLORS(1)] & 0x0f) << 4));

		color_prom++;
	}
	color_prom += TOTAL_COLORS(1);

	// sprites
	for (int i = 0; i < TOTAL_COLORS(2); i++)
	{
		int const c = (color_prom[0] & 0x0f) | ((color_prom[TOTAL_COLORS(2)] & 0x0f) << 4);

		palette.set_pen_indirect(
				m_gfxdecode->gfx(2)->colorbase() + i,
				(c & 0x80) ? (c & 0x7f) : 0x80);

		color_prom++;
	}
	color_prom += TOTAL_COLORS(2);

	// foreground characters
	for (int i = 0; i < TOTAL_COLORS(0); i++)
	{
		palette.set_pen_indirect(
				m_gfxdecode->gfx(0)->colorbase() + i,
				BIT(i, 0) ? (i >> 1) : 0x80);
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(xevious_state::get_fg_tile_info)
{
	uint8_t attr = m_xevious_fg_colorram[tile_index];

	/* the hardware has two character sets, one normal and one x-flipped. When
	   screen is flipped, character y flip is done by the hardware inverting the
	   timing signals, while x flip is done by selecting the 2nd character set.
	   We reproduce this here, but since the tilemap system automatically flips
	   characters when screen is flipped, we have to flip them back. */
	uint8_t color = ((attr & 0x03) << 4) | ((attr & 0x3c) >> 2);
	tileinfo.set(0,
			m_xevious_fg_videoram[tile_index] | (flip_screen() ? 0x100 : 0),
			color,
			TILE_FLIPYX((attr & 0xc0) >> 6) ^ (flip_screen() ? TILE_FLIPX : 0));
}

TILE_GET_INFO_MEMBER(xevious_state::get_bg_tile_info)
{
	uint8_t code = m_xevious_bg_videoram[tile_index];
	uint8_t attr = m_xevious_bg_colorram[tile_index];
	uint8_t color = ((attr & 0x3c) >> 2) | ((code & 0x80) >> 3) | ((attr & 0x03) << 5);
	tileinfo.set(1,
			code + ((attr & 0x01) << 8),
			color,
			TILE_FLIPYX((attr & 0xc0) >> 6));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void xevious_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xevious_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xevious_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap->set_scrolldx(-20,288+27);
	m_bg_tilemap->set_scrolldy(-16,-16);
	m_fg_tilemap->set_scrolldx(-32,288+32);
	m_fg_tilemap->set_scrolldy(-18,-10);
	m_fg_tilemap->set_transparent_pen(0);
	m_xevious_bs[0] = 0;
	m_xevious_bs[1] = 0;

	save_item(NAME(m_xevious_bs));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void xevious_state::xevious_fg_videoram_w(offs_t offset, uint8_t data)
{
	m_xevious_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void xevious_state::xevious_fg_colorram_w(offs_t offset, uint8_t data)
{
	m_xevious_fg_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void xevious_state::xevious_bg_videoram_w(offs_t offset, uint8_t data)
{
	m_xevious_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void xevious_state::xevious_bg_colorram_w(offs_t offset, uint8_t data)
{
	m_xevious_bg_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void xevious_state::xevious_vh_latch_w(offs_t offset, uint8_t data)
{
	int scroll = data + ((offset&0x01)<<8);   /* A0 -> D8 */

	int reg = (offset >> 4) & 0x0f;

	switch (reg)
	{
	case 0:
		m_bg_tilemap->set_scrollx(0, scroll);
		break;
	case 1:
		m_fg_tilemap->set_scrollx(0, scroll);
		break;
	case 2:
		m_bg_tilemap->set_scrolly(0, scroll);
		break;
	case 3:
		m_fg_tilemap->set_scrolly(0, scroll);
		break;
	case 7:
		flip_screen_set(scroll & 1);
		break;
	default:
		logerror("CRTC WRITE REG: %x  Data: %03x\n",reg, scroll);
		break;
	}
}


/* emulation for schematic 9B */
void xevious_state::xevious_bs_w(offs_t offset, uint8_t data)
{
	m_xevious_bs[offset & 1] = data;
}

uint8_t xevious_state::xevious_bb_r(offs_t offset)
{
	uint8_t *rom2a = memregion("gfx4")->base();
	uint8_t *rom2b = rom2a+0x1000;
	uint8_t *rom2c = rom2a+0x3000;
	int adr_2b,adr_2c;
	int dat1,dat2;

	/* get BS to 12 bit data from 2A,2B */
	adr_2b = ((m_xevious_bs[1] & 0x7e) << 6) | ((m_xevious_bs[0] & 0xfe) >> 1);

	if (adr_2b & 1)
	{
		/* high bits select */
		dat1 = ((rom2a[adr_2b >> 1] & 0xf0) << 4) | rom2b[adr_2b];
	}
	else
	{
		/* low bits select */
		dat1 = ((rom2a[adr_2b >> 1] & 0x0f) << 8) | rom2b[adr_2b];
	}

	adr_2c = ((dat1 & 0x1ff) << 2) | ((m_xevious_bs[1] & 1) << 1) | (m_xevious_bs[0] & 1);
	if (dat1 & 0x400) adr_2c ^= 1;
	if (dat1 & 0x200) adr_2c ^= 2;

	if (offset & 1)
	{
		/* return BB1 */
		dat2 = rom2c[adr_2c | 0x800];
	}
	else
	{
		/* return BB0 */
		dat2 = rom2c[adr_2c];
		/* swap bit 6 & 7 */
		dat2 = bitswap<8>(dat2, 6,7,5,4,3,2,1,0);
		/* flip x & y */
		if (dat1 & 0x400) dat2 ^= 0x40;
		if (dat1 & 0x200) dat2 ^= 0x80;
	}
	return dat2;
}


/*
background pattern data

colorram mapping
b000-bfff background attribute
          bit 0-1 COL:palette set select
          bit 2-5 AN :color select
          bit 6   AFF:Y flip
          bit 7   PFF:X flip
c000-cfff background pattern name
          bit 0-7 PP0-7

seet 8A
                                        2     +-------+
COL0,1 -------------------------------------->|backg. |
                                        1     |color  |
PP7------------------------------------------>|replace|
                                        4     | ROM   |  6
AN0-3 --------------------------------------->|  4H   |-----> color code 6 bit
        1  +-----------+      +--------+      |  4F   |
COL0  ---->|B8   ROM 3C| 16   |custom  |  2   |       |
        8  |           |----->|shifter |----->|       |
PP0-7 ---->|B0-7 ROM 3D|      |16->2*8 |      |       |
           +-----------+      +--------+      +-------+

font rom controller
       1  +-----+     +--------+
ANF   --->| ROM |  8  |shift   |  1
       8  | 3B  |---->|reg     |-----> font data
PP0-7 --->|     |     |8->1*8  |
          +-----+     +--------+

font color ( not use color map )
        2  |
COL0-1 --->|  color code 6 bit
        4  |
AN0-3  --->|

sprite

ROM 3M,3L color replace table for sprite

*/




/***************************************************************************

  Display refresh

***************************************************************************/

void xevious_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	uint8_t const *const spriteram = m_xevious_sr3 + 0x780;
	uint8_t const *const spriteram_2 = m_xevious_sr1 + 0x780;
	uint8_t const *const spriteram_3 = m_xevious_sr2 + 0x780;

	for (int offs = 0;offs < 0x80;offs += 2)
	{
		if ((spriteram[offs + 1] & 0x40) == 0)  /* I'm not sure about this one */
		{
			int bank, code;
			if (spriteram_3[offs] & 0x80)
			{
				bank = 2;
				code = (spriteram[offs] & 0x3f) + 0x100;
			}
			else
			{
				bank = 2;
				code = spriteram[offs];
			}

			int color = spriteram[offs + 1] & 0x7f;
			int flipx = spriteram_3[offs] & 4;
			int flipy = spriteram_3[offs] & 8;

			int sx = spriteram_2[offs + 1] - 40 + 0x100*(spriteram_3[offs + 1] & 1);
			int sy = 28*8-spriteram_2[offs]-1;

			if (flip_screen())
			{
				flipx = !flipx;
				flipy = !flipy;
			}

			uint32_t transmask = m_palette->transpen_mask(*m_gfxdecode->gfx(bank), color, 0x80);

			if (spriteram_3[offs] & 2)  /* double height (?) */
			{
				if (spriteram_3[offs] & 1)  /* double width, double height */
				{
					code &= ~3;
					m_gfxdecode->gfx(bank)->transmask(bitmap,cliprect,
							code+3,color,flipx,flipy,
							flipx ? sx : sx+16,flipy ? sy-16 : sy,transmask);
					m_gfxdecode->gfx(bank)->transmask(bitmap,cliprect,
							code+1,color,flipx,flipy,
							flipx ? sx : sx+16,flipy ? sy : sy-16,transmask);
				}
				code &= ~2;
				m_gfxdecode->gfx(bank)->transmask(bitmap,cliprect,
						code+2,color,flipx,flipy,
						flipx ? sx+16 : sx,flipy ? sy-16 : sy,transmask);
				m_gfxdecode->gfx(bank)->transmask(bitmap,cliprect,
						code,color,flipx,flipy,
						flipx ? sx+16 : sx,flipy ? sy : sy-16,transmask);
			}
			else if (spriteram_3[offs] & 1) /* double width */
			{
				code &= ~1;
				m_gfxdecode->gfx(bank)->transmask(bitmap,cliprect,
						code,color,flipx,flipy,
						flipx ? sx+16 : sx,flipy ? sy-16 : sy,transmask);
				m_gfxdecode->gfx(bank)->transmask(bitmap,cliprect,
						code+1,color,flipx,flipy,
						flipx ? sx : sx+16,flipy ? sy-16 : sy,transmask);
			}
			else    /* normal */
			{
				m_gfxdecode->gfx(bank)->transmask(bitmap,cliprect,
						code,color,flipx,flipy,sx,sy,transmask);
			}
		}
	}
}


uint32_t xevious_state::screen_update_xevious(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}


void battles_state::machine_start()
{
	xevious_state::machine_start();

	save_item(NAME(m_customio));
	save_item(NAME(m_customio_command));
	save_item(NAME(m_customio_prev_command));
	save_item(NAME(m_customio_command_count));
	save_item(NAME(m_customio_data));
	save_item(NAME(m_sound_played));
}

void battles_state::machine_reset()
{
	xevious_state::machine_reset();

	m_customio_command = 0;
	m_customio_prev_command = 0;
	m_customio_command_count = 0;
	m_customio_data = 0;
	m_sound_played = 0;
}


void xevious_state::init_xevious()
{
	uint8_t *rom = memregion("gfx3")->base() + 0x5000;
	for (int i = 0; i < 0x2000; i++)
		rom[i + 0x2000] = rom[i] >> 4;
}

void xevious_state::init_xevios()
{
	/* convert one of the sprite ROMs to the format used by Xevious */
	uint8_t *rom = memregion("gfx3")->base();
	for (int A = 0x5000; A < 0x7000; A++)
	{
		rom[A] = bitswap<8>(rom[A],1,3,5,7,0,2,4,6);
	}

	/* convert one of tile map ROMs to the format used by Xevious */
	rom = memregion("gfx4")->base();
	for (int A = 0x0000; A < 0x1000; A++)
	{
		rom[A] = bitswap<8>(rom[A],3,7,5,1,2,6,4,0);
	}

	init_xevious();
}


static INPUT_PORTS_START( xevious )
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
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(    0x18, "10K, 40K, Every 40K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x14, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x10, "20K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x1c, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00) // factory default = "20K, 60K, Every60K"
	PORT_DIPSETTING(    0x0c, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x08, "20K, 80K, Every 80K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x04, "20K and 60K Only" )      PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x18, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x14, "20K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x1c, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x0c, "20K, 80K, Every 80K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x08, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x04, "20K and 80K Only" )      PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:6,7")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x60, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x02, 0x02, "Flags Award Bonus Life" )    PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:6,7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* same as xevious but different "Coin B" Dip Switch and "Copyright" Dip Switch instead of "Freeze" */
static INPUT_PORTS_START( xeviousa )
	PORT_INCLUDE( xevious )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	/* when switch is on Namco, high score names are 10 letters long */
	PORT_DIPNAME( 0x80, 0x80, "Copyright" )         PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "Namco" )
	PORT_DIPSETTING(    0x80, "Atari/Namco" )
INPUT_PORTS_END

/* same as xevious but "Copyright" Dip Switch instead of "Freeze" */
static INPUT_PORTS_START( xeviousb )
	PORT_INCLUDE( xevious )

	PORT_MODIFY("DSWB")
	/* when switch is on Namco, high score names are 10 letters long */
	PORT_DIPNAME( 0x80, 0x80, "Copyright" )         PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "Namco" )
	PORT_DIPSETTING(    0x80, "Atari/Namco" )
INPUT_PORTS_END

/* same as xevious but different "Coin B" Dip Switch and inverted "Freeze" Dip Switch */
static INPUT_PORTS_START( sxevious )
	PORT_INCLUDE( xevious )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )            PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/**********************************************************************************************
  Xevious & clones
**********************************************************************************************/

/*
    Xevious - Namco Version

    Single/Dual Board?
*/

ROM_START( xevious )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "xvi_1.3p",     0x0000, 0x1000, CRC(09964dda) SHA1(4882b25b0938a903f3a367455ba788a30759b5b0) )
	ROM_LOAD( "xvi_2.3m",     0x1000, 0x1000, CRC(60ecce84) SHA1(8adc60a5fcbca74092518dbc570ffff0f04c5b17) )
	ROM_LOAD( "xvi_3.2m",     0x2000, 0x1000, CRC(79754b7d) SHA1(c6a154858716e1f073b476824b183de20e06d093) )
	ROM_LOAD( "xvi_4.2l",     0x3000, 0x1000, CRC(c7d4bbf0) SHA1(4b846de204d08651253d3a141677c8a31626af07) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xvi_5.3f",     0x0000, 0x1000, CRC(c85b703f) SHA1(15f1c005b9d806a384ab1f2240b9c580bfe83893) )
	ROM_LOAD( "xvi_6.3j",     0x1000, 0x1000, CRC(e18cdaad) SHA1(6b79efee1a9642edb9f752101737132401248aed) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "xvi-3.1f",     0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) ) /* N82S153N */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
    Xevious - Atari Version

    CPU/Sound Board: A039785
    Video Board:     A039787
*/

ROM_START( xeviousa )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "xea-1m-a.bin", 0x0000, 0x2000, CRC(8c2b50ec) SHA1(f770873b711d838556dde67a8aac8a7f572fcc5b) )
	ROM_LOAD( "xea-1l-a.bin", 0x2000, 0x2000, CRC(0821642b) SHA1(c6c322c61d0985a2ac59f5e92d4e351107afb9eb) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xea-4c-a.bin", 0x0000, 0x2000, CRC(14d8fa03) SHA1(e8114141394adda86184b146f2497cfeef7fc2eb) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "xvi-3.1f",     0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) ) /* N82S153N - 137294-001*/

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( xeviousb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "1m.bin",       0x0000, 0x2000, CRC(e82a22f6) SHA1(6fd09a7fb263cda3d5268cc6d7bfe71a57ac4b47) )
	ROM_LOAD( "1l.bin",       0x2000, 0x2000, CRC(13831df9) SHA1(a7892d1d98868a83a5d1092976873b82577e9e94) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "4c.bin",       0x0000, 0x2000, CRC(827e7747) SHA1(d22645d71b164613834336e26e6942506a0e7eaa) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "xvi-3.1f",     0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) ) /* N82S153N - 137294-001*/

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( xeviousc )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "xvi_u_.3p",    0x0000, 0x1000, CRC(7b203868) SHA1(3bafaa42bccddfaf8d9197e93416a731b7f8fb94) )
	ROM_LOAD( "xv_2-2.3m",    0x1000, 0x1000, CRC(b6fe738e) SHA1(23cdf1f2c2642f9bc3f843b5c338372027032380) )
	ROM_LOAD( "xv_2-3.2m",    0x2000, 0x1000, CRC(dbd52ff5) SHA1(eb42393720fc1fd4a1f6cdba87ac4177fd5827fe) )
	ROM_LOAD( "xvi_u_.2l",    0x3000, 0x1000, CRC(ad12af53) SHA1(ff3a96d6f7357fb2d33cd9d77d53477b9071ffc9) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xv2_5.3f",     0x0000, 0x1000, CRC(f8cc2861) SHA1(9b02c00cff6c771d46776416295f9e12a2166cc5) )
	ROM_LOAD( "xvi_6.3j",     0x1000, 0x1000, CRC(e18cdaad) SHA1(6b79efee1a9642edb9f752101737132401248aed) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "xvi-3.1f",     0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) ) /* N82S153N - 137294-001*/

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( xeviousd ) // original Atari PCBs A039785 + A039787 (both PCBs had a 21 handwritten with a black marker)
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "1983_atari_8309_136018_218.1m", 0x0000, 0x2000, CRC(cf690308) SHA1(f036c2c9ce161becf24e3fa336c1ee3926bdc6ca) )
	ROM_LOAD( "1983_atari_8307_136018_219.1l", 0x2000, 0x2000, CRC(408c4b64) SHA1(91cbb46334d2c8e7d0e1281197092ccd2e45035b) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "1983_atari_8306_136018_120.4c", 0x0000, 0x2000, CRC(14d8fa03) SHA1(e8114141394adda86184b146f2497cfeef7fc2eb) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "1983_atari_8306_136018_127.2c", 0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "1983_atari_8307_136018_104.3b", 0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "1983_atari_8307_136018_105.3c", 0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "1983_atari_8307_136018_106.3d", 0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "1983_atari_8307_136018_107.4m", 0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "1983_atari_8307_136018_109.4p", 0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "1983_atari_8307_136018_108.4n", 0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "1983_atari_8307_136018_110.4r", 0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                                  0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "1983_atari_8307_136018_101.2a", 0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "1983_atari_8307_136018_102.2b", 0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "1983_atari_8307_136018_103.2c", 0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	// from here on nothing was dumped for this set. They all probably match, given similarity.
	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "1983_atari_136018_115.6a", 0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "1983_atari_136018_116.6d", 0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "1983_atari_136018_117.6e", 0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "1983_atari_136018_114.4h", 0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "1983_atari_136018_113.4f", 0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "1983_atari_136018_111.3l", 0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "1983_atari_136018_112.3m", 0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "n82s153n.1f", 0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "1983_atari_136018_129.8m", 0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "1983_atari_136018_128.6m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
    Xevious Bootleg

    Dual Boards with no markings except row/column designations
*/

ROM_START( xevios )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "4.7h",         0x0000, 0x1000, CRC(1f8ca4c0) SHA1(9fdaa2e0016c07e274544f8334778fe81b8344a5) )
	ROM_LOAD( "5.6h",         0x1000, 0x1000, CRC(2e47ce8f) SHA1(fb35dd086e98279a5f17036f624ef5294c777d84) )
	ROM_LOAD( "6.5h",         0x2000, 0x1000, CRC(79754b7d) SHA1(c6a154858716e1f073b476824b183de20e06d093) )
	ROM_LOAD( "7.4h",         0x3000, 0x1000, CRC(17f48277) SHA1(ffe590acf07985355ef91fbe0fc3dcf6e8fd62fd) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "8.2h",         0x0000, 0x1000, CRC(c85b703f) SHA1(15f1c005b9d806a384ab1f2240b9c580bfe83893) )
	ROM_LOAD( "9.1h",         0x1000, 0x1000, CRC(e18cdaad) SHA1(6b79efee1a9642edb9f752101737132401248aed) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "3.9h",         0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "17.8f",        0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "18.9f",        0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "19.11f",       0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "13.4d",        0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "15.7d",        0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "14.6d",        0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "16.8d",        0x5000, 0x2000, CRC(44262c04) SHA1(4291f83193d11064c2ba6a9af27951b93bb945c3) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "10.1d",        0x0000, 0x1000, CRC(10baeebb) SHA1(c544c9e0bb7a1ef93b3f2c2c1397f659d5334373) )
	ROM_LOAD( "11.2d",        0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "12.3d",        0x3000, 0x1000, CRC(51a4e83b) SHA1(fbf3b1e47b75c5e0b297ee2cd6597b1dfd80bc6f) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "8.12h",        0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "9.13h",        0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "10.14h",       0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "7.14f",        0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "6.13f",        0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "6.5c",         0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "5.6c",         0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "1.10f",        0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "2.10c",        0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "user1", 0 ) /* unknown roms */
	/* extra ROMs (function unknown, could be emulation of the custom I/O */
	/* chip with a Z80): */
	ROM_LOAD( "1.16j",        0x0000, 0x1000, CRC(2618f0ce) SHA1(54e8644b5609d6f6ec717a7469c76901eb79f26e) )
	ROM_LOAD( "2.17b",        0x1000, 0x2000, CRC(de359fac) SHA1(a55df9984bfffafeadae8a5a63b07f1fa9c5eebf) )

	ROM_REGION( 0x002c, "pals", 0 ) /* Located on the video board */
	ROM_LOAD( "pal10l8.16a.bin", 0x0000, 0x002c, CRC(6fb9bd9a) SHA1(698b5fc19f5873b02a4bed7d9ec1f24763a6fef7) )
ROM_END

/*
    Battles (Xevious Bootleg)

    Three Boards with no markings except row/column designations
*/

ROM_START( battles )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU (Located on the top board) */
	ROM_LOAD( "bg1.d9",      0x0000, 0x2000, CRC(b6e4f4f3) SHA1(ceaaa63b50e75dcb05aeb68574336dfe56a8434a) )
	ROM_LOAD( "bg2.d10",     0x2000, 0x2000, CRC(47017bc8) SHA1(0da73ae079fb6a64eed56197e2c88609ef34166c) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU (Located on the top board) */
	ROM_LOAD( "bg3.d12",     0x0000, 0x2000, CRC(0ede5706) SHA1(65b235c5abe487612e11d0235410f1ca59b06e95) )

	ROM_REGION( 0x10000, "sub2", 0 ) /* (Located on the top board) */
	ROM_LOAD( "bg4.d13",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x10000, "sub3", 0 ) /* 64k for the CUSTOM I/O Emulation CPU (Located on the top board) */
	ROM_LOAD( "bg5.h5",      0x0000, 0x1000, CRC(23107dfb) SHA1(74c49a5648faab632ae5ed8dd18a1d8b39837e2d) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* (Located on the middle board) */
	ROM_LOAD( "bg9.c10",     0x0000, 0x1000, CRC(5bd6e9ae) SHA1(f16c7eec39fce856c775b2b81ab55fb42376850e) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 ) /* (Located on the middle board) */
	ROM_LOAD( "bg10.c8",     0x0000, 0x1000, CRC(b43ea55d) SHA1(06f4c4e7fc71b9e173c3bdf91c40f47750051b5e) )    /* bg pattern B0 */
	ROM_LOAD( "bg11.c7",     0x1000, 0x1000, CRC(73603931) SHA1(1f7824b107a5a3d5c3434f02f17173a1f85fd29c) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 ) /* (Located on the bottom board) */
	ROM_LOAD( "bg13.c6",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "bg14.c4",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "bg12.c8",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "bg15.c2",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps (Located on the middle board) */
	ROM_LOAD( "bg6.b14",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "bg7.b12",     0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "bg8.b11",     0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",    0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",    0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",   0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "b_-bpr.bin",  0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "b_6bpr.bin",  0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "b_4bpr.bin",  0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "b_5bpr.bin",  0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",    0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( battles2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "bg1.h7",        0x0000, 0x1000, CRC(475d848a) SHA1(44a4abd1e922ba1fd89a5066a5d325f748236366) )
	ROM_LOAD( "bg2.h6",        0x1000, 0x1000, CRC(c66f2a87) SHA1(34f029253c8d49678d90aa7a37dba6bf856c5f0e) )
	ROM_LOAD( "bg3.h5",        0x2000, 0x1000, CRC(79754b7d) SHA1(c6a154858716e1f073b476824b183de20e06d093) )
	ROM_LOAD( "bg4.h4",        0x3000, 0x1000, CRC(3da1a0a2) SHA1(5924cf88349808866bd160e77d0bddcf9247cbfc) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bg5.h2",        0x0000, 0x1000, CRC(c85b703f) SHA1(15f1c005b9d806a384ab1f2240b9c580bfe83893) )
	ROM_LOAD( "bg6.h1",        0x1000, 0x1000, CRC(e18cdaad) SHA1(6b79efee1a9642edb9f752101737132401248aed) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "bg7.h8",        0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bg17.f8",       0x0000, 0x1000, CRC(9571e400) SHA1(23027b9ebd006aa4635f89921824bbb460463a6e) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bg18.f9",       0x0000, 0x1000, CRC(b43ea55d) SHA1(06f4c4e7fc71b9e173c3bdf91c40f47750051b5e) )    /* bg pattern B0 */
	ROM_LOAD( "bg19.f11",      0x1000, 0x1000, CRC(73603931) SHA1(1f7824b107a5a3d5c3434f02f17173a1f85fd29c) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "bg13.d4",       0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "bg15.d7",       0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "bg14.d6",       0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "bg16.d8",       0x5000, 0x2000, CRC(44262c04) SHA1(4291f83193d11064c2ba6a9af27951b93bb945c3) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                  0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "bg10.d1",       0x0000, 0x1000, CRC(10baeebb) SHA1(c544c9e0bb7a1ef93b3f2c2c1397f659d5334373) )
	ROM_LOAD( "bg11.d2",       0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "bg12.d3",       0x3000, 0x1000, CRC(51a4e83b) SHA1(fbf3b1e47b75c5e0b297ee2cd6597b1dfd80bc6f) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "82s123_bg.h12", 0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "82s123_bg.h13", 0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "82s123_bg.h14", 0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "7643_bg.f14",   0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "7643_bg.f13",   0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "7643_bg.c5",    0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "7643_bg.c6",    0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s123_bg.f10", 0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "82s123_bg.c10", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "user1", 0 ) /* unknown roms */
	/* extra ROMs (function unknown, could be emulation of the custom I/O */
	/* chip with a Z80): */
	ROM_LOAD( "bg9.j16",        0x0000, 0x1000, CRC(2618f0ce) SHA1(54e8644b5609d6f6ec717a7469c76901eb79f26e) )
	ROM_LOAD( "bg8.b17",        0x1000, 0x2000, CRC(de359fac) SHA1(a55df9984bfffafeadae8a5a63b07f1fa9c5eebf) )

	ROM_REGION( 0x002c, "pals", 0 ) /* Located on the video board */
	ROM_LOAD( "pal10l8_bg.a16", 0x0000, 0x002c, CRC(10ea110e) SHA1(8f165a04a221e6492d4cac0f485927b8caf58160) )
ROM_END

ROM_START( sxevious )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "cpu_3p.rom",   0x0000, 0x1000, CRC(1c8d27d5) SHA1(2c41303d8c74acb5840295a4b460a39a9a8e21bb) )
	ROM_LOAD( "cpu_3m.rom",   0x1000, 0x1000, CRC(fd04e615) SHA1(7169e7f3bd1e9cfae9671b89f2a45f56b968e1ff) )
	ROM_LOAD( "xv3_3.2m",     0x2000, 0x1000, CRC(294d5404) SHA1(ecc39fb2c0065a36f20541747089b4e30dfb99b1) )
	ROM_LOAD( "xv3_4.2l",     0x3000, 0x1000, CRC(6a44bf92) SHA1(0ca726f7f9528789f2a718df55e59406a283cdfa) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xv3_5.3f",     0x0000, 0x1000, CRC(d4bd3d81) SHA1(5831bb306bd650779207936bfd00f25864733abb) )
	ROM_LOAD( "xv3_6.3j",     0x1000, 0x1000, CRC(af06be5f) SHA1(5a020822387ab8c69214db961180760fa9853e6e) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( sxeviousj )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "xv3_1.3p",     0x0000, 0x1000, CRC(afbc3372) SHA1(9001856aad0f31b40443f21b7a895e4101684307) )
	ROM_LOAD( "xv3_2.3m",     0x1000, 0x1000, CRC(1854a5ee) SHA1(2fb4034d9d757376df59378df539bf41d99ed43e) )
	ROM_LOAD( "xv3_3.2m",     0x2000, 0x1000, CRC(294d5404) SHA1(ecc39fb2c0065a36f20541747089b4e30dfb99b1) )
	ROM_LOAD( "xv3_4.2l",     0x3000, 0x1000, CRC(6a44bf92) SHA1(0ca726f7f9528789f2a718df55e59406a283cdfa) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xv3_5.3f",     0x0000, 0x1000, CRC(d4bd3d81) SHA1(5831bb306bd650779207936bfd00f25864733abb) )
	ROM_LOAD( "xv3_6.3j",     0x1000, 0x1000, CRC(af06be5f) SHA1(5a020822387ab8c69214db961180760fa9853e6e) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

} // anonymous namespace


/* Original Namco hardware, with Namco Customs */

//    YEAR, NAME,      PARENT,   MACHINE, INPUT,    STATE,         INIT,         MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1982, xevious,   0,        xevious, xevious,  xevious_state, init_xevious, ROT90,  "Namco", "Xevious (Namco)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, xeviousa,  xevious,  xevious, xeviousa, xevious_state, init_xevious, ROT90,  "Namco (Atari license)", "Xevious (Atari, harder)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, xeviousb,  xevious,  xevious, xeviousb, xevious_state, init_xevious, ROT90,  "Namco (Atari license)", "Xevious (Atari)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, xeviousc,  xevious,  xevious, xeviousa, xevious_state, init_xevious, ROT90,  "Namco (Atari license)", "Xevious (Atari, Namco PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, xeviousd,  xevious,  xevious, xeviousa, xevious_state, init_xevious, ROT90,  "Namco (Atari license)", "Xevious (Atari, set 4)", MACHINE_SUPPORTS_SAVE )

GAME( 1984, sxevious,  0,        xevious, sxevious, xevious_state, init_xevious, ROT90,  "Namco", "Super Xevious", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sxeviousj, sxevious, xevious, sxevious, xevious_state, init_xevious, ROT90,  "Namco", "Super Xevious (Japan)", MACHINE_SUPPORTS_SAVE )

/* Bootlegs with replacement I/O chips */

GAME( 1982, xevios,    xevious,  xevious, xevious,  xevious_state, init_xevios,  ROT90,  "bootleg", "Xevios", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, battles,   xevious,  battles, xevious,  battles_state, init_xevious, ROT90,  "bootleg", "Battles (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, battles2,  xevious,  xevious, xevious,  xevious_state, init_xevios,  ROT90,  "bootleg", "Battles (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
