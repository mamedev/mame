/*********************************************************

    Irem GA20 PCM Sound Chip

*********************************************************/
#pragma once

#ifndef __IREMGA20_H__
#define __IREMGA20_H__

WRITE16_HANDLER( irem_ga20_w );
READ16_HANDLER( irem_ga20_r );

SND_GET_INFO( iremga20 );
#define SOUND_IREMGA20 SND_GET_INFO_NAME( iremga20 )

#endif /* __IREMGA20_H__ */
