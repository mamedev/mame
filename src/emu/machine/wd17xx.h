/*********************************************************************

    wd17xx.h

    Implementations of the Western Digital 17xx and 27xx families of
    floppy disk controllers

*********************************************************************/

#ifndef __WD17XX_H__
#define __WD17XX_H__

#include "devcb.h"


/***************************************************************************
    MACROS
***************************************************************************/

class wd1770_device : public device_t
{
public:
	wd1770_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	wd1770_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~wd1770_device() { global_free(m_token); }

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

extern const device_type WD1770;

class fd1771_device : public wd1770_device
{
public:
	fd1771_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1771;

class fd1781_device : public wd1770_device
{
public:
	fd1781_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1781;

class fd1791_device : public wd1770_device
{
public:
	fd1791_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1791;

class fd1792_device : public wd1770_device
{
public:
	fd1792_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1792;

class fd1793_device : public wd1770_device
{
public:
	fd1793_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1793;

class fd1794_device : public wd1770_device
{
public:
	fd1794_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1794;

class fd1795_device : public wd1770_device
{
public:
	fd1795_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1795;

class fd1797_device : public wd1770_device
{
public:
	fd1797_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1797;

class fd1761_device : public wd1770_device
{
public:
	fd1761_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1761;

class fd1762_device : public wd1770_device
{
public:
	fd1762_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1762;

class fd1763_device : public wd1770_device
{
public:
	fd1763_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1763;

class fd1764_device : public wd1770_device
{
public:
	fd1764_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1764;

class fd1765_device : public wd1770_device
{
public:
	fd1765_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1765;

class fd1767_device : public wd1770_device
{
public:
	fd1767_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1767;

class wd2791_device : public wd1770_device
{
public:
	wd2791_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type WD2791;

class wd2793_device : public wd1770_device
{
public:
	wd2793_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type WD2793;

class wd2795_device : public wd1770_device
{
public:
	wd2795_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type WD2795;

class wd2797_device : public wd1770_device
{
public:
	wd2797_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type WD2797;

class wd1772_device : public wd1770_device
{
public:
	wd1772_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type WD1772;

class wd1773_device : public wd1770_device
{
public:
	wd1773_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type WD1773;

class mb8866_device : public wd1770_device
{
public:
	mb8866_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type MB8866;

class mb8876_device : public wd1770_device
{
public:
	mb8876_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type MB8876;

class mb8877_device : public wd1770_device
{
public:
	mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type MB8877;




/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* Interface */
typedef struct _wd17xx_interface wd17xx_interface;
struct _wd17xx_interface
{
	devcb_read_line in_dden_func;
	devcb_write_line out_intrq_func;
	devcb_write_line out_drq_func;
	const char *floppy_drive_tags[4];
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
void wd17xx_reset(device_t *device);

/* the following are not strictly part of the wd179x hardware/emulation
but will be put here for now until the flopdrv code has been finalised more */
void wd17xx_set_drive(device_t *device, UINT8);		/* set drive wd179x is accessing */
void wd17xx_set_side(device_t *device, UINT8);		/* set side wd179x is accessing */

void wd17xx_set_pause_time(device_t *device, int usec);       /* default is 40 usec if not set */
void wd17xx_index_pulse_callback(device_t *controller, device_t *img, int state);

READ8_DEVICE_HANDLER( wd17xx_status_r );
READ8_DEVICE_HANDLER( wd17xx_track_r );
READ8_DEVICE_HANDLER( wd17xx_sector_r );
READ8_DEVICE_HANDLER( wd17xx_data_r );

WRITE8_DEVICE_HANDLER( wd17xx_command_w );
WRITE8_DEVICE_HANDLER( wd17xx_track_w );
WRITE8_DEVICE_HANDLER( wd17xx_sector_w );
WRITE8_DEVICE_HANDLER( wd17xx_data_w );

READ8_DEVICE_HANDLER( wd17xx_r );
WRITE8_DEVICE_HANDLER( wd17xx_w );

WRITE_LINE_DEVICE_HANDLER( wd17xx_mr_w );
WRITE_LINE_DEVICE_HANDLER( wd17xx_rdy_w );
READ_LINE_DEVICE_HANDLER( wd17xx_mo_r );
WRITE_LINE_DEVICE_HANDLER( wd17xx_tr00_w );
WRITE_LINE_DEVICE_HANDLER( wd17xx_idx_w );
WRITE_LINE_DEVICE_HANDLER( wd17xx_wprt_w );
WRITE_LINE_DEVICE_HANDLER( wd17xx_dden_w );
READ_LINE_DEVICE_HANDLER( wd17xx_drq_r );
READ_LINE_DEVICE_HANDLER( wd17xx_intrq_r );

extern const wd17xx_interface default_wd17xx_interface;
extern const wd17xx_interface default_wd17xx_interface_2_drives;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_FD1771_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1771, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1781_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1781, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1791_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1791, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1792_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1792, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1793_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1793, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1794_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1794, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1795_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1795, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1797_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1797, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1761_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1761, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1762_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1762, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1763_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1763, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1764_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1764, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1765_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1765, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1767_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1767, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD2793_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD2793, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD2795_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD2795, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD2797_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD2797, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1770_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1770, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1772_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1772, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1773_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1773, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MB8866_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MB8866, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MB8876_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MB8876, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MB8877_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MB8877, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* __WD17XX_H__ */
