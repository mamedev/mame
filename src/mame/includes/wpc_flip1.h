// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpc_flip1.h
 *
 */

#ifndef WPC_FLIP1_H_
#define WPC_FLIP1_H_

#include "includes/wpc_dot.h"

class wpc_flip1_state : public wpc_dot_state
{
public:
	wpc_flip1_state(const machine_config &mconfig, device_type type, const char *tag)
		: wpc_dot_state(mconfig, type, tag)
	{ }
public:
	DECLARE_DRIVER_INIT(wpc_flip1);
};

#endif /* WPC_FLIP1_H_ */
