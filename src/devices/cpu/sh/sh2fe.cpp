// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    sh2fe.c

    Front end for SH-2 recompiler

***************************************************************************/

#include "emu.h"
#include "sh2.h"
#include "sh2comn.h"
#include "cpu/drcfe.h"


/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

sh2_frontend::sh2_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: sh_frontend(device, window_start, window_end, max_sequence)
{
}



bool sh2_frontend::describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	return true;
}
