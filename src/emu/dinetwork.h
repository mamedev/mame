// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic
#ifndef MAME_EMU_DINETWORK_H
#define MAME_EMU_DINETWORK_H

class osd_netdev;

class device_network_interface : public device_interface
{
public:
	device_network_interface(const machine_config &mconfig, device_t &device, u32 bandwidth, u32 mtu = 1500);
	virtual ~device_network_interface();

	void interface_pre_start() override;
	void interface_post_start() override;

	void set_interface(int id);
	void set_promisc(bool promisc);
	void set_mac(const char *mac);
	void set_loopback(bool loopback);

	const char *get_mac() const { return m_mac; }
	bool get_promisc() const { return m_promisc; }
	int get_interface() const { return m_intf; }

	int send(u8 *buf, int len, int fcs = 0);

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
	// bandwidth in bytes per second
	u32 m_bandwidth;
	// maximum transmission unit, used for device polling time
	u32 m_mtu;
	std::unique_ptr<osd_netdev> m_dev;
	int m_intf;
	bool m_loopback_control;

	emu_timer *m_send_timer;
	emu_timer *m_recv_timer;
};


// iterator
typedef device_interface_enumerator<device_network_interface> network_interface_enumerator;

#endif // MAME_EMU_DINETWORK_H
