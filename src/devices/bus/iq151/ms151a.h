// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __IQ151_MS151A_H__
#define __IQ151_MS151A_H__

#include "emu.h"
#include "iq151.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iq151_ms151a_device

class iq151_ms151a_device :
		public device_t,
		public device_iq151cart_interface
{
public:
	// construction/destruction
	iq151_ms151a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_stop();

	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data);
	virtual void io_read(offs_t offset, UINT8 &data);
	virtual void io_write(offs_t offset, UINT8 data);

	// XY 4130/4131
	UINT8 plotter_status();
	void plotter_update(UINT8 offset, UINT8 data);

private:

	UINT8 *     m_rom;
	INT32       m_posx;
	INT32       m_posy;
	UINT8       m_pen;

	bitmap_ind16 *  m_paper;
};


// device type definition
extern const device_type IQ151_MS151A;

#endif  /* __IQ151_MS151A_H__ */
