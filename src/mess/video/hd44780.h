/***************************************************************************

        Hitachi HD44780 LCD controller

***************************************************************************/

#pragma once

#ifndef __HD44780_H__
#define __HD44780_H__


#define MCFG_HD44780_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, HD44780, 0 )

#define MCFG_HD44780_LCD_SIZE(_lines, _chars) \
	hd44780_device::static_set_lcd_size(*device, _lines, _chars);

#define MCFG_HD44780_PIXEL_UPDATE_CB(_cb) \
	hd44780_device::static_set_pixel_update_cb(*device, _cb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef void (*hd44780_pixel_update_func)(device_t &device, bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state);
#define HD44780_PIXEL_UPDATE(name) void name(device_t &device, bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state)


// ======================> hd44780_device

class hd44780_device :  public device_t
{
public:
	// construction/destruction
	hd44780_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	hd44780_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_lcd_size(device_t &device, int _lines, int _chars) { hd44780_device &dev=downcast<hd44780_device &>(device); dev.m_lines = _lines; dev.m_chars = _chars; }
	static void static_set_pixel_update_cb(device_t &device, hd44780_pixel_update_func _cb) { downcast<hd44780_device &>(device).m_pixel_update_func = _cb; }

	// device interface
	virtual DECLARE_WRITE8_MEMBER(write);
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(control_write);
	virtual DECLARE_READ8_MEMBER(control_read);
	virtual DECLARE_WRITE8_MEMBER(data_write);
	virtual DECLARE_READ8_MEMBER(data_read);
	virtual UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// optional information overrides
	const rom_entry *device_rom_region() const;

private:
	// internal helper
	void set_busy_flag(UINT16 usec);
	void update_ac(int direction);
	void pixel_update(bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state);

	// internal state
	static const device_timer_id TIMER_BUSY = 0;
	static const device_timer_id TIMER_BLINKING = 1;

	emu_timer * m_blink_timer;
	emu_timer * m_busy_timer;

	UINT8       m_lines;          // number of lines
	UINT8       m_chars;          // chars for line
	hd44780_pixel_update_func m_pixel_update_func; // pixel update callback

	bool        m_busy_flag;      // busy flag
	UINT8       m_ddram[0x80];    // internal display data RAM
	UINT8       m_cgram[0x40];    // internal chargen RAM
	UINT8 *     m_cgrom;          // internal chargen ROM
	INT8        m_ac;             // address counter
	UINT8       m_dr;             // data register
	UINT8       m_ir;             // instruction register
	UINT8       m_active_ram;     // DDRAM or CGRAM
	bool        m_display_on;     // display on/off
	bool        m_cursor_on;      // cursor on/off
	bool        m_blink_on;       // blink on/off
	bool        m_shift_on;       // shift on/off
	INT8        m_disp_shift;     // display shift
	INT8        m_direction;      // auto increment/decrement
	UINT8       m_data_len;       // interface data length 4 or 8 bit
	UINT8       m_num_line;       // number of lines
	UINT8       m_char_size;      // char size 5x8 or 5x10
	bool        m_blink;
	bool        m_first_cmd;
	bool        m_nibble;

	enum        { DDRAM, CGRAM };
};

// device type definition
extern const device_type HD44780;

#endif
