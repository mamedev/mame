// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    swim.h

    Implementation of the Apple SWIM FDC controller; used on (less)
    early Macs

*********************************************************************/
#ifndef MAME_MACHINE_SWIM_H
#define MAME_MACHINE_SWIM_H

#pragma once

#include "machine/applefdc.h"


/***************************************************************************
    DEVICE
***************************************************************************/

DECLARE_DEVICE_TYPE(LEGACY_SWIM, swim_device)

class swim_device : public applefdc_base_device
{
public:
	swim_device(const machine_config &mconfig, const char *tag, device_t *owner, const applefdc_interface *intrf)
		: swim_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_config(intrf);
	}

	swim_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// other overrides
	virtual void iwm_modereg_w(uint8_t data) override;

private:
	uint8_t       m_swim_mode = 0;
	uint8_t       m_swim_magic_state = 0;
	uint8_t       m_parm_offset = 0;
	uint8_t       m_ism_regs[8]{};
	uint8_t       m_parms[16]{};
};

#endif // MAME_MACHINE_SWIM_H
