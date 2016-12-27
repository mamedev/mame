// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/********************************************************************************
 *
 * mame/includes/mzr8105
 *
 ********************************************************************************/

#ifndef MZR8300_H
#define MZR8300_H
#pragma once

#include "emu.h"
#include "bus/vme/vme.h"

class vme_p1_mzr8300_device :
		public device_t,
		public device_vme_p1_card_interface
{
public:
		// construction/destruction
		vme_p1_mzr8300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
		vme_p1_mzr8300_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;

		DECLARE_READ8_MEMBER(mzr8300_r);
		DECLARE_WRITE8_MEMBER(mzr8300_w);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
};

#endif // MZR8300_H
