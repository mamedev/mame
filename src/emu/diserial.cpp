// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic

/***************************************************************************

        Serial device interface

***************************************************************************/

#include "emu.h"

device_serial_interface::device_serial_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "serial"),
	m_start_bit_hack_for_external_clocks(false),
	m_df_start_bit_count(0),
	m_df_word_length(0),
	m_df_parity(PARITY_NONE),
	m_df_stop_bit_count(STOP_BITS_0),
	m_rcv_register_data(0x8000),
	m_rcv_flags(0),
	m_rcv_bit_count_received(0),
	m_rcv_bit_count(0),
	m_rcv_byte_received(0),
	m_rcv_framing_error(false),
	m_rcv_parity_error(false),
	m_tra_register_data(0),
	m_tra_flags(TRANSMIT_REGISTER_EMPTY),
	m_tra_bit_count_transmitted(0),
	m_tra_bit_count(0),
	m_rcv_clock(nullptr),
	m_tra_clock(nullptr),
	m_rcv_rate(attotime::never),
	m_tra_rate(attotime::never),
	m_rcv_line(0),
	m_tra_clock_state(false),
	m_rcv_clock_state(false)
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
}

device_serial_interface::~device_serial_interface()
{
}

void device_serial_interface::register_save_state(save_manager &save, device_t *device)
{
	std::string module = device->name();
	std::string tag = device->tag();
	save.save_item(device, module, tag, 0, NAME(m_df_start_bit_count));
	save.save_item(device, module, tag, 0, NAME(m_df_word_length));
	save.save_item(device, module, tag, 0, NAME(m_df_parity));
	save.save_item(device, module, tag, 0, NAME(m_df_stop_bit_count));
	save.save_item(device, module, tag, 0, NAME(m_rcv_register_data));
	save.save_item(device, module, tag, 0, NAME(m_rcv_flags));
	save.save_item(device, module, tag, 0, NAME(m_rcv_bit_count_received));
	save.save_item(device, module, tag, 0, NAME(m_rcv_bit_count));
	save.save_item(device, module, tag, 0, NAME(m_rcv_byte_received));
	save.save_item(device, module, tag, 0, NAME(m_rcv_framing_error));
	save.save_item(device, module, tag, 0, NAME(m_rcv_parity_error));
	save.save_item(device, module, tag, 0, NAME(m_tra_register_data));
	save.save_item(device, module, tag, 0, NAME(m_tra_flags));
	save.save_item(device, module, tag, 0, NAME(m_tra_bit_count_transmitted));
	save.save_item(device, module, tag, 0, NAME(m_tra_bit_count));
	save.save_item(device, module, tag, 0, NAME(m_rcv_rate));
	save.save_item(device, module, tag, 0, NAME(m_tra_rate));
	save.save_item(device, module, tag, 0, NAME(m_rcv_line));
	save.save_item(device, module, tag, 0, NAME(m_tra_clock_state));
	save.save_item(device, module, tag, 0, NAME(m_rcv_clock_state));
}

void device_serial_interface::interface_pre_start()
{
	m_rcv_clock = device().timer_alloc(RCV_TIMER_ID);
	m_tra_clock = device().timer_alloc(TRA_TIMER_ID);
	m_rcv_clock_state = false;
	m_tra_clock_state = false;
}

void device_serial_interface::set_rcv_rate(const attotime &rate)
{
	m_rcv_rate = rate/2;
	receive_register_reset();
	m_rcv_clock->adjust(attotime::never);
}

void device_serial_interface::set_tra_rate(const attotime &rate)
{
	m_tra_rate = rate/2;
	transmit_register_reset();
	m_tra_clock->adjust(attotime::never);
}

void device_serial_interface::tra_edge()
{
	if (!is_transmit_register_empty())
	{
		tra_callback();
		if (is_transmit_register_empty())
			tra_complete();
	}

	if (is_transmit_register_empty() && !m_tra_rate.is_never())
	{
		m_tra_clock->adjust(attotime::never);
	}
}

void device_serial_interface::rcv_edge()
{
	rcv_callback();
	if(is_receive_register_full())
	{
		m_rcv_clock->adjust(attotime::never);
		rcv_complete();
	}
}

WRITE_LINE_MEMBER(device_serial_interface::tx_clock_w)
{
	if(state != m_tra_clock_state) {
		m_tra_clock_state = state;
		if(m_tra_clock_state)
			tra_edge();
	}
}

WRITE_LINE_MEMBER(device_serial_interface::rx_clock_w)
{
	if(state != m_rcv_clock_state) {
		m_rcv_clock_state = state;
		if(!m_rcv_clock_state)
			rcv_edge();
	}
}

WRITE_LINE_MEMBER(device_serial_interface::clock_w)
{
	tx_clock_w(state);
	rx_clock_w(state);
}

void device_serial_interface::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id) {
	case TRA_TIMER_ID: tx_clock_w(!m_tra_clock_state); break;
	case RCV_TIMER_ID: rx_clock_w(!m_rcv_clock_state); break;
	}
}


void device_serial_interface::set_data_frame(int start_bit_count, int data_bit_count, parity_t parity, stop_bits_t stop_bits)
{
	m_df_word_length = data_bit_count;

	switch (stop_bits)
	{
	case STOP_BITS_0:
	default:
		m_df_stop_bit_count = 0;
		break;

	case STOP_BITS_1:
		m_df_stop_bit_count = 1;
		break;

	case STOP_BITS_1_5:
		m_df_stop_bit_count = 2; // TODO: support 1.5 stop bits
		break;

	case STOP_BITS_2:
		m_df_stop_bit_count = 2;
		break;
	}

	m_df_parity = parity;
	m_df_start_bit_count = start_bit_count;

	m_rcv_bit_count = m_df_word_length + m_df_stop_bit_count;

	if (m_df_parity != PARITY_NONE)
	{
		m_rcv_bit_count++;
	}
}

void device_serial_interface::receive_register_reset()
{
	m_rcv_bit_count_received = 0;
	m_rcv_flags &=~RECEIVE_REGISTER_FULL;
	if (m_df_start_bit_count == 0)
	{
		m_rcv_flags |= RECEIVE_REGISTER_SYNCHRONISED;
		m_rcv_flags &=~RECEIVE_REGISTER_WAITING_FOR_START_BIT;
	}
	else
	{
		m_rcv_flags &=~RECEIVE_REGISTER_SYNCHRONISED;
		m_rcv_flags |= RECEIVE_REGISTER_WAITING_FOR_START_BIT;
	}
}

WRITE_LINE_MEMBER(device_serial_interface::rx_w)
{
	m_rcv_line = state;
	if(m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
		return;
	receive_register_update_bit(state);
	if(m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
	{
		if(m_rcv_clock && !(m_rcv_rate.is_never()))
			// make start delay just a bit longer to make sure we are called after the sender
			m_rcv_clock->adjust(((m_rcv_rate*3)/2), 0, m_rcv_rate);
		else if(m_start_bit_hack_for_external_clocks)
			m_rcv_bit_count_received--;
	}
	return;
}

/* this is generic code to be used in serial chip implementations */
/* the majority of serial chips work in the same way and this code will work */
/* for them */

/* receive a bit */
void device_serial_interface::receive_register_update_bit(int bit)
{
	int previous_bit;

	//LOG(("receive register receive bit: %1x\n",bit));
	previous_bit = (m_rcv_register_data & 0x8000) ? 1 : 0;

	/* shift previous bit 7 out */
	m_rcv_register_data = m_rcv_register_data>>1;
	/* shift new bit in */
	m_rcv_register_data = (m_rcv_register_data & 0x7fff) | (bit<<15);
	/* update bit count received */

	/* asynchronous mode */
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
				m_rcv_framing_error = false;
				m_rcv_parity_error = false;
			}
		}
	}
	else
	if (m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
	{
		m_rcv_bit_count_received++;

		if (!bit && (m_rcv_bit_count_received > (m_rcv_bit_count - m_df_stop_bit_count)))
		{
			m_rcv_framing_error = true;
		}

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
	assert(m_rcv_bit_count >0 && m_rcv_bit_count <= 16);
	data = m_rcv_register_data>>(16-m_rcv_bit_count);

	/* mask off other bits so data byte has 0's in unused bits */
	data &= ~(0xff<<m_df_word_length);

	m_rcv_byte_received  = data;

	if(m_df_parity == PARITY_NONE)
		return;

	//unsigned char computed_parity;
	//unsigned char parity_received;

	/* get state of parity bit received */
	//parity_received = (m_rcv_register_data>>m_df_word length) & 0x01;

	/* parity enable? */
	switch (m_df_parity)
	{
		/* check parity */
		case PARITY_ODD:
		case PARITY_EVEN:
		{
			/* compute parity for received bits */
			//computed_parity = serial_helper_get_parity(data);

			if (m_df_parity == PARITY_ODD)
			{
				/* odd parity */


			}
			else
			{
				/* even parity */


			}

		}
		break;
		case PARITY_MARK:
		case PARITY_SPACE:
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

	if(m_tra_clock && !m_tra_rate.is_never())
		m_tra_clock->adjust(m_tra_rate, 0, m_tra_rate);

	m_tra_bit_count_transmitted = 0;
	m_tra_bit_count = 0;
	m_tra_flags &=~TRANSMIT_REGISTER_EMPTY;

	/* start bit */
	for (i=0; i<m_df_start_bit_count; i++)
	{
		transmit_register_add_bit(0);
	}

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
	if (m_df_parity!=PARITY_NONE)
	{
		/* odd or even parity */
		unsigned char parity = 0;
		switch(m_df_parity)
		{
		case PARITY_EVEN:
		case PARITY_ODD:

			/* get parity */
			/* if parity = 0, data has even parity - i.e. there is an even number of one bits in the data */
			/* if parity = 1, data has odd parity - i.e. there is an odd number of one bits in the data */
			parity = serial_helper_get_parity(data_byte);
			break;
		case PARITY_MARK:
			parity = 1;
			break;
		case PARITY_SPACE:
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

bool device_serial_interface::is_receive_register_full()
{
	return m_rcv_flags & RECEIVE_REGISTER_FULL;
}

bool device_serial_interface::is_transmit_register_empty()
{
	return m_tra_flags & TRANSMIT_REGISTER_EMPTY;
}

const char *device_serial_interface::parity_tostring(parity_t parity)
{
	switch (parity)
	{
	case PARITY_NONE:
		return "NONE";

	case PARITY_ODD:
		return "ODD";

	case PARITY_EVEN:
		return "EVEN";

	case PARITY_MARK:
		return "MARK";

	case PARITY_SPACE:
		return "SPACE";

	default:
		return "UNKNOWN";
	}
}

const char *device_serial_interface::stop_bits_tostring(stop_bits_t stop_bits)
{
	switch (stop_bits)
	{
	case STOP_BITS_0:
		return "0";

	case STOP_BITS_1:
		return "1";

	case STOP_BITS_1_5:
		return "1.5";

	case STOP_BITS_2:
		return "2";

	default:
		return "UNKNOWN";
	}
}
