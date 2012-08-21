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

DECLARE_LEGACY_DEVICE(APPLEFDC, applefdc);
DECLARE_LEGACY_DEVICE(IWM, iwm);
DECLARE_LEGACY_DEVICE(SWIM, swim);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _applefdc_interface applefdc_interface;
struct _applefdc_interface
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
READ8_DEVICE_HANDLER(applefdc_r);
WRITE8_DEVICE_HANDLER(applefdc_w);

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
