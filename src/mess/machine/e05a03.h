/***************************************************************************

    E05A03 Gate Array (used in the Epson LX-800)

***************************************************************************/

#ifndef __E05A03_H__
#define __E05A03_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct e05a03_interface
{
	devcb_read8 in_data_func;

	devcb_write_line out_nlq_lp_func;
	devcb_write_line out_pe_lp_func;
	devcb_write_line out_pe_func;
	devcb_write_line out_reso_func;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
DECLARE_WRITE8_DEVICE_HANDLER( e05a03_w );
DECLARE_READ8_DEVICE_HANDLER( e05a03_r );

WRITE_LINE_DEVICE_HANDLER( e05a03_home_w ); /* home position signal */
WRITE_LINE_DEVICE_HANDLER( e05a03_fire_w ); /* printhead solenoids trigger */
WRITE_LINE_DEVICE_HANDLER( e05a03_strobe_w );
READ_LINE_DEVICE_HANDLER( e05a03_busy_r );
WRITE_LINE_DEVICE_HANDLER( e05a03_resi_w ); /* reset input */
WRITE_LINE_DEVICE_HANDLER( e05a03_init_w ); /* centronics init */


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class e05a03_device : public device_t
{
public:
	e05a03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~e05a03_device() { global_free(m_token); }

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

extern const device_type E05A03;


#define MCFG_E05A03_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, E05A03, 0) \
	MCFG_DEVICE_CONFIG(_intf)


#endif /* __E05A03_H__ */
