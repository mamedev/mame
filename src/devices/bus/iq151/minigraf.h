// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __IQ151_MINIGRAF_H__
#define __IQ151_MINIGRAF_H__

#include "emu.h"
#include "iq151.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iq151_minigraf_device

class iq151_minigraf_device :
		public device_t,
		public device_iq151cart_interface
{
public:
	// construction/destruction
	iq151_minigraf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;

	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void io_write(offs_t offset, UINT8 data) override;

	// Aritma MINIGRAF 0507
	void plotter_update(UINT8 control);
	int get_direction(UINT8 old_val, UINT8 new_val);

private:

	UINT8 *     m_rom;
	INT16       m_posx;
	INT16       m_posy;
	UINT8       m_pen;
	UINT8       m_control;

	std::unique_ptr<bitmap_ind16>  m_paper;
};


// device type definition
extern const device_type IQ151_MINIGRAF;

#endif  /* __IQ151_MINIGRAF_H__ */
