// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    sh4fe.h

    Front end for SH-4 recompiler

***************************************************************************/
#ifndef MAME_CPU_SH_SH4FE_H
#define MAME_CPU_SH_SH4FE_H

#pragma once

#include "sh4.h"
#include "sh_fe.h"


class sh34_base_device::sh4_frontend : public sh_common_execution::frontend
{
public:
	sh4_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:
	virtual uint16_t read_word(opcode_desc &desc) override;

	virtual bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
	virtual bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
	virtual bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;

private:
	bool describe_op1111_0x13(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_op1111_0xf13(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
};

#endif // MAME_CPU_SH_SH4FE_H
