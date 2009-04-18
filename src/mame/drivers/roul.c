/* Super Lucky Roulette?

driver by Roberto Zandona'
thanks to Angelo Salese for some precious advice

TO DO:
- check palette

Has 36 pin Cherry master looking edge connector

.u12 2764 stickered 1
.u19 27256 stickered 2
.u15 tibpal16l8-25 (checksum was 0)
.u56 tibpal16l8-25 (checksum was 0)
.u38 82s123
.u53 82s123

Z80 x2
Altera Ep1810LC-45
20.000 MHz crystal
video 464p10 x4 (board silcksreeend 4416)
AY-3-8912A

ROM text showed SUPER LUCKY ROULETTE LEISURE ENT





blitter:
there are 4 registers:

reg[0] -> y
reg[1] -> x
reg[3] & 0x0f -> color
reg[3] & 0x10 -> y direction (to the up or to the down)
reg[3] & 0x20 -> x direction (to the right or to the left)
reg[3] & 0x40 -> width used in y direction
reg[3] & 0x80 -> width used in x direction
reg[2] -> width (number of pixel to draw)

with a write in reg[2] the command is executed

not handled commands with reg[3] & 0xc0 == 0x00
handled wrongly commands with reg[3] & 0xc0 == 0xc0 (handled like reg[3] & 0x40)
*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "roul.lh"

#define VIDEOBUF_SIZE 256*256

UINT8 reg[0x10];
UINT8 *videobuf;

static UINT8 lamp_old = 0;

static PALETTE_INIT( roul )
{
	int bit0, bit1, bit2 , r, g, b;
	int brightness;
	int i;

	for (i = 0; i < 0x20; ++i)
	{
		brightness = (color_prom[0] >> 7) & 0x01;
		bit0 = brightness;
		bit1 = ((color_prom[0] >> 0) & 0x01) * brightness;
		bit2 = ((color_prom[0] >> 1) & 0x01) * brightness;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = brightness;
		bit1 = ((color_prom[0] >> 2) & 0x01) * brightness;
		bit2 = ((color_prom[0] >> 3) & 0x01) * brightness;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = brightness;
		bit1 = ((color_prom[0] >> 4) & 0x01) * brightness;
		bit2 = ((color_prom[0] >> 5) & 0x01) * brightness;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

static READ8_HANDLER( testf5_r )
{
	logerror("Read unknown port $f5 at %04x\n",cpu_get_pc(space->cpu));
	return mame_rand(space->machine) & 0x00ff;
}

static WRITE8_HANDLER( testfx_w )
{
	reg[offset] = data;
	if (offset==2)
	{
		int i;
		int width	= reg[2];
		int y		= reg[0];
		int x		= reg[1];
		int color	= reg[3] & 0x0f;
		int xdirection = 1, ydirection = 1;

		if (reg[3] & 0x10) ydirection = -1;
		if (reg[3] & 0x20) xdirection = -1;

		if (reg[3] & 0x40)
			for (i = 0; i < width; i++ )
				videobuf[(y + i * ydirection) * 256 + x] = color;
		else if (reg[3] & 0x80)
			for (i = 0; i < width; i++ )
				videobuf[y * 256 + x + i * xdirection] = color;
		else
			logerror("Write [%02x] -> %02x\n",offset,data);
	}
if (offset > 3) logerror("Write [%02x] -> %02x\n",offset,data);
}

static WRITE8_HANDLER( sound_latch_w )
{
 	soundlatch_w(space, 0, data & 0xff);
 	cpu_set_input_line(space->machine->cpu[1], 0, HOLD_LINE);
}

static WRITE8_HANDLER( ball_w )
{
	int lamp = data;

	output_set_lamp_value(data, 1);
	output_set_lamp_value(lamp_old, 0);
	lamp_old = lamp;
}

static ADDRESS_MAP_START( roul_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( roul_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf4) AM_WRITE(testfx_w)
	AM_RANGE(0xf5, 0xf5) AM_READ(testf5_r)
	AM_RANGE(0xf8, 0xf8) AM_READ_PORT("DSW")
	AM_RANGE(0xf9, 0xf9) AM_WRITE(ball_w)
	AM_RANGE(0xfa, 0xfa) AM_READ_PORT("IN0")
	AM_RANGE(0xfd, 0xfd) AM_READ_PORT("IN1")
	AM_RANGE(0xfe, 0xfe) AM_WRITE(sound_latch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x13ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay", ay8910_address_data_w)
ADDRESS_MAP_END

static VIDEO_START(roul)
{
	videobuf = auto_malloc(VIDEOBUF_SIZE * sizeof(*videobuf));
	memset(videobuf, 0, VIDEOBUF_SIZE * sizeof(*videobuf));
}

static VIDEO_UPDATE(roul)
{
	int i,j;
	for (i = 0; i < 256; i++)
		for (j = 0; j < 256; j++)
			*BITMAP_ADDR16(bitmap, j, i) = videobuf[j * 256 + 255 - i];
	return 0;
}

static INPUT_PORTS_START( roul )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_NAME("Coin 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_Q)
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_T)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_NAME("Bet") 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_D)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_DRIVER_START( roul )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(roul_map, 0)
	MDRV_CPU_IO_MAP(roul_cpu_io_map,0)
	MDRV_CPU_VBLANK_INT("screen",nmi_line_pulse)

	MDRV_CPU_ADD("soundcpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(sound_map, 0)
	MDRV_CPU_IO_MAP(sound_cpu_io_map,0)

	MDRV_PALETTE_INIT(roul)

 	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(roul)
	MDRV_VIDEO_UPDATE(roul)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910, 1000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

ROM_START(roul)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("roul.u19",	0x0000, 0x8000, CRC(1ec37876) SHA1(c2877646dad9daebc55db57d513ad448b1f4c923) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("roul.u12",	0x0000, 0x1000, CRC(356fe025) SHA1(bca69e090a852454e921130afbdd28021b62c44e) )
	ROM_CONTINUE(0x0000,0x1000)

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "roul.u53",	0x0000, 0x0020, CRC(1965dfaa) SHA1(114eccd3e478902ac7dbb10b9425784231ff581e) )
	ROM_LOAD( "roul.u38",	0x0020, 0x0020, CRC(23ae22c1) SHA1(bf0383462976ec6341ffa8a173264ce820bc654a) )
ROM_END

GAMEL( 1990, roul,  0,   roul, roul, 0, ROT0, "bootleg", "Super Lucky Roulette", GAME_IMPERFECT_GRAPHICS, layout_roul )
