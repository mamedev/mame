// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    Epson EX-800 dot matrix printer emulation

**********************************************************************/

#pragma once

#ifndef __EPSON_EX800__
#define __EPSON_EX800__

#include "emu.h"
#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/beep.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> epson_ex800_t

class epson_ex800_t :  public device_t,
						public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_ex800_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t devsel_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void devsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gate5a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gate5a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iosel_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iosel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gate7a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gate7a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void online_switch(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;

	int m_irq_state;
};



// device type definition
extern const device_type EPSON_EX800;



#endif
