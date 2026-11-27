// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

    Fujitsu MB8764 family DSP

***************************************************************************/

#ifndef MAME_CPU_MB8764_MB8764_H
#define MAME_CPU_MB8764_MB8764_H

#pragma once

class mb8764_base_device : public cpu_device
{
public:
	enum
	{
		MB8764_PC = 1, MB8764_PCS,
		MB8764_A, MB8764_B, MB8764_D,
		MB8764_X, MB8764_Y,
		MB8764_C0, MB8764_C1, MB8764_VP, MB8764_DMC,
		MB8764_PGM, MB8764_PGT, MB8764_EXA,
		MB8764_EI, MB8764_EO
	};

	// REQ: the output instruction puts EO on the data bus
	auto out_cb() { return m_out_cb.bind(); }

	// flag input FL0
	void fl0_w(int state);

	// one word through the input port (AIF, RCK)
	void data_w(u16 data);

protected:
	mb8764_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program_map);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override;
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override;
	virtual u32 execute_min_cycles() const noexcept override;
	virtual u32 execute_max_cycles() const noexcept override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface implementation
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	memory_access<10, 2, -2, ENDIANNESS_BIG>::specific m_program;

private:
	void data_map(address_map &map) ATTR_COLD;

	void execute_one(u32 op);
	void alu(u8 code);
	s32 product() const;
	u16 d_bus() const;
	u8 index_y(u8 address) const;
	u16 ram_r(bool internal, u8 address);
	void ram_w(bool internal, u8 address, u16 data);
	void load_regs(u8 field, u16 data);
	void clear(u32 mask);
	void set(u32 mask);
	bool condition(u32 op);

	address_space_config m_program_config;
	address_space_config m_data_config;

	memory_access<10, 2, -2, ENDIANNESS_BIG>::cache m_cache;
	memory_access<8, 1, -1, ENDIANNESS_BIG>::specific m_data;

	devcb_write16 m_out_cb;

	int m_icount;

	u16 m_pc;
	u16 m_pcs;
	s16 m_a;
	s16 m_b;
	s32 m_d;
	s32 m_tr[2];
	u8 m_x;
	u8 m_y;
	u8 m_c0;
	u8 m_c1;
	u8 m_vp;
	u8 m_dmc;
	u8 m_pgm;
	u8 m_pgt;
	u16 m_exa;
	u16 m_ei;
	u16 m_eo;
	u32 m_st;
};

class tvf16_device : public mb8764_base_device
{
public:
	tvf16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// load state: one program word in two halves
	void program_w(u16 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	void program_map(address_map &map) ATTR_COLD;

	u8 m_load_address;
	u16 m_load_high;
};

DECLARE_DEVICE_TYPE(TVF16, tvf16_device)

#endif // MAME_CPU_MB8764_MB8764_H
