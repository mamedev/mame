/*
Neo Print
doesn't seem to have anything in common with a standard NeoGeo apart from how the carts look.


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"

static UINT16* npvidram;

static READ16_HANDLER( np_rnd_read )
{
	return 0xffff;// mame_rand(space->machine);
}

static READ16_HANDLER( np_rnd_read2 )
{
	return 0x0000;// mame_rand(space->machine);
}

static READ16_HANDLER( np_rnd_read3 )
{
	if(input_code_pressed(space->machine, KEYCODE_Z))
		return 0x0000;

	return 0x8000;// mame_rand(space->machine);
}

static READ16_HANDLER( np_rnd_read4 )
{
	return 0x0100;// mame_rand(space->machine);
}

static ADDRESS_MAP_START( neoprint_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
/*  AM_RANGE(0x100000, 0x17ffff) multi-cart or banking, some writes points here if anything lies there */
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x300000, 0x30ffff) AM_RAM
	AM_RANGE(0x400000, 0x43ffff) AM_RAM AM_BASE(&npvidram)
	AM_RANGE(0x500000, 0x51ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x600000, 0x600001) AM_READ(np_rnd_read4)
	AM_RANGE(0x600002, 0x600003) AM_READ(np_rnd_read3) AM_WRITENOP
	AM_RANGE(0x600006, 0x600007) AM_READ(np_rnd_read) // input
	AM_RANGE(0x600008, 0x600009) AM_READ(np_rnd_read) // input
	AM_RANGE(0x60000a, 0x60000b) AM_READ(np_rnd_read2)
	AM_RANGE(0x60000c, 0x60000d) AM_READ(np_rnd_read) // input

	AM_RANGE(0x70001e, 0x70001f) AM_WRITENOP //watchdog
ADDRESS_MAP_END

static INPUT_PORTS_START( neoprint )
	PORT_START("P1_P2")
INPUT_PORTS_END


static const gfx_layout neoprint_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, RGN_FRAC(0,2)+8, RGN_FRAC(0,2)+0 },
	{ 0,1,2,3,4,5,6,7, 256,257,258,259,260,261,262,263 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	32*16,
};

static GFXDECODE_START( neoprint )
	GFXDECODE_ENTRY( "gfx1", 0, neoprint_layout,   0x0, 0x1000 )
GFXDECODE_END

VIDEO_START(neoprint)
{

}

VIDEO_UPDATE(neoprint)
{
//	static UINT32 test = 0;
	bitmap_fill(bitmap, cliprect, 0);

	{
		int i, y, x;
		const gfx_element *gfx = screen->machine->gfx[0];

		i = 0x400; // FIXME: map base register

		for (y=0;y<32;y++)
		{
			for (x=0;x<32;x++)
			{
				UINT16 dat = npvidram[i*2]>>2;

				drawgfx_transpen(bitmap,cliprect,gfx,dat,0,0,0,x*16,y*16,0);


				i++;
			}

		}

	}

	return 0;
}

class neoprint_state : public driver_device
{
public:
	neoprint_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }
};


static MACHINE_CONFIG_START( neoprint, neoprint_state )

	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(neoprint_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold) // lv1,2,3 valid?

	MDRV_GFXDECODE(neoprint)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x10000)

	MDRV_VIDEO_START(neoprint)
	MDRV_VIDEO_UPDATE(neoprint)
MACHINE_CONFIG_END


ROM_START( npcartv1 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ep1.bin", 0x000000, 0x80000, CRC(18606198) SHA1(d968e09131c22769e22c7310aca1f02e739f38f1) )
//	ROM_RELOAD(						 0x100000, 0x80000 ) /* checks the same string from above to be present there? Why? */

	ROM_REGION( 0x20000, "z80", 0 ) /* Z80 program */
	ROM_LOAD( "m1.bin", 0x00000, 0x20000, CRC(b2d38e12) SHA1(ab96c5d3d22eb71ed6e0a03f3ff5d4b23e72fad8) )

	ROM_REGION( 0x080000, "snd", 0 ) /* Samples */
	ROM_LOAD( "v1.bin", 0x00000, 0x80000, CRC(2d6608f9) SHA1(7dbde1c305ab3438b7fe7417816427c682371bd4) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c1.bin", 0x00000, 0x80000, CRC(b89f1fb6) SHA1(e711f91a7872b2e0edc3f42a726d969096d684f2) )
	ROM_LOAD( "c2.bin", 0x80000, 0x80000, CRC(7ce39dc2) SHA1(c5be90657350258b670b55dd9c77f7899133ced3) )
ROM_END

	/* logo: Neo Print
    small text: Cassette supporting Neo Print and Neo Print Multi
    (cassette=cartridge)
    title: '98 NeoPri Best 44 version */

ROM_START( 98best44 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "p060-ep1", 0x000000, 0x080000, CRC(d42e505d) SHA1(0ad6b0288f36c339832730a03e53cbc07dab4f82))
//	ROM_RELOAD(						 0x100000, 0x80000 ) /* checks the same string from above to be present there? Why? */

	ROM_REGION( 0x20000, "z80", 0 ) /* Z80 program */
	ROM_LOAD( "pt004-m1",	 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) )

	ROM_REGION( 0x200000, "snd", 0 ) /* Samples */
	ROM_LOAD( "pt004-v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "pt060-c1", 0x000000, 0x100000, CRC(22a23090) SHA1(0e219fcfea6ca2ddf4b7b4197aac8bc55a29d5cf) )
	ROM_LOAD( "pt060-c2", 0x100000, 0x100000, CRC(66a8e56a) SHA1(adfd1e52d52806a785f1e9b1ae2ac969b6ed60af) )
ROM_END


GAME( 199?, npcartv1,    0,        neoprint,    neoprint,   0, ROT0, "SNK", "Neo Print V1", GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 1998, 98best44,    0,        neoprint,    neoprint,   0, ROT0, "SNK", "'98 NeoPri Best 44 (Japan)", GAME_NO_SOUND | GAME_NOT_WORKING )
