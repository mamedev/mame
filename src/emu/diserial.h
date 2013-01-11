#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DISERIAL_H__
#define __DISERIAL_H__

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
/* parity selections */
/* if all the bits are added in a byte, if the result is:
    even -> parity is even
    odd -> parity is odd
*/
enum
{
	SERIAL_PARITY_NONE,     /* no parity. a parity bit will not be in the transmitted/received data */
	SERIAL_PARITY_ODD,      /* odd parity */
	SERIAL_PARITY_EVEN,     /* even parity */
	SERIAL_PARITY_MARK,     /* one parity */
	SERIAL_PARITY_SPACE     /* zero parity */
};

/*
    CTS = Clear to Send. (INPUT)
    Other end of connection is ready to accept data


    NOTE:

      This output is active low on serial chips (e.g. 0 is CTS is set),
      but here it is active high!
*/
#define SERIAL_STATE_CTS    0x0001

/*
    RTS = Request to Send. (OUTPUT)
    This end is ready to send data, and requests if the other
    end is ready to accept it

    NOTE:

      This output is active low on serial chips (e.g. 0 is RTS is set),
      but here it is active high!
*/
#define SERIAL_STATE_RTS    0x0002

/*
    DSR = Data Set ready. (INPUT)
    Other end of connection has data


    NOTE:

      This output is active low on serial chips (e.g. 0 is DSR is set),
      but here it is active high!
*/
#define SERIAL_STATE_DSR    0x0004

/*
    DTR = Data terminal Ready. (OUTPUT)
    TX contains new data.

    NOTE:

      This output is active low on serial chips (e.g. 0 is DTR is set),
      but here it is active high!
*/
#define SERIAL_STATE_DTR    0x0008
/* RX = Recieve data. (INPUT) */
#define SERIAL_STATE_RX_DATA    0x00010
/* TX = Transmit data. (OUTPUT) */
#define SERIAL_STATE_TX_DATA    0x00020

// ======================> device_serial_interface
class device_serial_interface : public device_interface
{
public:
	// construction/destruction
	device_serial_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_serial_interface();

	virtual void input_callback(UINT8 state) = 0;

	void set_data_frame(int num_data_bits, int stop_bit_count, int parity_code);

	void receive_register_reset();
	void receive_register_update_bit(int bit);
	void receive_register_extract();

	void set_rcv_rate(int baud);
	void set_tra_rate(int baud);

	void transmit_register_reset();
	void transmit_register_add_bit(int bit);
	void transmit_register_setup(UINT8 data_byte);
	UINT8 transmit_register_get_data_bit();
	UINT8 transmit_register_send_bit();

	UINT8 serial_helper_get_parity(UINT8 data) { return m_serial_parity_table[data]; }

	UINT8 get_in_data_bit()  { return ((m_input_state & SERIAL_STATE_RX_DATA)>>4) & 1; }
	void set_out_data_bit(UINT8 data)  { m_connection_state &=~SERIAL_STATE_TX_DATA; m_connection_state |=(data<<5); }

	void serial_connection_out();

	bool is_receive_register_full();
	bool is_transmit_register_empty();

	UINT8 get_received_char() { return m_rcv_byte_received; }

	void set_other_connection(device_serial_interface *other_connection);

	void connect(device_serial_interface *other_connection);
	UINT8 check_for_start(UINT8 bit);
protected:
	UINT8 m_input_state;
	UINT8 m_connection_state;
	virtual void tra_callback() { }
	virtual void rcv_callback() { receive_register_update_bit(m_rcv_line); }
	virtual void tra_complete() { }
	virtual void rcv_complete() { }

	// interface-level overrides
	virtual void interface_pre_start();
private:
	void tra_timer(void *ptr, int param);
	void rcv_timer(void *ptr, int param);

	UINT8 m_serial_parity_table[256];

	// Data frame
	// length of word in bits
	UINT8 m_df_word_length;
	// parity state
	UINT8 m_df_parity;
	// number of stop bits
	UINT8 m_df_stop_bit_count;

	// Receive register
	/* data */
	UINT16 m_rcv_register_data;
	/* flags */
	UINT8 m_rcv_flags;
	/* bit count received */
	UINT8 m_rcv_bit_count_received;
	/* length of data to receive - includes data bits, parity bit and stop bit */
	UINT8 m_rcv_bit_count;
	/* the byte of data received */
	UINT8 m_rcv_byte_received;

	// Transmit register
	/* data */
	UINT16 m_tra_register_data;
	/* flags */
	UINT8 m_tra_flags;
	/* number of bits transmitted */
	UINT8 m_tra_bit_count_transmitted;
	/* length of data to send */
	UINT8 m_tra_bit_count;

	emu_timer *m_rcv_clock;
	emu_timer *m_tra_clock;
	int m_rcv_baud;
	int m_tra_baud;
	UINT8 m_rcv_line;

	device_serial_interface *m_other_connection;
};


class serial_source_device :  public device_t,
								public device_serial_interface
{
public:
	// construction/destruction
	serial_source_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void input_callback(UINT8 state);
	void send_bit(UINT8 data);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type SERIAL_SOURCE;

#define MCFG_SERIAL_SOURCE_ADD(_tag)    \
	MCFG_DEVICE_ADD((_tag), SERIAL_SOURCE, 0)

#endif  /* __DISERIAL_H__ */
