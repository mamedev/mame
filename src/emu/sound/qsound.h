/*********************************************************

    Capcom Q-Sound system

*********************************************************/

#ifndef __QSOUND_H__
#define __QSOUND_H__

#define QSOUND_CLOCK    4000000   /* default 4MHz clock */

struct QSound_interface {
	int region;					/* memory region of sample ROM(s) */
};

WRITE8_HANDLER( qsound_data_h_w );
WRITE8_HANDLER( qsound_data_l_w );
WRITE8_HANDLER( qsound_cmd_w );
READ8_HANDLER( qsound_status_r );

#endif /* __QSOUND_H__ */
