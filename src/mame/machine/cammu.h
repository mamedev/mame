// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#pragma once

#ifndef CAMMU_H_
#define CAMMU_H_


// the following enables a very crude instruction cache - it has known (future)
// problems, but speeds up cpu execution quite noticeably in the short term by
// avoiding some of the delays in the mame memory subsystem
#define ICACHE_ENTRIES 32768

#define MCFG_CAMMU_SSW_CB(_sswcb) \
	devcb = &cammu_device::static_set_ssw_callback(*device, DEVCB_##_sswcb);

class cammu_device : public device_t, public device_memory_interface
{
public:
	cammu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &static_set_ssw_callback(device_t &device, _Object object) { return downcast<cammu_device &>(device).m_ssw_func.set_callback(object); }

	virtual DECLARE_ADDRESS_MAP(map, 32) = 0;

	DECLARE_READ32_MEMBER(mmu_r);
	DECLARE_WRITE32_MEMBER(mmu_w);

	DECLARE_READ32_MEMBER(cammu_r) { return m_cammu[offset]; }
	DECLARE_WRITE32_MEMBER(cammu_w) { m_cammu[offset] = data; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config (address_spacenum spacenum) const override;

private:
	address_space_config m_main_space_config;
	address_space_config m_io_space_config;
	address_space_config m_boot_space_config;
	address_space *m_main_space;
	address_space *m_io_space;
	address_space *m_boot_space;
	devcb_read32 m_ssw_func;

	u32 m_cammu[1024];

#ifdef ICACHE_ENTRIES
	struct icache
	{
		u32 offset;
		u32 data;
	} m_icache[ICACHE_ENTRIES];
#endif
};

class cammu_c4t_device : public cammu_device
{
public:
	cammu_c4t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 32) override;
};

class cammu_c4i_device : public cammu_device
{
public:
	cammu_c4i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 32) override;
};

class cammu_c3_device : public cammu_device
{
public:
	cammu_c3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 32) override;
};

// device type definitions
extern const device_type CAMMU_C4T;
extern const device_type CAMMU_C4I;
extern const device_type CAMMU_C3;

#endif
