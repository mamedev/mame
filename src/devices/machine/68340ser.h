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

	// Module registers not in the DUART part
	uint8_t m_mcrh;
	uint8_t m_mcrl;
	uint8_t m_ilr;
	uint8_t m_ivr;

	enum {
		REG_MCRH	= 0,
		REG_MCRL	= 1,
		REG_ILR		= 4,
		REG_IVR		= 5,
	};

	enum {
		REG_MCRH_STP	= 0x80,
		REG_MCRH_FRZ1	= 0x40,
		REG_MCRH_FRZ2	= 0x20,
		REG_MCRH_ICCS	= 0x10,
	};
	
	enum {
		REG_MCRL_SUPV	= 0x80,
		REG_MCRL_ARBLV	= 0x0f,
	};

	enum {
		REG_ILR_MASK	= 0x07
	};
};

DECLARE_DEVICE_TYPE(M68340_SERIAL_MODULE, m68340_serial)

#endif // MAME_MACHINE_68340SER_H
