// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/***************************************************************************

  Texas Instruments TMS3556 Video Display Processor

 ***************************************************************************/


#pragma once

#ifndef __TMS3556_H__
#define __TMS3556_H__

///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

#define TMS3556_TOP_BORDER 1
#define TMS3556_BOTTOM_BORDER 1
#define TMS3556_LEFT_BORDER 8
#define TMS3556_RIGHT_BORDER 8
#define TMS3556_TOTAL_WIDTH (320 + TMS3556_LEFT_BORDER + TMS3556_RIGHT_BORDER)
#define TMS3556_TOTAL_HEIGHT (250 + TMS3556_TOP_BORDER + TMS3556_BOTTOM_BORDER)

/* if DOUBLE_WIDTH set, the horizontal resolution is doubled */
#define TMS3556_DOUBLE_WIDTH 0

#define TMS3556_MODE_OFF    0
#define TMS3556_MODE_TEXT   1
#define TMS3556_MODE_BITMAP 2
#define TMS3556_MODE_MIXED  3

#define VDP_POINTER m_control_regs[0]
#define VDP_COL     m_control_regs[1]
#define VDP_ROW     m_control_regs[2]
#define VDP_STAT    m_control_regs[3]
#define VDP_CM1     m_control_regs[4]
#define VDP_CM2     m_control_regs[5]
#define VDP_CM3     m_control_regs[6]
#define VDP_CM4     m_control_regs[7]
#define VDP_BAMT    m_address_regs[0]
#define VDP_BAMP    m_address_regs[1]
#define VDP_BAPA    m_address_regs[2]
#define VDP_BAGC0   m_address_regs[3]
#define VDP_BAGC1   m_address_regs[4]
#define VDP_BAGC2   m_address_regs[5]
#define VDP_BAGC3   m_address_regs[6]
#define VDP_BAMTF   m_address_regs[7]

///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_TMS3556_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TMS3556, 0)

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

typedef enum { dma_read, dma_write } dma_mode_tt;


// ======================> tms3556_device

class tms3556_device :  public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	tms3556_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( vram_r );
	DECLARE_WRITE8_MEMBER( vram_w );
	DECLARE_READ8_MEMBER( reg_r );
	DECLARE_WRITE8_MEMBER( reg_w );
	DECLARE_READ8_MEMBER( initptr_r );

	void interrupt(running_machine &machine);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// address space configurations
	const address_space_config      m_space_config;

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);

	void draw_line_empty(UINT16 *ln);
	void draw_line_text_common(UINT16 *ln);
	void draw_line_bitmap_common(UINT16 *ln);
	void draw_line_text(UINT16 *ln);
	void draw_line_bitmap(UINT16 *ln);
	void draw_line_mixed(UINT16 *ln);
	void draw_line(bitmap_ind16 &bmp, int line);
	void interrupt_start_vblank(void);

private:
	// registers
	UINT8 m_control_regs[8];
	UINT16 m_address_regs[8];

	// register interface
	int m_reg_access_phase;

	int m_row_col_written;
	int m_bamp_written;
	int m_colrow;
	dma_mode_tt m_vdp_acmpxy_mode;
	UINT16 m_vdp_acmpxy;
	UINT16 m_vdp_acmp;
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
extern const device_type TMS3556;


#endif
