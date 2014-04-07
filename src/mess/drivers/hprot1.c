/***************************************************************************

  HENRY Prot I - brazilian document timestamp printer
  http://www.dataponto.com.br/protocoladores-prot1.html

  Driver by Felipe Sanches
  Technical info at https://www.garoa.net.br/wiki/HENRY

  Licensed under GPLv2 or later.

  NOTE: Even though the MAME/MESS project has been adopting a non-commercial additional licensing clause, I do allow commercial usage of my portion of the code according to the plain terms of the GPL license (version 2 or later). This is useful if you happen to use my code in another project or in case the other MAME/MESS developers happen to drop the non-comercial clause completely. I suggest that other developers consider doing the same. --Felipe Sanches

  Changelog:

   2014 JAN 03 [Felipe Sanches]:
   * Initial driver skeleton
   * Address lines bitswaping
   * LCD works. We can see boot messages.
   * Inputs are not working correctly

TO-DO list:

  There seems to be an eeprom or a realtime clock placed at U2 (DIP8):
  pin1 -> 8031 pin 14 (T0: Timer 0 external input)
  pin2 -> crystal at X2 (labeled 32.768)
  pin3 -> ?
  pin4 -> GND
  pin5 -> ?
  pin6 -> 8031 pin 5 (Port 1 bit 4)
  pin7 -> 8031 pin 4 (Port 1 bit 3)
  pin8 -> VCC

(no context): unmapped io memory write to 20003 = FF & FF
(no context): unmapped io memory write to 20002 = FF & FF
(no context): unmapped io memory write to 20001 = FF & FF
(no context): unmapped io memory write to 20000 = FF & FF
':maincpu' (01EB): unmapped io memory write to 20001 = 7F & FF
':maincpu' (01EE): unmapped io memory write to 20003 = FF & FF
':maincpu' (01F0): unmapped io memory write to 20001 = 77 & FF
':maincpu' (3500): unmapped io memory write to 20003 = FF & FF
':maincpu' (0208): unmapped io memory write to 20001 = 77 & FF
':maincpu' (13F7): unmapped io memory write to 20001 = 77 & FF
':maincpu' (13EF): unmapped io memory write to 20001 = 67 & FF
':maincpu' (13F7): unmapped io memory write to 20001 = 77 & FF
The last 2 lines repeat endlessly.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/hd44780.h"
#include "rendlay.h"

class hprot1_state : public driver_device
{
public:
	hprot1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd44780")
	{ }

	//DECLARE_WRITE8_MEMBER(henry_io_w);
	DECLARE_READ8_MEMBER(henry_io_r);
	DECLARE_DRIVER_INIT(hprot1);
	DECLARE_PALETTE_INIT(hprot1);
private:
	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

#define LOG_IO_PORTS 0

static ADDRESS_MAP_START(i80c31_prg, AS_PROGRAM, 8, hprot1_state)
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

DRIVER_INIT_MEMBER( hprot1_state, hprot1 )
{
	int i;
	UINT8 *ROM = memregion("maincpu")->base();
	UINT8 bitswapped_ROM[0x10000];

	for(i=0x0000;i<0x10000;i++)
		bitswapped_ROM[i] = ROM[i];

	for(i=0x0000;i<0x10000;i++)
		ROM[BITSWAP16(i, 15, 14, 13, 12, 11, 10, 9, 8, 3, 2, 1, 0, 4, 5, 6, 7)] = bitswapped_ROM[i];
}

//A4 = display RS
//A5 = display R/W
//(A11 == 0) && (A10 == 0) => display CS
//(A14 == 1) && (A15 == 1) => enable signal for the mux that selects peripherals
//11?? 00?? ??00 ???? write data
//11?? 00?? ??01 ???? write command
//11?? 00?? ??10 ???? read data
//11?? 00?? ??11 ???? read command
//mirror=0x33cf

//write: 0xc400 => U12 (?)
//write: 0xc800 => U11 (?)
//read:  0xc020 => display
//write: 0xc000 => display
//write: 0xc010 => display

//P1.4 => WhatchDog Input (after timeout resets CPU)

static ADDRESS_MAP_START(i80c31_io, AS_IO, 8, hprot1_state)
	AM_RANGE(0x0000,0x7fff) AM_RAM
	AM_RANGE(0xc000,0xc000) AM_MIRROR(0x33cf) AM_DEVWRITE("hd44780", hd44780_device, control_write)
	AM_RANGE(0xc010,0xc010) AM_MIRROR(0x33cf) AM_DEVWRITE("hd44780", hd44780_device, data_write)
	AM_RANGE(0xc020,0xc020) AM_MIRROR(0x33cf) AM_DEVREAD("hd44780", hd44780_device, control_read)
	AM_RANGE(0xc030,0xc030) AM_MIRROR(0x33cf) AM_DEVREAD("hd44780", hd44780_device, data_read)
	//AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READWRITE(henry_io_r, henry_io_w)
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READ(henry_io_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( hprot1 )
	PORT_START("inputs")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Upper Black Button") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Lower Black Button") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Blue Button") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Paper Detector") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("XMIN Endstop") PORT_CODE(KEYCODE_E)
INPUT_PORTS_END

void hprot1_state::machine_start()
{
}

void hprot1_state::machine_reset()
{
}

READ8_MEMBER(hprot1_state::henry_io_r)
{
	switch (offset)
	{
		case 0x01:
		{
			UINT8 value = (ioport("inputs")->read()) & 0x67;
#if LOG_IO_PORTS
			printf("value:%02X\n", value);
#endif
			return value;
		}
		default:
#if LOG_IO_PORTS
			printf("Unhandled I/O Read at offset 0x%02X (return 0)\n", offset);
#endif
			return 0;
	}
}

/*
WRITE8_MEMBER(hprot1_state::henry_io_w)
{
    static UINT8 p0=0, p1=0, p2=0, p3=0;
    switch (offset)
    {
        case 0x00:
        {
            if (data != p0)
            {
                p0=data;
#if LOG_IO_PORTS
                printf("Write to P0: %02X\n", data);
#endif
            }
            break;
        }
        case 0x01:
        {
            if (data != p1)
            {
                p1=data;
                if (data != 0xFF && data != 0xEF)
#if LOG_IO_PORTS
                printf("Write to P1: %02X\n", data);
#endif
            }
            break;
        }
        case 0x02:
        {
            if (data != p2)
            {
                p2=data;
#if LOG_IO_PORTS
                printf("Write to P2: %02X\n", data);
#endif
            }
            break;
        }
        case 0x03:
        {
            if (data != p3)
            {
                p3=data;
#if LOG_IO_PORTS
                printf("Write to P3: %02X\n", data);
#endif
            }
            break;
        }
    }
}
*/

PALETTE_INIT_MEMBER(hprot1_state, hprot1)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static const gfx_layout henry_prot_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( hprot1 )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, henry_prot_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( hprot1, hprot1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C31, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(i80c31_prg)
	MCFG_CPU_IO_MAP(i80c31_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(6*16, 9*2)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16-1, 0, 9*2-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(hprot1_state, hprot1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hprot1)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16)
MACHINE_CONFIG_END

ROM_START( hprot1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "henry_prot1_rev1_v19.bin",  0x00000, 0x10000, CRC(dd7787fd) SHA1(61a37dd406b3440d568bd6da75a9fdc8a0f0e1e3) )
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   CLASS           INIT   COMPANY    FULLNAME                    FLAGS */
COMP( 2004, hprot1, 0,      0,      hprot1,     hprot1, hprot1_state, hprot1, "HENRY", "Henry Prot I v19 (REV.1)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND)
