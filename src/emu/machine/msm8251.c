/*********************************************************************

    msm8251.c

    MSM/Intel 8251 Universal Synchronous/Asynchronous Receiver Transmitter code

*********************************************************************/

#include "emu.h"
#include "msm8251.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _msm8251_t msm8251_t;
struct _msm8251_t
{
	devcb_resolved_read_line	in_rxd_func;
	devcb_resolved_write_line	out_txd_func;
	devcb_resolved_read_line	in_dsr_func;
	devcb_resolved_write_line	out_dtr_func;
	devcb_resolved_write_line	out_rts_func;
	devcb_resolved_write_line	out_rxrdy_func;
	devcb_resolved_write_line	out_txrdy_func;
	devcb_resolved_write_line	out_txempty_func;
	devcb_resolved_write_line	out_syndet_func;

	/* flags controlling how msm8251_control_w operates */
	UINT8 flags;
	/* offset into sync_bytes used during sync byte transfer */
	UINT8 sync_byte_offset;
	/* number of sync bytes written so far */
	UINT8 sync_byte_count;
	/* the sync bytes written */
	UINT8 sync_bytes[2];
	/* status of msm8251 */
	UINT8 status;
	UINT8 command;
	/* mode byte - bit definitions depend on mode - e.g. synchronous, asynchronous */
	UINT8 mode_byte;

	/* data being received */
	UINT8 data;

	/* receive reg */
	serial_receive_register receive_reg;
	/* transmit reg */
	serial_transmit_register transmit_reg;

	data_form form;

	/* the serial connection that data is transfered over */
	/* this is usually connected to the serial device */
	serial_connection connection;
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

static void msm8251_in_callback(running_machine &machine, int id, unsigned long state);
static void msm8251_update_tx_empty(device_t *device);
static void msm8251_update_tx_ready(device_t *device);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE msm8251_t *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MSM8251);

	return (msm8251_t *) downcast<legacy_device_base *>(device)->token();
}


INLINE const msm8251_interface *get_interface(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MSM8251);

	return (const msm8251_interface *) device->static_config();
}


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

const msm8251_interface default_msm8251_interface = { DEVCB_NULL, };


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    msm8251_in_callback
-------------------------------------------------*/

static void msm8251_in_callback(running_machine &machine, int id, unsigned long state)
{
	device_t *device;
	msm8251_t *uart;
	int changed;

	/* NPW 29-Nov-2008 - These two lines are a hack and indicate why our "serial" infrastructure needs to be updated */
	device = machine.device("uart");
	uart = get_token(device);

	changed = uart->connection.input_state^state;

	uart->connection.input_state = state;

	/* did cts change state? */
	if (changed & SERIAL_STATE_CTS)
	{
		/* yes */
		/* update tx ready */
		/* msm8251_update_tx_ready(device); */
	}
}



/*-------------------------------------------------
    DEVICE_START( msm8251 )
-------------------------------------------------*/

static DEVICE_START( msm8251 )
{
	msm8251_t *uart = get_token(device);
	const msm8251_interface *intf = get_interface(device);

	serial_helper_setup();

	// resolve callbacks
	uart->out_rxrdy_func.resolve(intf->out_rxrdy_func, *device);
	uart->out_txrdy_func.resolve(intf->out_txrdy_func, *device);
	uart->out_txempty_func.resolve(intf->out_txempty_func, *device);

	/* setup this side of the serial connection */
	serial_connection_init(device->machine(),&uart->connection);
	serial_connection_set_in_callback(device->machine(),&uart->connection, msm8251_in_callback);
	uart->connection.input_state = 0;
}



/*-------------------------------------------------
    msm8251_update_rx_ready
-------------------------------------------------*/

static void msm8251_update_rx_ready(device_t *device)
{
	msm8251_t *uart = get_token(device);
	int state;

	state = uart->status & MSM8251_STATUS_RX_READY;

	/* masked? */
	if ((uart->command & (1<<2))==0)
	{
		state = 0;
	}

	uart->out_rxrdy_func(state != 0);
}



/*-------------------------------------------------
    msm8251_receive_clock
-------------------------------------------------*/

void msm8251_receive_clock(device_t *device)
{
	msm8251_t *uart = get_token(device);

	/* receive enable? */
	if (uart->command & (1<<2))
	{
		//logerror("MSM8251\n");
		/* get bit received from other side and update receive register */
		receive_register_update_bit(&uart->receive_reg, get_in_data_bit(uart->connection.input_state));

		if (uart->receive_reg.flags & RECEIVE_REGISTER_FULL)
		{
			receive_register_extract(&uart->receive_reg, &uart->form);
			msm8251_receive_character(device, uart->receive_reg.byte_received);
		}
	}
}



/*-------------------------------------------------
    msm8251_transmit_clock
-------------------------------------------------*/

void msm8251_transmit_clock(device_t *device)
{
	msm8251_t *uart = get_token(device);

	/* transmit enable? */
	if (uart->command & (1<<0))
	{

		/* transmit register full? */
		if ((uart->status & MSM8251_STATUS_TX_READY)==0)
		{
			/* if transmit reg is empty */
			if ((uart->transmit_reg.flags & TRANSMIT_REGISTER_EMPTY)!=0)
			{
				/* set it up */
				transmit_register_setup(&uart->transmit_reg, &uart->form, uart->data);
				/* msm8251 transmit reg now empty */
				uart->status |=MSM8251_STATUS_TX_EMPTY;
				/* ready for next transmit */
				uart->status |=MSM8251_STATUS_TX_READY;

				msm8251_update_tx_empty(device);
				msm8251_update_tx_ready(device);
			}
		}

		/* if transmit is not empty... transmit data */
		if ((uart->transmit_reg.flags & TRANSMIT_REGISTER_EMPTY)==0)
		{
	//      logerror("MSM8251\n");
			transmit_register_send_bit(device->machine(),&uart->transmit_reg, &uart->connection);
		}
	}

#if 0
	/* hunt mode? */
	/* after each bit has been shifted in, it is compared against the current sync byte */
	if (uart->command & (1<<7))
	{
		/* data matches sync byte? */
		if (uart->data == uart->sync_bytes[uart->sync_byte_offset])
		{
			/* sync byte matches */
			/* update for next sync byte? */
			uart->sync_byte_offset++;

			/* do all sync bytes match? */
			if (uart->sync_byte_offset == uart->sync_byte_count)
			{
				/* ent hunt mode */
				uart->command &=~(1<<7);
			}
		}
		else
		{
			/* if there is no match, reset */
			uart->sync_byte_offset = 0;
		}
	}
#endif
}



/*-------------------------------------------------
    msm8251_update_tx_ready
-------------------------------------------------*/

static void msm8251_update_tx_ready(device_t *device)
{
	msm8251_t *uart = get_token(device);

	/* clear tx ready state */
	int tx_ready;

	/* tx ready output is set if:
        DB Buffer Empty &
        CTS is set &
        Transmit enable is 1
    */

	tx_ready = 0;

	/* transmit enable? */
	if ((uart->command & (1<<0))!=0)
	{
		/* other side has rts set (comes in as CTS at this side) */
		if (uart->connection.input_state & SERIAL_STATE_CTS)
		{
			if (uart->status & MSM8251_STATUS_TX_EMPTY)
			{
				/* enable transfer */
				tx_ready = 1;
			}
		}
	}

	uart->out_txrdy_func(tx_ready);
}



/*-------------------------------------------------
    msm8251_update_tx_empty
-------------------------------------------------*/

static void msm8251_update_tx_empty(device_t *device)
{
	msm8251_t *uart = get_token(device);

	if (uart->status & MSM8251_STATUS_TX_EMPTY)
	{
		/* tx is in marking state (high) when tx empty! */
		set_out_data_bit(uart->connection.State, 1);
		serial_connection_out(device->machine(),&uart->connection);
	}

	uart->out_txempty_func((uart->status & MSM8251_STATUS_TX_EMPTY) != 0);
}



/*-------------------------------------------------
    DEVICE_RESET( msm8251 )
-------------------------------------------------*/

static DEVICE_RESET( msm8251 )
{
	msm8251_t *uart = get_token(device);

	LOG(("MSM8251: Reset\n"));

	/* what is the default setup when the 8251 has been reset??? */

	/* msm8251 datasheet explains the state of tx pin at reset */
	/* tx is set to 1 */
	set_out_data_bit(uart->connection.State,1);

	/* assumption, rts is set to 1 */
	uart->connection.State &= ~SERIAL_STATE_RTS;
	serial_connection_out(device->machine(), &uart->connection);

	transmit_register_reset(&uart->transmit_reg);
	receive_register_reset(&uart->receive_reg);
	/* expecting mode byte */
	uart->flags |= MSM8251_EXPECTING_MODE;
	/* not expecting a sync byte */
	uart->flags &= ~MSM8251_EXPECTING_SYNC_BYTE;

	/* no character to read by cpu */
	/* transmitter is ready and is empty */
	uart->status = MSM8251_STATUS_TX_EMPTY | MSM8251_STATUS_TX_READY;
	uart->mode_byte = 0;
	uart->command = 0;

	/* update tx empty pin output */
	msm8251_update_tx_empty(device);
	/* update rx ready pin output */
	msm8251_update_rx_ready(device);
	/* update tx ready pin output */
	msm8251_update_tx_ready(device);
}



/*-------------------------------------------------
    WRITE8_DEVICE_HANDLER(msm8251_control_w)
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER(msm8251_control_w)
{
	msm8251_t *uart = get_token(device);

	if (uart->flags & MSM8251_EXPECTING_MODE)
	{
		if (uart->flags & MSM8251_EXPECTING_SYNC_BYTE)
		{
			LOG(("MSM8251: Sync byte\n"));

			LOG(("Sync byte: %02x\n", data));
			/* store sync byte written */
			uart->sync_bytes[uart->sync_byte_offset] = data;
			uart->sync_byte_offset++;

			if (uart->sync_byte_offset == uart->sync_byte_count)
			{
				/* finished transfering sync bytes, now expecting command */
				uart->flags &= ~(MSM8251_EXPECTING_MODE | MSM8251_EXPECTING_SYNC_BYTE);
				uart->sync_byte_offset = 0;
			//  uart->status = MSM8251_STATUS_TX_EMPTY | MSM8251_STATUS_TX_READY;
			}
		}
		else
		{
			LOG(("MSM8251: Mode byte\n"));

			uart->mode_byte = data;

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

				LOG(("MSM8251: Asynchronous operation\n"));

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

				uart->form.word_length = ((data>>2) & 0x03)+5;
				uart->form.parity = SERIAL_PARITY_NONE;
				switch ((data>>6) & 0x03)
				{
					case 0:
					case 1:
						uart->form.stop_bit_count =  1;
						break;
					case 2:
					case 3:
						uart->form.stop_bit_count =  2;
						break;
				}
				receive_register_setup(&uart->receive_reg, &uart->form);


#if 0
				/* data bits */
				uart->receive_char_length = (((data>>2) & 0x03)+5);

				if (data & (1<<4))
				{
					/* parity */
					uart->receive_char_length++;
				}

				/* stop bits */
				uart->receive_char_length++;

				uart->receive_flags &=~MSM8251_TRANSFER_RECEIVE_SYNCHRONISED;
				uart->receive_flags |= MSM8251_TRANSFER_RECEIVE_WAITING_FOR_START_BIT;
#endif
				/* not expecting mode byte now */
				uart->flags &= ~MSM8251_EXPECTING_MODE;
//              uart->status = MSM8251_STATUS_TX_EMPTY | MSM8251_STATUS_TX_READY;
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
				LOG(("MSM8251: Synchronous operation\n"));

				/* setup for sync byte(s) */
				uart->flags |= MSM8251_EXPECTING_SYNC_BYTE;
				uart->sync_byte_offset = 0;
				if (data & 0x07)
				{
					uart->sync_byte_count = 1;
				}
				else
				{
					uart->sync_byte_count = 2;
				}

			}
		}
	}
	else
	{
		/* command */
		LOG(("MSM8251: Command byte\n"));

		uart->command = data;

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

		uart->connection.State &=~SERIAL_STATE_RTS;
		if (data & (1<<5))
		{
			/* rts set to 0 */
			uart->connection.State |= SERIAL_STATE_RTS;
		}

		uart->connection.State &=~SERIAL_STATE_DTR;
		if (data & (1<<1))
		{
			uart->connection.State |= SERIAL_STATE_DTR;
		}

		if ((data & (1<<0))==0)
		{
			/* held in high state when transmit disable */
			set_out_data_bit(uart->connection.State,1);
		}


		/* refresh outputs */
		serial_connection_out(device->machine(), &uart->connection);

		if (data & (1<<4))
		{
			uart->status &= ~(MSM8251_STATUS_PARITY_ERROR | MSM8251_STATUS_OVERRUN_ERROR | MSM8251_STATUS_FRAMING_ERROR);
		}

		if (data & (1<<6))
		{
			device->reset();
		}

		msm8251_update_rx_ready(device);
		msm8251_update_tx_ready(device);

	}
}



/*-------------------------------------------------
    READ8_DEVICE_HANDLER(msm8251_status_r)
-------------------------------------------------*/

READ8_DEVICE_HANDLER(msm8251_status_r)
{
	msm8251_t *uart = get_token(device);

	LOG(("status: %02x\n", uart->status));
	return uart->status;
}



/*-------------------------------------------------
    WRITE8_DEVICE_HANDLER(msm8251_data_w)
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER(msm8251_data_w)
{
	msm8251_t *uart = get_token(device);

	uart->data = data;

	logerror("write data: %02x\n",data);

	/* writing clears */
	uart->status &=~MSM8251_STATUS_TX_READY;

	/* if transmitter is active, then tx empty will be signalled */

	msm8251_update_tx_ready(device);
}



/*-------------------------------------------------
    msm8251_receive_character - called when last
    bit of data has been received
-------------------------------------------------*/

void msm8251_receive_character(device_t *device, UINT8 ch)
{
	msm8251_t *uart = get_token(device);

	logerror("msm8251 receive char: %02x\n",ch);

	uart->data = ch;

	/* char has not been read and another has arrived! */
	if (uart->status & MSM8251_STATUS_RX_READY)
	{
		uart->status |= MSM8251_STATUS_OVERRUN_ERROR;
	}
	uart->status |= MSM8251_STATUS_RX_READY;

	msm8251_update_rx_ready(device);
}



/*-------------------------------------------------
    READ8_DEVICE_HANDLER(msm8251_data_r) - read data
-------------------------------------------------*/

READ8_DEVICE_HANDLER(msm8251_data_r)
{
	msm8251_t *uart = get_token(device);

	logerror("read data: %02x, STATUS=%02x\n",uart->data,uart->status);
	/* reading clears */
	uart->status &= ~MSM8251_STATUS_RX_READY;

	msm8251_update_rx_ready(device);
	return uart->data;
}



/*-------------------------------------------------
    msm8251_connect_to_serial_device - initialise
    transfer using serial device - set the callback
    which will be called when serial device has
    updated it's state
-------------------------------------------------*/

void msm8251_connect_to_serial_device(device_t *device, device_t *image)
{
	msm8251_t *uart = get_token(device);
	serial_device_connect(image, &uart->connection);
}



/*-------------------------------------------------
    msm8251_connect
-------------------------------------------------*/

void msm8251_connect(device_t *device, serial_connection *other_connection)
{
	msm8251_t *uart = get_token(device);
	serial_connection_link(device->machine(), &uart->connection, other_connection);
}


/*-------------------------------------------------
    DEVICE_GET_INFO( msm8251 )
-------------------------------------------------*/

DEVICE_GET_INFO( msm8251 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(msm8251_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(msm8251);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(msm8251);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Intel 8251 UART");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Intel 8251 UART");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:						/* Nothing */								break;
	}
}

DEFINE_LEGACY_DEVICE(MSM8251, msm8251);
