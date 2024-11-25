// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_TSENG_H
#define MAME_VIDEO_PC_VGA_TSENG_H

#pragma once

#include "video/pc_vga.h"

#include "screen.h"


class tseng_vga_device :  public svga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	tseng_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

protected:
	tseng_vga_device(const machine_config &mconfig, const char *tag, device_type type, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	virtual void io_3bx_3dx_map(address_map &map) override ATTR_COLD;
	virtual void io_3cx_map(address_map &map) override ATTR_COLD;

	u8 ramdac_hidden_mask_r(offs_t offset);
	void ramdac_hidden_mask_w(offs_t offset, u8 data);
	u8 ramdac_hidden_windex_r(offs_t offset);

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;
	virtual void attribute_map(address_map &map) override ATTR_COLD;

	virtual void recompute_params() override;

	virtual uint32_t latch_start_addr() override;

	struct
	{
		uint8_t reg_3d8;
		uint8_t dac_ctrl;
		uint8_t dac_state;
		uint8_t horz_overflow;
		uint8_t aux_ctrl;
		bool ext_reg_ena;
		uint8_t misc1;
		uint8_t misc2;
		uint8_t crtc_reg31;
		uint8_t crtc_overflow_high;
		uint8_t crtc_ext_start;
		uint8_t rcconf;
		uint8_t vsconf1;
		uint8_t vsconf2;
	}et4k;
};

class et4kw32i_vga_device :  public tseng_vga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	et4kw32i_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 acl_index_r(offs_t offset);
	void acl_index_w(offs_t offset, u8 data);
	u8 acl_data_r(offs_t offset);
	void acl_data_w(offs_t offset, u8 data);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void io_3cx_map(address_map &map) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

private:
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_acl_space_config;
	address_space_config m_mmu_space_config;

	void acl_map(address_map &map) ATTR_COLD;
	void mmu_map(address_map &map) ATTR_COLD;

	u8 m_acl_idx;

	struct {
		u16 xpos, ypos;
		u32 address;
	}m_crtcb;

	struct {
		u8 control;
	}m_ima;

	template <unsigned N> u8 mmu_blit_r(offs_t offset);
	template <unsigned N> void mmu_blit_w(offs_t offset, u8 data);
	template <unsigned N> u8 mmu_base_address_r(offs_t offset);
	template <unsigned N> void mmu_base_address_w(offs_t offset, u8 data);

	struct {
		u8 control;
		u32 base_address[3];
	}m_mmu;
};


// device type definition
DECLARE_DEVICE_TYPE(TSENG_VGA, tseng_vga_device)
DECLARE_DEVICE_TYPE(ET4KW32I_VGA, et4kw32i_vga_device)

#endif // MAME_VIDEO_PC_VGA_TSENG_H
