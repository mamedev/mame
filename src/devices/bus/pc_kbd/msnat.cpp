// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Microsoft Natural Keybaord emulation


TODO:
- Keyboard LEDs

***************************************************************************/

#include "emu.h"
#include "msnat.h"
#include "cpu/mcs51/mcs51.h"


/***************************************************************************
    ONSTANTS
***************************************************************************/

#define LOG     0


/*****************************************************************************
    INPUT PORTS
*****************************************************************************/

static INPUT_PORTS_START( microsoft_natural )
	PORT_START( "P2.0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y')                      // 15
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_H)            PORT_CHAR('H')                      // 23
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_J)            PORT_CHAR('J')                      // 24
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_N)            PORT_CHAR('N')                      // 31
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_M)            PORT_CHAR('M')                      // 32
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_U)            PORT_CHAR('U')                      // 16
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_6)            PORT_CHAR('6')                      // 07
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_7)            PORT_CHAR('7')                      // 08

	PORT_START( "P2.1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=')                      // 0D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']')                      // 1B
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('\\')                     // 2B    2 spots for backslash?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))      // E0 4B
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)                       // 1C
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(8)                        // 0E
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))        // 3F
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))        // 40

	PORT_START( "P2.2" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_9)            PORT_CHAR('9')                      // 0A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_O)            PORT_CHAR('O')                      // 18
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[')                      // 1A
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_L)            PORT_CHAR('L')                      // 26
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')                      // 34
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-')                      // 0C
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))        // 41
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))        // 42

	PORT_START( "P2.3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_0)            PORT_CHAR('0')                      // 0B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';')                      // 27
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('\'')                     // 28
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/')                      // 35
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))      // E0 50
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_P)            PORT_CHAR('P')                      // 19
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F9)           PORT_CHAR(UCHAR_MAMEKEY(F9))        // 43
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F10)          PORT_CHAR(UCHAR_MAMEKEY(F10))       // 44

	PORT_START( "P2.4" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))        // E0 48
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                   PORT_NAME("Unknown 73")             // 73 TODO
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                   PORT_NAME("\\ 2nd?")                // 2B    2 spots for backslash??
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_RWIN)         PORT_CHAR(UCHAR_MAMEKEY(RWIN))      // E0 5C
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     // E0 4D
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')                      // 39

	PORT_START( "P2.5" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_HOME)         PORT_CHAR(UCHAR_MAMEKEY(HOME))      // E0 47
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))     // 48
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))     // 4C
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))     // 50
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))     // 52
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_END)          PORT_CHAR(UCHAR_MAMEKEY(END))       // E0 4F
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F11)          PORT_CHAR(UCHAR_MAMEKEY(F11))       // 57
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F12)          PORT_CHAR(UCHAR_MAMEKEY(F12))       // 58

	PORT_START( "P2.6" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_INSERT)       PORT_CHAR(UCHAR_MAMEKEY(INSERT))    // E0 52
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_DEL)          PORT_CHAR(UCHAR_MAMEKEY(DEL))       // E0 53
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))     // 4D
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))     // 51
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))   // 53
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))     // 49
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))        // 3D
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))        // 3E

	PORT_START( "P2.7" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_SLASH_PAD)    PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) // E0
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))     // 47
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))     // 4B
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  // 4E
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))     // 4F
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  // 37   TODO
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_PRTSCR)       PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))    // E0 2A E0 37
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_MENU)         PORT_CHAR(UCHAR_MAMEKEY(MENU))      // E0 5D

	PORT_START( "P1.0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LWIN)         PORT_CHAR(UCHAR_MAMEKEY(LWIN))      // E0 5B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                   PORT_NAME("INT5 7E")                // 7E INT5
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_SCRLOCK)      PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))   // 46
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) // E0 1C

	PORT_START( "P1.1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))  // 1D
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_RCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))  // E0 1D
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  // 3A
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF

	PORT_START( "P1.2" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_PGUP)         PORT_CHAR(UCHAR_MAMEKEY(PGUP))      // E0 49
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_2)            PORT_CHAR('2')                      // 03
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_W)            PORT_CHAR('W')                      // 11
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_S)            PORT_CHAR('S')                      // 1F
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_X)            PORT_CHAR('X')                      // 2D
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_PGDN)         PORT_CHAR(UCHAR_MAMEKEY(PGDN))      // E0 51
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))        // 3B
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))        // 3C

	PORT_START( "P1.3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LALT)         PORT_CHAR(UCHAR_MAMEKEY(LALT))      // 38
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_RALT)         PORT_CHAR(UCHAR_MAMEKEY(RALT))      // E0 38
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) // 4A
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_NUMLOCK)      PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))   // 45

	PORT_START( "P1.4" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_I)            PORT_CHAR('I')                      // 17
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_K)            PORT_CHAR('K')                      // 25
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_D)            PORT_CHAR('D')                      // 20
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_C)            PORT_CHAR('C')                      // 2E
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')                      // 33
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_E)            PORT_CHAR('E')                      // 12
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_3)            PORT_CHAR('3')                      // 04
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_8)            PORT_CHAR('8')                      // 09

	PORT_START( "P1.5" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_R)            PORT_CHAR('R')                      // 13
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F)            PORT_CHAR('F')                      // 21
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_G)            PORT_CHAR('G')                      // 22
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_V)            PORT_CHAR('V')                      // 2F
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_B)            PORT_CHAR('B')                      // 30
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_T)            PORT_CHAR('T')                      // 14
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_4)            PORT_CHAR('4')                      // 05
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_5)            PORT_CHAR('5')                      // 06

	PORT_START( "P1.6" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_1)            PORT_CHAR('1')                      // 02
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Q)            PORT_CHAR('Q')                      // 10
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_A)            PORT_CHAR('A')                      // 1E
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                   PORT_NAME("INT1 56")                // 56 INT1?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z')                      // 2C
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_TAB)          PORT_CHAR(9)                        // 0F
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))       // 01
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('`')                      // 29

	PORT_START( "P1.7" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))    // 2A
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))    // 36
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_CANCEL)       PORT_CHAR(UCHAR_MAMEKEY(CANCEL))    // E1 1D 45 E1 9D C5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                                                                         // FF

INPUT_PORTS_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PC_KBD_MICROSOFT_NATURAL      = &device_creator<pc_kbd_microsoft_natural_device>;

/*****************************************************************************
    ADDRESS MAPS
*****************************************************************************/

static ADDRESS_MAP_START( microsoft_natural_io, AS_IO, 8, pc_kbd_microsoft_natural_device )
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_READWRITE(p0_read, p0_write)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_WRITE(p1_write)
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_WRITE(p2_write)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(p3_read, p3_write)
ADDRESS_MAP_END


/*****************************************************************************
    MACHINE CONFIG
*****************************************************************************/

MACHINE_CONFIG_FRAGMENT( microsoft_natural )
	MCFG_CPU_ADD("ms_natrl_cpu", I8051, XTAL_6MHz)
	MCFG_CPU_IO_MAP(microsoft_natural_io)
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( microsoft_natural )
	ROM_REGION(0x1000, "ms_natrl_cpu", 0)
	ROM_LOAD("natural.bin", 0x0000, 0x1000, CRC(aa8243ab) SHA1(72134882a5c03e785db07cc54dfb7572c0a730d9))
ROM_END


pc_kbd_microsoft_natural_device::pc_kbd_microsoft_natural_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_KBD_MICROSOFT_NATURAL, "Microsoft Natural Keyboard", tag, owner, clock, "ms_natural", __FILE__)
	, device_pc_kbd_interface(mconfig, *this)
	, m_cpu(*this, "ms_natrl_cpu")
	, m_p2_0(*this, "P2.0")
	, m_p2_1(*this, "P2.1")
	, m_p2_2(*this, "P2.2")
	, m_p2_3(*this, "P2.3")
	, m_p2_4(*this, "P2.4")
	, m_p2_5(*this, "P2.5")
	, m_p2_6(*this, "P2.6")
	, m_p2_7(*this, "P2.7")
	, m_p1_0(*this, "P1.0")
	, m_p1_1(*this, "P1.1")
	, m_p1_2(*this, "P1.2")
	, m_p1_3(*this, "P1.3")
	, m_p1_4(*this, "P1.4")
	, m_p1_5(*this, "P1.5")
	, m_p1_6(*this, "P1.6")
	, m_p1_7(*this, "P1.7"), m_p0(0), m_p1(0), m_p2(0), m_p3(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void pc_kbd_microsoft_natural_device::device_start()
{
	set_pc_kbdc_device();

	/* setup savestates */
	save_item(NAME(m_p0));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_p3));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void pc_kbd_microsoft_natural_device::device_reset()
{
	/* set default values */
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor pc_kbd_microsoft_natural_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( microsoft_natural );
}


ioport_constructor pc_kbd_microsoft_natural_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( microsoft_natural );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *pc_kbd_microsoft_natural_device::device_rom_region() const
{
	return ROM_NAME( microsoft_natural );
}


WRITE_LINE_MEMBER( pc_kbd_microsoft_natural_device::clock_write )
{
}


WRITE_LINE_MEMBER( pc_kbd_microsoft_natural_device::data_write )
{
}


READ8_MEMBER( pc_kbd_microsoft_natural_device::p0_read )
{
	UINT8 data = 0xFF;

	if (LOG)
		logerror("%s: P0 read. P1 = %02x, P2 = %02x\n", tag().c_str(), m_p1, m_p2 );

	if ( ! ( m_p2 & 0x01 ) )
	{
		data &= m_p2_0->read();
	}

	if ( ! ( m_p2 & 0x02 ) )
	{
		data &= m_p2_1->read();
	}

	if ( ! ( m_p2 & 0x04 ) )
	{
		data &= m_p2_2->read();
	}

	if ( ! ( m_p2 & 0x08 ) )
	{
		data &= m_p2_3->read();
	}

	if ( ! ( m_p2 & 0x10 ) )
	{
		data &= m_p2_4->read();
	}

	if ( ! ( m_p2 & 0x20 ) )
	{
		data &= m_p2_5->read();
	}

	if ( ! ( m_p2 & 0x40 ) )
	{
		data &= m_p2_6->read();
	}

	if ( ! ( m_p2 & 0x80 ) )
	{
		data &= m_p2_7->read();
	}

	if ( ! ( m_p1 & 0x01 ) )
	{
		data &= m_p1_0->read();
	}

	if ( ! ( m_p1 & 0x02 ) )
	{
		data &= m_p1_1->read();
	}

	if ( ! ( m_p1 & 0x04 ) )
	{
		data &= m_p1_2->read();
	}

	if ( ! ( m_p1 & 0x08 ) )
	{
		data &= m_p1_3->read();
	}

	if ( ! ( m_p1 & 0x10 ) )
	{
		data &= m_p1_4->read();
	}

	if ( ! ( m_p1 & 0x20 ) )
	{
		data &= m_p1_5->read();
	}

	if ( ! ( m_p1 & 0x40 ) )
	{
		data &= m_p1_6->read();
	}

	if ( ! ( m_p1 & 0x80 ) )
	{
		data &= m_p1_7->read();
	}

	return data;
}


WRITE8_MEMBER( pc_kbd_microsoft_natural_device::p0_write )
{
	m_p0 = data;
}


WRITE8_MEMBER( pc_kbd_microsoft_natural_device::p1_write )
{
	m_p1 = data;
}


WRITE8_MEMBER( pc_kbd_microsoft_natural_device::p2_write )
{
	m_p2 = data;
}


READ8_MEMBER( pc_kbd_microsoft_natural_device::p3_read )
{
	UINT8 data = m_p3 & ~0x21;

	// (Incoming) Clock signal is tied to the T1/P3.5 pin
	data |= (clock_signal() ? 0x20 : 0x00);

	// (Incoming) Data signal is tied to the RXD/P3.0 pin
	data |= ( data_signal() ? 0x01 : 0x00 );

	return data;
}


WRITE8_MEMBER( pc_kbd_microsoft_natural_device::p3_write )
{
	if ( m_pc_kbdc )
	{
		// (Outgoing) data signal is tied to the WR/P3.6 pin
		m_pc_kbdc->data_write_from_kb( BIT(data, 6) );

		// (Outgoing) clock signal is tied to the T0/P3.4 pin
		m_pc_kbdc->clock_write_from_kb( BIT(data, 4) );
	}

	m_p3 = data;
}
