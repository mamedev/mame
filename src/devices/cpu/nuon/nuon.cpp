// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    VM Labs Aries 3 "NUON Multi-Media Architecture" simulator

    - Changelist -
      10 Mar. 2018
      - Initial skeleton version.
*/

#include "emu.h"
#include "nuon.h"
#include "nuondasm.h"

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

DEFINE_DEVICE_TYPE(NUON,   nuon_device,   "nuon",   "Aries 3 \"Nuon\"")

//-------------------------------------------------
//  nuon_device - constructor
//-------------------------------------------------

nuon_device::nuon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, NUON, tag, owner, clock)
	, m_program_configs{{"program_mpe0", ENDIANNESS_BIG, 32, 32},
						{"program_mpe1", ENDIANNESS_BIG, 32, 32},
						{"program_mpe2", ENDIANNESS_BIG, 32, 32},
						{"program_mpe3", ENDIANNESS_BIG, 32, 32}}
{
}

//-------------------------------------------------
//  unimplemented_opcode - bail on unspuported
//  instruction
//-------------------------------------------------

void nuon_device::unimplemented_opcode(uint32_t op)
{
//  machine().debug_break();
	fatalerror("Nuon: unknown opcode (%08x) at %08x\n", op, m_pc);
}


