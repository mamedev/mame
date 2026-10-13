// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 dual SID socket adapter emulation

**********************************************************************/

#ifndef MAME_BUS_C64_DUALSID_H
#define MAME_BUS_C64_DUALSID_H

#pragma once

#include "sid.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_sid_dual6581_device

class c64_sid_dual6581_device : public device_t, public device_c64_sid_interface
{
public:
	c64_sid_dual6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_c64_sid_interface implementation
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	c64_sid_dual6581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void sid_config(mos6581_device &sid) ATTR_COLD;

	required_device<mos6581_device> m_sid1;
	required_device<mos6581_device> m_sid2;
	required_ioport m_config;

private:
	bool m_a8;

	bool sid2_selected(offs_t offset) const { return BIT(offset, m_a8 ? 8 : 5); }
};


// ======================> c64_sid_dual8580_device

class c64_sid_dual8580_device : public c64_sid_dual6581_device
{
public:
	c64_sid_dual8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(C64_SID_DUAL6581, c64_sid_dual6581_device)
DECLARE_DEVICE_TYPE(C64_SID_DUAL8580, c64_sid_dual8580_device)

#endif // MAME_BUS_C64_DUALSID_H
