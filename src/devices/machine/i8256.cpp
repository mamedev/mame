// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/*

    Intel 8256(AH) Multifunction microprocessor support controller emulation

*/

#include "emu.h"
#include "i8256.h"

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(I8256, i8256_device, "intel_8256", "Intel 8256AH Multifunction microprocessor support controller")

i8256_device::i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, I8256, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_in_inta_cb(*this, 0),
	m_out_int_cb(*this),
	m_in_extint_cb(*this, 0),
	m_rxc(0),
	m_rxd(1),
	m_cts(1),
	m_txc(0),
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

	m_timer = timer_alloc(FUNC(i8256_device::timer_check), this);
}

void i8256_device::device_reset()
{
	m_command1 = 0;
	m_command2 = 0;
	m_command3 = 0;
	m_mode = 0;
	m_port1_control = 0;
	m_interrupts = 0;

	m_tx_buffer = 0;
	m_rx_buffer = 0;
	m_port1_int = 0;
	m_port2_int = 0;
	memset(m_timers, 0, sizeof(m_timers));

	m_status = 0x30; // TRE and TBE

	m_timer->adjust(attotime::from_hz(16000), 0, attotime::from_hz(16000));
}

TIMER_CALLBACK_MEMBER(i8256_device::timer_check)
{
	for (int i = 0; i < 5; ++i)
	{
		if (m_timers[i] > 0)
		{
			m_timers[i]--;
			if (m_timers[i] == 0 && BIT(m_interrupts,timer_interrupt[i])) // If the interrupt is enabled
			{
				m_current_interrupt_level = timer_interrupt[i];
				m_out_int_cb(1); // it occurs when the counter changes from 1 to 0.
			}
		}
	}
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
			m_out_int_cb(0);
			return m_current_interrupt_level*4;
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

				if (BIT(m_command1,I8256_CMD1_FRQ))
					m_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));
				else
					m_timer->adjust(attotime::from_hz(16000), 0, attotime::from_hz(16000));

				if (BIT(m_command1,I8256_CMD1_8086))
					LOG("I8256 Enabled 8086 mode\n");

				m_data_bits_count = 8-((BIT(m_command1, I8256_CMD1_L0)) | (BIT(m_command1, I8256_CMD1_L1) << 1));
				m_stop_bits = stopBits[(BIT(m_command1, I8256_CMD1_S0)) | (BIT(m_command1, I8256_CMD1_S1) << 1)];

				set_data_frame(1, m_data_bits_count, m_parity, m_stop_bits);
			}
			break;
		case I8256_REG_CMD2:
			if (m_command2 != data)
			{
				m_command2 = data;

				set_rate(baudRates[m_command2 & 0x0f]);

				if (BIT(m_command2,I8256_CMD2_PARITY_ENABLE))
					m_parity = BIT(m_command2,I8256_CMD2_EVEN_PARITY) ? PARITY_EVEN : PARITY_ODD;
				else
					m_parity = PARITY_NONE;

				set_data_frame(1, m_data_bits_count, m_parity, m_stop_bits);

				LOG("I8256 Clock Scale: %u\n", sysclockDivider[(m_command2 & 0x30 >> 4)]);
				if((clock() / sysclockDivider[(m_command2 & 0x30 >> 4)]) != 1024000)
					logerror("I8256 Internal Clock should be 1024000, calculated: %u\n", (clock() / sysclockDivider[(m_command2 & 0x30 >> 4)]));
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
			LOG("I8256 write serial: %u\n", data);
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


/*-------------------------------------------------
    receive_clock
-------------------------------------------------*/

void i8256_device::receive_clock()
{
	// receive enable?
	if (BIT(m_command3, I8256_CMD3_RxE))
	{
		const bool sync = is_receive_register_synchronized();
		if (sync)
		{
			--m_rxc_count;
			if (m_rxc_count)
				return;
		}

		//logerror("i8256\n");
		// get bit received from other side and update receive register
		//LOGBITS("8256: Rx Sampled %d\n", m_rxd);
		receive_register_update_bit(m_rxd);
		if (is_receive_register_synchronized())
			m_rxc_count = sync ? m_br_factor : (3 * m_br_factor / 2);

		if (is_receive_register_full())
		{
			receive_register_extract();
			if (is_receive_parity_error())
				m_status |= I8256_STATUS_PARITY_ERROR;
			if (is_receive_framing_error())
				m_status |= I8256_STATUS_FRAMING_ERROR;
			receive_character(get_received_char());
		}
	}
}

void i8256_device::sync1_rxc()
{
	// is rx enabled?
	if (!BIT(m_command3, I8256_CMD3_RxE))
		return;

	u8 need_parity = BIT(m_command2, I8256_CMD2_PARITY_ENABLE);

	// see about parity
	if (need_parity && (m_rxd_bits == m_data_bits_count))
	{
		if ((population_count_32(m_sync1) & 1) != m_rxd)
			m_status |= I8256_STATUS_PARITY_ERROR;
		// and then continue on as if everything was ok
	}
	else
	{
		// add bit to byte
		m_sync1 = (m_sync1 >> 1) | (m_rxd << (m_data_bits_count-1));
	}

	// is byte complete? if not, quit
	m_rxd_bits++;
	if (m_rxd_bits < (m_data_bits_count + need_parity))
		return;

	// now we have a synchronised byte, and parity has been dealt with

	// copy byte to rx buffer
	receive_character(m_sync1);

	m_rxd_bits = 0;
	m_sync1 = 0;
}

void i8256_device::sync2_rxc()
{
	// is rx enabled?
	if (!BIT(m_command3, I8256_CMD3_RxE))
		return;

	u8 need_parity = BIT(m_command2, I8256_CMD2_PARITY_ENABLE);

	// see about parity
	if (need_parity && (m_rxd_bits == m_data_bits_count))
	{
		if ((population_count_32(m_sync1) & 1) != m_rxd)
			m_status |= I8256_STATUS_PARITY_ERROR;
		// and then continue on as if everything was ok
	}
	else
	{
		// add bit to byte
		m_sync1 = (m_sync1 >> 1) | (m_rxd << (m_data_bits_count-1));
		m_sync2 = (m_sync2 >> 1) | (m_rxd << (m_data_bits_count*2-1));
	}

	// is byte complete? if not, quit
	m_rxd_bits++;
	if (m_rxd_bits < (m_data_bits_count + need_parity))
		return;

	// now we have a synchronised byte, and parity has been dealt with

	// copy byte to rx buffer
	receive_character(m_sync1);

	m_rxd_bits = 0;
	m_sync1 = 0;
	m_sync2 = 0;
}


/*-------------------------------------------------
    check_for_tx_start
-------------------------------------------------*/

void i8256_device::check_for_tx_start()
{
	if (!BIT(m_status,I8256_STATUS_TR_EMPTY))
		start_tx();
}


/*-------------------------------------------------
    start_tx
-------------------------------------------------*/

void i8256_device::start_tx()
{
	LOG("start_tx %02x\n", m_tx_data);
	transmit_register_setup(m_tx_data);
	m_status &= ~I8256_STATUS_TR_EMPTY;
}


/*-------------------------------------------------
    transmit_clock
-------------------------------------------------*/

void i8256_device::transmit_clock()
{
	m_txc_count++;
	if (m_txc_count != m_br_factor)
		return;

	m_txc_count = 0;

	if (is_transmit_register_empty())
		start_tx();

	// if diserial has bits to send, make them so
	if (!is_transmit_register_empty())
	{
		uint8_t data = transmit_register_get_data_bit();
		LOG("I8256: Tx Present a %d\n", data);
		m_txd_handler(data);
	}
}

void i8256_device::receive_character(uint8_t ch)
{
	LOG("I8256: receive_character %02x\n", ch);

	m_rx_data = ch;

	LOG("status RX READY test %02x\n", m_status);
	// char has not been read and another has arrived!
	if (BIT(m_status, I8256_STATUS_RB_FULL))
	{
		m_status |= I8256_STATUS_OVERRUN_ERROR;
		LOG("status overrun set\n");
	}
}

void i8256_device::write_rxd(int state)
{
	m_rxd = state ? 1 : 0;
	LOG("I8256: Presented a %d\n", m_rxd);
	//device_serial_interface::rx_w(m_rxd);
}

void i8256_device::write_cts(int state)
{
	m_cts = state ? 1 : 0;

	if (started())
		check_for_tx_start();
}

void i8256_device::write_rxc(int state)
{
	state = state ? 1 : 0;

	if (!m_rxc && state)
	{
		if (m_sync_byte_count == 1)
			sync1_rxc();
		else if (m_sync_byte_count == 2)
			sync2_rxc();
		else
			receive_clock();
	}

	m_rxc = state;
}

void i8256_device::write_txc(int state)
{
	state = state ? 1 : 0;

	if (m_txc != state)
	{
		m_txc = state;

		if (!m_txc)
			transmit_clock();
	}
}
