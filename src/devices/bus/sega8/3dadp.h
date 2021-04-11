// license:BSD-3-Clause
// copyright-holders:Enik Land
#ifndef MAME_BUS_SEGA8_3D_ADAPTOR_H
#define MAME_BUS_SEGA8_3D_ADAPTOR_H

#pragma once

#include "sega8_slot.h"
#include "rom.h"
#include "bus/sms_3d/s3dport.h"


// ======================> sega8_3d_adaptor_device

class sega8_3d_adaptor_device : public sega8_rom_device
{
public:
	// construction/destruction
	sega8_3d_adaptor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_resolve_objects() override;

	// writing
	virtual DECLARE_WRITE_LINE_MEMBER(write_sscope) override { m_port_3d->write_sscope(state); }

private:
	required_device<sms_3d_port_device> m_port_3d;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA8_ROM_3D_ADAPTOR, sega8_3d_adaptor_device)

#endif // MAME_BUS_SEGA8_3D_ADAPTOR_H
