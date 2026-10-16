// license:BSD-3-Clause
// copyright-holders:Tuth

#include "emu.h"
#include "jcm_wba.h"


DEFINE_DEVICE_TYPE(JCM_WBA, jcm_wba_device, "jcm_wba", "JCM WBA bill validator (ID-003)")

static INPUT_PORTS_START( jcm_wba )
	PORT_START("BILLS") // the euro notes the configuration menu lists for channels 2-8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BILL1 ) PORT_NAME("Bill 5 EUR (channel 2)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bill 10 EUR (channel 3)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bill 20 EUR (channel 4)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bill 50 EUR (channel 5)")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bill 100 EUR (channel 6)")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bill 200 EUR (channel 7)")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bill 500 EUR (channel 8)")
INPUT_PORTS_END

jcm_wba_device::jcm_wba_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, JCM_WBA, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, m_tx_cb(*this)
	, m_bills(*this, "BILLS")
{
}

ioport_constructor jcm_wba_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(jcm_wba);
}

void jcm_wba_device::device_start()
{
	m_timer = timer_alloc(FUNC(jcm_wba_device::step), this);
	save_item(NAME(m_rx)); save_item(NAME(m_rxn));
	save_item(NAME(m_status)); save_item(NAME(m_escrow));
	save_item(NAME(m_enable)); save_item(NAME(m_security)); save_item(NAME(m_comm));
	save_item(NAME(m_inhibit)); save_item(NAME(m_direction)); save_item(NAME(m_optional));
	save_item(NAME(m_prev_bills));
}

void jcm_wba_device::device_reset()
{
	set_data_frame(1, 8, PARITY_EVEN, STOP_BITS_1);
	set_rate(9600);
	receive_register_reset();
	transmit_register_reset();
	clear_fifo();
	m_tx_cb(1);
	m_rxn = 0;
	m_status = ST_POWER_UP;
	m_escrow = 0;
	m_enable[0] = m_enable[1] = 0;      // defaults (6-7)
	m_security[0] = m_security[1] = 0;
	m_comm = 0;
	m_inhibit = 1;
	m_direction = 0;
	m_optional[0] = m_optional[1] = 0;
	m_prev_bills = 0;
	m_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

u16 jcm_wba_device::crc(const u8 *d, int n)
{
	// P(X) = X^16 + X^12 + X^5 + 1, LSB first, initial value 0: FC 05 11 -> 5627h (appendix 1)
	u16 c = 0;
	for (int i = 0; i < n; i++)
	{
		c ^= d[i];
		for (int b = 0; b < 8; b++)
			c = (c & 1) ? (c >> 1) ^ 0x8408 : (c >> 1);
	}
	return c;
}

void jcm_wba_device::send(const u8 *body, int n)
{
	u8 f[256];
	f[0] = 0xfc;
	f[1] = n + 4;
	memcpy(&f[2], body, n);
	const u16 c = crc(f, n + 2);
	f[n + 2] = c & 0xff;
	f[n + 3] = c >> 8;
	for (int i = 0; i < n + 4; i++)
		transmit_byte(f[i]);
}

void jcm_wba_device::set_status(u8 st)
{
	if (st == m_status)
		return;
	m_status = st;
	// interrupt modes: ENQ on every change (mode 1) or on the states that need the controller (mode 2)
	const int mode = m_comm & 3;
	if (mode == 1 || (mode == 2 && (st == ST_ESCROW || st == ST_VEND_VALID || st == ST_INITIALIZE || st >= 0x40)))
	{
		const u8 e = ENQ;
		send(&e, 1);
	}
}

void jcm_wba_device::received_byte(u8 byte)
{
	if (m_rxn == 0 && byte != 0xfc)
		return;
	m_rx[m_rxn++] = byte;
	if (m_rxn >= 2 && (m_rx[1] < 5 || m_rxn >= m_rx[1]))
	{
		if (m_rx[1] >= 5 && crc(m_rx, m_rxn - 2) == (m_rx[m_rxn - 2] | (m_rx[m_rxn - 1] << 8)))
			command();
		else
			logerror("frame dropped (%d bytes, CRC or length error)\n", m_rxn);
		m_rxn = 0;
	}
}

void jcm_wba_device::command()
{
	const u8 cmd = m_rx[2];
	const u8 *d = &m_rx[3];
	const int dn = m_rx[1] - 5;
	u8 r[64];
	switch (cmd)
	{
	case 0x11: // STATUS REQUEST (6-1)
		r[0] = m_status;
		if (m_status == ST_ESCROW)
		{
			r[1] = m_escrow;
			send(r, 2);
		}
		else if (m_status == ST_REJECTING)
		{
			r[1] = 0x71;
			send(r, 2);
		}
		else
			send(r, 1);
		if (m_status == ST_POWER_UP)
			break;
		if (m_status == ST_INITIALIZE)
			set_status(m_inhibit & 1 ? ST_DISABLE : ST_ENABLE); // 7-1: INITIALIZE, then standby
		else if (m_status == ST_STACKED)
			set_status(m_inhibit & 1 ? ST_DISABLE : ST_ENABLE);
		break;

	case 0x40: // RESET: always accepted (6-3)
		r[0] = ACK;
		send(r, 1);
		m_status = ST_INITIALIZE;
		break;

	case 0x41: case 0x42: // STACK-1 / STACK-2, valid in ESCROW
		r[0] = (m_status == ST_ESCROW) ? ACK : INVALID;
		send(r, 1);
		if (m_status == ST_ESCROW)
		{
			set_status(ST_STACKING);
			m_timer->adjust(attotime::from_msec(500), 1, attotime::from_msec(10));
		}
		break;

	case 0x43: // RETURN, valid in ESCROW
		r[0] = (m_status == ST_ESCROW) ? ACK : INVALID;
		send(r, 1);
		if (m_status == ST_ESCROW)
		{
			set_status(ST_RETURNING);
			m_timer->adjust(attotime::from_msec(500), 2, attotime::from_msec(10));
		}
		break;

	case 0x44: case 0x45: // HOLD, WAIT
		r[0] = ACK;
		send(r, 1);
		break;

	case 0x50: // ACK from the controller: answer to VEND VALID (6-4)
		if (m_status == ST_VEND_VALID)
		{
			set_status(ST_STACKED);
			m_timer->adjust(attotime::from_msec(300), 3, attotime::from_msec(10));
		}
		break;

	case 0xc0: if (dn >= 2) { m_enable[0] = d[0]; m_enable[1] = d[1]; } goto echo;
	case 0xc1: if (dn >= 2) { m_security[0] = d[0]; m_security[1] = d[1]; } goto echo;
	case 0xc2: if (dn >= 1) m_comm = d[0]; goto echo;
	case 0xc3:
		if (dn >= 1)
		{
			m_inhibit = d[0];
			if (m_status == ST_ENABLE || m_status == ST_DISABLE)
				set_status(m_inhibit & 1 ? ST_DISABLE : ST_ENABLE);
		}
		goto echo;
	case 0xc4: if (dn >= 1) m_direction = d[0]; goto echo;
	case 0xc5: if (dn >= 2) { m_optional[0] = d[0]; m_optional[1] = d[1]; } goto echo;
	echo: // setting commands are echoed back (6-5)
		memcpy(r, &m_rx[2], dn + 1);
		send(r, dn + 1);
		break;

	case 0x80: r[0] = cmd; r[1] = m_enable[0]; r[2] = m_enable[1]; send(r, 3); break;
	case 0x81: r[0] = cmd; r[1] = m_security[0]; r[2] = m_security[1]; send(r, 3); break;
	case 0x82: r[0] = cmd; r[1] = m_comm; send(r, 2); break;
	case 0x83: r[0] = cmd; r[1] = m_inhibit; send(r, 2); break;
	case 0x84: r[0] = cmd; r[1] = m_direction; send(r, 2); break;
	case 0x85: r[0] = cmd; r[1] = m_optional[0]; r[2] = m_optional[1]; send(r, 3); break;

	case 0x88: // VERSION REQUEST: "machine (country)-model-stacker ID003-interface+version date crc" (6-7 (6))
	{
		static const char ver[] = "i(EUR)-10-SS ID003-03V280-30 14JAN03 3A5E";
		r[0] = cmd;
		memcpy(&r[1], ver, sizeof(ver) - 1);
		send(r, sizeof(ver));
		logerror("version request: placeholder version string, the real WBA string is not known\n");
		break;
	}

	case 0x89: // BOOT VERSION REQUEST: "Bxx " (6-7 (7))
		r[0] = cmd; r[1] = 'B'; r[2] = '0'; r[3] = '1'; r[4] = ' ';
		send(r, 5);
		break;

	case 0x8a: // CURRENCY ASSIGN REQUEST (6-7 (9)): 61h-69h, country, denomination integer and exponent
	{
		static const u8 eur[9][2] = { {0,0}, {5,0}, {10,0}, {20,0}, {50,0}, {1,2}, {2,2}, {5,2}, {0,0} };
		r[0] = cmd;
		for (int i = 0; i < 9; i++)
		{
			r[1 + i * 4] = 0x61 + i;
			r[2 + i * 4] = eur[i][0] ? 0x0b : 0x00; // TODO: euro country type from Table 1-1 (unknown)
			r[3 + i * 4] = eur[i][0];
			r[4 + i * 4] = eur[i][1];
		}
		send(r, 37);
		break;
	}

	default:
		logerror("unhandled command %02x (%d data bytes)\n", cmd, dn);
		r[0] = INVALID;
		send(r, 1);
		break;
	}
}

TIMER_CALLBACK_MEMBER(jcm_wba_device::step)
{
	switch (param)
	{
	case 0: // standby: a note pressed while ENABLE is inserted (channel = bit + 2, escrow 61h + channel - 1)
	{
		const u8 b = m_bills->read();
		const u8 edge = b & ~m_prev_bills;
		m_prev_bills = b;
		if (m_status == ST_ENABLE && edge)
		{
			int bit = 0;
			while (!BIT(edge, bit))
				bit++;
			if (!BIT(m_enable[0], bit + 1))
			{
				m_escrow = 0x61 + bit + 1;
				set_status(ST_ACCEPTING);
				m_timer->adjust(attotime::from_msec(700), 4, attotime::from_msec(10));
			}
		}
		break;
	}
	case 1: set_status(ST_VEND_VALID); m_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10)); break;
	case 2: set_status(m_inhibit & 1 ? ST_DISABLE : ST_ENABLE); m_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10)); break;
	case 3: set_status(ST_STACKED); m_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10)); break;
	case 4: set_status(ST_ESCROW); m_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10)); break;
	}
}
