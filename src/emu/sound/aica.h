/*

    Sega/Yamaha AICA emulation
*/

#ifndef __AICA_H__
#define __AICA_H__


struct aica_interface
{
	int master;
	int roffset;                /* offset in the region */
	devcb_write_line irq_callback; /* irq callback */
	devcb_write_line master_irq_callback;
};

void aica_set_ram_base(device_t *device, void *base, int size);

// AICA register access
DECLARE_READ16_DEVICE_HANDLER( aica_r );
DECLARE_WRITE16_DEVICE_HANDLER( aica_w );

// MIDI I/O access
DECLARE_WRITE16_DEVICE_HANDLER( aica_midi_in );
DECLARE_READ16_DEVICE_HANDLER( aica_midi_out_r );

class aica_device : public device_t,
									public device_sound_interface
{
public:
	aica_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~aica_device();

	// access to legacy token
	struct aica_state *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	struct aica_state *m_token;
};

extern const device_type AICA;


#endif /* __AICA_H__ */
