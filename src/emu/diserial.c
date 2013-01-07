/***************************************************************************

        Serial device interface

***************************************************************************/

#include "emu.h"


/* receive is waiting for start bit. The transition from high-low indicates
start of start bit. This is used to synchronise with the data being transfered */
#define RECEIVE_REGISTER_WAITING_FOR_START_BIT 0x01
/* receive is synchronised with data, data bits will be clocked in */
#define RECEIVE_REGISTER_SYNCHRONISED 0x02
/* set if receive register has been filled */
#define RECEIVE_REGISTER_FULL 0x04

/* register is empty and ready to be filled with data */
#define TRANSMIT_REGISTER_EMPTY 0x0001

device_serial_interface::device_serial_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device),
	  m_other_connection(NULL)
{
	/* if sum of all bits in the byte is even, then the data
    has even parity, otherwise it has odd parity */
	for (int i=0; i<256; i++)
	{
		int sum = 0;
		int data = i;

		for (int b=0; b<8; b++)
		{
			sum+=data & 0x01;
			data = data>>1;
		}

		m_serial_parity_table[i] = sum & 0x01;
	}
	m_rcv_clock = NULL;
	m_tra_clock = NULL;
	m_tra_baud = 0;
	m_rcv_baud = 0;
	m_tra_flags = 0;
	m_rcv_register_data = 0x8000;
}

device_serial_interface::~device_serial_interface()
{
}

//-------------------------------------------------
//  interface_pre_start - work to be done prior to
//  actually starting a device
//-------------------------------------------------

void device_serial_interface::interface_pre_start()
{
	m_rcv_clock = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_serial_interface::rcv_timer), this));
	m_tra_clock = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_serial_interface::tra_timer), this));
}

void device_serial_interface::set_rcv_rate(int baud)
{
	m_rcv_baud = baud;
	receive_register_reset();
	m_rcv_clock->adjust(attotime::never);
}

void device_serial_interface::set_tra_rate(int baud)
{
	m_tra_baud = baud;
	transmit_register_reset();
	m_tra_clock->adjust(attotime::never);
}

void device_serial_interface::tra_timer(void *ptr, int param)
{
	tra_callback();
	if(is_transmit_register_empty())
	{
		m_tra_clock->adjust(attotime::never);
		tra_complete();
	}
}

void device_serial_interface::rcv_timer(void *ptr, int param)
{
	rcv_callback();
	if(is_receive_register_full())
	{
		m_rcv_clock->adjust(attotime::never);
		rcv_complete();
	}
}

void device_serial_interface::set_data_frame(int num_data_bits, int stop_bit_count, int parity_code)
{
	m_df_word_length = num_data_bits;
	m_df_stop_bit_count = stop_bit_count;
	m_df_parity = parity_code;

	m_rcv_bit_count = m_df_word_length + m_df_stop_bit_count;

	if (m_df_parity != SERIAL_PARITY_NONE)
	{
		m_rcv_bit_count++;
	}
}

void device_serial_interface::receive_register_reset()
{
	m_rcv_bit_count_received = 0;
	m_rcv_flags &=~RECEIVE_REGISTER_FULL;
	m_rcv_flags &=~RECEIVE_REGISTER_SYNCHRONISED;
	m_rcv_flags |= RECEIVE_REGISTER_WAITING_FOR_START_BIT;
}

UINT8 device_serial_interface::check_for_start(UINT8 bit)
{
	m_rcv_line = bit;
	if(m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
		return 0;
	receive_register_update_bit(bit);
	if(m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
	{
		if(m_rcv_clock && m_rcv_baud)
			// make start delay just a bit longer to make sure we are called after the sender
			m_rcv_clock->adjust(attotime::from_hz((m_rcv_baud*2)/3), 0, attotime::from_hz(m_rcv_baud));
		return 1;
	}
	return 0;
}

/* this is generic code to be used in serial chip implementations */
/* the majority of serial chips work in the same way and this code will work */
/* for them */

/* receive a bit */
void device_serial_interface::receive_register_update_bit(int bit)
{
	int previous_bit;

	//LOG(("receive register receive bit: %1x\n",bit));
	previous_bit = (m_rcv_register_data & 0x8000) && 1;

	/* shift previous bit 7 out */
	m_rcv_register_data = m_rcv_register_data>>1;
	/* shift new bit in */
	m_rcv_register_data = (m_rcv_register_data & 0x7fff) | (bit<<15);
	/* update bit count received */
	m_rcv_bit_count_received++;

	/* asyncrhonouse mode */
	if (m_rcv_flags & RECEIVE_REGISTER_WAITING_FOR_START_BIT)
	{
		/* the previous bit is stored in uart.receive char bit 0 */
		/* has the bit state changed? */
		if (((previous_bit ^ bit) & 0x01)!=0)
		{
			/* yes */
			if (bit==0)
			{
				//logerror("receive register saw start bit\n");

				/* seen start bit! */
				/* not waiting for start bit now! */
				m_rcv_flags &=~RECEIVE_REGISTER_WAITING_FOR_START_BIT;
				m_rcv_flags |=RECEIVE_REGISTER_SYNCHRONISED;
				/* reset bit count received */
				m_rcv_bit_count_received = 0;
			}
		}
	}
	else
	if (m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
	{
		/* received all bits? */
		if (m_rcv_bit_count_received==m_rcv_bit_count)
		{
			m_rcv_bit_count_received = 0;
			m_rcv_flags &=~RECEIVE_REGISTER_SYNCHRONISED;
			m_rcv_flags |= RECEIVE_REGISTER_WAITING_FOR_START_BIT;
			//logerror("receive register full\n");
			m_rcv_flags |= RECEIVE_REGISTER_FULL;
		}
	}
}

void device_serial_interface::receive_register_extract()
{
	UINT8 data;

	receive_register_reset();

	/* strip off stop bits and parity */
	data = m_rcv_register_data>>(16-m_rcv_bit_count);

	/* mask off other bits so data byte has 0's in unused bits */
	data &= ~(0xff<<m_df_word_length);

	m_rcv_byte_received  = data;

	if(m_df_parity == SERIAL_PARITY_NONE)
		return;

	//unsigned char computed_parity;
	//unsigned char parity_received;

	/* get state of parity bit received */
	//parity_received = (m_rcv_register_data>>m_df_word length) & 0x01;

	/* parity enable? */
	switch (m_df_parity)
	{
		/* check parity */
		case SERIAL_PARITY_ODD:
		case SERIAL_PARITY_EVEN:
		{
			/* compute parity for received bits */
			//computed_parity = serial_helper_get_parity(data);

			if (m_df_parity == SERIAL_PARITY_ODD)
			{
				/* odd parity */


			}
			else
			{
				/* even parity */


			}

		}
		break;
		case SERIAL_PARITY_MARK:
		case SERIAL_PARITY_SPACE:
			//computed_parity = parity_received;
			break;
	}
}


/***** TRANSMIT REGISTER *****/

void device_serial_interface::transmit_register_reset()
{
	m_tra_flags |=TRANSMIT_REGISTER_EMPTY;
}

/* used to construct data in stream format */
void device_serial_interface::transmit_register_add_bit(int bit)
{
	/* combine bit */
	m_tra_register_data = m_tra_register_data<<1;
	m_tra_register_data &=~1;
	m_tra_register_data|=(bit & 0x01);
	m_tra_bit_count++;
}


/* generate data in stream format ready for transfer */
void device_serial_interface::transmit_register_setup(UINT8 data_byte)
{
	int i;
	unsigned char transmit_data;

	if(m_tra_clock && m_tra_baud)
		m_tra_clock->adjust(attotime::from_hz(m_tra_baud), 0, attotime::from_hz(m_tra_baud));

	m_tra_bit_count_transmitted = 0;
	m_tra_bit_count = 0;
	m_tra_flags &=~TRANSMIT_REGISTER_EMPTY;

	/* start bit */
	transmit_register_add_bit(0);

	/* data bits */
	transmit_data = data_byte;
	for (i=0; i<m_df_word_length; i++)
	{
		int databit;

		/* get bit from data */
		databit = transmit_data & 0x01;
		/* add bit to formatted byte */
		transmit_register_add_bit(databit);
		transmit_data = transmit_data>>1;
	}

	/* parity */
	if (m_df_parity!=SERIAL_PARITY_NONE)
	{
		/* odd or even parity */
		unsigned char parity = 0;
		switch(m_df_parity)
		{
		case SERIAL_PARITY_EVEN:
		case SERIAL_PARITY_ODD:

			/* get parity */
			/* if parity = 0, data has even parity - i.e. there is an even number of one bits in the data */
			/* if parity = 1, data has odd parity - i.e. there is an odd number of one bits in the data */
			parity = serial_helper_get_parity(data_byte);
			break;
		case SERIAL_PARITY_MARK:
			parity = 1;
			break;
		case SERIAL_PARITY_SPACE:
			parity = 0;
			break;
		}
		transmit_register_add_bit(parity);
	}

	/* stop bit(s) */
	for (i=0; i<m_df_stop_bit_count; i++)
	{
		transmit_register_add_bit(1);
	}
}


/* get a bit from the transmit register */
UINT8 device_serial_interface::transmit_register_get_data_bit()
{
	int bit;

	bit = (m_tra_register_data>>(m_tra_bit_count-1-m_tra_bit_count_transmitted))&1;

	m_tra_bit_count_transmitted++;

	/* have all bits of this stream formatted byte been sent? */
	if (m_tra_bit_count_transmitted==m_tra_bit_count)
	{
		/* yes - generate a new byte to send */
		m_tra_flags |= TRANSMIT_REGISTER_EMPTY;
	}

	return bit;
}

UINT8 device_serial_interface::transmit_register_send_bit()
{
	UINT8 data = transmit_register_get_data_bit();

	/* set tx data bit */
	m_connection_state &=~SERIAL_STATE_TX_DATA;
	m_connection_state|=(data<<5);

	/* state out through connection */
	serial_connection_out();

	return data;
}



/* this converts state at this end to a state the other end can accept */
/* e.g. CTS at this end becomes RTS at other end.
        RTS at this end becomes CTS at other end.
        TX at this end becomes RX at other end.
        RX at this end becomes TX at other end.
        etc

        The same thing is done inside the serial null-terminal lead */

static UINT8 serial_connection_spin_bits(UINT8 input_status)
{
	return
		/* cts -> rts */
		(((input_status & 0x01)<<1) |
		/* rts -> cts */
		((input_status>>1) & 0x01) |
		/* dsr -> dtr */
		(((input_status>>2) & 0x01)<<3) |
		/* dtr -> dsr */
		(((input_status>>3) & 0x01)<<2) |
		/* rx -> tx */
		(((input_status>>4) & 0x01)<<5) |
		/* tx -> rx */
		(((input_status>>5) & 0x01)<<4));
}

void device_serial_interface::serial_connection_out()
{

	if (m_other_connection!=NULL)
	{
		UINT8 state_at_other_end = serial_connection_spin_bits(m_connection_state);

		m_other_connection->input_callback(state_at_other_end);
	}
}

bool device_serial_interface::is_receive_register_full()
{
	return m_rcv_flags & RECEIVE_REGISTER_FULL;
}

bool device_serial_interface::is_transmit_register_empty()
{
	return m_tra_flags & TRANSMIT_REGISTER_EMPTY;
}

void device_serial_interface::set_other_connection(device_serial_interface *other_connection)
{
	m_other_connection = other_connection;
}

/* join two serial connections together */
void device_serial_interface::connect(device_serial_interface *other_connection)
{
	/* both connections should have their in connection setup! */
	/* the in connection is the callback they use to update their state based
    on the output from the other side */
	set_other_connection(other_connection);
	other_connection->set_other_connection(this);

	/* let b know the state of a */
	serial_connection_out();
	/* let a know the state of b */
	other_connection->serial_connection_out();
}



const device_type SERIAL_SOURCE = &device_creator<serial_source_device>;

//-------------------------------------------------
//  serial_source_device - constructor
//-------------------------------------------------

serial_source_device::serial_source_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SERIAL_SOURCE, "Serial source", tag, owner, clock),
	  device_serial_interface(mconfig, *this)
{
}

void serial_source_device::device_start()
{
}

void serial_source_device::input_callback(UINT8 state)
{
	m_input_state = state;
}

void serial_source_device::send_bit(UINT8 data)
{
	set_out_data_bit(data);
	serial_connection_out();
}
