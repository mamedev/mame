// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Alpha Denshi "NeoGeo" palette devices

***************************************************************************/

#ifndef MAME_VIDEO_ALPHA68K_PALETTE_H
#define MAME_VIDEO_ALPHA68K_PALETTE_H

#pragma once



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
	void set_entries(u32 entries) { m_entries = entries; }
	// terminology is from NeoGeo development wiki page
	u16 get_reference_pen() { return 0; }
	u16 get_backdrop_pen() { return m_entries-1; }

	// I/O operations
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_READ16_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint32_t palette_entries() const override { return m_entries; }
private:
	std::vector<uint16_t> m_paletteram;
	u32 m_entries;
	void create_rgb_lookups();
	uint8_t      m_palette_lookup[32][4];
	inline void set_color_entry(u16 offset, u16 pal_data, int shift);
};


// device type definition
DECLARE_DEVICE_TYPE(ALPHA68K_PALETTE, alpha68k_palette_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_VIDEO_ALPHA68K_PALETTE_H
