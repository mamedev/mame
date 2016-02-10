// license:BSD-3-Clause
// copyright-holders:Couriersud
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

	virtual int init(const osd_options &options) override { return 0; }
};

MODULE_DEFINITION(NETDEV_NONE, netdev_none)
