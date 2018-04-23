// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_VME_MVME350_H
#define MAME_BUS_VME_VME_MVME350_H

#pragma once

#include "bus/vme/vme.h"

DECLARE_DEVICE_TYPE(VME_MVME350, vme_mvme350_card_device)

class vme_mvme350_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_mvme350_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Shared memory methods to be exported to the VME bus
//  virtual DECLARE_READ16_MEMBER (read16) override;
//  virtual DECLARE_WRITE16_MEMBER (write16) override;

protected:
	vme_mvme350_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void mvme350_mem(address_map &map);
};

#endif // MAME_BUS_VME_VME_MVME350_H
