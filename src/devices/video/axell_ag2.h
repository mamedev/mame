// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Axell AG-2 / AX51201 graphics display controller.
#ifndef MAME_VIDEO_AXELL_AG2_H
#define MAME_VIDEO_AXELL_AG2_H

#pragma once

#include "axell_ag2_rm1.h"

#include "dirom.h"

#include "multibyte.h"

#include <vector>


class ax51201_device : public device_t, public device_rom_interface<26>, public device_video_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	ax51201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	ax51201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 vram_size);

	// Output indices 0, 1, 2, 3 correspond to pending bits 0, 2, 4, 6.
	template <u32 N> auto event_cb() { return m_event_cb[N].bind(); }

	// 128 KiB host aperture: list RAM, with the registers at +1fc00. Offsets
	// are bytes, independent of the host CPU's bus width.
	void map(address_map &map) ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	static constexpr int AS_VRAM = 1;

	static constexpr u32 REG_BASE = 0x1fc00;
	static constexpr u32 HOST_SIZE = 0x20000;
	static constexpr u32 CGROM_SIZE = 1U << 26;
	static constexpr u32 VRAM_X_BITS = 12;

	// drawing parameters gathered once per cell
	struct cell_context
	{
		u32 format;       // pixel format 0-4
		bool rgb565;      // texels or palette entries are RGB565 rather than RGB555 with a transparency bit
		u32 palette_base; // first palette entry, in 16-bit VRAM units
		u32 blend_mode;   // modulation operation
		u32 blend_level;  // modulation strength
		u32 dest_x;       // VRAM position of window coordinate 0
		u32 dest_y;
	};

	class linear_decompressor;

	u8 ram_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);
	void reg_w(offs_t offset, u8 data);
	void command_w(u8 data);
	void event_mask_w(u8 data);
	u8 event_status_r();
	void event_status_w(u8 data);
	void display_control_w(u8 data);
	void display_select_w(u8 data);
	void display_base_w(offs_t offset, u8 data);
	void display_timing_w(offs_t offset, u8 data);

	void vblank(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(complete);
	void schedule_event(u8 event, u64 clocks);
	void update_events();
	void transfer();
	void rm1_reset();
	void rm1_clear();
	void rm1_advance();
	void rm1_initialize();
	void rm1_restore();
	u32 rm1_read(u32 address, u32 bytes);
	const u8 *rm1_payload(u32 address, u32 size);
	u32 rm1_decode_next();
	void rm1_write_frame(u32 slot, const axell_rm1::frame &picture);
	u32 draw_list();
	u32 draw_sheet(u16 sheet);
	u32 draw_cell(u16 cell, u16 sheet);
	bool sample_pixel(const cell_context &context, int u, int v, u16 &color) const;
	void draw_pixel(const cell_context &context, int x, int y, u16 color);
	void clear_frame_buffer(bool second);
	void update_screen_size();
	u16 reg16(u32 a) const { return get_u16be(&m_host[REG_BASE + a]); }
	u8 reg8(u32 a) const { return m_host[REG_BASE + a]; }
	u8 list8(u32 a) const { return m_host[0x10000 | (a & 0xffff)]; }
	u16 list16(u32 a) const { return (list8(a) << 8) | list8(a + 1); }
	u32 vram_addr(u32 x, u32 y) const { return ((y << VRAM_X_BITS) | (x & 0xfff)) & m_vram_mask; }
	u16 vram16(u32 x, u32 y) const;
	void vram_w16(u32 x, u32 y, u16 data);
	u8 rom8(u32 a) { return read_byte(a & (CGROM_SIZE - 1)); }
	bool decompress(u32 source, int width, int height, int x, int y);

	address_space_config m_vram_config;
	devcb_write_line::array<4> m_event_cb;
	memory_share_creator<u8> m_host;
	memory_share_creator<u8> m_vram;
	const u32 m_vram_size;
	const u32 m_vram_mask;
	emu_timer *m_completion_timer[3];
	u8 m_pending;
	u8 m_active; // pending & enabled, as last driven to the outputs
	bool m_vblank;
	// RM1 references are rebuilt from the CG ROM after loading a state.
	axell_rm1::decoder m_rm1_decoder;
	axell_rm1::frame m_rm1_anchor_picture;
	std::vector<u8> m_rm1_buffer;
	bool m_rm1_active;
	bool m_rm1_have_anchor;
	u32 m_rm1_base;
	u32 m_rm1_first_packet;
	u32 m_rm1_next_packet;
	u32 m_rm1_packet_count;
	u32 m_rm1_max_payload;
	u32 m_rm1_decoded;
	u32 m_rm1_next_display;
	u32 m_rm1_anchor_display;
	u8 m_rm1_anchor_advance;
	u8 m_rm1_ready_slot;
};

DECLARE_DEVICE_TYPE(AX51201, ax51201_device)

#endif // MAME_VIDEO_AXELL_AG2_H
