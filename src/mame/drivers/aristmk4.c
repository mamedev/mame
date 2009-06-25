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
			// 0x0800 probably flipy

			drawgfx_opaque(bitmap,cliprect,gfx,tile,color,flipx,0,x*8,y*8);
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
	  AM_RANGE(0x0800, 0x1fff) AM_RAM
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

static DRIVER_INIT( aristmk4 )
{
	//...
}

GAME( 1994, eforest,0, aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Enchanted Forest - 12XF528902", GAME_NOT_WORKING|GAME_NO_SOUND )
