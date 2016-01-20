// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer,R. Belmont
/*
 * threecom3c505.h - 3COM 3C505 ethernet controller
 *
 *  Created on: August 27, 2010
 *      Author: Hans Ostermeyer
 *
 */

#pragma once

#ifndef THREECOM3C505_H_
#define THREECOM3C505_H_

#include "emu.h"
#include "bus/isa/isa.h"

#define CMD_BUFFER_SIZE 100
#define ETH_BUFFER_SIZE 2048
#define PGM_BUFFER_SIZE 0x2000

#define ETHERNET_ADDR_SIZE 6                 /* size of ethernet addr */

#define RX_FIFO_SIZE 32

// ======================> PCB data structure

#pragma pack(1)

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
struct pcb_struct
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
		INT16 failed;
		struct Rcv_resp rcv_resp;
		struct Xmit_resp xmit_resp;
		struct Netstat netstat;
		struct Selftest selftest;
		struct Info info;
		struct Memdump memdump;
		UINT8 raw[62];
	} data;
};

#pragma pack()

// ======================> threecom3c505_device

class threecom3c505_device:  public device_t,
						public device_network_interface,
						public device_isa16_card_interface
{
public:
	// construction/destruction
	threecom3c505_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	threecom3c505_device(const machine_config &mconfig, device_type type, std::string tag, device_t *owner, UINT32 clock);

	// device register I/O
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

	static void set_verbose(int on_off);

	required_ioport m_iobase;
	required_ioport m_irqdrq;
	required_ioport m_romopts;

	virtual void recv_cb(UINT8 *data, int length) override;

protected:
	virtual int tx_data(device_t *, const UINT8 *, int);
	virtual int setfilter(device_t *, int);

	const char *cpu_context();

	// device-level overrides
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;

private:
	// device-level overrides
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual ioport_constructor device_input_ports() const override;

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
		int is_full() {return  m_length >= m_data.size(); };
		UINT16 get_length() { return m_length; };
		UINT16 get_size() { return m_data.size(); };
		UINT8 *get_data() { return &m_data[0]; };
		void copy(data_buffer *db) const;
		void log(const char *title) const;

	private:
		const char *cpu_context() { return m_device->cpu_context(); }

		threecom3c505_device *m_device; // pointer back to our device
		UINT16 m_length;
		dynamic_buffer m_data;
	};

	/* data_buffer fifo (used to buffer the received data) */
	class data_buffer_fifo
	{
	public:
		data_buffer_fifo();
		~data_buffer_fifo();
		void start(threecom3c505_device *device, INT32 size, INT32 db_size);
		void reset();
		int put(const UINT8 data[], const int length);
		int get(data_buffer *db);
		int is_empty () { return m_get_index == m_put_index; }
		int is_full () { return ((m_put_index + 1) % m_size) == m_get_index; }
	private:
		const char *cpu_context() { return m_device->cpu_context(); }

		threecom3c505_device *m_device; // pointer back to our device
		UINT16 m_size;
		UINT16 m_count;
		UINT16 m_get_index;
		UINT16 m_put_index;
		data_buffer *m_db[RX_FIFO_SIZE];
	};

	void set_filter_list();
	void set_interrupt(enum line_state state);

	void log_command();
	void log_response();

	void do_receive_command();
	void set_command_pending(int onoff);

	int ethernet_packet_is_for_me(const UINT8 mac_address[]);

	void write_command_port( UINT8 data);
	UINT8 read_command_port();
	void write_data_port( UINT8 data);
	UINT8 read_data_port();
	void write_control_port( UINT8 data);
	UINT8 read_status_port();

	void do_command();

	UINT8 m_reg[16];

	UINT8 m_status;
	UINT8 m_control;

	UINT8 m_command_buffer[CMD_BUFFER_SIZE];
	int m_command_index;
	int m_command_pending;
	int m_wait_for_ack;
	int m_wait_for_nak;

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

	pcb_struct m_rcv_response;

	UINT16 m_microcode_version;
	UINT16 m_microcode_running;

	UINT16 m_i82586_config;

	struct Netstat m_netstat;

	UINT8 m_station_address[ETHERNET_ADDR_SIZE];
	UINT8 m_multicast_list[ETHERNET_ADDR_SIZE*2];
	UINT8 m_filter_list[ETHERNET_ADDR_SIZE*4];

	enum line_state irq_state;

	emu_timer * m_do_command_timer; // timer to delay command execution

	bool m_installed;
	int m_irq, m_drq;
};

// device type definition
extern const device_type ISA16_3C505;

#endif /* THREECOM3C505_H_ */
