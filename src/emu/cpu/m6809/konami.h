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

typedef device_delegate<void (int lines)> konami_line_cb_delegate;
#define KONAMICPU_LINE_CB_MEMBER(_name)   void _name(int lines)

#define MCFG_KONAMICPU_LINE_CB(_class, _method) \
	konami_cpu_device::set_line_callback(*device, konami_line_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


// device type definition
extern const device_type KONAMI;

// ======================> konami_cpu_device

class konami_cpu_device : public m6809_base_device
{
public:
	// construction/destruction
	konami_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// configuration
	static void set_line_callback(device_t &device, konami_line_cb_delegate callback) { downcast<konami_cpu_device &>(device).m_set_lines = callback; }

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
	konami_line_cb_delegate m_set_lines;

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

#endif /* __KONAMI_CPU_H__ */
