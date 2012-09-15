#ifndef MM58274C_H
#define MM58274C_H

/***************************************************************************
    MACROS
***************************************************************************/

class mm58274c_device : public device_t
{
public:
	mm58274c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mm58274c_device() { global_free(m_token); }

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

extern const device_type MM58274C;


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
/* interface */
/*
    Initializes the clock chip.
    day1 must be set to a value from 0 (sunday), 1 (monday) ...
    to 6 (saturday) and is needed to correctly retrieve the day-of-week
    from the host system clock.
*/
struct mm58274c_interface
{
	int	mode24;		/* 24/12 mode */
	int	day1;		/* first day of week */
};

READ8_DEVICE_HANDLER ( mm58274c_r );
WRITE8_DEVICE_HANDLER( mm58274c_w );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MM58274C_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MM58274C, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* MM58274C_H */
