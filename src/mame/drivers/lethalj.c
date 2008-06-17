/***************************************************************************

    The Game Room Lethal Justice hardware

    driver by Aaron Giles

    Games supported:
        * Lethal Justice
        * Egg Venture
        * Ripper Ribit
        * Chicken Farm
        * Crazzy Clownz

    Known bugs:
        * some DIP switches not understood

****************************************************************************

         EU21     EU18     EU20   32.000MHz
         M6295    M6295    M6295       Xilinx
   Dip-8 2.000Mhz 2.000Mhz 2.000Mhz
J        VC9                 GUNCN
A  Dip-4 VC8                 Xilinx     EGR4
M               Mach210                 EGR6.3
M   M5M442256x4 Mach210 Mach210 Mach210 EGR3
A          11.2896MHz                   EGR2
     TMS34010           W241024x4       EGR1
 2803A                                  EGR5.3
    40.000MHz   Bt121 Mach210 Mach210 Mach210


Chips:
 TMS34010FNL-40     Main CPU
 Xilinx XC3042-70   Field Programmable Gate Array
 Bt121KPJ80         Triple 8-bit 80MHz VideoDAC
 AMD Mach210A-10JC  Programmable Logic Device (CPLD)
 ST ULN2803A        8 Darlington Transistor Array with common emitter

Note 1: Lethal Justice uses a 11.0592MHz OSC instead of the 11.2896MHz
Note 2: Lethal Justice uses a TMS34010FNL-50 instead of the TMS34010FNL-40

***************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "lethalj.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"


#define MASTER_CLOCK		XTAL_40MHz
#define SOUND_CLOCK			XTAL_2MHz

#define VIDEO_CLOCK			XTAL_11_289MHz
#define VIDEO_CLOCK_LETHALJ	XTAL_11_0592MHz



/*************************************
 *
 *  Custom inputs
 *
 *************************************/

static CUSTOM_INPUT( ticket_status )
{
	return ticket_dispenser_0_r(field->port->machine, 0) >> 7;
}


static CUSTOM_INPUT( cclownz_paddle )
{
	int value = input_port_read(field->port->machine, "PADDLE");
	return ((value << 4) & 0xf00) | (value & 0x00f);
}



/*************************************
 *
 *  Output controls
 *
 *************************************/

static WRITE16_HANDLER( ripribit_control_w )
{
	coin_counter_w(0, data & 1);
	ticket_dispenser_0_w(machine, 0, ((data >> 1) & 1) << 7);
	output_set_lamp_value(0, (data >> 2) & 1);
}


static WRITE16_HANDLER( cfarm_control_w )
{
	ticket_dispenser_0_w(machine, 0, ((data >> 0) & 1) << 7);
	output_set_lamp_value(0, (data >> 2) & 1);
	output_set_lamp_value(1, (data >> 3) & 1);
	output_set_lamp_value(2, (data >> 4) & 1);
	coin_counter_w(0, (data >> 7) & 1);
}


static WRITE16_HANDLER( cclownz_control_w )
{
	ticket_dispenser_0_w(machine, 0, ((data >> 0) & 1) << 7);
	output_set_lamp_value(0, (data >> 2) & 1);
	output_set_lamp_value(1, (data >> 4) & 1);
	output_set_lamp_value(2, (data >> 5) & 1);
	coin_counter_w(0, (data >> 6) & 1);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( lethalj_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM
	AM_RANGE(0x04000000, 0x0400000f) AM_READWRITE(OKIM6295_status_0_lsb_r, OKIM6295_data_0_lsb_w)
	AM_RANGE(0x04000010, 0x0400001f) AM_READWRITE(OKIM6295_status_1_lsb_r, OKIM6295_data_1_lsb_w)
	AM_RANGE(0x04100000, 0x0410000f) AM_READWRITE(OKIM6295_status_2_lsb_r, OKIM6295_data_2_lsb_w)
//  AM_RANGE(0x04100010, 0x0410001f) AM_READNOP     /* read but never examined */
	AM_RANGE(0x04200000, 0x0420001f) AM_WRITENOP	/* clocks bits through here */
	AM_RANGE(0x04300000, 0x0430007f) AM_READ(lethalj_gun_r)
	AM_RANGE(0x04400000, 0x0440000f) AM_WRITENOP	/* clocks bits through here */
	AM_RANGE(0x04500010, 0x0450001f) AM_READ(input_port_0_word_r)
	AM_RANGE(0x04600000, 0x0460000f) AM_READ(input_port_1_word_r)
	AM_RANGE(0x04700000, 0x0470007f) AM_WRITE(lethalj_blitter_w)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xc0000240, 0xc000025f) AM_WRITENOP	/* seems to be a bug in their code, one of many. */
	AM_RANGE(0xff800000, 0xffffffff) AM_ROM AM_REGION(REGION_USER1, 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( lethalj )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )		/* ??? Seems to be rigged up to the auto scroll, and acts as a fast forward*/
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x00c0, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Lives ))
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0300, "5" )
	PORT_DIPNAME( 0x0c10, 0x0010, "Right Gun Offset" )
	PORT_DIPSETTING(      0x0000, "-4" )
	PORT_DIPSETTING(      0x0400, "-3" )
	PORT_DIPSETTING(      0x0800, "-2" )
	PORT_DIPSETTING(      0x0c00, "-1" )
	PORT_DIPSETTING(      0x0010, "0" )
	PORT_DIPSETTING(      0x0410, "+1" )
	PORT_DIPSETTING(      0x0810, "+2" )
	PORT_DIPSETTING(      0x0c10, "+3" )
	PORT_DIPNAME( 0x3020, 0x0020, "Left Gun Offset" )
	PORT_DIPSETTING(      0x0000, "-4" )
	PORT_DIPSETTING(      0x1000, "-3" )
	PORT_DIPSETTING(      0x2000, "-2" )
	PORT_DIPSETTING(      0x3000, "-1" )
	PORT_DIPSETTING(      0x0020, "0" )
	PORT_DIPSETTING(      0x1020, "+1" )
	PORT_DIPSETTING(      0x2020, "+2" )
	PORT_DIPSETTING(      0x3020, "+3" )
	PORT_DIPNAME( 0x4000, 0x0000, "DIP E" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x4000, "1" )
	PORT_DIPNAME( 0x8000, 0x8000, "Global Gun Offset" )
	PORT_DIPSETTING(      0x0000, "-2.5" )
	PORT_DIPSETTING(      0x8000, "+0" )

	PORT_START_TAG("LIGHT0_X")			/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START_TAG("LIGHT0_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START_TAG("LIGHT1_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START_TAG("LIGHT1_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( eggventr )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0070, 0x0000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x0040, DEF_STR( 8C_1C ))
	PORT_DIPSETTING(      0x0030, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x0070, DEF_STR( Free_Play ))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) // Verified Correct
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0300, "6" )
	PORT_DIPNAME( 0x0c00, 0x0800, DEF_STR( Difficulty ) ) // According to info from The Gameroom
	PORT_DIPSETTING(      0x0c00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Slot Machine" ) // Verified Correct - Unused for the Deluxe version?? Yes, the slot machine
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) // is present in the code as a 'bonus stage' (when the egg reaches Vegas?),
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) ) // but not actually called (EC).
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x7f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START_TAG("LIGHT0_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START_TAG("LIGHT0_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START_TAG("LIGHT1_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START_TAG("LIGHT1_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( eggvntdx )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0070, 0x0000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x0040, DEF_STR( 8C_1C ))
	PORT_DIPSETTING(      0x0030, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x0070, DEF_STR( Free_Play ))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) // Verified Correct
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0300, "6" )
	PORT_DIPNAME( 0x0c00, 0x0800, DEF_STR( Difficulty ) ) // According to info from The Gameroom
	PORT_DIPSETTING(      0x0c00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x7f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START_TAG("LIGHT0_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START_TAG("LIGHT0_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START_TAG("LIGHT1_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START_TAG("LIGHT1_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( ripribit )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(ticket_status, NULL)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ))
	PORT_DIPNAME( 0x0700, 0x0200, "Starting Jackpot" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0200, "10" )
	PORT_DIPSETTING(      0x0300, "15" )
	PORT_DIPSETTING(      0x0400, "20" )
	PORT_DIPSETTING(      0x0500, "25" )
	PORT_DIPSETTING(      0x0600, "30" )
	PORT_DIPSETTING(      0x0700, "35" )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x1000, "2" )
	PORT_DIPSETTING(      0x1800, "3" )
	PORT_DIPNAME( 0xe000, 0x8000, "Points per Ticket" )
	PORT_DIPSETTING(      0xe000, "200" )
	PORT_DIPSETTING(      0xc000, "300" )
	PORT_DIPSETTING(      0xa000, "400" )
	PORT_DIPSETTING(      0x8000, "500" )
	PORT_DIPSETTING(      0x6000, "600" )
	PORT_DIPSETTING(      0x4000, "700" )
	PORT_DIPSETTING(      0x2000, "800" )
	PORT_DIPSETTING(      0x0000, "1000" )

	PORT_START_TAG("LIGHT0_X")				/* fake analog X */
	PORT_START_TAG("LIGHT0_Y")				/* fake analog Y */
	PORT_START_TAG("LIGHT1_X")				/* fake analog X */
	PORT_START_TAG("LIGHT1_Y")				/* fake analog Y */
INPUT_PORTS_END


static INPUT_PORTS_START( cfarm )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ))
    PORT_DIPNAME( 0x0700, 0x0300, "Starting Jackpot" )
    PORT_DIPSETTING(      0x0000, "0" )
    PORT_DIPSETTING(      0x0100, "5" )
    PORT_DIPSETTING(      0x0200, "8" )
    PORT_DIPSETTING(      0x0300, "10" )
    PORT_DIPSETTING(      0x0400, "12" )
    PORT_DIPSETTING(      0x0500, "15" )
    PORT_DIPSETTING(      0x0600, "18" )
    PORT_DIPSETTING(      0x0700, "20" )
    PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0000, "0" )
    PORT_DIPSETTING(      0x0800, "1" )
    PORT_DIPSETTING(      0x1000, "2" )
    PORT_DIPSETTING(      0x1800, "3" )
    PORT_DIPNAME( 0xe000, 0x8000, "Eggs per Ticket" )
    PORT_DIPSETTING(      0xe000, "1" )
    PORT_DIPSETTING(      0xc000, "2" )
    PORT_DIPSETTING(      0xa000, "3" )
    PORT_DIPSETTING(      0x8000, "4" )
    PORT_DIPSETTING(      0x6000, "5" )
    PORT_DIPSETTING(      0x4000, "6" )
    PORT_DIPSETTING(      0x2000, "8" )
    PORT_DIPSETTING(      0x0000, "10" )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(ticket_status, NULL)
	PORT_BIT( 0x0006, IP_ACTIVE_LOW, IPT_UNUSED )
 	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
 	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
 	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("LIGHT0_X")				/* fake analog X */
	PORT_START_TAG("LIGHT0_Y")				/* fake analog Y */
	PORT_START_TAG("LIGHT1_X")				/* fake analog X */
	PORT_START_TAG("LIGHT1_Y")				/* fake analog Y */
INPUT_PORTS_END


static INPUT_PORTS_START( cclownz )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ))
    PORT_DIPNAME( 0x0700, 0x0700, "Starting Jackpot" )
    PORT_DIPSETTING(      0x0000, "0" )
    PORT_DIPSETTING(      0x0100, "2" )
    PORT_DIPSETTING(      0x0200, "5" )
    PORT_DIPSETTING(      0x0300, "8" )
    PORT_DIPSETTING(      0x0400, "10" )
    PORT_DIPSETTING(      0x0500, "15" )
    PORT_DIPSETTING(      0x0600, "20" )
    PORT_DIPSETTING(      0x0700, "30" )
    PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0000, "0" )
    PORT_DIPSETTING(      0x0800, "1" )
    PORT_DIPSETTING(      0x1000, "2" )
    PORT_DIPSETTING(      0x1800, "3" )
    PORT_DIPNAME( 0xe000, 0x8000, "Points per Ticket" )
    PORT_DIPSETTING(      0xe000, "700" )
    PORT_DIPSETTING(      0xc000, "900" )
    PORT_DIPSETTING(      0xa000, "1200" )
    PORT_DIPSETTING(      0x8000, "1500" )
    PORT_DIPSETTING(      0x6000, "1800" )
    PORT_DIPSETTING(      0x4000, "2100" )
    PORT_DIPSETTING(      0x2000, "2400" )
    PORT_DIPSETTING(      0x0000, "3000" )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x0f0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(cclownz_paddle, NULL)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(ticket_status, NULL)
	PORT_BIT( 0x0060, IP_ACTIVE_LOW, IPT_UNUSED )
 	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("LIGHT0_X")				/* fake analog X */
	PORT_START_TAG("LIGHT0_Y")				/* fake analog Y */
	PORT_START_TAG("LIGHT1_X")				/* fake analog X */
	PORT_START_TAG("LIGHT1_Y")				/* fake analog Y */

	PORT_START_TAG("PADDLE")
 	PORT_BIT( 0x00ff, 0x0000, IPT_PADDLE ) PORT_PLAYER(1) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_CENTERDELTA(0) PORT_REVERSE
INPUT_PORTS_END



/*************************************
 *
 *  34010 configuration
 *
 *************************************/

static const tms34010_config tms_config =
{
	FALSE,							/* halt on reset */
	"main",							/* the screen operated on */
	VIDEO_CLOCK,					/* pixel clock */
	1,								/* pixels per clock */
	lethalj_scanline_update,		/* scanline update */
	NULL,							/* generate interrupt */
	NULL,							/* write to shiftreg function */
	NULL							/* read from shiftreg function */
};

static const tms34010_config tms_config_lethalj =
{
	FALSE,							/* halt on reset */
	"main",							/* the screen operated on */
	VIDEO_CLOCK_LETHALJ,			/* pixel clock */
	1,								/* pixels per clock */
	lethalj_scanline_update,		/* scanline update */
	NULL,							/* generate interrupt */
	NULL,							/* write to shiftreg function */
	NULL							/* read from shiftreg function */
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( gameroom )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", TMS34010, MASTER_CLOCK)
	MDRV_CPU_CONFIG(tms_config)
	MDRV_CPU_PROGRAM_MAP(lethalj_map,0)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(VIDEO_CLOCK, 701, 0, 512, 263, 0, 236)

	MDRV_VIDEO_START(lethalj)
	MDRV_VIDEO_UPDATE(tms340x0)

	MDRV_PALETTE_INIT(RRRRR_GGGGG_BBBBB)
	MDRV_PALETTE_LENGTH(32768)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, SOUND_CLOCK)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.26)

	MDRV_SOUND_ADD(OKIM6295, SOUND_CLOCK)
	MDRV_SOUND_CONFIG(okim6295_interface_region_2_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.26)

	MDRV_SOUND_ADD(OKIM6295, SOUND_CLOCK)
	MDRV_SOUND_CONFIG(okim6295_interface_region_3_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.26)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( lethalj )
	MDRV_IMPORT_FROM( gameroom )

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_CONFIG(tms_config_lethalj)

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_RAW_PARAMS(VIDEO_CLOCK_LETHALJ, 689, 0, 512, 259, 0, 236)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( lethalj )
	ROM_REGION16_LE( 0x100000, REGION_USER1, 0 )		/* 34010 code */
	ROM_LOAD16_BYTE( "vc8",  0x000000, 0x080000, CRC(8d568e1d) SHA1(e4dd3794789f9ccd7be8374978a3336f2b79136f) )
	ROM_LOAD16_BYTE( "vc9",  0x000001, 0x080000, CRC(8f22add4) SHA1(e773d3ae9cf512810fc266e784d21ed115c8830c) )

	ROM_REGION16_LE( 0x600000, REGION_GFX1, 0 )			/* graphics data */
	ROM_LOAD16_BYTE( "gr1",  0x000000, 0x100000, CRC(27f7b244) SHA1(628b29c066e217e1fe54553ea3ed98f86735e262) )
	ROM_LOAD16_BYTE( "gr2",  0x000001, 0x100000, CRC(1f25d3ab) SHA1(bdb8a3c546cdee9a5630c47b9c5079a956e8a093) )
	ROM_LOAD16_BYTE( "gr4",  0x200000, 0x100000, CRC(c5838b4c) SHA1(9ad03d0f316eb31fdf0ca6f65c02a27d3406d072) )
	ROM_LOAD16_BYTE( "gr3",  0x200001, 0x100000, CRC(ba9fa057) SHA1(db6f11a8964870f04f94fef6f1b1a58168a942ad) )
	ROM_LOAD16_BYTE( "gr6",  0x400000, 0x100000, CRC(51c99b85) SHA1(9a23bf21a73d2884b49c64a8f42c288534c79dc5) )
	ROM_LOAD16_BYTE( "gr5",  0x400001, 0x100000, CRC(80dda9b5) SHA1(d8a79cad112bc7d9e4ba31a950e4807581f3bf46) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )				/* sound data */
	ROM_LOAD( "sound1.u20", 0x00000, 0x40000, CRC(7d93ca66) SHA1(9e1dc0efa5d0f770c7e1f10de56fbf5620dea437) )

	ROM_REGION( 0x40000, REGION_SOUND2, 0 )				/* sound data */
	ROM_LOAD( "sound1.u21",    0x00000, 0x40000, CRC(7d3beae0) SHA1(5ec753c5fd5ca0f9492c9e274703a1aa758062a7) )

	ROM_REGION( 0x40000, REGION_SOUND3, 0 )				/* sound data */
	ROM_LOAD( "sound1.u18", 0x00000, 0x40000, CRC(7d93ca66) SHA1(9e1dc0efa5d0f770c7e1f10de56fbf5620dea437) )
ROM_END


ROM_START( eggventr )
	ROM_REGION16_LE( 0x100000, REGION_USER1, 0 )		/* 34010 code */
	ROM_LOAD16_BYTE( "eggvc8.10", 0x000000, 0x020000, CRC(225d1164) SHA1(b0dc55f2e8ded1fe7874de05987fcf879772289e) )
	ROM_LOAD16_BYTE( "eggvc9.10", 0x000001, 0x020000, CRC(42f6e904) SHA1(11be8e7383a218aac0e1a63236bbdb7cca0993bf) )
	ROM_COPY( REGION_USER1, 0x000000, 0x040000, 0x040000 )
	ROM_COPY( REGION_USER1, 0x000000, 0x080000, 0x080000 )

	ROM_REGION16_LE( 0x600000, REGION_GFX1, 0 )			/* graphics data */
	ROM_LOAD16_BYTE( "egr1.bin",  0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.bin",  0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "egr4.bin",  0x200000, 0x100000, CRC(4ea5900e) SHA1(20341337ee3c6c22580c52312156b818f4187693) )
	ROM_LOAD16_BYTE( "egr3.bin",  0x200001, 0x100000, CRC(3f8dfc73) SHA1(83a168069f896ea7e67a97c6d591d09b19d5f486) )
	ROM_LOAD16_BYTE( "egr6.3",    0x400000, 0x100000, CRC(f299d818) SHA1(abbb333c43675d34c59201b5d297779cfea8b092) )
	ROM_LOAD16_BYTE( "egr5.3",    0x400001, 0x100000, CRC(ebfca07b) SHA1(20465d14b41d99651166f221057737d7b3cc770c) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )				/* sound data */
	ROM_LOAD( "eu20.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )

	ROM_REGION( 0x40000, REGION_SOUND2, 0 )				/* sound data */
	ROM_LOAD( "eu21.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )

	ROM_REGION( 0x40000, REGION_SOUND3, 0 )				/* sound data */
	ROM_LOAD( "eu18.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )
ROM_END


ROM_START( eggvent7 )
	ROM_REGION16_LE( 0x100000, REGION_USER1, 0 )		/* 34010 code */
	ROM_LOAD16_BYTE( "eggvc8.7", 0x000000, 0x020000, CRC(99999899) SHA1(e3908600fa711baa7f7562f86498ec7e988a5bea) )
	ROM_LOAD16_BYTE( "eggvc9.7", 0x000001, 0x020000, CRC(1b608155) SHA1(256dd981515d57f806a3770bdc6ff46b9000f7f3) )
	ROM_COPY( REGION_USER1, 0x000000, 0x040000, 0x040000 )
	ROM_COPY( REGION_USER1, 0x000000, 0x080000, 0x080000 )

	ROM_REGION16_LE( 0x600000, REGION_GFX1, 0 )			/* graphics data */
	ROM_LOAD16_BYTE( "egr1.bin",  0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.bin",  0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "egr4.bin",  0x200000, 0x100000, CRC(4ea5900e) SHA1(20341337ee3c6c22580c52312156b818f4187693) )
	ROM_LOAD16_BYTE( "egr3.bin",  0x200001, 0x100000, CRC(3f8dfc73) SHA1(83a168069f896ea7e67a97c6d591d09b19d5f486) )
	ROM_LOAD16_BYTE( "egr6.3",    0x400000, 0x100000, CRC(f299d818) SHA1(abbb333c43675d34c59201b5d297779cfea8b092) )
	ROM_LOAD16_BYTE( "egr5.3",    0x400001, 0x100000, CRC(ebfca07b) SHA1(20465d14b41d99651166f221057737d7b3cc770c) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )				/* sound data */
	ROM_LOAD( "eu20.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )

	ROM_REGION( 0x40000, REGION_SOUND2, 0 )				/* sound data */
	ROM_LOAD( "eu21.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )

	ROM_REGION( 0x40000, REGION_SOUND3, 0 )				/* sound data */
	ROM_LOAD( "eu18.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )
ROM_END


ROM_START( eggvntdx )
	ROM_REGION16_LE( 0x100000, REGION_USER1, 0 )		/* 34010 code */
	ROM_LOAD16_BYTE( "eggdlx.vc8", 0x000000, 0x080000, CRC(d7f56141) SHA1(3c16b509fd1c763e452c27084fb0e90cde3947f7) )
	ROM_LOAD16_BYTE( "eggdlx.vc9", 0x000001, 0x080000, CRC(cc5f122e) SHA1(e719a3937378df605cdb86c59a534808473c8f90) )

	ROM_REGION16_LE( 0x600000, REGION_GFX1, 0 )			/* graphics data */
	ROM_LOAD16_BYTE( "egr1.bin",     0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.bin",     0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "eggdlx.gr4",   0x200000, 0x100000, CRC(cfb1e28b) SHA1(8d535a27158acee893233cf2012b4ab0ffc8dc03) )
	ROM_LOAD16_BYTE( "eggdlx.gr3",   0x200001, 0x100000, CRC(a7da3891) SHA1(9139c846006bbed4bdb183659a5b40aaa0000708) )
	ROM_LOAD16_BYTE( "eggdlx.gr6",   0x400000, 0x100000, CRC(97d02e8a) SHA1(6f9532fb031953c1187782b4fce5a0cfaf9461b3) )
	ROM_LOAD16_BYTE( "eggdlx.gr5",   0x400001, 0x100000, CRC(387d9176) SHA1(9f26f97cab8baeea1d5e4860a8a35a55bdc601e8) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )				/* sound data */
	ROM_LOAD( "eu20.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )

	ROM_REGION( 0x40000, REGION_SOUND2, 0 )				/* sound data */
	ROM_LOAD( "eu21.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )

	ROM_REGION( 0x40000, REGION_SOUND3, 0 )				/* sound data */
	ROM_LOAD( "eu18.bin", 0x00000, 0x40000, CRC(3760b1db) SHA1(70e258a6036f9ce26b354c4df57e0e4d2c871bcb) )
ROM_END


ROM_START( cclownz )
	ROM_REGION16_LE( 0x100000, REGION_USER1, 0 )		/* 34010 code */
	ROM_LOAD16_BYTE( "cc-v1-vc8.bin", 0x000000, 0x080000, CRC(433fe6ac) SHA1(dea7aede9882ee52be88927418b7395418757d12) )
	ROM_LOAD16_BYTE( "cc-v1-vc9.bin", 0x000001, 0x080000, CRC(9d1b3dae) SHA1(44a97c38bc9685e97721722c67505832fa06b44d) )

	ROM_REGION16_LE( 0x600000, REGION_GFX1, 0 )			/* graphics data */
	ROM_LOAD16_BYTE( "cc-gr1.bin",   0x000000, 0x100000, CRC(17c0ab2a) SHA1(f5ec66f4ac3292ef74f6434fe3ef17f9e977e8f6) )
	ROM_LOAD16_BYTE( "cc-gr2.bin",   0x000001, 0x100000, CRC(dead9528) SHA1(195ad9f7da61ecb5a364da92ba837aa3fcb3a347) )
	ROM_LOAD16_BYTE( "cc-gr4.bin",   0x200000, 0x100000, CRC(78cceed8) SHA1(bc8e5bb625072b17a5711402b07a39ea4a87a0f8) )
	ROM_LOAD16_BYTE( "cc-gr3.bin",   0x200001, 0x100000, CRC(af836fee) SHA1(9e32d5030d3bc5ff106242e5d4969b0150b2c516) )
	ROM_LOAD16_BYTE( "cc-gr6.bin",   0x400000, 0x100000, CRC(889d2771) SHA1(3222d7105c3a68e2050f00b07e8d84d57a9f7a19) )
	ROM_LOAD16_BYTE( "cc-gr5.bin",   0x400001, 0x100000, CRC(2a15ef8f) SHA1(3e33cff2657bb1371acf25641080aff2d8da6c05) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 )				/* sound data */
	ROM_LOAD( "cc-s-u20.bin", 0x00000, 0x80000, CRC(252fc4b5) SHA1(bbc6c3599869f3f46d3df4f3f8d0a8d88d8e0132) )

	ROM_REGION( 0x80000, REGION_SOUND2, 0 )				/* sound data */
	ROM_LOAD( "cc-s-u21.bin", 0x00000, 0x80000, CRC(6c3da4ed) SHA1(f10cbea6e03ada5ac1535041636e96b6224967fa) )

	ROM_REGION( 0x80000, REGION_SOUND3, 0 )				/* sound data */
	ROM_LOAD( "cc-s-u18.bin", 0x00000, 0x80000, CRC(9cdf87af) SHA1(77dfc0bc1d535b5d585071dd4e9deb367003ab2d) )

	ROM_REGION( 0x80000, REGION_USER2, 0 ) /* convert these */
	ROM_LOAD( "vc-12.jed", 0x0000, 0x3f03, CRC(6947ea9e) SHA1(5a418cd04851841a49beeeea274c1441fefde173) )
	ROM_LOAD( "vc-16.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-22.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-23.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	ROM_LOAD( "vc-24.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	/* 25 / 26 are secure? */
ROM_END


ROM_START( ripribit )
	ROM_REGION16_LE( 0x100000, REGION_USER1, 0 )		/* 34010 code */
	ROM_LOAD16_BYTE( "rr_v2-84-vc8.bin", 0x000000, 0x080000, CRC(5ecc432d) SHA1(073062528fbcf63be7e3c6695d60d048430f6e4b) )
	ROM_LOAD16_BYTE( "rr_v2-84-vc9.bin", 0x000001, 0x080000, CRC(d9bae3f8) SHA1(fcf8099ebe170ad5778aaa533bcfd1e5ead46e6b) )

	ROM_REGION16_LE( 0x600000, REGION_GFX1, 0 )			/* graphics data */
	ROM_LOAD16_BYTE( "rr-gr1.bin",   0x000000, 0x100000, CRC(e02c79b7) SHA1(75e352424c449cd5cba1057555928d7ee13ab113) )
	ROM_LOAD16_BYTE( "rr-gr2.bin",   0x000001, 0x100000, CRC(09f48db7) SHA1(d0156c6e3d05ff81540c0eeb66e9a5e7fc4d053c) )
	ROM_LOAD16_BYTE( "rr-gr4.bin",   0x200000, 0x100000, CRC(94d0db81) SHA1(aa46c2e5a627cf01c1d57002204ec3419f0d4503) )
	ROM_LOAD16_BYTE( "rr-gr3.bin",   0x200001, 0x100000, CRC(b65e1a36) SHA1(4feb7ea0bec509fa07d27c76e5a3904b8d1690c4) )
	ROM_LOAD16_BYTE( "rr-gr6.bin",   0x400000, 0x100000, CRC(c9ac211b) SHA1(75cbfa0f875da82d510d75ad28b9db0892b3da85) )
	ROM_LOAD16_BYTE( "rr-gr5.bin",   0x400001, 0x100000, CRC(84ae466a) SHA1(4e7b3dc27a46f735ff13a753806b3688f34a64fe) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 )  /* sound data (music) */
	ROM_LOAD( "rr-s-u20.bin", 0x00000, 0x80000, CRC(c345b779) SHA1(418058bbda74727ec99ac375982c9cd2c8bc5c86) )

	ROM_REGION( 0x80000, REGION_SOUND2, ROMREGION_ERASE00 )				/* sound data */
//  ROM_LOAD( "rr-s-u21.bin", 0x00000, 0x80000 ) // not populated

	ROM_REGION( 0x80000, REGION_SOUND3, 0 ) /* sound data (effects) */
	ROM_LOAD( "rr-s-u18.bin", 0x00000, 0x80000, CRC(badb9cb6) SHA1(716d65b5ff8d3f8ff25ae70426ce318af9a92b7e) )

	ROM_REGION( 0x80000, REGION_USER2, 0 ) /* convert these */
	ROM_LOAD( "vc-12.jed", 0x0000, 0x3f03, CRC(6947ea9e) SHA1(5a418cd04851841a49beeeea274c1441fefde173) )
	ROM_LOAD( "vc-16.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-22.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-23.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	ROM_LOAD( "vc-24.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	/* 25 / 26 are secure? */
ROM_END


ROM_START( cfarm )
	ROM_REGION16_LE( 0x100000, REGION_USER1, 0 )		/* 34010 code */
	ROM_LOAD16_BYTE( "cf-v2-vc8.bin", 0x000000, 0x080000, CRC(93bcf145) SHA1(134ac3ee4fd837f56fb0b338289cf03108346539) )
	ROM_LOAD16_BYTE( "cf-v2-vc9.bin", 0x000001, 0x080000, CRC(954421f9) SHA1(bf1faa9b085f066d1e2ff6ee01c468b1c1d945e9) )

	ROM_REGION16_LE( 0x600000, REGION_GFX1, 0 )			/* graphics data */
	ROM_LOAD16_BYTE( "cf-gr1.bin",   0x000000, 0x100000, CRC(2241a06e) SHA1(f07a99372bb951dd345378da212b41cb8204e782) )
	ROM_LOAD16_BYTE( "cf-gr2.bin",   0x000001, 0x100000, CRC(31182263) SHA1(d5d36f9b5d612f681e6aa563831b6704bc05489e) )
	ROM_LOAD16_BYTE( "cf-gr4.bin",   0x200000, 0x100000, CRC(0883a6f2) SHA1(ef259dcdc7b1325f15a98f6c97ecb965b2b6f9b1) )
	ROM_LOAD16_BYTE( "cf-gr3.bin",   0x200001, 0x100000, CRC(572f45d6) SHA1(a48cb6ab16654d5e07e8833e2848802ddc0e2667) )
	ROM_LOAD16_BYTE( "cf-gr6.bin",   0x400000, 0x100000, CRC(8709a62c) SHA1(3691fb055155ae339c78ec8b7f485aa7d576556b) )
	ROM_LOAD16_BYTE( "cf-gr5.bin",   0x400001, 0x100000, CRC(6de18621) SHA1(9e83f8ed3a2999ee4fdca389c5e792c5b1293717) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 )				/* sound data */
	ROM_LOAD( "cf-s-u20.bin", 0x00000, 0x80000, CRC(715a12dd) SHA1(374185b062853f3e2ea069ea53494cbe3d8dd511) )

	ROM_REGION( 0x80000, REGION_SOUND2, 0 )				/* sound data */
	ROM_LOAD( "cf-s-u21.bin", 0x00000, 0x80000, CRC(bc27e3d5) SHA1(a25215b8314fe44974e9efe78cdc10de34f7bfba) )

	ROM_REGION( 0x80000, REGION_SOUND3, 0 )				/* sound data */
	ROM_LOAD( "cf-s-u18.bin", 0x00000, 0x80000, CRC(63984658) SHA1(5594965c9304850187859ba730aff26001782f0f) )

	ROM_REGION( 0x80000, REGION_USER2, 0 ) /* convert these */
	ROM_LOAD( "vc-12.jed", 0x0000, 0x3f03, CRC(6947ea9e) SHA1(5a418cd04851841a49beeeea274c1441fefde173) )
	ROM_LOAD( "vc-16.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-22.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-23.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	ROM_LOAD( "vc-24.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	/* 25 / 26 are secure? */
ROM_END



/*************************************
 *
 *  Driver-specific initialization
 *
 *************************************/

static DRIVER_INIT( ripribit )
{
	ticket_dispenser_init(200, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x04100010, 0x0410001f, 0, 0, ripribit_control_w);
}


static DRIVER_INIT( cfarm )
{
	ticket_dispenser_init(200, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x04100010, 0x0410001f, 0, 0, cfarm_control_w);
}


static DRIVER_INIT( cclownz )
{
	ticket_dispenser_init(200, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x04100010, 0x0410001f, 0, 0, cclownz_control_w);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1996, lethalj,  0,        lethalj,  lethalj,  0,        ROT0,  "The Game Room", "Lethal Justice", 0 )
GAME( 1997, eggventr, 0,        gameroom, eggventr, 0,        ROT0,  "The Game Room", "Egg Venture (Release 10)", 0 )
GAME( 1997, eggvent7, eggventr, gameroom, eggventr, 0,        ROT0,  "The Game Room", "Egg Venture (Release 7)", 0 )
GAME( 1997, eggvntdx, eggventr, gameroom, eggvntdx, 0,        ROT0,  "The Game Room", "Egg Venture Deluxe", 0 )
GAME( 1997, ripribit, 0,        gameroom, ripribit, ripribit, ROT0,  "LAI Games",     "Ripper Ribbit (Version 2.8.4)", 0 )
GAME( 1999, cfarm,    0,        gameroom, cfarm,    cfarm,    ROT90, "LAI Games",     "Chicken Farm (Version 2.0)", 0 )
GAME( 1999, cclownz,  0,        gameroom, cclownz,  cclownz,  ROT0,  "LAI Games",     "Crazzy Clownz (Version 1.0)", 0 )
