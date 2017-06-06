// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Qubbesoft QubIDE emulation

**********************************************************************/

#ifndef MAME_BUS_QL_QUBIDE_H
#define MAME_BUS_QL_QUBIDE_H

#pragma once

#include "exp.h"
#include "machine/ataintf.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qubide_device

class qubide_device : public device_t, public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	qubide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_ql_expansion_card_interface overrides
	virtual uint8_t read(address_space &space, offs_t offset, uint8_t data) override;
	virtual void write(address_space &space, offs_t offset, uint8_t data) override;

private:
	required_device<ata_interface_device> m_ata;
	required_memory_region m_rom;
	required_ioport m_j1_j5;

	offs_t m_base;
	uint16_t m_ata_data;
};



// device type definition
DECLARE_DEVICE_TYPE(QUBIDE, qubide_device)

#endif // MAME_BUS_QL_QUBIDE_H
