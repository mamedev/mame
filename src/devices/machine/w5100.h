
#ifndef MAME_MACHINE_W5100_H
#define MAME_MACHINE_W5100_H

#pragma once

#include "dinetwork.h"

class w5100_socket_device;


struct wiznet_eth_info
{
	static constexpr const unsigned header_length = 14;
	uint8_t dest_mac[6];
	uint8_t src_mac[6];
	uint16_t type;

	// static eth_info from_buffer(const uint8_t *data, int length);
};

struct wiznet_ip_info
{
	static constexpr const unsigned default_header_length = 20;
	unsigned header_length;
	uint32_t src_ip;
	uint32_t dest_ip;
	uint16_t fragment;
	uint16_t total_length;
	uint8_t ttl;
	uint8_t tos;
	uint8_t proto;

	// static ip_info from_buffer(const uint8_t *data, int length);
};

struct wiznet_tcp_info
{
	static constexpr const unsigned default_header_length = 20;
	unsigned header_length;
	uint32_t sequence_number;
	uint32_t ack_number;
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t flags;
	uint16_t window_size;
	uint16_t option_mss;

	// static tcp_info from_buffer(const uint8_t *data, int length);
};

struct wiznet_udp_info
{
	static constexpr const unsigned header_length = 8;
	uint16_t udp_length;
	uint16_t src_port;
	uint16_t dest_port;

	// static udp_info from_buffer(const uint8_t *data, int length);
};




class w5100_device : public device_t, public device_network_interface
{
public:

	w5100_device(machine_config const& mconfig, char const *tag, device_t *owner, u32 clock);

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t data);

	auto irq_handler() { return m_irq_handler.bind(); }




	// called from sockets
	constexpr unsigned rtr() const { return m_rtr ? m_rtr : 1; }
	constexpr unsigned rcr() const { return m_rcr; }

	void update_ethernet_irq();

	void update_socket_tx_bufsize();
	void update_socket_rx_bufsize();

	void send_or_queue(uint8_t *buffer, int length);
	bool busy(void)
	{
		return m_send_timer->enabled();
	}


	void copy_from_tx_buffer(int sn, unsigned offset, uint8_t *buffer, int length) const;
	void copy_to_rx_buffer(int sn, unsigned offset, const uint8_t *buffer, int length);


	// igmp support...
	void send_igmp_join(const uint8_t *mac, wiznet_ip_info &ip, unsigned version);
	void send_igmp_leave(const uint8_t *mac, wiznet_ip_info &ip, unsigned version);

	// arp functionality...
	bool is_broadcast_ip(uint32_t ip, uint8_t *mac) const;
	uint32_t arp_ip(uint32_t ip) const;
	void send_arp_request(uint32_t ip);

	void build_ip_header(uint8_t *frame, const uint8_t *mac, wiznet_ip_info &, int data_length);
	void build_udp_header(uint8_t *frame, const uint8_t *mac, wiznet_ip_info &, wiznet_udp_info &, int data_length);
	void build_tcp_header(uint8_t *frame, const uint8_t *mac, wiznet_ip_info &, wiznet_tcp_info &, int data_length);


protected:


	w5100_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void recv_cb(u8 *buffer, int length) override;
	virtual void send_complete_cb(int result) override;

private:




	void update_rmsr();
	void update_tmsr();

	void write_register(offs_t offset, uint8_t data);
	uint8_t read_register(offs_t offset);


	void process_arp_request(const uint8_t *arp, int length);
	void process_arp_reply(const uint8_t *arp, int length);
	void process_icmp_unreachable(const uint8_t *data, int length);

	void send_icmp_reply(const uint8_t *mac, const wiznet_ip_info &ip, const uint8_t *icmp, int length);
	void send_icmp_unreachable(const uint8_t *mac, const wiznet_ip_info &ip, const uint8_t *);



	required_device_array<w5100_socket_device, 4> m_sockets;

	unsigned m_rx_buffer_size[4];
	unsigned m_rx_buffer_offset[4];
	unsigned m_tx_buffer_size[4];
	unsigned m_tx_buffer_offset[4];

	std::unique_ptr<uint8_t[]> m_rx_buffer;
	std::unique_ptr<uint8_t[]> m_tx_buffer;
	std::vector<uint8_t> m_frame_queue;

	uint16_t m_idm;
	uint16_t m_identification;
	uint32_t m_irq_state;

	devcb_write_line m_irq_handler;

	// common registers
	uint8_t m_mr;
	uint32_t m_gateway;
	uint32_t m_subnet;
	uint32_t m_ip;
	uint8_t m_shar[6];
	uint8_t m_ir;
	uint8_t m_imr;
	uint16_t m_rtr;
	uint8_t m_rcr;
	uint8_t m_rmsr;
	uint8_t m_tmsr;
	uint8_t m_ptimer;
	uint8_t m_pmagic;

	uint32_t m_uipr;
	uint16_t m_uport;

};


DECLARE_DEVICE_TYPE(W5100, w5100_device)

#endif
