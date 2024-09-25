// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1000C

*/

#ifndef MAME_CPU_TMS1000_TMS1000C_H
#define MAME_CPU_TMS1000_TMS1000C_H

#pragma once

#include "tms1000.h"


class tms1000c_cpu_device : public tms1000_cpu_device
{
public:
	tms1000c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual u32 decode_micro(offs_t offset) override;
};


DECLARE_DEVICE_TYPE(TMS1000C, tms1000c_cpu_device)

#endif // MAME_CPU_TMS1000_TMS1000C_H
