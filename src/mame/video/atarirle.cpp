// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarirle.c

    RLE sprite handling for early-to-mid 90's Atari raster games.

****************************************************************************

    Description:

    Beginning with Hydra, and continuing through to Primal Rage, Atari used
    RLE-compressed sprites. These sprites were decoded, colored, and scaled
    on the fly using an AMD 29C101 ALU unit. The instructions for the ALU
    were read from 3 512-byte PROMs and fed into the instruction input.

    See the bottom of the source for more details on the operation of these
    components.

***************************************************************************/

#include "emu.h"
#include "atarirle.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const device_type ATARI_RLE_OBJECTS = &device_creator<atari_rle_objects_device>;

enum { atarirle_hilite_index = -1 };



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  round_to_powerof2: Rounds a number up to the
//  nearest power of 2. Even powers of 2 are
//  rounded up to the next greatest power
//  (e.g., 4 returns 8).
//-------------------------------------------------

inline int atari_rle_objects_device::round_to_powerof2(int value)
{
	int log = 0;

	if (value == 0)
		return 1;
	while ((value >>= 1) != 0)
		log++;
	return 1 << (log + 1);
}



//**************************************************************************
//  CORE DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_rle_objects_device: Constructor
//-------------------------------------------------

atari_rle_objects_device::atari_rle_objects_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ATARI_RLE_OBJECTS, "Atari RLE Motion Objects", tag, owner, clock, "atari_rle", __FILE__),
		device_video_interface(mconfig, *this),
		m_rombase(*this, DEVICE_SELF)
{
}


//-------------------------------------------------
//  static_set_config: Set the tag of the
//  sound CPU
//-------------------------------------------------

void atari_rle_objects_device::static_set_config(device_t &device, const atari_rle_objects_config &config)
{
	atari_rle_objects_device &target = downcast<atari_rle_objects_device &>(device);
	static_cast<atari_rle_objects_config &>(target) = config;
}


//-------------------------------------------------
//  control_write: Write handler for MO control
//  bits.
//-------------------------------------------------

WRITE8_MEMBER(atari_rle_objects_device::control_write)
{
//logerror("atarirle_control_w(%d)\n", bits);

	// do nothing if nothing changed
	int oldbits = m_control_bits;
	if (oldbits == data)
		return;

	// force a partial update first
	int scanline = m_screen->vpos();
	m_screen->update_partial(scanline);

	// if the erase flag was set, erase the front map
	if ((oldbits & ATARIRLE_CONTROL_ERASE) != 0)
	{
		// compute the top and bottom of the rect
		rectangle cliprect = m_cliprect;
		if (m_partial_scanline + 1 > cliprect.min_y)
			cliprect.min_y = m_partial_scanline + 1;
		if (scanline < cliprect.max_y)
			cliprect.max_y = scanline;

//logerror("  partial erase %d-%d (frame %d)\n", cliprect.min_y, cliprect.max_y, (oldbits & ATARIRLE_CONTROL_FRAME) >> 2);

		// erase the bitmap
		m_vram[0][(oldbits & ATARIRLE_CONTROL_FRAME) >> 2].fill(0, cliprect);
		if (m_vrammask.mask() != 0)
			m_vram[1][(oldbits & ATARIRLE_CONTROL_FRAME) >> 2].fill(0, cliprect);
	}

	// update the bits
	m_control_bits = data;

	// if mogo is set, do a render on the rising edge
	if ((oldbits & ATARIRLE_CONTROL_MOGO) == 0 && (data & ATARIRLE_CONTROL_MOGO) != 0)
	{
		if (m_command == ATARIRLE_COMMAND_DRAW)
			sort_and_render();
		else if (m_command == ATARIRLE_COMMAND_CHECKSUM)
			compute_checksum();
	}

	// remember where we left off
	m_partial_scanline = scanline;
}


//-------------------------------------------------
//  command_write: Write handler for MO command
//  bits.
//-------------------------------------------------

WRITE8_MEMBER(atari_rle_objects_device::command_write)
{
	m_command = data;
}


//-------------------------------------------------
//  vblank_callback: VBLANK callback.
//-------------------------------------------------

void atari_rle_objects_device::vblank_callback(screen_device &screen, bool state)
{
	// on the rising edge, if the erase flag is set, erase to the end of the screen
	if (state)
	{
		if (m_control_bits & ATARIRLE_CONTROL_ERASE)
		{
			// compute top only; bottom is equal to visible_area
			rectangle cliprect = m_cliprect;
			if (m_partial_scanline + 1 > cliprect.min_y)
				cliprect.min_y = m_partial_scanline + 1;

	//logerror("  partial erase %d-%d (frame %d)\n", cliprect.min_y, cliprect.max_y, (m_control_bits & ATARIRLE_CONTROL_FRAME) >> 2);

			// erase the bitmap
			m_vram[0][(m_control_bits & ATARIRLE_CONTROL_FRAME) >> 2].fill(0, cliprect);
			if (m_vrammask.mask() != 0)
				m_vram[1][(m_control_bits & ATARIRLE_CONTROL_FRAME) >> 2].fill(0, cliprect);
		}

		// reset the partial scanline to -1 so we can detect full updates
		m_partial_scanline = -1;
	}
}


//-------------------------------------------------
//  device_start: Configures the motion objects
//  using the input description. Allocates all
//  memory necessary and generates the attribute
//  lookup table.
//-------------------------------------------------

void atari_rle_objects_device::device_start()
{
	// resolve our memory
	memory_share *share = owner()->memshare(tag());
	if (share == nullptr)
		throw emu_fatalerror("Unable to find memory share '%s' needed for Atari RLE device", tag());
	m_ram.set(*share, 2);

	// register a VBLANK callback
	m_screen->register_vblank_callback(vblank_state_delegate(FUNC(atari_rle_objects_device::vblank_callback), this));

	// build and allocate the generic tables
	build_rle_tables();

	// determine the masks first
	m_codemask.set(m_code_entry);
	m_colormask.set(m_color_entry);
	m_xposmask.set(m_xpos_entry);
	m_yposmask.set(m_ypos_entry);
	m_scalemask.set(m_scale_entry);
	m_hflipmask.set(m_hflip_entry);
	m_ordermask.set(m_order_entry);
	m_prioritymask.set(m_priority_entry);
	m_vrammask.set(m_vram_entry);

	// copy in the basic data
	m_bitmapwidth   = round_to_powerof2(m_xposmask.mask());
	m_bitmapheight  = round_to_powerof2(m_yposmask.mask());
	m_bitmapxmask   = m_bitmapwidth - 1;
	m_bitmapymask   = m_bitmapheight - 1;

	// set up the graphics ROM
	m_objectcount   = count_objects();

	// set up a cliprect
	m_cliprect      = m_screen->visible_area();
	if (m_rightclip != 0)
	{
		m_cliprect.min_x = m_leftclip;
		m_cliprect.max_x = m_rightclip;
	}

	// compute the checksums
	memset(m_checksums, 0, sizeof(m_checksums));
	for (int sumchunk = 0; sumchunk < m_rombase.bytes() / 0x20000; sumchunk++)
	{
		const UINT16 *csbase = &m_rombase[0x10000 * sumchunk];
		int cursum = 0;
		for (int word = 0; word < 0x10000; word++)
			cursum += *csbase++;
		m_checksums[sumchunk] = cursum;
	}

	// allocate the object info and scan the objects
	m_info.resize(m_objectcount);
	for (int objnum = 0; objnum < m_objectcount; objnum++)
		prescan_rle(objnum);

	// register the bitmaps with the target screen
	m_screen->register_screen_bitmap(m_vram[0][0]);
	m_screen->register_screen_bitmap(m_vram[0][1]);
	m_vram[0][0].fill(0);
	m_vram[0][1].fill(0);

	// allocate alternate bitmaps if needed
	if (m_vrammask.mask() != 0)
	{
		m_screen->register_screen_bitmap(m_vram[1][0]);
		m_screen->register_screen_bitmap(m_vram[1][1]);
		m_vram[1][0].fill(0);
		m_vram[1][1].fill(0);
	}

	// register for save states
	save_item(NAME(m_vram[0][0]));
	save_item(NAME(m_vram[0][1]));
	if (m_vrammask.mask() != 0)
	{
		save_item(NAME(m_vram[1][0]));
		save_item(NAME(m_vram[1][1]));
	}
	save_item(NAME(m_partial_scanline));
	save_item(NAME(m_control_bits));
	save_item(NAME(m_command));
}


//-------------------------------------------------
//  device_reset: Reset the state of the device.
//-------------------------------------------------

void atari_rle_objects_device::device_reset()
{
	m_partial_scanline = -1;
}


//-------------------------------------------------
//  build_rle_tables: Builds internal table for
//  RLE mapping.
//-------------------------------------------------

void atari_rle_objects_device::build_rle_tables()
{
	// assign the tables
	m_rle_table[0] = &m_rle_table_data[0x000];
	m_rle_table[1] = &m_rle_table_data[0x100];
	m_rle_table[2] = m_rle_table[3] = &m_rle_table_data[0x200];
	m_rle_table[4] = m_rle_table[6] = &m_rle_table_data[0x300];
	m_rle_table[5] = m_rle_table[7] = &m_rle_table_data[0x400];

	// set the bpps
	m_rle_bpp[0] = 4;
	m_rle_bpp[1] = m_rle_bpp[2] = m_rle_bpp[3] = 5;
	m_rle_bpp[4] = m_rle_bpp[5] = m_rle_bpp[6] = m_rle_bpp[7] = 6;

	// build the 4bpp table
	for (int i = 0; i < 256; i++)
		m_rle_table[0][i] = (((i & 0xf0) + 0x10) << 4) | (i & 0x0f);

	// build the 5bpp table
	for (int i = 0; i < 256; i++)
		m_rle_table[2][i] = (((i & 0xe0) + 0x20) << 3) | (i & 0x1f);

	// build the special 5bpp table
	for (int i = 0; i < 256; i++)
	{
		if ((i & 0x0f) == 0)
			m_rle_table[1][i] = (((i & 0xf0) + 0x10) << 4) | (i & 0x0f);
		else
			m_rle_table[1][i] = (((i & 0xe0) + 0x20) << 3) | (i & 0x1f);
	}

	// build the 6bpp table
	for (int i = 0; i < 256; i++)
		m_rle_table[5][i] = (((i & 0xc0) + 0x40) << 2) | (i & 0x3f);

	// build the special 6bpp table
	for (int i = 0; i < 256; i++)
	{
		if ((i & 0x0f) == 0)
			m_rle_table[4][i] = (((i & 0xf0) + 0x10) << 4) | (i & 0x0f);
		else
			m_rle_table[4][i] = (((i & 0xc0) + 0x40) << 2) | (i & 0x3f);
	}
}



//-------------------------------------------------
//  count_objects: Determines the number of objects in the
//  motion object ROM.
//-------------------------------------------------

int atari_rle_objects_device::count_objects()
{
	// first determine the lowest address of all objects
	int lowest_address = m_rombase.length();
	for (int objoffset = 0; objoffset < lowest_address; objoffset += 4)
	{
		int offset = ((m_rombase[objoffset + 2] & 0xff) << 16) | m_rombase[objoffset + 3];
//logerror("count_objects: objoffset=%d offset=%08X\n", objoffset, offset);
		if (offset > objoffset && offset < lowest_address)
			lowest_address = offset;
	}

	// that determines how many objects
	return lowest_address / 4;
}


//-------------------------------------------------
//  prescan_rle: Prescans an RLE object, computing the
//  width, height, and other goodies.
//-------------------------------------------------

void atari_rle_objects_device::prescan_rle(int which)
{
	object_info &info = m_info[which];

	// look up the offset
	UINT16 *base = (UINT16 *)&m_rombase[which * 4];
	const UINT16 *end = &m_rombase[0] + m_rombase.length();
	info.xoffs = (INT16)base[0];
	info.yoffs = (INT16)base[1];

	// determine the depth and table
	int flags = base[2];
	info.bpp = m_rle_bpp[(flags >> 8) & 7];
	const UINT16 *table = info.table = m_rle_table[(flags >> 8) & 7];

	// determine the starting offset
	int offset = ((base[2] & 0xff) << 16) | base[3];
	info.data = base = (UINT16 *)&m_rombase[offset];

	// make sure it's valid
	if (offset < which * 4 || offset >= m_rombase.length())
	{
		info.data = nullptr;
		return;
	}

	// first pre-scan to determine the width and height
	int width = 0;
	int height;
	for (height = 0; height < 1024 && base < end; height++)
	{
		int tempwidth = 0;
		int entry_count = *base++;

		// if the high bit is set, assume we're inverted
		if (entry_count & 0x8000)
		{
			entry_count ^= 0xffff;

			// also change the ROM data so we don't have to do this again at runtime
			base[-1] ^= 0xffff;
		}

		// we're done when we hit 0
		if (entry_count == 0)
			break;

		// track the width
		while (entry_count-- && base < end)
		{
			int word = *base++;
			int count/*, value*/;

			// decode the low byte first
			count = table[word & 0xff];
			//value = count & 0xff;
			tempwidth += count >> 8;

			// decode the upper byte second
			count = table[word >> 8];
			//value = count & 0xff;
			tempwidth += count >> 8;
		}

		// only remember the max
		if (tempwidth > width)
			width = tempwidth;
	}

	// fill in the data
	info.width = width;
	info.height = height;
}



//-------------------------------------------------
//  compute_checksum: Compute the checksum values
//  on the ROMs.
//-------------------------------------------------

void atari_rle_objects_device::compute_checksum()
{
	// number of checksums is in the first word
	int reqsums = m_ram.read(0) + 1;
	if (reqsums > 256)
		reqsums = 256;

	// stuff them back
	for (int i = 0; i < reqsums; i++)
		m_ram.write(i, m_checksums[i]);
}



//-------------------------------------------------
//  sort_and_render: Render all motion objects in
//  order.
//-------------------------------------------------

void atari_rle_objects_device::sort_and_render()
{
	// struct for sorting
	struct sort_entry_t
	{
		sort_entry_t *  next;
		int             entry;
	};

	// sort the motion objects into their proper priorities
	sort_entry_t *list_head[256] = { nullptr };
	sort_entry_t sort_entry[256];
	for (int objnum = 0; objnum < 256; objnum++)
	{
		int order = m_ordermask.extract(m_ram, objnum * 8);
		sort_entry[objnum].entry = objnum * 8;
		sort_entry[objnum].next = list_head[order];
		list_head[order] = &sort_entry[objnum];
	}

	// now loop back and process
	int bitmap_index = (~m_control_bits & ATARIRLE_CONTROL_FRAME) >> 2;
	int count = 0;
int hilite = -1;
	for (int order = 1; order < 256; order++)
		for (sort_entry_t *current = list_head[order]; current != nullptr; current = current->next)
		{
			// extract scale and code
			int scale = m_scalemask.extract(m_ram, current->entry);
			int code = m_codemask.extract(m_ram, current->entry);

			// make sure they are in range
			if (scale > 0 && code < m_objectcount)
			{
				int hflip = m_hflipmask.extract(m_ram, current->entry);
				int color = m_colormask.extract(m_ram, current->entry);
				int priority = m_prioritymask.extract(m_ram, current->entry);
				int x = m_xposmask.extract(m_ram, current->entry);
				int y = m_yposmask.extract(m_ram, current->entry);
				int which = m_vrammask.extract(m_ram, current->entry);

if (count++ == atarirle_hilite_index)
	hilite = current->entry;

				if (x & ((m_xposmask.mask() + 1) >> 1))
					x = (INT16)(x | ~m_xposmask.mask());
				if (y & ((m_yposmask.mask() + 1) >> 1))
					y = (INT16)(y | ~m_yposmask.mask());
				x += m_cliprect.min_x;

				// merge priority and color
				color = (color << 4) | (priority << ATARIRLE_PRIORITY_SHIFT);

				// render to one or both bitmaps
				bitmap_ind16 &bitmap = m_vram[which][bitmap_index];
				draw_rle(bitmap, m_cliprect, code, color, hflip, 0, x, y, scale, scale);
			}
		}

	if (hilite != -1)
		hilite_object(m_vram[0][bitmap_index], hilite);
}


//-------------------------------------------------
//  draw_rle: Render a single RLE-compressed motion
//  object.
//-------------------------------------------------

void atari_rle_objects_device::draw_rle(bitmap_ind16 &bitmap, const rectangle &clip, int code, int color, int hflip, int vflip, int x, int y, int xscale, int yscale)
{
	// bail on a NULL object
	const object_info &info = m_info[code];
	if (info.data == nullptr)
		return;

	//
	int scaled_xoffs = (xscale * info.xoffs) >> 12;
	int scaled_yoffs = (yscale * info.yoffs) >> 12;

	// we're hflipped, account for it
	if (hflip)
		scaled_xoffs = ((xscale * info.width) >> 12) - scaled_xoffs;

//if (clip.min_y == m_screen->visible_area().min_y)
//logerror("   Sprite: c=%04X l=%04X h=%d X=%4d (o=%4d w=%3d) Y=%4d (o=%4d h=%d) s=%04X\n",
//  code, color, hflip,
//  x, -scaled_xoffs, (xscale * info.width) >> 12,
//  y, -scaled_yoffs, (yscale * info.height) >> 12, xscale);

	// adjust for the x and y offsets
	x -= scaled_xoffs;
	y -= scaled_yoffs;

	// draw it with appropriate flipping
	UINT32 palettebase = m_palettebase + color;
	if (!hflip)
		draw_rle_zoom(bitmap, clip, info, palettebase, x, y, xscale << 4, yscale << 4);
	else
		draw_rle_zoom_hflip(bitmap, clip, info, palettebase, x, y, xscale << 4, yscale << 4);
}



//-------------------------------------------------
//  draw_rle_zoom: Draw an RLE-compressed object to
//  a 16-bit bitmap.
//-------------------------------------------------

void atari_rle_objects_device::draw_rle_zoom(bitmap_ind16 &bitmap, const rectangle &clip, const object_info &info, UINT32 palette, int sx, int sy, int scalex, int scaley)
{
	// determine scaled size; make sure we didn't end up with 0
	int scaled_width = (scalex * info.width + 0x7fff) >> 16;
	int scaled_height = (scaley * info.height + 0x7fff) >> 16;
	if (scaled_width == 0) scaled_width = 1;
	if (scaled_height == 0) scaled_height = 1;

	// compute the remaining parameters
	int dx = (info.width << 16) / scaled_width;
	int dy = (info.height << 16) / scaled_height;
	int ex = sx + scaled_width - 1;
	int ey = sy + scaled_height - 1;
	int sourcey = dy / 2;

	// left edge clip
	int pixels_to_skip = 0;
	bool xclipped = false;
	if (sx < clip.min_x)
		pixels_to_skip = clip.min_x - sx, xclipped = true;
	if (sx > clip.max_x)
		return;

	// right edge clip
	if (ex > clip.max_x)
		ex = clip.max_x, xclipped = true;
	else if (ex < clip.min_x)
		return;

	// top edge clip
	if (sy < clip.min_y)
	{
		sourcey += (clip.min_y - sy) * dy;
		sy = clip.min_y;
	}
	else if (sy > clip.max_y)
		return;

	// bottom edge clip
	if (ey > clip.max_y)
		ey = clip.max_y;
	else if (ey < clip.min_y)
		return;

	// loop top to bottom
	const UINT16 *row_start = info.data;
	const UINT16 *table = info.table;
	int current_row = 0;
	for (int y = sy; y <= ey; y++, sourcey += dy)
	{
		UINT16 *dest = &bitmap.pix16(y, sx);
		int  sourcex = dx / 2, rle_end = 0;

		// loop until we hit the row we're on
		for ( ; current_row != (sourcey >> 16); current_row++)
			row_start += 1 + *row_start;

		// grab our starting parameters from this row
		const UINT16 *base = row_start;
		int entry_count = *base++;

		// non-clipped case
		if (!xclipped)
		{
			// decode the pixels
			for (int entry = 0; entry < entry_count; entry++)
			{
				// decode the low byte first
				int word = *base++;
				int count = table[word & 0xff];
				int value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				// store copies of the value until we pass the end of this chunk
				if (value)
				{
					value += palette;
					while (sourcex < rle_end)
						*dest++ = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest++, sourcex += dx;
				}

				// decode the upper byte second
				count = table[word >> 8];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				// store copies of the value until we pass the end of this chunk
				if (value)
				{
					value += palette;
					while (sourcex < rle_end)
						*dest++ = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest++, sourcex += dx;
				}
			}
		}

		// clipped case
		else
		{
			const UINT16 *end = &bitmap.pix16(y, ex);
			int to_be_skipped = pixels_to_skip;

			// decode the pixels
			for (int entry = 0; entry < entry_count && dest <= end; entry++)
			{
				// decode the low byte first
				int word = *base++;
				int count = table[word & 0xff];
				int value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				// store copies of the value until we pass the end of this chunk
				if (to_be_skipped)
				{
					while (to_be_skipped && sourcex < rle_end)
						dest++, sourcex += dx, to_be_skipped--;
					if (to_be_skipped) goto next3;
				}
				if (value)
				{
					value += palette;
					while (sourcex < rle_end && dest <= end)
						*dest++ = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest++, sourcex += dx;
				}

			next3:
				// decode the upper byte second
				count = table[word >> 8];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				// store copies of the value until we pass the end of this chunk
				if (to_be_skipped)
				{
					while (to_be_skipped && sourcex < rle_end)
						dest++, sourcex += dx, to_be_skipped--;
					if (to_be_skipped) goto next4;
				}
				if (value)
				{
					value += palette;
					while (sourcex < rle_end && dest <= end)
						*dest++ = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest++, sourcex += dx;
				}
			next4:
				;
			}
		}
	}
}



//-------------------------------------------------
//  draw_rle_zoom_hflip: Draw an RLE-compressed
//  object to a 16-bit bitmap with horizontal
//  flip.
//-------------------------------------------------

void atari_rle_objects_device::draw_rle_zoom_hflip(bitmap_ind16 &bitmap, const rectangle &clip, const object_info &info, UINT32 palette, int sx, int sy, int scalex, int scaley)
{
	// determine scaled size; make sure we didn't end up with 0
	int scaled_width = (scalex * info.width + 0x7fff) >> 16;
	int scaled_height = (scaley * info.height + 0x7fff) >> 16;
	if (scaled_width == 0) scaled_width = 1;
	if (scaled_height == 0) scaled_height = 1;

	// compute the remaining parameters
	int dx = (info.width << 16) / scaled_width;
	int dy = (info.height << 16) / scaled_height;
	int ex = sx + scaled_width - 1;
	int ey = sy + scaled_height - 1;
	int sourcey = dy / 2;

	// left edge clip
	int pixels_to_skip = 0;
	bool xclipped = false;
	if (sx < clip.min_x)
		sx = clip.min_x, xclipped = true;
	if (sx > clip.max_x)
		return;

	// right edge clip
	if (ex > clip.max_x)
		pixels_to_skip = ex - clip.max_x, xclipped = true;
	else if (ex < clip.min_x)
		return;

	// top edge clip
	if (sy < clip.min_y)
	{
		sourcey += (clip.min_y - sy) * dy;
		sy = clip.min_y;
	}
	else if (sy > clip.max_y)
		return;

	// bottom edge clip
	if (ey > clip.max_y)
		ey = clip.max_y;
	else if (ey < clip.min_y)
		return;

	// loop top to bottom
	const UINT16 *row_start = info.data;
	const UINT16 *table = info.table;
	int current_row = 0;
	for (int y = sy; y <= ey; y++, sourcey += dy)
	{
		UINT16 *dest = &bitmap.pix16(y, ex);
		int sourcex = dx / 2, rle_end = 0;

		// loop until we hit the row we're on
		for ( ; current_row != (sourcey >> 16); current_row++)
			row_start += 1 + *row_start;

		// grab our starting parameters from this row
		const UINT16 *base = row_start;
		int entry_count = *base++;

		// non-clipped case
		if (!xclipped)
		{
			// decode the pixels
			for (int entry = 0; entry < entry_count; entry++)
			{
				// decode the low byte first
				int word = *base++;
				int count = table[word & 0xff];
				int value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				// store copies of the value until we pass the end of this chunk
				if (value)
				{
					value += palette;
					while (sourcex < rle_end)
						*dest-- = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest--, sourcex += dx;
				}

				// decode the upper byte second
				count = table[word >> 8];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				// store copies of the value until we pass the end of this chunk
				if (value)
				{
					value += palette;
					while (sourcex < rle_end)
						*dest-- = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest--, sourcex += dx;
				}
			}
		}

		// clipped case
		else
		{
			const UINT16 *start = &bitmap.pix16(y, sx);
			int to_be_skipped = pixels_to_skip;

			// decode the pixels
			for (int entry = 0; entry < entry_count && dest >= start; entry++)
			{
				// decode the low byte first
				int word = *base++;
				int count = table[word & 0xff];
				int value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				// store copies of the value until we pass the end of this chunk
				if (to_be_skipped)
				{
					while (to_be_skipped && sourcex < rle_end)
						dest--, sourcex += dx, to_be_skipped--;
					if (to_be_skipped) goto next3;
				}
				if (value)
				{
					value += palette;
					while (sourcex < rle_end && dest >= start)
						*dest-- = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest--, sourcex += dx;
				}

			next3:
				// decode the upper byte second
				count = table[word >> 8];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				// store copies of the value until we pass the end of this chunk
				if (to_be_skipped)
				{
					while (to_be_skipped && sourcex < rle_end)
						dest--, sourcex += dx, to_be_skipped--;
					if (to_be_skipped) goto next4;
				}
				if (value)
				{
					value += palette;
					while (sourcex < rle_end && dest >= start)
						*dest-- = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest--, sourcex += dx;
				}
			next4:
				;
			}
		}
	}
}


//-------------------------------------------------
//  hilite_object: Hilight an object by drawing a
//  flashing box around it
//-------------------------------------------------

void atari_rle_objects_device::hilite_object(bitmap_ind16 &bitmap, int hilite)
{
	// extract scale and code
	int scale = m_scalemask.extract(m_ram, hilite);
	int code = m_codemask.extract(m_ram, hilite);

	// make sure they are in range
	if (scale > 0 && code < m_objectcount)
	{
		int hflip = m_hflipmask.extract(m_ram, hilite);
		int color = m_colormask.extract(m_ram, hilite);
		int priority = m_prioritymask.extract(m_ram, hilite);
		int x = m_xposmask.extract(m_ram, hilite);
		int y = m_yposmask.extract(m_ram, hilite);

		if (x & ((m_xposmask.mask() + 1) >> 1))
			x = (INT16)(x | ~m_xposmask.mask());
		if (y & ((m_yposmask.mask() + 1) >> 1))
			y = (INT16)(y | ~m_yposmask.mask());
		x += m_cliprect.min_x;

		// merge priority and color
		color = (color << 4) | (priority << ATARIRLE_PRIORITY_SHIFT);

		const object_info &info = m_info[code];
		int scaled_xoffs = (scale * info.xoffs) >> 12;
		int scaled_yoffs = (scale * info.yoffs) >> 12;

		// we're hflipped, account for it
		if (hflip)
			scaled_xoffs = ((scale * info.width) >> 12) - scaled_xoffs;

		// adjust for the x and y offsets
		x -= scaled_xoffs;
		y -= scaled_yoffs;

		do
		{
			// make sure we didn't end up with 0
			int scaled_width = (scale * info.width + 0x7fff) >> 12;
			int scaled_height = (scale * info.height + 0x7fff) >> 12;
			if (scaled_width == 0) scaled_width = 1;
			if (scaled_height == 0) scaled_height = 1;

			// compute the remaining parameters
			int sx = x;
			int sy = y;
			int ex = sx + scaled_width - 1;
			int ey = sy + scaled_height - 1;

			// left edge clip
			const rectangle &visarea = m_screen->visible_area();
			if (sx < visarea.min_x)
				sx = visarea.min_x;
			if (sx > visarea.max_x)
				break;

			// right edge clip
			if (ex > visarea.max_x)
				ex = visarea.max_x;
			else if (ex < visarea.min_x)
				break;

			// top edge clip
			if (sy < visarea.min_y)
				sy = visarea.min_y;
			else if (sy > visarea.max_y)
				break;

			// bottom edge clip
			if (ey > visarea.max_y)
				ey = visarea.max_y;
			else if (ey < visarea.min_y)
				break;

			for (int ty = sy; ty <= ey; ty++)
			{
				bitmap.pix16(ty, sx) = machine().rand() & 0xff;
				bitmap.pix16(ty, ex) = machine().rand() & 0xff;
			}
			for (int tx = sx; tx <= ex; tx++)
			{
				bitmap.pix16(sy, tx) = machine().rand() & 0xff;
				bitmap.pix16(ey, tx) = machine().rand() & 0xff;
			}
		} while (0);
		fprintf(stderr, "   Sprite: c=%04X l=%04X h=%d X=%4d (o=%4d w=%3d) Y=%4d (o=%4d h=%d) s=%04X\n",
			code, color, hflip,
			x, -scaled_xoffs, (scale * info.width) >> 12,
			y, -scaled_yoffs, (scale * info.height) >> 12, scale);
	}
}



//**************************************************************************
//  SPRITE PARAMETER
//**************************************************************************

//-------------------------------------------------
//  sprite_parameter: Constructor
//-------------------------------------------------

atari_rle_objects_device::sprite_parameter::sprite_parameter()
	: m_word(0),
		m_shift(0),
		m_mask(0)
{
}


//-------------------------------------------------
//  set: Sets the mask via an input 4-word mask.
//-------------------------------------------------

bool atari_rle_objects_device::sprite_parameter::set(const UINT16 input[8])
{
	// determine the word and make sure it's only 1
	m_word = 0xffff;
	for (int i = 0; i < 8; i++)
		if (input[i])
		{
			if (m_word == 0xffff)
				m_word = i;
			else
				return false;
		}

	// if all-zero, it's valid
	if (m_word == 0xffff)
	{
		m_word = m_shift = m_mask = 0;
		return true;
	}

	// determine the shift and final mask
	m_shift = 0;
	UINT16 temp = input[m_word];
	while (!(temp & 1))
	{
		m_shift++;
		temp >>= 1;
	}
	m_mask = temp;
	return true;
}




/***************************************************************************

    The mapping of the bits from the PROMs is like this:

        D23 ->
        D22 ->
        D21 ->
        D20 ->
        D19 ->
        D18 ->
        D17 -> instruction bit 8
        D16 -> instruction bit 7

        D15 -> instruction bit 6
        D14 -> instruction bit 5
        D13 -> instruction bit 4
        D12 -> instruction bit 3
        D11 -> instruction bit 2
        D10 -> instruction bit 1 (modified via a PAL)
        D9  -> instruction bit 0
        D8  -> carry in

        D7  -> A bit 3
        D6  -> A bit 2
        D5  -> A bit 1
        D4  -> A bit 0
        D3  -> B bit 3
        D2  -> B bit 2
        D1  -> B bit 1
        D0  -> B bit 0

    Although much of the logic is contained in the ALU, the program counter
    is fed externally. Jumps are decoded like this:

        if (D13 && D14)
        {
            switch (D11 | D10 | D9)
            {
                case 0: condition = true;
                case 1: condition = ALU.LT;
                case 2: condition = ALU.Z;
                case 3: condition = ALU.N;
                case 4: condition = BLT.HFLIP;
                case 5: condition = /BLT.SCAN;
                case 6: condition = ALU.C;
                case 7: condition = MOTIMEP;
            }
            condition ^= D12;
            if (condition)
                PC = D8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0;
        }

    Here is the code from Guardians of the Hood:

                      I  C A B
                     --- - - -
    000: A0 C8 00 -> 064 0 0 0 (latch UC.RASCAS)
    001: 01 C8 66 -> 0E4 0 6 6
    002: 01 89 66 -> 0C4 1 6 6
    003: 03 82 66 -> 1C1 0 6 6
    004: 01 B8 65 -> 0DC 0 6 5
    005: 01 82 66 -> 0C1 0 6 6
    006: 01 89 55 -> 0C4 1 5 5
    007: 01 C8 77 -> 0E4 0 7 7
    008: 01 89 77 -> 0C4 1 7 7
    009: 03 82 77 -> 1C1 0 7 7
    00A: 03 82 77 -> 1C1 0 7 7
    00B: 03 82 77 -> 1C1 0 7 7
    00C: 03 82 77 -> 1C1 0 7 7
    00D: 01 C8 88 -> 0E4 0 8 8
    00E: 01 B8 79 -> 0DC 0 7 9
    00F: 03 82 99 -> 1C1 0 9 9
    010: 01 82 99 -> 0C1 0 9 9
    011: 01 B8 61 -> 0DC 0 6 1
    012: 01 82 11 -> 0C1 0 1 1
    013: 01 96 11 -> 0CB 0 1 1
    014: 03 82 11 -> 1C1 0 1 1
    015: 03 82 11 -> 1C1 0 1 1
    016: 03 82 11 -> 1C1 0 1 1
    017: 03 82 11 -> 1C1 0 1 1
    018: 00 EE 18 -> 077 0 1 8 if /MOTIMEP JUMP 018
    019: 84 B8 11 -> 05C 0 1 1
    01A: 01 B8 72 -> 0DC 0 7 2
    01B: 00 C8 00 -> 064 0 0 0
    01C: 9C B8 88 -> 05C 0 8 8
    01D: 98 B8 88 -> 05C 0 8 8
    01E: 01 96 22 -> 0CB 0 2 2
    01F: 00 F4 1C -> 07A 0 1 C if NZ JUMP 01C
    020: 00 38 11 -> 01C 0 1 1
    021: 03 34 00 -> 19A 0 0 0
    022: 03 34 00 -> 19A 0 0 0
    023: 03 34 00 -> 19A 0 0 0
    024: 00 11 99 -> 008 1 9 9
    025: 02 34 00 -> 11A 0 0 0
    026: 84 B4 00 -> 05A 0 0 0
    027: 00 C8 00 -> 064 0 0 0
    028: 00 C8 00 -> 064 0 0 0
    029: 10 3E 00 -> 01F 0 0 0
    02A: 00 E5 3B -> 072 1 3 B if Z JUMP 13B
    02B: 00 05 00 -> 002 1 0 0
    02C: 9C B4 00 -> 05A 0 0 0
    02D: 98 B4 00 -> 05A 0 0 0
    02E: 00 EE 2E -> 077 0 2 E if /MOTIMEP JUMP 02E
    02F: 10 3E 00 -> 01F 0 0 0
    030: 00 05 00 -> 002 1 0 0
    031: 00 F4 35 -> 07A 0 3 5 if NZ JUMP 035
    032: A8 B8 55 -> 05C 0 5 5 (latch UC.HFLIP)
    033: A8 FE 35 -> 07F 0 3 5 if MOTIMEP JUMP 035 (latch UC.HFLIP)
    034: 00 E1 47 -> 070 1 4 7 JUMP 147
    035: 01 B8 72 -> 0DC 0 7 2
    036: 01 B8 8F -> 0DC 0 8 F
    037: 01 83 5F -> 0C1 1 5 F
    038: 84 B8 FF -> 05C 0 F F
    039: 00 EE 39 -> 077 0 3 9 if /MOTIMEP JUMP 039
    03A: 19 BE FF -> 0DF 0 F F
    03B: 00 E4 68 -> 072 0 6 8 if Z JUMP 068
    03C: 84 B8 88 -> 05C 0 8 8
    03D: 00 EE 3D -> 077 0 3 D if /MOTIMEP JUMP 03D
    03E: 19 BE AA -> 0DF 0 A A
    03F: 00 EE 3F -> 077 0 3 F if /MOTIMEP JUMP 03F
    040: 19 BE BB -> 0DF 0 B B
    041: 00 EE 41 -> 077 0 4 1 if /MOTIMEP JUMP 041
    042: 19 BE CC -> 0DF 0 C C
    043: 00 EE 43 -> 077 0 4 3 if /MOTIMEP JUMP 043
    044: 19 BE DD -> 0DF 0 D D
    045: 00 EE 45 -> 077 0 4 5 if /MOTIMEP JUMP 045
    046: 19 BE EE -> 0DF 0 E E
    047: 00 C8 00 -> 064 0 0 0
    048: 01 82 1F -> 0C1 0 1 F
    049: 84 B8 FF -> 05C 0 F F
    04A: 11 BE FF -> 0DF 0 F F
    04B: 00 C8 00 -> 064 0 0 0
    04C: 00 C8 00 -> 064 0 0 0
    04D: 9C B8 99 -> 05C 0 9 9
    04E: 90 B8 99 -> 05C 0 9 9
    04F: 00 C8 00 -> 064 0 0 0
    050: 84 B8 99 -> 05C 0 9 9
    051: 9C B8 AA -> 05C 0 A A
    052: 98 B8 AA -> 05C 0 A A
    053: 00 C8 00 -> 064 0 0 0
    054: 00 C8 00 -> 064 0 0 0
    055: 9C B8 BB -> 05C 0 B B
    056: 98 B8 BB -> 05C 0 B B
    057: 00 C8 00 -> 064 0 0 0
    058: 00 C8 00 -> 064 0 0 0
    059: 9C B8 CC -> 05C 0 C C
    05A: 98 B8 CC -> 05C 0 C C
    05B: 00 C8 00 -> 064 0 0 0
    05C: 00 C8 00 -> 064 0 0 0
    05D: 9C B8 DD -> 05C 0 D D
    05E: 98 B8 DD -> 05C 0 D D
    05F: 00 C8 00 -> 064 0 0 0
    060: 00 C8 00 -> 064 0 0 0
    061: 9C B8 EE -> 05C 0 E E
    062: 98 B8 EE -> 05C 0 E E
    063: 00 C8 00 -> 064 0 0 0
    064: 00 C8 00 -> 064 0 0 0
    065: 9C B8 FF -> 05C 0 F F
    066: 98 B8 FF -> 05C 0 F F
    067: 01 82 69 -> 0C1 0 6 9
    068: 01 82 68 -> 0C1 0 6 8
    069: 01 96 22 -> 0CB 0 2 2
    06A: 00 F4 36 -> 07A 0 3 6 if NZ JUMP 036
    06B: 01 B8 72 -> 0DC 0 7 2
    06C: 01 96 22 -> 0CB 0 2 2
    06D: 01 C8 44 -> 0E4 0 4 4
    06E: 01 96 44 -> 0CB 0 4 4
    06F: 01 B8 43 -> 0DC 0 4 3
    070: 02 B8 44 -> 15C 0 4 4
    071: 02 B8 44 -> 15C 0 4 4
    072: 02 B8 44 -> 15C 0 4 4
    073: 02 B8 44 -> 15C 0 4 4
    074: 02 B8 33 -> 15C 0 3 3
    075: 01 93 43 -> 0C9 1 4 3
    076: 02 B8 33 -> 15C 0 3 3
    077: 00 38 33 -> 01C 0 3 3
    078: 01 C8 33 -> 0E4 0 3 3
    079: 01 89 33 -> 0C4 1 3 3
    07A: 01 82 33 -> 0C1 0 3 3
    07B: 01 C8 00 -> 0E4 0 0 0
    07C: 00 B8 00 -> 05C 0 0 0
    07D: 00 F4 87 -> 07A 0 8 7 if NZ JUMP 087
    07E: 01 89 11 -> 0C4 1 1 1
    07F: 84 B8 11 -> 05C 0 1 1
    080: 00 EE 80 -> 077 0 8 0 if /MOTIMEP JUMP 080
    081: 11 BE 00 -> 0DF 0 0 0
    082: A4 B8 22 -> 05C 0 2 2 (latch UC.COLOR)
    083: 01 96 22 -> 0CB 0 2 2
    084: 00 F6 7C -> 07B 0 7 C if P JUMP 07C
    085: 8C B8 22 -> 05C 0 2 2
    086: 00 E0 86 -> 070 0 8 6 JUMP 086
    087: 84 B8 00 -> 05C 0 0 0
    088: 00 EE 88 -> 077 0 8 8 if /MOTIMEP JUMP 088
    089: 19 BE 55 -> 0DF 0 5 5
    08A: A8 B8 55 -> 05C 0 5 5 (latch UC.HFLIP)
    08B: 03 82 55 -> 1C1 0 5 5
    08C: 00 C8 00 -> 064 0 0 0
    08D: 19 BE FF -> 0DF 0 F F
    08E: A4 B8 FF -> 05C 0 F F (latch UC.COLOR)
    08F: 00 C8 00 -> 064 0 0 0
    090: 00 C8 00 -> 064 0 0 0
    091: 19 BE 66 -> 0DF 0 6 6
    092: B0 C8 00 -> 064 0 0 0 (latch UC.FORMAT)
    093: 00 C8 00 -> 064 0 0 0
    094: 00 C8 00 -> 064 0 0 0
    095: 19 BE 77 -> 0DF 0 7 7
    096: 00 C8 00 -> 064 0 0 0
    097: 00 C8 00 -> 064 0 0 0
    098: 00 C8 00 -> 064 0 0 0
    099: 19 BE 88 -> 0DF 0 8 8
    09A: 00 C8 00 -> 064 0 0 0
    09B: 00 C8 00 -> 064 0 0 0
    09C: 00 C8 00 -> 064 0 0 0
    09D: 19 BE 00 -> 0DF 0 0 0
    09E: 84 B8 55 -> 05C 0 5 5
    09F: 00 C8 00 -> 064 0 0 0
    0A0: 00 C8 00 -> 064 0 0 0
    0A1: 09 BE DD -> 0DF 0 D D
    0A2: 00 C8 00 -> 064 0 0 0
    0A3: 00 C8 00 -> 064 0 0 0
    0A4: 09 BE EE -> 0DF 0 E E
    0A5: 01 C8 AA -> 0E4 0 A A
    0A6: 01 C8 BB -> 0E4 0 B B
    0A7: 09 BE 99 -> 0DF 0 9 9
    0A8: 01 B8 8F -> 0DC 0 8 F
    0A9: 01 82 FF -> 0C1 0 F F
    0AA: 09 BE 55 -> 0DF 0 5 5
    0AB: 01 C8 CC -> 0E4 0 C C
    0AC: 00 B8 DD -> 05C 0 D D
    0AD: 00 F6 B1 -> 07B 0 B 1 if P JUMP 0B1
    0AE: 01 A9 DD -> 0D4 1 D D
    0AF: 01 82 4C -> 0C1 0 4 C
    0B0: 01 89 CC -> 0C4 1 C C
    0B1: 01 82 CC -> 0C1 0 C C
    0B2: 00 B8 EE -> 05C 0 E E
    0B3: 00 F6 B7 -> 07B 0 B 7 if P JUMP 0B7
    0B4: 01 A9 EE -> 0D4 1 E E
    0B5: 01 82 4C -> 0C1 0 4 C
    0B6: 01 89 CC -> 0C4 1 C C
    0B7: 03 82 CC -> 1C1 0 C C
    0B8: 01 82 FF -> 0C1 0 F F
    0B9: 43 86 DA -> 1C3 0 D A
    0BA: 43 86 EB -> 1C3 0 E B
    0BB: 01 82 FF -> 0C1 0 F F
    0BC: 43 86 DA -> 1C3 0 D A
    0BD: 43 86 EB -> 1C3 0 E B
    0BE: 01 82 FF -> 0C1 0 F F
    0BF: 43 86 DA -> 1C3 0 D A
    0C0: 43 86 EB -> 1C3 0 E B
    0C1: 01 82 FF -> 0C1 0 F F
    0C2: 43 86 DA -> 1C3 0 D A
    0C3: 43 86 EB -> 1C3 0 E B
    0C4: 01 82 FF -> 0C1 0 F F
    0C5: 43 86 DA -> 1C3 0 D A
    0C6: 43 86 EB -> 1C3 0 E B
    0C7: 01 82 FF -> 0C1 0 F F
    0C8: 43 86 DA -> 1C3 0 D A
    0C9: 43 86 EB -> 1C3 0 E B
    0CA: 01 82 FF -> 0C1 0 F F
    0CB: 43 86 DA -> 1C3 0 D A
    0CC: 43 86 EB -> 1C3 0 E B
    0CD: 01 82 FF -> 0C1 0 F F
    0CE: 41 86 DA -> 0C3 0 D A
    0CF: 41 86 EB -> 0C3 0 E B
    0D0: 02 B8 DD -> 15C 0 D D
    0D1: 02 B8 EE -> 15C 0 E E
    0D2: 01 82 FF -> 0C1 0 F F
    0D3: 41 86 DA -> 0C3 0 D A
    0D4: 41 86 EB -> 0C3 0 E B
    0D5: 02 B8 DD -> 15C 0 D D
    0D6: 02 B8 EE -> 15C 0 E E
    0D7: 01 82 FF -> 0C1 0 F F
    0D8: 41 86 DA -> 0C3 0 D A
    0D9: 41 86 EB -> 0C3 0 E B
    0DA: 02 B8 DD -> 15C 0 D D
    0DB: 02 B8 EE -> 15C 0 E E
    0DC: 01 82 FF -> 0C1 0 F F
    0DD: 41 86 DA -> 0C3 0 D A
    0DE: 41 86 EB -> 0C3 0 E B
    0DF: 00 B8 CC -> 05C 0 C C
    0E0: 00 F6 E2 -> 07B 0 E 2 if P JUMP 0E2
    0E1: 01 A9 AA -> 0D4 1 A A
    0E2: 01 82 CC -> 0C1 0 C C
    0E3: 00 F6 E5 -> 07B 0 E 5 if P JUMP 0E5
    0E4: 01 A9 BB -> 0D4 1 B B
    0E5: 01 C8 CC -> 0E4 0 C C
    0E6: 01 89 CC -> 0C4 1 C C
    0E7: 03 82 CC -> 1C1 0 C C
    0E8: 03 82 CC -> 1C1 0 C C
    0E9: 03 82 CC -> 1C1 0 C C
    0EA: 01 B8 4F -> 0DC 0 4 F
    0EB: 03 82 FF -> 1C1 0 F F
    0EC: 00 D2 48 -> 069 0 4 8
    0ED: 00 E5 19 -> 072 1 1 9 if Z JUMP 119
    0EE: 02 B8 88 -> 15C 0 8 8
    0EF: 02 B8 88 -> 15C 0 8 8
    0F0: 02 B8 88 -> 15C 0 8 8
    0F1: 02 B8 88 -> 15C 0 8 8
    0F2: 01 80 88 -> 0C0 0 8 8
    0F3: 84 B8 88 -> 05C 0 8 8
    0F4: 00 EE F4 -> 077 0 F 4 if /MOTIMEP JUMP 0F4
    0F5: 19 BE 88 -> 0DF 0 8 8
    0F6: AC B8 88 -> 05C 0 8 8 (latch UC.RATE)
    0F7: 01 82 88 -> 0C1 0 8 8
    0F8: 02 B8 88 -> 15C 0 8 8
    0F9: 00 F8 FC -> 07C 0 F C if /BLT.HFLIP JUMP 0FC
    0FA: 01 82 A6 -> 0C1 0 A 6
    0FB: 00 E0 FD -> 070 0 F D JUMP 0FD
    0FC: 01 93 A6 -> 0C9 1 A 6
    0FD: 01 93 B7 -> 0C9 1 B 7
    0FE: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    0FF: 84 B8 55 -> 05C 0 5 5
    100: 01 C8 EE -> 0E4 0 E E
    101: 00 C8 00 -> 064 0 0 0
    102: 09 BE DD -> 0DF 0 D D
    103: 00 E6 7C -> 073 0 7 C if N JUMP 07C
    104: 00 93 F7 -> 049 1 F 7
    105: 00 ED 0B -> 076 1 0 B if C JUMP 10B
    106: B4 B8 DD -> 05C 0 D D (latch UC.TRANS)
    107: B8 B8 77 -> 05C 0 7 7 (latch UC.XYPOS)
    108: A0 B8 33 -> 05C 0 3 3 (latch UC.RASCAS)
    109: B8 B8 66 -> 05C 0 6 6 (latch UC.XYPOS)
    10A: BC C8 00 -> 064 0 0 0 (latch UC.SCAN)
    10B: 01 82 C7 -> 0C1 0 C 7
    10C: 01 82 8E -> 0C1 0 8 E
    10D: 00 D2 4E -> 069 0 4 E
    10E: 00 E5 14 -> 072 1 1 4 if Z JUMP 114
    10F: 01 93 4E -> 0C9 1 4 E
    110: 01 96 EE -> 0CB 0 E E
    111: 01 83 D5 -> 0C1 1 D 5
    112: 00 FD 14 -> 07E 1 1 4 if NC JUMP 114
    113: 01 89 99 -> 0C4 1 9 9
    114: 00 FB 14 -> 07D 1 1 4 if BLT.SCAN JUMP 114
    115: A0 C8 00 -> 064 0 0 0 (latch UC.RASCAS)
    116: 84 B8 55 -> 05C 0 5 5
    117: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    118: 00 E1 02 -> 070 1 0 2 JUMP 102
    119: AC B8 88 -> 05C 0 8 8 (latch UC.RATE)
    11A: 00 F9 1D -> 07C 1 1 D if /BLT.HFLIP JUMP 11D
    11B: 01 82 A6 -> 0C1 0 A 6
    11C: 00 E1 1E -> 070 1 1 E JUMP 11E
    11D: 01 93 A6 -> 0C9 1 A 6
    11E: 01 93 B7 -> 0C9 1 B 7
    11F: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    120: 84 B8 55 -> 05C 0 5 5
    121: 01 B8 4E -> 0DC 0 4 E
    122: 01 89 EE -> 0C4 1 E E
    123: 02 B8 EE -> 15C 0 E E
    124: 09 BE DD -> 0DF 0 D D
    125: 00 E6 7C -> 073 0 7 C if N JUMP 07C
    126: 01 82 8E -> 0C1 0 8 E
    127: 00 D2 4E -> 069 0 4 E
    128: 00 E5 33 -> 072 1 3 3 if Z JUMP 133
    129: 01 93 4E -> 0C9 1 4 E
    12A: 01 96 EE -> 0CB 0 E E
    12B: 00 93 F7 -> 049 1 F 7
    12C: 00 ED 32 -> 076 1 3 2 if C JUMP 132
    12D: B4 B8 DD -> 05C 0 D D (latch UC.TRANS)
    12E: B8 B8 77 -> 05C 0 7 7 (latch UC.XYPOS)
    12F: A0 B8 33 -> 05C 0 3 3 (latch UC.RASCAS)
    130: B8 B8 66 -> 05C 0 6 6 (latch UC.XYPOS)
    131: BC C8 00 -> 064 0 0 0 (latch UC.SCAN)
    132: 01 82 C7 -> 0C1 0 C 7
    133: 01 83 D5 -> 0C1 1 D 5
    134: 00 FD 36 -> 07E 1 3 6 if NC JUMP 136
    135: 01 89 99 -> 0C4 1 9 9
    136: 00 FB 36 -> 07D 1 3 6 if BLT.SCAN JUMP 136
    137: A0 C8 00 -> 064 0 0 0 (latch UC.RASCAS)
    138: 84 B8 55 -> 05C 0 5 5
    139: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    13A: 00 E1 24 -> 070 1 2 4 JUMP 124
    13B: 08 3E 00 -> 01F 0 0 0
    13C: 00 EF 3C -> 077 1 3 C if /MOTIMEP JUMP 13C
    13D: 18 3E 00 -> 01F 0 0 0
    13E: 00 E5 75 -> 072 1 7 5 if Z JUMP 175
    13F: 00 C8 00 -> 064 0 0 0
    140: 9C B8 44 -> 05C 0 4 4
    141: 98 B8 44 -> 05C 0 4 4
    142: 00 C8 00 -> 064 0 0 0
    143: 00 C8 00 -> 064 0 0 0
    144: 9C B8 11 -> 05C 0 1 1
    145: 98 B8 11 -> 05C 0 1 1
    146: 00 E1 5A -> 070 1 5 A JUMP 15A
    147: 00 38 11 -> 01C 0 1 1
    148: 03 34 00 -> 19A 0 0 0
    149: 03 34 00 -> 19A 0 0 0
    14A: 03 34 00 -> 19A 0 0 0
    14B: 00 11 99 -> 008 1 9 9
    14C: 00 EF 4C -> 077 1 4 C if /MOTIMEP JUMP 14C
    14D: 02 34 00 -> 11A 0 0 0
    14E: 84 B4 00 -> 05A 0 0 0
    14F: 01 C8 00 -> 0E4 0 0 0
    150: 9C B8 00 -> 05C 0 0 0
    151: 98 B8 00 -> 05C 0 0 0
    152: 00 C8 00 -> 064 0 0 0
    153: 00 C8 00 -> 064 0 0 0
    154: 9C B8 11 -> 05C 0 1 1
    155: 98 B8 11 -> 05C 0 1 1
    156: 00 C8 00 -> 064 0 0 0
    157: 00 C8 00 -> 064 0 0 0
    158: 9C B8 44 -> 05C 0 4 4
    159: 98 B8 44 -> 05C 0 4 4
    15A: 09 BE DD -> 0DF 0 D D
    15B: 01 82 8E -> 0C1 0 8 E
    15C: 00 D2 4E -> 069 0 4 E
    15D: 00 E5 67 -> 072 1 6 7 if Z JUMP 167
    15E: 01 93 4E -> 0C9 1 4 E
    15F: 01 96 EE -> 0CB 0 E E
    160: 00 93 F7 -> 049 1 F 7
    161: 00 ED 67 -> 076 1 6 7 if C JUMP 167
    162: B4 B8 DD -> 05C 0 D D (latch UC.TRANS)
    163: B8 B8 77 -> 05C 0 7 7 (latch UC.XYPOS)
    164: A0 B8 33 -> 05C 0 3 3 (latch UC.RASCAS)
    165: B8 B8 66 -> 05C 0 6 6 (latch UC.XYPOS)
    166: BC C8 00 -> 064 0 0 0 (latch UC.SCAN)
    167: 01 82 C7 -> 0C1 0 C 7
    168: 01 83 D5 -> 0C1 1 D 5
    169: 00 FD 6C -> 07E 1 6 C if NC JUMP 16C
    16A: 01 89 99 -> 0C4 1 9 9
    16B: 01 C8 00 -> 0E4 0 0 0
    16C: 01 96 00 -> 0CB 0 0 0
    16D: 00 F5 6C -> 07A 1 6 C if NZ JUMP 16C
    16E: A0 C8 00 -> 064 0 0 0 (latch UC.RASCAS)
    16F: 84 B8 55 -> 05C 0 5 5
    170: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    171: 01 C8 00 -> 0E4 0 0 0
    172: 01 96 00 -> 0CB 0 0 0
    173: 00 F5 72 -> 07A 1 7 2 if NZ JUMP 172
    174: 00 E1 5A -> 070 1 5 A JUMP 15A
    175: 84 B8 88 -> 05C 0 8 8
    176: 00 EF 76 -> 077 1 7 6 if /MOTIMEP JUMP 176
    177: 19 BE AA -> 0DF 0 A A
    178: B0 B8 AA -> 05C 0 A A (latch UC.FORMAT)
    179: 01 C8 77 -> 0E4 0 7 7
    17A: 84 B8 77 -> 05C 0 7 7
    17B: 01 C8 BB -> 0E4 0 B B
    17C: 00 C8 00 -> 064 0 0 0
    17D: 00 C8 00 -> 064 0 0 0
    17E: 00 C8 00 -> 064 0 0 0
    17F: 08 3E 00 -> 01F 0 0 0
    180: 01 80 BB -> 0C0 0 B B
    181: 01 89 77 -> 0C4 1 7 7
    182: 00 F5 7F -> 07A 1 7 F if NZ JUMP 17F
    183: 84 B8 AA -> 05C 0 A A
    184: 00 C8 00 -> 064 0 0 0
    185: 00 C8 00 -> 064 0 0 0
    186: 9C B8 BB -> 05C 0 B B
    187: 98 B8 BB -> 05C 0 B B
    188: 01 96 AA -> 0CB 0 A A
    189: 00 F7 8B -> 07B 1 8 B if P JUMP 18B
    18A: 00 E1 8A -> 070 1 8 A JUMP 18A
    18B: 00 C8 00 -> 064 0 0 0
    18C: 00 E1 78 -> 070 1 7 8 JUMP 178

***************************************************************************/
