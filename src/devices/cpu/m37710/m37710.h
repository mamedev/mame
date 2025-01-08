// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud
#ifndef MAME_CPU_M37710_M37710_H
#define MAME_CPU_M37710_M37710_H

#pragma once

/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

M37710 CPU Emulator v0.1

*/

#include "m7700ds.h"


/* ======================================================================== */
/* =============================== DEFINES ================================ */
/* ======================================================================== */
/*
   Input lines - used with cpunum_set_input_line() and the like.
   WARNING: these are in the same order as the vector table for simplicity.
   Do not alter this order!
*/

enum
{
	// these interrupts are maskable
	M37710_LINE_DMA3 = 0,
	M37710_LINE_DMA2,
	M37710_LINE_DMA1,
	M37710_LINE_DMA0,
	M37710_LINE_ADC,
	M37710_LINE_UART1XMIT,
	M37710_LINE_UART1RECV,
	M37710_LINE_UART0XMIT,
	M37710_LINE_UART0RECV,
	M37710_LINE_TIMERB2,
	M37710_LINE_TIMERB1,
	M37710_LINE_TIMERB0,
	M37710_LINE_TIMERA4,
	M37710_LINE_TIMERA3,
	M37710_LINE_TIMERA2,
	M37710_LINE_TIMERA1,
	M37710_LINE_TIMERA0,
	M37710_LINE_IRQ2,
	M37710_LINE_IRQ1,
	M37710_LINE_IRQ0,
	// these interrupts are non-maskable
	M37710_LINE_WATCHDOG,
	M37710_LINE_DEBUG,
	M37710_LINE_BRK,
	M37710_LINE_ZERODIV,
	M37710_LINE_RESET,

	// these are not interrupts, they're signals external hardware can send
	M37710_LINE_TIMERA0IN,
	M37710_LINE_TIMERA1IN,
	M37710_LINE_TIMERA2IN,
	M37710_LINE_TIMERA3IN,
	M37710_LINE_TIMERA4IN,
	M37710_LINE_TIMERB0IN,
	M37710_LINE_TIMERB1IN,
	M37710_LINE_TIMERB2IN,

	M37710_LINE_TIMERA0OUT,
	M37710_LINE_TIMERA1OUT,
	M37710_LINE_TIMERA2OUT,
	M37710_LINE_TIMERA3OUT,
	M37710_LINE_TIMERA4OUT,
	M37710_LINE_TIMERB0OUT,
	M37710_LINE_TIMERB1OUT,
	M37710_LINE_TIMERB2OUT,

	M37710_LINE_MAX
};

#define M37710_INTERRUPT_MAX (M37710_LINE_RESET + 1)
#define M37710_MASKABLE_INTERRUPTS (M37710_INTERRUPT_MAX - 5)


/* Registers - used by m37710_set_reg() and m37710_get_reg() */
enum
{
	M37710_PC=1, M37710_S, M37710_PS, M37710_A, M37710_B, M37710_X, M37710_Y,
	M37710_PG, M37710_DT, M37710_DPR, M37710_E,
	M37710_NMI_STATE, M37710_IRQ_STATE
};


// internal ROM region
#define M37710_INTERNAL_ROM_REGION "internal"
#define M37710_INTERNAL_ROM(_tag) (_tag ":" M37710_INTERNAL_ROM_REGION)

class m37710_cpu_device : public cpu_device, public m7700_disassembler::config
{
public:
	auto p0_in_cb() { return m_port_in_cb[0].bind(); }
	auto p0_out_cb() { return m_port_out_cb[0].bind(); }
	auto p1_in_cb() { return m_port_in_cb[1].bind(); }
	auto p1_out_cb() { return m_port_out_cb[1].bind(); }
	auto p2_in_cb() { return m_port_in_cb[2].bind(); }
	auto p2_out_cb() { return m_port_out_cb[2].bind(); }
	auto p3_in_cb() { return m_port_in_cb[3].bind(); }
	auto p3_out_cb() { return m_port_out_cb[3].bind(); }
	auto p4_in_cb() { return m_port_in_cb[4].bind(); }
	auto p4_out_cb() { return m_port_out_cb[4].bind(); }
	auto p5_in_cb() { return m_port_in_cb[5].bind(); }
	auto p5_out_cb() { return m_port_out_cb[5].bind(); }
	auto p6_in_cb() { return m_port_in_cb[6].bind(); }
	auto p6_out_cb() { return m_port_out_cb[6].bind(); }
	auto p7_in_cb() { return m_port_in_cb[7].bind(); }
	auto p7_out_cb() { return m_port_out_cb[7].bind(); }
	auto p8_in_cb() { return m_port_in_cb[8].bind(); }
	auto p8_out_cb() { return m_port_out_cb[8].bind(); }
	auto p9_in_cb() { return m_port_in_cb[9].bind(); }
	auto p9_out_cb() { return m_port_out_cb[9].bind(); }
	auto p10_in_cb() { return m_port_in_cb[10].bind(); }
	auto p10_out_cb() { return m_port_out_cb[10].bind(); }
	auto an0_cb() { return m_analog_cb[0].bind(); }
	auto an1_cb() { return m_analog_cb[1].bind(); }
	auto an2_cb() { return m_analog_cb[2].bind(); }
	auto an3_cb() { return m_analog_cb[3].bind(); }
	auto an4_cb() { return m_analog_cb[4].bind(); }
	auto an5_cb() { return m_analog_cb[5].bind(); }
	auto an6_cb() { return m_analog_cb[6].bind(); }
	auto an7_cb() { return m_analog_cb[7].bind(); }

protected:
	void ad_register_map(address_map &map) ATTR_COLD;
	void uart0_register_map(address_map &map) ATTR_COLD;
	void uart1_register_map(address_map &map) ATTR_COLD;
	void timer_register_map(address_map &map) ATTR_COLD;
	void timer_6channel_register_map(address_map &map) ATTR_COLD;
	void irq_register_map(address_map &map) ATTR_COLD;

	// internal registers
	template <int Base> uint8_t port_r(offs_t offset);
	template <int Base> void port_w(offs_t offset, uint8_t data);
	uint8_t get_port_reg(int p);
	uint8_t get_port_dir(int p);
	void set_port_reg(int p, uint8_t data);
	void set_port_dir(int p, uint8_t data);
	void da_reg_w(offs_t offset, uint8_t data);
	void pulse_output_w(offs_t offset, uint8_t data);
	uint8_t ad_control_r();
	void ad_control_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(ad_timer_cb);
	uint8_t ad_sweep_r();
	void ad_sweep_w(uint8_t data);
	uint16_t ad_result_r(offs_t offset);
	uint8_t uart0_mode_r();
	void uart0_mode_w(uint8_t data);
	uint8_t uart1_mode_r();
	void uart1_mode_w(uint8_t data);
	void uart0_baud_w(uint8_t data);
	void uart1_baud_w(uint8_t data);
	void uart0_tbuf_w(uint16_t data);
	void uart1_tbuf_w(uint16_t data);
	uint8_t uart0_ctrl_reg0_r();
	void uart0_ctrl_reg0_w(uint8_t data);
	uint8_t uart1_ctrl_reg0_r();
	void uart1_ctrl_reg0_w(uint8_t data);
	uint8_t uart0_ctrl_reg1_r();
	void uart0_ctrl_reg1_w(uint8_t data);
	uint8_t uart1_ctrl_reg1_r();
	void uart1_ctrl_reg1_w(uint8_t data);
	uint16_t uart0_rbuf_r();
	uint16_t uart1_rbuf_r();
	uint8_t count_start_r();
	void count_start_w(uint8_t data);
	void one_shot_start_w(uint8_t data);
	uint8_t up_down_r();
	void up_down_w(uint8_t data);
	uint16_t timer_reg_r(offs_t offset, uint16_t mem_mask);
	void timer_reg_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t timer_mode_r(offs_t offset);
	void timer_mode_w(offs_t offset, uint8_t data);
	uint8_t proc_mode_r(offs_t offset);
	void proc_mode_w(uint8_t data);
	void watchdog_timer_w(uint8_t data);
	uint8_t watchdog_freq_r();
	void watchdog_freq_w(uint8_t data);
	uint8_t waveform_mode_r();
	void waveform_mode_w(uint8_t data);
	uint8_t rto_control_r();
	void rto_control_w(uint8_t data);
	uint8_t dram_control_r();
	void dram_control_w(uint8_t data);
	void refresh_timer_w(uint8_t data);
	uint16_t dmac_control_r(offs_t offset, uint16_t mem_mask);
	void dmac_control_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	template <int Level> uint8_t int_control_r();
	template <int Level> void int_control_w(uint8_t data);
	uint8_t get_int_control(int level);
	void set_int_control(int level, uint8_t data);

	// construction/destruction
	m37710_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor map_delegate);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 20; /* rough guess */ }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual std::vector<std::pair<int, const address_space_config *>> memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual bool get_m_flag() const override;
	virtual bool get_x_flag() const override;

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	// I/O port callbacks
	devcb_read8::array<11> m_port_in_cb;
	devcb_write8::array<11> m_port_out_cb;

	// A-D callbacks
	devcb_read16::array<8> m_analog_cb;

	uint32_t m_a;         /* Accumulator */
	uint32_t m_b;         /* holds high byte of accumulator */
	uint32_t m_ba;        /* Secondary Accumulator */
	uint32_t m_bb;        /* holds high byte of secondary accumulator */
	uint32_t m_x;         /* Index Register X */
	uint32_t m_y;         /* Index Register Y */
	uint32_t m_xh;        /* holds high byte of x */
	uint32_t m_yh;        /* holds high byte of y */
	uint32_t m_s;         /* Stack Pointer */
	uint32_t m_pc;        /* Program Counter */
	uint32_t m_ppc;       /* Previous Program Counter */
	uint32_t m_pg;        /* Program Bank (shifted left 16) */
	uint32_t m_dt;        /* Data Bank (shifted left 16) */
	uint32_t m_dpr;       /* Direct Page Register */
	uint32_t m_flag_e;        /* Emulation Mode Flag */
	uint32_t m_flag_m;        /* Memory/Accumulator Select Flag */
	uint32_t m_flag_x;        /* Index Select Flag */
	uint32_t m_flag_n;        /* Negative Flag */
	uint32_t m_flag_v;        /* Overflow Flag */
	uint32_t m_flag_d;        /* Decimal Mode Flag */
	uint32_t m_flag_i;        /* Interrupt Mask Flag */
	uint32_t m_flag_z;        /* Zero Flag (inverted) */
	uint32_t m_flag_c;        /* Carry Flag */
	uint32_t m_line_irq;      /* Bitmask of pending IRQs */
	uint32_t m_ipl;       /* Interrupt priority level (top of PSW) */
	uint32_t m_ir;        /* Instruction Register */
	uint32_t m_im;        /* Immediate load value */
	uint32_t m_im2;       /* Immediate load target */
	uint32_t m_im3;       /* Immediate load target */
	uint32_t m_im4;       /* Immediate load target */
	uint32_t m_irq_delay;     /* delay 1 instruction before checking irq */
	int m_ICount;     /* cycle count */
	uint32_t m_source;        /* temp register */
	uint32_t m_destination;   /* temp register */
	memory_access<24, 1, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<24, 1, 0, ENDIANNESS_LITTLE>::specific m_program;
	uint32_t m_stopped;       /* Sets how the CPU is stopped */

	// ports
	uint8_t m_port_regs[11];
	uint8_t m_port_dir[11];

	// A/D
	uint8_t m_ad_control;
	uint8_t m_ad_sweep;
	uint16_t m_ad_result[8];
	emu_timer *m_ad_timer;

	// UARTs
	uint8_t m_uart_mode[2];
	uint8_t m_uart_baud[2];
	uint8_t m_uart_ctrl_reg0[2];
	uint8_t m_uart_ctrl_reg1[2];

	// timers
	uint8_t m_count_start;
	uint8_t m_one_shot_start;
	uint8_t m_up_down_reg;
	uint16_t m_timer_reg[8];
	uint8_t m_timer_mode[8];
	attotime m_reload[8];
	emu_timer *m_timers[8];
	int m_timer_out[8];

	// misc. internal registers
	uint8_t m_proc_mode;
	uint8_t m_watchdog_freq;
	uint8_t m_rto_control;
	uint8_t m_dram_control;
	uint16_t m_dmac_control;

	// DMA
	uint32_t m_dma_src[4], m_dma_dst[4], m_dma_cnt[4], m_dma_mode[4];

	// interrupt controller
	uint8_t m_int_control[M37710_MASKABLE_INTERRUPTS];

	// for debugger
	uint32_t m_debugger_pc;
	uint32_t m_debugger_pg;
	uint32_t m_debugger_dt;
	uint32_t m_debugger_ps;
	uint32_t m_debugger_a;
	uint32_t m_debugger_b;

	// Statics
	typedef void (m37710_cpu_device::*opcode_func)();
	typedef uint32_t (m37710_cpu_device::*get_reg_func)(int regnum);
	typedef void (m37710_cpu_device::*set_reg_func)(int regnum, uint32_t val);
	typedef int  (m37710_cpu_device::*execute_func)(int cycles);

	static const int m37710_irq_vectors[M37710_INTERRUPT_MAX];
	static const char *const m37710_tnames[8];
	static const char *const m37710_intnames[M37710_INTERRUPT_MAX];
	static const opcode_func *const m37710i_opcodes[4];
	static const opcode_func *const m37710i_opcodes2[4];
	static const opcode_func *const m37710i_opcodes3[4];
	static const get_reg_func m37710i_get_reg[4];
	static const set_reg_func m37710i_set_reg[4];
	static const execute_func m37710i_execute[4];
	static const opcode_func m37710i_opcodes_M0X0[];
	static const opcode_func m37710i_opcodes_M0X1[];
	static const opcode_func m37710i_opcodes_M1X0[];
	static const opcode_func m37710i_opcodes_M1X1[];
	static const opcode_func m37710i_opcodes42_M0X0[];
	static const opcode_func m37710i_opcodes42_M0X1[];
	static const opcode_func m37710i_opcodes42_M1X0[];
	static const opcode_func m37710i_opcodes42_M1X1[];
	static const opcode_func m37710i_opcodes89_M0X0[];
	static const opcode_func m37710i_opcodes89_M0X1[];
	static const opcode_func m37710i_opcodes89_M1X0[];
	static const opcode_func m37710i_opcodes89_M1X1[];

	const opcode_func *m_opcodes;    /* opcodes with no prefix */
	const opcode_func *m_opcodes42;  /* opcodes with 0x42 prefix */
	const opcode_func *m_opcodes89;  /* opcodes with 0x89 prefix */
	get_reg_func m_get_reg;
	set_reg_func m_set_reg;
	execute_func m_execute;

	// Implementation
	void m37710i_set_execution_mode(uint32_t mode);
	TIMER_CALLBACK_MEMBER( m37710_timer_cb );
	void m37710_external_tick(int timer, int state);
	void m37710_recalc_timer(int timer);
	uint32_t m37710i_get_reg_M0X0(int regnum);
	uint32_t m37710i_get_reg_M0X1(int regnum);
	uint32_t m37710i_get_reg_M1X0(int regnum);
	uint32_t m37710i_get_reg_M1X1(int regnum);
	void m37710i_set_reg_M0X0(int regnum, uint32_t val);
	void m37710i_set_reg_M0X1(int regnum, uint32_t val);
	void m37710i_set_reg_M1X0(int regnum, uint32_t val);
	void m37710i_set_reg_M1X1(int regnum, uint32_t val);
	int m37710i_execute_M0X0(int cycles);
	int m37710i_execute_M0X1(int cycles);
	int m37710i_execute_M1X0(int cycles);
	int m37710i_execute_M1X1(int cycles);
	void m37710i_update_irqs();
	void m37710_set_pc(unsigned val);
	unsigned m37710_get_sp();
	void m37710_set_sp(unsigned val);
	unsigned m37710_get_reg(int regnum);
	void m37710_set_reg(int regnum, unsigned value);
	void m37710_set_irq_line(int line, int state);
	void m37710_restore_state();
	uint32_t m37710i_read_8_normal(uint32_t address);
	uint32_t m37710i_read_8_immediate(uint32_t address);
	uint32_t m37710i_read_8_direct(uint32_t address);
	void m37710i_write_8_normal(uint32_t address, uint32_t value);
	void m37710i_write_8_direct(uint32_t address, uint32_t value);
	uint32_t m37710i_read_16_normal(uint32_t address);
	uint32_t m37710i_read_16_immediate(uint32_t address);
	uint32_t m37710i_read_16_direct(uint32_t address);
	void m37710i_write_16_normal(uint32_t address, uint32_t value);
	void m37710i_write_16_direct(uint32_t address, uint32_t value);
	uint32_t m37710i_read_24_normal(uint32_t address);
	uint32_t m37710i_read_24_immediate(uint32_t address);
	uint32_t m37710i_read_24_direct(uint32_t address);
	void m37710i_push_8(uint32_t value);
	uint32_t m37710i_pull_8();
	void m37710i_push_16(uint32_t value);
	uint32_t m37710i_pull_16();
	void m37710i_push_24(uint32_t value);
	uint32_t m37710i_pull_24();
	void m37710i_jump_16(uint32_t address);
	void m37710i_jump_24(uint32_t address);
	void m37710i_branch_8(uint32_t offset);
	void m37710i_branch_16(uint32_t offset);
	uint32_t m37710i_get_reg_ps();
	void m37710i_set_reg_ipl(uint32_t value);
	void m37710i_interrupt_software(uint32_t vector);
	void m37710i_set_flag_m0x0(uint32_t value);
	void m37710i_set_flag_m0x1(uint32_t value);
	void m37710i_set_flag_m1x0(uint32_t value);
	void m37710i_set_flag_m1x1(uint32_t value);
	void m37710i_set_reg_ps_m0x0(uint32_t value);
	void m37710i_set_reg_ps_m0x1(uint32_t value);
	void m37710i_set_reg_ps_m1x0(uint32_t value);
	void m37710i_set_reg_ps_m1x1(uint32_t value);
	uint32_t EA_IMM8();
	uint32_t EA_IMM16();
	uint32_t EA_IMM24();
	uint32_t EA_D();
	uint32_t EA_A();
	uint32_t EA_AL();
	uint32_t EA_DX();
	uint32_t EA_DY();
	uint32_t EA_AX();
	uint32_t EA_ALX();
	uint32_t EA_AY();
	uint32_t EA_DI();
	uint32_t EA_DLI();
	uint32_t EA_AI();
	uint32_t EA_ALI();
	uint32_t EA_DXI();
	uint32_t EA_DIY();
	uint32_t EA_DLIY();
	uint32_t EA_AXI();
	uint32_t EA_S();
	uint32_t EA_SIY();
	void m37710i_00_M0X0();
	void m37710i_01_M0X0();
	void m37710i_02_M0X0();
	void m37710i_03_M0X0();
	void m37710i_04_M0X0();
	void m37710i_05_M0X0();
	void m37710i_06_M0X0();
	void m37710i_07_M0X0();
	void m37710i_08_M0X0();
	void m37710i_09_M0X0();
	void m37710i_0a_M0X0();
	void m37710i_0b_M0X0();
	void m37710i_0c_M0X0();
	void m37710i_0d_M0X0();
	void m37710i_0e_M0X0();
	void m37710i_0f_M0X0();
	void m37710i_10_M0X0();
	void m37710i_11_M0X0();
	void m37710i_12_M0X0();
	void m37710i_13_M0X0();
	void m37710i_14_M0X0();
	void m37710i_15_M0X0();
	void m37710i_16_M0X0();
	void m37710i_17_M0X0();
	void m37710i_18_M0X0();
	void m37710i_19_M0X0();
	void m37710i_1a_M0X0();
	void m37710i_1b_M0X0();
	void m37710i_1c_M0X0();
	void m37710i_1d_M0X0();
	void m37710i_1e_M0X0();
	void m37710i_1f_M0X0();
	void m37710i_20_M0X0();
	void m37710i_21_M0X0();
	void m37710i_22_M0X0();
	void m37710i_23_M0X0();
	void m37710i_24_M0X0();
	void m37710i_25_M0X0();
	void m37710i_26_M0X0();
	void m37710i_27_M0X0();
	void m37710i_28_M0X0();
	void m37710i_29_M0X0();
	void m37710i_2a_M0X0();
	void m37710i_2b_M0X0();
	void m37710i_2c_M0X0();
	void m37710i_2d_M0X0();
	void m37710i_2e_M0X0();
	void m37710i_2f_M0X0();
	void m37710i_30_M0X0();
	void m37710i_31_M0X0();
	void m37710i_32_M0X0();
	void m37710i_33_M0X0();
	void m37710i_34_M0X0();
	void m37710i_35_M0X0();
	void m37710i_36_M0X0();
	void m37710i_37_M0X0();
	void m37710i_38_M0X0();
	void m37710i_39_M0X0();
	void m37710i_3a_M0X0();
	void m37710i_3b_M0X0();
	void m37710i_3c_M0X0();
	void m37710i_3d_M0X0();
	void m37710i_3e_M0X0();
	void m37710i_3f_M0X0();
	void m37710i_40_M0X0();
	void m37710i_41_M0X0();
	void m37710i_42_M0X0();
	void m37710i_43_M0X0();
	void m37710i_44_M0X0();
	void m37710i_45_M0X0();
	void m37710i_46_M0X0();
	void m37710i_47_M0X0();
	void m37710i_48_M0X0();
	void m37710i_49_M0X0();
	void m37710i_4a_M0X0();
	void m37710i_4b_M0X0();
	void m37710i_4c_M0X0();
	void m37710i_4d_M0X0();
	void m37710i_4e_M0X0();
	void m37710i_4f_M0X0();
	void m37710i_50_M0X0();
	void m37710i_51_M0X0();
	void m37710i_52_M0X0();
	void m37710i_53_M0X0();
	void m37710i_54_M0X0();
	void m37710i_55_M0X0();
	void m37710i_56_M0X0();
	void m37710i_57_M0X0();
	void m37710i_58_M0X0();
	void m37710i_59_M0X0();
	void m37710i_5a_M0X0();
	void m37710i_5b_M0X0();
	void m37710i_5c_M0X0();
	void m37710i_5d_M0X0();
	void m37710i_5e_M0X0();
	void m37710i_5f_M0X0();
	void m37710i_60_M0X0();
	void m37710i_61_M0X0();
	void m37710i_62_M0X0();
	void m37710i_63_M0X0();
	void m37710i_64_M0X0();
	void m37710i_65_M0X0();
	void m37710i_66_M0X0();
	void m37710i_67_M0X0();
	void m37710i_68_M0X0();
	void m37710i_69_M0X0();
	void m37710i_6a_M0X0();
	void m37710i_6b_M0X0();
	void m37710i_6c_M0X0();
	void m37710i_6d_M0X0();
	void m37710i_6e_M0X0();
	void m37710i_6f_M0X0();
	void m37710i_70_M0X0();
	void m37710i_71_M0X0();
	void m37710i_72_M0X0();
	void m37710i_73_M0X0();
	void m37710i_74_M0X0();
	void m37710i_75_M0X0();
	void m37710i_76_M0X0();
	void m37710i_77_M0X0();
	void m37710i_78_M0X0();
	void m37710i_79_M0X0();
	void m37710i_7a_M0X0();
	void m37710i_7b_M0X0();
	void m37710i_7c_M0X0();
	void m37710i_7d_M0X0();
	void m37710i_7e_M0X0();
	void m37710i_7f_M0X0();
	void m37710i_80_M0X0();
	void m37710i_81_M0X0();
	void m37710i_82_M0X0();
	void m37710i_83_M0X0();
	void m37710i_84_M0X0();
	void m37710i_85_M0X0();
	void m37710i_86_M0X0();
	void m37710i_87_M0X0();
	void m37710i_88_M0X0();
	void m37710i_89_M0X0();
	void m37710i_8a_M0X0();
	void m37710i_8b_M0X0();
	void m37710i_8c_M0X0();
	void m37710i_8d_M0X0();
	void m37710i_8e_M0X0();
	void m37710i_8f_M0X0();
	void m37710i_90_M0X0();
	void m37710i_91_M0X0();
	void m37710i_92_M0X0();
	void m37710i_93_M0X0();
	void m37710i_94_M0X0();
	void m37710i_95_M0X0();
	void m37710i_96_M0X0();
	void m37710i_97_M0X0();
	void m37710i_98_M0X0();
	void m37710i_99_M0X0();
	void m37710i_9a_M0X0();
	void m37710i_9b_M0X0();
	void m37710i_9c_M0X0();
	void m37710i_9d_M0X0();
	void m37710i_9e_M0X0();
	void m37710i_9f_M0X0();
	void m37710i_a0_M0X0();
	void m37710i_a1_M0X0();
	void m37710i_a2_M0X0();
	void m37710i_a3_M0X0();
	void m37710i_a4_M0X0();
	void m37710i_a5_M0X0();
	void m37710i_a6_M0X0();
	void m37710i_a7_M0X0();
	void m37710i_a8_M0X0();
	void m37710i_a9_M0X0();
	void m37710i_aa_M0X0();
	void m37710i_ab_M0X0();
	void m37710i_ac_M0X0();
	void m37710i_ad_M0X0();
	void m37710i_ae_M0X0();
	void m37710i_af_M0X0();
	void m37710i_b0_M0X0();
	void m37710i_b1_M0X0();
	void m37710i_b2_M0X0();
	void m37710i_b3_M0X0();
	void m37710i_b4_M0X0();
	void m37710i_b5_M0X0();
	void m37710i_b6_M0X0();
	void m37710i_b7_M0X0();
	void m37710i_b8_M0X0();
	void m37710i_b9_M0X0();
	void m37710i_ba_M0X0();
	void m37710i_bb_M0X0();
	void m37710i_bc_M0X0();
	void m37710i_bd_M0X0();
	void m37710i_be_M0X0();
	void m37710i_bf_M0X0();
	void m37710i_c0_M0X0();
	void m37710i_c1_M0X0();
	void m37710i_c2_M0X0();
	void m37710i_c3_M0X0();
	void m37710i_c4_M0X0();
	void m37710i_c5_M0X0();
	void m37710i_c6_M0X0();
	void m37710i_c7_M0X0();
	void m37710i_c8_M0X0();
	void m37710i_c9_M0X0();
	void m37710i_ca_M0X0();
	void m37710i_cb_M0X0();
	void m37710i_cc_M0X0();
	void m37710i_cd_M0X0();
	void m37710i_ce_M0X0();
	void m37710i_cf_M0X0();
	void m37710i_d0_M0X0();
	void m37710i_d1_M0X0();
	void m37710i_d2_M0X0();
	void m37710i_d3_M0X0();
	void m37710i_d4_M0X0();
	void m37710i_d5_M0X0();
	void m37710i_d6_M0X0();
	void m37710i_d7_M0X0();
	void m37710i_d8_M0X0();
	void m37710i_d9_M0X0();
	void m37710i_da_M0X0();
	void m37710i_db_M0X0();
	void m37710i_dc_M0X0();
	void m37710i_dd_M0X0();
	void m37710i_de_M0X0();
	void m37710i_df_M0X0();
	void m37710i_e0_M0X0();
	void m37710i_e1_M0X0();
	void m37710i_e2_M0X0();
	void m37710i_e3_M0X0();
	void m37710i_e4_M0X0();
	void m37710i_e5_M0X0();
	void m37710i_e6_M0X0();
	void m37710i_e7_M0X0();
	void m37710i_e8_M0X0();
	void m37710i_e9_M0X0();
	void m37710i_ea_M0X0();
	void m37710i_eb_M0X0();
	void m37710i_ec_M0X0();
	void m37710i_ed_M0X0();
	void m37710i_ee_M0X0();
	void m37710i_ef_M0X0();
	void m37710i_f0_M0X0();
	void m37710i_f1_M0X0();
	void m37710i_f2_M0X0();
	void m37710i_f3_M0X0();
	void m37710i_f4_M0X0();
	void m37710i_f5_M0X0();
	void m37710i_f6_M0X0();
	void m37710i_f7_M0X0();
	void m37710i_f8_M0X0();
	void m37710i_f9_M0X0();
	void m37710i_fa_M0X0();
	void m37710i_fb_M0X0();
	void m37710i_fc_M0X0();
	void m37710i_fd_M0X0();
	void m37710i_fe_M0X0();
	void m37710i_ff_M0X0();
	void m37710i_101_M0X0();
	void m37710i_103_M0X0();
	void m37710i_105_M0X0();
	void m37710i_107_M0X0();
	void m37710i_109_M0X0();
	void m37710i_10a_M0X0();
	void m37710i_10d_M0X0();
	void m37710i_10f_M0X0();
	void m37710i_111_M0X0();
	void m37710i_112_M0X0();
	void m37710i_113_M0X0();
	void m37710i_115_M0X0();
	void m37710i_117_M0X0();
	void m37710i_119_M0X0();
	void m37710i_11a_M0X0();
	void m37710i_11b_M0X0();
	void m37710i_11d_M0X0();
	void m37710i_11f_M0X0();
	void m37710i_121_M0X0();
	void m37710i_123_M0X0();
	void m37710i_125_M0X0();
	void m37710i_127_M0X0();
	void m37710i_129_M0X0();
	void m37710i_12a_M0X0();
	void m37710i_12d_M0X0();
	void m37710i_12f_M0X0();
	void m37710i_131_M0X0();
	void m37710i_132_M0X0();
	void m37710i_133_M0X0();
	void m37710i_135_M0X0();
	void m37710i_137_M0X0();
	void m37710i_139_M0X0();
	void m37710i_13a_M0X0();
	void m37710i_13b_M0X0();
	void m37710i_13d_M0X0();
	void m37710i_13f_M0X0();
	void m37710i_141_M0X0();
	void m37710i_143_M0X0();
	void m37710i_145_M0X0();
	void m37710i_147_M0X0();
	void m37710i_148_M0X0();
	void m37710i_149_M0X0();
	void m37710i_14a_M0X0();
	void m37710i_14d_M0X0();
	void m37710i_14f_M0X0();
	void m37710i_151_M0X0();
	void m37710i_152_M0X0();
	void m37710i_153_M0X0();
	void m37710i_155_M0X0();
	void m37710i_157_M0X0();
	void m37710i_159_M0X0();
	void m37710i_15b_M0X0();
	void m37710i_15d_M0X0();
	void m37710i_15f_M0X0();
	void m37710i_161_M0X0();
	void m37710i_163_M0X0();
	void m37710i_165_M0X0();
	void m37710i_167_M0X0();
	void m37710i_168_M0X0();
	void m37710i_169_M0X0();
	void m37710i_16a_M0X0();
	void m37710i_16d_M0X0();
	void m37710i_16f_M0X0();
	void m37710i_171_M0X0();
	void m37710i_172_M0X0();
	void m37710i_173_M0X0();
	void m37710i_175_M0X0();
	void m37710i_177_M0X0();
	void m37710i_179_M0X0();
	void m37710i_17b_M0X0();
	void m37710i_17d_M0X0();
	void m37710i_17f_M0X0();
	void m37710i_181_M0X0();
	void m37710i_183_M0X0();
	void m37710i_185_M0X0();
	void m37710i_187_M0X0();
	void m37710i_18a_M0X0();
	void m37710i_18d_M0X0();
	void m37710i_18f_M0X0();
	void m37710i_191_M0X0();
	void m37710i_192_M0X0();
	void m37710i_193_M0X0();
	void m37710i_195_M0X0();
	void m37710i_197_M0X0();
	void m37710i_198_M0X0();
	void m37710i_199_M0X0();
	void m37710i_19d_M0X0();
	void m37710i_19f_M0X0();
	void m37710i_1a1_M0X0();
	void m37710i_1a3_M0X0();
	void m37710i_1a5_M0X0();
	void m37710i_1a7_M0X0();
	void m37710i_1a8_M0X0();
	void m37710i_1a9_M0X0();
	void m37710i_1aa_M0X0();
	void m37710i_1ad_M0X0();
	void m37710i_1af_M0X0();
	void m37710i_1b1_M0X0();
	void m37710i_1b2_M0X0();
	void m37710i_1b3_M0X0();
	void m37710i_1b5_M0X0();
	void m37710i_1b7_M0X0();
	void m37710i_1b9_M0X0();
	void m37710i_1bd_M0X0();
	void m37710i_1bf_M0X0();
	void m37710i_1c1_M0X0();
	void m37710i_1c3_M0X0();
	void m37710i_1c5_M0X0();
	void m37710i_1c7_M0X0();
	void m37710i_1c9_M0X0();
	void m37710i_1cd_M0X0();
	void m37710i_1cf_M0X0();
	void m37710i_1d1_M0X0();
	void m37710i_1d2_M0X0();
	void m37710i_1d3_M0X0();
	void m37710i_1d5_M0X0();
	void m37710i_1d7_M0X0();
	void m37710i_1d9_M0X0();
	void m37710i_1dd_M0X0();
	void m37710i_1df_M0X0();
	void m37710i_1e1_M0X0();
	void m37710i_1e3_M0X0();
	void m37710i_1e5_M0X0();
	void m37710i_1e7_M0X0();
	void m37710i_1e9_M0X0();
	void m37710i_1ed_M0X0();
	void m37710i_1ef_M0X0();
	void m37710i_1f1_M0X0();
	void m37710i_1f2_M0X0();
	void m37710i_1f3_M0X0();
	void m37710i_1f5_M0X0();
	void m37710i_1f7_M0X0();
	void m37710i_1f9_M0X0();
	void m37710i_1fd_M0X0();
	void m37710i_1ff_M0X0();
	void m37710i_200_M0X0();
	void m37710i_201_M0X0();
	void m37710i_203_M0X0();
	void m37710i_205_M0X0();
	void m37710i_207_M0X0();
	void m37710i_209_M0X0();
	void m37710i_20d_M0X0();
	void m37710i_20f_M0X0();
	void m37710i_211_M0X0();
	void m37710i_212_M0X0();
	void m37710i_213_M0X0();
	void m37710i_215_M0X0();
	void m37710i_217_M0X0();
	void m37710i_219_M0X0();
	void m37710i_21d_M0X0();
	void m37710i_21f_M0X0();
	void m37710i_221_M0X0();
	void m37710i_223_M0X0();
	void m37710i_225_M0X0();
	void m37710i_227_M0X0();
	void m37710i_228_M0X0();
	void m37710i_229_M0X0();
	void m37710i_22d_M0X0();
	void m37710i_22f_M0X0();
	void m37710i_231_M0X0();
	void m37710i_232_M0X0();
	void m37710i_233_M0X0();
	void m37710i_235_M0X0();
	void m37710i_237_M0X0();
	void m37710i_239_M0X0();
	void m37710i_23d_M0X0();
	void m37710i_23f_M0X0();
	void m37710i_249_M0X0();
	void m37710i_2c2_M0X0();
	void m37710i_00_M0X1();
	void m37710i_01_M0X1();
	void m37710i_02_M0X1();
	void m37710i_03_M0X1();
	void m37710i_04_M0X1();
	void m37710i_05_M0X1();
	void m37710i_06_M0X1();
	void m37710i_07_M0X1();
	void m37710i_08_M0X1();
	void m37710i_09_M0X1();
	void m37710i_0a_M0X1();
	void m37710i_0b_M0X1();
	void m37710i_0c_M0X1();
	void m37710i_0d_M0X1();
	void m37710i_0e_M0X1();
	void m37710i_0f_M0X1();
	void m37710i_10_M0X1();
	void m37710i_11_M0X1();
	void m37710i_12_M0X1();
	void m37710i_13_M0X1();
	void m37710i_14_M0X1();
	void m37710i_15_M0X1();
	void m37710i_16_M0X1();
	void m37710i_17_M0X1();
	void m37710i_18_M0X1();
	void m37710i_19_M0X1();
	void m37710i_1a_M0X1();
	void m37710i_1b_M0X1();
	void m37710i_1c_M0X1();
	void m37710i_1d_M0X1();
	void m37710i_1e_M0X1();
	void m37710i_1f_M0X1();
	void m37710i_20_M0X1();
	void m37710i_21_M0X1();
	void m37710i_22_M0X1();
	void m37710i_23_M0X1();
	void m37710i_24_M0X1();
	void m37710i_25_M0X1();
	void m37710i_26_M0X1();
	void m37710i_27_M0X1();
	void m37710i_28_M0X1();
	void m37710i_29_M0X1();
	void m37710i_2a_M0X1();
	void m37710i_2b_M0X1();
	void m37710i_2c_M0X1();
	void m37710i_2d_M0X1();
	void m37710i_2e_M0X1();
	void m37710i_2f_M0X1();
	void m37710i_30_M0X1();
	void m37710i_31_M0X1();
	void m37710i_32_M0X1();
	void m37710i_33_M0X1();
	void m37710i_34_M0X1();
	void m37710i_35_M0X1();
	void m37710i_36_M0X1();
	void m37710i_37_M0X1();
	void m37710i_38_M0X1();
	void m37710i_39_M0X1();
	void m37710i_3a_M0X1();
	void m37710i_3b_M0X1();
	void m37710i_3c_M0X1();
	void m37710i_3d_M0X1();
	void m37710i_3e_M0X1();
	void m37710i_3f_M0X1();
	void m37710i_40_M0X1();
	void m37710i_41_M0X1();
	void m37710i_42_M0X1();
	void m37710i_43_M0X1();
	void m37710i_44_M0X1();
	void m37710i_45_M0X1();
	void m37710i_46_M0X1();
	void m37710i_47_M0X1();
	void m37710i_48_M0X1();
	void m37710i_49_M0X1();
	void m37710i_4a_M0X1();
	void m37710i_4b_M0X1();
	void m37710i_4c_M0X1();
	void m37710i_4d_M0X1();
	void m37710i_4e_M0X1();
	void m37710i_4f_M0X1();
	void m37710i_50_M0X1();
	void m37710i_51_M0X1();
	void m37710i_52_M0X1();
	void m37710i_53_M0X1();
	void m37710i_54_M0X1();
	void m37710i_55_M0X1();
	void m37710i_56_M0X1();
	void m37710i_57_M0X1();
	void m37710i_58_M0X1();
	void m37710i_59_M0X1();
	void m37710i_5a_M0X1();
	void m37710i_5b_M0X1();
	void m37710i_5c_M0X1();
	void m37710i_5d_M0X1();
	void m37710i_5e_M0X1();
	void m37710i_5f_M0X1();
	void m37710i_60_M0X1();
	void m37710i_61_M0X1();
	void m37710i_62_M0X1();
	void m37710i_63_M0X1();
	void m37710i_64_M0X1();
	void m37710i_65_M0X1();
	void m37710i_66_M0X1();
	void m37710i_67_M0X1();
	void m37710i_68_M0X1();
	void m37710i_69_M0X1();
	void m37710i_6a_M0X1();
	void m37710i_6b_M0X1();
	void m37710i_6c_M0X1();
	void m37710i_6d_M0X1();
	void m37710i_6e_M0X1();
	void m37710i_6f_M0X1();
	void m37710i_70_M0X1();
	void m37710i_71_M0X1();
	void m37710i_72_M0X1();
	void m37710i_73_M0X1();
	void m37710i_74_M0X1();
	void m37710i_75_M0X1();
	void m37710i_76_M0X1();
	void m37710i_77_M0X1();
	void m37710i_78_M0X1();
	void m37710i_79_M0X1();
	void m37710i_7a_M0X1();
	void m37710i_7b_M0X1();
	void m37710i_7c_M0X1();
	void m37710i_7d_M0X1();
	void m37710i_7e_M0X1();
	void m37710i_7f_M0X1();
	void m37710i_80_M0X1();
	void m37710i_81_M0X1();
	void m37710i_82_M0X1();
	void m37710i_83_M0X1();
	void m37710i_84_M0X1();
	void m37710i_85_M0X1();
	void m37710i_86_M0X1();
	void m37710i_87_M0X1();
	void m37710i_88_M0X1();
	void m37710i_89_M0X1();
	void m37710i_8a_M0X1();
	void m37710i_8b_M0X1();
	void m37710i_8c_M0X1();
	void m37710i_8d_M0X1();
	void m37710i_8e_M0X1();
	void m37710i_8f_M0X1();
	void m37710i_90_M0X1();
	void m37710i_91_M0X1();
	void m37710i_92_M0X1();
	void m37710i_93_M0X1();
	void m37710i_94_M0X1();
	void m37710i_95_M0X1();
	void m37710i_96_M0X1();
	void m37710i_97_M0X1();
	void m37710i_98_M0X1();
	void m37710i_99_M0X1();
	void m37710i_9a_M0X1();
	void m37710i_9b_M0X1();
	void m37710i_9c_M0X1();
	void m37710i_9d_M0X1();
	void m37710i_9e_M0X1();
	void m37710i_9f_M0X1();
	void m37710i_a0_M0X1();
	void m37710i_a1_M0X1();
	void m37710i_a2_M0X1();
	void m37710i_a3_M0X1();
	void m37710i_a4_M0X1();
	void m37710i_a5_M0X1();
	void m37710i_a6_M0X1();
	void m37710i_a7_M0X1();
	void m37710i_a8_M0X1();
	void m37710i_a9_M0X1();
	void m37710i_aa_M0X1();
	void m37710i_ab_M0X1();
	void m37710i_ac_M0X1();
	void m37710i_ad_M0X1();
	void m37710i_ae_M0X1();
	void m37710i_af_M0X1();
	void m37710i_b0_M0X1();
	void m37710i_b1_M0X1();
	void m37710i_b2_M0X1();
	void m37710i_b3_M0X1();
	void m37710i_b4_M0X1();
	void m37710i_b5_M0X1();
	void m37710i_b6_M0X1();
	void m37710i_b7_M0X1();
	void m37710i_b8_M0X1();
	void m37710i_b9_M0X1();
	void m37710i_ba_M0X1();
	void m37710i_bb_M0X1();
	void m37710i_bc_M0X1();
	void m37710i_bd_M0X1();
	void m37710i_be_M0X1();
	void m37710i_bf_M0X1();
	void m37710i_c0_M0X1();
	void m37710i_c1_M0X1();
	void m37710i_c2_M0X1();
	void m37710i_c3_M0X1();
	void m37710i_c4_M0X1();
	void m37710i_c5_M0X1();
	void m37710i_c6_M0X1();
	void m37710i_c7_M0X1();
	void m37710i_c8_M0X1();
	void m37710i_c9_M0X1();
	void m37710i_ca_M0X1();
	void m37710i_cb_M0X1();
	void m37710i_cc_M0X1();
	void m37710i_cd_M0X1();
	void m37710i_ce_M0X1();
	void m37710i_cf_M0X1();
	void m37710i_d0_M0X1();
	void m37710i_d1_M0X1();
	void m37710i_d2_M0X1();
	void m37710i_d3_M0X1();
	void m37710i_d4_M0X1();
	void m37710i_d5_M0X1();
	void m37710i_d6_M0X1();
	void m37710i_d7_M0X1();
	void m37710i_d8_M0X1();
	void m37710i_d9_M0X1();
	void m37710i_da_M0X1();
	void m37710i_db_M0X1();
	void m37710i_dc_M0X1();
	void m37710i_dd_M0X1();
	void m37710i_de_M0X1();
	void m37710i_df_M0X1();
	void m37710i_e0_M0X1();
	void m37710i_e1_M0X1();
	void m37710i_e2_M0X1();
	void m37710i_e3_M0X1();
	void m37710i_e4_M0X1();
	void m37710i_e5_M0X1();
	void m37710i_e6_M0X1();
	void m37710i_e7_M0X1();
	void m37710i_e8_M0X1();
	void m37710i_e9_M0X1();
	void m37710i_ea_M0X1();
	void m37710i_eb_M0X1();
	void m37710i_ec_M0X1();
	void m37710i_ed_M0X1();
	void m37710i_ee_M0X1();
	void m37710i_ef_M0X1();
	void m37710i_f0_M0X1();
	void m37710i_f1_M0X1();
	void m37710i_f2_M0X1();
	void m37710i_f3_M0X1();
	void m37710i_f4_M0X1();
	void m37710i_f5_M0X1();
	void m37710i_f6_M0X1();
	void m37710i_f7_M0X1();
	void m37710i_f8_M0X1();
	void m37710i_f9_M0X1();
	void m37710i_fa_M0X1();
	void m37710i_fb_M0X1();
	void m37710i_fc_M0X1();
	void m37710i_fd_M0X1();
	void m37710i_fe_M0X1();
	void m37710i_ff_M0X1();
	void m37710i_101_M0X1();
	void m37710i_103_M0X1();
	void m37710i_105_M0X1();
	void m37710i_107_M0X1();
	void m37710i_109_M0X1();
	void m37710i_10a_M0X1();
	void m37710i_10d_M0X1();
	void m37710i_10f_M0X1();
	void m37710i_111_M0X1();
	void m37710i_112_M0X1();
	void m37710i_113_M0X1();
	void m37710i_115_M0X1();
	void m37710i_117_M0X1();
	void m37710i_119_M0X1();
	void m37710i_11a_M0X1();
	void m37710i_11b_M0X1();
	void m37710i_11d_M0X1();
	void m37710i_11f_M0X1();
	void m37710i_121_M0X1();
	void m37710i_123_M0X1();
	void m37710i_125_M0X1();
	void m37710i_127_M0X1();
	void m37710i_129_M0X1();
	void m37710i_12a_M0X1();
	void m37710i_12d_M0X1();
	void m37710i_12f_M0X1();
	void m37710i_131_M0X1();
	void m37710i_132_M0X1();
	void m37710i_133_M0X1();
	void m37710i_135_M0X1();
	void m37710i_137_M0X1();
	void m37710i_139_M0X1();
	void m37710i_13a_M0X1();
	void m37710i_13b_M0X1();
	void m37710i_13d_M0X1();
	void m37710i_13f_M0X1();
	void m37710i_141_M0X1();
	void m37710i_143_M0X1();
	void m37710i_145_M0X1();
	void m37710i_147_M0X1();
	void m37710i_148_M0X1();
	void m37710i_149_M0X1();
	void m37710i_14a_M0X1();
	void m37710i_14d_M0X1();
	void m37710i_14f_M0X1();
	void m37710i_151_M0X1();
	void m37710i_152_M0X1();
	void m37710i_153_M0X1();
	void m37710i_155_M0X1();
	void m37710i_157_M0X1();
	void m37710i_159_M0X1();
	void m37710i_15b_M0X1();
	void m37710i_15d_M0X1();
	void m37710i_15f_M0X1();
	void m37710i_161_M0X1();
	void m37710i_163_M0X1();
	void m37710i_165_M0X1();
	void m37710i_167_M0X1();
	void m37710i_168_M0X1();
	void m37710i_169_M0X1();
	void m37710i_16a_M0X1();
	void m37710i_16d_M0X1();
	void m37710i_16f_M0X1();
	void m37710i_171_M0X1();
	void m37710i_172_M0X1();
	void m37710i_173_M0X1();
	void m37710i_175_M0X1();
	void m37710i_177_M0X1();
	void m37710i_179_M0X1();
	void m37710i_17b_M0X1();
	void m37710i_17d_M0X1();
	void m37710i_17f_M0X1();
	void m37710i_181_M0X1();
	void m37710i_183_M0X1();
	void m37710i_185_M0X1();
	void m37710i_187_M0X1();
	void m37710i_18a_M0X1();
	void m37710i_18d_M0X1();
	void m37710i_18f_M0X1();
	void m37710i_191_M0X1();
	void m37710i_192_M0X1();
	void m37710i_193_M0X1();
	void m37710i_195_M0X1();
	void m37710i_197_M0X1();
	void m37710i_198_M0X1();
	void m37710i_199_M0X1();
	void m37710i_19d_M0X1();
	void m37710i_19f_M0X1();
	void m37710i_1a1_M0X1();
	void m37710i_1a3_M0X1();
	void m37710i_1a5_M0X1();
	void m37710i_1a7_M0X1();
	void m37710i_1a8_M0X1();
	void m37710i_1a9_M0X1();
	void m37710i_1aa_M0X1();
	void m37710i_1ad_M0X1();
	void m37710i_1af_M0X1();
	void m37710i_1b1_M0X1();
	void m37710i_1b2_M0X1();
	void m37710i_1b3_M0X1();
	void m37710i_1b5_M0X1();
	void m37710i_1b7_M0X1();
	void m37710i_1b9_M0X1();
	void m37710i_1bd_M0X1();
	void m37710i_1bf_M0X1();
	void m37710i_1c1_M0X1();
	void m37710i_1c3_M0X1();
	void m37710i_1c5_M0X1();
	void m37710i_1c7_M0X1();
	void m37710i_1c9_M0X1();
	void m37710i_1cd_M0X1();
	void m37710i_1cf_M0X1();
	void m37710i_1d1_M0X1();
	void m37710i_1d2_M0X1();
	void m37710i_1d3_M0X1();
	void m37710i_1d5_M0X1();
	void m37710i_1d7_M0X1();
	void m37710i_1d9_M0X1();
	void m37710i_1dd_M0X1();
	void m37710i_1df_M0X1();
	void m37710i_1e1_M0X1();
	void m37710i_1e3_M0X1();
	void m37710i_1e5_M0X1();
	void m37710i_1e7_M0X1();
	void m37710i_1e9_M0X1();
	void m37710i_1ed_M0X1();
	void m37710i_1ef_M0X1();
	void m37710i_1f1_M0X1();
	void m37710i_1f2_M0X1();
	void m37710i_1f3_M0X1();
	void m37710i_1f5_M0X1();
	void m37710i_1f7_M0X1();
	void m37710i_1f9_M0X1();
	void m37710i_1fd_M0X1();
	void m37710i_1ff_M0X1();
	void m37710i_200_M0X1();
	void m37710i_201_M0X1();
	void m37710i_203_M0X1();
	void m37710i_205_M0X1();
	void m37710i_207_M0X1();
	void m37710i_209_M0X1();
	void m37710i_20d_M0X1();
	void m37710i_20f_M0X1();
	void m37710i_211_M0X1();
	void m37710i_212_M0X1();
	void m37710i_213_M0X1();
	void m37710i_215_M0X1();
	void m37710i_217_M0X1();
	void m37710i_219_M0X1();
	void m37710i_21d_M0X1();
	void m37710i_21f_M0X1();
	void m37710i_221_M0X1();
	void m37710i_223_M0X1();
	void m37710i_225_M0X1();
	void m37710i_227_M0X1();
	void m37710i_228_M0X1();
	void m37710i_229_M0X1();
	void m37710i_22d_M0X1();
	void m37710i_22f_M0X1();
	void m37710i_231_M0X1();
	void m37710i_232_M0X1();
	void m37710i_233_M0X1();
	void m37710i_235_M0X1();
	void m37710i_237_M0X1();
	void m37710i_239_M0X1();
	void m37710i_23d_M0X1();
	void m37710i_23f_M0X1();
	void m37710i_249_M0X1();
	void m37710i_2c2_M0X1();
	void m37710i_00_M1X0();
	void m37710i_01_M1X0();
	void m37710i_02_M1X0();
	void m37710i_03_M1X0();
	void m37710i_04_M1X0();
	void m37710i_05_M1X0();
	void m37710i_06_M1X0();
	void m37710i_07_M1X0();
	void m37710i_08_M1X0();
	void m37710i_09_M1X0();
	void m37710i_0a_M1X0();
	void m37710i_0b_M1X0();
	void m37710i_0c_M1X0();
	void m37710i_0d_M1X0();
	void m37710i_0e_M1X0();
	void m37710i_0f_M1X0();
	void m37710i_10_M1X0();
	void m37710i_11_M1X0();
	void m37710i_12_M1X0();
	void m37710i_13_M1X0();
	void m37710i_14_M1X0();
	void m37710i_15_M1X0();
	void m37710i_16_M1X0();
	void m37710i_17_M1X0();
	void m37710i_18_M1X0();
	void m37710i_19_M1X0();
	void m37710i_1a_M1X0();
	void m37710i_1b_M1X0();
	void m37710i_1c_M1X0();
	void m37710i_1d_M1X0();
	void m37710i_1e_M1X0();
	void m37710i_1f_M1X0();
	void m37710i_20_M1X0();
	void m37710i_21_M1X0();
	void m37710i_22_M1X0();
	void m37710i_23_M1X0();
	void m37710i_24_M1X0();
	void m37710i_25_M1X0();
	void m37710i_26_M1X0();
	void m37710i_27_M1X0();
	void m37710i_28_M1X0();
	void m37710i_29_M1X0();
	void m37710i_2a_M1X0();
	void m37710i_2b_M1X0();
	void m37710i_2c_M1X0();
	void m37710i_2d_M1X0();
	void m37710i_2e_M1X0();
	void m37710i_2f_M1X0();
	void m37710i_30_M1X0();
	void m37710i_31_M1X0();
	void m37710i_32_M1X0();
	void m37710i_33_M1X0();
	void m37710i_34_M1X0();
	void m37710i_35_M1X0();
	void m37710i_36_M1X0();
	void m37710i_37_M1X0();
	void m37710i_38_M1X0();
	void m37710i_39_M1X0();
	void m37710i_3a_M1X0();
	void m37710i_3b_M1X0();
	void m37710i_3c_M1X0();
	void m37710i_3d_M1X0();
	void m37710i_3e_M1X0();
	void m37710i_3f_M1X0();
	void m37710i_40_M1X0();
	void m37710i_41_M1X0();
	void m37710i_42_M1X0();
	void m37710i_43_M1X0();
	void m37710i_44_M1X0();
	void m37710i_45_M1X0();
	void m37710i_46_M1X0();
	void m37710i_47_M1X0();
	void m37710i_48_M1X0();
	void m37710i_49_M1X0();
	void m37710i_4a_M1X0();
	void m37710i_4b_M1X0();
	void m37710i_4c_M1X0();
	void m37710i_4d_M1X0();
	void m37710i_4e_M1X0();
	void m37710i_4f_M1X0();
	void m37710i_50_M1X0();
	void m37710i_51_M1X0();
	void m37710i_52_M1X0();
	void m37710i_53_M1X0();
	void m37710i_54_M1X0();
	void m37710i_55_M1X0();
	void m37710i_56_M1X0();
	void m37710i_57_M1X0();
	void m37710i_58_M1X0();
	void m37710i_59_M1X0();
	void m37710i_5a_M1X0();
	void m37710i_5b_M1X0();
	void m37710i_5c_M1X0();
	void m37710i_5d_M1X0();
	void m37710i_5e_M1X0();
	void m37710i_5f_M1X0();
	void m37710i_60_M1X0();
	void m37710i_61_M1X0();
	void m37710i_62_M1X0();
	void m37710i_63_M1X0();
	void m37710i_64_M1X0();
	void m37710i_65_M1X0();
	void m37710i_66_M1X0();
	void m37710i_67_M1X0();
	void m37710i_68_M1X0();
	void m37710i_69_M1X0();
	void m37710i_6a_M1X0();
	void m37710i_6b_M1X0();
	void m37710i_6c_M1X0();
	void m37710i_6d_M1X0();
	void m37710i_6e_M1X0();
	void m37710i_6f_M1X0();
	void m37710i_70_M1X0();
	void m37710i_71_M1X0();
	void m37710i_72_M1X0();
	void m37710i_73_M1X0();
	void m37710i_74_M1X0();
	void m37710i_75_M1X0();
	void m37710i_76_M1X0();
	void m37710i_77_M1X0();
	void m37710i_78_M1X0();
	void m37710i_79_M1X0();
	void m37710i_7a_M1X0();
	void m37710i_7b_M1X0();
	void m37710i_7c_M1X0();
	void m37710i_7d_M1X0();
	void m37710i_7e_M1X0();
	void m37710i_7f_M1X0();
	void m37710i_80_M1X0();
	void m37710i_81_M1X0();
	void m37710i_82_M1X0();
	void m37710i_83_M1X0();
	void m37710i_84_M1X0();
	void m37710i_85_M1X0();
	void m37710i_86_M1X0();
	void m37710i_87_M1X0();
	void m37710i_88_M1X0();
	void m37710i_89_M1X0();
	void m37710i_8a_M1X0();
	void m37710i_8b_M1X0();
	void m37710i_8c_M1X0();
	void m37710i_8d_M1X0();
	void m37710i_8e_M1X0();
	void m37710i_8f_M1X0();
	void m37710i_90_M1X0();
	void m37710i_91_M1X0();
	void m37710i_92_M1X0();
	void m37710i_93_M1X0();
	void m37710i_94_M1X0();
	void m37710i_95_M1X0();
	void m37710i_96_M1X0();
	void m37710i_97_M1X0();
	void m37710i_98_M1X0();
	void m37710i_99_M1X0();
	void m37710i_9a_M1X0();
	void m37710i_9b_M1X0();
	void m37710i_9c_M1X0();
	void m37710i_9d_M1X0();
	void m37710i_9e_M1X0();
	void m37710i_9f_M1X0();
	void m37710i_a0_M1X0();
	void m37710i_a1_M1X0();
	void m37710i_a2_M1X0();
	void m37710i_a3_M1X0();
	void m37710i_a4_M1X0();
	void m37710i_a5_M1X0();
	void m37710i_a6_M1X0();
	void m37710i_a7_M1X0();
	void m37710i_a8_M1X0();
	void m37710i_a9_M1X0();
	void m37710i_aa_M1X0();
	void m37710i_ab_M1X0();
	void m37710i_ac_M1X0();
	void m37710i_ad_M1X0();
	void m37710i_ae_M1X0();
	void m37710i_af_M1X0();
	void m37710i_b0_M1X0();
	void m37710i_b1_M1X0();
	void m37710i_b2_M1X0();
	void m37710i_b3_M1X0();
	void m37710i_b4_M1X0();
	void m37710i_b5_M1X0();
	void m37710i_b6_M1X0();
	void m37710i_b7_M1X0();
	void m37710i_b8_M1X0();
	void m37710i_b9_M1X0();
	void m37710i_ba_M1X0();
	void m37710i_bb_M1X0();
	void m37710i_bc_M1X0();
	void m37710i_bd_M1X0();
	void m37710i_be_M1X0();
	void m37710i_bf_M1X0();
	void m37710i_c0_M1X0();
	void m37710i_c1_M1X0();
	void m37710i_c2_M1X0();
	void m37710i_c3_M1X0();
	void m37710i_c4_M1X0();
	void m37710i_c5_M1X0();
	void m37710i_c6_M1X0();
	void m37710i_c7_M1X0();
	void m37710i_c8_M1X0();
	void m37710i_c9_M1X0();
	void m37710i_ca_M1X0();
	void m37710i_cb_M1X0();
	void m37710i_cc_M1X0();
	void m37710i_cd_M1X0();
	void m37710i_ce_M1X0();
	void m37710i_cf_M1X0();
	void m37710i_d0_M1X0();
	void m37710i_d1_M1X0();
	void m37710i_d2_M1X0();
	void m37710i_d3_M1X0();
	void m37710i_d4_M1X0();
	void m37710i_d5_M1X0();
	void m37710i_d6_M1X0();
	void m37710i_d7_M1X0();
	void m37710i_d8_M1X0();
	void m37710i_d9_M1X0();
	void m37710i_da_M1X0();
	void m37710i_db_M1X0();
	void m37710i_dc_M1X0();
	void m37710i_dd_M1X0();
	void m37710i_de_M1X0();
	void m37710i_df_M1X0();
	void m37710i_e0_M1X0();
	void m37710i_e1_M1X0();
	void m37710i_e2_M1X0();
	void m37710i_e3_M1X0();
	void m37710i_e4_M1X0();
	void m37710i_e5_M1X0();
	void m37710i_e6_M1X0();
	void m37710i_e7_M1X0();
	void m37710i_e8_M1X0();
	void m37710i_e9_M1X0();
	void m37710i_ea_M1X0();
	void m37710i_eb_M1X0();
	void m37710i_ec_M1X0();
	void m37710i_ed_M1X0();
	void m37710i_ee_M1X0();
	void m37710i_ef_M1X0();
	void m37710i_f0_M1X0();
	void m37710i_f1_M1X0();
	void m37710i_f2_M1X0();
	void m37710i_f3_M1X0();
	void m37710i_f4_M1X0();
	void m37710i_f5_M1X0();
	void m37710i_f6_M1X0();
	void m37710i_f7_M1X0();
	void m37710i_f8_M1X0();
	void m37710i_f9_M1X0();
	void m37710i_fa_M1X0();
	void m37710i_fb_M1X0();
	void m37710i_fc_M1X0();
	void m37710i_fd_M1X0();
	void m37710i_fe_M1X0();
	void m37710i_ff_M1X0();
	void m37710i_101_M1X0();
	void m37710i_103_M1X0();
	void m37710i_105_M1X0();
	void m37710i_107_M1X0();
	void m37710i_109_M1X0();
	void m37710i_10a_M1X0();
	void m37710i_10d_M1X0();
	void m37710i_10f_M1X0();
	void m37710i_111_M1X0();
	void m37710i_112_M1X0();
	void m37710i_113_M1X0();
	void m37710i_115_M1X0();
	void m37710i_117_M1X0();
	void m37710i_119_M1X0();
	void m37710i_11a_M1X0();
	void m37710i_11b_M1X0();
	void m37710i_11d_M1X0();
	void m37710i_11f_M1X0();
	void m37710i_121_M1X0();
	void m37710i_123_M1X0();
	void m37710i_125_M1X0();
	void m37710i_127_M1X0();
	void m37710i_129_M1X0();
	void m37710i_12a_M1X0();
	void m37710i_12d_M1X0();
	void m37710i_12f_M1X0();
	void m37710i_131_M1X0();
	void m37710i_132_M1X0();
	void m37710i_133_M1X0();
	void m37710i_135_M1X0();
	void m37710i_137_M1X0();
	void m37710i_139_M1X0();
	void m37710i_13a_M1X0();
	void m37710i_13b_M1X0();
	void m37710i_13d_M1X0();
	void m37710i_13f_M1X0();
	void m37710i_141_M1X0();
	void m37710i_143_M1X0();
	void m37710i_145_M1X0();
	void m37710i_147_M1X0();
	void m37710i_148_M1X0();
	void m37710i_149_M1X0();
	void m37710i_14a_M1X0();
	void m37710i_14d_M1X0();
	void m37710i_14f_M1X0();
	void m37710i_151_M1X0();
	void m37710i_152_M1X0();
	void m37710i_153_M1X0();
	void m37710i_155_M1X0();
	void m37710i_157_M1X0();
	void m37710i_159_M1X0();
	void m37710i_15b_M1X0();
	void m37710i_15d_M1X0();
	void m37710i_15f_M1X0();
	void m37710i_161_M1X0();
	void m37710i_163_M1X0();
	void m37710i_165_M1X0();
	void m37710i_167_M1X0();
	void m37710i_168_M1X0();
	void m37710i_169_M1X0();
	void m37710i_16a_M1X0();
	void m37710i_16d_M1X0();
	void m37710i_16f_M1X0();
	void m37710i_171_M1X0();
	void m37710i_172_M1X0();
	void m37710i_173_M1X0();
	void m37710i_175_M1X0();
	void m37710i_177_M1X0();
	void m37710i_179_M1X0();
	void m37710i_17b_M1X0();
	void m37710i_17d_M1X0();
	void m37710i_17f_M1X0();
	void m37710i_181_M1X0();
	void m37710i_183_M1X0();
	void m37710i_185_M1X0();
	void m37710i_187_M1X0();
	void m37710i_18a_M1X0();
	void m37710i_18d_M1X0();
	void m37710i_18f_M1X0();
	void m37710i_191_M1X0();
	void m37710i_192_M1X0();
	void m37710i_193_M1X0();
	void m37710i_195_M1X0();
	void m37710i_197_M1X0();
	void m37710i_198_M1X0();
	void m37710i_199_M1X0();
	void m37710i_19d_M1X0();
	void m37710i_19f_M1X0();
	void m37710i_1a1_M1X0();
	void m37710i_1a3_M1X0();
	void m37710i_1a5_M1X0();
	void m37710i_1a7_M1X0();
	void m37710i_1a8_M1X0();
	void m37710i_1a9_M1X0();
	void m37710i_1aa_M1X0();
	void m37710i_1ad_M1X0();
	void m37710i_1af_M1X0();
	void m37710i_1b1_M1X0();
	void m37710i_1b2_M1X0();
	void m37710i_1b3_M1X0();
	void m37710i_1b5_M1X0();
	void m37710i_1b7_M1X0();
	void m37710i_1b9_M1X0();
	void m37710i_1bd_M1X0();
	void m37710i_1bf_M1X0();
	void m37710i_1c1_M1X0();
	void m37710i_1c3_M1X0();
	void m37710i_1c5_M1X0();
	void m37710i_1c7_M1X0();
	void m37710i_1c9_M1X0();
	void m37710i_1cd_M1X0();
	void m37710i_1cf_M1X0();
	void m37710i_1d1_M1X0();
	void m37710i_1d2_M1X0();
	void m37710i_1d3_M1X0();
	void m37710i_1d5_M1X0();
	void m37710i_1d7_M1X0();
	void m37710i_1d9_M1X0();
	void m37710i_1dd_M1X0();
	void m37710i_1df_M1X0();
	void m37710i_1e1_M1X0();
	void m37710i_1e3_M1X0();
	void m37710i_1e5_M1X0();
	void m37710i_1e7_M1X0();
	void m37710i_1e9_M1X0();
	void m37710i_1ed_M1X0();
	void m37710i_1ef_M1X0();
	void m37710i_1f1_M1X0();
	void m37710i_1f2_M1X0();
	void m37710i_1f3_M1X0();
	void m37710i_1f5_M1X0();
	void m37710i_1f7_M1X0();
	void m37710i_1f9_M1X0();
	void m37710i_1fd_M1X0();
	void m37710i_1ff_M1X0();
	void m37710i_200_M1X0();
	void m37710i_201_M1X0();
	void m37710i_203_M1X0();
	void m37710i_205_M1X0();
	void m37710i_207_M1X0();
	void m37710i_209_M1X0();
	void m37710i_20d_M1X0();
	void m37710i_20f_M1X0();
	void m37710i_211_M1X0();
	void m37710i_212_M1X0();
	void m37710i_213_M1X0();
	void m37710i_215_M1X0();
	void m37710i_217_M1X0();
	void m37710i_219_M1X0();
	void m37710i_21d_M1X0();
	void m37710i_21f_M1X0();
	void m37710i_221_M1X0();
	void m37710i_223_M1X0();
	void m37710i_225_M1X0();
	void m37710i_227_M1X0();
	void m37710i_228_M1X0();
	void m37710i_229_M1X0();
	void m37710i_22d_M1X0();
	void m37710i_22f_M1X0();
	void m37710i_231_M1X0();
	void m37710i_232_M1X0();
	void m37710i_233_M1X0();
	void m37710i_235_M1X0();
	void m37710i_237_M1X0();
	void m37710i_239_M1X0();
	void m37710i_23d_M1X0();
	void m37710i_23f_M1X0();
	void m37710i_249_M1X0();
	void m37710i_2c2_M1X0();
	void m37710i_00_M1X1();
	void m37710i_01_M1X1();
	void m37710i_02_M1X1();
	void m37710i_03_M1X1();
	void m37710i_04_M1X1();
	void m37710i_05_M1X1();
	void m37710i_06_M1X1();
	void m37710i_07_M1X1();
	void m37710i_08_M1X1();
	void m37710i_09_M1X1();
	void m37710i_0a_M1X1();
	void m37710i_0b_M1X1();
	void m37710i_0c_M1X1();
	void m37710i_0d_M1X1();
	void m37710i_0e_M1X1();
	void m37710i_0f_M1X1();
	void m37710i_10_M1X1();
	void m37710i_11_M1X1();
	void m37710i_12_M1X1();
	void m37710i_13_M1X1();
	void m37710i_14_M1X1();
	void m37710i_15_M1X1();
	void m37710i_16_M1X1();
	void m37710i_17_M1X1();
	void m37710i_18_M1X1();
	void m37710i_19_M1X1();
	void m37710i_1a_M1X1();
	void m37710i_1b_M1X1();
	void m37710i_1c_M1X1();
	void m37710i_1d_M1X1();
	void m37710i_1e_M1X1();
	void m37710i_1f_M1X1();
	void m37710i_20_M1X1();
	void m37710i_21_M1X1();
	void m37710i_22_M1X1();
	void m37710i_23_M1X1();
	void m37710i_24_M1X1();
	void m37710i_25_M1X1();
	void m37710i_26_M1X1();
	void m37710i_27_M1X1();
	void m37710i_28_M1X1();
	void m37710i_29_M1X1();
	void m37710i_2a_M1X1();
	void m37710i_2b_M1X1();
	void m37710i_2c_M1X1();
	void m37710i_2d_M1X1();
	void m37710i_2e_M1X1();
	void m37710i_2f_M1X1();
	void m37710i_30_M1X1();
	void m37710i_31_M1X1();
	void m37710i_32_M1X1();
	void m37710i_33_M1X1();
	void m37710i_34_M1X1();
	void m37710i_35_M1X1();
	void m37710i_36_M1X1();
	void m37710i_37_M1X1();
	void m37710i_38_M1X1();
	void m37710i_39_M1X1();
	void m37710i_3a_M1X1();
	void m37710i_3b_M1X1();
	void m37710i_3c_M1X1();
	void m37710i_3d_M1X1();
	void m37710i_3e_M1X1();
	void m37710i_3f_M1X1();
	void m37710i_40_M1X1();
	void m37710i_41_M1X1();
	void m37710i_42_M1X1();
	void m37710i_43_M1X1();
	void m37710i_44_M1X1();
	void m37710i_45_M1X1();
	void m37710i_46_M1X1();
	void m37710i_47_M1X1();
	void m37710i_48_M1X1();
	void m37710i_49_M1X1();
	void m37710i_4a_M1X1();
	void m37710i_4b_M1X1();
	void m37710i_4c_M1X1();
	void m37710i_4d_M1X1();
	void m37710i_4e_M1X1();
	void m37710i_4f_M1X1();
	void m37710i_50_M1X1();
	void m37710i_51_M1X1();
	void m37710i_52_M1X1();
	void m37710i_53_M1X1();
	void m37710i_54_M1X1();
	void m37710i_55_M1X1();
	void m37710i_56_M1X1();
	void m37710i_57_M1X1();
	void m37710i_58_M1X1();
	void m37710i_59_M1X1();
	void m37710i_5a_M1X1();
	void m37710i_5b_M1X1();
	void m37710i_5c_M1X1();
	void m37710i_5d_M1X1();
	void m37710i_5e_M1X1();
	void m37710i_5f_M1X1();
	void m37710i_60_M1X1();
	void m37710i_61_M1X1();
	void m37710i_62_M1X1();
	void m37710i_63_M1X1();
	void m37710i_64_M1X1();
	void m37710i_65_M1X1();
	void m37710i_66_M1X1();
	void m37710i_67_M1X1();
	void m37710i_68_M1X1();
	void m37710i_69_M1X1();
	void m37710i_6a_M1X1();
	void m37710i_6b_M1X1();
	void m37710i_6c_M1X1();
	void m37710i_6d_M1X1();
	void m37710i_6e_M1X1();
	void m37710i_6f_M1X1();
	void m37710i_70_M1X1();
	void m37710i_71_M1X1();
	void m37710i_72_M1X1();
	void m37710i_73_M1X1();
	void m37710i_74_M1X1();
	void m37710i_75_M1X1();
	void m37710i_76_M1X1();
	void m37710i_77_M1X1();
	void m37710i_78_M1X1();
	void m37710i_79_M1X1();
	void m37710i_7a_M1X1();
	void m37710i_7b_M1X1();
	void m37710i_7c_M1X1();
	void m37710i_7d_M1X1();
	void m37710i_7e_M1X1();
	void m37710i_7f_M1X1();
	void m37710i_80_M1X1();
	void m37710i_81_M1X1();
	void m37710i_82_M1X1();
	void m37710i_83_M1X1();
	void m37710i_84_M1X1();
	void m37710i_85_M1X1();
	void m37710i_86_M1X1();
	void m37710i_87_M1X1();
	void m37710i_88_M1X1();
	void m37710i_89_M1X1();
	void m37710i_8a_M1X1();
	void m37710i_8b_M1X1();
	void m37710i_8c_M1X1();
	void m37710i_8d_M1X1();
	void m37710i_8e_M1X1();
	void m37710i_8f_M1X1();
	void m37710i_90_M1X1();
	void m37710i_91_M1X1();
	void m37710i_92_M1X1();
	void m37710i_93_M1X1();
	void m37710i_94_M1X1();
	void m37710i_95_M1X1();
	void m37710i_96_M1X1();
	void m37710i_97_M1X1();
	void m37710i_98_M1X1();
	void m37710i_99_M1X1();
	void m37710i_9a_M1X1();
	void m37710i_9b_M1X1();
	void m37710i_9c_M1X1();
	void m37710i_9d_M1X1();
	void m37710i_9e_M1X1();
	void m37710i_9f_M1X1();
	void m37710i_a0_M1X1();
	void m37710i_a1_M1X1();
	void m37710i_a2_M1X1();
	void m37710i_a3_M1X1();
	void m37710i_a4_M1X1();
	void m37710i_a5_M1X1();
	void m37710i_a6_M1X1();
	void m37710i_a7_M1X1();
	void m37710i_a8_M1X1();
	void m37710i_a9_M1X1();
	void m37710i_aa_M1X1();
	void m37710i_ab_M1X1();
	void m37710i_ac_M1X1();
	void m37710i_ad_M1X1();
	void m37710i_ae_M1X1();
	void m37710i_af_M1X1();
	void m37710i_b0_M1X1();
	void m37710i_b1_M1X1();
	void m37710i_b2_M1X1();
	void m37710i_b3_M1X1();
	void m37710i_b4_M1X1();
	void m37710i_b5_M1X1();
	void m37710i_b6_M1X1();
	void m37710i_b7_M1X1();
	void m37710i_b8_M1X1();
	void m37710i_b9_M1X1();
	void m37710i_ba_M1X1();
	void m37710i_bb_M1X1();
	void m37710i_bc_M1X1();
	void m37710i_bd_M1X1();
	void m37710i_be_M1X1();
	void m37710i_bf_M1X1();
	void m37710i_c0_M1X1();
	void m37710i_c1_M1X1();
	void m37710i_c2_M1X1();
	void m37710i_c3_M1X1();
	void m37710i_c4_M1X1();
	void m37710i_c5_M1X1();
	void m37710i_c6_M1X1();
	void m37710i_c7_M1X1();
	void m37710i_c8_M1X1();
	void m37710i_c9_M1X1();
	void m37710i_ca_M1X1();
	void m37710i_cb_M1X1();
	void m37710i_cc_M1X1();
	void m37710i_cd_M1X1();
	void m37710i_ce_M1X1();
	void m37710i_cf_M1X1();
	void m37710i_d0_M1X1();
	void m37710i_d1_M1X1();
	void m37710i_d2_M1X1();
	void m37710i_d3_M1X1();
	void m37710i_d4_M1X1();
	void m37710i_d5_M1X1();
	void m37710i_d6_M1X1();
	void m37710i_d7_M1X1();
	void m37710i_d8_M1X1();
	void m37710i_d9_M1X1();
	void m37710i_da_M1X1();
	void m37710i_db_M1X1();
	void m37710i_dc_M1X1();
	void m37710i_dd_M1X1();
	void m37710i_de_M1X1();
	void m37710i_df_M1X1();
	void m37710i_e0_M1X1();
	void m37710i_e1_M1X1();
	void m37710i_e2_M1X1();
	void m37710i_e3_M1X1();
	void m37710i_e4_M1X1();
	void m37710i_e5_M1X1();
	void m37710i_e6_M1X1();
	void m37710i_e7_M1X1();
	void m37710i_e8_M1X1();
	void m37710i_e9_M1X1();
	void m37710i_ea_M1X1();
	void m37710i_eb_M1X1();
	void m37710i_ec_M1X1();
	void m37710i_ed_M1X1();
	void m37710i_ee_M1X1();
	void m37710i_ef_M1X1();
	void m37710i_f0_M1X1();
	void m37710i_f1_M1X1();
	void m37710i_f2_M1X1();
	void m37710i_f3_M1X1();
	void m37710i_f4_M1X1();
	void m37710i_f5_M1X1();
	void m37710i_f6_M1X1();
	void m37710i_f7_M1X1();
	void m37710i_f8_M1X1();
	void m37710i_f9_M1X1();
	void m37710i_fa_M1X1();
	void m37710i_fb_M1X1();
	void m37710i_fc_M1X1();
	void m37710i_fd_M1X1();
	void m37710i_fe_M1X1();
	void m37710i_ff_M1X1();
	void m37710i_101_M1X1();
	void m37710i_103_M1X1();
	void m37710i_105_M1X1();
	void m37710i_107_M1X1();
	void m37710i_109_M1X1();
	void m37710i_10a_M1X1();
	void m37710i_10d_M1X1();
	void m37710i_10f_M1X1();
	void m37710i_111_M1X1();
	void m37710i_112_M1X1();
	void m37710i_113_M1X1();
	void m37710i_115_M1X1();
	void m37710i_117_M1X1();
	void m37710i_119_M1X1();
	void m37710i_11a_M1X1();
	void m37710i_11b_M1X1();
	void m37710i_11d_M1X1();
	void m37710i_11f_M1X1();
	void m37710i_121_M1X1();
	void m37710i_123_M1X1();
	void m37710i_125_M1X1();
	void m37710i_127_M1X1();
	void m37710i_129_M1X1();
	void m37710i_12a_M1X1();
	void m37710i_12d_M1X1();
	void m37710i_12f_M1X1();
	void m37710i_131_M1X1();
	void m37710i_132_M1X1();
	void m37710i_133_M1X1();
	void m37710i_135_M1X1();
	void m37710i_137_M1X1();
	void m37710i_139_M1X1();
	void m37710i_13a_M1X1();
	void m37710i_13b_M1X1();
	void m37710i_13d_M1X1();
	void m37710i_13f_M1X1();
	void m37710i_141_M1X1();
	void m37710i_143_M1X1();
	void m37710i_145_M1X1();
	void m37710i_147_M1X1();
	void m37710i_148_M1X1();
	void m37710i_149_M1X1();
	void m37710i_14a_M1X1();
	void m37710i_14d_M1X1();
	void m37710i_14f_M1X1();
	void m37710i_151_M1X1();
	void m37710i_152_M1X1();
	void m37710i_153_M1X1();
	void m37710i_155_M1X1();
	void m37710i_157_M1X1();
	void m37710i_159_M1X1();
	void m37710i_15b_M1X1();
	void m37710i_15d_M1X1();
	void m37710i_15f_M1X1();
	void m37710i_161_M1X1();
	void m37710i_163_M1X1();
	void m37710i_165_M1X1();
	void m37710i_167_M1X1();
	void m37710i_168_M1X1();
	void m37710i_169_M1X1();
	void m37710i_16a_M1X1();
	void m37710i_16d_M1X1();
	void m37710i_16f_M1X1();
	void m37710i_171_M1X1();
	void m37710i_172_M1X1();
	void m37710i_173_M1X1();
	void m37710i_175_M1X1();
	void m37710i_177_M1X1();
	void m37710i_179_M1X1();
	void m37710i_17b_M1X1();
	void m37710i_17d_M1X1();
	void m37710i_17f_M1X1();
	void m37710i_181_M1X1();
	void m37710i_183_M1X1();
	void m37710i_185_M1X1();
	void m37710i_187_M1X1();
	void m37710i_18a_M1X1();
	void m37710i_18d_M1X1();
	void m37710i_18f_M1X1();
	void m37710i_191_M1X1();
	void m37710i_192_M1X1();
	void m37710i_193_M1X1();
	void m37710i_195_M1X1();
	void m37710i_197_M1X1();
	void m37710i_198_M1X1();
	void m37710i_199_M1X1();
	void m37710i_19d_M1X1();
	void m37710i_19f_M1X1();
	void m37710i_1a1_M1X1();
	void m37710i_1a3_M1X1();
	void m37710i_1a5_M1X1();
	void m37710i_1a7_M1X1();
	void m37710i_1a8_M1X1();
	void m37710i_1a9_M1X1();
	void m37710i_1aa_M1X1();
	void m37710i_1ad_M1X1();
	void m37710i_1af_M1X1();
	void m37710i_1b1_M1X1();
	void m37710i_1b2_M1X1();
	void m37710i_1b3_M1X1();
	void m37710i_1b5_M1X1();
	void m37710i_1b7_M1X1();
	void m37710i_1b9_M1X1();
	void m37710i_1bd_M1X1();
	void m37710i_1bf_M1X1();
	void m37710i_1c1_M1X1();
	void m37710i_1c3_M1X1();
	void m37710i_1c5_M1X1();
	void m37710i_1c7_M1X1();
	void m37710i_1c9_M1X1();
	void m37710i_1cd_M1X1();
	void m37710i_1cf_M1X1();
	void m37710i_1d1_M1X1();
	void m37710i_1d2_M1X1();
	void m37710i_1d3_M1X1();
	void m37710i_1d5_M1X1();
	void m37710i_1d7_M1X1();
	void m37710i_1d9_M1X1();
	void m37710i_1dd_M1X1();
	void m37710i_1df_M1X1();
	void m37710i_1e1_M1X1();
	void m37710i_1e3_M1X1();
	void m37710i_1e5_M1X1();
	void m37710i_1e7_M1X1();
	void m37710i_1e9_M1X1();
	void m37710i_1ed_M1X1();
	void m37710i_1ef_M1X1();
	void m37710i_1f1_M1X1();
	void m37710i_1f2_M1X1();
	void m37710i_1f3_M1X1();
	void m37710i_1f5_M1X1();
	void m37710i_1f7_M1X1();
	void m37710i_1f9_M1X1();
	void m37710i_1fd_M1X1();
	void m37710i_1ff_M1X1();
	void m37710i_200_M1X1();
	void m37710i_201_M1X1();
	void m37710i_203_M1X1();
	void m37710i_205_M1X1();
	void m37710i_207_M1X1();
	void m37710i_209_M1X1();
	void m37710i_20d_M1X1();
	void m37710i_20f_M1X1();
	void m37710i_211_M1X1();
	void m37710i_212_M1X1();
	void m37710i_213_M1X1();
	void m37710i_215_M1X1();
	void m37710i_217_M1X1();
	void m37710i_219_M1X1();
	void m37710i_21d_M1X1();
	void m37710i_21f_M1X1();
	void m37710i_221_M1X1();
	void m37710i_223_M1X1();
	void m37710i_225_M1X1();
	void m37710i_227_M1X1();
	void m37710i_228_M1X1();
	void m37710i_229_M1X1();
	void m37710i_22d_M1X1();
	void m37710i_22f_M1X1();
	void m37710i_231_M1X1();
	void m37710i_232_M1X1();
	void m37710i_233_M1X1();
	void m37710i_235_M1X1();
	void m37710i_237_M1X1();
	void m37710i_239_M1X1();
	void m37710i_23d_M1X1();
	void m37710i_23f_M1X1();
	void m37710i_249_M1X1();
	void m37710i_2c2_M1X1();

};


class m37702s1_device : public m37710_cpu_device
{
public:
	// construction/destruction
	m37702s1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	void map(address_map &map) ATTR_COLD;
};

class m37702m2_device : public m37710_cpu_device
{
public:
	// construction/destruction
	m37702m2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	m37702m2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	void map(address_map &map) ATTR_COLD;
};

class m37710s4_device : public m37710_cpu_device
{
public:
	// construction/destruction
	m37710s4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	void map(address_map &map) ATTR_COLD;
};

class m37720s1_device : public m37710_cpu_device
{
public:
	// construction/destruction
	m37720s1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	void map(address_map &map) ATTR_COLD;
};

class m37730s2_device : public m37710_cpu_device
{
public:
	// construction/destruction
	m37730s2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	void map(address_map &map) ATTR_COLD;
};

class m37732s4_device : public m37710_cpu_device
{
public:
	// construction/destruction
	m37732s4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	void map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(M37702M2, m37702m2_device)
DECLARE_DEVICE_TYPE(M37702S1, m37702s1_device)
DECLARE_DEVICE_TYPE(M37710S4, m37710s4_device)
DECLARE_DEVICE_TYPE(M37720S1, m37720s1_device)
DECLARE_DEVICE_TYPE(M37730S2, m37730s2_device)
DECLARE_DEVICE_TYPE(M37732S4, m37732s4_device)


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif // MAME_CPU_M37710_M37710_H
