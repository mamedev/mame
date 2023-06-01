// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_SIS_H
#define MAME_VIDEO_SIS_H

#pragma once

#include "video/pc_vga.h"

class sis630_svga_device : public svga_device
{
public:
	sis630_svga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual u8 port_03c0_r(offs_t offset) override;
	virtual void port_03c0_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint8_t crtc_reg_read(uint8_t index) override;
	virtual void crtc_reg_write(uint8_t index, uint8_t data) override;
	virtual uint8_t seq_reg_read(uint8_t index) override;
	virtual void seq_reg_write(uint8_t index, uint8_t data) override;
	virtual uint16_t offset() override;
	virtual void recompute_params() override;

	u8 m_crtc_ext_regs[0x100]{};
	u8 m_seq_ext_regs[0x100]{};
	u8 m_ramdac_mode = 0;
	u8 m_ext_misc_ctrl_0 = 0;
	u8 m_ext_vert_overflow = 0;
	u8 m_ext_horz_overflow[2]{};
	u32 m_svga_bank_reg_w = 0;
	u32 m_svga_bank_reg_r = 0;
	bool m_unlock_reg = false;

	std::tuple<u8, u8> flush_true_color_mode();
//  bool m_dual_seg_mode = false;
};

DECLARE_DEVICE_TYPE(SIS630_SVGA, sis630_svga_device)

#endif // MAME_VIDEO_SIS_H
