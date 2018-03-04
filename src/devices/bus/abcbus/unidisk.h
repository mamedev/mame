// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MyAB UNI DISK floppy disk controller emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_UNIDISK_H
#define MAME_BUS_ABCBUS_UNIDISK_H

#pragma once

#include "abcbus.h"
#include "cpu/tms9900/tms9995.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> unidisk_t

class unidisk_t :  public device_t,
				   public device_abcbus_card_interface
{
public:
	// construction/destruction
	unidisk_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;

private:
	required_device<cpu_device> m_maincpu;

	bool m_cs;

	void unidisk_io(address_map &map);
	void unidisk_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(UNIDISK, unidisk_t)

#endif // MAME_BUS_ABCBUS_UNIDISK_H
