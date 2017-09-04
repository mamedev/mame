// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_68340SER_H
#define MAME_MACHINE_68340SER_H

#pragma once

#include "machine/mc68681.h"

class m68340_cpu_device;

class m68340_serial : public m68340_serial_device
{
	friend class m68340_cpu_device;

public:
	m68340_serial(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	READ8_MEMBER( read ) override;
	WRITE8_MEMBER( write ) override;
	DECLARE_WRITE_LINE_MEMBER(irq_w);

protected:
	m68340_cpu_device *m_cpu;

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
