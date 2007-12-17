#ifndef __SP0256_H__
#define __SP0256_H__

/*
   GI SP0256 Narrator Speech Processor

   By Joe Zbiciak. Ported to MESS by tim lindner.

 Copyright (c) 1998-2000, Joseph Zbiciak, all rights reserved.
 Copyright (c) 2006, tim lindner, all rights reserved.

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

struct sp0256_interface {
	void (*lrq_callback)(int state);
	void (*sby_callback)(int state);
	int	memory_region;
};

void sp0256_bitrevbuff(UINT8 *buffer, unsigned int start, unsigned int length);

WRITE8_HANDLER( sp0256_ALD_w );

READ16_HANDLER( spb640_r );
WRITE16_HANDLER( spb640_w );

#endif
