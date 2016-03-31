// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pc_xporter.h

    Implementation of the Applied Engineering PC Transporter card

*********************************************************************/

#pragma once

#include "emu.h"
#include "a2bus.h"
#include "machine/genpc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_pcxporter_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_pcxporter_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_pcxporter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;
	virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_c800(address_space &space, UINT16 offset) override;
	virtual void write_c800(address_space &space, UINT16 offset, UINT8 data) override;

private:
	UINT8 m_ram[768*1024];
	UINT8 m_regs[0x400];
	UINT32 m_offset;
};

// device type definition
extern const device_type A2BUS_PCXPORTER;
