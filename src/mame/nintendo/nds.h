// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, R. Belmont
#pragma once

#ifndef MAME_NINTENDO_NDS_H
#define MAME_NINTENDO_NDS_H

#include "cpu/arm7/arm7.h"
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

	void nds(machine_config &config);

private:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	// ARM7
	uint32_t arm7_io_r(offs_t offset, uint32_t mem_mask = ~0);
	void arm7_io_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// ARM9
	uint32_t arm9_io_r(offs_t offset, uint32_t mem_mask = ~0);
	void arm9_io_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t wram_first_half_r(offs_t offset);
	uint32_t wram_second_half_r(offs_t offset);
	void wram_first_half_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void wram_second_half_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t wram_arm7mirror_r(offs_t offset);
	void wram_arm7mirror_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void nds7_wram_map(address_map &map) ATTR_COLD;
	void nds9_wram_map(address_map &map) ATTR_COLD;
	void nds_arm7_map(address_map &map) ATTR_COLD;
	void nds_arm9_map(address_map &map) ATTR_COLD;

	required_device<arm7_cpu_device> m_arm7;
	required_device<arm946es_cpu_device> m_arm9;
	required_region_ptr<uint32_t> m_firmware;
	required_device<address_map_bank_device> m_arm7wrambnk, m_arm9wrambnk;
	required_shared_ptr<uint32_t> m_arm7ram;

	enum {
		TIMER_OFFSET = (0x100/4),
		RTC_OFFSET = (0x138/4),
		IPCSYNC_OFFSET = (0x180/4),
		AUX_SPI_CNT_OFFSET = (0x1a0/4),
		GAMECARD_BUS_CTRL_OFFSET = (0x1a4/4),
		GAMECARD_DATA_OFFSET = (0x1a8/4),
		GAMECARD_DATA_2_OFFSET = (0x1ac/4),
		SPI_CTRL_OFFSET = (0x1c0/4),
		IME_OFFSET = (0x208/4),
		IE_OFFSET = (0x210/4),
		IF_OFFSET = (0x214/4),
		WRAMSTAT_OFFSET = (0x241/4),
		VRAMCNT_A_OFFSET = (0x240/4),
		WRAMCNT_OFFSET = (0x244/4),
		VRAMCNT_H_OFFSET = (0x248/4),
		POSTFLG_OFFSET = (0x300/4),
		GAMECARD_DATA_IN_OFFSET = (0x100010/4),
		POSTFLG_PBF_SHIFT = 0,
		POSTFLG_RAM_SHIFT = 1,
		POSTFLG_PBF_MASK = (1 << POSTFLG_PBF_SHIFT),
		POSTFLG_RAM_MASK = (1 << POSTFLG_RAM_SHIFT),
		GAMECARD_DATA_READY = (1 << 23),
		GAMECARD_BLOCK_BUSY = (1 << 31)
	};

	uint32_t m_arm7_postflg = 0;
	uint32_t m_arm9_postflg = 0;
	uint32_t m_gamecard_ctrl = 0, m_cartdata_len = 0;
	uint32_t m_ime[2]{}, m_ie[2]{}, m_if[2]{};
	uint16_t m_arm7_ipcsync = 0, m_arm9_ipcsync = 0, m_spicnt = 0;
	uint8_t m_WRAM[0x8000]{};
	uint8_t m_wramcnt = 0;
	uint8_t m_vramcnta = 0, m_vramcntb = 0, m_vramcntc = 0, m_vramcntd = 0;
	uint8_t m_vramcnte = 0, m_vramcntf = 0, m_vramcntg = 0, m_vramcnth = 0, m_vramcnti = 0;
	bool m_arm7halted = false;

	// DMA
	emu_timer *m_dma_timer[8]{};
	//uint32_t m_dma_src[8]{};
	//uint32_t m_dma_dst[8]{};
	//uint16_t m_dma_cnt[8]{};

	// Timers
	uint32_t m_timer_regs[8]{};
	uint16_t m_timer_reload[8]{};
	int m_timer_recalc[8]{};
	double m_timer_hz[8]{};

	emu_timer *m_tmr_timer[8]{}, *m_irq_timer = nullptr;

	TIMER_CALLBACK_MEMBER(dma_complete);
	TIMER_CALLBACK_MEMBER(timer_expire);
	TIMER_CALLBACK_MEMBER(handle_irq);

	void request_irq(int which_cpu, uint32_t int_type);
	void dma_exec(int ch);
};

#endif // MAME_NINTENDO_NDS_H
