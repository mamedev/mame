// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    AXC51-CORE (AppoTech Inc.)

    used in

    AX208 SoC

 *****************************************************************************/

#ifndef MAME_CPU_MCS51_AXC51_CORE_H
#define MAME_CPU_MCS51_AXC51_CORE_H

#pragma once

#include "mcs51.h"

DECLARE_DEVICE_TYPE(AX208, ax208_cpu_device)

class ax208_cpu_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	ax208_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

#endif // MAME_CPU_MCS51_AXC51_CORE_H
