/**********************************************************************

    8250 UART interface and emulation

**********************************************************************/

#ifndef __INS8250_H_
#define __INS8250_H_

#include "devlegcy.h"

DECLARE_LEGACY_DEVICE(INS8250, ins8250);
DECLARE_LEGACY_DEVICE(INS8250A, ins8250a);
DECLARE_LEGACY_DEVICE(NS16450, ns16450);
DECLARE_LEGACY_DEVICE(NS16550, ns16550);
DECLARE_LEGACY_DEVICE(NS16550A, ns16550a);
DECLARE_LEGACY_DEVICE(PC16550D, pc16550d);

#define UART8250_HANDSHAKE_OUT_DTR				0x01
#define UART8250_HANDSHAKE_OUT_RTS				0x02

#define UART8250_HANDSHAKE_IN_DSR				0x020
#define UART8250_HANDSHAKE_IN_CTS				0x010
#define UART8250_INPUTS_RING_INDICATOR			0x0040
#define UART8250_INPUTS_DATA_CARRIER_DETECT		0x0080


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*ins8250_interrupt_func)(device_t *device, int state);
typedef void (*ins8250_transmit_func)(device_t *device, int data);
typedef void (*ins8250_handshake_out_func)(device_t *device, int data);
typedef void (*ins8250_refresh_connect_func)(device_t *device);

#define INS8250_INTERRUPT(name)			void name(device_t *device, int state)
#define INS8250_TRANSMIT(name)			void name(device_t *device, int data)
#define INS8250_HANDSHAKE_OUT(name)		void name(device_t *device, int data)
#define INS8250_REFRESH_CONNECT(name)	void name(device_t *device)

typedef struct
{
	long clockin;
	ins8250_interrupt_func			interrupt;

	ins8250_transmit_func			transmit;
	ins8250_handshake_out_func		handshake_out;

	// refresh object connected to this port
	ins8250_refresh_connect_func	refresh_connected;
} ins8250_interface;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_INS8250_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, INS8250, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#define MCFG_NS16450_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, NS16450, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#define MCFG_NS16550_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, NS16550, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void ins8250_receive(device_t *device, int data);
void ins8250_handshake_in(device_t *device, int new_msr);

READ8_DEVICE_HANDLER( ins8250_r );
WRITE8_DEVICE_HANDLER( ins8250_w );


#endif /* __INS8250_H_ */
