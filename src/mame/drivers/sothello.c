/*
 Super Othello (c)1986 Fujiwara/Success

    driver by Tomasz Slanina

         1    2    3    4     5     6     7      8      9     10     11    12
+---------------------------------------------------------------------------------+
|                                                                                 |
+-+    LA460  LA6324  M5205  X3   Z80A    1      2    5816P  74374  74138         |  A
  |                                                                               |
  |                                                                               |
+-+        74367 Y3014 74174 74174       7404   7474  74138  7404   7432          |  B
|                                                                                 |
|                                                                                 |
|            74367 DSW1 YM2203    Z80A    3      4      5           6264          |  C
| J                                                                               |
| A                                                                               |
| M   C1663 74367  DSW2           7408 74125   7404  74138   74139  74174  7408   |  D
| M           X2       7414  7474                                                 |
| A                                                                               |
|     C1663 V9938 41464 41464       X1   7474  74139  7432   74157  74244  7432   |  E
|                                                                                 |
|                                                                                 |
+-+   C1663       41464 41464     6809B   6     6264   6264  6264   74244  74245  |  F
  |                                                                               |
  |                                                                               |
+-+   C1663                                                                       |  H
|                                                                                 |
+---------------------------------------------------------------------------------+

CPU  : Z80A(x2) 68B09
Sound: YM2203?(surface scrached) + M5205
OSC  : 8.0000MHz(X1)   21.477 MHz(X2)   384kHz(X3)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "video/v9938.h"


class sothello_state : public driver_device
{
public:
	sothello_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_subcpu_status;
	int m_soundcpu_busy;
	int m_msm_data;
};


#define VDP_MEM             0x40000

#define MAINCPU_CLOCK       (XTAL_21_4772MHz/6)
#define SOUNDCPU_CLOCK      (XTAL_21_4772MHz/6)
#define YM_CLOCK            (XTAL_21_4772MHz/12)
#define MSM_CLOCK           (XTAL_384kHz)
#define SUBCPU_CLOCK        (XTAL_8MHz/4)


/* main Z80 */

static WRITE8_HANDLER(bank_w)
{
    UINT8 *RAM = space->machine().region("maincpu")->base();
    int bank=0;
    switch(data^0xff)
    {
        case 1: bank=0; break;
        case 2: bank=1; break;
        case 4: bank=2; break;
        case 8: bank=3; break;
    }
    memory_set_bankptr(space->machine(),"bank1",&RAM[bank*0x4000+0x10000]);
}

static TIMER_CALLBACK( subcpu_suspend )
{
    machine.device<cpu_device>("sub")->suspend(SUSPEND_REASON_HALT, 1);
}

static TIMER_CALLBACK( subcpu_resume )
{
    machine.device<cpu_device>("sub")->resume(SUSPEND_REASON_HALT);
    cputag_set_input_line(machine, "sub", INPUT_LINE_NMI, PULSE_LINE);
}

static READ8_HANDLER( subcpu_halt_set )
{
	sothello_state *state = space->machine().driver_data<sothello_state>();
    space->machine().scheduler().synchronize(FUNC(subcpu_suspend));
    state->m_subcpu_status|=2;
    return 0;
}

static READ8_HANDLER( subcpu_halt_clear )
{
	sothello_state *state = space->machine().driver_data<sothello_state>();
    space->machine().scheduler().synchronize(FUNC(subcpu_resume));
    state->m_subcpu_status&=~1;
    state->m_subcpu_status&=~2;
    return 0;
}

static READ8_HANDLER(subcpu_comm_status )
{
	sothello_state *state = space->machine().driver_data<sothello_state>();
    return state->m_subcpu_status;
}

static READ8_HANDLER( soundcpu_status_r )
{
	sothello_state *state = space->machine().driver_data<sothello_state>();
    return state->m_soundcpu_busy;
}

static ADDRESS_MAP_START( maincpu_mem_map, AS_PROGRAM, 8 )
    AM_RANGE(0x0000, 0x7fff) AM_ROM
    AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
    AM_RANGE(0xc000, 0xdfff) AM_RAM AM_MIRROR(0x1800) AM_SHARE("share1")
    AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( maincpu_io_map, AS_IO, 8 )
    ADDRESS_MAP_GLOBAL_MASK(0xff)
    AM_RANGE( 0x00, 0x0f) AM_READ_PORT("INPUT1")
    AM_RANGE( 0x10, 0x1f) AM_READ_PORT("INPUT2")
    AM_RANGE( 0x20, 0x2f) AM_READ_PORT("SYSTEM")
    AM_RANGE( 0x30, 0x30) AM_READ(subcpu_halt_set)
    AM_RANGE( 0x31, 0x31) AM_READ(subcpu_halt_clear)
    AM_RANGE( 0x32, 0x32) AM_READ(subcpu_comm_status)
    AM_RANGE( 0x33, 0x33) AM_READ(soundcpu_status_r)
    AM_RANGE( 0x40, 0x4f) AM_WRITE(soundlatch_w)
    AM_RANGE( 0x50, 0x50) AM_WRITE(bank_w)
    AM_RANGE( 0x60, 0x61) AM_MIRROR(0x02) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
						/* not sure, but the A1 line is ignored, code @ $8b8 */
    AM_RANGE( 0x70, 0x70) AM_WRITE( v9938_0_vram_w ) AM_READ( v9938_0_vram_r )
    AM_RANGE( 0x71, 0x71) AM_WRITE( v9938_0_command_w ) AM_READ( v9938_0_status_r )
    AM_RANGE( 0x72, 0x72) AM_WRITE( v9938_0_palette_w )
    AM_RANGE( 0x73, 0x73) AM_WRITE( v9938_0_register_w )
ADDRESS_MAP_END

/* sound Z80 */

static WRITE8_DEVICE_HANDLER(msm_cfg_w)
{
/*
     bit 0 = RESET
     bit 1 = 4B/3B 0
     bit 2 = S2    1
     bit 3 = S1    2
*/
    msm5205_playmode_w(device, BITSWAP8((data>>1), 7,6,5,4,3,0,1,2)); /* or maybe 7,6,5,4,3,0,2,1 ??? */
    msm5205_reset_w(device,data&1);
}

static WRITE8_HANDLER( msm_data_w )
{
	sothello_state *state = space->machine().driver_data<sothello_state>();
    state->m_msm_data = data;

}

static WRITE8_HANDLER(soundcpu_busyflag_set_w)
{
	sothello_state *state = space->machine().driver_data<sothello_state>();
    state->m_soundcpu_busy=1;
}

static WRITE8_HANDLER(soundcpu_busyflag_reset_w)
{
	sothello_state *state = space->machine().driver_data<sothello_state>();
    state->m_soundcpu_busy=0;
}

static WRITE8_HANDLER(soundcpu_int_clear_w)
{
    cputag_set_input_line(space->machine(), "soundcpu", 0, CLEAR_LINE );
}

static ADDRESS_MAP_START( soundcpu_mem_map, AS_PROGRAM, 8 )
    AM_RANGE(0x0000, 0xdfff) AM_ROM
    AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( soundcpu_io_map, AS_IO, 8 )
    ADDRESS_MAP_GLOBAL_MASK(0xff)
    AM_RANGE(0x00, 0x00) AM_READ(soundlatch_r)
    AM_RANGE(0x01, 0x01) AM_WRITE(msm_data_w)
    AM_RANGE(0x02, 0x02) AM_DEVWRITE("msm", msm_cfg_w)
    AM_RANGE(0x03, 0x03) AM_WRITE(soundcpu_busyflag_set_w)
    AM_RANGE(0x04, 0x04) AM_WRITE(soundcpu_busyflag_reset_w)
    AM_RANGE(0x05, 0x05) AM_WRITE(soundcpu_int_clear_w)
ADDRESS_MAP_END

/* sub 6809 */

static void unlock_shared_ram(address_space *space)
{
	sothello_state *state = space->machine().driver_data<sothello_state>();
    if(!space->machine().device<cpu_device>("sub")->suspended(SUSPEND_REASON_HALT))
    {
        state->m_subcpu_status|=1;
    }
    else
    {
        logerror("Sub cpu active! @%x\n",cpu_get_pc(&space->device()));
    }
}

static WRITE8_HANDLER(subcpu_status_w)
{
    unlock_shared_ram(space);
}

static READ8_HANDLER(subcpu_status_r)
{
    unlock_shared_ram(space);
    return 0;
}

static ADDRESS_MAP_START( subcpu_mem_map, AS_PROGRAM, 8 )
    AM_RANGE(0x0000, 0x1fff) AM_READWRITE(subcpu_status_r,subcpu_status_w)
    AM_RANGE(0x2000, 0x77ff) AM_RAM
    AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("share1")  /* upper 0x800 of 6264 is shared  with main cpu */
    AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( sothello )
 PORT_START("INPUT1")
  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
  PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)

  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

 PORT_START("INPUT2")

  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
  PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

 PORT_START("SYSTEM")
  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

  PORT_BIT( 0xf2, IP_ACTIVE_LOW, IPT_UNUSED )

 PORT_START("DSWA")
  PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )
  PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
  PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
  PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
  PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
  PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
  PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
  PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
  PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )

  PORT_DIPNAME( 0x1c, 0x10, "Timer" )
  PORT_DIPSETTING(    0x1c, "15" )
  PORT_DIPSETTING(    0x18, "20" )
  PORT_DIPSETTING(    0x14, "25" )
  PORT_DIPSETTING(    0x10, "30" )
  PORT_DIPSETTING(    0x0c, "35" )
  PORT_DIPSETTING(    0x08, "40" )
  PORT_DIPSETTING(    0x04, "45" )
  PORT_DIPSETTING(    0x00, "50" )

  PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )

 PORT_START("DSWB")
  PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )
  PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
  PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
  PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
  PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )

  PORT_DIPNAME( 0x30, 0x10, "Matta" ) /* undo moves */
  PORT_DIPSETTING(    0x30, "0" )
  PORT_DIPSETTING(    0x20, "1" )
  PORT_DIPSETTING(    0x10, "2" )
  PORT_DIPSETTING(    0x00, "3" )

  PORT_DIPNAME( 0x08, 0x08, "Games for 2 players" )
  PORT_DIPSETTING(    0x08, "1" )
  PORT_DIPSETTING(    0x00, "2" )

  PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static void irqhandler(device_t *device, int irq)
{
    cputag_set_input_line(device->machine(), "sub", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static void sothello_vdp_interrupt(running_machine &machine, int i)
{
    cputag_set_input_line(machine, "maincpu", 0, (i ? HOLD_LINE : CLEAR_LINE));
}

static TIMER_DEVICE_CALLBACK( sothello_interrupt )
{
    v9938_interrupt(timer.machine(), 0);
}

static void adpcm_int(device_t *device)
{
	sothello_state *state = device->machine().driver_data<sothello_state>();
    /* only 4 bits are used */
    msm5205_data_w( device, state->m_msm_data & 0x0f );
    cputag_set_input_line(device->machine(), "soundcpu", 0, ASSERT_LINE );
}


static const msm5205_interface msm_interface =
{
    adpcm_int,      /* interrupt function */
    MSM5205_S48_4B  /* changed on the fly */
};

static VIDEO_START( sothello )
{
    VIDEO_START_CALL(generic_bitmapped);
    v9938_init (machine, 0, *machine.primary_screen, machine.generic.tmpbitmap, MODEL_V9938, VDP_MEM, sothello_vdp_interrupt);
    v9938_reset(0);
}

static MACHINE_RESET(sothello)
{
    v9938_reset(0);
}

static const ym2203_interface ym2203_config =
{
    {
        AY8910_LEGACY_OUTPUT,
        AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSWA"),
		DEVCB_INPUT_PORT("DSWB"),
        DEVCB_NULL,
        DEVCB_NULL,
    },
    irqhandler
};

static MACHINE_CONFIG_START( sothello, sothello_state )

    /* basic machine hardware */

    MCFG_CPU_ADD("maincpu",Z80, MAINCPU_CLOCK)
    MCFG_CPU_PROGRAM_MAP(maincpu_mem_map)
    MCFG_CPU_IO_MAP(maincpu_io_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", sothello_interrupt, "screen", 0, 1)

    MCFG_CPU_ADD("soundcpu",Z80, SOUNDCPU_CLOCK)
    MCFG_CPU_PROGRAM_MAP(soundcpu_mem_map)
    MCFG_CPU_IO_MAP(soundcpu_io_map)

    MCFG_CPU_ADD("sub",M6809, SUBCPU_CLOCK)
    MCFG_CPU_PROGRAM_MAP(subcpu_mem_map)

    MCFG_QUANTUM_TIME(attotime::from_hz(600))

    MCFG_MACHINE_RESET(sothello)

    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_REFRESH_RATE(60)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
    MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
    MCFG_SCREEN_SIZE(512 + 32, (212 + 28) * 2)
    MCFG_SCREEN_VISIBLE_AREA(0, 512 + 32 - 1, 0, (212 + 28) * 2 - 1)
    MCFG_SCREEN_UPDATE(generic_bitmapped)

    MCFG_PALETTE_LENGTH(512)
    MCFG_PALETTE_INIT( v9938 )
    MCFG_VIDEO_START(sothello)

    /* sound hardware */
    MCFG_SPEAKER_STANDARD_MONO("mono")
    MCFG_SOUND_ADD("ymsnd", YM2203, YM_CLOCK)
    MCFG_SOUND_CONFIG(ym2203_config)
    MCFG_SOUND_ROUTE(0, "mono", 0.25)
    MCFG_SOUND_ROUTE(1, "mono", 0.25)
    MCFG_SOUND_ROUTE(2, "mono", 0.25)
    MCFG_SOUND_ROUTE(3, "mono", 0.50)

    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

    MCFG_SOUND_ADD("msm",MSM5205, MSM_CLOCK)
    MCFG_SOUND_CONFIG(msm_interface)
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sothello )
    ROM_REGION( 0x20000, "maincpu", 0 )
    ROM_LOAD( "3.7c",   0x0000, 0x8000, CRC(47f97bd4) SHA1(52c9638f098fdcf66903fad7dafe3ab171758572) )
    ROM_LOAD( "4.8c",   0x10000, 0x8000, CRC(a98414e9) SHA1(6d14e1f9c79b95101e0aa101034f398af09d7f32) )
    ROM_LOAD( "5.9c",   0x18000, 0x8000, CRC(e5b5d61e) SHA1(2e4b3d85f41d0796a4d61eae40dd824769e1db86) )

    ROM_REGION( 0x10000, "soundcpu", 0 )
    ROM_LOAD( "1.7a",   0x0000, 0x8000, CRC(6951536a) SHA1(64d07a692d6a167334c825dc173630b02584fdf6) )
    ROM_LOAD( "2.8a",   0x8000, 0x8000, CRC(9c535317) SHA1(b2e69b489e111d6f8105e68fade6e5abefb825f7) )

    ROM_REGION( 0x10000, "sub", 0 )
    ROM_LOAD( "6.7f",   0x8000, 0x8000, CRC(ee80fc78) SHA1(9a9d7925847d7a36930f0761c70f67a9affc5e7c) )
ROM_END

GAME( 1986, sothello,  0,       sothello,  sothello,  0, ROT0, "Success / Fujiwara", "Super Othello", 0 )
