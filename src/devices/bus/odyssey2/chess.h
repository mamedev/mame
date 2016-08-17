// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __O2_CHESS_H
#define __O2_CHESS_H

#include "slot.h"
#include "rom.h"
#include "cpu/z80/z80.h"


// ======================> o2_chess_device

class o2_chess_device : public o2_rom_device
{
	virtual machine_config_constructor device_mconfig_additions() const override;
//  virtual const rom_entry *device_rom_region() const;

public:
	// construction/destruction
	o2_chess_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

private:
	required_device<nsc800_device> m_cpu;
};



// device type definition
extern const device_type O2_ROM_CHESS;


#endif
