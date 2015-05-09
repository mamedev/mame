// license:BSD-3-Clause
// copyright-holders:Carl
#if defined(OSD_NET_USE_TAPTUN)

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include "emu.h"
#include "osdnet.h"
#include "modules/osdmodule.h"
#include "netdev_module.h"

#ifdef __linux__
#define IFF_TAP     0x0002
#define IFF_NO_PI   0x1000
#define TUNSETIFF     _IOW('T', 202, int)
#endif

class taptun_module : public osd_module, public netdev_module
{
public:

	taptun_module()
	: osd_module(OSD_NETDEV_PROVIDER, "taptun"), netdev_module()
	{
	}
	virtual ~taptun_module() { }

	virtual int init(const osd_options &options);
	virtual void exit();

	virtual bool probe() { return true; }
};



class netdev_tap : public osd_netdev
{
public:
	netdev_tap(const char *name, class device_network_interface *ifdev, int rate);
	~netdev_tap();

	int send(UINT8 *buf, int len);
	void set_mac(const char *mac);
protected:
	int recv_dev(UINT8 **buf);
private:
	int m_fd;
	char m_ifname[10];
	char m_mac[6];
	UINT8 m_buf[2048];
};

netdev_tap::netdev_tap(const char *name, class device_network_interface *ifdev, int rate)
	: osd_netdev(ifdev, rate)
{
#ifdef __linux__
	struct ifreq ifr;

	m_fd = -1;
	if((m_fd = open("/dev/net/tun", O_RDWR)) == -1) {
		osd_printf_verbose("tap: open failed %d\n", errno);
		return;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	sprintf(ifr.ifr_name, "tap-mess-%d-0", getuid());
	if(ioctl(m_fd, TUNSETIFF, (void *)&ifr) == -1) {
		osd_printf_verbose("tap: ioctl failed %d\n", errno);
		close(m_fd);
		m_fd = -1;
		return;
	}
	osd_printf_verbose("netdev_tap: network up!\n");
	strncpy(m_ifname, ifr.ifr_name, 10);
	fcntl(m_fd, F_SETFL, O_NONBLOCK);

#else
	m_fd = -1;
#endif
}

netdev_tap::~netdev_tap()
{
	close(m_fd);
}

void netdev_tap::set_mac(const char *mac)
{
	memcpy(m_mac, mac, 6);
}

int netdev_tap::send(UINT8 *buf, int len)
{
	if(m_fd == -1) return 0;
	len = write(m_fd, buf, len);
	return (len == -1)?0:len;
}

int netdev_tap::recv_dev(UINT8 **buf)
{
	int len;
	if(m_fd == -1) return 0;
	// exit if we didn't receive anything, got an error, got a broadcast or multicast packet,
	// are in promiscuous mode or got a packet with our mac.
	do {
		len = read(m_fd, m_buf, sizeof(m_buf));
	} while((len > 0) && memcmp(get_mac(), m_buf, 6) && !get_promisc() && !(m_buf[0] & 1));
	*buf = m_buf;
	return (len == -1)?0:len;
}

static CREATE_NETDEV(create_tap)
{
	class netdev_tap *dev = global_alloc(netdev_tap(ifname, ifdev, rate));
	return dynamic_cast<osd_netdev *>(dev);
}

int taptun_module::init(const osd_options &options)
{
	add_netdev("tap", "TAP/TUN Device", create_tap);
	return 0;
}

void taptun_module::exit()
{
	clear_netdev();
}


#else
	#include "modules/osdmodule.h"
	#include "netdev_module.h"

	MODULE_NOT_SUPPORTED(taptun_module, OSD_NETDEV_PROVIDER, "taptun")
#endif


MODULE_DEFINITION(NETDEV_TAPTUN, taptun_module)
