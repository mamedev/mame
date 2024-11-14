// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarimo.h

    Common motion object management functions for Atari raster games.

***************************************************************************/

#ifndef MAME_ATARI_ATARIMO_H
#define MAME_ATARI_ATARIMO_H

#include "video/sprite.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// description of the motion objects
struct atari_motion_objects_config
{
	struct entry { uint16_t data[4]; };
	struct dual_entry { uint16_t data_lower[4]; uint16_t data_upper[4]; };

	uint8_t               m_gfxindex;           // index to which gfx system
	uint8_t               m_bankcount;          // number of motion object banks
	bool                m_linked;             // are the entries linked?
	bool                m_split;              // are the entries split?
	bool                m_reverse;            // render in reverse order?
	bool                m_swapxy;             // render in swapped X/Y order?
	bool                m_nextneighbor;       // does the neighbor bit affect the next object?
	uint16_t              m_slipheight;         // pixels per SLIP entry (0 for no-slip)
	uint8_t               m_slipoffset;         // pixel offset for SLIPs
	uint16_t              m_maxperline;         // maximum number of links to visit/scanline (0=all)

	uint16_t              m_palettebase;        // base palette entry
	uint16_t              m_maxcolors;          // maximum number of colors (remove me)
	uint8_t               m_transpen;           // transparent pen index

	entry               m_link_entry;           // mask for the link
	dual_entry          m_code_entry;           // mask for the code index
	dual_entry          m_color_entry;          // mask for the color/priority
	entry               m_xpos_entry;           // mask for the X position
	entry               m_ypos_entry;           // mask for the Y position
	entry               m_width_entry;          // mask for the width, in tiles*/
	entry               m_height_entry;         // mask for the height, in tiles
	entry               m_hflip_entry;          // mask for the horizontal flip
	entry               m_vflip_entry;          // mask for the vertical flip
	entry               m_priority_entry;       // mask for the priority
	entry               m_neighbor_entry;       // mask for the neighbor
	entry               m_absolute_entry;       // mask for absolute coordinates
	entry               m_special_entry;        // mask for the special value
	uint16_t              m_specialvalue;         // resulting value to indicate "special"
};



// ======================> atari_motion_objects_device

// device type definition
DECLARE_DEVICE_TYPE(ATARI_MOTION_OBJECTS, atari_motion_objects_device)

class atari_motion_objects_device : public sprite16_device_ind16,
									public device_video_interface,
									public atari_motion_objects_config
{
	static const int MAX_PER_BANK = 1024;

public:
	// construction/destruction
	template <typename T>
	atari_motion_objects_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag, const atari_motion_objects_config &config)
		: atari_motion_objects_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
		set_config(config);
	}

	atari_motion_objects_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_gfxdecode(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_config(const atari_motion_objects_config &config) { static_cast<atari_motion_objects_config &>(*this) = config; }
	void set_xoffset(int xoffset) { m_xoffset = xoffset; }

	// getters
	int bank() const { return m_bank; }
	int xscroll() const { return m_xscroll; }
	int yscroll() const { return m_yscroll; }
	std::vector<uint32_t> &code_lookup() { return m_codelookup; }
	std::vector<uint8_t> &color_lookup() { return m_colorlookup; }
	std::vector<uint8_t> &gfx_lookup() { return m_gfxlookup; }

	// setters
	void set_bank(int bank) { m_bank = bank; }
	void set_xscroll(int xscroll) { m_xscroll = xscroll & m_bitmapxmask; }
	void set_yscroll(int yscroll) { m_yscroll = yscroll & m_bitmapymask; }
	void set_scroll(int xscroll, int yscroll) { set_xscroll(xscroll); set_yscroll(yscroll); }
	void set_slipram(uint16_t *ram) { m_slipram = ram; }

	// rendering
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void apply_stain(bitmap_ind16 &bitmap, uint16_t *pf, uint16_t const *mo, int x, int y);

	// memory access
	uint16_t &slipram(int offset) { return m_slipram[offset]; }

	// constants
	static const int PRIORITY_SHIFT = 12;
	static const uint16_t PRIORITY_MASK = (0xffff << PRIORITY_SHIFT) & 0xffff;
	static const uint16_t DATA_MASK = PRIORITY_MASK ^ 0xffff;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(force_update);

private:
	// internal helpers
	int compute_log(int value);
	int round_to_powerof2(int value);
	void build_active_list(int link);
	void render_object(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint16_t *entry);

	// a sprite parameter, which is a word index + shift + mask
	class sprite_parameter
	{
	public:
		sprite_parameter();
		bool set(const atari_motion_objects_config::entry &input) { return set(input.data); }
		bool set(const uint16_t input[4]);
		uint16_t extract(const uint16_t *data) const { return (data[m_word] >> m_shift) & m_mask; }
		uint16_t shift() const { return m_shift; }
		uint16_t mask() const { return m_mask; }

	private:
		uint16_t              m_word;             // word index
		uint16_t              m_shift;            // shift amount
		uint16_t              m_mask;             // final mask
	};

	// a sprite parameter, which is a word index + shift + mask
	class dual_sprite_parameter
	{
	public:
		dual_sprite_parameter();
		bool set(const atari_motion_objects_config::dual_entry &input);
		uint32_t extract(const uint16_t *data) const { return m_lower.extract(data) | (m_upper.extract(data) << m_uppershift); }
		uint32_t mask() const { return m_lower.mask() | (m_upper.mask() << m_uppershift); }

	private:
		sprite_parameter    m_lower;            // lower parameter
		sprite_parameter    m_upper;            // upper parameter
		uint16_t              m_uppershift;       // upper shift
	};

	// parameter masks
	sprite_parameter        m_linkmask;         // mask for the link
	sprite_parameter        m_gfxmask;          // mask for the graphics bank
	dual_sprite_parameter   m_codemask;         // mask for the code index
	dual_sprite_parameter   m_colormask;        // mask for the color
	sprite_parameter        m_xposmask;         // mask for the X position
	sprite_parameter        m_yposmask;         // mask for the Y position
	sprite_parameter        m_widthmask;        // mask for the width, in tiles*/
	sprite_parameter        m_heightmask;       // mask for the height, in tiles
	sprite_parameter        m_hflipmask;        // mask for the horizontal flip
	sprite_parameter        m_vflipmask;        // mask for the vertical flip
	sprite_parameter        m_prioritymask;     // mask for the priority
	sprite_parameter        m_neighbormask;     // mask for the neighbor
	sprite_parameter        m_absolutemask;     // mask for absolute coordinates
	sprite_parameter        m_specialmask;      // mask for the special value

	// derived tile information
	int                     m_tilewidth;          // width of non-rotated tile
	int                     m_tileheight;         // height of non-rotated tile
	int                     m_tilexshift;         // bits to shift X coordinate when drawing
	int                     m_tileyshift;         // bits to shift Y coordinate when drawing

	// derived bitmap information
	int                     m_bitmapwidth;        // width of the full playfield bitmap
	int                     m_bitmapheight;       // height of the full playfield bitmap
	int                     m_bitmapxmask;        // x coordinate mask for the playfield bitmap
	int                     m_bitmapymask;        // y coordinate mask for the playfield bitmap

	// derived sprite information
	int                     m_entrycount;         // number of entries per bank
	int                     m_entrybits;          // number of bits needed to represent entrycount
	int                     m_spriterammask;      // combined mask when accessing sprite RAM with raw addresses
	int                     m_spriteramsize;      // total size of sprite RAM, in entries
	int                     m_slipshift;          // log2(pixels_per_SLIP)
	int                     m_sliprammask;        // combined mask when accessing SLIP RAM with raw addresses
	int                     m_slipramsize;        // total size of SLIP RAM, in entries

	// live state
	emu_timer *             m_force_update_timer = nullptr;   // timer for forced updating
	uint32_t                m_bank;               // current bank number
	uint32_t                m_xscroll;            // xscroll offset
	uint32_t                m_yscroll;            // yscroll offset

	// arrays
	uint16_t *              m_slipram;    // pointer to the SLIP RAM
	optional_shared_ptr<u16> m_slipramshare;
	std::vector<uint32_t>   m_codelookup;       // lookup table for codes
	std::vector<uint8_t>    m_colorlookup;       // lookup table for colors
	std::vector<uint8_t>    m_gfxlookup;         // lookup table for graphics

	uint16_t                m_activelist[MAX_PER_BANK*4]; // active list
	uint16_t *              m_activelast;           // last entry in the active list

	uint32_t                m_last_xpos;          // (during processing) the previous X position
	uint32_t                m_next_xpos;          // (during processing) the next X position

	int                     m_xoffset;            // global xoffset for sprites

	required_device<gfxdecode_device> m_gfxdecode;
};


#endif // MAME_ATARI_ATARIMO_H
