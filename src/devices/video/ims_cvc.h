// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_VIDEO_IMS_CVC_H
#define MAME_VIDEO_IMS_CVC_H

#pragma once

#include "machine/bankdev.h"
#include "machine/ram.h"

class ims_cvc_device
	: public device_t
	, public device_palette_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// configuration
	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_vram(T &&tag) { m_vram.set_tag(std::forward<T>(tag)); }

	virtual void map(address_map &map) = 0;
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) = 0;

protected:
	ims_cvc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_palette_interface overrides
	virtual u32 palette_entries() const override { return 256; }

	u32 colour_palette_r(const offs_t offset);
	void colour_palette_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU);
	u32 halfsync_r() { return m_halfsync; }
	void halfsync_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_halfsync); }
	u32 backporch_r() { return m_backporch; }
	void backporch_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_backporch); }
	u32 display_r() { return m_display; }
	void display_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_display); }
	u32 shortdisplay_r() { return m_shortdisplay; }
	void shortdisplay_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_shortdisplay); }
	u32 broadpulse_r() { return m_broadpulse; }
	void broadpulse_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_broadpulse); }
	u32 vsync_r() { return m_vsync; }
	void vsync_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_vsync); }
	u32 vblank_r() { return m_vblank; }
	void vblank_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_vblank); }
	u32 vdisplay_r() { return m_vdisplay; }
	void vdisplay_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_vdisplay); }
	u32 linetime_r() { return m_linetime; }
	void linetime_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_linetime); }
	//u32 tos_r() { return m_tos; }
	//void tos_w(const u32 data) { m_tos = data; }
	u32 meminit_r() { return m_meminit; }
	void meminit_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_meminit); }
	u32 transferdelay_r() { return m_transferdelay; }
	void transferdelay_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_transferdelay); }

	u32 mask_r() { return m_mask; }
	void mask_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_mask); }
	u32 tos_r() { return m_tos; }
	void tos_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_tos); }

	virtual void boot_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_boot); }

	required_device<screen_device> m_screen;
	required_device<ram_device> m_vram;

protected:
	// device state
	u32 m_halfsync;
	u32 m_backporch;
	u32 m_display;
	u32 m_shortdisplay;
	u32 m_broadpulse;
	u32 m_vsync;
	u32 m_vblank;
	u32 m_vdisplay;
	u32 m_linetime;
	u32 m_meminit;
	u32 m_transferdelay;

	u32 m_mask;
	u32 m_tos;
	u32 m_boot;
};

class g300_device : public ims_cvc_device
{
public:
	g300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	virtual void device_start() override;

	u32 control_r() { return m_control; }
	void control_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_control); }

private:
	u32 m_control;
};

class g332_device : public ims_cvc_device
{
public:
	g332_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	enum boot_mask : u32
	{
		PLL_MULTIPLIER = 0x000001f, // pll multiplier
		PLL_SELECT     = 0x0000020, // pll clock select
		ALIGN_64       = 0x0000040, // 64 bit address alignment
	};

	enum control_a_mask
	{
		VTG_ENABLE     = 0x000001, // video timing generator enable
		INTL_ENABLE    = 0x000002, // interlace enable
		INTL_FORMAT    = 0x000004, // ccir/eia interlace format
		SLAVE_MODE     = 0x000008, // slave/master operating mode
		SYNC_PATTERN   = 0x000010, // plain/tesselated sync
		SYNC_FORMAT    = 0x000020, // separate/composite sync
		VIDEO_FORMAT   = 0x000040, // video only/composite video + sync
		BLANK_LEVEL    = 0x000080, // blanking pedestal/no blank pedestal
		BLANK_IO       = 0x000100, // CBlank is output/input
		BLANK_FUNC     = 0x000200, // undelayed ClkDisable/delayed CBlank
		BLANK_FORCE    = 0x000400, // screen blanked/no action
		BLANK_DISABLE  = 0x000800, // blanking disabled/enabled
		ADDR_INC       = 0x003000, // VRAM address increment
		DMA_DISABLE    = 0x004000, // DMA disabled/enabled
		SYNC_DELAY     = 0x038000,
		INTERLEAVE     = 0x040000, // interleave enabled/disabled
		SAMPLE_DELAY   = 0x080000,
		PIXEL_BITS     = 0x700000, // bits per pixel
		CURSOR_DISABLE = 0x800000, // cursor disabled/enabled
	};
	enum control_a_addr_inc_mask
	{
		INC_1    = 0x000000,
		INC_256  = 0x001000,
		INC_512  = 0x002000,
		INC_1024 = 0x003000,
	};
	enum control_a_bpp_mask
	{
		BPP_1  = 0x000000,
		BPP_2  = 0x100000,
		BPP_4  = 0x200000,
		BPP_8  = 0x300000,
		BPP_15 = 0x400000,
		BPP_16 = 0x500000,
	};

protected:
	g332_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_palette_interface overrides
	virtual u32 palette_entries() const override { return 256 + 3; }

	virtual void microport_map(address_map &map);

	virtual void boot_w(offs_t offset, u32 data, u32 mem_mask = 0x00ffffffU) override;

	u32 cursor_palette_r(const offs_t offset);
	void cursor_palette_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU);

	u32 vpreequalise_r() { return m_vpreequalise; }
	void vpreequalise_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_vpreequalise); }
	u32 vpostequalise_r() { return m_vpostequalise; }
	void vpostequalise_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_vpostequalise); }
	u32 linestart_r() { return m_linestart; }
	void linestart_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_linestart); }
	u32 control_a_r() { return m_control_a; }
	void control_a_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU);
	u32 control_b_r() { return m_control_b; }
	void control_b_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffffU) { COMBINE_DATA(&m_control_b); }

private:
	required_device<address_map_bank_device> m_microport;

	u32 m_vpreequalise;
	u32 m_vpostequalise;
	u32 m_linestart;
	u32 m_control_a;
	u32 m_control_b;
};

class g364_device : public g332_device
{
public:
	g364_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(G300, g300_device)
DECLARE_DEVICE_TYPE(G332, g332_device)
DECLARE_DEVICE_TYPE(G364, g364_device)

#endif // MAME_VIDEO_IMS_CVC_H
