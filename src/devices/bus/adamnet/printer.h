// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam printer controller emulation

**********************************************************************/

#ifndef MAME_BUS_ADAMNET_PRINTER_H
#define MAME_BUS_ADAMNET_PRINTER_H

#pragma once

#include "adamnet.h"
#include "cpu/m6800/m6801.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_printer_device

class adam_printer_device :  public device_t,
								public device_adamnet_card_interface
{
public:
	// construction/destruction
	adam_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

private:
	required_device<m6801_cpu_device> m_maincpu;

	void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
	uint8_t p3_r();
	uint8_t p4_r();
	void p4_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(ADAM_PRN, adam_printer_device)

#endif // MAME_BUS_ADAMNET_PRINTER_H
