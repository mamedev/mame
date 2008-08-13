/***************************************************************************

    ppcfe.h

    Front-end for PowerPC recompiler

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __PPCFE_H__
#define __PPCFE_H__

#include "cpu/drcfe.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* register flags 0 */
#define REGFLAG_R(n)					(1 << (n))
#define REGFLAG_RZ(n)					(((n) == 0) ? 0 : REGFLAG_R(n))

/* register flags 1 */
#define REGFLAG_FR(n)					(1 << (n))

/* register flags 2 */
#define REGFLAG_CR(n)					(0xf0000000 >> (4 * (n)))
#define REGFLAG_CR_BIT(n)				(0x80000000 >> (n))

/* register flags 3 */
#define REGFLAG_XER_CA					(1 << 0)
#define REGFLAG_XER_OV					(1 << 1)
#define REGFLAG_XER_SO					(1 << 2)
#define REGFLAG_XER_COUNT				(1 << 3)
#define REGFLAG_CTR						(1 << 4)
#define REGFLAG_LR						(1 << 5)
#define REGFLAG_FPSCR(n)				(1 << (6 + (n)))



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int ppcfe_describe(void *param, opcode_desc *desc, const opcode_desc *prev);

#endif /* __PPCFE_H__ */
