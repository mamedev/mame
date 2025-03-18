// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    DataBoard 4107 Winchester interface emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_DB4107_H
#define MAME_BUS_ABCBUS_DB4107_H

#pragma once

#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "machine/z80dma.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> databoard_4107_device

class databoard_4107_device : public device_t,
							  public device_abcbus_card_interface
{
public:
	// construction/destruction
	databoard_4107_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;

private:
	required_device<z80_device> m_maincpu;
	required_device<z80dma_device> m_dma;

	bool m_cs;

	void databoard_4107_io(address_map &map) ATTR_COLD;
	void databoard_4107_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(DATABOARD_4107, databoard_4107_device)

#endif // MAME_BUS_ABCBUS_DB4107_H
