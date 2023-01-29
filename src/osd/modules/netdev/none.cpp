// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * none.c
 *
 */
#include "netdev_module.h"

#include "modules/osdmodule.h"


namespace osd {

namespace {

class netdev_none : public osd_module, public netdev_module
{
public:
	netdev_none() : osd_module(OSD_NETDEV_PROVIDER, "none"), netdev_module()
	{
	}

	virtual ~netdev_none() { }
	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
};

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(NETDEV_NONE, osd::netdev_none)
