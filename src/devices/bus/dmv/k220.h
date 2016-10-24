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
	dmv_k220_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_out0(int state);
	void write_out1(int state);
	void write_out2(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// dmvcart_interface overrides
	virtual bool read(offs_t offset, uint8_t &data) override;
	virtual bool write(offs_t offset, uint8_t data) override;

private:
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_memory_region m_ram;
	required_memory_region m_rom;

	uint8_t   m_portc;
};


// device type definition
extern const device_type DMV_K220;

#endif  /* __DMV_K220_H__ */
