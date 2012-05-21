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

DECLARE_LEGACY_DEVICE(FD1771, fd1771);
DECLARE_LEGACY_DEVICE(FD1781, fd1781);
DECLARE_LEGACY_DEVICE(FD1791, fd1791);
DECLARE_LEGACY_DEVICE(FD1792, fd1792);
DECLARE_LEGACY_DEVICE(FD1793, fd1793);
DECLARE_LEGACY_DEVICE(FD1794, fd1794);
DECLARE_LEGACY_DEVICE(FD1795, fd1795);
DECLARE_LEGACY_DEVICE(FD1797, fd1797);
DECLARE_LEGACY_DEVICE(FD1761, fd1761);
DECLARE_LEGACY_DEVICE(FD1762, fd1762);
DECLARE_LEGACY_DEVICE(FD1763, fd1763);
DECLARE_LEGACY_DEVICE(FD1764, fd1764);
DECLARE_LEGACY_DEVICE(FD1765, fd1765);
DECLARE_LEGACY_DEVICE(FD1767, fd1767);
DECLARE_LEGACY_DEVICE(WD2791, wd2791);
DECLARE_LEGACY_DEVICE(WD2793, wd2793);
DECLARE_LEGACY_DEVICE(WD2795, wd2795);
DECLARE_LEGACY_DEVICE(WD2797, wd2797);
DECLARE_LEGACY_DEVICE(WD1770, wd1770);
DECLARE_LEGACY_DEVICE(WD1772, wd1772);
DECLARE_LEGACY_DEVICE(WD1773, wd1773);
DECLARE_LEGACY_DEVICE(MB8866, mb8866);
DECLARE_LEGACY_DEVICE(MB8876, mb8876);
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
