// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Qubbesoft QubIDE emulation

**********************************************************************/

#pragma once

#ifndef __QUBIDE__
#define __QUBIDE__

#include "exp.h"
#include "machine/ataintf.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qubide_t

class qubide_t : public device_t,
					public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	qubide_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	qubide_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int ram_size);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_ql_expansion_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data);
	virtual void write(address_space &space, offs_t offset, UINT8 data);

private:
	required_device<ata_interface_device> m_ata;
	required_memory_region m_rom;
	required_ioport m_j1_j5;

	offs_t m_base;
	UINT16 m_ata_data;
};



// device type definition
extern const device_type QUBIDE;



#endif
