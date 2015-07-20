// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devcpu.h

    CPU device definitions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVCPU_H__
#define __DEVCPU_H__

#include "emuopts.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// CPU information constants
const int MAX_REGS = 256;
enum
{
	// --- the following bits of info are returned as 64-bit signed integers ---
	CPUINFO_INT_FIRST = 0x00000,

	CPUINFO_INT_CPU_SPECIFIC = 0x08000,                     // R/W: CPU-specific values start here

	// --- the following bits of info are returned as pointers to data or functions ---
	CPUINFO_PTR_FIRST = 0x10000,

		// CPU-specific additions
		CPUINFO_PTR_INSTRUCTION_COUNTER = 0x14000,
															// R/O: int *icount

	CPUINFO_PTR_CPU_SPECIFIC = 0x18000, // R/W: CPU-specific values start here

	// --- the following bits of info are returned as pointers to functions ---
	CPUINFO_FCT_FIRST = 0x20000,

	CPUINFO_FCT_CPU_SPECIFIC = 0x28000, // R/W: CPU-specific values start here

	// --- the following bits of info are returned as NULL-terminated strings ---
	CPUINFO_STR_FIRST = 0x30000,

	CPUINFO_STR_CPU_SPECIFIC = 0x38000  // R/W: CPU-specific values start here
};



//**************************************************************************
//  CPU DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CPU_ADD MCFG_DEVICE_ADD
#define MCFG_CPU_MODIFY MCFG_DEVICE_MODIFY
#define MCFG_CPU_REPLACE MCFG_DEVICE_REPLACE

#define MCFG_CPU_CLOCK MCFG_DEVICE_CLOCK
#define MCFG_CPU_CONFIG MCFG_DEVICE_CONFIG

#define MCFG_CPU_PROGRAM_MAP MCFG_DEVICE_PROGRAM_MAP
#define MCFG_CPU_DATA_MAP MCFG_DEVICE_DATA_MAP
#define MCFG_CPU_IO_MAP MCFG_DEVICE_IO_MAP
#define MCFG_CPU_DECRYPTED_OPCODES_MAP MCFG_DEVICE_DECRYPTED_OPCODES_MAP

#define MCFG_CPU_VBLANK_INT_DRIVER MCFG_DEVICE_VBLANK_INT_DRIVER
#define MCFG_CPU_PERIODIC_INT_DRIVER MCFG_DEVICE_PERIODIC_INT_DRIVER
#define MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER
#define MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE

#define MCFG_CPU_VBLANK_INT_REMOVE MCFG_DEVICE_VBLANK_INT_REMOVE
#define MCFG_CPU_PERIODIC_INT_REMOVE MCFG_DEVICE_PERIODIC_INT_REMOVE
#define MCFG_CPU_IRQ_ACKNOWLEDGE_REMOVE MCFG_DEVICE_IRQ_ACKNOWLEDGE_REMOVE


//**************************************************************************
//  MACROS
//**************************************************************************

#define CPU_DISASSEMBLE_NAME(name)      cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)           offs_t CPU_DISASSEMBLE_NAME(name)(cpu_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options)
#define CPU_DISASSEMBLE_CALL(name)      CPU_DISASSEMBLE_NAME(name)(device, buffer, pc, oprom, opram, options)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cpu_device

class cpu_device :  public device_t,
					public device_execute_interface,
					public device_memory_interface,
					public device_state_interface,
					public device_disasm_interface
{
	friend resource_pool_object<cpu_device>::~resource_pool_object();

protected:
	// construction/destruction
	cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~cpu_device();
};


typedef offs_t (*cpu_disassemble_func)(cpu_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);


#endif  /* __CPUINTRF_H__ */
