// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    sh4fe.c

    Front end for SH-2 recompiler

***************************************************************************/

#include "emu.h"
#include "sh4.h"
#include "sh4comn.h"
#include "cpu/drcfe.h"


/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

sh4_frontend::sh4_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: sh_frontend(device, window_start, window_end, max_sequence)
{
}

bool sh4_frontend::describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	return false;
}

bool sh4_frontend::describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	return false;
}

bool sh4_frontend::describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	return true;
}