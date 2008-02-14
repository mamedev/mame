/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#ifndef __SEGAPCM_H__
#define __SEGAPCM_H__

#define   BANK_256    (11)
#define   BANK_512    (12)
#define   BANK_12M    (13)
#define   BANK_MASK7    (0x70<<16)
#define   BANK_MASKF    (0xf0<<16)
#define   BANK_MASKF8   (0xf8<<16)

struct SEGAPCMinterface
{
	int  bank;
	int  region;
};

WRITE8_HANDLER( SegaPCM_w );
READ8_HANDLER( SegaPCM_r );

#endif
