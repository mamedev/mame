// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VCS_COMPUMAT_H
#define __VCS_COMPUMAT_H

#include "rom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a26_rom_cm_device

class a26_rom_cm_device : public a26_rom_f6_device
{
public:
	// construction/destruction
	a26_rom_cm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
};


// device type definition
extern const device_type A26_ROM_COMPUMATE;

#endif
