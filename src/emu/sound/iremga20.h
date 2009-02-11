/*********************************************************

    Irem GA20 PCM Sound Chip

*********************************************************/
#pragma once

#ifndef __IREMGA20_H__
#define __IREMGA20_H__

WRITE8_DEVICE_HANDLER( irem_ga20_w );
READ8_DEVICE_HANDLER( irem_ga20_r );

DEVICE_GET_INFO( iremga20 );
#define SOUND_IREMGA20 DEVICE_GET_INFO_NAME( iremga20 )

#endif /* __IREMGA20_H__ */
