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
    CONSTANTS
***************************************************************************/

enum
{
	DL1416B,
	DL1416T,
	MAX_DL1416_TYPES
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*dl1416_update_func)(device_t *device, int digit, int data);

typedef struct _dl1416_interface dl1416_interface;
struct _dl1416_interface
{
	int type;
	dl1416_update_func update;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DL1416_ADD(_tag, _type, _update) \
	MCFG_DEVICE_ADD(_tag, DL1416, 0) \
	MCFG_DEVICE_CONFIG_DATA32(dl1416_interface, type, _type) \
	MCFG_DEVICE_CONFIG_DATAPTR(dl1416_interface, update, _update)

#define MCFG_DL1416B_ADD(_tag, _update) \
	MCFG_DL1416_ADD(_tag, DL1416B, _update)

#define MCFG_DL1416T_ADD(_tag, _update) \
	MCFG_DL1416_ADD(_tag, DL1416T, _update)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* inputs */
WRITE_LINE_DEVICE_HANDLER( dl1416_wr_w ); /* write enable */
WRITE_LINE_DEVICE_HANDLER( dl1416_ce_w ); /* chip enable */
WRITE_LINE_DEVICE_HANDLER( dl1416_cu_w ); /* cursor enable */
WRITE8_DEVICE_HANDLER( dl1416_data_w );

/* device get info callback */
DECLARE_LEGACY_DEVICE(DL1416, dl1416);

#endif /* DL1416_H_ */
