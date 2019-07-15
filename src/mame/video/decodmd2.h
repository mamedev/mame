// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball DMD Type 2 Display
 */

#ifndef MAME_VIDEO_DECODMD2_H
#define MAME_VIDEO_DECODMD2_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "video/mc6845.h"


class decodmd_type2_device : public device_t
{
public:
	template <typename T>
	decodmd_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&gfxregion_tag)
		: decodmd_type2_device(mconfig, tag, owner, clock)
	{
		set_gfxregion(std::forward<T>(gfxregion_tag));
	}

	decodmd_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(crtc_address_w);
	DECLARE_WRITE8_MEMBER(crtc_register_w);
	DECLARE_READ8_MEMBER(crtc_status_r);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_READ8_MEMBER(busy_r);
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_READ8_MEMBER(ctrl_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(status_w);

	template <typename T> void set_gfxregion(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

	void decodmd2_map(address_map &map);
protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	uint16_t start_address() const { return ((m_crtc_reg[0x0c] << 8) & 0x3f00) | (m_crtc_reg[0x0d] & 0xff); }

private:
	required_device<cpu_device> m_cpu;
	required_device<mc6845_device> m_mc6845;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_memory_bank m_rambank;
	required_device<ram_device> m_ram;
	required_region_ptr<uint8_t> m_rom;

	uint8_t m_crtc_index;
	uint8_t m_crtc_reg[0x100];
	uint8_t m_latch;
	uint8_t m_status;
	uint8_t m_ctrl;
	uint8_t m_busy;
	uint8_t m_command;

	TIMER_DEVICE_CALLBACK_MEMBER(dmd_firq);
	MC6845_UPDATE_ROW(crtc_update_row);
};

DECLARE_DEVICE_TYPE(DECODMD2, decodmd_type2_device)

#endif // MAME_VIDEO_DECODMD2_H
