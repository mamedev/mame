// license:BSD-3-Clause
// copyright-holders:Felipe Sanches, Sandro Ronco
/***************************************************************************

        NT7534 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_NT7534_H
#define MAME_VIDEO_NT7534_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define NT7534_PIXEL_UPDATE(name) void name(bitmap_ind16 &bitmap, uint8_t line, uint8_t pos, uint8_t y, uint8_t x, int state)


// ======================> nt7534_device

class nt7534_device : public device_t
{
public:
	typedef device_delegate<void (bitmap_ind16 &bitmap, uint8_t line, uint8_t pos, uint8_t y, uint8_t x, int state)> pixel_update_delegate;

	// construction/destruction
	nt7534_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename... T> void set_pixel_update_cb(T &&... args) { m_pixel_update_cb.set(std::forward<T>(args)...); }

	// device interface
	virtual void write(offs_t offset, uint8_t data);
	virtual uint8_t read(offs_t offset);
	virtual void control_write(uint8_t data);
	virtual uint8_t control_read();
	virtual void data_write(uint8_t data);
	virtual uint8_t data_read();

	const uint8_t *render();
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	nt7534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

private:
	// internal helper
	void set_busy_flag(uint16_t usec);
	void update_nibble(int rs, int rw);

	// internal state
	emu_timer * m_busy_timer;

	pixel_update_delegate m_pixel_update_cb; // pixel update callback

	bool        m_busy_flag;
	uint8_t     m_ddram[9*132];        // internal display data RAM
	uint8_t     m_page;                // page address
	uint16_t    m_column;              // column address
	uint16_t    m_backup_column;       // column address
	uint8_t     m_display_start_line;
	uint8_t     m_dr;                  // data register
	uint8_t     m_ir;                  // instruction register
	bool        m_display_on;          // display on/off
	bool        m_entire_display_on;
	bool        m_reverse;             // Reverse display
	bool        m_read_modify_write;
	bool        m_adc;
	int         m_direction;           // auto increment/decrement (-1 or +1)
	uint8_t     m_data_len;            // interface data length 4 or 8 bit
	int         m_rs_state;
	int         m_rw_state;
	bool        m_nibble;
};

// device type definition
DECLARE_DEVICE_TYPE(NT7534,    nt7534_device)

#endif // MAME_VIDEO_NT7534_H
