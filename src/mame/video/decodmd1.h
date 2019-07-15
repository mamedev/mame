// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Data East Pinball DMD Type 1 display
 */

#ifndef MAME_VIDEO_DECODMD1_H
#define MAME_VIDEO_DECODMD1_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/ram.h"
#include "machine/timer.h"


class decodmd_type1_device : public device_t
{
public:
	template <typename T>
	decodmd_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&gfxregion_tag)
		: decodmd_type1_device(mconfig, tag, owner, clock)
	{
		set_gfxregion(std::forward<T>(gfxregion_tag));
	}

	decodmd_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_READ8_MEMBER(busy_r);
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_READ8_MEMBER(ctrl_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(dmd_port_r);
	DECLARE_WRITE8_MEMBER(dmd_port_w);

	DECLARE_WRITE_LINE_MEMBER(blank_w);
	DECLARE_WRITE_LINE_MEMBER(status_w);
	DECLARE_WRITE_LINE_MEMBER(rowdata_w);
	DECLARE_WRITE_LINE_MEMBER(rowclock_w);
	DECLARE_WRITE_LINE_MEMBER(test_w);

	template <typename T> void set_gfxregion(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

	void decodmd1_io_map(address_map &map);
	void decodmd1_map(address_map &map);
protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static constexpr unsigned B_CLR = 0x01;
	static constexpr unsigned B_SET = 0x02;
	static constexpr unsigned B_CLK = 0x04;

	uint8_t m_latch;
	uint8_t m_status;
	uint8_t m_ctrl;
	uint8_t m_busy;
	uint8_t m_command;
	uint8_t m_rowclock;
	uint8_t m_rowdata;
	uint32_t m_rowselect;
	uint8_t m_blank;
	uint32_t m_pxdata1;
	uint32_t m_pxdata2;
	uint32_t m_pxdata1_latched;
	uint32_t m_pxdata2_latched;
	bool m_frameswap;
	uint32_t m_pixels[0x200];
	uint8_t m_busy_lines;
	uint32_t m_prevrow;

	required_device<cpu_device> m_cpu;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_device<ram_device> m_ram;
	required_device<hc259_device> m_bitlatch;
	required_region_ptr<uint8_t> m_rom;

	void output_data();
	void set_busy(uint8_t input, uint8_t val);
	uint32_t screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	TIMER_DEVICE_CALLBACK_MEMBER(dmd_nmi);
};

DECLARE_DEVICE_TYPE(DECODMD1, decodmd_type1_device)


#endif // MAME_VIDEO_DECODMD1_H
