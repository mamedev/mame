// license:BSD-3-Clause
// copyright-holders:Carl
// psx multitap emulation

#include "multitap.h"

const device_type PSX_MULTITAP = &device_creator<psx_multitap_device>;

psx_multitap_device::psx_multitap_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	device_t(mconfig, PSX_MULTITAP, "Playstation Multitap", tag, owner, clock, "psx_multitap", __FILE__),
	device_psx_controller_interface(mconfig, *this),
	m_activeport(0),
	m_singlemode(false),
	m_nextmode(false),
	m_tapmc(false),
	m_porta(*this, "a"),
	m_portb(*this, "b"),
	m_portc(*this, "c"),
	m_portd(*this, "d")
{
}

static MACHINE_CONFIG_FRAGMENT( psx_multitap )
	MCFG_PSX_CTRL_PORT_ADD("a", psx_controllers_nomulti, "digital_pad")
	MCFG_PSX_CTRL_PORT_ADD("b", psx_controllers_nomulti, NULL)
	MCFG_PSX_CTRL_PORT_ADD("c", psx_controllers_nomulti, NULL)
	MCFG_PSX_CTRL_PORT_ADD("d", psx_controllers_nomulti, NULL)
MACHINE_CONFIG_END

machine_config_constructor psx_multitap_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( psx_multitap );
}

void psx_multitap_device::device_start()
{
	m_porta->setup_ack_cb(psx_controller_port_device::void_cb(FUNC(psx_multitap_device::ack), this));
	m_portb->setup_ack_cb(psx_controller_port_device::void_cb(FUNC(psx_multitap_device::ack), this));
	m_portc->setup_ack_cb(psx_controller_port_device::void_cb(FUNC(psx_multitap_device::ack), this));
	m_portd->setup_ack_cb(psx_controller_port_device::void_cb(FUNC(psx_multitap_device::ack), this));
	m_nextmode = false;

	save_item(NAME(m_activeport));
	save_item(NAME(m_cack));
	save_item(NAME(m_singlemode));
	save_item(NAME(m_nextmode));
	save_item(NAME(m_tapmc));
	save_item(NAME(m_data));
}

void psx_multitap_device::interface_pre_reset()
{
	m_activeport = -1;
	m_singlemode = m_nextmode;
	m_tapmc = false;
	m_cack[0] = m_cack[1] = m_cack[2] = m_cack[3] = true;
	memset(m_data, 0xff, sizeof(m_data));
	m_porta->sel_w(false);
	m_portb->sel_w(false);
	m_portc->sel_w(false);
	m_portd->sel_w(false);
	m_porta->sel_w(true);
	m_portb->sel_w(true);
	m_portc->sel_w(true);
	m_portd->sel_w(true);
	device_psx_controller_interface::interface_pre_reset();
}

void psx_multitap_device::set_tx_line(bool tx, int port)
{
	psx_controller_port_device *dev;
	switch(port)
	{
		default:
		case 0:
			dev = m_porta;
			break;
		case 1:
			dev = m_portb;
			break;
		case 2:
			dev = m_portc;
			break;
		case 3:
			dev = m_portd;
			break;
	}
	dev->clock_w(0);
	dev->tx_w(tx);
	dev->clock_w(1);
}

bool psx_multitap_device::get_rx_line(int port)
{
	psx_controller_port_device *dev;
	switch(port)
	{
		default:
		case 0:
			dev = m_porta;
			break;
		case 1:
			dev = m_portb;
			break;
		case 2:
			dev = m_portc;
			break;
		case 3:
			dev = m_portd;
			break;
	}
	return dev->rx_r();
}

void psx_multitap_device::do_pad()
{
	bool tx = device_psx_controller_interface::m_owner->tx_r();

	// we don't know which controller until after the first byte
	if((m_singlemode || m_tapmc) && (m_count >= 1))
	{
		if((m_count == 2) && !m_bit && !m_tapmc)
			m_nextmode = !tx;

		set_tx_line(tx, m_activeport);
		m_rx = get_rx_line(m_activeport);
		m_bit = (m_bit + 1) % 8;
		if(!m_bit)
			m_count++;
		return;
	}

	if(!m_count)
	{
		// first send the select byte to all devices until we know whether it's accessing
		// a controller or memcard
		if(!m_bit)
		{
			m_porta->sel_w(false);
			m_portb->sel_w(false);
			m_portc->sel_w(false);
			m_portd->sel_w(false);
		}
		device_psx_controller_interface::do_pad();
		set_tx_line(tx, 0);
		set_tx_line(tx, 1);
		set_tx_line(tx, 2);
		set_tx_line(tx, 3);
		if(!m_bit)
		{
			m_count = 1;
			m_tapmc = m_memcard;
			m_memcard = false; // make sure we still receive clocks
			if(m_singlemode || m_tapmc)
			{
				m_activeport = (m_idata & 0xf) - 1;
				m_porta->sel_w((m_activeport == 0) ? false : true);
				m_portb->sel_w((m_activeport == 1) ? false : true);
				m_portc->sel_w((m_activeport == 2) ? false : true);
				m_portd->sel_w((m_activeport == 3) ? false : true);
			}
		}
		return;
	}
	else if(m_count <= 2)
		return device_psx_controller_interface::do_pad();
	else if(m_count < 11)
	{
		if((m_count == 3) && !m_bit)
			m_nextmode = !m_idata;

		if((m_count < 5) && m_cack[0] && m_cack[1] && m_cack[2] && m_cack[3])
			return; // no acks? hang up.

		// all of the ports are polled here, port a is passed though.  the data
		// from the other ports is stored and can be retrieved at a much higher clock rate
		// don't poll a port that is inactive or done
		if(!m_cack[0])
		{
			set_tx_line(tx, 0);
			m_rx = m_porta->rx_r();
		}
		else
		{
			m_rx = true;
			m_porta->sel_w(true);
		}

		if(!m_cack[1])
		{
			set_tx_line(tx, 1);
			m_data[0][m_count - 3] &= ~(!m_portb->rx_r() << m_bit);
		}
		else
			m_portb->sel_w(true);

		if(!m_cack[2])
		{
			set_tx_line(tx, 2);
			m_data[1][m_count - 3] &= ~(!m_portc->rx_r() << m_bit);
		}
		else
			m_portc->sel_w(true);

		if(!m_cack[3])
		{
			set_tx_line(tx, 3);
			m_data[2][m_count - 3] &= ~(!m_portd->rx_r() << m_bit);
		}
		else
			m_portd->sel_w(true);
	}
	else if(m_count < 19)
		// send stored port b data
		m_rx = ((m_data[0][m_count - 11] & (1 << m_bit)) ? 1 : 0);
	else if(m_count < 27)
		// send stored port c data
		m_rx = ((m_data[1][m_count - 19] & (1 << m_bit)) ? 1 : 0);
	else
		// send stored port d data
		m_rx = ((m_data[2][m_count - 27] & (1 << m_bit)) ? 1 : 0);

	if(m_bit == 7)
	{
		// ports won't ack if they are done
		m_cack[0] = m_cack[1] = m_cack[2] = m_cack[3] = true;
		if(m_count < 11)
			m_ack_timer->adjust(attotime::from_usec(12), 0); // give a bit of time for the ports to ack
		else if(m_count < 35)
			m_ack_timer->adjust(attotime::from_usec(10), 0);
	}

	m_bit = (m_bit + 1) % 8;
	if(!m_bit)
		m_count++;
}

bool psx_multitap_device::get_pad(int count, UINT8 *odata, UINT8 idata)
{
	if(!count)
		*odata = 0x80;
	else
		*odata = 0x5a;
	return true;
}

void psx_multitap_device::ack()
{
	if(m_activeport != -1)
	{
		switch(m_activeport)
		{
			case 0:
				m_ack = m_porta->ack_r();
				break;
			case 1:
				m_ack = m_portb->ack_r();
				break;
			case 2:
				m_ack = m_portc->ack_r();
				break;
			case 3:
				m_ack = m_portd->ack_r();
				break;
			default:
				return;
		}
		device_psx_controller_interface::m_owner->ack();
		return;
	}
	if(!m_porta->ack_r())
		m_cack[0] = false;
	if(!m_portb->ack_r())
		m_cack[1] = false;
	if(!m_portc->ack_r())
		m_cack[2] = false;
	if(!m_portd->ack_r())
		m_cack[3] = false;
}
