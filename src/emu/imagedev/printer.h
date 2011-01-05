/****************************************************************************

    printer.h

    Code for handling printer devices

****************************************************************************/

#ifndef __PRINTER_H__
#define __PRINTER_H__

#include "image.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*online_func)(device_t *device, int state);

typedef struct _printer_config printer_config;
struct _printer_config
{
	online_func online;
};


DECLARE_LEGACY_IMAGE_DEVICE(PRINTER, printer);


#define MCFG_PRINTER_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PRINTER, 0) \

#define MCFG_PRINTER_ONLINE(_online) \
	MCFG_DEVICE_CONFIG_DATAPTR(printer_config, online, _online)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* checks to see if a printer is ready */
int printer_is_ready(device_t *printer);

/* outputs data to a printer */
void printer_output(device_t *printer, UINT8 data);

#endif /* __PRINTER_H__ */
