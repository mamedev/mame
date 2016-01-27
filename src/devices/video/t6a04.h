// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Toshiba T6A04 LCD controller

***************************************************************************/

#pragma once

#ifndef __T6A04_H__
#define __T6A04_H__

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> t6a04_device

class t6a04_device : public device_t
{
public:
	// construction/destruction
	t6a04_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_size(device_t &device, int w, int h)
	{
		t6a04_device &dev = downcast<t6a04_device &>(device);
		dev.m_width = w;
		dev.m_height = h;
	}

	// device interface
	DECLARE_WRITE8_MEMBER(control_write);
	DECLARE_READ8_MEMBER(control_read);
	DECLARE_WRITE8_MEMBER(data_write);
	DECLARE_READ8_MEMBER(data_read);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_validity_check(validity_checker &valid) const override;

private:
	UINT8 m_busy_flag;
	UINT8 m_lcd_ram[960];   //7680 bit (64*120)
	UINT8 m_display_on;
	UINT8 m_contrast;
	UINT8 m_xpos;
	UINT8 m_ypos;
	UINT8 m_zpos;
	INT8  m_direction;
	UINT8 m_active_counter;
	UINT8 m_word_len;
	UINT8 m_opa1;
	UINT8 m_opa2;
	UINT8 m_output_reg;

	UINT8 m_height;           // number of lines
	UINT8 m_width;            // pixels for line
};

// device type definition
extern const device_type T6A04;

#define MCFG_T6A04_SIZE(_width, _height) \
	t6a04_device::set_size(*device, _width, _height);

#endif
