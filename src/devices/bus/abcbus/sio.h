// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_BUS_ABCBUS_SIO_H
#define MAME_BUS_ABCBUS_SIO_H

#pragma once

#include "abcbus.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_sio_device

class abc_sio_device :  public device_t,
						public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual uint8_t abcbus_xmemfl(offs_t offset) override;

private:
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_sio;
	required_memory_region m_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_SIO, abc_sio_device)

#endif // MAME_BUS_ABCBUS_SIO_H
