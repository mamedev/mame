// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef INCLUDES_NDS_H
#define INCLUDES_NDS_H

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

class nds_state : public driver_device
{
public:
	nds_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_arm9(*this, "arm9"),
		m_arm7(*this, "arm7"),
		m_firmware(*this, "firmware")
	{ }

	required_device<arm946es_cpu_device> m_arm9;
	required_device<arm7_cpu_device> m_arm7;

	void machine_start() override;
	void machine_reset() override;

protected:
	required_region_ptr<uint32_t> m_firmware;
};

#endif // INCLUDES_NDS_H
