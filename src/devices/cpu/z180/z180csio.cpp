// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Sandro Ronco, Miodrag Milanovic
/*********************************************************************

    z180csio.cpp

*********************************************************************/

/*********************************************************************

    TODO:
    - Handle entering IOSTOP mode.
    - Handle mid-transfer clock frequency changes.

*********************************************************************/

#include "emu.h"
#include "z180.h"

//#define VERBOSE 1
#include "logmacro.h"


// 0x0a CSI/O control/status register (EF is read-only)
static constexpr u8 Z180_CNTR_EF   = 0x80;
static constexpr u8 Z180_CNTR_EIE  = 0x40;
static constexpr u8 Z180_CNTR_RE   = 0x20;
static constexpr u8 Z180_CNTR_TE   = 0x10;
static constexpr u8 Z180_CNTR_SS   = 0x07;

static constexpr u8 Z180_CNTR_MASK = 0xf7;


//**************************************************************************
//  z180csio_device
//**************************************************************************

z180csio_device::z180csio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, Z180CSIO, tag, owner, clock)
	, m_cks_cb(*this)
	, m_txs_cb(*this)
	, m_internal_clock(nullptr)
	, m_cntr(0)
	, m_trdr(0)
	, m_shift_cnt(0)
	, m_irq(0)
	, m_cks_in(1)
	, m_rxs_in(1)
	, m_cks_out(1)
	, m_txs_out(1)
{
}


void z180csio_device::device_resolve_objects()
{
	// set default input line state
	m_cks_in = 1;
	m_rxs_in = 1;
}


void z180csio_device::device_start()
{
	// TRDR is not affected by reset - set it here to make behaviour deterministic
	m_trdr = 0;

	save_item(NAME(m_cntr));
	save_item(NAME(m_trdr));
	save_item(NAME(m_shift_cnt));
	save_item(NAME(m_irq));
	save_item(NAME(m_cks_in));
	save_item(NAME(m_rxs_in));
	save_item(NAME(m_cks_out));
	save_item(NAME(m_txs_out));

	m_internal_clock = timer_alloc(timer_expired_delegate(FUNC(z180csio_device::internal_clock), this));
}


void z180csio_device::device_reset()
{
	m_cntr = 0x07;
	m_shift_cnt = 0;
	m_irq = 0;
	m_cks_out = 1;
	m_txs_out = 1; // TODO: is this affected by reset?

	m_internal_clock->adjust(attotime::never);

	m_cks_cb(m_cks_out);
	m_txs_cb(m_txs_out);
}


void z180csio_device::state_add(device_state_interface &parent)
{
	parent.state_add(Z180_CNTR, "CNTR", m_cntr).mask(Z180_CNTR_MASK);
	parent.state_add(Z180_TRDR, "TRDR", m_trdr);
}


u8 z180csio_device::cntr_r()
{
	LOG("Z180 CNTR rd $%02x\n", m_cntr);
	return m_cntr & Z180_CNTR_MASK;
}


u8 z180csio_device::trdr_r()
{
	// TODO: from manual page 47: "Program access of TRDR only occurs if EF = 1."
	// Should access be suppressed if EF is clear?

	LOG("Z180 TRDR rd $%02x\n", m_trdr);
	if (!machine().side_effects_disabled())
		m_cntr &= ~Z180_CNTR_EF;

	return m_trdr;
}


void z180csio_device::cntr_w(u8 data)
{
	// TODO:
	// From manual page 47: "TE and RE are never both set to 1 at the same time."
	// If one attempts to write 1 to both at the same time, which takes precedence?

	LOG("Z180 CNTR wr $%02x\n", data);

	if (data & (Z180_CNTR_RE | Z180_CNTR_TE))
	{
		// if receive or transmit will be enabled, start clock if necessary
		if (!(m_cntr & (Z180_CNTR_RE | Z180_CNTR_TE)))
		{
			if ((data & Z180_CNTR_SS) != 7)
				m_internal_clock->adjust(attotime::from_hz(clock()));
			else
				m_internal_clock->adjust(attotime::never);
		}
	}
	else
	{
		m_shift_cnt = 0;
		if (!m_cks_out)
		{
			// this probably takes at least one clock to take actually happen
			m_cks_out = 1;
			if ((data & Z180_CNTR_SS) != 7)
				m_cks_cb(1);
		}
		m_internal_clock->adjust(attotime::never);
	}

	// TODO: if switching internal/external clock, update CKS output and trigger a clock edge if necessary

	m_cntr = (m_cntr & Z180_CNTR_EF) | (data & ~Z180_CNTR_EF & Z180_CNTR_MASK); // EF is read-only
}


void z180csio_device::trdr_w(u8 data)
{
	// TODO: from manual page 47: "Program access of TRDR only occurs if EF = 1."
	// Should access be suppressed if EF is clear?

	LOG("Z180 TRDR wr $%02x\n", data);
	m_cntr &= ~Z180_CNTR_EF;
	m_trdr = data;
}


TIMER_CALLBACK_MEMBER(z180csio_device::internal_clock)
{
	if ((m_cntr & Z180_CNTR_SS) != 7)
	{
		m_cks_out ^= 1;
		m_cks_cb(m_cks_out);
		clock_edge(m_cks_out);

		if (m_cntr & (Z180_CNTR_RE | Z180_CNTR_TE))
		{
			int div;
			switch (m_cntr & Z180_CNTR_SS)
			{
			default: // just to pacify compilers
			case 0: div = 20;       break;
			case 1: div = 40;       break;
			case 2: div = 80;       break;
			case 3: div = 160;      break;
			case 4: div = 320;      break;
			case 5: div = 640;      break;
			case 6: div = 1280;     break;
			}
			m_internal_clock->adjust(attotime::from_hz(clock() / div));
		}
	}
}


void z180csio_device::cks_wr(int state)
{
	state = state ? 1 : 0;
	if (m_cks_in != state)
	{
		m_cks_in = state;
		if ((m_cntr & Z180_CNTR_SS) == 0x07)
			clock_edge(m_cks_in);
	}
}


void z180csio_device::rxs_wr(int state)
{
	m_rxs_in = state ? 1 : 0;
}


void z180csio_device::clock_edge(u8 cks)
{
	if (!cks)
	{
		// TXS updated on falling edge
		if (m_cntr & Z180_CNTR_TE)
		{
			u8 const txs = BIT(m_trdr, m_shift_cnt);
			if (m_txs_out != txs)
			{
				m_txs_out = txs;
				m_txs_cb(txs);
			}
		}
	}
	else
	{
		// Sample RXS on rising edge (ignore minimum setup/sampling time)
		if (m_cntr & Z180_CNTR_RE)
			m_trdr = (m_trdr & ~(u8(1) << m_shift_cnt)) | (m_rxs_in << m_shift_cnt);

		// EF/RE/TE updated on rising edge
		if (m_cntr & (Z180_CNTR_RE | Z180_CNTR_TE))
		{
			m_shift_cnt = (m_shift_cnt + 1) & 7;
			if (!m_shift_cnt)
				m_cntr = Z180_CNTR_EF | (m_cntr & ~(Z180_CNTR_RE | Z180_CNTR_TE));
		}
	}
}


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(Z180CSIO, z180csio_device, "z180csio", "Z180 CSIO")
