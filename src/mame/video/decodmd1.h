// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Data East Pinball DMD Type 1 display
 */

#ifndef MAME_VIDEO_DECODMD1_H
#define MAME_VIDEO_DECODMD1_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/ram.h"

#define MCFG_DECODMD_TYPE1_ADD(_tag, _region) \
	MCFG_DEVICE_ADD(_tag, DECODMD1, 0) \
	decodmd_type1_device::static_set_gfxregion(*device, _region);

class decodmd_type1_device : public device_t
{
public:
	decodmd_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	required_device<cpu_device> m_cpu;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_device<ram_device> m_ram;
	memory_region* m_rom;

	uint32_t screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );

	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_READ8_MEMBER(busy_r);
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_READ8_MEMBER(ctrl_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(status_w);
	DECLARE_READ8_MEMBER(dmd_port_r);
	DECLARE_WRITE8_MEMBER(dmd_port_w);
	TIMER_DEVICE_CALLBACK_MEMBER(dmd_nmi);

	static void static_set_gfxregion(device_t &device, const char *tag);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
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
	uint8_t m_bank;
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
	const char* m_gfxtag;

	void output_data();
	void set_busy(uint8_t input, uint8_t val);
};

DECLARE_DEVICE_TYPE(DECODMD1, decodmd_type1_device)


#endif // MAME_VIDEO_DECODMD1_H
