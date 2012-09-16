/*****************************************************************************
 *
 * machine/upd7002.h
 *
 * uPD7002 Analogue to Digital Converter
 *
 * Driver by Gordon Jefferyes <mess_bbc@gjeffery.dircon.co.uk>
 *
 ****************************************************************************/

#ifndef UPD7002_H_
#define UPD7002_H_

/***************************************************************************
    MACROS
***************************************************************************/

class uPD7002_device : public device_t
{
public:
	uPD7002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~uPD7002_device() { global_free(m_token); }

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

extern const device_type UPD7002;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*uPD7002_get_analogue_func)(device_t *device, int channel_number);
#define UPD7002_GET_ANALOGUE(name)	int name(device_t *device, int channel_number )

typedef void (*uPD7002_eoc_func)(device_t *device, int data);
#define UPD7002_EOC(name)	void name(device_t *device, int data )


struct uPD7002_interface
{
	uPD7002_get_analogue_func get_analogue_func;
	uPD7002_eoc_func		  EOC_func;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* Standard handlers */

READ8_DEVICE_HANDLER ( uPD7002_EOC_r );
READ8_DEVICE_HANDLER ( uPD7002_r );
WRITE8_DEVICE_HANDLER ( uPD7002_w );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_UPD7002_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, UPD7002, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#endif /* UPD7002_H_ */
