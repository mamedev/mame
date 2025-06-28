// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*
Konami 051960/051937
-------------
Sprite generators. Designed to work in pair. The 051960 manages the sprite
list and produces an address that is fed to the gfx ROMs. The data from the
ROMs is sent to the 051937, along with color code and other stuff from the
051960. The 051937 outputs up to 12 bits of palette index, plus "shadow" and
transparency information.
Both chips are interfaced to the main CPU, through 8-bit data buses and 11
bits of address space. The 051937 sits in the range 000-007, while the 051960
in the range 400-7ff (all RAM). The main CPU can read the gfx ROM data though
the 051937 data bus, while the 051960 provides the address lines.
The 051960 is designed to directly address 1MB of ROM space, since it produces
18 address lines that go to two 16-bit wide ROMs (the 051937 has a 32-bit data
bus to the ROMs). However, the addressing space can be increased by using one
or more of the "color attribute" bits of the sprites as bank selectors.
Moreover a few games store the gfx data in the ROMs in a format different from
the one expected by the 051960, and use external logic to reorder the address
and/or data lines.
The 051960 can also generate IRQ, FIRQ and NMI signals.

memory map:
000-007 is for the 051937, but also seen by the 051960
400-7ff is 051960 only
000     R  bit 0 = busy flag for sprite dma (does not toggle if bit 4 is set)
                   aliens waits for it to be 0 before starting to copy sprite data
                   thndrx2 needs it to pulse for the startup checks to succeed
000     W  bit 0 = irq enable/acknowledge
           bit 1 = firq enable/acknowledge
           bit 2 = nmi enable/acknowledge
           bit 3 = flip screen (applies to sprites only, not tilemaps)
           bit 4 = disable internal sprite processing
                   used by Devastators, TMNT, Aliens, Chequered Flag, maybe others
                   aliens sets it just after checking bit 0, and before copying
                   the sprite data
           bit 5 = enable gfx ROM reading
           bit 6 = let cpu address bits 2~5 pass through CA0~3 when bit 5 is set
001     W  bit 0 = invert shadow for all pens
           bit 1 = force shadows for pen 0x0f
           bit 2 = disable shadows for pen 0x0f (priority over bit 1)
           Devastators and MIA set bit 1.
           Ultraman sets the register to 0x0f.
           Chequered Flag sets bit 0 when shadows should be highlights.
           Both Ultraman and Chequered Flag disagree with inverting shadow for
           all pens, so this flag is not emulated (the way described above).
002-003 W  selects the portion of the gfx ROMs to be read.
004     W  bit 0 = OC6 when gfx ROM reading is enabled
           bit 1 = OC7 when gfx ROM reading is enabled
           Aliens uses this to select the ROM bank to be read, but Punk Shot
           and TMNT don't, they use another bit of the registers above. Many
           other games write to this register before testing.
           Bits 2-7 of 003 go to OC0-OC5.
004-007 R  reads data from the gfx ROMs (32 bits in total). The address of the
           data is determined by the register above and by the last address
           accessed on the 051960; plus bank switch bits for larger ROMs.
           It seems that the data can also be read directly from the 051960
           address space: 88 Games does this. First it reads 004 and discards
           the result, then it reads from the 051960 the data at the address
           it wants. The normal order is the opposite, read from the 051960 at
           the address you want, discard the result, and fetch the data from
           004-007.
400-7ff RW sprite RAM, 8 bytes per sprite

*/

#include "emu.h"
#include "k051960.h"

#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(K051960, k051960_device, "k051960", "K051960 Sprite Generator")

const gfx_layout k051960_device::spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 8, 16, 24 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*32+0, 8*32+1, 8*32+2, 8*32+3, 8*32+4, 8*32+5, 8*32+6, 8*32+7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8
};

// cuebrick, mia and tmnt connect the lower four output lines from the K051937
// (i.e. the ones outputting ROM data rather than attribute data) to the mixer
// in reverse order.
const gfx_layout k051960_device::spritelayout_reverse =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24, 16, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*32+0, 8*32+1, 8*32+2, 8*32+3, 8*32+4, 8*32+5, 8*32+6, 8*32+7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8
};

// In gradius3, the gfx ROMs are directly connected to one of the 68K CPUs
// rather than being read the usual way; moreover, the ROM data lines are
// connected in different ways to the 68K and to the K051937.
// Rather than copy the ROM region and bitswap one copy, we (currently)
// just use an alternate gfx layout for this game.
const gfx_layout k051960_device::spritelayout_gradius3 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
		32*8+2*4, 32*8+3*4, 32*8+0*4, 32*8+1*4, 32*8+6*4, 32*8+7*4, 32*8+4*4, 32*8+5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		64*8+0*32, 64*8+1*32, 64*8+2*32, 64*8+3*32, 64*8+4*32, 64*8+5*32, 64*8+6*32, 64*8+7*32 },
	128*8
};

GFXDECODE_MEMBER( k051960_device::gfxinfo )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, spritelayout, 0, 1)
GFXDECODE_END

GFXDECODE_MEMBER( k051960_device::gfxinfo_reverse )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, spritelayout_reverse, 0, 1)
GFXDECODE_END

GFXDECODE_MEMBER( k051960_device::gfxinfo_gradius3 )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, spritelayout_gradius3, 0, 1)
GFXDECODE_END


k051960_device::k051960_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, K051960, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo)
	, device_video_interface(mconfig, *this)
	, m_sprite_rom(*this, DEVICE_SELF)
	, m_scanline_timer(nullptr)
	, m_k051960_cb(*this)
	, m_shadow_config_cb(*this)
	, m_irq_handler(*this)
	, m_firq_handler(*this)
	, m_nmi_handler(*this)
	, m_romoffset(0)
	, m_control(0)
	, m_nmi_count(0)
	, m_shadow_config(0)
{
}

void k051960_device::set_plane_order(int order)
{
	switch (order)
	{
		case K051960_PLANEORDER_BASE:
			set_info(gfxinfo);
			break;

		case K051960_PLANEORDER_MIA:
			set_info(gfxinfo_reverse);
			break;

		case K051960_PLANEORDER_GRADIUS3:
			set_info(gfxinfo_gradius3);
			break;

		default:
			fatalerror("Unknown plane_order\n");
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k051960_device::device_start()
{
	// assumes it can make an address mask with m_sprite_rom.length() - 1
	assert(!(m_sprite_rom.length() & (m_sprite_rom.length() - 1)));

	// make sure our screen is started
	if (!screen().started())
		throw device_missing_dependencies();
	if (!palette().device().started())
		throw device_missing_dependencies();

	// bind callbacks
	m_k051960_cb.resolve();

	// allocate scanline timer and start at first scanline
	m_scanline_timer = timer_alloc(FUNC(k051960_device::scanline_callback), this);
	m_scanline_timer->adjust(screen().time_until_pos(0), 0);

	m_sprites_busy = timer_alloc(timer_expired_delegate());

	decode_gfx();
	gfx(0)->set_colors(palette().entries() / gfx(0)->depth());

	if (VERBOSE && !(palette().shadows_enabled()))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	memset(m_ram, 0, sizeof(m_ram));
	memset(m_buffer, 0, sizeof(m_buffer));

	// register for save states
	save_item(NAME(m_romoffset));
	save_item(NAME(m_control));
	save_item(NAME(m_nmi_count));
	save_item(NAME(m_shadow_config));
	save_item(NAME(m_spriterombank));
	save_item(NAME(m_ram));
	save_item(NAME(m_buffer));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k051960_device::device_reset()
{
	for (int i = 0; i < 5; i++)
		k051937_w(i, 0);

	m_romoffset = 0;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

TIMER_CALLBACK_MEMBER(k051960_device::scanline_callback)
{
	int scanline = param;

	// NMI every 32 scanlines (not on 16V)
	if ((++m_nmi_count & 3) == 3 && BIT(m_control, 2))
		m_nmi_handler(ASSERT_LINE);

	// FIRQ is when?

	// vblank
	if (scanline == 240)
	{
		if (BIT(m_control, 0))
			m_irq_handler(ASSERT_LINE);

		// do the sprite dma, unless sprite processing was disabled
		if (!BIT(m_control, 4))
		{
			memcpy(m_buffer, m_ram, sizeof(m_buffer));

			// count number of active sprites in the buffer
			int active = 0;
			for (int i = 0; i < sizeof(m_buffer); i += 8)
				active += BIT(m_buffer[i], 7);

			// 32 clocks per active sprite, around 18 clocks per inactive sprite
			const u32 ticks = (active * 32) + (128 - active) * 18;
			m_sprites_busy->adjust(attotime::from_ticks(ticks * 4, clock()));
		}
	}

	// wait for next line
	scanline += 8;
	if (scanline >= screen().height())
		scanline = 0;

	m_scanline_timer->adjust(screen().time_until_pos(scanline), scanline);
}

u8 k051960_device::k051960_fetchromdata(offs_t offset)
{
	int code, color, pri, off1, addr;
	bool shadow;

	addr = m_romoffset + (m_spriterombank[0] << 8) + ((m_spriterombank[1] & 0x03) << 16);
	code = (addr & 0x3ffe0) >> 5;
	off1 = addr & 0x1f;
	color = ((m_spriterombank[1] & 0xfc) >> 2) + ((m_spriterombank[2] & 0x03) << 6);
	pri = 0;
	shadow = false;
	m_k051960_cb(&code, &color, &pri, &shadow);

	addr = (code << 7) | (off1 << 2) | offset;
	addr &= m_sprite_rom.length() - 1;

	//popmessage("%s: addr %06x", machine().describe_context(), addr);
	return m_sprite_rom[addr];
}

u8 k051960_device::k051960_r(offs_t offset)
{
	if (BIT(m_control, 5))
	{
		// the 051960 remembers the last address read and uses it when reading the sprite ROMs
		if (!machine().side_effects_disabled())
			m_romoffset = (offset & 0x3fc) >> 2;

		return k051960_fetchromdata(offset & 3); // only 88 Games reads the ROMs from here
	}
	else
		return m_ram[offset];
}

void k051960_device::k051960_w(offs_t offset, u8 data)
{
	m_ram[offset] = data;
}


/* should this be split by k051960? */
u8 k051960_device::k051937_r(offs_t offset)
{
	offset &= 7;

	if (BIT(m_control, 5) && offset & 4)
		return k051960_fetchromdata(offset & 3);
	else if (offset == 0)
		return m_sprites_busy->enabled() ? 1 : 0;

	//logerror("%s: read unknown 051937 address %x\n", m_maincpu->pc(), offset);
	return 0;
}

void k051960_device::k051937_w(offs_t offset, u8 data)
{
	offset &= 7;

	if (offset == 0)
	{
		// clear interrupts
		if (BIT(~data & m_control, 0))
			m_irq_handler(CLEAR_LINE);

		if (BIT(~data & m_control, 1))
			m_firq_handler(CLEAR_LINE);

		if (BIT(~data & m_control, 2))
			m_nmi_handler(CLEAR_LINE);

		//if (data & 0xc2) popmessage("051937 reg 00 = %02x",data);
		m_control = data;
	}
	else if (offset == 1)
	{
		if (0)
			logerror("%s: %02x to 051937 address %x\n", machine().describe_context(), data, offset);

		// callback for setting palette shadow mode
		if (BIT(data ^ m_shadow_config, 0))
			m_shadow_config_cb(data & 1);

		m_shadow_config = data & 0x07;
	}
	else if (offset >= 2 && offset < 5)
	{
		m_spriterombank[offset - 2] = data;
	}
	else
	{
		//logerror("%s: write %02x to unknown 051937 address %x\n", m_maincpu->pc(), data, offset);
	}
}


/*
 * Sprite Format
 * ------------------
 *
 * Byte | Bit(s)   | Use
 * -----+-76543210-+----------------
 *   0  | x------- | active (show this sprite)
 *   0  | -xxxxxxx | priority order
 *   1  | xxx----- | sprite size (see below)
 *   1  | ---xxxxx | sprite code (high 5 bits)
 *   2  | xxxxxxxx | sprite code (low 8 bits)
 *   3  | xxxxxxxx | "color", but depends on external connections (see below)
 *   4  | xxxxxx-- | zoom y (0 = normal, >0 = shrink)
 *   4  | ------x- | flip y
 *   4  | -------x | y position (high bit)
 *   5  | xxxxxxxx | y position (low 8 bits)
 *   6  | xxxxxx-- | zoom x (0 = normal, >0 = shrink)
 *   6  | ------x- | flip x
 *   6  | -------x | x position (high bit)
 *   7  | xxxxxxxx | x position (low 8 bits)
 *
 * Example of "color" field for Punk Shot:
 *   3  | x------- | shadow
 *   3  | -xx----- | priority
 *   3  | ---x---- | use second gfx ROM bank
 *   3  | ----xxxx | color code
 *
 * shadow enables transparent shadows. Note that it applies to pen 0x0f ONLY.
 * The rest of the sprite remains normal.
 * Note that Aliens also uses the shadow bit to select the second sprite bank.
 */

void k051960_device::k051960_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int min_priority, int max_priority)
{
	static constexpr int NUM_SPRITES = 128;

	int offs, pri_code;
	int sortedlist[NUM_SPRITES];
	u8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (offs = 0; offs < 0x400; offs += 8)
	{
		if (m_buffer[offs] & 0x80)
		{
			pri_code = m_buffer[offs] & 0x7f;
			if (max_priority == -1) /* draw front to back when using priority buffer */
				pri_code ^= 0x7f;

			sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = 0; pri_code < NUM_SPRITES; pri_code++)
	{
		int ox, oy, code, color, pri, size, w, h, x, y, flipx, flipy, zoomx, zoomy;
		bool shadow;
		/* sprites can be grouped up to 8x8. The draw order is
		     0  1  4  5 16 17 20 21
		     2  3  6  7 18 19 22 23
		     8  9 12 13 24 25 28 29
		    10 11 14 15 26 27 30 31
		    32 33 36 37 48 49 52 53
		    34 35 38 39 50 51 54 55
		    40 41 44 45 56 57 60 61
		    42 43 46 47 58 59 62 63
		*/
		static const int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
		static const int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
		static const int width[8] =  { 1, 2, 1, 2, 4, 2, 4, 8 };
		static const int height[8] = { 1, 1, 2, 2, 2, 4, 4, 8 };

		offs = sortedlist[pri_code];
		if (offs == -1)
			continue;

		code = m_buffer[offs + 2] + ((m_buffer[offs + 1] & 0x1f) << 8);
		color = m_buffer[offs + 3] & 0xff;
		pri = 0;
		shadow = !BIT(m_shadow_config, 2) && (BIT(m_shadow_config, 1) || BIT(color, 7));
		m_k051960_cb(&code, &color, &pri, &shadow);

		if (max_priority != -1)
			if (pri < min_priority || pri > max_priority)
				continue;

		size = (m_buffer[offs + 1] & 0xe0) >> 5;
		w = width[size];
		h = height[size];

		if (w >= 2) code &= ~0x01;
		if (h >= 2) code &= ~0x02;
		if (w >= 4) code &= ~0x04;
		if (h >= 4) code &= ~0x08;
		if (w >= 8) code &= ~0x10;
		if (h >= 8) code &= ~0x20;

		ox = (256 * m_buffer[offs + 6] + m_buffer[offs + 7]) & 0x01ff;
		oy = 256 - ((256 * m_buffer[offs + 4] + m_buffer[offs + 5]) & 0x01ff);
		flipx = m_buffer[offs + 6] & 0x02;
		flipy = m_buffer[offs + 4] & 0x02;

		// X zoom is linear, 128x128 factors are accurate compared to PCB, but
		// off-by-1 at several places for smaller sprite sizes.
		zoomx = (m_buffer[offs + 6] & 0xfc) >> 2;
		zoomx = 0x10000 / 128 * (128 - zoomx);

		// Y zoom is not linear, it can't be expressed as an exponential function.
		// These values were visually obtained from Devastators.
		static const u8 zoomy_table[0x40] =
		{
			0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0a, 0x0c,
			0x0e, 0x0f, 0x11, 0x12, 0x14, 0x15, 0x16, 0x18,
			0x19, 0x1a, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
			0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
			0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2e, 0x2f, 0x30,
			0x31, 0x31, 0x32, 0x33, 0x34, 0x34, 0x35, 0x36,
			0x36, 0x37, 0x38, 0x38, 0x39, 0x39, 0x3a, 0x3b,
			0x3b, 0x3c, 0x3c, 0x3d, 0x3d, 0x3e, 0x3e, 0x3f
		};

		zoomy = (m_buffer[offs + 4] & 0xfc) >> 2;
		zoomy = 128 - zoomy_table[zoomy];

		// accumulated rounding up for each sprite height
		for (int i = 0; i < 3; i++)
			if (h <= (1 << i)) zoomy = (zoomy + 1) / 2;

		zoomy *= 8 / h;
		zoomy = 0x10000 / 128 * zoomy;

		if (BIT(m_control, 3))
		{
			ox = 512 - (zoomx * w >> 12) - ox;
			oy = 256 - (zoomy * h >> 12) - oy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawmode_table[gfx(0)->granularity() - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		if (zoomx == 0x10000 && zoomy == 0x10000)
		{
			int sx, sy;

			for (y = 0; y < h; y++)
			{
				sy = oy + 16 * y;

				for (x = 0; x < w; x++)
				{
					int c = code;

					sx = ox + 16 * x;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (max_priority == -1)
						gfx(0)->prio_transtable(bitmap,cliprect,
								c,color,
								flipx,flipy,
								(sx & 0x1ff) - 96, sy,
								priority_bitmap,pri,
								drawmode_table);
					else
						gfx(0)->transtable(bitmap,cliprect,
								c,color,
								flipx,flipy,
								(sx & 0x1ff) - 96, sy,
								drawmode_table);
				}
			}
		}
		else
		{
			int sx, sy, zw, zh;

			for (y = 0; y < h; y++)
			{
				sy = oy + ((zoomy * y + (1 << 11)) >> 12);
				zh = (oy + ((zoomy * (y + 1) + (1 << 11)) >> 12)) - sy;

				for (x = 0; x < w; x++)
				{
					int c = code;

					sx = ox + ((zoomx * x + (1 << 11)) >> 12);
					zw = (ox + ((zoomx * (x + 1) + (1 << 11)) >> 12)) - sx;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (max_priority == -1)
						gfx(0)->prio_zoom_transtable(bitmap,cliprect,
								c,color,
								flipx,flipy,
								(sx & 0x1ff) - 96, sy,
								(zw << 16) / 16,(zh << 16) / 16,
								priority_bitmap,pri,
								drawmode_table);
					else
						gfx(0)->zoom_transtable(bitmap,cliprect,
								c,color,
								flipx,flipy,
								(sx & 0x1ff) - 96, sy,
								(zw << 16) / 16,(zh << 16) / 16,
								drawmode_table);
				}
			}
		}
	}
}
