/* Ricoh RF5C400 emulator */

#pragma once

#ifndef __RF5C400_H__
#define __RF5C400_H__

READ16_HANDLER( rf5c400_0_r );
WRITE16_HANDLER( rf5c400_0_w );

SND_GET_INFO( rf5c400 );

#endif /* __RF5C400_H__ */
