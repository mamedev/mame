// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   lh6801.h
 *   portable lh5801 emulator interface
 *
 *
 *****************************************************************************/
#ifndef MAME_CPU_LH5801_LH5801_H
#define MAME_CPU_LH5801_LH5801_H

#pragma once

/*
lh5801

little endian

ph, pl p
sh, sl s

xh, xl x
yh, yl y
uh, ul u

a A

0 0 0 H V Z IE C

TM 9bit polynomial?

pu pv disp flipflops

bf flipflop (break key connected)

    me0, me1 chip select for 2 64kb memory blocks

in0-in7 input pins

    mi maskable interrupt input (fff8/9)
    timer fffa/b
    nmi non .. (fffc/d)
    reset fffe/f
e ?



lh5811 chip
pa 8bit io
pb 8bit io
pc 8bit
*/


// input lines
enum
{
	LH5801_LINE_MI     //maskable interrupt
};


class lh5801_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	lh5801_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto in_func() { return m_in_func.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
	virtual uint32_t execute_min_cycles() const noexcept override { return 2; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 19; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == LH5801_LINE_MI || inputnum == INPUT_LINE_NMI; }
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
	address_space_config m_io_config;

	devcb_read8 m_in_func;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program; // ME0
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;      // ME1

	PAIR m_s;
	PAIR m_p;
	PAIR m_u;
	PAIR m_x;
	PAIR m_y;
	int m_tm; //9 bit

	uint8_t m_t, m_a;

	int m_bf;
	int m_dp;
	int m_pu;
	int m_pv;

	uint16_t m_oldpc;

	int m_irq_state;

	uint8_t m_ir_flipflop[3];   //interrupt request flipflop: IR0, IR1, IR2
	int m_lines_status[2];    //MI and NMI lines status

	int m_idle;
	int m_icount;

	void check_irq();
	void lh5801_instruction_fd();
	void lh5801_instruction();
	uint8_t lh5801_add_generic(int left, int right, int carry);
	uint16_t lh5801_readop_word();
	void lh5801_adc(uint8_t data);
	void lh5801_add_mem(address_space &space, int addr, uint8_t data);
	void lh5801_adr(PAIR *reg);
	void lh5801_sbc(uint8_t data);
	void lh5801_cpa(uint8_t a, uint8_t b);
	uint8_t lh5801_decimaladd_generic(int left, int right, int carry);
	void lh5801_dca(uint8_t data);
	void lh5801_dcs(uint8_t data);
	void lh5801_and(uint8_t data);
	void lh5801_and_mem(address_space &space, int addr, uint8_t data);
	void lh5801_bit(uint8_t a, uint8_t b);
	void lh5801_eor(uint8_t data);
	void lh5801_ora(uint8_t data);
	void lh5801_ora_mem(address_space &space, int addr, uint8_t data);
	void lh5801_lda(uint8_t data);
	void lh5801_lde(PAIR *reg);
	void lh5801_sde(PAIR *reg);
	void lh5801_lin(PAIR *reg);
	void lh5801_sin(PAIR *reg);
	void lh5801_dec(uint8_t *adr);
	void lh5801_inc(uint8_t *adr);
	void lh5801_pop();
	void lh5801_pop_word(PAIR *reg);
	void lh5801_rtn();
	void lh5801_rti();
	void lh5801_push(uint8_t data);
	void lh5801_push_word(uint16_t data);
	void lh5801_jmp(uint16_t adr);
	void lh5801_branch_plus(int taken);
	void lh5801_branch_minus(int taken);
	void lh5801_lop();
	void lh5801_sjp();
	void lh5801_vector(int taken, int nr);
	void lh5801_aex();
	void lh5801_drl(address_space &space, int adr);
	void lh5801_drr(address_space &space, int adr);
	void lh5801_rol();
	void lh5801_ror();
	void lh5801_shl();
	void lh5801_shr();
	void lh5801_am(int value);
	void lh5801_ita();
};


DECLARE_DEVICE_TYPE(LH5801, lh5801_cpu_device)


#endif // MAME_CPU_LH5801_LH5801_H
