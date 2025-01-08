// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Intel 82730

    Text Coprocessor

***************************************************************************/

#ifndef MAME_VIDEO_I82730_H
#define MAME_VIDEO_I82730_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define I82730_UPDATE_ROW(name) \
	   void name(bitmap_rgb32 &bitmap, uint16_t *data, uint8_t lc, uint16_t y, int x_count)

// ======================> i82730_device

class i82730_device : public device_t, public device_video_interface
{
public:
	typedef device_delegate<void (bitmap_rgb32 &bitmap, uint16_t *data, uint8_t lc, uint16_t y, int x_count)> update_row_delegate;

	// construction/destruction
	template <typename T>
	i82730_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: i82730_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
	}
	i82730_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto sint() { return m_sint_handler.bind(); }

	// inline configuration
	template <typename... T> void set_update_row_callback(T &&... args) { m_update_row_cb.set(std::forward<T>(args)...); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ca_w(int state);
	void irst_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// status
	enum
	{
		DUR  = 0x001,  // data underrun
		LPU  = 0x002,  // light pen update
		DBOR = 0x004,  // data buffer overrun
		EONF = 0x008,  // end of n frames
		FDE  = 0x010,  // frame data error
		RCC  = 0x020,  // reserved channel command executed
		RDC  = 0x040,  // reserved data stream command executed
		DIP  = 0x080,  // display in progress
		VDIP = 0x100   // virtual display in progress
	};

	bool sysbus_16bit() { return BIT(m_sysbus, 0); }

	uint8_t read_byte(offs_t address);
	uint16_t read_word(offs_t address);
	void write_byte(offs_t address, uint8_t data);
	void write_word(offs_t address, uint16_t data);

	void update_interrupts();
	void mode_set();
	void execute_command();

	bool dscmd_endrow();
	bool dscmd_eof();
	bool dscmd_eol();
	bool dscmd_fulrowdescrpt(uint8_t param);
	bool dscmd_sl_scroll_strt(uint8_t param);
	bool dscmd_sl_scroll_end(uint8_t param);
	bool dscmd_tab_to(uint8_t param);
	bool dscmd_max_dma_count(uint8_t param);
	bool dscmd_endstrg();
	bool dscmd_skip(uint8_t param);
	bool dscmd_repeat(uint8_t param);
	bool dscmd_sub_sup(uint8_t param);
	bool dscmd_rpt_sub_sup(uint8_t param);
	bool dscmd_set_gen_pur_attrib(uint8_t param);
	bool dscmd_set_field_attrib();
	bool dscmd_init_next_process();

	bool execute_datastream_command(uint8_t command, uint8_t param);

	void load_row();
	void attention();

	TIMER_CALLBACK_MEMBER(row_update);

	devcb_write_line m_sint_handler;
	update_row_delegate m_update_row_cb;

	required_device<cpu_device> m_cpu;
	address_space *m_program;

	emu_timer *m_row_timer;

	bitmap_rgb32 m_bitmap;

	// internal registers
	bool m_initialized;
	bool m_mode_set;
	int m_ca;
	bool m_ca_latch;

	uint8_t m_sysbus;
	uint32_t m_ibp; // intermediate block pointer
	uint32_t m_cbp; // command block pointer

	bool m_list_switch;
	bool m_auto_line_feed;
	uint8_t m_max_dma_count;
	uint32_t m_lptr;
	uint16_t m_status;
	uint16_t m_intmask;

	uint32_t m_sptr;

	struct modeset
	{
		// horizontal modes
		uint8_t burst_length;
		uint8_t burst_space;
		uint8_t line_length;
		uint8_t hsyncstp;
		uint8_t hfldstrt;
		uint8_t hfldstp;
		uint8_t hbrdstrt;
		uint8_t hbrdstp;
		uint8_t scroll_margin;
		// char row characteristics
		bool rvv_row;
		bool blk_row;
		bool dbl_hgt;
		bool wdef;
		uint8_t lpr;
		uint8_t nrmstrt;
		uint8_t nrmstp;
		uint8_t supstrt;
		uint8_t supstp;
		uint8_t substrt;
		uint8_t substp;
		uint8_t cur1strt;
		uint8_t cur1stp;
		uint8_t cur2strt;
		uint8_t cur2stp;
		uint8_t u2_line_sel;
		uint8_t u1_line_sel;
		uint16_t field_attribute_mask;
		// vertical modes
		uint16_t frame_length;
		uint16_t vsyncstp;
		uint16_t vfldstrt;
		uint16_t vfldstp;
		// blink control
		uint8_t duty_cyc_cursor;
		uint8_t cursor_blink;
		uint8_t frame_int_count;
		uint8_t duty_cyc_char;
		uint8_t char_blink;
		bool ile;
		bool rfe;
		bool bpol;
		bool bue;
		bool cr2_cd;
		bool cr1_cd;
		bool cr2_be;
		bool cr1_be;
		// atrribute bit selects
		uint8_t reverse_video;
		uint8_t blinking_char;
		bool cr2_rvv;
		bool cr1_rvv;
		bool cr2_oe;
		bool cr1_oe;
		uint8_t abs_line_count;
		uint8_t invisible_char;
		uint8_t underline2;
		uint8_t underline1;
	} m_mb;

	uint16_t m_row_buffer[2][200];
	uint16_t *m_row; // pointer to currently active row buffer
	uint8_t m_dma_count;
	uint8_t m_row_count; // maximum 200
	int m_row_index; // 0 or 1

	struct cursor
	{
		uint8_t x;
		uint8_t y;
	} m_cursor[2];
};

// device type definition
DECLARE_DEVICE_TYPE(I82730, i82730_device)

#endif // MAME_VIDEO_I82730_H
