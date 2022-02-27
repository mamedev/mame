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

#include "screen.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(PPU_2C02,    ppu2c02_device,       "ppu2c02",    "2C02 PPU")
DEFINE_DEVICE_TYPE(PPU_2C03B,   ppu2c03b_device,      "ppu2c03b",   "2C03B PPC")
DEFINE_DEVICE_TYPE(PPU_2C04,    ppu2c04_device,       "ppu2c04",    "2C04 PPU")
DEFINE_DEVICE_TYPE(PPU_2C07,    ppu2c07_device,       "ppu2c07",    "2C07 PPU")
DEFINE_DEVICE_TYPE(PPU_PALC,    ppupalc_device,       "ppupalc",    "Generic PAL Clone PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_01, ppu2c05_01_device,    "ppu2c05_01", "2C05_01 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_02, ppu2c05_02_device,    "ppu2c05_02", "2C05_02 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_03, ppu2c05_03_device,    "ppu2c05_03", "2C05_03 PPU")
DEFINE_DEVICE_TYPE(PPU_2C05_04, ppu2c05_04_device,    "ppu2c05_04", "2C05_04 PPU")
DEFINE_DEVICE_TYPE(PPU_2C04C,   ppu2c04_clone_device, "ppu2c04c",   "2C04 Clone PPU")


// default address map
void ppu2c0x_device::ppu2c0x(address_map& map)
{
	if (!has_configured_map(0))
	{
		map(0x0000, 0x3eff).ram();
		map(0x3f00, 0x3fff).rw(FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
		//map(0x0000, 0x3fff).ram();
	}
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ppu2c0x_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  ppu2c0x_device - constructor
//-------------------------------------------------

void ppu2c0x_device::device_config_complete()
{
	/* reset the callbacks */
	m_scanline_callback_proc.set(nullptr);
	m_hblank_callback_proc.set(nullptr);
	m_vidaccess_callback_proc.set(nullptr);
	m_latch.set(nullptr);
}

ppu2c0x_device::ppu2c0x_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, address_map_constructor internal_map) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 17, 0, internal_map),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_scanline(0),  // reset the scanline count
	m_videoram_addr_mask(0x3fff),
	m_global_refresh_mask(0x7fff),
	m_line_write_increment_large(32),
	m_paletteram_in_ppuspace(false),
	m_tile_page(0),
	m_back_color(0),
	m_refresh_data(0),
	m_x_fine(0),
	m_toggle(0),
	m_tilecount(0),
	m_latch(*this),
	m_scanline_callback_proc(*this),
	m_hblank_callback_proc(*this),
	m_vidaccess_callback_proc(*this),
	m_int_callback(*this),
	m_refresh_latch(0),
	m_add(1),
	m_videomem_addr(0),
	m_data_latch(0),
	m_buffered_data(0),
	m_sprite_page(0),
	m_scan_scale(1), // set the scan scale (this is for dual monitor vertical setups)
	m_draw_phase(0),
	m_use_sprite_write_limitation(true)
{
	for (auto& elem : m_regs)
		elem = 0;

	m_scanlines_per_frame = NTSC_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE;

	/* usually, no security value... */
	m_security_value = 0;
}

ppu2c0x_device::ppu2c0x_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(ppu2c0x_device::ppu2c0x), this))
{
	m_paletteram_in_ppuspace = true;
}

ppu2c0x_rgb_device::ppu2c0x_rgb_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock),
	m_palette_data(*this, "palette")
{
}

// NTSC NES
ppu2c02_device::ppu2c02_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, PPU_2C02, tag, owner, clock)
{
}

// Playchoice 10
ppu2c03b_device::ppu2c03b_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C03B, tag, owner, clock)
{
}

// Vs. Unisystem
ppu2c04_device::ppu2c04_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C04, tag, owner, clock)
{
}

// PAL NES
ppu2c07_device::ppu2c07_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, PPU_2C07, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
}

// PAL clones
ppupalc_device::ppupalc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, PPU_PALC, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_PALC;
}

// The PPU_2C05 variants have different protection value, set at device start, but otherwise are all the same...
// Vs. Unisystem (Ninja Jajamaru Kun)
ppu2c05_01_device::ppu2c05_01_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_01, tag, owner, clock)
{
	m_security_value = 0x1b;    // game (jajamaru) doesn't seem to ever actually check it
}
// Vs. Unisystem (Mighty Bomb Jack)
ppu2c05_02_device::ppu2c05_02_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_02, tag, owner, clock)
{
	m_security_value = 0x3d;
}
// Vs. Unisystem (Gumshoe)
ppu2c05_03_device::ppu2c05_03_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_03, tag, owner, clock)
{
	m_security_value = 0x1c;
}
// Vs. Unisystem (Top Gun)
ppu2c05_04_device::ppu2c05_04_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_rgb_device(mconfig, PPU_2C05_04, tag, owner, clock)
{
	m_security_value = 0x1b;
}

// Vs. Unisystem (Super Mario Bros. bootlegs)
ppu2c04_clone_device::ppu2c04_clone_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, PPU_2C04C, tag, owner, clock),
	m_palette_data(*this, "palette")
{
	m_scanlines_per_frame = VS_CLONE_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_VS_CLONE;

	// background and sprites are always enabled; monochrome and color emphasis aren't supported
	m_regs[PPU_CONTROL1] = ~(PPU_CONTROL1_COLOR_EMPHASIS | PPU_CONTROL1_DISPLAY_MONO);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ppu2c0x_device::start_nopalram()
{
	// bind our handler
	m_int_callback.resolve_safe();

	// allocate timers
	m_hblank_timer = timer_alloc(TIMER_HBLANK);
	m_nmi_timer = timer_alloc(TIMER_NMI);
	m_scanline_timer = timer_alloc(TIMER_SCANLINE);

	/* initialize the scanline handling portion */
	m_scanline_timer->adjust(screen().time_until_pos(1));
	m_hblank_timer->adjust(m_cpu->cycles_to_attotime(260) / 3); // ??? FIXME - hardcoding NTSC, need better calculation
	m_nmi_timer->adjust(attotime::never);

	/* allocate a screen bitmap, videomem and spriteram, a dirtychar array and the monochromatic colortable */
	m_bitmap = std::make_unique<bitmap_rgb32>(VISIBLE_SCREEN_WIDTH, VISIBLE_SCREEN_HEIGHT);
	m_spriteram = make_unique_clear<uint8_t[]>(SPRITERAM_SIZE);

	init_palette_tables();

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
	save_item(NAME(m_vblank_first_scanline));
	save_item(NAME(m_regs));
	save_item(NAME(m_draw_phase));
	save_item(NAME(m_tilecount));
	save_pointer(NAME(m_spriteram), SPRITERAM_SIZE);

	save_item(NAME(*m_bitmap));
}

void ppu2c0x_device::device_start()
{
	start_nopalram();

	m_palette_ram.resize(0x20, 0x00);

	save_item(NAME(m_palette_ram));
}

void ppu2c04_clone_device::device_start()
{
	ppu2c0x_device::device_start();

	/* this PPU clone draws sprites into a frame buffer before displaying them,
	causing sprite rendering to be one frame behind tile/background rendering
	(mainly noticeable during scrolling)
	to simulate that, we can just have a secondary OAM buffer and swap them
	at the end of each frame.

	(theoretically this can cause the wrong sprite tiles to be drawn for
	one frame after changing CHR banks, but the Vs. SMB bootlegs that use
	this clone hardware don't actually have CHR bank switching anyway.
	also generally affects PPU-side read timings involving the OAM, but
	this still doesn't seem to matter for Vs. SMB specifically)
	*/
	m_spritebuf = make_unique_clear<uint8_t[]>(SPRITERAM_SIZE);
	save_pointer(NAME(m_spritebuf), SPRITERAM_SIZE);
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

uint8_t ppu2c0x_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void ppu2c0x_device::writebyte(offs_t address, uint8_t data)
{
	space().write_byte(address, data);
}


inline uint16_t ppu2c0x_device::apply_grayscale_and_emphasis(uint8_t color)
{
	uint16_t palval = color;
	palval &= (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO) ? 0x30 : 0x3f;
	palval |= (m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 1;

	return palval;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*************************************
 *
 *  PPU Palette Initialization
 *
 *************************************/

void ppu2c0x_device::apply_color_emphasis_and_clamp(bool is_pal_or_dendy, int color_emphasis, double& R, double& G, double& B)
{
	if (is_pal_or_dendy) // PAL machines swap the colour emphasis bits, this means the red/blue highlighting on rampart tally bar doesn't look as good
	{
		color_emphasis = bitswap<3>(color_emphasis, 2, 0, 1);
	}

	static constexpr double rgb_mod[8][3] =
	{
		//  R      G      B
		{ 1.0,   1.0,   1.0   },
		{ 1.24,  0.915, 0.743 },
		{ 0.794, 1.09,  0.882 },
		{ 0.905, 1.03,  1.28  },
		{ 0.741, 0.987, 1.0   },
		{ 1.02,  0.908, 0.979 },
		{ 1.02,  0.98,  0.653 },
		{ 0.75,  0.75,  0.75  }
	};

	// Clipping, in case of saturation
	R = std::clamp(R * rgb_mod[color_emphasis][0], 0.0, 255.0);
	G = std::clamp(G * rgb_mod[color_emphasis][1], 0.0, 255.0);
	B = std::clamp(B * rgb_mod[color_emphasis][2], 0.0, 255.0);
}

rgb_t ppu2c0x_device::nespal_to_RGB(int color_intensity, int color_num, int color_emphasis, bool is_pal_or_dendy)
{
	const double tint = 0.22; /* adjust to taste */
	const double hue = 287.0;

	const double Kr = 0.2989;
	const double Kb = 0.1145;
	const double Ku = 2.029;
	const double Kv = 1.140;

	static const double brightness[3][4] =
	{
		{ 0.50, 0.75, 1.0, 1.0 },
		{ 0.29, 0.45, 0.73, 0.9 },
		{ 0, 0.24, 0.47, 0.77 }
	};

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
	double R = (y + Kv * v) * 255.0;
	double G = (y - (Kb * Ku * u + Kr * Kv * v) / (1 - Kb - Kr)) * 255.0;
	double B = (y + Ku * u) * 255.0;

	apply_color_emphasis_and_clamp(is_pal_or_dendy, color_emphasis, R, G, B);

	return rgb_t(floor(R + .5), floor(G + .5), floor(B + .5));
}

void ppu2c0x_device::init_palette_tables()
{
	bool is_pal = m_scanlines_per_frame != NTSC_SCANLINES_PER_FRAME;

	/* This routine builds a palette using a transformation from */
	/* the YUV (Y, B-Y, R-Y) to the RGB color space */

	/* The NES has a 64 color palette                        */
	/* 16 colors, with 4 luminance levels for each color     */
	/* The 16 colors circle around the YUV color space,      */

	int entry = 0;

	/* Loop through the emphasis modes (8 total) */
	for (int color_emphasis = 0; color_emphasis < 8; color_emphasis++)
	{
		/* loop through the 4 intensities */
		for (int color_intensity = 0; color_intensity < 4; color_intensity++)
		{
			/* loop through the 16 colors */
			for (int color_num = 0; color_num < 16; color_num++)
			{
				rgb_t col = nespal_to_RGB(color_intensity, color_num, color_emphasis, is_pal);

				m_nespens[entry] = (uint32_t)col;
				entry++;

			}
		}
	}
}

void ppu2c0x_rgb_device::init_palette_tables()
{
	/* Loop through the emphasis modes (8 total) */
	int entry = 0;
	for (int color_emphasis = 0; color_emphasis < 8; color_emphasis++)
	{
		for (int color_num = 0; color_num < 64; color_num++)
		{
			int R = ((color_emphasis & 1) ? 7 : m_palette_data[color_num * 3]);
			int G = ((color_emphasis & 2) ? 7 : m_palette_data[color_num * 3 + 1]);
			int B = ((color_emphasis & 4) ? 7 : m_palette_data[color_num * 3 + 2]);

			m_nespens[entry] = (pal3bit(R)<<16) | (pal3bit(G)<<8) | pal3bit(B);

			//set_pen_color(entry++, pal3bit(R), pal3bit(G), pal3bit(B));
			entry++;
		}
	}
}

void ppu2c04_clone_device::init_palette_tables()
{
	/* clone HW doesn't use color emphasis bits.
	   however, it does have two separate palettes: colors 0-63 for background, and 64-127 for sprites
	   (although the tile and sprite colors are identical in the Vs. SMB bootleg ROMs)
	*/
	for (int color_num = 0; color_num < 64*2; color_num++)
	{
		/* A7 line on palette ROMs is always high, color bits are in reverse order */
		u8 color = m_palette_data[color_num | 0x80];
		int R = bitswap<3>(color, 0, 1, 2);
		int G = bitswap<3>(color, 3, 4, 5);
		int B = bitswap<2>(color, 6, 7);

		m_nespens[color_num] = (pal3bit(R) << 16) | (pal3bit(G) << 8) | pal2bit(B);
	}
}

/*************************************
 *
 *  PPU Initialization and Disposal
 *
 *************************************/

//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void ppu2c0x_device::device_timer(emu_timer& timer, device_timer_id id, int param)
{
	int blanked, vblank;

	switch (id)
	{
	case TIMER_HBLANK:
		blanked = (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
		vblank = ((m_scanline >= m_vblank_first_scanline - 1) && (m_scanline < m_scanlines_per_frame - 1)) ? 1 : 0;

		//update_scanline();

		if (!m_hblank_callback_proc.isnull())
			m_hblank_callback_proc(m_scanline, vblank, blanked);

		m_hblank_timer->adjust(attotime::never);
		break;

	case TIMER_NMI:
		// Actually fire the VMI
		m_int_callback(ASSERT_LINE);
		m_int_callback(CLEAR_LINE);

		m_nmi_timer->adjust(attotime::never);
		break;

	case TIMER_SCANLINE:
		blanked = (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
		vblank = ((m_scanline >= m_vblank_first_scanline - 1) && (m_scanline < m_scanlines_per_frame - 1)) ? 1 : 0;
		int next_scanline;

		/* if a callback is available, call it */
		if (!m_scanline_callback_proc.isnull())
			m_scanline_callback_proc(m_scanline, vblank, blanked);

		/* update the scanline that just went by */
		update_scanline();

		/* increment our scanline count */
		m_scanline++;

		//logerror("starting scanline %d (MAME %d, beam %d)\n", m_scanline, device->screen().vpos(), device->screen().hpos());

		/* Note: this is called at the _end_ of each scanline */
		if (m_scanline == m_vblank_first_scanline)
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
			//logerror("vblank ending\n");
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
		m_hblank_timer->adjust(m_cpu->cycles_to_attotime(260) / 3); // ??? FIXME - hardcoding NTSC, need better calculation

		// trigger again at the start of the next scanline
		m_scanline_timer->adjust(screen().time_until_pos(next_scanline * m_scan_scale));
		break;
	}
}


void ppu2c0x_device::read_tile_plane_data(int address, int color)
{
	m_planebuf[0] = readbyte(address & 0x1fff);
	m_planebuf[1] = readbyte((address + 8) & 0x1fff);
}

void ppu2c0x_device::shift_tile_plane_data(uint8_t& pix)
{
	pix = BIT(m_planebuf[0], 7) | (BIT(m_planebuf[1], 7) << 1);
	m_planebuf[0] <<= 1;
	m_planebuf[1] <<= 1;
}

void ppu2c0x_device::draw_tile_pixel(uint8_t pix, int color, uint32_t back_pen, uint32_t*& dest)
{
	uint16_t palval = pix ? m_palette_ram[((4 * color) + pix) & 0x1f] : back_pen;

	*dest = m_nespens[apply_grayscale_and_emphasis(palval)];
}

void ppu2c0x_device::draw_tile(uint8_t* line_priority, int color_byte, int color_bits, int address, int start_x, uint32_t back_pen, uint32_t*& dest)
{
	int color = (color_byte >> color_bits) & 0x03;

	read_tile_plane_data(address, color);

	/* render the pixel */
	for (int i = 0; i < 8; i++)
	{
		uint8_t pix;
		shift_tile_plane_data(pix);

		if ((start_x + i) >= 0 && (start_x + i) < VISIBLE_SCREEN_WIDTH)
		{
			draw_tile_pixel(pix, color, back_pen, dest);

			// priority marking
			if (pix)
				line_priority[start_x + i] |= 0x02;
		}
		dest++;
	}
}


// m_refresh_data is important as it is updated during rendering, and overwritten when you write new scroll values
// making raster effects more complex than on other systems
// https://retrocomputing.stackexchange.com/questions/1898/how-can-i-create-a-split-scroll-effect-in-an-nes-game

void ppu2c0x_device::draw_background(uint8_t* line_priority)
{
	bitmap_rgb32& bitmap = *m_bitmap;

	/* determine where in the nametable to start drawing from */
	/* based on the current scanline and scroll regs */
	uint8_t  scroll_x_coarse = m_refresh_data & 0x001f;
	uint8_t  scroll_y_coarse = (m_refresh_data & 0x03e0) >> 5;
	uint16_t nametable = m_refresh_data & 0x0c00;
	uint8_t  scroll_y_fine = (m_refresh_data & 0x7000) >> 12;

	int x = scroll_x_coarse;

	/* get the tile index */
	int tile_index = (nametable | 0x2000) + scroll_y_coarse * 32;

	/* set up dest */
	int start_x = (m_x_fine ^ 0x07) - 7;
	uint32_t* dest = &bitmap.pix(m_scanline, start_x);

	m_tilecount = 0;

	/* draw the 32 or 33 tiles that make up a line */
	while (m_tilecount < 34)
	{
		int color_byte;
		int color_bits;
		int pos;
		int index1;
		int page, page2, address;

		index1 = tile_index + x;

		// page2 is the output of the nametable read (this section is the FIRST read per tile!)
		page2 = readbyte(index1);

		// this is attribute table stuff! (actually read 2 in PPUspeak)!
		/* Figure out which byte in the color table to use */
		pos = ((index1 & 0x380) >> 4) | ((index1 & 0x1f) >> 2);
		page = (index1 & 0x0c00) >> 10;
		address = 0x3c0 + pos;
		color_byte = readbyte((((page * 0x400) + address) & 0xfff) + 0x2000);

		/* figure out which bits in the color table to use */
		color_bits = ((index1 & 0x40) >> 4) + (index1 & 0x02);

		// 27/12/2002
		if (!m_latch.isnull())
			m_latch((m_tile_page << 10) | (page2 << 4));

		if (start_x < VISIBLE_SCREEN_WIDTH)
		{
			// need to read 0x0000 or 0x1000 + 16*nametable data
			address = ((m_tile_page) ? 0x1000 : 0) + (page2 * 16);
			// plus something that accounts for y
			address += scroll_y_fine;

			draw_tile(line_priority, color_byte, color_bits, address, start_x, m_back_color, dest);

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
		dest = &bitmap.pix(m_scanline);
		for (int i = 0; i < 8; i++)
		{
			draw_back_pen(dest, m_back_color);
			dest++;

			line_priority[i] ^= 0x02;
		}
	}
}

void ppu2c04_clone_device::draw_background(uint8_t* line_priority)
{
	// nametable selection is ignored below the hardwired scroll split position
	if (m_scanline < 31)
		m_refresh_data &= ~0x0c00;

	ppu2c0x_device::draw_background(line_priority);
}

void ppu2c0x_device::draw_back_pen(uint32_t* dest, int back_pen)
{
	*dest = m_nespens[apply_grayscale_and_emphasis(back_pen)];
}

void ppu2c0x_device::draw_background_pen()
{
	bitmap_rgb32& bitmap = *m_bitmap;

	// Fill this scanline with the background pen.
	for (int i = 0; i < bitmap.width(); i++)
		draw_back_pen(&bitmap.pix(m_scanline, i), m_back_color);
}

void ppu2c0x_device::read_sprite_plane_data(int address)
{
	m_planebuf[0] = readbyte(address & 0x1fff);
	m_planebuf[1] = readbyte((address + 8) & 0x1fff);
}

void ppu2c0x_device::make_sprite_pixel_data(uint8_t& pixel_data, int flipx)
{
	if (flipx)
	{
		pixel_data = (m_planebuf[0] & 1) | ((m_planebuf[1] & 1) << 1);
		m_planebuf[0] >>= 1;
		m_planebuf[1] >>= 1;
	}
	else
	{
		pixel_data = BIT(m_planebuf[0], 7) | (BIT(m_planebuf[1], 7) << 1);
		m_planebuf[0] <<= 1;
		m_planebuf[1] <<= 1;
	}
}

void ppu2c0x_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32& bitmap)
{
	uint16_t palval = m_palette_ram[((4 * color) | pixel_data) & 0x1f];
	uint32_t pix = m_nespens[apply_grayscale_and_emphasis(palval)];

	bitmap.pix(m_scanline, sprite_xpos + pixel) = pix;
}

void ppu2c04_clone_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32 &bitmap)
{
	/* clone PPU clips sprites at the screen edges */
	if ((sprite_xpos + pixel < 8) || (sprite_xpos + pixel) >= (VISIBLE_SCREEN_WIDTH - 6))
		return;

	uint16_t palval = m_palette_ram[((4 * color) | pixel_data) & 0x1f];
	uint32_t pix = m_nespens[palval | 0x40];
	bitmap.pix(m_scanline, sprite_xpos + pixel) = pix;
}

void ppu2c0x_device::read_extra_sprite_bits(int sprite_index)
{
	// needed for some clones
}

bool ppu2c0x_device::is_spritepixel_opaque(int pixel_data, int color)
{
	if (pixel_data)
		return true;
	else
		return false;
}

void ppu2c0x_device::draw_sprite_pixel_low(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, uint8_t* line_priority)
{
	if (is_spritepixel_opaque(pixel_data, color))
	{
		/* has the background (or another sprite) already been drawn here? */
		if ((sprite_xpos + pixel) < VISIBLE_SCREEN_WIDTH)
		{
			if (!line_priority[sprite_xpos + pixel])
			{
				/* no, draw */
				draw_sprite_pixel(sprite_xpos, color, pixel, pixel_data, bitmap);
			}
			/* indicate that a sprite was drawn at this location, even if it's not seen */
			line_priority[sprite_xpos + pixel] |= 0x01;
		}
	}

	/* set the "sprite 0 hit" flag if appropriate */
	if (sprite_index == 0 && (pixel_data & 0x03) && ((sprite_xpos + pixel) < 255) && (line_priority[sprite_xpos + pixel] & 0x02))
		m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
}

void ppu2c0x_device::draw_sprite_pixel_high(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, uint8_t* line_priority)
{
	if (is_spritepixel_opaque(pixel_data, color))
	{
		if ((sprite_xpos + pixel) < VISIBLE_SCREEN_WIDTH)
		{
			/* has another sprite been drawn here? */
			if (!(line_priority[sprite_xpos + pixel] & 0x01))
			{
				/* no, draw */
				draw_sprite_pixel(sprite_xpos, color, pixel, pixel_data, bitmap);
				line_priority[sprite_xpos + pixel] |= 0x01;
			}
		}
	}

	/* set the "sprite 0 hit" flag if appropriate */
	if (sprite_index == 0 && (pixel_data & 0x03) && ((sprite_xpos + pixel) < 255) && (line_priority[sprite_xpos + pixel] & 0x02))
		m_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
}

int ppu2c0x_device::apply_sprite_pattern_page(int index1, int size)
{
	if (size == 8)
		index1 += (m_sprite_page == 0) ? 0 : 0x1000;

	return index1;
}

void ppu2c0x_device::draw_sprites(uint8_t* line_priority)
{
	bitmap_rgb32& bitmap = *m_bitmap;

	int sprite_xpos, sprite_ypos, sprite_index;
	int tile, index1;
	int pri;

	int flipx, flipy, color;
	int size;
	int sprite_count = 0;
	int sprite_line;

	int first_pixel;
	int pixel;

	/* determine if the sprites are 8x8 or 8x16 */
	size = (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;

	first_pixel = (m_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES_L8) ? 0 : 8;

	for (sprite_index = 0; sprite_index < SPRITERAM_SIZE; sprite_index += 4)
	{
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

		tile = m_spriteram[sprite_index + 1];
		color = (m_spriteram[sprite_index + 2] & 0x03) + 4;
		pri = m_spriteram[sprite_index + 2] & 0x20;
		flipx = m_spriteram[sprite_index + 2] & 0x40;
		flipy = m_spriteram[sprite_index + 2] & 0x80;
		read_extra_sprite_bits(sprite_index);

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

		if (size == 16 && sprite_line > 7)
		{
			tile++;
			sprite_line -= 8;
		}

		index1 = tile * 16;

		index1 = apply_sprite_pattern_page(index1, size);

		read_sprite_plane_data(index1 + sprite_line);

		/* if there are more than 8 sprites on this line, set the flag */
		if (sprite_count == 8)
		{
			m_regs[PPU_STATUS] |= PPU_STATUS_8SPRITES;
			//logerror ("> 8 sprites, scanline: %d\n", m_scanline);

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
				uint8_t pixel_data;
				make_sprite_pixel_data(pixel_data, flipx);

				/* is this pixel non-transparent? */
				if (sprite_xpos + pixel >= first_pixel)
				{
					draw_sprite_pixel_low(bitmap, pixel_data, pixel, sprite_xpos, color, sprite_index, line_priority);
				}
			}
		}
		else
		{
			/* draw the high-priority sprites */
			for (pixel = 0; pixel < 8; pixel++)
			{
				uint8_t pixel_data;
				make_sprite_pixel_data(pixel_data, flipx);

				/* is this pixel non-transparent? */
				if (sprite_xpos + pixel >= first_pixel)
				{
					draw_sprite_pixel_high(bitmap, pixel_data, pixel, sprite_xpos, color, sprite_index, line_priority);
				}
			}
		}
	}
}

void ppu2c04_clone_device::draw_sprites(uint8_t *line_priority)
{
	ppu2c0x_device::draw_sprites(line_priority);

	if (m_scanline == BOTTOM_VISIBLE_SCANLINE)
	{
		/* this frame's sprite buffer is cleared after being displayed
		and the other one that was filled this frame will be displayed next frame */
		m_spriteram.swap(m_spritebuf);
		memset(m_spritebuf.get(), 0, SPRITERAM_SIZE);
	}
}

/*************************************
 *
 *  Scanline Rendering and Update
 *
 *************************************/

void ppu2c0x_device::render_scanline()
{
	uint8_t line_priority[VISIBLE_SCREEN_WIDTH];

	/* lets see how long it takes */
	g_profiler.start(PROFILER_USER1);

	/* clear the line priority for this scanline */
	memset(line_priority, 0, VISIBLE_SCREEN_WIDTH);

	m_draw_phase = PPU_DRAW_BG;

	/* see if we need to render the background */
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND)
	{
		draw_background(line_priority);
	}
	else
	{
		draw_background_pen();
	}

	m_draw_phase = PPU_DRAW_OAM;

	/* if sprites are on, draw them, but we call always to process them */
	draw_sprites(line_priority);

	m_draw_phase = PPU_DRAW_BG;

	/* done updating, whew */
	g_profiler.stop();
}

void ppu2c0x_device::scanline_increment_fine_ycounter()
{
	/* increment the fine y-scroll */
	m_refresh_data += 0x1000;

	/* if it's rolled, increment the coarse y-scroll */
	if (m_refresh_data & 0x8000)
	{
		uint16_t tmp;
		tmp = (m_refresh_data & 0x03e0) + 0x20;
		m_refresh_data &= 0x7c1f;

		/* handle bizarro scrolling rollover at the 30th (not 32nd) vertical tile */
		if (tmp == 0x03c0)
			m_refresh_data ^= 0x0800;
		else
			m_refresh_data |= tmp & 0x03e0;

		//logerror("updating refresh_data: %04x\n", m_refresh_data);
	}
}

void ppu2c0x_device::update_visible_enabled_scanline()
{
	if (m_scanline_timer->remaining() == attotime::zero)
	{
		/* If background or sprites are enabled, copy the ppu address latch */
		/* Copy only the scroll x-coarse and the x-overflow bit */
		m_refresh_data &= ~0x041f;
		m_refresh_data |= m_refresh_latch & 0x041f;
	}

	//logerror("updating refresh_data: %04x (scanline: %d)\n", m_refresh_data, m_scanline);
	render_scanline();
}

void ppu2c0x_device::update_visible_disabled_scanline()
{
	bitmap_rgb32& bitmap = *m_bitmap;
	uint32_t back_pen = m_back_color;

	if (m_paletteram_in_ppuspace)
	{
		/* cache the background pen */
		if (m_videomem_addr >= 0x3f00)
		{
			// If the PPU's VRAM address happens to point into palette ram space while
			// both the sprites and background are disabled, the PPU paints the scanline
			// with the palette entry at the VRAM address instead of the usual background
			// pen. Micro Machines makes use of this feature.
			// Note, the PPU DOES access normally unused background palette colors.
			back_pen = m_palette_ram[m_videomem_addr & 0x1f];
		}
	}

	// Fill this scanline with the background pen.
	for (int i = 0; i < bitmap.width(); i++)
		draw_back_pen(&bitmap.pix(m_scanline, i), back_pen);
}

void ppu2c0x_device::update_visible_scanline()
{
	/* Render this scanline if appropriate */
	if (m_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES))
	{
		update_visible_enabled_scanline();
	}
	else
	{
		update_visible_disabled_scanline();
	}

	if (m_scanline_timer->remaining() == attotime::zero)
	{
		scanline_increment_fine_ycounter();
	}
}

void ppu2c0x_device::update_scanline()
{
	if (m_scanline <= BOTTOM_VISIBLE_SCANLINE)
	{
		update_visible_scanline();
	}
}

/*************************************
*
*   PPU Memory functions
*
*************************************/

void ppu2c0x_device::palette_write(offs_t offset, uint8_t data)
{
	// palette RAM is only 6 bits wide
	data &= 0x3f;

	if (offset & 0x3)
	{
		// regular pens, no mirroring
		m_palette_ram[offset & 0x1f] = data;
	}
	else
	{
		// transparent pens are mirrored!
		if (0 == (offset & 0xf))
		{
			m_back_color = data;
		}
		m_palette_ram[offset & 0xf] = m_palette_ram[(offset & 0xf) + 0x10] = data;
	}
}

uint8_t ppu2c0x_device::palette_read(offs_t offset)
{
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
		return m_palette_ram[offset & 0x1f] & 0x30;
	else
		return m_palette_ram[offset & 0x1f];
}

/*************************************
 *
 *  PPU Registers Read
 *
 *************************************/

uint8_t ppu2c0x_device::read(offs_t offset)
{
	if (offset >= PPU_MAX_REG)
	{
		logerror("PPU %s: Attempting to read past the chip: offset %x\n", this->tag(), offset);
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
			m_latch(m_videomem_addr & 0x3fff);

		if ((m_videomem_addr >= 0x3f00) && m_paletteram_in_ppuspace)
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

uint8_t ppu2c04_clone_device::read(offs_t offset)
{
	switch (offset & 7)
	{
	case PPU_STATUS: /* 2 */
		// $2002 on this clone only contains the sprite 0 hit flag,
		// and it's hardwired to trigger after a specific scanline, not based on actual sprite positions
		if (m_scanline < 31 || m_scanline >= (m_vblank_first_scanline - 1))
			return ~PPU_STATUS_SPRITE0_HIT;
		return 0xff;

	case PPU_SPRITE_DATA: /* 4 */
		return m_spritebuf[m_regs[PPU_SPRITE_ADDRESS]];
	}

	return ppu2c0x_device::read(offset);
}

/*************************************
 *
 *  PPU Registers Write
 *
 *************************************/

void ppu2c0x_device::write(offs_t offset, uint8_t data)
{
	if (offset >= PPU_MAX_REG)
	{
		logerror("PPU %s: Attempting to write past the chip: offset %x, data %x\n", this->tag(), offset, data);
		offset &= PPU_MAX_REG - 1;
	}

#ifdef MAME_DEBUG
	if (m_scanline <= BOTTOM_VISIBLE_SCANLINE)
	{
		logerror("PPU register %d write %02x during non-vblank scanline %d (MAME %d, beam pos: %d)\n", offset, data, m_scanline, screen().vpos(), screen().hpos());
	}
#endif

	/* on the RC2C05, PPU_CONTROL0 and PPU_CONTROL1 are swapped (protection) */
	if (m_security_value && !(offset & 6))
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

		m_add = (data & PPU_CONTROL0_INC) ? m_line_write_increment_large : 1;
		//logerror("control0 write: %02x (scanline: %d)\n", data, m_scanline);
		break;

	case PPU_CONTROL1: /* 1 */
		//logerror("control1 write: %02x (scanline: %d)\n", data, m_scanline);
		m_regs[PPU_CONTROL1] = data;
		break;

	case PPU_SPRITE_ADDRESS: /* 3 */
		m_regs[PPU_SPRITE_ADDRESS] = data;
		break;

	case PPU_SPRITE_DATA: /* 4 */
		// If the PPU is currently rendering the screen, 0xff is written instead of the desired data.
		if (m_use_sprite_write_limitation)
			if (m_scanline <= BOTTOM_VISIBLE_SCANLINE)
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
			//logerror("scroll write 2: %d, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
		}
		else
		{
			/* first write */
			m_refresh_latch &= (0xffe0 & m_global_refresh_mask);
			m_refresh_latch |= (data & 0xf8) >> 3;

			m_x_fine = data & 7;
			//logerror("scroll write 1: %d, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
		}

		m_toggle ^= 1;
		break;

	case PPU_ADDRESS: /* 6 */
		if (m_toggle)
		{
			/* second write */
			m_refresh_latch &= (0xff00 & m_global_refresh_mask);
			m_refresh_latch |= data;
			m_refresh_data = m_refresh_latch;

			m_videomem_addr = m_refresh_latch;
			//logerror("vram addr write 2: %02x, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
		}
		else
		{
			/* first write */
			m_refresh_latch &= 0x00ff;
			m_refresh_latch |= (data & (m_videoram_addr_mask >> 8)) << 8;
			//logerror("vram addr write 1: %02x, %04x (scanline: %d)\n", data, m_refresh_latch, m_scanline);
		}

		m_toggle ^= 1;
		break;

	case PPU_DATA: /* 7 */
	{
		int tempAddr = m_videomem_addr & m_videoram_addr_mask;

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
		else // this codepath is identical?
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

void ppu2c04_clone_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
	case PPU_CONTROL0: /* 0 */
		data &= 0x01 | PPU_CONTROL0_INC | PPU_CONTROL0_NMI; /* other bits of $2000 are ignored by this clone */
		data |= PPU_CONTROL0_CHR_SELECT;
		break;

	case PPU_CONTROL1: /* 1 */
	case PPU_SPRITE_ADDRESS: /* 3 */
		return; /* $2001 and $2003 do nothing on this clone */

	case PPU_SPRITE_DATA: /* 4 */
		m_spritebuf[m_regs[PPU_SPRITE_ADDRESS]] = data;
		m_regs[PPU_SPRITE_ADDRESS] = (m_regs[PPU_SPRITE_ADDRESS] + 1) & 0xff;
		return;

	case PPU_SCROLL: /* 5 */
		if (m_toggle)
			data = 0; /* no vertical scroll */
		break;

	case PPU_ADDRESS: /* 6 */
		/* $2006 doesn't affect scroll latching */
		if (m_toggle)
		{
			/* second write */
			set_vram_dest((get_vram_dest() & 0xff00) | data);
		}
		else
		{
			/* first write */
			set_vram_dest((get_vram_dest() & 0x00ff) | (data << 8));
		}

		m_toggle ^= 1;
		return;
	}

	ppu2c0x_device::write(offset, data);
}

uint16_t ppu2c0x_device::get_vram_dest()
{
	return m_videomem_addr;
}

void ppu2c0x_device::set_vram_dest(uint16_t dest)
{
	m_videomem_addr = dest;
}

/*************************************
 *
 *  Sprite DMA
 *
 *************************************/

void ppu2c0x_device::spriteram_dma(address_space& space, const uint8_t page)
{
	int address = page << 8;

	for (int i = 0; i < SPRITERAM_SIZE; i++)
	{
		uint8_t spriteData = space.read_byte(address + i);
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

void ppu2c0x_device::render(bitmap_rgb32& bitmap, int flipx, int flipy, int sx, int sy, const rectangle& cliprect)
{
	if (m_scanline_timer->remaining() != attotime::zero)
	{
		// Partial line update, need to render first (especially for light gun emulation).
		update_scanline();
	}
	copybitmap(bitmap, *m_bitmap, flipx, flipy, sx, sy, cliprect);
}

uint32_t ppu2c0x_device::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	render(bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
