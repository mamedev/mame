/*********************************************************

    Capcom Q-Sound system

*********************************************************/

#pragma once

#ifndef __QSOUND_H__
#define __QSOUND_H__

#define QSOUND_CLOCK    4000000   /* default 4MHz clock */

WRITE8_HANDLER( qsound_data_h_w );
WRITE8_HANDLER( qsound_data_l_w );
WRITE8_HANDLER( qsound_cmd_w );
READ8_HANDLER( qsound_status_r );

SND_GET_INFO( qsound );
#define SOUND_QSOUND SND_GET_INFO_NAME( qsound )

#endif /* __QSOUND_H__ */
