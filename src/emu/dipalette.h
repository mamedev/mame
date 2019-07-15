// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dipalette.h

    Device palette interface.

******************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DIPALETTE_H
#define MAME_EMU_DIPALETTE_H


//**************************************************************************
//  CONSTANTS
//**************************************************************************

constexpr float PALETTE_DEFAULT_SHADOW_FACTOR = 0.6f;
constexpr float PALETTE_DEFAULT_HIGHLIGHT_FACTOR = 1.0f/PALETTE_DEFAULT_SHADOW_FACTOR;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef u16 indirect_pen_t;


// ======================> device_palette_interface

class device_palette_interface : public device_interface
{
	friend class screen_device;

	static constexpr int MAX_SHADOW_PRESETS = 4;

public:
	// getters
	u32 entries() const { return palette_entries(); }
	u32 indirect_entries() const { return palette_indirect_entries(); }
	palette_t *palette() const { return m_palette; }
	const pen_t &pen(int index) const { return m_pens[index]; }
	const pen_t *pens() const { return m_pens; }
	pen_t *shadow_table() const { return m_shadow_table; }
	rgb_t pen_color(pen_t pen) const { return m_palette->entry_color(pen); }
	double pen_contrast(pen_t pen) const { return m_palette->entry_contrast(pen); }
	pen_t black_pen() const { return m_black_pen; }
	pen_t white_pen() const { return m_white_pen; }
	bool shadows_enabled() const { return palette_shadows_enabled(); }
	bool hilights_enabled() const { return palette_hilights_enabled(); }

	// setters
	void set_pen_color(pen_t pen, rgb_t rgb) { m_palette->entry_set_color(pen, rgb); }
	void set_pen_red_level(pen_t pen, u8 level) { m_palette->entry_set_red_level(pen, level); }
	void set_pen_green_level(pen_t pen, u8 level) { m_palette->entry_set_green_level(pen, level); }
	void set_pen_blue_level(pen_t pen, u8 level) { m_palette->entry_set_blue_level(pen, level); }
	void set_pen_color(pen_t pen, u8 r, u8 g, u8 b) { m_palette->entry_set_color(pen, rgb_t(r, g, b)); }
	void set_pen_colors(pen_t color_base, const rgb_t *colors, int color_count) { while (color_count--) set_pen_color(color_base++, *colors++); }
	template <size_t N> void set_pen_colors(pen_t color_base, const rgb_t (&colors)[N]) { set_pen_colors(color_base, colors, N); }
	void set_pen_colors(pen_t color_base, const std::vector<rgb_t> &colors) { for (unsigned int i=0; i != colors.size(); i++) set_pen_color(color_base+i, colors[i]); }
	void set_pen_contrast(pen_t pen, double bright) { m_palette->entry_set_contrast(pen, bright); }

	// indirection (aka colortables)
	indirect_pen_t pen_indirect(int index) const { return m_indirect_pens[index]; }
	rgb_t indirect_color(int index) const { return m_indirect_colors[index]; }
	void set_indirect_color(int index, rgb_t rgb);
	void set_pen_indirect(pen_t pen, indirect_pen_t index);
	u32 transpen_mask(gfx_element &gfx, u32 color, indirect_pen_t transcolor) const;

	// shadow config
	void set_shadow_factor(double factor) { assert(m_shadow_group != 0); m_palette->group_set_contrast(m_shadow_group, factor); }
	void set_highlight_factor(double factor) { assert(m_hilight_group != 0); m_palette->group_set_contrast(m_hilight_group, factor); }
	void set_shadow_mode(int mode) { assert(mode >= 0 && mode < MAX_SHADOW_PRESETS); m_shadow_table = m_shadow_tables[mode].base; }

protected:
	// construction/destruction
	device_palette_interface(const machine_config &mconfig, device_t &device);

	// interface-level overrides
	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_pre_save() override;
	virtual void interface_post_load() override;
	virtual void interface_post_stop() override;

	// configuration-related overrides
	virtual u32 palette_entries() const = 0;
	virtual u32 palette_indirect_entries() const { return 0; }
	virtual bool palette_shadows_enabled() const { return false; }
	virtual bool palette_hilights_enabled() const { return false; }

private:
	// internal helpers
	void allocate_palette(u32 numentries);
	void allocate_color_tables();
	void allocate_shadow_tables();
public: // needed by konamigx
	void set_shadow_dRGB32(int mode, int dr, int dg, int db, bool noclip);
private:
	void configure_rgb_shadows(int mode, float factor);

	// internal state
	palette_t *         m_palette;              // the palette itself
	const pen_t *       m_pens;                 // remapped palette pen numbers
	bitmap_format       m_format;               // format assumed for palette data
	pen_t *             m_shadow_table;         // table for looking up a shadowed pen
	u32                 m_shadow_group;         // index of the shadow group, or 0 if none
	u32                 m_hilight_group;        // index of the hilight group, or 0 if none
	pen_t               m_white_pen;            // precomputed white pen value
	pen_t               m_black_pen;            // precomputed black pen value

	// indirection state
	std::vector<rgb_t> m_indirect_colors;          // actual colors set for indirection
	std::vector<indirect_pen_t> m_indirect_pens;   // indirection values

	struct shadow_table_data
	{
		pen_t *            base;               // pointer to the base of the table
		s16                dr;                 // delta red value
		s16                dg;                 // delta green value
		s16                db;                 // delta blue value
		bool               noclip;             // clip?
	};
	shadow_table_data   m_shadow_tables[MAX_SHADOW_PRESETS]; // array of shadow table data

	std::vector<pen_t> m_save_pen;           // pens for save/restore
	std::vector<float> m_save_contrast;      // brightness for save/restore

	std::vector<pen_t> m_pen_array;
	std::vector<pen_t> m_shadow_array;
	std::vector<pen_t> m_hilight_array;
};

// interface type iterator
typedef device_interface_iterator<device_palette_interface> palette_interface_iterator;


#endif  // MAME_EMU_DIPALETTE_H
