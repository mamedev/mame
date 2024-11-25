// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/***************************************************************************

  Texas Instruments TMS3556 Video Display Processor

 ***************************************************************************/

#ifndef MAME_VIDEO_TMS3556_H
#define MAME_VIDEO_TMS3556_H

#pragma once


///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

/* if DOUBLE_WIDTH set, the horizontal resolution is doubled */
#define TMS3556_DOUBLE_WIDTH 0


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************


// ======================> tms3556_device

class tms3556_device : public device_t, public device_memory_interface, public device_video_interface
{
public:
	static constexpr unsigned TOP_BORDER = 1;
	static constexpr unsigned BOTTOM_BORDER = 1;
	static constexpr unsigned LEFT_BORDER = 8;
	static constexpr unsigned RIGHT_BORDER = 8;
	static constexpr unsigned TOTAL_WIDTH = 320 + LEFT_BORDER + RIGHT_BORDER;
	static constexpr unsigned TOTAL_HEIGHT = 250 + TOP_BORDER + BOTTOM_BORDER;

	// construction/destruction
	tms3556_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t vram_r();
	void vram_w(uint8_t data);
	uint8_t reg_r();
	void reg_w(uint8_t data);
	uint8_t initptr_r();

	void interrupt();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_config_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// address space configurations
	const address_space_config      m_space_config;

	inline uint8_t readbyte(offs_t address);
	inline void writebyte(offs_t address, uint8_t data);

	void draw_line_empty(uint16_t *ln);
	void draw_line_text_common(uint16_t *ln);
	void draw_line_bitmap_common(uint16_t *ln);
	void draw_line_text(uint16_t *ln);
	void draw_line_bitmap(uint16_t *ln);
	void draw_line_mixed(uint16_t *ln);
	void draw_line(bitmap_ind16 &bmp, int line);
	void interrupt_start_vblank(void);

	void tms3556(address_map &map) ATTR_COLD;

	enum dma_mode_tt : u8 { dma_read, dma_write };

	static constexpr uint8_t MODE_OFF    = 0;
	static constexpr uint8_t MODE_TEXT   = 1;
	static constexpr uint8_t MODE_BITMAP = 2;
	static constexpr uint8_t MODE_MIXED  = 3;

	// registers
	uint8_t m_control_regs[8];
	uint16_t m_address_regs[8];

	// register interface
	uint8_t m_reg, m_reg2;

	int m_row_col_written;
	int m_bamp_written;
	int m_colrow;
	dma_mode_tt m_vdp_acmpxy_mode;
	uint16_t m_vdp_acmpxy;
	uint16_t m_vdp_acmp;
	int m_init_read;

	int m_scanline;             // scanline counter
	int m_blink, m_blink_count; // blinking
	int m_bg_color;             // background color for current line
	int m_name_offset;          // current offset in name table
	int m_cg_flag;              // c/g flag (mixed mode only)
	int m_char_line_counter;    // character line counter (decrements from 10, 0 when we have reached
								// last line of character row)
	int m_dbl_h_phase[40];      // double height phase flags (one per horizontal character position)

	bitmap_ind16 m_bitmap;
};


// device type definition
DECLARE_DEVICE_TYPE(TMS3556, tms3556_device)

#endif // MAME_VIDEO_TMS3556_H
