#pragma once

#ifndef __ISA_IDE8_H__
#define __ISA_IDE8_H__

#include "emu.h"
#include "machine/isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_ide_device

class isa8_ide_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual const rom_entry *device_rom_region() const;

	//bool is_primary() { return m_is_primary; }
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);

	UINT8 get_latch_in() { return data_high_in; }
	void set_latch_in(UINT8 new_latch) { data_high_in=new_latch; }
	UINT8 get_latch_out() { return data_high_out; }
	void set_latch_out(UINT8 new_latch) { data_high_out=new_latch; }
	

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "isa_ide8"; }

private:
	// internal state
//	bool m_is_primary;
	
	// Interupt request
	UINT8	irq;
	
	// Data latch for high byte in and out
	UINT8 data_high_in;
	UINT8 data_high_out;
};


// device type definition
extern const device_type ISA8_IDE;

#endif  /* __ISA_IDE_H__ */
