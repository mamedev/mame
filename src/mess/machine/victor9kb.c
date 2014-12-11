// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

/*

Keyboard PCB Layout
----------

Marking on PCB back: A65-02307-201D 007

|------------------------------------------------------------------------------------=
| 22-908-03 22-950-3B .   XTAL 8021  74LS14     [804x]           [EPROM]  [???]   CN1=___
|         X       X       X       X       X       X        X      X   X      X      X    |
| X    X   X   X   X   X   X   X   X   X   X   X   X   X    X     X   X    X   X   X   X |
| X     X   X   X   X   X   X   X   X   X   X   X   X   X    X    X   X    X   X   X   X |
| X     X    X   X   X   X   X   X   X   X   X   X   X   X    X   X   X    X   X   X   X |
| X    X   X  X   X   X   X   X   X   X   X   X   X       X       X   X    X   X   X   X |
| X     X    marking             X                 X              X   X    X   X   X   X |
|----------------------------------------------------------------------------------------|
                                         

Notes:
    All IC's shown.
    XTAL        - 3.579545Mhz Crystal, marked "48-300-010" (front) and "3.579545Mhz" (back)
    8021        - Intel 8021 MCU, marked: "iP8021 2137 // 8227 // 20-8021-139 // (C) INTEL 77"
    22-908-03   - Exar Semiconductor XR22-008-03 keyboard matrix capacitive readout latch
    22-950-3B   - Exar Semiconductor XR22-050-3B keyboard matrix row driver with 4 to 12 decoder/demultiplexer
    CN1         - keyboard data connector (SIP, 7 pins, right angle)

    [804x]      - unpopulated space for a 40 pin 804x or 803x MCU
    [EPROM]     - unpopulated space for an EPROM, if a ROMless 803x MCU was used
    [???]       - unpopulated space for an unknown NDIP10 IC or DIP-switch array
    X           - capacitive sensor pad for one key
    marking     - PCB trace marking: "KTC // A65-02307-007 // PCB 201 D"


Key Layout (USA Variant): (the S0x markings appear on the back of the PCB)
|------------------------------------------------------------------------------------=
| 22-908-03 22-950-3B .   XTAL 8021  74LS14     [804x]           [EPROM]  [???]   CN1=___
|         01      02      03      04      05      06       07     08  09     10     11   | <- the leftmost X is S01
| 12   13  14  15  16  17  18  19  20  21  22  23  24  25   26    27  28   29  30  31 32 |
| 33    34  35  36  37  38  39  40  41  42  43  44  45  46   47   48  49   50  51  52 53 |
| 54    55   56  57  58  59  60  61  62  63  64  65  66  67   68  69  70   71  72  73 74 |
| 75   76  77 78  79  80  81  82  83  84  85  86  87      88      89  90   91  92  93 94 |
| 95    96   marking             97                98             99 100  101 102 103 104| <- the rightmost X is S104
|----------------------------------------------------------------------------------------|

   key - Shifted(top)/Unshifted(bottom)/Alt(front) (if no slashes in description assume key has just one symbol on it)
   S01 - [1]
   S02 - [2]
   S03 - [3]
   S04 - [4]
   S05 - [5]
   S06 - [6]
   S07 - [7]
   S08 - [8]
   S09 - UNUSED (under the [8] key, no metal contact on key)
   S10 - [9]
   S11 - [10]
   
   S12 - CLR/HOME
   S13 - (Degree symbol U+00B0)/(+- symbol U+00B1)/(Pi symbol U+03C0)
   S14 - !/1/|
   S15 - @/2/<
   S16 - #/3/>
   S17 - $/4/(centered closed dot U+00B7)
   S18 - %/5/(up arrow symbol U+2191)
   S19 - (cent symbol U+00A2)/6/(logical not symbol U+00AC)
   S20 - &/7/^
   S21 - * /8/`
   S22 - (/9/{
   S23 - )/0/}
   S24 - _/-/~
   S25 - +/=/\
   S26 - BACKSPACE
   S27 - INS
   S28 - DEL
   S29 - MODE CALC/= (white keypad key)
   S30 - % (white keypad key)
   S31 - (division symbol U+00F7) (white keypad key)
   S32 - (multiplication symbol U+00D7) (white keypad key)
   
   S33 - (up arrow, SCRL, down arrow)//VTAB
   S34 - TAB//BACK
   S35 - Q
   S36 - W
   S37 - E
   S38 - R
   S39 - T
   S40 - Y
   S41 - U
   S42 - I
   S43 - O
   S44 - P
   S45 - (1/4 symbol U+00BC)/(1/2 symbol U+00BD)
   S46 - [/]
   S47 - UNUSED (under the RETURN key, no metal contact on key)
   S48 - ERASE/EOL
   S49 - REQ/CAN
   S50 - 7 (white keypad key)
   S51 - 8 (white keypad key)
   S52 - 9 (white keypad key)
   S53 - - (white keypad key)
   
   S54 - (OFF,RVS,ON)//ESC
   S55 - LOCK//CAPS LOCK
   S56 - A
   S57 - S
   S58 - D
   S59 - F
   S60 - G
   S61 - H
   S62 - J
   S63 - K
   S64 - L
   S65 - :/;
   S66 - "/'
   S67 - UNUSED (under the RETURN key, no metal contact on key)
   S68 - RETURN
   S69 - WORD/(left arrow U+2190)/(volume up U+1F508 plus U+25B4) (i.e. 'Previous Word')
   S70 - WORD/(right arrow U+2192)/(volume down U+1F508 plus U+25BE) (i.e. 'Next Word')
   S71 - 4 (white keypad key)
   S72 - 5 (white keypad key)
   S73 - 6 (white keypad key)
   S74 - + (white keypad key)
   
   S75 - (OFF,UNDL,ON)
   S76 - SHIFT (left shift)
   S77 - UNUSED (under the left SHIFT key, no metal contact on key)
   S78 - Z
   S79 - X
   S80 - C
   S81 - V
   S82 - B
   S83 - N
   S84 - M
   S85 - ,/, (yes, both are comma)
   S86 - ./. (yes, both are period/fullstop)
   S87 - ?// (this is the actual / key)
   S88 - SHIFT (right shift)
   S89 - (up arrow U+2191)//(brightness up U+263C plus U+25B4)
   S90 - (down arrow U+2193)//(brightness down U+263C plus U+25BE)
   S91 - 1 (white keypad key)
   S92 - 2 (white keypad key)
   S93 - 3 (white keypad key)
   S94 - ENTER (white keypad key)
   
   S95 - RPT
   S96 - ALT
   S97 - (spacebar)
   S98 - PAUSE/CONT
   S99 - (left arrow U+2190)//(contrast up U+25D0 plus U+25B4) (U+1F313 can be used in place of U+25D0)
   S100 - (right arrow U+2192)//(contrast down U+25D0 plus U+25BE) ''
   S101 - 0 (white keypad key)
   S102 - 00 (white keypad key) ('double zero')
   S103 - . (white keypad key)
   S104 - UNUSED (under the ENTER (keypad) key, no metal contact on key)
   
   Note that the five unused key contacts:
   S09, S47, S67, S77 and S104
   may be used on international variants of the Victor 9000/Sirius 1 Keyboard.
   
*/

#include "victor9kb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8021_TAG       "z3"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VICTOR9K_KEYBOARD = &device_creator<victor9k_keyboard_device>;


//-------------------------------------------------
//  ROM( victor9k_keyboard )
//-------------------------------------------------

ROM_START( victor9k_keyboard )
	ROM_REGION( 0x400, I8021_TAG, 0)
	ROM_LOAD( "20-8021-139.z3", 0x000, 0x400, CRC(0fe9d53d) SHA1(61d92ba90f98f8978bbd9303c1ac3134cde8cdcb) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *victor9k_keyboard_device::device_rom_region() const
{
	return ROM_NAME( victor9k_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

static ADDRESS_MAP_START( victor9k_keyboard_io, AS_IO, 8, victor9k_keyboard_device )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(kb_p1_r, kb_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(kb_p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(kb_t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( victor9k_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( victor9k_keyboard )
	MCFG_CPU_ADD(I8021_TAG, I8021, XTAL_3_579545MHz)
	MCFG_CPU_IO_MAP(victor9k_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor victor9k_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( victor9k_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( victor9k_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( victor9k_keyboard )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor victor9k_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( victor9k_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  victor9k_keyboard_device - constructor
//-------------------------------------------------

victor9k_keyboard_device::victor9k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VICTOR9K_KEYBOARD, "Victor 9000 Keyboard", tag, owner, clock, "victor9kb", __FILE__),
	m_maincpu(*this, I8021_TAG),
	m_y0(*this, "Y0"),
	m_y1(*this, "Y1"),
	m_y2(*this, "Y2"),
	m_y3(*this, "Y3"),
	m_y4(*this, "Y4"),
	m_y5(*this, "Y5"),
	m_y6(*this, "Y6"),
	m_y7(*this, "Y7"),
	m_y8(*this, "Y8"),
	m_y9(*this, "Y9"),
	m_ya(*this, "YA"),
	m_yb(*this, "YB"),
	m_yc(*this, "YC"),
	m_kbrdy_handler(*this),
	m_kbdata_handler(*this),
	m_y(0),
	m_kbrdy(-1),
	m_kbdata(-1),
	m_kback(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victor9k_keyboard_device::device_start()
{
	// resolve callbacks
	m_kbrdy_handler.resolve_safe();
	m_kbdata_handler.resolve_safe();

	// state saving
	save_item(NAME(m_y));
	save_item(NAME(m_kbrdy));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_kback));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victor9k_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  kback_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( victor9k_keyboard_device::kback_w )
{
	//logerror("KBACK %u\n", state);
	m_kback = state;
}


//-------------------------------------------------
//  kb_p1_r -
//-------------------------------------------------

READ8_MEMBER( victor9k_keyboard_device::kb_p1_r )
{
	UINT8 data = 0xff;

	switch (m_y)
	{
		case 0: data &= m_y0->read(); break;
		case 1: data &= m_y1->read(); break;
		case 2: data &= m_y2->read(); break;
		case 3: data &= m_y3->read(); break;
		case 4: data &= m_y4->read(); break;
		case 5: data &= m_y5->read(); break;
		case 6: data &= m_y6->read(); break;
		case 7: data &= m_y7->read(); break;
		case 8: data &= m_y8->read(); break;
		case 9: data &= m_y9->read(); break;
		case 0xa: data &= m_ya->read(); break;
		case 0xb: data &= m_yb->read(); break;
		case 0xc: data &= m_yc->read(); break;
	}

	return data;
}


//-------------------------------------------------
//  kb_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( victor9k_keyboard_device::kb_p1_w )
{
	if ((data & 0xf0) == 0x20)
	{
		m_y = data & 0x0f;
	}

	//logerror("%s P1 %02x\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  kb_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( victor9k_keyboard_device::kb_p2_w )
{
	/*

	    bit     description

	    P20     ?
	    P21     KBRDY
	    P22     ?
	    P23     KBDATA

	*/

	int kbrdy = BIT(data, 1);

	if (m_kbrdy != kbrdy)
	{
		m_kbrdy = kbrdy;
		m_kbrdy_handler(m_kbrdy);
	}

	int kbdata = BIT(data, 3);

	if (m_kbdata != kbdata)
	{
		m_kbdata = kbdata;
		m_kbdata_handler(m_kbdata);
	}

	//logerror("%s P2 %01x\n", machine().describe_context(), data&0x0f);
}


//-------------------------------------------------
//  kb_t1_r -
//-------------------------------------------------

READ8_MEMBER( victor9k_keyboard_device::kb_t1_r )
{
	return m_kback;
}
