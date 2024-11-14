// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_SIS_H
#define MAME_VIDEO_PC_VGA_SIS_H

#pragma once

#include "video/pc_vga.h"

class sis6236_vga_device : public svga_device
{
public:
	sis6236_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

protected:
	sis6236_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void io_3cx_map(address_map &map) override ATTR_COLD;

	virtual void sequencer_map(address_map &map) override ATTR_COLD;

	virtual uint16_t offset() override;
	virtual void recompute_params() override;

	u8 m_ramdac_mode = 0;
	u8 m_ext_misc_ctrl[10]{};
	//u16 m_ext_config_status = 0;
	u8 m_ext_scratch[5]{};
	u8 m_ext_vert_overflow = 0;
	u8 m_ext_horz_overflow[2]{};
	u8 m_bus_width = 0;
	u8 m_ext_dclk[3]{};
	u8 m_ext_eclk[3]{};
	u8 m_ext_clock_gen = 0;
	u8 m_ext_clock_source_select = 0;
	bool m_unlock_reg = false;

	std::tuple<u8, u8> flush_true_color_mode();
//  bool m_dual_seg_mode = false;
};

class sis630_vga_device : public sis6236_vga_device
{
public:
	sis630_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
//  virtual void device_start() override ATTR_COLD;
//  virtual void device_reset() override ATTR_COLD;

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SIS6236_VGA, sis6236_vga_device)
DECLARE_DEVICE_TYPE(SIS630_VGA, sis630_vga_device)

#endif // MAME_VIDEO_PC_VGA_SIS_H
