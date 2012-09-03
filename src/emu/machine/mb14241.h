/*****************************************************************************

    MB14241 shifter IC emulation

 *****************************************************************************/

#ifndef __MB14241_H__
#define __MB14241_H__

#include "devlegcy.h"


class mb14241_device : public device_t
{
public:
	mb14241_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mb14241_device() { global_free(m_token); }

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

extern const device_type MB14241;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MB14241_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MB14241, 0)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE8_DEVICE_HANDLER ( mb14241_shift_count_w );
WRITE8_DEVICE_HANDLER ( mb14241_shift_data_w );
READ8_DEVICE_HANDLER( mb14241_shift_result_r );


#endif /* __MB14241_H__ */
