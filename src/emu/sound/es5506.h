/**********************************************************************************************
 *
 *   Ensoniq ES5505/6 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __ES5506_H__
#define __ES5506_H__

#include "devlegcy.h"

struct es5505_interface
{
	const char * region0;						/* memory region where the sample ROM lives */
	const char * region1;						/* memory region where the sample ROM lives */
	void (*irq_callback)(device_t *device, int state);	/* irq callback */
	UINT16 (*read_port)(device_t *device);			/* input port read */
};

DECLARE_READ16_DEVICE_HANDLER( es5505_r );
DECLARE_WRITE16_DEVICE_HANDLER( es5505_w );
void es5505_voice_bank_w(device_t *device, int voice, int bank);

class es5506_device : public device_t,
                                  public device_sound_interface
{
public:
	es5506_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	es5506_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~es5506_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type ES5506;



struct es5506_interface
{
	const char * region0;						/* memory region where the sample ROM lives */
	const char * region1;						/* memory region where the sample ROM lives */
	const char * region2;						/* memory region where the sample ROM lives */
	const char * region3;						/* memory region where the sample ROM lives */
	void (*irq_callback)(device_t *device, int state);	/* irq callback */
	UINT16 (*read_port)(device_t *device);			/* input port read */
};

DECLARE_READ8_DEVICE_HANDLER( es5506_r );
DECLARE_WRITE8_DEVICE_HANDLER( es5506_w );
void es5506_voice_bank_w(device_t *device, int voice, int bank);

class es5505_device : public es5506_device
{
public:
	es5505_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
private:
	// internal state
};

extern const device_type ES5505;


#endif /* __ES5506_H__ */
