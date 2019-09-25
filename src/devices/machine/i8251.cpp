// license:BSD-3-Clause
// copyright-holders:smf
/*********************************************************************

    i8251.c

    Intel 8251 Universal Synchronous/Asynchronous Receiver Transmitter code
    NEC uPD71051 is a clone

    The V53/V53A use a customized version with only the Asynchronous mode
    and a split command / mode register



*********************************************************************/

#include "emu.h"
#include "i8251.h"

#define LOG_STAT    (1U << 1)
#define LOG_COM     (1U << 2)
#define LOG_MODE    (1U << 3)
#define LOG_BITS    (1U << 4)

//#define VERBOSE (LOG_BITS|LOG_GENERAL)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSTAT(...)  LOGMASKED(LOG_STAT,  __VA_ARGS__)
#define LOGCOM(...)   LOGMASKED(LOG_COM,   __VA_ARGS__)
#define LOGMODE(...)  LOGMASKED(LOG_MODE,  __VA_ARGS__)
#define LOGBITS(...)  LOGMASKED(LOG_BITS,  __VA_ARGS__)

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(I8251,   i8251_device,  "i8251",    "Intel 8251 USART")
DEFINE_DEVICE_TYPE(V5X_SCU, v5x_scu_device, "v5x_scu", "NEC V5X SCU")


//-------------------------------------------------
//  i8251_device - constructor
//-------------------------------------------------

i8251_device::i8251_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_txd_handler(*this),
	m_dtr_handler(*this),
	m_rts_handler(*this),
	m_rxrdy_handler(*this),
	m_txrdy_handler(*this),
	m_txempty_handler(*this),
	m_syndet_handler(*this),
	m_cts(1),
	m_dsr(1),
	m_rxd(0),
	m_rxc(0),
	m_txc(0)
{
}

i8251_device::i8251_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8251_device(mconfig, I8251, tag, owner, clock)
{
}

v5x_scu_device::v5x_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8251_device(mconfig, V5X_SCU, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8251_device::device_start()
{
	// resolve callbacks
	m_txd_handler.resolve_safe();
	m_rts_handler.resolve_safe();
	m_dtr_handler.resolve_safe();
	m_rxrdy_handler.resolve_safe();
	m_txrdy_handler.resolve_safe();
	m_txempty_handler.resolve_safe();
	save_item(NAME(m_flags));
	save_item(NAME(m_sync_byte_offset));
	save_item(NAME(m_sync_byte_count));
	save_item(NAME(m_sync_bytes));
	save_item(NAME(m_status));
	save_item(NAME(m_command));
	save_item(NAME(m_mode_byte));
	save_item(NAME(m_cts));
	save_item(NAME(m_dsr));
	save_item(NAME(m_rxd));
	save_item(NAME(m_rxc));
	save_item(NAME(m_txc));
	save_item(NAME(m_rxc_count));
	save_item(NAME(m_txc_count));
	save_item(NAME(m_br_factor));
	save_item(NAME(m_rx_data));
	save_item(NAME(m_tx_data));
}




/*-------------------------------------------------
    update_rx_ready
-------------------------------------------------*/

void i8251_device::update_rx_ready()
{
	int state = m_status & I8251_STATUS_RX_READY;

	// masked?
	if (!BIT(m_command, 2))
		state = 0;

	m_rxrdy_handler(state != 0);
}



/*-------------------------------------------------
    receive_clock
-------------------------------------------------*/

void i8251_device::receive_clock()
{
	/* receive enable? */
	if (BIT(m_command, 2))
	{
		const bool sync = is_receive_register_synchronized();
		if (sync)
		{
			--m_rxc_count;
			if (m_rxc_count)
				return;
		}

		//logerror("I8251\n");
		/* get bit received from other side and update receive register */
		//LOGBITS("8251: Rx Sampled %d\n", m_rxd);
		receive_register_update_bit(m_rxd);
		if (is_receive_register_synchronized())
			m_rxc_count = sync ? m_br_factor : (3 * m_br_factor / 2);

		if (is_receive_register_full())
		{
			receive_register_extract();
			if (is_receive_parity_error())
				m_status |= I8251_STATUS_PARITY_ERROR;
			if (is_receive_framing_error())
				m_status |= I8251_STATUS_FRAMING_ERROR;
			receive_character(get_received_char());
		}
	}
}

/*-------------------------------------------------
    is_tx_enabled
-------------------------------------------------*/
bool i8251_device::is_tx_enabled() const
{
	return BIT(m_command , 0) && !m_cts;
}

/*-------------------------------------------------
    check_for_tx_start
-------------------------------------------------*/
void i8251_device::check_for_tx_start()
{
	if (is_tx_enabled() && (m_status & (I8251_STATUS_TX_EMPTY | I8251_STATUS_TX_READY)) == I8251_STATUS_TX_EMPTY)
	{
		start_tx();
	}
}

/*-------------------------------------------------
    start_tx
-------------------------------------------------*/
void i8251_device::start_tx()
{
	LOG("start_tx %02x\n", m_tx_data);
	transmit_register_setup(m_tx_data);
	m_status &= ~I8251_STATUS_TX_EMPTY;
	m_status |= I8251_STATUS_TX_READY;
}

/*-------------------------------------------------
    transmit_clock
-------------------------------------------------*/

void i8251_device::transmit_clock()
{
	m_txc_count++;
	if (m_txc_count != m_br_factor)
		return;

	m_txc_count = 0;

	if (is_transmit_register_empty())
	{
		if ((m_status & I8251_STATUS_TX_READY) == 0 && (is_tx_enabled() || (m_flags & I8251_DELAYED_TX_EN) != 0))
			start_tx();
		else
			m_status |= I8251_STATUS_TX_EMPTY;

		update_tx_ready();
		update_tx_empty();
	}

	/* if diserial has bits to send, make them so */
	if (!is_transmit_register_empty())
	{
		uint8_t data = transmit_register_get_data_bit();
		LOGBITS("8251: Tx Present a %d\n", data);
		m_txd_handler(data);
	}

#if 0
	/* hunt mode? */
	/* after each bit has been shifted in, it is compared against the current sync byte */
	if (BIT(m_command, 7))
	{
		/* data matches sync byte? */
		if (m_data == m_sync_bytes[m_sync_byte_offset])
		{
			/* sync byte matches */
			/* update for next sync byte? */
			m_sync_byte_offset++;

			/* do all sync bytes match? */
			if (m_sync_byte_offset == m_sync_byte_count)
			{
				/* ent hunt mode */
				m_command &= ~(1<<7);
			}
		}
		else
		{
			/* if there is no match, reset */
			m_sync_byte_offset = 0;
		}
	}
#endif
}



/*-------------------------------------------------
    update_tx_ready
-------------------------------------------------*/

void i8251_device::update_tx_ready()
{
	/* clear tx ready state */
	int tx_ready;

	/* tx ready output is set if:
	    DB Buffer Empty &
	    CTS is set &
	    Transmit enable is 1
	*/

	tx_ready = is_tx_enabled() && (m_status & I8251_STATUS_TX_READY) != 0;

	m_txrdy_handler(tx_ready);
}



/*-------------------------------------------------
    update_tx_empty
-------------------------------------------------*/

void i8251_device::update_tx_empty()
{
	if (m_status & I8251_STATUS_TX_EMPTY)
	{
		// return TxD to marking state (high) if not sending break character
		m_txd_handler(!BIT(m_command, 3));
	}

	m_txempty_handler((m_status & I8251_STATUS_TX_EMPTY) != 0);
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8251_device::device_reset()
{
	LOG("I8251: Reset\n");

	/* what is the default setup when the 8251 has been reset??? */

	/* i8251 datasheet explains the state of tx pin at reset */
	/* tx is set to 1 */
	m_txd_handler(1);

	/* assumption */
	m_rts_handler(1);
	m_dtr_handler(1);

	transmit_register_reset();
	receive_register_reset();
	m_flags = 0;
	/* expecting mode byte */
	m_flags |= I8251_EXPECTING_MODE;
	/* not expecting a sync byte */
	m_flags &= ~I8251_EXPECTING_SYNC_BYTE;

	/* no character to read by cpu */
	/* transmitter is ready and is empty */
	m_status = I8251_STATUS_TX_EMPTY | I8251_STATUS_TX_READY;
	LOGSTAT("status is reset to %02x\n", m_status);
	m_mode_byte = 0;
	m_command = 0;
	m_rx_data = 0;
	m_tx_data = 0;
	m_rxc_count = m_txc_count = 0;
	m_br_factor = 1;

	/* update tx empty pin output */
	update_tx_empty();
	/* update rx ready pin output */
	update_rx_ready();
	/* update tx ready pin output */
	update_tx_ready();
}



/*-------------------------------------------------
    control_w
-------------------------------------------------*/

void i8251_device::command_w(uint8_t data)
{
	/* command */
	m_command = data;

	LOG("I8251: Command byte: %02x\n", data);
	LOGCOM(" Tx enable: %d\n", data & 0x01 ? 1 : 0); // bit 0: 0 = transmit disable 1 = transmit enable
	LOGCOM(" DTR      : %d\n", data & 0x02 ? 1 : 0); // bit 1: 0 = /DTR set to 1    1 = /DTR set to 0
	LOGCOM(" Rx enable: %d\n", data & 0x04 ? 1 : 0); // bit 2: 0 = receive disable  1 = receive enable
	LOGCOM(" Send BRK : %d\n", data & 0x08 ? 1 : 0); // bit 3: 0 = normal operation 1 = send break character
	LOGCOM(" Err reset: %d\n", data & 0x10 ? 1 : 0); // bit 4: 0 = normal operation 1 = reset error flag
	LOGCOM(" RTS      : %d\n", data & 0x20 ? 1 : 0); // bit 5: 0 = /RTS set to 1    1 = /RTS set to 0
	LOGCOM(" Reset    : %d\n", data & 0x40 ? 1 : 0); // bit 6: 0 = normal operation 1 = internal reset
	LOGCOM(" Hunt mode: %d\n", data & 0x80 ? 1 : 0); // bit 7: 0 = normal operation 1 = hunt mode

	m_rts_handler(!BIT(data, 5));
	m_dtr_handler(!BIT(data, 1));

	if (BIT(data, 4))
	{
		LOGSTAT("status errors are reset\n");
		m_status &= ~(I8251_STATUS_PARITY_ERROR | I8251_STATUS_OVERRUN_ERROR | I8251_STATUS_FRAMING_ERROR);
	}

	if (BIT(data, 6))
	{
		// datasheet says "returns to mode format", not
		// completely resets the chip.  behavior of DEC Rainbow
		// backs this up.
		m_flags |= I8251_EXPECTING_MODE;
	}

	check_for_tx_start();
	update_rx_ready();
	update_tx_ready();
	update_tx_empty();
}

void i8251_device::mode_w(uint8_t data)
{
	LOG("I8251: Mode byte = %02x\n", data);

	m_mode_byte = data;

	/* Synchronous or Asynchronous? */
	if ((data & 0x03) != 0)
	{
		LOGMODE(" Asynchronous mode\n");
		// bit 1,0: baud rate factor - 0x00 = defines mode byte for synchronous mode, 0x01 = x1, 0x02 = x16, 0x03 = x64
		LOGMODE("  Baud div : %s\n", std::array<char const *, 4> {{"invalid", "x1", "x16", "x64"}}[(data & (0x02 | 0x01)) >> 0]);
		// bit 3,2: character length - 0x00 = 5 bits, 0x01 = 6 bits, 0x02 = 7 bits, 0x03 = 8 bits
		LOGMODE("  Char len : %s\n", std::array<char const *, 4> {{"5", "6", "7", "8"}}[(data & (0x08 | 0x04)) >> 2]);
		LOGMODE("  Parity ck: %s\n", data & 0x10 ? "enable" : "disable"); // bit 4: parity test enable 0 = disable    1 = enable
		LOGMODE("  Parity   : %s\n", data & 0x20 ? "even" : "odd");       // bit 5: parity type        0 = parity odd 1 = parity even
		// bit 7,6: stop bit length - 0x00 = inhibit, 0x01 = 1 bit, 0x02 = 1.5 bits, 0x03 = 2 bits
		LOGMODE("  Stop Bits: %s\n", std::array<char const *, 4> {{"inhibit", "1", "1.5", "2"}}[(data & (0x80 | 0x40)) >> 6]);

		const int data_bits_count = ((data >> 2) & 0x03) + 5;
		LOG("Character length: %d\n", data_bits_count);

		parity_t parity;
		if (BIT(data, 4))
		{
			if (BIT(data, 5))
			{
				parity = PARITY_EVEN;
			}
			else
			{
				parity = PARITY_ODD;
			}
		}
		else
		{
			parity = PARITY_NONE;
		}

		stop_bits_t stop_bits;
		switch ((data >> 6) & 0x03)
		{
		case 0:
		default:
			stop_bits = STOP_BITS_0;
			break;

		case 1:
			stop_bits = STOP_BITS_1;
			break;

		case 2:
			stop_bits = STOP_BITS_1_5;
			break;

		case 3:
			stop_bits = STOP_BITS_2;
			break;
		}


		set_data_frame(1, data_bits_count, parity, stop_bits);
		receive_register_reset();

		switch (data & 0x03)
		{
		case 1: m_br_factor = 1; break;
		case 2: m_br_factor = 16; break;
		case 3: m_br_factor = 64; break;
		}

		m_txc_count = 0;

#if 0
		/* data bits */
		m_receive_char_length = (((data >> 2) & 0x03) + 5);

		if (BIT(data, 4))
		{
			/* parity */
			m_receive_char_length++;
		}

		/* stop bits */
		m_receive_char_length++;

		m_receive_flags &= ~I8251_TRANSFER_RECEIVE_SYNCHRONISED;
		m_receive_flags |= I8251_TRANSFER_RECEIVE_WAITING_FOR_START_BIT;
#endif
		/* not expecting mode byte now */
		m_flags &= ~I8251_EXPECTING_MODE;
		LOGSTAT("status WAS reset, but not anymore\n");
		//              m_status = I8251_STATUS_TX_EMPTY | I8251_STATUS_TX_READY;
	}
	else
	{
		/*  bit 7: Number of sync characters
		        0 = 1 character
		        1 = 2 character
		        bit 6: Synchronous mode
		        0 = Internal synchronisation
		        1 = External synchronisation
		        bit 5: parity type
		        0 = parity odd
		        1 = parity even
		        bit 4: parity test enable
		        0 = disable
		        1 = enable
		        bit 3,2: character length
		        0 = 5 bits
		        1 = 6 bits
		        2 = 7 bits
		        3 = 8 bits
		        bit 1,0 = 0
		        */
		LOG("I8251: Synchronous operation\n");

		/* setup for sync byte(s) */
		m_flags |= I8251_EXPECTING_SYNC_BYTE;
		m_sync_byte_offset = 0;
		if (BIT(data, 7))
		{
			m_sync_byte_count = 1;
		}
		else
		{
			m_sync_byte_count = 2;
		}

	}
}

void i8251_device::control_w(uint8_t data)
{
	if (m_flags & I8251_EXPECTING_MODE)
	{
		if (m_flags & I8251_EXPECTING_SYNC_BYTE)
		{
			LOG("I8251: Sync byte\n");

			LOG("Sync byte: %02x\n", data);
			/* store sync byte written */
			m_sync_bytes[m_sync_byte_offset] = data;
			m_sync_byte_offset++;

			if (m_sync_byte_offset == m_sync_byte_count)
			{
				/* finished transfering sync bytes, now expecting command */
				m_flags &= ~(I8251_EXPECTING_MODE | I8251_EXPECTING_SYNC_BYTE);
				m_sync_byte_offset = 0;
				LOGSTAT("status was reset but not anymore\n");
			//  m_status = I8251_STATUS_TX_EMPTY | I8251_STATUS_TX_READY;
			}
		}
		else
		{
			mode_w(data);
		}
	}
	else
	{
		command_w(data);
	}
}



/*-------------------------------------------------
    status_r
-------------------------------------------------*/

uint8_t i8251_device::status_r()
{
	uint8_t status = (m_dsr << 7) | m_status;

	//LOG("status read: %02x\n", status);
	LOGSTAT(" TxRDY  : %d\n", status & 0x01 ? 1 : 0);
	LOGSTAT(" RxRDY  : %d\n", status & 0x02 ? 1 : 0);
	LOGSTAT(" TxEMPTY: %d\n", status & 0x04 ? 1 : 0);
	LOGSTAT(" Parity : %d\n", status & 0x08 ? 1 : 0);
	LOGSTAT(" Overrun: %d\n", status & 0x10 ? 1 : 0);
	LOGSTAT(" Framing: %d\n", status & 0x20 ? 1 : 0);
	LOGSTAT(" Syn/Brk: %d\n", status & 0x40 ? 1 : 0);
	LOGSTAT(" DSR    : %d\n\n", status & 0x80 ? 1 : 0);

	return status;
}



/*-------------------------------------------------
    data_w
-------------------------------------------------*/

void i8251_device::data_w(uint8_t data)
{
	m_tx_data = data;

	LOG("8251: data_w %02x\n" , data);

	/* writing clears */
	m_status &=~I8251_STATUS_TX_READY;
	LOGSTAT("8251: status cleared TX_READY by data_w\n");
	update_tx_ready();

	// Store state of tx enable when writing to DB buffer
	if (is_tx_enabled())
	{
		m_flags |= I8251_DELAYED_TX_EN;
	} else {
		m_flags &= ~I8251_DELAYED_TX_EN;
	}

	check_for_tx_start();

	/* if transmitter is active, then tx empty will be signalled */

	update_tx_ready();
	update_tx_empty();
}



/*-------------------------------------------------
    receive_character - called when last
    bit of data has been received
-------------------------------------------------*/

void i8251_device::receive_character(uint8_t ch)
{
	LOG("receive_character %02x\n", ch);

	m_rx_data = ch;

	LOGSTAT("status RX READY test %02x\n", m_status);
	/* char has not been read and another has arrived! */
	if (m_status & I8251_STATUS_RX_READY)
	{
		m_status |= I8251_STATUS_OVERRUN_ERROR;
		LOGSTAT("status overrun set\n");
	}

	LOGSTAT("status pre RX READY set %02x\n", m_status);
	m_status |= I8251_STATUS_RX_READY;
	LOGSTAT("status post RX READY set %02x\n", m_status);

	update_rx_ready();
}



/*-------------------------------------------------
    data_r - read data
-------------------------------------------------*/

uint8_t i8251_device::data_r()
{
	LOG("read data: %02x, STATUS=%02x\n",m_rx_data,m_status);
	/* reading clears */
	if (!machine().side_effects_disabled())
	{
		m_status &= ~I8251_STATUS_RX_READY;
		LOGSTAT("status RX_READY cleared\n");
		update_rx_ready();
	}
	return m_rx_data;
}


uint8_t i8251_device::read(offs_t offset)
{
	if (BIT(offset, 0))
		return status_r();
	else
		return data_r();
}

void i8251_device::write(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		control_w(data);
	else
		data_w(data);
}


WRITE_LINE_MEMBER(i8251_device::write_rxd)
{
	m_rxd = state;
	LOGBITS("8251: Presented a %d\n", m_rxd);
	//  device_serial_interface::rx_w(state);
}

WRITE_LINE_MEMBER(i8251_device::write_cts)
{
	m_cts = state;

	check_for_tx_start();
	update_tx_ready();
	update_tx_empty();
}

WRITE_LINE_MEMBER(i8251_device::write_dsr)
{
	m_dsr = !state;
}

WRITE_LINE_MEMBER(i8251_device::write_rxc)
{
	if (m_rxc != state)
	{
		m_rxc = state;

		if (m_rxc)
			receive_clock();
	}
}

WRITE_LINE_MEMBER(i8251_device::write_txc)
{
	if (m_txc != state)
	{
		m_txc = state;

		if (!m_txc)
			transmit_clock();
	}
}

READ_LINE_MEMBER(i8251_device::txrdy_r)
{
	return is_tx_enabled() && (m_status & I8251_STATUS_TX_READY) != 0;
}

void v5x_scu_device::device_start()
{
	i8251_device::device_start();

	save_item(NAME(m_simk));
}

void v5x_scu_device::device_reset()
{
	// FIXME: blindly copied from v53.cpp - not verified
	m_simk = 0x03;

	i8251_device::device_reset();
}

u8 v5x_scu_device::read(offs_t offset)
{
	u8 data = 0;

	switch (offset)
	{
	case 0: data = data_r(); break;
	case 1: data = status_r(); break;
	case 2: break;
	case 3: data = simk_r(); break;
	}

	return data;
}

void v5x_scu_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: data_w(data); break;
	case 1: control_w(data); break;
	case 2: mode_w(data); break;
	case 3: simk_w(data); break;
	}
}
