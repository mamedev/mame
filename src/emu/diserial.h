// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic, Vas Crabb
#ifndef MAME_EMU_DISERIAL_H
#define MAME_EMU_DISERIAL_H

#pragma once

#include <cassert>

// Windows headers are crap, let me count the ways
#undef PARITY_NONE
#undef PARITY_ODD
#undef PARITY_EVEN
#undef PARITY_MARK
#undef PARITY_SPACE

// ======================> device_serial_interface
class device_serial_interface : public device_interface
{
public:
	enum
	{
		/* receive is waiting for start bit. The transition from high-low indicates
		start of start bit. This is used to synchronise with the data being transferred */
		RECEIVE_REGISTER_WAITING_FOR_START_BIT = 0x01,

		/* receive is synchronised with data, data bits will be clocked in */
		RECEIVE_REGISTER_SYNCHRONISED = 0x02,

		/* set if receive register has been filled */
		RECEIVE_REGISTER_FULL = 0x04
	};

	enum
	{
		/* register is empty and ready to be filled with data */
		TRANSMIT_REGISTER_EMPTY = 0x0001
	};

	/* parity selections */
	/* if all the bits are added in a byte, if the result is:
	   even -> parity is even
	   odd -> parity is odd
	*/

	enum parity_t
	{
		PARITY_NONE,     /* no parity. a parity bit will not be in the transmitted/received data */
		PARITY_ODD,      /* odd parity */
		PARITY_EVEN,     /* even parity */
		PARITY_MARK,     /* one parity */
		PARITY_SPACE     /* zero parity */
	};

	enum stop_bits_t
	{
		STOP_BITS_0,
		STOP_BITS_1 = 1,
		STOP_BITS_1_5 = 2,
		STOP_BITS_2 = 3
	};

	/* Communication lines.  Beware, everything is active high */
	enum
	{
		CTS = 0x0001, /* Clear to Send.       (INPUT)  Other end of connection is ready to accept data */
		RTS = 0x0002, /* Request to Send.     (OUTPUT) This end is ready to send data, and requests if the other */
						/*                               end is ready to accept it */
		DSR = 0x0004, /* Data Set ready.      (INPUT)  Other end of connection has data */
		DTR = 0x0008, /* Data terminal Ready. (OUTPUT) TX contains new data. */
		RX  = 0x0010, /* Receive data.        (INPUT)  */
		TX  = 0x0020  /* TX = Transmit data.  (OUTPUT) */
	};

	// construction/destruction
	device_serial_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_serial_interface();

	void rx_w(int state);
	void tx_clock_w(int state);
	void rx_clock_w(int state);
	void clock_w(int state);

protected:
	void set_data_frame(int start_bit_count, int data_bit_count, parity_t parity, stop_bits_t stop_bits);

	void receive_register_reset();
	void receive_register_update_bit(int bit);
	void receive_register_extract();

	void set_rcv_rate(const attotime &rate);
	void set_tra_rate(const attotime &rate);
	void set_rcv_rate(u32 clock, int div) { set_rcv_rate((clock && div) ? (attotime::from_hz(clock) * div) : attotime::never); }
	void set_tra_rate(u32 clock, int div) { set_tra_rate((clock && div) ? (attotime::from_hz(clock) * div) : attotime::never); }
	void set_rcv_rate(int baud) { set_rcv_rate(baud ? attotime::from_hz(baud) : attotime::never); }
	void set_tra_rate(int baud) { set_tra_rate(baud ? attotime::from_hz(baud) : attotime::never); }
	void set_rate(const attotime &rate) { set_rcv_rate(rate); set_tra_rate(rate); }
	void set_rate(u32 clock, int div) { set_rcv_rate(clock, div); set_tra_rate(clock, div); }
	void set_rate(int baud) { set_rcv_rate(baud); set_tra_rate(baud); }

	void transmit_register_reset();
	void transmit_register_add_bit(int bit);
	void transmit_register_setup(u8 data_byte);
	u8 transmit_register_get_data_bit();

	bool is_receive_register_full() const { return m_rcv_flags & RECEIVE_REGISTER_FULL; }
	bool is_transmit_register_empty() const { return m_tra_flags & TRANSMIT_REGISTER_EMPTY; }
	bool is_receive_register_synchronized() const { return m_rcv_flags & RECEIVE_REGISTER_SYNCHRONISED; }
	bool is_receive_register_shifting() const { return m_rcv_bit_count_received > 0; }
	bool is_receive_framing_error() const { return m_rcv_framing_error; }
	bool is_receive_parity_error() const { return m_rcv_parity_error; }

	u8 get_received_char() const { return m_rcv_byte_received; }

	virtual void tra_callback() { }
	virtual void rcv_callback() { receive_register_update_bit(m_rcv_line); }
	virtual void tra_complete() { }
	virtual void rcv_complete() { }

	// interface-level overrides
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	bool m_start_bit_hack_for_external_clocks;

	const char *parity_tostring(parity_t stop_bits);
	const char *stop_bits_tostring(stop_bits_t stop_bits);

private:
	TIMER_CALLBACK_MEMBER(rcv_clock) { rx_clock_w(!m_rcv_clock_state); }
	TIMER_CALLBACK_MEMBER(tra_clock) { tx_clock_w(!m_tra_clock_state); }

	// Data frame
	// number of start bits
	int m_df_start_bit_count;
	// length of word in bits
	u8 m_df_word_length;
	// parity state
	u8 m_df_parity;
	// number of TX stop bits
	u8 m_df_stop_bit_count;
	// min. number of RX stop bits
	u8 m_df_min_rx_stop_bit_count;

	// Receive register
	/* data */
	u16 m_rcv_register_data;
	/* flags */
	u8 m_rcv_flags;
	/* bit count received */
	u8 m_rcv_bit_count_received;
	/* length of data to receive - includes data bits, parity bit and stop bit */
	u8 m_rcv_bit_count;
	/* the byte of data received */
	u8 m_rcv_byte_received;

	bool m_rcv_framing_error;
	bool m_rcv_parity_error;

	// Transmit register
	/* data */
	u16 m_tra_register_data;
	/* flags */
	u8 m_tra_flags;
	/* number of bits transmitted */
	u8 m_tra_bit_count_transmitted;
	/* length of data to send */
	u8 m_tra_bit_count;

	emu_timer *m_rcv_clock;
	emu_timer *m_tra_clock;
	attotime m_rcv_rate;
	attotime m_tra_rate;
	u8 m_rcv_line;

	int m_tra_clock_state, m_rcv_clock_state;

	void tra_edge();
	void rcv_edge();
};


template <u32 FIFO_LENGTH>
class device_buffered_serial_interface : public device_serial_interface
{
protected:
	using device_serial_interface::device_serial_interface;

	void interface_post_start() override
	{
		device_serial_interface::interface_post_start();

		device().save_item(NAME(m_fifo));
		device().save_item(NAME(m_head));
		device().save_item(NAME(m_tail));
		device().save_item(NAME(m_empty));
	}

	virtual void tra_complete() override
	{
		assert(!m_empty || (m_head == m_tail));
		assert(m_head < std::size(m_fifo));
		assert(m_tail < std::size(m_fifo));

		if (!m_empty)
		{
			transmit_register_setup(m_fifo[m_head]);
			m_head = (m_head + 1U) % FIFO_LENGTH;
			m_empty = (m_head == m_tail) ? 1U : 0U;
		}
	}

	virtual void rcv_complete() override
	{
		receive_register_extract();
		received_byte(get_received_char());
	}

	void clear_fifo()
	{
		m_head = m_tail = 0U;
		m_empty = 1U;
	}

	void transmit_byte(u8 byte)
	{
		assert(!m_empty || (m_head == m_tail));
		assert(m_head < std::size(m_fifo));
		assert(m_tail < std::size(m_fifo));

		if (m_empty && is_transmit_register_empty())
		{
			transmit_register_setup(byte);
		}
		else if (m_empty || (m_head != m_tail))
		{
			m_fifo[m_tail] = byte;
			m_tail = (m_tail + 1U) % FIFO_LENGTH;
			m_empty = 0U;
		}
		else
		{
			device().logerror("FIFO overrun (byte = 0x%02x)", byte);
		}
	}

	bool fifo_empty() const
	{
		return m_empty;
	}

	bool fifo_full() const
	{
		return !m_empty && (m_head == m_tail);
	}

private:
	virtual void received_byte(u8 byte) = 0;

	u8  m_fifo[FIFO_LENGTH];
	u32 m_head = 0U, m_tail = 0U;
	u8  m_empty = 1U;
};

#endif // MAME_EMU_DISERIAL_H
