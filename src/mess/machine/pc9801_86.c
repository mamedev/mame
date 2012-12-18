/***************************************************************************

	NEC PC-9801-86 sound card

	Almost the same thing as PC-9801-86, but this one has YM2608 instead of
	YM2203

***************************************************************************/

#include "emu.h"
#include "machine/pc9801_86.h"
#include "machine/pic8259.h"
#include "sound/2608intf.h"

#define MAIN_CLOCK_X1 XTAL_1_9968MHz

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type PC9801_86 = &device_creator<pc9801_86_device>;


READ8_MEMBER(pc9801_86_device::opn_porta_r)
{
	if(m_joy_sel == 0x80)
		return ioport("OPNA_PA1")->read();

	if(m_joy_sel == 0xc0)
		return ioport("OPNA_PA2")->read();

//  0x81?
//  printf("%02x\n",m_joy_sel);
	return 0xff;
}

WRITE8_MEMBER(pc9801_86_device::opn_portb_w){ m_joy_sel = data; }

static void pc9801_sound_irq( device_t *device, int irq )
{
//  pc9801_state *state = device->machine().driver_data<pc9801_state>();

	/* TODO: seems to die very often */
	pic8259_ir4_w(device->machine().device("pic8259_slave"), irq);
}

static const ym2608_interface pc98_ym2608_intf =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, pc9801_86_device,opn_porta_r),
		DEVCB_NULL,//(pc9801_state,opn_portb_r),
		DEVCB_NULL,//(pc9801_state,opn_porta_w),
		DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, pc9801_86_device,opn_portb_w),
	},
	pc9801_sound_irq
};

static MACHINE_CONFIG_FRAGMENT( pc9801_86_config )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("opna", YM2608, MAIN_CLOCK_X1*4) // unknown clock / divider
	MCFG_SOUND_CONFIG(pc98_ym2608_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor pc9801_86_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pc9801_86_config );
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( pc9801_86 )
	PORT_START("OPNA_PA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OPNA_PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Joystick Button 2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor pc9801_86_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_86 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_86_device - constructor
//-------------------------------------------------

pc9801_86_device::pc9801_86_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC9801_86, "pc9801_86", tag, owner, clock),
//		m_maincpu(*owner, "maincpu"),
		m_opna(*this, "opna")
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void pc9801_86_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc9801_86_device::install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
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
			fatalerror("PC-9801-86: Bus width %d not supported\n", buswidth);
			break;
	}
}


void pc9801_86_device::device_start()
{
	install_device(0x0188, 0x018f, 0, 0, read8_delegate(FUNC(pc9801_86_device::pc9801_86_r), this), write8_delegate(FUNC(pc9801_86_device::pc9801_86_w), this) );
//	install_device(0xa460, 0xa463, 0, 0, read8_delegate(FUNC(pc9801_86_device::pc9801_86_ext_r), this), write8_delegate(FUNC(pc9801_86_device::pc9801_86_ext_w), this) );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_86_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


READ8_MEMBER(pc9801_86_device::pc9801_86_r)
{
	if((offset & 1) == 0)
		return ym2608_r(m_opna, space, offset >> 1);
	else // odd
	{
		printf("PC9801-86: Read to undefined port [%02x]\n",offset+0x188);
		return 0xff;
	}
}


WRITE8_MEMBER(pc9801_86_device::pc9801_86_w)
{
	if((offset & 1) == 0)
		ym2608_w(m_opna,space, offset >> 1,data);
	else // odd
		printf("PC9801-86: Write to undefined port [%02x] %02x\n",offset+0x188,data);
}

#if 0
READ8_MEMBER( pc9801_86_device::pc9801_86_ext_r )
{
	if(offset == 0)
	{
		printf("OPNA EXT read ID [%02x]\n",offset);
		return 0xff;
	}

	printf("OPNA EXT read unk [%02x]\n",offset);
	return 0xff;
}

WRITE8_MEMBER( pc9801_86_device::pc9801_86_ext_w )
{
	if(offset == 0)
	{
		printf("OPNA EXT write mask %02x -> [%02x]\n",data,offset);
		return;
	}

	printf("OPNA EXT write unk %02x -> [%02x]\n",data,offset);
}
#endif
