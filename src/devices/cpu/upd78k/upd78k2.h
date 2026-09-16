// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    NEC 78K/II series 8-bit single-chip microcontrollers

**********************************************************************/

#ifndef MAME_CPU_UPD78K_UPD78K2_H
#define MAME_CPU_UPD78K_UPD78K2_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd78k2_device

class upd78k2_device : public cpu_device
{
public:
	enum {
		UPD78K2_PC,
		UPD78K2_PSW, UPD78K2_RBS, UPD78K2_SP,
		UPD78K2_AX, UPD78K2_BC,
		UPD78K2_DE, UPD78K2_HL,
		UPD78K2_X, UPD78K2_A, UPD78K2_C, UPD78K2_B,
		UPD78K2_E, UPD78K2_D, UPD78K2_L, UPD78K2_H
	};

	// TODO: callbacks and configuration thereof

protected:
	upd78k2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int iram_bits, address_map_constructor mem_map, address_map_constructor sfr_map);

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
	inline u8 register_base() const noexcept;

	// address spaces, caches & configuration
	address_space_config m_program_config;
	address_space_config m_iram_config;
	address_space_config m_sfr_config;
	const offs_t m_iram_addrmask;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::cache m_program_cache;
	memory_access< 8, 1, 0, ENDIANNESS_LITTLE>::cache m_iram_cache;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::specific m_program_space;
	memory_access< 8, 1, 0, ENDIANNESS_LITTLE>::specific m_sfr_space;

	// core registers and execution state
	u16 m_pc;
	u16 m_ppc;
	u8 m_psw;
	u16 m_sp;
	s32 m_icount;
};

// ======================> upd78210_device

class upd78210_device : public upd78k2_device
{
public:
	// device type constructor
	upd78210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// type-specific internal memory maps
	void sfr_map(address_map &map) ATTR_COLD;
};

// ======================> upd78213_device

class upd78213_device : public upd78k2_device
{
public:
	// device type constructor
	upd78213_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// type-specific internal memory maps
	void mem_map(address_map &map) ATTR_COLD;
	void sfr_map(address_map &map) ATTR_COLD;
};

// device type declarations
DECLARE_DEVICE_TYPE(UPD78210, upd78210_device)
DECLARE_DEVICE_TYPE(UPD78213, upd78213_device)

#endif // MAME_CPU_UPD78K_UPD78K2_H
