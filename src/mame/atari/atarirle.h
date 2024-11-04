// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarirle.h

    Common RLE-based motion object management functions for early 90's
    Atari raster games.

***************************************************************************/

#ifndef MAME_ATARI_ATARIRLE_H
#define MAME_ATARI_ATARIRLE_H

#include "memarray.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define ATARIRLE_PRIORITY_SHIFT     12
#define ATARIRLE_BANK_SHIFT         15
#define ATARIRLE_PRIORITY_MASK      ((0xffff << ATARIRLE_PRIORITY_SHIFT) & 0xffff)
#define ATARIRLE_DATA_MASK          (ATARIRLE_PRIORITY_MASK ^ 0xffff)

#define ATARIRLE_CONTROL_MOGO       1
#define ATARIRLE_CONTROL_ERASE      2
#define ATARIRLE_CONTROL_FRAME      4

#define ATARIRLE_COMMAND_NOP        0
#define ATARIRLE_COMMAND_DRAW       1
#define ATARIRLE_COMMAND_CHECKSUM   2



//**************************************************************************
//  TYPES & STRUCTURES
//**************************************************************************

// description of the motion objects
struct atari_rle_objects_config
{
	struct entry { u16 data[8]; };

	u16          m_leftclip;           // left clip coordinate
	u16          m_rightclip;          // right clip coordinate
	u16          m_palettebase;        // base palette entry

	entry        m_code_entry;           // mask for the code index
	entry        m_color_entry;          // mask for the color
	entry        m_xpos_entry;           // mask for the X position
	entry        m_ypos_entry;           // mask for the Y position
	entry        m_scale_entry;          // mask for the scale factor
	entry        m_hflip_entry;          // mask for the horizontal flip
	entry        m_order_entry;          // mask for the order
	entry        m_priority_entry;       // mask for the priority
	entry        m_vram_entry;           // mask for the VRAM target
};


// ======================> atari_rle_objects_device

// device type definition
DECLARE_DEVICE_TYPE(ATARI_RLE_OBJECTS, atari_rle_objects_device)

class atari_rle_objects_device : public device_t,
									public device_video_interface,
									public atari_rle_objects_config
{
public:
	// construction/destruction
	atari_rle_objects_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, const atari_rle_objects_config &config)
		: atari_rle_objects_device(mconfig, tag, owner, clock)
	{
		set_config(config);
	}

	atari_rle_objects_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_config(const atari_rle_objects_config &config) { static_cast<atari_rle_objects_config &>(*this) = config; }

	// control handlers
	void control_write(u8 data);
	void command_write(u8 data);

	// render helpers
	void vblank_callback(screen_device &screen, bool state);

	// getters
	bitmap_ind16 &vram(int idx) { return m_vram[idx][(m_control_bits & ATARIRLE_CONTROL_FRAME) >> 2]; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// a sprite parameter, which is a word index + shift + mask
	class sprite_parameter
	{
	public:
		sprite_parameter();
		bool set(const atari_rle_objects_config::entry &input) { return set(input.data); }
		bool set(const u16 input[8]);
		u16 extract(memory_array &array, int offset) const { return (array.read(offset + m_word) >> m_shift) & m_mask; }
		u16 shift() const { return m_shift; }
		u16 mask() const { return m_mask; }

	private:
		u16          m_word;             // word index
		u16          m_shift;            // shift amount
		u16          m_mask;             // final mask
	};

	// internal structure describing each object in the ROMs
	struct object_info
	{
		s16          width;
		s16          height;
		s16          xoffs;
		s16          yoffs;
		u8           bpp;
		const u16 *  table;
		const u16 *  data;
	};

	// internal helpers
	inline int round_to_powerof2(int value);
	void build_rle_tables();
	int count_objects();
	void prescan_rle(int which);
	void compute_checksum();
	void sort_and_render();
	void draw_rle(bitmap_ind16 &bitmap, const rectangle &clip, int code, int color, int hflip, int vflip, int x, int y, int xscale, int yscale);
	void draw_rle_zoom(bitmap_ind16 &bitmap, const rectangle &clip, const object_info &info, u32 palette, int sx, int sy, int scalex, int scaley);
	void draw_rle_zoom_hflip(bitmap_ind16 &bitmap, const rectangle &clip, const object_info &info, u32 palette, int sx, int sy, int scalex, int scaley);
	void hilite_object(bitmap_ind16 &bitmap, int hilite);

	// derived state
	int             m_bitmapwidth = 0;        // width of the full playfield bitmap
	int             m_bitmapheight = 0;       // height of the full playfield bitmap
	int             m_bitmapxmask = 0;        // x coordinate mask for the playfield bitmap
	int             m_bitmapymask = 0;        // y coordinate mask for the playfield bitmap
	rectangle       m_cliprect;           // clipping rectangle

	// masks
	sprite_parameter    m_codemask;           // mask for the code index
	sprite_parameter    m_colormask;          // mask for the color
	sprite_parameter    m_xposmask;           // mask for the X position
	sprite_parameter    m_yposmask;           // mask for the Y position
	sprite_parameter    m_scalemask;          // mask for the scale factor
	sprite_parameter    m_hflipmask;          // mask for the horizontal flip
	sprite_parameter    m_ordermask;          // mask for the order
	sprite_parameter    m_prioritymask;       // mask for the priority
	sprite_parameter    m_vrammask;           // mask for the VRAM target

	// ROM information
	required_region_ptr<u16> m_rombase;    // pointer to the base of the GFX ROM
	int                 m_objectcount = 0;        // number of objects in the ROM
	std::vector<object_info> m_info;               // list of info records

	// rendering state
	bitmap_ind16     m_vram[2][2];         // pointers to VRAM bitmaps and backbuffers
	int              m_partial_scanline = 0;   // partial update scanline

	// control state
	u8               m_control_bits = 0;       // current control bits
	u8               m_command = 0;            // current command
	u16              m_checksums[256];     // checksums for each 0x40000 bytes
	memory_array     m_ram;

	// tables
	u8               m_rle_bpp[8];
	u16 *            m_rle_table[8]{};
	u16              m_rle_table_data[0x500];
};

#endif // MAME_ATARI_ATARIRLE_H
