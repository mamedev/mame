// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
/*********************************************************************

    beta.h

    Implementation of Beta disk drive support for Spectrum and clones

    04/05/2008 Created by Miodrag Milanovic

*********************************************************************/
#ifndef MAME_SINCLAIR_BETA_M_H
#define MAME_SINCLAIR_BETA_M_H

#pragma once

#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


#define BETA_DISK_TAG   "beta"

class beta_disk_device : public device_t
{
public:
	beta_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t status_r();
	uint8_t track_r();
	uint8_t sector_r();
	uint8_t data_r();
	uint8_t state_r();

	void param_w(uint8_t data);
	void command_w(uint8_t data);
	void track_w(uint8_t data);
	void sector_w(uint8_t data);
	void data_w(uint8_t data);
	void turbo_w(int state);

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
	required_device_array<floppy_connector, 4> m_floppy;
	output_finder<4> m_floppy_led;
	void fdc_hld_w(int state);
	void motors_control();
	u8 m_control;
	bool m_motor_active;

	static void floppy_formats(format_registration &fr);
};

DECLARE_DEVICE_TYPE(BETA_DISK, beta_disk_device)


#endif // MAME_SINCLAIR_BETA_M_H
