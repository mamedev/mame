/**********************************************************************

    NEC uPD1771

**********************************************************************/

#ifndef __UPD1771_H__
#define __UPD1771_H__

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct upd1771_interface
{
	devcb_write_line	ack_callback;
};


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class upd1771c_device : public device_t,
                                  public device_sound_interface
{
public:
	upd1771c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~upd1771c_device() { global_free(m_token); }

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

extern const device_type UPD1771C;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

DECLARE_READ8_DEVICE_HANDLER( upd1771_r );
DECLARE_WRITE8_DEVICE_HANDLER( upd1771_w );
WRITE_LINE_DEVICE_HANDLER( upd1771_pcm_w );

#endif /* __UPD1771_H__ */
