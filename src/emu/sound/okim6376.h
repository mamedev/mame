#pragma once

#ifndef __OKIM6376_H__
#define __OKIM6376_H__

/* an interface for the OKIM6376 and similar chips */

READ8_HANDLER( okim6376_status_0_r );
READ8_HANDLER( okim6376_status_1_r );
READ8_HANDLER( okim6376_status_2_r );
READ16_HANDLER( okim6376_status_0_lsb_r );
READ16_HANDLER( okim6376_status_1_lsb_r );
READ16_HANDLER( okim6376_status_2_lsb_r );
READ16_HANDLER( okim6376_status_0_msb_r );
READ16_HANDLER( okim6376_status_1_msb_r );
READ16_HANDLER( okim6376_status_2_msb_r );
WRITE8_HANDLER( okim6376_data_0_w );
WRITE8_HANDLER( okim6376_data_1_w );
WRITE8_HANDLER( okim6376_data_2_w );
WRITE16_HANDLER( okim6376_data_0_lsb_w );
WRITE16_HANDLER( okim6376_data_1_lsb_w );
WRITE16_HANDLER( okim6376_data_2_lsb_w );
WRITE16_HANDLER( okim6376_data_0_msb_w );
WRITE16_HANDLER( okim6376_data_1_msb_w );
WRITE16_HANDLER( okim6376_data_2_msb_w );

SND_GET_INFO( okim6376 );

#endif /* __OKIM6376_H__ */
