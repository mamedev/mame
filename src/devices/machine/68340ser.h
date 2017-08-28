// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_68340SER_H
#define MAME_MACHINE_68340SER_H

#pragma once

#include "machine/mc68681.h"

class m68340_serial : public device_t
{
public:
	m68340_serial(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	READ8_MEMBER( read );
	WRITE8_MEMBER( write );	

	//protected:
	required_device<m68340_serial_device> m_duart;
};

DECLARE_DEVICE_TYPE(M68340_SERIAL_MODULE, m68340_serial)

#endif // MAME_MACHINE_68340SER_H
