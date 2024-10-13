// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_ST9_ST905X_H
#define MAME_CPU_ST9_ST905X_H

#pragma once

class st9_device : public cpu_device
{
public:
	enum {
		ST9_PC, ST9_SSPR, ST9_USPR,
		ST9_CICR, ST9_FLAGR, ST9_RP0R, ST9_RP1R, ST9_PPR, ST9_MODER,
		ST9_R0, ST9_R1, ST9_R2, ST9_R3, ST9_R4, ST9_R5, ST9_R6, ST9_R7,
		ST9_R8, ST9_R9, ST9_R10, ST9_R11, ST9_R12, ST9_R13, ST9_R14, ST9_R15,
		ST9_RR0, ST9_RR2, ST9_RR4, ST9_RR6, ST9_RR8, ST9_RR10, ST9_RR12, ST9_RR14
	};

protected:
	// construction/destruction
	st9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor regmap);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// internal register handlers
	u8 cicr_r();
	void cicr_w(u8 data);
	u8 flagr_r();
	void flagr_w(u8 data);
	u8 rpr_r(offs_t offset);
	void rpr_w(offs_t offset, u8 data);
	u8 ppr_r();
	void ppr_w(u8 data);
	u8 moder_r();
	void moder_w(u8 data);
	u8 spr_r(offs_t offset);
	void spr_w(offs_t offset, u8 data);

private:
	// debugging helpers
	u8 debug_register_r(int r);
	void debug_register_w(int r, u8 data);
	u16 debug_rpair_r(int rr);
	void debug_rpair_w(int rr, u16 data);

	// address spaces
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_register_config;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_data;
	memory_access<8, 0, 0, ENDIANNESS_BIG>::specific m_register;

	// internal state
	u16 m_pc;
	u16 m_sspr;
	u16 m_uspr;
	u8 m_cicr;
	u8 m_flagr;
	u8 m_rpr[2];
	u8 m_ppr;
	u8 m_moder;
	s32 m_icount;
};

class st90r50_device : public st9_device
{
public:
	// device type constructor
	st90r50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	void register_map(address_map &map) ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(ST90R50, st90r50_device)

#endif // MAME_CPU_ST9_ST905X_H
