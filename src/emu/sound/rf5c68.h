/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#pragma once

#ifndef __RF5C68_H__
#define __RF5C68_H__

/******************************************/
WRITE8_HANDLER( rf5c68_reg_w );

READ8_HANDLER( rf5c68_r );
WRITE8_HANDLER( rf5c68_w );

SND_GET_INFO( rf5c68 );
#define SOUND_RF5C68 SND_GET_INFO_NAME( rf5c68 )

#endif /* __RF5C68_H__ */
