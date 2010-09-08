/**************************************************************************************************************

Videotronics Poker (c) 198? Videotronics

preliminary driver by Angelo Salese

Notes:
- Looks like the 2nd generation of Noraut Poker / Draw Poker Hi-Lo HW.

TODO:
- Understand how the 6840PTM hooks up, needed to let it work properly;
- I/Os;
- sound;

===============================================================================================================

Bought as "old poker game by videotronics early 80's"

Scratched on the CPU board  SN1069
Scratched on the CPU board  SN1069

CPU board
.0  2716    stickered   DRAWPKR2    located top left
                8-F
                REV A

.1  2716    stickered   DRAWPKR2    located next to .0
                0-7
                REV A

ROM board
Top of board left to right
.R0 2716    stickered   RA
                0-7

.R1 2716    stickered   RA
                8-F

.R2 2716    stickered   BA
                0-7

.R3 2716    stickered   BA
                8-F

.R4 2716    stickered   GA
                0-7

.R5 2716    stickered   GA
                8-F


Below top row left to right
.R6 2716    stickered   RB
                0-7

.R7 2716    stickered   RB
                8-F

.R8 2716    stickered   BB
                0-7

.R9 2716    stickered   BB
                8-F

.R10    2716    stickered   GB
                0-7

.R11    2716    stickered   GB
                8-F

ROM data showed cards

6809 cpu
4.000Mhz crystal
MC6840P
mm74c920J/mmc6551j-9    x2


**************************************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6840ptm.h"


class vpoker_state : public driver_device
{
public:
	vpoker_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


static VIDEO_START( vpoker )
{
	vpoker_state *state = machine->driver_data<vpoker_state>();
	state->videoram = auto_alloc_array(machine, UINT8, 0x200);
}

static VIDEO_UPDATE( vpoker )
{
	vpoker_state *state = screen->machine->driver_data<vpoker_state>();
	UINT8 *videoram = state->videoram;
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0x0000;

	int y,x;

	for (y=0;y<0x10;y++)
	{
		for (x=0;x<0x20;x++)
		{
			int tile = videoram[count];
			//int colour = tile>>12;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,0,0,0,x*16,y*16);

			count++;
		}
	}

	return 0;
}

static READ8_HANDLER( blitter_r )
{
	if(offset == 6)
		return mame_rand(space->machine);

	return 0;
}

static WRITE8_HANDLER( blitter_w )
{
	vpoker_state *state = space->machine->driver_data<vpoker_state>();
	UINT8 *videoram = state->videoram;
	static UINT8 blit_ram[8];

	blit_ram[offset] = data;

	if(offset == 2)
	{
		int blit_offs;

		blit_offs = (blit_ram[1] & 0x01)<<8|(blit_ram[2] & 0xff);

		videoram[blit_offs] = blit_ram[0];
//      printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",blit_ram[0],blit_ram[1],blit_ram[2],blit_ram[3],blit_ram[4],blit_ram[5],blit_ram[6],blit_ram[7]);
	}
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0400, 0x0407) AM_DEVREADWRITE("6840ptm", ptm6840_read, ptm6840_write)
	AM_RANGE(0x0800, 0x0807) AM_READ(blitter_r) AM_WRITE(blitter_w)
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( vpoker )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
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

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
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

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
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

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5" )
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

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, "IN6" )
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

	PORT_START("IN7")
	PORT_DIPNAME( 0x01, 0x01, "IN7" )
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

static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,4),
	3,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4),RGN_FRAC(2,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 , 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16  },
	16*16
};

static GFXDECODE_START( vpoker )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END

static PALETTE_INIT( vpoker )
{
	int i;

	for (i = 0; i < 7; i++)
	{
		rgb_t color;

		color = MAKE_RGB(pal1bit((i & 4) >> 2),pal1bit(i & 1),pal1bit((i & 2) >> 1));

		palette_set_color(machine, i, color);
	}
}

static WRITE_LINE_DEVICE_HANDLER( ptm_irq )
{
	cputag_set_input_line(device->machine, "maincpu", M6809_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

static const ptm6840_interface ptm_intf =
{
	XTAL_4MHz,
	{ 0, 0, 0 },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	DEVCB_LINE(ptm_irq)
};

static MACHINE_CONFIG_START( vpoker, vpoker_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",M6809,XTAL_4MHz)
	MDRV_CPU_PROGRAM_MAP(main_map)
//  MDRV_CPU_VBLANK_INT("screen",irq0_line_hold) //irq0 valid too

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 512-1, 0*8, 256-1)
	MDRV_GFXDECODE(vpoker)
	MDRV_PALETTE_LENGTH(8)
	MDRV_PALETTE_INIT(vpoker)

	MDRV_VIDEO_START(vpoker)
	MDRV_VIDEO_UPDATE(vpoker)

	/* 6840 PTM */
	MDRV_PTM6840_ADD("6840ptm", ptm_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
//  MDRV_SOUND_ADD("aysnd", AY8910, 8000000/4 /* guess */)
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( vpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vpoker.1",   0xf000, 0x0800, CRC(790f3c4e) SHA1(c60485cc44dd742a4a9398b98c2bde8a95f625f3) )
	ROM_RELOAD(             0x2000, 0x0800 )
	ROM_LOAD( "vpoker.0",   0xf800, 0x0800, CRC(8ad8ce66) SHA1(84b606ab9698b957b631070296a9e6e64fabdd8a) )
	ROM_RELOAD(             0x2800, 0x0800 )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "vpoker.r0",  0x0000, 0x0800, CRC(1581202c) SHA1(7882fde76d0529fbfdd1235a39d04333e83f8a2f) )
	ROM_LOAD16_BYTE( "vpoker.r1",  0x1000, 0x0800, CRC(27695350) SHA1(09d1e0e6d5d823f091fa941a96f7c5f045501145) )
	ROM_LOAD16_BYTE( "vpoker.r2",  0x2000, 0x0800, CRC(1c0eab90) SHA1(19f3110088124f73de980502aab374888924d6a5) )
	ROM_LOAD16_BYTE( "vpoker.r3",  0x3000, 0x0800, CRC(7a8cb6f9) SHA1(d233f0f592c22dab6827e34c2cb22dd301a054e1) )
	ROM_LOAD16_BYTE( "vpoker.r4",  0x4000, 0x0800, CRC(755c4f02) SHA1(d19db1b1b2d41643cb69bb6eed46b1851de384c9) )
	ROM_LOAD16_BYTE( "vpoker.r5",  0x5000, 0x0800, CRC(ccd32805) SHA1(fdff53942f06b5fc7a292364afb98721369cc0f4) )
	ROM_LOAD16_BYTE( "vpoker.r6",  0x0001, 0x0800, CRC(77860770) SHA1(bffc8f38e9f63518706c093afd9254be8e15773d) )
	ROM_LOAD16_BYTE( "vpoker.r7",  0x1001, 0x0800, CRC(1ca9e74e) SHA1(3a2e71fb2f21acfa864dda4e459f7f150bddb988) )
	ROM_LOAD16_BYTE( "vpoker.r8",  0x2001, 0x0800, CRC(68022a42) SHA1(72a924a8ecf327821e444c5fb3ddd62510d4fc13) )
	ROM_LOAD16_BYTE( "vpoker.r9",  0x3001, 0x0800, CRC(5a71f01c) SHA1(e86a40e0533b24e66a2245e97670f131bd68be06) )
	ROM_LOAD16_BYTE( "vpoker.r10", 0x4001, 0x0800, CRC(5e0a7011) SHA1(9981f080581ef97f482e9a4b4ea0447c8bf89fc8) )
	ROM_LOAD16_BYTE( "vpoker.r11", 0x5001, 0x0800, CRC(960b1e05) SHA1(c692835f3cd0be6c221623c3955977ba6d8fd0cf) )
ROM_END

GAME( 198?, vpoker,  0,       vpoker,  vpoker,  0, ROT0, "Videotronics", "Videotronics Poker", GAME_NOT_WORKING | GAME_NO_SOUND )
