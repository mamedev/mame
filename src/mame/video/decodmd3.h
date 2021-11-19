// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball DMD Type 3 Display
 */

#ifndef MAME_VIDEO_DECODMD3_H
#define MAME_VIDEO_DECODMD3_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "video/mc6845.h"


class decodmd_type3_device : public device_t
{
public:
	template <typename T>
	decodmd_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&gfxregion_tag)
		: decodmd_type3_device(mconfig, tag, owner, clock)
	{
		set_gfxregion(std::forward<T>(gfxregion_tag));
	}

	decodmd_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_gfxregion(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

	void data_w(uint8_t data);
	uint8_t busy_r();
	void ctrl_w(uint8_t data);
	uint16_t status_r();

	void decodmd3_map(address_map &map);
protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_cpu;
	required_device<mc6845_device> m_mc6845;
	required_memory_bank m_rombank;
	required_shared_ptr<uint16_t> m_ram;

	required_region_ptr<uint16_t> m_rom;

	uint8_t m_status;
	uint8_t m_crtc_index;
	uint8_t m_crtc_reg[0x100];
	uint8_t m_latch;
	uint8_t m_ctrl;
	uint8_t m_busy;
	uint8_t m_command;

	TIMER_DEVICE_CALLBACK_MEMBER(dmd_irq);
	MC6845_UPDATE_ROW(crtc_update_row);

	uint16_t latch_r();
	void status_w(uint16_t data);
	void crtc_address_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void crtc_register_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t crtc_status_r(offs_t offset, uint16_t mem_mask = ~0);
};

DECLARE_DEVICE_TYPE(DECODMD3, decodmd_type3_device)

#endif // MAME_VIDEO_DECODMD3_H
