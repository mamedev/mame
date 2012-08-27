/***************************************************************************

        Hitachi HD44780 LCD controller

***************************************************************************/

#pragma once

#ifndef __HD44780_H__
#define __HD44780_H__


#define MCFG_HD44780_ADD( _tag , _config) \
	MCFG_DEVICE_ADD( _tag, HD44780, 0 ) \
	MCFG_DEVICE_CONFIG(_config)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define HD44780_INTERFACE(name) \
	const hd44780_interface (name) =

typedef void (*hd44780_pixel_update_func)(device_t &device, bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state);
#define HD44780_PIXEL_UPDATE(name) void name(device_t &device, bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state)

// ======================> hd44780_interface

struct hd44780_interface
{
	UINT8 height;			// number of lines
	UINT8 width;			// chars for line
	hd44780_pixel_update_func pixel_update_func;	// pixel update callback
};

// ======================> hd44780_device

class hd44780_device :	public device_t,
						public hd44780_interface
{

public:
	// construction/destruction
	hd44780_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	hd44780_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// device interface
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
	virtual void device_config_complete();

protected:
	// internal helper
	void set_busy_flag(UINT16 usec);
	void update_ac(void);
	void pixel_update(bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state);
	// internal state
	static const device_timer_id BUSY_TIMER = 0;
	static const device_timer_id BLINKING_TIMER = 1;

	emu_timer *m_blink_timer;
	emu_timer *m_busy_timer;

	UINT8 m_busy_flag;

	UINT8 m_ddram[0x80];	//internal display data RAM
	UINT8 m_cgram[0x40];	//internal chargen RAM

	INT8 m_ac;				//address counter
	UINT8 m_ac_mode;		//0=DDRAM 1=CGRAM
	UINT8 m_data_bus_flag;	//0=none 1=write 2=read

	INT8 m_cursor_pos;		//cursor position
	UINT8 m_display_on;		//display on/off
	UINT8 m_cursor_on;		//cursor on/off
	UINT8 m_blink_on;		//blink on/off
	UINT8 m_shift_on;		//shift  on/off
	INT8 m_disp_shift;		//display shift

	INT8 m_direction;		//auto increment/decrement
	UINT8 m_data_len;		//interface data length 4 or 8 bit
	UINT8 m_num_line;		//number of lines
	UINT8 m_char_size;		//char size 5x8 or 5x10

	UINT8 m_blink;
};

// device type definition
extern const device_type HD44780;

#endif
