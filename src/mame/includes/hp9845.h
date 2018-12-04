// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// *******************************
// Driver for HP 9845B/C/T systems
// *******************************
#ifndef MAME_INCLUDES_HP9845_H
#define MAME_INCLUDES_HP9845_H

#pragma once

#include "cpu/hphybrid/hphybrid.h"
#include "machine/hp_taco.h"
#include "sound/beep.h"
#include "bus/hp9845_io/hp9845_io.h"
#include "emupal.h"
#include "screen.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/hp98x5_io_sys.h"

class hp9845_base_state : public driver_device
{
public:
	hp9845_base_state(const machine_config &mconfig, device_type type, const char *tag);

	DECLARE_INPUT_CHANGED_MEMBER(togglekey_changed);

protected:
	virtual void machine_start() override;
	virtual void device_reset() override;
	virtual void machine_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(gv_timer);

	virtual DECLARE_READ16_MEMBER(graphic_r) = 0;
	virtual DECLARE_WRITE16_MEMBER(graphic_w) = 0;
	attotime time_to_gv_mem_availability() const;

	TIMER_DEVICE_CALLBACK_MEMBER(kb_scan);
	DECLARE_READ16_MEMBER(kb_scancode_r);
	DECLARE_READ16_MEMBER(kb_status_r);
	DECLARE_WRITE16_MEMBER(kb_irq_clear_w);
	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off);

	DECLARE_WRITE_LINE_MEMBER(prt_irl_w);

	void hp9845_base(machine_config &config);
	void global_mem_map(address_map &map);
	void ppu_io_map(address_map &map);

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

	void setup_ram_block(unsigned block , unsigned offset);

	virtual void advance_gv_fsm(bool ds , bool trigger) = 0;
	void kb_scan_ioport(ioport_value pressed , ioport_port &port , unsigned idx_base , int& max_seq_len , unsigned& max_seq_idx);
	void update_kb_prt_irq();

	// Slot handling
	void set_irq_slot(unsigned slot , int state);
	void set_sts_slot(unsigned slot , int state);
	void set_flg_slot(unsigned slot , int state);
	void set_dmar_slot(unsigned slot , int state);

	// Character generator
	required_region_ptr<uint8_t> m_chargen;

	// Text mode video I/F
	typedef struct {
		uint8_t chars[ 80 ];
		uint8_t attrs[ 80 ];
		bool full;
	} video_buffer_t;

	bitmap_rgb32 m_bitmap;
	offs_t m_video_mar;
	uint16_t m_video_word;
	bool m_video_load_mar;
	bool m_video_first_mar;
	bool m_video_byte_idx;
	bool m_video_buff_idx;
	bool m_video_blanked;
	video_buffer_t m_video_buff[ 2 ];

	// Graphic video
	typedef enum {
		GV_STAT_RESET,
		GV_STAT_WAIT_DS_0 = GV_STAT_RESET,
		GV_STAT_WAIT_TRIG_0,
		GV_STAT_WAIT_MEM_0,
		GV_STAT_WAIT_DS_1,
		GV_STAT_WAIT_DS_2,
		GV_STAT_WAIT_TRIG_1,
		GV_STAT_WAIT_MEM_1,
		GV_STAT_WAIT_MEM_2
	} gv_fsm_state_t;

	bool m_graphic_sel;
	gv_fsm_state_t m_gv_fsm_state;
	bool m_gv_int_en;
	bool m_gv_dma_en;
	uint8_t m_gv_cmd; // U65 (GC)
	uint16_t m_gv_data_w;     // U29, U45, U28 & U44 (GC)
	uint16_t m_gv_data_r;     // U59 & U60 (GC)
	uint16_t m_gv_io_counter; // U1, U2, U14 & U15 (GC)
	uint16_t m_gv_cursor_x;   // U31 & U23 (GS)
	uint16_t m_gv_cursor_y;   // U15 & U8 (GS)
	bool m_gv_cursor_gc;    // U8 (GS)
	bool m_gv_cursor_fs;    // U8 (GS)

	// State of keyboard
	ioport_value m_kb_state[ 4 ];
	uint8_t m_kb_scancode;
	uint16_t m_kb_status;

	// Printer
	bool m_prt_irl;

	// SC of slots
	int m_slot_sc[ 4 ];
};

#endif // MAME_INCLUDES_HP9845_H
