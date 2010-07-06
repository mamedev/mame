/*
        Driver: aristmk4

        Manufacturer: Aristocrat Leisure Industries
        Platform: Aristocrat 540 Video ( MK 2.5 Video / MK IV )
        Driver by Palindrome & FraSher

        Technical Notes:

        68B09EP Motorola Processor
        R6545AP for CRT video controller
        UPD43256BCZ-70LL for 32kb of static ram used for 3 way electronic meters / 3 way memory
        U6264A for Standard 8K x 8 bit SRAM used for video buffer
        1 x R65C21P2  PIA - Peripheral Interface Adapter, connects to RTC and sends pulses to mechanical meters
        1 x 6522 VIA - 1 x Rockwell - Versatile Interface Adapter.
        2 x WF19054 = AY3-8910 sound chips driven by the 6522 VIA
        1 x PML 2852 ( programmable logic ) used as address decoder.
        1 x PML 2852 programmed as a PIA


        PIA provides output signals to six mechanical meters.
        It also provides the real time clock DS1287 to the CPU.

        VIA drives the programmable sound generators and generates
        a timing interrupt to the CPU (M6809_FIRQ_LINE)

        The VIA uses Port A to write to the D0-D7 on the AY8910s. Port B hooks first 4 bits up to BC1/BC2/BDIR and A9 on AY1 and A8 on AY2
        The remaining 4 bits are connected to other hardware, read via the VIA.

        The AY8910 named ay1 has writes on PORT B to the ZN434 DA convertor.
        The AY8910 named ay2 has writes to lamps and the light tower on Port A and B. these are implemented via the layout

        *************************************************************************************************************

        27/04/10 - FrasheR
        2 x Sound Chips connected to the 6522 VIA.


        16/05/10 - FrasheR
        Fixed VIA for good. 5010 - 501F.
        Hooked up push button inputs - FrasheR
        Hooked up ports for the PML 2852 U3 - FrasheR

        16/05/10 - Palindrome
        Lamp outputs and layout added - Palindrome
        NVRAM backup - Palindrome

        20/05/10 - Palindrome
        Connected SW7 for BGCOLOUR map select
        Added LK13. 3Mhz or 1.5 Mhz CPU speed select
        Added sound sample for mechanical meter pulse ( aristmk4.zip  ).

        30/5/10 - Palindrome
        Now using mc146818 rtc driver instead of rtc_get_reg.

        The mc146818 driver is buggy - reported problem to Firewave and issues will be addressed.
        In this driver, the wrong day of the month is shown, wrong hours are shown.
        rtc causes game to freeze if the game is left in audit mode with continuous writes to 0xA reg - 0x80 data.

        TODO:
        1.Create layouts for each game  ( each game is currently using the generic aristmk4.lay for now ).
        - Games may have different button configuration requirements ( ie.. 9 or 5 lines and different bet values )

        2.Extend the driver to use the keno keyboard input for keno games.

        3.Some with cashcade jackpot systems such as topgear do not work yet. Eforest does not work.

        5.Add note acceptor support

        6.Robot test

***********************************************************************************************************************************************/

#define MAIN_CLOCK	XTAL_12MHz


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "machine/8255ppi.h"
#include "deprecat.h"
#include "aristmk4.lh"
#include "state.h"
#include "sound/samples.h"
#include "machine/mc146818.h" // DALLAS1287 is functionally compatible.


int rtc_address_strobe = 0;
int rtc_data_strobe = 0;

running_device *samples;
UINT8 *shapeRomPtr;     // pointer to tile_gfx shape roms
UINT8 shapeRom[0xC000]; // shaperom restore array.

static UINT8 *mkiv_vram;
static UINT8 *nvram;    // backup
static UINT8 psg_data;
static int ay8910_1;
static int ay8910_2;
static int u3_p0_w;
static UINT8 cgdrsw;
static UINT8 ripple;
static int hopper_motor;
static int inscrd;

static VIDEO_START(aristmk4)
{
    int tile;
    for (tile = 0; tile < machine->gfx[0]->total_elements; tile++)
    {
		gfx_element_decode(machine->gfx[0], tile);
    }
}

INLINE void uBackgroundColour(running_machine *machine)
{
    /* SW7 can be set when the main door is open, this allows the colours for the background
        to be adjusted whilst the machine is running.

       There are 4 possible combinations for colour select via SW7, colours vary based on software installed.

    */
    switch(input_port_read(machine, "SW7"))
	{
        case 0x00:
             // restore defaults
             memcpy(shapeRomPtr,shapeRom, sizeof(shapeRom)); // restore defaults, both switches off
                                                             // OE enabled on both shapes
             break;
        case 0x01:
             // unselect U22 via SW7 . OE on U22 is low.
             memset(&shapeRomPtr[0x4000],0xff,0x2000);               // fill unused space with 0xff
             memcpy(&shapeRomPtr[0xa000],&shapeRom[0xa000], 0x2000); // restore defaults here
             break;
        case 0x02:
             // unselect U47 via SW7 . OE on U47 is low.
             memcpy(&shapeRomPtr[0x4000],&shapeRom[0x4000], 0x2000);
             memset(&shapeRomPtr[0xa000],0xff,0x2000);
             break;
        case 0x03:
             // unselect U47 & u22 via SW7. both output enable low.
             memset(&shapeRomPtr[0x4000],0xff,0x2000);
             memset(&shapeRomPtr[0xa000],0xff,0x2000);
             break;
    }
}

static VIDEO_UPDATE(aristmk4)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int x,y;
	int count = 0;
    int color;
    int tile;
    int bgtile;
    int flipx;
    int flipy;

	for (y=27;y--;)
	{
		for (x=38;x--;)
		{
            color = ((mkiv_vram[count]) & 0xe0) >> 5;
			tile = (mkiv_vram[count+1]|mkiv_vram[count]<<8) & 0x3ff;
			bgtile = (mkiv_vram[count+1]|mkiv_vram[count]<<8) & 0xff; // first 256 tiles
			uBackgroundColour(screen->machine); // read sw7
			gfx_element_decode(gfx, bgtile);    // force the machine to update only the first 256 tiles.
			                                    // as we only update the background, not the entire display.
			flipx = ((mkiv_vram[count]) & 0x04);
			flipy = ((mkiv_vram[count]) & 0x08);
			drawgfx_opaque(bitmap,cliprect,gfx,tile,color,flipx,flipy,(38-x-1)<<3,(27-y-1)<<3);
			count+=2;
		}
	}
	return 0;
}

static READ8_HANDLER(ldsw)
{

   int U3_p2_ret= input_port_read(space->machine, "5002");
   if(U3_p2_ret & 0x1)
   {
        return 0;
   }
   return cgdrsw = input_port_read(space->machine, "CGDRSW");
}

static READ8_HANDLER(cgdrr)
{

   if(cgdrsw) // is the LC closed
   {
        return ripple; // return a positive value from the ripple counter
   }
   return 0x0; // otherwise the counter outputs are set low.
}

static WRITE8_HANDLER(cgdrw)
{

    ripple = data;

}

static WRITE8_HANDLER(u3_p0)
{

	u3_p0_w = data;
	//logerror("u3_p0_w: %02X\n",u3_p0_w);

}

static READ8_HANDLER(u3_p2)
{

    int u3_p2_ret= input_port_read(space->machine, "5002");
    int u3_p3_ret= input_port_read(space->machine, "5003");

    output_set_lamp_value(19, (u3_p2_ret >> 4) & 1); //auditkey light
    output_set_lamp_value(20, (u3_p3_ret >> 2) & 1); //jackpotkey light

    if (u3_p0_w&0x20) // DOPTE on
    {
        if (u3_p3_ret&0x02) // door closed
    	        u3_p2_ret = u3_p2_ret^0x08; // DOPTI on
	}

    if (inscrd==0)
    {
		inscrd=input_port_read(space->machine, "insertcoin");
	}

    if (inscrd==1)
		u3_p2_ret=u3_p2_ret^0x02;

    return u3_p2_ret;

}





/******************************************************************************

PERIPHERAL INTERFACE ADAPTER CONFIGURATION

PORTA - DALLAS DS1287 RTC
PORTB - MECHANICAL METERS

******************************************************************************/




/*****************************************************************************/
/* DALLAS DS1287 OR mc146818
******************************************************************************/


//input a


static READ8_HANDLER(mkiv_pia_ina)
{
    return mc146818_port_r(space,1);
}

//output a

static WRITE8_HANDLER(mkiv_pia_outa)
{
    if(rtc_data_strobe)
    {
        mc146818_port_w(space,1,data);
        //logerror("rtc protocol write data: %02X\n",data);

    }
    else
    {
        mc146818_port_w(space,0,data);
        //logerror("rtc protocol write address: %02X\n",data);

    }

}

//output ca2
static WRITE8_DEVICE_HANDLER(mkiv_pia_ca2)
{
     rtc_address_strobe = data;
    // logerror("address strobe %02X\n", address_strobe);
}


//output cb2
static WRITE8_DEVICE_HANDLER(mkiv_pia_cb2)
{
     rtc_data_strobe = data;
     //logerror("data strobe: %02X\n", data);
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
            if(emet[i])
            {
                //logerror("Mechanical meter %d pulse: %02d\n",i+1, emet[i]);
                sample_start(samples,i,0, FALSE); // pulse sound for mechanical meters
            }
     }

}

/* sound interface for playing mechanical meter sound */

static const char *const meter_sample_names[] =
{
    "*aristmk4",
	"tick.wav",
	0
};

static const samples_interface meter_samples_interface =
{
	5,	/* one for each meter - can pulse simultaneously */
	meter_sample_names
};



/******************************************************************************

VERSATILE INTERFACE ADAPTER CONFIGURATION

******************************************************************************/


static TIMER_CALLBACK(coin_input_reset)
{

	inscrd=0; //reset credit input after 150msec

}

static TIMER_CALLBACK(hopper_reset)
{

	hopper_motor=0x01;

}


// Port A read (SW1)

static READ8_DEVICE_HANDLER(via_a_r)
{
	int psg_ret=0;

    if (ay8910_1&0x03) // SW1 read.
    {
    	psg_ret = ay8910_r(devtag_get_device(device->machine, "ay1"), 0);
    	//logerror("PSG porta ay1 returned %02X\n",psg_ret);
    }

	else if (ay8910_2&0x03) //i don't think we read anything from Port A on ay2, Can be removed once game works ok.
	{
		psg_ret = ay8910_r(devtag_get_device(device->machine, "ay2"), 0);
		//logerror("PSG porta ay2 returned %02X\n",psg_ret);
	}
	return psg_ret;

}


static READ8_DEVICE_HANDLER(via_b_r)
{

    int ret=input_port_read(device->machine, "via_port_b");

// Not expecting to read anything from port B on the AY8910's ( controls BC1, BC2 and BDIR )
// However there are extra 4 bits not going to the AY8910's on the schematics, which get read from here.
//   OPTA1  - Bit4 - Coin optics - A
//   OPTB1  - Bit5 - Coin optics - B
//   HOPCO1 - Bit6 - Hopper counter
//   CBOPT1 - Bit7 - Cash box optics
/* Coin input... CBOPT2 goes LOW, then the optic detectors OPTA1 / OPTB1 detect the coin passing */
/* The timer causes one credit, per 150ms or so... */

    switch(inscrd)
	{

		case 0x00:
			break;

		case 0x01:
			ret=ret^0x10;
			inscrd++;
			break;

		case 0x02:
			ret=ret^0x20;
			inscrd++;
			timer_set(device->machine, ATTOTIME_IN_MSEC(150), NULL, 0, coin_input_reset);
			break;

		default:
			break; //timer will reset the input
	}


// if user presses collect.. send coins to hopper.

    switch(hopper_motor)
	{
		case 0x00:
			ret=ret^0x40;
			timer_set(device->machine, ATTOTIME_IN_MSEC(175), NULL, 0, hopper_reset);
			hopper_motor=0x02;
			break;
		case 0x01:
			break; //default
		case 0x02:
			ret=ret^0x40;
			break;
		default:
			break;
	}
    return ret;
}


static WRITE8_DEVICE_HANDLER(via_a_w)
{   //via_b_w will handle sending the data to the ay8910, so just write the data for it to use later

	//logerror("VIA port A write %02X\n",data);
	psg_data = data;

}

static WRITE8_DEVICE_HANDLER(via_b_w)
{
	ay8910_1 = ( data & 0x0F ) ; //only need first 4 bits per schematics
                                 //NOTE: when bit 4 is off, we write to AY1, when bit 4 is on, we write to AY2
	ay8910_2 = ay8910_1;

	if ( ay8910_2 & 0x08 ) // is bit 4 on ?
	{
        ay8910_2  = (ay8910_2 | 0x02) ; // bit 2 is turned on as bit 4 hooks to bit 2 in the schematics
        ay8910_1  = 0x00; // write only to ay2
	}
	else
	{
		ay8910_2 = 0x00; // write only to ay1
	}

	//only need bc1/bc2 and bdir so drop bit 4.

	ay8910_1 = (ay8910_1 & 0x07);
	ay8910_2 = (ay8910_2 & 0x07);

	//PSG ay1

	switch(ay8910_1)
	{

		case 0x00:	//INACT -Nothing to do here. Inactive PSG
			break;

		case 0x03:  //READ - Nothing to do here. The read happens in via_a_r
			break;

		case 0x06:  //WRITE
        {
        	ay8910_data_w( devtag_get_device(device->machine, "ay1"), 0 , psg_data );
        	//logerror("VIA Port A write data ay1: %02X\n",psg_data);
        	break;
        }

		case 0x07:  //LATCH Address (set register)
        {
        	ay8910_address_w( devtag_get_device(device->machine, "ay1"), 0 , psg_data );
        	//logerror("VIA Port B write register ay1: %02X\n",psg_data);
        	break;
        }

		default:
			//logerror("Unknown PSG state on ay1: %02X\n",ay8910_1);
			break;
	}

	//PSG ay2

	switch(ay8910_2)
	{

		case 0x00:	//INACT - Nothing to do here. Inactive PSG
			break;

		case 0x02:	//INACT - '010' Nothing to do here. Inactive PSG. this will only happen on ay2 due to the bit 2 swap on 'inactive'
			break;

		case 0x03:  //READ - Nothing to do here. The read happens in via_a_r
			break;

		case 0x06:  //WRITE
        {
        	ay8910_data_w( devtag_get_device(device->machine, "ay2"), 0 , psg_data );
        	//logerror("VIA Port A write data ay2: %02X\n",psg_data);
        	break;
        }

		case 0x07:  //LATCH Address (set register)
        {
            ay8910_address_w( devtag_get_device(device->machine, "ay2"), 0 , psg_data );
            //logerror("VIA Port B write register ay2: %02X\n",psg_data);
            break;
        }

		default:
			//logerror("Unknown PSG state on ay2: %02X\n",ay8910_2);
			break;

	}

}

static READ8_DEVICE_HANDLER(via_ca2_r)
{
    //logerror("Via Port CA2 read %02X\n",0) ;
	// CA2 is connected to CDSOL1 on schematics ?

	return 0 ;

}

static READ8_DEVICE_HANDLER(via_cb2_r)
{
    //logerror("Via Port CB2 read %02X\n",0) ;
    // CB2 is connected to HOPMO1 on schematics ?

	return 0 ;

}


static WRITE8_DEVICE_HANDLER(via_ca2_w)
{
    //logerror("Via Port CA2 write %02X\n",data) ;
}


static WRITE8_DEVICE_HANDLER(via_cb2_w)
{
// CB2 = hopper motor (HOPMO1). When it is 0x01, it is not running (active low)
	// when it goes to 0, we're expecting to coins to be paid out, handled in via_b_r
	// as soon as it is 1, HOPCO1 to remain 'ON'

	if (data==0x01)
		hopper_motor=data;
	else if (hopper_motor<0x02)
		hopper_motor=data;
}


// Lamp output

static WRITE8_DEVICE_HANDLER(pblp_out)
{
    output_set_lamp_value(1, (data) & 1);
    output_set_lamp_value(5, (data >> 1) & 1);
    output_set_lamp_value(9, (data >> 2) & 1);
    output_set_lamp_value(11,(data >> 3) & 1);
    output_set_lamp_value(3, (data >> 4) & 1);
    output_set_lamp_value(4, (data >> 5) & 1);
    output_set_lamp_value(2, (data >> 6) & 1);
    output_set_lamp_value(10,(data >> 7) & 1);

    //logerror("Lights port A %02X\n",data);
}

static WRITE8_DEVICE_HANDLER(pbltlp_out)
{
    output_set_lamp_value(8,  (data) & 1);
    output_set_lamp_value(12, (data >> 1) & 1);
    output_set_lamp_value(6,  (data >> 2) & 1);
    output_set_lamp_value(7,  (data >> 3) & 1);
    output_set_lamp_value(14, (data >> 4) & 1); // light tower
    output_set_lamp_value(15, (data >> 5) & 1); // light tower
    output_set_lamp_value(16, (data >> 6) & 1); // light tower
    output_set_lamp_value(17, (data >> 7) & 1); // light tower

    //logerror("Lights port B: %02X\n",data);


}

static WRITE8_HANDLER(mlamps)
{
    /* TAKE WIN AND GAMBLE LAMPS */
    output_set_lamp_value(18, (data >> 5) & 1);
    output_set_lamp_value(13, (data >> 6) & 1);

}


static WRITE8_DEVICE_HANDLER(zn434_w)
{

	// Introducted to prevent warning in log for write to AY1 PORT B
	// this is a write to the ZN434 DA convertors..

}


static ADDRESS_MAP_START( aristmk4_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&mkiv_vram) // video ram -  chips U49 / U50
	AM_RANGE(0x0800, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x1800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x1801, 0x1801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x2000, 0x3fff) AM_ROM  // graphics rom map
	AM_RANGE(0x4000, 0x4fff) AM_RAMBANK("bank1") AM_BASE_SIZE_GENERIC(nvram)

	AM_RANGE(0x5000, 0x5000) AM_WRITE(u3_p0)
	AM_RANGE(0x5002, 0x5002) AM_READ(u3_p2)
	AM_RANGE(0x5003, 0x5003) AM_READ_PORT("5003")
	AM_RANGE(0x5005, 0x5005) AM_READ(ldsw)
	AM_RANGE(0x500d, 0x500d) AM_READ_PORT("500d")
	AM_RANGE(0x500e, 0x500e) AM_READ_PORT("500e")
	AM_RANGE(0x500f, 0x500f) AM_READ_PORT("500f")
	AM_RANGE(0x5010, 0x501f) AM_DEVREADWRITE("via6522_0",via_r,via_w)
	AM_RANGE(0x5200, 0x5200) AM_READ_PORT("5200")
	AM_RANGE(0x5201, 0x5201) AM_READ_PORT("5201")
	AM_RANGE(0x527f, 0x5281) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5300, 0x5300) AM_READ_PORT("5300")
	AM_RANGE(0x5380, 0x5383) AM_DEVREADWRITE("pia6821_0",pia6821_r,pia6821_w)  // RTC data - PORT A , mechanical meters - PORTB ??
	AM_RANGE(0x5440, 0x5440) AM_WRITE(mlamps) // take win and gamble lamps
	AM_RANGE(0x5468, 0x5468) AM_READWRITE(cgdrr,cgdrw) // 4020 ripple counter outputs
	AM_RANGE(0x6000, 0xffff) AM_ROM  // game roms
ADDRESS_MAP_END

static INPUT_PORTS_START(aristmk4)

    /***********************************************************************************************************/

    PORT_START("via_port_b")
    PORT_DIPNAME( 0x40, 0x40, "HOPCO1" )
    PORT_DIPSETTING(    0x40, DEF_STR( On ) ) PORT_DIPLOCATION("AY:3")
    PORT_DIPNAME( 0x80, 0x80, "CBOPT1" )
    PORT_DIPSETTING(    0x80, DEF_STR( On ) ) PORT_DIPLOCATION("AY:4")

	/************************************************************************************************************

    5002

    ************************************************************************************************************/

    PORT_START("5002")
	PORT_DIPNAME( 0x01, 0x00, "HOPCO2") // coins out hopper 2 , why triggers logic door ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "CBOPT2") // coin in cash box 2
	PORT_DIPSETTING(    0x02, DEF_STR( On ) ) PORT_DIPLOCATION("5002:2")
	PORT_DIPNAME( 0x04, 0x00, "HOPHI2") // hopper 2 full
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DOPTI")  // photo optic door
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("AUDTSW") PORT_TOGGLE PORT_CODE(KEYCODE_K)
	PORT_DIPNAME( 0x20, 0x00, "HOPLO1") // hopper 1 low
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "HOPLO2") // hopper 2 low
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("ROBO/HOP RESET - PB6") PORT_CODE(KEYCODE_Z)


    /************************************************************************************************************

    5003

    ************************************************************************************************************/

    PORT_START("5003")
	PORT_DIPNAME( 0x01, 0x00, "OPTAUI") // opto audit in
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("DSWDT") PORT_TOGGLE PORT_CODE(KEYCODE_M) // main door switch
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("JKPTSW") PORT_TOGGLE PORT_CODE(KEYCODE_J) // jackpot reset switch
	PORT_DIPNAME( 0x08, 0x00, "HOPHI1") // hopper 1 full
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "OPTA2") // coin in a2
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "OPTB2") // coin in b2
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "PTRTAC") // printer taco
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "PTRHOM") // printer home
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/************************************************************************************************************

    5300

    ************************************************************************************************************/

    PORT_START("5300")
	PORT_DIPNAME( 0x01, 0x00, "5300-1")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5300-2")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5300-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "5300-4")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5300-5")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "5300-6")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5300-7")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "5300-8")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("500d")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5 CREDITS PER LINE") PORT_CODE(KEYCODE_T) // 5 credits per line
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("10 CREDITS PER LINE") PORT_CODE(KEYCODE_Y) // 10 credits per line
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("COLLECT") PORT_CODE(KEYCODE_Q)
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("RESERVE") PORT_CODE(KEYCODE_A) // reserve
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("GAMBLE") PORT_CODE(KEYCODE_U) // auto gamble & gamble
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("TAKE WIN") PORT_CODE(KEYCODE_J)
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_U)
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_I)

    PORT_START("500e")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1 CREDIT PER LINE") PORT_CODE(KEYCODE_W) // 1 credit per line
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PLAY 1") PORT_CODE(KEYCODE_S)
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2 CREDITS PER LINE") PORT_CODE(KEYCODE_E) // 2 credits per line
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PLAY 9") PORT_CODE(KEYCODE_H)
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3 CREDITS PER LINE") PORT_CODE(KEYCODE_R) // 3 credits per line
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PLAY 7") PORT_CODE(KEYCODE_G)
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PLAY 5") PORT_CODE(KEYCODE_F)
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PLAY 3") PORT_CODE(KEYCODE_D)

    PORT_START("500f")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_1)
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_2)
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_3)
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("AUX - PB5") PORT_CODE(KEYCODE_X)
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("MEM TEST - PB4") PORT_CODE(KEYCODE_C)
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("HOPPER TEST - PB3") PORT_CODE(KEYCODE_V)
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PRINT DATA - PB2") PORT_CODE(KEYCODE_B)
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("CLOCK INIT - PB1") PORT_CODE(KEYCODE_N)

	PORT_START("5200")
	PORT_DIPNAME( 0x01, 0x00, "5200-1")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5200-2")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5200-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "5200-4")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5200-5")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "5200-6")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5200-7")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "5200-8")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("5201")
	PORT_DIPNAME( 0x01, 0x00, "5201-1")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5201-2")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5201-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "5201-4") //fixes link offline error
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5201-5")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "5201-6")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5201-7")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "5201-8")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )



	PORT_START("insertcoin")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Insert Credit")

	/************************************************************************************************************

    Logic Door switch

    ************************************************************************************************************/

    PORT_START("CGDRSW")
    PORT_DIPNAME( 0x10, 0x10, "CGDRSW" ) PORT_DIPLOCATION("CGDRSW:1")	/* toggle switch */
    PORT_DIPSETTING(    0x00, "Open" )
	PORT_DIPSETTING(    0x10, "Closed" )

	/************************************** LINKS ***************************************************************/

    PORT_START("LK13")
    PORT_DIPNAME( 0x10, 0x10, "Speed Select" ) PORT_DIPLOCATION("LK13:1")
    PORT_DIPSETTING(    0x00, "3 Mhz" )
	PORT_DIPSETTING(    0x10, "1.5 Mhz" )


	/********************************* Dip switch for background color *************************************************/

	PORT_START("SW7")
	PORT_DIPNAME( 0x01, 0x01, "SW7 - U22 BG COLOR" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW7:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "SW7 - U47 BG COLOR" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW7:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )


	/********************************* 9 way control rotary switches ***************************************************/

	PORT_START("SW3")
    PORT_DIPNAME( 0x0f, 0x00, "SW3 - M/C NO" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )

	PORT_START("SW4")
    PORT_DIPNAME( 0x0f, 0x00, "SW4 - M/C NO" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )

	PORT_START("SW5")
    PORT_DIPNAME( 0x0f, 0x00, "SW5 - M/C NO" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )

	PORT_START("SW6")
    PORT_DIPNAME( 0x0f, 0x00, "SW6 - M/C NO" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )

	/***************** DIP SWITCHES **********************************************************************/

    PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1 - Maxbet rejection" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW1 - Hopper pay limit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW1 - Hopper pay limit - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW1 - Hopper pay limit - S3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW1 - Cash credit option" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW1 - Link Jackpot - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW1 - Link Jackpot - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW1 - Auto spin" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2 - Maximum credit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2 - Maximum credit - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2 - Maximum credit - S3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2 - Jackpot limit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW2 - Jackpot limit - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2 - Jackpot limit - S3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW2 - Auto J/P payout" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2 - Unconnected" )
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
	GFXDECODE_ENTRY("tile_gfx",0x0,layout8x8x6, 0, 8 )
GFXDECODE_END

static const ay8910_interface ay8910_config1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW1"),
    DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(zn434_w) // Port write to set Vout of the DA convertors ( 2 x ZN434 )
};

static const ay8910_interface ay8910_config2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL, // Port A read
	DEVCB_NULL, // Port B read
	DEVCB_HANDLER(pblp_out),   // Port A write - goes to lamps on the buttons x8
	DEVCB_HANDLER(pbltlp_out)  // Port B write - goes to lamps on the buttons x4 and light tower x4
};

static WRITE8_DEVICE_HANDLER(firq)
{
       cputag_set_input_line(device->machine, "maincpu", M6809_FIRQ_LINE, data ? ASSERT_LINE : CLEAR_LINE);
}

static const via6522_interface via_interface =
{
	/*inputs : A/B               */ DEVCB_HANDLER(via_a_r),DEVCB_HANDLER(via_b_r),
	/*inputs : CA/B1,CA/B2       */ DEVCB_NULL,DEVCB_NULL,DEVCB_HANDLER(via_ca2_r),DEVCB_HANDLER(via_cb2_r),
	/*outputs: A/B               */ DEVCB_HANDLER(via_a_w), DEVCB_HANDLER(via_b_w),
	/*outputs: CA/B1,CA/B2       */ DEVCB_NULL,DEVCB_NULL,DEVCB_HANDLER(via_ca2_w),DEVCB_HANDLER(via_cb2_w),
	/*irq                        */ DEVCB_HANDLER(firq)

	// CA1 is connected to +5V, CB1 is not connected.
};

static const pia6821_interface aristmk4_pia1_intf =
{
    DEVCB_MEMORY_HANDLER("maincpu",PROGRAM,mkiv_pia_ina),     /* port A in */
	DEVCB_NULL, 	/* port B in */
    DEVCB_NULL,     /* line CA1 in */
    DEVCB_NULL,     /* line CB1 in */
    DEVCB_NULL,     /* line CA2 in */
    DEVCB_NULL,     /* line CB2 in */
    DEVCB_MEMORY_HANDLER("maincpu",PROGRAM,mkiv_pia_outa),     /* port A out */
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

/* read m/c number */

static READ8_DEVICE_HANDLER(pa1_r)
{
	return (input_port_read(device->machine, "SW3") << 4) + input_port_read(device->machine, "SW4");
}

static READ8_DEVICE_HANDLER(pb1_r)
{
	return (input_port_read(device->machine, "SW5") << 4) + input_port_read(device->machine, "SW6");
}

static READ8_DEVICE_HANDLER(pc1_r)
{
    return 0;
}

static const ppi8255_interface ppi8255_intf1 =
{
	DEVCB_HANDLER(pa1_r),
	DEVCB_HANDLER(pb1_r),
	DEVCB_HANDLER(pc1_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
};

/* same as Casino Winner HW */
static PALETTE_INIT( aristmk4 )
{
	int i;

	for (i = 0;i < machine->total_colors();i++)
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



static DRIVER_INIT( aristmk4 )
{
    mc146818_init(machine, MC146818_IGNORE_CENTURY);
	shapeRomPtr = (UINT8 *)memory_region(machine, "tile_gfx");
    memcpy(shapeRom,shapeRomPtr,sizeof(shapeRom)); // back up
}

static MACHINE_START( aristmk4 )
{

	samples = devtag_get_device(machine, "samples");
    state_save_register_global_pointer(machine, nvram,0x1000); // nvram
}


static MACHINE_RESET( aristmk4 )
{
    /* mark 4 has a link on the motherboard to switch between 1.5Mhz and 3Mhz clock speed */
    switch(input_port_read(machine, "LK13"))  // cpu speed cotrol.. 3mhz or 1.5mhz
    {
        case  0x00:
            cpu_set_clock(devtag_get_device(machine, "maincpu"), MAIN_CLOCK/4);  // 3 Mhz
            break;
        case  0x10:
            cpu_set_clock(devtag_get_device(machine, "maincpu"), MAIN_CLOCK/8);  // 1.5 Mhz
            break;
    }

}


static MACHINE_DRIVER_START( aristmk4 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, MAIN_CLOCK/8) // 1.5mhz (goldenc needs a bit faster for some reason)
	MDRV_CPU_PROGRAM_MAP(aristmk4_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)
	MDRV_MACHINE_START(aristmk4)
	MDRV_NVRAM_HANDLER( mc146818 )
    MDRV_MACHINE_RESET(aristmk4 )
	MDRV_NVRAM_HANDLER(generic_0fill)

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

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf1 )
	MDRV_VIA6522_ADD("via6522_0", 0, via_interface)	/* 1 MHz.(only 1 or 2 MHz.are valid) */
	MDRV_PIA6821_ADD("pia6821_0", aristmk4_pia1_intf)
    MDRV_MC6845_ADD("crtc", MC6845, MAIN_CLOCK/8, mc6845_intf)

    MDRV_SPEAKER_STANDARD_MONO("mono")

    // the Mark IV has X 2 AY8910 sound chips which are tied to the VIA
    MDRV_SOUND_ADD("ay1", AY8910 , MAIN_CLOCK/8)
    MDRV_SOUND_CONFIG(ay8910_config1)
    MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

    MDRV_SOUND_ADD("ay2", AY8910 , MAIN_CLOCK/8)
    MDRV_SOUND_CONFIG(ay8910_config2)
    MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

    MDRV_SOUND_ADD("samples", SAMPLES, 0)
    MDRV_SOUND_CONFIG(meter_samples_interface)
    MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.05)


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
	ROM_LOAD("a_u87.bin", 0x06000, 0x2000, CRC(03c2890f) SHA1(10d479b7ccece813676ad815a96169bbf259c49d)) // game code
	ROM_LOAD("a_u86.bin", 0x08000, 0x8000, CRC(36125194) SHA1(dc681dc60b25893ca3ee101f6813c22b914771f5)) // game code

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
	ROM_REGION(0xc000, "tile_gfx", ROMREGION_ERASEFF)
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
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
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
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
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

ROM_START( wtigernz )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(9492b242) SHA1(26bb14cba8e8c3cdbcb4b4903da9592b0a1f8cb3)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(f639ef56) SHA1(5d49deee95df29cd4f5c69fea01bb752aaf2ce99)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(08624625) SHA1(3c052220b171f8ef009484f0ea38074b538f542b)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(4bce2fa1) SHA1(8c25cd51ea61a4a9ff1238d1617e38b2cd298c53))
	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(da141f20) SHA1(e0ebeeff2e085a30032d29748f5aa6116428aaa8))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(13783f87) SHA1(662f6afdd027c3d139d7dfcd45a4a2a5a2bf2101))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(7dfd06ec) SHA1(51fbc3d24e270edb8de432a99ca28695e42e72a6))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(177a45ea) SHA1(6b044f88c79de571a007fb71ff2f99587babe474))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( ffortune )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(45047c35) SHA1(4af572a23bca33a360c4711f24fb113167f90447)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(9a8b0eae) SHA1(ffd0419566c2352e3d750040405a760bd75c87d5)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(f8bad3c2) SHA1(c3cffeaa34c9c7e8127f69cd1dcbc9d56bd32ed9)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(7caba194) SHA1(b0f3f4464ba6a89b572c257b87939457d4f0b2d4))
	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(195967f0) SHA1(f76ba3c4e8b12d480ab1e4c1147bd7971ce8d688))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(dc44c3ab) SHA1(74f6230798832f321f7c53c161eac6c552689113))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(b0a04c83) SHA1(57247867db6417c525c4c3cdcc409523037e00fd))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(cd24ee39) SHA1(12798e14f7f6308e130da824ffc7c577a36cef04))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( autmoon )

	ROM_REGION(0x10000, "maincpu", 0 )
	 /* VIDEO AND SOUND EPROM */
	ROM_LOAD("u59.bin", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // sound and video rom

	 /* GAME EPROMS */
	ROM_LOAD("u87.bin", 0x06000, 0x2000, CRC(30ca1eed) SHA1(540635a8b94c14aefa1d8404226d9e1046776111)) // game code
	ROM_LOAD("u86.bin", 0x08000, 0x8000, CRC(8153a60b) SHA1(54b8a0467645161d827bf8cb9fbceb0d00f9639f)) // game code

	/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("u20.bin", 0x00000, 0x2000, CRC(fcbbc62e) SHA1(794a7d974e67183468a77a6a81a6f05e0569e229)) // gfx
	ROM_LOAD("u21.bin", 0x02000, 0x2000, CRC(9e6f940e) SHA1(1ad9e7c6231a8d16e868a79d313efccbd1ff58ee))
	ROM_LOAD("u22.bin", 0x04000, 0x2000, CRC(1a2ff3a9) SHA1(bddfc3eedcdf9237a31a4b42d062e986beafed39))
	ROM_LOAD("u45.bin", 0x06000, 0x2000, CRC(c8d29af8) SHA1(e35f67d6708b26c93617c967aa50c629f7019788))
	ROM_LOAD("u46.bin", 0x08000, 0x2000, CRC(fa126a77) SHA1(31d6096c58653a45176b6373835f83c8f2c46f80))
	ROM_LOAD("u47.bin", 0x0a000, 0x2000, CRC(50307da0) SHA1(6418a51cf915b37fa11f47d000e4229dacf95951))

	 /* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("u71.bin", 0x0000, 0x0200,  CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END



GAMEL( 1994, eforest,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Enchanted Forest - 12XF528902", GAME_NOT_WORKING,layout_aristmk4)
GAMEL( 1995, eforesta,eforest,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Enchanted Forest - 4VXFC818", 0,layout_aristmk4 )
GAMEL( 1996, eforestb,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Enchanted Forest - 3VXFC5343 (New Zealand)", 0,layout_aristmk4 )
GAMEL( 1994, 3bagflnz,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "3 Bags Full - 3VXFC5345 (New Zealand)", 0,layout_aristmk4 )
GAMEL( 1996, blkrhino,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Black Rhino - 3VXFC5344 (New Zealand)", 0,layout_aristmk4 )
GAMEL( 1996, kgbird,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "K.G Bird - 4VXFC5341 (New Zealand, 87.98%)", 0,layout_aristmk4 )
GAMEL( 1996, kgbirda,  kgbird,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "K.G Bird - 4VXFC5341 (New Zealand, 91.97%)", 0,layout_aristmk4 )
GAMEL( 1998, swtht2nz,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Sweet Hearts II - 1VXFC5461 (New Zealand)", 0,layout_aristmk4 )
GAMEL( 1996, goldenc,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Golden Canaries - 1VXFC5462", 0,layout_aristmk4 )
GAMEL( 1996, topgear,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Top Gear - 4VXFC969", GAME_NOT_WORKING,layout_aristmk4 )
GAMEL( 1996, wtigernz,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "White Tiger - 3VXFC5342 (New Zealand)", 0,layout_aristmk4 )
GAMEL( 1998, phantomp,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Phantom Pays - 4VXFC5431 (New Zealand)", 0,layout_aristmk4 )
GAMEL( 2000, coralr2,		0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Coral Riches II - 1VXFC5472 (New Zealand)", 0,layout_aristmk4 )
GAMEL( 1999, ffortune,      0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Fantasy Fortune", 0,layout_aristmk4 )
GAMEL( 1999, autmoon,       0,aristmk4, aristmk4, aristmk4, ROT0,  "Aristocrat", "Autumn Moon", 0,layout_aristmk4 )
