// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Mug Smashers (c)199? Electronic Devices (Italy) / 3D Games (England)
    driver by David Haywood - Dip Switches and Inputs by stephh

 a side scrolling beat-em-up, borrows ideas from Combatribes, including
 the music (apparently) and sound hardware!

 there is also a Spanish company logo in the graphic roms

 a 1990 copyright can be found in the sound program so its 1990 at the
 earliest

*/

/* working notes:

68k interrupts
lev 1 : 0x64 : 0000 0100 - just rte
lev 2 : 0x68 : 0000 0100 - just rte
lev 3 : 0x6c : 0000 0100 - just rte
lev 4 : 0x70 : 0000 0100 - just rte
lev 5 : 0x74 : 0000 0100 - just rte
lev 6 : 0x78 : 0000 0102 - vblank?
lev 7 : 0x7c : 0000 0100 - just rte

to fix:

attribute bits on bg tiles - what are they for?

is scrolling / bg placement 100% correct?


dsw note:

the DSW ports are a bit odd, from reading the test mode it appears the
board has 2 physical dipswitches, however these are mapped over multiple
real addresses, obviously this gives us two ways of dealing with them,
USE_FAKE_INPUT_PORTS is used at compile time to give an option of which
behavior we use .

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/mugsmash.h"

WRITE16_MEMBER(mugsmash_state::mugsmash_reg2_w)
{
	m_regs2[offset] = data;
	//popmessage ("Regs2 %04x, %04x, %04x, %04x", m_regs2[0], m_regs2[1], m_regs2[2], m_regs2[3]);

	switch (offset)
	{
	case 1:
		soundlatch_byte_w(space, 1, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
		break;

	default:
		break;
	}

}


/* Ports mapping :

    $180000.w : 0123456789ABCDEF
                x---------------    right     (player 1)
                -x--------------    left      (player 1)
                --x-------------    up        (player 1)
                ---x------------    down      (player 1)
                ----x-----------    button 1  (player 1)
                -----x----------    button 2  (player 1)
                ------x---------    button 3  (player 1)
                -------x--------    start     (player 1)
                --------x-------    coin 1
                ---------x------    coin 2
                ----------x-----    unused
                -----------x----    unused
                ------------x---    SW1-7     ("Color Test")     *
                -------------x--    SW1-8     ("Draw SF.")       *
                --------------x-    unused
                ---------------x    unused

    $180002.w : 0123456789ABCDEF
                x---------------    right     (player 2)
                -x--------------    left      (player 2)
                --x-------------    up        (player 2)
                ---x------------    down      (player 2)
                ----x-----------    button 1  (player 2)
                -----x----------    button 2  (player 2)
                ------x---------    button 3  (player 2)
                -------x--------    start     (player 2)
                --------x-------    SW1-1     ("Test Mode")
                ---------x------    SW1-2     ("Coin/Credit")
                ----------x-----    SW1-3     ("Coin/Credit")
                -----------x----    SW1-4     ("Coin/Credit")
                ------------x---    SW1-5     ("Continue")
                -------------x--    SW1-6     ("Sound Test")     *
                --------------x-    unused
                ---------------x    unused

    $180004.w : 0123456789ABCDEF
                x---------------    unused
                -x--------------    unused
                --x-------------    unused
                ---x------------    unused
                ----x-----------    unused
                -----x----------    unused
                ------x---------    unused
                -------x--------    unused
                --------x-------    SW2-1     ("Sound Demo")
                ---------x------    SW2-2     ("Lives Num")
                ----------x-----    SW2-3     ("Lives Num")
                -----------x----    SW2-4     ("Not Used")
                ------------x---    SW2-5     ("Diff Level")
                -------------x--    SW2-6     ("Diff Level")
                --------------x-    unused
                ---------------x    unused

    $180006.w : 0123456789ABCDEF
                x---------------    unused
                -x--------------    unused
                --x-------------    unused
                ---x------------    unused
                ----x-----------    unused
                -----x----------    unused
                ------x---------    unused
                -------x--------    unused
                --------x-------    SW2-7     ("Draw Obj")       *
                ---------x------    SW2-8     ("Screen Pause")
                ----------x-----    unused
                -----------x----    unused
                ------------x---    unused
                -------------x--    unused
                --------------x-    unused
                ---------------x    unused

    * only available when you are in "test mode"

*/

#define USE_FAKE_INPUT_PORTS    0

#if USE_FAKE_INPUT_PORTS
READ16_MEMBER(mugsmash_state::mugsmash_input_ports_r)
{
	UINT16 data = 0xffff;

	switch (offset)
	{
		case 0 :
			data = (ioport("P1")->read() & 0xff) | ((ioport("DSW1")->read() & 0xc0) << 6) | ((ioport("IN0")->read() & 0x03) << 8);
			break;
		case 1 :
			data = (ioport("P2")->read() & 0xff) | ((ioport("DSW1")->read() & 0x3f) << 8);
			break;
		case 2 :
			data = ((ioport("DSW2")->read() & 0x3f) << 8);
			break;
		case 3 :
			data = ((ioport("DSW2")->read() & 0xc0) << 2);
			break;
	}

	return (data);
}
#endif

static ADDRESS_MAP_START( mugsmash_map, AS_PROGRAM, 16, mugsmash_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080fff) AM_RAM_WRITE(mugsmash_videoram1_w) AM_SHARE("videoram1")
	AM_RANGE(0x082000, 0x082fff) AM_RAM_WRITE(mugsmash_videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0x0c0000, 0x0c0007) AM_WRITE(mugsmash_reg_w) AM_SHARE("regs1") /* video registers*/
	AM_RANGE(0x100000, 0x1005ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x140000, 0x140007) AM_WRITE(mugsmash_reg2_w) AM_SHARE("regs2") /* sound + ? */
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM /* main ram? */
	AM_RANGE(0x1c4000, 0x1cffff) AM_RAM
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("spriteram") /* sprite ram */
#if USE_FAKE_INPUT_PORTS
	AM_RANGE(0x180000, 0x180007) AM_READ(mugsmash_input_ports_r)
#else
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("IN0")
	AM_RANGE(0x180002, 0x180003) AM_READ_PORT("IN1")
	AM_RANGE(0x180004, 0x180005) AM_READ_PORT("IN2")
	AM_RANGE(0x180006, 0x180007) AM_READ_PORT("IN3")
#endif
ADDRESS_MAP_END

static ADDRESS_MAP_START( mugsmash_sound_map, AS_PROGRAM, 8, mugsmash_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ymsnd", ym2151_device,read,write)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


#define MUGSMASH_PLAYER_INPUT( player, start ) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, start )


#if USE_FAKE_INPUT_PORTS
static INPUT_PORTS_START( mugsmash )
	PORT_START("P1")    /* Fake IN0 (player 1 inputs) */
	MUGSMASH_PLAYER_INPUT( 1, IPT_START1 )

	PORT_START("P2")    /* Fake IN1 (player 2 inputs) */
	MUGSMASH_PLAYER_INPUT( 2, IPT_START2 )

	PORT_START("IN0")   /* Fake IN2 (system inputs) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")  /* Fake IN3 (SW1) */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DSW1:1")                  // SW1-1
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:2,3,4")           // SW1-2 to SW1-4
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("DSW1:5")    // SW1-5
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Sound Test" ) PORT_DIPLOCATION("DSW1:6")             // SW1-6 (in "test mode" only)
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Color Test" ) PORT_DIPLOCATION("DSW1:7")             // SW1-7 (in "test mode" only)
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Draw SF." ) PORT_DIPLOCATION("DSW1:8")                   // SW1-8 (in "test mode" only)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* Fake IN4 (SW2) */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW2:1")       // SW2-1
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW2:2,3")           // SW2-2 and SW2-3
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x06, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSW2:4" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW2:5,6")      // SW2-5 and SW2-6
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Draw Objects" ) PORT_DIPLOCATION("DSW2:7")               // SW2-7 (in "test mode" only)
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" ) PORT_DIPLOCATION("DSW2:8")                 // SW2-8 (= "Screen Pause" in "test mode")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END
#else
static INPUT_PORTS_START( mugsmash )
	PORT_START("IN0")   /* IN0 - $180000.w */
	MUGSMASH_PLAYER_INPUT( 1, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x1000, 0x1000, "Color Test" )            // SW1-7 (in "test mode" only)
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Draw SF." )              // SW1-8 (in "test mode" only)
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* IN1 - $180002.w */
	MUGSMASH_PLAYER_INPUT( 2, IPT_START2 )
	PORT_SERVICE( 0x0100, IP_ACTIVE_LOW )                   // SW1-1
	PORT_DIPNAME( 0x0e00, 0x0000, DEF_STR( Coinage ) )      // SW1-2 to SW1-4
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Allow_Continue ) )   // SW1-5
	PORT_DIPSETTING(      0x1000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Sound Test" )            // SW1-6 (in "test mode" only)
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* IN2 - $180004.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  // SW2-1
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0200, DEF_STR( Lives ) )        // SW2-2 and SW2-3
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0600, "4" )
	PORT_DIPNAME( 0x0800, 0x0800, "Unused SW 2-4" )         // SW2-4
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x1000, DEF_STR( Difficulty ) )   // SW2-5 and SW2-6
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Very_Hard ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* IN3 - $180006.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "Draw Objects" )          // SW2-7 (in "test mode" only)
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Freeze" )                // SW2-8 (= "Screen Pause" in "test mode")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END
#endif

static const gfx_layout mugsmash_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 16,20,24,28,0,4,8,12,48,52,56,60,32,36,40,44 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64},
	16*64
};

static const gfx_layout mugsmash2_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{   0x080000*3*8,   0x080000*2*8,   0x080000*1*8,   0x080000*0*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7  },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( mugsmash )
	GFXDECODE_ENTRY( "gfx1", 0, mugsmash_layout,   0x00, 16  ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, mugsmash2_layout,  0x100, 256  ) /* bg tiles */
GFXDECODE_END

void mugsmash_state::machine_start()
{
}

static MACHINE_CONFIG_START( mugsmash, mugsmash_state )

	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(mugsmash_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mugsmash_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)  /* Guess */
	MCFG_CPU_PROGRAM_MAP(mugsmash_sound_map)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(mugsmash_state, screen_update_mugsmash)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mugsmash)

	MCFG_PALETTE_ADD("palette", 0x300)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)   /* music */
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_OKIM6295_ADD("oki", 1122000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50) /* sound fx */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

ROM_START( mugsmash )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mugs_04.bin", 0x00000, 0x40000, CRC(2498fd27) SHA1(7b746efe8aaf346e4489118ac2a3fc9929a55b83) )
	ROM_LOAD16_BYTE( "mugs_05.bin", 0x00001, 0x40000, CRC(95efb40b) SHA1(878c0a3754aa728f58044c6a7f243724b718fe1b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "mugs_03.bin", 0x00000, 0x10000 , CRC(0101df2d) SHA1(35e1efa4a11c0f9d9db5ee057926e5de29c3a4c1) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "mugs_02.bin", 0x00000, 0x20000, CRC(f92a7f4a) SHA1(3717ef64876be9ada378b449749918ce9072073a) )
	ROM_LOAD( "mugs_01.bin", 0x20000, 0x20000, CRC(1a3a0b39) SHA1(8847530027cf4be03ffbc6d78dee97b459d03a04) )

	ROM_REGION( 0x300000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mugs_11.bin", 0x000000, 0x080000, CRC(1c9f5acf) SHA1(dd8329ed05a3467844c26d3f89ffb6213aba2034) )
	ROM_LOAD16_BYTE( "mugs_10.bin", 0x000001, 0x080000, CRC(6b3c22d9) SHA1(7ba6d754c08ed5b2be282ffd6a674c3a4aa0e9b2) )
	ROM_LOAD16_BYTE( "mugs_09.bin", 0x100000, 0x080000, CRC(4e9490f3) SHA1(e5f195c9bee3b92c559d1100c1019473a30ba28e) )
	ROM_LOAD16_BYTE( "mugs_08.bin", 0x100001, 0x080000, CRC(716328d5) SHA1(d1b84a25985acfb7ccb1582ef9ac8267250888c0) )
	ROM_LOAD16_BYTE( "mugs_07.bin", 0x200000, 0x080000, CRC(9e3167fd) SHA1(8c73c26e8e50e8f2ee3307f5aef23caba90c22eb) )
	ROM_LOAD16_BYTE( "mugs_06.bin", 0x200001, 0x080000, CRC(8df75d29) SHA1(d0add24ac974da4636d2631f5590516de0f8df4a) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles */
	ROM_LOAD( "mugs_12.bin", 0x000000, 0x080000, CRC(c0a6ed98) SHA1(13850c6bcca65bdc782040c470c4966aee19551d) )
	ROM_LOAD( "mugs_13.bin", 0x080000, 0x080000, CRC(e2be8595) SHA1(077b1a262c54acf74e6ec37702bcfed41bc31000) )
	ROM_LOAD( "mugs_14.bin", 0x100000, 0x080000, CRC(24e81068) SHA1(1e33aa7d2b873dd13d5823880c46d3d3e867d6b6) )
	ROM_LOAD( "mugs_15.bin", 0x180000, 0x080000, CRC(82e8187c) SHA1(c7a0e1b3d90dbbe2588886a27a07a9c336447ae3) )
ROM_END

GAME( 1990?, mugsmash, 0, mugsmash, mugsmash, driver_device, 0, ROT0, "Electronic Devices Italy / 3D Games England", "Mug Smashers", MACHINE_SUPPORTS_SAVE )
