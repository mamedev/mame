/*

  Enchanted Forest  1994 Aristocrat Leisure Industries
  Manufacturer: Aristocrat Leisure Industries
  Platform: Mark IV 540 Video

  Driver by Palindrome


Technical Notes:

      MC6809 Motorola Processor
      R6545AP for CRT video controller
      UPD43256BCZ-70LL for 32kb of static ram used for 3 way electronic meters / 3 way memory
      U6264A for Standard 8K x 8 bit SRAM used for video buffer
      1 x R65C21P2  PIA - Peripheral Interface Adapter, connects to RTC and sends pulses to mechanical meters
      1 x 6522 VIA - 1 x Rockwell - Versatile Interface Adapter.
      2 x WF19054 = AY3-8910 sound chips driven by the 6522 VIA
      1 x PML 2852 ( programmable logic ) used as address decoder.

      VIA drives the programmable sound generators and generates
      a timing interrupt to the CPU (M6809_FIRQ_LINE)

      PIA provides output signals to six mechanical meters.
      It also provides the real time clock DS1287 to the CPU.


***********************************************************************************************************************************************/

#define MAIN_CLOCK	XTAL_12MHz


#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"



static UINT8 *mkiv_vram;

static VIDEO_START(aristmk4)
{
}

static VIDEO_UPDATE(aristmk4)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int x,y;
	int count = 0;

	for (y=0;y<27;y++)
	{
		for (x=0;x<38;x++)
		{
			int tile = (mkiv_vram[count+1]|mkiv_vram[count]<<8) & 0x3ff;
			int color = ((mkiv_vram[count]) & 0xe0) >> 5;
			int flipx = ((mkiv_vram[count]) & 0x04);
			int flipy = ((mkiv_vram[count]) & 0x08);

			drawgfx_opaque(bitmap,cliprect,gfx,tile,color,flipx,flipy,x*8,y*8);
			count+=2;
		}
	}
	return 0;
}



static WRITE8_HANDLER(test_w)
{

}

static READ8_HANDLER(test_r)
{
       return 0;
}


/******************************************************************************

PERIPHERAL INTERFACE ADAPTER CONFIGURATION

PORTA - DALLAS DS1287 RTC
PORTB - MECHANICAL METERS

******************************************************************************/



static int pia_data = 0;


/*****************************************************************************/
/* DALLAS DS1287
******************************************************************************/

static UINT8 bcd(UINT8 data)
{
	return ((data / 10) << 4) | (data % 10);
}


static UINT8 rtc_get_reg(running_machine *machine,int address_register)
{
	mame_system_time systime;
	mame_get_current_datetime(machine, &systime);
	switch(address_register)
	{
		case 0x00:
			//seconds
			return bcd(systime.local_time.second);
		case 0x01:
			//seconds alarm
			return 0;
		case 0x02:
			//minutes
            return bcd(systime.local_time.minute);
		case 0x03:
			//minutes alarm
            return 0;//bcd(systime.local_time.minute);
		case 0x04:
			//hours
			return bcd(systime.local_time.hour);
		case 0x05:
		    //hours alarm
			return 0;//bcd(systime.local_time.hour);
		case 0x06:
			//day of week
			return bcd(systime.local_time.mday);
		case 0x07:
			//date of month
			return bcd(systime.local_time.mday);
		case 0x08:
			//month
			return bcd(systime.local_time.month + 1);
		case 0x09:
			//year
			return bcd(systime.local_time.year % 10);
		case 0x0A:
			//register A
			return 0xA;
		case 0x0B:
			//register B
			return 0xB;
		case 0x0C:
			//register C
			return 0xC;
		case 0x0D:
			//register D
			return 0xD;
		default:
			fatalerror("DALLAS DS1287: Unknown register %02X", address_register);
			return 0x00;
     }
}

// RTC CLOCK

// datain
static WRITE8_HANDLER(mkiv_datain_pia_w)
{
    const device_config *pia_0 = devtag_get_device(space->machine, "pia6821_0");
    //logerror("CPU ===> PIA: %02X\n", data);
    pia6821_w(pia_0, offset, data);

}

// data out
static READ8_HANDLER(mkiv_dataout_pia_r)
{
    return pia_data;

}

//input a
static READ8_DEVICE_HANDLER(mkiv_pia_ina)
{
    // logerror("PIA Port A Read Handler: %02X\n", rtc_data);
    pia_data = rtc_get_reg(device->machine,pia_data); //  RTC
    return pia_data;


}

//output a
static WRITE8_DEVICE_HANDLER(mkiv_pia_outa)
{
     pia_data = data;
     //logerror("PIA Port A Write Handler: %02X\n", data);

}

//output ca2
static WRITE8_DEVICE_HANDLER(mkiv_pia_ca2)
{

     //logerror("PIA Port CA2 Write Handler: %02X\n", data);
}



//output b
static WRITE8_DEVICE_HANDLER(mkiv_pia_outb)
{

     UINT8 emet[5];
     int i = 0;
     //pia_data = data;
     emet[0] = data & 0x01;	/* emet1  -  bit 1 - PB0 */
     						/* seren1 -  bit 2 - PB1 */
     emet[1] = data & 0x04; /* emet3  -  bit 3 - PB2 */
     emet[2] = data & 0x08; /* emet4  -  bit 4 - PB3 */
     emet[3] = data & 0x10; /* emet5  -  bit 5 - PB4 */
     emet[4] = data & 0x20; /* emet6  -  bit 6 - PB5 */


     for(i = 0;i<sizeof(emet);i++)
     {
        //   logerror("Mechanical meter %d pulse: %02d\n",i+1, emet[i]);
     }
     //logerror("\n");

}


//output cb2
static WRITE8_DEVICE_HANDLER(mkiv_pia_cb2)
{

     //logerror("PIA Port CB2 Write Handler: %02X\n", data);
}

/******************************************************************************

VERSATILE INTERFACE ADAPTER CONFIGURATION

******************************************************************************/


static READ8_DEVICE_HANDLER(via_a_r)
{
    return 0;
}

static READ8_DEVICE_HANDLER(via_b_r)
{
    return 0;
}

static WRITE8_DEVICE_HANDLER(via_a_w)
{

}

static WRITE8_DEVICE_HANDLER(via_b_w)
{

}



static ADDRESS_MAP_START( aristmk4_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&mkiv_vram) // video ram -  chips U49 / U50
	AM_RANGE(0x0800, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x1800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x1801, 0x1801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x2000, 0x3fff) AM_ROM  // graphics rom map
	AM_RANGE(0x4000, 0x4fff) AM_RAM  // SRAM for meters - U74
	// VIA
	AM_RANGE(0x5000, 0x500f) AM_MIRROR(0x0010) AM_DEVREADWRITE("via6522_0",via_r,via_w)
	// PIA
	AM_RANGE(0x5380, 0x5383) AM_READWRITE(mkiv_dataout_pia_r,mkiv_datain_pia_w) // RTC data - PORT A , mechanical meters - PORTB ??
	AM_RANGE(0x5468, 0x5468) AM_READWRITE(test_r,test_w) // audit switch and cage door switch ???
	AM_RANGE(0x6000, 0xffff) AM_ROM  // game roms
ADDRESS_MAP_END

static INPUT_PORTS_START(aristmk4)

    PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Maxbet rejection" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Hopper pay limit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Hopper pay limit - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Hopper pay limit - S3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Cash credit option" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Link Jackpot - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Link Jackpot - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Auto spin" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Maximum credit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Maximum credit - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Maximum credit S3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Jackpot limit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Jackpot limit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Jackpot limit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto J/P payout" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "unknown" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout layout8x8x6 =
{
	8,8,
	RGN_FRAC(1,6),
	6,
	{
		RGN_FRAC(5,6),
		RGN_FRAC(2,6),
		RGN_FRAC(4,6),
		RGN_FRAC(1,6),
		RGN_FRAC(3,6),
		RGN_FRAC(0,6)
	},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};



static GFXDECODE_START(aristmk4)
	GFXDECODE_ENTRY( "tile_gfx", 0, layout8x8x6, 0, 8 )
GFXDECODE_END



static const ay8910_interface ay8910_config1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static const ay8910_interface ay8910_config2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW1"),
    DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/*
static const via6522_interface via_interface =
{
    // DEVCB_HANDLER(via_a_in), DEVCB_HANDLER(via_b_in),
    // DEVCB_HANDLER(input_ca1), DEVCB_HANDLER(input_cb1), DEVCB_HANDLER(input_ca2), DEVCB_HANDLER(input_cb2),
    /// DEVCB_NULL, DEVCB_HANDLER(output_b),
    // DEVCB_HANDLER(output_ca1), DEVCB_HANDLER(output_cb1), DEVCB_HANDLER(output_ca2), DEVCB_HANDLER(output_cb2),
   //  DEVCB_CPU_INPUT_LINE("maincpu", M6809_FIRQ_LINE)
}; */

static const via6522_interface via_interface =
{
	/*inputs : A/B         */ DEVCB_HANDLER(via_a_r),DEVCB_HANDLER(via_b_r),
	/*inputs : CA/B1,CA/B2 */ DEVCB_NULL,DEVCB_NULL,DEVCB_NULL,DEVCB_NULL,
	/*outputs: A/B         */ DEVCB_HANDLER(via_a_w), DEVCB_HANDLER(via_b_w),
	/*outputs: CA/B1,CA/B2 */ DEVCB_NULL,DEVCB_NULL,DEVCB_NULL,DEVCB_NULL,
	/*irq                  */ DEVCB_CPU_INPUT_LINE("maincpu", M6809_FIRQ_LINE)
};

static const pia6821_interface aristmk4_pia1_intf =
{
    DEVCB_HANDLER(mkiv_pia_ina),     /* port A in */
	DEVCB_NULL, 	/* port B in */
    DEVCB_NULL,     /* line CA1 in */
    DEVCB_NULL,     /* line CB1 in */
    DEVCB_NULL,     /* line CA2 in */
    DEVCB_NULL,     /* line CB2 in */
    DEVCB_HANDLER(mkiv_pia_outa),     /* port A out */
    DEVCB_HANDLER(mkiv_pia_outb),     /* port B out */
    DEVCB_HANDLER(mkiv_pia_ca2),     /* line CA2 out */
    DEVCB_HANDLER(mkiv_pia_cb2),     /* port CB2 out */
    DEVCB_NULL,       /* IRQA */
    DEVCB_NULL        /* IRQB */
};

static const mc6845_interface mc6845_intf =
{
	/* in fact is a mc6845 driving 4 pixels by memory address.
       that's why the big horizontal parameters */

	"screen",	/* screen we are acting on */
	4,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};



static INTERRUPT_GEN( mkiv_irq )
{
}

/* same as Casino Winner HW */
static PALETTE_INIT( aristmk4 )
{
	int i;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		bit2 = (color_prom[0] >> 4) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 5) & 0x01;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

static MACHINE_DRIVER_START( aristmk4 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, MAIN_CLOCK/4) // 3 MHZ
	MDRV_CPU_PROGRAM_MAP(aristmk4_map)
	MDRV_CPU_VBLANK_INT("screen", mkiv_irq )

    /* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 304-1, 0, 216-1)	/* from the crtc registers... updated by crtc */

	MDRV_GFXDECODE(aristmk4)
	MDRV_PALETTE_LENGTH(512)
	MDRV_PALETTE_INIT(aristmk4)

	MDRV_VIDEO_START(aristmk4)
	MDRV_VIDEO_UPDATE(aristmk4)

	MDRV_VIA6522_ADD("via6522_0", MAIN_CLOCK/6, via_interface)	/* 1 MHz.(only 1 or 2 MHz.are valid) */
	MDRV_PIA6821_ADD("pia6821_0", aristmk4_pia1_intf)
    MDRV_MC6845_ADD("crtc", MC6845, MAIN_CLOCK/8, mc6845_intf)

    MDRV_SPEAKER_STANDARD_MONO("mono")
    // the Mark IV has X 2 AY8910 sound chips which are tied to the VIA
    MDRV_SOUND_ADD("ay1", AY8910, MAIN_CLOCK/8)
    MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
    MDRV_SOUND_ADD("ay2", AY8910, MAIN_CLOCK/8)
    MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

MACHINE_DRIVER_END

ROM_START( 3bagflnz )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(ba97a469) SHA1(fee56fe7116d1f1aab2b0f2526101d4eb87f0bf1)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(c632c7c7) SHA1(f3090d037f71a0cf099bb55abbc509cf95f0cbba)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, BAD_DUMP CRC(44babe95) SHA1(047c00ebb21030563921108b8e24f62e9ef44a10)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, BAD_DUMP CRC(06218c95) SHA1(cbda8e50fd4e9c8a3c51a006921a85d4bfaa6f78))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, BAD_DUMP CRC(191e73f1) SHA1(e6d510b155f9cd3427a70346e5ff28969309be4e))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, BAD_DUMP CRC(054c55cb) SHA1(3df1893095f867220f3d6a52a40bcdffbfc8b529))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, BAD_DUMP CRC(7a4e8b80) SHA1(35711d6a8f5675ad6c6496bf8e7e5a73504f2409))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, BAD_DUMP CRC(609ecf9e) SHA1(9d819bb71f62eb4dd1b3d71748e87c7d77e2afe6))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( blkrhino )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(7aed16f5) SHA1(0229387e352da8e7278e5bc5c61079742d05d900)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(4739f0f0) SHA1(231b6ad26b6b5d413dbd0a23257e86814978449b)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(0559fe98) SHA1(2ffb7b3ce3b7ba3bd846cae514b66b1c1a3be91f)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(c0b94f7b) SHA1(8fc3bc53c532407b77682e5e9ac6a625081d22a3))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(2f4f0fe5) SHA1(b6c75bd3b6281a2de7bfea8162c39d58b0e8fa32))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(e483b4cd) SHA1(1cb3f77e7d470d7dcd8e50a0f59298d5546e8b58))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(4a0ce91d) SHA1(e2f853c69fb256870c9809cdfbba2b40b47a0004))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(b265276e) SHA1(8fc0b7a0c12549b4138c51eb91b74f13282909dd))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( coralr2 )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(f51e541b) SHA1(00f5b9019cdae77d4b5745156b92343d22ad3a6e)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(d8d27f65) SHA1(19aec2a29e9d3ecbd8ecfd74ae60cfbf197d2faa)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(5156f5ec) SHA1(8b4d0699b4477531d513e21f549fcc0ee6ea82ee)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(bf27732a) SHA1(9383dfc37c5c3ad0d628f2134f010e977e25ef39))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(a563c2fa) SHA1(10dab35515e2d8332d114a5f103343403334a65f))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(73814767) SHA1(91c77d7b634bd8a5c32e0ceeb54a8bbeedfe8130))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(e13ec0ed) SHA1(80d5ef2d980a8fe1f2bb28b512022518ffc82de1))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(30e88bb4) SHA1(dfcd21c6fc50123dfcc0e60429948c650a6de625))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( eforest )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(b2f79725) SHA1(66842130b49276bda91e211514af0ab074d2c283)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(547207f3) SHA1(aedae50abb4cffa0434abfe606a11fbbba037197)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(00446ff1) SHA1(e37e2782669667efab07ab3cd4e2c9f87770add5)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(75670af6) SHA1(c6ce3dcc9e46ebdee65220cc0e0c8b43ee786ec0))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(531378f3) SHA1(7ce52ccfb38c8078eeb77e43cff8e426bd8d4d0f))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(e4dd4c72) SHA1(caeb1f754bcb7304b9a61786fb818eea7714808f))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(75ad8e3f) SHA1(87812850f08f8ad3057d0e5a2a20ad8acba01a26))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(af9f9869) SHA1(1dac81470889a5fc5b58f3ad0c8dfa1369a800e8))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( eforesta )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(03c2890f) SHA1(10d479b7ccece813676ad815a96169bbf259c49d)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(36125194) SHA1(dc681dc60b25893ca3ee101f6813c22b914771f5)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(00446ff1) SHA1(e37e2782669667efab07ab3cd4e2c9f87770add5)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(75670af6) SHA1(c6ce3dcc9e46ebdee65220cc0e0c8b43ee786ec0))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(531378f3) SHA1(7ce52ccfb38c8078eeb77e43cff8e426bd8d4d0f))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(e4dd4c72) SHA1(caeb1f754bcb7304b9a61786fb818eea7714808f))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(75ad8e3f) SHA1(87812850f08f8ad3057d0e5a2a20ad8acba01a26))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(af9f9869) SHA1(1dac81470889a5fc5b58f3ad0c8dfa1369a800e8))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( eforestb )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(49b9c5ef) SHA1(bd1761f41ddb3f19b6b923de77743a2b5ec078e1)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(a3eb0c09) SHA1(5a0947f2f36a87dffe4041fbaebaabb1c694bafe)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(bf3a23b0) SHA1(00405e0c0ac03ecffba1077bacf61265cca72130)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(ba171964) SHA1(7d43559965f467f07419f77d07d7d34ae60d2e90))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(531378f3) SHA1(7ce52ccfb38c8078eeb77e43cff8e426bd8d4d0f))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(e4dd4c72) SHA1(caeb1f754bcb7304b9a61786fb818eea7714808f))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(a3ca69b0) SHA1(c4bdd8afbb4d076f07d4a14a7e7ac8907a0cb7ec))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(af9f9869) SHA1(1dac81470889a5fc5b58f3ad0c8dfa1369a800e8))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( goldenc )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(11b569f7) SHA1(270e1be6bf2a75400af174ceb65436bb6a381a62)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(9714b080) SHA1(41c7d840f600ddff31794ebe949f89c89bd4f2ad)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(d4b18412) SHA1(a42a06dbfc55730b27b3857646bfa34ae0e3cb32)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(80e22d51) SHA1(5e187070d300209e31f603aa561011e17d4305d2))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(1f84ed74) SHA1(df2af247972d6540fd4aac31b51f3aa44248061c))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(9d267ef1) SHA1(3781e63552036dc7613b21704a4456ddfb67433f))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(a3ca369e) SHA1(e3076c9f3017991b93214bebf7f5227d995eeda1))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(844fa43b) SHA1(b8ef6cc2aca955f41b15cd8e3c281eee4b611e80))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( swtht2nz )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(ae10c63f) SHA1(80e5aca4dec7d2503bf7be81ed8b761ebbe4c174)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(053e71f0) SHA1(4a45bd11b53347be90402cea7bd94a648d6b8129)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(1e38dfc3) SHA1(40a75fc35ebd49ea9c21cb42c30a2aba988c3139)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(77caf3fa) SHA1(559898ccffffd8f59c555722dea75600c823997f))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(76babc55) SHA1(0902497ad2222490a690fe77feacc350d2997403))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(da9514b5) SHA1(d63562095cec463864dfd2c580aa93f45adef853))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(4d03c73f) SHA1(7ae629a90feb87019cc01ecef804c5ba28861f00))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(c51e37bb) SHA1(8f3d9b61926fe21089559736b3458fe3b84618f2))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( kgbird )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(5e7c1762) SHA1(2e80be06c7737aca304d46f3c3f1efd24c570cfd)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(3450c544) SHA1(f8883ce3b4bd9073ec6bc985f4666b46f17de092)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(df737d57) SHA1(2ec7efe55938ee11376d12d51516c4094ad3fc01)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(8e9a65d9) SHA1(e305b8d75b9666377498abf3e2801033effb969b))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(5614ee55) SHA1(3eb3872aa8d2b8c2bd798fd46cc715c64bf35714))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(cf496cf2) SHA1(cf097835b5f3d5a656ff84063c54d6b1d40703cd))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(e155c8d4) SHA1(9c50152dd420d545a88eaea98c2dd2ef49cf056a))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(f91b66ba) SHA1(4f5d0f0562c6a6029ad6d76507091a159983d6f4))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( kgbirda )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(21c05874) SHA1(9ddcd34817bc6f88cb2a94374e492d29dd56fb9a)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(3450c544) SHA1(f8883ce3b4bd9073ec6bc985f4666b46f17de092)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(df737d57) SHA1(2ec7efe55938ee11376d12d51516c4094ad3fc01)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(8e9a65d9) SHA1(e305b8d75b9666377498abf3e2801033effb969b))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(5614ee55) SHA1(3eb3872aa8d2b8c2bd798fd46cc715c64bf35714))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(cf496cf2) SHA1(cf097835b5f3d5a656ff84063c54d6b1d40703cd))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(e155c8d4) SHA1(9c50152dd420d545a88eaea98c2dd2ef49cf056a))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(f91b66ba) SHA1(4f5d0f0562c6a6029ad6d76507091a159983d6f4))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( phantomp )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(84e8eeb5) SHA1(95dcbae79b42463480fb3dd2594570070ba1a3ef)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(a6aa3d6f) SHA1(64d97c52355d5d0faebe1ee704f6ad46cc90f0f1)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(0f73cf57) SHA1(f99aa9671297d8cefeff86e642af5ea3e7f6f6fb)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(2449d69e) SHA1(181d7d093dce1acc332255cab5d56a9043bcab47))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(5cb0f179) SHA1(041f7baa5a36f544a98832753ff54ca5238f12c5))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(75f94143) SHA1(aac2b0bee1a0d83b25c6fd21f00803209b621543))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(6ead5ffc) SHA1(1611d5e2dd5ea06525b6079577a45e713a8065d5))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(c1fb4f23) SHA1(6c9a4e52bd0312c9b49f91a1f563fecd87e5bb82))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( topgear )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(5628f477) SHA1(8517905b4d4174fea79e2e3ed38c80fcc6506c6a)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(d5afa54e) SHA1(4268c0ddb9beab68348ba520d47bea64b875d8a7)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(e3163956) SHA1(b3b55be33fad96858dc683860d72c81ed02b3d97)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(9ce936cb) SHA1(cca6ec0190a61cb0b52fbe1b11fb678f5e0960df))
   	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(972f091a) SHA1(b94a04e9503fb6f1a687c854076cfc9629ed7b6a))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(27fd4204) SHA1(0d082a4297a384c992188dd43be0ecb706117c13))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(186f3e3b) SHA1(57f82a79a3d24090f33f5525207d6697e954cdf5))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(dc7d2dab) SHA1(16d223f28b377fafb478d6124fc0eb6d7dd7d591))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

static DRIVER_INIT( aristmk4 )
{
	//...
}

GAME( 1994, eforest,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Enchanted Forest - 12XF528902", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, eforesta,eforest, aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Enchanted Forest - 4VXFC818", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, eforestb,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Enchanted Forest - 3VXFC5343 (New Zealand)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1994, 3bagflnz,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "3 Bags Full - 3VXFC5345 (New Zealand)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, blkrhino,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Black Rhino - 3VXFC5344 (New Zealand)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, kgbird,			0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "K.G Bird - 4VXFC5341 (New Zealand, 87.98%)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, kgbirda,kgbird,   aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "K.G Bird - 4VXFC5341 (New Zealand, 91.97%)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1998, swtht2nz,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Sweet Hearts II - 1VXFC5461 (New Zealand)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, goldenc,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Golden Canaries - 1VXFC5462", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, topgear,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Top Gear - 4VXFC969", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1998, phantomp,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Phantom Pays - 4VXFC5431 (New Zealand)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, coralr2,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Coral Riches II - 1VXFC5472 (New Zealand)", GAME_NOT_WORKING|GAME_NO_SOUND )
