// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Oxford Computer Systems Interpod IEC to IEEE interface emulation

*********************************************************************/

#ifndef MAME_BUS_CBMIEC_INTERPOD_H
#define MAME_BUS_CBMIEC_INTERPOD_H

#pragma once

#include "cbmiec.h"
#include "bus/rs232/rs232.h"
#include "bus/ieee488/ieee488.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/mos6530n.h"
#include "machine/6850acia.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> interpod_t

class interpod_t : public device_t,
				   public device_cbm_iec_interface
{
public:
	// construction/destruction
	interpod_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<mos6532_new_device> m_riot;
	required_device<acia6850_device> m_acia;
	required_device<ieee488_device> m_ieee;
	required_device<rs232_port_device> m_rs232;

	void interpod_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(INTERPOD, interpod_t)


#endif // MAME_BUS_CBMIEC_INTERPOD_H
