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
	ns32000_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, bool cg16 = false);

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

	// Time needed to read or write a memory operand: the first bus cycle
	// costs 3 (4 with address translation), each additional one 4 (5).
	// The number of bus cycles depends on the bus width of the device,
	// the operand size, and operand alignment.
	unsigned top(size_code const size, u32 const address = 0) const
	{
		unsigned const tmmu = m_mmu ? 1 : 0;
		unsigned const bytes = size + 1;
		unsigned const bpt = 1U << Width;

		// number of bus cycles required for the transfer
		unsigned const n = (bytes + (address & (bpt - 1)) + (bpt - 1)) >> Width;

		return 3 + (n - 1) * 4 + n * tmmu;
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

	// EXTBLT (NS32CG16) asserts this around the block transfer so an external
	// BPU wired up in the driver can snoop the source/destination bus cycles
	// via memory taps; the base CPU has no BPU and ignores it.
	virtual void bpu_window(bool active) { }

	// slave protocol helpers
	virtual u16 slave(u8 opbyte, u16 opword, addr_mode op1, addr_mode op2);
	u16 slave_slow(ns32000_slow_slave_interface &slave, u8 opbyte, u16 opword, addr_mode op1, addr_mode op2);
	u16 slave_fast(ns32000_fast_slave_interface &slave, u8 opbyte, u16 opword, addr_mode op1, addr_mode op2);

	u32 m_cfg; // configuration register

	// the external Series 32000 MMU (NS32082/NS32382) is consulted only after
	// SETCFG has enabled it (CFG M); the NS32532's on-chip MMU gates internally
	// (via MSR/MCR) and ignores CFG M, so it clears this in its constructor.
	bool m_mmu_uses_cfg_m;

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
	bool m_cg16;   // NS32CG16 graphics-instruction variant enabled
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
	virtual translate_result translate(address_space &space, unsigned st, offs_t &address, bool user, bool write, bool rdwrval = false, bool suppress = false) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 const PSR_MSK() override { return 0x0ff7; }
	virtual void lpr(unsigned reg, addr_mode const mode, bool user, unsigned &tex) override;
	virtual void spr(unsigned reg, addr_mode const mode, bool user, unsigned &tex) override;
	virtual u16 slave(u8 opbyte, u16 opword, addr_mode op1, addr_mode op2) override;

private:
	static constexpr unsigned TLB_ENTRIES = 1024; // power of two

	struct tlb_entry
	{
		u32  tag;   // virtual page number (address >> 12)
		u32  pfn;   // physical frame (PTE.PFN, page-aligned)
		u8   pl;    // effective (most restrictive) protection level
		bool as;    // address space
		bool ci;    // cache inhibit
		bool m;     // level-2 PTE modified bit
		bool valid;
	};

	void tlb_flush();                      // invalidate all entries
	void tlb_flush_as(bool as);            // invalidate one address space (PTBn load)
	void tlb_invalidate(u32 va, bool as);  // invalidate one page (IVARn write)

	address_space_config m_pt1_config;
	address_space_config m_pt2_config;

	// memory management registers
	u32 m_ptb[2];  // page table base pointer
	u32 m_tear;    // translation exception address
	u32 m_mcr;     // memory management control
	u32 m_msr;     // memory management status

	// translation look-aside buffer (Section 3.4.4).  Caches completed page
	// table walks so the common case avoids two extra memory reads per access.
	// Modelled with more entries than the real 64-entry TLB purely for emulation
	// speed; capacity does not affect correctness as long as invalidation (PTBn
	// load, IVARn write) is exact.  R/M bits follow the hardware: a write to a
	// cached page whose recorded M bit is clear still walks, to set PTE.M.
	tlb_entry m_tlb[TLB_ENTRIES];

	// debug registers
	u32 m_dcr; // debug condition
	u32 m_dsr; // debug status
	u32 m_car; // compare address
	u32 m_bpc; // breakpoint program counter
};

// NS32CG16 — graphics/printer variant: an NS32016 (24-bit address, 16-bit bus)
// plus the Series 32000 graphics instructions (Format-5 extensions).  The base
// constructor's cg16 flag enables that instruction group; the chip is otherwise
// an NS32016.
class ns32cg16_device : public ns32000_device<24, 1>
{
public:
	ns32cg16_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// EXTBLT asserts this for the duration of the block transfer; a DP8510/
	// DP8511 BITBLT processing unit attached in the driver uses it to gate the
	// memory taps that route the source/destination words through the BPU.
	auto out_bpu() { return m_out_bpu.bind(); }

protected:
	// /BPU is active low; output the physical level (0 = low/asserted)
	virtual void bpu_window(bool active) override { m_out_bpu(active ? 0 : 1); }

private:
	devcb_write_line m_out_bpu;
};

DECLARE_DEVICE_TYPE(NS32008, ns32008_device)
DECLARE_DEVICE_TYPE(NS32016, ns32016_device)
DECLARE_DEVICE_TYPE(NS32032, ns32032_device)
DECLARE_DEVICE_TYPE(NS32332, ns32332_device)
DECLARE_DEVICE_TYPE(NS32532, ns32532_device)
DECLARE_DEVICE_TYPE(NS32CG16, ns32cg16_device)

#endif // MAME_CPU_NS32000_NS32000_H
