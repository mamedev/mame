/*
    strata.h: header file for strata.c
*/

class strataflash_device : public device_t
{
public:
	strataflash_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~strataflash_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;
};

extern const device_type STRATAFLASH;


#define MCFG_STRATAFLASH_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, STRATAFLASH, 0)

int strataflash_load(device_t *device, emu_file *file);
int strataflash_save(device_t *device, emu_file *file);
UINT8 strataflash_8_r(device_t *device, UINT32 address);
void strataflash_8_w(device_t *device, UINT32 address, UINT8 data);
UINT16 strataflash_16_r(device_t *device, offs_t offset);
void strataflash_16_w(device_t *device, offs_t offset, UINT16 data);
