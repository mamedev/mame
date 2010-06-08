#pragma once

#ifndef __SAA1099_H__
#define __SAA1099_H__

#include "devlegcy.h"

/**********************************************
    Philips SAA1099 Sound driver
**********************************************/

WRITE8_DEVICE_HANDLER( saa1099_control_w );
WRITE8_DEVICE_HANDLER( saa1099_data_w );

DECLARE_LEGACY_SOUND_DEVICE(SAA1099, saa1099);

#endif /* __SAA1099_H__ */
