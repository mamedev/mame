#ifndef _DIGITALKER_H_
#define _DIGITALKER_H_

#include "devlegcy.h"

void digitalker_0_cs_w(device_t *device, int line);
void digitalker_0_cms_w(device_t *device, int line);
void digitalker_0_wr_w(device_t *device, int line);
int digitalker_0_intr_r(device_t *device);
DECLARE_WRITE8_DEVICE_HANDLER(digitalker_data_w);

class digitalker_device : public device_t,
                                  public device_sound_interface
{
public:
	digitalker_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~digitalker_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type DIGITALKER;


#endif
