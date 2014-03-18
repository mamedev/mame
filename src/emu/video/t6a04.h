// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Toshiba T6A04 LCD controller

***************************************************************************/

#pragma once

#ifndef __T6A04_H__
#define __T6A04_H__

#define MCFG_T6A04_ADD( _tag, _config ) \
	MCFG_DEVICE_ADD( _tag, T6A04, 0 ) \
	MCFG_DEVICE_CONFIG(_config)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> t6a04_interface

struct t6a04_interface
{
	UINT8 height;           // number of lines
	UINT8 width;            // pixels for line
};

// ======================> t6a04_device

class t6a04_device : public device_t,
						public t6a04_interface
{
public:
	// construction/destruction
	t6a04_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device interface
	DECLARE_WRITE8_MEMBER(control_write);
	DECLARE_READ8_MEMBER(control_read);
	DECLARE_WRITE8_MEMBER(data_write);
	DECLARE_READ8_MEMBER(data_read);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;

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
};

// device type definition
extern const device_type T6A04;

#endif
