/******************************************************************************

Hit Poker (c) 1997 Accept LTD

preliminary driver by Angelo Salese & David Haywood
Many thanks to Olivier Galibert for the CPU identify effort ;-)

TODO:
- CPU core bugs?
- Protection controls inputs;
- Understand & fix EEPROM emulation;
- Hangs during attract mode, eeprom or protection?
- complete video HW (unknown bits and hblank);
- 24Khz monitor isn't supported, it changes the resolution to 648 x 480 and
  changes the register 9 (raster lines x character lines) from 7 to 0xf.
- sound (I've heard something reasonable during tests, so it could be OK);

================================================================================

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
- set a bp at 1185, the "bad crc 000" msg is caused by this routine.
  (kludged to work)
- set a bp at 121f then pc=1223
- set a bp at 3a50 then pc+=2, it should now enter into attract mode

***************************************************************************/


#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

static UINT8 *hitpoker_sys_regs;
static UINT8 hitpoker_pic_data;
static UINT8 *videoram;
static UINT8 *paletteram;
static UINT8 *colorram;

#define CRTC_CLOCK XTAL_3_579545MHz

static VIDEO_START(hitpoker)
{
	videoram = auto_alloc_array(machine, UINT8, 0x35ff);
	paletteram = auto_alloc_array(machine, UINT8, 0x1000);
	colorram = auto_alloc_array(machine, UINT8, 0x2000);
}

static VIDEO_UPDATE(hitpoker)
{
	int count = 0;
	int y,x;

	bitmap_fill(bitmap, cliprect, 0);

	for (y=0;y<31;y++)
	{
		for (x=0;x<81;x++) //it's probably 80 + 1 global line attribute at the start of each line
		{
			int tile,color,gfx_bpp;

			tile = (((videoram[count]<<8)|(videoram[count+1])) & 0x3fff);
			gfx_bpp = (colorram[count] & 0x80)>>7; //flag between 4 and 8 bpp
			color = gfx_bpp ? ((colorram[count] & 0x70)>>4) : (colorram[count] & 0xf);

			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[gfx_bpp],tile,color,0,0,x*8,y*8);

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
//  UINT8 *ROM = memory_region(space->machine, "maincpu");

//  if(hitpoker_sys_regs[0x00] & 0x10)
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

	/* RGB565 */
	b = ((datax)&0xf800)>>11;
	g = ((datax)&0x07e0)>>5;
	r = ((datax)&0x001f)>>0;

	palette_set_color_rgb(space->machine, offset, pal5bit(r), pal6bit(g), pal5bit(b));
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
		//printf("W INDEX %02x\n",data);
	}
	if(offset == 1)
	{
		eeprom_index = (eeprom_index & 0xff)|((data & 0x1)<<8);
		//data & 0x4: eeprom clock
		//printf("W CLOCK + INDEX %02x\n",data);
	}
}

static READ8_HANDLER( eeprom_r )
{
	static UINT8 tmp;
	tmp = eeprom_data[eeprom_index];
	if((eeprom_index & 0x1f) == 0x1f)
		tmp = 0xaa;
	//printf("%02x\n",eeprom_index);
	eeprom_index++;
	//eeprom_index&=0x1f;
	return tmp;
}

static READ8_HANDLER( hitpoker_pic_r )
{
//  logerror("R\n");

	if(offset == 0)
	{
		if(cpu_get_pc(space->cpu) == 0x3143 ||
		   cpu_get_pc(space->cpu) == 0x314e ||
		   cpu_get_pc(space->cpu) == 0x3164 ||
		   cpu_get_pc(space->cpu) == 0x3179)
			return hitpoker_pic_data;

		return (hitpoker_pic_data & 0x7f) | (hitpoker_pic_data & 0x40 ? 0x80 : 0x00);
	}

	return hitpoker_sys_regs[offset];
}

static WRITE8_HANDLER( hitpoker_pic_w )
{
	if(offset == 0)
		hitpoker_pic_data = (data & 0xff);// | (data & 0x40) ? 0x80 : 0x00;
//  logerror("%02x W\n",data);
	hitpoker_sys_regs[offset] = data;
}

#if 0
static READ8_HANDLER( test_r )
{
	return mame_rand(space->machine);
}
#endif

/* overlap empty rom addresses */
static ADDRESS_MAP_START( hitpoker_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_RAM // stack ram
	AM_RANGE(0x1000, 0x103f) AM_RAM // internal I/O
	AM_RANGE(0x8000, 0xb5ff) AM_READWRITE(hitpoker_vram_r,hitpoker_vram_w)
	AM_RANGE(0xb600, 0xbdff) AM_RAM
	AM_RANGE(0xbe0a, 0xbe0a) AM_READ_PORT("IN0")
	AM_RANGE(0xbe0c, 0xbe0c) AM_READ_PORT("IN2") //irq ack?
	AM_RANGE(0xbe0d, 0xbe0d) AM_READ(rtc_r)
	AM_RANGE(0xbe0e, 0xbe0e) AM_READ_PORT("IN1")
	AM_RANGE(0xbe50, 0xbe51) AM_WRITE(eeprom_w)
	AM_RANGE(0xbe53, 0xbe53) AM_READ(eeprom_r)
	AM_RANGE(0xbe80, 0xbe80) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xbe81, 0xbe81) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xbe90, 0xbe91) AM_DEVREADWRITE("aysnd", ay8910_r,ay8910_address_data_w)
	AM_RANGE(0xbea0, 0xbea0) AM_READ_PORT("VBLANK") //probably other bits as well
//  AM_RANGE(0xbe00, 0xbeff) AM_READ(test_r)
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(hitpoker_cram_r,hitpoker_cram_w)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(hitpoker_paletteram_r,hitpoker_paletteram_w)
	AM_RANGE(0x0000, 0xbdff) AM_ROM
	AM_RANGE(0xbf00, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hitpoker_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MC68HC11_IO_PORTA, MC68HC11_IO_PORTA) AM_READWRITE(hitpoker_pic_r,hitpoker_pic_w) AM_BASE(&hitpoker_sys_regs)
ADDRESS_MAP_END

static INPUT_PORTS_START( hitpoker )
	PORT_START("VBLANK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) //resets the game?
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
	PORT_DIPNAME( 0x20, 0x20, "H-Blank" ) //scanline counter probably
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Monitor" ) //a JP probably
	PORT_DIPSETTING(    0x40, "15KHz" )
	PORT_DIPSETTING(    0x00, "24KHz" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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


static const gfx_layout hitpoker_layout_4bpp =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout hitpoker_layout_8bpp =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+12,0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static GFXDECODE_START( hitpoker )
	GFXDECODE_ENTRY( "gfx1", 0, hitpoker_layout_4bpp,   0, 0x100  )
	GFXDECODE_ENTRY( "gfx1", 0, hitpoker_layout_8bpp,   0, 8  )
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

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_INPUT_PORT("DSW2"),
	DEVCB_NULL,
	DEVCB_NULL
};

static const hc11_config hitpoker_config =
{
	0, //has extended internal I/O
	0x100  //internal RAM size
};

static INTERRUPT_GEN( hitpoker_irq )
{
	cpu_set_input_line(device, MC68HC11_IRQ_LINE, HOLD_LINE);
}

static MACHINE_DRIVER_START( hitpoker )
	MDRV_CPU_ADD("maincpu", MC68HC11,1000000)
	MDRV_CPU_PROGRAM_MAP(hitpoker_map)
	MDRV_CPU_IO_MAP(hitpoker_io)
	MDRV_CPU_CONFIG(hitpoker_config)
	MDRV_CPU_VBLANK_INT("screen", hitpoker_irq)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(648, 480) //setted by the CRTC
	MDRV_SCREEN_VISIBLE_AREA(0, 648-1, 0, 240-1)

	MDRV_MC6845_ADD("crtc", H46505, CRTC_CLOCK/2, mc6845_intf)	/* hand tuned to get ~60 fps */

	MDRV_GFXDECODE(hitpoker)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(hitpoker)
	MDRV_VIDEO_UPDATE(hitpoker)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1500000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static DRIVER_INIT(hitpoker)
{
	UINT8 *ROM = memory_region(machine, "maincpu");

	#if 1
	ROM[0x1220] = 0x01; //patch eeprom write?
	ROM[0x1221] = 0x01;
	ROM[0x1222] = 0x01;

	ROM[0x10c6] = 0x01;
	ROM[0x10c7] = 0x01; //patch the checksum routine
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

