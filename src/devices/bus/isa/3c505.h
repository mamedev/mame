// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer,R. Belmont
/*
 * threecom3c505.h - 3COM 3C505 ethernet controller
 *
 *  Created on: August 27, 2010
 *      Author: Hans Ostermeyer
 *
 */

#ifndef MAME_BUS_ISA_3C505_H
#define MAME_BUS_ISA_3C505_H

#pragma once

#include "cpu/i86/i186.h"
#include "machine/i82586.h"
#include "machine/ram.h"

#include "bus/isa/isa.h"

// ======================> PCB data structure

#pragma pack(1)

struct Memconf
{
	uint16_t cmd_q, rcv_q, mcast, frame, rcv_b, progs;
};

struct Rcv_pkt
{
	uint16_t buf_ofs, buf_seg, buf_len, timeout;
};

struct Xmit_pkt
{
	uint16_t buf_ofs, buf_seg, pkt_len;
};

struct Rcv_resp
{
	uint16_t buf_ofs, buf_seg, buf_len, pkt_len, timeout, status;
	uint32_t timetag;
};

struct Xmit_resp
{
	uint16_t buf_ofs, buf_seg, c_stat, status;
};

struct Netstat
{
	uint32_t tot_recv, tot_xmit;
	uint16_t err_CRC, err_align, err_res, err_ovrrun;
};

struct Selftest
{
	uint16_t error;
	union
	{
		uint16_t ROM_cksum;
		struct
		{
			uint16_t ofs, seg;
		} RAM;
		uint16_t i82586;
	} failure;
};

struct Info
{
	uint8_t minor_vers, major_vers;
	uint16_t ROM_cksum, RAM_sz, free_ofs, free_seg;
};

struct Memdump
{
	uint16_t size, off, seg;
};

/*
 Primary Command Block. The most important data structure. All communication
 between the host and the adapter is done with these. (Except for the actual
 Ethernet data, which has different packaging.)
 */
struct pcb_struct
{
	uint8_t command;
	uint8_t length;
	union
	{
		struct Memconf memconf;
		uint16_t configure;
		struct Rcv_pkt rcv_pkt;
		struct Xmit_pkt xmit_pkt;
		uint8_t multicast[10][6];
		uint8_t eth_addr[6];
		int16_t failed;
		struct Rcv_resp rcv_resp;
		struct Xmit_resp xmit_resp;
		struct Netstat netstat;
		struct Selftest selftest;
		struct Info info;
		struct Memdump memdump;
		uint8_t raw[62];
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
	threecom3c505_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device register I/O
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

	static void set_verbose(int on_off);

	virtual void recv_cb(uint8_t *data, int length) override;

protected:
	static constexpr unsigned CMD_BUFFER_SIZE = 100;
	static constexpr unsigned ETH_BUFFER_SIZE = 2048;
	static constexpr unsigned PGM_BUFFER_SIZE = 0x2000;

	static constexpr unsigned ETHERNET_ADDR_SIZE = 6;

	static constexpr unsigned RX_FIFO_SIZE = 32;

	threecom3c505_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual int tx_data(device_t *, const uint8_t *, int);
	virtual int setfilter(device_t *, int);

	std::string cpu_context() const;
	template <typename Format, typename... Params> void logerror(Format &&fmt, Params &&... args) const;

	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_ioport m_iobase;
	required_ioport m_irqdrq;
	required_ioport m_romopts;

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
		void start(threecom3c505_device *device, int32_t size);
		void reset();
		int append(uint8_t data);
		uint8_t get(int i) { return m_data[i]; };
		uint16_t get_word(int i) { return (m_data[i*2+1] << 8) + m_data[i*2]; };
		int is_empty() {return  m_length == 0; };
		int is_full() {return  m_length >= m_data.size(); };
		uint16_t get_length() { return m_length; };
		uint16_t get_size() { return m_data.size(); };
		uint8_t *get_data() { return &m_data[0]; };
		void copy(data_buffer *db) const;
		void log(const char *title) const;

	private:
		std::string cpu_context() const { return m_device->cpu_context(); }

		threecom3c505_device *m_device; // pointer back to our device
		uint16_t m_length;
		std::vector<uint8_t> m_data;
	};

	/* data_buffer fifo (used to buffer the received data) */
	class data_buffer_fifo
	{
	public:
		data_buffer_fifo();
		~data_buffer_fifo();
		void start(threecom3c505_device *device, int32_t size, int32_t db_size);
		void reset();
		int put(const uint8_t data[], const int length);
		int get(data_buffer *db);
		int is_empty () { return m_get_index == m_put_index; }
		int is_full () { return ((m_put_index + 1) % m_size) == m_get_index; }
	private:
		std::string cpu_context() const { return m_device->cpu_context(); }

		threecom3c505_device *m_device; // pointer back to our device
		uint16_t m_size;
		uint16_t m_count;
		uint16_t m_get_index;
		uint16_t m_put_index;
		data_buffer *m_db[RX_FIFO_SIZE];
	};

	void set_filter_list();
	void set_interrupt(enum line_state state);

	void log_command();
	void log_response();

	void do_receive_command();
	void set_command_pending(int onoff);

	int ethernet_packet_is_for_me(const uint8_t mac_address[]);

	void write_command_port( uint8_t data);
	uint8_t read_command_port();
	void write_data_port( uint8_t data);
	uint8_t read_data_port();
	void write_control_port( uint8_t data);
	uint8_t read_status_port();

	void do_command();

	uint8_t m_reg[16];

	uint8_t m_status;
	uint8_t m_control;

	uint8_t m_command_buffer[CMD_BUFFER_SIZE];
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

	uint16_t m_microcode_version;
	uint16_t m_microcode_running;

	uint16_t m_i82586_config;

	struct Netstat m_netstat;

	uint8_t m_station_address[ETHERNET_ADDR_SIZE];
	uint8_t m_multicast_list[ETHERNET_ADDR_SIZE*2];
	uint8_t m_filter_list[ETHERNET_ADDR_SIZE*4];

	enum line_state irq_state;

	emu_timer * m_do_command_timer; // timer to delay command execution

	bool m_installed;
	int m_irq, m_drq;
};

class isa16_3c505_device
	: public device_t
	, public device_isa16_card_interface
{
public:
	isa16_3c505_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_READ16_MEMBER(host_r);
	DECLARE_WRITE16_MEMBER(host_w);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	void map_main(address_map &map);
	void map_io(address_map &map);

	enum acr_mask : u8
	{
		ACR_ASF1 = 0x01, // adapter status flag 1
		ACR_ASF2 = 0x02, // adapter status flag 2
		ACR_ASF3 = 0x04, // adapter status flag 3
		ACR_LED1 = 0x08, // led control 1
		ACR_LED2 = 0x10, // led control 2
		ACR_R586 = 0x20, // reset 82586
		ACR_FLSH = 0x40, // flush data register
		ACR_LPBK = 0x80, // loopback control

		ACR_ASF  = 0x07,
	};
	enum asr_mask : u8
	{
		ASR_HSF1 = 0x01, // host status flag 1
		ASR_HSF2 = 0x02, // host status flag 2
		ASR_SWTC = 0x04, // external switch
		ASR_8_16 = 0x08, // 8/16 bit
		ASR_DIR  = 0x10, // direction flag
		ASR_HCRF = 0x20, // host command register full
		ASR_ACRE = 0x40, // adapter command register empty
		ASR_ARDY = 0x80, // data register ready

		ASR_HSF  = 0x03,
	};
	enum hcr_mask : u8
	{
		HCR_HSF1 = 0x01, // host status flag 1
		HCR_HSF2 = 0x02, // host status flag 2
		HCR_CMDE = 0x04, // command register interrupt enable
		HCR_TCEN = 0x08, // terminal count interrupt enable
		HCR_DIR  = 0x10, // direction flag
		HCR_DMAE = 0x20, // dma enable
		HCR_FLSH = 0x40, // flush data register
		HCR_ATTN = 0x80, // attention

		HCR_HSF  = 0x03,
	};
	enum hsr_mask : u8
	{
		HSR_ASF1 = 0x01, // adapter status flag 1
		HSR_ASF2 = 0x02, // adapter status flag 2
		HSR_ASF3 = 0x04, // adapter status flag 3
		HSR_DONE = 0x08, // dma done
		HSR_DIR  = 0x10, // direction
		HSR_ACRF = 0x20, // adapter command register full
		HSR_HCRE = 0x40, // host command register empty
		HSR_HRDY = 0x80, // data register ready

		HSR_ASF  = 0x07,
	};
	enum hdr_mask : u8
	{
		HDR_BRST = 0x01, // dma burst
	};

	// adapter register helpers
	u8 acmd_r();
	u8 acr_r() { return m_acr; };
	u8 asr_r() { return m_asr; };
	u16 adata_r();
	void acmd_w(u8 data);
	void acr_w(u8 data);
	void adata_w(u16 data);

	// host register helpers
	u8 hcmd_r();
	u16 hdata_r();
	void hcmd_w(u8 data);
	void hcr_w(u8 data);
	void hdata_w(u16 data);

	void update_irq(int state);
	void update_rdy(u8 const acr, u8 const hcr);

private:
	required_device<i80186_cpu_device> m_cpu;
	required_device<i82586_device> m_net;
	required_device<ram_device> m_ram;

	output_finder<2> m_led;

	required_ioport m_iobase;
	required_ioport m_irqdrq;
	required_ioport m_romopts;
	required_ioport m_test;

	u8 m_acmdr; // adapter command register
	u8 m_acr;   // adapter control register
	u8 m_asr;   // adapter status register

	u8 m_hcmdr; // host command register
	u8 m_hcr;   // host control register
	u8 m_hsr;   // host status register
	u8 m_hdr;   // host aux dma register

	util::fifo<u16, 10> m_data;

	unsigned m_isa_irq;
	unsigned m_isa_drq;

	bool m_cpu_irq_asserted;
	bool m_isa_irq_asserted;
	bool m_installed;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA16_3C505, threecom3c505_device)
DECLARE_DEVICE_TYPE(ISA16_3C505_LLE, isa16_3c505_device)

#endif // MAME_BUS_ISA_3C505_H
