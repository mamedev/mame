// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_IQ151_MS151A_H
#define MAME_BUS_IQ151_MS151A_H

#pragma once

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
	iq151_ms151a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void io_read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

	// XY 4130/4131
	uint8_t plotter_status();
	void plotter_update(uint8_t offset, uint8_t data);

private:

	uint8_t *     m_rom;
	int32_t       m_posx;
	int32_t       m_posy;
	uint8_t       m_pen;

	std::unique_ptr<bitmap_ind16> m_paper;
};


// device type definition
DECLARE_DEVICE_TYPE(IQ151_MS151A, iq151_ms151a_device)

#endif // MAME_BUS_IQ151_MS151A_H
