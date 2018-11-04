// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_CAMMU_H
#define MAME_MACHINE_CAMMU_H

#pragma once

#define MCFG_CAMMU_SSW_CB(_sswcb) \
	devcb = &cammu_device::static_set_ssw_callback(*device, DEVCB_##_sswcb);

#define MCFG_CAMMU_EXCEPTION_CB(_exceptioncb) \
	devcb = &cammu_device::static_set_exception_callback(*device, DEVCB_##_exceptioncb);

#define MCFG_CAMMU_LINK(_tag) \
	cammu_c3_device::static_add_linked(*device, _tag);

class cammu_device : public device_t, public device_memory_interface
{
public:
	template <class Object> static devcb_base &static_set_ssw_callback(device_t &device, Object &&cb) { return downcast<cammu_device &>(device).m_ssw_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &static_set_exception_callback(device_t &device, Object &&cb) { return downcast<cammu_device &>(device).m_exception_func.set_callback(std::forward<Object>(cb)); }

	static const u32 CAMMU_PAGE_SIZE = 0x1000;
	static const u32 CAMMU_PAGE_MASK = (CAMMU_PAGE_SIZE - 1);

	enum ssw_mask : u32
	{
		SSW_M  = 0x04000000, // mapped mode
		SSW_KU = 0x08000000, // user protect key
		SSW_UU = 0x10000000, // user data mode
		SSW_K  = 0x20000000, // protect key
		SSW_U  = 0x40000000, // user mode

		SSW_PL = 0x78000000  // protection level relevant bits
	};

	enum exception_vectors : u16
	{
		// data memory trap group
		EXCEPTION_D_CORRECTED_MEMORY_ERROR     = 0x108,
		EXCEPTION_D_UNCORRECTABLE_MEMORY_ERROR = 0x110,
		EXCEPTION_D_ALIGNMENT_FAULT            = 0x120,
		EXCEPTION_D_PAGE_FAULT                 = 0x128,
		EXCEPTION_D_READ_PROTECT_FAULT         = 0x130,
		EXCEPTION_D_WRITE_PROTECT_FAULT        = 0x138,

		// instruction memory trap group
		EXCEPTION_I_CORRECTED_MEMORY_ERROR     = 0x288,
		EXCEPTION_I_UNCORRECTABLE_MEMORY_ERROR = 0x290,
		EXCEPTION_I_ALIGNMENT_FAULT            = 0x2a0,
		EXCEPTION_I_PAGE_FAULT                 = 0x2a8,
		EXCEPTION_I_EXECUTE_PROTECT_FAULT      = 0x2b0,
	};

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

	enum pte_st_mask : u32
	{
		ST_0 = 0x00000000, // private, write-through, main memory space
		ST_1 = 0x00000200, // shared, write-through, main memory space
		ST_2 = 0x00000400, // private, copy-back, main memory space
		ST_3 = 0x00000600, // noncacheable, main memory space
		ST_4 = 0x00000800, // noncacheable, i/o space
		ST_5 = 0x00000a00, // noncacheable, boot space
		ST_6 = 0x00000c00, // cache purge
		ST_7 = 0x00000e00  // slave i/o
	};

	enum va_mask : u32
	{
		VA_POFS = 0x00000fff, // page offset
		VA_PTI  = 0x003ff000, // page table index
		VA_PTDI = 0xffc00000  // page table directory index
	};

	virtual void map(address_map &map) = 0;

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

protected:
	cammu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	//virtual bool memory_translate(int spacenum, int intention, offs_t &address) override;

	enum access_t
	{
		ACCESS_R = 1,
		ACCESS_W = 2,
		ACCESS_X = 3
	};

	address_space *translate_address(const offs_t virtual_address, const access_t mode, offs_t *physical_address);

	u32 get_pte(const u32 va, const bool user);

	virtual bool get_access(const access_t mode, const u32 pte, const u32 ssw) const = 0;
	virtual bool get_alignment() const = 0;
	virtual u32 get_pdo(const bool user) const = 0;
	virtual address_space *get_ust_space() const = 0;

	virtual void set_fault_address(const u32 va) = 0;

	address_space_config m_main_space_config;
	address_space_config m_io_space_config;
	address_space_config m_boot_space_config;

	address_space *m_main_space;
	address_space *m_io_space;
	address_space *m_boot_space;

	devcb_read32 m_ssw_func;
	devcb_write16 m_exception_func;

	struct
	{
		u32 va;
		u32 pte;
	}
	m_tlb[2];

private:
};

class cammu_c4_device : public cammu_device
{
public:
	DECLARE_READ32_MEMBER(s_pdo_r) { return m_s_pdo; }
	DECLARE_WRITE32_MEMBER(s_pdo_w) { m_s_pdo = ((m_s_pdo & ~mem_mask) | (data & mem_mask)) & PDO_MASK; }
	DECLARE_READ32_MEMBER(u_pdo_r) { return m_u_pdo; }
	DECLARE_WRITE32_MEMBER(u_pdo_w) { m_u_pdo = ((m_u_pdo & ~mem_mask) | (data & mem_mask)) & PDO_MASK; }

	virtual DECLARE_READ32_MEMBER(control_r) = 0;
	virtual DECLARE_WRITE32_MEMBER(control_w) = 0;

	DECLARE_READ32_MEMBER(i_fault_r) { return m_i_fault; }
	DECLARE_WRITE32_MEMBER(i_fault_w) { m_i_fault = data; }
	DECLARE_READ32_MEMBER(fault_address_1_r) { return m_fault_address_1; }
	DECLARE_WRITE32_MEMBER(fault_address_1_w) { m_fault_address_1 = data; }
	DECLARE_READ32_MEMBER(fault_address_2_r) { return m_fault_address_2; }
	DECLARE_WRITE32_MEMBER(fault_address_2_w) { m_fault_address_2 = data; }
	DECLARE_READ32_MEMBER(fault_data_1_lo_r) { return m_fault_data_1_lo; }
	DECLARE_WRITE32_MEMBER(fault_data_1_lo_w) { m_fault_data_1_lo = data; }
	DECLARE_READ32_MEMBER(fault_data_1_hi_r) { return m_fault_data_1_hi; }
	DECLARE_WRITE32_MEMBER(fault_data_1_hi_w) { m_fault_data_1_hi = data; }
	DECLARE_READ32_MEMBER(fault_data_2_lo_r) { return m_fault_data_2_lo; }
	DECLARE_WRITE32_MEMBER(fault_data_2_lo_w) { m_fault_data_2_lo = data; }
	DECLARE_READ32_MEMBER(fault_data_2_hi_r) { return m_fault_data_2_hi; }
	DECLARE_WRITE32_MEMBER(fault_data_2_hi_w) { m_fault_data_2_hi = data; }

protected:
	cammu_c4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u32 cammu_id);

	virtual void device_start() override;

	virtual bool get_access(const access_t mode, const u32 pte, const u32 ssw) const override;
	virtual u32 get_pdo(const bool user) const override { return user ? m_u_pdo : m_s_pdo; }

	virtual void set_fault_address(const u32 va) override { m_fault_address_1 = va; }

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

	virtual void map(address_map &map) override;

	DECLARE_READ32_MEMBER(ram_line_r) { return m_ram_line; }
	DECLARE_WRITE32_MEMBER(ram_line_w) { m_ram_line = data; }

	DECLARE_READ32_MEMBER(htlb_offset_r) { return m_htlb_offset; }
	DECLARE_WRITE32_MEMBER(htlb_offset_w) { m_htlb_offset = data; }

	DECLARE_READ32_MEMBER(c4_bus_poll_r) { return m_c4_bus_poll; }
	DECLARE_WRITE32_MEMBER(c4_bus_poll_w) { m_c4_bus_poll = data; }

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

	virtual DECLARE_READ32_MEMBER(control_r) override { return m_control; }
	virtual DECLARE_WRITE32_MEMBER(control_w) override { m_control = ((m_control & (~mem_mask | CNTL_CID)) | (data & (mem_mask & ~CNTL_CID))); }
	DECLARE_READ32_MEMBER(bio_control_r) { return m_bio_control; }
	DECLARE_WRITE32_MEMBER(bio_control_w) { m_bio_control = data; }
	DECLARE_READ32_MEMBER(bio_address_tag_r) { return m_bio_address_tag; }
	DECLARE_WRITE32_MEMBER(bio_address_tag_w) { m_bio_address_tag = data; }

	DECLARE_READ32_MEMBER(cache_data_lo_r) { return m_cache_data_lo; }
	DECLARE_WRITE32_MEMBER(cache_data_lo_w) { m_cache_data_lo = data; }
	DECLARE_READ32_MEMBER(cache_data_hi_r) { return m_cache_data_hi; }
	DECLARE_WRITE32_MEMBER(cache_data_hi_w) { m_cache_data_hi = data; }
	DECLARE_READ32_MEMBER(cache_cpu_tag_r) { return m_cache_cpu_tag; }
	DECLARE_WRITE32_MEMBER(cache_cpu_tag_w) { m_cache_cpu_tag = data; }
	DECLARE_READ32_MEMBER(cache_system_tag_valid_r) { return m_cache_system_tag_valid; }
	DECLARE_WRITE32_MEMBER(cache_system_tag_valid_w) { m_cache_system_tag_valid = data; }
	DECLARE_READ32_MEMBER(cache_system_tag_r) { return m_cache_system_tag; }
	DECLARE_WRITE32_MEMBER(cache_system_tag_w) { m_cache_system_tag = data; }
	DECLARE_READ32_MEMBER(tlb_va_line_r) { return m_tlb_va_line; }
	DECLARE_WRITE32_MEMBER(tlb_va_line_w) { m_tlb_va_line = data; }
	DECLARE_READ32_MEMBER(tlb_ra_line_r) { return m_tlb_ra_line; }
	DECLARE_WRITE32_MEMBER(tlb_ra_line_w) { m_tlb_ra_line = data; }

protected:
	virtual void device_start() override;

	virtual bool get_alignment() const override { return (m_control & CNTL_ATD) == 0; }
	virtual address_space *get_ust_space() const override { return (m_control & CNTL_IOTS) ? m_io_space : m_main_space; }

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

	virtual void map(address_map &map) override;

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

	enum control_cid_mask : u32
	{
		CID_C4I = 0x02000000 // c4i cammu identification
	};

	virtual DECLARE_READ32_MEMBER(control_r) override { return m_control; }
	virtual DECLARE_WRITE32_MEMBER(control_w) override { m_control = ((m_control & (~mem_mask | CNTL_CID)) | (data & (mem_mask & ~CNTL_CID))); }

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
	DECLARE_READ32_MEMBER(reset_r) { return m_reset; }
	DECLARE_WRITE32_MEMBER(reset_w) { m_reset = data; }

	DECLARE_READ32_MEMBER(clr_s_data_tlb_r) { return m_clr_s_data_tlb; }
	DECLARE_WRITE32_MEMBER(clr_s_data_tlb_w) { m_clr_s_data_tlb = data; }
	DECLARE_READ32_MEMBER(clr_u_data_tlb_r) { return m_clr_u_data_tlb; }
	DECLARE_WRITE32_MEMBER(clr_u_data_tlb_w) { m_clr_u_data_tlb = data; }
	DECLARE_READ32_MEMBER(clr_s_insn_tlb_r) { return m_clr_s_insn_tlb; }
	DECLARE_WRITE32_MEMBER(clr_s_insn_tlb_w) { m_clr_s_insn_tlb = data; }
	DECLARE_READ32_MEMBER(clr_u_insn_tlb_r) { return m_clr_u_insn_tlb; }
	DECLARE_WRITE32_MEMBER(clr_u_insn_tlb_w) { m_clr_u_insn_tlb = data; }

	DECLARE_READ32_MEMBER(test_data_r) { return m_test_data; }
	DECLARE_WRITE32_MEMBER(test_data_w) { m_test_data = data; }

	DECLARE_READ32_MEMBER(test_address_r) { return m_test_address; }
	DECLARE_WRITE32_MEMBER(test_address_w) { m_test_address = data; }

protected:
	virtual void device_start() override;

	virtual bool get_alignment() const override { return (m_control & CNTL_ATD) == 0; }
	virtual address_space *get_ust_space() const override;

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

	static void static_add_linked(device_t &device, const char *const tag);

	virtual void map(address_map &map) override;
	virtual void map_global(address_map &map);

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

	enum ust_mask : u32
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

	DECLARE_READ32_MEMBER(s_pdo_r) { return m_s_pdo; }
	DECLARE_WRITE32_MEMBER(s_pdo_w) { m_s_pdo = ((m_s_pdo & ~mem_mask) | (data & mem_mask)) & PDO_MASK; }
	DECLARE_READ32_MEMBER(u_pdo_r) { return m_u_pdo; }
	DECLARE_WRITE32_MEMBER(u_pdo_w) { m_u_pdo = ((m_u_pdo & ~mem_mask) | (data & mem_mask)) & PDO_MASK; }
	DECLARE_READ32_MEMBER(fault_r) { return m_fault; }
	DECLARE_WRITE32_MEMBER(fault_w) { m_fault = data; }
	DECLARE_READ32_MEMBER(control_r) { return m_control; }
	DECLARE_WRITE32_MEMBER(control_w) { m_control = ((m_control & (~mem_mask | CNTL_CID)) | (data & (mem_mask & ~CNTL_CID))); }
	DECLARE_READ32_MEMBER(reset_r) { return m_reset; }
	DECLARE_WRITE32_MEMBER(reset_w) { m_reset = data; }

	// global methods - relay to each linked device
	DECLARE_WRITE32_MEMBER(g_s_pdo_w)   { for (cammu_c3_device *dev : m_linked) dev->s_pdo_w(space, offset, data, mem_mask); }
	DECLARE_WRITE32_MEMBER(g_u_pdo_w)   { for (cammu_c3_device *dev : m_linked) dev->u_pdo_w(space, offset, data, mem_mask); }
	DECLARE_WRITE32_MEMBER(g_fault_w)   { for (cammu_c3_device *dev : m_linked) dev->fault_w(space, offset, data, mem_mask); }
	DECLARE_WRITE32_MEMBER(g_control_w) { for (cammu_c3_device *dev : m_linked) dev->control_w(space, offset, data, mem_mask); }
	DECLARE_WRITE32_MEMBER(g_reset_w)   { for (cammu_c3_device *dev : m_linked) dev->reset_w(space, offset, data, mem_mask); }

protected:
	virtual void device_reset() override;
	virtual void device_start() override;

	virtual bool get_access(const access_t mode, const u32 pte, const u32 ssw) const override;
	virtual bool get_alignment() const override { return m_control & CNTL_ATE; }
	virtual u32 get_pdo(const bool user) const override { return user ? m_u_pdo : m_s_pdo; }
	virtual address_space *get_ust_space() const override { return m_main_space; }

	virtual void set_fault_address(const u32 va) override { m_fault = va; }

private:
	enum c3_access_t : u8
	{
		N   = 0, // no access
		R   = 1, // read permitted
		W   = 2, // write permitted
		RW  = 3, // read and write permitted
		E   = 4, // execute permitted
		RE  = 5, // read and execute permitted
		RWE = 7  // read, write and execute permitted
	};

	static const u8 i_cammu_column[];
	static const u8 d_cammu_column[];
	static const c3_access_t cammu_matrix[][16];

	u32 m_s_pdo;
	u32 m_u_pdo;
	u32 m_fault;
	u32 m_control;
	u32 m_reset;

	std::vector<cammu_c3_device *> m_linked;
};

// device type definitions
DECLARE_DEVICE_TYPE(CAMMU_C4T, cammu_c4t_device)
DECLARE_DEVICE_TYPE(CAMMU_C4I, cammu_c4i_device)
DECLARE_DEVICE_TYPE(CAMMU_C3,  cammu_c3_device)

#endif // MAME_MACHINE_CAMMU_H
