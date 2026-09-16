// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-603 Coleco Game Adapter for SVI-318/328

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_EXPANDER_SV603_H
#define MAME_BUS_SVI3X8_EXPANDER_SV603_H

#pragma once

#include "expander.h"
#include "sound/sn76496.h"
#include "bus/coleco/cartridge/exp.h"
#include "bus/coleco/controller/ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sv603_device : public device_t, public device_svi_expander_interface
{
public:
	// construction/destruction
	sv603_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

	template<int N> void joy_irq_w(int state);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_bios;
	required_device<sn76489a_device> m_snd;
	required_device<colecovision_control_port_device> m_joy[2];
	required_device<colecovision_cartridge_slot_device> m_cart;
};

// device type declaration
DECLARE_DEVICE_TYPE(SV603, sv603_device)

#endif // MAME_BUS_SVI3X8_EXPANDER_SV603_H
