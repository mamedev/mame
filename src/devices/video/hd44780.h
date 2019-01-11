// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Hitachi HD44780 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_HD44780_H
#define MAME_VIDEO_HD44780_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define HD44780_PIXEL_UPDATE(name) void name(bitmap_ind16 &bitmap, uint8_t line, uint8_t pos, uint8_t y, uint8_t x, int state)


// ======================> hd44780_device

class hd44780_device : public device_t
{
public:
	typedef device_delegate<void (bitmap_ind16 &bitmap, uint8_t line, uint8_t pos, uint8_t y, uint8_t x, int state)> pixel_update_delegate;

	// construction/destruction
	hd44780_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// static configuration helpers
	void set_lcd_size(int lines, int chars) { m_lines = lines; m_chars = chars; }
	template <typename... T> void set_pixel_update_cb(T &&... args) { m_pixel_update_cb = pixel_update_delegate(std::forward<T>(args)...); }
	void set_pixel_update_cb(pixel_update_delegate callback) { m_pixel_update_cb = callback; }
	template <class FunctionClass> void set_pixel_update_cb(const char *devname,
		void (FunctionClass::*callback)(bitmap_ind16 &, uint8_t, uint8_t, uint8_t, uint8_t, int), const char *name)
	{
		set_pixel_update_cb(pixel_update_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass> void set_pixel_update_cb(
		void (FunctionClass::*callback)(bitmap_ind16 &, uint8_t, uint8_t, uint8_t, uint8_t, int), const char *name)
	{
		set_pixel_update_cb(pixel_update_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}

	// device interface
	virtual void write(offs_t offset, u8 data);
	virtual u8 read(offs_t offset);
	virtual void control_write(u8 data);
	virtual u8 control_read();
	virtual void data_write(u8 data);
	virtual u8 data_read();

	const uint8_t *render();
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	hd44780_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	// charset
	enum
	{
		CHARSET_HD44780_A00,
		CHARSET_KS0066_F05 /*,
		CHARSET_HD44780_A01,
		CHARSET_HD44780_A02,
		CHARSET_KS0066_F00,
		CHARSET_KS0066_F03,
		CHARSET_KS0066_F04,
		CHARSET_KS0066_F06,
		CHARSET_KS0066_F59
		*/
	};

	void set_charset_type(int type);

private:
	// internal helper
	void set_busy_flag(uint16_t usec);
	void correct_ac();
	void update_ac(int direction);
	void update_nibble(int rs, int rw);
	void shift_display(int direction);
	void pixel_update(bitmap_ind16 &bitmap, uint8_t line, uint8_t pos, uint8_t y, uint8_t x, int state);

	// internal state
	static constexpr device_timer_id TIMER_BUSY = 0;
	static constexpr device_timer_id TIMER_BLINKING = 1;

	emu_timer * m_blink_timer;
	emu_timer * m_busy_timer;

	uint8_t       m_lines;          // number of lines
	uint8_t       m_chars;          // chars for line
	pixel_update_delegate m_pixel_update_cb; // pixel update callback

	bool        m_busy_flag;      // busy flag
	uint8_t       m_ddram[0x80];    // internal display data RAM
	uint8_t       m_cgram[0x40];    // internal chargen RAM
	uint8_t const *m_cgrom;
	optional_region_ptr<uint8_t> m_cgrom_region; // internal chargen ROM
	int         m_ac;             // address counter
	uint8_t       m_dr;             // data register
	uint8_t       m_ir;             // instruction register
	uint8_t       m_active_ram;     // DDRAM or CGRAM
	bool        m_display_on;     // display on/off
	bool        m_cursor_on;      // cursor on/off
	bool        m_blink_on;       // blink on/off
	bool        m_shift_on;       // shift on/off
	int         m_disp_shift;     // display shift
	int         m_direction;      // auto increment/decrement (-1 or +1)
	uint8_t       m_data_len;       // interface data length 4 or 8 bit
	uint8_t       m_num_line;       // number of lines
	uint8_t       m_char_size;      // char size 5x8 or 5x10
	bool        m_blink;
	bool        m_first_cmd;
	int         m_rs_state;
	int         m_rw_state;
	bool        m_nibble;
	int         m_charset_type;
	uint8_t       m_render_buf[80 * 16];

	enum        { DDRAM, CGRAM };
};

// ======================> ks0066_f05_device

class ks0066_f05_device :  public hd44780_device
{
public:
	// construction/destruction
	ks0066_f05_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(HD44780,    hd44780_device)
DECLARE_DEVICE_TYPE(KS0066_F05, ks0066_f05_device)

#endif // MAME_VIDEO_HD44780_H
