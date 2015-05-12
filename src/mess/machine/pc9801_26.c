// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-26 sound card

    Legacy sound card for PC-98xx family, composed by a single YM2203

    TODO:
    - joystick code should be shared between -26, -86 and -118

***************************************************************************/

#include "emu.h"
#include "machine/pc9801_26.h"
#include "machine/pic8259.h"
#include "sound/2203intf.h"

#define MAIN_CLOCK_X1 XTAL_1_9968MHz

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type PC9801_26 = &device_creator<pc9801_26_device>;



READ8_MEMBER(pc9801_26_device::opn_porta_r)
{
	if(m_joy_sel & 0x80)
		return ioport(m_joy_sel & 0x40 ? "OPN_PA2" : "OPN_PA1")->read();

	return 0xff;
}

WRITE8_MEMBER(pc9801_26_device::opn_portb_w){ m_joy_sel = data; }

WRITE_LINE_MEMBER(pc9801_26_device::pc9801_sound_irq)
{
	/* TODO: seems to die very often */
	machine().device<pic8259_device>(":pic8259_slave")->ir4_w(state);
}

static MACHINE_CONFIG_FRAGMENT( pc9801_26_config )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("opn", YM2203, MAIN_CLOCK_X1*2) // unknown clock / divider
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(pc9801_26_device, pc9801_sound_irq))
	MCFG_AY8910_PORT_A_READ_CB(READ8(pc9801_26_device, opn_porta_r))
	//MCFG_AY8910_PORT_B_READ_CB(READ8(pc9801_state, opn_portb_r))
	//MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(pc9801_state, opn_porta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(pc9801_26_device, opn_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor pc9801_26_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pc9801_26_config );
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( pc9801_26 )
	PORT_START("OPN_PA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OPN_PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Joystick Button 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OPN_DSW")
	PORT_CONFNAME( 0x01, 0x01, "PC-9801-26: Port Base" )
	PORT_CONFSETTING(    0x00, "0x088" )
	PORT_CONFSETTING(    0x01, "0x188" )
INPUT_PORTS_END

ioport_constructor pc9801_26_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_26 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_26_device - constructor
//-------------------------------------------------

pc9801_26_device::pc9801_26_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC9801_26, "pc9801_26", tag, owner, clock, "pc9801_26", __FILE__),
//      m_maincpu(*owner, "maincpu"),
		m_opn(*this, "opn")
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void pc9801_26_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc9801_26_device::install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
{
	int buswidth = machine().firstcpu->space_config(AS_IO)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			machine().firstcpu->space(AS_IO).install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0);
			break;
		case 16:
			machine().firstcpu->space(AS_IO).install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0xffff);
			break;
		case 32:
			machine().firstcpu->space(AS_IO).install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0xffffffff);
			break;
		default:
			fatalerror("PC-9801-26: Bus width %d not supported\n", buswidth);
	}
}


void pc9801_26_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_26_device::device_reset()
{
	UINT16 port_base = (ioport("OPN_DSW")->read() & 1) << 8;
	install_device(port_base + 0x0088, port_base + 0x008b, 0, 0, read8_delegate(FUNC(pc9801_26_device::pc9801_26_r), this), write8_delegate(FUNC(pc9801_26_device::pc9801_26_w), this) );
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


READ8_MEMBER(pc9801_26_device::pc9801_26_r)
{
	if((offset & 1) == 0)
	{
		return offset & 4 ? 0xff : m_opn->read(space, offset >> 1);
	}
	else // odd
	{
		printf("Read to undefined port [%02x]\n",offset+0x188);
		return 0xff;
	}
}


WRITE8_MEMBER(pc9801_26_device::pc9801_26_w)
{
	if((offset & 5) == 0)
		m_opn->write(space, offset >> 1, data);
	else // odd
		printf("PC9801-26: Write to undefined port [%02x] %02x\n",offset+0x188,data);
}
