// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_MZR8300_H
#define MAME_BUS_VME_MZR8300_H

#pragma once

#include "bus/vme/vme.h"
#include "machine/am9513.h"
#include "machine/z80sio.h"

DECLARE_DEVICE_TYPE(VME_MZR8300, vme_mzr8300_card_device)

class vme_mzr8300_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_mzr8300_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

//  virtual uint8_t read8(offs_t offset) override;
//  virtual void write8(offs_t offset, uint8_t data) override;

protected:
	vme_mzr8300_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<upd7201_device, 2> m_sio;
	required_device<am9513_device> m_stc;
};

#endif // MAME_BUS_VME_MZR8300_H
