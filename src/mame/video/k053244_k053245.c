/*

053245/053244
-------------
Sprite generators. The 053245 has a 16-bit data bus to the main CPU.
The sprites are buffered, a write to 006 activates to copy between the
main ram and the buffer.

053244 memory map (but the 053245 sees and processes them too):
000-001  W global X offset
002-003  W global Y offset
004      W unknown
005      W bit 0 = flip screen X
           bit 1 = flip screen Y
           bit 2 = unknown, used by Parodius
           bit 4 = enable gfx ROM reading
           bit 5 = unknown, used by Rollergames
006     RW accessing this register copies the sprite ram to the internal buffer
007      W unknown
008-009  W low 16 bits of the ROM address to read
00a-00b  W high bits of the ROM address to read.  3 bits for most games, 1 for asterix
00c-00f R  reads data from the gfx ROMs (32 bits in total). The address of the
           data is determined by the registers above; plus bank switch bits for
           larger ROMs.


*/

#include "emu.h"
#include "k053244_k053245.h"
#include "konami_helper.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

const device_type K053244 = &device_creator<k05324x_device>;

k05324x_device::k05324x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053244, "Konami 053244 & 053245", tag, owner, clock, "k05324x", __FILE__),
	m_ram(NULL),
	m_buffer(NULL),
	m_gfx(NULL),
	//m_regs[0x10],
	m_rombank(0),
	m_ramsize(0),
	m_z_rejection(0),
	m_gfxdecode(*this),
	m_palette(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void k05324x_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<k05324x_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void k05324x_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<k05324x_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k05324x_device::device_config_complete()
{
	// inherit a copy of the static data
	const k05324x_interface *intf = reinterpret_cast<const k05324x_interface *>(static_config());
	if (intf != NULL)
	*static_cast<k05324x_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_gfx_memory_region = "";
		m_gfx_num = 0;
		m_plane_order = 0;
		m_dx = 0;
		m_dy = 0;
		m_deinterleave = 0;
		m_callback = NULL;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k05324x_device::device_start()
{
	UINT32 total;
	static const gfx_layout spritelayout =
	{
		16,16,
		0,
		4,
		{ 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7,
				8*32+0, 8*32+1, 8*32+2, 8*32+3, 8*32+4, 8*32+5, 8*32+6, 8*32+7 },
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
				16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
		128*8
	};

	/* decode the graphics */
	switch (m_plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = machine().root_device().memregion(m_gfx_memory_region)->bytes() / 128;
		konami_decode_gfx(machine(), m_gfxdecode, m_palette, m_gfx_num, machine().root_device().memregion(m_gfx_memory_region)->base(), total, &spritelayout, 4);
		break;

	default:
		fatalerror("Unsupported plane_order\n");
	}

	if (VERBOSE && !(m_palette->shadows_enabled()))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	/* deinterleave the graphics, if needed */
	konami_deinterleave_gfx(machine(), m_gfx_memory_region, m_deinterleave);

	m_ramsize = 0x800;

	m_z_rejection = -1;
	m_gfx = m_gfxdecode->gfx(m_gfx_num);
	m_ram = auto_alloc_array_clear(machine(), UINT16, m_ramsize / 2);

	m_buffer = auto_alloc_array_clear(machine(), UINT16, m_ramsize / 2);

	save_pointer(NAME(m_ram), m_ramsize / 2);
	save_pointer(NAME(m_buffer), m_ramsize / 2);
	save_item(NAME(m_rombank));
	save_item(NAME(m_z_rejection));
	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k05324x_device::device_reset()
{
	int i;

	m_rombank = 0;

	for (i = 0; i < 0x10; i++)
		m_regs[i] = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k05324x_device::k053245_set_sprite_offs( int offsx, int offsy )
{
	m_dx = offsx;
	m_dy = offsy;
}

READ16_MEMBER( k05324x_device::k053245_word_r )
{
	return m_ram[offset];
}

WRITE16_MEMBER( k05324x_device::k053245_word_w )
{
	COMBINE_DATA(m_ram + offset);
}

READ8_MEMBER( k05324x_device::k053245_r )
{
	if(offset & 1)
		return m_ram[offset >> 1] & 0xff;
	else
		return (m_ram[offset >> 1] >> 8) & 0xff;
}


WRITE8_MEMBER( k05324x_device::k053245_w )
{
	if(offset & 1)
		m_ram[offset >> 1] = (m_ram[offset >> 1] & 0xff00) | data;
	else
		m_ram[offset >> 1] = (m_ram[offset >> 1] & 0x00ff) | (data << 8);
}

void k05324x_device::k053245_clear_buffer( )
{
	int i, e;

	for (e = m_ramsize / 2, i = 0; i < e; i += 8)
		m_buffer[i] = 0;
}

void k05324x_device::k053245_update_buffer( )
{
	memcpy(m_buffer, m_ram, m_ramsize);
}

READ8_MEMBER( k05324x_device::k053244_r )
{
	if ((m_regs[5] & 0x10) && offset >= 0x0c && offset < 0x10)
	{
		int addr;

		addr = (m_rombank << 19) | ((m_regs[11] & 0x7) << 18)
			| (m_regs[8] << 10) | (m_regs[9] << 2)
			| ((offset & 3) ^ 1);
		addr &= machine().root_device().memregion(m_gfx_memory_region)->bytes() - 1;

		//  popmessage("%s: offset %02x addr %06x", machine().describe_context(), offset & 3, addr);

		return machine().root_device().memregion(m_gfx_memory_region)->base()[addr];
	}
	else if (offset == 0x06)
	{
		k053245_update_buffer();
		return 0;
	}
	else
	{
		//logerror("%s: read from unknown 053244 address %x\n", machine().describe_context(), offset);
		return 0;
	}
}

WRITE8_MEMBER( k05324x_device::k053244_w )
{
	m_regs[offset] = data;

	switch(offset)
	{
	case 0x05:
//      if (data & 0xc8)
//          popmessage("053244 reg 05 = %02x",data);
		/* bit 2 = unknown, Parodius uses it */
		/* bit 5 = unknown, Rollergames uses it */
//      logerror("%s: write %02x to 053244 address 5\n", space.machine().describe_context(), data);
		break;

	case 0x06:
		k053245_update_buffer();
		break;
	}
}


READ16_MEMBER( k05324x_device::k053244_lsb_r )
{
	return k053244_r(space, offset);
}

WRITE16_MEMBER( k05324x_device::k053244_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		k053244_w(space, offset, data & 0xff);
}

READ16_MEMBER( k05324x_device::k053244_word_r )
{
	return (k053244_r(space, offset * 2) << 8) | k053244_r(space, offset * 2 + 1);
}

WRITE16_MEMBER( k05324x_device::k053244_word_w )
{
	if (ACCESSING_BITS_8_15)
		k053244_w(space, offset * 2, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		k053244_w(space, offset * 2 + 1, data & 0xff);
}

void k05324x_device::k053244_bankselect( int bank )
{
	m_rombank = bank;
}

void k05324x_device::k05324x_set_z_rejection( int zcode )
{
	m_z_rejection = zcode;
}

/*
 * Sprite Format
 * ------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | x--------------- | active (show this sprite)
 *   0  | -x-------------- | maintain aspect ratio (when set, zoom y acts on both axis)
 *   0  | --x------------- | flip y
 *   0  | ---x------------ | flip x
 *   0  | ----xxxx-------- | sprite size (see below)
 *   0  | ---------xxxxxxx | priority order
 *   1  | --xxxxxxxxxxxxxx | sprite code. We use an additional bit in TMNT2, but this is
 *                           probably not accurate (protection related so we can't verify)
 *   2  | ------xxxxxxxxxx | y position
 *   3  | ------xxxxxxxxxx | x position
 *   4  | xxxxxxxxxxxxxxxx | zoom y (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   5  | xxxxxxxxxxxxxxxx | zoom x (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   6  | ------x--------- | mirror y (top half is drawn as mirror image of the bottom)
 *   6  | -------x-------- | mirror x (right half is drawn as mirror image of the left)
 *   6  | --------x------- | shadow
 *   6  | ---------xxxxxxx | "color", but depends on external connections
 *   7  | ---------------- |
 *
 * shadow enables transparent shadows. Note that it applies to pen 0x0f ONLY.
 * The rest of the sprite remains normal.
 */

void k05324x_device::k053245_sprites_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap )
{
#define NUM_SPRITES 128
	int offs, pri_code, i;
	int sortedlist[NUM_SPRITES];
	int flipscreenX, flipscreenY, spriteoffsX, spriteoffsY;
	UINT8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	flipscreenX = m_regs[5] & 0x01;
	flipscreenY = m_regs[5] & 0x02;
	spriteoffsX = (m_regs[0] << 8) | m_regs[1];
	spriteoffsY = (m_regs[2] << 8) | m_regs[3];

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (i = m_ramsize / 2, offs = 0; offs < i; offs += 8)
	{
		pri_code = m_buffer[offs];
		if (pri_code & 0x8000)
		{
			pri_code &= 0x007f;

			if (offs && pri_code == m_z_rejection)
				continue;

			if (sortedlist[pri_code] == -1)
				sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES - 1; pri_code >= 0; pri_code--)
	{
		int ox, oy, color, code, size, w, h, x, y, flipx, flipy, mirrorx, mirrory, shadow, zoomx, zoomy, pri;

		offs = sortedlist[pri_code];
		if (offs == -1)
			continue;

		/* the following changes the sprite draw order from
		     0  1  4  5 16 17 20 21
		     2  3  6  7 18 19 22 23
		     8  9 12 13 24 25 28 29
		    10 11 14 15 26 27 30 31
		    32 33 36 37 48 49 52 53
		    34 35 38 39 50 51 54 55
		    40 41 44 45 56 57 60 61
		    42 43 46 47 58 59 62 63

		    to

		     0  1  2  3  4  5  6  7
		     8  9 10 11 12 13 14 15
		    16 17 18 19 20 21 22 23
		    24 25 26 27 28 29 30 31
		    32 33 34 35 36 37 38 39
		    40 41 42 43 44 45 46 47
		    48 49 50 51 52 53 54 55
		    56 57 58 59 60 61 62 63
		*/

		/* NOTE: from the schematics, it looks like the top 2 bits should be ignored */
		/* (there are not output pins for them), and probably taken from the "color" */
		/* field to do bank switching. However this applies only to TMNT2, with its */
		/* protection mcu creating the sprite table, so we don't know where to fetch */
		/* the bits from. */
		code = m_buffer[offs + 1];
		code = ((code & 0xffe1) + ((code & 0x0010) >> 2) + ((code & 0x0008) << 1)
					+ ((code & 0x0004) >> 1) + ((code & 0x0002) << 2));
		color = m_buffer[offs + 6] & 0x00ff;
		pri = 0;

		m_callback(machine(), &code, &color, &pri);

		size = (m_buffer[offs] & 0x0f00) >> 8;

		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* zoom control:
		   0x40 = normal scale
		  <0x40 enlarge (0x20 = double size)
		  >0x40 reduce (0x80 = half size)
		*/
		zoomy = m_buffer[offs + 4];
		if (zoomy > 0x2000)
			continue;

		if (zoomy)
			zoomy = (0x400000 + zoomy / 2) / zoomy;
		else
			zoomy = 2 * 0x400000;
		if ((m_buffer[offs] & 0x4000) == 0)
		{
			zoomx = m_buffer[offs + 5];
			if (zoomx > 0x2000)
				continue;
			if (zoomx)
				zoomx = (0x400000 + zoomx / 2) / zoomx;
			else
				zoomx = 2 * 0x400000;
//          else zoomx = zoomy; /* workaround for TMNT2 */
		}
		else
			zoomx = zoomy;

		ox = m_buffer[offs+3] + spriteoffsX;
		oy = m_buffer[offs+2];

		ox += m_dx;
		oy += m_dy;

		flipx = m_buffer[offs] & 0x1000;
		flipy = m_buffer[offs] & 0x2000;
		mirrorx = m_buffer[offs + 6] & 0x0100;
		if (mirrorx)
			flipx = 0; // documented and confirmed

		mirrory = m_buffer[offs + 6] & 0x0200;
		shadow = m_buffer[offs + 6] & 0x0080;

		if (flipscreenX)
		{
			ox = 512 - ox;
			if (!mirrorx)
				flipx = !flipx;
		}
		if (flipscreenY)
		{
			oy = -oy;
			if (!mirrory)
				flipy = !flipy;
		}

		ox = (ox + 0x5d) & 0x3ff;
		if (ox >= 768) ox -= 1024;
		oy = (-(oy + spriteoffsY + 0x07)) & 0x3ff;
		if (oy >= 640) oy -= 1024;

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

		drawmode_table[m_gfx->granularity() - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = 0; y < h; y++)
		{
			int sx, sy, zw, zh;

			sy = oy + ((zoomy * y + (1 << 11)) >> 12);
			zh = (oy + ((zoomy * (y + 1) + (1 << 11)) >> 12)) - sy;

			for (x = 0; x < w; x++)
			{
				int c, fx, fy;

				sx = ox + ((zoomx * x + (1 << 11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1 << 11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ (2*x < w))
					{
						/* mirror left/right */
						c += (w - x - 1);
						fx = 1;
					}
					else
					{
						c += x;
						fx = 0;
					}
				}
				else
				{
					if (flipx) c += w-1-x;
					else c += x;
					fx = flipx;
				}
				if (mirrory)
				{
					if ((flipy == 0) ^ (2*y >= h))
					{
						/* mirror top/bottom */
						c += 8 * (h - y - 1);
						fy = 1;
					}
					else
					{
						c += 8 * y;
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += 8 * (h - 1 - y);
					else c += 8 * y;
					fy = flipy;
				}

				/* the sprite can start at any point in the 8x8 grid, but it must stay */
				/* in a 64 entries window, wrapping around at the edges. The animation */
				/* at the end of the saloon level in Sunset Riders breaks otherwise. */
				c = (c & 0x3f) | (code & ~0x3f);

				if (zoomx == 0x10000 && zoomy == 0x10000)
				{
					m_gfx->prio_transtable(bitmap,cliprect,
							c,color,
							fx,fy,
							sx,sy,
							priority_bitmap,pri,
							drawmode_table);
				}
				else
				{
					m_gfx->prio_zoom_transtable(bitmap,cliprect,
							c,color,
							fx,fy,
							sx,sy,
							(zw << 16) / 16,(zh << 16) / 16,
							priority_bitmap,pri,
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
		fwrite(m_buffer, 0x800, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}

/* Lethal Enforcers has 2 of these chips hooked up in parallel to give 6bpp gfx.. let's cheat a
  bit and make emulating it a little less messy by using a custom function instead */
void k05324x_device::k053245_sprites_draw_lethal( bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap )
{
#define NUM_SPRITES 128
	int offs, pri_code, i;
	int sortedlist[NUM_SPRITES];
	int flipscreenX, flipscreenY, spriteoffsX, spriteoffsY;
	UINT8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	flipscreenX = m_regs[5] & 0x01;
	flipscreenY = m_regs[5] & 0x02;
	spriteoffsX = (m_regs[0] << 8) | m_regs[1];
	spriteoffsY = (m_regs[2] << 8) | m_regs[3];

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (i = m_ramsize / 2, offs = 0; offs < i; offs += 8)
	{
		pri_code = m_buffer[offs];
		if (pri_code & 0x8000)
		{
			pri_code &= 0x007f;

			if (offs && pri_code == m_z_rejection)
				continue;

			if (sortedlist[pri_code] == -1)
				sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES - 1; pri_code >= 0; pri_code--)
	{
		int ox, oy, color, code, size, w, h, x, y, flipx, flipy, mirrorx, mirrory, shadow, zoomx, zoomy, pri;

		offs = sortedlist[pri_code];
		if (offs == -1)
			continue;

		/* the following changes the sprite draw order from
		     0  1  4  5 16 17 20 21
		     2  3  6  7 18 19 22 23
		     8  9 12 13 24 25 28 29
		    10 11 14 15 26 27 30 31
		    32 33 36 37 48 49 52 53
		    34 35 38 39 50 51 54 55
		    40 41 44 45 56 57 60 61
		    42 43 46 47 58 59 62 63

		    to

		     0  1  2  3  4  5  6  7
		     8  9 10 11 12 13 14 15
		    16 17 18 19 20 21 22 23
		    24 25 26 27 28 29 30 31
		    32 33 34 35 36 37 38 39
		    40 41 42 43 44 45 46 47
		    48 49 50 51 52 53 54 55
		    56 57 58 59 60 61 62 63
		*/

		/* NOTE: from the schematics, it looks like the top 2 bits should be ignored */
		/* (there are not output pins for them), and probably taken from the "color" */
		/* field to do bank switching. However this applies only to TMNT2, with its */
		/* protection mcu creating the sprite table, so we don't know where to fetch */
		/* the bits from. */
		code = m_buffer[offs + 1];
		code = ((code & 0xffe1) + ((code & 0x0010) >> 2) + ((code & 0x0008) << 1)
					+ ((code & 0x0004) >> 1) + ((code & 0x0002) << 2));
		color = m_buffer[offs + 6] & 0x00ff;
		pri = 0;

		m_callback(machine(), &code, &color, &pri);

		size = (m_buffer[offs] & 0x0f00) >> 8;

		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* zoom control:
		   0x40 = normal scale
		  <0x40 enlarge (0x20 = double size)
		  >0x40 reduce (0x80 = half size)
		*/
		zoomy = m_buffer[offs + 4];
		if (zoomy > 0x2000)
			continue;
		if (zoomy)
			zoomy = (0x400000 + zoomy / 2) / zoomy;
		else
			zoomy = 2 * 0x400000;
		if ((m_buffer[offs] & 0x4000) == 0)
		{
			zoomx = m_buffer[offs + 5];
			if (zoomx > 0x2000)
				continue;
			if (zoomx)
				zoomx = (0x400000 + zoomx / 2) / zoomx;
			else
				zoomx = 2 * 0x400000;
//          else zoomx = zoomy; /* workaround for TMNT2 */
		}
		else
			zoomx = zoomy;

		ox = m_buffer[offs + 3] + spriteoffsX;
		oy = m_buffer[offs + 2];

		ox += m_dx;
		oy += m_dy;

		flipx = m_buffer[offs] & 0x1000;
		flipy = m_buffer[offs] & 0x2000;
		mirrorx = m_buffer[offs + 6] & 0x0100;
		if (mirrorx)
			flipx = 0; // documented and confirmed
		mirrory = m_buffer[offs + 6] & 0x0200;
		shadow = m_buffer[offs + 6] & 0x0080;

		if (flipscreenX)
		{
			ox = 512 - ox;
			if (!mirrorx) flipx = !flipx;
		}
		if (flipscreenY)
		{
			oy = -oy;
			if (!mirrory) flipy = !flipy;
		}

		ox = (ox + 0x5d) & 0x3ff;
		if (ox >= 768) ox -= 1024;
		oy = (-(oy + spriteoffsY + 0x07)) & 0x3ff;
		if (oy >= 640) oy -= 1024;

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

		drawmode_table[m_gfxdecode->gfx(0)->granularity() - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = 0; y < h; y++)
		{
			int sx, sy, zw, zh;

			sy = oy + ((zoomy * y + (1<<11)) >> 12);
			zh = (oy + ((zoomy * (y+1) + (1<<11)) >> 12)) - sy;

			for (x = 0; x < w; x++)
			{
				int c, fx, fy;

				sx = ox + ((zoomx * x + (1 << 11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1 << 11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ (2 * x < w))
					{
						/* mirror left/right */
						c += (w - x - 1);
						fx = 1;
					}
					else
					{
						c += x;
						fx = 0;
					}
				}
				else
				{
					if (flipx) c += w-1-x;
					else c += x;
					fx = flipx;
				}
				if (mirrory)
				{
					if ((flipy == 0) ^ (2 * y >= h))
					{
						/* mirror top/bottom */
						c += 8 * (h - y - 1);
						fy = 1;
					}
					else
					{
						c += 8 * y;
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += 8 * (h - 1 - y);
					else c += 8 * y;
					fy = flipy;
				}

				/* the sprite can start at any point in the 8x8 grid, but it must stay */
				/* in a 64 entries window, wrapping around at the edges. The animation */
				/* at the end of the saloon level in Sunset Riders breaks otherwise. */
				c = (c & 0x3f) | (code & ~0x3f);

				if (zoomx == 0x10000 && zoomy == 0x10000)
				{
					m_gfxdecode->gfx(0)->prio_transtable(bitmap,cliprect, /* hardcoded to 0 (decoded 6bpp gfx) for le */
							c,color,
							fx,fy,
							sx,sy,
							priority_bitmap,pri,
							drawmode_table);
				}
				else
				{
					m_gfxdecode->gfx(0)->prio_zoom_transtable(bitmap,cliprect,  /* hardcoded to 0 (decoded 6bpp gfx) for le */
							c,color,
							fx,fy,
							sx,sy,
							(zw << 16) / 16,(zh << 16) / 16,
							priority_bitmap,pri,
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
		fwrite(m_buffer, 0x800, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}

READ16_MEMBER( k05324x_device::k053244_reg_word_r )
{
	return(m_regs[offset * 2] << 8 | m_regs[offset * 2 + 1]);
}
