// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/*

    Intel 8256(AH) Multifunction microprocessor support controller emulation

*/

#include "emu.h"
#include "i8256.h"

//#define VERBOSE 1
#include "logmacro.h"


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

constexpr device_serial_interface::stop_bits_t STOP_BITS[4] =
{
	device_serial_interface::STOP_BITS_1,
	device_serial_interface::STOP_BITS_1_5,
	device_serial_interface::STOP_BITS_2,
	device_serial_interface::STOP_BITS_0
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

constexpr int BAUD_RATES[16] = { 0, 0, 0, 19200, 9600, 4800, 2400, 1200, 600, 300, 200, 150, 110, 100, 75, 50 };

enum
{
	I8256_SCLK_DIV5, // 5.12 MHz
	I8256_SCLK_DIV3, // 3.072 MHz
	I8256_SCLK_DIV2, // 2.048 MHz
	I8256_SCLK_DIV1  // 1.024 MHz
};

constexpr int SYS_CLOCK_DIVIDER[4] = {5,3,2,1};

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
} // anonymous namespace

DEFINE_DEVICE_TYPE(I8256, i8256_device, "intel_8256", "Intel 8256AH Multifunction microprocessor support controller")

i8256_device::i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, I8256, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_in_inta_cb(*this, 0),
	m_out_int_cb(*this),
	m_in_extint_cb(*this, 0),
	m_txd_handler(*this),
	m_in_p2_cb(*this, 0),
	m_out_p2_cb(*this),
	m_in_p1_cb(*this, 0),
	m_out_p1_cb(*this),
	m_timer(nullptr)
{
}

void i8256_device::device_start()
{
	// internal register
	save_item(NAME(m_command1));
	save_item(NAME(m_command2));
	save_item(NAME(m_command3));
	save_item(NAME(m_mode));
	save_item(NAME(m_port1_control));
	save_item(NAME(m_interrupts));
	save_item(NAME(m_current_interrupt_level));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_port1_int));
	save_item(NAME(m_port2_int));
	save_item(NAME(m_timers));
	save_item(NAME(m_status));
	save_item(NAME(m_modification));

	save_item(NAME(m_parity));
	save_item(NAME(m_stop_bits));
	save_item(NAME(m_br_factor));
	
	m_timer = timer_alloc(FUNC(i8256_device::timer_check), this);

	memset(m_timers, 0, sizeof(m_timers));
}

void i8256_device::device_reset()
{
	m_command1 = 0;
	m_command2 = 0;
	m_command3 = 0;
	m_mode = 0;
	m_port1_control = 0;
	m_interrupts = 0;
	m_current_interrupt_level = 0;
	m_out_int_cb(CLEAR_LINE);

	m_status = 0x30; // TRE and TBE

	reset_timer();
}

uint8_t i8256_device::acknowledge()
{
	LOG("i8256_device::acknowledge %d\n", m_current_interrupt_level);
	if (BIT(m_command3,I8256_CMD3_IAE) == 0)
		return 0x00;

	const uint8_t vector = m_current_interrupt_level;

	if (!machine().side_effects_disabled())
	{
		m_out_int_cb(CLEAR_LINE);
		m_status &= ~(1 << I8256_STATUS_INT);
		m_current_interrupt_level = 0;
	}

	if (BIT(m_command1,I8256_CMD1_8086)) // 8086 mode, TODO: only on second INTA
		return 0x40 | vector;
	else
		return 0xc7 | (vector << 3); // 8085 mode c7 = rst 0, ff = rst 7
}

void i8256_device::reset_timer()
{
	int divider = 64; //default is 16kHz from the datasheet, it may later be changed to a slower one
	if (BIT(m_command1, I8256_CMD1_FRQ))
	{
		divider = 1024;
	}
	const attotime TIME = attotime::from_hz((clock() / SYS_CLOCK_DIVIDER[(m_command2 & 0x30) >> 4]) / divider);
	m_timer->adjust(TIME, 0, TIME);
}

void i8256_device::gen_interrupt(uint8_t level)
{
	LOG("i8256_device::gen_interrupt %d\n", level);
	if (BIT(m_interrupts, level))
	{
		m_current_interrupt_level = level;
		m_out_int_cb(ASSERT_LINE);
		m_status |= (1 << I8256_STATUS_INT);
	}
}

TIMER_CALLBACK_MEMBER(i8256_device::timer_check)
{
	bool t24 = BIT(m_mode, I8256_MODE_T24);
	bool t35 = BIT(m_mode, I8256_MODE_T35);
	for (int i = 0; i < 5; ++i)
	{
		if ((i == 1 && t24) || (i == 2 && t35)) {
			// cascaded low timer
			int high_index = (i == 1) ? 3 : 4;
			int int_level = (i == 1) ? I8256_INT_TIMER4 : I8256_INT_TIMER5;
			if (m_timers[i] > 0) {
				m_timers[i]--;
				if (m_timers[i] == 0) {
					if (m_timers[high_index] > 0) {
						m_timers[high_index]--;
						m_timers[i] = 255;
					}
					if (m_timers[high_index] == 0 ) 
						gen_interrupt(int_level);
				}
			}
		} else if (!((i == 3 && t24) || (i == 4 && t35))) {
			// normal timer, not cascaded high
			if (m_timers[i] > 0) {
				m_timers[i]--;
				if (m_timers[i] == 0 && BIT(m_interrupts, timer_interrupt[i])) {
					// For Timer2, only trigger if BITI=0
					if (i == I8256_INT_TIMER2 && BIT(m_command1, I8256_CMD1_BITI))
						continue;

					gen_interrupt(timer_interrupt[i]);
				}
			}
		}
	}
}

void i8256_device::update_status()
{
	// TR Empty
	if (is_transmit_register_empty())
		m_status |= (1 << I8256_STATUS_TR_EMPTY);
	else
		m_status &= ~(1 << I8256_STATUS_TR_EMPTY);

	if (is_receive_framing_error())
		m_status |= (1 << I8256_STATUS_FRAMING_ERROR);
	else
		m_status &= ~(1 << I8256_STATUS_FRAMING_ERROR);

	if (is_receive_parity_error())
		m_status |= (1 << I8256_STATUS_PARITY_ERROR);
	else
		m_status &= ~(1 << I8256_STATUS_PARITY_ERROR);
}

uint8_t i8256_device::read(offs_t offset)
{
	// In the 8-bit mode, AD0-AD3 are used to select the proper register, while AD1-AD4 are used in the 16-bit mode.
	// AD4 in the 8-bit mote is ignored as an address, while AD0 in the 16-bit mode is used as a second chip select, active low.
	if (BIT(m_command1,I8256_CMD1_8086))
		offset = offset >> 1;

	u8 reg = offset & 0x0f;

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
		case I8256_REG_INTEN:
			return m_interrupts;
		case I8256_REG_INTAD:
			if (!machine().side_effects_disabled())
			{
				m_out_int_cb(CLEAR_LINE);
				m_status &= ~(1 << I8256_STATUS_INT);
			}
			return m_current_interrupt_level*4;
		case I8256_REG_BUFFER:
			if (!machine().side_effects_disabled())
				m_status &= ~(1 << I8256_STATUS_RB_FULL);
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

void i8256_device::write(offs_t offset, u8 data)
{
	u8 reg = offset & 0x0f;

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

				reset_timer();

				if (BIT(m_command1,I8256_CMD1_8086))
					LOG("I8256 Enabled 8086 mode\n");

				m_data_bits_count = 8 - (BIT(m_command1, I8256_CMD1_L0) | (BIT(m_command1, I8256_CMD1_L1) << 1));
				m_stop_bits = BIT(m_command1, I8256_CMD1_S0) | (BIT(m_command1, I8256_CMD1_S1) << 1);

				set_data_frame(1, m_data_bits_count, (parity_t)m_parity, STOP_BITS[m_stop_bits]);
			}
			break;
		case I8256_REG_CMD2:
			if (m_command2 != data)
			{
				m_command2 = data;

				uint8_t baud_sel = m_command2 & 0x0f;

				set_rate(BAUD_RATES[baud_sel]);

				if (baud_sel == 1) {
					m_br_factor = 32;
				} else if (baud_sel == 2) {
					m_br_factor = 64;
				} else if (baud_sel == 3) {
					m_br_factor = 32;
				} else if (baud_sel > 3) {
					m_br_factor = 64;
				} else {
					m_br_factor = 1;
				}

				if (BIT(m_command2,I8256_CMD2_PARITY_ENABLE))
					m_parity = BIT(m_command2,I8256_CMD2_EVEN_PARITY) ? PARITY_EVEN : PARITY_ODD;
				else
					m_parity = PARITY_NONE;

				set_data_frame(1, m_data_bits_count, (parity_t)m_parity, STOP_BITS[m_stop_bits]);

				LOG("I8256 Clock Scale: %u\n", SYS_CLOCK_DIVIDER[(m_command2 & 0x30) >> 4]);
				if ((clock() / SYS_CLOCK_DIVIDER[(m_command2 & 0x30) >> 4]) != 1024000)
					logerror("I8256 Internal Clock should be 1024000, calculated: %u\n", (clock() / SYS_CLOCK_DIVIDER[(m_command2 & 0x30) >> 4]));
			}
			break;
		case I8256_REG_CMD3:
			m_command3 = data;
			if (BIT(m_command3,I8256_CMD3_RST))
			{
				m_interrupts = 0;
				m_status = 0x30;
			}
			break;
		case I8256_REG_MODE:
			m_mode = data;
			break;
		case I8256_REG_PORT1C:
			m_port1_control = data;
			break;
		case I8256_REG_INTEN:
			m_interrupts = m_interrupts | data;
			break;
		case I8256_REG_INTAD: // reset interrupt
			m_interrupts = m_interrupts & ~data;
			break;
		case I8256_REG_BUFFER:
			m_tx_buffer = data;
			transmit_register_setup(m_tx_buffer);
			m_status &= ~ (1 << I8256_STATUS_TB_EMPTY);
			update_status();
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
			m_timers[reg-10] = data;
			break;
		case I8256_REG_TIMER4:
			if (BIT(m_mode, I8256_MODE_T24)) m_timers[1] = 255;
			m_timers[reg-10] = data;
			break;
		case I8256_REG_TIMER5:
			if (BIT(m_mode, I8256_MODE_T35)) m_timers[2] = 255;
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
	uint8_t input = m_in_p1_cb(0);
	// For output bits: use latched value, for input bits: use current input
	return (m_port1_int & m_port1_control) | (input & ~m_port1_control);
}

void i8256_device::p1_w(uint8_t data)
{
	// Check for P17 interrupt if BITI=1
	if (BIT(m_command1, I8256_CMD1_BITI) && !BIT(m_port1_int, 7) && BIT(data, 7))
	{
		gen_interrupt(I8256_INT_TIMER2);
	}

	m_port1_int = (m_port1_int & ~m_port1_control) | (data & m_port1_control);
	m_out_p1_cb(0, m_port1_int & m_port1_control);
}

uint8_t i8256_device::p2_r()
{
	uint8_t p2c = m_mode & 0x07;
	uint8_t result = 0;
	
	switch (p2c)
	{
		case I8256_PORT2C_IO:
			result = m_in_p2_cb(0) & 0x0f;
			result |= m_port2_int & 0xf0;
			break;
		case I8256_PORT2C_OI:
			result = m_port2_int & 0x0f;
			result |= m_in_p2_cb(0) & 0xf0;
			break;
		case I8256_PORT2C_OO:
			result = m_port2_int;
			break;
		case I8256_PORT2C_HI:
			// TODO Handshake
			result = m_in_p2_cb(0);
			break;
		case I8256_PORT2C_HO:
			// TODO Handshake
			result = m_port2_int;
			break;
		case I8256_PORT2C_DNU:
			// Do not use
			result = 0;
			break;
		case I8256_PORT2C_TEST:
			// TODO Test mode
			result = 0;
			break;
		case I8256_PORT2C_II:
		default:
			result = m_in_p2_cb(0);
			break;
	}
	return result;
}

void i8256_device::p2_w(uint8_t data)
{
	uint8_t p2c = m_mode & 0x07;
	m_port2_int = data;
	uint8_t port2_data = 0;
	switch (p2c)
	{
		case I8256_PORT2C_IO: port2_data = m_port2_int & 0x0f; break;
		case I8256_PORT2C_OI: port2_data = m_port2_int & 0xf0; break;
		case I8256_PORT2C_OO: port2_data = m_port2_int; break;
		case I8256_PORT2C_HI: port2_data = 0; break;
		case I8256_PORT2C_HO: port2_data = m_port2_int; break;
		case I8256_PORT2C_DNU: port2_data = 0; break;
		case I8256_PORT2C_TEST: port2_data = 0; break;
		default: port2_data = 0; break;
	}
	if (p2c == I8256_PORT2C_IO || p2c == I8256_PORT2C_OI || p2c == I8256_PORT2C_OO || p2c == I8256_PORT2C_HO)
		m_out_p2_cb(0, port2_data);
}


/*-------------------------------------------------
    serial interface
-------------------------------------------------*/

void i8256_device::rcv_complete()
{
	receive_register_extract();
	m_rx_buffer = get_received_char();
	m_status |= (1 << I8256_STATUS_RB_FULL);
	gen_interrupt(I8256_INT_RX);
	update_status();
}

void i8256_device::tra_callback()
{
	m_txd_handler(transmit_register_get_data_bit());
}

void i8256_device::tra_complete()
{
	m_status |= (1 << I8256_STATUS_TB_EMPTY);
	gen_interrupt(I8256_INT_TX);
	update_status();
}

void i8256_device::write_rxd(int state)
{
	device_serial_interface::rx_w(state);
}

void i8256_device::write_txc(int state)
{
	device_serial_interface::tx_clock_w(state);
}

void i8256_device::write_rxc(int state)
{
	device_serial_interface::rx_clock_w(state);
}
