// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson
/*************************************************************************

    nmk112.h

**************************************************************************/

#ifndef MAME_MACHINE_NMK112_H
#define MAME_MACHINE_NMK112_H

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class nmk112_device : public device_t
{
public:
	nmk112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> nmk112_device &set_rom0_tag(T &&tag) { m_rom[0].set_tag(std::forward<T>(tag)); return *this; }
	template <typename T> nmk112_device &set_rom1_tag(T &&tag) { m_rom[1].set_tag(std::forward<T>(tag)); return *this; }
	template <typename T> nmk112_device &set_oki0_space_tag(T &&tag) { m_oki0_space.set_tag(std::forward<T>(tag), 0); return *this; }
	template <typename T> nmk112_device &set_oki1_space_tag(T &&tag) { m_oki1_space.set_tag(std::forward<T>(tag), 0); return *this; }
	nmk112_device &set_page_mask(uint8_t mask) { m_page_mask = ~mask; return *this; }

	void okibank_w(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	void do_bankswitch(offs_t offset, uint8_t data);
	bool is_paged(uint8_t chip) const { return (m_page_mask & (1 << chip)); }
	uint32_t page_offset(uint8_t chip, uint8_t slot) const { return (is_paged(chip) && (slot == 0)) ? 0x400 : 0; }

	memory_bank_array_creator<4> m_samplebank[2];
	memory_bank_array_creator<4> m_tablebank[2];
	optional_region_ptr_array<uint8_t, 2> m_rom;
	optional_address_space m_oki0_space;
	optional_address_space m_oki1_space;

	// internal state

	/* which chips have their sample address table divided into pages */
	uint8_t m_page_mask;

	uint8_t m_current_bank[8];

	uint32_t m_size[2];
};

DECLARE_DEVICE_TYPE(NMK112, nmk112_device)


#endif // MAME_MACHINE_NMK112_H
