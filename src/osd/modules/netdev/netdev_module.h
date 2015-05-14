// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * netdev_module.h
 *
 */

#ifndef NETDEV_MODULE_H_
#define NETDEV_MODULE_H_

#include "osdepend.h"
#include "modules/osdmodule.h"

//============================================================
//  CONSTANTS
//============================================================

#define OSD_NETDEV_PROVIDER   "netdevprovider"

class netdev_module
{
public:
	virtual ~netdev_module() { }
	// no specific routines below ... may change
};


#endif /* NETDEV_MODULE_H_ */
