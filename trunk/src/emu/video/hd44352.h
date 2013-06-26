/***************************************************************************

        Hitachi HD44352 LCD controller

***************************************************************************/

#pragma once

#ifndef __hd44352_H__
#define __hd44352_H__


#define MCFG_HD44352_ADD( _tag, _clock, _config) \
	MCFG_DEVICE_ADD( _tag, HD44352, _clock ) \
	MCFG_DEVICE_CONFIG( _config )

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> hd44352_interface

struct hd44352_interface
{
	devcb_write_line    m_on_cb;        // ON line
};

// ======================> hd44352_device

class hd44352_device :
						public device_t,
						public hd44352_interface
{
public:
	// construction/destruction
	hd44352_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device interface
	UINT8 data_read();
	void data_write(UINT8 data);
	void control_write(UINT8 data);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;

private:
	UINT8 compute_newval(UINT8 type, UINT8 oldval, UINT8 newval);
	UINT8 get_char(UINT16 pos);

	static const device_timer_id ON_TIMER = 1;
	emu_timer *m_on_timer;

	UINT8 m_video_ram[2][0x180];
	UINT8 m_control_lines;
	UINT8 m_data_bus;
	UINT8 m_par[3];
	UINT8 m_state;
	UINT16 m_bank;
	UINT16 m_offset;
	UINT8 m_char_width;
	UINT8 m_lcd_on;
	UINT8 m_scroll;
	UINT32 m_contrast;

	UINT8 m_custom_char[4][8];      // 4 chars * 8 bytes
	UINT8 m_byte_count;
	UINT8 m_cursor_status;
	UINT8 m_cursor[8];
	UINT8 m_cursor_x;
	UINT8 m_cursor_y;
	UINT8 m_cursor_lcd;

	devcb_resolved_write_line m_on;         // ON line callback
};

// device type definition
extern const device_type HD44352;

#endif
