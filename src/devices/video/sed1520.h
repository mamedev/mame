// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        SED1520 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_SED1520_H
#define MAME_VIDEO_SED1520_H

#pragma once


#define SED1520CB_UPDATE(cls, fnc) sed1520_device::screen_update_delegate((&cls::fnc), (#cls "::" #fnc), DEVICE_SELF, ((cls *)nullptr))
#define SED1520CB_DEVUPDATE(tag, cls, fnc) sed1520_device::screen_update_delegate((&cls::fnc), (#cls "::" #fnc), (tag), ((cls *)nullptr))

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define SED1520_UPDATE_CB(name) uint32_t name(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *vram, int start_line, int adc)


// ======================> sed1520_device

class sed1520_device :  public device_t
{
public:
	typedef device_delegate<uint32_t (bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *vram, int start_line, int adc)> screen_update_delegate;

	// construction/destruction
	sed1520_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// sconfiguration helpers
	template <typename... T> void set_screen_update_cb(T &&... args) { m_screen_update_cb.set(std::forward<T>(args)...); }

	// device interface
	virtual DECLARE_WRITE8_MEMBER(write);
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(control_write);
	virtual DECLARE_READ8_MEMBER(status_read);
	virtual DECLARE_WRITE8_MEMBER(data_write);
	virtual DECLARE_READ8_MEMBER(data_read);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint8_t     m_lcd_on;
	uint8_t     m_busy;
	uint8_t     m_page;
	uint8_t     m_column;
	uint8_t     m_old_column;
	uint8_t     m_start_line;
	uint8_t     m_adc;
	uint8_t     m_static_drive;
	bool        m_modify_write;
	screen_update_delegate m_screen_update_cb;

	uint8_t     m_vram[0x140];
};


// device type definition
DECLARE_DEVICE_TYPE(SED1520, sed1520_device)

#endif // MAME_VIDEO_SED1520_H
