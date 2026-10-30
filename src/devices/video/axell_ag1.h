// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen, Morten Shearman Kirkegaard
/**********************************************************************

    Axell AG-1 sprite / graphics display controller family

    AX51101 - Z80-style 8-bit host interface, DRAM CGRAM (Gunpey)
    AX51102 - generic 16-bit host interface, SDRAM CGRAM (Taito
              Dinoking series)

    The chip renders a list of "sheets" (sprites, with per-sheet
    scaling and intensity / translucency modulation), each made of a
    list of "cells" (4/6/8bpp or RGB rectangles with per-cell palette,
    alpha, scaling, flipping and rotation), from character data living
    in a large linear CGRAM organised as 2048 or 4096 byte rows.
    Character data is fetched from an attached CG-ROM (2048-byte rows)
    by an integrated blitter which supports a Huffman/RLE compression
    scheme.

    The display lists live in a 64KB "sprite control" memory space the
    host shares with the chip; the owner provides an address map for
    it (space 0).  The 256 colour x 256 set palette also lives in CGRAM
    (16-bit RGB555 entries at (colour * 2, set)), as do the frame
    buffers of the AX51102.  Space 1 exposes the CGRAM for debugging.

**********************************************************************/

#ifndef MAME_VIDEO_AXELL_AG1_H
#define MAME_VIDEO_AXELL_AG1_H

#pragma once

#include <memory>
#include <utility>


class axell_ag1_device_base : public device_t, public device_memory_interface, public device_video_interface
{
public:
	// configuration
	template <typename T> void set_cgrom_tag(T &&tag) { m_cgrom.set_tag(std::forward<T>(tag)); }

	// interrupt outputs: the AX51101 has a single INT output (line 0, any
	// enabled cause), the AX51102 has one output per interrupt cause bit
	// (line N = cause bit N) so boards can route them to separate CPU inputs
	template <u32 N> auto irq_callback() { return m_irq_cb[N].bind(); }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// host access to the sprite control memory (for boards where it is only
	// reachable through the chip's bus)
	u8 list_r(offs_t offset) { return m_list->read_byte(offset); }
	void list_w(offs_t offset, u8 data) { m_list->write_byte(offset, data); }

protected:
	// interrupt cause bits (shared by both variants)
	static constexpr u8 IRQ_LIST_BUSY = 0x02; // AX51102: sprite control RAM write collided with the chip (host must retry)
	static constexpr u8 IRQ_BLITTER   = 0x04; // blitter transfer complete
	static constexpr u8 IRQ_LIST_DONE = 0x10; // display list transfer complete
	static constexpr u8 IRQ_FRAME_B   = 0x20; // AX51102: second per-frame interrupt (hkuranai uses it as its frame sync)
	static constexpr u8 IRQ_FRAME     = 0x40; // start of vertical blanking

	// decoded cell (sprite element) parameters
	struct cell_params
	{
		bool end;        // last cell of the list
		bool disabled;   // cell is not drawn
		bool flip_x;     // mirror the cell
		bool flip_y;
		u8 bpp_sel;      // 0 = 4bpp, 1 = 6bpp, 2 = 8bpp, 3 = RGB
		u8 palette;      // palette selector
		u8 alpha;        // 0 = solid
		u8 quadrant;     // rotation quadrant (signs of the cosine / sine below)
		u32 src_x;       // source position in CGRAM (bit address within the row)
		u32 src_y;       // source row, 2 fractional bits
		s32 dst_x;       // position within the sheet
		s32 dst_y;
		u32 dst_w;       // size on screen
		u32 dst_h;
		// source distance covered across the whole cell: horizontally
		// |w cos| / |w sin|, vertically |h sin| / |h cos| - for an
		// unrotated cell this is just the source width and height
		u32 w_cos;
		u32 w_sin;
		u32 h_sin;
		u32 h_cos;
	};

	// drawing targets
	struct bitmap_target;
	struct cgram_target;

	axell_ag1_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, endianness_t host_endianness, address_map_constructor list_map, u32 cgram_x_bits, u32 cgram_y_bits, u8 cgram_fill);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// variant hooks
	virtual void draw_display_list(bitmap_rgb32 &bitmap, const rectangle &cliprect) = 0;
	virtual void update_irq() = 0;
	virtual void vblank(screen_device &screen, bool state) = 0;

	// interrupt handling
	void raise_irq(u8 causes);
	void ack_irq(u8 causes);
	void set_irq_mask(u8 mask);
	u8 irq_cause() const { return m_irq_cause; }
	u8 irq_mask() const { return m_irq_mask; }
	u8 active_irqs() const { return m_irq_cause & m_irq_mask; }

	// blitter
	void start_blit(u16 src_x, u16 src_y, u16 dst_x, u16 dst_y, u16 xsize, u16 ysize, u8 mode, u8 irq_on_completion);
	void schedule_irq(u8 cause, const attotime &delay);

	// display list memory access
	u8 list_read_byte(offs_t address) { return m_list->read_byte(address); }
	u16 list_read_word(offs_t address) { return m_list->read_word(address); }

	// drawing
	template <typename Target> void draw_cell(Target &target, const cell_params &cell, s32 x, s32 y, int darken);

	// CGRAM access
	u32 cgram_addr(u32 x, u32 y) const { return (x & m_cgram_x_mask) | ((y & m_cgram_y_mask) << m_cgram_x_bits); }
	u16 cgram_read16(u32 x, u32 y) const
	{
		const u32 a = cgram_addr(x << 1, y);
		return m_cgram[a] | (m_cgram[a + 1] << 8);
	}
	void cgram_write16(u32 x, u32 y, u16 data)
	{
		const u32 a = cgram_addr(x << 1, y);
		m_cgram[a] = data & 0xff;
		m_cgram[a + 1] = data >> 8;
	}
	u16 palette_entry(u32 color) const { return cgram_read16(color & 0xff, color >> 8); }

	std::unique_ptr<u32[]> m_rgb555_lut;

	// register file, indexed by host byte address
	u8 m_regs[0x100];

	devcb_write_line::array<8> m_irq_cb;
	u8 m_irq_lines;

private:
	u8 get_cgrom_byte(u32 x, u32 y) const;

	template <typename Target> void draw_pixel(Target &target, int x, int y, u16 color_data, int alpha, int darken);

	// decompressor
	bool decompress_sprite(int ix, int iy, int ow, int oh, int dx, int dy);

	TIMER_CALLBACK_MEMBER(irq_timer);

	required_region_ptr<u8> m_cgrom;
	address_space_config m_list_config;
	address_space_config m_cgram_config;
	address_space *m_list;

	const u32 m_cgram_x_bits;
	const u32 m_cgram_x_mask;
	const u32 m_cgram_y_mask;
	const u8 m_cgram_fill;
	std::unique_ptr<u8[]> m_cgram;

	emu_timer *m_irq_timer[2];
	u8 m_irq_cause;
	u8 m_irq_mask;
};


// ======================> ax51101_device

class ax51101_device : public axell_ag1_device_base
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	ax51101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// 64 byte-wide registers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void draw_display_list(bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void update_irq() override;
	virtual void vblank(screen_device &screen, bool state) override;

private:
	u16 reg16(u32 offset) const { return m_regs[offset] | (m_regs[offset + 1] << 8); }
	void decode_cell(offs_t address, cell_params &cell);
};


// ======================> ax51102_device

class ax51102_device : public axell_ag1_device_base
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	ax51102_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// 256 byte-wide registers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// host access to the built-in 2KB sprite control RAM
	u8 sprram_r(offs_t offset) { return list_r(0x8000 + (offset & 0x7ff)); }
	void sprram_w(offs_t offset, u8 data) { list_w(0x8000 + (offset & 0x7ff), data); }

protected:
	virtual void draw_display_list(bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void update_irq() override;
	virtual void vblank(screen_device &screen, bool state) override;

private:
	u16 reg16(u32 offset) const { return (m_regs[offset] << 8) | m_regs[offset + 1]; }
	void list_map(address_map &map) ATTR_COLD;
	void command_w(u8 data);
	void decode_cell(offs_t address, cell_params &cell);
	void render_display_list();
	void draw_sheet(cgram_target &target, offs_t sheet, u16 cell_base);
	void clear_frame_buffer(bool second);
	void update_screen_timing();

	bitmap_rgb32 m_display;
};


DECLARE_DEVICE_TYPE(AX51101, ax51101_device)
DECLARE_DEVICE_TYPE(AX51102, ax51102_device)

#endif // MAME_VIDEO_AXELL_AG1_H
