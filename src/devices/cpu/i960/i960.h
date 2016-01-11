// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
#pragma once

#ifndef __I960_H__
#define __I960_H__


enum
{
	I960_PFP = 0,
	I960_SP  = 1,
	I960_RIP = 2,
	I960_FP  = 31,

	I960_R0 = 0,
	I960_R1 = 1,
	I960_R2 = 2,
	I960_R3 = 3,
	I960_R4 = 4,
	I960_R5 = 5,
	I960_R6 = 6,
	I960_R7 = 7,
	I960_R8 = 8,
	I960_R9 = 9,
	I960_R10 = 10,
	I960_R11 = 11,
	I960_R12 = 12,
	I960_R13 = 13,
	I960_R14 = 14,
	I960_R15 = 15,
	I960_G0 = 16,
	I960_G1 = 17,
	I960_G2 = 18,
	I960_G3 = 19,
	I960_G4 = 20,
	I960_G5 = 21,
	I960_G6 = 22,
	I960_G7 = 23,
	I960_G8 = 24,
	I960_G9 = 25,
	I960_G10 = 26,
	I960_G11 = 27,
	I960_G12 = 28,
	I960_G13 = 29,
	I960_G14 = 30,
	I960_G15 = 31,

	I960_SAT = 32,
	I960_PRCB = 33,
	I960_PC = 34,
	I960_AC = 35,
	I960_IP = 36,
	I960_PIP = 37
};

enum
{
	I960_IRQ0 = 0,
	I960_IRQ1 = 1,
	I960_IRQ2 = 2,
	I960_IRQ3 = 3
};


enum { I960_RCACHE_SIZE = 4 };


class i960_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	i960_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// call from any read/write handler for a memory area that can't be bursted
	// on the real hardware (e.g. Model 2's interrupt control registers)
	void i960_noburst() { m_bursting = 0; }

	void i960_stall() { m_IP = m_PIP; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; } /* ???? TODO: Exact timing unknown */
	virtual UINT32 execute_max_cycles() const override { return 1; } /* ???? TODO: Exact timing unknown */
	virtual UINT32 execute_input_lines() const override { return 4; }
	virtual UINT32 execute_default_irq_vector() const override { return 0xffffffff; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	UINT32 m_r[0x20];
	UINT32 m_rcache[I960_RCACHE_SIZE][0x10];
	UINT32 m_rcache_frame_addr[I960_RCACHE_SIZE];
	// rcache_pos = how deep in the stack we are.  0-(I960_RCACHE_SIZE-1) means in-cache.
	// I960_RCACHE_SIZE or greater means out of cache, must save to memory.
	INT32 m_rcache_pos;

	double m_fp[4];

	UINT32 m_SAT;
	UINT32 m_PRCB;
	UINT32 m_PC;
	UINT32 m_AC;
	UINT32 m_IP;
	UINT32 m_PIP;
	UINT32 m_ICR;
	int m_bursting;

	int m_immediate_irq;
	int m_immediate_vector;
	int  m_immediate_pri;

	address_space *m_program;
	direct_read_data *m_direct;

	int m_icount;

	UINT32 i960_read_dword_unaligned(UINT32 address);
	UINT16 i960_read_word_unaligned(UINT32 address);
	void i960_write_dword_unaligned(UINT32 address, UINT32 data);
	void i960_write_word_unaligned(UINT32 address, UINT16 data);
	void send_iac(UINT32 adr);
	UINT32 get_ea(UINT32 opcode);
	UINT32 get_1_ri(UINT32 opcode);
	UINT32 get_2_ri(UINT32 opcode);
	UINT64 get_2_ri64(UINT32 opcode);
	void set_ri(UINT32 opcode, UINT32 val);
	void set_ri2(UINT32 opcode, UINT32 val, UINT32 val2);
	void set_ri64(UINT32 opcode, UINT64 val);
	double get_1_rif(UINT32 opcode);
	double get_2_rif(UINT32 opcode);
	void set_rif(UINT32 opcode, double val);
	double get_1_rifl(UINT32 opcode);
	double get_2_rifl(UINT32 opcode);
	void set_rifl(UINT32 opcode, double val);
	UINT32 get_1_ci(UINT32 opcode);
	UINT32 get_2_ci(UINT32 opcode);
	UINT32 get_disp(UINT32 opcode);
	UINT32 get_disp_s(UINT32 opcode);
	void cmp_s(INT32 v1, INT32 v2);
	void cmp_u(UINT32 v1, UINT32 v2);
	void concmp_s(INT32 v1, INT32 v2);
	void concmp_u(UINT32 v1, UINT32 v2);
	void cmp_d(double v1, double v2);
	void bxx(UINT32 opcode, int mask);
	void bxx_s(UINT32 opcode, int mask);
	void test(UINT32 opcode, int mask);
	void execute_op(UINT32 opcode);
	void take_interrupt(int vector, int lvl);
	void check_irqs();
	void do_call(UINT32 adr, int type, UINT32 stack);
	void do_ret_0();
	void do_ret();
};


extern const device_type I960;


#endif /* __I960_H__ */
