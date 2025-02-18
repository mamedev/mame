// license:BSD-3-Clause
// copyright-holders:David Graves
/***************************************************************************

Slapshot (c) Taito 1994
Operation Wolf 3 (c) Taito 1994
--------

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria.)

                *****

Slapshot uses one or two newer Taito custom ics, but the hardware is
very similar to the Taito F2 system, especially F2 games using the same
TC0480SCP tilemap generator (e.g. Metal Black).

This game has 6 separate layers of graphics - four 32x32 tiled scrolling
zoomable background planes of 16x16 tiles, a text plane with 64x64 8x8
character tiles with character definitions held in ram, and a sprite
plane with zoomable 16x16 sprites. This sprite system appears to be
identical to the one used in F2 and F3 games.

Slapshot switches in and out of the double-width tilemap mode of the
TC0480SCP. This is unusual, as most games stick to one width.

The palette generator is 8 bits per color gun like the Taito F3 system.
Like Metal Black the palette space is doubled, and the first half used
for sprites only so the second half can be devoted to tilemaps.

The main cpu is a 68000.

There is a slave Z80 which interfaces with a YM2610B for sound.
Commands are written to it by the 68000 (as in the Taito F2 games).


Slapshot custom ics
-------------------

TC0480SCP (IC61)    - known tilemap chip
TC0640FIO (IC83)    - new version of TC0510NIO io chip?
TC0650FDA (IC84)    - (palette?)
TC0360PRI (IC56)    - (common in pri/color combo on F2 boards)
TC0530SYC (IC58)    - known sound comm chip
TC0520TBC (IC36)    - known object chip
TC0540OBN (IC54)    - known object chip


TODO
====

Some hanging notes (try F2 while music is playing).

Sprite colors issue: when you do a super-shot, on the cut
screen the man (it's always the American) should be black.

Col $f8 is used for the man, col $fc for the red/pink
"explosion" under the puck. (Use this to track where they are
in spriteram quickly.) Both of these colors are only set
when the first super-shot happens, so it's clear those
colors are for the super-shot... but screenshot evidence
proves the man should be entirely black.

Extract common sprite stuff from this and taito_f2 ?


Code
----
$854 marks start of service mode

-----------------

Operation Wolf 3 is on almost identical hardware to Slapshot. It uses
far more graphics data and samples than Slapshot.

Compared to Taito's gun game Under Fire (1993), the hardware here is
obviously underpowered. Large data roms help the 68000 throw around the
gfx (a method used in Dino Rex) but can't disguise that it should have
been done using enhanced Z system or F3 system hardware.

***************************************************************************

Operation Wolf 3 (US Version) - (c) 1994 Taito America Corp.

Main Board K11E0801A - Not to Scale:-)

       D74 17                       Sub Board Connector
  D74 20      MC68000P12F                                    D74-05     SW2
       D74 18                                                D74-06
  D74 16         MK48T08B-10        TC0480SCP                84256A-70L
                                                             84256A-70L
MB8421-90LP  D74-02  84256A-70L             26.6860MHz  TC0640FIO
             D74-03  84256A-70L
             D74-04
                     TC0540OBN    TC0360PRI              TC0650FDA
                 TC0520TBC
                                    32.0000MHz    Y3016-F
               D74-01     TC0530SYC     D74 19    YM2610B
                                                  Z0840004PSC



Sub Board K91X0488A
 basically a few connectors, Caps, resistors & ADC0809CNN

Chips:
 Main: MC68000P12F
Sound: Z084004PSC, YM2610B, Y3016-F
  OSC: 32.000MHz, 26.6860MHz

Taito Custom:
  TC0480SCP
  TC0640FIO
  TC0650FDA
  TC0530SYC
  TC0520TBC
  TC0540OBN
  TC0360PRI

ST TimeKeeper Ram MK48T08B-10 - Lithium Battery backed RAM chip
MB8421-90LP - Dual Port SRAM
ADC0809CNN - 8-bit Microprocessor Compatible A/D Converter
             With 8-Channel Multiplexer
 DataSheet:  http://www.national.com/ds/AD/ADC0808.pdf

Region byte at offset 0x031:
    d74_21.1  0x02  World Version
    d74_20.1  0x01  US Version
    d74_15.1  0x00  Japanese Version
***************************************************************************/

#include "emu.h"
#include "slapshot.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/adc0808.h"
#include "machine/timekpr.h"
#include "sound/ymopn.h"
#include "screen.h"
#include "speaker.h"


/***********************************************************
                INTERRUPTS
***********************************************************/

TIMER_CALLBACK_MEMBER(slapshot_state::trigger_int6)
{
	m_maincpu->set_input_line(6, HOLD_LINE);
}


INTERRUPT_GEN_MEMBER(slapshot_state::interrupt)
{
	m_int6_timer->adjust(m_maincpu->cycles_to_attotime(200000 - 500));
	device.execute().set_input_line(5, HOLD_LINE);
}


/**********************************************************
                GAME INPUTS
**********************************************************/

u16 slapshot_state::service_input_r(offs_t offset)
{
	switch (offset)
	{
		case 0x03:
			return ((m_io_system->read() & 0xef) |
					(m_io_service->read() & 0x10))  << 8;  /* IN3 + service switch */

		default:
			return m_tc0640fio->read(offset) << 8;
	}
}

void slapshot_state::coin_control_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 0));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 1));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3));
}

/*****************************************************
                SOUND
*****************************************************/

void slapshot_state::sound_bankswitch_w(u8 data)
{
	m_z80bank->set_entry(data & 3);
}

/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void slapshot_state::slapshot_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x500000, 0x50ffff).ram(); /* main RAM */
	map(0x600000, 0x60ffff).ram().share(m_spriteram);   /* sprite ram */
	map(0x700000, 0x701fff).ram().share(m_spriteext);   /* debugging */
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));    /* tilemaps */
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x900000, 0x907fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa00000, 0xa03fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00); /* nvram (only low bytes used) */
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0xff00);  /* priority chip */
	map(0xc00000, 0xc0000f).rw(m_tc0640fio, FUNC(tc0640fio_device::halfword_byteswap_r), FUNC(tc0640fio_device::halfword_byteswap_w));
	map(0xc00020, 0xc0002f).r(FUNC(slapshot_state::service_input_r));  /* service mirror */
	map(0xd00000, 0xd00000).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0xd00002, 0xd00002).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

void slapshot_state::opwolf3_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x500000, 0x50ffff).ram(); /* main RAM */
	map(0x600000, 0x60ffff).ram().share(m_spriteram);   /* sprite ram */
	map(0x700000, 0x701fff).ram().share(m_spriteext);   /* debugging */
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));    /* tilemaps */
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x900000, 0x907fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa00000, 0xa03fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00); /* nvram (only low bytes used) */
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0xff00);  /* priority chip */
	map(0xc00000, 0xc0000f).rw(m_tc0640fio, FUNC(tc0640fio_device::halfword_byteswap_r), FUNC(tc0640fio_device::halfword_byteswap_w));
	map(0xc00020, 0xc0002f).r(FUNC(slapshot_state::service_input_r));   /* service mirror */
	map(0xd00000, 0xd00000).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0xd00002, 0xd00002).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xe00000, 0xe0000f).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask16(0xff00);
//  map(0xe80000, 0xe80001) // gun recoil here?
}


/***************************************************************************/

void slapshot_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_z80bank);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610b_device::read), FUNC(ym2610b_device::write));
	map(0xe200, 0xe200).nopr().w(m_tc0140syt, FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw(m_tc0140syt, FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).nopw(); /* pan */
	map(0xea00, 0xea00).nopr();
	map(0xee00, 0xee00).nopw(); /* ? */
	map(0xf000, 0xf000).nopw(); /* ? */
	map(0xf200, 0xf200).w(FUNC(slapshot_state::sound_bankswitch_w));
}


/***********************************************************
             INPUT PORTS (DIPs in nvram)
***********************************************************/

/* Tags below are the ones expected by TC0640FIO_halfword_byteswap_r */
static INPUT_PORTS_START( slapshot )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) /* bit is service switch at c0002x */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("SERVICE")      /* IN5, so we can OR in service switch */
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
INPUT_PORTS_END

/* Tags below are the ones expected by TC0640FIO_halfword_byteswap_r */
static INPUT_PORTS_START( opwolf3 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("1 Player Start/Button3")// also button 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("2 Player Start/Button3")// also button 3
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* bit is service switch at c0002x */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)

	PORT_START("GUN1X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("GUN1Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("GUN2X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("GUN2Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(2)
INPUT_PORTS_END

/***********************************************************
                GFX DECODING

***********************************************************/

static const gfx_layout layout_6bpp_hi =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0,1) },
	{ STEP4(3*2,-2), STEP4(7*2,-2), STEP4(11*2,-2), STEP4(15*2,-2) },
	{ STEP16(0,16*2) },
	16*16*2
};

static GFXDECODE_START( gfx_slapshot )
	GFXDECODE_ENTRY( "sprites",    0, gfx_16x16x4_packed_lsb, 0, 256 ) // low 4bpp of 6bpp sprites
	GFXDECODE_ENTRY( "sprites_hi", 0, layout_6bpp_hi,         0, 256 ) // hi 2bpp of 6bpp sprites
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

void slapshot_state::machine_start()
{
	m_z80bank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);

	m_int6_timer = timer_alloc(FUNC(slapshot_state::trigger_int6), this);
}


void slapshot_state::slapshot(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2); /* MC68000P12F, 32MHz/2 or 26.6860MHz/2 ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &slapshot_state::slapshot_map);
	m_maincpu->set_vblank_int("screen", FUNC(slapshot_state::interrupt));

	z80_device &audiocpu(Z80(config, "audiocpu", 32_MHz_XTAL/8)); /* 4 MHz */
	audiocpu.set_addrmap(AS_PROGRAM, &slapshot_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0640FIO(config, m_tc0640fio, 0);
	m_tc0640fio->read_1_callback().set_ioport("COINS");
	m_tc0640fio->read_2_callback().set_ioport("BUTTONS");
	m_tc0640fio->read_3_callback().set_ioport("SYSTEM");
	m_tc0640fio->write_4_callback().set(FUNC(slapshot_state::coin_control_w));
	m_tc0640fio->read_7_callback().set_ioport("JOY");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(slapshot_state::screen_update));
	screen.screen_vblank().set(FUNC(slapshot_state::screen_vblank_no_buffer));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_slapshot);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 8192);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(30 + 3, 9);
	m_tc0480scp->set_offsets_tx(-1, -1);
	m_tc0480scp->set_offsets_flip(0, 2);
	m_tc0480scp->set_col_base(4096);

	TC0360PRI(config, m_tc0360pri, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 32_MHz_XTAL/4)); /* 8 MHz */
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);

	MK48T08(config, "mk48t08", 0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->nmi_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	m_tc0140syt->reset_callback().set_inputline("audiocpu", INPUT_LINE_RESET);
}

void slapshot_state::opwolf3(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2); /* MC68000P12F, 32MHz/2 or 26.6860MHz/2 ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &slapshot_state::opwolf3_map);
	m_maincpu->set_vblank_int("screen", FUNC(slapshot_state::interrupt));

	z80_device &audiocpu(Z80(config, "audiocpu", 32_MHz_XTAL/8)); /* 4 MHz */
	audiocpu.set_addrmap(AS_PROGRAM, &slapshot_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	adc0809_device &adc(ADC0809(config, "adc", 500000)); // unknown clock
	adc.eoc_ff_callback().set_inputline("maincpu", 3);
	adc.in_callback<0>().set_ioport("GUN1X");
	adc.in_callback<1>().set_ioport("GUN1Y");
	adc.in_callback<2>().set_ioport("GUN2X");
	adc.in_callback<3>().set_ioport("GUN2Y");

	TC0640FIO(config, m_tc0640fio, 0);
	m_tc0640fio->read_1_callback().set_ioport("COINS");
	m_tc0640fio->read_2_callback().set_ioport("BUTTONS");
	m_tc0640fio->read_3_callback().set_ioport("SYSTEM");
	m_tc0640fio->write_4_callback().set(FUNC(slapshot_state::coin_control_w));
	m_tc0640fio->read_7_callback().set_ioport("JOY");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(slapshot_state::screen_update));
	screen.screen_vblank().set(FUNC(slapshot_state::screen_vblank_no_buffer));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_slapshot);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 8192);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(30 + 3, 9);
	m_tc0480scp->set_offsets_tx(-1, -1);
	m_tc0480scp->set_offsets_flip(0, 2);
	m_tc0480scp->set_col_base(4096);

	TC0360PRI(config, m_tc0360pri, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 32_MHz_XTAL/4)); /* 8 MHz */
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);

	MK48T08(config, "mk48t08", 0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->nmi_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	m_tc0140syt->reset_callback().set_inputline("audiocpu", INPUT_LINE_RESET);
}


/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( slapshot )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "promat.ic3",  0x00000, 0x80000, CRC(58e61833) SHA1(35ee07ab165618686ee98d60444d77070853d09b) ) /* yellow PROMAT label, but should be D71-xx - need to verify number */
	ROM_LOAD16_BYTE( "promat.ic1",  0x00001, 0x80000, CRC(4d404f76) SHA1(d74b9d67e0fd35884526f79aa00f76bf936ab79f) ) /* yellow PROMAT label, but should be D71-yy - need to verify number */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD    ( "d71-07.77",    0x00000, 0x10000, CRC(dd5f670c) SHA1(743a9563c40fe40178c9ec8eece71a08380c2239) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d71-04.79", 0x00000, 0x80000, CRC(b727b81c) SHA1(9f56160e2b3e4d59cfa96b5c013f4e368781666e) )  /* SCR */
	ROM_LOAD32_WORD( "d71-05.80", 0x00002, 0x80000, CRC(7b0f5d6d) SHA1(a54e4a651dc7cdc160286afb3d38531c7b9396b1) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "d71-01.23", 0x000000, 0x100000, CRC(0b1e8c27) SHA1(ffa452f7414f3d61edb69bb61b29a0cc8d9176d0) )    /* OBJ 4bpp */
	ROM_LOAD16_BYTE( "d71-02.24", 0x000001, 0x100000, CRC(ccaaea2d) SHA1(71b507f215f37e991abae5523642417a6b23a70d) )

	ROM_REGION( 0x100000, "sprites_hi", 0 )
	ROM_LOAD       ( "d71-03.25", 0x000000, 0x100000, CRC(dccef9ec) SHA1(ee7a49727b822cf4c1d7acff994b77ea6191c423) )    /* OBJ 2bpp */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "d71-06.37", 0x00000, 0x80000, CRC(f3324188) SHA1(70dd724441eae8614218bc7f0f51860bd2462f0c) )

	/* no Delta-T samples */

//  Pals (not dumped)
//  ROM_LOAD( "d71-08.40",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-09.57",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-10.60",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-11.42",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-12.59",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-13.8",   0x00000, 0x00???, NO_DUMP )
ROM_END

ROM_START( slapshotj )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d71-15.3",  0x00000, 0x80000, CRC(1470153f) SHA1(63fd5314fcaafba7326fd9481e3c686901dde65c) )
	ROM_LOAD16_BYTE( "d71-16.1",  0x00001, 0x80000, CRC(f13666e0) SHA1(e8b475163ea7da5ee3f2b900004cc67c684bab75) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD    ( "d71-07.77",    0x00000, 0x10000, CRC(dd5f670c) SHA1(743a9563c40fe40178c9ec8eece71a08380c2239) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d71-04.79", 0x00000, 0x80000, CRC(b727b81c) SHA1(9f56160e2b3e4d59cfa96b5c013f4e368781666e) )  /* SCR */
	ROM_LOAD32_WORD( "d71-05.80", 0x00002, 0x80000, CRC(7b0f5d6d) SHA1(a54e4a651dc7cdc160286afb3d38531c7b9396b1) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "d71-01.23", 0x000000, 0x100000, CRC(0b1e8c27) SHA1(ffa452f7414f3d61edb69bb61b29a0cc8d9176d0) )    /* OBJ 4bpp */
	ROM_LOAD16_BYTE( "d71-02.24", 0x000001, 0x100000, CRC(ccaaea2d) SHA1(71b507f215f37e991abae5523642417a6b23a70d) )

	ROM_REGION( 0x100000, "sprites_hi", 0 )
	ROM_LOAD       ( "d71-03.25", 0x000000, 0x100000, CRC(dccef9ec) SHA1(ee7a49727b822cf4c1d7acff994b77ea6191c423) )    /* OBJ 2bpp */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "d71-06.37", 0x00000, 0x80000, CRC(f3324188) SHA1(70dd724441eae8614218bc7f0f51860bd2462f0c) )

	/* no Delta-T samples */

//  Pals (not dumped)
//  ROM_LOAD( "d71-08.40",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-09.57",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-10.60",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-11.42",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-12.59",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-13.8",   0x00000, 0x00???, NO_DUMP )
ROM_END

ROM_START( opwolf3 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d74_16.3",  0x000000, 0x80000, CRC(198ff1f6) SHA1(f5b51e39cd73ea56cbf53731d3c885bfcecbd696) )
	ROM_LOAD16_BYTE( "d74_21.1",  0x000001, 0x80000, CRC(c61c558b) SHA1(6340eb83ba4cd8d7c63b22ea738c8367c87c1de1) )
	ROM_LOAD16_BYTE( "d74_18.18", 0x100000, 0x80000, CRC(bd5d7cdb) SHA1(29f1cd7b86bc05f873e93f088194113da87a3b86) ) // data ???
	ROM_LOAD16_BYTE( "d74_17.17", 0x100001, 0x80000, CRC(ac35a672) SHA1(8136bd076443bfaeb3d339971d88951e8b2b59b4) ) // data ???

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "d74_22.77", 0x00000, 0x10000, CRC(118374a6) SHA1(cc1d0d28efdf1df3e648e7d932405811854ba4ee) )

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d74_05.80", 0x000000, 0x200000, CRC(85ea64cc) SHA1(1960a934191c451df1554323d47f6fc64939b0ce) ) /* SCR */
	ROM_LOAD32_WORD( "d74_06.81", 0x000002, 0x200000, CRC(2fa1e08d) SHA1(f1f34b308202fe08e73535424b5b4e3d91295224) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "d74_02.23", 0x000000, 0x200000, CRC(aab86332) SHA1(b9133407504e9ef4fd5ae7d284cdb0c7f78f9a99) ) /* OBJ 4bpp */
	ROM_LOAD16_BYTE( "d74_03.24", 0x000001, 0x200000, CRC(3f398916) SHA1(4b6a3ee0baf5f32e24e5040f233300f1ca347fe7) )

	ROM_REGION( 0x200000, "sprites_hi", 0 )
	ROM_LOAD( "d74_04.25", 0x000000, 0x200000, CRC(2f385638) SHA1(1ba2ec7d9b1c491e1cc6d7e646e09ef2bc063f25) ) /* OBJ 2bpp */

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d74_01.37", 0x000000, 0x200000, CRC(115313e0) SHA1(51a69e7a26960b1328ccefeaec0fb26bdccc39f2) )

	/* no Delta-T samples */

	ROM_REGION( 0xc00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "d74-08",    0x000, 0x117, NO_DUMP ) // type unknown
	ROM_LOAD( "d74-09.8",  0x200, 0x117, CRC(f1bf65c3) SHA1(c42f8f115cef9e5bbc608177b26d52c92e65c653) ) // PALCE16V8Q-15PC4
	ROM_LOAD( "d74-10.40", 0x400, 0x157, CRC(c9ce583a) SHA1(372ba0f04c66e713c25eadb5029fb48a86e0bd52) ) // PALCE20V8Q-15PC4
	ROM_LOAD( "d74-11",    0x600, 0x117, NO_DUMP ) // type unknown
	ROM_LOAD( "d74-12.1",  0x800, 0x157, CRC(6965e38a) SHA1(9df15de347b7960cfdddb15dbd936df8e139f437) ) // PALCE20V8Q-15PC4
	ROM_LOAD( "d74-13.2",  0xa00, 0x157, CRC(c52df77c) SHA1(7acc4e24d2841191800f63bb96a568d8aa0d874e) ) // PALCE20V8Q-15PC4
ROM_END

ROM_START( opwolf3u )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d74_16.3",  0x000000, 0x80000, CRC(198ff1f6) SHA1(f5b51e39cd73ea56cbf53731d3c885bfcecbd696) )
	ROM_LOAD16_BYTE( "d74_20.1",  0x000001, 0x80000, CRC(960fd892) SHA1(2584a048d29a96b69428fba2b71269ea6ccf9010) )
	ROM_LOAD16_BYTE( "d74_18.18", 0x100000, 0x80000, CRC(bd5d7cdb) SHA1(29f1cd7b86bc05f873e93f088194113da87a3b86) ) // data ???
	ROM_LOAD16_BYTE( "d74_17.17", 0x100001, 0x80000, CRC(ac35a672) SHA1(8136bd076443bfaeb3d339971d88951e8b2b59b4) ) // data ???

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "d74_19.77", 0x00000, 0x10000, CRC(05d53f06) SHA1(48b0cd68ad3758f424552a4e3833c5a1c2f1825b) )

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d74_05.80", 0x000000, 0x200000, CRC(85ea64cc) SHA1(1960a934191c451df1554323d47f6fc64939b0ce) ) /* SCR */
	ROM_LOAD32_WORD( "d74_06.81", 0x000002, 0x200000, CRC(2fa1e08d) SHA1(f1f34b308202fe08e73535424b5b4e3d91295224) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "d74_02.23", 0x000000, 0x200000, CRC(aab86332) SHA1(b9133407504e9ef4fd5ae7d284cdb0c7f78f9a99) ) /* OBJ 6bpp */
	ROM_LOAD16_BYTE( "d74_03.24", 0x000001, 0x200000, CRC(3f398916) SHA1(4b6a3ee0baf5f32e24e5040f233300f1ca347fe7) )

	ROM_REGION( 0x200000, "sprites_hi", 0 )
	ROM_LOAD( "d74_04.25", 0x000000, 0x200000, CRC(2f385638) SHA1(1ba2ec7d9b1c491e1cc6d7e646e09ef2bc063f25) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d74_01.37", 0x000000, 0x200000, CRC(115313e0) SHA1(51a69e7a26960b1328ccefeaec0fb26bdccc39f2) )

	/* no Delta-T samples */

	ROM_REGION( 0xc00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "d74-08",    0x000, 0x117, NO_DUMP ) // type unknown
	ROM_LOAD( "d74-09.8",  0x200, 0x117, CRC(f1bf65c3) SHA1(c42f8f115cef9e5bbc608177b26d52c92e65c653) ) // PALCE16V8Q-15PC4
	ROM_LOAD( "d74-10.40", 0x400, 0x157, CRC(c9ce583a) SHA1(372ba0f04c66e713c25eadb5029fb48a86e0bd52) ) // PALCE20V8Q-15PC4
	ROM_LOAD( "d74-11",    0x600, 0x117, NO_DUMP ) // type unknown
	ROM_LOAD( "d74-12.1",  0x800, 0x157, CRC(6965e38a) SHA1(9df15de347b7960cfdddb15dbd936df8e139f437) ) // PALCE20V8Q-15PC4
	ROM_LOAD( "d74-13.2",  0xa00, 0x157, CRC(c52df77c) SHA1(7acc4e24d2841191800f63bb96a568d8aa0d874e) ) // PALCE20V8Q-15PC4
ROM_END

ROM_START( opwolf3j )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d74_16.3",  0x000000, 0x80000, CRC(198ff1f6) SHA1(f5b51e39cd73ea56cbf53731d3c885bfcecbd696) )
	ROM_LOAD16_BYTE( "d74_15.1",  0x000001, 0x80000, CRC(a6015c65) SHA1(4659c04fed64d3efc2df662770cb4bee285c2ec5) )
	ROM_LOAD16_BYTE( "d74_18.18", 0x100000, 0x80000, CRC(bd5d7cdb) SHA1(29f1cd7b86bc05f873e93f088194113da87a3b86) ) // data ???
	ROM_LOAD16_BYTE( "d74_17.17", 0x100001, 0x80000, CRC(ac35a672) SHA1(8136bd076443bfaeb3d339971d88951e8b2b59b4) ) // data ???

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "d74_19.77", 0x00000, 0x10000, CRC(05d53f06) SHA1(48b0cd68ad3758f424552a4e3833c5a1c2f1825b) ) /* verified correct for the Japanese set */

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d74_05.80", 0x000000, 0x200000, CRC(85ea64cc) SHA1(1960a934191c451df1554323d47f6fc64939b0ce) ) /* SCR */
	ROM_LOAD32_WORD( "d74_06.81", 0x000002, 0x200000, CRC(2fa1e08d) SHA1(f1f34b308202fe08e73535424b5b4e3d91295224) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "d74_02.23", 0x000000, 0x200000, CRC(aab86332) SHA1(b9133407504e9ef4fd5ae7d284cdb0c7f78f9a99) ) /* OBJ 6bpp */
	ROM_LOAD16_BYTE( "d74_03.24", 0x000001, 0x200000, CRC(3f398916) SHA1(4b6a3ee0baf5f32e24e5040f233300f1ca347fe7) )

	ROM_REGION( 0x200000, "sprites_hi", 0 )
	ROM_LOAD( "d74_04.25", 0x000000, 0x200000, CRC(2f385638) SHA1(1ba2ec7d9b1c491e1cc6d7e646e09ef2bc063f25) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d74_01.37", 0x000000, 0x200000, CRC(115313e0) SHA1(51a69e7a26960b1328ccefeaec0fb26bdccc39f2) )

	/* no Delta-T samples */

	ROM_REGION( 0xc00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "d74-08",    0x000, 0x117, NO_DUMP ) // type unknown
	ROM_LOAD( "d74-09.8",  0x200, 0x117, CRC(f1bf65c3) SHA1(c42f8f115cef9e5bbc608177b26d52c92e65c653) ) // PALCE16V8Q-15PC4
	ROM_LOAD( "d74-10.40", 0x400, 0x157, CRC(c9ce583a) SHA1(372ba0f04c66e713c25eadb5029fb48a86e0bd52) ) // PALCE20V8Q-15PC4
	ROM_LOAD( "d74-11",    0x600, 0x117, NO_DUMP ) // type unknown
	ROM_LOAD( "d74-12.1",  0x800, 0x157, CRC(6965e38a) SHA1(9df15de347b7960cfdddb15dbd936df8e139f437) ) // PALCE20V8Q-15PC4
	ROM_LOAD( "d74-13.2",  0xa00, 0x157, CRC(c52df77c) SHA1(7acc4e24d2841191800f63bb96a568d8aa0d874e) ) // PALCE20V8Q-15PC4
ROM_END


void slapshot_state::driver_init()
{
	/* convert from 2bits into 4bits format */
	gfx_element *gx0 = m_gfxdecode->gfx(0);
	gfx_element *gx1 = m_gfxdecode->gfx(1);

	// allocate memory for the assembled data
	m_decoded_gfx = std::make_unique<u8[]>(gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	u8 *dest = m_decoded_gfx.get();
	for (int c = 0; c < gx0->elements(); c++)
	{
		const u8 *c0base = gx0->get_data(c);
		const u8 *c1base = gx1->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const u8 *c0 = c0base;
			const u8 *c1 = c1base;

			for (int x = 0; x < gx0->width(); x++)
			{
				u8 hipix = *c1++;
				*dest++ = (*c0++ & 0xf) | ((hipix << 4) & 0x30);
			}
			c0base += gx0->rowbytes();
			c1base += gx1->rowbytes();
		}
	}

	gx0->set_raw_layout(m_decoded_gfx.get(), gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_colors(4096 / 64);
	gx0->set_granularity(64);
	m_gfxdecode->set_gfx(1, nullptr);
}

GAME( 1994, slapshot,  0,        slapshot, slapshot, slapshot_state, driver_init, ROT0, "Taito Corporation Japan",   "Slap Shot (Ver 3.0 O)",    MACHINE_SUPPORTS_SAVE ) // 7/1  12:00  Ver 3.0 O
GAME( 1994, slapshotj, slapshot, slapshot, slapshot, slapshot_state, driver_init, ROT0, "Taito Corporation",         "Slap Shot (Ver 2.2 J)",    MACHINE_SUPPORTS_SAVE ) // 6/8  12:00  Ver 2.2 J
GAME( 1994, opwolf3,   0,        opwolf3,  opwolf3,  slapshot_state, driver_init, ROT0, "Taito Corporation Japan",   "Operation Wolf 3 (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, opwolf3u,  opwolf3,  opwolf3,  opwolf3,  slapshot_state, driver_init, ROT0, "Taito America Corporation", "Operation Wolf 3 (US)",    MACHINE_SUPPORTS_SAVE )
GAME( 1994, opwolf3j,  opwolf3,  opwolf3,  opwolf3,  slapshot_state, driver_init, ROT0, "Taito Corporation",         "Operation Wolf 3 (Japan)", MACHINE_SUPPORTS_SAVE )
