/*********************************************************************

    8255ppi.h

    Intel 8255 PPI I/O chip

*********************************************************************/

#ifndef _8255PPI_H_
#define _8255PPI_H_

#define MAX_8255 8

typedef struct
{
	int num;							 /* number of PPIs to emulate */
	read8_handler portAread[MAX_8255];
	read8_handler portBread[MAX_8255];
	read8_handler portCread[MAX_8255];
	write8_handler portAwrite[MAX_8255];
	write8_handler portBwrite[MAX_8255];
	write8_handler portCwrite[MAX_8255];
} ppi8255_interface;


/* Init */
void ppi8255_init( const ppi8255_interface *intfce);

/* Read/Write */
UINT8 ppi8255_r ( int which, offs_t offset );
void ppi8255_w( int which, offs_t offset, UINT8 data );

void ppi8255_set_portAread( int which, read8_handler portAread);
void ppi8255_set_portBread( int which, read8_handler portBread);
void ppi8255_set_portCread( int which, read8_handler portCread);

void ppi8255_set_portAwrite( int which, write8_handler portAwrite);
void ppi8255_set_portBwrite( int which, write8_handler portBwrite);
void ppi8255_set_portCwrite( int which, write8_handler portCwrite);

void ppi8255_set_portA( int which, UINT8 data );
void ppi8255_set_portB( int which, UINT8 data );
void ppi8255_set_portC( int which, UINT8 data );

UINT8 ppi8255_get_portA( int which );
UINT8 ppi8255_get_portB( int which );
UINT8 ppi8255_get_portC( int which );

/* Helpers */
READ8_HANDLER( ppi8255_0_r );
READ8_HANDLER( ppi8255_1_r );
READ8_HANDLER( ppi8255_2_r );
READ8_HANDLER( ppi8255_3_r );
READ8_HANDLER( ppi8255_4_r );
READ8_HANDLER( ppi8255_5_r );
READ8_HANDLER( ppi8255_6_r );
READ8_HANDLER( ppi8255_7_r );
WRITE8_HANDLER( ppi8255_0_w );
WRITE8_HANDLER( ppi8255_1_w );
WRITE8_HANDLER( ppi8255_2_w );
WRITE8_HANDLER( ppi8255_3_w );
WRITE8_HANDLER( ppi8255_4_w );
WRITE8_HANDLER( ppi8255_5_w );
WRITE8_HANDLER( ppi8255_6_w );
WRITE8_HANDLER( ppi8255_7_w );

READ16_HANDLER( ppi8255_16le_0_r );
READ16_HANDLER( ppi8255_16le_1_r );
READ16_HANDLER( ppi8255_16le_2_r );
READ16_HANDLER( ppi8255_16le_3_r );
READ16_HANDLER( ppi8255_16le_4_r );
READ16_HANDLER( ppi8255_16le_5_r );
READ16_HANDLER( ppi8255_16le_6_r );
READ16_HANDLER( ppi8255_16le_7_r );
WRITE16_HANDLER( ppi8255_16le_0_w );
WRITE16_HANDLER( ppi8255_16le_1_w );
WRITE16_HANDLER( ppi8255_16le_2_w );
WRITE16_HANDLER( ppi8255_16le_3_w );
WRITE16_HANDLER( ppi8255_16le_4_w );
WRITE16_HANDLER( ppi8255_16le_5_w );
WRITE16_HANDLER( ppi8255_16le_6_w );
WRITE16_HANDLER( ppi8255_16le_7_w );

#ifdef MESS
/* Peek at the ports */
UINT8 ppi8255_peek( int which, offs_t offset );
#endif

#endif /* _8255PPI_H_ */
