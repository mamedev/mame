// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-118 sound card

    YMF288 + some extra ports

    TODO:
    - preliminary, presumably needs CS-4231 too
    - joystick code should be shared between -26, -86 and -118

***************************************************************************/

#include "emu.h"
#include "machine/pc9801_118.h"
#include "machine/pic8259.h"
#include "sound/2608intf.h"

#define MAIN_CLOCK_X2 XTAL_2_4576MHz

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type PC9801_118 = &device_creator<pc9801_118_device>;


READ8_MEMBER(pc9801_118_device::opn_porta_r)
{
	if(m_joy_sel & 0x80)
		return ioport(m_joy_sel & 0x40 ? "OPN3_PA2" : "OPN3_PA1")->read();

	return 0xff;
}

WRITE8_MEMBER(pc9801_118_device::opn_portb_w){ m_joy_sel = data; }

WRITE_LINE_MEMBER(pc9801_118_device::pc9801_sound_irq)
{
	/* TODO: seems to die very often */
	machine().device<pic8259_device>(":pic8259_slave")->ir4_w(state);
}

static MACHINE_CONFIG_FRAGMENT( pc9801_118_config )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("opn3", YM2608, MAIN_CLOCK_X2*4) // actually YMF288, unknown clock / divider, might be X1 x 5 actually
	MCFG_YM2608_IRQ_HANDLER(WRITELINE(pc9801_118_device, pc9801_sound_irq))
	MCFG_AY8910_PORT_A_READ_CB(READ8(pc9801_118_device, opn_porta_r))
	//MCFG_AY8910_PORT_B_READ_CB(READ8(pc9801_state, opn_portb_r))
	//MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(pc9801_state, opn_porta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(pc9801_118_device, opn_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor pc9801_118_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pc9801_118_config );
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( pc9801_118 )
	PORT_START("OPN3_PA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OPN3_PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Joystick Button 2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OPN3_DSW")
	PORT_CONFNAME( 0x01, 0x00, "PC-9801-118: Port Base" )
	PORT_CONFSETTING(    0x00, "0x088" )
	PORT_CONFSETTING(    0x01, "0x188" )
INPUT_PORTS_END

ioport_constructor pc9801_118_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_118 );
}

// RAM
ROM_START( pc9801_118 )
	ROM_REGION( 0x100000, "opn3", ROMREGION_ERASE00 )
ROM_END

const rom_entry *pc9801_118_device::device_rom_region() const
{
	return ROM_NAME( pc9801_118 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_118_device - constructor
//-------------------------------------------------

pc9801_118_device::pc9801_118_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC9801_118, "pc9801_118", tag, owner, clock, "pc9801_118", __FILE__),
//      m_maincpu(*owner, "maincpu"),
		m_opn3(*this, "opn3")
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void pc9801_118_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc9801_118_device::install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
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
			fatalerror("PC-9801-118: Bus width %d not supported\n", buswidth);
	}
}


void pc9801_118_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_118_device::device_reset()
{
	UINT16 port_base = (ioport("OPN3_DSW")->read() & 1) << 8;
	install_device(port_base + 0x0088, port_base + 0x008f, 0, 0, read8_delegate(FUNC(pc9801_118_device::pc9801_118_r), this), write8_delegate(FUNC(pc9801_118_device::pc9801_118_w), this) );
	install_device(0xa460, 0xa463, 0, 0, read8_delegate(FUNC(pc9801_118_device::pc9801_118_ext_r), this), write8_delegate(FUNC(pc9801_118_device::pc9801_118_ext_w), this) );
	m_ext_reg = 1; // TODO: enabled or disabled?
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


READ8_MEMBER(pc9801_118_device::pc9801_118_r)
{
	if(((offset & 5) == 0) || m_ext_reg)
		return m_opn3->read(space, offset >> 1);
	else // odd
	{
		//printf("PC9801-118: Read to undefined port [%02x]\n",offset+0x188);
		return 0xff;
	}
}


WRITE8_MEMBER(pc9801_118_device::pc9801_118_w)
{
	if(((offset & 5) == 0) || m_ext_reg)
		m_opn3->write(space, offset >> 1,data);
	//else // odd
	//  printf("PC9801-118: Write to undefined port [%02x] %02x\n",offset+0x188,data);
}

READ8_MEMBER( pc9801_118_device::pc9801_118_ext_r )
{
	if(offset == 0)
	{
		printf("OPN3 EXT read ID [%02x]\n",offset);
		return 0x80 | (m_ext_reg & 1);
	}

	printf("OPN3 EXT read unk [%02x]\n",offset);
	return 0xff;
}

WRITE8_MEMBER( pc9801_118_device::pc9801_118_ext_w )
{
	if(offset == 0)
	{
		m_ext_reg = data & 1;
		/* TODO: apparently writing a 1 doubles the available channels (and presumably enables CS-4231 too) */
		if(data)
			printf("PC-9801-118: extended register %02x write\n",data);
		return;
	}

	printf("OPN3 EXT write unk %02x -> [%02x]\n",data,offset);
}
