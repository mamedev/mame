// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series UART

***************************************************************************/

#include "emu.h"
#include "f2mc16_uart.h"

#define LOG_READ (1U << 1)
#define LOG_WRITE (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_READ | LOG_WRITE)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGREAD(...)    LOGMASKED(LOG_READ, __VA_ARGS__)
#define LOGWRITE(...)   LOGMASKED(LOG_WRITE, __VA_ARGS__)

namespace {

struct SMR { enum : uint8_t
{
	SOE = 1 << 0,
	SCKE = 1 << 1,
	CS = 7 << 3,
	CS_RESERVED = 5 << 3,
	CS_INTERNAL_TIMER = 6 << 3,
	CS_EXTERNAL_CLOCK = 7 << 3,
	MD = 3 << 6,
	MD_ASYNC_NORMAL = 0 << 6,
	MD_ASYNC_MULTIPROCESSOR = 1 << 6,
	MD_SYNC = 2 << 6,
	MD_PROHIBITED = 3 << 6
}; };

struct SCR { enum : uint8_t
{
	PEN = 1 << 7,
	P = 1 << 6,
	SBL = 1 << 5,
	CL = 1 << 4,
	AD = 1 << 3,
	REC = 1 << 2,
	RXE = 1 << 1,
	TXE = 1 << 0
}; };

struct SSR { enum : uint8_t
{
	PE = 1 << 7,
	ORE = 1 << 6,
	FRE = 1 << 5,
	RDRF = 1 << 4,
	TDRE = 1 << 3,
	RIE = 1 << 1,
	TIE = 1 << 0
}; };

struct CDCR { enum : uint8_t
{
	DIV = 15 << 0
}; };

} // anonymous namespace

DEFINE_DEVICE_TYPE(F2MC16_UART, f2mc16_uart_device, "f2mc16_uart", "F2MC16 UART")

f2mc16_uart_device::f2mc16_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t rx_vector, uint8_t tx_vector) :
	f2mc16_uart_device(mconfig, tag, owner, clock)
{
	m_intc.set_tag(intc);
	m_rx_vector = rx_vector;
	m_tx_vector = tx_vector;
}

f2mc16_uart_device::f2mc16_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, F2MC16_UART, tag, owner, clock),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_sck_cb(*this),
	m_sck_hz_cb(*this),
	m_sot_cb(*this),
	m_internal_timer_changed(attotime::zero),
	m_internal_timer_hz(0),
	m_peripheral_clock_hz(0),
	m_sck_in_hz(0),
	m_sck_in(1),
	m_sck_out(1),
	m_sck_out_hz(0),
	m_sin(1),
	m_sin_prev(0),
	m_sot(1),
	m_hz(0),
	m_clock_count(0),
	m_tx_bit(-1),
	m_tx_bits(0),
	m_tx_shift(0),
	m_rx_bit(-1),
	m_rx_shift(0),
	m_smr(0),
	m_sidr(0),
	m_sodr(0)
{
}

void f2mc16_uart_device::device_start()
{
	m_rx_timer = timer_alloc(FUNC(f2mc16_uart_device::rx_timer_callback), this);
	m_tx_timer = timer_alloc(FUNC(f2mc16_uart_device::tx_timer_callback), this);

	save_item(NAME(m_internal_timer_changed));
	save_item(NAME(m_rx_start_time));
	save_item(NAME(m_tx_start_time));
	save_item(NAME(m_rx_ticks));
	save_item(NAME(m_tx_ticks));
	save_item(NAME(m_sck_in));
	save_item(NAME(m_sck_in_hz));
	save_item(NAME(m_sck_out));
	save_item(NAME(m_sck_out_hz));
	save_item(NAME(m_sin));
	save_item(NAME(m_sin_prev));
	save_item(NAME(m_sot));
	save_item(NAME(m_internal_timer_hz));
	save_item(NAME(m_hz));
	save_item(NAME(m_clock_count));
	save_item(NAME(m_tx_bit));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_rx_bit));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_smr));
	save_item(NAME(m_scr));
	save_item(NAME(m_sidr));
	save_item(NAME(m_sodr));
	save_item(NAME(m_ssr));
	save_item(NAME(m_cdcr));

	m_sck_cb(m_sck_out);
	m_sck_hz_cb(m_sck_out_hz);
	m_sot_cb(m_sot);

	update_serial();
}

void f2mc16_uart_device::device_reset()
{
	m_smr = 0;
	m_scr = SCR::REC;
	m_ssr = SSR::TDRE;
	m_cdcr = CDCR::DIV;

	update_serial();
}

void f2mc16_uart_device::device_clock_changed()
{
	m_peripheral_clock_hz = clock();
	update_serial();
}

void f2mc16_uart_device::internal_timer_hz(uint32_t hz)
{
	if (machine().scheduler().currently_executing())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_uart_device::update_internal_timer), this), hz);
	else
		update_internal_timer(hz);
}

void f2mc16_uart_device::internal_timer(int state)
{
	// TODO:
}

void f2mc16_uart_device::sck_hz(uint32_t hz)
{
	// TODO: track when hz changes.
	m_sck_in_hz = hz;

	update_serial();
}

void f2mc16_uart_device::sck(int state)
{
	if (m_sck_in != state)
	{
		m_sck_in = state;

		update_serial();
	}
}

void f2mc16_uart_device::sin(int state)
{
	if (m_sin != state)
	{
		m_sin = state;

		update_serial();
	}
}

uint8_t f2mc16_uart_device::smr_r()
{
	return m_smr;
}

void f2mc16_uart_device::smr_w(uint8_t data)
{
	m_smr = data & (SMR::MD | SMR::CS | SMR::SCKE | SMR::SOE);

	update_serial();
}

uint8_t f2mc16_uart_device::scr_r()
{
	return m_scr | SMR::CS;
}

void f2mc16_uart_device::scr_w(uint8_t data)
{
	if (!(data & SCR::REC))
	{
		m_ssr &= ~(SSR::PE | SSR::ORE | SSR::FRE);
		data |= SCR::REC;
	}

	m_scr = data & (SCR::PEN | SCR::P | SCR::SBL | SCR::CL | SCR::AD | SCR::REC | SCR::RXE | SCR::TXE);

	update_serial();
}

uint8_t f2mc16_uart_device::sidr_r()
{
	if (!machine().side_effects_disabled())
	{
		LOGREAD("%s read data %02x\n",machine().describe_context(), tag(), m_sidr);

		if (m_ssr & SSR::RDRF)
		{
			m_ssr &= ~SSR::RDRF;
			update_serial();
		}
	}

	return m_sidr;
}

void f2mc16_uart_device::sodr_w(uint8_t data)
{
	LOGWRITE("%s write data %02x\n", machine().describe_context(), data);

	m_sodr = data;
	m_ssr &= ~SSR::TDRE;

	update_serial();
}

uint8_t f2mc16_uart_device::ssr_r()
{
	return m_ssr;
}

void f2mc16_uart_device::ssr_w(uint8_t data)
{
	m_ssr = (m_ssr & ~(SSR::TIE | SSR::RIE)) | (data & (SSR::TIE | SSR::RIE));

	update_serial();
}

void f2mc16_uart_device::cdcr_w(uint8_t data)
{
	m_cdcr = data & CDCR::DIV;

	update_serial();
}

TIMER_CALLBACK_MEMBER(f2mc16_uart_device::update_internal_timer)
{
	m_internal_timer_hz = param;
	m_internal_timer_changed = machine().time();

	update_serial();
}

void f2mc16_uart_device::update_serial()
{
	const int clock_counts[] = { 16, 16, 1, 0 };
	const int baud_rate_generator[][5] =
	{
		{ 13, 4, 26, 52, 104 },
		{ 0, 0, 2, 4, 8 }
	};

	uint8_t clock_count = clock_counts[(m_smr & SMR::MD) >> 6];
	uint32_t hz;

	if ((m_smr & SMR::CS) == SMR::CS_RESERVED)
		hz = 0;
	else if ((m_smr & SMR::CS) == SMR::CS_INTERNAL_TIMER)
		hz = m_internal_timer_hz;
	else if ((m_smr & SMR::CS) == SMR::CS_EXTERNAL_CLOCK)
		hz = (m_smr & SMR::SCKE) ? 0 : m_sck_in_hz;
	else if (baud_rate_generator[(m_smr & SMR::MD) == SMR::MD_SYNC][(m_smr & SMR::CS) >> 3])
		hz = m_peripheral_clock_hz / (0x10 - m_cdcr) / baud_rate_generator[(m_smr & SMR::MD) == SMR::MD_SYNC][(m_smr & SMR::CS) >> 3];
	else
		hz = 0;

	//if (m_hz != hz || m_clock_count != clock_count)
	  //printf("%s baud %d\n", tag(), clock_count ? (hz / clock_count) : 0);

	m_clock_count = clock_count;
	m_hz = hz;

	uint32_t sck_out_clock = (m_smr & SMR::SCKE) ? m_hz : 0;
	if (m_sck_out_hz != sck_out_clock)
	{
		m_sck_out_hz = sck_out_clock;
		m_sck_hz_cb(m_sck_out_hz);
	}

	if (!m_sin && m_sin_prev && m_rx_bit < 0 &&
		((m_smr & SMR::MD) == SMR::MD_ASYNC_NORMAL || (m_smr & SMR::MD) == SMR::MD_ASYNC_MULTIPROCESSOR))
		m_rx_timer->adjust(attotime::from_hz((m_hz / m_clock_count) * 2));

	m_sin_prev = m_sin;

	if (m_tx_bit < 0 && !(m_ssr & SSR::TDRE))
		m_tx_timer->adjust(attotime::zero);

	m_intc->set_irq(m_tx_vector, (m_ssr & SSR::TIE) && (m_ssr & SSR::TDRE));
	m_intc->set_irq(m_rx_vector, (m_ssr & SSR::RIE) && (m_ssr & SSR::RDRF));
	m_intc->set_completion_request(m_rx_vector, (SSR::PE | SSR::ORE | SSR::FRE) != 0);
}

TIMER_CALLBACK_MEMBER(f2mc16_uart_device::rx_timer_callback)
{
	int data_bits = ((m_smr & SMR::MD) != SMR::MD_ASYNC_NORMAL || (m_scr & SCR::CL)) ? 8 : 7;
	int parity_bits = ((m_smr & SMR::MD) == SMR::MD_ASYNC_MULTIPROCESSOR ||
		((m_smr & SMR::MD) == SMR::MD_ASYNC_NORMAL && (m_scr & SCR::P))) ? 1 : 0;
	int start_stop_bits = (((m_smr & SMR::MD) == SMR::MD_ASYNC_NORMAL ||
		(m_smr & SMR::MD) == SMR::MD_ASYNC_MULTIPROCESSOR)) ? 2 : 0;
	int length = data_bits + parity_bits + start_stop_bits;

	if (m_rx_bit < 0)
	{
		if (start_stop_bits && m_sin)
			return;

		m_rx_bit = 0;
		m_rx_shift = 0;
	}

	if (m_sin)
		m_rx_shift |= 1 << m_rx_bit;
	m_rx_bit++;

	if (m_rx_bit >= length)
	{
		if ((m_smr & SMR::MD) == SMR::MD_ASYNC_NORMAL ||
			(m_smr & SMR::MD) == SMR::MD_ASYNC_MULTIPROCESSOR)
		{
			m_rx_shift >>= 1;

			if (!m_sin)
				m_ssr |= SSR::FRE;

			if ((m_smr & SMR::MD) == SMR::MD_ASYNC_NORMAL && (m_scr & SCR::PEN))
			{
				int parity = (m_scr & SCR::P) ? 1 : 0;
				for (int i = 0; i < data_bits; i++)
					parity ^= BIT(m_rx_shift, i);

				if (parity)
					m_ssr |= SSR::PE;
			}

			if (!(m_rx_shift & (1 << data_bits)))
				m_ssr |= SSR::FRE;
		}

		m_rx_bit = -1;

		if (m_ssr & SSR::RDRF)
			m_ssr |= SSR::ORE;
		else
		{
			m_ssr |= SSR::RDRF;
			m_sidr = BIT(m_rx_shift, 0, data_bits);
		}

		update_serial();
		return;
	}

	m_rx_timer->adjust(attotime::from_hz(m_hz / m_clock_count));
}

TIMER_CALLBACK_MEMBER(f2mc16_uart_device::tx_timer_callback)
{
	if (m_tx_bit == m_tx_bits)
		m_tx_bit = -1;

	if (m_tx_bit < 0)
	{
		if (!(m_ssr & SSR::TDRE))
		{
			m_tx_bit = 0;
			m_tx_bits = ((m_smr & SMR::MD) != SMR::MD_ASYNC_NORMAL || (m_scr & SCR::CL)) ? 8 : 7;
			m_tx_shift = BIT(m_sodr, 0, m_tx_bits);

			if ((m_smr & SMR::MD) == SMR::MD_ASYNC_MULTIPROCESSOR)
			{
				if (m_scr & SCR::AD)
					m_tx_shift |= 1 << m_tx_bits;
				m_tx_bits++;
			}
			else if ((m_scr & SCR::PEN) && (m_smr & SMR::MD) == SMR::MD_ASYNC_NORMAL)
			{
				int parity = (m_scr & SCR::P) ? 1 : 0;
				for (int i = 0; i < m_tx_bits; i++)
					parity ^= BIT(m_tx_shift, i);

				if (parity)
					m_tx_shift |= 1 << m_tx_bits;
				m_tx_bits++;
			}

			if ((m_smr & SMR::MD) == SMR::MD_ASYNC_NORMAL ||
				(m_smr & SMR::MD) == SMR::MD_ASYNC_MULTIPROCESSOR)
			{
				int stop_bits = (m_scr & SCR::SBL) ? 2 : 1;
				for (int i = 0; i < stop_bits; i++)
				{
					m_tx_shift |= 1 << m_tx_bits;
					m_tx_bits++;
				}

				m_tx_shift <<= 1;
				m_tx_bits++;
			}
			else if (m_rx_bit < 0 && (m_scr & SCR::RXE) && (m_smr & SMR::MD) == SMR::MD_SYNC)
				m_rx_timer->adjust(attotime::from_hz((m_hz / m_clock_count) * 2));

			m_ssr |= SSR::TDRE;
			update_serial();
		}
		else
			return;
	}

	m_sot_cb(BIT(m_tx_shift, m_tx_bit));
	m_tx_bit++;
	m_tx_timer->adjust(attotime::from_hz(m_hz / m_clock_count));
}
