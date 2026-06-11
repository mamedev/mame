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
	I8256_CHARLEN_8,
	I8256_CHARLEN_7,
	I8256_CHARLEN_6,
	I8256_CHARLEN_5
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

const char timer_interrupt[5] = {I8256_INT_TIMER1, I8256_INT_TIMER2, I8256_INT_TIMER3, I8256_INT_TIMER4, I8256_INT_TIMER5};

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
	I8256_STATUS_FRAMING_ERROR,
	I8256_STATUS_OVERRUN_ERROR,
	I8256_STATUS_PARITY_ERROR,
	I8256_STATUS_BREAK,
	I8256_STATUS_TR_EMPTY,
	I8256_STATUS_TB_EMPTY,
	I8256_STATUS_RB_FULL,
	I8256_STATUS_INT
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
	m_in_inta_cb(*this, 0),
	m_out_int_cb(*this),
	m_in_extint_cb(*this, 0),
	m_txd_handler(*this),
	m_in_p2_cb(*this, 0),
	m_out_p2_cb(*this),
	m_in_p1_cb(*this, 0),
	m_out_p1_cb(*this),
	m_timer(nullptr),
	m_brg_timer(nullptr),
	m_rxd(1),
	m_cts(1),
	m_rxc(0),
	m_txc(0)
{
}

void i8256_device::device_start()
{
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
	m_timer->adjust(attotime::from_hz(16000), 0, attotime::from_hz(16000));
}

void i8256_device::soft_reset()
{
	m_status = 1 << I8256_STATUS_TR_EMPTY | 1 << I8256_STATUS_TB_EMPTY;
	m_int_enable = 0;
	m_int_request = 0;

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
}

uint8_t i8256_device::read(offs_t offset)
{
	// In the 8-bit mode, AD0-AD3 are used to select the proper register, while AD1-AD4 are used in the 16-bit mode.
	// AD4 in the 8-bit mote is ignored as an address, while AD0 in the 16-bit mode is used as a second chip select, active low.
	if (BIT(m_command1,I8256_CMD1_8086))
		offset = offset >> 1;

	uint8_t reg = offset & 0x0f;

	switch (reg)
	{
		case I8256_REG_CMD1:
			return m_command1;
		case I8256_REG_CMD2:
			return m_command2;
		case I8256_REG_CMD3:
			return m_command3 & 0x76; // When command Register 3 is read, bits 0, 3, and 7 will always be zero.
		case I8256_REG_MODE:
		   return m_mode;
		case I8256_REG_PORT1C:
			return m_port1_control;
		case I8256_REG_BUFFER:
			return m_rx_buffer;
		case I8256_REG_PORT1:
			return m_port1_int;
		case I8256_REG_PORT2:
			return m_port2_int;
		case I8256_REG_TIMER1:
		case I8256_REG_TIMER2:
		case I8256_REG_TIMER3:
		case I8256_REG_TIMER4:
		case I8256_REG_TIMER5:
			return m_timers[reg-10];
		case I8256_REG_STATUS:
			return m_status;
		default:
			LOG("I8256 Read unmapped register: %u\n", reg);
			return 0xff;
	}
}

void i8256_device::write(offs_t offset, uint8_t data)
{
	uint8_t reg = offset & 0x0f;

	// In the 8-bit mode, AD0-AD3 are used to select the proper register, while AD1-AD4 are used in the 16-bit mode.
	// AD4 in the 8-bit mote is ignored as an address.

	if (BIT(m_command1,I8256_CMD1_8086))
	{
		if (!BIT(offset,0)) // AD0 in the 16-bit mode is used as a second chip select, active low.
			reg = (offset >> 1) & 0x0f;
		else
			return;
	}

	switch (reg)
	{
		case I8256_REG_CMD1:
			if (m_command1 != data)
			{
				m_command1 = data;

				if (BIT(m_command1,I8256_CMD1_FRQ))
					m_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));
				else
					m_timer->adjust(attotime::from_hz(16000), 0, attotime::from_hz(16000));

				if (BIT(m_command1,I8256_CMD1_8086))
					LOG("I8256 Enabled 8086 mode\n");
			}
			break;
		case I8256_REG_CMD2:
			if (m_command2 != data)
			{
				m_command2 = data;

				LOG("I8256 Clock Scale: %u\n", SYS_CLOCK_DIVIDER[(m_command2 & 0x30 >> 4)]);
				if ((clock() / SYS_CLOCK_DIVIDER[(m_command2 & 0x30 >> 4)]) != 1024000)
					logerror("I8256 Internal Clock should be 1024000, calculated: %u\n", (clock() / SYS_CLOCK_DIVIDER[(m_command2 & 0x30 >> 4)]));
			}
			break;
		case I8256_REG_CMD3:
			m_command3 = data;
			if (BIT(m_command3,I8256_CMD3_RST))
			{
				m_status = 0x30;
			}
			break;
		case I8256_REG_MODE:
			m_mode = data;
			break;
		case I8256_REG_PORT1C:
			m_port1_control = data;
			break;
		case I8256_REG_BUFFER:
			LOGTX("I8256 write serial: %u\n", data);
			m_tx_buffer = data;
			break;
		case I8256_REG_PORT1:
			m_port1_int = data;
			break;
		case I8256_REG_PORT2:
			m_port2_int = data;
			break;
		case I8256_REG_TIMER1:
		case I8256_REG_TIMER2:
		case I8256_REG_TIMER3:
		case I8256_REG_TIMER4:
		case I8256_REG_TIMER5:
			m_timers[reg-10] = data;
			break;
		case I8256_REG_STATUS:
			m_modification = data;
			break;
		default:
			LOG("I8256 Unmapped write %02x to %02x\n", data, reg);
			break;
	}
}

uint8_t i8256_device::p1_r()
{
	// if control bit is 0 (input), read from callback else use output latch
	uint8_t input = m_in_p1_cb(0);
	uint8_t result = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_port1_control, i)) // output
			result |= (m_port1_int & (1 << i));
		else // input
			result |= (input & (1 << i));
	}
	return result;
}

void i8256_device::p1_w(uint8_t data)
{
	m_port1_int = (m_port1_int & ~m_port1_control) | (data & m_port1_control);
	m_out_p1_cb(0, m_port1_int & m_port1_control);
}

uint8_t i8256_device::p2_r()
{
	uint8_t p2c = m_mode & 0x03;
	if (p2c == I8256_PORT2C_II || p2c == I8256_PORT2C_IO)
		return m_in_p2_cb(0);
	else
		return m_port2_int;
}

void i8256_device::p2_w(uint8_t data)
{
	uint8_t p2c = m_mode & 0x03;
	m_port2_int = data;
	uint8_t port2_data = 0;
	switch (p2c)
	{
		case I8256_PORT2C_IO: port2_data = m_port2_int & 0x0f; break;
		case I8256_PORT2C_OI: port2_data = m_port2_int & 0xf0; break;
		case I8256_PORT2C_OO: port2_data = m_port2_int; break;
		default: port2_data = 0; break;
	}
	if (p2c == I8256_PORT2C_IO || p2c == I8256_PORT2C_OI || p2c == I8256_PORT2C_OO)
		m_out_p2_cb(0, port2_data);
}


void i8256_device::write_rxd(int state)
{
	m_rxd = state ? 1 : 0;
	LOGRX("I8256: Presented a %d\n", m_rxd);
}

void i8256_device::write_cts(int state)
{
	m_cts = state ? 1 : 0;
}

void i8256_device::write_rxc(int state)
{
	m_rxc = state ? 1 : 0;
}

void i8256_device::write_txc(int state)
{
	m_txc = state ? 1 : 0;
}
