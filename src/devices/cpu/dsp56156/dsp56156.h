// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56156.h
    Interface file for the portable Motorola/Freescale DSP56156 emulator.
    Written by Andrew Gardner

***************************************************************************/


#ifndef MAME_CPU_DSP56156_DSP56156_H
#define MAME_CPU_DSP56156_DSP56156_H

#pragma once

#include <algorithm>


// IRQ Lines
// MODA and MODB are also known as IRQA and IRQB
#define DSP56156_IRQ_MODA  0
#define DSP56156_IRQ_MODB  1
#define DSP56156_IRQ_MODC  2
#define DSP56156_IRQ_RESET 3  /* Is this needed? */

namespace DSP_56156 {

class dsp56156_device;

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/
// 5-4 Host Interface
struct dsp56156_host_interface
{
	dsp56156_host_interface()
		: hcr(nullptr), hsr(nullptr), htrx(nullptr)
		, icr(0), cvr(0), isr(0), ivr(0), trxh(0), trxl(0)
		, bootstrap_offset(0)
	{
	}

	// **** DSP56156 side **** //
	// Host Control Register
	uint16_t *hcr;

	// Host Status Register
	uint16_t *hsr;

	// Host Transmit/Receive Data
	uint16_t *htrx;

	// **** Host CPU side **** //
	// Interrupt Control Register
	uint8_t icr;

	// Command Vector Register
	uint8_t cvr;

	// Interrupt Status Register
	uint8_t isr;

	// Interrupt Vector Register
	uint8_t ivr;

	// Transmit / Receive Registers
	uint8_t trxh;
	uint8_t trxl;

	// HACK - Host interface bootstrap write offset
	uint16_t bootstrap_offset;
};

// 1-9 ALU
struct dsp56156_data_alu
{
	dsp56156_data_alu()
	{
		x.d = 0;
		y.d = 0;
		a.q = 0;
		b.q = 0;
	}

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
struct dsp56156_agu
{
	dsp56156_agu()
		: r0(0), r1(0), r2(0), r3(0)
		, n0(0), n1(0), n2(0), n3(0)
		, m0(0), m1(0), m2(0), m3(0)
		, temp(0)
	{
	}

	// Four address registers
	uint16_t r0;
	uint16_t r1;
	uint16_t r2;
	uint16_t r3;

	// Four offset registers
	uint16_t n0;
	uint16_t n1;
	uint16_t n2;
	uint16_t n3;

	// Four modifier registers
	uint16_t m0;
	uint16_t m1;
	uint16_t m2;
	uint16_t m3;

	// Used in loop processing
	uint16_t temp;

	// FM.4-5 - hmmm?
	// uint8_t status;

	// Basics
};

// 1-11 Program Control Unit (PCU)
struct dsp56156_pcu
{
	dsp56156_pcu()
		: pc(0), la(0), lc(0), sr(0), omr(0), sp(0)
		, service_interrupts(nullptr)
		, reset_vector(0)
		, ipc(0)
	{
		for (auto &s : ss)
			s.d = 0;
		std::fill(std::begin(pending_interrupts), std::end(pending_interrupts), 0);
	}

	uint16_t pc;     // Program Counter
	uint16_t la;     // Loop Address
	uint16_t lc;     // Loop Counter
	uint16_t sr;     // Status Register
	uint16_t omr;    // Operating Mode Register
	uint16_t sp;     // Stack Pointer
	PAIR     ss[16]; // Stack (TODO: 15-level?)

	// Controls IRQ processing
	void (*service_interrupts)(void);

	// A list of pending interrupts (indices into dsp56156_interrupt_sources array)
	int8_t pending_interrupts[32];

	// Basics

	// Other PCU internals
	uint16_t reset_vector;
	uint16_t ipc;
};

// 1-8 The dsp56156 CORE
struct dsp56156_core
{
	dsp56156_core()
		: modA_state(false), modB_state(false), modC_state(false), reset_state(false)
		, bootstrap_mode(0)
		, repFlag(0), repAddr(0)
		, icount(0)
		, ppc(0)
		, op(0)
		, interrupt_cycles(0)
		, output_pins_changed(nullptr)
		, device(nullptr)
		, program_ram(nullptr)
	{
		std::fill(std::begin(peripheral_ram), std::end(peripheral_ram), 0);
	}

	// PROGRAM CONTROLLER
	dsp56156_pcu PCU;

	// ADR ALU (AGU)
	dsp56156_agu AGU;

	// CLOCK GEN
	//static emu_timer *dsp56156_timer;   // 1-5, 1-8 - Clock gen

	// DATA ALU
	dsp56156_data_alu ALU;

	// OnCE

	// IBS and BITFIELD UNIT

	// Host Interface
	dsp56156_host_interface HI;

	// IRQ line states
	bool modA_state;
	bool modB_state;
	bool modC_state;
	bool reset_state;

	// HACK - Bootstrap mode state variable.
	uint8_t bootstrap_mode;

	uint8_t   repFlag;    // Knowing if we're in a 'repeat' state (dunno how the processor does this)
	uint32_t  repAddr;    // The address of the instruction to repeat...

	// MAME internal stuff
	int icount;

	uint32_t          ppc;
	uint32_t          op;
	int               interrupt_cycles;
	void              (*output_pins_changed)(uint32_t pins);
	dsp56156_device   *device;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::cache cache;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific program;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific data;

	uint16_t peripheral_ram[0x40];
	uint16_t *program_ram;
};


class dsp56156_device : public cpu_device
{
public:
	dsp56156_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	uint16_t peripheral_register_r(offs_t offset);
	void peripheral_register_w(offs_t offset, uint16_t data);

	void host_interface_write(uint8_t offset, uint8_t data);
	uint8_t host_interface_read(uint8_t offset);

	void dsp56156_program_map(address_map &map) ATTR_COLD;
	void dsp56156_x_data_map(address_map &map) ATTR_COLD;

	auto portc_cb() { return portC_cb.bind(); }

	void output_portc(uint16_t value) { portC_cb(value); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return clocks; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return cycles; }
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 8; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == DSP56156_IRQ_RESET; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	required_shared_ptr<uint16_t> m_program_ram;

	dsp56156_core m_core;

	devcb_write16 portC_cb;

	void agu_init();
	void alu_init();
};

} // namespace DSP_56156


DECLARE_DEVICE_TYPE_NS(DSP56156, DSP_56156, dsp56156_device)
using DSP_56156::dsp56156_device;

#endif // MAME_CPU_DSP56156_DSP56156_H
