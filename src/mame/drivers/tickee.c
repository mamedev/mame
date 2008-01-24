/***************************************************************************

    Raster Elite Tickee Tickats hardware

    driver by Aaron Giles

    Games supported:
        * Tickee Tickats
        * Ghost Hunter
        * Tuts Tomb

    Known bugs:
        * (Tickee) gun sometimes misfires

***************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/ticket.h"
#include "video/tlc34076.h"
#include "sound/ay8910.h"


#define CPU_CLOCK			XTAL_40MHz
#define VIDEO_CLOCK			XTAL_14_31818MHz



/* local variables */
static UINT16 *tickee_control;
static UINT16 *tickee_vram;
static emu_timer *setup_gun_timer;



/*************************************
 *
 *  Compute X/Y coordinates
 *
 *************************************/

INLINE void get_crosshair_xy(running_machine *machine, int player, int *x, int *y)
{
	*x = (((readinputport(4 + player * 2) & 0xff) * (machine->screen[0].visarea.max_x - machine->screen[0].visarea.min_x)) >> 8) + machine->screen[0].visarea.min_x;
	*y = (((readinputport(5 + player * 2) & 0xff) * (machine->screen[0].visarea.max_y - machine->screen[0].visarea.min_y)) >> 8) + machine->screen[0].visarea.min_y;
}



/*************************************
 *
 *  Light gun interrupts
 *
 *************************************/

static TIMER_CALLBACK( trigger_gun_interrupt )
{
	/* fire the IRQ at the correct moment */
	cpunum_set_input_line(machine, 0, param, ASSERT_LINE);
}


static TIMER_CALLBACK( clear_gun_interrupt )
{
	/* clear the IRQ on the next scanline? */
	cpunum_set_input_line(machine, 0, param, CLEAR_LINE);
}


static TIMER_CALLBACK( setup_gun_interrupts )
{
	int beamx, beamy;

	/* set a timer to do this again next frame */
	timer_adjust(setup_gun_timer, video_screen_get_time_until_pos(0, 0, 0), 0, attotime_zero);

	/* only do work if the palette is flashed */
	if (!tickee_control[2])
		return;

	/* generate interrupts for player 1's gun */
	get_crosshair_xy(machine, 0, &beamx, &beamy);
	timer_set(video_screen_get_time_until_pos(0, beamy,     beamx + 50), NULL, 0, trigger_gun_interrupt);
	timer_set(video_screen_get_time_until_pos(0, beamy + 1, beamx + 50), NULL, 0, clear_gun_interrupt);

	/* generate interrupts for player 2's gun */
	get_crosshair_xy(machine, 1, &beamx, &beamy);
	timer_set(video_screen_get_time_until_pos(0, beamy,     beamx + 50), NULL, 1, trigger_gun_interrupt);
	timer_set(video_screen_get_time_until_pos(0, beamy + 1, beamx + 50), NULL, 1, clear_gun_interrupt);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

static VIDEO_START( tickee )
{
	/* start a timer going on the first scanline of every frame */
	setup_gun_timer = timer_alloc(setup_gun_interrupts, NULL);
	timer_adjust(setup_gun_timer, video_screen_get_time_until_pos(0, 0, 0), 0, attotime_zero);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void scanline_update(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT16 *src = &tickee_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = BITMAP_ADDR32(bitmap, scanline, 0);
	const rgb_t *pens = tlc34076_get_pens();
	int coladdr = params->coladdr << 1;
	int x;

	/* blank palette: fill with pen 255 */
	if (tickee_control[2])
	{
		for (x = params->heblnk; x < params->hsblnk; x++)
			dest[x] = pens[0xff];
	}
	else
		/* copy the non-blanked portions of this scanline */
		for (x = params->heblnk; x < params->hsblnk; x += 2)
		{
			UINT16 pixels = src[coladdr++ & 0xff];
			dest[x + 0] = pens[pixels & 0xff];
			dest[x + 1] = pens[pixels >> 8];
		}
}


/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_RESET( tickee )
{
	ticket_dispenser_init(100, 0, 1);
	tlc34076_reset(6);
}



/*************************************
 *
 *  Miscellaneous control bits
 *
 *************************************/

static READ8_HANDLER( port1_r )
{
	return input_port_1_r(offset) | (ticket_dispenser_0_r(0) >> 5) | (ticket_dispenser_1_r(0) >> 6);
}

static READ8_HANDLER( port2_r )
{
	return input_port_3_r(offset) | (ticket_dispenser_0_r(0) >> 5) | (ticket_dispenser_1_r(0) >> 6);
}



/*************************************
 *
 *  Miscellaneous control bits
 *
 *************************************/

static WRITE16_HANDLER( tickee_control_w )
{
	UINT16 olddata = tickee_control[offset];

	/* offsets:

        2 = palette flash (0 normally, 1 when trigger is pressed)
        3 = ticket motor (bit 3 = 0 for left motor, bit 2 = 0 for right motor)
        6 = lamps? (changing all the time)
    */

	COMBINE_DATA(&tickee_control[offset]);

	if (offset == 3)
	{
		ticket_dispenser_0_w(0, (data & 8) << 4);
		ticket_dispenser_1_w(0, (data & 4) << 5);
	}

	if (olddata != tickee_control[offset])
		logerror("%08X:tickee_control_w(%d) = %04X (was %04X)\n", activecpu_get_pc(), offset, tickee_control[offset], olddata);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( tickee_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(&tickee_vram)
	AM_RANGE(0x02000000, 0x02ffffff) AM_ROM AM_REGION(REGION_USER1, 0)
	AM_RANGE(0x04000000, 0x04003fff) AM_RAM AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x04100000, 0x041000ff) AM_READWRITE(tlc34076_lsb_r, tlc34076_lsb_w)
	AM_RANGE(0x04200000, 0x0420000f) AM_READWRITE(AY8910_read_port_0_lsb_r, AY8910_control_port_0_lsb_w)
	AM_RANGE(0x04200010, 0x0420001f) AM_WRITE(AY8910_write_port_0_lsb_w)
	AM_RANGE(0x04200100, 0x0420010f) AM_READWRITE(AY8910_read_port_1_lsb_r, AY8910_control_port_1_lsb_w)
	AM_RANGE(0x04200110, 0x0420011f) AM_WRITE(AY8910_write_port_1_lsb_w)
	AM_RANGE(0x04400000, 0x0440007f) AM_WRITE(tickee_control_w) AM_BASE(&tickee_control)
	AM_RANGE(0x04400040, 0x0440004f) AM_READ(input_port_3_word_r)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xc0000240, 0xc000025f) AM_WRITENOP		/* seems to be a bug in their code */
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION(REGION_USER1, 0)
ADDRESS_MAP_END


/* addreses in the 04x range shifted slightly...*/
static ADDRESS_MAP_START( ghoshunt_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(&tickee_vram)
	AM_RANGE(0x02000000, 0x02ffffff) AM_ROM AM_REGION(REGION_USER1, 0)
	AM_RANGE(0x04100000, 0x04103fff) AM_RAM AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x04200000, 0x042000ff) AM_READWRITE(tlc34076_lsb_r, tlc34076_lsb_w)
	AM_RANGE(0x04300000, 0x0430000f) AM_READWRITE(AY8910_read_port_0_lsb_r, AY8910_control_port_0_lsb_w)
	AM_RANGE(0x04300010, 0x0430001f) AM_WRITE(AY8910_write_port_0_lsb_w)
	AM_RANGE(0x04300100, 0x0430010f) AM_READWRITE(AY8910_read_port_1_lsb_r, AY8910_control_port_1_lsb_w)
	AM_RANGE(0x04300110, 0x0430011f) AM_WRITE(AY8910_write_port_1_lsb_w)
	AM_RANGE(0x04500000, 0x0450007f) AM_WRITE(tickee_control_w) AM_BASE(&tickee_control)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xc0000240, 0xc000025f) AM_WRITENOP		/* seems to be a bug in their code */
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION(REGION_USER1, 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( tickee )
	PORT_START
	PORT_DIPNAME( 0x03, 0x01, "Game Time/Diff" )
	PORT_DIPSETTING(    0x03, "Very Fast/Very Easy" )
	PORT_DIPSETTING(    0x02, "Fast/Easy" )
	PORT_DIPSETTING(    0x01, "Average/Hard" )
	PORT_DIPSETTING(    0x00, "Slow/Very Hard" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "Last Box Tickets" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "25" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ))

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* right ticket status */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* left ticket status */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( ghoshunt )
	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "Messages in Play")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x02, "Fixed Ticketing")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, "Setting")
	PORT_DIPSETTING(    0x04, "Custom")
	PORT_DIPSETTING(    0x00, DEF_STR( Standard ))
	PORT_DIPNAME( 0x08, 0x08, "Messages")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x30, 0x00, "Tickets")
	PORT_DIPSETTING(    0x30, "5")
	PORT_DIPSETTING(    0x20, "10")
	PORT_DIPSETTING(    0x10, "15")
	PORT_DIPSETTING(    0x00, "20")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ))

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* right ticket status */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* left ticket status */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xd8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* right ticket status */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* left ticket status */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xd8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const struct AY8910interface ay8910_interface_1 =
{
	input_port_0_r,
 	input_port_2_r
};

static const struct AY8910interface ay8910_interface_2 =
{
	port1_r,
	port2_r
};



/*************************************
 *
 *  34010 configuration
 *
 *************************************/

static const tms34010_config tms_config =
{
	FALSE,							/* halt on reset */
	0,								/* the screen operated on */
	VIDEO_CLOCK/2,					/* pixel clock */
	1,								/* pixels per clock */
	scanline_update,				/* scanline callback */
	NULL,							/* generate interrupt */
	NULL,							/* write to shiftreg function */
	NULL							/* read from shiftreg function */
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( tickee )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", TMS34010, XTAL_40MHz)
	MDRV_CPU_CONFIG(tms_config)
	MDRV_CPU_PROGRAM_MAP(tickee_map,0)

	MDRV_MACHINE_RESET(tickee)
	MDRV_NVRAM_HANDLER(generic_1fill)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_START(tickee)
	MDRV_VIDEO_UPDATE(tms340x0)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 444, 0, 320, 233, 0, 200)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2149, VIDEO_CLOCK/8)
	MDRV_SOUND_CONFIG(ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(YM2149, VIDEO_CLOCK/8)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ghoshunt )
	MDRV_IMPORT_FROM(tickee)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(ghoshunt_map,0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( tickee )
	ROM_REGION16_LE( 0x200000, REGION_USER1, 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "3.ic4",  0x000000, 0x80000, CRC(5b1e399c) SHA1(681608f06bbaf3d258e9f4768a8a6c5047ad08ec) )
	ROM_LOAD16_BYTE( "2.ic3",  0x000001, 0x80000, CRC(1b26d4bb) SHA1(40266ec0fe5897eba85072e5bb39973d34f97546) )
	ROM_LOAD16_BYTE( "1.ic2",  0x100000, 0x80000, CRC(f7f0309e) SHA1(4a93e0e203f5a340a56b770a40b9ab00e131644d) )
	ROM_LOAD16_BYTE( "4.ic5",  0x100001, 0x80000, CRC(ceb0f559) SHA1(61923fe09e1dfde1eaae297ccbc672bc74a70397) )
ROM_END


/*

Ghost Hunter
Hanaho Games, 1996

  and

Tut's Tomb
Island Design Inc., 1996

PCB Layout
----------

|-----------------------------------------------|
|VOL                              CN106  CN103  |
|        NTE1423      ADV476KN50E    74LS175    |
|                                               |
|   YM2149            V52C4258Z80    74LS138    |
|                                               |
|   YM2149            V52C4258Z80    74LS373    |
|                                               |
|         DIPSW(8)    V52C4258Z80    74LS373    |
|J                                              |
|A         74LS161    V52C4258Z80    GHOSTHUN.7K|
|M  74LS74                                      |
|M         74LS273    |------|       GHOSTHUN.7J|
|A                    |TMS   |                  |
|          74LS74     |34010 |       GHOSTHUN.7H|
|                     |------|                  |
|          74LS14                    GHOSTHUN.7G|
|   ULN2803           |---|                     |
| TIP122   TIP122     |110|          DS1220Y    |
| TIP122   TIP122     |---|    40MHz 14.31818MHz|
| TIP122   TIP122    74LS273                    |
|                              74LS245  74LS374 |
|-------------------------------------|--CN2--|-|
(All IC's shown)                      |-------|

Notes:
      34010 clocks - INCLK: 40.000MHz, VCLK: 7.15909MHz, HSYNC: 16.1kHz, VSYNC: 69Hz, BLANK: 69Hz
      YM2149 clock - 1.7897725MHz [14.31818/8]
      VSync        - 69Hz
      HSync        - 15.78kHz
      110          - AMD MACH110 High Density Electrically-Erasable CMOS Programmable Logic (PLCC44)
      ADV476KN50E  - Analog Devices ADV476KN50E CMOS Monolithic 256x18 Color Palette RAM-DAC (DIP28)
      DS1220Y      - Dallas Semiconductor DS1220Y 16K Nonvolatile SRAM (DIP24)
      V52C4258Z80  - Vitelic V52C4258Z80 ?? possibly 256K x8 SRAM (ZIP28)
      NTE1423      - NTE1423 5.7W Power Amplifier (SIP8)
      CN2          - DB25 connector
      CN103/106    - 4-pin connector for gun hookup
      ULN2803      - Motorola ULN2803 Octal High Voltage, High Current Darlington Transistor Arrays (DIP18)
      TIP122       - Motorola TIP122 General-Purpose NPN Darlington Transistor (TO-220)
*/

ROM_START( ghoshunt )
	ROM_REGION16_LE( 0x200000, REGION_USER1, 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "ghosthun.7g",  0x000001, 0x80000, CRC(d59716c2) SHA1(717a1a1c5c559569f9e7bc4ae4356d112f0cf4eb) )
	ROM_LOAD16_BYTE( "ghosthun.7h",  0x000000, 0x80000, CRC(ef38bfc8) SHA1(12b8f29f4da120f14126cbcdf4019bedd97063c3) )
	ROM_LOAD16_BYTE( "ghosthun.7j",  0x100001, 0x80000, CRC(763d7c79) SHA1(f0dec99feeeefeddda6a88276dc306a30a58f4e4) )
	ROM_LOAD16_BYTE( "ghosthun.7k",  0x100000, 0x80000, CRC(71e6099e) SHA1(2af6f1aa304eed849c90d95d17643cb12b05baab) )
ROM_END


ROM_START( tutstomb )
	ROM_REGION16_LE( 0x200000, REGION_USER1, 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "tutstomb.7g",  0x000001, 0x80000, CRC(b74d3cf2) SHA1(2221b565362183a97a959389e8a0a026ca89e0ce) )
	ROM_LOAD16_BYTE( "tutstomb.7h",  0x000000, 0x80000, CRC(177f3afb) SHA1(845f982a66a8b69b0ea0045399102e8bb33f7fbf) )
	ROM_LOAD16_BYTE( "tutstomb.7j",  0x100001, 0x80000, CRC(69094f31) SHA1(eadae8847d0ff1568e63f71bf09a84dc443fdc1c))
	ROM_LOAD16_BYTE( "tutstomb.7k",  0x100000, 0x80000, CRC(bc362df8) SHA1(7b15c646e99c916d850629e4e758b1dbb329639a) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, tickee,   0, tickee,   tickee,   0, ROT0, "Raster Elite",  "Tickee Tickats", 0 )
GAME( 1996, ghoshunt, 0, ghoshunt, ghoshunt, 0, ROT0, "Hanaho Games",  "Ghost Hunter", 0 )
GAME( 1996, tutstomb, 0, ghoshunt, ghoshunt, 0, ROT0, "Island Design", "Tut's Tomb", 0 )
