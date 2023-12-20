#ifndef MAME_MACHINE_W5100_SOCKET_H
#define MAME_MACHINE_W5100_SOCKET_H

#pragma once

#include "w5100.h"
#include "util/tcp_sequence.h"

class w5100_device;

class w5100_socket_device : public device_t
{
public:


	w5100_socket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int socket_number);
	w5100_socket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t read_register(offs_t offet);
	void write_register(offs_t offset, uint8_t data);



	void process_pending();

	bool process_udp(const uint8_t *mac, const wiznet_ip_info &ip, const wiznet_udp_info &udp, const uint8_t *data, int data_length);
	bool process_tcp(const uint8_t *mac, const wiznet_ip_info &ip, const wiznet_tcp_info &tcp, const uint8_t *data, int data_length);
	bool process_ipraw(const uint8_t *mac, const wiznet_ip_info &ip, const uint8_t *data, int data_length);
	bool process_macraw(const uint8_t *data, int data_length);

	void process_arp_reply(const uint8_t *mac, uint32_t ip);
	void process_igmp_query(uint32_t ip);


	void set_rx_buffer_size(unsigned size);
	void set_tx_buffer_size(unsigned size);
	unsigned get_rx_buffer_size() const;
	unsigned get_tx_buffer_size() const;


	void tcp_send_reset(const uint8_t *mac, const wiznet_ip_info &ip, const wiznet_tcp_info &tcp);

	constexpr bool ir() const { return m_ir; }

protected:
	virtual void device_reset() override;
	virtual void device_start() override;

private:

	wiznet_ip_info socket_ip_info() const;
	wiznet_tcp_info socket_tcp_info(unsigned flags, util::tcp_sequence seq, util::tcp_sequence ack) const;
	wiznet_udp_info socket_udp_info(unsigned data_length = 0) const;


	void command(unsigned cr);
	void command_open();
	void command_listen();
	void command_connect();
	void command_disconnect();
	void command_close();
	void command_recv();
	void command_send();
	void command_send_mac();
	void command_send_keep();

	void intern_close(unsigned irqs = 0);
	void intern_connect();
	void intern_send();
	bool intern_recv(const uint8_t *header, int header_size, const uint8_t *data, int data_size);
	bool intern_recv(const uint8_t *data, int data_size)
	{
		return intern_recv(nullptr, 0, data, data_size);
	}
	uint16_t recv_window_size() const;

	void intern_send_tcp(bool resend = false);


	bool before_send();

	uint32_t tcp_generate_iss() const;
	void tcp_send_segment(unsigned flags, util::tcp_sequence seq, util::tcp_sequence ack);
	void tcp_send(bool resend);

	void tcp_process_segment(const wiznet_tcp_info &tcp, const uint8_t *seg_data, int seg_len);



	void arp_start_timer();
	void arp_update_timer();
	void tcp_start_timer();
	void tcp_update_timer();

	void delayed_ack_timer(int param);
	void resend_timer(int param);


	w5100_device * const m_parent;
	const unsigned m_sn;

	// visible registers:
	uint8_t m_mr;
	uint8_t m_ir;
	uint8_t m_sr;
	uint16_t m_port;
	uint8_t m_dhar[6];
	uint32_t m_dipr;
	uint16_t m_dport;
	uint16_t m_mss;
	uint8_t m_proto;
	uint8_t m_tos;
	uint8_t m_ttl;
	uint16_t m_frag;
	uint8_t m_rx_buf_size;
	uint8_t m_tx_buf_size;
	uint16_t m_tx_rd;
	uint16_t m_tx_wr;
	uint16_t m_rx_rd;
	uint16_t m_rx_wr;


	// invisible registers
	uint8_t m_active_proto;

	uint16_t m_pending_mss;
	uint32_t m_pending_dipr;
	uint8_t m_pending_dhar[6];
	uint16_t m_pending_dport;
	uint16_t m_pending_rx_rd;
	uint16_t m_pending_tx_wr;


	// tcp state
	util::tcp_sequence m_snd_una; // oldest unack seq number
	util::tcp_sequence m_snd_nxt; // next seq number to send
	util::tcp_sequence m_rcv_nxt; // receive next

	util::tcp_sequence m_irs;     // initial recv seq number
	util::tcp_sequence m_iss;     // initial send seq

	util::tcp_sequence m_snd_wl1; // seg seq of last window update
	util::tcp_sequence m_snd_wl2; // seg ack of last window update

	util::tcp_sequence m_resend_seq;

	uint32_t m_snd_wnd; // send window


	// timer housekeeping
	uint16_t m_current_rcr;
	uint16_t m_current_rtr;
	emu_timer *m_delayed_ack_timer;
	emu_timer *m_resend_timer;


	bool m_arp_valid;
	bool m_send_in_progress;
	bool m_resend_in_progress;
	bool m_arp_in_progress;
	uint32_t m_arp_ip;

};

DECLARE_DEVICE_TYPE(W5100_SOCKET, w5100_socket_device)

#endif
