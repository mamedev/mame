/*****************************************************************************
 *
 * machine/i8271.h
 *
 ****************************************************************************/

#ifndef I8271_H_
#define I8271_H_

/***************************************************************************
    MACROS
***************************************************************************/

class i8271_device : public device_t
{
public:
	i8271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~i8271_device() { global_free(m_token); }

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

extern const device_type I8271;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct i8271_interface 
{
	void (*interrupt)(device_t *device, int state);
	void (*dma_request)(device_t *device, int state, int read_);
	const char *floppy_drive_tags[2];
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
READ8_DEVICE_HANDLER (i8271_r);
WRITE8_DEVICE_HANDLER(i8271_w);

READ8_DEVICE_HANDLER (i8271_dack_r);
WRITE8_DEVICE_HANDLER(i8271_dack_w);

READ8_DEVICE_HANDLER (i8271_data_r);
WRITE8_DEVICE_HANDLER(i8271_data_w);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8271_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, I8271, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* I8271_H_ */
