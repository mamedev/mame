// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// **************************
// Driver for HP 9845B system
// **************************
#ifndef _HP9845_H_
#define _HP9845_H_

#include "cpu/hphybrid/hphybrid.h"
#include "machine/hp_taco.h"
#include "sound/beep.h"
#include "bus/hp9845_io/hp9845_io.h"

class hp9845b_state : public driver_device
{
public:
	hp9845b_state(const machine_config &mconfig, device_type type, const char *tag);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(gv_timer);

	void vblank_w(screen_device &screen, bool state);

	void set_graphic_mode(bool graphic);
	DECLARE_READ16_MEMBER(graphic_r);
	DECLARE_WRITE16_MEMBER(graphic_w);
	attotime time_to_gv_mem_availability(void) const;
	void advance_gv_fsm(bool ds , bool trigger);
	void update_graphic_bits(void);

	IRQ_CALLBACK_MEMBER(irq_callback);
	void update_irq(void);
	void irq_w(uint8_t sc , int state);
	void update_flg_sts(void);
	void sts_w(uint8_t sc , int state);
	void flg_w(uint8_t sc , int state);
	void install_readwrite_handler(uint8_t sc , read16_delegate rhandler, write16_delegate whandler);

	TIMER_DEVICE_CALLBACK_MEMBER(kb_scan);
	DECLARE_READ16_MEMBER(kb_scancode_r);
	DECLARE_READ16_MEMBER(kb_status_r);
	DECLARE_WRITE16_MEMBER(kb_irq_clear_w);
	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off);

	DECLARE_WRITE8_MEMBER(pa_w);

	DECLARE_WRITE_LINE_MEMBER(t15_irq_w);
	DECLARE_WRITE_LINE_MEMBER(t15_flg_w);
	DECLARE_WRITE_LINE_MEMBER(t15_sts_w);

private:
	required_device<hp_5061_3001_cpu_device> m_lpu;
	required_device<hp_5061_3001_cpu_device> m_ppu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_gv_timer;
	required_ioport m_io_key0;
	required_ioport m_io_key1;
	required_ioport m_io_key2;
	required_ioport m_io_key3;
	required_device<hp_taco_device> m_t15;
	required_device<beep_device> m_beeper;
	required_device<timer_device> m_beep_timer;
	required_device<hp9845_io_slot_device> m_io_slot0;

	void set_video_mar(uint16_t mar);
	void video_fill_buff(bool buff_idx);
	void video_render_buff(unsigned video_scanline , unsigned line_in_row, bool buff_idx);
	void graphic_video_render(unsigned video_scanline);

	// Character generator
	const uint8_t *m_chargen;

	// Optional character generator
	const uint8_t *m_optional_chargen;

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
	uint8_t m_video_attr;
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
	uint16_t m_gv_cursor_w;   // U38 & U39 (GS)
	uint16_t m_gv_cursor_x;   // U31 & U23 (GS)
	uint16_t m_gv_cursor_y;   // U15 & U8 (GS)
	bool m_gv_cursor_gc;    // U8 (GS)
	bool m_gv_cursor_fs;    // U8 (GS)
	std::vector<uint16_t> m_graphic_mem;

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
};

#endif /* _HP9845_H_ */
