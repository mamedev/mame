/*********************************************************************

    formats/g64_dsk.h

    Floppy format code for Commodore 1541 GCR disk images

*********************************************************************/

#ifndef __G64_DSK__
#define __G64_DSK__

#include "flopimg.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define G64_SYNC_MARK           0x3ff       /* 10 consecutive 1-bits */

#define G64_BUFFER_SIZE         16384
#define G64_SPEED_BLOCK_SIZE    1982

const int C2040_BITRATE[] =
{
	XTAL_16MHz/16,  /* tracks  1-17 */
	XTAL_16MHz/15,  /* tracks 18-24 */
	XTAL_16MHz/14,  /* tracks 25-30 */
	XTAL_16MHz/13   /* tracks 31-42 */
};

const int C8050_BITRATE[] =
{
	XTAL_12MHz/2/16,    /* tracks  1-39 */
	XTAL_12MHz/2/15,    /* tracks 40-53 */
	XTAL_12MHz/2/14,    /* tracks 54-65 */
	XTAL_12MHz/2/13     /* tracks 65-84 */
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

FLOPPY_IDENTIFY( g64_dsk_identify );
FLOPPY_CONSTRUCT( g64_dsk_construct );

#endif
