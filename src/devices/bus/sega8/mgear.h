// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SEGA8_MGEAR_H
#define __SEGA8_MGEAR_H

#include "sega8_slot.h"
#include "rom.h"


// ======================> sega8_mgear_device

class sega8_mgear_device : public sega8_rom_device
{
public:
	// construction/destruction
	sega8_mgear_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override { return m_subslot->read_cart(space, offset); }
	virtual DECLARE_WRITE8_MEMBER(write_cart) override { m_subslot->write_cart(space, offset, data); }
	virtual DECLARE_WRITE8_MEMBER(write_mapper) override { m_subslot->write_mapper(space, offset, data); }
	virtual int get_lphaser_xoffs() override { return m_subslot->m_cart ? m_subslot->m_cart->get_lphaser_xoffs() : -1; }

	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	required_device<sega8_cart_slot_device> m_subslot;
};


// device type definition
extern const device_type SEGA8_ROM_MGEAR;


#endif
