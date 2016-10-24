// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#pragma once

#ifndef __ISA_IDE_H__
#define __ISA_IDE_H__

#include "emu.h"
#include "isa.h"
#include "machine/idectrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_ide_device

class isa16_ide_device : public device_t,
	public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	bool is_primary() { return m_is_primary; }
	void ide_interrupt(int state);
	DECLARE_ADDRESS_MAP(map, 16);
	DECLARE_ADDRESS_MAP(alt_map, 8);
	uint8_t ide16_alt_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void ide16_alt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	bool m_is_primary;
	required_device<ide_controller_device> m_ide;
};


// device type definition
extern const device_type ISA16_IDE;

#endif  /* __ISA_IDE_H__ */
