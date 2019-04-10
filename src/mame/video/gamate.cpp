// license:BSD-3-Clause
// copyright-holders:David Haywood, Peter Wilhelmsen, Kevtris

/*
    Notes:

    Some games are glitchy, most of these glitches are verified to happen on hardware
    for example

    Badly flipped sprites in Tornado and Insect War
    Heavy flickering sprites in many games

    Most of these issues are difficult to notice on real hardware due to the poor
    quality display.

    Thanks to Kevtris for the documentation on which this implementation is based
    (some comments taken directly from this)
    http://blog.kevtris.org/blogfiles/Gamate%20Inside.txt

    ToDo:

    Emulate vram pull / LCD refresh timings more accurately.
    Interrupt should maybe be in here, not in drivers/gamate.cpp?
    Verify both Window modes act the same as hardware.
*/

#include "emu.h"
#include "video/gamate.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(GAMATE_VIDEO, gamate_video_device, "gamate_vid", "Gamate Video Hardware")

void gamate_video_device::regs_map(address_map &map)
{
	map(0x01, 0x01).w(FUNC(gamate_video_device::lcdcon_w));
	map(0x02, 0x02).w(FUNC(gamate_video_device::xscroll_w));
	map(0x03, 0x03).w(FUNC(gamate_video_device::yscroll_w));
	map(0x04, 0x04).w(FUNC(gamate_video_device::xpos_w));
	map(0x05, 0x05).w(FUNC(gamate_video_device::ypos_w));
	map(0x06, 0x06).r(FUNC(gamate_video_device::vram_r));
	map(0x07, 0x07).w(FUNC(gamate_video_device::vram_w));
}

void gamate_video_device::vram_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("vram"); // 2x 8KB SRAMs
}

gamate_video_device::gamate_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GAMATE_VIDEO, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_vram_space_config("vramspace", ENDIANNESS_BIG, 8, 14, 0, address_map_constructor(FUNC(gamate_video_device::vram_map), this)),
	m_vram(*this, "vram"),
	m_vramaddress(0),
	m_bitplaneselect(0),
	m_scrollx(0),
	m_scrolly(0),
	m_window(0),
	m_swapplanes(0),
	m_incrementdir(0),
	m_displayblank(0)
{
}

void gamate_video_device::set_vram_addr_lower_5bits(uint8_t data)
{
	m_vramaddress = (m_vramaddress & 0x3fe0) | (data & 0x1f);
}

void gamate_video_device::set_vram_addr_upper_8bits(uint8_t data)
{
	m_vramaddress = (m_vramaddress & 0x001f) | (data << 5);
}

void gamate_video_device::increment_vram_address()
{
	if (m_incrementdir)
		m_vramaddress += 0x20;
	else
		m_vramaddress++;

	m_vramaddress &= 0x1fff;
}

WRITE8_MEMBER(gamate_video_device::lcdcon_w)
{
	/*
	NXWS ???E
	E: When set, stops the LCD controller from refreshing the LCD.  This can
	   damage the LCD material because the invert signal is no longer toggling,
	   and the pixel/frame/row clocks/pulses are not being output.
	S: Swap plane bits.  When set, flip bit planes 0 and 1.
	W: D0-DF is mapped in at rows 00-0Fh at the top of the screen, with no
	   X scroll for those rows. (see window bit info below)
	X: When clear the video address increments by 1. When set, it increments
	   by 32.
	N: When set, clears the LCD by blanking the data.  The LCD refresh still occurs.
	*/
	m_displayblank = (data & 0x80);
	m_incrementdir = (data & 0x40);
	m_window = (data & 0x20);
	m_swapplanes = (data & 0x10);
	// setting data & 0x01 is bad
}

WRITE8_MEMBER(gamate_video_device::xscroll_w)
{
	/*
	XXXX XXXX
	X: 8 bit Xscroll value
	*/
	m_scrollx = data;
}

WRITE8_MEMBER(gamate_video_device::yscroll_w)
{
	/*
	YYYY YYYY
	Y: 8 bit Yscroll value
	*/
	m_scrolly = data;
}

WRITE8_MEMBER(gamate_video_device::xpos_w)
{
	/*
	BxxX XXXX
	B: Bitplane. 0 = lower (bitplane 0), 1 = upper (bitplane 1)
	X: 5 lower bits of the 13 bit VRAM address.
	*/
	m_bitplaneselect = (data & 0x80) >> 7;
	set_vram_addr_lower_5bits(data & 0x1f);
}

WRITE8_MEMBER(gamate_video_device::ypos_w)
{
	/*
	YYYY YYYY
	Y: 8 upper bits of 13 bit VRAM address.
	*/
	set_vram_addr_upper_8bits(data);
}

READ8_MEMBER(gamate_video_device::vram_r)
{
	uint16_t address = m_vramaddress << 1;

	if (m_bitplaneselect)
		address += 1;

	uint8_t ret = m_vramspace->read_byte(address);

	increment_vram_address();

	return ret;
}

WRITE8_MEMBER(gamate_video_device::vram_w)
{
	uint16_t address = m_vramaddress << 1;

	if (m_bitplaneselect)
		address += 1;

	m_vramspace->write_byte(address, data);

	increment_vram_address();
}

void gamate_video_device::get_real_x_and_y(int &ret_x, int &ret_y, int scanline)
{
	/* the Gamate video has 2 'Window' modes,
	   Mode 1 is enabled with an actual register
	   Mode 2 is enabled automatically based on the yscroll value

	   both modes seem designed to allow for a non-scrolling status bar at
	   the top of the display.
	*/

	if (m_scrolly < 0xc8)
	{
		ret_y = scanline + m_scrolly;

		if (ret_y >= 0xc8)
			ret_y -= 0xc8;

		ret_x = m_scrollx;

		if (m_window) /* Mode 1 Window */
		{
			if (scanline < 0x10)
			{
				ret_x = 0;
				ret_y = 0xd0 + scanline;
			}
		}
	}
	else /* Mode 2, do any games use this ? does above Window logic override this if enabled? */
	{
		ret_x = m_scrollx;

		/*
		    Using Yscroll values of C8-CF, D8-DF, E8-EF, and F8-FF will result in the same
		    effect as if a Yscroll value of 00h were used.
		*/
		if (m_scrolly & 0x08) // values of C8-CF, D8-DF, E8-EF, and F8-FF
		{
			ret_y = 0x00;
			ret_x = m_scrollx;
		}
		else
		{
			/*
			    Values D0-D7, E0-E7, and F0-F7 all produce a bit more useful effect.  The upper
			    1-8 scanlines will be pulled from rows F8-FFh in VRAM (i.e. 1F00h = row F8h).

			    If F0 is selected, then the upper 8 rows will be the last 8 rows in VRAM-
			    1F00-1FFFh area.  If F1 is selected, the upper 8 rows will be the last 7 rows
			    in VRAM and so on.  This special window area DOES NOT SCROLL with X making it
			    useful for status bars.  I don't think any games actually used it, though.
			*/
			int fixedscanlines = m_scrolly & 0x7;

			if (scanline <= fixedscanlines)
			{
				ret_x = 0;
				ret_y = 0xf8 + scanline + (7-fixedscanlines);
			}
			else
			{
				// no yscroll in this mode?
				ret_x = m_scrollx;
				ret_y = scanline;// +m_scrolly;

				//if (ret_y >= 0xc8)
				//  ret_y -= 0xc8;
			}

		}
	}
}

int gamate_video_device::get_pixel_from_vram(int x, int y)
{
	x &= 0xff;
	y &= 0xff;

	int x_byte = x >> 3;
	x &= 0x7; // x pixel;

	int address = ((y * 0x20) + x_byte) << 1;

	int plane0 = (m_vram[address] >> (7-x)) & 0x1;
	int plane1 = (m_vram[address + 1] >> (7-x)) & 0x1;

	if (!m_swapplanes)
		return plane0 | (plane1 << 1);
	else
		return plane1 | (plane0 << 1); // does any game use this?
}

uint32_t gamate_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		//printf("updating scanline %d\n", y);
		int real_x, real_y;
		get_real_x_and_y(real_x, real_y, y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			int pix = get_pixel_from_vram(x + real_x, real_y);

			if (m_displayblank)
				pix = 0;

			bitmap.pix16(y, x) = pix;
		}
	}

	return 0;
}

device_memory_interface::space_config_vector gamate_video_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_vram_space_config),
	};
}

// this palette is taken from megaduck, from videos it looks similar
static const unsigned char palette_gamate[] = {
	0x6B, 0xA6, 0x4A, 0x43, 0x7A, 0x63, 0x25, 0x59, 0x55, 0x12, 0x42, 0x4C
};

void gamate_video_device::gamate_palette(palette_device &palette) const
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_gamate[i * 3 + 0], palette_gamate[i * 3 + 1], palette_gamate[i * 3 + 2]);
}

/*
    Of the 150 scanlines emitted, all contain pixel data pulled from RAM. There are
    exactly 72900 clocks per frame, so at the nominal 4.433MHz rate, this means the
    frame rate is 60.8093Hz.
*/

void gamate_video_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60.8093);
	screen.set_size(160, 150);
	screen.set_visarea(0, 160-1, 0, 150-1);
	screen.set_screen_update(FUNC(gamate_video_device::screen_update));
	screen.set_palette("palette");
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE); // close approximate until we use timers to emulate exact video update
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));

	PALETTE(config, "palette", FUNC(gamate_video_device::gamate_palette), 4);
}

void gamate_video_device::device_start()
{
	m_vramspace = &space(0);

	save_item(NAME(m_vramaddress));
	save_item(NAME(m_bitplaneselect));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_window));
	save_item(NAME(m_swapplanes));
	save_item(NAME(m_incrementdir));
	save_item(NAME(m_displayblank));
}

void gamate_video_device::device_reset()
{
	m_vramaddress = 0;
	m_bitplaneselect = 0;
	m_scrollx = 0;
	m_scrolly = 10;
	m_window = 0;
	m_swapplanes = 0;
	m_incrementdir = 0;
	m_displayblank = 0;
}
