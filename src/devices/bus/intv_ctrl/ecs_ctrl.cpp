// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Mattel Intellivision ECS hack for controller port emulation
 
    FIXME: This device is the best I could come up with to emulate
    the fact that Keyboard and Synth controllers for the ECS should be
    plugged in both ECS control ports, while the 3rd and 4th additional
    hand controller should only be plugged in a single port.
    Since the core currently does not allow a single device mounted
    in more than a slot, this has been worked around with this device
    which supports as options
    - ECS keyboard
    - ECS synth 
    - a pair of Intellivision controller
 
    All the code for both the controller port and the slot devices has
    been included in this single source file to make easier to clean
    them up once we extend the core to support this kind of setup
    (necessary for Atari 2600 Compumate as well)

**********************************************************************/

#include "ecs_ctrl.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type INTVECS_CONTROL_PORT = &device_creator<intvecs_control_port_device>;


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_intvecs_control_port_interface - constructor
//-------------------------------------------------

device_intvecs_control_port_interface::device_intvecs_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_port = dynamic_cast<intvecs_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_intvecs_control_port_interface - destructor
//-------------------------------------------------

device_intvecs_control_port_interface::~device_intvecs_control_port_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  intvecs_control_port_device - constructor
//-------------------------------------------------

intvecs_control_port_device::intvecs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, INTVECS_CONTROL_PORT, "Mattel Intellivision ECS control port (HACK)", tag, owner, clock, "intvecs_control_port", __FILE__),
						device_slot_interface(mconfig, *this), m_device(nullptr)
{
}


//-------------------------------------------------
//  intvecs_control_port_device - destructor
//-------------------------------------------------

intvecs_control_port_device::~intvecs_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void intvecs_control_port_device::device_start()
{
	m_device = dynamic_cast<device_intvecs_control_port_interface *>(get_card_device());
}


UINT8 intvecs_control_port_device::read_portA()
{
	UINT8 data = 0;
	if (m_device)
		data |= m_device->read_portA();
	return data;
}

UINT8 intvecs_control_port_device::read_portB()
{
    UINT8 data = 0;
    if (m_device)
        data |= m_device->read_portB();
    return data;
}

void intvecs_control_port_device::write_portA(UINT8 data)
{
    if (m_device)
        m_device->write_portA(data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( intvecs_control_port_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( intvecs_control_port_devices )
	SLOT_INTERFACE("ctrls", ECS_CTRLS)
    SLOT_INTERFACE("keybd", ECS_KEYBD)
    SLOT_INTERFACE("synth", ECS_SYNTH)
SLOT_INTERFACE_END





//**************************************************************************
//  ACTUAL SLOT DEVICES - included here until core issues are solved...
//**************************************************************************


//-------------------------------------------------
//  ECS_CTRLS - A pair of hand controllers
//-------------------------------------------------

const device_type ECS_CTRLS = &device_creator<intvecs_ctrls_device>;

static SLOT_INTERFACE_START( intvecs_controller )
    SLOT_INTERFACE("handctrl", INTV_HANDCTRL)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( intvecs_ctrls )
    MCFG_INTV_CONTROL_PORT_ADD("port1", intvecs_controller, "handctrl")
    MCFG_INTV_CONTROL_PORT_ADD("port2", intvecs_controller, "handctrl")
MACHINE_CONFIG_END


machine_config_constructor intvecs_ctrls_device::device_mconfig_additions() const
{
    return MACHINE_CONFIG_NAME( intvecs_ctrls );
}

intvecs_ctrls_device::intvecs_ctrls_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, ECS_CTRLS, "Mattel Intellivision ECS Hand Controller x2 (HACK)", tag, owner, clock, "intvecs_ctrls", __FILE__),
    device_intvecs_control_port_interface(mconfig, *this),
    m_hand1(*this, "port1"),
    m_hand2(*this, "port2")
{
}

void intvecs_ctrls_device::device_start()
{
}

void intvecs_ctrls_device::device_reset()
{
}

UINT8 intvecs_ctrls_device::read_portA()
{
    return m_hand1->read_ctrl();
}

UINT8 intvecs_ctrls_device::read_portB()
{
    return m_hand2->read_ctrl();
}

//-------------------------------------------------
//  ECS_KEYBD - Keyboard
//-------------------------------------------------

const device_type ECS_KEYBD = &device_creator<intvecs_keybd_device>;

static INPUT_PORTS_START( intvecs_keybd )
/*
 ECS matrix scanned by setting 0xFE bits to output and reading 0xFF
 ECS Keyboard Layout:
 FF\FE  Bit 7   Bit 6   Bit 5   Bit 4   Bit 3   Bit 2   Bit 1   Bit 0
 Bit 0  NC      RTN     0       ESC     P       ;       .       (left)
 Bit 1  L       O       8       9       I       K       M       ,
 Bit 2  J       U       6       7       Y       H       B       N
 Bit 3  G       T       4       5       R       F       C       V
 Bit 4  D       E       2       3       W       S       Z       X
 Bit 5  A       CTL     (right) 1       Q       (up)    (down)  (space)
 Bit 6  SHIFT   NC      NC      NC      NC      NC      NC      NC
 
 Shifted keys that differ from pc:
 Key        : 1 2 5 6 7 (left) (right) (up) (down)
 Shift + key: = " + - /  %      '       ^    ?
 */

    PORT_START("ROW.0")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RTN")           PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)       PORT_CHAR('0') PORT_CHAR(')')
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)     PORT_CHAR(UCHAR_MAMEKEY(ESC))
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)       PORT_CHAR('P')
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)   PORT_CHAR(';') PORT_CHAR(':')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)    PORT_CHAR('.')
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)    PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR('%')

    PORT_START("ROW.1")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)       PORT_CHAR('L')
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)       PORT_CHAR('O')
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('*')
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR('(')
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)       PORT_CHAR('I')
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)       PORT_CHAR('K')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)       PORT_CHAR('M')
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',')

    PORT_START("ROW.2")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)       PORT_CHAR('J')
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)       PORT_CHAR('U')
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR('-')
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('/')
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)       PORT_CHAR('Y')
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)       PORT_CHAR('H')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)       PORT_CHAR('B')
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)       PORT_CHAR('N')

    PORT_START("ROW.3")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)       PORT_CHAR('G')
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)       PORT_CHAR('T')
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)       PORT_CHAR('4') PORT_CHAR('$')
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)       PORT_CHAR('R')
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)       PORT_CHAR('F')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)       PORT_CHAR('C')
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)       PORT_CHAR('V')

    PORT_START("ROW.4")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)       PORT_CHAR('D')
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)       PORT_CHAR('E')
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)       PORT_CHAR('2') PORT_CHAR('"')
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)       PORT_CHAR('W')
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)       PORT_CHAR('S')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)       PORT_CHAR('Z')
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)       PORT_CHAR('X')

    PORT_START("ROW.5")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)       PORT_CHAR('A')
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTL") PORT_CODE(KEYCODE_RCONTROL) PORT_CODE(KEYCODE_LCONTROL)
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)   PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR('\'')
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('=')
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)       PORT_CHAR('Q')
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)      PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR('^')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR('?')
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)   PORT_CHAR(' ')

    PORT_START("ROW.6")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor intvecs_keybd_device::device_input_ports() const
{
    return INPUT_PORTS_NAME( intvecs_keybd );
}

intvecs_keybd_device::intvecs_keybd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, ECS_KEYBD, "Mattel Intellivision ECS Keyboard", tag, owner, clock, "intvecs_keybd", __FILE__),
    device_intvecs_control_port_interface(mconfig, *this),
    m_keybd(*this, "ROW")
{
}

void intvecs_keybd_device::device_start()
{
    save_item(NAME(m_psg_portA));
}

void intvecs_keybd_device::device_reset()
{
    m_psg_portA = 0;
}

UINT8 intvecs_keybd_device::read_portB()
{
    UINT8 val = 0xff;
    // return correct result if more than one bit of 0xFE is set
    for (int i = 0; i < 7; i++)
    {
        if (BIT(m_psg_portA, i))
            val &= m_keybd[i]->read();
    }
    return val;
}

void intvecs_keybd_device::write_portA(UINT8 data)
{
    m_psg_portA = (~data) & 0xff;
}



//-------------------------------------------------
//  ECS_SYNTH - Synth
//-------------------------------------------------

const device_type ECS_SYNTH = &device_creator<intvecs_synth_device>;


static INPUT_PORTS_START( intvecs_synth )
/*
 ECS Synthesizer Layout:
 FF\FE  Bit 7   Bit 6   Bit 5   Bit 4   Bit 3   Bit 2   Bit 1   Bit 0
 Bit 0  G2      Gb2     F2      E2      Eb2     D2      Db2     C2
 Bit 1  Eb3     D3      Db3     C3      B2      Bb2     A2      Ab2
 Bit 2  B3      Bb3     A3      Ab3     G3      Gb3     F3      E3
 Bit 3  G4      Gb4     F4      E4      Eb4     D4      Db4     C4
 Bit 4  Eb5     D5      Db5     C5      B4      Bb4     A4      Ab4
 Bit 5  B5      Bb5     A5      Ab5     G5      Gb5     F5      E5
 Bit 6  C6      NC      NC      NC      NC      NC      NC      NC
 */
    PORT_START("SYNTH.0")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G2")
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Gb2")
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E2")
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Eb2")
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D2")
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Db2")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C2")

    PORT_START("SYNTH.1")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Eb3")
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D3")
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Db3")
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C3")
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B2")
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bb2")
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A2")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ab2")

    PORT_START("SYNTH.2")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B3")
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bb3")
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A3")
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ab3")
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G3")
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Gb3")
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E3")

    PORT_START("SYNTH.3")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G4")
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Gb4")
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E4")
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Eb4")
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D4")
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Db4")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C4")

    PORT_START("SYNTH.4")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Eb5")
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D5")
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Db5")
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C5")
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B4")
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bb4")
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A4")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ab4")

    PORT_START("SYNTH.5")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B5")
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bb5")
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A5")
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ab5")
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G5")
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Gb5")
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E5")

    PORT_START("SYNTH.6")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C6")
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


ioport_constructor intvecs_synth_device::device_input_ports() const
{
    return INPUT_PORTS_NAME( intvecs_synth );
}


intvecs_synth_device::intvecs_synth_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, ECS_SYNTH, "Mattel Intellivision ECS Synthetizer", tag, owner, clock, "intvecs_synth", __FILE__),
    device_intvecs_control_port_interface(mconfig, *this),
    m_synth(*this, "SYNTH")
{
}

void intvecs_synth_device::device_start()
{
    save_item(NAME(m_psg_portA));
}

void intvecs_synth_device::device_reset()
{
    m_psg_portA = 0;
}

UINT8 intvecs_synth_device::read_portB()
{
    UINT8 val = 0xff;
    // return correct result if more than one bit of 0xFE is set
    for (int i = 0; i < 7; i++)
    {
        if (BIT(m_psg_portA, i))
            val &= m_synth[i]->read();
    }
    return val;
}

void intvecs_synth_device::write_portA(UINT8 data)
{
    m_psg_portA = (~data) & 0xff;
}

