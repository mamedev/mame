/*****************************************************************************
 *
 * video/dl1416.h
 *
 * DL1416
 *
 * 4-Digit 16-Segment Alphanumeric Intelligent Display
 * with Memory/Decoder/Driver
 *
 * See video/dl1416.c for more info
 *
 ****************************************************************************/

#ifndef DL1416_H_
#define DL1416_H_

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*dl1416_update_func)(device_t *device, int digit, int data);

struct dl1416_interface
{
	dl1416_update_func update;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DL1416B_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, DL1416B, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_DL1416T_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, DL1416T, 0) \
	MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* inputs */
WRITE_LINE_DEVICE_HANDLER( dl1416_wr_w ); /* write enable */
WRITE_LINE_DEVICE_HANDLER( dl1416_ce_w ); /* chip enable */
WRITE_LINE_DEVICE_HANDLER( dl1416_cu_w ); /* cursor enable */
WRITE8_DEVICE_HANDLER( dl1416_data_w );

/* device get info callback */
class dl1416_device : public device_t
{
public:
	dl1416_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~dl1416_device() { global_free(m_token); }

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

class dl1416b_device : public dl1416_device
{
public:
	dl1416b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type DL1416B;

class dl1416t_device : public dl1416_device
{
public:
	dl1416t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type DL1416T;


#endif /* DL1416_H_ */
