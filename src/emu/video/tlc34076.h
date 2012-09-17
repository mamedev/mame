/***************************************************************************

    tlc34076.h

    Basic implementation of the TLC34076 palette chip and similar
    compatible chips.

***************************************************************************/

#define TLC34076_6_BIT		0
#define TLC34076_8_BIT		1

const pen_t *tlc34076_get_pens(device_t *device);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct tlc34076_config
{
	int res_sel;
};

extern const tlc34076_config tlc34076_6_bit_intf;

class tlc34076_device : public device_t
{
public:
	tlc34076_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tlc34076_device() { global_free(m_token); }

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

extern const device_type TLC34076;



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TLC34076_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, TLC34076, 0) \
	MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

DECLARE_WRITE8_DEVICE_HANDLER( tlc34076_w );
DECLARE_READ8_DEVICE_HANDLER( tlc34076_r );
