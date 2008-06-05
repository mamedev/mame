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

/* register flags */
#define REGFLAG_R(n)					((UINT64)1 << (n))
#define REGFLAG_RZ(n)					(((n) == 0) ? 0 : REGFLAG_R(n))
#define REGFLAG_CR(n)					REGFLAG_R(32 + (n))
#define REGFLAG_XER						REGFLAG_R(40)
#define REGFLAG_CTR						REGFLAG_R(41)
#define REGFLAG_LR						REGFLAG_R(42)
#define REGFLAG_FPSCR					REGFLAG_R(32)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int ppcfe_describe(void *param, opcode_desc *desc);

#endif
