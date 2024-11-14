// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    ARM7TDMI CPU component of the AP2010 LSI

***************************************************************************/

#ifndef MAME_CPU_ARM7_AP2010CPU_H
#define MAME_CPU_ARM7_AP2010CPU_H

#pragma once

#include "arm7.h"

class ap2010cpu_device : public arm7_cpu_device
{
public:
	ap2010cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void add_hotspot(offs_t pc);

protected:
	// device_execute_interface overrides
	virtual void execute_run() override;

private:
	uint32_t m_hotspot_select = 0;
	uint32_t m_hotspot[ARM7_MAX_HOTSPOTS];
};

// device type definition
DECLARE_DEVICE_TYPE(AP2010CPU, ap2010cpu_device)

#endif // MAME_CPU_ARM7_AP2010CPU_H
