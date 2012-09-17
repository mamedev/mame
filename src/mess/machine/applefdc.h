/*********************************************************************

    applefdc.h

    Implementation of various Apple Floppy Disk Controllers, including
    the classic Apple controller and the IWM (Integrated Woz Machine)
    chip

    Nate Woods
    Raphael Nabet
    R. Belmont

*********************************************************************/

#ifndef __APPLEFDC_H__
#define __APPLEFDC_H__

#include "emu.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define APPLEFDC_PH0	0x01
#define APPLEFDC_PH1	0x02
#define APPLEFDC_PH2	0x04
#define APPLEFDC_PH3	0x08

class applefdc_base_device : public device_t
{
public:
	applefdc_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~applefdc_base_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start() { }
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

class applefdc_device : public applefdc_base_device
{
public:
	applefdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type APPLEFDC;

class iwm_device : public applefdc_base_device
{
public:
	iwm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type IWM;

class swim_device : public applefdc_base_device
{
public:
	swim_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type SWIM;




/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct applefdc_interface
{
	void (*set_lines)(device_t *device, UINT8 lines);
	void (*set_enable_lines)(device_t *device, int enable_mask);

	UINT8 (*read_data)(device_t *device);
	void (*write_data)(device_t *device, UINT8 data);
	int (*read_status)(device_t *device);
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/
/* read/write handlers */
DECLARE_READ8_DEVICE_HANDLER(applefdc_r);
DECLARE_WRITE8_DEVICE_HANDLER(applefdc_w);

/* accessor */
UINT8 applefdc_get_lines(device_t *device);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_APPLEFDC_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, APPLEFDC, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_APPLEFDC_MODIFY(_tag, _intrf) \
  MCFG_DEVICE_MODIFY(_tag)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_IWM_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, IWM, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_IWM_MODIFY(_tag, _intrf) \
  MCFG_DEVICE_MODIFY(_tag)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_SWIM_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, SWIM, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_SWIM_MODIFY(_tag, _intrf) \
  MCFG_DEVICE_MODIFY(_tag)	      \
  MCFG_DEVICE_CONFIG(_intrf)


#endif /* __APPLEFDC_H__ */
