// license:BSD-3-Clause
// copyright-holders:Brice Onken

// Sony NWB-512 Monochrome Framebuffer and NWP-512D 15" Monochrome CRT Monitor

#ifndef MAME_SONY_NWB_512_H
#define MAME_SONY_NWB_512_H

#pragma once

#include "machine/ram.h"
#include "screen.h"

class nwb_512_device : public device_t
{
public:
	nwb_512_device(const machine_config &mconfig, const char *tag, device_t *owner) : nwb_512_device(mconfig, tag, owner, 0)
	{
	}

	nwb_512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T>
	void set_vram(T &&tag) { m_vram.set_tag(std::forward<T>(tag)); }

	void map(address_map &map);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	required_device<screen_device> m_nwp_512d;
	required_device<ram_device> m_vram;

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

		uint16_t destination_x = 0;			// DRXA
		uint16_t current_destination_x = 0; // DXA
		uint16_t destination_y = 0;			// DYA

		uint16_t source_x = 0;		   // SRXA
		uint16_t current_source_x = 0; // SXA
		uint16_t source_y = 0;		   // SYA

		uint16_t limit_x = 0; // LXA
		uint16_t limit_y = 0; // YC
		uint8_t rop_function = 0;

		// Status
		bool busy = false; // operation in progress

		// Configuration flags
		bool suppress_advance = false; // false = auto-increment after rop
		bool x_direction = false;	   // true = left -> right
		bool y_direction = false;	   // true = top -> bottom
		bool row_fill = false;		   // true = write data fifo word across whole line
		bool fb_read = false;		   // Tri-state with write - read = fb -> host, write = host -> fb, both = fb -> fb copy
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

		bool is_host_to_fb() const
		{
			return !fb_read && fb_write;
		}

		bool is_fb_to_fb() const
		{
			return fb_read && fb_write;
		}

		void check_enabled() const
		{
			if (!(ctrl & 0x1))
			{
				fatalerror("nwb_512: framebuffer not enabled when interaction started");
			}
		}

		void advance_sxa()
		{
			current_source_x += x_direction ? 16 : -16;
		}

		void advance_dxa()
		{
			current_destination_x += x_direction ? 16 : -16;
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

	// TODO: split by what is mapped and what is a helper
	void destination_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void source_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void limit_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void shift_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void rop_config_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void pixel_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t fifo_r(offs_t offset, uint16_t mem_mask);
	void fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t status_r();
	void ctrl_w(uint16_t data);
	void planemask_w(uint8_t data);

	void reset_state();
	void check_operation_start(offs_t offset);
	void set_pixel(uint16_t x, uint16_t y, bool on);
	uint16_t execute_rop(uint8_t opcode, uint16_t source, uint16_t destination);
	uint16_t shift(uint16_t data);
	void rop_cycle(uint16_t data);
	void rop(uint16_t source);
	void end_row();
	void start_operation();
	uint16_t coordinate_to_data(uint16_t x, uint16_t y);
	rgb_t current_dim_color();
};

DECLARE_DEVICE_TYPE(NWB_512, nwb_512_device)

#endif
