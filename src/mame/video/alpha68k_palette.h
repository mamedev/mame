// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Alpha Denshi "NeoGeo" palette devices

***************************************************************************/

#ifndef MAME_VIDEO_ALPHA68K_PALETTE_H
#define MAME_VIDEO_ALPHA68K_PALETTE_H

#pragma once

#include <algorithm>

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> alpha68k_palette_device

class alpha68k_palette_device : public device_t,
								public device_palette_interface
{
public:
	// construction/destruction
	alpha68k_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_banknum(u32 banknum) { m_banknum = std::max<u32>(1, banknum); }
	void set_has_shadow(u32 has_shadow) { m_has_shadow = has_shadow; }
	void set_entries(u32 entries) { m_entries = entries; }

	// terminology is from NeoGeo development wiki page
	// getters
	u16 get_reference_pen() { return 0; }
	u16 get_backdrop_pen() { return m_entries-1; }
	const pen_t *pens() { return device_palette_interface::pens() + m_bank_base + (m_shadow ? m_shadow_base : 0); }

	// setters
	void set_bank(u32 bank) { if ((m_bank < m_banknum) && (m_bank != bank)) { m_bank = bank; m_bank_base = m_bank * m_entries; } }
	void set_shadow(bool shadow) { if (m_has_shadow) { m_shadow = shadow; } }

	// I/O operations
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint32_t palette_entries() const override { return m_entries * m_banknum * (m_has_shadow ? 2 : 1); }
private:
	// internal states
	std::vector<uint16_t> m_paletteram;
	std::unique_ptr<int[]> m_sync_color_shift;

	// configurations
	u32 m_banknum;
	bool m_has_shadow;
	u32 m_entries;

	// bank status
	u32 m_bank;
	u32 m_bank_base;

	// shadow status
	bool m_shadow;
	u32 m_shadow_base;

	void create_rgb_lookups();
	uint8_t      m_palette_lookup[32][4];
	inline void set_color_entry(u16 offset, u16 pal_data);
};


// device type definition
DECLARE_DEVICE_TYPE(ALPHA68K_PALETTE, alpha68k_palette_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_VIDEO_ALPHA68K_PALETTE_H
