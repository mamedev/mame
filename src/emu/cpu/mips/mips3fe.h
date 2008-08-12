/***************************************************************************

    mips3fe.h

    Front-end for MIPS3 recompiler

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MIPS3FE_H__
#define __MIPS3FE_H__

#include "cpu/drcfe.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* register flags 0 */
#define REGFLAG_R(n)					(((n) == 0) ? 0 : (1 << (n)))

/* register flags 1 */
#define REGFLAG_CPR1(n)					(1 << (n))

/* register flags 2 */
#define REGFLAG_LO						(1 << 0)
#define REGFLAG_HI						(1 << 1)
#define REGFLAG_FCC						(1 << 2)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int mips3fe_describe(void *param, opcode_desc *desc, const opcode_desc *prev);

#endif /* __MIPS3FE_H__ */
