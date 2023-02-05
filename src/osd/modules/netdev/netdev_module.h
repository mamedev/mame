// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * netdev_module.h
 *
 */
#ifndef MAME_OSD_NETDEV_NETDEV_MODULE_H
#define MAME_OSD_NETDEV_NETDEV_MODULE_H

#pragma once


//============================================================
//  CONSTANTS
//============================================================

#define OSD_NETDEV_PROVIDER   "networkprovider"

class netdev_module
{
public:
	virtual ~netdev_module() = default;
	// no specific routines below ... may change
};

#endif // MAME_OSD_NETDEV_NETDEV_MODULE_H
