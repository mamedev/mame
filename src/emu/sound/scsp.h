/*
    SCSP (YMF292-F) header
*/

#pragma once

#ifndef __SCSP_H__
#define __SCSP_H__

#include "devlegcy.h"

struct scsp_interface
{
	int roffset;				/* offset in the region */
	void (*irq_callback)(device_t *device, int state);	/* irq callback */
	devcb_write_line   main_irq;
};

void scsp_set_ram_base(device_t *device, void *base);

// SCSP register access
DECLARE_READ16_DEVICE_HANDLER( scsp_r );
DECLARE_WRITE16_DEVICE_HANDLER( scsp_w );

// MIDI I/O access (used for comms on Model 2/3)
DECLARE_WRITE16_DEVICE_HANDLER( scsp_midi_in );
DECLARE_READ16_DEVICE_HANDLER( scsp_midi_out_r );

extern UINT32* stv_scu;

class scsp_device : public device_t,
                                  public device_sound_interface
{
public:
	scsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~scsp_device() { global_free(m_token); }

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

extern const device_type SCSP;


#endif /* __SCSP_H__ */
