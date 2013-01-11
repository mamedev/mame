/*********************************************************************

    6551.h

    MOS Technology 6551 Asynchronous Communications Interface Adapter

    A1 A0  Write                    Read
     0  0  Transmit Data Register   Receiver Data Register
     0  1  Programmed Reset         Status Register
     1  0              Command Register
     1  1              Control Register

*********************************************************************/

#include "emu.h"
#include "6551acia.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ACIA6551 = &device_creator<acia6551_device>;

//-------------------------------------------------
//  acia6551_device - constructor
//-------------------------------------------------

acia6551_device::acia6551_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ACIA6551, "MOS Technology 6551 ACIA", tag, owner, clock),
		device_serial_interface(mconfig, *this)
{
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    input_callback - called when other side
    has updated state
-------------------------------------------------*/
void acia6551_device::input_callback(UINT8 state)
{
	m_input_state = state;
}

/*-------------------------------------------------
    timer_callback
-------------------------------------------------*/
void acia6551_device::timer_callback()
{
	/* get bit received from other side and update receive register */
	receive_register_update_bit(get_in_data_bit());

	if (is_receive_register_full())
	{
		receive_register_extract();
		receive_character(get_received_char());
	}

	/* transmit register full? */
	if ((m_status_register & (1<<4))==0)
	{
		/* if transmit reg is empty */
		if (is_transmit_register_empty())
		{
			/* set it up */
			transmit_register_setup(m_transmit_data_register);
			/* acia transmit reg now empty */
			m_status_register |=(1<<4);
			/* and refresh ints */
			refresh_ints();
		}
	}

	/* if transmit is not empty... transmit data */
	if (!is_transmit_register_empty())
	{
	//  logerror("UART6551\n");
		transmit_register_send_bit();
	}
}

static TIMER_CALLBACK(acia_6551_timer_callback)
{
	reinterpret_cast<acia6551_device *>(ptr)->timer_callback();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acia6551_device::device_start()
{
	/* transmit data reg is empty */
	m_status_register = (1<<4);
	m_timer = machine().scheduler().timer_alloc(FUNC(acia_6551_timer_callback), (void *) this);

	transmit_register_reset();
	receive_register_reset();
}



/*-------------------------------------------------
    refresh_ints - update interrupt output
-------------------------------------------------*/

void acia6551_device::refresh_ints()
{
	int interrupt_state;

	interrupt_state = 0;

	/* receive interrupts */

	/* receive data register full? */
	if (m_status_register & (1<<3))
	{
		/* receiver interrupt enable? */
		if ((m_command_register & (1<<1))==0)
		{
			/* trigger a interrupt */
			interrupt_state = 1;
		}
	}

	/* set state of irq bit in status register */
	m_status_register &= ~(1<<7);

	if (interrupt_state)
	{
		m_status_register |=(1<<7);
	}

	//if (m_irq_callback != NULL)
		//(*m_irq_callback)(interrupt_state);
}



/*-------------------------------------------------
    receive_character
-------------------------------------------------*/

void acia6551_device::receive_character(UINT8 ch)
{
	/* receive register full? */
	if (m_status_register & (1<<3))
	{
		/* set overrun error */
		m_status_register |= (1<<2);
		return;
	}

	/* set new byte */
	m_receive_data_register = ch;
	/* receive register is full */
	m_status_register |= (1<<3);
	/* update ints */
	refresh_ints();
}



/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(acia6551_device::read)
{
	unsigned char data;

	data = 0x0ff;
	switch (offset & 0x03)
	{
		case 0:
		{
			/*  clear parity error, framing error and overrun error */
			/* when read of data reigster is done */
			m_status_register &=~((1<<0) | (1<<1) | (1<<2));
			/* clear receive data register full flag */
			m_status_register &=~(1<<3);
			/* return data */
			data = m_receive_data_register;
		}
		break;

	/*
	Status                    cleared by
	b0  Parity error * (1: error)       self clearing **
	b1  Framing error * (1: error)      self clearing **
	b2  Overrun * (1: error)            self clearing **
	b3  Receive Data Register Full (1: full)  Read Receive Data Register
	b4  Transmit Data Reg Empty (1: empty) Write Transmit Data Register
	b5  DCD (0: DCD low, 1: DCD high) Not resettable, reflects DCD state
	b6  DSR (0: DSR low, 1: DCD high) Not resettable, reflects DSR state
	b7  IRQ (0: no int., 1: interrupt)  Read Status Register
	*/
		case 1:
		{
			data = m_status_register;
			/* clear interrupt */
			m_status_register &= ~(1<<7);
		}
		break;

		case 2:
			data = m_command_register;
			break;

		case 3:
			data = m_control_register;
			break;

		default:
			break;
	}

	//logerror("6551 R %04x %02x\n",offset & 0x03,data);

	return data;
}



/*-------------------------------------------------
    update_data_form
-------------------------------------------------*/

void acia6551_device::update_data_form()
{
	int word_length = 8-((m_control_register>>5) & 0x03);
	int stop_bit_count = (m_control_register>>7)+1;
	int parity = 0;
	if (m_command_register & (1<<5))
	{
		parity = SERIAL_PARITY_ODD;
	}
	else
	{
		parity = SERIAL_PARITY_NONE;
	}

	set_data_frame(word_length, stop_bit_count, parity);
}



/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(acia6551_device::write)
{
	//logerror("6551 W %04x %02x\n",offset & 0x03, data);

	switch (offset & 0x03)
	{
		case 0:
		{
			/* clear transmit data register empty */
			m_status_register &= ~(1<<4);
			/* store byte */
			m_transmit_data_register = data;

		}
		break;

		case 1:
		{
			/* telstrat writes 0x07f! */
		}
		break;


		/*
		Command Register:

		b0  Data Terminal Ready
		    0 : disable receiver and all interrupts (DTR high)
		    1 : enable receiver and all interrupts (DTR low)
		b1  Receiver Interrupt Enable
		    0 : IRQ interrupt enabled from bit 3 of status register
		    1 : IRQ interrupt disabled
		b3,b2   Transmitter Control
		        Transmit Interrupt    RTS level    Transmitter
		    00     disabled     high        off
		    01     enabled      low     on
		    10     disabled     low     on
		    11     disabled     low    Transmit BRK
		b4  Normal/Echo Mode for Receiver
		    0 : normal
		    1 : echo (bits 2 and 3 must be 0)
		b5  Parity Enable
		    0 : parity disabled, no parity bit generated or received
		    1 : parity enabled
		b7,b6   Parity
		    00 : odd parity receiver and transmitter
		    01 : even parity receiver and transmitter
		    10 : mark parity bit transmitted, parity check disabled
		    11 : space parity bit transmitted, parity check disabled
		*/

		case 2:
		{
			m_command_register = data;

			/* update state of dtr */
			m_connection_state &=~SERIAL_STATE_DTR;
			if (m_command_register & (1<<0))
			{
				m_connection_state |=SERIAL_STATE_DTR;
			}

			/* update state of rts */
			switch ((m_command_register>>2) & 0x03)
			{
				case 0:
				{
					m_connection_state &=~SERIAL_STATE_RTS;
				}
				break;

				case 1:
				case 2:
				case 3:
				{
					m_connection_state |=SERIAL_STATE_RTS;
				}
				break;
			}

			serial_connection_out();
			update_data_form();
		}
		break;

		/*
		Control register:

		b3-b0   baud rate generator:
		    0000 : 16x external clock
		    0001 : 50 baud
		    0010 : 75 25
		    0011 : 110 35
		    0100 : 134.5
		    0101 : 150
		    0110 : 300 150
		    0111 : 600 300
		    1000 : 1200 600
		    1001 : 1800 600
		    1010 : 2400 600
		    1011 : 3600 1200
		    1100 : 4800 1200
		    1101 : 7200 2400
		    1110 : 9600 2400
		    1111 : 19,200 9600
		b4  receiver clock source
		    0 : external receiver clock
		    1 : baud rate generator
		b6,b5   word length
		    00 : 8 bits
		    01 : 7
		    10 : 6
		    11 : 5
		b7  stop bits
		    0 : 1 stop bit
		    1 : 2 stop bits
		        (1 stop bit if parity and word length = 8)
		        (1 1/2 stop bits if word length = 5 and no parity)
		*/
		case 3:
		{
			unsigned char previous_control_register;

			previous_control_register = m_control_register;

			if (((previous_control_register^data) & 0x07)!=0)
			{
				int rate;

				rate = data & 0x07;

				/* baud rate changed? */
				m_timer->reset();

				if (rate==0)
				{
					/* 16x external clock */
					logerror("6551: external clock not supported!\n");
				}
				else
				{
					int baud_rate;

					switch (rate)
					{
						default:
						case 1:
						{
							baud_rate = 50;
						}
						break;

						case 2:
						{
							baud_rate = 75;
						}
						break;

						case 3:
						{
							baud_rate = 110;
						}
						break;

						case 4:
						{
							baud_rate = 135;
						}
						break;

						case 5:
						{
							baud_rate = 150;
						}
						break;

						case 6:
						{
							baud_rate = 300;
						}
						break;

						case 7:
						{
							baud_rate = 600;
						}
						break;

						case 8:
						{
							baud_rate = 1200;
						}
						break;

						case 9:
						{
							baud_rate = 1800;
						}
						break;

						case 10:
						{
							baud_rate = 2400;
						}
						break;

						case 11:
						{
							baud_rate = 3600;
						}
						break;

						case 12:
						{
							baud_rate = 4800;
						}
						break;

						case 13:
						{
							baud_rate = 7200;
						}
						break;

						case 14:
						{
							baud_rate = 9600;
						}
						break;

						case 15:
						{
							baud_rate = 19200;
						}
						break;
					}

					m_timer->adjust(attotime::zero, 0, attotime::from_hz(baud_rate));
				}
			}

			m_control_register = data;
			update_data_form();
		}
		break;

		default:
			break;
	}

}
