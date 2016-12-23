// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpc_flip2.h
 *
 */

#ifndef WPC_FLIP2_H_
#define WPC_FLIP2_H_

#include "includes/wpc_flip1.h"

class wpc_flip2_state : public wpc_flip1_state
{
public:
	wpc_flip2_state(const machine_config &mconfig, device_type type, const char *tag)
		: wpc_flip1_state(mconfig, type, tag)
	{ }
public:
	DECLARE_DRIVER_INIT(wpc_flip2);
};

#endif /* WPC_FLIP2_H_ */
