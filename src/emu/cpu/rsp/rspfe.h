/***************************************************************************

    rspfe.h

    Front-end for RSP recompiler

    Copyright the MESS team
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __RSPFE_H__
#define __RSPFE_H__

#include "cpu/drcfe.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* register flags 0 */
#define REGFLAG_R(n)					(((n) == 0) ? 0 : (1 << (n)))

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int rspfe_describe(void *param, opcode_desc *desc, const opcode_desc *prev);

#endif /* __RSPFE_H__ */
