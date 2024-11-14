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

#define HD44780_PIXEL_UPDATE(name) void name(bitmap_ind16 &bitmap, u8 line, u8 pos, u8 y, u8 x, int state)


// device type declarations
DECLARE_DEVICE_TYPE(HD44780,  hd44780_device)
DECLARE_DEVICE_TYPE(HD44780U, hd44780u_device)
DECLARE_DEVICE_TYPE(SED1278,  sed1278_device)
DECLARE_DEVICE_TYPE(KS0066,   ks0066_device)


class hd44780_device : public device_t
{
public:
	typedef device_delegate<void (bitmap_ind16 &bitmap, u8 line, u8 pos, u8 y, u8 x, int state)> pixel_update_delegate;

	// construction/destruction
	hd44780_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	void set_lcd_size(int lines, int chars) { m_lines = lines; m_chars = chars; }
	template <typename... T> void set_pixel_update_cb(T &&... args) { m_pixel_update_cb.set(std::forward<T>(args)...); }
	void set_busy_factor(float f) { m_busy_factor = f; } // it's a workaround for inaccurate busy flag emulation
	void set_function_set_at_any_time(bool v = true) { m_function_set_at_any_time = v; }

	// device interface
	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);
	void control_w(u8 data) { write(0, data); }
	u8 control_r() { return read(0); }
	void data_w(u8 data) { write(1, data); }
	u8 data_r() { return read(1); }

	u8 db_r();
	void db_w(u8 data);
	void rs_w(int state);
	void rw_w(int state);
	void e_w(int state);

	const u8 *render();
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	hd44780_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);
	TIMER_CALLBACK_MEMBER(blink_tick);

	virtual void control_write(u8 data);
	virtual u8 control_read();
	virtual void data_write(u8 data);
	virtual u8 data_read();

private:
	enum        { DDRAM, CGRAM };

	// internal helper
	void set_busy_flag(uint16_t cycles);
	void correct_ac();
	void update_ac(int direction);
	void update_nibble(int rs, int rw);
	void shift_display(int direction);
	void pixel_update(bitmap_ind16 &bitmap, u8 line, u8 pos, u8 y, u8 x, int state);

	// internal state
	emu_timer * m_busy_timer;
	emu_timer * m_blink_timer;

	u8           m_lines;          // number of lines
	u8           m_chars;          // chars for line
	pixel_update_delegate m_pixel_update_cb; // pixel update callback

	float       m_busy_factor;
	bool        m_busy_flag;      // busy flag
	u8          m_ddram[0x80];    // internal display data RAM
	u8          m_cgram[0x40];    // internal chargen RAM
	u8 const *m_cgrom;
	optional_region_ptr<u8> m_cgrom_region; // internal chargen ROM
	int         m_ac;             // address counter
	u8          m_dr;             // data register
	u8          m_ir;             // instruction register
	u8          m_active_ram;     // DDRAM or CGRAM
	bool        m_display_on;     // display on/off
	bool        m_cursor_on;      // cursor on/off
	bool        m_blink_on;       // blink on/off
	bool        m_shift_on;       // shift on/off
	int         m_disp_shift;     // display shift
	int         m_direction;      // auto increment/decrement (-1 or +1)
	u8          m_data_len;       // interface data length 4 or 8 bit
	u8          m_num_line;       // number of lines
	u8          m_char_size;      // char size 5x8 or 5x10
	bool        m_blink;
	bool        m_first_cmd;
	int         m_rs_input;
	int         m_rw_input;
	u8          m_db_input;
	bool        m_enabled;
	int         m_rs_state;
	int         m_rw_state;
	bool        m_nibble;
	u8          m_render_buf[80 * 16];
	bool        m_function_set_at_any_time;
};


class hd44780u_device : public hd44780_device
{
public:
	// construction/destruction
	hd44780u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static auto parent_rom_device_type() { return &HD44780; }

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


class sed1278_device : public hd44780_device
{
public:
	// construction/destruction
	sed1278_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static auto parent_rom_device_type() { return &HD44780; }

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


class ks0066_device : public hd44780_device
{
public:
	// construction/destruction
	ks0066_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static auto parent_rom_device_type() { return &HD44780; }

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

#endif // MAME_VIDEO_HD44780_H
