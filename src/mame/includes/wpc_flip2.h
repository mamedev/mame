// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpc_flip2.h
 *
 */

#ifndef MAME_INCLUDES_WPC_FLIP2_H
#define MAME_INCLUDES_WPC_FLIP2_H

#pragma once

#include "includes/wpc_flip1.h"

class wpc_flip2_state : public wpc_flip1_state
{
public:
	wpc_flip2_state(const machine_config &mconfig, device_type type, const char *tag)
		: wpc_flip1_state(mconfig, type, tag)
	{ }

	void init_wpc_flip2();
	void wpc_flip2(machine_config &config);

protected:
	void wpc_flip2_map(address_map &map);
};

#endif // MAME_INCLUDES_WPC_FLIP2_H
