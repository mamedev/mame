/*****************************************************************************
 *
 * machine/ins8154.h
 *
 * INS8154 N-Channel 128-by-8 Bit RAM Input/Output (RAM I/O)
 *
 ****************************************************************************/

#ifndef INS8154_H_
#define INS8154_H_

/***************************************************************************
    MACROS
***************************************************************************/

#define INS8154  DEVICE_GET_INFO_NAME(ins8154)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*ins8154_irq_func)(running_device *device, int state);
#define INS8154_IRQ(name)	void name(running_device *device, int state )

/******************* Interface **********************************************/

typedef struct _ins8154_interface ins8154_interface;

struct _ins8154_interface
{
	read8_device_func in_a_func;
	read8_device_func in_b_func;
	write8_device_func out_a_func;
	write8_device_func out_b_func;
	ins8154_irq_func irq_func;
};

DEVICE_GET_INFO( ins8154 );


/******************* Standard 8-bit CPU interfaces, D0-D7 *******************/

READ8_DEVICE_HANDLER( ins8154_r );
WRITE8_DEVICE_HANDLER( ins8154_w );


/******************* 8-bit A/B port interfaces ******************************/

WRITE8_DEVICE_HANDLER( ins8154_porta_w );
WRITE8_DEVICE_HANDLER( ins8154_portb_w );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_INS8154_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, INS8154, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#endif /* INS8154_H_ */
