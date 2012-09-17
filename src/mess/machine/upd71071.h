#ifndef UPD71071_H_
#define UPD71071_H_

#include "emu.h"

struct upd71071_intf
{
	const char* cputag;
	int clock;
	UINT16 (*dma_read[4])(running_machine &machine);
	void (*dma_write[4])(running_machine &machine, UINT16 data);
};

int upd71071_dmarq(device_t* device,int state,int channel);

class upd71071_device : public device_t
{
public:
	upd71071_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~upd71071_device() { global_free(m_token); }

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

extern const device_type UPD71071;


#define MCFG_UPD71071_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, UPD71071, 0) \
	MCFG_DEVICE_CONFIG(_config)

DECLARE_READ8_DEVICE_HANDLER(upd71071_r);
DECLARE_WRITE8_DEVICE_HANDLER(upd71071_w);

#endif /*UPD71071_H_*/
