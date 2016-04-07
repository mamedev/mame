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

// recompilation parameters
#define MCFG_CPU_FORCE_NO_DRC() \
	cpu_device::static_set_force_no_drc(*device, true);



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

public:
	// configuration helpers
	static void static_set_force_no_drc(device_t &device, bool value);
	bool allow_drc() const;

protected:
	// construction/destruction
	cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~cpu_device();

private:
	// configured state
	bool                    m_force_no_drc;             // whether or not to force DRC off
};


typedef offs_t (*cpu_disassemble_func)(cpu_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);


#endif  /* __CPUINTRF_H__ */
