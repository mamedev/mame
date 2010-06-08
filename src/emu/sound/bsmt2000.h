/**********************************************************************************************
 *
 *   BSMT2000 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __BSMT2000_H__
#define __BSMT2000_H__

#include "devlegcy.h"

WRITE16_DEVICE_HANDLER( bsmt2000_data_w );

DECLARE_LEGACY_SOUND_DEVICE(BSMT2000, bsmt2000);

#endif /* __BSMT2000_H__ */
