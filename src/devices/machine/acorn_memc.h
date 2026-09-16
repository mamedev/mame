// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller, Sandro Ronco
/**************************************************************************************************

    Acorn RISC Machine Memory Controller (MEMC)

**************************************************************************************************/

#ifndef MAME_MACHINE_ACORN_MEMC_H
#define MAME_MACHINE_ACORN_MEMC_H

#pragma once

#include "machine/acorn_vidc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> acorn_memc_device

class acorn_memc_device : public device_t, public device_memory_interface
{
public:
	acorn_memc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	acorn_memc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&vidc_tag)
		: acorn_memc_device(mconfig, tag, owner, clock)
	{
		m_vidc.set_tag(std::forward<T>(vidc_tag));
	}

	auto abort_w()   { return m_abort_w.bind(); }
	auto sirq_w()    { return m_sirq_w.bind();  }

	// enable/disable the output of the correct DRAM row and column for DRAM access.
	// This allows to emulate the correct DRAM mirrors that are used by RISC OS to detect the installed RAM,
	// but requires a bitswap on every access and is not required by machines with 2 or more MB of RAM.
	void output_dram_rowcol(bool v)  { m_output_dram_rowcol = v; }

	uint32_t logical_r(offs_t offset, uint32_t mem_mask = ~0);
	void logical_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void page_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void registers_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t high_mem_r(offs_t offset, uint32_t mem_mask = ~0);
	void high_mem_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void spvmd_w(int state);
	void sndrq_w(int state);
	void vidrq_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

private:
	void memc_map_debug_commands(const std::vector<std::string_view> &params);
	uint32_t dram_address(uint32_t address);
	bool is_valid_access(int page, bool write);
	uint32_t invalid_access(bool is_write, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void do_sound_dma();
	void do_video_dma();

	static constexpr const int m_page_sizes[4] = { 4096, 8192, 16384, 32768 };
	optional_device<acorn_vidc10_device> m_vidc;
	const address_space_config m_space_config;

	address_space *     m_space;
	devcb_write_line    m_abort_w;
	devcb_write_line    m_sirq_w;
	bool                m_output_dram_rowcol;
	int                 m_spvmd;
	uint8_t             m_pagesize;
	bool                m_latchrom;
	bool                m_video_dma_on;
	bool                m_sound_dma_on;
	bool                m_cursor_enabled;
	bool                m_os_mode;
	uint32_t            m_vidinit;
	uint32_t            m_vidstart;
	uint32_t            m_vidend;
	uint32_t            m_vidcur;
	uint32_t            m_cinit;
	uint32_t            m_sndstart;
	uint32_t            m_sndend;
	uint32_t            m_sndcur;
	uint32_t            m_sndendcur;
	int16_t             m_pages[0x2000];        // the logical RAM area is 32 megs, and the smallest page size is 4k
	uint8_t             m_pages_ppl[0x2000];
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_MEMC, acorn_memc_device)

#endif // MAME_MACHINE_ACORN_MEMC_H
