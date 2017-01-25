// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef VME_MVME350_H
#define VME_MVME350_H
#pragma once

#include "bus/vme/vme.h"

extern const device_type VME_MVME350;

class vme_mvme350_card_device :
	public device_t
	,public device_vme_card_interface
{
public:
	vme_mvme350_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_mvme350_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// Shared memory methods to be exported to the VME bus
//  virtual DECLARE_READ16_MEMBER (read16) override;
//  virtual DECLARE_WRITE16_MEMBER (write16) override;
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
private:
};

#endif // VME_MVME350_H
