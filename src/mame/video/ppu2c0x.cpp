// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Brad Oliver, Fabio Priuli
/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

    2009-04: Changed NES PPU to be a device (Nathan Woods)
    2009-07: Changed NES PPU to use a device memory map (Robert Bohms)

    Current known bugs

    General:

    * PPU timing is imprecise for updates that happen mid-scanline. Some games
      may demand more precision.

    NES-specific:

    * Micro Machines has minor rendering glitches (needs better timing).
    * Mach Rider has minor road rendering glitches (needs better timing).
    * Rad Racer demonstrates road glitches: it changes horizontal scrolling mid-line.

******************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* constant definitions */
#define VISIBLE_SCREEN_WIDTH         (32*8) /* Visible screen width */
#define VISIBLE_SCREEN_HEIGHT        (30*8) /* Visible screen height */
#define VIDEOMEM_SIZE           0x1000  /* videomem size */
#define VIDEOMEM_PAGE_SIZE      0x400   /* videomem page size */
#define SPRITERAM_SIZE          0x100   /* spriteram size */
#define SPRITERAM_MASK          (0x100-1)   /* spriteram size */
#define CHARGEN_NUM_CHARS       512     /* max number of characters handled by the chargen */

/* default monochromatic colortable */
static const pen_t default_colortable_mono[] =
{
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
};

/* default colortable */
static const pen_t default_colortable[] =
{
	0,1,2,3,
	0,5,6,7,
	0,9,10,11,
	0,13,14,15,
	0,17,18,19,
	0,21,22,23,
	0,25,26,27,
	0,29,30,31,
};

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type PPU_2C02 = &device_creator<ppu2c02_device>;
const device_type PPU_2C03B = &device_creator<ppu2c03b_device>;
const device_type PPU_2C04 = &device_creator<ppu2c04_device>;
const device_type PPU_2C07 = &device_creator<ppu2c07_device>;
const device_type PPU_2C05_01 = &device_creator<ppu2c05_01_device>;
const device_type PPU_2C05_02 = &device_creator<ppu2c05_02_device>;
const device_type PPU_2C05_03 = &device_creator<ppu2c05_03_device>;
const device_type PPU_2C05_04 = &device_creator<ppu2c05_04_device>;


// default address map
static ADDRESS_MAP_START( ppu2c0x, AS_0, 8, ppu2c0x_device )
	AM_RANGE(0x0000, 0x3eff) AM_RAM
	AM_RANGE(0x3f00, 0x3fff) AM_READWRITE(palette_read, palette_write)
//  AM_RANGE(0x0000, 0x3fff) AM_RAM
ADDRESS_MAP_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *ppu2c0x_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}


// static
void ppu2c0x_device::set_nmi_delegate(device_t &device,ppu2c0x_nmi_delegate cb)
{
	ppu2c0x_device &dev = downcast<ppu2c0x_device &>(device);
	dev.m_nmi_callback_proc = cb;
}
//-------------------------------------------------
//  ppu2c0x_device - constructor
//-------------------------------------------------

void ppu2c0x_device::device_config_complete()
{
	/* reset the callbacks */
	m_latch = ppu2c0x_latch_delegate();
	m_scanline_callback_proc = ppu2c0x_scanline_delegate();
	m_hblank_callback_proc = ppu2c0x_hblank_delegate();
	m_vidaccess_callback_proc = ppu2c0x_vidaccess_delegate();
}

ppu2c0x_device::ppu2c0x_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
				: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
					device_memory_interface(mconfig, *this),
					device_video_interface(mconfig, *this),
					m_space_config("videoram", ENDIANNESS_LITTLE, 8, 17, 0, nullptr, *ADDRESS_MAP_NAME(ppu2c0x)),
					m_cpu(*this),
					m_scanline(0),  // reset the scanline count
					m_refresh_data(0),
					m_refresh_latch(0),
					m_x_fine(0),
					m_toggle(0),
					m_add(1),
					m_videomem_addr(0),
					m_data_latch(0),
					m_buffered_data(0),
					m_tile_page(0),
					m_sprite_page(0),
					m_back_color(0),
					m_color_base(0),
					m_scan_scale(1), // set the scan scale (this is for dual monitor vertical setups)
					m_tilecount(0),
					m_draw_phase(0),
					m_use_sprite_write_limitation(true)
{
	for (auto & elem : m_regs)
		elem = 0;

	memset(m_palette_ram, 0, ARRAY_LENGTH(m_palette_ram));

	m_scanlines_per_frame = PPU_NTSC_SCANLINES_PER_FRAME;

	/* usually, no security value... */
	m_security_value = 0;

	m_nmi_callback_proc = ppu2c0x_nmi_delegate();
}


// NTSC NES
ppu2c02_device::ppu2c02_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : ppu2c0x_device(mconfig, PPU_2C02, "2C02 PPU", tag, owner, clock, "ppu2c02", __FILE__)
{
}

// Playchoice 10
ppu2c03b_device::ppu2c03b_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : ppu2c0x_device(mconfig, PPU_2C03B, "2C03B PPU", tag, owner, clock, "ppu2c03b", __FILE__)
{
}

// Vs. Unisystem
ppu2c04_device::ppu2c04_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : ppu2c0x_device(mconfig, PPU_2C04, "2C04 PPU", tag, owner, clock, "ppu2c04", __FILE__)
{
}

// PAL NES
ppu2c07_device::ppu2c07_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : ppu2c0x_device(mconfig, PPU_2C07, "2C07 PPU", tag, owner, clock, "ppu2c07", __FILE__)
{
	m_scanlines_per_frame = PPU_PAL_SCANLINES_PER_FRAME;
}

// The PPU_2C05 variants have different protection value, set at device start, but otherwise are all the same...
// Vs. Unisystem (Ninja Jajamaru Kun)
ppu2c05_01_device::ppu2c05_01_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : ppu2c0x_device(mconfig, PPU_2C05_01, "2C05_01 PPU", tag, owner, clock, "ppu2c05_01", __FILE__)
{
	m_security_value = 0x1b;    // game (jajamaru) doesn't seem to ever actually check it
}
// Vs. Unisystem (Mighty Bomb Jack)
ppu2c05_02_device::ppu2c05_02_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : ppu2c0x_device(mconfig, PPU_2C05_02, "2C05_02 PPU", tag, owner, clock, "ppu2c05_02", __FILE__)
{
	m_security_value = 0x3d;
}
// Vs. Unisystem (Gumshoe)
ppu2c05_03_device::ppu2c05_03_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : ppu2c0x_device(mconfig, PPU_2C05_03, "2C05_03 PPU", tag, owner, clock, "ppu2c05_03", __FILE__)
{
	m_security_value = 0x1c;
}
// Vs. Unisystem (Top Gun)
ppu2c05_04_device::ppu2c05_04_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : ppu2c0x_device(mconfig, PPU_2C05_04, "2C05_04 PPU", tag, owner, clock, "ppu2c05_04", __FILE__)
{
	m_security_value = 0x1b;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ppu2c0x_device::device_start()
{
	// bind our handler
	m_nmi_callback_proc.bind_relative_to(*owner());

	// allocate timers
	m_hblank_timer = timer_alloc(TIMER_HBLANK);
	m_nmi_timer = timer_alloc(TIMER_NMI);
	m_scanline_timer = timer_alloc(TIMER_SCANLINE);

	/* initialize the scanline handling portion */
	m_scanline_timer->adjust(m_screen->time_until_pos(1));
	m_hblank_timer->adjust(m_cpu->cycles_to_attotime(86.67)); // ??? FIXME - hardcoding NTSC, need better calculation
	m_nmi_timer->adjust(attotime::never);

	/* allocate a screen bitmap, videomem and spriteram, a dirtychar array and the monochromatic colortable */
	m_bitmap = std::make_unique<bitmap_ind16>(VISIBLE_SCREEN_WIDTH, VISIBLE_SCREEN_HEIGHT);
	m_spriteram = make_unique_clear<UINT8[]>(SPRITERAM_SIZE);
	m_colortable = std::make_unique<pen_t[]>(ARRAY_LENGTH(default_colortable));
	m_colortable_mono = std::make_unique<pen_t[]>(ARRAY_LENGTH(default_colortable_mono));

	/* initialize the color tables */
	for (int i = 0; i < ARRAY_LENGTH(default_colortable_mono); i++)
	{
		/* monochromatic table */
		m_colortable_mono[i] = default_colortable_mono[i] + m_color_base;

		/* color table */
		m_colortable[i] = default_colortable[i] + m_color_base;
	}

	// register for state saving
	save_item(NAME(m_scanline));
	save_item(NAME(m_refresh_data));
	save_item(NAME(m_refresh_latch));
	save_item(NAME(m_x_fine));
	save_item(NAME(m_toggle));
	save_item(NAME(m_add));
	save_item(NAME(m_videomem_addr));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_buffered_data));
	save_item(NAME(m_tile_page));
	save_item(NAME(m_sprite_page));
	save_item(NAME(m_back_color));
	save_item(NAME(m_scan_scale));
	save_item(NAME(m_scanlines_per_frame));
	save_item(NAME(m_regs));
	save_item(NAME(m_palette_ram));
	save_item(NAME(m_draw_phase));
	save_item(NAME(m_tilecount));
	save_pointer(NAME(m_spriteram.get()), SPRITERAM_SIZE);
	save_pointer(NAME(m_colortable.get()), ARRAY_LENGTH(default_colortable));
	save_pointer(NAME(m_colortable_mono.get()), ARRAY_LENGTH(default_colortable_mono));
	save_item(NAME(*m_bitmap));
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 ppu2c0x_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void ppu2c0x_device::writebyte(offs_t address, UINT8 data)
{
	space().write_byte(address, data);
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*************************************
 *
 *  PPU Palette Initialization
 *
 *************************************/

void ppu2c0x_device::init_palette( palette_device &palette, int first_entry )
{
	/* This routine builds a palette using a transformation from */
	/* the YUV (Y, B-Y, R-Y) to the RGB color space */

	/* The NES has a 64 color palette                        */
	/* 16 colors, with 4 luminance levels for each color     */
	/* The 16 colors circle around the YUV color space,      */

	int color_intensity, color_num, color_emphasis;

	double R, G, B;

	double tint = 0.22; /* adjust to taste */
	double hue = 287.0;

	double Kr = 0.2989;
	double Kb = 0.1145;
	double Ku = 2.029;
	double Kv = 1.140;

	static const double brightness[3][4] =
	{
		{ 0.50, 0.75, 1.0, 1.0 },
		{ 0.29, 0.45, 0.73, 0.9 },
		{ 0, 0.24, 0.47, 0.77 }
	};

	/* Loop through the emphasis modes (8 total) */
	for (color_emphasis = 0; color_emphasis < 8; color_emphasis++)
	{
		/*
		double r_mod = 0.0;
		double g_mod = 0.0;
		double b_mod = 0.0;

		switch (color_emphasis)
		{
		    case 0: r_mod = 1.0;  g_mod = 1.0;  b_mod = 1.0;  break;
		    case 1: r_mod = 1.24; g_mod = .915; b_mod = .743; break;
		    case 2: r_mod = .794; g_mod = 1.09; b_mod = .882; break;
		    case 3: r_mod = .905; g_mod = 1.03; b_mod = 1.28; break;
		    case 4: r_mod = .741; g_mod = .987; b_mod = 1.0;  break;
		    case 5: r_mod = 1.02; g_mod = .908; b_mod = .979; break;
		    case 6: r_mod = 1.02; g_mod = .98;  b_mod = .653; break;
		    case 7: r_mod = .75;  g_mod = .75;  b_mod = .75;  break;
		}
		*/

		/* loop through the 4 intensities */
		for (color_intensity = 0; color_intensity < 4; color_intensity++)
		{
			/* loop through the 16 colors */
			for (color_num = 0; color_num < 16; color_num++)
			{
				double sat;
				double y, u, v;
				double rad;

				switch (color_num)
				{
					case 0:
						sat = 0; rad = 0;
						y = brightness[0][color_intensity];
						break;

					case 13:
						sat = 0; rad = 0;
						y = brightness[2][color_intensity];
						break;

					case 14:
					case 15:
						sat = 0; rad = 0; y = 0;
						break;

					default:
						sat = tint;
						rad = M_PI * ((color_num * 30 + hue) / 180.0);
						y = brightness[1][color_intensity];
						break;
				}

				u = sat * cos(rad);
				v = sat * sin(rad);

				/* Transform to RGB */
				R = (y + Kv * v) * 255.0;
				G = (y - (Kb * Ku * u + Kr * Kv * v) / (1 - Kb - Kr)) * 255.0;
				B = (y + Ku * u) * 255.0;

				/* Clipping, in case of saturation */
				if (R < 0)
					R = 0;
				if (R > 255)
					R = 255;
				if (G < 0)
					G = 0;
				if (G > 255)
					G = 255;
				if (B < 0)
					B = 0;
				if (B > 255)
					B = 255;

				/* Round, and set the value */
				palette.set_pen_color(first_entry++, floor(R + .5), floor(G + .5), floor(B + .5));
			}
		}
	}

	/* color tables are modified at run-time, and are initialized on 'ppu2c0x_reset' */
}

void ppu2c0x_device::init_palette_rgb( palette_device &palette, int first_entry )
{
	int color_emphasis, color_num;

	int R, G, B;

	UINT8 *palette_data = machine().root_device().memregion("palette")->base();

	/* Loop through the emphasis modes (8 total) */
	for (color_emphasis = 0; color_emphasis < 8; color_emphasis++)
	{
		for (color_num = 0; color_num < 64; color_num++)
			{
				R = ((color_emphasis & 1) ? 7 : palette_data[color_num * 3]);
				G = ((color_emphasis & 2) ? 7 : palette_data[color_num * 3 + 1]);
				B = ((color_emphasis & 4) ? 7 : palette_data[color_num * 3 + 2]);

				palette.set_pen_color(first_entry++, pal3bit(R), pal3bit(G), pal3bit(B));
			}
	}

	/* color tables are modified at run-time, and are initialized on 'ppu2c0x_reset' */
}

#if 0
/* the charlayout we use for the chargen */
static const gfx_layout ppu_charlayout =
{
	8, 8,   /* 8*8 characters */
	0,
	2,      /* 2 bits per pixel */
	{ 8*8, 0 }, /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 consecutive bytes */
};
#endif

/*************************************
 *
 *  PPU Initialization and Disposal
 *
 *************************************/

//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void ppu2c0x_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int blanked, vblank;
	int *regs = &m_regs[0];

	switch (id)
	{
		case TIMER_HBLANK:
			blanked = (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
			vblank = ((m_scanline >= PPU_VBLANK_FIRST_SCANLINE - 1) && (m_scanline < m_scanlines_per_frame - 1)) ? 1 : 0;

			//update_scanline();

			if (!m_hblank_callback_proc.isnull())
				m_hblank_callback_proc(m_scanline, vblank, blanked);

			m_hblank_timer->adjust(attotime::never);
			break;

		case TIMER_NMI:
			// Actually fire the VMI
			if (!m_nmi_callback_proc.isnull())
				m_nmi_callback_proc(regs);

			m_nmi_timer->adjust(attotime::never);
			break;

		case TIMER_SCANLINE:
			blanked = (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
			vblank = ((m_scanline >= PPU_VBLANK_FIRST_SCANLINE - 1) && (m_scanline < m_scanlines_per_frame - 1)) ? 1 : 0;
			int next_scanline;

			/* if a callback is available, call it */
			if (!m_scanline_callback_proc.isnull())
				m_scanline_callback_proc(m_scanline, vblank, blanked);

			/* update the scanline that just went by */
			update_scanline();

			/* increment our scanline count */
			m_scanline++;

			//  logerror("starting scanline %d (MAME %d, beam %d)\n", m_scanline, device->m_screen->vpos(), device->m_screen->hpos());

			/* Note: this is called at the _end_ of each scanline */
			if (m_scanline == PPU_VBLANK_FIRST_SCANLINE)
			{
				// logerror("vblank starting\n");
				/* We just entered VBLANK */
				m_regs[PPU_STATUS] |= PPU_STATUS_VBLANK;

				/* If NMI's are set to be triggered, go for it */
				if (m_regs[PPU_CONTROL0] & PPU_CONTROL0_NMI)
				{
					// We need an ever-so-slight delay between entering vblank and firing an NMI - enough so that
					// a game can read the high bit of $2002 before the NMI is called (potentially resetting the bit
					// via a read from $2002 in the NMI handler).
					// B-Wings is an example game that needs this.
					m_nmi_timer->adjust(m_cpu->cycles_to_attotime(4));
				}
			}

			if (m_scanline == m_scanlines_per_frame - 1)
			{
				// logerror("vblank ending\n");
				/* clear the vblank & sprite hit flag */
				m_regs[PPU_STATUS] &= ~(PPU_STATUS_VBLANK | PPU_STATUS_SPRITE0_HIT | PPU_STATUS_8SPRITES);
			}

			/* see if we rolled */
			else if (m_scanline == m_scanlines_per_frame)
			{
				/* if background or sprites are enabled, copy the ppu address latch */
				if (!blanked)
					m_refresh_data = m_refresh_latch;

				/* reset the scanline count */
				m_scanline = 0;
				//logerror("sprite 0 x: %d y: %d num: %d\n", m_spriteram[3], m_spriteram[0] + 1, m_spriteram[1]);
			}

			next_scanline = m_scanline + 1;
			if (next_scanline == m_scanlines_per_frame)
				next_scanline = 0;

			// Call us back when the hblank starts for this scanline
			m_hblank_timer->adjust(m_cpu->cycles_to_attotime(86.67)); // ??? FIXME - hardcoding NTSC, need better calculation

			// trigger again at the start of the next scanline
			m_scanline_timer->adjust(m_screen->time_until_pos(next_scanline * m_scan_scale));
			break;
	}
}

void ppu2c0x_device::draw_background( UINT8 *line_priority )
{
	bitmap_ind16 &bitmap = *m_bitmap;
	int start_x = (m_x_fine ^ 0x07) - 7;
	UINT16 back_pen;
	UINT16 *dest;

	UINT8 scroll_x_coarse, scroll_y_coarse, scroll_y_fine, color_mask;
	int x, tile_index, i;

	const pen_t *color_table;
	const pen_t *paldata;

	m_tilecount = 0;

	/* setup the color mask and colortable to use */
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
	{
		color_mask = 0xf0;
		color_table = m_colortable_mono.get();
	}
	else
	{
		color_mask = 0xff;
		color_table = m_colortable.get();
	}

	/* cache the background pen */
	back_pen = (m_back_color & color_mask) + m_color_base;

	/* determine where in the nametable to start drawing from */
	/* based on the current scanline and scroll regs */
	scroll_x_coarse =  m_refresh_data & 0x1f;
	scroll_y_coarse = (m_refresh_data & 0x3e0) >> 5;
	scroll_y_fine   = (m_refresh_data & 0x7000) >> 12;

	x = scroll_x_coarse;

	/* get the tile index */
	tile_index = ((m_refresh_data & 0xc00) | 0x2000) + scroll_y_coarse * 32;

	/* set up dest */
	dest = &bitmap.pix16(m_scanline, start_x);

	/* draw the 32 or 33 tiles that make up a line */
	while (m_tilecount < 34)
	{
		int color_byte;
		int color_bits;
		int pos;
		int index1;
		int page, page2, address;
		UINT16 pen;

		index1 = tile_index + x;

		// this is attribute table stuff! (actually read 2 in PPUspeak)!
		/* Figure out which byte in the color table to use */
		pos = ((index1 & 0x380) >> 4) | ((index1 & 0x1f) >> 2);
		page = (index1 & 0x0c00) >> 10;
		address = 0x3c0 + pos;
		color_byte = readbyte((((page * 0x400) + address) & 0xfff) + 0x2000);

		/* figure out which bits in the color table to use */
		color_bits = ((index1 & 0x40) >> 4) + (index1 & 0x02);

		// page2 is the output of the nametable read (this section is the FIRST read per tile!)
		address = index1 & 0x3ff;
		page2 = readbyte(index1);

		// 27/12/2002
		if (!m_latch.isnull())
			m_latch((m_tile_page << 10) | (page2 << 4));

		if (start_x < VISIBLE_SCREEN_WIDTH)
		{
			UINT8 plane1, plane2;
			paldata = &color_table[4 * (((color_byte >> color_bits) & 0x03))];

			// need to read 0x0000 or 0x1000 + 16*nametable data
			address = ((m_tile_page) ? 0x1000 : 0) + (page2 * 16);
			// plus something that accounts for y
			address += scroll_y_fine;

			plane1 = readbyte((address & 0x1fff));
			plane2 = readbyte((address + 8) & 0x1fff);

			/* render the pixel */
			for (i = 0; i < 8; i++)
			{
				UINT8 pix;
				pix = ((plane1 >> 7) & 1) | (((plane2 >> 7) & 1) << 1);
				plane1 = plane1 << 1;
				plane2 = plane2 << 1;
				if ((start_x + i) >= 0 && (start_x + i) < VISIBLE_SCREEN_WIDTH)
				{
					if (pix)
					{
						pen = paldata[pix];
						line_priority[start_x + i] |= 0x02;
					}
					else
					{
						pen = back_pen;
					}
					*dest = pen;
				}
				dest++;
			}

			start_x += 8;

			/* move to next tile over and toggle the horizontal name table if necessary */
			x++;
			if (x > 31)
			{
				x = 0;
				tile_index ^= 0x400;
			}
		}
		m_tilecount++;
	}

	/* if the left 8 pixels for the background are off, blank 'em */
	if (!(m_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND_L8))
	{
		dest = &bitmap.pix16(m_scanline);
		for (i = 0; i < 8; i++)
		{
			*(dest++) = back_pen;
			line_priority[i] ^= 0x02;
		}
	}
}

void ppu2c0x_device::draw_sprites( UINT8 *line_priority )
{
	bitmap_ind16 &bitmap = *m_bitmap;

	int sprite_xpos, sprite_ypos, sprite_index;
	int tile, index1;
	int pri;

	int flipx, flipy, color;
	int size;
	int sprite_count = 0;
	int sprite_line;

	int first_pixel;

	const pen_t *paldata;
	int pixel;

	/* determine if the sprites are 8x8 or 8x16 */
	size = (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;

	first_pixel = (m_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES_L8)? 0: 8;

	for (sprite_index = 0; sprite_index < SPRITERAM_SIZE; sprite_index += 4)
	{
		UINT8 plane1;
		UINT8 plane2;

		sprite_ypos = m_spriteram[sprite_index] + 1;
		sprite_xpos = m_spriteram[sprite_index + 3];

		// The sprite collision acts funny on the last pixel of a scanline.
		// The various scanline latches update while the last few pixels
		// are being drawn. Since we don't do cycle-by-cycle PPU emulation,
		// we fudge it a bit here so that sprite 0 collisions are detected
		// when, e.g., sprite x is 254, sprite y is 29 and we're rendering
		// at the end of scanline 28.
		// Battletoads needs this level of precision to be playable.
		if ((sprite_index == 0) && (sprite_xpos == 254))
		{
			sprite_ypos--;
			/* set the "sprite 0 hit" flag if appropriate */
			if (line_priority[sprite_xpos] & 0x02)
				m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
		}

		/* if the sprite isn't visible, skip it */
		if ((sprite_ypos + size <= m_scanline) || (sprite_ypos > m_scanline))
			continue;

		tile  =  m_spriteram[sprite_index + 1];
		color = (m_spriteram[sprite_index + 2] & 0x03) + 4;
		pri   =  m_spriteram[sprite_index + 2] & 0x20;
		flipx =  m_spriteram[sprite_index + 2] & 0x40;
		flipy =  m_spriteram[sprite_index + 2] & 0x80;

		if (size == 16)
		{
			/* if it's 8x16 and odd-numbered, draw the other half instead */
			if (tile & 0x01)
			{
				tile &= ~0x01;
				tile |= 0x100;
			}
		}

		if (!m_latch.isnull())
			m_latch((m_sprite_page << 10) | ((tile & 0xff) << 4));

		/* compute the character's line to draw */
		sprite_line = m_scanline - sprite_ypos;

		if (flipy)
			sprite_line = (size - 1) - sprite_line;

		paldata = &m_colortable[4 * color];

		if (size == 16 && sprite_line > 7)
		{
			tile++;
			sprite_line -= 8;
		}

		index1 = tile * 16;
		if (size == 8)
			index1 += ((m_sprite_page == 0) ? 0 : 0x1000);

		plane1 = readbyte((index1 + sprite_line + 0) & 0x1fff);
		plane2 = readbyte((index1 + sprite_line + 8) & 0x1fff);

		/* if there are more than 8 sprites on this line, set the flag */
		if (sprite_count == 8)
		{
			m_regs[PPU_STATUS] |= PPU_STATUS_8SPRITES;
//          logerror ("> 8 sprites, scanline: %d\n", m_scanline);

			/* the real NES only draws up to 8 sprites - the rest should be invisible */
			break;
		}

		sprite_count++;

		/* abort drawing if sprites aren't rendered */
		if (!(m_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES))
			continue;

		if (pri)
		{
			/* draw the low-priority sprites */
			for (pixel = 0; pixel < 8; pixel++)
			{
				UINT8 pixel_data;
				if (flipx)
				{
					pixel_data = (plane1 & 1) + ((plane2 & 1) << 1);
					plane1 = plane1 >> 1;
					plane2 = plane2 >> 1;
				}
				else
				{
					pixel_data = ((plane1 >> 7) & 1) | (((plane2 >> 7) & 1) << 1);
					plane1 = plane1 << 1;
					plane2 = plane2 << 1;
				}

				/* is this pixel non-transparent? */
				if (sprite_xpos + pixel >= first_pixel)
				{
					if (pixel_data)
					{
						/* has the background (or another sprite) already been drawn here? */
						if ((sprite_xpos + pixel) < VISIBLE_SCREEN_WIDTH)
						{
							if (!line_priority[sprite_xpos + pixel])
							{
								/* no, draw */
								bitmap.pix16(m_scanline, sprite_xpos + pixel) = paldata[pixel_data];
							}
							/* indicate that a sprite was drawn at this location, even if it's not seen */
							line_priority[sprite_xpos + pixel] |= 0x01;
						}
					}

					/* set the "sprite 0 hit" flag if appropriate */
					if (sprite_index == 0 && (pixel_data & 0x03) && ((sprite_xpos + pixel) < 255) && (line_priority[sprite_xpos + pixel] & 0x02))
						m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
				}
			}
		}
		else
		{
			/* draw the high-priority sprites */
			for (pixel = 0; pixel < 8; pixel++)
			{
				UINT8 pixel_data;
				if (flipx)
				{
					pixel_data = (plane1 & 1) + ((plane2 & 1) << 1);
					plane1 = plane1 >> 1;
					plane2 = plane2 >> 1;
				}
				else
				{
					pixel_data = ((plane1 >> 7) & 1) | (((plane2 >> 7) & 1) << 1);
					plane1 = plane1 << 1;
					plane2 = plane2 << 1;
				}

				/* is this pixel non-transparent? */
				if (sprite_xpos + pixel >= first_pixel)
				{
					if (pixel_data)
					{
						if ((sprite_xpos + pixel) < VISIBLE_SCREEN_WIDTH)
						{
							/* has another sprite been drawn here? */
							if (!(line_priority[sprite_xpos + pixel] & 0x01))
							{
								/* no, draw */
								bitmap.pix16(m_scanline, sprite_xpos + pixel) = paldata[pixel_data];
								line_priority[sprite_xpos + pixel] |= 0x01;
							}
						}
					}

					/* set the "sprite 0 hit" flag if appropriate */
					if (sprite_index == 0 && (pixel_data & 0x03) && ((sprite_xpos + pixel) < 255) && (line_priority[sprite_xpos + pixel] & 0x02))
						m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
				}
			}
		}
	}
}

/*************************************
 *
 *  Scanline Rendering and Update
 *
 *************************************/

void ppu2c0x_device::render_scanline( void )
{
	UINT8 line_priority[VISIBLE_SCREEN_WIDTH];

	/* lets see how long it takes */
	g_profiler.start(PROFILER_USER1);

	/* clear the line priority for this scanline */
	memset(line_priority, 0, VISIBLE_SCREEN_WIDTH);

	m_draw_phase = PPU_DRAW_BG;

	/* see if we need to render the background */
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND)
		draw_background(line_priority);
	else
	{
		bitmap_ind16 &bitmap = *m_bitmap;
		UINT8 color_mask;
		UINT16 back_pen;
		int i;

		/* setup the color mask and colortable to use */
		if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
			color_mask = 0xf0;
		else
			color_mask = 0xff;

		/* cache the background pen */
		back_pen = (m_back_color & color_mask) + m_color_base;

		// Fill this scanline with the background pen.
		for (i = 0; i < bitmap.width(); i++)
			bitmap.pix16(m_scanline, i) = back_pen;
	}

	m_draw_phase = PPU_DRAW_OAM;

	/* if sprites are on, draw them, but we call always to process them */
	draw_sprites(line_priority);

	m_draw_phase = PPU_DRAW_BG;

	/* done updating, whew */
	g_profiler.stop();
}

void ppu2c0x_device::update_scanline( void )
{
	if (m_scanline <= PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		/* Render this scanline if appropriate */
		if (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES))
		{
			/* If background or sprites are enabled, copy the ppu address latch */
			/* Copy only the scroll x-coarse and the x-overflow bit */
			m_refresh_data &= ~0x041f;
			m_refresh_data |= (m_refresh_latch & 0x041f);

//          logerror("updating refresh_data: %04x (scanline: %d)\n", m_refresh_data, m_scanline);
			render_scanline();
		}
		else
		{
			bitmap_ind16 &bitmap = *m_bitmap;
			UINT8 color_mask;
			UINT16 back_pen;
			int i;

			/* setup the color mask and colortable to use */
			if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				color_mask = 0xf0;
			else
				color_mask = 0xff;

			/* cache the background pen */
			if (m_videomem_addr >= 0x3f00)
			{
				// If the PPU's VRAM address happens to point into palette ram space while
				// both the sprites and background are disabled, the PPU paints the scanline
				// with the palette entry at the VRAM address instead of the usual background
				// pen. Micro Machines makes use of this feature.
				int pen_num;

				if (m_videomem_addr & 0x03)
					pen_num = m_palette_ram[m_videomem_addr & 0x1f];
				else
					pen_num = m_palette_ram[0];

				back_pen = pen_num + m_color_base;
			}
			else
				back_pen = (m_back_color & color_mask) + m_color_base;

			// Fill this scanline with the background pen.
			for (i = 0; i < bitmap.width(); i++)
				bitmap.pix16(m_scanline, i) = back_pen;
		}

		/* increment the fine y-scroll */
		m_refresh_data += 0x1000;

		/* if it's rolled, increment the coarse y-scroll */
		if (m_refresh_data & 0x8000)
		{
			UINT16 tmp;
			tmp = (m_refresh_data & 0x03e0) + 0x20;
			m_refresh_data &= 0x7c1f;

			/* handle bizarro scrolling rollover at the 30th (not 32nd) vertical tile */
			if (tmp == 0x03c0)
				m_refresh_data ^= 0x0800;
			else
				m_refresh_data |= (tmp & 0x03e0);

//          logerror("updating refresh_data: %04x\n", m_refresh_data);
		}
	}

}

/*************************************
*
*   PPU Memory functions
*
*************************************/

WRITE8_MEMBER( ppu2c0x_device::palette_write )
{
	int color_base = m_color_base;
	int color_emphasis = (m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) * 2;

	// palette RAM is only 6 bits wide
	data &= 0x3f;

	// transparent pens are mirrored!
	if (offset & 0x3)
	{
		m_palette_ram[offset & 0x1f] = data;
		m_colortable[offset & 0x1f] = color_base + data + color_emphasis;
		m_colortable_mono[offset & 0x1f] = color_base + (data & 0xf0) + color_emphasis;
	}
	else
	{
		int i;
		if (0 == (offset & 0xf))
		{
			m_back_color = data;
			for (i = 0; i < 32; i += 4)
			{
				m_colortable[i] = color_base + data + color_emphasis;
				m_colortable_mono[i] = color_base + (data & 0xf0) + color_emphasis;
			}
		}
		m_palette_ram[offset & 0xf] = m_palette_ram[(offset & 0xf) + 0x10] = data;
	}
}

READ8_MEMBER( ppu2c0x_device::palette_read )
{
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
		return (m_palette_ram[offset & 0x1f] & 0x30);

	else
		return (m_palette_ram[offset & 0x1f]);
}

/*************************************
 *
 *  PPU Registers Read
 *
 *************************************/

READ8_MEMBER( ppu2c0x_device::read )
{
	if (offset >= PPU_MAX_REG)
	{
		logerror("PPU %s: Attempting to read past the chip: offset %x\n", this->tag().c_str(), offset);
		offset &= PPU_MAX_REG - 1;
	}

	// see which register to read
	switch (offset & 7)
	{
		case PPU_STATUS: /* 2 */
			// The top 3 bits of the status register are the only ones that report data. The
			// remainder contain whatever was last in the PPU data latch, except on the RC2C05 (protection)
			if (m_security_value)
				m_data_latch = (m_regs[PPU_STATUS] & 0xc0) | m_security_value;
			else
				m_data_latch = m_regs[PPU_STATUS] | (m_data_latch & 0x1f);

			// Reset hi/lo scroll toggle
			m_toggle = 0;

			// If the vblank bit is set, clear all status bits but the 2 sprite flags
			if (m_data_latch & PPU_STATUS_VBLANK)
				m_regs[PPU_STATUS] &= 0x60;
			break;

		case PPU_SPRITE_DATA: /* 4 */
			m_data_latch = m_spriteram[m_regs[PPU_SPRITE_ADDRESS]];
			break;

		case PPU_DATA: /* 7 */
			if (!m_latch.isnull())
				m_latch( m_videomem_addr & 0x3fff);

			if (m_videomem_addr >= 0x3f00)
			{
				m_data_latch = readbyte(m_videomem_addr);
				// buffer the mirrored NT data
				m_buffered_data = readbyte(m_videomem_addr & 0x2fff);
			}
			else
			{
				m_data_latch = m_buffered_data;
				m_buffered_data = readbyte(m_videomem_addr);
			}

			m_videomem_addr += m_add;
			break;

		default:
			break;
	}

	return m_data_latch;
}


/*************************************
 *
 *  PPU Registers Write
 *
 *************************************/

WRITE8_MEMBER( ppu2c0x_device::write )
{
	int color_base;

	color_base = m_color_base;

	if (offset >= PPU_MAX_REG)
	{
		logerror("PPU %s: Attempting to write past the chip: offset %x, data %x\n", this->tag().c_str(), offset, data);
		offset &= PPU_MAX_REG - 1;
	}

#ifdef MAME_DEBUG
	if (m_scanline <= PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		logerror("PPU register %d write %02x during non-vblank scanline %d (MAME %d, beam pos: %d)\n", offset, data, m_scanline, m_screen->vpos(), m_screen->hpos());
	}
#endif

	/* on the RC2C05, PPU_CONTROL0 and PPU_CONTROL1 are swapped (protection) */
	if ((m_security_value) && !(offset & 6))
		offset ^= 1;

	switch (offset & 7)
	{
		case PPU_CONTROL0: /* 0 */
			m_regs[PPU_CONTROL0] = data;

			/* update the name table number on our refresh latches */
			m_refresh_latch &= 0x73ff;
			m_refresh_latch |= (data & 3) << 10;

			/* the char ram bank points either 0x0000 or 0x1000 (page 0 or page 4) */
			m_tile_page = (data & PPU_CONTROL0_CHR_SELECT) >> 2;
			m_sprite_page = (data & PPU_CONTROL0_SPR_SELECT) >> 1;

			m_add = (data & PPU_CONTROL0_INC) ? 32 : 1;
//          logerror("control0 write: %02x (scanline: %d)\n", data, m_scanline);
			break;

		case PPU_CONTROL1: /* 1 */
			/* if color intensity has changed, change all the color tables to reflect them */
			if ((data & PPU_CONTROL1_COLOR_EMPHASIS) != (m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS))
			{
				int i;
				for (i = 0; i <= 0x1f; i++)
				{
					UINT8 oldColor = m_palette_ram[i];

					m_colortable[i] = color_base + oldColor + (data & PPU_CONTROL1_COLOR_EMPHASIS) * 2;
				}
			}

//          logerror("control1 write: %02x (scanline: %d)\n", data, m_scanline);
			m_regs[PPU_CONTROL1] = data;
			break;

		case PPU_SPRITE_ADDRESS: /* 3 */
			m_regs[PPU_SPRITE_ADDRESS] = data;
			break;

		case PPU_SPRITE_DATA: /* 4 */
			// If the PPU is currently rendering the screen, 0xff is written instead of the desired data.
			if (m_use_sprite_write_limitation)
				if (m_scanline <= PPU_BOTTOM_VISIBLE_SCANLINE)
					data = 0xff;
			m_spriteram[m_regs[PPU_SPRITE_ADDRESS]] = data;
			m_regs[PPU_SPRITE_ADDRESS] = (m_regs[PPU_SPRITE_ADDRESS] + 1) & 0xff;
			break;

		case PPU_SCROLL: /* 5 */
			if (m_toggle)
			{
				/* second write */
				m_refresh_latch &= 0x0c1f;
				m_refresh_latch |= (data & 0xf8) << 2;
				m_refresh_latch |= (data & 0x07) << 12;
//              logerror("scroll write 2: %d, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
			}
			else
			{
				/* first write */
				m_refresh_latch &= 0x7fe0;
				m_refresh_latch |= (data & 0xf8) >> 3;

				m_x_fine = data & 7;
//              logerror("scroll write 1: %d, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
			}

			m_toggle ^= 1;
			break;

		case PPU_ADDRESS: /* 6 */
			if (m_toggle)
			{
				/* second write */
				m_refresh_latch &= 0x7f00;
				m_refresh_latch |= data;
				m_refresh_data = m_refresh_latch;

				m_videomem_addr = m_refresh_latch;
//              logerror("vram addr write 2: %02x, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
			}
			else
			{
				/* first write */
				m_refresh_latch &= 0x00ff;
				m_refresh_latch |= (data & 0x3f) << 8;
//              logerror("vram addr write 1: %02x, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
			}

			m_toggle ^= 1;
			break;

		case PPU_DATA: /* 7 */
			{
				int tempAddr = m_videomem_addr & 0x3fff;

				if (!m_latch.isnull())
					m_latch(tempAddr);

				/* if there's a callback, call it now */
				if (!m_vidaccess_callback_proc.isnull())
					data = m_vidaccess_callback_proc(tempAddr, data);

				/* see if it's on the chargen portion */
				if (tempAddr < 0x2000)
				{
					/* store the data */
					writebyte(tempAddr, data);
				}

				else
				{
					writebyte(tempAddr, data);
				}
				/* increment the address */
				m_videomem_addr += m_add;
			}
			break;

		default:
			/* ignore other registers writes */
			break;
	}

	m_data_latch = data;
}

/*************************************
 *
 *  Sprite DMA
 *
 *************************************/

void ppu2c0x_device::spriteram_dma( address_space &space, const UINT8 page )
{
	int i;
	int address = page << 8;

	for (i = 0; i < SPRITERAM_SIZE; i++)
	{
		UINT8 spriteData = space.read_byte(address + i);
		space.write_byte(0x2004, spriteData);
	}

	// should last 513 CPU cycles.
	space.device().execute().adjust_icount(-513);
}

/*************************************
 *
 *  PPU Rendering
 *
 *************************************/

void ppu2c0x_device::render( bitmap_ind16 &bitmap, int flipx, int flipy, int sx, int sy )
{
	copybitmap(bitmap, *m_bitmap, flipx, flipy, sx, sy, bitmap.cliprect());
}

/*************************************
 *
 *  Utility functions
 *
 *************************************/

int ppu2c0x_device::get_pixel( int x, int y )
{
	if (x >= VISIBLE_SCREEN_WIDTH)
		x = VISIBLE_SCREEN_WIDTH - 1;

	if (y >= VISIBLE_SCREEN_HEIGHT)
		y = VISIBLE_SCREEN_HEIGHT - 1;

	return m_bitmap->pix16(y, x);
}
