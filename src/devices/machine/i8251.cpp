// license:BSD-3-Clause
// copyright-holders:smf, Robbbert
/*********************************************************************

    i8251.cpp

    Intel 8251 Universal Synchronous/Asynchronous Receiver Transmitter code
    NEC uPD71051 is a clone

    The V53/V53A use a customized version with only the Asynchronous mode
    and a split command / mode register

To Do:
- BRKDET: if, in Async mode, 16 low RxD bits in succession are clocked in,
          the SYNDET pin & status must go high. It will go low upon a
          status read, same as what happens with sync.

- SYNC/BISYNC with PARITY is not tested, and therefore possibly buggy.


*********************************************************************/

#include "emu.h"
#include "i8251.h"

//#define VERBOSE 1
#include "logmacro.h"


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
	m_syndet_handler.resolve_safe();
	save_item(NAME(m_flags));
	save_item(NAME(m_sync_byte_count));
	save_item(NAME(m_status));
	save_item(NAME(m_command));
	save_item(NAME(m_mode_byte));
	save_item(NAME(m_delayed_tx_en));
	save_item(NAME(m_sync1));
	save_item(NAME(m_sync2));
	save_item(NAME(m_sync8));
	save_item(NAME(m_sync16));
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
	save_item(NAME(m_syndet_pin));
	save_item(NAME(m_hunt_on));
	save_item(NAME(m_ext_syn_set));
	save_item(NAME(m_rxd_bits));
	save_item(NAME(m_data_bits_count));
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

bool i8251_device::calc_parity(u8 ch)
{
	bool data = 0;
	for (u8 b = 0; b < 8; b++)
		data ^= BIT(ch, b);
	return data;
}

void i8251_device::sync1_rxc()
{
	// is rx enabled?
	if (!BIT(m_command, 2))
		return;

	// if ext sync, and syndet low, quit
	// Todo: what should happen here?
	if (m_syndet_pin && !m_ext_syn_set)
		return;

	u8 need_parity = BIT(m_mode_byte, 4);

	// see about parity
	if (need_parity && (m_rxd_bits == m_data_bits_count))
	{
		if (calc_parity(m_sync1) != m_rxd)
			m_status |= I8251_STATUS_PARITY_ERROR;
		// and then continue on as if everything was ok
	}
	else
	{
		// add bit to byte
		m_sync1 = (m_sync1 >> 1) | (m_rxd << (m_data_bits_count-1));
	}

	// if we are in hunt mode, the byte loaded has to
	// be the sync byte. If not, go around again.
	// if we leave hunt mode now, the sync byte must not go to the receive buffer
	bool was_in_hunt_mode = false;
	if (m_hunt_on)
	{
		if (m_sync1 == m_sync8)
		{
			m_rxd_bits = m_data_bits_count;
			m_hunt_on = false;
			was_in_hunt_mode = true;
		}
		else
			return;
	}

	// is byte complete? if not, quit
	m_rxd_bits++;
	if (m_rxd_bits < (m_data_bits_count + need_parity))
		return;

	// now we have a synchronised byte, and parity has been dealt with

	// copy byte to rx buffer
	if (!was_in_hunt_mode)
		receive_character(m_sync1);

	// Is it a sync byte? syndet gets indicated whenever
	// a sync byte passes by, regardless of hunt_mode status.
	if (m_sync1 == m_sync8)
		update_syndet(true);

	m_rxd_bits = 0;
	m_sync1 = 0;
}

void i8251_device::sync2_rxc()
{
	// is rx enabled?
	if (!BIT(m_command, 2))
		return;

	// if ext sync, and syndet low, quit
	if (m_syndet_pin && !m_ext_syn_set)
		return;

	u8 need_parity = BIT(m_mode_byte, 4);

	// see about parity
	if (need_parity && (m_rxd_bits == m_data_bits_count))
	{
		if (calc_parity(m_sync1) != m_rxd)
			m_status |= I8251_STATUS_PARITY_ERROR;
		// and then continue on as if everything was ok
	}
	else
	{
		// add bit to byte
		m_sync1 = (m_sync1 >> 1) | (m_rxd << (m_data_bits_count-1));
		m_sync2 = (m_sync2 >> 1) | (m_rxd << (m_data_bits_count*2-1));
	}

	// if we are in hunt mode, the byte loaded has to
	// be the sync byte. If not, go around again.
	// if we leave hunt mode now, the sync byte must not go to the receive buffer
	bool was_in_hunt_mode = false;
	if (m_hunt_on)
	{
		if (m_sync2 == m_sync16)
		{
			m_rxd_bits = m_data_bits_count;
			m_hunt_on = false;
			was_in_hunt_mode = true;
		}
		else
			return;
	}

	// is byte complete? if not, quit
	m_rxd_bits++;
	if (m_rxd_bits < (m_data_bits_count + need_parity))
		return;

	// now we have a synchronised byte, and parity has been dealt with

	// copy byte to rx buffer
	if (!was_in_hunt_mode)
		receive_character(m_sync1);

	// Is it a sync byte? syndet gets indicated whenever
	// a sync byte passes by, regardless of hunt_mode status.
	if (m_sync2 == m_sync16)
		update_syndet(true);

	m_rxd_bits = 0;
	m_sync1 = 0;
	m_sync2 = 0;
}

/*-------------------------------------------------
    is_tx_enabled
-------------------------------------------------*/
bool i8251_device::is_tx_enabled() const
{
	return BIT(m_command, 0) && !m_cts;
}

/*-------------------------------------------------
    check_for_tx_start
-------------------------------------------------*/
void i8251_device::check_for_tx_start()
{
	if (is_tx_enabled() && (m_status & (I8251_STATUS_TX_EMPTY | I8251_STATUS_TX_READY)) == I8251_STATUS_TX_EMPTY)
		start_tx();
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
		if ((m_status & I8251_STATUS_TX_READY) == 0 && (is_tx_enabled() || m_delayed_tx_en))
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
		m_txd_handler(data);
	}
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



/*---------------------------------------------------
    update_syndet - indicates that a sync
    character string has been received
    1 = valid sync; 0 = hunt mode on, or status read
-----------------------------------------------------*/

void i8251_device::update_syndet(bool voltage)
{
	LOG("I8251: Syndet %d\n",voltage);
	// Sanity check
	if (voltage && m_hunt_on)
		printf("I8251: Syndet - invalid parameters\n");

	// Adjust status register
	if (voltage)
		m_status |= 0x40;
	else
		m_status &= ~0x40;

	// If syndet is set as an output pin, indicate new status
	if (!m_syndet_pin)
		m_syndet_handler(voltage);
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
	/* expecting mode byte */
	m_flags = I8251_NEXT_MODE;

	/* no character to read by cpu */
	/* transmitter is ready and is empty */
	m_status = I8251_STATUS_TX_EMPTY | I8251_STATUS_TX_READY;
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
	LOG("I8251: Command byte\n");

	m_command = data;

	LOG("Command byte: %02x\n", data);

	if (BIT(data, 7))
		LOG("hunt mode\n");

	if (BIT(data, 5))
		LOG("/rts set to 0\n");
	else
		LOG("/rts set to 1\n");

	if (BIT(data, 2))
		LOG("receive enable\n");
	else
		LOG("receive disable\n");

	if (BIT(data, 1))
		LOG("/dtr set to 0\n");
	else
		LOG("/dtr set to 1\n");

	if (BIT(data, 0))
		LOG("transmit enable\n");
	else
		LOG("transmit disable\n");


	/*  bit 7:
	        0 = normal operation
	        1 = hunt mode
	    bit 6:
	        0 = normal operation
	        1 = internal reset
	    bit 5:
	        0 = /RTS set to 1
	        1 = /RTS set to 0
	    bit 4:
	        0 = normal operation
	        1 = reset error flag
	    bit 3:
	        0 = normal operation
	        1 = send break character
	    bit 2:
	        0 = receive disable
	        1 = receive enable
	    bit 1:
	        0 = /DTR set to 1
	        1 = /DTR set to 0
	    bit 0:
	        0 = transmit disable
	        1 = transmit enable
	*/

	m_rts_handler(!BIT(data, 5));
	m_dtr_handler(!BIT(data, 1));

	if (BIT(data, 4))
		m_status &= ~(I8251_STATUS_PARITY_ERROR | I8251_STATUS_OVERRUN_ERROR | I8251_STATUS_FRAMING_ERROR);

	if (BIT(data, 6))
	{
		// datasheet says "returns to mode format", not
		// completely resets the chip.  behavior of DEC Rainbow
		// backs this up.
		m_flags = I8251_NEXT_MODE;
	}

	// Hunt mode
	m_hunt_on = false;
	update_syndet(false);
	if (m_sync_byte_count)
		m_hunt_on = BIT(data, 7);
	if (m_hunt_on)
		m_ext_syn_set = false;

	if (BIT(data, 3))
		m_txd_handler(0);

	check_for_tx_start();
	update_rx_ready();
	update_tx_ready();
	update_tx_empty();
}

void i8251_device::mode_w(uint8_t data)
{
	LOG("I8251: Mode byte = %02X\n", data);

	m_mode_byte = data;

		/*  Asynchronous

		    bit 7,6: stop bit length
		    0 = inhibit
		    1 = 1 bit
		    2 = 1.5 bits
		    3 = 2 bits
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
		    bit 1,0: baud rate factor
		    0 = defines command byte for synchronous or asynchronous
		    1 = x1
		    2 = x16
		    3 = x64

		    Synchronous

		    bit 7: Number of sync characters
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

	m_data_bits_count = ((data >> 2) & 0x03) + 5;
	LOG("Character length: %d\n", m_data_bits_count);

	parity_t parity = PARITY_NONE;
	switch (data & 0x30)
	{
		case 0x10:
			LOG("Enable ODD parity checking.\n");
			parity = PARITY_ODD;
			break;

		case 0x30:
			LOG("Enable EVEN parity checking.\n");
			parity = PARITY_EVEN;
			break;

		default:
			LOG("Disable parity check.\n");
	}

	stop_bits_t stop_bits = STOP_BITS_0;
	m_br_factor = 1;
	m_flags = I8251_NEXT_COMMAND;
	m_sync_byte_count = 0;
	m_syndet_pin = false;

	/* Synchronous or Asynchronous? */
	if (data & 0x03)
	{
		LOG("I8251: Asynchronous operation\n");

		switch (data & 0xc0)
		{
			case 0x40:
				stop_bits = STOP_BITS_1;
				LOG("stop bit: 1 bit\n");
				break;

			case 0x80:
				stop_bits = STOP_BITS_1_5;
				LOG("stop bit: 1.5 bits\n");
				break;

			case 0xc0:
				stop_bits = STOP_BITS_2;
				LOG("stop bit: 2 bits\n");
				break;

			default:
				LOG("stop bit: inhibit\n");
				break;
		}

		set_data_frame(1, m_data_bits_count, parity, stop_bits);

		switch (data & 0x03)
		{
			case 2:
				m_br_factor = 16;
				break;

			case 3:
				m_br_factor = 64;
				break;
		}
	}
	else
	{
		LOG("I8251: Synchronous operation\n");

		/* setup for sync byte(s) */
		m_flags = BIT(data, 7) ? I8251_NEXT_SYNC2 : I8251_NEXT_SYNC1;
		m_sync_byte_count = BIT(data, 7) ? 1 : 2;
		set_data_frame(0, m_data_bits_count, parity, stop_bits);
		m_syndet_pin = BIT(data, 6);
		m_sync8 = 0;
		m_sync16 = 0;
		m_rxd_bits = 0;
		m_sync1 = 0;
	}

	receive_register_reset();
	m_txc_count = 0;
}

void i8251_device::sync1_w(uint8_t data)
{
	m_sync16 = (data << 8);
	m_flags = I8251_NEXT_SYNC2;
}

void i8251_device::sync2_w(uint8_t data)
{
	if (m_sync_byte_count == 2)
		m_sync16 |= data;
	else
		m_sync8 = data;

	m_flags = I8251_NEXT_COMMAND;
}

void i8251_device::control_w(uint8_t data)
{
	if (m_flags == I8251_NEXT_SYNC1)
		sync1_w(data);
	else
	if (m_flags == I8251_NEXT_SYNC2)
		sync2_w(data);
	else
	if (m_flags == I8251_NEXT_MODE)
		mode_w(data);
	else
		command_w(data);
}



/*-------------------------------------------------
    status_r
-------------------------------------------------*/

uint8_t i8251_device::status_r()
{
	uint8_t status = (m_dsr << 7) | m_status;

	LOG("status: %02x\n", status);

	// Syndet always goes off after status read
	update_syndet(false);
	return status;
}



/*-------------------------------------------------
    data_w
-------------------------------------------------*/

void i8251_device::data_w(uint8_t data)
{
	m_tx_data = data;

	LOG("data_w %02x\n", data);

	/* writing clears */
	m_status &=~I8251_STATUS_TX_READY;
	update_tx_ready();

	// Store state of tx enable when writing to DB buffer
	m_delayed_tx_en = is_tx_enabled();
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

	/* char has not been read and another has arrived! */
	if (m_status & I8251_STATUS_RX_READY)
		m_status |= I8251_STATUS_OVERRUN_ERROR;

	m_status |= I8251_STATUS_RX_READY;

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
		update_rx_ready();
	}
	return m_rx_data;
}


uint8_t i8251_device::read(offs_t offset)
{
	if (offset)
		return status_r();
	else
		return data_r();
}

void i8251_device::write(offs_t offset, uint8_t data)
{
	if (offset)
		control_w(data);
	else
		data_w(data);
}


WRITE_LINE_MEMBER(i8251_device::write_rxd)
{
	m_rxd = state;
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
	if (!m_rxc && state)
	{
		if (m_sync_byte_count == 1)
			sync1_rxc();
		else
		if (m_sync_byte_count == 2)
			sync2_rxc();
		else
			receive_clock();
	}

	m_rxc = state;
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

// forcibly kill hunt mode
WRITE_LINE_MEMBER(i8251_device::write_syn)
{
	if (m_syndet_pin && state)    // must be set as input
	{
		m_ext_syn_set = true;
		m_hunt_on = false;
		update_syndet(true);
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
