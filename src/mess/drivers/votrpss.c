/******************************************************************************
*
*  Votrax Personal Speech System Driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with help from Kevin 'kevtris' Horton
*  Special thanks to Professor Nicholas Gessler for loaning several PSS units
*
*  The votrax PSS was sold from around 35th week of 1982 until october, 1990 (LONG product life)

<kevtris> timer0 = baud clock
<kevtris> timer1 = pitch (duty cycle output to modify RC)
<kevtris> timer2 = volume (duty cycle output to control trans. gate which does vol. control)
<kevtris> portb: pin 6 through pin 13 of parallel port
<kevtris> portc 0 = NC, 1 = GND, 2 = pin 5, 3 = /RXINTEN, 4 = pin 15, 5 = pin 14 through inverter, 6 = 8910 enable, 7 = from pin 4 through inverter (I believe that's for the parallel port)
<kevtris> porta: pin 16 through 23 of parallel port
<kevtris> that's the 8255
<kevtris> on the AY-3-8910:
<kevtris> IOA0-A5 = phoneme #
<kevtris> IOA6 = strobe (SC-01)
<kevtris> IOA7 = vochord control, 0 = off, 1 = on
<kevtris> IOB0-IOB7 = dip switches
<kevtris> there ya go, complete IO port map
<LordNLptp> cool :)
<kevtris> I pinned the IO to the serial port from the 8251 but I don't think it's too useful
<LordNLptp> eh, its useful
<kevtris> I drew out the schematic for the analog section
<LordNLptp> oh that will be useful
<kevtris> but I didn't draw out the digital part, just made an I/O and memory map but I can't find them
<LordNLptp> i also see the ay is running at 2 MHz
<kevtris> sounds about right
<kevtris> got it
<kevtris> I drew out the clocking section too
<kevtris> 8MHz xtal
<kevtris> 4MHz for the Z80
<kevtris> 2MHz for the 8253 and '8910
<kevtris> then the dividers also generate the system reset signals for the 8251
<kevtris> and a periodic IRQ
<LordNLptp> on the z80?
<LordNLptp> ok how does that work?
<kevtris> IRQ rate is umm
<kevtris> 122Hz
<kevtris> (8MHz / 65536)
<kevtris> oh the 8251 is not reset by the counters
<kevtris> it just takes a positive reset off an inverter that resets the counters
<kevtris> PIT2 is gated by 8MHz / 256
<kevtris> PIT1 is gated by 8MHz / 4096
<kevtris> PIT0 is not gated
<kevtris> (i.e. it always runs)
<LordNLptp> oh boy, this is sounding more fun by the minute
<kevtris> aaand your luck is about to run out
<kevtris> that's all I got
<LordNLptp> thats good enough, saved me a lot of work
<kevtris> but that's everything you need
<LordNLptp> yeah

*  (driver structure copied from vtech1.c)
******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/z80/z80.h"
#include "votrpss.lh"

/* Components */
//#include "sound/ay8910.h"
//#include "sound/votrax.h"
//#include "machine/i8255.h"
//#include "machine/pit8253.h"
//#include "machine/i8251.h"

/* For testing */
#include "machine/terminal.h"


class votrpss_state : public driver_device
{
public:
	votrpss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(votrpss_00_r);
	DECLARE_READ8_MEMBER(votrpss_02_r);
	DECLARE_READ8_MEMBER(votrpss_41_r);
	DECLARE_WRITE8_MEMBER( votrpss_kbd_put );
	UINT8 m_term_data;
	UINT8 m_term_status;
};

READ8_MEMBER( votrpss_state::votrpss_02_r )
{
	return m_term_status;
}

READ8_MEMBER( votrpss_state::votrpss_00_r )
{
	m_term_status = 0;
	return m_term_data;
}

READ8_MEMBER( votrpss_state::votrpss_41_r )
{
	return 1;
}


/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(votrpss_mem, AS_PROGRAM, 8, votrpss_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM /* main roms (in potted module) */
	AM_RANGE(0x4000, 0x7fff) AM_NOP /* open bus/space for expansion rom (reads as 0xFF) */
	AM_RANGE(0x8000, 0x8fff) AM_RAM /* onboard memory (2x 6116) */
	AM_RANGE(0x9000, 0xbfff) AM_NOP /* open bus (space for memory expansion, checked by main roms, will be used if found)*/
	AM_RANGE(0xc000, 0xdfff) AM_ROM /* 'personality rom', containing self-test code and optional user code */
	AM_RANGE(0xe000, 0xffff) AM_NOP /* open bus (space for more personality rom, not normally used) */
ADDRESS_MAP_END

static ADDRESS_MAP_START(votrpss_io, AS_IO, 8, votrpss_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(votrpss_00_r)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x02, 0x02) AM_READ(votrpss_02_r)
	AM_RANGE(0x41, 0x41) AM_READ(votrpss_41_r)
	//AM_RANGE(0x00, 0xff) AM_NOP /* temporary */
	//AM_RANGE(0x00, 0x03) AM_READWRITE(8255ppi_r, 8255ppi_w) AM_MIRROR (0x3c)
	//AM_RANGE(0x40, 0x41) AM_READWRITE(i8251_r, i8251_w) AM_MIRROR (0x3e)
	//AM_RANGE(0x80, 0x83) AM_READWRITE(pit8253_r, pit8253_w) AM_MIRROR (0x3c)
	//AM_RANGE(0xc0, 0xc3) AM_READWRITE(ay8910_r, ay8910_W) AM_MIRROR (0x3c)
ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(votrpss)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Baud Rate" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x01, "4800" )
	PORT_DIPSETTING(    0x02, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x04, "600" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x06, "150" )
	PORT_DIPSETTING(    0x07, "75" )
	PORT_DIPNAME( 0x08, 0x00, "Serial Handshaking" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "RTS/CTS" )
	PORT_DIPSETTING(    0x08, "XON/XOFF" )
	PORT_DIPNAME( 0x10, 0x00, "Parity bit behavior" )   PORT_DIPLOCATION("SW1:5") /* note: only firmware 3.C (1984?) and up handle this bit; on earlier firmwares, its function is 'unused' */
	PORT_DIPSETTING(    0x00, "Bit 8 ignored/zeroed" )
	PORT_DIPSETTING(    0x10, "Bit 8 treated as data" )
	PORT_DIPNAME( 0x20, 0x20, "Startup Message" )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR ( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Default Communications Port" )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Serial/RS-232" )
	PORT_DIPSETTING(    0x40, "Parallel" )
	PORT_DIPNAME( 0x80, 0x00, "Self Test Mode" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR ( On )  )
INPUT_PORTS_END

WRITE8_MEMBER( votrpss_state::votrpss_kbd_put )
{
	m_term_data = data;
	m_term_status = 0x20;
}

static GENERIC_TERMINAL_INTERFACE( votrpss_terminal_intf )
{
	DEVCB_DRIVER_MEMBER(votrpss_state, votrpss_kbd_put)
};

/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( votrpss, votrpss_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz/2)  /* 4.000 MHz, verified */
	MCFG_CPU_PROGRAM_MAP(votrpss_mem)
	MCFG_CPU_IO_MAP(votrpss_io)
	//MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT(layout_votrpss)

	/* sound hardware */
	//MCFG_SPEAKER_STANDARD_MONO("mono")
	//MCFG_SOUND_ADD("ay1", AY8910, XTAL_8MHz/4) /* 2.000 MHz, verified */
	//MCFG_SOUND_CONFIG(ay8910_config)
	//MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	//votrax goes here too

	/* printer */
	//MCFG_PRINTER_ADD("printer")

	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, votrpss_terminal_intf)
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(votrpss)
	ROM_REGION(0x10000, "maincpu", 0)
	/* old logo PSS, version 3.A (1982), selftest 3.0? (1982) */
	//ROM_LOAD("u-2.3.A.bin",   0x0000, 0x2000, NO_DUMP )) /* 3.A 1982 */
	//ROM_LOAD("u-3.3.A.bin",   0x2000, 0x2000, NO_DUMP )) /* 3.A 1982 */
	//ROM_LOAD("u-4.3.0.bin", 0xc000, 0x2000, NO_DUMP )) /* 3.0? */

	/* old logo PSS, version 3.B (late 82/early83), selftest 3.0? (1982) */
	//ROM_LOAD("u-2.3.B.bin",   0x0000, 0x2000, NO_DUMP )) /* 3.B 1983? */
	//ROM_LOAD("u-3.3.B.bin",   0x2000, 0x2000, NO_DUMP )) /* 3.B 1983? */
	//ROM_LOAD("u-4.3.0.bin", 0xc000, 0x2000, NO_DUMP )) /* 3.0? */

	/* old or new logo PSS, Version 3.C (1984?), selftest 3.1 (1985?) */
	ROM_LOAD("u-2.1985.bin",   0x0000, 0x2000, CRC(410c58cf) SHA1(6e181e61ab9c268e3772fbeba101302fd40b09a2)) /* 3.C 1984?; The 1987/1988 version marked "U-2 // 090788" matches this rom */
	ROM_LOAD("u-3.1985.bin",   0x2000, 0x2000, CRC(1439492e) SHA1(46af8ccac6fdb93cbeb8a6d57dce5898e0e0d623)) /* 3.C 1984? */
	ROM_LOAD("u-4.100985.bin", 0xc000, 0x2000, CRC(0b7c4260) SHA1(56f0b6b1cd7b1104e09a9962583121c112337984)) /* 3.1 10/09/85 */

ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                            FLAGS */
COMP( 1982, votrpss,   0,          0,      votrpss,   votrpss, driver_device, 0,      "Votrax",        "Personal Speech System", GAME_NOT_WORKING | GAME_NO_SOUND)
