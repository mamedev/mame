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

#define VERBOSE_LEVEL   (0)

#define ENABLE_VERBOSE_LOG (0)

static inline void ATTR_PRINTF(3, 4) verboselogout(device_t &dev, uint32_t pc, const char *s_fmt, ...)
{
	va_list v;
	char buf[32768];
	va_start(v, s_fmt);
	vsprintf(buf, s_fmt, v);
	va_end(v);
	dev.logerror("%08x: %s", pc, buf);
}

#define verboselog(x,y,...) do { if (ENABLE_VERBOSE_LOG && (VERBOSE_LEVEL >= y)) verboselogout(*this, x, __VA_ARGS__); } while (false)

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


