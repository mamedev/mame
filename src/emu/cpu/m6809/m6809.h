/*** m6809: Portable 6809 emulator ******************************************/

#pragma once

#ifndef __M6809_H__
#define __M6809_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CPU_M6809_CONFIG(_config) \
	m6809_base_device::static_set_config(*device, _config); \

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m6809_device;

// ======================> m6809_config

struct m6809_config
{
	bool m_encrypt_only_first_byte;
};


// device type definition
extern const device_type M6809;
extern const device_type M6809E;

// ======================> m6809_device

// Used by core CPU interface
class m6809_base_device : public cpu_device,
						  public m6809_config
{
public:
	// construction/destruction
	m6809_base_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, const device_type type, int divider);

	// inline configuration helpers
	static void static_set_config(device_t &device, const m6809_config &config);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const;
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, astring &string);

private:
	UINT32 RM16(UINT32 addr);
	void WM16(UINT32 addr, PAIR *p);

	void IIError();
	void fetch_effective_address();

	void check_irq_lines();
	void set_irq_line(int irqline, int state);
	void update_state();

	offs_t disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);

	#include "6809tbl.h"

	// opcode/condition tables
	static const UINT8 m_flags8i[256];
	static const UINT8 m_flags8d[256];
	static const UINT8 m_index_cycle_em[256];
	static const UINT8 m_cycles1[256];

	typedef void (m6809_base_device::*ophandler)();

	ophandler m_opcode[256];

	static const ophandler s_opcodetable[256];

protected:
	const char *m_tag;

	// address spaces
	const address_space_config m_program_config;

	// CPU registers
	PAIR	m_pc; 		/* Program counter */
	PAIR	m_ppc;		/* Previous program counter */
	PAIR	m_d;		/* Accumulator a and b */
	PAIR	m_dp; 		/* Direct Page register (page in MSB) */
	PAIR	m_u, m_s;	/* Stack pointers */
	PAIR	m_x, m_y;	/* Index registers */
	UINT8	m_cc;
	UINT8	m_ireg;		/* First opcode */
	UINT8	m_irq_state[2];

	int 	m_extra_cycles;	/* cycles used up by interrupts */

	device_irq_acknowledge_callback m_irq_callback;

	PAIR	m_ea;		/* effective address */

	// other internal states
    int 	m_icount;
	UINT8	m_int_state;	/* SYNC and CWAI flags */
	UINT8	m_nmi_state;
	int		m_clock_divider;

	// address spaces
    address_space *m_program;
    direct_read_data *m_direct;
};

// device type definition
extern const device_type ATMEGA88;
extern const device_type ATMEGA644;

// ======================> m6809_device

class m6809_device : public m6809_base_device
{
public:
	// construction/destruction
	m6809_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	 : m6809_base_device(mconfig, tag, owner, clock, M6809, 1) { }
};

// ======================> m6809e_device

class m6809e_device : public m6809_base_device
{
public:
	// construction/destruction
	m6809e_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	 : m6809_base_device(mconfig, tag, owner, clock, M6809E, 4) { }
};

enum
{
	M6809_PC=1, M6809_S, M6809_CC ,M6809_A, M6809_B, M6809_U, M6809_X, M6809_Y,
	M6809_DP
};

#define M6809_IRQ_LINE	0	/* IRQ line number */
#define M6809_FIRQ_LINE 1   /* FIRQ line number */

/* M6809e has LIC line to indicate opcode/data fetch */


CPU_DISASSEMBLE( m6809 );

#endif /* __M6809_H__ */
