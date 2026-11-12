// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/*

    Intel 8256(AH) Multifunction microprocessor support controller emulation

*/

#include "emu.h"
#include "i8256.h"

#include <algorithm>
#include <iterator>
#include <limits>

#define LOG_SETUP (1U << 1)
#define LOG_RX    (1U << 2)
#define LOG_TX    (1U << 3)
#define LOG_INT   (1U << 4)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP)
#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGRX(...)    LOGMASKED(LOG_RX,    __VA_ARGS__)
#define LOGTX(...)    LOGMASKED(LOG_TX,    __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)


namespace {

enum // MUART REGISTERS
{
	I8256_REG_CMD1,
	I8256_REG_CMD2,
	I8256_REG_CMD3,
	I8256_REG_MODE,
	I8256_REG_PORT1C,
	I8256_REG_INTEN,
	I8256_REG_INTAD,
	I8256_REG_BUFFER,
	I8256_REG_PORT1,
	I8256_REG_PORT2,
	I8256_REG_TIMER1,
	I8256_REG_TIMER2,
	I8256_REG_TIMER3,
	I8256_REG_TIMER4,
	I8256_REG_TIMER5,
	I8256_REG_STATUS,
};

enum
{
	I8256_CMD1_FRQ,
	I8256_CMD1_8086,
	I8256_CMD1_BITI,
	I8256_CMD1_BRKI,
	I8256_CMD1_S0,
	I8256_CMD1_S1,
	I8256_CMD1_L0,
	I8256_CMD1_L1
};

enum
{
	I8256_STOP_1,
	I8256_STOP_15,
	I8256_STOP_2,
	I8256_STOP_075
};

enum
{
	I8256_CMD2_B0,
	I8256_CMD2_B1,
	I8256_CMD2_B2,
	I8256_CMD2_B3,
	I8256_CMD2_C0,
	I8256_CMD2_C1,
	I8256_CMD2_EVEN_PARITY,
	I8256_CMD2_PARITY_ENABLE
};

enum
{
	I8256_BAUD_TXC,
	I8256_BAUD_TXC64,
	I8256_BAUD_TXC32,
	I8256_BAUD_19200,
	I8256_BAUD_9600,
	I8256_BAUD_4800,
	I8256_BAUD_2400,
	I8256_BAUD_1200,
	I8256_BAUD_600,
	I8256_BAUD_300,
	I8256_BAUD_200,
	I8256_BAUD_150,
	I8256_BAUD_110,
	I8256_BAUD_100,
	I8256_BAUD_75,
	I8256_BAUD_50
};

constexpr int BAUD_RATES[16] = {0, 0, 0, 19200, 9600, 4800, 2400, 1200, 600, 300, 200, 150, 110, 100, 75, 50};
constexpr uint16_t EXTERNAL_CLOCK_DIVIDER[3] = {1, 64, 32};

constexpr int SYS_CLOCK_DIVIDER[4] = {5, 3, 2, 1};

enum
{
	I8256_CMD3_RST,
	I8256_CMD3_TBRK,
	I8256_CMD3_SBRK,
	I8256_CMD3_END,
	I8256_CMD3_NIE,
	I8256_CMD3_IAE,
	I8256_CMD3_RxE,
	I8256_CMD3_SET
};

enum
{
	I8256_INT_TIMER1,
	I8256_INT_TIMER2,
	I8256_INT_EXTINT,
	I8256_INT_TIMER3,
	I8256_INT_RX,
	I8256_INT_TX,
	I8256_INT_TIMER4,
	I8256_INT_TIMER5
};

constexpr uint8_t TIMER_INTERRUPT[5] = {I8256_INT_TIMER1, I8256_INT_TIMER2, I8256_INT_TIMER3, I8256_INT_TIMER4, I8256_INT_TIMER5};

enum
{
	I8256_MODE_P2C0,
	I8256_MODE_P2C1,
	I8256_MODE_P2C2,
	I8256_MODE_CT2,
	I8256_MODE_CT3,
	I8256_MODE_T5C,
	I8256_MODE_T24,
	I8256_MODE_T35
};

enum // Upper / Lower
{
	I8256_PORT2C_II,
	I8256_PORT2C_IO,
	I8256_PORT2C_OI,
	I8256_PORT2C_OO,
	I8256_PORT2C_HI,
	I8256_PORT2C_HO,
	I8256_PORT2C_DNU,
	I8256_PORT2C_TEST
};

enum
{
	I8256_STATUS_FRAMING_ERROR  = 0x01,
	I8256_STATUS_OVERRUN_ERROR  = 0x02,
	I8256_STATUS_PARITY_ERROR   = 0x04,
	I8256_STATUS_BREAK          = 0x08,
	I8256_STATUS_TR_EMPTY       = 0x10,
	I8256_STATUS_TB_EMPTY       = 0x20,
	I8256_STATUS_RB_FULL        = 0x40,
	I8256_STATUS_INT            = 0x80
};

enum
{
	I8256_MOD_DSC,
	I8256_MOD_TME,
	I8256_MOD_RS0,
	I8256_MOD_RS1,
	I8256_MOD_RS2,
	I8256_MOD_RS3,
	I8256_MOD_RS4,
	I8256_MOD_0
};

enum
{
	I8256_STATE_START,
	I8256_STATE_DATA,
	I8256_STATE_STOP,
	I8256_STATE_BREAK,
	I8256_STATE_MARK
};
} // anonymous namespace

DEFINE_DEVICE_TYPE(I8256, i8256_device, "intel_8256", "Intel 8256AH Multifunction microprocessor support controller")

i8256_device::i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, I8256, tag, owner, clock),
	m_out_int_cb(*this),
	m_txd_handler(*this),
	m_txc_handler(*this),
	m_rxc_handler(*this),
	m_in_p1_cb(*this, 0),
	m_out_p1_cb(*this),
	m_in_p2_cb(*this, 0),
	m_out_p2_cb(*this),
	m_timer(nullptr),
	m_brg_timer(nullptr),
	m_test_clock_timer(nullptr),
	m_test_clock(false),
	m_mode(0),
	m_port1_control(0),
	m_port1_input(0),
	m_rxd(1),
	m_cts(0),
	m_extint(0),
	m_rxc(0),
	m_txc(0),
	m_rxc_output(1),
	m_txc_output(1),
	m_int_state(false),
	m_txd(1)
{
}

void i8256_device::device_start()
{
	m_timer = timer_alloc(FUNC(i8256_device::timer_check), this);
	m_brg_timer = timer_alloc(FUNC(i8256_device::brg_tick), this);
	m_test_clock_timer = timer_alloc(FUNC(i8256_device::test_clock_tick), this);
	m_txc_handler(m_txc_output);
	m_rxc_handler(m_rxc_output);

	// buffers, parallel port latches and timer counts retain their contents on reset
	m_rx_buffer = 0;
	m_tx_buffer = 0;
	m_port1_int = 0;
	m_port2_int = 0;
	m_port2_input_latch = 0;
	m_port2_handshake = true;
	std::fill(std::begin(m_timers), std::end(m_timers), 0);
	m_timer4_read_latch = 0;
	m_timer5_read_latch = 0;

	save_item(NAME(m_test_clock));
	save_item(NAME(m_command1));
	save_item(NAME(m_command2));
	save_item(NAME(m_command3));
	save_item(NAME(m_mode));
	save_item(NAME(m_port1_control));
	save_item(NAME(m_modification));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_int_request));
	save_item(NAME(m_int_service));
	save_item(NAME(m_status));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_port1_int));
	save_item(NAME(m_port2_int));
	save_item(NAME(m_port2_input_latch));
	save_item(NAME(m_port2_handshake));
	save_item(NAME(m_timers));
	save_item(NAME(m_timer5_reload));
	save_item(NAME(m_timer5_running));
	save_item(NAME(m_timer4_read_latch));
	save_item(NAME(m_timer5_read_latch));
	save_item(NAME(m_timer4_read_latched));
	save_item(NAME(m_timer5_read_latched));

	save_item(NAME(m_data_bits));
	save_item(NAME(m_parity_enable));
	save_item(NAME(m_parity_even));
	save_item(NAME(m_stop_sel));
	save_item(NAME(m_divide));
	save_item(NAME(m_rx_sample));

	save_item(NAME(m_port1_input));
	save_item(NAME(m_rxd));
	save_item(NAME(m_cts));
	save_item(NAME(m_extint));
	save_item(NAME(m_rxc));
	save_item(NAME(m_txc));
	save_item(NAME(m_rxc_output));
	save_item(NAME(m_txc_output));
	save_item(NAME(m_int_state));

	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_counter));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_rx_tx_active));

	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_counter));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_break));
	save_item(NAME(m_cts_pending));
	save_item(NAME(m_cts_counter));
	save_item(NAME(m_txd));
}

void i8256_device::device_reset()
{
	const bool port1_output = port1_output_mask() != 0;
	const bool port2_output = port2_output_mask() != 0;

	m_command1 = 0;
	m_command2 = 0;
	m_command3 = 0;
	m_mode = 0;
	m_timer5_reload = 0;
	m_timer5_running = false;
	m_timer4_read_latched = false;
	m_timer5_read_latched = false;
	m_port1_control = 0;
	m_modification = 0;
	m_test_clock = false;
	m_test_clock_timer->adjust(attotime::never);
	if (port1_output)
		m_out_p1_cb(0, 0);
	if (port2_output)
		m_out_p2_cb(0, 0);

	m_data_bits = 8;
	m_parity_enable = false;
	m_parity_even = false;
	m_stop_sel = I8256_STOP_1;
	m_divide = 1;
	m_rx_sample = 16; // sample at bit center

	soft_reset();

	m_brg_timer->adjust(attotime::never);
	update_timer_rate();
}

void i8256_device::soft_reset()
{
	m_status = I8256_STATUS_TR_EMPTY | I8256_STATUS_TB_EMPTY;
	m_int_enable = 0;
	m_int_request = 0;
	m_int_service = 0;
	update_int();
	set_port2_handshake(true);

	m_rx_state = I8256_STATE_START;
	m_rx_counter = 0;
	m_rx_bits = 0;
	m_rx_shift = 0;
	m_rx_parity = false;
	m_rx_tx_active = false;

	m_tx_state = I8256_STATE_START;
	m_tx_counter = 0;
	m_tx_bits = 0;
	m_tx_shift = 0;
	m_tx_parity = false;
	m_tx_break = false;
	m_cts_pending = false;
	m_cts_counter = 0;
	output_txd(1);
	output_txc(1);
	output_rxc(1);
}

// reconfigure the counter/timer clock from the prescaled system clock; the
// system clock prescaler is selected by CMD2 bits 4-5 and the timer divider
// (16 kHz vs 1 kHz at the nominal 1.024 MHz internal clock) by CMD1 FRQ
void i8256_device::update_timer_rate()
{
	const int divider = BIT(m_command1, I8256_CMD1_FRQ) ? 1024 : 64;
	const attotime period = attotime::from_hz((clock() / SYS_CLOCK_DIVIDER[BIT(m_command2, I8256_CMD2_C0, 2)]) / divider);
	m_timer->adjust(period, 0, period);
}

// timer interrupts are automatically disabled when the request is generated
void i8256_device::request_timer_interrupt(int level)
{
	// Port 2 handshaking replaces Timer 5 as the level 7 interrupt source.
	if (level == I8256_INT_TIMER5 && port2_handshake_mode())
		return;
	request_interrupt(level);
	m_int_enable &= ~(1 << level);
}

// decrement a timer or cascaded timer pair; single timers generate an
// interrupt request on the transition from 01 to 00, cascaded ones on
// the transition from 0001 to 0000
void i8256_device::count_timer(int i)
{
	// timers 4 and 5 hold the upper bytes of cascaded timers 2 and 3
	const bool t24 = (i == 1) && BIT(m_mode, I8256_MODE_T24);
	const bool t35 = (i == 2) && BIT(m_mode, I8256_MODE_T35);
	const int high = t24 ? 3 : 4;

	// Retriggerable Timer 5 (or the 3/5 pair) waits for a falling edge on P15.
	if ((i == 4 || t35) && BIT(m_mode, I8256_MODE_T5C) && !m_timer5_running)
		return;

	if (m_timers[i]-- == 0)
	{
		// timers count down continuously and wrap around past zero; the low
		// byte of a cascaded pair borrows from the upper byte (FFFFH on wrap)
		if (t24 || t35)
			m_timers[high]--;
		return;
	}

	if (m_timers[i] != 0)
		return;

	if (t24 || t35)
	{
		if (m_timers[high] == 0)
			request_timer_interrupt(t24 ? I8256_INT_TIMER4 : I8256_INT_TIMER3); // levels 6 and 3
		return;
	}

	// with BITI set, level 1 belongs to the port 1 P17 interrupt instead of timer 2
	if (i == 1 && BIT(m_command1, I8256_CMD1_BITI))
		return;

	request_timer_interrupt(TIMER_INTERRUPT[i]);
}

TIMER_CALLBACK_MEMBER(i8256_device::timer_check)
{
	for (int i = 0; i < int(std::size(m_timers)); i++)
	{
		// the upper bytes of cascaded pairs do not count on their own
		if ((i == 3 && BIT(m_mode, I8256_MODE_T24)) || (i == 4 && BIT(m_mode, I8256_MODE_T35)))
			continue;

		// with CT2/CT3 set, timers 2 and 3 count port 1 events instead of the internal clock
		if ((i == 1 && BIT(m_mode, I8256_MODE_CT2)) || (i == 2 && BIT(m_mode, I8256_MODE_CT3)))
			continue;

		count_timer(i);
	}
}

//**************************************************************************
//  INTERRUPT CONTROLLER
//**************************************************************************

void i8256_device::update_int()
{
	// EXTINT is level sensitive, unlike the latched internal requests.
	if (m_extint && BIT(m_int_enable, I8256_INT_EXTINT))
		m_int_request |= 1 << I8256_INT_EXTINT;
	else
		m_int_request &= ~(1 << I8256_INT_EXTINT);

	const bool state = pending_interrupt() >= 0;

	if (state)
		m_status |= I8256_STATUS_INT;
	else
		m_status &= ~I8256_STATUS_INT;

	if (m_int_state != state)
	{
		m_int_state = state;
		m_out_int_cb(state ? 1 : 0);
	}
}

void i8256_device::request_interrupt(int level)
{
	if (BIT(m_int_enable, level))
	{
		LOGINT("interrupt request on level %d\n", level);
		m_int_request |= 1 << level;
		update_int();
	}
}

int i8256_device::pending_interrupt() const
{
	for (int level = 0; level < 8; level++)
	{
		// Only requests above the highest priority in service are eligible.
		if (BIT(m_command3, I8256_CMD3_NIE) && BIT(m_int_service, level))
			break;
		if (BIT(m_int_request, level))
			return level;
	}
	return -1;
}

// acknowledge the highest priority eligible interrupt (level 0 is highest)
int i8256_device::acknowledge()
{
	const int level = pending_interrupt();
	if (level >= 0 && !machine().side_effects_disabled())
	{
		m_int_request &= ~(1 << level);
		if (BIT(m_command3, I8256_CMD3_NIE))
			m_int_service |= 1 << level;
		update_int();
	}
	return level;
}

uint8_t i8256_device::inta_r()
{
	if (!BIT(m_command3, I8256_CMD3_IAE))
		return 0xff;

	const int level = std::max(acknowledge(), 0);

	if (BIT(m_command1, I8256_CMD1_8086))
		return 0x40 + level; // interrupt vector
	else
		return 0xc7 | (level << 3); // RST n instruction
}

void i8256_device::write_extint(int state)
{
	state = state ? 1 : 0;
	if (m_extint == state)
		return;

	m_extint = state;
	update_int();
}

//**************************************************************************
//  MICROPROCESSOR INTERFACE
//**************************************************************************

uint8_t i8256_device::read(offs_t offset)
{
	uint8_t reg = offset & 0x0f;

	// In 8-bit mode, AD0-AD3 select the register and AD4 is ignored, while
	// AD1-AD4 are used in 16-bit mode and AD0 is a second chip select, active low.
	if (BIT(m_command1, I8256_CMD1_8086))
	{
		if (BIT(offset, 0))
			return 0xff;
		reg = BIT(offset, 1, 4);
	}

	switch (reg)
	{
		case I8256_REG_CMD1:
			return m_command1;
		case I8256_REG_CMD2:
			return m_command2;
		case I8256_REG_CMD3:
			// when command register 3 is read, bits 0, 3, and 7 will always be zero
			return m_command3 & 0x76;
		case I8256_REG_MODE:
			return m_mode;
		case I8256_REG_PORT1C:
			return m_port1_control;
		case I8256_REG_INTEN:
			return m_int_enable;
		case I8256_REG_INTAD:
		{
			// the identifier is the number of the interrupt level multiplied by 4;
			// reading has the same effect as a hardware interrupt acknowledge
			const int level = std::max(acknowledge(), 0);
			return level * 4;
		}
		case I8256_REG_BUFFER:
			if (!machine().side_effects_disabled())
				m_status &= ~I8256_STATUS_RB_FULL;
			return m_rx_buffer;
		case I8256_REG_PORT1:
			return read_port1();
		case I8256_REG_PORT2:
			return read_port2();
		case I8256_REG_TIMER1:
			return m_timers[0];
		case I8256_REG_TIMER2:
		case I8256_REG_TIMER3:
		case I8256_REG_TIMER4:
		case I8256_REG_TIMER5:
		{
			const int timer = reg - I8256_REG_TIMER1;
			const bool timer4 = reg == I8256_REG_TIMER2 || reg == I8256_REG_TIMER4;
			if (!BIT(m_mode, timer4 ? I8256_MODE_T24 : I8256_MODE_T35))
				return m_timers[timer];

			const bool high = reg == I8256_REG_TIMER4 || reg == I8256_REG_TIMER5;
			uint16_t &latch = timer4 ? m_timer4_read_latch : m_timer5_read_latch;
			bool &latched = timer4 ? m_timer4_read_latched : m_timer5_read_latched;
			if (high && !latched && !machine().side_effects_disabled())
			{
				const int low = timer4 ? 1 : 2;
				latch = (uint16_t(m_timers[low + 2]) << 8) | m_timers[low];
				latched = true;
			}

			const uint8_t data = latched ? BIT(latch, high ? 8 : 0, 8) : m_timers[timer];
			if (!high && !machine().side_effects_disabled())
				latched = false;
			return data;
		}
		case I8256_REG_STATUS:
		{
			const uint8_t status = m_status;
			if (!machine().side_effects_disabled())
			{
				uint8_t clear = I8256_STATUS_OVERRUN_ERROR | I8256_STATUS_PARITY_ERROR | I8256_STATUS_BREAK;
				// reading the status register does not reset FE in transmission mode
				if (!BIT(m_modification, I8256_MOD_TME))
					clear |= I8256_STATUS_FRAMING_ERROR;
				m_status &= ~clear;
			}
			return status;
		}
	}
	return 0xff;
}

void i8256_device::write(offs_t offset, uint8_t data)
{
	uint8_t reg = offset & 0x0f;

	if (BIT(m_command1, I8256_CMD1_8086))
	{
		if (BIT(offset, 0))
			return;
		reg = BIT(offset, 1, 4);
	}

	switch (reg)
	{
		case I8256_REG_CMD1:
		{
			const uint8_t old_mask = port1_output_mask();
			const uint8_t old_output = port1_output();
			const bool frq_changed = BIT(m_command1 ^ data, I8256_CMD1_FRQ);
			m_command1 = data;
			if (old_mask != port1_output_mask() || old_output != port1_output())
				m_out_p1_cb(0, port1_output());
			if (frq_changed)
				update_timer_rate();

			m_data_bits = 8 - BIT(data, I8256_CMD1_L0, 2);
			if (m_stop_sel != BIT(data, I8256_CMD1_S0, 2))
			{
				m_cts_pending = false;
				m_cts_counter = 0;
			}
			m_stop_sel = BIT(data, I8256_CMD1_S0, 2);

			LOGSETUP("%s CR1: %d data bits, stop bit select %d, %s mode\n", machine().describe_context(),
					m_data_bits, m_stop_sel, BIT(data, I8256_CMD1_8086) ? "8086" : "8085");
			break;
		}
		case I8256_REG_CMD2:
		{
			const uint8_t old_p1_output = port1_output();
			const uint8_t changed = m_command2 ^ data;
			m_command2 = data;

			m_parity_enable = BIT(data, I8256_CMD2_PARITY_ENABLE);
			m_parity_even = BIT(data, I8256_CMD2_EVEN_PARITY);

			const int prescaler = SYS_CLOCK_DIVIDER[BIT(data, I8256_CMD2_C0, 2)];
			const uint32_t internal_clock = clock() / prescaler;
			LOGSETUP("%s CR2: %s parity, system clock prescaler /%d, baud select %X\n", machine().describe_context(),
					m_parity_enable ? (m_parity_even ? "even" : "odd") : "no",
					prescaler, data & 0x0f);
			if (internal_clock != 1'024'000)
				logerror("internal clock should be 1024000, calculated: %u\n", internal_clock);

			// the system clock prescaler also feeds the counter/timer
			if (BIT(changed, I8256_CMD2_C0, 2))
				update_timer_rate();

			const uint8_t baud = data & 0x0f;
			if (changed & 0x3f)
				update_test_clock();
			if (old_p1_output != port1_output())
				m_out_p1_cb(0, port1_output());
			if (BIT(changed, I8256_CMD2_B0, 4))
			{
				output_txc(1);
				output_rxc(1);
			}
			if (!(changed & 0x3f))
				break;
			if (baud <= I8256_BAUD_TXC32)
			{
				m_divide = EXTERNAL_CLOCK_DIVIDER[baud];
				m_brg_timer->adjust(attotime::never);
			}
			else
			{
				// internal baud rate generator; the bit clock is emulated in
				// 1/32 bit steps, the granularity of the modification register
				// baud rates are nominal at a 1.024 MHz prescaled system clock
				m_divide = 32;
				const attotime period = attotime::from_hz(BAUD_RATES[baud] * 32 *
						double(clock()) / (prescaler * 1'024'000));
				m_brg_timer->adjust(period, 0, period);
			}
			break;
		}
		case I8256_REG_CMD3:
			// bit 7 high sets any bits which are also high, bit 7 low resets them
			if (BIT(data, I8256_CMD3_SET))
				m_command3 |= data & 0x7f;
			else
				m_command3 &= ~(data & 0x72); // RST, SBRK and END are set-only

			if (BIT(m_command3, I8256_CMD3_RST))
			{
				soft_reset();
				m_command3 &= ~(1 << I8256_CMD3_RST);
			}

			if (BIT(m_command3, I8256_CMD3_NIE) && BIT(m_command3, I8256_CMD3_END))
			{
				// END completes the innermost (highest priority) interrupt in service.
				for (int level = 0; level < 8; level++)
				{
					if (BIT(m_int_service, level))
					{
						m_int_service &= ~(1 << level);
						break;
					}
				}
			}
			m_command3 &= ~(1 << I8256_CMD3_END);
			update_int();
			break;
		case I8256_REG_MODE:
		{
			const uint8_t old_mask = port2_output_mask();
			const uint8_t old_p1_mask = port1_output_mask();
			const uint8_t old_p1_output = port1_output();
			const bool old_test_clock = test_clock_enabled();
			if (!BIT(m_mode, I8256_MODE_T5C) && BIT(data, I8256_MODE_T5C))
				m_timer5_running = false;
			m_mode = data;
			if (old_test_clock != test_clock_enabled())
				update_test_clock();
			if (old_p1_mask != port1_output_mask() || old_p1_output != port1_output())
				m_out_p1_cb(0, port1_output());
			const uint8_t mask = port2_output_mask();
			if (old_mask != mask)
				m_out_p2_cb(0, m_port2_int & mask);
			if (!BIT(m_mode, I8256_MODE_T24))
				m_timer4_read_latched = false;
			if (!BIT(m_mode, I8256_MODE_T35))
				m_timer5_read_latched = false;
			break;
		}
		case I8256_REG_PORT1C:
			if (m_port1_control != data)
			{
				const uint8_t old_mask = port1_output_mask();
				const uint8_t old_output = port1_output();
				const bool old_test_clock = test_clock_enabled();
				m_port1_control = data;
				if (old_test_clock != test_clock_enabled())
					update_test_clock();
				if (old_mask != port1_output_mask() || old_output != port1_output())
					m_out_p1_cb(0, port1_output());
			}
			break;
		case I8256_REG_INTEN: // set interrupts
			m_int_enable |= data;
			update_int();
			break;
		case I8256_REG_INTAD: // reset interrupts
			m_int_enable &= ~data;
			m_int_request &= ~data;
			update_int();
			break;
		case I8256_REG_BUFFER:
			LOGTX("%s transmitter buffer %02X\n", machine().describe_context(), data);
			m_tx_buffer = data;
			m_status &= ~I8256_STATUS_TB_EMPTY;
			break;
		case I8256_REG_PORT1:
			write_port1(data);
			break;
		case I8256_REG_PORT2:
			write_port2(data);
			break;
		case I8256_REG_TIMER1:
			m_timers[0] = data;
			break;
		case I8256_REG_TIMER2:
			m_timers[1] = data;
			m_timer4_read_latched = false;
			break;
		case I8256_REG_TIMER3:
			m_timers[2] = data;
			m_timer5_read_latched = false;
			break;
		case I8256_REG_TIMER4:
			m_timers[3] = data;
			if (BIT(m_mode, I8256_MODE_T24))
				m_timers[1] = 0xff;
			m_timer4_read_latched = false;
			break;
		case I8256_REG_TIMER5:
			if (BIT(m_mode, I8256_MODE_T5C))
			{
				// In retriggerable mode a write loads only the save register and stops counting.
				m_timer5_reload = data;
				m_timer5_running = false;
			}
			else
			{
				m_timers[4] = data;
				if (BIT(m_mode, I8256_MODE_T35))
					m_timers[2] = 0xff;
			}
			m_timer5_read_latched = false;
			break;
		case I8256_REG_STATUS: // modification register
			m_modification = data & 0x7f;
			m_rx_sample = (BIT(data, I8256_MOD_RS4) ? 32 : 16) - BIT(data, I8256_MOD_RS0, 4);
			LOGSETUP("%s modification: sample point %d/32%s%s\n", machine().describe_context(), m_rx_sample,
					BIT(data, I8256_MOD_DSC) ? ", start bit check disabled" : "",
					BIT(data, I8256_MOD_TME) ? ", transmission mode" : "");
			break;
	}
}

//**************************************************************************
//  PARALLEL PORTS
//**************************************************************************

void i8256_device::write_p1(uint8_t data)
{
	const uint8_t rising = data & ~m_port1_input;
	const uint8_t falling = ~data & m_port1_input;
	m_port1_input = data;
	if (port2_handshake_mode())
	{
		// STB asserts IBF low; ACK releases OBF high. Both interrupt on the rising edge.
		if (BIT(falling, 0))
			set_port2_handshake((m_mode & 0x07) == I8256_PORT2C_HO);
		if (BIT(rising, 0))
		{
			if ((m_mode & 0x07) == I8256_PORT2C_HI)
				m_port2_input_latch = m_in_p2_cb(0);
			request_interrupt(I8256_INT_TIMER5);
		}
	}
	if (BIT(falling, 5) && BIT(m_mode, I8256_MODE_T5C))
	{
		m_timers[4] = m_timer5_reload;
		if (BIT(m_mode, I8256_MODE_T35))
			m_timers[2] = 0xff;
		m_timer5_running = true;
	}
	if (BIT(rising, 2) && BIT(m_mode, I8256_MODE_CT2))
		count_timer(1);
	if (BIT(rising, 3) && BIT(m_mode, I8256_MODE_CT3))
		count_timer(2);
	if (BIT(rising, 7) && BIT(m_command1, I8256_CMD1_BITI))
		request_interrupt(I8256_INT_TIMER2);
}

uint8_t i8256_device::read_port1()
{
	// Special-function inputs read the saved output latch, not the pin.
	const uint8_t input = m_in_p1_cb.isunset() ? m_port1_input : m_in_p1_cb(0);
	const uint8_t special_inputs = port1_special_input_mask();
	const uint8_t output = test_clock_enabled() ?
			(port1_output() & ~0x10) | (m_port1_int & 0x10) : port1_output();
	return (input & ~(port1_output_mask() | special_inputs)) | output | (m_port1_int & special_inputs);
}

uint8_t i8256_device::port1_special_input_mask() const
{
	uint8_t mask = 0;
	if (BIT(m_mode, I8256_MODE_CT2))
		mask |= 0x04;
	if (BIT(m_mode, I8256_MODE_CT3))
		mask |= 0x08;
	if (BIT(m_mode, I8256_MODE_T5C))
		mask |= 0x20;
	if (BIT(m_command1, I8256_CMD1_BRKI))
		mask |= 0x40;
	if (BIT(m_command1, I8256_CMD1_BITI))
		mask |= 0x80;
	return mask;
}

uint8_t i8256_device::port1_output_mask() const
{
	const uint8_t mask = m_port1_control & ~port1_special_input_mask();
	return port2_handshake_mode() ? (mask & ~0x03) | 0x02 : mask;
}

uint8_t i8256_device::port1_output() const
{
	const uint8_t output = m_port1_int & port1_output_mask();
	if (test_clock_enabled())
		return (output & ~0x10) | (m_test_clock ? 0x10 : 0x00);
	if (port2_handshake_mode())
		return (output & ~0x03) | (m_port2_handshake ? 0x02 : 0x00);
	return output;
}

bool i8256_device::test_clock_enabled() const
{
	return (m_mode & 0x07) == I8256_PORT2C_TEST && BIT(m_port1_control, 4) &&
			(m_command2 & 0x0f) > I8256_BAUD_TXC32;
}

void i8256_device::update_test_clock()
{
	m_test_clock = false;
	if (test_clock_enabled())
	{
		const uint8_t baud = m_command2 & 0x0f;
		// P14 carries the raw BRG clock: 64x baud, except 32x at 19.2 kbaud.
		const int multiplier = (baud == I8256_BAUD_19200) ? 32 : 64;
		const int prescaler = SYS_CLOCK_DIVIDER[BIT(m_command2, I8256_CMD2_C0, 2)];
		const attotime period = attotime::from_hz(BAUD_RATES[baud] * multiplier * 2 *
				double(clock()) / (prescaler * 1'024'000));
		m_test_clock_timer->adjust(period, 0, period);
	}
	else
		m_test_clock_timer->adjust(attotime::never);
}

bool i8256_device::port2_handshake_mode() const
{
	return (m_mode & 0x07) == I8256_PORT2C_HI || (m_mode & 0x07) == I8256_PORT2C_HO;
}

void i8256_device::set_port2_handshake(bool state)
{
	if (m_port2_handshake != state)
	{
		m_port2_handshake = state;
		if (port2_handshake_mode())
			m_out_p1_cb(0, port1_output());
	}
}

void i8256_device::write_port1(uint8_t data)
{
	// all bits are latched; input pins output the latch if the direction is changed later
	const uint8_t old_output = port1_output();
	m_port1_int = data;
	if (old_output != port1_output())
		m_out_p1_cb(0, port1_output());
}

uint8_t i8256_device::read_port2()
{
	switch (m_mode & 0x07)
	{
		case I8256_PORT2C_II:
			return m_in_p2_cb(0);
		case I8256_PORT2C_IO:
			return (m_in_p2_cb(0) & 0xf0) | (m_port2_int & 0x0f);
		case I8256_PORT2C_OI:
			return (m_in_p2_cb(0) & 0x0f) | (m_port2_int & 0xf0);
		case I8256_PORT2C_OO:
			return m_port2_int;
		case I8256_PORT2C_HI:
		{
			const uint8_t data = m_port2_input_latch;
			if (!machine().side_effects_disabled())
				set_port2_handshake(true);
			return data;
		}
		case I8256_PORT2C_HO:
			return m_port2_int;
		case I8256_PORT2C_TEST:
			return 0;
		case I8256_PORT2C_DNU:
		default:
			// "do not use"
			return 0;
	}
}

void i8256_device::write_port2(uint8_t data)
{
	const uint8_t mask = port2_output_mask();
	const uint8_t changed = (m_port2_int ^ data) & mask;
	m_port2_int = data;
	if (changed)
		m_out_p2_cb(0, m_port2_int & mask);
	if ((m_mode & 0x07) == I8256_PORT2C_HO)
		set_port2_handshake(false);
}

uint8_t i8256_device::port2_output_mask() const
{
	switch (m_mode & 0x07)
	{
		case I8256_PORT2C_IO: return 0x0f;
		case I8256_PORT2C_OI: return 0xf0;
		case I8256_PORT2C_OO: return 0xff;
		case I8256_PORT2C_HO: return 0xff;
		default:
			return 0;
	}
}

//**************************************************************************
//  SERIAL RECEIVER
//**************************************************************************

// position of the sample point within a bit, in bit clocks
uint16_t i8256_device::rx_sample_point() const
{
	return std::max<uint16_t>(1, (m_rx_sample * m_divide) / 32);
}

void i8256_device::receiver_tick()
{
	// Track transmitter activity from start bit detection to the stop bit sample.
	if (m_rx_state == I8256_STATE_START && !m_rx_counter)
		m_rx_tx_active = false;
	if (!(m_status & I8256_STATUS_TR_EMPTY))
		m_rx_tx_active = true;

	m_rx_counter++;

	// RxC rises at each data sample and stays high throughout framing bits.
	if ((m_command2 & 0x0f) != I8256_BAUD_TXC)
		output_rxc(!(m_rx_state == I8256_STATE_DATA && m_rx_bits < m_data_bits &&
				m_rx_counter >= m_divide / 2 && m_rx_counter < m_divide));

	switch (m_rx_state)
	{
		case I8256_STATE_START:
			if (m_rx_counter == 1 && m_rxd)
			{
				// line is marking, no start bit yet
				m_rx_counter = 0;
			}
			else if (m_rx_counter >= rx_sample_point())
			{
				// the start bit is checked once at sampling time (unless disabled by DSC)
				if (!m_rxd || BIT(m_modification, I8256_MOD_DSC))
				{
					LOGRX("%s RX start bit\n", machine().describe_context());
					m_rx_state = I8256_STATE_DATA;
					m_rx_counter = 0;
					m_rx_shift = 0;
					m_rx_bits = 0;
					m_rx_parity = false;
				}
				else
				{
					LOGRX("%s RX false start bit\n", machine().describe_context());
					m_rx_counter = 0;
				}
			}
			break;

		case I8256_STATE_DATA:
			if (m_rx_counter >= m_divide)
			{
				m_rx_counter = 0;

				if (m_rxd)
				{
					// Mid-frame format changes can leave the bit count beyond the shift register.
					if (m_rx_bits < std::numeric_limits<uint16_t>::digits)
						m_rx_shift |= uint16_t(1) << m_rx_bits;
					m_rx_parity = !m_rx_parity;
				}
				m_rx_bits++;

				if (m_rx_bits == m_data_bits + (m_parity_enable ? 1 : 0))
					m_rx_state = I8256_STATE_STOP;
			}
			break;

		case I8256_STATE_STOP:
			// the character is transferred and all status bits are evaluated
			// when the first stop bit is sampled
			if (m_rx_counter >= m_divide)
			{
				m_rx_counter = 0;
				m_rx_state = I8256_STATE_START;

				const bool fe = !m_rxd;
				const uint8_t data = m_rx_shift & util::make_bitmask<uint16_t>(m_data_bits);

				// A break requires all data bits and the optional parity bit to be low.
				if (fe && m_rx_shift == 0)
				{
					// break character: BD is set even when the receiver is disabled,
					// and the character is not transferred to the receiver buffer
					LOGRX("%s RX break detected\n", machine().describe_context());
					m_status |= I8256_STATUS_BREAK;
					request_interrupt(I8256_INT_RX);
					m_rx_state = I8256_STATE_BREAK;
				}
				else if (BIT(m_command3, I8256_CMD3_RxE))
				{
					LOGRX("%s RX character %02X\n", machine().describe_context(), data);

					// on overrun the previous character is lost
					if (m_status & I8256_STATUS_RB_FULL)
						m_status |= I8256_STATUS_OVERRUN_ERROR;
					m_rx_buffer = data;
					m_status |= I8256_STATUS_RB_FULL;

					if (m_parity_enable && (m_parity_even ? m_rx_parity : !m_rx_parity))
						m_status |= I8256_STATUS_PARITY_ERROR;

					if (BIT(m_modification, I8256_MOD_TME))
					{
						// in transmission mode FE indicates that the transmitter
						// was active during the reception of a character
						if (m_rx_tx_active)
							m_status |= I8256_STATUS_FRAMING_ERROR;
						else
							m_status &= ~I8256_STATUS_FRAMING_ERROR;
					}
					else if (fe)
						m_status |= I8256_STATUS_FRAMING_ERROR;

					request_interrupt(I8256_INT_RX);
				}
			}
			break;

		case I8256_STATE_BREAK:
			// the receiver is idled until the line returns to marking
			if (m_rxd)
			{
				m_rx_state = I8256_STATE_START;
				m_rx_counter = 0;
			}
			break;
	}
}

//**************************************************************************
//  SERIAL TRANSMITTER
//**************************************************************************

uint16_t i8256_device::stop_length() const
{
	switch (m_stop_sel)
	{
		default:
		case I8256_STOP_1:   return m_divide;
		case I8256_STOP_15:  return m_divide + m_divide / 2;
		case I8256_STOP_2:   return m_divide * 2;
		case I8256_STOP_075: return std::max<uint16_t>(1, (m_divide * 3) / 4);
	}
}

void i8256_device::output_txd(int state)
{
	if (m_txd != state)
	{
		m_txd = state;
		m_txd_handler(state);
	}
}

void i8256_device::output_txc(int state)
{
	if (m_txc_output != state)
	{
		m_txc_output = state;
		m_txc_handler(state);
	}
}

void i8256_device::output_rxc(int state)
{
	if (m_rxc_output != state)
	{
		m_rxc_output = state;
		m_rxc_handler(state);
	}
}

void i8256_device::check_break_in()
{
	if (BIT(m_command1, I8256_CMD1_BRKI))
	{
		const uint8_t input = m_in_p1_cb.isunset() ? m_port1_input : m_in_p1_cb(0);
		if (!BIT(input, 6))
		{
			m_status |= I8256_STATUS_BREAK;
			request_interrupt(I8256_INT_TX);
		}
	}
}

void i8256_device::start_transmit()
{
	m_tx_counter = 0;

	if (BIT(m_command3, I8256_CMD3_SBRK))
	{
		// single character break: start, data, parity and stop bits all low
		LOGTX("%s TX single character break\n", machine().describe_context());
		m_tx_break = true;
		m_tx_bits = 0;
		m_status &= ~I8256_STATUS_TR_EMPTY;
		m_tx_state = I8256_STATE_DATA;
		if ((m_command2 & 0x0f) > I8256_BAUD_TXC32)
			output_txc(0);
		output_txd(0);
	}
	else if (BIT(m_command3, I8256_CMD3_TBRK))
	{
		// continuous break inhibits buffer transfers, but not SBRK timing
		output_txd(0);
	}
	else if (m_tx_break)
	{
		// Return to marking for one bit before transferring data after a break.
		m_tx_break = false;
		m_tx_state = I8256_STATE_MARK;
		output_txd(1);
	}
	else if (!(m_status & I8256_STATUS_TB_EMPTY) &&
		(m_cts_pending || (m_stop_sel != I8256_STOP_075 && !m_cts && !m_cts_counter)))
	{
		// transfer the buffer to the transmitter register
		m_cts_pending = false;
		m_cts_counter = 0;
		m_tx_shift = m_tx_buffer;
		LOGTX("%s TX character %02X\n", machine().describe_context(), m_tx_shift);
		m_tx_break = false;
		m_tx_bits = 0;
		m_tx_parity = false;
		m_status |= I8256_STATUS_TB_EMPTY;
		m_status &= ~I8256_STATUS_TR_EMPTY;
		request_interrupt(I8256_INT_TX);
		m_tx_state = I8256_STATE_DATA;
		if ((m_command2 & 0x0f) > I8256_BAUD_TXC32)
			output_txc(0);
		output_txd(0); // start bit
	}
	else
		output_txd(1);
}

void i8256_device::transmitter_tick()
{
	if (m_cts_counter && !--m_cts_counter)
		m_cts_pending = true;

	m_tx_counter++;

	if ((m_command2 & 0x0f) > I8256_BAUD_TXC32)
	{
		// TxC falls at bit boundaries and rises at bit centres. A fractional
		// final stop bit returns high before the next start resets the phase.
		const bool active = m_tx_state == I8256_STATE_DATA ||
				(m_tx_state == I8256_STATE_STOP && m_tx_counter < stop_length());
		output_txc(!active || (m_tx_counter % m_divide) >= m_divide / 2);
	}

	switch (m_tx_state)
	{
		case I8256_STATE_START:
			start_transmit();
			break;

		case I8256_STATE_DATA:
			if (m_tx_counter >= m_divide)
			{
				m_tx_counter = 0;

				// SBRK clears after the last data bit, allowing another break to be queued.
				if (m_tx_break && m_tx_bits == m_data_bits)
					m_command3 &= ~(1 << I8256_CMD3_SBRK);

				if (m_tx_bits < m_data_bits)
				{
					const int bit = m_tx_break ? 0 : BIT(m_tx_shift, m_tx_bits);
					m_tx_bits++;
					if (bit)
						m_tx_parity = !m_tx_parity;
					output_txd(bit);
				}
				else if (m_tx_bits == m_data_bits && m_parity_enable)
				{
					m_tx_bits++;
					const bool parity = m_parity_even ? m_tx_parity : !m_tx_parity;
					output_txd(m_tx_break ? 0 : parity);
				}
				else
				{
					m_tx_state = I8256_STATE_STOP;
					output_txd(m_tx_break ? 0 : 1);
				}
			}
			break;

		case I8256_STATE_STOP:
			if (m_tx_counter >= stop_length())
			{
				// P16 senses break-in at the end of the last stop bit, before TBE or TRE.
				check_break_in();

				m_tx_counter = 0;
				m_tx_state = I8256_STATE_START;

				m_status |= I8256_STATUS_TR_EMPTY;
				// TRE generates an interrupt when the transmitter register finished
				// transmitting and the buffer is empty
				if (m_status & I8256_STATUS_TB_EMPTY)
					request_interrupt(I8256_INT_TX);

				start_transmit();
			}
			break;

		case I8256_STATE_MARK:
			if (m_tx_counter >= m_divide)
			{
				m_tx_state = I8256_STATE_START;
				start_transmit();
			}
			break;
	}

	// TxC may run independently of RxC, so latch activity between receiver ticks.
	if (!(m_status & I8256_STATUS_TR_EMPTY) &&
		(m_rx_state == I8256_STATE_DATA || m_rx_state == I8256_STATE_STOP ||
			(m_rx_state == I8256_STATE_START && m_rx_counter)))
		m_rx_tx_active = true;
}

//**************************************************************************
//  SERIAL CLOCKING
//**************************************************************************

TIMER_CALLBACK_MEMBER(i8256_device::brg_tick)
{
	receiver_tick();
	transmitter_tick();
}

TIMER_CALLBACK_MEMBER(i8256_device::test_clock_tick)
{
	m_test_clock = !m_test_clock;
	m_out_p1_cb(0, port1_output());
}

void i8256_device::write_rxd(int state)
{
	m_rxd = state ? 1 : 0;
}

void i8256_device::write_cts(int state)
{
	state = state ? 1 : 0;
	const bool falling = m_cts && !state;
	m_cts = state;
	if (state)
		m_cts_counter = 0;
	else if (falling &&
		!(m_status & I8256_STATUS_TB_EMPTY))
	{
		const uint16_t minimum_stop = (m_stop_sel == I8256_STOP_075) ? stop_length() : m_divide / 2;
		// Pulses during data or before the permitted stop interval are ignored.
		if (m_tx_state == I8256_STATE_START || m_tx_state == I8256_STATE_MARK ||
			(m_tx_state == I8256_STATE_STOP && m_tx_counter >= minimum_stop))
		{
			if (m_stop_sel == I8256_STOP_075 || (m_command2 & 0x0f) == I8256_BAUD_TXC)
			{
				m_cts_pending = true;
				// In 0.75-stop mode, an eligible edge starts the next frame immediately.
				if (m_stop_sel == I8256_STOP_075 && m_tx_state != I8256_STATE_MARK && !m_tx_break &&
					!BIT(m_command3, I8256_CMD3_SBRK) && !BIT(m_command3, I8256_CMD3_TBRK))
				{
					if (m_tx_state == I8256_STATE_STOP)
						check_break_in();
					start_transmit();
				}
			}
			else
				m_cts_counter = m_divide / 32;
		}
	}
}

void i8256_device::write_rxc(int state)
{
	state = state ? 1 : 0;
	if (m_rxc == state)
		return;
	m_rxc = state;

	// RxC is only an input when external 1x clocks are selected;
	// data is clocked into the receiver on the rising edge
	if ((m_command2 & 0x0f) == I8256_BAUD_TXC && state)
		receiver_tick();
}

void i8256_device::write_txc(int state)
{
	state = state ? 1 : 0;
	if (m_txc == state)
		return;
	m_txc = state;

	const uint8_t baud = m_command2 & 0x0f;
	if (baud > I8256_BAUD_TXC32)
		return; // TxC is an output when the internal baud rate generator is used

	if (state)
	{
		// in the /64 and /32 modes TxC is a common clock for receiver and transmitter
		if (baud != I8256_BAUD_TXC)
			receiver_tick();
	}
	else
	{
		// data is clocked out of the transmitter on the falling edge
		transmitter_tick();
	}
}
