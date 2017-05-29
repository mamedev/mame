// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A7800_HISCORE_H
#define MAME_BUS_A7800_HISCORE_H

#pragma once

#include "a78_slot.h"
#include "rom.h"


// ======================> a78_hiscore_device

class a78_hiscore_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_hiscore_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_04xx) override;
	virtual DECLARE_READ8_MEMBER(read_10xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_10xx) override;
	virtual DECLARE_READ8_MEMBER(read_30xx) override;
	virtual DECLARE_READ8_MEMBER(read_40xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_40xx) override;

protected:
	required_device<a78_cart_slot_device> m_hscslot;
};



// device type definition
DECLARE_DEVICE_TYPE(A78_HISCORE, a78_hiscore_device)


#endif // MAME_BUS_A7800_HISCORE_H
