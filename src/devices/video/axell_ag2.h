// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Axell AG-2 / AX51201 graphics display controller.
#ifndef MAME_VIDEO_AXELL_AG2_H
#define MAME_VIDEO_AXELL_AG2_H

#pragma once

#include "axell_ag2_rm1.h"

#include "dirom.h"

#include <array>
#include <string>
#include <vector>


class axell_ag2_device : public device_t, public device_rom_interface<26>, public device_video_interface
{
public:
	axell_ag2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	axell_ag2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 vram_size);

	// Output indices 0, 1, 2, 3 correspond to pending bits 0, 2, 4, 6.
	template <u32 N> auto event_cb() { return m_event_cb[N].bind(); }

	// Host offsets are bytes, independent of the board CPU's bus width.
	u8 host_r(offs_t offset);
	void host_w(offs_t offset, u8 data);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	static constexpr int AS_HOST = 1;
	static constexpr int AS_VRAM = 2;

	static constexpr u32 REG_BASE = 0x1fc00;
	static constexpr u32 HOST_SIZE = 0x20000;
	static constexpr u32 CGROM_SIZE = 1U << 26;
	static constexpr u32 VRAM_X_BITS = 12;

	class linear_decompressor;

	void host_map(address_map &map) ATTR_COLD;
	void vblank(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(complete);
	void schedule_event(u8 event, u64 clocks);
	void update_events();
	void command_w(u8 data);
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
	bool sample_pixel(u16 cell, u16 sheet, int u, int v, u16 &color) const;
	void draw_pixel(int x, int y, u16 color, u8 modulation);
	void clear_frame_buffer(bool second);
	void update_screen_size();
	u16 reg16(u32 a) const { return (m_host[REG_BASE + a] << 8) | m_host[REG_BASE + a + 1]; }
	u8 reg8(u32 a) const { return m_host[REG_BASE + a]; }
	u8 list8(u32 a) const { return m_host[0x10000 | (a & 0xffff)]; }
	u16 list16(u32 a) const { return (list8(a) << 8) | list8(a + 1); }
	u32 vram_addr(u32 x, u32 y) const { return ((y << VRAM_X_BITS) | (x & 0xfff)) & m_vram_mask; }
	u16 vram16(u32 x, u32 y) const;
	void vram_w16(u32 x, u32 y, u16 data);
	u8 rom8(u32 a) { return read_byte(a & (CGROM_SIZE - 1)); }
	bool decompress(u32 source, int width, int height, int x, int y);
	void trace(bool write, offs_t offset, u8 data);
	void flush_repeats();
	static const char *register_name(u8 offset);

	address_space_config m_host_config;
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

	// Diagnostics do not affect emulated state.
	std::array<u64, 256> m_reads;
	std::array<u64, 256> m_writes;
	std::string m_last_context;
	u32 m_last_offset;
	u8 m_last_data;
	bool m_last_write;
	u64 m_repeats;
	u64 m_transfers;
	u64 m_draws;
	u64 m_decode_errors;
	u64 m_rm1_commands;
};

DECLARE_DEVICE_TYPE(AXELL_AG2, axell_ag2_device)

#endif // MAME_VIDEO_AXELL_AG2_H
