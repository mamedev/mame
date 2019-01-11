// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic
#ifndef MAME_EMU_DINETWORK_H
#define MAME_EMU_DINETWORK_H

class osd_netdev;

class device_network_interface : public device_interface
{
public:
	device_network_interface(const machine_config &mconfig, device_t &device, float bandwidth);
	virtual ~device_network_interface();

	void interface_pre_start() override;

	void set_interface(int id);
	void set_promisc(bool promisc);
	void set_mac(const char *mac);

	const char *get_mac() const { return m_mac; }
	bool get_promisc() const { return m_promisc; }
	int get_interface() const { return m_intf; }

	int send(u8 *buf, int len) const;

	// TODO: de-virtualise this when existing devices implement delayed receive
	virtual void recv_cb(u8 *buf, int len);

	// delayed transmit/receive handlers
	virtual void send_complete_cb(int result) {}
	virtual int recv_start_cb(u8 *buf, int len) { return 0; }
	virtual void recv_complete_cb(int result) {}

protected:
	TIMER_CALLBACK_MEMBER(send_complete);
	TIMER_CALLBACK_MEMBER(recv_complete);

	bool m_promisc;
	char m_mac[6];
	float m_bandwidth;
	std::unique_ptr<osd_netdev> m_dev;
	int m_intf;

	emu_timer *m_send_timer;
	emu_timer *m_recv_timer;
};


// iterator
typedef device_interface_iterator<device_network_interface> network_interface_iterator;

#endif // MAME_EMU_DINETWORK_H
