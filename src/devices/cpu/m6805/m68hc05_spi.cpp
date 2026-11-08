// license:BSD-3-Clause
// copyright-holders:Adam Di Carlo
/***************************************************************************

    Motorola M68HC05 serial peripheral interface (SPI)

    Section references are to the MC68HC05C4/C8 data sheet (ADI991R2).

    Implemented: master mode with bit-level SCK and MOSI activity and MISO
    sampling (6.2, 6.3), the four master rates (6.4.1), CPOL and CPHA,
    SPIF/WCOL/MODF with their clearing sequences (6.4.2) and the SPIE
    interrupt.

    Not implemented: slave mode (logged when selected).  SCK and MOSI are not
    released when SPE clears or on a mode fault: they keep their last level.
    Port D reads aren't affected by SPE; 2.2.2 says only that PD2-PD5 read
    the pins with SPE clear.

***************************************************************************/

#include "emu.h"
#include "m68hc05_spi.h"


#define LOG_SPI     (1U << 1)

//#define VERBOSE (LOG_SPI)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGSPI(...)     LOGMASKED(LOG_SPI, __VA_ARGS__)


namespace {

// 6.4.1: SCK is E/2, E/4, E/16 or E/32
constexpr u8 SCK_DIVISOR[4] = { 2, 4, 16, 32 };

constexpr u32 BITS = 8;

} // anonymous namespace


DEFINE_DEVICE_TYPE(M68HC05_SPI, m68hc05_spi_device, "m68hc05_spi", "Motorola M68HC05 SPI")


m68hc05_spi_device::m68hc05_spi_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, M68HC05_SPI, tag, owner, clock),
	m_sck_cb(*this),
	m_mosi_cb(*this),
	m_miso_cb(*this, 1),
	m_irq_cb(*this),
	m_timer(nullptr),
	m_spcr(0x00),
	m_spsr(0x00), m_spsr_seen(0x00),
	m_rdr(0x00),
	m_out(0x00),
	m_in(0x00),
	m_half_clocks(0),
	m_busy(false),
	m_ss(true),
	m_irq_state(false),
	m_sck(-1)
{
}


void m68hc05_spi_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(m68hc05_spi_device::spcr_r), FUNC(m68hc05_spi_device::spcr_w));
	map(0x1, 0x1).r(FUNC(m68hc05_spi_device::spsr_r));
	map(0x2, 0x2).rw(FUNC(m68hc05_spi_device::spdr_r), FUNC(m68hc05_spi_device::spdr_w));
}


void m68hc05_spi_device::device_start()
{
	m_timer = timer_alloc(FUNC(m68hc05_spi_device::tick), this);

	save_item(NAME(m_spcr));
	save_item(NAME(m_spsr));
	save_item(NAME(m_spsr_seen));
	save_item(NAME(m_rdr));
	save_item(NAME(m_out));
	save_item(NAME(m_in));
	save_item(NAME(m_half_clocks));
	save_item(NAME(m_busy));
	save_item(NAME(m_ss));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_sck));
}

void m68hc05_spi_device::device_reset()
{
	// table 3-1: SPE, SPIE, MSTR, SPIF, WCOL and MODF are cleared; 6.4.1: CPOL, CPHA and
	// SPR1:0 are not affected.  SPDR is left alone too (inferred: the sheet doesn't say).
	m_spcr &= SPCR_CPOL | SPCR_CPHA | SPCR_SPR;
	m_spsr = 0x00;
	m_spsr_seen = 0x00;

	m_busy = false;
	m_timer->adjust(attotime::never);
	m_sck = -1;

	m_irq_state = false;
	m_irq_cb(0);
}


u8 m68hc05_spi_device::spcr_r()
{
	return m_spcr;
}

void m68hc05_spi_device::spcr_w(u8 data)
{
	if (machine().side_effects_disabled())
	{
		set_spcr(data);
		return;
	}

	data &= SPCR_MASK;

	// 6.4.2 B4: MODF clears by an SPSR read with MODF set followed by an SPCR write, and
	// SPE and MSTR can't be set while it is set except in that sequence
	if (m_spsr & SPSR_MODF)
	{
		if (m_spsr_seen & SPSR_MODF)
		{
			clear_flags(SPSR_MODF);
			m_spsr_seen &= ~SPSR_MODF;
		}
		else
		{
			data &= ~(SPCR_SPE | SPCR_MSTR);
		}
	}

	u8 const rise(data & ~m_spcr);
	m_spcr = data;
	if ((rise & SPCR_SPE) && !mstr())
		logerror("%s: slave mode selected, not supported\n", machine().describe_context());

	// inferred: clearing SPE abandons a byte in progress (6.4.1 B6 only says SPE clear
	// disconnects the SPI from the pins)
	if (!spe())
		abort_transfer();
	update_sck();
	check_modf();
	update_irq();
}

u8 m68hc05_spi_device::spsr_r()
{
	if (!machine().side_effects_disabled())
		m_spsr_seen = m_spsr;
	return m_spsr & SPSR_MASK;
}

u8 m68hc05_spi_device::spdr_r()
{
	// 6.4.2: SPIF and WCOL clear by an SPSR read followed by an SPDR access
	if (!machine().side_effects_disabled() && (m_spsr_seen & (SPSR_SPIF | SPSR_WCOL)))
	{
		clear_flags(m_spsr_seen & (SPSR_SPIF | SPSR_WCOL));
		m_spsr_seen &= ~(SPSR_SPIF | SPSR_WCOL);
	}
	return m_rdr;
}

void m68hc05_spi_device::spdr_w(u8 data)
{
	if (machine().side_effects_disabled())
		return;

	// 6.4.2 B6: a write while a transfer is under way sets WCOL and is lost
	if (m_busy)
	{
		LOGSPI("%s: write SPDR %02X during transfer, write collision\n", machine().describe_context(), data);
		set_flags(SPSR_WCOL);
		return;
	}

	// 6.4.2 B7: writes are inhibited while SPIF is set until SPSR has been read
	if ((m_spsr & SPSR_SPIF) && !(m_spsr_seen & SPSR_SPIF))
	{
		LOGSPI("%s: write SPDR %02X inhibited, SPIF not acknowledged\n", machine().describe_context(), data);
		return;
	}

	bool const start(spe() && mstr());

	// 6.4.2 B6: if the write starts the next transfer, only SPIF is cleared by the sequence
	u8 const clear(m_spsr_seen & (start ? SPSR_SPIF : (SPSR_SPIF | SPSR_WCOL)));
	if (clear)
	{
		clear_flags(clear);
		m_spsr_seen &= ~clear;
	}

	// 6.4.3: the write goes straight into the shift register.  Inferred: with SPE or MSTR
	// clear it starts nothing.
	m_out = data;
	if (start)
	{
		LOGSPI("sending %02X, E/%u\n", data, SCK_DIVISOR[m_spcr & SPCR_SPR]);
		m_busy = true;
		m_half_clocks = 0;
		m_in = 0;

		// Figure 6-2: with CPHA clear the MSB is on MOSI half an SCK cycle before the first
		// clock edge; with CPHA set it goes out on the first edge
		if (!cpha())
			m_mosi_cb(BIT(m_out, 7));
		m_timer->adjust(half_period());
	}
}


void m68hc05_spi_device::ss_w(int state)
{
	m_ss = bool(state);
	check_modf();
}


TIMER_CALLBACK_MEMBER(m68hc05_spi_device::tick)
{
	// Figure 6-2: ticks alternate between the leading and the trailing clock edge.
	// CPHA clear: data is captured on the leading edge and the next bit goes out on the
	// trailing one.  CPHA set: the next bit goes out on the leading edge and data is
	// captured on the trailing one.
	u32 const n(m_half_clocks++);
	u32 const bit(n >> 1);

	// MISO is sampled before the edge that captures it: the slave changes it after
	if (!BIT(n, 0))
	{
		if (!cpha())
			m_in = (m_in << 1) | (m_miso_cb() ? 1 : 0);
		write_sck(cpol() ? 0 : 1);
		if (cpha())
			m_mosi_cb(BIT(m_out, BITS - 1 - bit));
	}
	else
	{
		if (cpha())
			m_in = (m_in << 1) | (m_miso_cb() ? 1 : 0);
		write_sck(cpol() ? 1 : 0);
		if (!cpha() && (bit < (BITS - 1)))
			m_mosi_cb(BIT(m_out, BITS - 2 - bit));
	}

	// a callback may have lowered SS, and the mode fault ended the transfer
	if (!m_busy)
		return;

	if (n < (2 * BITS - 1))
	{
		m_timer->adjust(half_period());
	}
	else
	{
		// 6.4.2 B7: the shift register moves to the receive buffer as SPIF sets.  Setting
		// it at the 8th trailing edge is inferred: the sheet gives no edge.
		m_busy = false;
		m_rdr = m_in;
		LOGSPI("transfer complete, received %02X\n", m_rdr);
		set_flags(SPSR_SPIF);
	}
}


attotime m68hc05_spi_device::half_period() const
{
	return attotime::from_ticks(SCK_DIVISOR[m_spcr & SPCR_SPR] / 2, clock());
}


void m68hc05_spi_device::set_spcr(u8 data)
{
	m_spcr = data & SPCR_MASK;
	if (!spe())
		abort_transfer();
	update_irq();
}

void m68hc05_spi_device::set_spsr(u8 data)
{
	// a flag that changes here isn't covered by an earlier SPSR read
	u8 const old(m_spsr);
	m_spsr = data & SPSR_MASK;
	m_spsr_seen &= old & m_spsr;
	update_irq();
}


void m68hc05_spi_device::set_flags(u8 mask)
{
	// a flag set by the hardware after an SPSR read isn't covered by that read
	m_spsr |= mask;
	m_spsr_seen &= ~mask;
	update_irq();
}

void m68hc05_spi_device::clear_flags(u8 mask)
{
	m_spsr &= ~mask;
	update_irq();
}

void m68hc05_spi_device::update_irq()
{
	// WCOL doesn't request an interrupt (6.4.2)
	bool const state((m_spcr & SPCR_SPIE) && (m_spsr & (SPSR_SPIF | SPSR_MODF)));
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_irq_cb(state ? 1 : 0);
	}
}

void m68hc05_spi_device::write_sck(int state)
{
	m_sck = state;
	m_sck_cb(state);
}

void m68hc05_spi_device::update_sck()
{
	// an idle master holds SCK at CPOL
	int const idle(cpol() ? 1 : 0);
	if (spe() && mstr() && !m_busy && (m_sck != idle))
		write_sck(idle);
}

void m68hc05_spi_device::check_modf()
{
	// 6.2.3: a master seeing SS low becomes a slave: MODF sets, MSTR and SPE clear.
	// SS is treated as a level and abandons a byte in progress (both inferred).
	if (!m_ss && spe() && mstr())
	{
		LOGSPI("SS low in master mode, mode fault\n");
		m_spcr &= ~(SPCR_SPE | SPCR_MSTR);
		abort_transfer();
		set_flags(SPSR_MODF);
	}
}

void m68hc05_spi_device::abort_transfer()
{
	if (m_busy)
	{
		LOGSPI("transfer aborted\n");
		m_busy = false;
		m_timer->adjust(attotime::never);
	}
}
