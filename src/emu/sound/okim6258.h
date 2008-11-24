#pragma once

#ifndef __OKIM6258_H__
#define __OKIM6258_H__

/* an interface for the OKIM6258 and similar chips */

struct _okim6258_interface
{
	int divider;
	int adpcm_type;
	int output_12bits;
};

typedef struct _okim6258_interface okim6258_interface;

#define FOSC_DIV_BY_1024	0
#define FOSC_DIV_BY_768		1
#define FOSC_DIV_BY_512		2

#define TYPE_3BITS        	0
#define TYPE_4BITS			1

#define	OUTPUT_10BITS		0
#define	OUTPUT_12BITS		1

void okim6258_set_divider(int which, int val);
void okim6258_set_clock(int which, int val);

READ8_HANDLER( okim6258_status_0_r );
READ16_HANDLER( okim6258_status_0_lsb_r );
READ16_HANDLER( okim6258_status_0_msb_r );
WRITE8_HANDLER( okim6258_data_0_w );
READ8_HANDLER( okim6258_data_0_r );
WRITE8_HANDLER( okim6258_ctrl_0_w );
WRITE16_HANDLER( okim6258_data_0_lsb_w );
WRITE16_HANDLER( okim6258_ctrl_0_lsb_w );
READ16_HANDLER( okim6258_data_0_lsb_r );
READ16_HANDLER( okim6258_data_0_msb_r );

#endif /* __OKIM6258_H__ */
