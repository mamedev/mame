// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega Mega Drive 4-Player Adaptor emulation

    Known as Team Player Multi-Player Adaptor in the US
    Known as Sega Tap (セガタップ) in Japan

    Everything here is guessed based on behaviour expected by
    software.  It isn't clear exactly how downstream devices are
    clocked, and how controllers are identified.

**********************************************************************/

#include "emu.h"
#include "teamplayer.h"

#include "controllers.h"

#include <algorithm>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

class sms_teamplayer_device : public device_t, public device_sms_control_interface
{
public:
	sms_teamplayer_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 in_r() override;
	virtual void out_w(u8 data, u8 mem_mask) override;

	DECLARE_INPUT_CHANGED_MEMBER(reselect);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	enum : u8
	{
		IDENT_3BUTTON = 0x0,
		IDENT_6BUTTON = 0x1,
		IDENT_MOUSE = 0x2,
		IDENT_UNSUPPORTED = 0xf
	};

	enum : u8
	{
		PHASE_START = 0,
		PHASE_UNUSED,
		PHASE_IDENT1,
		PHASE_IDENT2,
		PHASE_PORT_A,
		PHASE_PORT_B,
		PHASE_PORT_C,
		PHASE_PORT_D,
		PHASE_FINAL
	};

	template <unsigned N> void th_up_w(int state);

	TIMER_CALLBACK_MEMBER(probe_step);

	void selected();
	void deselected();
	void th_falling();
	void th_rising();
	void probe(unsigned step);
	void next_port(u8 tl);
	void next_nybble(u8 tl);

	static u8 identify(u8 const (&response)[8]);

	required_device_array<sms_control_port_device, 4> m_ports;
	required_ioport m_select;

	emu_timer *m_probe_timer;

	u8 m_data_down;
	u8 m_mask_down;
	u8 m_th_up[4];
	u8 m_out;

	u8 m_phase;
	u8 m_nybble;

	u8 m_probe[4][8];
	u8 m_ident[4];
};



INPUT_PORTS_START( sms_teamplayer )
	PORT_START("SELECT")
	PORT_CONFNAME(0x07, 0x04, "Switch") PORT_CHANGED_MEMBER(DEVICE_SELF, sms_teamplayer_device, reselect, 0)
	PORT_CONFSETTING(   0x00, "A")
	PORT_CONFSETTING(   0x01, "B")
	PORT_CONFSETTING(   0x02, "C")
	PORT_CONFSETTING(   0x03, "D")
	PORT_CONFSETTING(   0x04, "Multi")
INPUT_PORTS_END



sms_teamplayer_device::sms_teamplayer_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_TEAM_PLAYER, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_ports(*this, "%c", 'a'),
	m_select(*this, "SELECT"),
	m_probe_timer(nullptr),
	m_data_down(0x3f),
	m_mask_down(0x00),
	m_out(0x33),
	m_phase(PHASE_START),
	m_nybble(0)
{
}


INPUT_CHANGED_MEMBER(sms_teamplayer_device::reselect)
{
	if (!BIT(m_data_down, 6))
	{
		if (m_ports.size() <= oldval)
		{
			deselected();
		}
		else if (m_ports.size() <= newval)
		{
			selected();
			if (!BIT(m_data_down, 5))
				th_falling();
		}
	}

	if ((m_ports.size() > newval) || BIT(m_data_down, 6))
	{
		for (unsigned i = 0; m_ports.size() > i; ++i)
			m_ports[i]->out_w((newval == i) ? m_data_down : 0x7f, (newval == i) ? m_mask_down : 0x00);
	}
	th_w((m_ports.size() > newval) ? m_th_up[newval] : 1);
}


u8 sms_teamplayer_device::in_r()
{
	u8 const sel = m_select->read();
	if (m_ports.size() > sel)
	{
		u8 const result = m_ports[sel]->in_r();
		LOG(
				"%s: read passthrough %c = 0x%02X\n",
				machine().describe_context(),
				'A' + sel,
				result);
		return result;
	}
	else if (PHASE_UNUSED == m_phase)
	{
		bool ready = true;
		for (unsigned i = 0; ready && (m_ports.size() > i); ++i)
		{
			if ((IDENT_MOUSE == m_ident[i]) && BIT(m_ports[i]->in_r() ^ m_out, 4))
				ready = false;
		}
		u8 const result = m_out ^ (ready ? 0x00 : 0x10);

		LOG("%s: read mouse ready = 0x%02X\n", machine().describe_context(), result);
		return result;
	}
	else if ((PHASE_PORT_A <= m_phase) && (PHASE_PORT_D >= m_phase))
	{
		u8 const port = m_phase - PHASE_PORT_A;
		if (IDENT_MOUSE == m_ident[port])
		{
			u8 const result = m_ports[port]->in_r() ^ ((BIT(m_nybble, 0) ^ BIT(m_out, 4)) << 4);
			LOG(
					"%s: read mouse passthrough %c = 0x%02X\n",
					machine().describe_context(),
					'A' + port,
					result);
			return result;
		}
	}

	LOG("%s: read data = 0x%02X\n", machine().describe_context(), m_out);
	return m_out;
}


void sms_teamplayer_device::out_w(u8 data, u8 mem_mask)
{
	u8 const sel = m_select->read();
	if (m_ports.size() > sel)
	{
		m_ports[sel]->out_w(data, mem_mask);
	}
	else
	{
		u8 const changed = data ^ m_data_down;
		if (BIT(changed, 6))
		{
			if (BIT(data, 6))
			{
				deselected();
			}
			else
			{
				selected();
				if (!BIT(data, 5))
					th_falling();
			}
		}
		else if (!BIT(m_data_down, 6) && BIT(changed, 5))
		{
			if (BIT(data, 5))
				th_rising();
			else
				th_falling();
		}
	}
	m_data_down = data;
	m_mask_down = mem_mask;
}


void sms_teamplayer_device::device_add_mconfig(machine_config &config)
{
	SMS_CONTROL_PORT(config, m_ports[0], sms_control_port_devices, SMS_CTRL_OPTION_MD_PAD);
	m_ports[0]->th_handler().set(FUNC(sms_teamplayer_device::th_up_w<0>));

	SMS_CONTROL_PORT(config, m_ports[1], sms_control_port_devices, SMS_CTRL_OPTION_MD_PAD);
	m_ports[1]->th_handler().set(FUNC(sms_teamplayer_device::th_up_w<1>));

	SMS_CONTROL_PORT(config, m_ports[2], sms_control_port_devices, SMS_CTRL_OPTION_MD_PAD);
	m_ports[2]->th_handler().set(FUNC(sms_teamplayer_device::th_up_w<2>));

	SMS_CONTROL_PORT(config, m_ports[3], sms_control_port_devices, SMS_CTRL_OPTION_MD_PAD);
	m_ports[3]->th_handler().set(FUNC(sms_teamplayer_device::th_up_w<3>));
}


ioport_constructor sms_teamplayer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sms_teamplayer);
}


void sms_teamplayer_device::device_resolve_objects()
{
	m_data_down = 0x3f;
	m_mask_down = 0x00;
	std::fill(std::begin(m_th_up), std::end(m_th_up), 1);
}


void sms_teamplayer_device::device_start()
{
	m_probe_timer = timer_alloc(FUNC(sms_teamplayer_device::probe_step), this);

	m_out = 0x30;
	m_phase = PHASE_START;
	m_nybble = 0;

	for (auto &response : m_probe)
		std::fill(std::begin(response), std::end(response), 0x3f);
	std::fill(std::begin(m_ident), std::end(m_ident), IDENT_UNSUPPORTED);

	save_item(NAME(m_data_down));
	save_item(NAME(m_mask_down));
	save_item(NAME(m_th_up));
	save_item(NAME(m_out));
	save_item(NAME(m_phase));
	save_item(NAME(m_nybble));
	save_item(NAME(m_probe));
	save_item(NAME(m_ident));
}


template <unsigned N>
void sms_teamplayer_device::th_up_w(int state)
{
	if (bool(state) != bool(m_th_up[N]))
	{
		m_th_up[N] = state;
		if (m_select->read() == N)
			th_w(state);
	}
}


TIMER_CALLBACK_MEMBER(sms_teamplayer_device::probe_step)
{
	probe(param + 1);

	if (6 > param)
	{
		for (auto &port : m_ports)
			port->out_w(BIT(param, 0) ? 0x3f : 0x7f, BIT(param, 0) ? 0x40 : 0x00);
		m_probe_timer->adjust(attotime::from_usec(1), param + 1);
	}
	else
	{
		for (unsigned i = 0; m_ports.size() > i; ++i)
		{
			m_ident[i] = identify(m_probe[i]);
			if (IDENT_MOUSE != m_ident[i])
				m_ports[i]->out_w(0x7f, 0x00);
			else if ((PHASE_UNUSED == m_phase) && !BIT(m_data_down, 5))
				m_ports[i]->out_w(0x1f, 0x60);

			LOG(
					"port %c probe response %02X %02X %02X %02X %02X %02X %02X %02X ident %02X\n",
					'A' + i,
					m_probe[i][0],
					m_probe[i][1],
					m_probe[i][2],
					m_probe[i][3],
					m_probe[i][4],
					m_probe[i][5],
					m_probe[i][6],
					m_probe[i][7],
					m_ident[i]);
		}

		if ((PHASE_UNUSED == m_phase) && !BIT(m_data_down, 5))
			m_out = 0x20;
	}
}


void sms_teamplayer_device::selected()
{
	LOG("%s: selected\n", machine().describe_context());
	probe(0);
	for (auto &port : m_ports)
		port->out_w(0x3f, 0x40);
	m_probe_timer->adjust(attotime::from_usec(1), 0);
	m_out = 0x3f;
	m_phase = PHASE_START;
	m_nybble = 0;
}


void sms_teamplayer_device::deselected()
{
	LOG("%s: deselected\n", machine().describe_context());
	m_probe_timer->enable(false);
	for (auto &port : m_ports)
		port->out_w(0x7f, 0x00);
	m_out = 0x33;
	m_phase = PHASE_START;
	m_nybble = 0;
}


void sms_teamplayer_device::th_falling()
{
	LOG("%s: TR falling\n", machine().describe_context());
	switch (m_phase)
	{
	case PHASE_START:
		if (!m_probe_timer->enabled())
		{
			for (unsigned i = 0; m_ports.size() > i; ++i)
			{
				if (IDENT_MOUSE == m_ident[i])
					m_ports[i]->out_w(0x1f, 0x60);
			}
			m_out = 0x20;
		}
		++m_phase;
		break;

	case PHASE_UNUSED:
	case PHASE_IDENT1:
		{
			++m_phase;
			u8 const val = m_ident[(m_phase - PHASE_IDENT1) * 2];
			LOG(
					"%s: report port %c ident 0x%X\n",
					machine().describe_context(),
					'A' + ((m_phase - PHASE_IDENT1) * 2),
					val);
			m_out = 0x20 | val;
		}
		break;

	case PHASE_IDENT2:
		next_port(0x00);
		break;

	case PHASE_PORT_A:
	case PHASE_PORT_B:
	case PHASE_PORT_C:
	case PHASE_PORT_D:
		next_nybble(0x00);
		break;

	default:
		m_out = 0x20;
	}
}


void sms_teamplayer_device::th_rising()
{
	LOG("%s: TR rising\n", machine().describe_context());
	switch (m_phase)
	{
	case PHASE_UNUSED:
		for (unsigned i = 0; m_ports.size() > i; ++i)
		{
			if (IDENT_MOUSE == m_ident[i])
				m_ports[i]->out_w(0x3f, 0x40);
		}
		m_out = 0x30;
		break;

	case PHASE_IDENT1:
	case PHASE_IDENT2:
		{
			assert(!m_probe_timer->enabled());
			u8 const val = m_ident[((m_phase - PHASE_IDENT1) * 2) + 1];
			LOG(
					"%s: report port %c ident 0x%X\n",
					machine().describe_context(),
					'A' + ((m_phase - PHASE_IDENT1) * 2) + 1,
					val);
			m_out = 0x30 | val;
		}
		break;

	case PHASE_PORT_A:
	case PHASE_PORT_B:
	case PHASE_PORT_C:
	case PHASE_PORT_D:
		next_nybble(0x10);
		break;

	default:
		m_out = 0x30;
	}
}


void sms_teamplayer_device::probe(unsigned step)
{
	for (unsigned i = 0; m_ports.size() > i; ++i)
		m_probe[i][step] = m_ports[i]->in_r();
}


void sms_teamplayer_device::next_port(u8 tl)
{
	++m_phase;
	m_nybble = 0;
	while ((PHASE_PORT_D >= m_phase) && (IDENT_UNSUPPORTED == m_ident[m_phase - PHASE_PORT_A]))
		++m_phase;

	if (PHASE_PORT_D >= m_phase)
	{
		u8 const port = m_phase - PHASE_PORT_A;
		LOG(
				"%s: report port %c data\n",
				machine().describe_context(),
				'A' + port);
		switch (m_ident[port])
		{
		case IDENT_3BUTTON:
		case IDENT_6BUTTON:
			m_out = 0x20 | tl | (m_probe[port][0] & 0x0f);
			break;
		case IDENT_MOUSE: // FIXME: mouse support
			m_ports[port]->out_w(0x1f, 0x60);
			m_out = 0x20 | tl;
			break;
		}
	}
	else
	{
		LOG("%s: no more ports with supported controllers\n", machine().describe_context());
		m_out = 0x20 | tl;
	}
}


void sms_teamplayer_device::next_nybble(u8 tl)
{
	u8 const port = m_phase - PHASE_PORT_A;
	switch (m_ident[port])
	{
	case IDENT_3BUTTON:
		if (!m_nybble++)
			m_out = 0x20 | tl | (BIT(m_probe[port][1], 4, 2) << 2) | BIT(m_probe[port][0], 4, 2);
		else
			next_port(tl);
		break;
	case IDENT_6BUTTON:
		switch (m_nybble++)
		{
		case 0:
			m_out = 0x20 | tl | (BIT(m_probe[port][1], 4, 2) << 2) | BIT(m_probe[port][0], 4, 2);
			break;
		case 1:
			m_out = 0x20 | tl | (m_probe[port][6] & 0x0f);
			break;
		default:
			next_port(tl);
		}
		break;
	case IDENT_MOUSE:
		// FIXME: mouse support
		if (5 > m_nybble++)
		{
			u8 const odd = BIT(m_nybble, 0);
			m_ports[port]->out_w(odd ? 0x3f : 0x1f, odd ? 0x40 : 0x60);
			m_out = 0x20 | tl;
		}
		else
		{
			m_ports[port]->out_w(0x7f, 0x40);
			next_port(tl);
		}
		break;
	default:
		next_port(tl);
	}
}


u8 sms_teamplayer_device::identify(u8 const (&response)[8])
{
	if (!(response[0] & 0x0f) && (0x0b == (response[1] & 0x0f)) && !(response[2] & 0x0f) && (0x0b == (response[3] & 0x0f)))
		return IDENT_MOUSE;
	else if ((response[1] & 0x0c) || (response[3] & 0x0c) || (response[5] & 0x0c))
		return IDENT_UNSUPPORTED; // Mega Drive pads pull these bits down for detection
	else if (!(response[5] & 0x03))
		return IDENT_6BUTTON;
	else
		return IDENT_3BUTTON;
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_TEAM_PLAYER, device_sms_control_interface, sms_teamplayer_device, "sms_teamplayer", "Sega Mega Drive 4-Player Adaptor")
