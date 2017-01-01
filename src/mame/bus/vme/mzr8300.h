// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef VME_MZR8300_H
#define VME_MZR8300_H
#pragma once

#include "bus/vme/vme.h"
//#include "includes/mzr8300.h"
#include "bus/vme/mzr8300.h"

extern const device_type VME_MZR8300;

class vme_mzr8300_card_device : 
	public device_t
	,public device_vme_p1_card_interface
{
public:
	vme_mzr8300_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_mzr8300_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	// optional information overrides

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_READ8_MEMBER (read8) override;
	virtual DECLARE_WRITE8_MEMBER (write8) override;
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
private:
};

#endif // VME_MZR8300_H
