/*********************************************************************

    konami.h

    Portable Konami CPU emulator

**********************************************************************/

#pragma once

#ifndef __KONAMI_CPU_H__
#define __KONAMI_CPU_H__

#include "m6809.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// callbacks
typedef void (*konami_set_lines_func)(device_t *device, int lines);
#define KONAMI_SETLINES_CALLBACK(name) void name(device_t *device, int lines)

// device type definition
extern const device_type KONAMI;

// ======================> konami_cpu_device

class konami_cpu_device : public m6809_base_device
{
public:
	// construction/destruction
	konami_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// configuration
	void configure_set_lines(konami_set_lines_func func) { m_set_lines = func; }

protected:
	// device-level overrides
	virtual void device_start();

	// device_execute_interface overrides
	virtual void execute_run();

	// device_disasm_interface overrides
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	typedef m6809_base_device super;

	// incidentals
	konami_set_lines_func m_set_lines;

	// konami-specific addressing modes
	UINT16 &ireg();
	UINT8 read_operand();
	UINT8 read_operand(int ordinal);
	void write_operand(UINT8 data);
	void write_operand(int ordinal, UINT8 data);
	exgtfr_register read_exgtfr_register(UINT8 reg);
	void write_exgtfr_register(UINT8 reg, exgtfr_register value);

	// instructions
	void lmul();
	void divx();

	// miscellaneous
	template<class T> T safe_shift_right(T value, UINT32 shift);
	template<class T> T safe_shift_right_unsigned(T value, UINT32 shift);
	template<class T> T safe_shift_left(T value, UINT32 shift);
	void set_lines(UINT8 data);
	void execute_one();
};

#define KONAMI_IRQ_LINE 0   /* IRQ line number */
#define KONAMI_FIRQ_LINE 1   /* FIRQ line number */


//**************************************************************************
//  FUNCTIONS
//**************************************************************************

void konami_configure_set_lines(device_t *device, konami_set_lines_func func);

#endif /* __KONAMI_CPU_H__ */
