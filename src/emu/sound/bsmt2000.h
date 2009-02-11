/**********************************************************************************************
 *
 *   BSMT2000 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __BSMT2000_H__
#define __BSMT2000_H__

WRITE16_DEVICE_HANDLER( bsmt2000_data_w );

DEVICE_GET_INFO( bsmt2000 );
#define SOUND_BSMT2000 DEVICE_GET_INFO_NAME( bsmt2000 )

#endif /* __BSMT2000_H__ */
