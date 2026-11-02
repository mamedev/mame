// license:BSD-3-Clause
// copyright-holders:Brice Onken

// Sony NWB-512 Monochrome Framebuffer and NWP-512D 15" Monochrome CRT Monitor

#ifndef MAME_SONY_NWB_512_H
#define MAME_SONY_NWB_512_H

#pragma once

#include "screen.h"

class nwb_512_device : public device_t
{
public:
	nwb_512_device(const machine_config &mconfig, const char *tag, device_t *owner) : nwb_512_device(mconfig, tag, owner, 0)
	{
	}

	nwb_512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// The NWB-512 only works with the NWP-512D monitor, per the Sony NEWS Technical Manual
	required_device<screen_device> m_nwp_512d;

	// VRAM consists of two types of memory chips:
	// 4x NEC uPD41264C-12 64K x 4 dual-port DRAM chips (visible screen and small off-screen area)
	// 12x Fujitsu MB81464-12 64K x 4 DRAM chips (off-screen buffer)
	memory_share_creator<uint16_t> m_vram;

	// Framebuffer state
	// CXD1029: raster operation engine
	// ROCC - Raster Operation Cycle Controller, state machine for framebuffer
	// Where the NWS-1500 service manual uses specific names for signals, they are noted below.
	// Technically, they may not have used this terminology on this framebuffer, but it seems
	// to work the same way, at least.
	struct state
	{
		// Registers
		uint16_t ctrl = 0;

		uint16_t destination_x = 0;         // DRXA
		uint16_t current_destination_x = 0; // DXA
		uint16_t destination_y = 0;         // DYA

		uint16_t source_x = 0;         // SRXA
		uint16_t current_source_x = 0; // SXA
		uint16_t source_y = 0;         // SYA

		uint16_t limit_x = 0; // LXA
		uint16_t limit_y = 0; // YC
		uint8_t rop_function = 0;

		// Status flags
		//  req exec: 00 = idle, 10 = command running, 01 = waiting for host data, 11 = fb busy
		// While req is set by fifo_w/fifo_r and on fb->fb copies, it is mostly a placeholder
		// for now because fb ops are executed instantly. Once the timing is emulated, then
		// req will be more important.
		bool req = false;  // FB executing op
		bool exec = false; // FB executing command

		// Configuration flags
		bool x_direction = false; // true = left -> right
		bool y_direction = false; // true = top -> bottom
		bool row_fill = false;
		bool fb_read = false;
		bool fb_write = false;
		bool pixel_mode = false;

		// Internal state
		uint16_t shift = 0;
		uint16_t carry = 0; // leftover bits after shifting
		uint16_t source_words_per_row = 0;
		uint16_t dest_words_per_row = 0;
		uint16_t source_words_left = 0;
		uint16_t dest_words_left = 0;
		uint16_t mask1 = 0;
		uint16_t mask2 = 0;
		bool prime = false;
		bool prime_pending = false; // /PRED - when src phase > dst phase, there are two source words needed to get/set first dst/src (or one source word that needs to be shifted for alignment purposes)

		bool is_fb_to_host() const
		{
			return fb_read && !fb_write;
		}

		bool is_fb_to_fb() const
		{
			return fb_read && fb_write;
		}

		bool enabled() const
		{
			return ctrl & 0x1;
		}

		bool busy() const
		{
			return req && exec;
		}

		void advance_sxa()
		{
			current_source_x += x_direction ? 16 : -16;
			--source_words_left;
		}

		void advance_dxa()
		{
			current_destination_x += x_direction ? 16 : -16;
			--dest_words_left;
		}

		void advance_sya()
		{
			source_y += y_direction ? 1 : -1;
		}

		void advance_dya()
		{
			destination_y += y_direction ? 1 : -1;
		}

		void load_shift_count()
		{
			// Determines how much to shift source by to align it with destination
			shift = (destination_x % 16 - source_x % 16) & 0xf;
		}

	} m_state;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	// Register read/write ports
	void destination_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void source_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void limit_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void rop_config_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void pixel_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t fifo_r(offs_t offset, uint16_t mem_mask);
	void fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t status_r();
	void ctrl_w(uint16_t data);

	// Helper functions
	void reset_state();
	void check_mem_mask(uint16_t mem_mask);
	void check_operation_start(offs_t offset);
	uint16_t coordinate_to_data(uint16_t x, uint16_t y);
	uint16_t execute_rop(uint8_t opcode, uint16_t source, uint16_t destination);
	uint16_t shift(uint16_t data);
	uint16_t shift_peek(uint16_t data) const;
	void rop(uint16_t source);
	void rop_cycle(uint16_t data);
	void end_row();
	void start_operation();
	void end_operation();
	void set_pixel(uint16_t x, uint16_t y, bool on);
	rgb_t current_dim_color();
};

DECLARE_DEVICE_TYPE(NWB_512, nwb_512_device)

#endif // MAME_SONY_NWB_512_H
