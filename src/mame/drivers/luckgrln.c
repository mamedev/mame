/*

 Lucky Girl (newer 1991 version on different hardware?)
  -- there is an early 'Lucky Girl' which appears to be running on Nichibutsu like hardware.

 The program rom extracted from the Z180 also refers to this as Lucky 74..

 TODO:
 - interrupt generation (nested NMIs? disabled for now)
 - video emulation (uses tilexor)

 Lucky Girl
 Wing 1991

 PCB labels

 "DREW'S UNDER WARRANTY IF REMOVED N3357"
 "No.A 120307 LICENSE WING SEAL"
 "EAGLE No.A 120307"

 ROMs

 1C - EAGLE 1       27C010 equiv. mask ROM
 1E - EAGLE 2       27C010 equiv. mask ROM
 1H - EAGLE 3       27C010 equiv. mask ROM
 1L - FALCON 4      27C010 equiv. mask ROM
 1N - FALCON 5      27C010 equiv. mask ROM
 1P - FALCON 6      27C010 equiv. mask ROM
 1T - FALCON 13     27C010 EPROM

 * ROMs 1-6 may need redumping, they could be half size.

 RAMs

 2x LH5186D-01L     28-pin PDIP     Video RAM and work RAM (battery backed)
 2x CY7C128A-25PC   24-pin PDIP     Probably color RAM

 Customs

 2x 06B53P          28-pin PDIP     Unknown
 1x 06B30P          40-pin PDIP     Unknown
 1x 101810P         64-pin SDIP     Unknown
 1x HG62E11B10P     64-pin SDIP     Hitachi gate array (custom)
 1x CPU module      90-pin SDIP

 Others

 1x HD6845SP        40-pin PDIP     CRTC
 1x MB3771           8-pin PDIP     Reset generator
 1x Oki M62X428     18-pin PDIP     Real-time clock

 CPU module

 Text on label:

 [2] 9015 1994.12
     LUCKY GIRL DREW'S

 Text underneath label

 WE 300B 1H2

 Looks like Hitachi part markings to me.

 Other notes

 Has some optocouplers, high voltage drivers, and what looks like additional
 I/O conenctors.

 Reset switch cuts power supply going to Video/Work RAM.


*/


#include "emu.h"
#include "cpu/z180/z180.h"
#include "video/mc6845.h"

static UINT8 *luck_vram1,*luck_vram2,*luck_vram3;
static UINT8 nmi_enable;

static VIDEO_START(luckgrln)
{

}

static VIDEO_UPDATE(luckgrln)
{
	int y,x;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			UINT16 tile = (luck_vram1[count] & 0xff);
			UINT16 enc_tile = (luck_vram2[count] & 0xff) | (luck_vram3[count] << 8);
			UINT16 dec_tile;
			UINT8 col = 0;
			UINT8 region = 0;
			//UINT8 region = (luck_vram2[count] & 0x20) >> 5;

			dec_tile = enc_tile & 0x0200 ? 0x10 : 0x00;
			dec_tile |= (enc_tile & 0x00f0)>>4;

			tile = tile|(dec_tile<<8);

			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[region],tile,col,0,0,x*8,y*8);

			count++;
		}
	}
	return 0;
}

static ADDRESS_MAP_START( mainmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x03fff) AM_ROM
	AM_RANGE(0x10000, 0x1ffff) AM_ROM AM_REGION("rom_data",0x10000)
	AM_RANGE(0x20000, 0x2ffff) AM_ROM AM_REGION("rom_data",0x00000)

	AM_RANGE(0x0c200, 0x0c5ff) AM_RAM
	AM_RANGE(0x0ca00, 0x0cdff) AM_RAM

	AM_RANGE(0x0d000, 0x0d03f) AM_RAM
	AM_RANGE(0x0d200, 0x0d2ff) AM_RAM
	AM_RANGE(0x0d400, 0x0d43f) AM_RAM

	AM_RANGE(0x0d800, 0x0dfff) AM_RAM

	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM AM_BASE(&luck_vram1)
	AM_RANGE(0x0e800, 0x0efff) AM_RAM AM_BASE(&luck_vram2)
	AM_RANGE(0x0f000, 0x0f7ff) AM_RAM AM_BASE(&luck_vram3)

	/* patch NMI checks until I understand. */
	AM_RANGE(0x0faff, 0x0faff) AM_NOP
	AM_RANGE(0x0fb02, 0x0fb02) AM_NOP

	AM_RANGE(0x0f800, 0x0ffff) AM_RAM
	AM_RANGE(0xf0000, 0xfffff) AM_RAM
ADDRESS_MAP_END

static WRITE8_HANDLER( output_w )
{
	/* correct? */
	nmi_enable = data & 1;
	// other values unknown
//  printf("%02x\n",data);
}

static READ8_HANDLER( test_r )
{
	return mame_rand(space->machine);
}

static ADDRESS_MAP_START( portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff) // i think
	AM_RANGE(0x0000, 0x003f) AM_RAM // Z180 internal regs
	AM_RANGE(0x0060, 0x0060) AM_WRITE(output_w)
	AM_RANGE(0x0090, 0x009d) AM_READ(test_r)
	AM_RANGE(0x00b0, 0x00b0) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x00b1, 0x00b1) AM_DEVWRITE("crtc", mc6845_register_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( luckgrln )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(0,6),RGN_FRAC(1,6),RGN_FRAC(2,6),RGN_FRAC(3,6),RGN_FRAC(4,6),RGN_FRAC(5,6) },
	{ STEP8(0,1) },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8},
	8*8
};
static GFXDECODE_START( luckgrln )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};

static INTERRUPT_GEN( luckgrln_irq )
{
	#if 0
	if(nmi_enable)
		cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
	#endif
}

static MACHINE_DRIVER_START( luckgrln )
	MDRV_CPU_ADD("maincpu", Z180,8000000)
	MDRV_CPU_PROGRAM_MAP(mainmap)
	MDRV_CPU_IO_MAP(portmap)
	MDRV_CPU_VBLANK_INT("screen", luckgrln_irq)

	MDRV_MC6845_ADD("crtc", H46505, 6000000/4, mc6845_intf)	/* unknown clock, hand tuned to get ~60 fps */

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_GFXDECODE(luckgrln)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(luckgrln)
	MDRV_VIDEO_UPDATE(luckgrln)
MACHINE_DRIVER_END

static DRIVER_INIT( luckgrln )
{
	int i;
	UINT8 x,v;
	UINT8* rom = memory_region(machine,"rom_data");

	for (i=0;i<0x20000;i++)
	{
		x = rom[i];
		v = 0xfe + (i & 0xf)*0x3b + ((i >> 4) & 0xf)*0x9c + ((i >> 8) & 0xf)*0xe1 + ((i >> 12) & 0x7)*0x10;
		v += ((((i >> 4) & 0xf) + ((i >> 2) & 3)) >> 2) * 0x50;
		x ^= ~v;
		x = (x << (i & 7)) | (x >> (8-(i & 7)));
		rom[i] = x;
	}

	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(rom, 0x20000, 1, fp);
			fclose(fp);
		}
	}
	#endif

	// ??
//  memory_set_bankptr(machine, "bank1",&rom[0x010000]);
}

ROM_START( luckgrln )
	ROM_REGION( 0x4000, "maincpu", 0 ) // internal Z180 rom
	ROM_LOAD( "lucky74.bin",  0x00000, 0x4000, CRC(fa128e05) SHA1(97a9534b8414f984159271db48b153b0724d22f9) )

	ROM_REGION( 0x20000, "rom_data", 0 ) // external data / cpu rom
	ROM_LOAD( "falcon.13",  0x00000, 0x20000, CRC(f7a717fd) SHA1(49a39b84620876ee2faf73aaa405a1e17cab2da2) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "eagle.1", 0x00000, 0x20000, CRC(37209082) SHA1(ffb30da5920886f37c6b97e03f5a8ec3b6265e68) )
	ROM_LOAD( "eagle.2", 0x20000, 0x20000, CRC(bdb2d694) SHA1(3e58fe3f6b447181e3a85f0fc2a0c996231bc8e8) )
	ROM_LOAD( "eagle.3", 0x40000, 0x20000, CRC(2c765389) SHA1(d5697c73cc939aa46f36c2dd87e90bba2536e347))

	ROM_REGION( 0x60000, "gfx2", 0 ) // luckgrlns - 3bpp
	ROM_LOAD( "falcon.4", 0x00000, 0x20000, CRC(369eaddf) SHA1(52387ea63e5c8fb0c27b796026152a06b68467af) )
	ROM_LOAD( "falcon.5", 0x20000, 0x20000, CRC(c9ac1fe7) SHA1(fc027002754b90cc49ca74fac5240a99a194c0b3))
	ROM_LOAD( "falcon.6", 0x40000, 0x20000, CRC(bfb02c87) SHA1(1b5ca562ed76eb3f1b4a52d379a6af07e79b6ee5))
ROM_END

GAME( 1991, luckgrln,  0,    luckgrln, luckgrln,  luckgrln, ROT0, "Wing Co., Ltd.", "Lucky Girl (newer Z180 based hardware)", GAME_NOT_WORKING|GAME_NO_SOUND )

