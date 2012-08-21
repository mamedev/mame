/*****************************************************************************
 *
 * machine/upd7002.h
 *
 * uPD7002 Analogue to Digital Converter
 *
 * Driver by Gordon Jefferyes <mess_bbc@gjeffery.dircon.co.uk>
 *
 ****************************************************************************/

#ifndef UPD7002_H_
#define UPD7002_H_

/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(UPD7002, uPD7002);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*uPD7002_get_analogue_func)(device_t *device, int channel_number);
#define UPD7002_GET_ANALOGUE(name)	int name(device_t *device, int channel_number )

typedef void (*uPD7002_eoc_func)(device_t *device, int data);
#define UPD7002_EOC(name)	void name(device_t *device, int data )


typedef struct _uPD7002_interface uPD7002_interface;
struct _uPD7002_interface
{
	uPD7002_get_analogue_func get_analogue_func;
	uPD7002_eoc_func		  EOC_func;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* Standard handlers */

READ8_DEVICE_HANDLER ( uPD7002_EOC_r );
READ8_DEVICE_HANDLER ( uPD7002_r );
WRITE8_DEVICE_HANDLER ( uPD7002_w );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_UPD7002_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, UPD7002, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#endif /* UPD7002_H_ */
