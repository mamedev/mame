// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
#ifndef MAME_CPU_I960_I960_H
#define MAME_CPU_I960_I960_H

#pragma once


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


class i960_cpu_device :  public cpu_device
{
public:
	static constexpr uint16_t BURST = 0x0001;

	// construction/destruction
	i960_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void i960_stall()
	{
		m_stalled = true;
		m_IP = m_PIP;
	}

protected:
	enum { I960_RCACHE_SIZE = 4 };

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; } /* ???? TODO: Exact timing unknown */
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; } /* ???? TODO: Exact timing unknown */
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void burst_stall_save(uint32_t t1, uint32_t t2, int index, int size, bool iswriteop);

	struct {
		uint32_t t1 = 0, t2 = 0;
		int index = 0, size = 0;
		bool burst_mode = false;
		bool iswriteop = false;
	}m_stall_state;
	bool m_stalled;

	address_space_config m_program_config;

	uint32_t m_r[0x20];
	uint32_t m_rcache[I960_RCACHE_SIZE][0x10];
	uint32_t m_rcache_frame_addr[I960_RCACHE_SIZE];
	// rcache_pos = how deep in the stack we are.  0-(I960_RCACHE_SIZE-1) means in-cache.
	// I960_RCACHE_SIZE or greater means out of cache, must save to memory.
	int32_t m_rcache_pos;

	double m_fp[4];

	uint32_t m_SAT;
	uint32_t m_PRCB;
	uint32_t m_PC;
	uint32_t m_AC;
	uint32_t m_IP;
	uint32_t m_PIP;
	uint32_t m_ICR;

	int m_immediate_irq;
	int m_immediate_vector;
	int  m_immediate_pri;

	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;

	int m_icount;

	uint32_t i960_read_dword_unaligned(uint32_t address);
	std::pair<uint32_t, uint16_t> i960_read_dword_unaligned_flags(uint32_t address);
	uint16_t i960_read_word_unaligned(uint32_t address);
	void i960_write_dword_unaligned(uint32_t address, uint32_t data);
	uint16_t i960_write_dword_unaligned_flags(uint32_t address, uint32_t data);
	void i960_write_word_unaligned(uint32_t address, uint16_t data);
	void send_iac(uint32_t adr);
	uint32_t get_ea(uint32_t opcode);
	uint32_t get_1_ri(uint32_t opcode);
	uint32_t get_2_ri(uint32_t opcode);
	uint64_t get_2_ri64(uint32_t opcode);
	void set_ri(uint32_t opcode, uint32_t val);
	void set_ri2(uint32_t opcode, uint32_t val, uint32_t val2);
	void set_ri64(uint32_t opcode, uint64_t val);
	double get_1_rif(uint32_t opcode);
	double get_2_rif(uint32_t opcode);
	void set_rif(uint32_t opcode, double val);
	double get_1_rifl(uint32_t opcode);
	double get_2_rifl(uint32_t opcode);
	void set_rifl(uint32_t opcode, double val);
	uint32_t get_1_ci(uint32_t opcode);
	uint32_t get_2_ci(uint32_t opcode);
	uint32_t get_disp(uint32_t opcode);
	uint32_t get_disp_s(uint32_t opcode);
	void cmp_s(int32_t v1, int32_t v2);
	void cmp_u(uint32_t v1, uint32_t v2);
	void concmp_s(int32_t v1, int32_t v2);
	void concmp_u(uint32_t v1, uint32_t v2);
	void cmp_d(double v1, double v2);
	void bxx(uint32_t opcode, int mask);
	void bxx_s(uint32_t opcode, int mask);
	void fxx(uint32_t opcode, int mask);
	void test(uint32_t opcode, int mask);
	void execute_op(uint32_t opcode);
	void execute_burst_stall_op(uint32_t opcode);
	void take_interrupt(int vector, int lvl);
	void check_irqs();
	void do_call(uint32_t adr, int type, uint32_t stack);
	void do_ret_0();
	void do_ret();
};


DECLARE_DEVICE_TYPE(I960, i960_cpu_device)

#endif // MAME_CPU_I960_I960_H
