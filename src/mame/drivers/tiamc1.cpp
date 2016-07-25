// license:BSD-3-Clause
// copyright-holders:Eugene Sandulenko
/*****************************************************************************

  TIA-MC1 driver

  driver by Eugene Sandulenko
  special thanks to Shiru for his standalone emulator and documentation

  Games supported:
      * Billiard
      * Konek-Gorbunok (Little Humpbacked Horse)
      * Snezhnaja Koroleva (Snow Queen)
      * S.O.S.
	  * Gorodki
	  * Kot-Rybolov

  Other games known to exist on this hardware (interchangeable by the ROM swap):
      * Avtogonki
      * Istrebitel'
      * Kotigoroshko
      * Ostrov Drakona
      * Ostrov Sokrovisch
      * Perehvatchik
      * Zvezdnyj Rycar'

 ***************************************************************

  During bootup hold F2 to enter test mode.
  Also use F2 to switch the screens during the gameplay (feature of the original
    machine)

 ***************************************************************

  This is one of the last USSR-made arcades. Several games are known to exist on
  this hardware. It was created by the state company Terminal (Vinnitsa, Ukraine),
  which was later turned into EXTREMA-Ukraine company. These days it manufactures
  various gambling machines (http://www.extrema-ua.com).

 ***************************************************************

  TIA-MC1 arcade internals

  This arcade machine contains four PCBs:

    BEIA-100
      Main CPU board. Also contains address PROM, color DAC, input ports and sound

    BEIA-101
      Video board 1. Background generator, video tiles RAM and video sync schematics

    BEIA-102
      Video board 2. Sprite generator, video buffer RAM (hardware triple buffering)

    BEIA-103
      ROM banks, RAM. Contains ROMs and multiplexors


  BEIA-103 PCB Layout

    |----------------------------------|
    |                                  |
  |--| g1.d17   RU8A                   |
  |  |                                 |
  |  |                                 |
  |  | g2.d17   RU8A   IR13   a2.b07   |
  |  |                                 |
  |  |                                 |
  |  | g3.d17   RU8A   IR13   a3.g07  |--|
  |  |                                |  |
  |--|                                |  |
    |  g4.d17   RU8A    kp11          |  |
    |                                 |  |
    |                                 |  |
    |  g5.d17          IR13   a5.l07  |  |
  |--|                                |  |
  |  |           id7                  |  |
  |  | g6.d17          IR13   a6.r07  |--|
  |  |           id7                   |
  |  |                                 |
  |  | g7.d17                          |
  |  |           ll1                   |
  |  |                                 |
  |--|  ap6      le1                   |
    |                                  |
    |----------------------------------|

  Notes:

   g1.d17 \
   g2.d17 |
   g3.d17 |
   g4.d17 |
   g5.d17 |
   g6.d17 +- EPROM K573RF4A (2764 analog)
   g7.d17 |
   a2.b07 |
   a3.g07 |
   a5.l07 |
   a6.r07 /

   RU8A - KR537RU8A SRAM 2k x8 (TC5516 analog)
   IR13 - K155IR13 register (74198 analog)
   kp11 - K555KP11 selector/multiplexor (74LS257 analog)
   id7  - K555ID7 (74LS138 analog)
   ap6  - K555AP6 (74LS245 analog)
   ll1  - K555LL1 (74LS32 analog)
   le1  - K555LE1 (74LS02 analog)

 ***************************************************************

   TODO:
     - Use machine/pit8253.c in sound
     - Check sprites priorities on the real hardware
     - Check vertical background scrolling on the real hardware
     - What charset control is used for?
	 - Kot Rybolov viewable area size likely controlled by pit8253 timers 0 and 1, used in test mode only

*/

#define MASTER_CLOCK    (15750000)
#define CPU_CLOCK       (MASTER_CLOCK / 9)
#define SND_CLOCK       (MASTER_CLOCK / 9)
#define PIXEL_CLOCK     (MASTER_CLOCK / 3)

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "includes/tiamc1.h"

void tiamc1_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	tiamc1_bankswitch_w(space, 0, 0);
}

WRITE8_MEMBER(tiamc1_state::tiamc1_control_w)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
}

WRITE_LINE_MEMBER(tiamc1_state::pit8253_2_w)
{
	m_speaker->level_w(state);
}


static ADDRESS_MAP_START( tiamc1_map, AS_PROGRAM, 8, tiamc1_state )
	AM_RANGE(0xb000, 0xb7ff) AM_WRITE(tiamc1_videoram_w)
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kotrybolov_map, AS_PROGRAM, 8, tiamc1_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(kot_videoram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tiamc1_io_map, AS_IO, 8, tiamc1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x4f) AM_WRITE(tiamc1_sprite_y_w) /* sprites Y */
	AM_RANGE(0x50, 0x5f) AM_WRITE(tiamc1_sprite_x_w) /* sprites X */
	AM_RANGE(0x60, 0x6f) AM_WRITE(tiamc1_sprite_n_w) /* sprites # */
	AM_RANGE(0x70, 0x7f) AM_WRITE(tiamc1_sprite_a_w) /* sprites attributes */
	AM_RANGE(0xa0, 0xaf) AM_WRITE(tiamc1_palette_w)  /* color ram */
	AM_RANGE(0xbc, 0xbc) AM_WRITE(tiamc1_bg_vshift_w)/* background V scroll */
	AM_RANGE(0xbd, 0xbd) AM_WRITE(tiamc1_bg_hshift_w)/* background H scroll */
	AM_RANGE(0xbe, 0xbe) AM_WRITE(tiamc1_bankswitch_w) /* VRAM selector */
	AM_RANGE(0xbf, 0xbf) AM_WRITENOP                 /* charset control */
	AM_RANGE(0xc0, 0xc3) AM_DEVWRITE("2x8253", tiamc1_sound_device, tiamc1_timer0_w)   /* timer 0 */
	AM_RANGE(0xd0, 0xd3) AM_DEVREADWRITE("kr580vv55a", i8255_device, read, write)    /* input ports + coin counters & lockout */
	AM_RANGE(0xd4, 0xd7) AM_DEVWRITE("2x8253", tiamc1_sound_device, tiamc1_timer1_w)   /* timer 1 */
	AM_RANGE(0xda, 0xda) AM_DEVWRITE("2x8253", tiamc1_sound_device, tiamc1_timer1_gate_w) /* timer 1 gate control */
ADDRESS_MAP_END

static ADDRESS_MAP_START( kotrybolov_io_map, AS_IO, 8, tiamc1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x0f) AM_WRITE(tiamc1_sprite_y_w)	// sprites Y
	AM_RANGE(0x10, 0x1f) AM_WRITE(tiamc1_sprite_x_w)	// sprites X
	AM_RANGE(0x20, 0x2f) AM_WRITE(tiamc1_sprite_n_w)	// sprites #
	AM_RANGE(0x30, 0x3f) AM_WRITE(tiamc1_sprite_a_w)	// sprites attributes
	AM_RANGE(0xe0, 0xef) AM_WRITE(tiamc1_palette_w)		// color ram
	AM_RANGE(0xf0, 0xf0) AM_WRITE(tiamc1_bg_vshift_w)	// background V scroll
	AM_RANGE(0xf4, 0xf4) AM_WRITE(tiamc1_bg_hshift_w)	// background H scroll
	AM_RANGE(0xf8, 0xf8) AM_WRITE(kot_bankswitch_w)		// character rom offset
	AM_RANGE(0xc0, 0xc3) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0xd0, 0xd3) AM_DEVREADWRITE("kr580vv55a", i8255_device, read, write)    /* input ports + coin counters & lockout */
ADDRESS_MAP_END


static INPUT_PORTS_START( tiamc1 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 0 JOYSTICK_RIGHT */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 2 JOYSTICK_RIGHT */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 3 JOYSTICK_RIGHT */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 0 JOYSTICK_LEFT */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 2 JOYSTICK_LEFT */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 3 JOYSTICK_LEFT */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 0 JOYSTICK_UP */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 2 JOYSTICK_UP */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 3 JOYSTICK_UP */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 0 JOYSTICK_DOWN */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )    /* Player 2 JOYSTICK_DOWN */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )    /* OUT:coin lockout */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* OUT:game counter */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* RAZR ??? */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) // Kick
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // Jump
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( gorodki )
	PORT_START("IN0")
	PORT_BIT( 0xff, 152, IPT_AD_STICK_X) PORT_MINMAX(96, 208) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )    /* OUT:coin lockout */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* OUT:game counter */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* RAZR ??? */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) // right button
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // left button
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( kot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 0 JOYSTICK_RIGHT */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 2 JOYSTICK_RIGHT */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 3 JOYSTICK_RIGHT */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 0 JOYSTICK_LEFT */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 2 JOYSTICK_LEFT */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 3 JOYSTICK_LEFT */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 0 JOYSTICK_UP */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 2 JOYSTICK_UP */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 3 JOYSTICK_UP */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 0 JOYSTICK_DOWN */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )    /* Player 2 JOYSTICK_DOWN */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )    /* OUT:coin lockout */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* OUT:game counter */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* RAZR ??? */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // Punch / Right
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // Kick / Left 
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


static const gfx_layout sprites16x16_layout =
{
	16,16,
	256,
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 256*16*8+0, 256*16*8+1, 256*16*8+2, 256*16*8+3, 256*16*8+4, 256*16*8+5, 256*16*8+6, 256*16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static const gfx_layout char_layout =
{
	8,8,
	256,
	4,
	{ 256*8*8*3, 256*8*8*2, 256*8*8*1, 256*8*8*0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout char_rom_layout =
{
	8,8,
	256,
	4,
	{ 1024*8*8*3, 1024*8*8*2, 1024*8*8*1, 1024*8*8*0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( tiamc1 )
	GFXDECODE_ENTRY( nullptr, 0x0000, char_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, sprites16x16_layout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( kot )
	GFXDECODE_ENTRY( nullptr, 0x0000, char_rom_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, sprites16x16_layout, 0, 16 )
GFXDECODE_END


static MACHINE_CONFIG_START( tiamc1, tiamc1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(tiamc1_map)
	MCFG_CPU_IO_MAP(tiamc1_io_map)

	MCFG_DEVICE_ADD("kr580vv55a", I8255A, 0)  /* soviet clone of i8255 */
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(tiamc1_state, tiamc1_control_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 336, 0, 256, 312, 0, 256)		// pixel clock and htotal comes from docs/schematics, the rest is guess (determined by undumped PROM)
	MCFG_SCREEN_UPDATE_DRIVER(tiamc1_state, screen_update_tiamc1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tiamc1)
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(tiamc1_state, tiamc1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("2x8253", TIAMC1, SND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(kot, tiamc1)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kotrybolov_map)
	MCFG_CPU_IO_MAP(kotrybolov_io_map)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_VIDEO_START_OVERRIDE(tiamc1_state, kot)
	MCFG_SCREEN_UPDATE_DRIVER(tiamc1_state, screen_update_kot)

	MCFG_GFXDECODE_MODIFY("gfxdecode", kot)

	MCFG_DEVICE_REMOVE("2x8253")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(PIXEL_CLOCK / 4)
	MCFG_PIT8253_CLK2(SND_CLOCK)				// guess
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(tiamc1_state, pit8253_2_w))
MACHINE_CONFIG_END


ROM_START( konek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g1.d17", 0x00000, 0x2000, CRC(f41d82c9) SHA1(63ac1be2ad58af0e5ef2d33e5c8d790769d80af9) )
	ROM_LOAD( "g2.d17", 0x02000, 0x2000, CRC(b44e7491) SHA1(ff4cb1d76a36f504d670a207ee25556c5faad435) )
	ROM_LOAD( "g3.d17", 0x04000, 0x2000, CRC(91301282) SHA1(cb448a1bb7a9c1768f870a8c062e37807431c9c7) )
	ROM_LOAD( "g4.d17", 0x06000, 0x2000, CRC(3ff0c20b) SHA1(3d999c05b3986149e569630779ed5581fc202842) )
	ROM_LOAD( "g5.d17", 0x08000, 0x2000, CRC(e3196d30) SHA1(a03d9f75926be9fcf5ee05df8b00fbf87361ea5b) )
	ROM_FILL( 0xa000, 0x2000, 0x00 ) /* g6.d17 is unpopulated */
	ROM_LOAD( "g7.d17", 0x0c000, 0x2000, CRC(fe4e9fdd) SHA1(2033585a6c53455d1dafee85cbb807d424ed231d) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a2.b07", 0x00000, 0x2000, CRC(9eed06ee) SHA1(1b64a3f8fe3df4b4870315dbdf69bf60b1c272d0) )
	ROM_LOAD( "a3.g07", 0x02000, 0x2000, CRC(eeff9b77) SHA1(5dc66292a59f24277a8c2f38158a2e1d58f81338) )
	ROM_LOAD( "a5.l07", 0x04000, 0x2000, CRC(fff9e089) SHA1(f0d64dceaf72da785d55316bf8a7433faa09fabb) )
	ROM_LOAD( "a6.r07", 0x06000, 0x2000, CRC(092e8ee2) SHA1(6c4842e992c592b9f0663e039668f61a7b56700f) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "prom100.e10", 0x0000, 0x100, NO_DUMP ) /* i/o ports map 256x8 */
	ROM_LOAD( "prom101.a01", 0x0100, 0x100, NO_DUMP ) /* video sync 256x8 */
	ROM_LOAD( "prom102.b03", 0x0200, 0x080, NO_DUMP ) /* sprites rom index 256x4 */
	ROM_LOAD( "prom102.b06", 0x0280, 0x080, NO_DUMP ) /* buffer optimization logic 256x4 */
	ROM_LOAD( "prom102.b05", 0x0300, 0x100, NO_DUMP ) /* sprites rom index 256x8 */

ROM_END

ROM_START( sosterm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "04.1g", 0x00000, 0x2000, CRC(d588081e) SHA1(5dd9f889e932ee356f8e511b22b424eaeb502ef9) )
	ROM_LOAD( "05.2g", 0x02000, 0x2000, CRC(b44e7491) SHA1(ff4cb1d76a36f504d670a207ee25556c5faad435) )
	ROM_LOAD( "06.3g", 0x04000, 0x2000, CRC(34dacde6) SHA1(6c91e4dc1d3c85768a94fb4c7d38f29c23664740) )
	ROM_LOAD( "07.4g", 0x06000, 0x2000, CRC(9f6f8cdd) SHA1(3fa3928935d98906fdf07ed372764456d7a9729a) )
	ROM_LOAD( "08.5g", 0x08000, 0x2000, CRC(25e70da4) SHA1(ec77b0b79c0477c0939022d7f2a24ae48e4530bf) )
	ROM_FILL( 0xa000, 0x2000, 0x00 ) /* 09.6g is unpopulated */
	ROM_LOAD( "10.7g", 0x0c000, 0x2000, CRC(22bc9997) SHA1(fd638529e29d9fd32dd22534cb748841dde9a2c3) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "00.2a", 0x00000, 0x2000, CRC(a1c7f07a) SHA1(2ae702258be48ba70c126bfe94fbeec3353fc75a) )
	ROM_LOAD( "01.3a", 0x02000, 0x2000, CRC(788b4036) SHA1(a0020ae1720cc2e5a6db0f8fe9350de43246f552) )
	ROM_LOAD( "02.5a", 0x04000, 0x2000, CRC(9506cf9b) SHA1(3e54593d4452b956509877d9b6b26aa3e3a90beb) )
	ROM_LOAD( "03.6a", 0x06000, 0x2000, CRC(5a0c14e1) SHA1(3eebe2c3ce114b87723fa6571623ee065a0b5646) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "prom100.e10", 0x0000, 0x100, NO_DUMP ) /* i/o ports map 256x8 */
	ROM_LOAD( "prom101.a01", 0x0100, 0x100, NO_DUMP ) /* video sync 256x8 */
	ROM_LOAD( "prom102.b03", 0x0200, 0x080, NO_DUMP ) /* sprites rom index 256x4 */
	ROM_LOAD( "prom102.b06", 0x0280, 0x080, NO_DUMP ) /* buffer optimization logic 256x4 */
	ROM_LOAD( "prom102.b05", 0x0300, 0x100, NO_DUMP ) /* sprites rom index 256x8 */
ROM_END

ROM_START( koroleva )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "04.1g", 0x00000, 0x2000, CRC(c3701225) SHA1(ac059116521b06cb5347741d3ee2778c2e68a74e) )
	ROM_LOAD( "05.2g", 0x02000, 0x2000, CRC(1b3742ce) SHA1(908ad9eb0e79baac53eed195355d0d1bdf6b5a25) )
	ROM_LOAD( "06.3g", 0x04000, 0x2000, CRC(48074786) SHA1(145749053cd00c8547024c9afe3ab0ff7d8f5ff9) )
	ROM_LOAD( "07.4g", 0x06000, 0x2000, CRC(41a4adb5) SHA1(cdbdf6884307dd0f1fc991e6e1bc4c4fdc351ab1) )
	ROM_LOAD( "08.5g", 0x08000, 0x2000, CRC(8f379d95) SHA1(0ea70bc14c52b1f4b38b0d14e4249252a2577f2a) )
	ROM_FILL( 0xa000, 0x2000, 0x00 ) /* 09.6g is unpopulated */
	ROM_LOAD( "10.7g", 0x0c000, 0x2000, CRC(397f41f8) SHA1(2d07462afad39dda067114ce8d47e64d6a854283) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "00.2a", 0x00000, 0x2000, CRC(6f39f8be) SHA1(2b20cdab7064851c552d92d5bc9084df854eafd1) )
	ROM_LOAD( "01.3a", 0x02000, 0x2000, CRC(7bdfdd19) SHA1(8b971689050f9d608225226eb5cadbb4050c7d1f) )
	ROM_LOAD( "02.5a", 0x04000, 0x2000, CRC(97770b0f) SHA1(cf4605e31f8c57a76bfda6a7ea329058da8b8c9c) )
	ROM_LOAD( "03.6a", 0x06000, 0x2000, CRC(9b0a686a) SHA1(f02910db9f862ec017bb3834c58e96e780fb6322) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "prom100.e10", 0x0000, 0x100, NO_DUMP ) /* i/o ports map 256x8 */
	ROM_LOAD( "prom101.a01", 0x0100, 0x100, NO_DUMP ) /* video sync 256x8 */
	ROM_LOAD( "prom102.b03", 0x0200, 0x080, NO_DUMP ) /* sprites rom index 256x4 */
	ROM_LOAD( "prom102.b06", 0x0280, 0x080, NO_DUMP ) /* buffer optimization logic 256x4 */
	ROM_LOAD( "prom102.b05", 0x0300, 0x100, NO_DUMP ) /* sprites rom index 256x8 */
ROM_END

// this game ROM board have only one 2KB main CPU RAM IC populated
ROM_START( bilyard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "04.1g", 0x00000, 0x2000, CRC(a44f913d) SHA1(f01a0e931fb0f719bc7b3f1ca7802dd747c8a56f) )
	ROM_LOAD( "05.2g", 0x02000, 0x2000, CRC(6e41219f) SHA1(b09a16c9bd48b503ec0f2c636f021199d7ac7924) )
	ROM_FILL( 0x04000, 0x2000, 0x00 ) /* 06.3g is unpopulated */
	ROM_FILL( 0x06000, 0x2000, 0x00 ) /* 07.4g is unpopulated */
	ROM_FILL( 0x08000, 0x2000, 0x00 ) /* 08.5g is unpopulated */
	ROM_FILL( 0x0a000, 0x2000, 0x00 ) /* 09.6g is unpopulated */
	ROM_LOAD( "10.7g", 0x0c000, 0x2000, CRC(173adb85) SHA1(53f27b45e61365907e8996c283ae70ca5b498129) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "00.2a", 0x00000, 0x2000, CRC(6f72e043) SHA1(a60f66326ff0a55e2624231efcbceff700d9ceee) )
	ROM_LOAD( "01.3a", 0x02000, 0x2000, CRC(daddbbb5) SHA1(1460aebcbb57180e05930845703ff6325d85702a) )
	ROM_LOAD( "02.5a", 0x04000, 0x2000, CRC(3d744d33) SHA1(f1375098e81986715d0497b09df0c6622bd75b9a) )
	ROM_LOAD( "03.6a", 0x06000, 0x2000, CRC(8bfc0b15) SHA1(221efdce516274d3b1d9009d11dc9ed6cd67ef12) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "prom100.e10", 0x0000, 0x100, NO_DUMP ) /* i/o ports map 256x8 */
	ROM_LOAD( "prom101.a01", 0x0100, 0x100, NO_DUMP ) /* video sync 256x8 */
	ROM_LOAD( "prom102.b03", 0x0200, 0x080, NO_DUMP ) /* sprites rom index 256x4 */
	ROM_LOAD( "prom102.b06", 0x0280, 0x080, NO_DUMP ) /* buffer optimization logic 256x4 */
	ROM_LOAD( "prom102.b05", 0x0300, 0x100, NO_DUMP ) /* sprites rom index 256x8 */
ROM_END

// this game ROM board have only one 2KB main CPU RAM IC populated
ROM_START( gorodki )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "70.1g", 0x00000, 0x2000, CRC(bd3eb624) SHA1(acfc7e7186daf399f858a0d0cf462b0eaabb3f9e) )
	ROM_LOAD( "71.2g", 0x02000, 0x2000, CRC(5a9ebd8d) SHA1(4a6682a4bd2eb3c852c1383b0564fe491e7af30a) )
	ROM_LOAD( "72.3g", 0x04000, 0x2000, CRC(edcc5c13) SHA1(6bebb4c28758a3b3c45318201eb13e5d81db7521) )
	ROM_LOAD( "73.4g", 0x06000, 0x2000, CRC(ae69b9f3) SHA1(08882fbf917ba17e95b27ba21db666e7832c1894) )
	ROM_FILL( 0x08000, 0x2000, 0x00 ) /* 08.5g is unpopulated */
	ROM_FILL( 0x0a000, 0x2000, 0x00 ) /* 09.6g is unpopulated */
	ROM_FILL( 0x0c000, 0x2000, 0x00 ) /* 10.7g is unpopulated */

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "66.2a", 0x00000, 0x2000, CRC(b3dd4dec) SHA1(2e399fca4ff0b98724f26a27b1ea8450d650cfb4) )
	ROM_LOAD( "67.3a", 0x02000, 0x2000, CRC(c94f5579) SHA1(757f063c857c81478925b1ae169de3c81b3533d4) )
	ROM_LOAD( "68.5a", 0x04000, 0x2000, CRC(0d64708d) SHA1(6a84b4293f0e983424ef361ab3ebf62ab5f8b21c) )
	ROM_LOAD( "69.6a", 0x06000, 0x2000, CRC(57c8ae81) SHA1(c73bbfaa53195a19599dd2bbc3948c819597b035) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "prom100.e10", 0x0000, 0x100, NO_DUMP ) /* i/o ports map 256x8 */
	ROM_LOAD( "prom101.a01", 0x0100, 0x100, NO_DUMP ) /* video sync 256x8 */
	ROM_LOAD( "prom102.b03", 0x0200, 0x080, NO_DUMP ) /* sprites rom index 256x4 */
	ROM_LOAD( "prom102.b06", 0x0280, 0x080, NO_DUMP ) /* buffer optimization logic 256x4 */
	ROM_LOAD( "prom102.b05", 0x0300, 0x100, NO_DUMP ) /* sprites rom index 256x8 */
ROM_END

/*
  later (cost reduced ?) version of TIA-MC1 hardware, uses single PCB main board design,
  ROM-based tile generator, sinle bank tile map RAM, single i8253

  Specs:
  1x KR580VM80A (Russian: KP580BM80A. Soviet clone of i8080).
  1x KR580VV55A (Russian: KP580BB55A. Soviet clone of i8255).
  1x KR580VI53 (Soviet clone of i8253)
  1x KR580VK28 (Russian: KP580BK28. Soviet clone of i8228: bus controllers/drivers).

  1x K174YH7 (soviet clone of TBA810, 7W audio amplifier).

  3x KR573RU8 (2Kx8 SRAM).
  2x KR541RU2 (1Kx4 RAM).
  9x KR531RU8 (16x4 RAM).

  There are some issues with the KR580VV55A.
  Sometimes the code tries to write through a port that was
  previously set as input. Not sure if it's a leftover, a bug,
  or just some differences against the i8255. Needs more research.

  KR580VI53/i8253 timers usage:
  0 - used to blank part of display in horizontal direction, apparently it's CLK0 is PIXEL_CLOCK/4, GATE0 is HBlank, OUT0 - blank video
  1 - used to blank part of display in vertical direction, apparently it's CLK1 is HBlank or OUT0, GATE1 is VBlank, OUT1 - blank video
  2 - sound generation

*/
ROM_START( kot )
	ROM_REGION( 0xc000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "854.6", 0x00000, 0x2000, CRC(44e5e8fc) SHA1(dafbace689f3834d5c6e952a2f6188fb190845e4) )
	ROM_LOAD( "855.7", 0x02000, 0x2000, CRC(0bb2e4b2) SHA1(7bbb45b18e3b444e3b6006a4670453ec0792e5d3) )
	ROM_LOAD( "856.8", 0x04000, 0x2000, CRC(9180c98f) SHA1(4085180b9e9772e965c487e7b02d88fcae973e87) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "850.5", 0x00000, 0x2000, CRC(5dc3a102) SHA1(e97d219f7004291438b991435b7fe5d5be01d468) )	// sprite gfx
	ROM_LOAD( "851.6", 0x02000, 0x2000, CRC(7db239a0) SHA1(af5772afff9009f63e2ab95c1cb00e047f3ed7e4) )
	ROM_LOAD( "852.7", 0x04000, 0x2000, CRC(c7700f88) SHA1(1a20cc60b083259070e4f1687b09a31fc763d47e) )
	ROM_LOAD( "853.8", 0x06000, 0x2000, CRC(b94bf1af) SHA1(da403c51fd78f99b82304c67f2197078f4ea0bf5) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "846.1", 0x00000, 0x2000, CRC(42447f4a) SHA1(bd35f2f5e468f9191680bf2c1800e09bb9ae1691) )	// tile gfx
	ROM_LOAD( "847.2", 0x02000, 0x2000, CRC(99ada5e8) SHA1(9425a515105ec9e9989aae736645b270e39420be) )
	ROM_LOAD( "848.3", 0x04000, 0x2000, CRC(a124cff4) SHA1(d1d8e6f725a6f30058d52cdbe80b598149cd6052) )
	ROM_LOAD( "849.4", 0x06000, 0x2000, CRC(5d27fda6) SHA1(f1afb39c7422caaa5eff53388f1b7241dd7c1cd7) )
ROM_END


GAME( 1988, konek,    0, tiamc1, tiamc1,  driver_device, 0, ROT0, "Terminal", "Konek-Gorbunok",     MACHINE_SUPPORTS_SAVE )
GAME( 1988, sosterm,  0, tiamc1, tiamc1,  driver_device, 0, ROT0, "Terminal", "S.O.S.",             MACHINE_SUPPORTS_SAVE )
GAME( 1988, koroleva, 0, tiamc1, tiamc1,  driver_device, 0, ROT0, "Terminal", "Snezhnaja Koroleva", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bilyard,  0, tiamc1, tiamc1,  driver_device, 0, ROT0, "Terminal", "Billiard",           MACHINE_SUPPORTS_SAVE )
GAME( 1988, gorodki,  0, tiamc1, gorodki, driver_device, 0, ROT0, "Terminal", "Gorodki",            MACHINE_SUPPORTS_SAVE )
GAME( 1988, kot,      0, kot,    kot,     driver_device, 0, ROT0, "Terminal", "Kot-Rybolov",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
