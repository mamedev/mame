// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    NEC 78K/III series 16/8-bit single-chip microcontrollers

**********************************************************************/

#ifndef MAME_CPU_UPD78K_UPD78K3_H
#define MAME_CPU_UPD78K_UPD78K3_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd78k3_device

class upd78k3_device : public cpu_device
{
public:
	enum {
		UPD78K3_PC,
		UPD78K3_PSW, UPD78K3_PSWL, UPD78K3_PSWH, UPD78K3_RBS, UPD78K3_SP,
		UPD78K3_RP0, UPD78K3_RP1, UPD78K3_RP2, UPD78K3_RP3,
		UPD78K3_RP4, UPD78K3_RP5, UPD78K3_RP6, UPD78K3_RP7,
		UPD78K3_AX, UPD78K3_BC,
		UPD78K3_VP, UPD78K3_UP, UPD78K3_DE, UPD78K3_HL,
		UPD78K3_R0, UPD78K3_R1, UPD78K3_R2, UPD78K3_R3,
		UPD78K3_R4, UPD78K3_R5, UPD78K3_R6, UPD78K3_R7,
		UPD78K3_R8, UPD78K3_R9, UPD78K3_R10, UPD78K3_R11,
		UPD78K3_R12, UPD78K3_R13, UPD78K3_R14, UPD78K3_R15,
		UPD78K3_X, UPD78K3_A, UPD78K3_C, UPD78K3_B,
		UPD78K3_VPL, UPD78K3_VPH, UPD78K3_UPL, UPD78K3_UPH,
		UPD78K3_E, UPD78K3_D, UPD78K3_L, UPD78K3_H
	};

	// TODO: callbacks and configuration thereof

protected:
	upd78k3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor sfr_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual void state_add_psw();

private:
	// internal memory map
	void iram_map(address_map &map) ATTR_COLD;

	// internal helpers
	inline u8 register_base() const noexcept;
protected:
	u8 iram_byte_r(offs_t offset);
	void iram_byte_w(offs_t offset, u8 data);

private:
	// address spaces, caches & configuration
	address_space_config m_program_config;
	address_space_config m_iram_config;
	address_space_config m_sfr_config;
	required_shared_ptr<u16> m_iram;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_program_cache;
	memory_access< 8, 1, 0, ENDIANNESS_LITTLE>::cache m_iram_cache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program_space;
	memory_access< 8, 1, 0, ENDIANNESS_LITTLE>::specific m_sfr_space;

	// core registers and execution state
	u16 m_pc;
	u16 m_ppc;
protected:
	u16 m_psw;
private:
	u16 m_sp;
	s32 m_icount;
};

// ======================> upd78312_device

class upd78312_device : public upd78k3_device
{
public:
	// device type constructor
	upd78312_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	upd78312_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// upd78k3_device overrides
	virtual void state_add_psw() override;

private:
	// type-specific internal memory maps
	void mem_map(address_map &map) ATTR_COLD;
	void sfr_map(address_map &map) ATTR_COLD;
};

// ======================> upd78310_device

class upd78310_device : public upd78312_device
{
public:
	// device type constructor
	upd78310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(UPD78310, upd78310_device)
DECLARE_DEVICE_TYPE(UPD78312, upd78312_device)

#endif // MAME_CPU_UPD78K_UPD7832_H
