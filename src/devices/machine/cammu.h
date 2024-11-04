// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_CAMMU_H
#define MAME_MACHINE_CAMMU_H

#pragma once

#include "cpu/clipper/common.h"

class cammu_device : public device_t
{
public:
	auto exception_callback() { return m_exception_func.bind(); }

	static const u32 CAMMU_PAGE_SIZE = 0x1000;
	static const u32 CAMMU_PAGE_MASK = (CAMMU_PAGE_SIZE - 1);

	enum pdo_mask : u32
	{
		PDO_MASK = 0xfffff000
	};

	enum ptde_mask : u32
	{
		PTDE_F   = 0x00000001, // page fault
		PTDE_PTO = 0xfffff000  // page table origin
	};

	enum pte_mask : u32
	{
		PTE_F     = 0x00000001, // page fault
		PTE_R     = 0x00000002, // referenced flag
		PTE_D     = 0x00000004, // dirty flag
		PTE_PL    = 0x00000078, // protection level
		PTE_S     = 0x00000180, // system reserved
		PTE_ST    = 0x00000e00, // system tag
		PTE_RA    = 0xfffff000, // real address

		PTE_CW    = 0x00000040, // copy on write (c400)
		PTE_NDREF = 0x00000080, // secondary reference (software) / copy on write (fault)?
		PTE_LOCK  = 0x00000100  // page lock (software)
	};

	static constexpr int PL_SHIFT = 3;
	static constexpr int ST_SHIFT = 9;

	enum va_mask : u32
	{
		VA_POFS = 0x00000fff, // page offset
		VA_PTI  = 0x003ff000, // page table index
		VA_PTDI = 0xffc00000  // page table directory index
	};

	enum system_tag_t : u8
	{
		ST0 = 0, // private, write-through, main memory space
		ST1 = 1, // shared, write-through, main memory space
		ST2 = 2, // private, copy-back, main memory space
		ST3 = 3, // noncacheable, main memory space
		ST4 = 4, // noncacheable, i/o space
		ST5 = 5, // noncacheable, boot space
		ST6 = 6, // cache purge
		ST7 = 7  // slave i/o
	};

	void set_spaces(address_space &main_space, address_space &io_space, address_space &boot_space);

	// translation lookaside buffer and register access
	virtual u32 cammu_r(const u32 address) = 0;
	virtual void cammu_w(const u32 address, const u32 data) = 0;

	template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, bool> load(const u32 ssw, const u32 address, U &&apply)
	{
		// check for cammu access
		if ((ssw & (SSW_UU | SSW_U)) || ((address & ~0x7ff) != 0x00004800))
		{
			translated_t t = translate_address(ssw, address, access_size(sizeof(T)), READ);

			if (!t.cache)
				return false;

			switch (sizeof(T))
			{
			case 1: apply(T(t.cache->read_byte(t.address))); break;
			case 2: apply(T(t.cache->read_word(t.address))); break;
			case 4: apply(T(t.cache->read_dword(t.address))); break;
			case 8: apply(T(t.cache->read_qword(t.address))); break;
			default:
				fatalerror("unhandled load 0x%08x size %d (%s)",
					address, access_size(sizeof(T)), machine().describe_context().c_str());
			}
		}
		else if (sizeof(T) == 4)
			apply(cammu_r(address));
		else
			fatalerror("unhandled cammu load 0x%08x size %d (%s)",
				address, access_size(sizeof(T)), machine().describe_context().c_str());

		return true;
	}

	template <typename T, typename U> std::enable_if_t<std::is_convertible<U, T>::value, bool> store(const u32 ssw, const u32 address, U data)
	{
		// check for cammu access
		if ((ssw & (SSW_UU | SSW_U)) || ((address & ~0x7ff) != 0x00004800))
		{
			translated_t t = translate_address(ssw, address, access_size(sizeof(T)), WRITE);

			if (!t.cache)
				return false;

			switch (sizeof(T))
			{
			case 1: t.cache->write_byte(t.address, T(data)); break;
			case 2: t.cache->write_word(t.address, T(data)); break;
			case 4: t.cache->write_dword(t.address, T(data)); break;
			case 8: t.cache->write_qword(t.address, T(data)); break;
			default:
				fatalerror("unhandled store 0x%08x size %d (%s)",
					address, access_size(sizeof(T)), machine().describe_context().c_str());
			}
		}
		else if (sizeof(T) == 4)
			cammu_w(address, data);
		else
			fatalerror("unhandled cammu store 0x%08x size %d (%s)",
				address, access_size(sizeof(T)), machine().describe_context().c_str());

		return true;
	}

	template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<T(T)>>::value, bool> modify(const u32 ssw, const u32 address, U &&apply)
	{
		translated_t t = translate_address(ssw, address, access_size(sizeof(T)), access_type(READ | WRITE));

		if (!t.cache)
			return false;

		switch (sizeof(T))
		{
		case 4: t.cache->write_dword(t.address, apply(T(t.cache->read_dword(t.address)))); break;
		default:
			fatalerror("unhandled modify 0x%08x size %d (%s)",
				address, access_size(sizeof(T)), machine().describe_context().c_str());
		}

		return true;
	}

	template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, bool> fetch(const u32 ssw, const u32 address, U &&apply)
	{
		translated_t t = translate_address(ssw, address, access_size(sizeof(T)), EXECUTE);

		if (!t.cache)
			return false;

		switch (sizeof(T))
		{
		case 2: apply(T(t.cache->read_word(t.address))); break;
		case 4:
			{
				// check for unaligned access
				if (address & 0x2)
				{
					// check for page span
					if ((address & CAMMU_PAGE_MASK) == (CAMMU_PAGE_SIZE - 2))
					{
						translated_t u = translate_address(ssw, address + 2, access_size(sizeof(u16)), EXECUTE);
						if (u.cache)
						{
							const u16 lsw = t.cache->read_word(t.address);
							const u16 msw = t.cache->read_word(u.address);

							apply((T(msw) << 16) | lsw);
						}
						else
							return false;
					}
					else
						apply(T(t.cache->read_dword_unaligned(t.address)));
				}
				else
					apply(T(t.cache->read_dword(t.address)));
			}
		break;
		default:
			fatalerror("unhandled fetch 0x%08x size %d (%s)\n",
				address, access_size(sizeof(T)), machine().describe_context().c_str());
		}

		return true;
	}

	// address translation for debugger
	bool memory_translate(const u32 ssw, const int spacenum, const int intention, offs_t &address, address_space *&target_space);

protected:
	cammu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	enum access_size : u8
	{
		BYTE  = 1,
		WORD  = 2,
		DWORD = 4,
		QWORD = 8
	};

	enum access_type : u8
	{
		READ    = 1,
		WRITE   = 2,
		EXECUTE = 4,

		// matrix abbreviations and combinations
		N       = 0,
		R       = READ,
		W       = WRITE,
		RW      = READ | WRITE,
		RE      = READ | EXECUTE,
		RWE     = READ | WRITE | EXECUTE,
	};

	struct translated_t
	{
		memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache *const cache;
		const u32 address;
	};

	struct pte_t
	{
		u32 entry;
		u32 address;
	};

	struct memory_t
	{
		address_space *space = nullptr;
		memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache cache;
	};

	// address translation
	virtual translated_t translate_address(const u32 ssw, const u32 virtual_address, const access_size size, const access_type mode);
	pte_t get_pte(const u32 va, const bool user);

	// helpers
	virtual bool get_access(const access_type mode, const u32 pte, const u32 ssw) const = 0;
	virtual bool get_alignment() const = 0;
	virtual u32 get_pdo(const bool user) const = 0;
	virtual system_tag_t get_ust_space() const = 0;
	virtual void set_fault(const u32 address, const exception_vector type) = 0;

	// device state
	devcb_write16 m_exception_func;
	memory_t m_memory[8];
};

class cammu_c4_device : public cammu_device
{
public:
	// TODO: translation lookaside buffer and register access
	virtual void map(address_map &map) = 0;
	virtual u32 cammu_r(const u32 address) override { return 0; }
	virtual void cammu_w(const u32 address, const u32 data) override {}

	void set_cammu_id(const u32 cammu_id) { m_control = cammu_id; }

	u32 s_pdo_r() { return m_s_pdo; }
	void s_pdo_w(offs_t offset, u32 data, u32 mem_mask = ~0) { m_s_pdo = ((m_s_pdo & ~mem_mask) | (data & mem_mask)) & PDO_MASK; }
	u32 u_pdo_r() { return m_u_pdo; }
	void u_pdo_w(offs_t offset, u32 data, u32 mem_mask = ~0) { m_u_pdo = ((m_u_pdo & ~mem_mask) | (data & mem_mask)) & PDO_MASK; }

	virtual u32 control_r() = 0;
	virtual void control_w(offs_t offset, u32 data, u32 mem_mask = ~0) = 0;

	u32 i_fault_r() { return m_i_fault; }
	void i_fault_w(u32 data) { m_i_fault = data; }
	u32 fault_address_1_r() { return m_fault_address_1; }
	void fault_address_1_w(u32 data) { m_fault_address_1 = data; }
	u32 fault_address_2_r() { return m_fault_address_2; }
	void fault_address_2_w(u32 data) { m_fault_address_2 = data; }
	u32 fault_data_1_lo_r() { return m_fault_data_1_lo; }
	void fault_data_1_lo_w(u32 data) { m_fault_data_1_lo = data; }
	u32 fault_data_1_hi_r() { return m_fault_data_1_hi; }
	void fault_data_1_hi_w(u32 data) { m_fault_data_1_hi = data; }
	u32 fault_data_2_lo_r() { return m_fault_data_2_lo; }
	void fault_data_2_lo_w(u32 data) { m_fault_data_2_lo = data; }
	u32 fault_data_2_hi_r() { return m_fault_data_2_hi; }
	void fault_data_2_hi_w(u32 data) { m_fault_data_2_hi = data; }

protected:
	cammu_c4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	virtual bool get_access(const access_type mode, const u32 pte, const u32 ssw) const override;
	virtual u32 get_pdo(const bool user) const override { return user ? m_u_pdo : m_s_pdo; }

	virtual void set_fault(const u32 address, const exception_vector type) override { m_fault_address_1 = address; m_exception_func(type); }

	u32 m_s_pdo;
	u32 m_u_pdo;
	u32 m_control;

	u32 m_i_fault;
	u32 m_fault_address_1;
	u32 m_fault_address_2;
	u32 m_fault_data_1_lo;
	u32 m_fault_data_1_hi;
	u32 m_fault_data_2_lo;
	u32 m_fault_data_2_hi;
};

class cammu_c4t_device : public cammu_c4_device
{
public:
	cammu_c4t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;

	u32 ram_line_r() { return m_ram_line; }
	void ram_line_w(u32 data) { m_ram_line = data; }

	u32 htlb_offset_r() { return m_htlb_offset; }
	void htlb_offset_w(u32 data) { m_htlb_offset = data; }

	u32 c4_bus_poll_r() { return m_c4_bus_poll; }
	void c4_bus_poll_w(u32 data) { m_c4_bus_poll = data; }

	enum control_mask : u32
	{
		CNTL_RUV   = 0x00000001, // reset user valid
		CNTL_RSV   = 0x00000002, // reset supervisor valid
		CNTL_DBWR  = 0x00000004, // disable bus watch read
		CNTL_ATD   = 0x00000008, // alignment trap disable
		CNTL_UST   = 0x00000030, // unmapped system tag
		CNTL_IOTS  = 0x00000040, // i/o tag select
		CNTL_UVS   = 0x00000080, // user valid status
		CNTL_PB    = 0x00000100, // purge busy
		CNTL_CICT  = 0x00000200, // clear i-side cache tags
		CNTL_CFR   = 0x00000400, // clear trap registers
		CNTL_HTLBD = 0x00000800, // htlb disable
		CNTL_CDCT  = 0x00001000, // clear d-side cache tags
		CNTL_CID   = 0xff000000  // cammu id
	};

	enum control_ust_mask : u32
	{
		UST_NCA = 0x00, // unmapped system tag, noncacheable
		UST_PWT = 0x10, // unmapped system tag, write through
		UST_PCB = 0x20, // unmapped system tag, copy back
		UST_PGE = 0x30  // unmapped system tag, purge mode
	};

	enum control_cid_mask : u32
	{
		CID_C4T = 0x00000000 // unknown
	};

	virtual u32 control_r() override { return m_control; }
	virtual void control_w(offs_t offset, u32 data, u32 mem_mask = ~0) override { m_control = ((m_control & (~mem_mask | CNTL_CID)) | (data & (mem_mask & ~CNTL_CID))); }
	u32 bio_control_r() { return m_bio_control; }
	void bio_control_w(u32 data) { m_bio_control = data; }
	u32 bio_address_tag_r() { return m_bio_address_tag; }
	void bio_address_tag_w(u32 data) { m_bio_address_tag = data; }

	u32 cache_data_lo_r() { return m_cache_data_lo; }
	void cache_data_lo_w(u32 data) { m_cache_data_lo = data; }
	u32 cache_data_hi_r() { return m_cache_data_hi; }
	void cache_data_hi_w(u32 data) { m_cache_data_hi = data; }
	u32 cache_cpu_tag_r() { return m_cache_cpu_tag; }
	void cache_cpu_tag_w(u32 data) { m_cache_cpu_tag = data; }
	u32 cache_system_tag_valid_r() { return m_cache_system_tag_valid; }
	void cache_system_tag_valid_w(u32 data) { m_cache_system_tag_valid = data; }
	u32 cache_system_tag_r() { return m_cache_system_tag; }
	void cache_system_tag_w(u32 data) { m_cache_system_tag = data; }
	u32 tlb_va_line_r() { return m_tlb_va_line; }
	void tlb_va_line_w(u32 data) { m_tlb_va_line = data; }
	u32 tlb_ra_line_r() { return m_tlb_ra_line; }
	void tlb_ra_line_w(u32 data) { m_tlb_ra_line = data; }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual bool get_alignment() const override { return (m_control & CNTL_ATD) == 0; }
	virtual system_tag_t get_ust_space() const override { return system_tag_t((m_control & (CNTL_IOTS | CNTL_UST)) >> 4); }

private:
	u32 m_ram_line;
	u32 m_htlb_offset;
	u32 m_c4_bus_poll;
	u32 m_bio_control;
	u32 m_bio_address_tag;

	u32 m_cache_data_lo;
	u32 m_cache_data_hi;
	u32 m_cache_cpu_tag;
	u32 m_cache_system_tag_valid;
	u32 m_cache_system_tag;
	u32 m_tlb_va_line;
	u32 m_tlb_ra_line;
};

class cammu_c4i_device : public cammu_c4_device
{
public:
	cammu_c4i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;

	enum control_mask : u32
	{
		CNTL_LRAS = 0x00000001, // tlb line replacement
		CNTL_BWWD = 0x00000002, // buswatch write disable
		CNTL_BWRD = 0x00000004, // buswatch read disable
		CNTL_FSR  = 0x00000010, // fake system response
		CNTL_ATD  = 0x00000100, // alignment trap disable
		CNTL_UMM  = 0x00003000, // unmapped mode address space select
		CNTL_POLL = 0x00030000, // poll bus signals
		CNTL_BM   = 0x00040000, // burst mode address space select
		CNTL_PZBS = 0x00080000, // page 0 boot select
		CNTL_CRR  = 0x00700000, // cache memory refresh rate
		CNTL_CID  = 0xff000000  // cammu identification
	};

	enum control_umm_mask : u32
	{
		UMM_MM    = 0x00000000, // mm space, noncacheable
		UMM_MMRIO = 0x00001000, // mm or i/o space, noncacheable
		UMM_IO    = 0x00002000  // i/o space noncacheable
	};

	enum control_crr_mask : u32
	{
		CRR_GT131  = 0x00000000, // clock rate over 131 MHz
		CRR_GT66   = 0x00100000, // clock rate over 66 MHz
		CRR_GT33   = 0x00200000, // clock rate over 33 MHz
		CRR_GT8    = 0x00300000, // clock rate over 8 MHz
		CRR_GT2    = 0x00400000, // clock rate over 2 MHz
		CRR_GT1    = 0x00500000, // clock rate over 1 MHz
		CRR_GTHALF = 0x00600000, // clock rate over 0.5 MHz
		CRR_OFF    = 0x00700000, // refresh off
	};

	// c4i cammu identification (rev 2 and rev 3 known to have existed)
	enum control_cid_mask : u32
	{
		CID_C4IR0 = 0x00000000,
		CID_C4IR2 = 0x02000000
	};

	virtual u32 control_r() override { return m_control; }
	virtual void control_w(offs_t offset, u32 data, u32 mem_mask) override { m_control = ((m_control & (~mem_mask | CNTL_CID)) | (data & (mem_mask & ~CNTL_CID))); }

	enum reset_mask : u32
	{
		RESET_CDCT  = 0x00000001, // clear data cache tags
		RESET_RDUV  = 0x00000100, // reset all d-side uv flags
		RESET_RDSV  = 0x00001000, // reset all d-side sv flags
		RESET_CICT  = 0x00010000, // clear ins. cache tags
		RESET_RIUV  = 0x01000000, // reset all i-side uv flags
		RESET_RISV  = 0x10000000, // reset all i-side sv flags
		RESET_FLUSH = 0x40000000, // flush out burst io buffer
		RESET_CFR   = 0x80000000  // clear fault registers
	};
	u32 reset_r() { return m_reset; }
	void reset_w(u32 data) { m_reset = data; }

	u32 clr_s_data_tlb_r() { return m_clr_s_data_tlb; }
	void clr_s_data_tlb_w(u32 data) { m_clr_s_data_tlb = data; }
	u32 clr_u_data_tlb_r() { return m_clr_u_data_tlb; }
	void clr_u_data_tlb_w(u32 data) { m_clr_u_data_tlb = data; }
	u32 clr_s_insn_tlb_r() { return m_clr_s_insn_tlb; }
	void clr_s_insn_tlb_w(u32 data) { m_clr_s_insn_tlb = data; }
	u32 clr_u_insn_tlb_r() { return m_clr_u_insn_tlb; }
	void clr_u_insn_tlb_w(u32 data) { m_clr_u_insn_tlb = data; }

	u32 test_data_r() { return m_test_data; }
	void test_data_w(u32 data) { m_test_data = data; }

	u32 test_address_r() { return m_test_address; }
	void test_address_w(u32 data) { m_test_address = data; }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual bool get_alignment() const override { return (m_control & CNTL_ATD) == 0; }
	// FIXME: don't really know how unmapped mode works on c4i
	virtual system_tag_t get_ust_space() const override { return (m_control & UMM_IO) ? ST4 : ST3; }

private:
	u32 m_reset;
	u32 m_clr_s_data_tlb;
	u32 m_clr_u_data_tlb;
	u32 m_clr_s_insn_tlb;
	u32 m_clr_u_insn_tlb;
	u32 m_test_data;
	u32 m_test_address;
};

class cammu_c3_device : public cammu_device
{
public:
	cammu_c3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void add_linked(cammu_c3_device *child) { m_linked.push_back(child); }

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// translation lookaside buffer and register access
	virtual u32 cammu_r(const u32 address) override;
	virtual void cammu_w(const u32 address, const u32 data) override;

	// address translation
	virtual translated_t translate_address(const u32 ssw, const u32 virtual_address, const access_size size, const access_type mode) override;

private:
	enum cammu_address_mask : u32
	{
		CAMMU_TLB_VA  = 0x00000001, // tlb va/ra select
		CAMMU_TLB_X   = 0x00000002, // tlb x/w line select
		CAMMU_TLB_SET = 0x000000fc, // tlb set select
		CAMMU_REG     = 0x000000ff, // register select
		CAMMU_SELECT  = 0x00000700, // cammu select
	};
	enum tlb_ra_mask : u32
	{
		TLB_RA_U  = 0x00000001, // used flag
		TLB_RA_R  = 0x00000002, // referenced flag
		TLB_RA_D  = 0x00000004, // dirty flag
		TLB_RA_PL = 0x00000078, // protection level
		TLB_RA_ST = 0x00000e00, // system tag
		TLB_RA_RA = 0xfffff000, // real address
	};
	enum tlb_va_mask : u32
	{
		TLB_VA_UV = 0x00000002, // user valid flag
		TLB_VA_SV = 0x00000004, // supervisor valid flag
		TLB_VA_VA = 0xfffc0000, // virtual address tag
	};

	/*
	* The C1/C3 CAMMU has 64-entry, two-way set associative TLB, with lines
	* grouped into W and X compartments. The associated U flag is set to
	* indicate that the W line of the set was most recently accessed, and
	* cleared when the X line was most recently accessed. On TLB miss, the
	* least recently used line as indicated by this flag is replaced.
	*
	* Each line consists of a real address field and a virtual address field.
	* The real address field format is practically identical to the page table
	* entry format.
	*/
	struct tlb_line_t
	{
		u32 ra; // real address field
		u32 va; // virtual address field

		memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache cache;
	};
	struct tlb_set_t
	{
		tlb_line_t w;
		tlb_line_t x;
		bool u;
	};

	enum cammu_select_mask : u32
	{
		CAMMU_D_TLB = 0x000, // d-cammu tlb
		CAMMU_D_REG = 0x100, // d-cammu register
		CAMMU_I_TLB = 0x200, // i-cammu tlb
		CAMMU_I_REG = 0x300, // i-cammu register
		CAMMU_G_TLB = 0x400, // global tlb
		CAMMU_G_REG = 0x500, // global register
	};
	enum cammu_register_mask : u8
	{
		CAMMU_REG_SPDO    = 0x04, // supervisor pdo register
		CAMMU_REG_UPDO    = 0x08, // user pdo register
		CAMMU_REG_FAULT   = 0x10, // fault register
		CAMMU_REG_CONTROL = 0x40, // control register
		CAMMU_REG_RESET   = 0x80, // reset register
	};

	enum control_mask : u32
	{
		CNTL_EP   = 0x00000001, // enable prefetch
		CNTL_EWCW = 0x00000002, // enable watch cpu writes
		CNTL_EWIW = 0x00000004, // enable watch i/o writes
		CNTL_EWIR = 0x00000008, // enable watch i/o reads
		CNTL_UST  = 0x00000030, // unmapped system tag
		CNTL_CV   = 0x00000100, // clear valid
		CNTL_ATE  = 0x00000200, // alignment trap enable
		CNTL_CID  = 0xff000000  // cammu id
	};
	enum control_ust_mask : u32
	{
		UST_0 = 0x00000000, // private, write-through, main memory space
		UST_1 = 0x00000010, // shared, write-through, main memory space
		UST_2 = 0x00000020, // private, copy-back, main memory space
		UST_3 = 0x00000030  // noncacheable, main memory space
	};
	enum control_cid_mask : u32
	{
		CID_C3 = 0x00000000 // unknown
	};

	enum reset_mask : u32
	{
		RESET_RLVW = 0x00000001, // reset all W line LV flags in cache
		RESET_RLVX = 0x00000002, // reset all X line LV flags in cache
		RESET_RSV  = 0x00000004, // reset all SV flags in tlb
		RESET_RUV  = 0x00000008, // reset all UV flags in tlb
		RESET_RD   = 0x00000010, // reset all D flags in tlb
		RESET_RR   = 0x00000020, // reset all R flags in tlb
		RESET_RU   = 0x00000040, // reset all U flags in cache
	};

	u32 tlb_r(const u8 address) const;
	void tlb_w(const u8 address, const u32 data);
	tlb_line_t &tlb_lookup(const bool user, const u32 virtual_address, const access_type mode);

	u32 s_pdo_r() const { return m_s_pdo; }
	void s_pdo_w(const u32 data) { m_s_pdo = data & PDO_MASK; }
	u32 u_pdo_r() const { return m_u_pdo; }
	void u_pdo_w(const u32 data) { m_u_pdo = data & PDO_MASK; }
	u32 fault_r() const { return m_fault; }
	void fault_w(const u32 data) { m_fault = data; }
	u32 control_r() const { return m_control; }
	void control_w(const u32 data) { m_control = (m_control & CNTL_CID) | (data & ~CNTL_CID); }
	void reset_w(const u32 data);

	virtual bool get_alignment() const override { return m_control & CNTL_ATE; }
	virtual system_tag_t get_ust_space() const override { return system_tag_t((m_control & CNTL_UST) >> 4); }
	virtual bool get_access(const access_type mode, const u32 pte, const u32 ssw) const override;
	virtual u32 get_pdo(const bool user) const override { return user ? m_u_pdo : m_s_pdo; }

	virtual void set_fault(const u32 address, const exception_vector type) override { m_fault = address; m_exception_func(type); }

	static const u8 protection_matrix[4][16];

	// device state
	std::vector<cammu_c3_device *> m_linked;

	u32 m_s_pdo;
	u32 m_u_pdo;
	u32 m_fault;
	u32 m_control;

	tlb_set_t m_tlb[64];
};

// device type definitions
DECLARE_DEVICE_TYPE(CAMMU_C4T, cammu_c4t_device)
DECLARE_DEVICE_TYPE(CAMMU_C4I, cammu_c4i_device)
DECLARE_DEVICE_TYPE(CAMMU_C3,  cammu_c3_device)

#endif // MAME_MACHINE_CAMMU_H
