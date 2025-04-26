// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * netdev_module.h
 *
 */
#ifndef MAME_OSD_NETDEV_NETDEV_MODULE_H
#define MAME_OSD_NETDEV_NETDEV_MODULE_H

#pragma once

#include "interface/nethandler.h"

#include <memory>
#include <vector>


//============================================================
//  CONSTANTS
//============================================================

#define OSD_NETDEV_PROVIDER   "networkprovider"

class netdev_module
{
public:
	virtual ~netdev_module() = default;

	virtual std::unique_ptr<osd::network_device> open_device(int id, osd::network_handler &handler) = 0;
	virtual std::vector<osd::network_device_info> list_devices() = 0;
};

#endif // MAME_OSD_NETDEV_NETDEV_MODULE_H
