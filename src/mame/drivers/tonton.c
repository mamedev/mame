/**************************************************************************

Waku Waku Doubutsu Land TonTon (c) 199? Success

HW based off MSX2

driver by Angelo Salese

TODO:
- dip switches;
- hopper mechanism;

===========================================================================

WAKUWAKU DOUBUTSU LAND TONTON (ANIMAL VIDEO SLOT)
(c)SUCCESS / CABINET :TAIYO JIDOKI (SUN AUTO MACHINE)

CPU   : Z80
SOUND : YM2149F
XTAL  : 21477.27KHz

TONTON.BIN  : MAIN ROM

tonton_state *state = machine.driver_data<tonton_state>();

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/v9938.h"
#include "deprecat.h"

class tonton_state : public driver_device
{
public:
	tonton_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	bitmap_t *m_vdp0_bitmap;
};

#define MAIN_CLOCK XTAL_21_4772MHz


// from MSX2 driver, may be not accurate for this HW
#define MSX2_XBORDER_PIXELS		16
#define MSX2_YBORDER_PIXELS		28
#define MSX2_TOTAL_XRES_PIXELS		256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS		212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS	8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS	14 * 2

static void tonton_vdp0_interrupt(running_machine &machine, int i)
{
	cputag_set_input_line (machine, "maincpu", 0, (i ? HOLD_LINE : CLEAR_LINE));
}

static VIDEO_START( tonton )
{
	tonton_state *state = machine.driver_data<tonton_state>();
	state->m_vdp0_bitmap = machine.primary_screen->alloc_compatible_bitmap();
	v9938_init (machine, 0, *machine.primary_screen, state->m_vdp0_bitmap, MODEL_V9938, 0x40000, tonton_vdp0_interrupt);
	v9938_reset(0);
}

static SCREEN_UPDATE( tonton )
{
	tonton_state *state = screen->machine().driver_data<tonton_state>();
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	copybitmap(bitmap, state->m_vdp0_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

static WRITE8_HANDLER( tonton_coin_counter_w )
{
	/* lockout perhaps? */
	coin_counter_w(space->machine(), offset, data & 0x01);

	// data & 2 is hopper related

	if(data & 0xfe)
		printf("%02x %02x\n",data,offset);
}

static ADDRESS_MAP_START( tonton_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tonton_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x00, 0x01) AM_WRITE(tonton_coin_counter_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("DSW1")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW2")
	AM_RANGE(0x88, 0x88) AM_READWRITE( v9938_0_vram_r, v9938_0_vram_w )
	AM_RANGE(0x89, 0x89) AM_READWRITE( v9938_0_status_r, v9938_0_command_w )
	AM_RANGE(0x8a, 0x8a) AM_WRITE( v9938_0_palette_w )
	AM_RANGE(0x8b, 0x8b) AM_WRITE( v9938_0_register_w )
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREAD("aysnd", ay8910_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( tonton )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("1. Pig")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("2. Penguin")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("3. Tiger")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("4. Cow")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("5. Bear")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("6. Elephant")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_M) PORT_NAME("7. Lion")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset Switch")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
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

static MACHINE_START( tonton )
{
}

static MACHINE_RESET( tonton )
{
}


INTERRUPT_GEN( tonton_interrupt )
{
	v9938_set_sprite_limit(0, 0);
	v9938_set_resolution(0, 0);
	v9938_interrupt(device->machine(), 0);
}

static MACHINE_CONFIG_START( tonton, tonton_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(tonton_map)
	MCFG_CPU_IO_MAP(tonton_io)
	MCFG_CPU_VBLANK_INT_HACK(tonton_interrupt, 262)

	MCFG_MACHINE_START(tonton)
	MCFG_MACHINE_RESET(tonton)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen",RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, MSX2_TOTAL_YRES_PIXELS)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
	MCFG_SCREEN_UPDATE(tonton)

	MCFG_PALETTE_LENGTH(512)
	MCFG_PALETTE_INIT( v9938 )

	MCFG_VIDEO_START(tonton)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", YM2149, MAIN_CLOCK/4) /* guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tonton )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tonton.bin",   0x0000, 0x10000, CRC(6c9cacfb) SHA1(21afd5a40b785300b013ac8cb31f5e4f480657ef) )
ROM_END

GAME( 199?, tonton,  0,   tonton,  tonton,  0, ROT0, "Success / Taiyo Jidoki", "Waku Waku Doubutsu Land TonTon (Japan)", 0 )
