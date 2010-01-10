/*********************************************************************

    8255ppi.h

    Intel 8255 PPI I/O chip

*********************************************************************/

#ifndef __8255PPI_H_
#define __8255PPI_H_



#define PPI8255		DEVICE_GET_INFO_NAME(ppi8255)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ppi8255_interface ppi8255_interface;
struct _ppi8255_interface
{
	devcb_read8 port_a_read;
	devcb_read8 port_b_read;
	devcb_read8 port_c_read;
	devcb_write8 port_a_write;
	devcb_write8 port_b_write;
	devcb_write8 port_c_write;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PPI8255_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PPI8255, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PPI8255_RECONFIG(_tag, _intrf) \
	MDRV_DEVICE_MODIFY(_tag) \
	MDRV_DEVICE_CONFIG(_intrf)



/* device interface */
DEVICE_GET_INFO(ppi8255);

READ8_DEVICE_HANDLER( ppi8255_r );
WRITE8_DEVICE_HANDLER( ppi8255_w );


void ppi8255_set_port_a_read( const device_config *device, const devcb_read8 *config );
void ppi8255_set_port_b_read( const device_config *device, const devcb_read8 *config );
void ppi8255_set_port_c_read( const device_config *device, const devcb_read8 *config );

void ppi8255_set_port_a_write( const device_config *device, const devcb_write8 *config );
void ppi8255_set_port_b_write( const device_config *device, const devcb_write8 *config );
void ppi8255_set_port_c_write( const device_config *device, const devcb_write8 *config );

void ppi8255_set_port_a( const device_config *device, UINT8 data );
void ppi8255_set_port_b( const device_config *device, UINT8 data );
void ppi8255_set_port_c( const device_config *device, UINT8 data );

UINT8 ppi8255_get_port_a( const device_config *device );
UINT8 ppi8255_get_port_b( const device_config *device );
UINT8 ppi8255_get_port_c( const device_config *device );

#endif /* __8255PPI_H_ */
