// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    sh2fe.cpp

    Front end for SH-2 recompiler

***************************************************************************/
#ifndef MAME_CPU_SH_SH2FE_H
#define MAME_CPU_SH_SH2FE_H

#pragma once

#include "sh2.h"
#include "sh_fe.h"


class sh2_device::sh2_frontend : public sh_common_execution::frontend
{
public:
	sh2_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

private:
	virtual bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
};

#endif // MAME_CPU_SH_SH2FE_H
