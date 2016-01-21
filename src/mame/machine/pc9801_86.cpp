// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-86 sound card

    Almost the same thing as PC-9801-86, but this one has YM2608 instead of
    YM2203

    TODO:
    - joystick code should be shared between -26, -86 and -118
    - Test all pcm modes
    - Make volume work
    - Recording

***************************************************************************/

#include "emu.h"
#include "machine/pc9801_86.h"

#define MAIN_CLOCK_X1 XTAL_1_9968MHz
#define QUEUE_SIZE 32768

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type PC9801_86 = &device_creator<pc9801_86_device>;


READ8_MEMBER(pc9801_86_device::opn_porta_r)
{
	if(m_joy_sel & 0x80)
		return ioport(m_joy_sel & 0x40 ? "OPNA_PA2" : "OPNA_PA1")->read();

	return 0xff;
}

WRITE8_MEMBER(pc9801_86_device::opn_portb_w){ m_joy_sel = data; }

WRITE_LINE_MEMBER(pc9801_86_device::sound_irq)
{
	m_fmirq = state ? true : false;
	/* TODO: seems to die very often */
	machine().device<pic8259_device>(":pic8259_slave")->ir4_w(state || (m_pcmirq ? ASSERT_LINE : CLEAR_LINE));
}

static MACHINE_CONFIG_FRAGMENT( pc9801_86_config )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("opna", YM2608, MAIN_CLOCK_X1*4) // unknown clock / divider
	MCFG_YM2608_IRQ_HANDLER(WRITELINE(pc9801_86_device, sound_irq))
	MCFG_AY8910_PORT_A_READ_CB(READ8(pc9801_86_device, opn_porta_r))
	//MCFG_AY8910_PORT_B_READ_CB(READ8(pc9801_state, opn_portb_r))
	//MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(pc9801_state, opn_porta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(pc9801_86_device, opn_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)
	MCFG_SOUND_ADD("dacl", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ADD("dacr", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor pc9801_86_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pc9801_86_config );
}

// RAM
ROM_START( pc9801_86 )
	ROM_REGION( 0x100000, "opna", ROMREGION_ERASE00 )
ROM_END

const rom_entry *pc9801_86_device::device_rom_region() const
{
	return ROM_NAME( pc9801_86 );
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

	PORT_START("OPNA_DSW")
	PORT_CONFNAME( 0x01, 0x01, "PC-9801-86: Port Base" )
	PORT_CONFSETTING(    0x00, "0x088" )
	PORT_CONFSETTING(    0x01, "0x188" )
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
	: device_t(mconfig, PC9801_86, "pc9801_86", tag, owner, clock, "pc9801_86", __FILE__),
		m_opna(*this, "opna"),
		m_dacl(*this, "dacl"),
		m_dacr(*this, "dacr"),
		m_queue(QUEUE_SIZE)
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
	}
}


void pc9801_86_device::device_start()
{
	m_dac_timer = timer_alloc();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_86_device::device_reset()
{
	UINT16 port_base = (ioport("OPNA_DSW")->read() & 1) << 8;
	install_device(port_base + 0x0088, port_base + 0x008f, 0, 0, read8_delegate(FUNC(pc9801_86_device::opn_r), this), write8_delegate(FUNC(pc9801_86_device::opn_w), this) );
	install_device(0xa460, 0xa463, 0, 0, read8_delegate(FUNC(pc9801_86_device::id_r), this), write8_delegate(FUNC(pc9801_86_device::mask_w), this));
	install_device(0xa464, 0xa46f, 0, 0, read8_delegate(FUNC(pc9801_86_device::pcm_r), this), write8_delegate(FUNC(pc9801_86_device::pcm_w), this));
	m_mask = 0;
	m_head = m_tail = m_count = 0;
	m_fmirq = m_pcmirq = false;
	m_irq_rate = 0;
	m_pcm_ctrl = m_pcm_mode = 0;
	memset(&m_queue[0], 0, QUEUE_SIZE);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


READ8_MEMBER(pc9801_86_device::opn_r)
{
	if((offset & 1) == 0)
		return m_opna->read(space, offset >> 1);
	else // odd
	{
		logerror("PC9801-86: Read to undefined port [%02x]\n",offset+0x188);
		return 0xff;
	}
}

WRITE8_MEMBER(pc9801_86_device::opn_w)
{
	if((offset & 1) == 0)
		m_opna->write(space, offset >> 1,data);
	else // odd
		logerror("PC9801-86: Write to undefined port [%02x] %02x\n",offset+0x188,data);
}

READ8_MEMBER(pc9801_86_device::id_r)
{
	return 0x40 | m_mask;
}

WRITE8_MEMBER(pc9801_86_device::mask_w)
{
	m_mask = data & 1;
}

READ8_MEMBER(pc9801_86_device::pcm_r)
{
	if((offset & 1) == 0)
	{
		switch(offset >> 1)
		{
			case 1:
				return ((queue_count() == QUEUE_SIZE) ? 0x80 : 0) |
						(!queue_count() ? 0x40 : 0);
			case 2:
				return m_pcm_ctrl | (m_pcmirq ? 0x10 : 0);
			case 3:
				return m_pcm_mode;
			case 4:
				return 0;
			case 5:
				return m_pcm_mute;
		}
	}
	else // odd
		logerror("PC9801-86: Read to undefined port [%02x]\n",offset+0xa464);
	return 0xff;
}

WRITE8_MEMBER(pc9801_86_device::pcm_w)
{
	const int rates[] = {44100, 33075, 22050, 16538, 11025, 8268, 5513, 4134};
	if((offset & 1) == 0)
	{
		switch(offset >> 1)
		{
			case 1:
				m_vol[data >> 5] = data & 0x0f;
				break;
			case 2:
				m_pcm_ctrl = data & ~0x10;
				if(data & 0x80)
					m_dac_timer->adjust(attotime::from_hz(rates[data & 7]), 0, attotime::from_hz(rates[data & 7]));
				else
					m_dac_timer->adjust(attotime::never);
				if(data & 8)
					m_head = m_tail = m_count = 0;
				if(!(data & 0x10))
				{
					m_pcmirq = false;
					machine().device<pic8259_device>(":pic8259_slave")->ir4_w(m_fmirq ? ASSERT_LINE : CLEAR_LINE);
				}
				break;
			case 3:
				if(m_pcm_ctrl & 0x20)
					m_irq_rate = (data + 1) * 128;
				else
					m_pcm_mode = data;
				break;
			case 4:
				if(queue_count() < QUEUE_SIZE)
				{
					m_queue[m_head++] = data;
					m_head %= QUEUE_SIZE;
					m_count++;
				}
				break;
			case 5:
				m_pcm_mute = data;
				break;
		}
	}
	else // odd
		logerror("PC9801-86: Write to undefined port [%02x] %02x\n",offset+0xa464,data);
}

int pc9801_86_device::queue_count()
{
	return m_count;
}

UINT8 pc9801_86_device::queue_pop()
{
	if(!queue_count())
		return 0;
	UINT8 ret = m_queue[m_tail++];
	m_tail %= QUEUE_SIZE;
	m_count--;
	return ret;
}

void pc9801_86_device::device_timer(emu_timer& timer, device_timer_id id, int param, void* ptr)
{
	INT16 lsample, rsample;

	if((m_pcm_ctrl & 0x40) || !(m_pcm_ctrl & 0x80) || !queue_count())
		return;

	switch(m_pcm_mode & 0x70)
	{
		case 0x70: // 8bit stereo
			m_dacl->write(queue_pop() << 8);
			m_dacr->write(queue_pop() << 8);
			break;
		case 0x60: // 8bit left only
			m_dacl->write(queue_pop() << 8);
			break;
		case 0x50: // 8bit right only
			m_dacr->write(queue_pop() << 8);
			break;
		case 0x30: // 16bit stereo
			lsample = queue_pop();
			lsample |= queue_pop() << 8;
			rsample = queue_pop();
			rsample |= queue_pop() << 8;
			m_dacl->write(lsample);
			m_dacr->write(rsample);
			break;
		case 0x20: // 16bit left only
			lsample = queue_pop();
			lsample |= queue_pop() << 8;
			m_dacl->write(lsample);
			break;
		case 0x10: // 16bit right only
			rsample = queue_pop();
			rsample |= queue_pop() << 8;
			m_dacr->write(rsample);
			break;
	}
	if((queue_count() < m_irq_rate) && (m_pcm_ctrl & 0x20))
	{
		m_pcmirq = true;
		machine().device<pic8259_device>(":pic8259_slave")->ir4_w(ASSERT_LINE);
	}
}
