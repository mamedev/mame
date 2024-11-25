// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    NEC 78K/IV series 16/8-bit single-chip microcontrollers

**********************************************************************/

#ifndef MAME_CPU_UPD78K_UPD78K4_H
#define MAME_CPU_UPD78K_UPD78K4_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd78k4_device

class upd78k4_device : public cpu_device
{
public:
	enum {
		UPD78K4_PC,
		UPD78K4_PSW, UPD78K4_PSWL, UPD78K4_PSWH, UPD78K4_RBS, UPD78K4_SP,
		UPD78K4_VVP, UPD78K4_UUP, UPD78K4_TDE, UPD78K4_WHL,
		UPD78K4_RP0, UPD78K4_RP1, UPD78K4_RP2, UPD78K4_RP3,
		UPD78K4_RP4, UPD78K4_RP5, UPD78K4_RP6, UPD78K4_RP7,
		UPD78K4_AX, UPD78K4_BC,
		UPD78K4_VP, UPD78K4_UP, UPD78K4_DE, UPD78K4_HL,
		UPD78K4_R0, UPD78K4_R1, UPD78K4_R2, UPD78K4_R3,
		UPD78K4_R4, UPD78K4_R5, UPD78K4_R6, UPD78K4_R7,
		UPD78K4_R8, UPD78K4_R9, UPD78K4_R10, UPD78K4_R11,
		UPD78K4_R12, UPD78K4_R13, UPD78K4_R14, UPD78K4_R15,
		UPD78K4_X, UPD78K4_A, UPD78K4_C, UPD78K4_B,
		UPD78K4_VPL, UPD78K4_VPH, UPD78K4_UPL, UPD78K4_UPH,
		UPD78K4_E, UPD78K4_D, UPD78K4_L, UPD78K4_H,
		UPD78K4_V, UPD78K4_U, UPD78K4_T, UPD78K4_W
	};

	// TODO: callbacks and configuration thereof

protected:
	upd78k4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor sfr_map);

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

private:
	// internal memory map
	void iram_map(address_map &map) ATTR_COLD;

	// internal helpers
	inline u16 register_base() const noexcept;

private:
	// address spaces, caches & configuration
	address_space_config m_program_config;
	address_space_config m_iram_config;
	address_space_config m_sfr_config;
	required_shared_ptr<u16> m_iram;

	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::cache m_program_cache;
	memory_access< 9, 1, 0, ENDIANNESS_LITTLE>::cache m_iram_cache;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::specific m_program_space;
	memory_access< 8, 1, 0, ENDIANNESS_LITTLE>::specific m_sfr_space;

	// core registers and execution state
	u32 m_pc;
	u32 m_ppc;
	u16 m_psw;
	u32 m_sp;
	u8 m_exp_reg[4];
	s32 m_icount;
};

// ======================> upd784031_device

class upd784031_device : public upd78k4_device
{
public:
	// device type constructor
	upd784031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// type-specific internal memory maps
	void mem_map(address_map &map) ATTR_COLD;
	void sfr_map(address_map &map) ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(UPD784031, upd784031_device)

#endif // MAME_CPU_UPD78K_UPD78K4_H
