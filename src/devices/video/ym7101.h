// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_YM7101_H
#define MAME_VIDEO_YM7101_H

#pragma once

#include "machine/timer.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ym7101_device : public device_t,
					  public device_memory_interface,
					  public device_video_interface,
					  public device_gfx_interface,
					  public device_mixer_interface
{
public:
	ym7101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void if16_map(address_map &map) ATTR_COLD;
	void if8_map(address_map &map) ATTR_COLD;

	auto vint_cb() { return m_vint_callback.bind(); }
	auto hint_cb() { return m_hint_callback.bind(); }
	auto sint_cb() { return m_sint_callback.bind(); }
	auto dtack_cb() { return m_dtack_cb.bind(); }
	auto mreq_cb() { return m_mreq_cb.bind(); }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void irq_ack()
	{
		if (machine().side_effects_disabled())
			return;

		if (m_vint_pending)
		{
			m_vint_pending = 0;
			m_vint_callback(0);
		}
		else if (m_hint_pending)
		{
			m_hint_pending = 0;
			m_hint_callback(0);
		}
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

//	virtual u32 palette_entries() const noexcept override { return 0x40 + 0x40 * 2; }
private:
	void vram_map(address_map &map) ATTR_COLD;
	void cram_map(address_map &map) ATTR_COLD;
	void vsram_map(address_map &map) ATTR_COLD;
	void regs_map(address_map &map) ATTR_COLD;

	const address_space_config m_space_vram_config;
	const address_space_config m_space_cram_config;
	const address_space_config m_space_vsram_config;
	const address_space_config m_space_regs_config;

	required_shared_ptr<u16> m_vram;
	required_shared_ptr<u16> m_cram;
	required_shared_ptr<u16> m_vsram;

	devcb_write_line m_vint_callback;
	devcb_write_line m_hint_callback;
	devcb_write_line m_sint_callback;
	devcb_read16     m_mreq_cb;
	devcb_write_line m_dtack_cb;

	required_device<palette_device> m_palette;
	required_device<segapsg_device> m_psg;

	std::unique_ptr<u16[]> m_sprite_cache;
	std::unique_ptr<u8[]> m_sprite_line;

	TIMER_CALLBACK_MEMBER(scan_timer_callback);
	TIMER_CALLBACK_MEMBER(vint_trigger_callback);
	TIMER_CALLBACK_MEMBER(hint_trigger_callback);
	TIMER_CALLBACK_MEMBER(dma_callback);
	emu_timer *m_scan_timer;
	emu_timer *m_vint_on_timer;
	emu_timer *m_hint_on_timer;
	emu_timer *m_dma_timer;

	enum command_write_state_t : u8 {
		FIRST_WORD,
		SECOND_WORD
	};

	struct {
		u32 latch;
		u32 address;
		u8 code;
		command_write_state_t write_state;
	} m_command;

	void update_command_state();

	enum dma_mode_t : u8 {
		MEMORY_TO_VRAM,
		VRAM_FILL,
		VRAM_COPY
	};

	struct {
		u32 source_address;
		u16 length;
		dma_mode_t mode;
		bool active;
		u16 fill;
	} m_dma;

	enum
	{
		AS_VDP_VRAM   = 0,
		AS_VDP_CRAM   = 1,
		AS_VDP_VSRAM  = 2,
		AS_VDP_IO     = 3,
	};

	u16 data_port_r(offs_t offset, u16 mem_mask);
	void data_port_w(offs_t offset, u16 data, u16 mem_mask);
	u16 control_port_r(offs_t offset, u16 mem_mask);
	void control_port_w(offs_t offset, u16 data, u16 mem_mask);
	u16 hv_counter_r(offs_t offset, u16 mem_mask);

	void vram_w(offs_t offset, u16 data, u16 mem_mask);
	void cram_w(offs_t offset, u16 data, u16 mem_mask);

	bool m_ie1;
	bool m_vr; // 128 KiB VRAM mode (undocumented)
	bool m_de;
	bool m_ie0;
	bool m_m1; // DMA enable
	u32 m_hscroll_address;
	u8 m_hsz, m_vsz;
	u8 m_auto_increment;
	u32 m_plane_a_name_table;
	u32 m_window_name_table;
	u32 m_plane_b_name_table;
	u32 m_sprite_attribute_table;
	u8 m_background_color;
	u16 m_hit; // HBlank interrupt rate
	u8 m_vs, m_hs; // Vertical/Horizontal scroll modes
	// window
	bool m_rigt;
	u8 m_whp;
	bool m_down;
	u8 m_wvp;

	int m_vint_pending, m_hint_pending;
	u16 m_vcounter; // irq4 counter

	bitmap_rgb32 m_bitmap;
	bool render_line(int scanline);
	void prepare_sprite_line(int scanline);

	DECLARE_GFXDECODE_MEMBER(gfxinfo);
};

DECLARE_DEVICE_TYPE(YM7101, ym7101_device)


#endif // MAME_VIDEO_YM7101_H
