// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/*

    Intel 8256(AH) Multifunction microprocessor support controller emulation

*/

#include "emu.h"
#include "i8256.h"

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

const uint8_t timer_interrupt[5] = {I8256_INT_TIMER1, I8256_INT_TIMER2, I8256_INT_TIMER3, I8256_INT_TIMER4, I8256_INT_TIMER5};

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
	I8256_STATE_BREAK
};
} // anonymous namespace

DEFINE_DEVICE_TYPE(I8256, i8256_device, "intel_8256", "Intel 8256AH Multifunction microprocessor support controller")

i8256_device::i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, I8256, tag, owner, clock),
	m_out_int_cb(*this),
	m_txd_handler(*this),
	m_in_p1_cb(*this, 0),
	m_out_p1_cb(*this),
	m_in_p2_cb(*this, 0),
	m_out_p2_cb(*this),
	m_timer(nullptr),
	m_brg_timer(nullptr),
	m_rxd(1),
	m_cts(0),
	m_extint(0),
	m_rxc(0),
	m_txc(0),
	m_int_state(false),
	m_txd(1)
{
}

void i8256_device::device_start()
{
	m_timer = timer_alloc(FUNC(i8256_device::timer_check), this);
	m_brg_timer = timer_alloc(FUNC(i8256_device::brg_tick), this);

	save_item(NAME(m_command1));
	save_item(NAME(m_command2));
	save_item(NAME(m_command3));
	save_item(NAME(m_mode));
	save_item(NAME(m_port1_control));
	save_item(NAME(m_modification));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_int_request));
	save_item(NAME(m_status));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_port1_int));
	save_item(NAME(m_port2_int));
	save_item(NAME(m_timers));

	save_item(NAME(m_data_bits));
	save_item(NAME(m_parity_enable));
	save_item(NAME(m_parity_even));
	save_item(NAME(m_stop_sel));
	save_item(NAME(m_divide));
	save_item(NAME(m_rx_sample));

	save_item(NAME(m_rxd));
	save_item(NAME(m_cts));
	save_item(NAME(m_extint));
	save_item(NAME(m_rxc));
	save_item(NAME(m_txc));
	save_item(NAME(m_int_state));

	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_counter));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_rx_parity));

	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_counter));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_break));
	save_item(NAME(m_txd));
}

void i8256_device::device_reset()
{
	m_command1 = 0;
	m_command2 = 0;
	m_command3 = 0;
	m_mode = 0;
	m_port1_control = 0;
	m_modification = 0;

	m_data_bits = 8;
	m_parity_enable = false;
	m_parity_even = false;
	m_stop_sel = I8256_STOP_1;
	m_divide = 1;
	m_rx_sample = 16; // sample at bit center

	m_rx_buffer = 0;
	m_tx_buffer = 0;
	m_port1_int = 0;
	m_port2_int = 0;

	memset(m_timers, 0, sizeof(m_timers));

	soft_reset();

	m_brg_timer->adjust(attotime::never);
	update_timer_rate();
}

void i8256_device::soft_reset()
{
	m_status = I8256_STATUS_TR_EMPTY | I8256_STATUS_TB_EMPTY;
	m_int_enable = 0;
	m_int_request = 0;
	update_int();

	m_rx_state = I8256_STATE_START;
	m_rx_counter = 0;
	m_rx_bits = 0;
	m_rx_shift = 0;
	m_rx_parity = false;

	m_tx_state = I8256_STATE_START;
	m_tx_counter = 0;
	m_tx_bits = 0;
	m_tx_shift = 0;
	m_tx_parity = false;
	m_tx_break = false;
	output_txd(1);
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

	if (m_timers[i] == 0)
	{
		// the low byte of a cascaded pair wraps and decrements the upper byte
		if ((t24 || t35) && m_timers[high] > 0)
		{
			m_timers[high]--;
			m_timers[i] = 255;
		}
		return;
	}

	m_timers[i]--;
	if (m_timers[i] != 0)
		return;

	if (t24 || t35)
	{
		if (m_timers[high] == 0)
			request_timer_interrupt(t24 ? I8256_INT_TIMER4 : I8256_INT_TIMER3); // levels 6 and 3
	}
	else if (i == 1 && BIT(m_command1, I8256_CMD1_BITI))
	{
		// with BITI set, level 1 belongs to the port 1 P17 interrupt instead of timer 2
	}
	else
	{
		request_timer_interrupt(timer_interrupt[i]);
	}
}

TIMER_CALLBACK_MEMBER(i8256_device::timer_check)
{
	for (int i = 0; i < 5; i++)
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
	const bool state = m_int_request != 0;

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

// acknowledge the highest priority pending interrupt (level 0 is highest)
int i8256_device::acknowledge()
{
	for (int level = 0; level < 8; level++)
	{
		if (BIT(m_int_request, level))
		{
			if (!machine().side_effects_disabled())
			{
				m_int_request &= ~(1 << level);
				update_int();
			}
			return level;
		}
	}
	return -1;
}

uint8_t i8256_device::inta_r()
{
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
	if (state)
		request_interrupt(I8256_INT_EXTINT);
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
		return p1_r();
	case I8256_REG_PORT2:
		return p2_r();
	case I8256_REG_TIMER1:
	case I8256_REG_TIMER2:
	case I8256_REG_TIMER3:
	case I8256_REG_TIMER4:
	case I8256_REG_TIMER5:
		return m_timers[reg - I8256_REG_TIMER1];
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
	default:
		LOG("read unmapped register: %u\n", reg);
		return 0xff;
	}
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
		const bool frq_changed = BIT(m_command1 ^ data, I8256_CMD1_FRQ);
		m_command1 = data;
		if (frq_changed)
			update_timer_rate();

		m_data_bits = 8 - (BIT(data, I8256_CMD1_L0) | (BIT(data, I8256_CMD1_L1) << 1));
		m_stop_sel = BIT(data, I8256_CMD1_S0, 2);

		LOGSETUP("CR1: %d data bits, stop bit select %d, %s mode\n",
				m_data_bits, m_stop_sel, BIT(data, I8256_CMD1_8086) ? "8086" : "8085");
		break;
	}

	case I8256_REG_CMD2:
	{
		const uint8_t changed = m_command2 ^ data;
		m_command2 = data;

		m_parity_enable = BIT(data, I8256_CMD2_PARITY_ENABLE);
		m_parity_even = BIT(data, I8256_CMD2_EVEN_PARITY);

		LOGSETUP("CR2: %s parity, system clock prescaler /%d, baud select %X\n",
				m_parity_enable ? (m_parity_even ? "even" : "odd") : "no",
				SYS_CLOCK_DIVIDER[BIT(data, I8256_CMD2_C0, 2)], data & 0x0f);
		if ((clock() / SYS_CLOCK_DIVIDER[BIT(data, I8256_CMD2_C0, 2)]) != 1'024'000)
			logerror("internal clock should be 1024000, calculated: %u\n", clock() / SYS_CLOCK_DIVIDER[BIT(data, I8256_CMD2_C0, 2)]);

		// the system clock prescaler also feeds the counter/timer
		if (BIT(changed, I8256_CMD2_C0, 2))
			update_timer_rate();

		const uint8_t baud = data & 0x0f;
		if (!(changed & 0x0f))
			break;
		switch (baud)
		{
			case I8256_BAUD_TXC:
				m_divide = 1;
				m_brg_timer->adjust(attotime::never);
				break;
			case I8256_BAUD_TXC64:
				m_divide = 64;
				m_brg_timer->adjust(attotime::never);
				break;
			case I8256_BAUD_TXC32:
				m_divide = 32;
				m_brg_timer->adjust(attotime::never);
				break;
			default:
			{
				// internal baud rate generator; the bit clock is emulated in
				// 1/32 bit steps, the granularity of the modification register
				m_divide = 32;
				const attotime period = attotime::from_hz(BAUD_RATES[baud] * 32);
				m_brg_timer->adjust(period, 0, period);
				break;
			}
		}
		break;
	}

	case I8256_REG_CMD3:
		// bit 7 high sets any bits which are also high, bit 7 low resets them
		if (BIT(data, I8256_CMD3_SET))
			m_command3 |= data & 0x7f;
		else
			m_command3 &= ~(data & 0x7f);

		if (BIT(m_command3, I8256_CMD3_RST))
		{
			soft_reset();
			m_command3 &= ~(1 << I8256_CMD3_RST);
		}

		// nested interrupt mode is not emulated
		m_command3 &= ~(1 << I8256_CMD3_END);
		break;

	case I8256_REG_MODE:
		m_mode = data;
		break;

	case I8256_REG_PORT1C:
		m_port1_control = data;
		break;

	case I8256_REG_INTEN: // set interrupts
		m_int_enable |= data;
		break;

	case I8256_REG_INTAD: // reset interrupts
		m_int_enable &= ~data;
		m_int_request &= ~data;
		update_int();
		break;

	case I8256_REG_BUFFER:
		LOGTX("transmitter buffer %02X\n", data);
		m_tx_buffer = data;
		m_status &= ~I8256_STATUS_TB_EMPTY;
		break;

	case I8256_REG_PORT1:
		p1_w(data);
		break;

	case I8256_REG_PORT2:
		p2_w(data);
		break;

	case I8256_REG_TIMER1:
	case I8256_REG_TIMER2:
	case I8256_REG_TIMER3:
	case I8256_REG_TIMER4:
	case I8256_REG_TIMER5:
		m_timers[reg - I8256_REG_TIMER1] = data;
		break;

	case I8256_REG_STATUS: // modification register
		m_modification = data & 0x7f;
		m_rx_sample = (BIT(data, I8256_MOD_RS4) ? 32 : 16) - BIT(data, I8256_MOD_RS0, 4);
		LOGSETUP("modification: sample point %d/32%s%s\n", m_rx_sample,
				BIT(data, I8256_MOD_DSC) ? ", start bit check disabled" : "",
				BIT(data, I8256_MOD_TME) ? ", transmission mode" : "");
		break;

	default:
		LOG("unmapped write %02X to %02X\n", data, reg);
		break;
	}
}

//**************************************************************************
//  PARALLEL PORTS
//**************************************************************************

uint8_t i8256_device::p1_r()
{
	// control bit 1 = output (read from latch), 0 = input (read from callback)
	return (m_in_p1_cb(0) & ~m_port1_control) | (m_port1_int & m_port1_control);
}

void i8256_device::p1_w(uint8_t data)
{
	// all bits are latched; input pins output the latch if the direction is changed later
	const uint8_t prev = m_port1_int;
	m_port1_int = data;

	// with BITI set, a low-to-high transition on P17 generates the
	// level 1 interrupt in place of timer 2
	if (BIT(m_command1, I8256_CMD1_BITI) && !BIT(prev, 7) && BIT(data, 7))
		request_interrupt(I8256_INT_TIMER2);

	// event counters 2 and 3 decrement on high-to-low transitions of P12 and P13
	if (BIT(m_mode, I8256_MODE_CT2) && BIT(prev, 2) && !BIT(data, 2))
		count_timer(1);
	if (BIT(m_mode, I8256_MODE_CT3) && BIT(prev, 3) && !BIT(data, 3))
		count_timer(2);

	m_out_p1_cb(0, m_port1_int & m_port1_control);
}

uint8_t i8256_device::p2_r()
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
		// TODO: byte handshake input (STB/IBF on port 1)
		return m_in_p2_cb(0);
	case I8256_PORT2C_HO:
		// TODO: byte handshake output (ACK/OBF on port 1)
		return m_port2_int;
	case I8256_PORT2C_TEST:
		// TODO: test mode places the baud rate generator output on port 1 P14
		return 0;
	case I8256_PORT2C_DNU:
	default:
		// "do not use"
		return 0;
	}
}

void i8256_device::p2_w(uint8_t data)
{
	m_port2_int = data;
	switch (m_mode & 0x07)
	{
	case I8256_PORT2C_IO:
		m_out_p2_cb(0, data & 0x0f);
		break;
	case I8256_PORT2C_OI:
		m_out_p2_cb(0, data & 0xf0);
		break;
	case I8256_PORT2C_OO:
		m_out_p2_cb(0, data);
		break;
	default: // TODO: input and handshake modes
		break;
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
	m_rx_counter++;

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
				LOGRX("RX start bit\n");
				m_rx_state = I8256_STATE_DATA;
				m_rx_counter = 0;
				m_rx_shift = 0;
				m_rx_bits = 0;
				m_rx_parity = false;
			}
			else
			{
				LOGRX("RX false start bit\n");
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
				m_rx_shift |= 1 << m_rx_bits;
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
			const uint8_t data = m_rx_shift & ((1 << m_data_bits) - 1);

			if (fe && data == 0)
			{
				// break character: BD is set even when the receiver is disabled,
				// and the character is not transferred to the receiver buffer
				LOGRX("RX break detected\n");
				m_status |= I8256_STATUS_BREAK;
				request_interrupt(I8256_INT_RX);
				m_rx_state = I8256_STATE_BREAK;
			}
			else if (BIT(m_command3, I8256_CMD3_RxE))
			{
				LOGRX("RX character %02X\n", data);

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
					if (!(m_status & I8256_STATUS_TR_EMPTY))
						m_status |= I8256_STATUS_FRAMING_ERROR;
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

void i8256_device::transmitter_tick()
{
	m_tx_counter++;

	switch (m_tx_state)
	{
	case I8256_STATE_START:
		m_tx_counter = 0;

		if (BIT(m_command3, I8256_CMD3_TBRK))
		{
			// continuous break, inhibits buffer transfers until cleared
			output_txd(0);
		}
		else if (BIT(m_command3, I8256_CMD3_SBRK))
		{
			// single character break: start, data, parity and stop bits all low
			LOGTX("TX single character break\n");
			m_tx_break = true;
			m_tx_bits = 0;
			m_status &= ~I8256_STATUS_TR_EMPTY;
			m_tx_state = I8256_STATE_DATA;
			output_txd(0);
		}
		else if (!(m_status & I8256_STATUS_TB_EMPTY) && !m_cts)
		{
			// transfer the buffer to the transmitter register
			m_tx_shift = m_tx_buffer;
			LOGTX("TX character %02X\n", m_tx_shift);
			m_tx_break = false;
			m_tx_bits = 0;
			m_tx_parity = false;
			m_status |= I8256_STATUS_TB_EMPTY;
			m_status &= ~I8256_STATUS_TR_EMPTY;
			request_interrupt(I8256_INT_TX);
			m_tx_state = I8256_STATE_DATA;
			output_txd(0); // start bit
		}
		else
			output_txd(1);
		break;

	case I8256_STATE_DATA:
		if (m_tx_counter >= m_divide)
		{
			m_tx_counter = 0;

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
			m_tx_counter = 0;
			m_tx_state = I8256_STATE_START;

			if (m_tx_break)
			{
				// SBRK is automatically cleared after one character time of break
				m_command3 &= ~(1 << I8256_CMD3_SBRK);
				m_tx_break = false;
				output_txd(1);
			}

			m_status |= I8256_STATUS_TR_EMPTY;
			// TRE generates an interrupt when the transmitter register finished
			// transmitting and the buffer is empty
			if (m_status & I8256_STATUS_TB_EMPTY)
				request_interrupt(I8256_INT_TX);
		}
		break;
	}
}

//**************************************************************************
//  SERIAL CLOCKING
//**************************************************************************

TIMER_CALLBACK_MEMBER(i8256_device::brg_tick)
{
	receiver_tick();
	transmitter_tick();
}

void i8256_device::write_rxd(int state)
{
	m_rxd = state ? 1 : 0;
}

void i8256_device::write_cts(int state)
{
	m_cts = state ? 1 : 0;
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
