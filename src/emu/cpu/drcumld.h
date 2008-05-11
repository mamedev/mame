/***************************************************************************

    drcumld.h

    Universal machine language disassembler.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DRCUMLD_H__
#define __DRCUMLD_H__

#include "drcuml.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* disassemble one instruction */
void drcuml_disasm(const drcuml_instruction *inst, char *buffer);


#endif
