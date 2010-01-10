/*********************************************************

    Konami 056800 MIRAC sound interface

*********************************************************/

#ifndef __K056800_H__
#define __K056800_H__



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*k056800_irq_cb)(running_machine *, int);


typedef struct _k056800_interface k056800_interface;
struct _k056800_interface
{
	k056800_irq_cb       irq_cb;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( k056800 );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define K056800 DEVICE_GET_INFO_NAME( k056800 )

#define MDRV_K056800_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K056800, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

READ32_DEVICE_HANDLER( k056800_host_r );
WRITE32_DEVICE_HANDLER( k056800_host_w );
READ16_DEVICE_HANDLER( k056800_sound_r );
WRITE16_DEVICE_HANDLER( k056800_sound_w );


#endif /* __K056800_H__ */


