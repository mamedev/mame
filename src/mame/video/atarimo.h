// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarimo.h

    Common motion object management functions for Atari raster games.

***************************************************************************/

#ifndef __ATARIMO__
#define __ATARIMO__

#include "sprite.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ATARI_MOTION_OBJECTS_ADD(_tag, _screen, _config) \
	MCFG_DEVICE_ADD(_tag, ATARI_MOTION_OBJECTS, 0) \
	MCFG_VIDEO_SET_SCREEN(_screen) \
	atari_motion_objects_device::static_set_config(*device, _config);

#define MCFG_ATARI_MOTION_OBJECTS_GFXDECODE(_gfxtag) \
	atari_motion_objects_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// description of the motion objects
struct atari_motion_objects_config
{
	struct entry { UINT16 data[4]; };
	struct dual_entry { UINT16 data_lower[4]; UINT16 data_upper[4]; };

	UINT8               m_gfxindex;           // index to which gfx system
	UINT8               m_bankcount;          // number of motion object banks
	bool                m_linked;             // are the entries linked?
	bool                m_split;              // are the entries split?
	bool                m_reverse;            // render in reverse order?
	bool                m_swapxy;             // render in swapped X/Y order?
	bool                m_nextneighbor;       // does the neighbor bit affect the next object?
	UINT16              m_slipheight;         // pixels per SLIP entry (0 for no-slip)
	UINT8               m_slipoffset;         // pixel offset for SLIPs
	UINT16              m_maxperline;         // maximum number of links to visit/scanline (0=all)

	UINT16              m_palettebase;        // base palette entry
	UINT16              m_maxcolors;          // maximum number of colors (remove me)
	UINT8               m_transpen;           // transparent pen index

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
	UINT16              m_specialvalue;         // resulting value to indicate "special"
};



// ======================> atari_motion_objects_device

// device type definition
extern const device_type ATARI_MOTION_OBJECTS;

class atari_motion_objects_device : public sprite16_device_ind16,
									public device_video_interface,
									public atari_motion_objects_config
{
	static const int MAX_PER_BANK = 1024;

public:
	// construction/destruction
	atari_motion_objects_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_config(device_t &device, const atari_motion_objects_config &config);

	// getters
	int bank() const { return m_bank; }
	int xscroll() const { return m_xscroll; }
	int yscroll() const { return m_yscroll; }
	std::vector<UINT16> &code_lookup() { return m_codelookup; }
	std::vector<UINT8> &color_lookup() { return m_colorlookup; }
	std::vector<UINT8> &gfx_lookup() { return m_gfxlookup; }

	// setters
	void set_bank(int bank) { m_bank = bank; }
	void set_xscroll(int xscroll) { m_xscroll = xscroll & m_bitmapxmask; }
	void set_yscroll(int yscroll) { m_yscroll = yscroll & m_bitmapymask; }
	void set_scroll(int xscroll, int yscroll) { set_xscroll(xscroll); set_yscroll(yscroll); }
	void set_slipram(UINT16 *ram) { m_slipram.set_target(ram, 2); }

	// rendering
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void apply_stain(bitmap_ind16 &bitmap, UINT16 *pf, UINT16 *mo, int x, int y);

	// memory access
	UINT16 &slipram(int offset) { return m_slipram[offset]; }

	// constants
	static const int PRIORITY_SHIFT = 12;
	static const UINT16 PRIORITY_MASK = (~0 << PRIORITY_SHIFT) & 0xffff;
	static const UINT16 DATA_MASK = PRIORITY_MASK ^ 0xffff;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// timer IDs
	enum
	{
		TID_FORCE_UPDATE
	};

	// internal helpers
	int compute_log(int value);
	int round_to_powerof2(int value);
	void build_active_list(int link);
	void render_object(bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT16 *entry);

	// a sprite parameter, which is a word index + shift + mask
	class sprite_parameter
	{
	public:
		sprite_parameter();
		bool set(const atari_motion_objects_config::entry &input) { return set(input.data); }
		bool set(const UINT16 input[4]);
		UINT16 extract(const UINT16 *data) const { return (data[m_word] >> m_shift) & m_mask; }
		UINT16 shift() const { return m_shift; }
		UINT16 mask() const { return m_mask; }

	private:
		UINT16              m_word;             // word index
		UINT16              m_shift;            // shift amount
		UINT16              m_mask;             // final mask
	};

	// a sprite parameter, which is a word index + shift + mask
	class dual_sprite_parameter
	{
	public:
		dual_sprite_parameter();
		bool set(const atari_motion_objects_config::dual_entry &input);
		UINT16 extract(const UINT16 *data) const { return m_lower.extract(data) | (m_upper.extract(data) << m_uppershift); }
		UINT16 mask() const { return m_lower.mask() | (m_upper.mask() << m_uppershift); }

	private:
		sprite_parameter    m_lower;            // lower parameter
		sprite_parameter    m_upper;            // upper parameter
		UINT16              m_uppershift;       // upper shift
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
	emu_timer *             m_force_update_timer;   // timer for forced updating
	UINT32                  m_bank;               // current bank number
	UINT32                  m_xscroll;            // xscroll offset
	UINT32                  m_yscroll;            // yscroll offset

	// arrays
	optional_shared_ptr<UINT16> m_slipram;    // pointer to the SLIP RAM
	std::vector<UINT16>   m_codelookup;       // lookup table for codes
	std::vector<UINT8>    m_colorlookup;       // lookup table for colors
	std::vector<UINT8>    m_gfxlookup;         // lookup table for graphics

	UINT16                  m_activelist[MAX_PER_BANK*4]; // active list
	UINT16 *                m_activelast;           // last entry in the active list

	UINT32                  m_last_xpos;          // (during processing) the previous X position
	UINT32                  m_next_xpos;          // (during processing) the next X position
	required_device<gfxdecode_device> m_gfxdecode;
};


#endif
