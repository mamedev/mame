/***************************************************************************

    NEC uPD4701

    Incremental Encoder Control

***************************************************************************/

#ifndef __UPD4701_H__
#define __UPD4701_H__

#include "devlegcy.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class upd4701_device : public device_t
{
public:
	upd4701_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~upd4701_device() { global_free(m_token); }

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

extern const device_type UPD4701;


#define MCFG_UPD4701_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, UPD4701, 0)


/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern DECLARE_WRITE8_DEVICE_HANDLER( upd4701_cs_w );
extern DECLARE_WRITE8_DEVICE_HANDLER( upd4701_xy_w );
extern DECLARE_WRITE8_DEVICE_HANDLER( upd4701_ul_w );
extern DECLARE_WRITE8_DEVICE_HANDLER( upd4701_resetx_w );
extern DECLARE_WRITE8_DEVICE_HANDLER( upd4701_resety_w );
extern DECLARE_WRITE16_DEVICE_HANDLER( upd4701_x_add );
extern DECLARE_WRITE16_DEVICE_HANDLER( upd4701_y_add );
extern DECLARE_WRITE8_DEVICE_HANDLER( upd4701_switches_set );

extern DECLARE_READ16_DEVICE_HANDLER( upd4701_d_r );
extern DECLARE_READ8_DEVICE_HANDLER( upd4701_cf_r );
extern DECLARE_READ8_DEVICE_HANDLER( upd4701_sf_r );


#endif  /* __UPD4701_H__ */
