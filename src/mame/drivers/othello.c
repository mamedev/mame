/*

Othello (version 3.0) - Success 1984
-------------------------------------

driver by Tomasz Slanina

CPU Board:
 D780C          - main CPU (Z80)
 HD46505SP      - CRTC
 D780-C         - Sound CPU (Z80)
 AY-3-8910 x2   - Sound
 D7751C         - ADPCM "Speech processor"
 D8243          - I/O Expander for D7751C (8048 based)

Video Board:
 almost empty - 3/4 sodlering pins not populated



Todo:

- hook up upd7751c sample player (it works correctly but there's main cpu side write(latch/command) missing)
- correct colors (based on the color DAC (24 resistors) on pcb
- cocktail mode
- map a bunch of unknown read/writes (related to above i think)

Notes:

DSw 1:2
Limit for help/undo (matta):
- when it's off, you can use each of them twice
 every time you win and advance to the next game
- when it's on, you can only use them twice throughout the game

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"


#define	TILE_WIDTH 6


class othello_state : public driver_device
{
public:
	othello_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;

	/* video-related */
	int    tile_bank;

	/* misc */
	int   ay_select;
	int   ack_data;
	UINT8 n7751_command;
//  UINT32 n7751_rom_address;
	int sound_addr;
	int n7751_busy;

	/* devices */
	running_device *maincpu;
	running_device *mc6845;
	running_device *n7751;
	running_device *ay1;
	running_device *ay2;
};


static MC6845_UPDATE_ROW( update_row )
{
	othello_state *state = device->machine->driver_data<othello_state>();
	int cx, x;
	UINT32 data_address;
	UINT32 tmp;

	const UINT8 *gfx = memory_region(device->machine, "gfx");

	for(cx = 0; cx < x_count; ++cx)
	{
		data_address = ((state->videoram[ma + cx] + state->tile_bank) << 4) | ra;
		tmp = gfx[data_address] | (gfx[data_address + 0x2000] << 8) | (gfx[data_address + 0x4000] << 16);

		for(x = 0; x < TILE_WIDTH; ++x)
		{
			*BITMAP_ADDR16(bitmap, y, (cx * TILE_WIDTH + x) ^ 1) = tmp & 0x0f;
			tmp >>= 4;
		}
	}
}

static PALETTE_INIT( othello )
{
	int i;
	for (i = 0; i < machine->total_colors(); i++)
	{
		palette_set_color(machine, i, MAKE_RGB(0xff, 0x00, 0xff));
	}

	/* only colors  2,3,7,9,c,d,f are used */
	palette_set_color(machine, 0x02, MAKE_RGB(0x00, 0xff, 0x00));
	palette_set_color(machine, 0x03, MAKE_RGB(0xff, 0x7f, 0x00));
	palette_set_color(machine, 0x07, MAKE_RGB(0x00, 0x00, 0x00));
	palette_set_color(machine, 0x09, MAKE_RGB(0xff, 0x00, 0x00));
	palette_set_color(machine, 0x0c, MAKE_RGB(0x00, 0x00, 0xff));
	palette_set_color(machine, 0x0d, MAKE_RGB(0x7f, 0x7f, 0x00));
	palette_set_color(machine, 0x0f, MAKE_RGB(0xff, 0xff, 0xff));
}

static VIDEO_UPDATE( othello )
{
	othello_state *state = screen->machine->driver_data<othello_state>();

	mc6845_update(state->mc6845, bitmap, cliprect);
	return 0;
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x97ff) AM_NOP /* not populated */
	AM_RANGE(0x9800, 0x9fff) AM_RAM AM_BASE_MEMBER(othello_state, videoram)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static READ8_HANDLER( unk_87_r )
{
	/* n7751_status_r ?  bit 7 = ack/status from device connected  to port 8a? */
	return mame_rand(space->machine);
}

static WRITE8_HANDLER( unk_8a_w )
{
	/*
    othello_state *state = space->machine->driver_data<othello_state>();

    state->n7751_command = (data & 0x07);
    cpu_set_input_line(state->n7751, 0, ((data & 0x08) == 0) ? ASSERT_LINE : CLEAR_LINE);
    //cpu_set_input_line(state->n7751, 0, (data & 0x02) ? CLEAR_LINE : ASSERT_LINE);
    cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(100));
    */

	logerror("8a -> %x\n", data);
}

static WRITE8_HANDLER( unk_8c_w )
{
	logerror("8c -> %x\n", data);
}

static READ8_HANDLER( unk_8c_r )
{
	return mame_rand(space->machine);
}

static READ8_HANDLER( sound_ack_r )
{
	othello_state *state = space->machine->driver_data<othello_state>();
	return state->ack_data;
}

static WRITE8_HANDLER( unk_8f_w )
{
	logerror("8f -> %x\n", data);
}

static WRITE8_HANDLER( tilebank_w )
{
	othello_state *state = space->machine->driver_data<othello_state>();
	state->tile_bank = (data == 0x0f) ? 0x100 : 0x00;
	logerror("tilebank -> %x\n", data);
}

static ADDRESS_MAP_START( main_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x08) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x09, 0x09) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x80, 0x80) AM_READ_PORT("INP")
	AM_RANGE(0x81, 0x81) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x83, 0x83) AM_READ_PORT("DSW")
	AM_RANGE(0x86, 0x86) AM_WRITE(tilebank_w)
	AM_RANGE(0x87, 0x87) AM_READ(unk_87_r)
	AM_RANGE(0x8a, 0x8a) AM_WRITE(unk_8a_w)
	AM_RANGE(0x8c, 0x8c) AM_READWRITE(unk_8c_r, unk_8c_w)
	AM_RANGE(0x8d, 0x8d) AM_READWRITE(sound_ack_r, soundlatch_w)
	AM_RANGE(0x8f, 0x8f) AM_WRITE(unk_8f_w)
ADDRESS_MAP_END

static READ8_HANDLER( latch_r )
{
	int retval = soundlatch_r(space, 0);
	soundlatch_clear_w(space, 0, 0);
	return retval;
}

static WRITE8_HANDLER( ay_select_w )
{
	othello_state *state = space->machine->driver_data<othello_state>();
	state->ay_select = data;
}

static WRITE8_HANDLER( ack_w )
{
	othello_state *state = space->machine->driver_data<othello_state>();
	state->ack_data = data;
}

static WRITE8_HANDLER( ay_address_w )
{
	othello_state *state = space->machine->driver_data<othello_state>();

	if (state->ay_select & 1) ay8910_address_w(state->ay1, 0, data);
	if (state->ay_select & 2) ay8910_address_w(state->ay2, 0, data);
}

static WRITE8_HANDLER( ay_data_w )
{
	othello_state *state = space->machine->driver_data<othello_state>();

	if (state->ay_select & 1) ay8910_data_w(state->ay1, 0, data);
	if (state->ay_select & 2) ay8910_data_w(state->ay2, 0, data);
}

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(latch_r)
	AM_RANGE(0x01, 0x01) AM_WRITE(ay_data_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(ay_address_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(ack_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(ay_select_w)
ADDRESS_MAP_END

static WRITE8_DEVICE_HANDLER( n7751_rom_control_w )
{
	othello_state *state = device->machine->driver_data<othello_state>();

	/* P4 - address lines 0-3 */
	/* P5 - address lines 4-7 */
	/* P6 - address lines 8-11 */
	/* P7 - ROM selects */
	switch (offset)
	{
		case 0:
			state->sound_addr = (state->sound_addr & ~0x00f) | ((data & 0x0f) << 0);
			break;

		case 1:
			state->sound_addr = (state->sound_addr & ~0x0f0) | ((data & 0x0f) << 4);
			break;

		case 2:
			state->sound_addr = (state->sound_addr & ~0xf00) | ((data & 0x0f) << 8);
			break;

		case 3:
			state->sound_addr &= 0xfff;
			{

				if (!BIT(data, 0)) state->sound_addr |= 0x0000;
				if (!BIT(data, 1)) state->sound_addr |= 0x1000;
				if (!BIT(data, 2)) state->sound_addr |= 0x2000;
				if (!BIT(data, 3)) state->sound_addr |= 0x3000;
			}
			break;
	}
}

static READ8_HANDLER( n7751_rom_r )
{
	othello_state *state = space->machine->driver_data<othello_state>();
	return memory_region(space->machine, "n7751data")[state->sound_addr];
}

static READ8_HANDLER( n7751_command_r )
{
	othello_state *state = space->machine->driver_data<othello_state>();
	return 0x80 | ((state->n7751_command & 0x07) << 4);
}

static WRITE8_DEVICE_HANDLER( n7751_p2_w )
{
	othello_state *state = device->machine->driver_data<othello_state>();

	/* write to P2; low 4 bits go to 8243 */
	i8243_p2_w(device, offset, data & 0x0f);

	/* output of bit $80 indicates we are ready (1) or busy (0) */
	/* no other outputs are used */
	state->n7751_busy = data;
}

static READ8_HANDLER( n7751_t1_r )
{
	/* T1 - labelled as "TEST", connected to ground */
	return 0;
}

static ADDRESS_MAP_START( n7751_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MCS48_PORT_T1,   MCS48_PORT_T1) AM_READ(n7751_t1_r)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2) AM_READ(n7751_command_r)
	AM_RANGE(MCS48_PORT_BUS,  MCS48_PORT_BUS) AM_READ(n7751_rom_r)
	AM_RANGE(MCS48_PORT_P1,   MCS48_PORT_P1) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2) AM_DEVWRITE("n7751_8243", n7751_p2_w)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE("n7751_8243", i8243_prog_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( othello )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, "Limit for Matta" )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )		PORT_DIPLOCATION("SW1:5") /* stored at $fd1e */
	PORT_DIPNAME( 0x60, 0x60, "Timer (seconds)" )	PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x40, "8" )
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )

	PORT_START("INP")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )  PORT_PLAYER(2)

	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

INPUT_PORTS_END

static const mc6845_interface h46505_intf =
{
	"screen",	/* screen we are acting on */
	TILE_WIDTH,	/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	update_row, /* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


static MACHINE_START( othello )
{
	othello_state *state = machine->driver_data<othello_state>();

	state->maincpu = machine->device("maincpu");
	state->mc6845 = machine->device("crtc");
	state->n7751 = machine->device("n7751");
	state->ay1 = machine->device("ay1");
	state->ay2 = machine->device("ay2");

	state_save_register_global(machine, state->tile_bank);
	state_save_register_global(machine, state->ay_select);
	state_save_register_global(machine, state->ack_data);
	state_save_register_global(machine, state->n7751_command);
	state_save_register_global(machine, state->sound_addr);
	state_save_register_global(machine, state->n7751_busy);
}

static MACHINE_RESET( othello )
{
	othello_state *state = machine->driver_data<othello_state>();

	state->tile_bank = 0;
	state->ay_select = 0;
	state->ack_data = 0;
	state->n7751_command = 0;
	state->sound_addr = 0;
	state->n7751_busy = 0;
}

static MACHINE_CONFIG_START( othello, othello_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,XTAL_8MHz/2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu",Z80,XTAL_3_579545MHz)
	MDRV_CPU_PROGRAM_MAP(audio_map)
	MDRV_CPU_IO_MAP(audio_portmap)

	MDRV_CPU_ADD("n7751", N7751, XTAL_6MHz)
	MDRV_CPU_IO_MAP(n7751_portmap)

	MDRV_I8243_ADD("n7751_8243", NULL, n7751_rom_control_w)

	MDRV_MACHINE_START(othello)
	MDRV_MACHINE_RESET(othello)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*6, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*6-1, 0*8, 64*8-1)

	MDRV_PALETTE_LENGTH(0x10)
	MDRV_PALETTE_INIT(othello)

	MDRV_VIDEO_UPDATE(othello)

	MDRV_MC6845_ADD("crtc", H46505, 1000000 /* ? MHz */, h46505_intf)	/* H46505 @ CPU clock */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("ay2", AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

ROM_START( othello )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4.ic59",   0x0000, 0x2000, CRC(9f82fe14) SHA1(59600264ccce787383827fc5aa0f2c23728f6946))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3.ic32",   0x0000, 0x2000, CRC(2bb4f75d) SHA1(29a659031acf0d50f374f440b8d353bcf98145a0))

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) )

	ROM_REGION( 0x4000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "1.ic48", 0x0000, 0x2000, CRC(c3807dea) SHA1(d6339380e1239f3e20bcca2fbc673ad72e9ca608))
	ROM_LOAD( "2.ic49", 0x2000, 0x2000, CRC(a945f3e7) SHA1(ea18efc18fda63ce1747287bbe2a9704b08daff8))

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "5.ic40",   0x0000, 0x2000, CRC(45fdc1ab) SHA1(f30f6002e3f34a647effac8b0116c8ed064e226a))
	ROM_LOAD( "6.ic41",   0x2000, 0x2000, CRC(467a731f) SHA1(af80e854522e53fb1b9af7945b2c803a654c6f65))
	ROM_LOAD( "7.ic42",   0x4000, 0x2000, CRC(a76705f7) SHA1(b7d2a65d65d065732ddd0b3b738749369b382b48))
ROM_END

GAME( 1984, othello,  0,       othello,  othello,  0, ROT0, "Success", "Othello (version 3.0)", GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
