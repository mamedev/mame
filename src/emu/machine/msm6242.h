/***************************************************************************

    MSM6242 Real Time Clock

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MSM6242_H__
#define __MSM6242_H__


#define MSM6242		DEVICE_GET_INFO_NAME(msm6242)

#define MDRV_MSM6242_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, MSM6242, 0)


/* device interface */
DEVICE_GET_INFO( msm6242 );

READ8_DEVICE_HANDLER( msm6242_r );
WRITE8_DEVICE_HANDLER( msm6242_w );


#endif /* __MSM6242_H__ */
