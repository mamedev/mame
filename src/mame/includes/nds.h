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
		m_arm7(*this, "arm7"),
		m_arm9(*this, "arm9"),
		m_firmware(*this, "firmware")
	{ }

	void machine_start() override;
	void machine_reset() override;

	// ARM7
	DECLARE_READ32_MEMBER(arm7_io_r);
	DECLARE_WRITE32_MEMBER(arm7_io_w);

	// ARM9
	DECLARE_READ32_MEMBER(arm9_io_r);
	DECLARE_WRITE32_MEMBER(arm9_io_w);

protected:
	required_device<arm7_cpu_device> m_arm7;
	required_device<arm946es_cpu_device> m_arm9;

	required_region_ptr<uint32_t> m_firmware;

	enum {
		POSTFLG_OFFSET = 0x300/4,
		POSTFLG_PBF_SHIFT = 0,
		POSTFLG_RAM_SHIFT = 1,
		POSTFLG_PBF_MASK = (1 << POSTFLG_PBF_SHIFT),
		POSTFLG_RAM_MASK = (1 << POSTFLG_RAM_SHIFT),
	};

	uint32_t m_arm7_postflg;
	uint32_t m_arm9_postflg;
};

#endif // INCLUDES_NDS_H
