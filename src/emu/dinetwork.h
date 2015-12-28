// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic
#ifndef __DINETWORK_H__
#define __DINETWORK_H__

class osd_netdev;

class device_network_interface : public device_interface
{
public:
	device_network_interface(const machine_config &mconfig, device_t &device, float bandwidth);
	virtual ~device_network_interface();

	void set_interface(int id);
	void set_promisc(bool promisc);
	void set_mac(const char *mac);

	const char *get_mac() { return m_mac; }
	bool get_promisc() { return m_promisc; }
	int get_interface() { return m_intf; }

	int send(UINT8 *buf, int len) const;
	virtual void recv_cb(UINT8 *buf, int len);

protected:
	bool m_promisc;
	char m_mac[6];
	float m_bandwidth;
	std::unique_ptr<osd_netdev> m_dev;
	int m_intf;
};


// iterator
typedef device_interface_iterator<device_network_interface> network_interface_iterator;

#endif
