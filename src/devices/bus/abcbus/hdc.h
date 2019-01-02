// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor XEBEC Winchester controller card emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_HDC_H
#define MAME_BUS_ABCBUS_HDC_H

#pragma once

#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "bus/scsi/scsihd.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_hdc_device

class abc_hdc_device :  public device_t,
						public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;

private:
	required_device<z80_device> m_maincpu;

	void abc_hdc_io(address_map &map);
	void abc_hdc_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_HDC, abc_hdc_device)

#endif // MAME_BUS_ABCBUS_HDC_H
