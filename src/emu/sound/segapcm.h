/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#pragma once

#ifndef __SEGAPCM_H__
#define __SEGAPCM_H__

#define   BANK_256    (11)
#define   BANK_512    (12)
#define   BANK_12M    (13)
#define   BANK_MASK7    (0x70<<16)
#define   BANK_MASKF    (0xf0<<16)
#define   BANK_MASKF8   (0xf8<<16)

typedef struct _sega_pcm_interface sega_pcm_interface;
struct _sega_pcm_interface
{
	int  bank;
};

WRITE8_HANDLER( sega_pcm_w );
READ8_HANDLER( sega_pcm_r );

#endif /* __SEGAPCM_H__ */
