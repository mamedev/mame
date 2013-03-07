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
#define TMS3556_DOUBLE_WIDTH 1

#define TMS3556_MODE_OFF    0
#define TMS3556_MODE_TEXT   1
#define TMS3556_MODE_BITMAP 2
#define TMS3556_MODE_MIXED  3


///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_TMS3556_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TMS3556, 0)

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> tms3556_device

class tms3556_device :  public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	tms3556_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( vram_r );
	DECLARE_WRITE8_MEMBER( vram_w );
	DECLARE_READ8_MEMBER( reg_r );
	DECLARE_WRITE8_MEMBER( reg_w );

	void interrupt(running_machine &machine);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

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
	UINT16 m_write_ptr;

	// register interface
	int m_reg_ptr;
	int m_reg_access_phase;
	int m_magical_mystery_flag;

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
