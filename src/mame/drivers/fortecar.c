/* Forte Card */

/*

TODO:
-eeprom
-bankswitch
-inputs
-missing color proms?

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

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/ay8910.h"
#include "machine/8255ppi.h"

static UINT8 *fortecar_ram;
static int bank;

static WRITE8_HANDLER( fortecar_videoregs_w )
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

static WRITE8_DEVICE_HANDLER( ppi0_portc_w )
{
	eeprom_write_bit(data & 0x04);
	eeprom_set_cs_line((data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
	eeprom_set_clock_line((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
}

static READ8_DEVICE_HANDLER( ppi0_portc_r )
{
//  popmessage("%s",cpuexec_describe_context(device->machine));
	return (~(eeprom_read_bit()<<1) & 2);
}

static const ppi8255_interface ppi0intf =
{
	DEVCB_INPUT_PORT("DSW1"),	DEVCB_INPUT_PORT("IN2"),	DEVCB_HANDLER(ppi0_portc_r),
	DEVCB_NULL,					DEVCB_NULL,					DEVCB_HANDLER(ppi0_portc_w)
};

static WRITE8_HANDLER( rom_bank_w )
{
	int new_bank = (data&0xff)>>0;

	if(bank!=new_bank) {
		UINT8 *ROM = memory_region(space->machine, "maincpu");
		UINT32 bankaddress;

		bank = new_bank;
		bankaddress = 0x10000 + 0x40 * bank;
		memory_set_bankptr(space->machine, 1, &ROM[bankaddress]);
	}
}

static WRITE8_DEVICE_HANDLER( ayporta_w )
{
}

static WRITE8_DEVICE_HANDLER( ayportb_w )
{
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
	AM_RANGE(0xc000, 0xc7ff) AM_READWRITE(SMH_BANK(1), SMH_ROM) //bank
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xd800, 0xffff) AM_RAM AM_BASE(&fortecar_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fortecar_ports, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x21) AM_WRITE(fortecar_videoregs_w) // MC6845?
	AM_RANGE(0x40, 0x40) AM_DEVREAD("ay", ay8910_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("ay", ay8910_address_data_w)
	AM_RANGE(0x60, 0x62) AM_DEVREADWRITE("fcppi0", ppi8255_r, ppi8255_w)//M5L8255AP
	AM_RANGE(0x81, 0x81) AM_WRITE(rom_bank_w) //completely wrong,might not be there...
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
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{ 8,9,10,11,0, 1, 2, 3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( fortecar )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

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

			tile = fortecar_ram[(count*4)+1] | (fortecar_ram[(count*4)+2]<<8);
			color = fortecar_ram[(count*4)+3];

			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],tile,color,0,0,x*8,y*8);
			count++;

		}
	}
	return 0;
}

static MACHINE_RESET(fortecar)
{
	bank = -1;
}

static MACHINE_DRIVER_START( fortecar )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,6000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(fortecar_map)
	MDRV_CPU_IO_MAP(fortecar_ports)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 256-1)

	MDRV_MACHINE_RESET(fortecar)
	MDRV_NVRAM_HANDLER(93C46) //GOLDSTAR GM76C256ALL-70

	MDRV_PPI8255_ADD("fcppi0", ppi0intf)

	MDRV_GFXDECODE(fortecar)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(fortecar)
	MDRV_VIDEO_UPDATE(fortecar)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 1500000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

ROM_START( fortecar )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "fortecar.u7", 0x00000, 0x0c000, CRC(2a4b3429) SHA1(8fa630dac949e758678a1a36b05b3412abe8ae16)  )
	ROM_CONTINUE(			 0x10000, 0x04000 )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fortecar.u38", 0x00000, 0x10000, CRC(c2090690) SHA1(f0aa8935b90a2ab6043555ece69f926372246648) )
	ROM_LOAD( "fortecar.u39", 0x10000, 0x10000, CRC(fc3ddf4f) SHA1(4a95b24c4edb67f6d59f795f86dfbd12899e01b0) )
	ROM_LOAD( "fortecar.u40", 0x20000, 0x10000, CRC(9693bb83) SHA1(e3e3bc750c89a1edd1072ce3890b2ce498dec633) )
ROM_END

static DRIVER_INIT( fortecar )
{
}

GAME( 19??, fortecar, 0, fortecar, fortecar, fortecar, ROT0, "Fortex LTD", "Forte Card",GAME_NOT_WORKING | GAME_WRONG_COLORS)
