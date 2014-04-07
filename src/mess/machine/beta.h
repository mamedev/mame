/*********************************************************************

    beta.h

    Implementation of Beta disk drive support for Spectrum and clones

    04/05/2008 Created by Miodrag Milanovic

*********************************************************************/
#ifndef __BETA_H__
#define __BETA_H__

#include "machine/wd17xx.h"


#define BETA_DISK_TAG   "beta"

class beta_disk_device : public device_t
{
public:
	beta_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
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

	DECLARE_WRITE_LINE_MEMBER(wd179x_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(wd179x_drq_w);

	int is_active();
	void enable();
	void disable();
	void clear_status();

	UINT8 m_betadisk_status;
	UINT8 m_betadisk_active;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	// internal state
	wd2793_device *m_wd179x;
};

extern const device_type BETA_DISK;


#define MCFG_BETA_DISK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BETA_DISK, 0)

#define MCFG_BETA_DISK_REMOVE(_tag)     \
	MCFG_DEVICE_REMOVE(_tag)

#endif /* __BETA_H__ */
