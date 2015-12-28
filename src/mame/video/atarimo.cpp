// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarimo.c

    Common motion object management functions for Atari raster games.

****************************************************************************

            gfx bks lnk spl rev swp nei pps sof max basepal maxcols trn link        code        upper       color       xpos        ypos        width       height      hflip       vflip       priority    neighbor    absolute    special
arcadecl    0   1   1   0   0   0   0   0   0   0   0x100   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0           0           0           0           0           0
atarisy1    0   8   1   1   0   0   0   0   0   x38 0x100   0x100   0   0,0,0,003f  0,ffff,0,0  0           0,ff00,0,0  0,0,3fe0,0  3fe0,0,0,0  0           000f,0,0,0  8000,0,0,0  0           0,0,8000,0  0           0           0,ffff,0,0  == ffff
atarisy2    1   1   1   0   0   0   0   0   0   0   0x000   0x040   15  0,0,0,07f8  0,07ff,0,0  0007,0,0,0  0,0,0,3000  0,0,ffc0,0  7fc0,0,0,0  0           0,3800,0,0  0,4000,0,0  0           0,0,0,c000  0,8000,0,0  0           0
badlands    1   1   0   1   0   0   0   0   0   0   0x080   0x080   0   003f,0,0,0  0fff,0,0,0  0           0,0,0,0007  0,0,0,ff80  0,ff80,0,0  0           0,000f,0,0  0           0           0,0,0,0008  0           0           0
batman      1   1   1   0   1   0   0   8   0   0   0x100   0x100   0   03ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0,0,0070,0  0           0           0
blstroid    1   1   1   0   0   0   0   0   0   0   0x000   0x100   0   0,0,0ff8,0  0,3fff,0,0  0           0,0,0,000f  0,0,0,ffc0  ff80,0,0,0  0           000f,0,0,0  0,8000,0,0  0,4000,0,0  0           0           0           0
cyberbal    1   1   1   0   0   0   1   1k  0   0   0x600   0x100   0   0,0,07f8,0  7fff,0,0,0  0           0,0,0,000f  0,0,0,ffc0  0,ff80,0,0  0           0,000f,0,0  8000,0,0,0  0           0           0,0,0,0010  0           0
eprom       0   1   1   0   1   0   0   8   0   0   0x100   0x100   0   03ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,0,0,0008  0           0,0,0070,0  0           0           0
guts        0   1   1   0   0   0   0   8   0   0   0x100   0x100   0   03ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,000f  0,8000,0,0  0           0,0,0070,0  0           0           0
gauntlet    0   1   1   1   0   0   0   8   1   0   0x100   0x100   0   0,0,0,03ff  7fff,0,0,0  0           0,000f,0,0  0,ff80,0,0  0,0,ff80,0  0,0,0038,0  0,0,0007,0  0,0,0040,0  0           0           0           0           0
klax        1   1   1   0   0   0   0   8   0   0   0x000   0x100   0   00ff,0,0,0  0,0fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,0,0,0008  0           0           0           0           0
offtwall    0   1   1   0   0   0   0   8   0   0   0x100   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0           0           0           0
rampart     0   1   1   0   0   0   0   8   0   0   0x100   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0           0           0           0
relief      1   1   1   0   0   0   0   8   0   0   0x100   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0           0           0           0
shuuz       1   1   1   0   0   0   0   8   0   0   0x000   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0           0           0           0
skullxbo    0   2   1   0   0   0   0   8   0   0   0x000   0x200   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ffc0,0  0,0,0,ff80  0,0,0,0070  0,0,0,000f  0,8000,0,0  0           0,0,0030,0  0           0           0
thunderj    1   1   1   0   1   0   0   8   0   0   0x100   0x100   0   03ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0,0,0070,0  0           0           0
toobin      1   1   1   0   0   1   0   1k  0   0   0x100   0x100   0   0,0,00ff,0  0,3fff,0,0  0           0,0,0,000f  0,0,0,ffc0  7fc0,0,0,0  0007,0,0,0  0038,0,0,0  0,4000,0,0  0,8000,0,0  0           0           8000,0,0,0  0
vindictr    0   1   1   0   0   0   0   8   0   0   0x100   0x100   0   0,0,0,03ff  7fff,0,0,0  0           0,000f,0,0  0,ff80,0,0  0,0,ff80,0  0,0,0038,0  0,0,0007,0  0,0,0040,0  0           0,0070,0,0  0           0           0
xybots      1   1   0   0   0   0   0   0   0   0   0x100   0x300   0   003f,0,0,0  3fff,0,0,0  0           0,0,0,000f  0,0,0,ff80  0,0,ff80,0  0           0,0,0007,0  8000,0,0,0  0           0,000f,0,0  0           0           0



Sorted

atarisy1    0   8   1   1   0   0   0   0   0   x38 0x100   0x100   0   0,0,0,003f  0,ffff,0,0  0           0,ff00,0,0  0,0,3fe0,0  3fe0,0,0,0  0           000f,0,0,0  8000,0,0,0  0           0,0,8000,0  0           0           0,ffff,0,0  == ffff

atarisy2    1   1   1   0   0   0   0   0   0   0   0x000   0x040   15  0,0,0,07f8  0,07ff,0,0  0007,0,0,0  0,0,0,3000  0,0,ffc0,0  7fc0,0,0,0  0           0,3800,0,0  0,4000,0,0  0           0,0,0,c000  0,8000,0,0  0           0

toobin      1   1   1   0   0   1   0   1k  0   0   0x100   0x100   0   0,0,00ff,0  0,3fff,0,0  0           0,0,0,000f  0,0,0,ffc0  7fc0,0,0,0  0007,0,0,0  0038,0,0,0  0,4000,0,0  0,8000,0,0  0           0           8000,0,0,0  0

gauntlet    0   1   1   1   0   0   0   8   1   0   0x100   0x100   0   0,0,0,03ff  7fff,0,0,0  0           0,000f,0,0  0,ff80,0,0  0,0,ff80,0  0,0,0038,0  0,0,0007,0  0,0,0040,0  0           0           0           0           0
vindictr    0   1   1   0   0   0   0   8   0   0   0x100   0x100   0   0,0,0,03ff  7fff,0,0,0  0           0,000f,0,0  0,ff80,0,0  0,0,ff80,0  0,0,0038,0  0,0,0007,0  0,0,0040,0  0           0,0070,0,0  0           0           0

xybots      1   1   0   0   0   0   0   0   0   0   0x100   0x300   0   003f,0,0,0  3fff,0,0,0  0           0,0,0,000f  0,0,0,ff80  0,0,ff80,0  0           0,0,0007,0  8000,0,0,0  0           0,000f,0,0  0           0           0

blstroid    1   1   1   0   0   0   0   0   0   0   0x000   0x100   0   0,0,0ff8,0  0,3fff,0,0  0           0,0,0,000f  0,0,0,ffc0  ff80,0,0,0  0           000f,0,0,0  0,8000,0,0  0,4000,0,0  0           0           0           0

badlands    1   1   0   1   0   0   0   0   0   0   0x080   0x080   0   003f,0,0,0  0fff,0,0,0  0           0,0,0,0007  0,0,0,ff80  0,ff80,0,0  0           0,000f,0,0  0           0           0,0,0,0008  0           0           0
cyberbal    1   1   1   0   0   0   1   1k  0   0   0x600   0x100   0   0,0,07f8,0  7fff,0,0,0  0           0,0,0,000f  0,0,0,ffc0  0,ff80,0,0  0           0,000f,0,0  8000,0,0,0  0           0           0,0,0,0010  0           0

batman      1   1   1   0   1   0   0   8   0   0   0x100   0x100   0   03ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0,0,0070,0  0           0           0
eprom       0   1   1   0   1   0   0   8   0   0   0x100   0x100   0   03ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,0,0,0008  0           0,0,0070,0  0           0           0
guts        0   1   1   0   0   0   0   8   0   0   0x100   0x100   0   03ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,000f  0,8000,0,0  0           0,0,0070,0  0           0           0
skullxbo    0   2   1   0   0   0   0   8   0   0   0x000   0x200   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ffc0,0  0,0,0,ff80  0,0,0,0070  0,0,0,000f  0,8000,0,0  0           0,0,0030,0  0           0           0
thunderj    1   1   1   0   1   0   0   8   0   0   0x100   0x100   0   03ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0,0,0070,0  0           0           0
arcadecl    0   1   1   0   0   0   0   0   0   0   0x100   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0           0           0           0           0           0
klax        1   1   1   0   0   0   0   8   0   0   0x000   0x100   0   00ff,0,0,0  0,0fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,0,0,0008  0           0           0           0           0
offtwall    0   1   1   0   0   0   0   8   0   0   0x100   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0           0           0           0
rampart     0   1   1   0   0   0   0   8   0   0   0x100   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0           0           0           0
relief      1   1   1   0   0   0   0   8   0   0   0x100   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0           0           0           0
shuuz       1   1   1   0   0   0   0   8   0   0   0x000   0x100   0   00ff,0,0,0  0,7fff,0,0  0           0,0,000f,0  0,0,ff80,0  0,0,0,ff80  0,0,0,0070  0,0,0,0007  0,8000,0,0  0           0           0           0           0

***************************************************************************/

#include "emu.h"
#include "atarimo.h"


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  compute_log: Computes the number of bits
//  necessary to hold a given value. The input must
//  be an even power of two.
//-------------------------------------------------

inline int atari_motion_objects_device::compute_log(int value)
{
	int log = 0;

	if (value == 0)
		return -1;
	while (!(value & 1))
		log++, value >>= 1;
	if (value != 1)
		return -1;
	return log;
}


//-------------------------------------------------
//  round_to_powerof2: Rounds a number up to the
//  nearest power of 2. Even powers of 2 are
//  rounded up to the next greatest power
//  (e.g., 4 returns 8).
//-------------------------------------------------

inline int atari_motion_objects_device::round_to_powerof2(int value)
{
	int log = 0;

	if (value == 0)
		return 1;
	while ((value >>= 1) != 0)
		log++;
	return 1 << (log + 1);
}



//**************************************************************************
//  CORE IMPLEMENTATION
//**************************************************************************

// device type definition
const device_type ATARI_MOTION_OBJECTS = &device_creator<atari_motion_objects_device>;

//-------------------------------------------------
//  atari_motion_objects_device - constructor
//-------------------------------------------------

atari_motion_objects_device::atari_motion_objects_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sprite16_device_ind16(mconfig, ATARI_MOTION_OBJECTS, "Atari Motion Objects", tag, owner, "atarimo", __FILE__),
		device_video_interface(mconfig, *this),
		m_tilewidth(0),
		m_tileheight(0),
		m_tilexshift(0),
		m_tileyshift(0),
		m_bitmapwidth(0),
		m_bitmapheight(0),
		m_bitmapxmask(0),
		m_bitmapymask(0),
		m_entrycount(0),
		m_entrybits(0),
		m_spriterammask(0),
		m_spriteramsize(0),
		m_slipshift(0),
		m_sliprammask(0),
		m_slipramsize(0),
		m_bank(0),
		m_xscroll(0),
		m_yscroll(0),
		m_slipram(*this, "slip"),
		m_activelast(nullptr),
		m_last_xpos(0),
		m_next_xpos(0),
		m_gfxdecode(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void atari_motion_objects_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<atari_motion_objects_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  static_set_config: Set the tag of the
//  sound CPU
//-------------------------------------------------

void atari_motion_objects_device::static_set_config(device_t &device, const atari_motion_objects_config &config)
{
	atari_motion_objects_device &target = downcast<atari_motion_objects_device &>(device);
	static_cast<atari_motion_objects_config &>(target) = config;
}


//-------------------------------------------------
//  draw: Render the motion objects to the
//  destination bitmap.
//-------------------------------------------------

void atari_motion_objects_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// compute start/stop bands
	int startband = ((cliprect.min_y + m_yscroll - m_slipoffset) & m_bitmapymask) >> m_slipshift;
	int stopband = ((cliprect.max_y + m_yscroll - m_slipoffset) & m_bitmapymask) >> m_slipshift;
	if (startband > stopband)
		startband -= m_bitmapheight >> m_slipshift;
	if (m_slipshift == 0)
		stopband = startband;

	// loop over SLIP bands
	for (int band = startband; band <= stopband; band++)
	{
		// compute the starting link and clip for the current band
		rectangle bandclip = cliprect;
		int link = 0;
		if (m_slipshift != 0)
		{
			// extract the link from the SLIP RAM
			link = (m_slipram[band & m_sliprammask] >> m_linkmask.shift()) & m_linkmask.mask();

			// compute minimum Y and wrap around if necessary
			bandclip.min_y = ((band << m_slipshift) - m_yscroll + m_slipoffset) & m_bitmapymask;
			if (bandclip.min_y >= bitmap.height())
				bandclip.min_y -= m_bitmapheight;

			// maximum Y is based on the minimum
			bandclip.max_y = bandclip.min_y + (1 << m_slipshift) - 1;

			// keep within the cliprect
			bandclip &= cliprect;
		}

		// if this matches the last link, we don't need to re-process the list
		build_active_list(link);

		// initialize the parameters
		m_next_xpos = 123456;

		// safety check
		if (m_activelist == m_activelast)
			continue;

		// set the start and end points
		UINT16 *first, *last;
		int step;
		if (m_reverse)
		{
			first = m_activelast - 4;
			last = m_activelist;
			step = -4;
		}
		else
		{
			first = m_activelist;
			last = m_activelast - 4;
			step = 4;
		}

		// render the mos
		for (UINT16 *current = first; ; current += step)
		{
			render_object(bitmap, bandclip, current);
			if (current == last)
				break;
		}
	}
}


//-------------------------------------------------
//  apply_stain: Mark high palette bits
//  starting at the given X,Y and continuing until
//  a stop or the end of line.
//-------------------------------------------------

void atari_motion_objects_device::apply_stain(bitmap_ind16 &bitmap, UINT16 *pf, UINT16 *mo, int x, int y)
{
	const UINT16 START_MARKER = ((4 << PRIORITY_SHIFT) | 2);
	const UINT16 END_MARKER =   ((4 << PRIORITY_SHIFT) | 4);
	bool offnext = false;

	for ( ; x < bitmap.width(); x++)
	{
		pf[x] |= 0x400;
		if (offnext && ((mo[x] == 0xffff) || (mo[x] & START_MARKER) != START_MARKER))
			break;
		offnext = (mo[x] != 0xffff) && ((mo[x] & END_MARKER) == END_MARKER);
	}
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_motion_objects_device::device_start()
{
	// call parent
	sprite16_device_ind16::device_start();

	// verify configuration
	gfx_element *gfx = m_gfxdecode->gfx(m_gfxindex);
	if (gfx == nullptr)
		throw emu_fatalerror("No gfxelement #%d!", m_gfxindex);

	// determine the masks
	m_linkmask.set(m_link_entry);
	m_codemask.set(m_code_entry);
	m_colormask.set(m_color_entry);
	m_xposmask.set(m_xpos_entry);
	m_yposmask.set(m_ypos_entry);
	m_widthmask.set(m_width_entry);
	m_heightmask.set(m_height_entry);
	m_hflipmask.set(m_hflip_entry);
	m_vflipmask.set(m_vflip_entry);
	m_prioritymask.set(m_priority_entry);
	m_neighbormask.set(m_neighbor_entry);
	m_absolutemask.set(m_absolute_entry);

	// derive tile information
	m_tilewidth = gfx->width();
	m_tileheight = gfx->height();
	m_tilexshift = compute_log(m_tilewidth);
	m_tileyshift = compute_log(m_tileheight);

	// derive bitmap information
	m_bitmapwidth = round_to_powerof2(m_xposmask.mask());
	m_bitmapheight = round_to_powerof2(m_yposmask.mask());
	m_bitmapxmask = m_bitmapwidth - 1;
	m_bitmapymask = m_bitmapheight - 1;

	// derive sprite information
	m_entrycount = round_to_powerof2(m_linkmask.mask());
	m_entrybits = compute_log(m_entrycount);
	m_spriteramsize = m_bankcount * m_entrycount;
	m_spriterammask = m_spriteramsize - 1;
	m_slipshift = (m_slipheight != 0) ? compute_log(m_slipheight) : 0;
	m_slipramsize = m_bitmapheight >> m_slipshift;
	m_sliprammask = m_slipramsize - 1;
	if (m_maxperline == 0)
		m_maxperline = MAX_PER_BANK;

	// allocate and initialize the code lookup
	int codesize = round_to_powerof2(m_codemask.mask());
	m_codelookup.resize(codesize);
	for (int i = 0; i < codesize; i++)
		m_codelookup[i] = i;

	// allocate and initialize the color lookup
	int colorsize = round_to_powerof2(m_colormask.mask());
	m_colorlookup.resize(colorsize);
	for (int i = 0; i < colorsize; i++)
		m_colorlookup[i] = i;

	// allocate and the gfx lookup
	int gfxsize = codesize / 256;
	m_gfxlookup.resize(gfxsize);
	for (int i = 0; i < gfxsize; i++)
		m_gfxlookup[i] = m_gfxindex;

	// allocate a timer to periodically force update
	m_force_update_timer = timer_alloc(TID_FORCE_UPDATE);
	m_force_update_timer->adjust(m_screen->time_until_pos(0));

	// register for save states
	save_item(NAME(m_bank));
	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
}


//-------------------------------------------------
//  device_reset: Handle a device reset by
//  clearing the interrupt lines and states
//-------------------------------------------------

void atari_motion_objects_device::device_reset()
{
	// call parent
	sprite16_device_ind16::device_reset();

	// reset the live state
	m_bank = 0;
}


//-------------------------------------------------
//  device_timer: Handle device-specific timer
//  calbacks
//-------------------------------------------------

void atari_motion_objects_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_FORCE_UPDATE:
			if (param > 0)
				m_screen->update_partial(param - 1);
			param += 64;
			if (param >= m_screen->visible_area().max_y)
				param = 0;
			timer.adjust(m_screen->time_until_pos(param), param);
			break;
	}
}


//-------------------------------------------------
//  build_active_list: Build a list of active
//  objects.
//-------------------------------------------------

void atari_motion_objects_device::build_active_list(int link)
{
	UINT16 *bankbase = &spriteram()[m_bank << (m_entrybits + 2)];
	UINT16 *current = &m_activelist[0];

	// visit all the motion objects and copy their data into the display list
	UINT8 visited[MAX_PER_BANK] = {0};
	for (int i = 0; i < m_maxperline && !visited[link]; i++)
	{
		// copy the current entry into the list
		UINT16 *modata = current;
		if (!m_split)
		{
			UINT16 *srcdata = &bankbase[link * 4];
			*current++ = srcdata[0];
			*current++ = srcdata[1];
			*current++ = srcdata[2];
			*current++ = srcdata[3];
		}
		else
		{
			UINT16 *srcdata = &bankbase[link];
			*current++ = srcdata[UINT32(0 << m_entrybits)];
			*current++ = srcdata[UINT32(1 << m_entrybits)];
			*current++ = srcdata[UINT32(2 << m_entrybits)];
			*current++ = srcdata[UINT32(3 << m_entrybits)];
		}

		// link to the next object
		visited[link] = 1;
		if (m_linked)
			link = m_linkmask.extract(modata);
		else
			link = (link + 1) & m_linkmask.mask();
	}

	// note the last entry
	m_activelast = current;
}


//-------------------------------------------------
//  render_object: Internal processing callback
//  that renders to the backing bitmap and then
//  copies the result  to the destination.
//-------------------------------------------------

void atari_motion_objects_device::render_object(bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT16 *entry)
{
	// select the gfx element and save off key information
	int rawcode = m_codemask.extract(entry);
	gfx_element *gfx = m_gfxdecode->gfx(m_gfxlookup[rawcode >> 8]);
	int save_granularity = gfx->granularity();
	int save_colorbase = gfx->colorbase();
	int save_colors = gfx->colors();
	gfx->set_granularity(1);
	gfx->set_colorbase(0);
	gfx->set_colors(65536);

	// extract data from the various words
	int code = m_codelookup[rawcode];
	int color = m_colorlookup[m_colormask.extract(entry)];
	int xpos = m_xposmask.extract(entry);
	int ypos = -m_yposmask.extract(entry);
	int hflip = m_hflipmask.extract(entry);
	int vflip = m_vflipmask.extract(entry);
	int width = m_widthmask.extract(entry) + 1;
	int height = m_heightmask.extract(entry) + 1;
	int priority = m_prioritymask.extract(entry);

	// compute the effective color, merging in priority
	color = (color * save_granularity) | (priority << PRIORITY_SHIFT);
	color += m_palettebase;

	// add in the scroll positions if we're not in absolute coordinates
	if (!m_absolutemask.extract(entry))
	{
		xpos -= m_xscroll;
		ypos -= m_yscroll;
	}

	// adjust for height
	ypos -= height << m_tileyshift;

	// handle previous hold bits
	if (m_next_xpos != 123456)
		xpos = m_next_xpos;
	m_next_xpos = 123456;

	// check for the hold bit
	if (m_neighbormask.extract(entry) != 0)
	{
		if (!m_nextneighbor)
			xpos = m_last_xpos + m_tilewidth;
		else
			m_next_xpos = xpos + m_tilewidth;
	}
	m_last_xpos = xpos;

	// adjust the final coordinates
	xpos &= m_bitmapxmask;
	ypos &= m_bitmapymask;
	if (xpos >= bitmap.width())
		xpos -= m_bitmapwidth;
	if (ypos >= bitmap.height())
		ypos -= m_bitmapheight;

	// is this one special?
	if (m_specialmask.mask() == 0 || m_specialmask.extract(entry) != m_specialvalue)
	{
		// adjust for h flip
		int xadv = m_tilewidth;
		if (hflip)
		{
			xpos += (width - 1) << m_tilexshift;
			xadv = -xadv;
		}

		// adjust for v flip
		int yadv = m_tileheight;
		if (vflip)
		{
			ypos += (height - 1) << m_tileyshift;
			yadv = -yadv;
		}

		// standard order is: loop over Y first, then X
		if (!m_swapxy)
		{
			// loop over the height
			for (int y = 0, sy = ypos; y < height; y++, sy += yadv)
			{
				// clip the Y coordinate
				if (sy <= cliprect.min_y - m_tileheight)
				{
					code += width;
					continue;
				}
				else if (sy > cliprect.max_y)
					break;

				// loop over the width
				for (int x = 0, sx = xpos; x < width; x++, sx += xadv, code++)
				{
					// clip the X coordinate
					if (sx <= -cliprect.min_x - m_tilewidth || sx > cliprect.max_x)
						continue;

					// draw the sprite
					gfx->transpen_raw(bitmap,cliprect, code, color, hflip, vflip, sx, sy, m_transpen);
					mark_dirty(sx, sx + m_tilewidth - 1, sy, sy + m_tileheight - 1);
				}
			}
		}

		// alternative order is swapped
		else
		{
			// loop over the width
			for (int x = 0, sx = xpos; x < width; x++, sx += xadv)
			{
				// clip the X coordinate
				if (sx <= cliprect.min_x - m_tilewidth)
				{
					code += height;
					continue;
				}
				else if (sx > cliprect.max_x)
					break;

				// loop over the height
				for (int y = 0, sy = ypos; y < height; y++, sy += yadv, code++)
				{
					// clip the X coordinate
					if (sy <= -cliprect.min_y - m_tileheight || sy > cliprect.max_y)
						continue;

					// draw the sprite
					gfx->transpen_raw(bitmap,cliprect, code, color, hflip, vflip, sx, sy, m_transpen);
					mark_dirty(sx, sx + m_tilewidth - 1, sy, sy + m_tileheight - 1);
				}
			}
		}
	}

	// restore original gfx information
	gfx->set_granularity(save_granularity);
	gfx->set_colorbase(save_colorbase);
	gfx->set_colors(save_colors);
}



//**************************************************************************
//  SPRITE PARAMETER
//**************************************************************************

//-------------------------------------------------
//  sprite_parameter: Constructor
//-------------------------------------------------

atari_motion_objects_device::sprite_parameter::sprite_parameter()
	: m_word(0),
	m_shift(0),
	m_mask(0)
{
}


//-------------------------------------------------
//  set: Sets the mask via an input 4-word mask.
//-------------------------------------------------

bool atari_motion_objects_device::sprite_parameter::set(const UINT16 input[4])
{
	// determine the word and make sure it's only 1
	m_word = 0xffff;
	for (int i = 0; i < 4; i++)
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


//-------------------------------------------------
//  dual_sprite_parameter: Constructor
//-------------------------------------------------

atari_motion_objects_device::dual_sprite_parameter::dual_sprite_parameter()
	: m_uppershift(0)
{
}


//-------------------------------------------------
//  set: Sets the mask via an input 4-word mask.
//-------------------------------------------------

bool atari_motion_objects_device::dual_sprite_parameter::set(const atari_motion_objects_config::dual_entry &input)
{
	// convert the lower and upper parts
	if (!m_lower.set(input.data_lower))
		return false;
	if (!m_upper.set(input.data_upper))
		return false;

	// determine the upper shift amount
	UINT16 temp = m_lower.mask();
	m_uppershift = 0;
	while (temp != 0)
	{
		m_uppershift++;
		temp >>= 1;
	}
	return true;
}
