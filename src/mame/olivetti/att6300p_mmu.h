// license:BSD-3-Clause
// copyright-holders:D. Donohoe

/**********************************************************************

    AT&T 6300 Plus virtualization emulation

**********************************************************************/

#ifndef MAME_OLIVETTI_ATT6300P_MMU_H
#define MAME_OLIVETTI_ATT6300P_MMU_H

#pragma once

class att6300p_mmu_device :  public device_t, public device_memory_interface
{
public:
	// construction/destruction
	att6300p_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// All CPU accesses are directed to the MMU via these calls
	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// MMU configuration
	void set_protected_mode_enabled(bool enabled);
	void set_mem_mapping(uint32_t target_addr[32]);
	void set_mem_setup_enabled(bool enabled);
	void set_io_setup_enabled(bool enabled);
	void set_memprot_enabled(bool enabled);
	void set_io_read_traps_enabled(bool enabled);
	void set_io_write_traps_enabled(bool enabled);

	auto trapio_callback() { return m_trapio.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	void update_fastpath();

	devcb_write32 m_trapio;

	const address_space_config m_mem_config;
	const address_space_config m_io_config;
	memory_access<20, 0, 0, ENDIANNESS_BIG>::specific m_mem8_space;
	memory_access<24, 1, 0, ENDIANNESS_LITTLE>::specific m_mem16_space;
	address_space *m_io;

	bool m_protected_mode;
	uint32_t m_map_table[32];
	uint32_t m_map_imask, m_map_omask;
	bool m_mem_wr_fastpath;
	uint32_t m_mem_prot_limit;
	bool m_mem_setup_enabled;
	bool m_io_setup_enabled;
	bool m_io_read_traps_enabled;
	bool m_io_write_traps_enabled;

	bool m_mem_prot_table[1024];
	uint8_t m_io_prot_table[4096];

	// Per-port IO protection flags
	enum {
		IO_PROT_NOTRAP			= 2,
		IO_PROT_INHIBIT_READ	= 4,
		IO_PROT_INHIBIT_WRITE	= 8,
	};

	// Flags for trapped IO accesses
	enum {
		TRAPIO_FLAG__IORC		= 2,	// Active Low
		TRAPIO_FLAG__LBHE		= 4,	// Active Low
		TRAPIO_FLAG_LA0			= 8
	};
};

// device type definition
DECLARE_DEVICE_TYPE(ATT6300P_MMU, att6300p_mmu_device)

#endif // MAME_OLIVETTI_ATT6300P_MMU_H
