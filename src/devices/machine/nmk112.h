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
	nmk112_device &set_page_mask(uint8_t mask) { m_page_mask = ~mask; return *this; }

	void okibank_w(offs_t offset, u8 data);

	void oki0_map(address_map &map) ATTR_COLD;
	void oki1_map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	bool is_paged(uint8_t chip) const { return BIT(m_page_mask, chip); }

	void oki_map(unsigned which, address_map &map) ATTR_COLD;

	memory_bank_array_creator<4> m_samplebank[2];
	memory_bank_array_creator<4> m_tablebank[2];
	optional_region_ptr_array<uint8_t, 2> m_rom;

	// internal state

	/* which chips have their sample address table divided into pages */
	uint8_t m_page_mask;

	uint8_t m_bankmask[2];
};

DECLARE_DEVICE_TYPE(NMK112, nmk112_device)


#endif // MAME_MACHINE_NMK112_H
