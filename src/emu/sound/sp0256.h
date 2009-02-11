#pragma once

#ifndef __SP0256_H__
#define __SP0256_H__

#include "devcb.h"

/*
   GI SP0256 Narrator Speech Processor

   By Joe Zbiciak. Ported to MESS by tim lindner.

 Copyright Joseph Zbiciak, all rights reserved.
 Copyright tim lindner, all rights reserved.

 - This source code is released as freeware for non-commercial purposes.
 - You are free to use and redistribute this code in modified or
   unmodified form, provided you list us in the credits.
 - If you modify this source code, you must add a notice to each
   modified source file that it has been changed.  If you're a nice
   person, you will clearly mark each change too.  :)
 - If you wish to use this for commercial purposes, please contact us at
   intvnut@gmail.com (Joe Zbiciak), tlindner@macmess.org (tim lindner)
 - This entire notice must remain in the source code.

*/

typedef struct _sp0256_interface sp0256_interface;
struct _sp0256_interface
{
	devcb_write_line lrq_callback;
	devcb_write_line sby_callback;
};

void sp0256_bitrevbuff(UINT8 *buffer, unsigned int start, unsigned int length);

WRITE8_DEVICE_HANDLER( sp0256_ALD_w );

READ16_DEVICE_HANDLER( spb640_r );
WRITE16_DEVICE_HANDLER( spb640_w );

DEVICE_GET_INFO( sp0256 );
#define SOUND_SP0256 DEVICE_GET_INFO_NAME( sp0256 )

#endif /* __SP0256_H__ */
