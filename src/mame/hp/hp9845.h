// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// *******************************
// Driver for HP 9845B/C/T systems
// *******************************
#ifndef MAME_HP_HP9845_H
#define MAME_HP_HP9845_H

#pragma once

#include "cpu/hphybrid/hphybrid.h"
#include "machine/hp_taco.h"
#include "sound/beep.h"
#include "bus/hp9845_io/hp9845_io.h"
#include "emupal.h"
#include "screen.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "hp98x5_io_sys.h"

class hp9845_base_state : public driver_device
{
public:
	hp9845_base_state(const machine_config &mconfig, device_type type, const char *tag);

	DECLARE_INPUT_CHANGED_MEMBER(togglekey_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(gv_timer);

	virtual uint16_t graphic_r(offs_t offset) = 0;
	virtual void graphic_w(offs_t offset, uint16_t data) = 0;
	attotime time_to_gv_mem_availability() const;

	TIMER_DEVICE_CALLBACK_MEMBER(kb_scan);
	uint16_t kb_scancode_r();
	uint16_t kb_status_r();
	void kb_irq_clear_w(uint16_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off);

	void prt_irl_w(int state);

	void hp9845_base(machine_config &config);
	void global_mem_map(address_map &map) ATTR_COLD;
	void ppu_io_map(address_map &map) ATTR_COLD;

	required_device<hp_5061_3001_cpu_device> m_lpu;
	required_device<hp_5061_3001_cpu_device> m_ppu;
	required_device<hp98x5_io_sys_device> m_io_sys;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_gv_timer;
	required_ioport_array<4> m_io_key;
	required_ioport m_io_shiftlock;
	required_device<hp_taco_device> m_t14;
	required_device<hp_taco_device> m_t15;
	required_device<beep_device> m_beeper;
	required_device<timer_device> m_beep_timer;
	required_device_array<hp9845_io_slot_device, 4> m_io_slot;
	required_device<ram_device> m_ram;
	output_finder<8> m_softkeys;
	output_finder<> m_shift_lock_led;
	output_finder<> m_prt_all_led;
	output_finder<> m_auto_st_led;

	void setup_ram_block(unsigned block , unsigned offset);

	virtual void advance_gv_fsm(bool ds , bool trigger) = 0;
	void kb_scan_ioport(ioport_value pressed , ioport_port &port , unsigned idx_base , int& max_seq_len , unsigned& max_seq_idx);
	void update_kb_prt_irq();

	// Slot handling
	void set_irq_slot(unsigned slot , int state);
	void set_sts_slot(unsigned slot , int state);
	void set_flg_slot(unsigned slot , int state);
	void set_irq_nextsc_slot(unsigned slot , int state);
	void set_sts_nextsc_slot(unsigned slot , int state);
	void set_flg_nextsc_slot(unsigned slot , int state);
	void set_dmar_slot(unsigned slot , int state);

	// Character generator
	required_region_ptr<uint8_t> m_chargen;

	// Text mode video I/F
	struct video_buffer_t {
		uint8_t chars[ 80 ]{};
		uint8_t attrs[ 80 ]{};
		bool full = 0;
	};

	bitmap_rgb32 m_bitmap;
	offs_t m_video_mar = 0;
	uint16_t m_video_word = 0;
	bool m_video_load_mar = false;
	bool m_video_first_mar = false;
	bool m_video_byte_idx = false;
	bool m_video_buff_idx = false;
	bool m_video_blanked = false;
	video_buffer_t m_video_buff[ 2 ];

	// Graphic video
	enum gv_fsm_state_t {
		GV_STAT_RESET,
		GV_STAT_WAIT_DS_0 = GV_STAT_RESET,
		GV_STAT_WAIT_TRIG_0,
		GV_STAT_WAIT_MEM_0,
		GV_STAT_WAIT_DS_1,
		GV_STAT_WAIT_DS_2,
		GV_STAT_WAIT_TRIG_1,
		GV_STAT_WAIT_MEM_1,
		GV_STAT_WAIT_MEM_2
	};

	bool m_graphic_sel = false;
	gv_fsm_state_t m_gv_fsm_state;
	bool m_gv_int_en = false;
	bool m_gv_dma_en = false;
	uint8_t m_gv_cmd = 0; // U65 (GC)
	uint16_t m_gv_data_w = 0;     // U29, U45, U28 & U44 (GC)
	uint16_t m_gv_data_r = 0;     // U59 & U60 (GC)
	uint16_t m_gv_io_counter = 0; // U1, U2, U14 & U15 (GC)
	uint16_t m_gv_cursor_x = 0;   // U31 & U23 (GS)
	uint16_t m_gv_cursor_y = 0;   // U15 & U8 (GS)
	bool m_gv_cursor_gc = false;    // U8 (GS)
	bool m_gv_cursor_fs = false;    // U8 (GS)

	// State of keyboard
	ioport_value m_kb_state[ 4 ];
	uint8_t m_kb_scancode = 0;
	uint16_t m_kb_status = 0;

	// Printer
	bool m_prt_irl = false;

	// SC of slots
	int m_slot_sc[ 4 ]{};
};

#endif // MAME_HP_HP9845_H
