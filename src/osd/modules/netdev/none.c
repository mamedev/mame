/*
 * none.c
 *
 */

#include "netdev_module.h"
#include "modules/osdmodule.h"

class netdev_none : public osd_module, public netdev_module
{
public:
	netdev_none()
	: osd_module(OSD_NETDEV_PROVIDER, "none"), netdev_module()
	{
	}
};

MODULE_DEFINITION(NETDEV_NONE, netdev_none)
