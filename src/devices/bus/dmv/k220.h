// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_K220_H__
#define __DMV_K220_H__

#include "emu.h"
#include "dmvbus.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_k220_device

class dmv_k220_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k220_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE_LINE_MEMBER(write_out0);
	DECLARE_WRITE_LINE_MEMBER(write_out1);
	DECLARE_WRITE_LINE_MEMBER(write_out2);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// dmvcart_interface overrides
	virtual bool read(offs_t offset, UINT8 &data);
	virtual bool write(offs_t offset, UINT8 data);

private:
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_memory_region m_ram;
	required_memory_region m_rom;

	UINT8   m_portc;
};


// device type definition
extern const device_type DMV_K220;

#endif  /* __DMV_K220_H__ */
