// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ODYSSEY2_CHESS_H
#define MAME_BUS_ODYSSEY2_CHESS_H

#pragma once

#include "slot.h"
#include "rom.h"
#include "cpu/z80/z80.h"


// ======================> o2_chess_device

class o2_chess_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_chess_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
//  virtual const rom_entry *device_rom_region() const;

private:
	required_device<nsc800_device> m_cpu;

	void chess_io(address_map &map);
	void chess_mem(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_CHESS, o2_chess_device)

#endif // MAME_BUS_ODYSSEY2_CHESS_H
