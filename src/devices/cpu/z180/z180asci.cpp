// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    z180asci.cpp

*********************************************************************/

#include "emu.h"
#include "z180.h"

//#define VERBOSE 1

#include "logmacro.h"

/* 00 ASCI control register A ch 0 */
static constexpr u8 Z180_CNTLA_MPE         = 0x80;
static constexpr u8 Z180_CNTLA_RE          = 0x40;
static constexpr u8 Z180_CNTLA_TE          = 0x20;
static constexpr u8 Z180_CNTLA_RTS0        = 0x10;
static constexpr u8 Z180_CNTLA_MPBR_EFR    = 0x08;
static constexpr u8 Z180_CNTLA_MODE        = 0x07;
static constexpr u8 Z180_CNTLA_MODE_DATA   = 0x04;
static constexpr u8 Z180_CNTLA_MODE_PARITY = 0x02;
static constexpr u8 Z180_CNTLA_MODE_STOPB  = 0x01;

/* 01 ASCI control register A ch 1 */
static constexpr u8 Z180_CNTLA1_CKA1D      = 0x10;

/* 02/03 ASCI control register B ch 0 */
static constexpr u8 Z180_CNTLB_MPBT        = 0x80;
static constexpr u8 Z180_CNTLB_MP          = 0x40;
static constexpr u8 Z180_CNTLB_CTS_PS      = 0x20;
static constexpr u8 Z180_CNTLB_PEO         = 0x10;
static constexpr u8 Z180_CNTLB_DR          = 0x08;
static constexpr u8 Z180_CNTLB_SS          = 0x07;

/* 04 ASCI status register 0 (all bits read-only except RIE and TIE) */
static constexpr u8 Z180_STAT_RDRF         = 0x80;
static constexpr u8 Z180_STAT_OVRN         = 0x40;
static constexpr u8 Z180_STAT_PE           = 0x20;
static constexpr u8 Z180_STAT_FE           = 0x10;
static constexpr u8 Z180_STAT_RIE          = 0x08;
static constexpr u8 Z180_STAT_DCD0         = 0x04;
static constexpr u8 Z180_STAT_TDRE         = 0x02;
static constexpr u8 Z180_STAT_TIE          = 0x01;

/* 05 ASCI status register 1 (all bits read-only except RIE, CTS1E and TIE) */
static constexpr u8 Z180_STAT1_CTS1E       = 0x04;

/* 12/13 (Z8S180/Z8L180) ASCI extension control register 0 (break detect is read-only) */
static constexpr u8 Z180_ASEXT_DCD0        = 0x40;
static constexpr u8 Z180_ASEXT_CTS0        = 0x20;
static constexpr u8 Z180_ASEXT_X1_BIT_CLK0 = 0x10;
static constexpr u8 Z180_ASEXT_BRG0_MODE   = 0x08;
static constexpr u8 Z180_ASEXT_BRK_EN      = 0x04;
static constexpr u8 Z180_ASEXT_BRK_DET     = 0x02;
static constexpr u8 Z180_ASEXT_BRK_SEND    = 0x01;

static constexpr u8 Z180_ASEXT0_MASK       = 0x7f;
static constexpr u8 Z180_ASEXT1_MASK       = 0x1f;


//**************************************************************************
//  z180asci_channel_base
//**************************************************************************

z180asci_channel_base::z180asci_channel_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const int id, const bool ext)
	: device_t(mconfig, type, tag, owner, clock)
	, m_rcv_clock(nullptr)
	, m_tra_clock(nullptr)
	, m_bit_rate(attotime::never)
	, m_sample_rate(attotime::never)
	, m_txa_handler(*this)
	, m_rts_handler(*this)
	, m_cka_handler(*this)
	, m_cts(0)
	, m_dcd(0)
	, m_irq(0)
	, m_rts(0)
	, m_divisor(0)
	, m_id(id)
	, m_ext(ext)
{
}


void z180asci_channel_base::device_resolve_objects()
{
	// resolve callbacks
	m_txa_handler.resolve_safe();
	m_rts_handler.resolve_safe();
	m_cka_handler.resolve_safe();
}

void z180asci_channel_base::device_start()
{
	save_item(NAME(m_asci_cntla));
	save_item(NAME(m_asci_cntlb));
	save_item(NAME(m_asci_stat));
	save_item(NAME(m_asci_tdr));
	save_item(NAME(m_asci_rdr));
	if (m_ext)
	{
		save_item(NAME(m_asci_ext));
		save_item(NAME(m_asci_tc.w));
	}
	save_item(NAME(m_tsr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_data_fifo));
	save_item(NAME(m_error_fifo));
	save_item(NAME(m_fifo_wr));
	save_item(NAME(m_fifo_rd));

	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_irq));
	save_item(NAME(m_txa));
	save_item(NAME(m_rxa));
	save_item(NAME(m_rts));

	save_item(NAME(m_divisor));

	save_item(NAME(m_clock_state));

	save_item(NAME(m_tx_counter));
	save_item(NAME(m_tx_state));

	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_counter));
	save_item(NAME(m_rx_count_to));
	save_item(NAME(m_rx_total_bits));
	save_item(NAME(m_rx_enabled));

	save_item(NAME(m_bit_rate));
	save_item(NAME(m_sample_rate));


	m_rcv_clock = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z180asci_channel_base::rcv_clock), this));
	m_tra_clock = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z180asci_channel_base::tra_clock), this));

}

void z180asci_channel_base::device_reset()
{
	m_asci_ext = 0;
	m_asci_tc.w = 0;

	m_fifo_wr = 0;
	m_fifo_rd = 0;

	output_txa(1);
	output_rts(1);

	m_tsr = 0;
	m_rxa = 1;
	m_clock_state = 0;
	m_tx_state = STATE_WAIT;
	m_rx_state = STATE_START;
	m_rx_bits = 0;
	m_rx_enabled = true;

	m_tx_counter = 0;
	m_rx_counter = 0;
	m_rx_count_to = 1;
}

void z180asci_channel_base::device_clock_changed()
{
	uint32_t brg_divisor;
	 // Divide ratio
	m_divisor = (m_asci_ext & Z180_ASEXT_X1_BIT_CLK0) ? 1 : ((m_asci_cntlb & Z180_CNTLB_DR) ? 64 : 16);

	if ((m_asci_cntlb & Z180_CNTLB_SS) == Z180_CNTLB_SS)
	{
		// External clock
		brg_divisor = 0;
	}
	else
	{
		if (m_asci_ext & Z180_ASEXT_BRG0_MODE)
		{
			// Extended boud rate generator mode
			brg_divisor = m_asci_tc.w + 2;
		}
		else
		{
			// Regular bitrate generator mode
			brg_divisor = 1 << (m_asci_cntlb & Z180_CNTLB_SS);
			brg_divisor *= ((m_asci_cntlb & Z180_CNTLB_CTS_PS) ? 30 : 10); // Prescale
		}
	}

	if (brg_divisor)
	{
		LOG("Z180 ASCI%d set bitrate %d\n", m_id, uint32_t(clock() / brg_divisor / m_divisor));
		m_bit_rate = attotime::from_hz(clock() / brg_divisor / m_divisor);
		m_sample_rate = attotime::from_hz(clock() / brg_divisor);
	}
	else
	{
		LOG("Z180 ASCI%d set bitrate 0, using external\n", m_id);
		m_bit_rate = attotime::never;
		m_sample_rate = attotime::never;
	}

	m_tra_clock->adjust(attotime::never);
	m_rx_state = STATE_START;
	m_rx_count_to = 1;
	if(!m_sample_rate.is_never())
		m_rcv_clock->adjust(m_sample_rate, 0, m_sample_rate);
}

uint8_t z180asci_channel_base::cntla_r()
{
	LOG("Z180 CNTLA%d rd $%02x\n", m_id, m_asci_cntla);
	return m_asci_cntla;
}

uint8_t z180asci_channel_base::cntlb_r()
{
	uint8_t data = (m_asci_cntlb & 0x0d) | (m_cts << 5);
	LOG("Z180 CNTLB%d rd $%02x\n", m_id, data);
	return data;
}

uint8_t z180asci_channel_base::stat_r()
{
	LOG("Z180 STAT%d  rd $%02x\n", m_id, m_asci_stat);
	return m_asci_stat;
}

uint8_t z180asci_channel_base::tdr_r()
{
	LOG("Z180 TDR%d   rd $%02x\n", m_id, m_asci_tdr);
	return m_asci_tdr;
}

uint8_t z180asci_channel_base::rdr_r()
{
	LOG("Z180 RDR%d   rd $%02x\n", m_id, m_asci_rdr);
	if (!machine().side_effects_disabled())
	{
		if (m_fifo_rd != m_fifo_wr)
		{
			m_asci_rdr = m_data_fifo[m_fifo_rd];
			m_asci_stat &= ~(Z180_STAT_OVRN | Z180_STAT_PE | Z180_STAT_FE);
			m_asci_stat |= m_error_fifo[m_fifo_rd];
			if (m_asci_stat & (Z180_STAT_OVRN | Z180_STAT_PE | Z180_STAT_FE))
				m_irq = 1;
			m_fifo_rd = (m_fifo_rd + 1) & 3;
			if (m_fifo_rd == m_fifo_wr) // empty
				m_asci_stat &= ~Z180_STAT_RDRF;
		}
	}
	return m_asci_rdr;
}

uint8_t z180asci_channel_base::asext_r()
{
	LOG("Z180 ASEXT%d rd $%02x\n", m_id, m_asci_ext);
	return m_asci_ext;
}

uint8_t z180asci_channel_base::astcl_r()
{
	LOG("Z180 ASTC%dL rd $%02x ($%04x)\n", m_id, m_asci_tc.b.l, m_asci_tc.w);
	return m_asci_tc.b.l;
}

uint8_t z180asci_channel_base::astch_r()
{
	LOG("Z180 ASTC%dH rd $%02x ($%04x)\n", m_id, m_asci_tc.b.h, m_asci_tc.w);
	return m_asci_tc.b.h;
}

void z180asci_channel_base::update_total_bits()
{
	m_rx_total_bits = (m_asci_cntla & Z180_CNTLA_MODE_DATA) ? 8 : 7;
	m_rx_total_bits += (m_asci_cntla & Z180_CNTLA_MODE_STOPB) ? 2 : 1;
	m_rx_total_bits += (m_asci_cntlb & Z180_CNTLB_MP) ? 1 : ((m_asci_cntla & Z180_CNTLA_MODE_PARITY) ? 1 : 0);
}

void z180asci_channel_base::cntla_w(uint8_t data)
{
	LOG("Z180 CNTLA%d wr $%02x\n", m_id, data);
	m_asci_cntla = data & ~(Z180_CNTLA_MPBR_EFR | Z180_CNTLA_RTS0);
	output_rts(BIT(data,4)); // Z180_CNTLA_RTS0
	if (data & Z180_CNTLA_MPBR_EFR) // Error Flag Reset
	{
		m_asci_stat &= ~(Z180_STAT_OVRN | Z180_STAT_PE | Z180_STAT_FE);
		m_asci_ext &= ~(Z180_ASEXT_BRK_DET);
	}
	update_total_bits();
}

void z180asci_channel_base::cntlb_w(uint8_t data)
{
	LOG("Z180 CNTLB%d wr $%02x\n", m_id, data);
	m_asci_cntlb = data;
	device_clock_changed();
	update_total_bits();
}

void z180asci_channel_base::tdr_w(uint8_t data)
{
	LOG("Z180 TDR%d   wr $%02x\n", m_id, data);
	m_asci_tdr = data;
	m_asci_stat &= ~Z180_STAT_TDRE;

	if (!m_bit_rate.is_never())
		m_tra_clock->adjust(m_bit_rate, 0, m_bit_rate);
}

void z180asci_channel_base::rdr_w(uint8_t data)
{
	LOG("Z180 RDR%d   wr $%02x\n", m_id, data);
	if (!(m_asci_stat & Z180_STAT_RDRF))
		set_fifo_data(data, 0);
}

void z180asci_channel_base::asext_w(uint8_t data)
{
	if (m_asci_ext & Z180_ASEXT_BRK_EN)
		m_tx_state = (m_asci_ext & Z180_ASEXT_BRK_SEND) ? STATE_BREAK : STATE_WAIT;
	device_clock_changed();
}

void z180asci_channel_base::astcl_w(uint8_t data)
{
	LOG("Z180 ASTC%dL wr $%02x\n", m_id, data);
	m_asci_tc.b.l = data;
	device_clock_changed();
}

void z180asci_channel_base::astch_w(uint8_t data)
{
	LOG("Z180 ASTC%dH wr $%02x\n", m_id, data);
	m_asci_tc.b.h = data;
	device_clock_changed();
}

DECLARE_WRITE_LINE_MEMBER( z180asci_channel_base::cts_wr )
{
	if (m_id)
	{
		// For channel 1, CTS can be disabled
		if ((m_asci_stat & Z180_STAT1_CTS1E) == 0) return;
	}
	else
	{
		// For channel 0, high resets TDRE
		if (m_ext && state && (m_asci_ext & Z180_ASEXT_CTS0) == 0)
			m_asci_stat |= Z180_STAT_TDRE;
	}
	m_cts = state;
}

DECLARE_WRITE_LINE_MEMBER( z180asci_channel_base::dcd_wr )
{
	if (m_id)
		return;

	// In extended mode, DCD autoenables RX if configured
	if (m_ext && (m_asci_ext & Z180_ASEXT_DCD0) == 0)
	{
		m_rx_enabled = state ? false : true;
		if (state)
			m_asci_ext &= ~(Z180_ASEXT_BRK_DET);
	}

	m_dcd = state;
	if (m_dcd)
		m_irq = 1;
}

DECLARE_WRITE_LINE_MEMBER( z180asci_channel_base::rxa_wr )
{
	m_rxa = state;
}

DECLARE_WRITE_LINE_MEMBER( z180asci_channel_base::cka_wr )
{
	// For channel 1, CKA can be disabled
	if (m_id && (m_asci_cntla & Z180_CNTLA1_CKA1D)) return;

	if(state != m_clock_state)
	{
		m_clock_state = state;
		if(!state)
		{
			m_tx_counter++;
			if (m_tx_counter != m_divisor) return;
			m_tx_counter = 0;
			if (m_asci_cntla & Z180_CNTLA_TE)
				transmit_edge();
		}
		else
		{
			m_rx_counter++;
			if (m_rx_counter != m_rx_count_to) return;
			m_rx_counter = 0;
			if (m_rx_enabled && (m_asci_cntla & Z180_CNTLA_RE))
				receive_edge();
		}
	}
}

void z180asci_channel_base::prepare_tsr()
{
	int bits = (m_asci_cntla & Z180_CNTLA_MODE_DATA) ? 8 : 7;

	m_tsr = (m_asci_cntla & Z180_CNTLA_MODE_STOPB) ? 3 : 1; // stop bit(s)

	if ((m_asci_cntlb & Z180_CNTLB_MP) || (m_asci_cntla & Z180_CNTLA_MODE_PARITY))
	{
		m_tsr <<= 1;
		if (m_asci_cntlb & Z180_CNTLB_MP)
		{
			m_tsr |= (m_asci_cntlb & Z180_CNTLB_MPBT) ? 1 : 0;
		}
		else
		{
			uint8_t parity = 0;
			for (int i = 0; i < bits; i++)
				parity ^= BIT(m_asci_tdr, i);
			if (m_asci_cntlb & Z180_CNTLB_PEO) parity ^= 1; // odd parity
			m_tsr |= parity;
		}
	}
	m_tsr <<= bits;
	m_tsr |= m_asci_tdr;
	m_tsr <<= 1; // start bit
}

void z180asci_channel_base::transmit_edge()
{
	if (m_asci_cntla & Z180_CNTLA_TE)
	{
		switch (m_tx_state)
		{
		case STATE_DATA:
			output_txa(BIT(m_tsr, 0));
			m_tsr >>= 1;
			if (m_tsr == 0)
			{
				m_asci_stat |= Z180_STAT_TDRE;
				if (m_asci_stat & Z180_STAT_TIE)
				{
					m_irq = 1;
				}
				m_tx_state = STATE_WAIT;
				m_tra_clock->adjust(attotime::never);
			}
			break;
		case STATE_WAIT:
			if ((m_asci_stat & Z180_STAT_TDRE) == 0)
			{
				prepare_tsr();
				m_tx_state = STATE_DATA;
			}
			break;
		case STATE_BREAK:
			output_txa(0);
			break;
		}
	}
}

void z180asci_channel_base::update_received()
{
	uint8_t rx_error = 0;
	int stop_bits = (m_asci_cntla & Z180_CNTLA_MODE_STOPB) ? 2 : 1;
	if (m_rsr == 0) // Break detect
	{
		m_asci_ext |= Z180_ASEXT_BRK_DET;
	}
	uint8_t stop_val = (m_asci_cntla & Z180_CNTLA_MODE_STOPB) ? 3: 1;
	if ((m_rsr & stop_val) != stop_val)
		rx_error |= Z180_STAT_FE;

	if (m_asci_cntlb & Z180_CNTLB_MP)
	{
		m_asci_cntla |= ((m_rsr >> stop_bits) & 1) ? Z180_CNTLA_MPBR_EFR : 0;
	}
	else if (m_asci_cntla & Z180_CNTLA_MODE_PARITY)
	{
		uint8_t parity = 0;
		for (int i = 0; i < ((m_asci_cntla & Z180_CNTLA_MODE_DATA) ? 8 : 7); i++)
			parity ^= BIT(m_rsr, i);
		if (m_asci_cntlb & Z180_CNTLB_PEO) parity ^= 1; // odd parity
		if (((m_rsr >> stop_bits) & 1) != parity)
			rx_error |= Z180_STAT_PE;
	}
	// Skip only if MPE mode active and MPB is 0
	if (!((m_asci_cntla & Z180_CNTLA_MPE) && ((m_asci_cntla & Z180_CNTLA_MPBR_EFR) == 0)))
	{
		set_fifo_data(m_rsr, rx_error);
	}
}

void z180asci_channel_base::receive_edge()
{
	if (!m_rx_enabled) return;

	if (m_asci_cntla & Z180_CNTLA_RE)
	{
		switch (m_rx_state)
		{
		case STATE_START:
			if(m_rxa == 0)
			{
				m_rx_state = STATE_DATA;
				m_rx_bits = 0;
				m_rsr = 0;
				if(!(m_bit_rate.is_never()))
					m_rcv_clock->adjust((m_divisor == 1) ? m_bit_rate : (m_bit_rate * 3) / 2, 0, m_bit_rate);
				else
					m_rx_count_to = (m_divisor == 1) ? m_divisor : (m_divisor * 3) / 2;
			}
			break;
		case STATE_DATA:
			m_rsr |= m_rxa << m_rx_bits;
			m_rx_bits++;
			m_rx_count_to = m_divisor;
			if (m_rx_bits == m_rx_total_bits)
			{
				update_received();
				m_rx_state = STATE_START;
				if(!m_sample_rate.is_never())
					m_rcv_clock->adjust(m_sample_rate, 0, m_sample_rate);
				else
					m_rx_count_to = 1;
			}
			break;
		}
	}
}

void z180asci_channel_base::set_fifo_data(uint8_t data, uint8_t error)
{
	m_data_fifo[m_fifo_wr] = data;
	m_error_fifo[m_fifo_wr] = error;
	if (((m_fifo_wr + 1) & 3) == m_fifo_rd) // overrun
	{
		m_error_fifo[m_fifo_wr] |= Z180_STAT_OVRN;
	}
	else
	{
		m_error_fifo[m_fifo_wr] &= Z180_STAT_OVRN;
		m_fifo_wr = (m_fifo_wr + 1) & 3;
	}
	m_asci_stat |= Z180_STAT_RDRF;
	if (m_asci_stat & Z180_STAT_RIE)
	{
		m_irq = 1;
	}
}

void z180asci_channel_base::output_txa(int txa)
{
	if (m_txa != txa)
	{
		m_txa = txa;
		m_txa_handler(m_txa);
	}
}

void z180asci_channel_base::output_rts(int rts)
{
	if (m_rts != rts)
	{
		m_rts = rts;
		m_rts_handler(m_rts);
	}
}

//**************************************************************************
//  z180asci_channel_0
//**************************************************************************

z180asci_channel_0::z180asci_channel_0(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool ext)
	: z180asci_channel_base(mconfig, type, tag, owner, clock, 0, ext)
{
}

z180asci_channel_0::z180asci_channel_0(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel_0(mconfig, Z180ASCI_CHANNEL_0, tag, owner, clock, false)
{
}

void z180asci_channel_0::device_reset()
{
	z180asci_channel_base::device_reset();
	cntla_w((m_asci_cntla & Z180_CNTLA_MPBR_EFR) | Z180_CNTLA_RTS0);
	cntlb_w((m_asci_cntlb & (Z180_CNTLB_MPBT | Z180_CNTLB_CTS_PS)) | 0x07);
	m_asci_stat = (m_asci_stat & Z180_STAT_DCD0) | Z180_STAT_TDRE;
}

void z180asci_channel_0::state_add(device_state_interface &parent)
{
	parent.state_add(Z180_CNTLA0, "CNTLA0",  m_asci_cntla);
	parent.state_add(Z180_CNTLB0, "CNTLB0",  m_asci_cntlb);
	parent.state_add(Z180_STAT0,  "STAT0",   m_asci_stat);
	parent.state_add(Z180_TDR0,   "TDR0",    m_asci_tdr);
	parent.state_add(Z180_RDR0,   "RDR0",    m_asci_rdr);
	if (m_ext)
	{
		parent.state_add(Z180_ASEXT0, "ASEXT0",  m_asci_ext).mask(Z180_ASEXT0_MASK);
		parent.state_add(Z180_ASTC0,  "ASTC0",   m_asci_tc.w);
	}
}

void z180asci_channel_0::stat_w(uint8_t data)
{
	LOG("Z180 STAT0  wr $%02x ($%02x)\n", data,  data & (Z180_STAT_RIE | Z180_STAT_TIE));
	m_asci_stat = (m_asci_stat & ~(Z180_STAT_RIE | Z180_STAT_TIE)) | (data & (Z180_STAT_RIE | Z180_STAT_TIE));
}

void z180asci_channel_0::asext_w(uint8_t data)
{
	LOG("Z180 ASEXT0 wr $%02x ($%02x)\n", data,  data & Z180_ASEXT0_MASK & ~Z180_ASEXT_BRK_DET);
	m_asci_ext = (m_asci_ext & Z180_ASEXT_BRK_DET) | (data & Z180_ASEXT0_MASK & ~Z180_ASEXT_BRK_DET);
	z180asci_channel_base::asext_w(data);
}

//**************************************************************************
//  z180asci_channel_1
//**************************************************************************

z180asci_channel_1::z180asci_channel_1(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool ext)
	: z180asci_channel_base(mconfig, type, tag, owner, clock, 1, ext)
{
}

z180asci_channel_1::z180asci_channel_1(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel_1(mconfig, Z180ASCI_CHANNEL_1, tag, owner, clock, false)
{
}
void z180asci_channel_1::device_reset()
{
	z180asci_channel_base::device_reset();
	cntla_w((m_asci_cntla & Z180_CNTLA_MPBR_EFR) | Z180_CNTLA1_CKA1D);
	cntlb_w((m_asci_cntlb & Z180_CNTLB_MPBT) | 0x07);
	m_asci_stat = Z180_STAT_TDRE;
}

void z180asci_channel_1::state_add(device_state_interface &parent)
{
	parent.state_add(Z180_CNTLA1, "CNTLA1",  m_asci_cntla);
	parent.state_add(Z180_CNTLB1, "CNTLB1",  m_asci_cntlb);
	parent.state_add(Z180_STAT1,  "STAT1",   m_asci_stat);
	parent.state_add(Z180_TDR1,   "TDR1",    m_asci_tdr);
	parent.state_add(Z180_RDR1,   "RDR1",    m_asci_rdr);
	if (m_ext)
	{
		parent.state_add(Z180_ASEXT1, "ASEXT1",  m_asci_ext).mask(Z180_ASEXT1_MASK);
		parent.state_add(Z180_ASTC1,  "ASTC1",   m_asci_tc.w);
	}
}

void z180asci_channel_1::stat_w(uint8_t data)
{
	LOG("Z180 STAT1  wr $%02x ($%02x)\n", data,  data & (Z180_STAT_RIE | Z180_STAT1_CTS1E | Z180_STAT_TIE));
	m_asci_stat = (m_asci_stat & ~(Z180_STAT_RIE | Z180_STAT1_CTS1E | Z180_STAT_TIE)) | (data & (Z180_STAT_RIE | Z180_STAT1_CTS1E | Z180_STAT_TIE));
}

void z180asci_channel_1::asext_w(uint8_t data)
{
	LOG("Z180 ASEXT1 wr $%02x ($%02x)\n", data,  data & Z180_ASEXT1_MASK & ~Z180_ASEXT_BRK_DET);
	m_asci_ext = (m_asci_ext & Z180_ASEXT_BRK_DET) | (data & Z180_ASEXT1_MASK & ~Z180_ASEXT_BRK_DET);
	z180asci_channel_base::asext_w(data);
}

//**************************************************************************
//  z180asci_ext_channel_0
//**************************************************************************

z180asci_ext_channel_0::z180asci_ext_channel_0(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel_0(mconfig, Z180ASCI_EXT_CHANNEL_0, tag, owner, clock, true)
{
}

//**************************************************************************
//  z180asci_channel_1
//**************************************************************************

z180asci_ext_channel_1::z180asci_ext_channel_1(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel_1(mconfig, Z180ASCI_EXT_CHANNEL_1, tag, owner, clock, true)
{
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(Z180ASCI_CHANNEL_0,   z180asci_channel_0,  "z180asci_channel_0",    "Z180 ASCI Channel 0")
DEFINE_DEVICE_TYPE(Z180ASCI_CHANNEL_1,   z180asci_channel_1,  "z180asci_channel_1",    "Z180 ASCI Channel 1")

DEFINE_DEVICE_TYPE(Z180ASCI_EXT_CHANNEL_0,   z180asci_ext_channel_0,  "z180asci_ext_channel_0",    "Z180 ASCI Extended Channel 0")
DEFINE_DEVICE_TYPE(Z180ASCI_EXT_CHANNEL_1,   z180asci_ext_channel_1,  "z180asci_ext_channel_1",    "Z180 ASCI Extended Channel 1")
