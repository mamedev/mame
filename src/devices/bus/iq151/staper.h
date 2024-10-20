// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_IQ151_STAPER_H
#define MAME_BUS_IQ151_STAPER_H

#pragma once

#include "iq151.h"
#include "machine/i8255.h"
#include "imagedev/printer.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iq151_staper_device

class iq151_staper_device :
		public device_t,
		public device_iq151cart_interface
{
public:
	// construction/destruction
	iq151_staper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// iq151cart_interface overrides
	virtual void io_read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

private:
	// i8255 callbacks
	uint8_t ppi_porta_r();
	void ppi_portb_w(uint8_t data);
	void ppi_portc_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(pc2_low_tick);

	required_device<i8255_device>           m_ppi;
	required_device<printer_image_device>   m_printer;

	emu_timer*      m_printer_timer;
	uint8_t           m_ppi_portc;
};


// device type definition
DECLARE_DEVICE_TYPE(IQ151_STAPER, iq151_staper_device)

#endif // MAME_BUS_IQ151_STAPER_H
