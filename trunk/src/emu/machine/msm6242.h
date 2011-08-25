/***************************************************************************

    MSM6242 Real Time Clock

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MSM6242_H__
#define __MSM6242_H__

#include "devlegcy.h"


DECLARE_LEGACY_DEVICE(MSM6242, msm6242);

#define MCFG_MSM6242_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MSM6242, 0)


READ8_DEVICE_HANDLER( msm6242_r );
WRITE8_DEVICE_HANDLER( msm6242_w );


#endif /* __MSM6242_H__ */
