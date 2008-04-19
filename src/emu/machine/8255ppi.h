/*********************************************************************

    8255ppi.h

    Intel 8255 PPI I/O chip

*********************************************************************/

#ifndef __8255PPI_H_
#define __8255PPI_H_

#define PPI8255		DEVICE_GET_INFO_NAME(ppi8255)


typedef struct
{
	read8_machine_func portAread;
	read8_machine_func portBread;
	read8_machine_func portCread;
	write8_machine_func portAwrite;
	write8_machine_func portBwrite;
	write8_machine_func portCwrite;
} ppi8255_interface;


/* device interface */
DEVICE_GET_INFO(ppi8255);

READ8_DEVICE_HANDLER( ppi8255_r );
WRITE8_DEVICE_HANDLER( ppi8255_w );


void ppi8255_set_portAread( const device_config *device, read8_machine_func portAread );
void ppi8255_set_portBread( const device_config *device, read8_machine_func portBread );
void ppi8255_set_portCread( const device_config *device, read8_machine_func portCread );

void ppi8255_set_portAwrite( const device_config *device, write8_machine_func portAwrite );
void ppi8255_set_portBwrite( const device_config *device, write8_machine_func portBwrite );
void ppi8255_set_portCwrite( const device_config *device, write8_machine_func portCwrite );

void ppi8255_set_portA( const device_config *device, UINT8 data );
void ppi8255_set_portB( const device_config *device, UINT8 data );
void ppi8255_set_portC( const device_config *device, UINT8 data );

UINT8 ppi8255_get_portA( const device_config *device );
UINT8 ppi8255_get_portB( const device_config *device );
UINT8 ppi8255_get_portC( const device_config *device );

#endif /* __8255PPI_H_ */
