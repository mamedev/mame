// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    NEC 78K/0 series 8-bit single-chip microcontrollers

**********************************************************************/

#ifndef MAME_CPU_UPD78K_UPD78K0_H
#define MAME_CPU_UPD78K_UPD78K0_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd78k0_device

class upd78k0_device : public cpu_device
{
public:
	enum {
		UPD78K0_PC, UPD78K0_PSW, UPD78K0_RBS, UPD78K0_SP,
		UPD78K0_AX, UPD78K0_BC,
		UPD78K0_DE, UPD78K0_HL,
		UPD78K0_X, UPD78K0_A, UPD78K0_C, UPD78K0_B,
		UPD78K0_D, UPD78K0_E, UPD78K0_H, UPD78K0_L
	};

	// configuration (TODO: callbacks)
	void set_subclock(u32 clock) { m_subclock = clock; }
	void set_subclock(const XTAL &xtal) { m_subclock = xtal.value(); }

protected:
	upd78k0_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 iram_size, address_map_constructor mem_map, address_map_constructor sfr_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	// internal memory map
	void iram_map(address_map &map) ATTR_COLD;

	// internal helpers
	inline u16 register_base() const noexcept;
	inline u16 debug_register_base() const noexcept;

	// address spaces, caches & configuration
	address_space_config m_program_config;
	address_space_config m_iram_config;
	address_space_config m_sfr_config;
	const u16 m_iram_size;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_program_cache;
	memory_access<10, 1, 0, ENDIANNESS_LITTLE>::cache m_iram_cache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program_space;
	memory_access< 8, 1, 0, ENDIANNESS_LITTLE>::specific m_sfr_space;

	// miscellaneous configuration
	u32 m_subclock;

	// core registers and execution state
	u16 m_pc;
	u16 m_ppc;
	u8 m_psw;
	u16 m_sp;
	s32 m_icount;
};

// ======================> upd78053_device

class upd78053_device : public upd78k0_device
{
public:
	// device type constructors
	upd78053_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename X>
	upd78053_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, X &&subclock)
		: upd78053_device(mconfig, tag, owner, clock)
	{
		set_subclock(std::forward<X>(subclock));
	}

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// type-specific internal memory maps
	void mem_map(address_map &map) ATTR_COLD;
	void sfr_map(address_map &map) ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(UPD78053, upd78053_device)

#endif // MAME_CPU_UPD78K_UPD78K0_H
