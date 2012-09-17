/*****************************************************************************
 *
 * audio/snes_spc.h
 *
 ****************************************************************************/

#ifndef __SNES_SPC_H__
#define __SNES_SPC_H__

#include "devcb.h"

#define SNES_SPCRAM_SIZE      0x10000


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class snes_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	snes_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~snes_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type SNES;



/***************************************************************************
    I/O PROTOTYPES
***************************************************************************/

DECLARE_READ8_DEVICE_HANDLER( spc_io_r );
DECLARE_WRITE8_DEVICE_HANDLER( spc_io_w );
DECLARE_READ8_DEVICE_HANDLER( spc_ram_r );
DECLARE_WRITE8_DEVICE_HANDLER( spc_ram_w );
DECLARE_READ8_DEVICE_HANDLER( spc_port_out );
DECLARE_WRITE8_DEVICE_HANDLER( spc_port_in );

UINT8 *spc_get_ram(device_t *device);
void spc700_set_volume(device_t *device,int volume);
void spc700_reset(device_t *device);

#endif /* __SNES_SPC_H__ */
