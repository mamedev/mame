/***************************************************************************

Hit Poker (c) 1997 Accept LTD

preliminary driver by Angelo Salese & David Haywood
Many thanks to Olivier Galibert for the CPU identify effort ;-)

TODO:
- Fix the CPU core bugs! (too many to list)
- video HW looks awkward;
- sound (I've heard something reasonable during tests, so it could be OK);

============================================================================

'Hit Poker'?

cpu hd46505SP (HD6845SP) <- ha, ha, ha... --"

other : ZC407615CFN (infralink)

chrystal : no idea

ram km6264BL X3
TMM 2018 X2
DALLAS REAL TIME CLK DS17487-5
SOUND YM2149F
DIP 1X4

============================================================================

Some debug tricks (let's test this CPU as more as possible):
- let it run then soft reset (it wants that ram at 0-0xff is equal to 0xff); *
- set a bp at 10c5 then pc=10c8, it currently fails the rom checksum *
- set a bp at 1185, the "bad crc 000" msg is caused by this routine. (DONE)
- set a bp at 121f then pc=1223
- set a bp at 3a50 then pc+=2, it should now enter into attract mode
  (at last!)

* If you disable the MC68HC11 internal ram / I/O regs it'll pass the ROM
  checksum, maybe it isn't a MC68HC11 but something without the I/O stuff?

***************************************************************************/


#include "driver.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/ay8910.h"

static UINT8 *hitpoker_sys_regs;
static UINT8 hitpoker_pic_data;

VIDEO_START(hitpoker)
{
	videoram = auto_alloc_array(machine, UINT8, 0x35ff);
	paletteram = auto_alloc_array(machine, UINT8, 0x1000);
	colorram = auto_alloc_array(machine, UINT8, 0x2000);

}

VIDEO_UPDATE(hitpoker)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0;
	int y,x;

	bitmap_fill(bitmap, cliprect, 0);

	for (y=0;y<30;y++)
	{
		count+=2;
		for (x=1;x<81;x++) //it's probably 80 + 1 global line attribute at the start of each line
		{
			int tile,color;

			tile = (((videoram[count]<<8)|(videoram[count+1])) & 0x3fff);
			color = (((colorram[count]<<8)|(colorram[count+1])) & 0xe000)>>13;
			drawgfx(bitmap,gfx,tile,color,0,0,(x-1)*8,(y+0)*8,cliprect,TRANSPARENCY_NONE,0);

			count+=2;
		}
	}

	return 0;
}

static READ8_HANDLER( hitpoker_vram_r )
{
	UINT8 *ROM = memory_region(space->machine, "maincpu");

	if(hitpoker_pic_data & 0x10)
		return videoram[offset];
	else
		return ROM[offset+0x8000];
}

static WRITE8_HANDLER( hitpoker_vram_w )
{
//	UINT8 *ROM = memory_region(space->machine, "maincpu");

//	if(hitpoker_sys_regs[0x00] & 0x10)
	videoram[offset] = data;
}

static READ8_HANDLER( hitpoker_cram_r )
{
	UINT8 *ROM = memory_region(space->machine, "maincpu");

	if(hitpoker_pic_data & 0x10)
		return colorram[offset];
	else
		return ROM[offset+0xc000];
}

static WRITE8_HANDLER( hitpoker_cram_w )
{
	colorram[offset] = data;
}

static READ8_HANDLER( hitpoker_paletteram_r )
{
	UINT8 *ROM = memory_region(space->machine, "maincpu");

	if(hitpoker_pic_data & 0x10)
		return paletteram[offset];
	else
		return ROM[offset+0xe000];
}

static WRITE8_HANDLER( hitpoker_paletteram_w )
{
	int r,g,b,datax;
	paletteram[offset] = data;
	offset>>=1;
	datax=256*paletteram[offset*2]+paletteram[offset*2+1];

	/* TODO: format is wrong */
	b = ((datax)&0xf800)>>11;
	g = ((datax)&0x07e0)>>5;
	r = ((datax)&0x001f)>>0;

	palette_set_color_rgb(space->machine, offset, pal5bit(r), pal6bit(g), pal5bit(b));
}

static WRITE8_HANDLER( hitpoker_crtc_w )
{
	static UINT8 address;

	if(offset == 0)
		address = data;
	else
	{
		switch(address)
		{
			default:
				logerror("Video Register %02x called with %02x data\n",address,data);
		}
	}
}

static READ8_HANDLER( rtc_r )
{
	return 0x80; //kludge it for now
}

static UINT8 eeprom_data[0x200];
static UINT16 eeprom_index;

/* tests 0x180, what EEPROM is this one??? */
static WRITE8_HANDLER( eeprom_w )
{
	if(offset == 0)
	{
		eeprom_index = (eeprom_index & 0x100)|(data & 0xff);
		printf("W INDEX %02x\n",data);
	}
	if(offset == 1)
	{
		eeprom_index = (eeprom_index & 0xff)|((data & 0x1)<<8);
		//data & 0x4: eeprom clock
		printf("W CLOCK + INDEX %02x\n",data);
	}
}

static READ8_HANDLER( eeprom_r )
{
	static UINT8 tmp;
	tmp = eeprom_data[eeprom_index];
	if((eeprom_index & 0x1f) == 0x1f)
		tmp = 0xaa;
	printf("%02x\n",eeprom_index);
	eeprom_index++;
	//eeprom_index&=0x1f;
	return tmp;
}

static READ8_HANDLER( hitpoker_pic_r )
{
//	logerror("R\n");

	if(cpu_get_pc(space->cpu) == 0x3143 ||
	   cpu_get_pc(space->cpu) == 0x314e ||
	   cpu_get_pc(space->cpu) == 0x3164 ||
	   cpu_get_pc(space->cpu) == 0x3179)
		return hitpoker_pic_data;

	return (hitpoker_pic_data & 0x7f) | (hitpoker_pic_data & 0x40 ? 0x80 : 0x00);
}

static WRITE8_HANDLER( hitpoker_pic_w )
{
	hitpoker_pic_data = (data & 0xff);// | (data & 0x40) ? 0x80 : 0x00;
//	logerror("%02x W\n",data);
}

#if 0
static READ8_HANDLER( test_r )
{
	return mame_rand(space->machine);
}
#endif

/* overlap empty rom addresses */
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_RAM // stack ram
	AM_RANGE(0x1000, 0x1000) AM_READWRITE(hitpoker_pic_r,hitpoker_pic_w) // protection
	AM_RANGE(0x1000, 0x103f) AM_RAM AM_BASE(&hitpoker_sys_regs) // hw regs?
	AM_RANGE(0x8000, 0xb5ff) AM_READWRITE(hitpoker_vram_r,hitpoker_vram_w)
	AM_RANGE(0xb600, 0xbdff) AM_RAM
	AM_RANGE(0xbe0c, 0xbe0c) AM_READNOP //irq ack?
	AM_RANGE(0xbe0d, 0xbe0d) AM_READ(rtc_r)
	AM_RANGE(0xbe50, 0xbe51) AM_WRITE(eeprom_w)
	AM_RANGE(0xbe53, 0xbe53) AM_READ(eeprom_r)
	AM_RANGE(0xbe80, 0xbe81) AM_WRITE(hitpoker_crtc_w)
	AM_RANGE(0xbe90, 0xbe91) AM_DEVREADWRITE("ay", ay8910_r,ay8910_address_data_w)
	AM_RANGE(0xbea0, 0xbea0) AM_READ_PORT("VBLANK") //probably other bits as well
//	AM_RANGE(0xbe00, 0xbeff) AM_READ(test_r)
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(hitpoker_cram_r,hitpoker_cram_w)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(hitpoker_paletteram_r,hitpoker_paletteram_w)
	AM_RANGE(0x0000, 0xbdff) AM_ROM
	AM_RANGE(0xbf00, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io, ADDRESS_SPACE_IO, 8 )
ADDRESS_MAP_END

static INPUT_PORTS_START( hitpoker )
	PORT_START("VBLANK")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) //scanline counter probably
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END



static const gfx_layout hitpoker_layout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+12,0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ 0*32, 1*32, 2*32, 3*32,4*32,5*32,6*32,7*32 },
	8*32
};

static GFXDECODE_START( hitpoker )
	GFXDECODE_ENTRY( "gfx1", 0, hitpoker_layout,   0, 8  )
GFXDECODE_END

static MACHINE_DRIVER_START( hitpoker )
	MDRV_CPU_ADD("maincpu", MC68HC11,1000000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 224-1)

	MDRV_GFXDECODE(hitpoker)
	MDRV_PALETTE_LENGTH(0x3000)

	MDRV_VIDEO_START(hitpoker)
	MDRV_VIDEO_UPDATE(hitpoker)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 1500000)
//	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

DRIVER_INIT(hitpoker)
{
	UINT8 *ROM = memory_region(machine, "maincpu");

	ROM[0x10c6] = 0x01;
	ROM[0x10c7] = 0x01; //patch the checksum routine for now...

	#if 0
	ROM[0x1128] = 0x01;
	ROM[0x1129] = 0x01;
	ROM[0x112a] = 0x01;

	ROM[0x1143] = 0x01;
	ROM[0x1144] = 0x01;
	ROM[0x1145] = 0x01;

	ROM[0x1152] = 0x01;
	ROM[0x1153] = 0x01;
	ROM[0x1154] = 0x01;

	ROM[0x115e] = 0x01;
	ROM[0x115f] = 0x01;
	ROM[0x1160] = 0x01;

	ROM[0x1167] = 0x01;
	ROM[0x1168] = 0x01;
	ROM[0x1169] = 0x01;

	ROM[0x1170] = 0x01;
	ROM[0x1171] = 0x01;
	ROM[0x1172] = 0x01;
	#endif
}

ROM_START( hitpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u4.bin",         0x00000, 0x10000, CRC(0016497a) SHA1(017320bfe05fea8a48e26a66c0412415846cee7c) )

	ROM_REGION( 0x10000, "pic", 0 )
	ROM_LOAD( "pic",			0x00000, 0x1000, NO_DUMP ) // unknown type

	ROM_REGION( 0x100000, "gfx1", 0 ) // tile 0x4c8 seems to contain something non-gfx related, could be tilemap / colour data, check!
	ROM_LOAD16_BYTE( "u42.bin",         0x00001, 0x40000, CRC(cbe56fec) SHA1(129bfd10243eaa7fb6a087f96de90228e6030353) )
	ROM_LOAD16_BYTE( "u43.bin",         0x00000, 0x40000, CRC(6c0d4283) SHA1(04a4fd82f5cc0ed9f548e490ac67d287227073c3) )
	ROM_LOAD16_BYTE( "u44.bin",         0x80001, 0x40000, CRC(e23d5f30) SHA1(ca8855301528aa4eeff40cb820943b4268f8596e) ) // the 'adult images' are 8bpp
	ROM_LOAD16_BYTE( "u45.bin",         0x80000, 0x40000, CRC(e65b3e52) SHA1(c0c1a360a4a1823bf71c0a4105ff41f4102862e8) ) //  the first part of these 2 is almost empty as the standard gfx are 4bpp
ROM_END

GAME( 1997, hitpoker,  0,    hitpoker, hitpoker,  hitpoker, ROT0, "Accept Ltd.", "Hit Poker (Bulgaria)", GAME_NOT_WORKING|GAME_NO_SOUND )

