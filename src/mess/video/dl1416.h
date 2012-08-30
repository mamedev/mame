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

typedef struct _dl1416_interface dl1416_interface;
struct _dl1416_interface
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
DECLARE_LEGACY_DEVICE(DL1416B, dl1416b);
DECLARE_LEGACY_DEVICE(DL1416T, dl1416t);

#endif /* DL1416_H_ */
