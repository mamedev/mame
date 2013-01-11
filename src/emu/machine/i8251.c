/*********************************************************************

    i8251.c

    Intel 8251 Universal Synchronous/Asynchronous Receiver Transmitter code

*********************************************************************/

#include "emu.h"
#include "i8251.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

const i8251_interface default_i8251_interface = { DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL };


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    i8251_in_callback
-------------------------------------------------*/

void i8251_device::input_callback(UINT8 state)
{
	int changed = m_input_state^state;

	m_input_state = state;

	/* did cts change state? */
	if (changed & SERIAL_STATE_CTS)
	{
		/* yes */
		/* update tx ready */
		/* update_tx_ready(); */
	}
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type I8251 = &device_creator<i8251_device>;

//-------------------------------------------------
//  i8251_device - constructor
//-------------------------------------------------

i8251_device::i8251_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8251, "I8251", tag, owner, clock),
		device_serial_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8251_device::device_config_complete()
{
	// inherit a copy of the static data
	const i8251_interface *intf = reinterpret_cast<const i8251_interface *>(static_config());
	if (intf != NULL)
		*static_cast<i8251_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_rxd_cb, 0, sizeof(m_in_rxd_cb));
		memset(&m_out_txd_cb, 0, sizeof(m_out_txd_cb));
		memset(&m_in_dsr_cb, 0, sizeof(m_in_dsr_cb));
		memset(&m_out_dtr_cb, 0, sizeof(m_out_dtr_cb));
		memset(&m_out_rts_cb, 0, sizeof(m_out_rts_cb));
		memset(&m_out_rxrdy_cb, 0, sizeof(m_out_rxrdy_cb));
		memset(&m_out_txrdy_cb, 0, sizeof(m_out_txrdy_cb));
		memset(&m_out_txempty_cb, 0, sizeof(m_out_txempty_cb));
		memset(&m_out_syndet_cb, 0, sizeof(m_out_syndet_cb));
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8251_device::device_start()
{
	// resolve callbacks
	m_in_rxd_func.resolve(m_in_rxd_cb,*this);
	m_out_txd_func.resolve(m_out_txd_cb,*this);
	m_in_dsr_func.resolve(m_in_dsr_cb,*this);
	m_out_rxrdy_func.resolve(m_out_rxrdy_cb, *this);
	m_out_txrdy_func.resolve(m_out_txrdy_cb, *this);
	m_out_txempty_func.resolve(m_out_txempty_cb, *this);

	m_input_state = 0;
}




/*-------------------------------------------------
    update_rx_ready
-------------------------------------------------*/

void i8251_device::update_rx_ready()
{
	int state;

	state = m_status & I8251_STATUS_RX_READY;

	/* masked? */
	if ((m_command & (1<<2))==0)
	{
		state = 0;
	}

	m_out_rxrdy_func(state != 0);
}



/*-------------------------------------------------
    receive_clock
-------------------------------------------------*/

void i8251_device::receive_clock()
{
	/* receive enable? */
	if (m_command & (1<<2))
	{
		//logerror("I8251\n");
		/* get bit received from other side and update receive register */
		if(m_in_rxd_func.isnull())
			receive_register_update_bit(get_in_data_bit());
		else
			receive_register_update_bit(m_in_rxd_func());

		if (is_receive_register_full())
		{
			receive_register_extract();
			receive_character(get_received_char());
		}
	}
}



/*-------------------------------------------------
    transmit_clock
-------------------------------------------------*/

void i8251_device::transmit_clock()
{
	/* transmit enable? */
	if (m_command & (1<<0))
	{
		/* transmit register full? */
		if ((m_status & I8251_STATUS_TX_READY)==0)
		{
			/* if transmit reg is empty */
			if (is_transmit_register_empty())
			{
				/* set it up */
				transmit_register_setup(m_data);
				/* i8251 transmit reg now empty */
				m_status |=I8251_STATUS_TX_EMPTY;
				/* ready for next transmit */
				m_status |=I8251_STATUS_TX_READY;

				update_tx_empty();
				update_tx_ready();
			}
		}

		/* if transmit is not empty... transmit data */
		if (!is_transmit_register_empty())
		{
			UINT8 data = transmit_register_get_data_bit();
	//      logerror("I8251\n");
			//transmit_register_send_bit();
			m_out_txd_func(data);

			m_connection_state &=~SERIAL_STATE_TX_DATA;
			m_connection_state|=(data<<5);
			serial_connection_out();
		}
	}

#if 0
	/* hunt mode? */
	/* after each bit has been shifted in, it is compared against the current sync byte */
	if (m_command & (1<<7))
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
				m_command &=~(1<<7);
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

	tx_ready = 0;

	/* transmit enable? */
	if ((m_command & (1<<0))!=0)
	{
		/* other side has rts set (comes in as CTS at this side) */
		if (m_input_state & SERIAL_STATE_CTS)
		{
			if (m_status & I8251_STATUS_TX_EMPTY)
			{
				/* enable transfer */
				tx_ready = 1;
			}
		}
	}

	m_out_txrdy_func(tx_ready);
}



/*-------------------------------------------------
    update_tx_empty
-------------------------------------------------*/

void i8251_device::update_tx_empty()
{
	if (m_status & I8251_STATUS_TX_EMPTY)
	{
		/* tx is in marking state (high) when tx empty! */
		set_out_data_bit(1);
		serial_connection_out();
	}

	m_out_txempty_func((m_status & I8251_STATUS_TX_EMPTY) != 0);
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8251_device::device_reset()
{
	LOG(("I8251: Reset\n"));

	/* what is the default setup when the 8251 has been reset??? */

	/* i8251 datasheet explains the state of tx pin at reset */
	/* tx is set to 1 */
	set_out_data_bit(1);

	/* assumption, rts is set to 1 */
	m_connection_state &= ~SERIAL_STATE_RTS;
	serial_connection_out();

	transmit_register_reset();
	receive_register_reset();
	/* expecting mode byte */
	m_flags |= I8251_EXPECTING_MODE;
	/* not expecting a sync byte */
	m_flags &= ~I8251_EXPECTING_SYNC_BYTE;

	/* no character to read by cpu */
	/* transmitter is ready and is empty */
	m_status = I8251_STATUS_TX_EMPTY | I8251_STATUS_TX_READY;
	m_mode_byte = 0;
	m_command = 0;

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

WRITE8_MEMBER(i8251_device::control_w)
{
	if (m_flags & I8251_EXPECTING_MODE)
	{
		if (m_flags & I8251_EXPECTING_SYNC_BYTE)
		{
			LOG(("I8251: Sync byte\n"));

			LOG(("Sync byte: %02x\n", data));
			/* store sync byte written */
			m_sync_bytes[m_sync_byte_offset] = data;
			m_sync_byte_offset++;

			if (m_sync_byte_offset == m_sync_byte_count)
			{
				/* finished transfering sync bytes, now expecting command */
				m_flags &= ~(I8251_EXPECTING_MODE | I8251_EXPECTING_SYNC_BYTE);
				m_sync_byte_offset = 0;
			//  m_status = I8251_STATUS_TX_EMPTY | I8251_STATUS_TX_READY;
			}
		}
		else
		{
			LOG(("I8251: Mode byte\n"));

			m_mode_byte = data;

			/* Synchronous or Asynchronous? */
			if ((data & 0x03)!=0)
			{
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
				*/

				LOG(("I8251: Asynchronous operation\n"));

				LOG(("Character length: %d\n", (((data>>2) & 0x03)+5)));

				if (data & (1<<4))
				{
					LOG(("enable parity checking\n"));
				}
				else
				{
					LOG(("parity check disabled\n"));
				}

				if (data & (1<<5))
				{
					LOG(("even parity\n"));
				}
				else
				{
					LOG(("odd parity\n"));
				}

				{
					UINT8 stop_bit_length;

					stop_bit_length = (data>>6) & 0x03;

					switch (stop_bit_length)
					{
						case 0:
						{
							/* inhibit */
							LOG(("stop bit: inhibit\n"));
						}
						break;

						case 1:
						{
							/* 1 */
							LOG(("stop bit: 1 bit\n"));
						}
						break;

						case 2:
						{
							/* 1.5 */
							LOG(("stop bit: 1.5 bits\n"));
						}
						break;

						case 3:
						{
							/* 2 */
							LOG(("stop bit: 2 bits\n"));
						}
						break;
					}
				}

				int word_length = ((data>>2) & 0x03)+5;
				int parity = SERIAL_PARITY_NONE;
				int stop_bit_count = 1;
				switch ((data>>6) & 0x03)
				{
					case 0:
					case 1:
						stop_bit_count =  1;
						break;
					case 2:
					case 3:
						stop_bit_count =  2;
						break;
				}
				set_data_frame(word_length,stop_bit_count,parity);

#if 0
				/* data bits */
				m_receive_char_length = (((data>>2) & 0x03)+5);

				if (data & (1<<4))
				{
					/* parity */
					m_receive_char_length++;
				}

				/* stop bits */
				m_receive_char_length++;

				m_receive_flags &=~I8251_TRANSFER_RECEIVE_SYNCHRONISED;
				m_receive_flags |= I8251_TRANSFER_RECEIVE_WAITING_FOR_START_BIT;
#endif
				/* not expecting mode byte now */
				m_flags &= ~I8251_EXPECTING_MODE;
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
				LOG(("I8251: Synchronous operation\n"));

				/* setup for sync byte(s) */
				m_flags |= I8251_EXPECTING_SYNC_BYTE;
				m_sync_byte_offset = 0;
				if (data & 0x07)
				{
					m_sync_byte_count = 1;
				}
				else
				{
					m_sync_byte_count = 2;
				}

			}
		}
	}
	else
	{
		/* command */
		LOG(("I8251: Command byte\n"));

		m_command = data;

		LOG(("Command byte: %02x\n", data));

		if (data & (1<<7))
		{
			LOG(("hunt mode\n"));
		}

		if (data & (1<<5))
		{
			LOG(("/rts set to 0\n"));
		}
		else
		{
			LOG(("/rts set to 1\n"));
		}

		if (data & (1<<2))
		{
			LOG(("receive enable\n"));
		}
		else
		{
			LOG(("receive disable\n"));
		}

		if (data & (1<<1))
		{
			LOG(("/dtr set to 0\n"));
		}
		else
		{
			LOG(("/dtr set to 1\n"));
		}

		if (data & (1<<0))
		{
			LOG(("transmit enable\n"));
		}
		else
		{
			LOG(("transmit disable\n"));
		}


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

		m_connection_state &=~SERIAL_STATE_RTS;
		if (data & (1<<5))
		{
			/* rts set to 0 */
			m_connection_state |= SERIAL_STATE_RTS;
		}

		m_connection_state &=~SERIAL_STATE_DTR;
		if (data & (1<<1))
		{
			m_connection_state |= SERIAL_STATE_DTR;
		}

		if ((data & (1<<0))==0)
		{
			/* held in high state when transmit disable */
			set_out_data_bit(1);
		}


		/* refresh outputs */
		serial_connection_out();

		if (data & (1<<4))
		{
			m_status &= ~(I8251_STATUS_PARITY_ERROR | I8251_STATUS_OVERRUN_ERROR | I8251_STATUS_FRAMING_ERROR);
		}

		if (data & (1<<6))
		{
			// datasheet says "returns to mode format", not
			// completely resets the chip.  behavior of DEC Rainbow
			// backs this up.
			m_flags |= I8251_EXPECTING_MODE;
		}

		update_rx_ready();
		update_tx_ready();

	}
}



/*-------------------------------------------------
    status_r
-------------------------------------------------*/

READ8_MEMBER(i8251_device::status_r)
{
	UINT8 dsr = !(m_in_dsr_func.isnull() ? 0 : m_in_dsr_func() != 0);
	UINT8 status = (dsr << 7) | m_status;

	LOG(("status: %02x\n", status));
	return status;
}



/*-------------------------------------------------
    data_w
-------------------------------------------------*/

WRITE8_MEMBER(i8251_device::data_w)
{
	m_data = data;

	logerror("write data: %02x\n",data);

	/* writing clears */
	m_status &=~I8251_STATUS_TX_READY;

	/* if transmitter is active, then tx empty will be signalled */

	update_tx_ready();
}



/*-------------------------------------------------
    receive_character - called when last
    bit of data has been received
-------------------------------------------------*/

void i8251_device::receive_character(UINT8 ch)
{
	logerror("i8251 receive char: %02x\n",ch);

	m_data = ch;

	/* char has not been read and another has arrived! */
	if (m_status & I8251_STATUS_RX_READY)
	{
		m_status |= I8251_STATUS_OVERRUN_ERROR;
	}
	m_status |= I8251_STATUS_RX_READY;

	update_rx_ready();
}



/*-------------------------------------------------
    data_r - read data
-------------------------------------------------*/

READ8_MEMBER(i8251_device::data_r)
{
	logerror("read data: %02x, STATUS=%02x\n",m_data,m_status);
	/* reading clears */
	m_status &= ~I8251_STATUS_RX_READY;

	update_rx_ready();
	return m_data;
}
