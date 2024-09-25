// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Toshiba T6A04 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_T6A04_H
#define MAME_VIDEO_T6A04_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> t6a04_device

class t6a04_device : public device_t
{
public:
	// construction/destruction
	t6a04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_size(int w, int h)
	{
		m_width = w;
		m_height = h;
	}

	// device interface
	void control_write(uint8_t data);
	uint8_t control_read();
	void data_write(uint8_t data);
	uint8_t data_read();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override;

private:
	uint8_t m_busy_flag;
	uint8_t m_lcd_ram[960];   //7680 bit (64*120)
	uint8_t m_display_on;
	uint8_t m_contrast;
	uint8_t m_xpos;
	uint8_t m_ypos;
	uint8_t m_zpos;
	int8_t  m_direction;
	uint8_t m_active_counter;
	uint8_t m_word_len;
	uint8_t m_opa1;
	uint8_t m_opa2;
	uint8_t m_output_reg;

	uint8_t m_height;           // number of lines
	uint8_t m_width;            // pixels for line
};

// device type definition
DECLARE_DEVICE_TYPE(T6A04, t6a04_device)

#endif // MAME_VIDEO_T6A04_H
