/*********************************************************************

    beta.h

    Implementation of Beta disk drive support for Spectrum and clones

    04/05/2008 Created by Miodrag Milanovic

*********************************************************************/
#ifndef __BETA_H__
#define __BETA_H__


int betadisk_is_active(device_t *device);
void betadisk_enable(device_t *device);
void betadisk_disable(device_t *device);
void betadisk_clear_status(device_t *device);

#define BETA_DISK_TAG   "beta"

class beta_disk_device : public device_t
{
public:
	beta_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~beta_disk_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
	// internal state
	void *m_token;
};

extern const device_type BETA_DISK;


#define MCFG_BETA_DISK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BETA_DISK, 0)

#define MCFG_BETA_DISK_REMOVE(_tag)     \
	MCFG_DEVICE_REMOVE(_tag)

DECLARE_READ8_DEVICE_HANDLER(betadisk_status_r);
DECLARE_READ8_DEVICE_HANDLER(betadisk_track_r);
DECLARE_READ8_DEVICE_HANDLER(betadisk_sector_r);
DECLARE_READ8_DEVICE_HANDLER(betadisk_data_r);
DECLARE_READ8_DEVICE_HANDLER(betadisk_state_r);

DECLARE_WRITE8_DEVICE_HANDLER(betadisk_param_w);
DECLARE_WRITE8_DEVICE_HANDLER(betadisk_command_w);
DECLARE_WRITE8_DEVICE_HANDLER(betadisk_track_w);
DECLARE_WRITE8_DEVICE_HANDLER(betadisk_sector_w);
DECLARE_WRITE8_DEVICE_HANDLER(betadisk_data_w);
#endif /* __BETA_H__ */
