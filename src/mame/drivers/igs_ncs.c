/* New Champion Skill by IGS
 -- the dump MAY be incomplete, there were 3 empty positions on the PCB near
    the gfx roms

Chips of Note

IGS 003C (near chip with TEST OK E0069281 label)
IGS 002
IGA 001A


'file'
KC8255A
9941
(near CPU roms)

UM3567 9946

5x 8 switch dips

Clocks
3.579545Mhz (near sound)
12Mhz


--- what is the CPU, it looks like either Z80 or Z180 based
 -- CPU rom is lightly encrypted (usual IGS style, some xors)

*/

#include "driver.h"
#include "cpu/z180/z180.h"
#include "sound/2413intf.h"
#include "deprecat.h"

static UINT8*ncs_video;
static UINT8*ncs_video2;

VIDEO_START(igs_ncs)
{

}

VIDEO_UPDATE(igs_ncs)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0;

	int y,x;


	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = ncs_video[count] | (ncs_video[count+0x800]<<8);
			int colour = (tile >> 12) & 0x7;

			drawgfx(bitmap,gfx,tile&0x1fff,colour+1,0,0,x*8,y*8,cliprect,TRANSPARENCY_NONE,0);

			count++;
		}
	}


	return 0;
}

static const gfx_layout layout_8x8x6 =
{
	8, 8,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2
};

static GFXDECODE_START( igs_ncs )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x6,  0, 16 )
GFXDECODE_END

static INPUT_PORTS_START( igs_ncs )
INPUT_PORTS_END

static ADDRESS_MAP_START( igs_ncs_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x00000, 0x0efff ) AM_ROM
	AM_RANGE( 0x0f000, 0x0ffff ) AM_RAM
ADDRESS_MAP_END

static READ8_HANDLER( ncs_random_r )
{
	return 0xff;// mame_rand(space->machine);
}

static ADDRESS_MAP_START( igs_ncs_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x2000, 0x23ff ) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split1_w ) AM_BASE( &paletteram )
	AM_RANGE( 0x2800, 0x2cff ) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split2_w ) AM_BASE( &paletteram_2 )


	AM_RANGE( 0x4000, 0x4000 ) AM_READ(ncs_random_r)
	AM_RANGE( 0x4001, 0x4001 ) AM_READ(ncs_random_r)
	AM_RANGE( 0x4002, 0x4002 ) AM_READ(ncs_random_r)
	AM_RANGE( 0x4003, 0x4003 ) AM_READ(ncs_random_r)
	AM_RANGE( 0x4004, 0x4004 ) AM_READ(ncs_random_r)


	AM_RANGE( 0x5081, 0x5081 ) AM_READ(ncs_random_r)
	AM_RANGE( 0x5082, 0x5082 ) AM_READ(ncs_random_r)

	AM_RANGE( 0x5082, 0x5091 ) AM_READ(ncs_random_r)

	AM_RANGE( 0x50a0, 0x50a0 ) AM_READ(ncs_random_r)
	AM_RANGE( 0x50c0, 0x50c0 ) AM_READ(ncs_random_r)

	AM_RANGE( 0x6000, 0x6fff ) AM_RAM AM_BASE(&ncs_video2) // 'reels' layer -- we're missing the gfx

	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE(&ncs_video)


	AM_RANGE( 0x8000, 0xffff ) AM_ROM AM_REGION("data", 0)

ADDRESS_MAP_END

static INTERRUPT_GEN( igs_ncs_interrupt )
{
	// wrong!!
	if (cpu_getiloops(device)&1)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
	else
		cpu_set_input_line(device, INPUT_LINE_IRQ0, HOLD_LINE);
}


static MACHINE_DRIVER_START( igs_ncs )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z180, XTAL_12MHz / 2)	/* CPU is..?? 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(igs_ncs_map,0)
	MDRV_CPU_IO_MAP(igs_ncs_portmap,0)
	MDRV_CPU_VBLANK_INT_HACK(igs_ncs_interrupt,2)
//	MDRV_CPU_VBLANK_INT("main",igs_ncs_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_GFXDECODE(igs_ncs)
	MDRV_PALETTE_LENGTH(0x400)

	MDRV_VIDEO_START(igs_ncs)
	MDRV_VIDEO_UPDATE(igs_ncs)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ym", YM2413, XTAL_3_579545MHz)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END



ROM_START( igs_ncs )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ncs_v100n.u20", 0x00000, 0x10000, CRC(2bb91de5) SHA1(b0b7b3b9cee1ce4da10cf78ef1c8079f3d9cafbf) )

	ROM_REGION( 0x10000, "data", 0 )
	ROM_LOAD( "ncs_v100n.u21", 0x00000, 0x10000, CRC(678e412c) SHA1(dba031d3576d098d314d6589dd1aeda44d17c650) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ncs_v100n.u50", 0x00000, 0x40000, CRC(ff2bb3dc) SHA1(364c948504003b4230fbdac74227842c802d4c12) )
	ROM_LOAD( "ncs_v100n.u51", 0x40000, 0x40000, CRC(f8530313) SHA1(b21d6de7d5d4b902008ceea7e1227545e0d1701b) )
	ROM_LOAD( "ncs_v100n.u52", 0x80000, 0x40000, CRC(2fa5b6df) SHA1(5bfc651297440f73692079f1806b1e40b457b7b8) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASEFF | ROMREGION_DISPOSE )
	// looks like these are needed for pre-game screens, sockets were empty
	ROM_LOAD( "ncs_v100n.u55", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "ncs_v100n.u56", 0x10000, 0x10000, NO_DUMP )
	ROM_LOAD( "ncs_v100n.u57", 0x20000, 0x10000, NO_DUMP )
ROM_END


DRIVER_INIT( igs_ncs )
{
	UINT8 *src = (UINT8 *) (memory_region(machine, "main"));
	int i;

	for(i = 0; i < 0x10000; i++)
	{
		/* bit 0 xor layer */
		if(i & 0x200)
		{
			if(i & 0x80)
			{
				if(~i & 0x02)
				{
					src[i] ^= 0x01;
				}
			}
			else
			{
				src[i] ^= 0x01;
			}
		}
		else
		{
			src[i] ^= 0x01;
		}

		/* bit 1 xor layer */
		if(i & 0x800)
		{
			if(i & 0x100)
			{
				if(i & 0x40)
				{
					src[i] ^= 0x02;
				}
			}
		}

		/* bit 5 xor layer */
		if(i & 0x100)
		{
			if(i & 0x40)
			{
				src[i] ^= 0x20;
			}
		}
		else
		{
			src[i] ^= 0x20;
		}
	}

}

GAME( 2000, igs_ncs, 0, igs_ncs, igs_ncs, igs_ncs, ROT0, "IGS", "New Champion Skill (v100n)", GAME_NOT_WORKING )

