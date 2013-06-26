/*********************************************************************

    micropolis.h

    Implementations of the Micropolis
    floppy disk controller for the Sorcerer

*********************************************************************/

#ifndef __MICROPOLIS_H__
#define __MICROPOLIS_H__

#include "devcb.h"


/***************************************************************************
    MACROS
***************************************************************************/

class micropolis_device : public device_t
{
public:
	micropolis_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~micropolis_device() { global_free(m_token); }

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

extern const device_type MICROPOLIS;




/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* Interface */
struct micropolis_interface
{
	devcb_read_line in_dden_func;
	devcb_write_line out_intrq_func;
	devcb_write_line out_drq_func;
	const char *floppy_drive_tags[4];
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
void micropolis_reset(device_t *device);

void micropolis_set_drive(device_t *device, UINT8); // set current drive (0-3)

DECLARE_READ8_DEVICE_HANDLER( micropolis_status_r );
DECLARE_READ8_DEVICE_HANDLER( micropolis_data_r );

DECLARE_WRITE8_DEVICE_HANDLER( micropolis_command_w );
DECLARE_WRITE8_DEVICE_HANDLER( micropolis_data_w );

DECLARE_READ8_DEVICE_HANDLER( micropolis_r );
DECLARE_WRITE8_DEVICE_HANDLER( micropolis_w );

extern const micropolis_interface default_micropolis_interface;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MICROPOLIS_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MICROPOLIS, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* __MICROPOLIS_H__ */
