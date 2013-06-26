#ifndef __DECOCASS_TAPE_H__
#define __DECOCASS_TAPE_H__

class decocass_tape_device : public device_t
{
public:
	decocass_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~decocass_tape_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type DECOCASS_TAPE;

UINT8 tape_get_status_bits(device_t *device);
UINT8 tape_is_present(device_t *device);
void tape_change_speed(device_t *device, INT8 newspeed);


#define MCFG_DECOCASS_TAPE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECOCASS_TAPE, 0)

#endif
