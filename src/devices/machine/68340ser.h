// license:BSD-3-Clause
// copyright-holders:David Haywood, Joakim Larsson Edstrom
#ifndef MAME_MACHINE_68340SER_H
#define MAME_MACHINE_68340SER_H

#pragma once

#include "machine/mc68681.h"

class m68340_cpu_device;

class mc68340_serial_module_device : public mc68340_duart_device
{
	friend class m68340_cpu_device;

public:
	mc68340_serial_module_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
	void irq_w(int state);

	uint8_t irq_level() const { return irq_pending() ? (m_ilr & REG_ILR_MASK) : 0; }
	uint8_t irq_vector() const { return m_ivr; }
	uint8_t arbitrate(uint8_t level) const { return (irq_level() == level) ? (m_mcrl & REG_MCRL_ARBLV) : 0; }

	void module_reset();

protected:
	m68340_cpu_device *m_cpu;

	// Module registers not in the DUART part
	uint8_t m_mcrh;
	uint8_t m_mcrl;
	uint8_t m_ilr;
	uint8_t m_ivr;

	enum {
		REG_MCRH    = 0,
		REG_MCRL    = 1,
		REG_ILR     = 4,
		REG_IVR     = 5,
	};

	enum {
		REG_MCRH_STP    = 0x80,
		REG_MCRH_FRZ1   = 0x40,
		REG_MCRH_FRZ2   = 0x20,
		REG_MCRH_ICCS   = 0x10,
	};

	enum {
		REG_MCRL_SUPV   = 0x80,
		REG_MCRL_ARBLV  = 0x0f,
	};

	enum {
		REG_ILR_MASK    = 0x07
	};
};

DECLARE_DEVICE_TYPE(MC68340_SERIAL_MODULE, mc68340_serial_module_device)

#endif // MAME_MACHINE_68340SER_H
