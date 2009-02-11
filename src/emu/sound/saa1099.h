#pragma once

#ifndef __SAA1099_H__
#define __SAA1099_H__

/**********************************************
    Philips SAA1099 Sound driver
**********************************************/

WRITE8_DEVICE_HANDLER( saa1099_control_w );
WRITE8_DEVICE_HANDLER( saa1099_data_w );

DEVICE_GET_INFO( saa1099 );
#define SOUND_SAA1099 DEVICE_GET_INFO_NAME( saa1099 )

#endif /* __SAA1099_H__ */
