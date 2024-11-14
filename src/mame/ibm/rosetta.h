// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_IBM_ROSETTA_H
#define MAME_IBM_ROSETTA_H

#pragma once

#include "cpu/romp/rsc.h"

class rosetta_device
	: public device_t
	, public rsc_cpu_interface
{
public:
	// ram size in doublewords
	enum ram_size : unsigned
	{
		RAM_NONE = 0,
		RAM_1M   = 0x0004'0000,
		RAM_2M   = 0x0008'0000,
		RAM_4M   = 0x0010'0000,
		RAM_8M   = 0x0020'0000,
		RAM_16M  = 0x0040'0000,
	};

	rosetta_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, ram_size ram = RAM_NONE);

	template <typename T> void set_mem(T &&tag, int spacenum) { m_mem_space.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_rom(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

	auto out_pchk() { return m_out_pchk.bind(); }
	auto out_mchk() { return m_out_mchk.bind(); }

	using rsc_mode = rsc_bus_interface::rsc_mode;

	// rsc_cpu_interface overrides
	virtual bool fetch(u32 address, u16 &data, rsc_mode const mode) override;

	// rsc_bus_interface overrides
	virtual bool mem_load(u32 address, u8 &data, rsc_mode const mode, bool sp) override { return load<u8>(address, data, mode, sp); }
	virtual bool mem_load(u32 address, u16 &data, rsc_mode const mode, bool sp) override { return load<u16>(address, data, mode, sp); }
	virtual bool mem_load(u32 address, u32 &data, rsc_mode const mode, bool sp) override { return load<u32>(address, data, mode, sp); }
	virtual bool mem_store(u32 address, u8 data, rsc_mode const mode, bool sp) override { return store<u8>(address, data, mode, sp); }
	virtual bool mem_store(u32 address, u16 data, rsc_mode const mode, bool sp) override { return store<u16>(address, data, mode, sp); }
	virtual bool mem_store(u32 address, u32 data, rsc_mode const mode, bool sp) override { return store<u32>(address, data, mode, sp); }
	virtual bool mem_modify(u32 address, std::function<u8(u8)> f, rsc_mode const mode) override { return modify<u8>(address, f, mode); }
	virtual bool mem_modify(u32 address, std::function<u16(u16)> f, rsc_mode const mode) override { return modify<u16>(address, f, mode); }
	virtual bool mem_modify(u32 address, std::function<u32(u32)> f, rsc_mode const mode) override { return modify<u32>(address, f, mode); }

	virtual bool pio_load(u32 address, u8 &data, rsc_mode const mode) override { return false; }
	virtual bool pio_load(u32 address, u16 &data, rsc_mode const mode) override { return false; }
	virtual bool pio_load(u32 address, u32 &data, rsc_mode const mode) override { return ior(address, data); }
	virtual bool pio_store(u32 address, u8 data, rsc_mode const mode) override { return false; }
	virtual bool pio_store(u32 address, u16 data, rsc_mode const mode) override { return false; }
	virtual bool pio_store(u32 address, u32 data, rsc_mode const mode) override { return iow(address, data); }
	virtual bool pio_modify(u32 address, std::function<u8(u8)> f, rsc_mode const mode) override { return false; }
	virtual bool pio_modify(u32 address, std::function<u16(u16)> f, rsc_mode const mode) override { return false; }
	virtual bool pio_modify(u32 address, std::function<u32(u32)> f, rsc_mode const mode) override { return false; }

protected:
	// device_t overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// virtual address translation
	bool translate(u32 &address, bool system_processor, bool store);

	// rsc_bus_interface implementation
	template <typename T> bool load(u32 address, T &data, rsc_mode const mode, bool sp);
	template <typename T> bool store(u32 address, T data, rsc_mode const mode, bool sp);
	template <typename T> bool modify(u32 address, std::function<T(T)> f, rsc_mode const mode);
	bool ior(u32 address, u32 &data);
	bool iow(u32 address, u32 data);

	// register read handlers
	u32 segment_r(offs_t offset);
	u32 control_r(offs_t offset);
	u32 tlb_r(offs_t offset);

	// register write handlers
	void segment_w(offs_t offset, u32 data);
	void control_w(offs_t offset, u32 data);
	void tlb_w(offs_t offset, u32 data);

	// memory read handlers
	u32 rom_r(offs_t offset, u32 mem_mask);
	u32 ram_r(offs_t offset, u32 mem_mask);
	u32 rca_r(offs_t offset);

	// memory write handlers
	void rom_w(offs_t offset, u32 data, u32 mem_mask);
	void ram_w(offs_t offset, u32 data, u32 mem_mask);
	void rca_w(offs_t offset, u32 data);

	u8 compute_ecc(u32 const data) const;
	unsigned check_ecc(u32 &data, u8 const ecc) const;

	struct tlb_entry
	{
		u32 field0 = 0; // address tag
		u32 field1 = 0; // real page, valid, key
		u32 field2 = 0; // write, transaction identifier, lockbits
	};
	tlb_entry tlb_search(u64 const virtual_address, bool const special);
	u32 tlb_reload(tlb_entry &tlb_entry, u64 const virtual_address, bool special = false);

	void tlb_inv_all(u32 data);
	void tlb_inv_segment(u32 data);
	void tlb_inv_address(u32 data);
	void compute_address(u32 data);

	void config_map();
	void config_tlb();

	enum mear_state : u32
	{
		UNLOCKED,
		LOCKED,
		MEMORY,
	};
	void set_mear(u32 const address, mear_state lock);
	void set_rmdr(u8 const ecc, bool lock);

	void set_pchk(bool state)
	{
		if (state != m_pchk_state)
		{
			m_pchk_state = state;

			// program check line is active low
			m_out_pchk(!m_pchk_state);
		}
	}

	void set_mchk(bool state)
	{
		// assume edge-triggered, active low
		if (state)
		{
			m_out_mchk(0);
			m_out_mchk(1);
		}
		else
			m_out_mchk(1);
	}

private:
	required_address_space m_mem_space;
	required_region_ptr<u32> m_rom;
	output_finder<2> m_leds;

	devcb_write_line m_out_pchk;
	devcb_write_line m_out_mchk;

	memory_access<24, 2, 0, ENDIANNESS_BIG>::cache m_mem;

	// registers
	u32 m_segment[16];
	u32 m_control[9];

	// lock and line state
	mear_state m_mear_lock;
	bool m_rmdr_lock;
	bool m_led_lock;
	bool m_pchk_state;

	// tlb state
	tlb_entry m_tlb[16][2];
	u16 m_tlb_lru;

	// storage
	std::unique_ptr<u32[]> m_ram;
	std::unique_ptr<u8[]> m_ecc;
	std::unique_ptr<u8[]> m_rca;

	// address translation constants
	u32 m_atag_mask;
	u32 m_page_mask;
	unsigned m_page_shift;
	u32 m_hat_base;
	u16 m_hat_mask;

	ram_size const m_ram_size;
};

DECLARE_DEVICE_TYPE(ROSETTA, rosetta_device)

#endif // MAME_IBM_ROSETTA_H
