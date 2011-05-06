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

DECLARE_LEGACY_DEVICE(WD1770, wd1770);
DECLARE_LEGACY_DEVICE(WD1771, wd1771);
DECLARE_LEGACY_DEVICE(WD1772, wd1772);
DECLARE_LEGACY_DEVICE(WD1773, wd1773);
DECLARE_LEGACY_DEVICE(WD1793, wd1793);
DECLARE_LEGACY_DEVICE(WD1795, wd1795);
DECLARE_LEGACY_DEVICE(WD1797, wd1797);
DECLARE_LEGACY_DEVICE(WD2793, wd2793);
DECLARE_LEGACY_DEVICE(WD2795, wd2795);
DECLARE_LEGACY_DEVICE(WD2797, wd2797);
DECLARE_LEGACY_DEVICE(WD177X, wd177x);
DECLARE_LEGACY_DEVICE(WD179X, wd179x);
DECLARE_LEGACY_DEVICE(MB8877, mb8877);


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

#define MCFG_WD1770_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1770, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1771_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1771, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1772_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1772, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1773_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1773, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1793_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1793, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1795_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1795, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1797_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1797, 0) \
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

#define MCFG_WD177X_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD177X, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD179X_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD179X, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MB8877_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MB8877, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#endif /* __WD17XX_H__ */
