// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
/*********************************************************************

    beta.h

    Implementation of Beta disk drive support for Spectrum and clones

    04/05/2008 Created by Miodrag Milanovic

*********************************************************************/
#ifndef MAME_MACHINE_BETA_H
#define MAME_MACHINE_BETA_H

#pragma once

#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


#define BETA_DISK_TAG   "beta"

class beta_disk_device : public device_t
{
public:
	beta_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(track_r);
	DECLARE_READ8_MEMBER(sector_r);
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_READ8_MEMBER(state_r);

	DECLARE_WRITE8_MEMBER(param_w);
	DECLARE_WRITE8_MEMBER(command_w);
	DECLARE_WRITE8_MEMBER(track_w);
	DECLARE_WRITE8_MEMBER(sector_w);
	DECLARE_WRITE8_MEMBER(data_w);

	int is_active();
	void enable();
	void disable();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint8_t m_betadisk_active;

	required_device<kr1818vg93_device> m_wd179x;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;

	DECLARE_FLOPPY_FORMATS(floppy_formats);
};

DECLARE_DEVICE_TYPE(BETA_DISK, beta_disk_device)


#endif // MAME_MACHINE_BETA_H
