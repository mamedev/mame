// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#pragma once
#ifndef INCLUDES_NDS_H
#define INCLUDES_NDS_H

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/bankdev.h"
#include "machine/timer.h"

class nds_state : public driver_device
{
public:
	nds_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_arm7(*this, "arm7"),
		m_arm9(*this, "arm9"),
		m_firmware(*this, "firmware"),
		m_arm7wrambnk(*this, "nds7wram"),
		m_arm9wrambnk(*this, "nds9wram"),
		m_arm7ram(*this, "arm7ram")
	{ }

	void machine_start() override;
	void machine_reset() override;

	// ARM7
	DECLARE_READ32_MEMBER(arm7_io_r);
	DECLARE_WRITE32_MEMBER(arm7_io_w);

	// ARM9
	DECLARE_READ32_MEMBER(arm9_io_r);
	DECLARE_WRITE32_MEMBER(arm9_io_w);

	DECLARE_READ32_MEMBER(wram_first_half_r);
	DECLARE_READ32_MEMBER(wram_second_half_r);
	DECLARE_WRITE32_MEMBER(wram_first_half_w);
	DECLARE_WRITE32_MEMBER(wram_second_half_w);
	DECLARE_READ32_MEMBER(wram_arm7mirror_r);
	DECLARE_WRITE32_MEMBER(wram_arm7mirror_w);

protected:
	required_device<arm7_cpu_device> m_arm7;
	required_device<arm946es_cpu_device> m_arm9;
	required_region_ptr<uint32_t> m_firmware;
	required_device<address_map_bank_device> m_arm7wrambnk, m_arm9wrambnk;
	required_shared_ptr<uint32_t> m_arm7ram;

	enum {
		IPCSYNC_OFFSET = 0x180/4,
		GAMECARD_BUS_CTRL_OFFSET = 0x1a4/4,
		WRAMSTAT_OFFSET = 0x241/4,
		VRAMCNT_A_OFFSET = 0x240/4,
		WRAMCNT_OFFSET = 0x244/4,
		VRAMCNT_H_OFFSET = 0x248/4,
		POSTFLG_OFFSET = 0x300/4,
		POSTFLG_PBF_SHIFT = 0,
		POSTFLG_RAM_SHIFT = 1,
		POSTFLG_PBF_MASK = (1 << POSTFLG_PBF_SHIFT),
		POSTFLG_RAM_MASK = (1 << POSTFLG_RAM_SHIFT),
	};

	uint32_t m_arm7_postflg;
	uint32_t m_arm9_postflg;
	uint16_t m_arm7_ipcsync, m_arm9_ipcsync;
	uint8_t m_WRAM[0x8000];
	uint8_t m_wramcnt;
	uint8_t m_vramcnta, m_vramcntb, m_vramcntc, m_vramcntd, m_vramcnte, m_vramcntf, m_vramcntg, m_vramcnth, m_vramcnti;
};

#endif // INCLUDES_NDS_H
