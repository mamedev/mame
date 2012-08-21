/*
 * threecom3c505.h - 3COM 3C505 ethernet controller
 *
 *  Created on: August 27, 2010
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 */

#pragma once

#ifndef THREECOM3C505_H_
#define THREECOM3C505_H_

#include "emu.h"

#define CMD_BUFFER_SIZE 100
#define ETH_BUFFER_SIZE 2048
#define PGM_BUFFER_SIZE 0x2000

#define	ETHERNET_ADDR_SIZE 6                 /* size of ethernet addr */

#define RX_FIFO_SIZE 32

/***************************************************************************
 FUNCTION PROTOTYPES
 ***************************************************************************/

READ8_DEVICE_HANDLER( threecom3c505_r );
WRITE8_DEVICE_HANDLER( threecom3c505_w );

int threecom3c505_receive(device_t *device, const UINT8 *data, int length);

void threecom3c505_set_verbose(int on_off);

//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_THREECOM3C505_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, THREECOM3C505, 0) \
	threecom3c505_device::static_set_interface(*device, _interface);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef void (*threecom3c505_set_irq)(device_t *, int);
typedef void (*threecom3c505_tx_init)(device_t *);
typedef int (*threecom3c505_tx_data)(device_t *, const UINT8 *, int);
typedef int (*threecom3c505_setfilter)(device_t *, int);

struct threecom3c505_interface
{
	threecom3c505_set_irq set_irq;
	threecom3c505_tx_init tx_init;
	threecom3c505_tx_data tx_data;
	threecom3c505_setfilter setfilter;
};

#define THREECOM3C505_INTERFACE(name) const struct threecom3c505_interface (name)

// ======================> PCB data structure

struct Memconf
{
	UINT16 cmd_q, rcv_q, mcast, frame, rcv_b, progs;
};

struct Rcv_pkt
{
	UINT16 buf_ofs, buf_seg, buf_len, timeout;
};

struct Xmit_pkt
{
	UINT16 buf_ofs, buf_seg, pkt_len;
};

struct Rcv_resp
{
	UINT16 buf_ofs, buf_seg, buf_len, pkt_len, timeout, status;
	UINT32 timetag;
};

struct Xmit_resp
{
	UINT16 buf_ofs, buf_seg, c_stat, status;
};

struct Netstat
{
	UINT32 tot_recv, tot_xmit;
	UINT16 err_CRC, err_align, err_res, err_ovrrun;
};

struct Selftest
{
	UINT16 error;
	union
	{
		UINT16 ROM_cksum;
		struct
		{
			UINT16 ofs, seg;
		} RAM;
		UINT16 i82586;
	} failure;
};

struct Info
{
	UINT8 minor_vers, major_vers;
	UINT16 ROM_cksum, RAM_sz, free_ofs, free_seg;
};

struct Memdump
{
	UINT16 size, off, seg;
};

/*
 Primary Command Block. The most important data structure. All communication
 between the host and the adapter is done with these. (Except for the actual
 Ethernet data, which has different packaging.)
 */
typedef struct
{
	UINT8 command;
	UINT8 length;
	union
	{
		struct Memconf memconf;
		UINT16 configure;
		struct Rcv_pkt rcv_pkt;
		struct Xmit_pkt xmit_pkt;
		UINT8 multicast[10][6];
		UINT8 eth_addr[6];
		UINT8 failed;
		struct Rcv_resp rcv_resp;
		struct Xmit_resp xmit_resp;
		struct Netstat netstat;
		struct Selftest selftest;
		struct Info info;
		struct Memdump memdump;
		UINT8 raw[62];
	} data;
} pcb_struct;

// ======================> threecom3c505_device

class threecom3c505_device:  public device_t,
					  public device_network_interface,
					  public threecom3c505_interface
{
public:
	// construction/destruction
	threecom3c505_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const threecom3c505_interface &interface);

	void recv_cb(UINT8 *buf, int len);
	bool mcast_chk(const UINT8 *buf, int len);

	// device register I/O
	UINT8 read_port(offs_t offset);
	void write_port(offs_t offset, UINT8 data);

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	const char *cpu_context();

	class data_buffer_fifo;

    /* data buffer */
	class data_buffer
	{
		friend class data_buffer_fifo;

	public:
		data_buffer();
		void start(threecom3c505_device *device, INT32 size);
		void reset();
		int append(UINT8 data);
		UINT8 get(int i) { return m_data[i]; };
		UINT16 get_word(int i) { return (m_data[i*2+1] << 8) + m_data[i*2]; };
		int is_empty() {return  m_length == 0; };
		int is_full() {return  m_length >= m_size; };
		UINT16 get_length() { return m_length; };
		UINT16 get_size() { return m_size; };
		UINT8 *get_data() { return m_data; };
		void copy(data_buffer *db) const;
		void log(const char *title) const;

	private:
		threecom3c505_device *m_device; // pointer back to our device
		UINT16 m_length;
		UINT16 m_size;
		UINT8 *m_data;
	};

    /* data_buffer fifo (used to buffer the received data) */
	class data_buffer_fifo
	{
	public:
		data_buffer_fifo();
		void start(threecom3c505_device *device, INT32 size, INT32 db_size);
		void reset();
		int put(const UINT8 data[], const int length);
		int get(data_buffer *db);
	private:
		threecom3c505_device *m_device; // pointer back to our device
		UINT16 m_size;
		UINT16 m_count;
		UINT16 m_get_index;
		UINT16 m_put_index;
		data_buffer *m_db[RX_FIFO_SIZE];
	};

	void set_filter_list();
	void set_interrupt(enum line_state state);
	static TIMER_CALLBACK( static_set_interrupt );

	void log_command();
	void log_response();
	void log_tx_data();
	void log_rx_data();

	void do_receive_command();
	void set_command_pending(int onoff);
	void do_command();

	int ethernet_packet_is_for_me(const UINT8 mac_address[]);

	void write_command_port( UINT8 data);
	UINT8 read_command_port();
	void write_data_port( UINT8 data);
	UINT8 read_data_port();
	void write_control_port( UINT8 data);
	UINT8 read_status_port();

	 // pointer to myself (nasty: used for cpu_context)
	threecom3c505_device *m_device;

	UINT8 m_reg[16];

	UINT8 m_status;
	UINT8 m_control;

	UINT8 m_command_buffer[CMD_BUFFER_SIZE];
	int m_command_index;
	int m_command_pending;
	int m_mc_f9_pending;
	int m_wait_for_ack;

	data_buffer_fifo m_rx_fifo;

	data_buffer m_rx_data_buffer; // the ethernet receive buffer
	int m_rx_data_index;
	int m_rx_pending;

	data_buffer m_tx_data_buffer; // the ethernet transmit buffer
	int m_tx_data_length;

	data_buffer m_program_buffer; // the program data buffer
	int m_program_length;

	pcb_struct m_response;
	int m_response_length;
	int m_response_index;

	UINT16 m_microcode_version;
	UINT16 m_microcode_running;

	UINT16 m_i82586_config;

	struct Netstat m_netstat;

	UINT8 m_station_address[ETHERNET_ADDR_SIZE];
	UINT8 m_multicast_list[ETHERNET_ADDR_SIZE*2];
	UINT8 m_filter_list[ETHERNET_ADDR_SIZE*4];

	enum line_state irq_state;

	emu_timer * m_timer; // timer to delay interrupts
};

// device type definition
extern const device_type THREECOM3C505;

#endif /* THREECOM3C505_H_ */
