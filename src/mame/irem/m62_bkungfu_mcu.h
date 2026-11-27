// license:BSD-3-Clause
// copyright-holders:David Haywood, Andrea Bogazzi

#ifndef MAME_IREM_M62_BKUNGFU_MCU_H
#define MAME_IREM_M62_BKUNGFU_MCU_H

#pragma once

#include "machine/timer.h"

DECLARE_DEVICE_TYPE(BKUNGFU_MCU, bkungfu_mcu_device)

class bkungfu_mcu_device : public device_t
{
public:
	bkungfu_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	auto tilemap_ram_w() { return m_tilemap_ram_w.bind(); }

	// live interface
	void clear();
	u8 mailbox_r(offs_t offset);
	void mailbox_from_main_w(offs_t offset, uint8_t data);
	void mailbox_w(offs_t offset, uint8_t data);
	void command_w(uint8_t command);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// 56 columns * 26 rows (1456 tiles) at 280usec is about 23 M62 video frames (408250us),
	// matching the video frame timing observed from the real pcb video on YouTube.
	// To get the bg tiles to draw after 8 frames, must speed up the timer by 18usec per call, 
	// in-conjunction with using starting draw column of 170 below.
	static constexpr int LEVEL_DRAW_STEP_USEC = 262;
	static constexpr int TILEMAP_ROWS = 26;
	static constexpr int TILEMAP_COLS = 224;	// 56 x 4 = 224 ( columns per row )
	static constexpr int START_DRAW_COL = 170;	// First column to draw using timed handler  => 128 + 32 + 16 - 6
	static constexpr int DRAW_TILEMAP_COLS = 2;	// Draw 2 columns at a time to match video

	// Clear Screen takes ~6 frames of 30FPS video of real pcb = ~102us steps for 64x32 vram
	static constexpr int CLEAR_TILEMAP_STEP_USEC = 102;

	void update_slot(uint8_t slot);
	void execute_slot(uint8_t slot);
	void complete(uint16_t offset);
	void mailbox_out(uint16_t offset, uint8_t data);
	uint8_t read_data(uint16_t address) const;
	uint8_t decrypt_data(uint16_t address) const;
	uint8_t decode_payload(uint16_t address) const;
	uint16_t decode_payload_word(uint16_t address) const;
	void draw_object(uint8_t id);
	void draw_level_strip(int column, int row);
	void draw_level_column_row(int column, int row, uint8_t tile, uint8_t attr);
	void draw_text(uint16_t table_offset, bool use_mailbox);
	void draw_credits_continue();
	void clear_tilemap();
	void write_number(int x, int y, uint8_t number);
	void write_lifebar(int xbase, int ybase, uint8_t energy, bool boss);
	void write_floor_dot(int which, bool lit);
	void vram_page_w(offs_t offset, uint8_t data);
	void write_data_to_tilemap(int column, int row);
	void store_level_tilemap_data();

	TIMER_CALLBACK_MEMBER(leveldraw_next);
	TIMER_CALLBACK_MEMBER(clear_tilemap_next);

	devcb_write8 m_tilemap_ram_w;
	devcb_write8 m_mailbox_out_w;
	uint8_t m_mailbox[0x800];
	uint32_t m_mailbox_mask;
	uint32_t m_data_rom_mask;
	uint16_t m_timer;
	uint32_t m_p1score;
	uint32_t m_topscore;
	uint32_t m_p2score;
	uint8_t m_lives;
	uint8_t m_player_energy;
	uint8_t m_boss_energy;
	uint8_t m_floorcount;
	uint8_t m_floorcount_state;
	uint8_t m_valid;
	bool m_initialized;
	bool m_running;
	uint8_t m_leveldraw_row;
	uint8_t m_leveldraw_column;
	uint8_t m_leveldraw_number;
	uint32_t m_clear_position;
	emu_timer *m_leveldraw_timer;
	emu_timer* m_clear_timer;
	required_region_ptr<uint8_t> m_data_rom;
	uint8_t m_tilemap_data[256*32*2];
};

#endif // MAME_IREM_M62_BKUNGFU_MCU_H
