#pragma once

#ifndef __IQ151_GRAFIK_H__
#define __IQ151_GRAFIK_H__

#include "emu.h"
#include "machine/iq151cart.h"
#include "machine/i8255.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iq151_grafik_device

class iq151_grafik_device :
		public device_t,
		public device_iq151cart_interface
{
public:
	// construction/destruction
	iq151_grafik_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// ppi8255 callback
	DECLARE_WRITE8_MEMBER(x_write);
	DECLARE_WRITE8_MEMBER(y_write);
	DECLARE_WRITE8_MEMBER(control_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "iq151_grafik"; }

	// iq151cart_interface overrides
	virtual void io_read(offs_t offset, UINT8 &data);
	virtual void io_write(offs_t offset, UINT8 data);
	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);

private:

	required_device<i8255_device> m_ppi8255;

	UINT8 * 	m_videoram;
	UINT8		m_posx;		// horizontal position
	UINT8		m_posy;		// vertical position
	UINT8		m_all;		// 0: bit mode 1: byte mode
	UINT8		m_pen;
	UINT8		m_fast;
	UINT8		m_ev;		// enable video out
	UINT8		m_ex;
	UINT8		m_sel;		// enable vram access
};


// device type definition
extern const device_type IQ151_GRAFIK;

#endif  /* __IQ151_GRAFIK_H__ */
