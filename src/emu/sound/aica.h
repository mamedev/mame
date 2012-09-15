/*

    Sega/Yamaha AICA emulation
*/

#ifndef __AICA_H__
#define __AICA_H__

#include "devlegcy.h"

struct aica_interface
{
	int master;
	int roffset;				/* offset in the region */
	void (*irq_callback)(device_t *device, int state);	/* irq callback */
};

void aica_set_ram_base(device_t *device, void *base, int size);

// AICA register access
READ16_DEVICE_HANDLER( aica_r );
WRITE16_DEVICE_HANDLER( aica_w );

// MIDI I/O access
WRITE16_DEVICE_HANDLER( aica_midi_in );
READ16_DEVICE_HANDLER( aica_midi_out_r );

class aica_device : public device_t,
                                  public device_sound_interface
{
public:
	aica_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~aica_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type AICA;


#endif /* __AICA_H__ */
