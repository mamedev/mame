// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom
#ifndef MAME_BUS_ISA_EIS_SAD8852_H
#define MAME_BUS_ISA_EIS_SAD8852_H

#pragma once

#include "isa.h"

class isa16_sad8852_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_sad8852_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(sad8852_r);
	DECLARE_WRITE8_MEMBER(sad8852_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// address maps
	void sad8852_mem(address_map &map);
	void sad8852_io(address_map &map);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// helpers
	required_ioport m_sw1;
	required_ioport m_j1;
	required_ioport m_j2;
	required_ioport m_isairq;
	bool m_installed;
	int m_irq;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA16_SAD8852, isa16_sad8852_device)

#endif  // MAME_BUS_ISA_EIS_SAD8852_H
