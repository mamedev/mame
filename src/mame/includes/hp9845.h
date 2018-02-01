// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// *******************************
// Driver for HP 9845B/C/T systems
// *******************************

#ifndef MAME_INCLUDES_HP9845_H
#define MAME_INCLUDES_HP9845_H

#include "cpu/hphybrid/hphybrid.h"
#include "machine/hp_taco.h"
#include "sound/beep.h"
#include "bus/hp9845_io/hp9845_io.h"
#include "screen.h"
#include "machine/ram.h"
#include "machine/timer.h"

class hp9845_base_state : public driver_device
{
public:
	hp9845_base_state(const machine_config &mconfig, device_type type, const char *tag);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(gv_timer);

	virtual DECLARE_READ16_MEMBER(graphic_r) = 0;
	virtual DECLARE_WRITE16_MEMBER(graphic_w) = 0;
	attotime time_to_gv_mem_availability(void) const;

	IRQ_CALLBACK_MEMBER(irq_callback);
	void update_irq(void);
	DECLARE_WRITE8_MEMBER(irq_w);
	void irq_w(uint8_t sc , int state);
	void update_flg_sts(void);
	DECLARE_WRITE8_MEMBER(sts_w);
	void sts_w(uint8_t sc , int state);
	DECLARE_WRITE8_MEMBER(flg_w);
	void flg_w(uint8_t sc , int state);

	TIMER_DEVICE_CALLBACK_MEMBER(kb_scan);
	DECLARE_READ16_MEMBER(kb_scancode_r);
	DECLARE_READ16_MEMBER(kb_status_r);
	DECLARE_WRITE16_MEMBER(kb_irq_clear_w);
	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off);

	DECLARE_WRITE8_MEMBER(pa_w);

	DECLARE_WRITE_LINE_MEMBER(prt_irl_w);
	DECLARE_WRITE_LINE_MEMBER(prt_flg_w);
	DECLARE_WRITE_LINE_MEMBER(prt_sts_w);
	DECLARE_WRITE_LINE_MEMBER(t14_irq_w);
	DECLARE_WRITE_LINE_MEMBER(t14_flg_w);
	DECLARE_WRITE_LINE_MEMBER(t14_sts_w);
	DECLARE_WRITE_LINE_MEMBER(t15_irq_w);
	DECLARE_WRITE_LINE_MEMBER(t15_flg_w);
	DECLARE_WRITE_LINE_MEMBER(t15_sts_w);

	DECLARE_INPUT_CHANGED_MEMBER(togglekey_changed);

	void hp9845_base(machine_config &config);
	void global_mem_map(address_map &map);
	void ppu_io_map(address_map &map);
protected:
	required_device<hp_5061_3001_cpu_device> m_lpu;
	required_device<hp_5061_3001_cpu_device> m_ppu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_gv_timer;
	required_ioport m_io_key0;
	required_ioport m_io_key1;
	required_ioport m_io_key2;
	required_ioport m_io_key3;
	required_ioport m_io_shiftlock;
	required_device<hp_taco_device> m_t14;
	required_device<hp_taco_device> m_t15;
	required_device<beep_device> m_beeper;
	required_device<timer_device> m_beep_timer;
	required_device<hp9845_io_slot_device> m_io_slot0;
	required_device<hp9845_io_slot_device> m_io_slot1;
	required_device<hp9845_io_slot_device> m_io_slot2;
	required_device<hp9845_io_slot_device> m_io_slot3;
	required_device<ram_device> m_ram;

	void setup_ram_block(unsigned block , unsigned offset);

	virtual void advance_gv_fsm(bool ds , bool trigger) = 0;
	void kb_scan_ioport(ioport_value pressed , ioport_port *port , unsigned idx_base , int& max_seq_len , unsigned& max_seq_idx);
	void update_kb_prt_irq();

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

	// Interrupt handling
	uint8_t m_irl_pending;
	uint8_t m_irh_pending;

	// FLG/STS handling
	uint8_t m_pa;
	uint16_t m_flg_status;
	uint16_t m_sts_status;

	// State of keyboard
	ioport_value m_kb_state[ 4 ];
	uint8_t m_kb_scancode;
	uint16_t m_kb_status;

	// Printer
	bool m_prt_irl;
};

#endif // MAME_INCLUDES_HP9845_H
