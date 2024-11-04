// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Patrick Mackinlay

#ifndef MAME_CPU_MIPS_MIPS1_H
#define MAME_CPU_MIPS_MIPS1_H

#pragma once

class mips1core_device_base : public cpu_device
{
public:
	// device configuration
	void set_endianness(endianness_t endianness) { m_endianness = endianness; }

	// input lines
	template <unsigned Coprocessor> auto in_brcond() { return m_in_brcond[Coprocessor].bind(); }
	void berr_w(int state) { m_bus_error = bool(state); }

protected:
	mips1core_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, u32 cpurev, size_t icache_size, size_t dcache_size, bool cache_pws);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 40; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// exceptions
	void generate_exception(u32 exception, bool refill = false);
	void address_error(int intention, u32 const address);

	// cop0
	virtual void handle_cop0(u32 const op);
	virtual u32 get_cop0_reg(unsigned const reg);
	virtual void set_cop0_reg(unsigned const reg, u32 const data);

	// other coprocessors
	virtual void handle_cop1(u32 const op);
	virtual void handle_cop2(u32 const op);
	virtual void handle_cop3(u32 const op);

	// load/store left/right opcodes
	void lwl(u32 const op);
	void lwr(u32 const op);
	void swl(u32 const op);
	void swr(u32 const op);

	// memory
	enum translate_result : unsigned { ERROR, UNCACHED, CACHED };
	virtual translate_result translate(int intention, offs_t &address, bool debug);
	template <typename T, bool Aligned = true, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, void> load(u32 address, U &&apply);
	template <typename T, bool Aligned = true> void store(u32 address, T data, T mem_mask = ~T(0));
	void fetch(u32 address, std::function<void(u32)> &&apply);

	// cache
	template <typename T> unsigned shift_factor(u32 address) const;
	struct cache
	{
		cache(size_t size)
			: size(size)
		{
		}

		struct line
		{
			enum tag_mask : u32
			{
				INV = 0x0000'0001, // cache line invalid
			};

			void invalidate() { tag = INV; }
			void update(u32 data, u32 mask = 0xffff'ffffU)
			{
				this->tag &= ~INV;
				this->data = (this->data & ~mask) | (data & mask);
			}

			u32 tag;
			u32 data;
		};

		size_t lines() const { return size / 4; }
		void start() { line = std::make_unique<struct line[]>(lines()); }

		size_t const size;
		std::unique_ptr<struct line[]> line;
	};
	std::tuple<struct cache::line &, bool> cache_lookup(u32 address, bool invalidate, bool icache = false);

	// debug helpers
	std::string debug_string(u32 string_pointer, unsigned const limit = 0);
	std::string debug_string_array(u32 array_pointer);

	// address spaces
	address_space_config const m_program_config_be;
	address_space_config const m_program_config_le;

	// configuration
	u32 const m_cpurev;
	endianness_t m_endianness;

	// core registers
	u32 m_pc;
	u32 m_r[32];
	u32 m_hi;
	u32 m_lo;

	// cop0 registers
	u32 m_cop0[32];

	// internal stuff
	int m_icount;
	enum branch_state : unsigned
	{
		NONE      = 0,
		DELAY     = 1, // delay slot instruction active
		BRANCH    = 2, // branch instruction active
		EXCEPTION = 3, // exception triggered
	}
	m_branch_state;
	u32 m_branch_target;

	// cache memory
	struct cache m_icache;
	struct cache m_dcache;
	translate_result const m_cache;
	bool const m_cache_pws; // cache supports partial-word store

	// I/O
	devcb_read_line::array<4> m_in_brcond;
	bool m_bus_error;
};

class mips1_device_base : public mips1core_device_base
{
public:
	// floating point coprocessor revision numbers recognised by RISC/os 4.52 and IRIX
	enum fpu_rev : u32
	{
		MIPS_R2360    = 0x0100, // MIPS R2360 Floating Point Board
		MIPS_R2010    = 0x0200, // MIPS R2010 VLSI Floating Point Chip
		MIPS_R2010A   = 0x0310, // MIPS R2010A VLSI Floating Point Chip
		MIPS_R3010    = 0x0320, // MIPS R3010 VLSI Floating Point Chip
		MIPS_R3010A   = 0x0330, // MIPS R3010A VLSI Floating Point Chip
		MIPS_R3010Av4 = 0x0340, // MIPS R3010A VLSI Floating Point Chip
		MIPS_R6010    = 0x0400, // MIPS R6010 Floating Point Chip
	};

	void set_fpu(u32 revision, unsigned interrupt = 3) { m_fcr0 = revision; m_fpu_irq = interrupt; }

protected:
	mips1_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, u32 cpurev, size_t icache_size, size_t dcache_size, bool cache_pws);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual translate_result translate(int intention, offs_t &address, bool debug) override;

	virtual void handle_cop0(u32 const op) override;
	virtual u32 get_cop0_reg(unsigned const reg) override;
	virtual void set_cop0_reg(unsigned const reg, u32 const data) override;

	virtual void handle_cop1(u32 const op) override;
	template <typename T> void set_cop1_reg(unsigned const reg, T const data);

private:
	u64 m_reset_time;
	u32 m_tlb[64][2]; // 0 is hi, 1 is lo
	unsigned m_tlb_mru[3][64];

	// cop1 registers
	u64 m_f[16];
	u32 m_fcr0;
	u32 m_fcr30;
	u32 m_fcr31;

	unsigned m_fpu_irq;
};

class r2000_device : public mips1_device_base
{
public:
	r2000_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size = 0, size_t dcache_size = 0);
};

class r2000a_device : public mips1_device_base
{
public:
	r2000a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size = 0, size_t dcache_size = 0);
};

class r3000_device : public mips1_device_base
{
public:
	r3000_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size = 0, size_t dcache_size = 0);
};

class r3000a_device : public mips1_device_base
{
public:
	r3000a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size = 0, size_t dcache_size = 0);
};

class r3041_device : public mips1core_device_base
{
public:
	r3041_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class r3051_device : public mips1core_device_base
{
public:
	r3051_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class r3052_device : public mips1core_device_base
{
public:
	r3052_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class r3052e_device : public mips1_device_base
{
public:
	r3052e_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class r3071_device : public mips1_device_base
{
public:
	r3071_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size = 16384, size_t dcache_size = 4096);
};

class r3081_device : public mips1_device_base
{
public:
	r3081_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size = 16384, size_t dcache_size = 4096);
};

class iop_device : public mips1core_device_base
{
public:
	iop_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(R2000,       r2000_device)
DECLARE_DEVICE_TYPE(R2000A,      r2000a_device)
DECLARE_DEVICE_TYPE(R3000,       r3000_device)
DECLARE_DEVICE_TYPE(R3000A,      r3000a_device)
DECLARE_DEVICE_TYPE(R3041,       r3041_device)
DECLARE_DEVICE_TYPE(R3051,       r3051_device)
DECLARE_DEVICE_TYPE(R3052,       r3052_device)
DECLARE_DEVICE_TYPE(R3052E,      r3052e_device)
DECLARE_DEVICE_TYPE(R3071,       r3071_device)
DECLARE_DEVICE_TYPE(R3081,       r3081_device)
DECLARE_DEVICE_TYPE(SONYPS2_IOP, iop_device)

#endif // MAME_CPU_MIPS_MIPS1_H
