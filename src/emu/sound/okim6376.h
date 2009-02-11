#pragma once

#ifndef __OKIM6376_H__
#define __OKIM6376_H__

/* an interface for the OKIM6376 and similar chips */

READ8_DEVICE_HANDLER( okim6376_r );
WRITE8_DEVICE_HANDLER( okim6376_w );

DEVICE_GET_INFO( okim6376 );
#define SOUND_OKIM6376 DEVICE_GET_INFO_NAME( okim6376 )

#endif /* __OKIM6376_H__ */
