/*
file   : readme.txt
author : Stefan Lindberg
created: 2009-01-03
updated: *
version: 1.0


Unknown Tazmi game, 1981?


Note:
Untested PCB.
A bet/gamble game i presume, possible "King Derby".
The PCB is marked 1981.

See included PCB pics.



Roms:

Name           Size     CRC32           Chip Type
----------------------------------------------------------------------------
im1_yk.g1      4096     0x1921605d      D2732D
im2_yk.f1      4096     0x8504314e      M5L2732K
im3_yk.e1      4096     0xb034314e      M5L2732K
im4_d.d6       4096     0x20f2d999      M5L2732K
im5_d.c6       4096     0xc192cecc      D2732D
im6_d.b6       4096     0x257f4e0d      D2732D
s1.d1          4096     0x26974007      D2732D
s10_a.l8       4096     0x37b2736f      D2732D
s2.e1          4096     0xbedebfa7      D2732D
s3.f1          4096     0x0aa59571      D2732D
s4.g1          4096     0xccd5fb0e      D2732D
s5.d2          4096     0x32613df3      D2732D
s6.e2          4096     0xa151c422      D2732D
s7.f2          4096     0x7cfcee55      D2732D
s8.g2          4096     0xad667c05      D2732D
s9_a.ka        4096     0xca82cd81      D2732D
sg1_b.e1       4096     0x92ef3c13      D2732D

*/

#define CLK_1	XTAL_20MHz
#define CLK_2	XTAL_3_579545MHz

#include "driver.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/8255ppi.h"
#include "sound/ay8910.h"

static UINT8 *kingdrby_vram,*kingdrby_attr;

static VIDEO_START(kingdrby)
{

}

static VIDEO_UPDATE(kingdrby)
{
	const gfx_element *gfx = screen->machine->gfx[1];
	int count = 0;

	int y,x;


	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int tile = kingdrby_vram[count]|kingdrby_attr[count]<<8;
			tile&=0x1ff;
			//int colour = tile>>12;
			drawgfx(bitmap,gfx,tile,0,0,0,x*8,y*8,cliprect,TRANSPARENCY_NONE,0);

			count++;
		}
	}
	return 0;
}

static WRITE8_DEVICE_HANDLER( outport0_w )
{
//	popmessage("PPI0 port C out: %02X", data);
}

static WRITE8_DEVICE_HANDLER( outport1_w )
{
//	popmessage("PPI1 port A out: %02X", data);
}

static WRITE8_DEVICE_HANDLER( outport2_w )
{
//	popmessage("PPI1 port C(upper) out: %02X", data);
}

#if 0
static READ8_HANDLER( ff_r )
{
	return 0xff;
}
#endif

static UINT8 latch;

static WRITE8_HANDLER( master_latch_w)
{
//	cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_NMI, PULSE_LINE);
	latch = data;
}

static READ8_HANDLER( slave_latch_r )
{
//	cpu_set_input_line(space->machine->cpu[0], INPUT_LINE_NMI, PULSE_LINE);
	return latch;
}

static READ8_HANDLER( master_latch_r )
{
	return mame_rand(space->machine);
}

static ADDRESS_MAP_START( master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x3885, 0x3885) AM_READWRITE(master_latch_r,master_latch_w)
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_BASE(&kingdrby_vram)
	AM_RANGE(0x5000, 0x53ff) AM_RAM AM_BASE(&kingdrby_attr)
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x3fff) AM_ROM //sound rom tested for the post check
	AM_RANGE(0x4000, 0x43ff) AM_RAM //backup ram
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE(PPI8255, "ppi8255_0", ppi8255_r, ppi8255_w)	/* I/O Ports */
	AM_RANGE(0x6000, 0x6003) AM_DEVREADWRITE(PPI8255, "ppi8255_1", ppi8255_r, ppi8255_w)	/* I/O Ports */
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x7400, 0x77ff) AM_RAM
	AM_RANGE(0x7600, 0x7600) AM_DEVWRITE(MC6845, "crtc", mc6845_address_w)
	AM_RANGE(0x7601, 0x7601) AM_DEVREADWRITE(MC6845, "crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x7c00, 0x7c00) AM_READ(slave_latch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_DEVREAD(SOUND, "ay", ay8910_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE(SOUND, "ay", ay8910_address_data_w)
ADDRESS_MAP_END

/*
  slave

  5000-5003 PPI group modes 0/0 - A & B as input, C (all) as output.
  6000-6003 PPI group modes 0/0 - B & C (lower) as input, A & C (upper) as output.

  7c00  unknown read.

  crtc:         6845 type.
  screen size:  384x272    registers 00 & 04. (value-1)
  visible area: 256x224    registers 01 & 06.

  the clocks are a guess, but is the only logical combination I found to get a reasonable vertical of ~53Hz.

  the code is stuck with last checksum calculation. both z80 are halted.

  feeding the slave 7c00 read with value !=0, the code requests something from master 3885.
  feeding both, we have unmapped slave writes to 7400-744b and 0's to 7801-780f.

*/

static const ppi8255_interface ppi8255_intf[2] =
{
	/* A & B as input, C (all) as output */
	{
		DEVCB_INPUT_PORT("IN0"),	/* Port A read */
		DEVCB_INPUT_PORT("IN1"),	/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_HANDLER(outport0_w)   /* Port C write */
	},

	/* B & C (lower) as input, A & C (upper) as output */
	{
		DEVCB_NULL,					/* Port A read */
		DEVCB_INPUT_PORT("IN2"),	/* Port B read */
		DEVCB_INPUT_PORT("IN3"),	/* Port C read */
		DEVCB_HANDLER(outport1_w),  /* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_HANDLER(outport2_w)	/* Port C write */
	}
};

static INPUT_PORTS_START( kingdrby )

	PORT_START("IN0")	// ppi0 (5000)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN1")	// ppi0 (5001)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)

	PORT_START("IN2")	// ppi1 (6001)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")	// ppi1 (6002)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout layout8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{
		RGN_FRAC(0,2),
		RGN_FRAC(1,2)
	},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout16x16x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{
		RGN_FRAC(0,2),
		RGN_FRAC(1,2),
	},
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( kingdrby )
	GFXDECODE_ENTRY( "gfx1", 0x000000, layout8x8x2, 0, 0x1 )
	GFXDECODE_ENTRY( "gfx2", 0x000000, layout8x8x2, 0, 0x1 )
GFXDECODE_END

static const mc6845_interface mc6845_intf =
{
	"main",	/* screen we are acting on */
	8,		/* number of pixels per video memory address */
	NULL,	/* before pixel update callback */
	NULL,	/* row update callback */
	NULL,	/* after pixel update callback */
	NULL,	/* callback for display state changes */
	NULL,	/* HSYNC callback */
	NULL	/* VSYNC callback */
};

static READ8_DEVICE_HANDLER( test3_r )
{
	static UINT8 x;

	if(input_code_pressed(KEYCODE_Z))
		x++;

	if(input_code_pressed(KEYCODE_X))
		x--;

	popmessage("%02x",x);

	return x;
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(test3_r),
	DEVCB_HANDLER(test3_r),
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_DRIVER_START( kingdrby )
	MDRV_CPU_ADD("master", Z80, CLK_2)
	MDRV_CPU_PROGRAM_MAP(master_map,0)
	MDRV_CPU_IO_MAP(master_io_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("slave", Z80, CLK_2)
	MDRV_CPU_PROGRAM_MAP(slave_map,0)
	MDRV_CPU_IO_MAP(slave_io_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)
//	MDRV_CPU_PERIODIC_INT(nmi_line_pulse,100)

	MDRV_CPU_ADD("sound", Z80, CLK_1/8)		/* Or maybe it's the 20 mhz one? */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_IO_MAP(sound_io_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold) //to be removed from here...

	MDRV_QUANTUM_PERFECT_CPU("master")

	//MDRV_MACHINE_RESET(kingdrby)

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )

	MDRV_GFXDECODE(kingdrby)
	MDRV_PALETTE_LENGTH(256)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 224-1)	/* controlled by CRTC */

	MDRV_VIDEO_START(kingdrby)
	MDRV_VIDEO_UPDATE(kingdrby)

	MDRV_MC6845_ADD("crtc", MC6845, CLK_1/32, mc6845_intf)	/* 53.333 Hz. guess */

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, CLK_1/16)	/* guess */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( kingdrby )
	ROM_REGION( 0x3000, "master", 0 )
	ROM_LOAD( "im4_d.d6",  0x0000, 0x1000, CRC(20F2D999) SHA1(91DB46059F32B4791460DF3330260F4E60F016A5) )
	ROM_LOAD( "im5_d.c6",  0x1000, 0x1000, CRC(C192CECC) SHA1(63436BF3D9C1E34F6549830C8164295B7758D666) )
	ROM_LOAD( "im6_d.b6",  0x2000, 0x1000, CRC(257F4E0D) SHA1(CD61F3CF70C536AA207EBFDD28BE54AC586B5249) )

	ROM_REGION( 0x1000, "sound", 0 )
	ROM_LOAD( "sg1_b.e1", 0x0000, 0x1000, CRC(92EF3C13) SHA1(1BF1E4106B37AADFC02822184510740E18A54D5C) )

	ROM_REGION( 0x4000, "slave", 0 )
	ROM_LOAD( "im1_yk.g1", 0x0000, 0x1000, CRC(1921605D) SHA1(0AA6F7195EA59D0080620AB02A737E5C319DD3E7) )
	ROM_LOAD( "im2_yk.f1", 0x1000, 0x1000, CRC(8504314E) SHA1(309645E17FB3149DCE57AE6844CC58652A1EEB35) )
	ROM_LOAD( "im3_yk.e1", 0x2000, 0x1000, CRC(B0E473EC) SHA1(234598548B2A2A8F53D40BC07C3B1759074B7D93) )
	ROM_COPY( "sound", 0x0000, 0x3000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "s1.d1",    0x0000, 0x1000, CRC(26974007) SHA1(5079DAF9AD7D84F935C256458060DB9497DAEF91) )
	ROM_LOAD( "s2.e1",    0x1000, 0x1000, CRC(BEDEBFA7) SHA1(5A2116ED4AF6BC4B72199017515980E4A937236C) )
	ROM_LOAD( "s3.f1",    0x2000, 0x1000, CRC(0AA59571) SHA1(5005FFDD0030E4D4C1D8033FD3C78177C0FBD1B0) )
	ROM_LOAD( "s4.g1",    0x3000, 0x1000, CRC(CCD5FB0E) SHA1(3EE4377D15E7731586B7A3457DBAE52EDAED72D3) )
	ROM_LOAD( "s5.d2",    0x4000, 0x1000, CRC(32613DF3) SHA1(21CE057C416E6F1D0A3E112D640B1CF52BA69206) )
	ROM_LOAD( "s6.e2",    0x5000, 0x1000, CRC(A151C422) SHA1(354EFAEE64C8CC457F96CBA4722F6A0DF66E14D3) )
	ROM_LOAD( "s7.f2",    0x6000, 0x1000, CRC(7CFCEE55) SHA1(590AC02941E82371D56113D052EB4D4BCDBF83B0) )
	ROM_LOAD( "s8.g2",    0x7000, 0x1000, CRC(AD667C05) SHA1(D9BDF3A125EBA2D40191B0659C2007CCBC6FD12B) )

	/* These last 2 ROMs are a good distance away on the PCB */
	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "s9_a.k8",  0x1000, 0x1000, CRC(CA82CD81) SHA1(FDF47DF7705C8D0AE70B5A0E29B35819F3D0749A) )
	ROM_LOAD( "s10_a.l8", 0x0000, 0x1000, CRC(37B2736F) SHA1(15EF3F563AEBD1F5506135C7C01E9A1DB30A9CCC) )
ROM_END

/*    YEAR  NAME    PARENT  MACHINE  INPUT    INIT     MONITOR COMPANY     FULLNAME   FLAGS */
GAME( 1981, kingdrby,  0,      kingdrby,   kingdrby,   0,       ROT0,   "Tazmi",    "King Derby?",   GAME_NOT_WORKING | GAME_NO_SOUND )
