// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_CAMMU_H
#define MAME_MACHINE_CAMMU_H

#pragma once

#define MCFG_CAMMU_SSW_CB(_sswcb) \
	devcb = &cammu_device::static_set_ssw_callback(*device, DEVCB_##_sswcb);

class cammu_device : public device_t, public device_memory_interface
{
public:
	template <class Object> static devcb_base &static_set_ssw_callback(device_t &device, Object &&cb) { return downcast<cammu_device &>(device).m_ssw_func.set_callback(std::forward<Object>(cb)); }

	virtual DECLARE_ADDRESS_MAP(map, 32) = 0;

	DECLARE_READ32_MEMBER(insn_r);

	DECLARE_READ32_MEMBER(data_r);
	DECLARE_WRITE32_MEMBER(data_w);

protected:
	cammu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	virtual u32 get_pte(u32 va, int user, bool data) = 0;

private:
	address_space_config m_main_space_config;
	address_space_config m_io_space_config;
	address_space_config m_boot_space_config;

protected:
	address_space *m_main_space;
	address_space *m_io_space;
	address_space *m_boot_space;

	struct
	{
		u32 va;
		u32 pte;
	} m_tlb[4];

private:
	devcb_read32 m_ssw_func;
};

class cammu_c4_device : public cammu_device
{
public:
	cammu_c4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER(s_pdo_r) { return m_s_pdo; }
	DECLARE_WRITE32_MEMBER(s_pdo_w) { m_s_pdo = data; }
	DECLARE_READ32_MEMBER(u_pdo_r) { return m_u_pdo; }
	DECLARE_WRITE32_MEMBER(u_pdo_w) { m_u_pdo = data; }

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
	u32 get_pte(u32 va, int user, bool data) override;

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

	virtual DECLARE_ADDRESS_MAP(map, 32) override;

	DECLARE_READ32_MEMBER(ram_line_r) { return m_ram_line; }
	DECLARE_WRITE32_MEMBER(ram_line_w) { m_ram_line = data; }

	DECLARE_READ32_MEMBER(htlb_offset_r) { return m_htlb_offset; }
	DECLARE_WRITE32_MEMBER(htlb_offset_w) { m_htlb_offset = data; }

	DECLARE_READ32_MEMBER(c4_bus_poll_r) { return m_c4_bus_poll; }
	DECLARE_WRITE32_MEMBER(c4_bus_poll_w) { m_c4_bus_poll = data; }
	virtual DECLARE_READ32_MEMBER(control_r) override { return m_control | 0x00000000; }
	virtual DECLARE_WRITE32_MEMBER(control_w) override { m_control = data & 0x00ffffff; }
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

	virtual DECLARE_ADDRESS_MAP(map, 32) override;

	virtual DECLARE_READ32_MEMBER(control_r) override { return m_control | 0x02000000; }
	virtual DECLARE_WRITE32_MEMBER(control_w) override { m_control = data & 0x00ffffff; }

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

	virtual DECLARE_ADDRESS_MAP(map, 32) override;

	DECLARE_READ32_MEMBER(d_s_pdo_r) { return m_d_s_pdo; }
	DECLARE_WRITE32_MEMBER(d_s_pdo_w) { m_d_s_pdo = data; }
	DECLARE_READ32_MEMBER(d_u_pdo_r) { return m_d_u_pdo; }
	DECLARE_WRITE32_MEMBER(d_u_pdo_w) { m_d_u_pdo = data; }
	DECLARE_READ32_MEMBER(d_fault_r) { return m_d_fault; }
	DECLARE_WRITE32_MEMBER(d_fault_w) { m_d_fault = data; }
	DECLARE_READ32_MEMBER(d_control_r) { return m_d_control; }
	DECLARE_WRITE32_MEMBER(d_control_w) { m_d_control = data; }
	DECLARE_READ32_MEMBER(d_reset_r) { return m_d_reset; }
	DECLARE_WRITE32_MEMBER(d_reset_w) { m_d_reset = data; }

	DECLARE_READ32_MEMBER(i_s_pdo_r) { return m_i_s_pdo; }
	DECLARE_WRITE32_MEMBER(i_s_pdo_w) { m_i_s_pdo = data; }
	DECLARE_READ32_MEMBER(i_u_pdo_r) { return m_i_u_pdo; }
	DECLARE_WRITE32_MEMBER(i_u_pdo_w) { m_i_u_pdo = data; }
	DECLARE_READ32_MEMBER(i_fault_r) { return m_i_fault; }
	DECLARE_WRITE32_MEMBER(i_fault_w) { m_i_fault = data; }
	DECLARE_READ32_MEMBER(i_control_r) { return m_i_control; }
	DECLARE_WRITE32_MEMBER(i_control_w) { m_i_control = data; }
	DECLARE_READ32_MEMBER(i_reset_r) { return m_i_reset; }
	DECLARE_WRITE32_MEMBER(i_reset_w) { m_i_reset = data; }

	DECLARE_WRITE32_MEMBER(g_s_pdo_w) { d_s_pdo_w(space, offset, data, mem_mask); i_s_pdo_w(space, offset, data, mem_mask); }
	DECLARE_WRITE32_MEMBER(g_u_pdo_w) { d_u_pdo_w(space, offset, data, mem_mask); i_u_pdo_w(space, offset, data, mem_mask); }
	DECLARE_WRITE32_MEMBER(g_fault_w) { d_fault_w(space, offset, data, mem_mask); i_fault_w(space, offset, data, mem_mask); }
	DECLARE_WRITE32_MEMBER(g_control_w) { d_control_w(space, offset, data, mem_mask); i_control_w(space, offset, data, mem_mask); }
	DECLARE_WRITE32_MEMBER(g_reset_w) { d_reset_w(space, offset, data, mem_mask); i_reset_w(space, offset, data, mem_mask); }

protected:
	u32 get_pte(u32 va, int user, bool data) override;

private:
	u32 m_d_s_pdo;
	u32 m_d_u_pdo;
	u32 m_d_fault;
	u32 m_d_control;
	u32 m_d_reset;
	u32 m_i_s_pdo;
	u32 m_i_u_pdo;
	u32 m_i_fault;
	u32 m_i_control;
	u32 m_i_reset;
};

// device type definitions
DECLARE_DEVICE_TYPE(CAMMU_C4T, cammu_c4t_device)
DECLARE_DEVICE_TYPE(CAMMU_C4I, cammu_c4i_device)
DECLARE_DEVICE_TYPE(CAMMU_C3,  cammu_c3_device)

#endif // MAME_MACHINE_CAMMU_H
