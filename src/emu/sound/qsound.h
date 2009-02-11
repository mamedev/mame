/*********************************************************

    Capcom Q-Sound system

*********************************************************/

#pragma once

#ifndef __QSOUND_H__
#define __QSOUND_H__

#define QSOUND_CLOCK    4000000   /* default 4MHz clock */

WRITE8_DEVICE_HANDLER( qsound_w );
READ8_DEVICE_HANDLER( qsound_r );

DEVICE_GET_INFO( qsound );
#define SOUND_QSOUND DEVICE_GET_INFO_NAME( qsound )

#endif /* __QSOUND_H__ */
