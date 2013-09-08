#pragma once

#ifndef __ISA_IDE8_H__
#define __ISA_IDE8_H__

#include "emu.h"
#include "machine/idectrl.h"
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

	DECLARE_READ8_MEMBER(ide8_r);
	DECLARE_WRITE8_MEMBER(ide8_w);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "isa_ide8"; }

private:
	required_device<ata_interface_device> m_ata;

	UINT8 m_irq_number;
	UINT8 m_d8_d15_latch;
};


// device type definition
extern const device_type ISA8_IDE;

#endif  /* __ISA_IDE_H__ */
