// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    SED1520 LCD controller
    SED1560 LCD controller
    EPL43102 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_SED1520_H
#define MAME_VIDEO_SED1520_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define SED1520_UPDATE_CB(name) uint32_t name(bitmap_ind16 &bitmap, const rectangle &cliprect, bool lcd_on, uint8_t *dram, uint8_t start_line, uint8_t adc, uint8_t duty)
#define SED1560_UPDATE_CB(name) uint32_t name(bitmap_ind16 &bitmap, const rectangle &cliprect, bool lcd_on, uint8_t *dram, uint8_t start_line, uint8_t adc, uint8_t duty, bool reverse, bool fill, uint8_t contrast, bool line_inv, uint8_t line_inv_num)
#define EPL43102_UPDATE_CB(name) uint32_t name(bitmap_ind16 &bitmap, const rectangle &cliprect, bool lcd_on, uint8_t *dram, uint8_t start_line, uint8_t adc, uint8_t duty, bool reverse, bool fill, uint8_t contrast, bool line_inv, uint8_t line_inv_num)


// ======================> sed15xx_device_base

class sed15xx_device_base : public device_t
{
public:
	// device interface
	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);
	virtual void control_write(uint8_t data) = 0;
	virtual uint8_t status_read();
	virtual void data_write(uint8_t data);
	virtual uint8_t data_read();

protected:
	// construction/destruction
	sed15xx_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t ddr_size, uint32_t page_size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

protected:
	const uint32_t m_ddr_size;
	const uint32_t m_page_size;

	// internal state
	uint8_t     m_lcd_on;
	uint8_t     m_busy;
	uint8_t     m_page;
	uint8_t     m_column;
	uint8_t     m_old_column;
	uint8_t     m_start_line;
	uint8_t     m_adc;
	bool        m_modify_write;
	uint8_t     m_data;
	uint8_t     m_duty;

	std::unique_ptr<uint8_t[]>  m_ddr;
};


// ======================> sed1520_device

class sed1520_device : public sed15xx_device_base
{
public:
	typedef device_delegate<uint32_t (bitmap_ind16 &bitmap, const rectangle &cliprect, bool lcd_on, uint8_t *dram, uint8_t start_line, uint8_t adc, uint8_t duty)> sed1520_update_delegate;

	// construction/destruction
	sed1520_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	template <typename... T> void set_screen_update_cb(T &&... args) { m_screen_update_cb.set(std::forward<T>(args)...); }

	// device interface
	virtual void control_write(uint8_t data) override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	sed1520_update_delegate m_screen_update_cb;

	// internal state
	bool m_static_drive;
};


// ======================> sed1560_device

class sed1560_device : public sed15xx_device_base
{
public:
	typedef device_delegate<uint32_t (bitmap_ind16 &bitmap, const rectangle &cliprect, bool lcd_on, uint8_t *dram, uint8_t start_line, uint8_t adc, uint8_t duty, bool reverse, bool fill, uint8_t contrast, bool line_inv, uint8_t line_inv_num)> sed1560_update_delegate;

	// construction/destruction
	sed1560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	template <typename... T> void set_screen_update_cb(T &&... args) { m_screen_update_cb.set(std::forward<T>(args)...); }

	// device interface
	virtual void control_write(uint8_t data) override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	sed1560_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t ddr_size, uint32_t page_size);

	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	sed1560_update_delegate m_screen_update_cb;

	// internal state
	bool    m_reverse;
	bool    m_fill;
	bool    m_line_inv;
	uint8_t m_line_inv_num;
protected:
	uint8_t m_contrast;
};


// ======================> epl43102_device

class epl43102_device : public sed1560_device
{
public:
	// construction/destruction
	epl43102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void control_write(uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	uint8_t m_last_command;
};


// device type definition
DECLARE_DEVICE_TYPE(SED1520, sed1520_device)
DECLARE_DEVICE_TYPE(SED1560, sed1560_device)
DECLARE_DEVICE_TYPE(EPL43102, epl43102_device)

#endif // MAME_VIDEO_SED1520_H
