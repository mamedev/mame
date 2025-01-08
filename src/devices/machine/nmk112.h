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
	template <typename T> nmk112_device &set_rom0_tag(T &&tag) { m_rom0.set_tag(std::forward<T>(tag)); return *this; }
	template <typename T> nmk112_device &set_rom1_tag(T &&tag) { m_rom1.set_tag(std::forward<T>(tag)); return *this; }
	nmk112_device &set_page_mask(uint8_t mask) { m_page_mask = ~mask; return *this; }

	void okibank_w(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	void do_bankswitch( int offset, int data );

	// internal state

	/* which chips have their sample address table divided into pages */
	uint8_t m_page_mask;

	uint8_t m_current_bank[8];

	optional_region_ptr<uint8_t> m_rom0, m_rom1;
	int   m_size0, m_size1;
};

DECLARE_DEVICE_TYPE(NMK112, nmk112_device)


#endif // MAME_MACHINE_NMK112_H
