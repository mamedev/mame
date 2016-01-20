// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
/*********************************************************************

    beta.h

    Implementation of Beta disk drive support for Spectrum and clones

    04/05/2008 Created by Miodrag Milanovic

*********************************************************************/
#ifndef __BETA_H__
#define __BETA_H__

#include "machine/wd_fdc.h"


#define BETA_DISK_TAG   "beta"

class beta_disk_device : public device_t
{
public:
	beta_disk_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~beta_disk_device() {}

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
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	int is_active();
	void enable();
	void disable();

	UINT8 m_betadisk_active;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<kr1818vg93_t> m_wd179x;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
};

extern const device_type BETA_DISK;


#define MCFG_BETA_DISK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BETA_DISK, 0)

#define MCFG_BETA_DISK_REMOVE(_tag)     \
	MCFG_DEVICE_REMOVE(_tag)

#endif /* __BETA_H__ */
