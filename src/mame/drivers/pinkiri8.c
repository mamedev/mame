/***************************************************************************

Pinkiri 8 skeleton driver

- current blocker is the video emulation i/o ports, it looks somewhat exotic.

============================================================================
Janshi
(c)1992 Eagle

CPU: HD647180X0P6 (16K EPROM internal rom)
Sound: AY-3-8910, M6295
Others: Battery
OSC: 32MHz, 21MHz & 12.2880MHz

ROMs:
1.1A         [92b140a5]
2.1B         [6de7e086]
3.1D         [4e94d8f2]
4.1F         [a5f6e3ef]
5.1H         [ff2cc769]
6.1K         [8197034d]
11.1L        [a7692ddf]



--- Team Japump!!! ---
Dumped by Chackn
04/May/2007

***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"

static UINT32 vram_addr;
static UINT8 vram_bank;

static VIDEO_START( pinkiri8 )
{

}

static VIDEO_UPDATE( pinkiri8 )
{
	static UINT8 *vram = memory_region(screen->machine, "vram");
	const gfx_element *gfx = screen->machine->gfx[0];

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	{
		UINT8 x,y;
		UINT16 spr_offs,i;

		for(i=0x00000;i<0x4000;i+=4)
		{
			spr_offs = ((vram[i+0+0x10000] & 0xff) | (vram[i+1+0x10000]<<16)) & 0x0fff;
			y = vram[i+2+0x10000];
			x = vram[i+3+0x10000];

			drawgfx_transpen(bitmap,cliprect,gfx,0,  0,0,0,x+0,y,0);
//			drawgfx_transpen(bitmap,cliprect,gfx,spr_offs+1,0,0,0,x+8,y,0);
//			drawgfx_transpen(bitmap,cliprect,gfx,spr_offs+2,0,0,0,x+16,y,0);
//			drawgfx_transpen(bitmap,cliprect,gfx,spr_offs+3,0,0,0,x+24,y,0);

		}
	}

	return 0;
}

static ADDRESS_MAP_START( pinkiri8_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0bfff) AM_ROM
	AM_RANGE(0x0c000, 0x0dfff) AM_RAM
	AM_RANGE(0x10000, 0x1ffff) AM_ROM
ADDRESS_MAP_END

static READ8_HANDLER( unk_r )
{
	return mame_rand(space->machine);
}

static READ8_HANDLER( unk2_r )
{
	return (mame_rand(space->machine) & 0xfe)| 1;
}

static WRITE8_HANDLER( output_regs_w )
{
	if(data & 0x40)
		cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_NMI, CLEAR_LINE);
	//data & 0x80 is probably NMI mask
}

static WRITE8_HANDLER( pinkiri8_vram_w )
{
	static UINT8 *vram = memory_region(space->machine, "vram");

	switch(offset)
	{
		case 0: vram_addr = (data & 0xff) | (vram_addr & 0xff00); break;
		case 1: vram_addr = (data << 8) | (vram_addr & 0x00ff); break;
		case 2:
			vram_bank = ((data ^ 0x06) & 0x06)>>1; //unknown purpose
			//printf("%02x\n",vram_bank);
			break;
		case 3:
			vram_addr++;
			vram_addr&=0xffff;
			vram[(vram_addr) | (vram_bank << 16)] = data;
			if(vram_addr <= 0xffff)
			{
				static UINT16 datax,pal_offs;
				static UINT8 r,g,b;

				pal_offs = vram_addr;

				datax = (vram[pal_offs & 0x1fff]) + (vram[(pal_offs & 0x1fff) | (0x2000)]<<8);

				r = ((datax)&0x001f)>>0;
				g = ((datax)&0x03e0)>>5;
				b = ((datax)&0x7c00)>>10;

				palette_set_color_rgb(space->machine, pal_offs & 0x1fff, pal5bit(r), pal5bit(g), pal5bit(b));
			}
			break;
	}
}

static ADDRESS_MAP_START( pinkiri8_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_RAM //Z180 internal I/O
	AM_RANGE(0x60, 0x60) AM_WRITE(output_regs_w)
	AM_RANGE(0x80, 0x83) AM_WRITE(pinkiri8_vram_w)
	AM_RANGE(0xb0, 0xb0) AM_WRITENOP //mux
	AM_RANGE(0xb0, 0xb1) AM_READ(unk_r) // mux inputs
	AM_RANGE(0xb2, 0xb2) AM_READ(unk2_r) //bit 0 causes a reset inside the NMI routine
	AM_RANGE(0xf8, 0xf8) AM_READ(unk_r) //test bit 0
	AM_RANGE(0xf9, 0xf9) AM_READ(unk_r)
	AM_RANGE(0xfa, 0xfa) AM_READ(unk_r) //test bit 7
	AM_RANGE(0xfb, 0xfb) AM_READ(unk_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( pinkiri8 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5),RGN_FRAC(3,5),RGN_FRAC(2,5),RGN_FRAC(1,5),RGN_FRAC(0,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( pinkiri8 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 0x100 )
GFXDECODE_END

static MACHINE_DRIVER_START( pinkiri8 )
	MDRV_CPU_ADD("maincpu",Z180,16000000)
	MDRV_CPU_PROGRAM_MAP(pinkiri8_map)
	MDRV_CPU_IO_MAP(pinkiri8_io)
	MDRV_CPU_VBLANK_INT("screen",nmi_line_assert)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(pinkiri8)
	MDRV_PALETTE_LENGTH(0x2000)

	MDRV_VIDEO_START(pinkiri8)
	MDRV_VIDEO_UPDATE(pinkiri8)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
//  MDRV_SOUND_ADD("aysnd", AY8910, 8000000/4 /* guess */)
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pinkiri8 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pinkiri8-ver.1.02.l1",   0x0000, 0x20000, CRC(f2df5b12) SHA1(e374e184a6a1e932550516011ec09a5accec9b03) )
	ROM_LOAD( "bios.rom", 0x0000, 0x4000, CRC(399df1ee) SHA1(8251f3aa7da4c7899c8e739c10b61260f4471311) ) //overlapped internal ROM

	ROM_REGION( 0x20000*5, "gfx1", 0 )
	ROM_LOAD( "pinkiri8-chr-01.a1",  0x00000, 0x20000, CRC(8ec73662) SHA1(9098348e519ce753dd7f38f0d855181bfc65aa42) )
	ROM_LOAD( "pinkiri8-chr-02.bc1", 0x20000, 0x20000, CRC(8dc20a65) SHA1(4062510fe06e8844a732754b7915a3b67ba2a3c5) )
	ROM_LOAD( "pinkiri8-chr-03.d1",  0x40000, 0x20000, CRC(bd5f269a) SHA1(7dfd039227551f0f0ed4afaafc76ca64a39a9b83) )
	ROM_LOAD( "pinkiri8-chr-04.ef1", 0x60000, 0x20000, CRC(4d0e5005) SHA1(4b90119c359c4de576131fd0e28d2fe1482ce74f) )
	ROM_LOAD( "pinkiri8-chr-05.h1",  0x80000, 0x20000, CRC(036ca165) SHA1(c4a2d6e394bbabcae1413d8a2916a19c90687edf) )

	ROM_REGION( 0x80000, "vram", ROMREGION_ERASE00)
ROM_END


ROM_START( janshi )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "11.1l",    0x00000, 0x20000, CRC(a7692ddf) SHA1(5e7f43d8337583977baf22a28bbcd9b2182c0cde) )
//	ROM_LOAD( "9009 1996.08 ron jan.bin", 0x00000, 0x4000, CRC(4eb74322) SHA1(84f864c0da3fb69948f6eb7ffecf0e722a882efc) ) //this is from an undumped ROMset
	ROM_LOAD( "[3] 9009 1992.1 new jansh.bin", 0x0000, 0x4000, CRC(63cd3f12) SHA1(aebac739bffaf043e6acffa978e935f73ee1385f) ) //overlapped internal ROM

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "1.1a", 0x000000, 0x40000, CRC(92b140a5) SHA1(f3b38563f74650604ed0faaf84460e0b04b386b7) )
	ROM_LOAD( "2.1b", 0x040000, 0x40000, CRC(6de7e086) SHA1(e87426264f0181c17383ffe0f7ec7ff5fce3d809) )
	ROM_LOAD( "3.1d", 0x080000, 0x40000, CRC(4e94d8f2) SHA1(a25f542943d74915fc82910baafb9ff9db1ffd70) )
	ROM_LOAD( "4.1f", 0x0c0000, 0x40000, CRC(a5f6e3ef) SHA1(f1f3d28b27eea682aa71855a311fb3abdf9af2cd) )
	ROM_LOAD( "5.1h", 0x100000, 0x40000, CRC(ff2cc769) SHA1(ba4cf2923cf3d4d815a9327595f8e1801c3c8a2b) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "6.1k", 0x00000, 0x40000, CRC(8197034d) SHA1(b501dc7a27b1faad1361c309afd726da14b8b5f5) )

	ROM_REGION( 0x80000, "vram", ROMREGION_ERASE00)

ROM_END

GAME( 2005?, pinkiri8,  0,   pinkiri8, pinkiri8,  0, ROT0, "<unknown>", "Pinkiri 8", GAME_NOT_WORKING| GAME_NO_SOUND )
GAME( 1992,  janshi,    0,   pinkiri8, pinkiri8,  0, ROT0, "Eagle", "Janshi", GAME_NOT_WORKING | GAME_NO_SOUND )
