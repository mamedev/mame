#pragma once

#ifndef __2612INTF_H__
#define __2612INTF_H__

void ym2612_update_request(void *param);

typedef struct _ym2612_interface ym2612_interface;
struct _ym2612_interface
{
	void (*handler)(running_machine *machine, int irq);
};


/************************************************/
/* Chip 0 functions                             */
/************************************************/
READ8_HANDLER( ym2612_status_port_0_a_r );  /* A=0 : OPN status */
READ8_HANDLER( ym2612_status_port_0_b_r );  /* A=2 : don't care */
READ8_HANDLER( ym2612_read_port_0_r );        /* A=1 : don't care */
WRITE8_HANDLER( ym2612_control_port_0_a_w ); /* A=0:OPN  address */
WRITE8_HANDLER( ym2612_control_port_0_b_w ); /* A=2:OPN2 address */
WRITE8_HANDLER( ym2612_data_port_0_a_w );    /* A=1:OPN  data    */
WRITE8_HANDLER( ym2612_data_port_0_b_w );    /* A=3:OPN2 data    */

/************************************************/
/* Chip 1 functions                             */
/************************************************/
READ8_HANDLER( ym2612_status_port_1_a_r );
READ8_HANDLER( ym2612_status_port_1_b_r );
READ8_HANDLER( ym2612_read_port_1_r );
WRITE8_HANDLER( ym2612_control_port_1_a_w );
WRITE8_HANDLER( ym2612_control_port_1_b_w );
WRITE8_HANDLER( ym2612_data_port_1_a_w );
WRITE8_HANDLER( ym2612_data_port_1_b_w );

SND_GET_INFO( ym2612 );
#define SOUND_YM2612 SND_GET_INFO_NAME( ym2612 )


typedef struct _ym3438_interface ym3438_interface;
struct _ym3438_interface
{
	void (*handler)(running_machine *machine, int irq);
};


/************************************************/
/* Chip 0 functions                             */
/************************************************/
READ8_HANDLER( ym3438_status_port_0_a_r );  /* A=0 : OPN status */
READ8_HANDLER( ym3438_status_port_0_b_r );  /* A=2 : don't care */
READ8_HANDLER( ym3438_read_port_0_r );        /* A=1 : don't care */
WRITE8_HANDLER( ym3438_control_port_0_a_w ); /* A=0:OPN  address */
WRITE8_HANDLER( ym3438_control_port_0_b_w ); /* A=2:OPN2 address */
WRITE8_HANDLER( ym3438_data_port_0_a_w );    /* A=1:OPN  data    */
WRITE8_HANDLER( ym3438_data_port_0_b_w );    /* A=3:OPN2 data    */

/************************************************/
/* Chip 1 functions                             */
/************************************************/
READ8_HANDLER( ym3438_status_port_1_a_r );
READ8_HANDLER( ym3438_status_port_1_b_r );
READ8_HANDLER( ym3438_read_port_1_r );
WRITE8_HANDLER( ym3438_control_port_1_a_w );
WRITE8_HANDLER( ym3438_control_port_1_b_w );
WRITE8_HANDLER( ym3438_data_port_1_a_w );
WRITE8_HANDLER( ym3438_data_port_1_b_w );

SND_GET_INFO( ym3438 );
#define SOUND_YM3438 SND_GET_INFO_NAME( ym3438 )

#endif /* __2612INTF_H__ */
