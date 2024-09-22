// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic

/***************************************************************************

        Serial device interface

***************************************************************************/

#include "emu.h"
#include "diserial.h"

#define LOG_SETUP  (1U << 1)
#define LOG_TX     (1U << 2)
#define LOG_RX     (1U << 3)
#define VERBOSE    (0)

#define LOG_OUTPUT_FUNC device().logerror
#include "logmacro.h"

device_serial_interface::device_serial_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "serial"),
	m_start_bit_hack_for_external_clocks(false),
	m_df_start_bit_count(0),
	m_df_word_length(0),
	m_df_parity(PARITY_NONE),
	m_df_stop_bit_count(STOP_BITS_0),
	m_df_min_rx_stop_bit_count(0),
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

void device_serial_interface::interface_pre_start()
{
	if (!m_rcv_clock)
		m_rcv_clock = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_serial_interface::rcv_clock), this));
	if (!m_tra_clock)
		m_tra_clock = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_serial_interface::tra_clock), this));
	m_rcv_clock_state = false;
	m_tra_clock_state = false;
}

void device_serial_interface::interface_post_start()
{
	device().save_item(NAME(m_df_start_bit_count));
	device().save_item(NAME(m_df_word_length));
	device().save_item(NAME(m_df_parity));
	device().save_item(NAME(m_df_stop_bit_count));
	device().save_item(NAME(m_df_min_rx_stop_bit_count));
	device().save_item(NAME(m_rcv_register_data));
	device().save_item(NAME(m_rcv_flags));
	device().save_item(NAME(m_rcv_bit_count_received));
	device().save_item(NAME(m_rcv_bit_count));
	device().save_item(NAME(m_rcv_byte_received));
	device().save_item(NAME(m_rcv_framing_error));
	device().save_item(NAME(m_rcv_parity_error));
	device().save_item(NAME(m_tra_register_data));
	device().save_item(NAME(m_tra_flags));
	device().save_item(NAME(m_tra_bit_count_transmitted));
	device().save_item(NAME(m_tra_bit_count));
	device().save_item(NAME(m_rcv_rate));
	device().save_item(NAME(m_tra_rate));
	device().save_item(NAME(m_rcv_line));
	device().save_item(NAME(m_tra_clock_state));
	device().save_item(NAME(m_rcv_clock_state));
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

void device_serial_interface::tx_clock_w(int state)
{
	if(state != m_tra_clock_state) {
		m_tra_clock_state = state;
		if(m_tra_clock_state)
			tra_edge();
	}
}

void device_serial_interface::rx_clock_w(int state)
{
	if(state != m_rcv_clock_state) {
		m_rcv_clock_state = state;
		if(!m_rcv_clock_state)
			rcv_edge();
	}
}

void device_serial_interface::clock_w(int state)
{
	tx_clock_w(state);
	rx_clock_w(state);
}


void device_serial_interface::set_data_frame(int start_bit_count, int data_bit_count, parity_t parity, stop_bits_t stop_bits)
{
	LOGMASKED(LOG_SETUP, "Start bits: %d; Data bits: %d; Parity: %s; Stop bits: %s\n", start_bit_count, data_bit_count, parity_tostring(parity), stop_bits_tostring(stop_bits));

	m_df_word_length = data_bit_count;

	switch (stop_bits)
	{
	case STOP_BITS_0:
	default:
		m_df_stop_bit_count = 0;
		m_df_min_rx_stop_bit_count = 0;
		break;

	case STOP_BITS_1:
		m_df_stop_bit_count = 1;
		m_df_min_rx_stop_bit_count = 1;
		break;

	case STOP_BITS_1_5:
		m_df_stop_bit_count = 2; // TODO: support 1.5 stop bits
		m_df_min_rx_stop_bit_count = 1;
		break;

	case STOP_BITS_2:
		m_df_stop_bit_count = 2;
		m_df_min_rx_stop_bit_count = 1;
		break;
	}

	m_df_parity = parity;
	m_df_start_bit_count = start_bit_count;

	/* Require at least one stop bit in async RX mode, none in sync RX mode. */
	m_rcv_bit_count = m_df_word_length + m_df_min_rx_stop_bit_count;

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

void device_serial_interface::rx_w(int state)
{
	m_rcv_line = state;
	if (m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
		return;
	receive_register_update_bit(state);
	if (m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
	{
		LOGMASKED(LOG_RX, "Receiver is synchronized\n");
		if (m_rcv_clock && !(m_rcv_rate.is_never()))
		{
			// make start delay half a cycle longer to make sure we are called after the sender
			m_rcv_clock->adjust(m_rcv_rate*2, 0, m_rcv_rate);
		}
		else if (m_start_bit_hack_for_external_clocks)
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
				LOGMASKED(LOG_RX, "Receiver saw start bit (%s)\n", device().machine().time().to_string());

				/* seen start bit! */
				/* not waiting for start bit now! */
				m_rcv_flags &=~RECEIVE_REGISTER_WAITING_FOR_START_BIT;
				m_rcv_flags |=RECEIVE_REGISTER_SYNCHRONISED;
				/* reset bit count received */
				m_rcv_bit_count_received = 0;
				m_rcv_framing_error = false;
				m_rcv_parity_error = false;
			}
			else
			{
				LOGMASKED(LOG_RX, "Receiver saw stop bit (%s)\n", device().machine().time().to_string());
			}
		}
	}
	else if (m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED)
	{
		LOGMASKED(LOG_RX, "Received bit %d as %d (%s)\n", m_rcv_bit_count_received, bit, device().machine().time().to_string());
		m_rcv_bit_count_received++;

		if (!bit && (m_rcv_bit_count_received > (m_rcv_bit_count - m_df_min_rx_stop_bit_count)))
		{
			LOGMASKED(LOG_RX, "Framing error\n");
			m_rcv_framing_error = true;
		}

		/* received all bits? */
		if (m_rcv_bit_count_received==m_rcv_bit_count)
		{
			m_rcv_bit_count_received = 0;
			m_rcv_flags &=~RECEIVE_REGISTER_SYNCHRONISED;
			m_rcv_flags |= RECEIVE_REGISTER_WAITING_FOR_START_BIT;
			LOGMASKED(LOG_RX, "Receive register full\n");
			m_rcv_flags |= RECEIVE_REGISTER_FULL;
		}
	}
}

void device_serial_interface::receive_register_extract()
{
	u8 data;

	receive_register_reset();

	/* strip off stop bits and parity */
	assert(m_rcv_bit_count >0 && m_rcv_bit_count <= 16);
	data = m_rcv_register_data>>(16-m_rcv_bit_count);

	/* mask off other bits so data byte has 0's in unused bits */
	data &= ~(0xff<<m_df_word_length);

	m_rcv_byte_received  = data;
	LOGMASKED(LOG_RX, "Receive data 0x%02x\n", m_rcv_byte_received);

	if(m_df_parity == PARITY_NONE)
		return;

	/* get state of parity bit received */
	u8 parity_received = (m_rcv_register_data >> (16 - m_rcv_bit_count + m_df_word_length)) & 0x01;

	/* parity enable? */
	switch (m_df_parity)
	{
	case PARITY_ODD:
		if (parity_received == serial_helper_get_parity(data))
			m_rcv_parity_error = true;
		break;

	case PARITY_EVEN:
		if (parity_received != serial_helper_get_parity(data))
			m_rcv_parity_error = true;
		break;

	case PARITY_MARK:
		if (!parity_received)
			m_rcv_parity_error = true;
		break;

	case PARITY_SPACE:
		if (parity_received)
			m_rcv_parity_error = true;
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
void device_serial_interface::transmit_register_setup(u8 data_byte)
{
	int i;
	u8 transmit_data;

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
		u8 parity = 0;
		switch (m_df_parity)
		{
		case PARITY_ODD:

			/* get parity */
			/* if parity = 0, data has even parity - i.e. there is an even number of one bits in the data */
			/* if parity = 1, data has odd parity - i.e. there is an odd number of one bits in the data */
			parity = serial_helper_get_parity(data_byte) ^ 1;
			break;
		case PARITY_EVEN:
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

	/* TX stop bit(s) */
	for (i=0; i<m_df_stop_bit_count; i++)
		transmit_register_add_bit(1);
}


/* get a bit from the transmit register */
u8 device_serial_interface::transmit_register_get_data_bit()
{
	int bit;

	bit = (m_tra_register_data>>(m_tra_bit_count-1-m_tra_bit_count_transmitted))&1;

	if (m_tra_bit_count_transmitted < m_df_start_bit_count)
		LOGMASKED(LOG_TX, "Transmitting start bit %d as %d (%s)\n", m_tra_bit_count_transmitted, bit, device().machine().time().to_string());
	else
		LOGMASKED(LOG_TX, "Transmitting bit %d as %d (%s)\n", m_tra_bit_count_transmitted - m_df_start_bit_count, bit, device().machine().time().to_string());
	m_tra_bit_count_transmitted++;

	/* have all bits of this stream formatted byte been sent? */
	if (m_tra_bit_count_transmitted==m_tra_bit_count)
	{
		/* yes - generate a new byte to send */
		LOGMASKED(LOG_TX, "Transmit register empty\n");
		m_tra_flags |= TRANSMIT_REGISTER_EMPTY;
	}

	return bit;
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
