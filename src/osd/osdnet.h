// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_OSD_OSDNET_H
#define MAME_OSD_OSDNET_H

#pragma once

#include "osdcomm.h"

#include <algorithm>
#include <array>
#include <memory>
#include <vector>


namespace osd { class network_handler; }
class osd_network_device;

#define CREATE_NETDEV(name) osd_network_device *name(const char *ifname, osd::network_handler &ifdev)
typedef osd_network_device *(*create_netdev)(const char *ifname, osd::network_handler &ifdev);

class osd_network_device
{
public:
	struct entry_t
	{
		entry_t()
		{
			std::fill(std::begin(name), std::end(name), 0);
			std::fill(std::begin(description), std::end(description), 0);
		}

		int id = 0;
		char name[256];
		char description[256];
		create_netdev func = nullptr;
	};

	osd_network_device(osd::network_handler &ifdev);
	virtual ~osd_network_device();

	void start();
	void stop();
	void poll();

	virtual int send(uint8_t *buf, int len);
	virtual void set_mac(const uint8_t *mac);
	virtual void set_promisc(bool promisc);

	const std::array<uint8_t, 6> &get_mac();
	bool get_promisc();

protected:
	virtual int recv_dev(uint8_t **buf);

private:
	osd::network_handler &m_dev;
	bool m_stopped;
};

osd_network_device *open_netdev(int id, osd::network_handler &ifdev);
void add_netdev(const char *name, const char *description, create_netdev func);
void clear_netdev();
const std::vector<std::unique_ptr<osd_network_device::entry_t>>& get_netdev_list();

#endif // MAME_OSD_OSDNET_H
