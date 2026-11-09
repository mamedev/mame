// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    sh2fe.cpp

    Front end for SH-2 recompiler

***************************************************************************/

#include "emu.h"
#include "sh2fe.h"


/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

sh2_device::sh2_frontend::sh2_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: frontend(device, window_start, window_end, max_sequence)
{
}

uint16_t sh2_device::sh2_frontend::read_word(opcode_desc &desc)
{
	// fetch through the cache-through window the same way the interpreter does
	if (desc.physpc < 0x40000000)
		desc.physpc &= m_sh->m_am;
	return m_sh->m_pr16(desc.physpc);
}



bool sh2_device::sh2_frontend::describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	return true;
}
