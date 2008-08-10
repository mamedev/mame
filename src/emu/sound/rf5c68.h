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


#endif /* __RF5C68_H__ */
