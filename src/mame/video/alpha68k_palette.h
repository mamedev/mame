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
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 read(offs_t offset);

protected:
	// construction/destruction
	alpha68k_palette_device(const machine_config &mconfig, device_type &type, const char *tag, device_t *owner, uint32_t clock);
	// device-level overrides
	virtual void device_start() override;

	virtual uint32_t palette_entries() const override { return m_entries; }

	virtual inline void set_color_entry(u16 offset);
	virtual inline void update_color(u16 entry, u16 offset);

	std::vector<uint16_t> m_paletteram;
	int m_sync_color_shift[2]; // 2 banks
	u32 m_entries;
	void create_rgb_lookups();
	uint8_t      m_palette_lookup[32][4];
};

// ======================> neogeo_palette_device

class neogeo_palette_device : public alpha68k_palette_device
{
public:
	// construction/destruction
	neogeo_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// setters
	void set_bank(bool bank) { if (m_bank != bank) { m_bank = bank; m_bankaddr = bank ? m_entries : 0; } }
	void set_shadow(bool shadow) { m_shadow = shadow; }

	// getters
	const pen_t *pens() { return device_palette_interface::pens() + (m_shadow ? (m_entries << 1) : 0) + m_bankaddr; }

	// I/O operations
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual uint32_t palette_entries() const override { return m_entries * 2 * 2; } // with bank and shadow

	virtual inline void set_color_entry(u16 offset) override;
	virtual inline void update_color(u16 entry, u16 offset) override;

private:
	bool m_bank; // palette bank
	offs_t m_bankaddr; // banked address base
	bool m_shadow; // overall shadow
};


// device type definition
DECLARE_DEVICE_TYPE(ALPHA68K_PALETTE, alpha68k_palette_device)
DECLARE_DEVICE_TYPE(NEOGEO_PALETTE,   neogeo_palette_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_VIDEO_ALPHA68K_PALETTE_H
