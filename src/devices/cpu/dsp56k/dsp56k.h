// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56k.h
    Interface file for the portable Motorola/Freescale DSP56k emulator.
    Written by Andrew Gardner

***************************************************************************/


#pragma once

#ifndef __DSP56K_H__
#define __DSP56K_H__

#include "emu.h"


// IRQ Lines
// MODA and MODB are also known as IRQA and IRQB
#define DSP56K_IRQ_MODA  0
#define DSP56K_IRQ_MODB  1
#define DSP56K_IRQ_MODC  2
#define DSP56K_IRQ_RESET 3  /* Is this needed? */


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/
// 5-4 Host Interface
struct dsp56k_host_interface
{
	// **** Dsp56k side **** //
	// Host Control Register
	UINT16* hcr;

	// Host Status Register
	UINT16* hsr;

	// Host Transmit/Receive Data
	UINT16* htrx;

	// **** Host CPU side **** //
	// Interrupt Control Register
	UINT8 icr;

	// Command Vector Register
	UINT8 cvr;

	// Interrupt Status Register
	UINT8 isr;

	// Interrupt Vector Register
	UINT8 ivr;

	// Transmit / Receive Registers
	UINT8 trxh;
	UINT8 trxl;

	// HACK - Host interface bootstrap write offset
	UINT16 bootstrap_offset;

};

// 1-9 ALU
struct dsp56k_data_alu
{
	// Four 16-bit input registers (can be accessed as 2 32-bit registers)
	PAIR x;
	PAIR y;

	// Two 32-bit accumulator registers + 8-bit accumulator extension registers
	PAIR64 a;
	PAIR64 b;

	// An accumulation shifter
	// One data bus shifter/limiter
	// A parallel, single cycle, non-pipelined Multiply-Accumulator (MAC) unit
	// Basics
};

// 1-10 Address Generation Unit (AGU)
struct dsp56k_agu
{
	// Four address registers
	UINT16 r0;
	UINT16 r1;
	UINT16 r2;
	UINT16 r3;

	// Four offset registers
	UINT16 n0;
	UINT16 n1;
	UINT16 n2;
	UINT16 n3;

	// Four modifier registers
	UINT16 m0;
	UINT16 m1;
	UINT16 m2;
	UINT16 m3;

	// Used in loop processing
	UINT16 temp;

	// FM.4-5 - hmmm?
	// UINT8 status;

	// Basics
};

// 1-11 Program Control Unit (PCU)
struct dsp56k_pcu
{
	// Program Counter
	UINT16 pc;

	// Loop Address
	UINT16 la;

	// Loop Counter
	UINT16 lc;

	// Status Register
	UINT16 sr;

	// Operating Mode Register
	UINT16 omr;

	// Stack Pointer
	UINT16 sp;

	// Stack (TODO: 15-level?)
	PAIR ss[16];

	// Controls IRQ processing
	void (*service_interrupts)(void);

	// A list of pending interrupts (indices into dsp56k_interrupt_sources array)
	INT8 pending_interrupts[32];

	// Basics

	// Other PCU internals
	UINT16 reset_vector;

};

// 1-8 The dsp56156 CORE
struct dsp56k_core
{
	// PROGRAM CONTROLLER
	dsp56k_pcu PCU;

	// ADR ALU (AGU)
	dsp56k_agu AGU;

	// CLOCK GEN
	//static emu_timer *dsp56k_timer;   // 1-5, 1-8 - Clock gen

	// DATA ALU
	dsp56k_data_alu ALU;

	// OnCE

	// IBS and BITFIELD UNIT

	// Host Interface
	dsp56k_host_interface HI;

	// IRQ line states
	UINT8 modA_state;
	UINT8 modB_state;
	UINT8 modC_state;
	UINT8 reset_state;

	// HACK - Bootstrap mode state variable.
	UINT8 bootstrap_mode;

	UINT8   repFlag;    // Knowing if we're in a 'repeat' state (dunno how the processor does this)
	UINT32  repAddr;    // The address of the instruction to repeat...


	/* MAME internal stuff */
	int icount;

	UINT32          ppc;
	UINT32          op;
	int             interrupt_cycles;
	void            (*output_pins_changed)(UINT32 pins);
	cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *data;

	UINT16 peripheral_ram[0x40];
	UINT16 *program_ram;
};


class dsp56k_device : public cpu_device
{
public:
	dsp56k_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	DECLARE_READ16_MEMBER( peripheral_register_r );
	DECLARE_WRITE16_MEMBER( peripheral_register_w );

	void  host_interface_write(UINT8 offset, UINT8 data);
	UINT8 host_interface_read(UINT8 offset);

	UINT16 get_peripheral_memory(UINT16 addr);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 2 - 1) / 2; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 2); }
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 8; }
	virtual UINT32 execute_input_lines() const override { return 4; }
	virtual UINT32 execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_DATA) ? &m_data_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	required_shared_ptr<UINT16> m_program_ram;

	dsp56k_core m_dsp56k_core;

	void agu_init();
	void alu_init();

};


extern const device_type DSP56156;


#endif /* __DSP56K_H__ */
