// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A5900 MCU

*/

#ifndef MAME_CPU_RW5000_A5900_H
#define MAME_CPU_RW5000_A5900_H

#pragma once

#include "a5000.h"


class a5900_cpu_device : public a5000_cpu_device
{
public:
	a5900_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	void program_512x8(address_map &map) ATTR_COLD;

	// opcode handlers
	virtual void op_read() override;
};


DECLARE_DEVICE_TYPE(A5900, a5900_cpu_device)

#endif // MAME_CPU_RW5000_A5900_H
