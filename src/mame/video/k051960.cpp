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
The 051960 can also genenrate IRQ, FIRQ and NMI signals.

memory map:
000-007 is for the 051937, but also seen by the 051960
400-7ff is 051960 only
000     R  bit 0 = vblank?
                   aliens waits for it to be 0 before starting to copy sprite data
                   thndrx2 needs it to pulse for the startup checks to succeed
000     W  bit 0 = irq acknowledge
           bit 1 = firq acknowledge?
           bit 2 = nmi enable/acknowledge
           bit 3 = flip screen (applies to sprites only, not tilemaps)
           bit 4 = unknown, used by Devastators, TMNT, Aliens, Chequered Flag, maybe others
                   aliens sets it just after checking bit 0, and before copying
                   the sprite data
           bit 5 = enable gfx ROM reading
001     W  Devastators sets bit 1, function unknown.
           Ultraman sets the register to 0x0f.
           None of the other games I tested seem to set this register to other than 0.
002-003 W  selects the portion of the gfx ROMs to be read.
004     W  Aliens uses this to select the ROM bank to be read, but Punk Shot
           and TMNT don't, they use another bit of the registers above. Many
           other games write to this register before testing.
           It is possible that bits 2-7 of 003 go to OC0-OC5, and bits 0-1 of
           004 go to OC6-OC7.
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

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

const device_type K051960 = &device_creator<k051960_device>;

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


k051960_device::k051960_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K051960, "K051960 Sprite Generator", tag, owner, clock, "k051960", __FILE__),
	device_gfx_interface(mconfig, *this, gfxinfo),
	m_ram(NULL),
	m_sprite_rom(NULL),
	m_sprite_size(0),
	m_screen_tag(NULL),
	m_screen(NULL),
	m_scanline_timer(NULL),
	m_irq_handler(*this),
	m_firq_handler(*this),
	m_nmi_handler(*this),
	m_romoffset(0),
	m_spriteflip(0),
	m_readroms(0),
	m_nmi_enabled(0)
{
}

void k051960_device::set_plane_order(device_t &device, int order)
{
	k051960_device &dev = downcast<k051960_device &>(device);

	switch (order)
	{
		case K051960_PLANEORDER_BASE:
			device_gfx_interface::static_set_info(dev, gfxinfo);
			break;

		case K051960_PLANEORDER_MIA:
			device_gfx_interface::static_set_info(dev, gfxinfo_reverse);
			break;

		case K051960_PLANEORDER_GRADIUS3:
			device_gfx_interface::static_set_info(dev, gfxinfo_gradius3);
			break;

		default:
			fatalerror("Unknown plane_order\n");
	}
}

//-------------------------------------------------
//  set_screen_tag - set screen we are attached to
//-------------------------------------------------

void k051960_device::set_screen_tag(device_t &device, device_t *owner, const char *tag)
{
	k051960_device &dev = dynamic_cast<k051960_device &>(device);
	dev.m_screen_tag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k051960_device::device_start()
{
	// make sure our screen is started
	m_screen = m_owner->subdevice<screen_device>(m_screen_tag);
	if (!m_screen->started())
		throw device_missing_dependencies();

	// allocate scanline timer and start at first scanline
	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(k051960_device::scanline_callback), this));
	m_scanline_timer->adjust(m_screen->time_until_pos(0));

	m_sprite_rom = region()->base();
	m_sprite_size = region()->bytes();

	decode_gfx();
	m_gfx[0]->set_colors(m_palette->entries() / m_gfx[0]->depth());

	if (VERBOSE && !(m_palette->shadows_enabled()))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	m_ram = auto_alloc_array_clear(machine(), UINT8, 0x400);

	// bind callbacks
	m_k051960_cb.bind_relative_to(*owner());

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_firq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();

	// register for save states
	save_item(NAME(m_romoffset));
	save_item(NAME(m_spriteflip));
	save_item(NAME(m_readroms));
	save_item(NAME(m_nmi_enabled));
	save_item(NAME(m_spriterombank));
	save_pointer(NAME(m_ram), 0x400);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k051960_device::device_reset()
{
	m_romoffset = 0;
	m_spriteflip = 0;
	m_readroms = 0;
	m_nmi_enabled = 0;

	m_spriterombank[0] = 0;
	m_spriterombank[1] = 0;
	m_spriterombank[2] = 0;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

TIMER_CALLBACK_MEMBER( k051960_device::scanline_callback )
{
	// range 0..255
	UINT8 y = m_screen->vpos();

	// 32v
	if ((y % 32 == 0) && m_nmi_enabled)
		m_nmi_handler(ASSERT_LINE);

	// vblank
	if (y == 240)
		m_irq_handler(ASSERT_LINE);

	// wait for next line
	m_scanline_timer->adjust(m_screen->time_until_pos(y + 1));
}

int k051960_device::k051960_fetchromdata( int byte )
{
	int code, color, pri, shadow, off1, addr;

	addr = m_romoffset + (m_spriterombank[0] << 8) + ((m_spriterombank[1] & 0x03) << 16);
	code = (addr & 0x3ffe0) >> 5;
	off1 = addr & 0x1f;
	color = ((m_spriterombank[1] & 0xfc) >> 2) + ((m_spriterombank[2] & 0x03) << 6);
	pri = 0;
	shadow = color & 0x80;
	m_k051960_cb(&code, &color, &pri, &shadow);

	addr = (code << 7) | (off1 << 2) | byte;
	addr &= m_sprite_size - 1;

//  popmessage("%s: addr %06x", machine().describe_context(), addr);

	return m_sprite_rom[addr];
}

READ8_MEMBER( k051960_device::k051960_r )
{
	if (m_readroms)
	{
		/* the 051960 remembers the last address read and uses it when reading the sprite ROMs */
		m_romoffset = (offset & 0x3fc) >> 2;
		return k051960_fetchromdata(offset & 3);    /* only 88 Games reads the ROMs from here */
	}
	else
		return m_ram[offset];
}

WRITE8_MEMBER( k051960_device::k051960_w )
{
	m_ram[offset] = data;
}


/* should this be split by k051960? */
READ8_MEMBER( k051960_device::k051937_r )
{
	if (m_readroms && offset >= 4 && offset < 8)
		return k051960_fetchromdata(offset & 3);
	else if (offset == 0)
		return m_screen->vblank() ? 1 : 0; // vblank?

	//logerror("%04x: read unknown 051937 address %x\n", space.device().safe_pc(), offset);
	return 0;
}

WRITE8_MEMBER( k051960_device::k051937_w )
{
	if (offset == 0)
	{
		//if (data & 0xc2) popmessage("051937 reg 00 = %02x",data);

		if (BIT(data, 0)) m_irq_handler(CLEAR_LINE);  // bit 0, irq ack
		if (BIT(data, 1)) m_firq_handler(CLEAR_LINE); // bit 1, firq ack?

		// bit 2, nmi enable/ack
		m_nmi_enabled = BIT(data, 2);
		if (m_nmi_enabled)
			m_nmi_handler(CLEAR_LINE);

		/* bit 3 = flip screen */
		m_spriteflip = data & 0x08;

		/* bit 4 used by Devastators and TMNT, unknown */

		/* bit 5 = enable gfx ROM reading */
		m_readroms = data & 0x20;
		//logerror("%04x: write %02x to 051937 address %x\n", space.device().safe_pc(), data, offset);
	}
	else if (offset == 1)
	{
		// unknown, Devastators writes 02 here in game
		if (0)
			logerror("%s: %02x to 051937 address %x\n", machine().describe_context(), data, offset);
	}
	else if (offset >= 2 && offset < 5)
	{
		m_spriterombank[offset - 2] = data;
	}
	else
	{
	//  popmessage("%04x: write %02x to 051937 address %x", space.device().safe_pc(), data, offset);
	//logerror("%04x: write %02x to unknown 051937 address %x\n", space.device().safe_pc(), data, offset);
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

void k051960_device::k051960_sprites_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int min_priority, int max_priority )
{
#define NUM_SPRITES 128
	int offs, pri_code;
	int sortedlist[NUM_SPRITES];
	UINT8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (offs = 0; offs < 0x400; offs += 8)
	{
		if (m_ram[offs] & 0x80)
		{
			if (max_priority == -1) /* draw front to back when using priority buffer */
				sortedlist[(m_ram[offs] & 0x7f) ^ 0x7f] = offs;
			else
				sortedlist[m_ram[offs] & 0x7f] = offs;
		}
	}

	for (pri_code = 0; pri_code < NUM_SPRITES; pri_code++)
	{
		int ox, oy, code, color, pri, shadow, size, w, h, x, y, flipx, flipy, zoomx, zoomy;
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

		code = m_ram[offs + 2] + ((m_ram[offs + 1] & 0x1f) << 8);
		color = m_ram[offs + 3] & 0xff;
		pri = 0;
		shadow = color & 0x80;
		m_k051960_cb(&code, &color, &pri, &shadow);

		if (max_priority != -1)
			if (pri < min_priority || pri > max_priority)
				continue;

		size = (m_ram[offs + 1] & 0xe0) >> 5;
		w = width[size];
		h = height[size];

		if (w >= 2) code &= ~0x01;
		if (h >= 2) code &= ~0x02;
		if (w >= 4) code &= ~0x04;
		if (h >= 4) code &= ~0x08;
		if (w >= 8) code &= ~0x10;
		if (h >= 8) code &= ~0x20;

		ox = (256 * m_ram[offs + 6] + m_ram[offs + 7]) & 0x01ff;
		oy = 256 - ((256 * m_ram[offs + 4] + m_ram[offs + 5]) & 0x01ff);
		flipx = m_ram[offs + 6] & 0x02;
		flipy = m_ram[offs + 4] & 0x02;
		zoomx = (m_ram[offs + 6] & 0xfc) >> 2;
		zoomy = (m_ram[offs + 4] & 0xfc) >> 2;
		zoomx = 0x10000 / 128 * (128 - zoomx);
		zoomy = 0x10000 / 128 * (128 - zoomy);

		if (m_spriteflip)
		{
			ox = 512 - (zoomx * w >> 12) - ox;
			oy = 256 - (zoomy * h >> 12) - oy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawmode_table[m_gfx[0]->granularity() - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

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
						m_gfx[0]->prio_transtable(bitmap,cliprect,
								c,color,
								flipx,flipy,
								sx & 0x1ff,sy,
								priority_bitmap,pri,
								drawmode_table);
					else
						m_gfx[0]->transtable(bitmap,cliprect,
								c,color,
								flipx,flipy,
								sx & 0x1ff,sy,
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
					zw = (ox + ((zoomx * (x+1) + (1 << 11)) >> 12)) - sx;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (max_priority == -1)
						m_gfx[0]->prio_zoom_transtable(bitmap,cliprect,
								c,color,
								flipx,flipy,
								sx & 0x1ff,sy,
								(zw << 16) / 16,(zh << 16) / 16,
								priority_bitmap,pri,
								drawmode_table);
					else
						m_gfx[0]->zoom_transtable(bitmap,cliprect,
								c,color,
								flipx,flipy,
								sx & 0x1ff,sy,
								(zw << 16) / 16,(zh << 16) / 16,
								drawmode_table);
				}
			}
		}
	}
#if 0
if (machine().input().code_pressed(KEYCODE_D))
{
	FILE *fp;
	fp=fopen("SPRITE.DMP", "w+b");
	if (fp)
	{
		fwrite(k051960_ram, 0x400, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}
