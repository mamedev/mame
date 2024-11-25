// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_MIPS_R4000_H
#define MAME_CPU_MIPS_R4000_H

#pragma once

DECLARE_DEVICE_TYPE(R4000, r4000_device)
DECLARE_DEVICE_TYPE(R4400, r4400_device)
DECLARE_DEVICE_TYPE(R4600, r4600_device)
DECLARE_DEVICE_TYPE(R5000, r5000_device)

class r4000_base_device : public cpu_device
{
public:
	enum config_mask : u32
	{
		CONFIG_K0 = 0x00000007, // kseg0 cache coherency
		CONFIG_CU = 0x00000008, // store conditional cache coherent
		CONFIG_DB = 0x00000010, // primary d-cache line 32 bytes
		CONFIG_IB = 0x00000020, // primary i-cache line 32 bytes
		CONFIG_DC = 0x000001c0, // primary d-cache size
		CONFIG_IC = 0x00000e00, // primary i-cache size
		CONFIG_EB = 0x00002000, // sub-block ordering
		CONFIG_EM = 0x00004000, // parity mode enable
		CONFIG_BE = 0x00008000, // big endian
		CONFIG_SM = 0x00010000, // dirty shared disable
		CONFIG_SC = 0x00020000, // secondary cache absent
		CONFIG_EW = 0x000c0000, // system port width
		CONFIG_SW = 0x00100000, // secondary cache port width
		CONFIG_SS = 0x00200000, // split secondary cache mode
		CONFIG_SB = 0x00c00000, // secondary cache line size
		CONFIG_EP = 0x0f000000, // transmit data pattern
		CONFIG_EC = 0x70000000, // system clock ratio
		CONFIG_CM = 0x80000000, // master/checker enable

		CONFIG_WM = 0x0000003f, // runtime-writable bits
	};
	void set_config(u32 data, u32 mem_mask = ~u32(0))
	{
		if (!configured())
			COMBINE_DATA(&m_cp0[CP0_Config]);
	}

	void bus_error() { m_bus_error = true; }

protected:
	enum cache_size
	{
		CACHE_4K   = 0,
		CACHE_8K   = 1,
		CACHE_16K  = 2,
		CACHE_32K  = 3,
		CACHE_64K  = 4,
		CACHE_128K = 5,
		CACHE_256K = 6,
		CACHE_512K = 7,
	};
	r4000_base_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, u32 prid, u32 fcr, cache_size icache_size, cache_size dcache_size, unsigned m32, unsigned m64, unsigned d32, unsigned d64, bool timer_interrupt_disabled);

	enum cp0_reg : int
	{
		CP0_Index    = 0,
		CP0_Random   = 1,
		CP0_EntryLo0 = 2,
		CP0_EntryLo1 = 3,
		CP0_Context  = 4,
		CP0_PageMask = 5,
		CP0_Wired    = 6,
		CP0_BadVAddr = 8,
		CP0_Count    = 9,
		CP0_EntryHi  = 10,
		CP0_Compare  = 11,
		CP0_Status   = 12,
		CP0_Cause    = 13,
		CP0_EPC      = 14,
		CP0_PRId     = 15,
		CP0_Config   = 16,
		CP0_LLAddr   = 17,
		CP0_WatchLo  = 18,
		CP0_WatchHi  = 19,
		CP0_XContext = 20,
		CP0_ECC      = 26,
		CP0_CacheErr = 27,
		CP0_TagLo    = 28,
		CP0_TagHi    = 29,
		CP0_ErrorEPC = 30,
	};

	enum cp0_sr_mask : u32
	{
		SR_IE    = 0x00000001, // interrupt enable
		SR_EXL   = 0x00000002, // exception level
		SR_ERL   = 0x00000004, // error level
		SR_KSU   = 0x00000018, // kernel/supervisor/user mode
		SR_UX    = 0x00000020, // 64-bit addressing user mode
		SR_SX    = 0x00000040, // 64-bit addressing supervisor mode
		SR_KX    = 0x00000080, // 64-bit addressing kernel mode
		SR_IMSW0 = 0x00000100, // software interrupt 0 enable
		SR_IMSW1 = 0x00000200, // software interrupt 1 enable
		SR_IMEX0 = 0x00000400, // external interrupt 0 enable
		SR_IMEX1 = 0x00000800, // external interrupt 1 enable
		SR_IMEX2 = 0x00001000, // external interrupt 2 enable
		SR_IMEX3 = 0x00002000, // external interrupt 3 enable
		SR_IMEX4 = 0x00004000, // external interrupt 4 enable
		SR_IMEX5 = 0x00008000, // external interrupt 5 enable
		SR_DE    = 0x00010000, // disable cache parity/ecc exceptions
		SR_CE    = 0x00020000, // cache ecc check enable
		SR_CH    = 0x00040000, // cache hit
		SR_SR    = 0x00100000, // soft reset
		SR_TS    = 0x00200000, // tlb shutdown (read only)
		SR_BEV   = 0x00400000, // bootstrap exception vectors
		SR_RE    = 0x02000000, // reverse endian
		SR_FR    = 0x04000000, // enable additional floating-point registers
		SU_RP    = 0x08000000, // reduced power
		SR_CU0   = 0x10000000, // coprocessor usable 0
		SR_CU1   = 0x20000000, // coprocessor usable 1
		SR_CU2   = 0x40000000, // coprocessor usable 2
		SR_CU3   = 0x80000000, // coprocessor usable 3

		SR_IMSW  = 0x00000300,
		SR_IM    = 0x0000ff00, // interrupt mask
		SR_DS    = 0x01ff0000, // diagnostic status
	};

	enum cp0_sr_ksu_mode : u32
	{
		SR_KSU_K = 0x00000000, // kernel mode
		SR_KSU_S = 0x00000008, // supervisor mode
		SR_KSU_U = 0x00000010, // user mode
	};

	enum cp0_cause_mask : u32
	{
		CAUSE_EXCCODE = 0x0000007c, // exception code
		CAUSE_IPSW0   = 0x00000100, // software interrupt 0 pending
		CAUSE_IPSW1   = 0x00000200, // software interrupt 1 pending
		CAUSE_IPEX0   = 0x00000400, // external interrupt 0 pending
		CAUSE_IPEX1   = 0x00000800, // external interrupt 1 pending
		CAUSE_IPEX2   = 0x00001000, // external interrupt 2 pending
		CAUSE_IPEX3   = 0x00002000, // external interrupt 3 pending
		CAUSE_IPEX4   = 0x00004000, // external interrupt 4 pending
		CAUSE_IPEX5   = 0x00008000, // external interrupt 5 pending
		CAUSE_CE      = 0x30000000, // coprocessor unit
		CAUSE_BD      = 0x80000000, // branch delay slot

		CAUSE_IPSW    = 0x00000300,
		CAUSE_IPHW    = 0x0000fc00,
		CAUSE_IP      = 0x0000ff00,
	};

	enum cp0_watchlo_mask : u32
	{
		WATCHLO_W      = 0x00000001, // trap on store
		WATCHLO_R      = 0x00000002, // trap on load
		WATCHLO_PADDR0 = 0xfffffff8, // physical address bits 31:3
	};
	enum cp0_watchhi_mask : u32
	{
		WATCHHI_PADDR1 = 0x0000000f, // physical address bits 35:32
	};

	enum cp0_tlb_mask : u64
	{
		TLB_MASK = 0x0000'0000'01ff'e000,
	};
	enum cp0_tlb_eh : u64
	{
		EH_ASID    = 0x0000'0000'0000'00ff, // address space id
		EH_G       = 0x0000'0000'0000'1000, // global (tlb only)
		EH_VPN2_32 = 0x0000'0000'ffff'e000, // virtual page number (32-bit mode)
		EH_VPN2_64 = 0x0000'00ff'ffff'e000, // virtual page number (64-bit mode)
		EH_R       = 0xc000'0000'0000'0000, // region (64-bit mode)

		EH_WM      = 0xc000'00ff'ffff'e0ff, // write mask
	};
	enum cp0_tlb_el : u64
	{
		EL_G   = 0x0000'0000'0000'0001, // global (entrylo only)
		EL_V   = 0x0000'0000'0000'0002, // valid
		EL_D   = 0x0000'0000'0000'0004, // dirty
		EL_C   = 0x0000'0000'0000'0038, // coherency
		EL_PFN = 0x0000'0000'3fff'ffc0, // page frame number

		EL_WM  = 0x0000'0000'3fff'fffe, // write mask
	};
	enum cp0_tlb_el_c : u64
	{
		C_0 = 0x00, // reserved
		C_1 = 0x08, // reserved
		C_2 = 0x10, // uncached
		C_3 = 0x18, // cacheable noncoherent (noncoherent)
		C_4 = 0x20, // cacheable coherent exclusive (exclusive)
		C_5 = 0x28, // cacheable coherent exclusive on write (sharable)
		C_6 = 0x30, // cacheable coherent update on write (update)
		C_7 = 0x38, // reserved
	};

	enum cp0_context_mask : u64
	{
		CONTEXT_PTEBASE = 0xffff'ffff'ff80'0000,
		CONTEXT_BADVPN2 = 0x0000'0000'007f'fff0,
	};

	enum cp0_xcontext_mask : u64
	{
		XCONTEXT_PTEBASE = 0xffff'fffe'0000'0000, // page table entry base
		XCONTEXT_R       = 0x0000'0001'8000'0000, // region
		XCONTEXT_BADVPN2 = 0x0000'0000'7fff'fff0, // bad virtual page number / 2
	};

	enum cp0_pagemask_mask : u32
	{
		PAGEMASK = 0x01ff'e000,
	};

	enum exception_mask : u32
	{
		EXCEPTION_INT   = 0x00000000, // interrupt
		EXCEPTION_MOD   = 0x00000004, // tlb modification
		EXCEPTION_TLBL  = 0x00000008, // tlb load
		EXCEPTION_TLBS  = 0x0000000c, // tlb store
		EXCEPTION_ADEL  = 0x00000010, // address error load
		EXCEPTION_ADES  = 0x00000014, // address error store
		EXCEPTION_IBE   = 0x00000018, // bus error (instruction fetch)
		EXCEPTION_DBE   = 0x0000001c, // bus error (data reference: load or store)
		EXCEPTION_SYS   = 0x00000020, // syscall
		EXCEPTION_BP    = 0x00000024, // breakpoint
		EXCEPTION_RI    = 0x00000028, // reserved instruction
		EXCEPTION_CPU   = 0x0000002c, // coprocessor unusable
		EXCEPTION_OV    = 0x00000030, // arithmetic overflow
		EXCEPTION_TR    = 0x00000034, // trap
		EXCEPTION_VCEI  = 0x00000038, // virtual coherency exception instruction
		EXCEPTION_FPE   = 0x0000003c, // floating point
		EXCEPTION_WATCH = 0x0000005c, // reference to watchhi/watchlo address
		EXCEPTION_VCED  = 0x0000007c, // virtual coherency exception data

		EXCEPTION_CP0   = 0x0000002c, // coprocessor 0 unusable
		EXCEPTION_CP1   = 0x1000002c, // coprocessor 1 unusable
		EXCEPTION_CP2   = 0x2000002c, // coprocessor 2 unusable
		EXCEPTION_CP3   = 0x3000002c, // coprocessor 3 unusable
	};

	enum cp1_fcr31_mask : u32
	{
		FCR31_RM = 0x00000003, // rounding mode

		FCR31_FI = 0x00000004, // inexact operation flag
		FCR31_FU = 0x00000008, // underflow flag
		FCR31_FO = 0x00000010, // overflow flag
		FCR31_FZ = 0x00000020, // divide by zero flag
		FCR31_FV = 0x00000040, // invalid operation flag

		FCR31_EI = 0x00000080, // inexact operation enable
		FCR31_EU = 0x00000100, // underflow enable
		FCR31_EO = 0x00000200, // overflow enable
		FCR31_EZ = 0x00000400, // divide by zero enable
		FCR31_EV = 0x00000800, // invalid operation enable

		FCR31_CI = 0x00001000, // inexact operation cause
		FCR31_CU = 0x00002000, // underflow cause
		FCR31_CO = 0x00004000, // overflow cause
		FCR31_CZ = 0x00008000, // divide by zero cause
		FCR31_CV = 0x00010000, // invalid operation cause
		FCR31_CE = 0x00020000, // unimplemented operation cause

		FCR31_C  = 0x00800000, // condition
		FCR31_FS = 0x01000000, // flush denormalized results

		FCR31_FM = 0x0000007c, // flag mask
		FCR31_EM = 0x00000f80, // enable mask
		FCR31_CM = 0x0001f000, // cause mask (except unimplemented)
	};

	enum mips3_registers : unsigned
	{
		MIPS3_R0  = 0,
		MIPS3_CP0 = 32,
		MIPS3_F0  = 64,

		MIPS3_PC  = 96,
		MIPS3_HI,
		MIPS3_LO,
		MIPS3_FCR30,
		MIPS3_FCR31,
	};

	enum cp0_taglo_mask : u32
	{
		TAGLO_PTAGLO = 0xffffff00, // physical adddress bits 35:12
		TAGLO_PSTATE = 0x000000c0, // primary cache state
		TAGLO_P      = 0x00000001, // primary tag even parity
		TAGLO_CS     = 0x00001c00, // scache status
		TAGLO_STAG   = 0xffffe000, // scache tag
	};
	enum icache_mask : u32
	{
		ICACHE_PTAG = 0x00ffffff, // physical tag
		ICACHE_V    = 0x01000000, // valid
		ICACHE_P    = 0x02000000, // even parity
	};
	enum dcache_mask : u32
	{
		DCACHE_PTAG = 0x00ffffff, // physical tag
		DCACHE_CS   = 0x01000000, // primary cache state
		DCACHE_P    = 0x02000000, // even parity for ptag and cs
		DCACHE_W    = 0x02000000, // write-back
		DCACHE_WP   = 0x02000000, // even parity for write-back
	};
	enum scache_mask : u32
	{
		SCACHE_CS   = 0x01c00000, // cache state
		SCACHE_STAG = 0x0007ffff, // physical tag
		SCACHE_PIDX = 0x00380000, // primary cache index
	};

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks * 2); }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles + 1) / 2; }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return *std::max_element(std::begin(m_hilo_cycles), std::end(m_hilo_cycles)); }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// cpu implementation
	virtual void handle_reserved_instruction(u32 const op);
	void cpu_execute(u32 const op);
	void cpu_exception(u32 exception, u16 const vector = 0x180);
	void cpu_lwl(u32 const op);
	void cpu_lwr(u32 const op);
	void cpu_ldl(u32 const op);
	void cpu_ldr(u32 const op);
	void cpu_swl(u32 const op);
	void cpu_swr(u32 const op);
	void cpu_sdl(u32 const op);
	void cpu_sdr(u32 const op);

	// cp0 implementation
	void cp0_cache(u32 const op);
	void cp0_execute(u32 const op);
	u64 cp0_get(unsigned const reg);
	void cp0_set(unsigned const reg, u64 const data);
	void cp0_tlbr();
	void cp0_tlbwi(u8 const index);
	void cp0_tlbwr();
	void cp0_tlbp();

	// cp0 helpers
	TIMER_CALLBACK_MEMBER(cp0_timer_callback);
	void cp0_update_timer(bool start = false);
	bool cp0_64() const;

	// cp1 implementation
	void cp1_unimplemented();
	template <typename T> bool cp1_op(T op);
	virtual void cp1_execute(u32 const op);
	virtual void cp1x_execute(u32 const op);
	template <typename T> void cp1_set(unsigned const reg, T const data);
	void cp1_mov_s(u32 const op);
	void cp1_mov_d(u32 const op);

	// cp2 implementation
	void cp2_execute(u32 const op);

	// address and memory handling
	enum translate_result : unsigned { ERROR, MISS, UNCACHED, CACHED };
	translate_result translate(int intention, bool debug, u64 &address);
	void address_error(int intention, u64 const address);

	template <typename T> void accessors(T &m);
	template <typename T, bool Aligned = true, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, bool> load(u64 program_address, U &&apply);
	template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(u64, T)>>::value, bool> load_linked(u64 program_address, U &&apply);
	template <typename T, bool Aligned = true, typename U> std::enable_if_t<std::is_convertible<U, T>::value, bool> store(u64 program_address, U data, T mem_mask = ~T(0));
	bool fetch(u64 address, std::function<void(u32)> &&apply);

	// debugging helpers
	std::string debug_string(u64 string_pointer, unsigned limit = 0);
	std::string debug_string_array(u64 array_pointer);
	std::string debug_unicode_string(u64 unicode_string_pointer);

	// configuration helpers
	void configure_scache();

	// device configuration state
	address_space_config m_program_config_le;
	address_space_config m_program_config_be;

	// memory access helpers
	memory_access<32, 3, 0, ENDIANNESS_LITTLE>::specific m_le;
	memory_access<32, 3, 0, ENDIANNESS_BIG>::specific m_be;

	std::function<u8(offs_t offset)> read_byte;
	std::function<u16(offs_t offset)> read_word;
	std::function<u32(offs_t offset)> read_dword;
	std::function<u64(offs_t offset)> read_qword;
	std::function<void(offs_t offset, u8 data)> write_byte;
	std::function<void(offs_t offset, u16 data, u16 mem_mask)> write_word;
	std::function<void(offs_t offset, u32 data, u32 mem_mask)> write_dword;
	std::function<void(offs_t offset, u64 data, u64 mem_mask)> write_qword;

	enum branch_state : u64
	{
		STATE   = 0x00000000'00000003,
		TARGET  = 0xffffffff'fffffffc,

		NONE    = 0,
		BRANCH  = 1, // retire delayed branch
		DELAY   = 2, // delay slot instruction
		NULLIFY = 3, // next instruction nullified
	};

	// runtime state
	int m_icount;

	// integer multiple/divide state
	unsigned const m_hilo_cycles[4];
	unsigned m_hilo_delay;

	// cpu state
	u64 m_pc;
	u64 m_r[32];
	u64 m_hi;
	u64 m_lo;
	u64 m_branch_state;

	// cp0 state
	u64 m_cp0[32];
	u64 m_cp0_timer_zero;
	emu_timer *m_cp0_timer;
	bool m_hard_reset;
	bool m_ll_active;
	bool m_bus_error;
	bool m_timer_interrupt_disabled;
	struct tlb_entry
	{
		u64 mask;
		u64 vpn;
		u64 pfn[2];

		unsigned low_bit;
	}
	m_tlb[48];
	unsigned m_tlb_mru[3][48];

	// cp1 state
	u64 m_f[32]; // floating point registers
	u32 m_fcr0;  // implementation and revision register
	u32 m_fcr30; // unknown
	u32 m_fcr31; // control/status register

	// experimental icache state
	u32 m_icache_mask_hi;
	u32 m_icache_mask_lo;
	unsigned m_icache_line_size;
	unsigned m_icache_shift;
	std::unique_ptr<u32[]> m_icache_tag;
	std::unique_ptr<u32[]> m_icache_data;

	// experimental scache state
	u32 m_scache_size; // Size in bytes
	u8 m_scache_line_size;
	u32 m_scache_line_index; // Secondary cache line shift
	u32 m_scache_tag_mask; // Mask for extracting the tag from a physical address
	u32 m_scache_tag_size;
	std::unique_ptr<u32[]> m_scache_tag;

	// statistics
	u64 m_tlb_scans;
	u64 m_tlb_loops;
	u64 m_icache_hits;
	u64 m_icache_misses;
};

class r4000_device : public r4000_base_device
{
public:
	// NOTE: R4000 chips prior to 3.0 have an xtlb bug
	r4000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, bool timer_interrupt_disabled)
		: r4000_base_device(mconfig, R4000, tag, owner, clock, 0x0430, 0x0500, CACHE_8K, CACHE_8K, 10, 20, 69, 133, timer_interrupt_disabled)
	{
		// no secondary cache
		m_cp0[CP0_Config] |= CONFIG_SC;
	}

	r4000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: r4000_device(mconfig, tag, owner, clock, false)
	{
	}
};

class r4400_device : public r4000_base_device
{
public:
	r4400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, bool timer_interrupt_disabled, u32 scache_size, u8 scache_line_size)
		: r4000_base_device(mconfig, R4400, tag, owner, clock, 0x0440, 0x0500, CACHE_16K, CACHE_16K, 10, 20, 69, 133, timer_interrupt_disabled)
	{
		m_scache_size = scache_size;
		m_scache_line_size = scache_line_size;
		configure_scache();
	}

	r4400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: r4400_device(mconfig, tag, owner, clock, false, 0, 0)
	{
	}
};

class r4600_device : public r4000_base_device
{
public:
	r4600_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, bool timer_interrupt_disabled)
		: r4000_base_device(mconfig, R4600, tag, owner, clock, 0x2020, 0x2020, CACHE_16K, CACHE_16K, 10, 12, 42, 74, timer_interrupt_disabled)
	{
		// no secondary cache
		m_cp0[CP0_Config] |= CONFIG_SC;
	}

	r4600_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: r4600_device(mconfig, tag, owner, clock, false)
		{
		}
};

class r5000_device : public r4000_base_device
{
public:
	r5000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, bool timer_interrupt_disabled)
		: r4000_base_device(mconfig, R5000, tag, owner, clock, 0x2320, 0x2320, CACHE_32K, CACHE_32K, 5, 9, 36, 68, timer_interrupt_disabled)
	{
		// no secondary cache
		m_cp0[CP0_Config] |= CONFIG_SC;
	}

	r5000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: r5000_device(mconfig, tag, owner, clock, false)
	{
	}

private:
	virtual void handle_reserved_instruction(u32 const op) override;
	virtual void cp1_execute(u32 const op) override;
	virtual void cp1x_execute(u32 const op) override;

	static u32 const s_fcc_masks[8];
	static u32 const s_fcc_shifts[8];
};
#endif // MAME_CPU_MIPS_R4000_H
