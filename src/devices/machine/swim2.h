// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Apple SWIM2 floppy disk controller

*********************************************************************/

#pragma once

#ifndef MAME_MACHINE_SWIM2_H
#define MAME_MACHINE_SWIM2_H

#include "applefdintf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class swim2_device : public applefdintf_device
{
public:
	// construction/destruction
	swim2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

	virtual void set_floppy(floppy_image_device *floppy) override;
	virtual floppy_image_device *get_floppy() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	floppy_image_device *m_floppy;
	u8 m_param[4];
	u8 m_mode, m_setup, m_param_idx;

	void show_mode() const;
};

DECLARE_DEVICE_TYPE(SWIM2, swim2_device)

#endif  /* MAME_MACHINE_SWIM2_H */
