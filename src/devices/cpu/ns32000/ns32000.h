// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_NS32000_NS32000_H
#define MAME_CPU_NS32000_NS32000_H

#pragma once

#include "common.h"

template <int HighBits, int Width>
class ns32000_device : public cpu_device
{
public:
	template <typename T> void set_fpu(T &&tag) { m_fpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_mmu(T &&tag) { m_mmu.set_tag(std::forward<T>(tag)); }

	void rdy_w(int state) { m_ready = !state; }

protected:
	ns32000_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface implementation
	virtual void state_string_export(device_state_entry const &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	enum addr_mode_type : unsigned
	{
		IMM, // immediate
		REG, // register
		MEM, // memory space
		REL, // memory relative
		EXT, // external
		TOS, // top of stack
	};
	enum access_class : unsigned
	{
		NONE,
		READ,
		WRITE,
		RMW,
		ADDR,
		REGADDR,
	};
	enum size_code : unsigned
	{
		SIZE_B = 0,
		SIZE_W = 1,
		SIZE_D = 3,
		SIZE_Q = 7,
	};

	// time needed to read or write a memory operand
	unsigned top(size_code const size, u32 const address = 0) const
	{
		unsigned const tmmu = m_mmu ? 1 : 0;

		switch (size)
		{
		case SIZE_B: return 3 + tmmu;
		case SIZE_W: return (address & 1) ? 7 + 2 * tmmu : 3 + tmmu;
		case SIZE_D: return (address & 1) ? 11 + 3 * tmmu : 7 + 2 * tmmu;
		case SIZE_Q: return (address & 1) ? 19 + 5 * tmmu : 15 + 4 * tmmu;
		}

		// can't happen
		return 0;
	}

	struct addr_mode
	{
		addr_mode(unsigned gen)
			: gen(gen)
			, access(NONE)
			, slave(false)
			, base(0)
			, disp(0)
			, imm(0)
			, tea(0)
		{};

		void read_i(size_code code)  { access = READ;    size = code; }
		void read_f(size_code code)  { access = READ;    size = code; slave = true; }
		void write_i(size_code code) { access = WRITE;   size = code; }
		void write_f(size_code code) { access = WRITE;   size = code; slave = true; }
		void rmw_i(size_code code)   { access = RMW;     size = code; }
		void rmw_f(size_code code)   { access = RMW;     size = code; slave = true; }
		void addr()                  { access = ADDR;    size = SIZE_D; }
		void regaddr()               { access = REGADDR; size = SIZE_D; }

		unsigned gen;
		addr_mode_type type;
		access_class access;
		size_code size;
		bool slave;
		u32 base;
		u32 disp;
		u64 imm;
		unsigned tea;
	};

	// memory accessors
	template <typename T> T mem_read(unsigned st, u32 address, bool user = false, bool pfs = false);
	template <typename T> void mem_write(unsigned st, u32 address, u64 data, bool user = false);

	// instruction fetch/decode helpers
	template <typename T> T fetch(unsigned &bytes);
	s32 displacement(unsigned &bytes);
	void decode(addr_mode *mode, unsigned &bytes);

	// operand read/write helpers
	u32 ea(addr_mode const mode);
	u64 gen_read(addr_mode mode);
	s64 gen_read_sx(addr_mode mode);
	void gen_write(addr_mode mode, u64 data);

	// register helpers
	u32 &SP();
	u32 &SP(bool user);
	virtual u32 const PSR_MSK() { return 0x0fe7; }

	// other execution helpers
	bool condition(unsigned const cc);
	void flags(u32 const src1, u32 const src2, u32 const dest, unsigned const size, bool const subtraction);
	void interrupt(unsigned const type);
	virtual void lpr(unsigned reg, addr_mode const mode, bool user, unsigned &tex);
	virtual void spr(unsigned reg, addr_mode const mode, bool user, unsigned &tex);

	// slave protocol helpers
	virtual u16 slave(u8 opbyte, u16 opword, addr_mode op1, addr_mode op2);
	u16 slave_slow(ns32000_slow_slave_interface &slave, u8 opbyte, u16 opword, addr_mode op1, addr_mode op2);
	u16 slave_fast(ns32000_fast_slave_interface &slave, u8 opbyte, u16 opword, addr_mode op1, addr_mode op2);

	u32 m_cfg; // configuration register

	typename memory_access<HighBits, Width, 0, ENDIANNESS_LITTLE>::specific m_bus[16];

private:
	u32 const m_address_mask;

	// configuration
	address_space_config m_program_config;
	address_space_config m_iam_config;
	address_space_config m_iac_config;
	address_space_config m_eim_config;
	address_space_config m_eic_config;
	address_space_config m_sif_config;
	address_space_config m_nif_config;
	address_space_config m_odt_config;
	address_space_config m_rmw_config;
	address_space_config m_ear_config;

	optional_device<ns32000_fpu_interface> m_fpu;
	optional_device<ns32000_mmu_interface> m_mmu;

	// emulation state
	int m_icount;

	u32 m_ssp;     // saved stack pointer
	u16 m_sps;     // saved program status

	u32 m_pc;      // program counter
	u32 m_sb;      // static base
	u32 m_fp;      // frame pointer
	u32 m_sp1;     // user stack pointer
	u32 m_sp0;     // interrupt stack pointer
	u32 m_intbase; // interrupt base
	u16 m_psr;     // processor status
	u16 m_mod;     // module

	u32 m_r[8];

	bool m_nmi_line;
	bool m_int_line;
	bool m_wait;
	bool m_sequential;
	bool m_ready;
};

class ns32008_device : public ns32000_device<24, 0>
{
public:
	ns32008_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class ns32016_device : public ns32000_device<24, 1>
{
public:
	ns32016_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class ns32032_device : public ns32000_device<24, 2>
{
public:
	ns32032_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class ns32332_device : public ns32000_device<32, 2>
{
public:
	ns32332_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class ns32532_device
	: public ns32000_device<32, 2>
	, public ns32000_mmu_interface
{
public:
	ns32532_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// ns32000_mmu_interface implementation
	virtual void state_add(device_state_interface &parent, int &index) override;
	virtual translate_result translate(address_space &space, unsigned st, u32 &address, bool user, bool write, bool rdwrval = false, bool suppress = false) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	virtual u32 const PSR_MSK() override { return 0x0ff7; }
	virtual void lpr(unsigned reg, addr_mode const mode, bool user, unsigned &tex) override;
	virtual void spr(unsigned reg, addr_mode const mode, bool user, unsigned &tex) override;
	virtual u16 slave(u8 opbyte, u16 opword, addr_mode op1, addr_mode op2) override;

private:
	address_space_config m_pt1_config;
	address_space_config m_pt2_config;

	// memory management registers
	u32 m_ptb[2];  // page table base pointer
	u32 m_tear;    // translation exception address
	u32 m_mcr;     // memory management control
	u32 m_msr;     // memory management status

	// debug registers
	u32 m_dcr; // debug condition
	u32 m_dsr; // debug status
	u32 m_car; // compare address
	u32 m_bpc; // breakpoint program counter
};

DECLARE_DEVICE_TYPE(NS32008, ns32008_device)
DECLARE_DEVICE_TYPE(NS32016, ns32016_device)
DECLARE_DEVICE_TYPE(NS32032, ns32032_device)
DECLARE_DEVICE_TYPE(NS32332, ns32332_device)
DECLARE_DEVICE_TYPE(NS32532, ns32532_device)

#endif // MAME_CPU_NS32000_NS32000_H
