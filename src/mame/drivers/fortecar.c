/* Forte Card */

/*

TODO:
-eeprom
-inputs
-missing color proms?

bp 529 do pc=53e

-----------------------------------------------------

Forte Card (POKER GAME)

CPU  SGS Z8400AB1 (Z80ACPU)
VIDEO CM607P
PIO M5L8255AP
snd ay38910/P (microchip)
+ 8251A

RAM 6116 + GOLDSTAR GM76C256ALL-70
dip 1X8

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/ay8910.h"
#include "machine/8255ppi.h"
#include "video/mc6845.h"

static UINT8 *fortecar_ram;
static int bank;

static VIDEO_START(fortecar)
{
}

static VIDEO_UPDATE(fortecar)
{
	int x,y,count;
	count = 0;

	for (y=0;y<0x1e;y++)
	{
		for(x=0;x<0x4b;x++)
		{
			int tile,color;

			tile = (fortecar_ram[(count*4)+1] | (fortecar_ram[(count*4)+2]<<8)) & 0xfff;
			color = fortecar_ram[(count*4)+3] & 3;

			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],tile,color,0,0,x*8,y*8);
			count++;

		}
	}

	return 0;
}

static WRITE8_DEVICE_HANDLER( ppi0_portc_w )
{
	eeprom_write_bit(device, data & 0x04);
	eeprom_set_cs_line(device, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
	eeprom_set_clock_line(device, (data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
}

static READ8_DEVICE_HANDLER( ppi0_portc_r )
{
//  popmessage("%s",cpuexec_describe_context(device->machine));
	return (~(eeprom_read_bit(device)<<1) & 2);
}

static const ppi8255_interface ppi0intf =
{
	DEVCB_INPUT_PORT("DSW1"),	DEVCB_INPUT_PORT("IN2"),	DEVCB_DEVICE_HANDLER("eeprom", ppi0_portc_r),
	DEVCB_NULL,					DEVCB_NULL,					DEVCB_DEVICE_HANDLER("eeprom", ppi0_portc_w)
};


static WRITE8_DEVICE_HANDLER( ayporta_w )
{
	logerror("AY port A write %02x\n",data);

	/*
    lamps for POST?
    0x01: RAM test d000-d7ff
    0x02: VRAM test d800-ffff
    0x04: Video SYNC test
    0x08: ROM check
    0x10: NVRAM check
    0x20: IRQ test
    0x40: Stack RAM check
    */
}

/* lamps? */
static WRITE8_DEVICE_HANDLER( ayportb_w )
{
	//logerror("AY port B write %02x\n",data);
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ayporta_w),
	DEVCB_HANDLER(ayportb_w)
};

static ADDRESS_MAP_START( fortecar_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xd800, 0xffff) AM_RAM AM_BASE(&fortecar_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fortecar_ports, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x21, 0x21) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0x40, 0x40) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x60, 0x63) AM_DEVREADWRITE("fcppi0", ppi8255_r, ppi8255_w)//M5L8255AP
//  AM_RANGE(0x80, 0x81) //8251A UART
	AM_RANGE(0xa0, 0xa0) AM_READ_PORT("IN0") //written too,multiplexer?
	AM_RANGE(0xa1, 0xa1) AM_READ_PORT("IN1")
ADDRESS_MAP_END


static INPUT_PORTS_START( fortecar )
	PORT_START("IN0")	/* 8bit */
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

	PORT_START("IN1")	/* 8bit */
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

	PORT_START("IN2")	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )

	PORT_START("DSW1")	/* 8bit */
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
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ 0,4, RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+4, RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+4 },
	{ 8,9,10,11,0, 1, 2, 3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( fortecar )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 4 )
GFXDECODE_END

static MACHINE_RESET(fortecar)
{
	bank = -1;
}

/* WRONG, just to see something */
static PALETTE_INIT( fortecar )
{
	int i,r,g,b;

	for (i = 0; i < 0x40*4; i++ )
	{
		g = (i >> 0) & 3;
		b = (i >> 2) & 3;
		r = (i >> 4) & 3;

		palette_set_color(machine, i, MAKE_RGB(pal2bit(r), pal2bit(g), pal2bit(b)));
	}
}

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

//51f
static MACHINE_CONFIG_START( fortecar, driver_device )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,6000000)		 /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(fortecar_map)
	MCFG_CPU_IO_MAP(fortecar_ports)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 256-1)

	MCFG_MACHINE_RESET(fortecar)

	MCFG_EEPROM_93C46_ADD("eeprom")
	MCFG_EEPROM_DEFAULT_VALUE(0)

	MCFG_PPI8255_ADD("fcppi0", ppi0intf)

	MCFG_GFXDECODE(fortecar)
	MCFG_PALETTE_LENGTH(0x100)
	MCFG_PALETTE_INIT(fortecar)

	MCFG_VIDEO_START(fortecar)
	MCFG_VIDEO_UPDATE(fortecar)

	MCFG_MC6845_ADD("crtc", MC6845, 6000000/4, mc6845_intf)	/* unknown type / hand tuned to get ~60 fps */

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 1500000)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( fortecar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fortecar.u7", 0x00000, 0x010000, CRC(2a4b3429) SHA1(8fa630dac949e758678a1a36b05b3412abe8ae16)  )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fortecar.u38", 0x00000, 0x10000, CRC(c2090690) SHA1(f0aa8935b90a2ab6043555ece69f926372246648) )
	ROM_LOAD( "fortecar.u39", 0x10000, 0x10000, CRC(fc3ddf4f) SHA1(4a95b24c4edb67f6d59f795f86dfbd12899e01b0) )
	ROM_LOAD( "fortecar.u40", 0x20000, 0x10000, CRC(9693bb83) SHA1(e3e3bc750c89a1edd1072ce3890b2ce498dec633) )

	ROM_REGION( 0x200, "prom", 0 )
	ROM_LOAD( "prom.x", 0x000, 0x200, NO_DUMP )
ROM_END

static DRIVER_INIT( fortecar )
{
}

GAME( 19??, fortecar, 0, fortecar, fortecar, fortecar, ROT0, "Fortex Ltd", "Forte Card",GAME_NOT_WORKING | GAME_WRONG_COLORS)
