// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_VME_MZR8300_H
#define MAME_BUS_VME_VME_MZR8300_H

#pragma once

#include "bus/vme/vme.h"

extern const device_type VME_MZR8300;

class vme_mzr8300_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_mzr8300_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

//  virtual DECLARE_READ8_MEMBER (read8) override;
//  virtual DECLARE_WRITE8_MEMBER (write8) override;

protected:
	vme_mzr8300_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif // MAME_BUS_VME_VME_MZR8300_H
