// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef VME_MZR8105_H
#define VME_MZR8105_H
#pragma once

#include "bus/vme/vme.h"
//#include "includes/mzr8105.h"
#include "bus/vme/mzr8105.h"

extern const device_type VME_MZR8105;

class vme_mzr8105_card_device : public device_t
						 ,public device_vme_p1_card_interface
{
public:
	vme_mzr8105_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_mzr8105_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
private:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif // VME_MZR8105_H
