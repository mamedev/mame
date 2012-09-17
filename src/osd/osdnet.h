#include "emu.h"

#ifndef __OSDNET_H__
#define __OSDNET_H__

class netdev;

#define CREATE_NETDEV(name) class netdev *name(const char *ifname, class device_network_interface *ifdev, int rate)
typedef class netdev *(*create_netdev)(const char *ifname, class device_network_interface *ifdev, int rate);

struct netdev_entry_t
{
	int id;
	char name[256];
	char description[256];
	create_netdev func;
	netdev_entry_t *m_next;
};

class netdev
{
public:
	netdev(class device_network_interface *ifdev, int rate);
	virtual ~netdev();

	virtual int send(UINT8 *buf, int len);
	virtual void set_mac(const char *mac);
	virtual void set_promisc(bool promisc);

	const char *get_mac();
	bool get_promisc();

protected:
	virtual int recv_dev(UINT8 **buf);

private:
	void recv(void *ptr, int param);

	class device_network_interface *m_dev;
};

class netdev *open_netdev(int id, class device_network_interface *ifdev, int rate);
void add_netdev(const char *name, const char *description, create_netdev func);
void clear_netdev();
const netdev_entry_t *netdev_first();
int netdev_count();
#endif
