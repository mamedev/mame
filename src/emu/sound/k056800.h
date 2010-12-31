/*********************************************************

    Konami 056800 MIRAC sound interface

*********************************************************/

#ifndef __K056800_H__
#define __K056800_H__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*k056800_irq_cb)(running_machine *, int);


typedef struct _k056800_interface k056800_interface;
struct _k056800_interface
{
	k056800_irq_cb       irq_cb;
};

DECLARE_LEGACY_DEVICE(K056800, k056800);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K056800_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K056800, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

READ32_DEVICE_HANDLER( k056800_host_r );
WRITE32_DEVICE_HANDLER( k056800_host_w );
READ16_DEVICE_HANDLER( k056800_sound_r );
WRITE16_DEVICE_HANDLER( k056800_sound_w );


#endif /* __K056800_H__ */


