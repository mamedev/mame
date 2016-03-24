// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*
Konami 052109/051962
-------------
These work in pair.
The 052109 manages 3 64x32 scrolling tilemaps with 8x8 characters, and
optionally generates timing clocks and interrupt signals. It uses 0x4000
bytes of RAM, and a variable amount of ROM. It cannot read the ROMs:
instead, it exports 21 bits (16 from the tilemap RAM + 3 for the character
raster line + 2 additional ones for ROM banking) and these are externally
used to generate the address of the required data on the ROM; the output of
the ROMs is sent to the 051962, along with a color code. In theory you could
have any combination of bits in the tilemap RAM, as long as they add to 16.
In practice, all the games supported so far standardize on the same format
which uses 3 bits for the color code and 13 bits for the character code.
The 051962 multiplexes the data of the three layers and converts it into
palette indexes and transparency bits which will be mixed later in the video
chain.
Priority is handled externally: these chips only generate the tilemaps, they
don't mix them.
Both chips are interfaced with the main CPU. When the RMRD pin is asserted,
the CPU can read the gfx ROM data. This is done by telling the 052109 which
dword to read (this is a combination of some banking registers, and the CPU
address lines), and then reading it from the 051962.

052109 inputs:
- address lines (AB0-AB15, AB13-AB15 seem to have a different function)
- data lines (DB0-DB7)
- misc interface stuff

052109 outputs:
- address lines for the private RAM (RA0-RA12)
- data lines for the private RAM (VD0-VD15)
- NMI, IRQ, FIRQ for the main CPU
- misc interface stuff
- ROM bank selector (CAB1-CAB2)
- character "code" (VC0-VC10)
- character "color" (COL0-COL7); used foc color but also bank switching and tile
  flipping. Exact meaning depends on externl connections. All evidence indicates
  that COL2 and COL3 select the tile bank, and are replaced with the low 2 bits
  from the bank register. The top 2 bits of the register go to CAB1-CAB2.
  However, this DOES NOT WORK with Gradius III. "color" seems to pass through
  unaltered.
- layer A horizontal scroll (ZA1H-ZA4H)
- layer B horizontal scroll (ZB1H-ZB4H)
- ????? (BEN)

051962 inputs:
- gfx data from the ROMs (VC0-VC31)
- color code (COL0-COL7); only COL4-COL7 seem to really be used for color; COL0
  is tile flip X.
- layer A horizontal scroll (ZA1H-ZA4H)
- layer B horizontal scroll (ZB1H-ZB4H)
- let main CPU read the gfx ROMs (RMRD)
- address lines to be used with RMRD (AB0-AB1)
- data lines to be used with RMRD (DB0-DB7)
- ????? (BEN)
- misc interface stuff

051962 outputs:
- FIX layer palette index (DFI0-DFI7)
- FIX layer transparency (NFIC)
- A layer palette index (DSA0-DSAD); DSAA-DSAD seem to be unused
- A layer transparency (NSAC)
- B layer palette index (DSB0-DSBD); DSBA-DSBD seem to be unused
- B layer transparency (NSBC)
- misc interface stuff


052109 memory layout:
0000-07ff: layer FIX tilemap (attributes)
0800-0fff: layer A tilemap (attributes)
1000-1fff: layer B tilemap (attributes)
180c-1833: A y scroll
1a00-1bff: A x scroll
1c00     : ?
1c80     : row/column scroll control
           ------xx layer A row scroll
                    00 = disabled
                    01 = disabled? (gradius3, vendetta)
                    10 = 32 lines
                    11 = 256 lines
           -----x-- layer A column scroll
                    0 = disabled
                    1 = 64 (actually 40) columns
           ---xx--- layer B row scroll
           --x----- layer B column scroll
           surpratk sets this register to 70 during the second boss. There is
           nothing obviously wrong so it's not clear what should happen.
           glfgreat sets it to 30 when showing the leader board
1d00     : bits 0 & 1 might enable NMI and FIRQ, not sure
         : bit 2 = IRQ enable
1d80     : ROM bank selector bits 0-3 = bank 0 bits 4-7 = bank 1
1e00     : ROM membank selector for ROM testing
1e80     : bit 0 = flip screen (applies to tilemaps only, not sprites)
         : bit 1 = set by crimfght, mainevt, surpratk, xmen, mia, punkshot, thndrx2, spy
         :         it seems to enable tile flip X, however flip X is handled by the
         :         051962 and it is not hardwired to a specific tile attribute.
         :         Note that xmen, punkshot and thndrx2 set the bit but the current
         :         drivers don't use flip X and seem to work fine.
         : bit 2 = enables tile flip Y when bit 1 of the tile attribute is set
1f00     : ROM bank selector bits 0-3 = bank 2 bits 4-7 = bank 3
2000-27ff: layer FIX tilemap (code)
2800-2fff: layer A tilemap (code)
3000-37ff: layer B tilemap (code)
3800-3807: nothing here, so the chip can share address space with a 051937
380c-3833: B y scroll
3a00-3bff: B x scroll
3c00-3fff: nothing here, so the chip can share address space with a 051960
3d80     : mirror of 1d80, but ONLY during ROM test (surpratk)
3e00     : mirror of 1e00, but ONLY during ROM test (surpratk)
3f00     : mirror of 1f00, but ONLY during ROM test (surpratk)
EXTRA ADDRESSING SPACE USED BY X-MEN:
4000-47ff: layer FIX tilemap (code high bits)
4800-4fff: layer A tilemap (code high bits)
5000-57ff: layer B tilemap (code high bits)

The main CPU doesn't have direct acces to the RAM used by the 052109, it has
to through the chip.
*/

#include "emu.h"
#include "k052109.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

const device_type K052109 = &device_creator<k052109_device>;

const gfx_layout k052109_device::charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 24, 16, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

const gfx_layout k052109_device::charlayout_ram =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

GFXDECODE_MEMBER( k052109_device::gfxinfo )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, charlayout, 0, 1)
GFXDECODE_END

GFXDECODE_MEMBER( k052109_device::gfxinfo_ram )
	GFXDECODE_DEVICE_RAM(DEVICE_SELF, 0, charlayout_ram, 0, 1)
GFXDECODE_END


k052109_device::k052109_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K052109, "K052109 Tilemap Generator", tag, owner, clock, "k052109", __FILE__),
	device_gfx_interface(mconfig, *this, gfxinfo),
	m_ram(nullptr),
	m_videoram_F(nullptr),
	m_videoram_A(nullptr),
	m_videoram_B(nullptr),
	m_videoram2_F(nullptr),
	m_videoram2_A(nullptr),
	m_videoram2_B(nullptr),
	m_colorram_F(nullptr),
	m_colorram_A(nullptr),
	m_colorram_B(nullptr),
	m_tileflip_enable(0),
	m_has_extra_video_ram(0),
	m_rmrd_line(0),
	m_irq_enabled(0),
	m_romsubbank(0),
	m_scrollctrl(0),
	m_char_rom(nullptr),
	m_char_size(0),
	m_screen_tag(nullptr),
	m_irq_handler(*this),
	m_firq_handler(*this),
	m_nmi_handler(*this)
{
}


void k052109_device::set_ram(device_t &device, bool ram)
{
	k052109_device &dev = downcast<k052109_device &>(device);

	if (ram)
		device_gfx_interface::static_set_info(dev, gfxinfo_ram);
	else
		device_gfx_interface::static_set_info(dev, gfxinfo);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k052109_device::device_start()
{
	if (m_screen_tag != nullptr)
	{
		// make sure our screen is started
		screen_device *screen = m_owner->subdevice<screen_device>(m_screen_tag);
		if (!screen->started())
			throw device_missing_dependencies();

		// and register a callback for vblank state
		screen->register_vblank_callback(vblank_state_delegate(FUNC(k052109_device::vblank_callback), this));
	}

	if (region() != nullptr)
	{
		m_char_rom = region()->base();
		m_char_size = region()->bytes();
	}

	decode_gfx();
	gfx(0)->set_colors(palette().entries() / gfx(0)->depth());

	m_ram = make_unique_clear<UINT8[]>(0x6000);

	m_colorram_F = &m_ram[0x0000];
	m_colorram_A = &m_ram[0x0800];
	m_colorram_B = &m_ram[0x1000];
	m_videoram_F = &m_ram[0x2000];
	m_videoram_A = &m_ram[0x2800];
	m_videoram_B = &m_ram[0x3000];
	m_videoram2_F = &m_ram[0x4000];
	m_videoram2_A = &m_ram[0x4800];
	m_videoram2_B = &m_ram[0x5000];

	m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(k052109_device::get_tile_info0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(k052109_device::get_tile_info1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(k052109_device::get_tile_info2),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);

	// bind callbacks
	m_k052109_cb.bind_relative_to(*owner());

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_firq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();

	save_pointer(NAME(m_ram.get()), 0x6000);
	save_item(NAME(m_rmrd_line));
	save_item(NAME(m_romsubbank));
	save_item(NAME(m_scrollctrl));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_charrombank));
	save_item(NAME(m_charrombank_2));
	save_item(NAME(m_has_extra_video_ram));
	machine().save().register_postload(save_prepost_delegate(FUNC(k052109_device::tileflip_reset), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k052109_device::device_reset()
{
	m_rmrd_line = CLEAR_LINE;
	m_irq_enabled = 0;
	m_romsubbank = 0;
	m_scrollctrl = 0;

	m_has_extra_video_ram = 0;

	for (int i = 0; i < 4; i++)
	{
		m_charrombank[i] = 0;
		m_charrombank_2[i] = 0;
	}
}

//-------------------------------------------------
//  set_screen_tag - set screen we are attached to
//-------------------------------------------------

void k052109_device::set_screen_tag(device_t &device, device_t *owner, const char *tag)
{
	k052109_device &dev = dynamic_cast<k052109_device &>(device);
	dev.m_screen_tag = tag;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k052109_device::vblank_callback(screen_device &screen, bool state)
{
	if (state)
		m_irq_handler(ASSERT_LINE);
}

READ8_MEMBER( k052109_device::read )
{
	if (m_rmrd_line == CLEAR_LINE)
	{
		if ((offset & 0x1fff) >= 0x1800)
		{
			if (offset >= 0x180c && offset < 0x1834)
			{   /* A y scroll */    }
			else if (offset >= 0x1a00 && offset < 0x1c00)
			{   /* A x scroll */    }
			else if (offset == 0x1d00)
			{   /* read for bitwise operations before writing */    }
			else if (offset >= 0x380c && offset < 0x3834)
			{   /* B y scroll */    }
			else if (offset >= 0x3a00 && offset < 0x3c00)
			{   /* B x scroll */    }
//          else
//logerror("%04x: read from unknown 052109 address %04x\n",space.device().safe_pc(),offset);
		}

		return m_ram[offset];
	}
	else    /* Punk Shot and TMNT read from 0000-1fff, Aliens from 2000-3fff */
	{
		assert (m_char_size != 0);

		int code = (offset & 0x1fff) >> 5;
		int color = m_romsubbank;
		int flags = 0;
		int priority = 0;
		int bank = m_charrombank[(color & 0x0c) >> 2] >> 2;   /* discard low bits (TMNT) */
		int addr;

		bank |= (m_charrombank_2[(color & 0x0c) >> 2] >> 2); // Surprise Attack uses this 2nd bank in the rom test

	if (m_has_extra_video_ram)
		code |= color << 8; /* kludge for X-Men */
	else
		m_k052109_cb(0, bank, &code, &color, &flags, &priority);

		addr = (code << 5) + (offset & 0x1f);
		addr &= m_char_size - 1;

//      logerror("%04x: off = %04x sub = %02x (bnk = %x) adr = %06x\n", space.device().safe_pc(), offset, m_romsubbank, bank, addr);

		return m_char_rom[addr];
	}
}

WRITE8_MEMBER( k052109_device::write )
{
	if ((offset & 0x1fff) < 0x1800) /* tilemap RAM */
	{
		if (offset >= 0x4000)
			m_has_extra_video_ram = 1;  /* kludge for X-Men */

		m_ram[offset] = data;
		m_tilemap[(offset & 0x1800) >> 11]->mark_tile_dirty(offset & 0x7ff);
	}
	else    /* control registers */
	{
		m_ram[offset] = data;

		if (offset >= 0x180c && offset < 0x1834)
		{   /* A y scroll */    }
		else if (offset >= 0x1a00 && offset < 0x1c00)
		{   /* A x scroll */    }
		else if (offset == 0x1c80)
		{
			if (m_scrollctrl != data)
			{
//popmessage("scrollcontrol = %02x", data);
//logerror("%04x: rowscrollcontrol = %02x\n", space.device().safe_pc(), data);
				m_scrollctrl = data;
			}
		}
		else if (offset == 0x1d00)
		{
//logerror("%04x: 052109 register 1d00 = %02x\n", space.device().safe_pc(), data);
			/* bit 2 = irq enable */
			/* the custom chip can also generate NMI and FIRQ, for use with a 6809 */
			m_irq_enabled = data & 0x04;
			if (m_irq_enabled)
				m_irq_handler(CLEAR_LINE);
		}
		else if (offset == 0x1d80)
		{
			int dirty = 0;

			if (m_charrombank[0] != (data & 0x0f))
				dirty |= 1;
			if (m_charrombank[1] != ((data >> 4) & 0x0f))
				dirty |= 2;

			if (dirty)
			{
				int i;

				m_charrombank[0] = data & 0x0f;
				m_charrombank[1] = (data >> 4) & 0x0f;

				for (i = 0; i < 0x1800; i++)
				{
					int bank = (m_ram[i]&0x0c) >> 2;
					if ((bank == 0 && (dirty & 1)) || (bank == 1 && (dirty & 2)))
					{
						m_tilemap[(i & 0x1800) >> 11]->mark_tile_dirty(i & 0x7ff);
					}
				}
			}
		}
		else if (offset == 0x1e00 || offset == 0x3e00) // Surprise Attack uses offset 0x3e00
		{
//logerror("%04x: 052109 register 1e00 = %02x\n",space.device().safe_pc(),data);
			m_romsubbank = data;
		}
		else if (offset == 0x1e80)
		{
//if ((data & 0xfe)) logerror("%04x: 052109 register 1e80 = %02x\n",space.device().safe_pc(),data);
			m_tilemap[0]->set_flip((data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_tilemap[1]->set_flip((data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_tilemap[2]->set_flip((data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			if (m_tileflip_enable != ((data & 0x06) >> 1))
			{
				m_tileflip_enable = ((data & 0x06) >> 1);

				m_tilemap[0]->mark_all_dirty();
				m_tilemap[1]->mark_all_dirty();
				m_tilemap[2]->mark_all_dirty();
			}
		}
		else if (offset == 0x1f00)
		{
			int dirty = 0;

			if (m_charrombank[2] != (data & 0x0f))
				dirty |= 1;

			if (m_charrombank[3] != ((data >> 4) & 0x0f))
				dirty |= 2;

			if (dirty)
			{
				int i;

				m_charrombank[2] = data & 0x0f;
				m_charrombank[3] = (data >> 4) & 0x0f;

				for (i = 0; i < 0x1800; i++)
				{
					int bank = (m_ram[i] & 0x0c) >> 2;
					if ((bank == 2 && (dirty & 1)) || (bank == 3 && (dirty & 2)))
						m_tilemap[(i & 0x1800) >> 11]->mark_tile_dirty(i & 0x7ff);
				}
			}
		}
		else if (offset >= 0x380c && offset < 0x3834)
		{   /* B y scroll */    }
		else if (offset >= 0x3a00 && offset < 0x3c00)
		{   /* B x scroll */    }
		else if (offset == 0x3d80) // Surprise Attack uses offset 0x3d80 in rom test
		{
			// mirroring this write, breaks Surprise Attack in game tilemaps
			m_charrombank_2[0] = data & 0x0f;
			m_charrombank_2[1] = (data >> 4) & 0x0f;
		}
		else if (offset == 0x3f00) // Surprise Attack uses offset 0x3f00 in rom test
		{
			// mirroring this write, breaks Surprise Attack in game tilemaps
			m_charrombank_2[2] = data & 0x0f;
			m_charrombank_2[3] = (data >> 4) & 0x0f;
		}
//      else
//          logerror("%04x: write %02x to unknown 052109 address %04x\n",space.device().safe_pc(),data,offset);
	}
}

READ16_MEMBER( k052109_device::word_r )
{
	return read(space, offset + 0x2000) | (read(space, offset) << 8);
}

WRITE16_MEMBER( k052109_device::word_w )
{
	if (ACCESSING_BITS_8_15)
		write(space, offset, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		write(space, offset + 0x2000, data & 0xff);
}

READ16_MEMBER( k052109_device::lsb_r )
{
	return read(space, offset);
}

WRITE16_MEMBER( k052109_device::lsb_w )
{
	if(ACCESSING_BITS_0_7)
		write(space, offset, data & 0xff);
}

void k052109_device::set_rmrd_line( int state )
{
	m_rmrd_line = state;
}

int k052109_device::get_rmrd_line( )
{
	return m_rmrd_line;
}


void k052109_device::tilemap_mark_dirty( int tmap_num )
{
	m_tilemap[tmap_num]->mark_all_dirty();
}


void k052109_device::tilemap_update( )
{
	int xscroll, yscroll, offs;

#if 0
{
popmessage("%x %x %x %x",
	m_charrombank[0],
	m_charrombank[1],
	m_charrombank[2],
	m_charrombank[3]);
}
#endif

	if ((m_scrollctrl & 0x03) == 0x02)
	{
		UINT8 *scrollram = &m_ram[0x1a00];

		m_tilemap[1]->set_scroll_rows(256);
		m_tilemap[1]->set_scroll_cols(1);
		yscroll = m_ram[0x180c];
		m_tilemap[1]->set_scrolly(0, yscroll);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * (offs & 0xfff8) + 0] + 256 * scrollram[2 * (offs & 0xfff8) + 1];
			xscroll -= 6;
			m_tilemap[1]->set_scrollx((offs + yscroll) & 0xff, xscroll);
		}
	}
	else if ((m_scrollctrl & 0x03) == 0x03)
	{
		UINT8 *scrollram = &m_ram[0x1a00];

		m_tilemap[1]->set_scroll_rows(256);
		m_tilemap[1]->set_scroll_cols(1);
		yscroll = m_ram[0x180c];
		m_tilemap[1]->set_scrolly(0, yscroll);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * offs + 0] + 256 * scrollram[2 * offs + 1];
			xscroll -= 6;
			m_tilemap[1]->set_scrollx((offs + yscroll) & 0xff, xscroll);
		}
	}
	else if ((m_scrollctrl & 0x04) == 0x04)
	{
		UINT8 *scrollram = &m_ram[0x1800];

		m_tilemap[1]->set_scroll_rows(1);
		m_tilemap[1]->set_scroll_cols(512);
		xscroll = m_ram[0x1a00] + 256 * m_ram[0x1a01];
		xscroll -= 6;
		m_tilemap[1]->set_scrollx(0, xscroll);
		for (offs = 0; offs < 512; offs++)
		{
			yscroll = scrollram[offs / 8];
			m_tilemap[1]->set_scrolly((offs + xscroll) & 0x1ff, yscroll);
		}
	}
	else
	{
		UINT8 *scrollram = &m_ram[0x1a00];

		m_tilemap[1]->set_scroll_rows(1);
		m_tilemap[1]->set_scroll_cols(1);
		xscroll = scrollram[0] + 256 * scrollram[1];
		xscroll -= 6;
		yscroll = m_ram[0x180c];
		m_tilemap[1]->set_scrollx(0, xscroll);
		m_tilemap[1]->set_scrolly(0, yscroll);
	}

	if ((m_scrollctrl & 0x18) == 0x10)
	{
		UINT8 *scrollram = &m_ram[0x3a00];

		m_tilemap[2]->set_scroll_rows(256);
		m_tilemap[2]->set_scroll_cols(1);
		yscroll = m_ram[0x380c];
		m_tilemap[2]->set_scrolly(0, yscroll);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * (offs & 0xfff8) + 0] + 256 * scrollram[2 * (offs & 0xfff8) + 1];
			xscroll -= 6;
			m_tilemap[2]->set_scrollx((offs + yscroll) & 0xff, xscroll);
		}
	}
	else if ((m_scrollctrl & 0x18) == 0x18)
	{
		UINT8 *scrollram = &m_ram[0x3a00];

		m_tilemap[2]->set_scroll_rows(256);
		m_tilemap[2]->set_scroll_cols(1);
		yscroll = m_ram[0x380c];
		m_tilemap[2]->set_scrolly(0, yscroll);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * offs + 0] + 256 * scrollram[2 * offs + 1];
			xscroll -= 6;
			m_tilemap[2]->set_scrollx((offs + yscroll) & 0xff, xscroll);
		}
	}
	else if ((m_scrollctrl & 0x20) == 0x20)
	{
		UINT8 *scrollram = &m_ram[0x3800];

		m_tilemap[2]->set_scroll_rows(1);
		m_tilemap[2]->set_scroll_cols(512);
		xscroll = m_ram[0x3a00] + 256 * m_ram[0x3a01];
		xscroll -= 6;
		m_tilemap[2]->set_scrollx(0, xscroll);
		for (offs = 0; offs < 512; offs++)
		{
			yscroll = scrollram[offs / 8];
			m_tilemap[2]->set_scrolly((offs + xscroll) & 0x1ff, yscroll);
		}
	}
	else
	{
		UINT8 *scrollram = &m_ram[0x3a00];

		m_tilemap[2]->set_scroll_rows(1);
		m_tilemap[2]->set_scroll_cols(1);
		xscroll = scrollram[0] + 256 * scrollram[1];
		xscroll -= 6;
		yscroll = m_ram[0x380c];
		m_tilemap[2]->set_scrollx(0, xscroll);
		m_tilemap[2]->set_scrolly(0, yscroll);
	}

#if 0
if ((m_scrollctrl & 0x03) == 0x01 ||
		(m_scrollctrl & 0x18) == 0x08 ||
		((m_scrollctrl & 0x04) && (m_scrollctrl & 0x03)) ||
		((m_scrollctrl & 0x20) && (m_scrollctrl & 0x18)) ||
		(m_scrollctrl & 0xc0) != 0)
	popmessage("scrollcontrol = %02x", m_scrollctrl);

if (machine().input().code_pressed(KEYCODE_F))
{
	FILE *fp;
	fp=fopen("TILE.DMP", "w+b");
	if (fp)
	{
		fwrite(m_ram, 0x6000, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
}

void k052109_device::tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, UINT32 flags, UINT8 priority )
{
	m_tilemap[tmap_num]->draw(screen, bitmap, cliprect, flags, priority);
}

int k052109_device::is_irq_enabled( )
{
	return m_irq_enabled;
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/*
  data format:
  video RAM    xxxxxxxx  tile number (low 8 bits)
  color RAM    xxxx----  depends on external connections (usually color and banking)
  color RAM    ----xx--  bank select (0-3): these bits are replaced with the 2
                         bottom bits of the bank register before being placed on
                         the output pins. The other two bits of the bank register are
                         placed on the CAB1 and CAB2 output pins.
  color RAM    ------xx  depends on external connections (usually banking, flip)
*/

void k052109_device::get_tile_info( tile_data &tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram1, UINT8 *vram2 )
{
	int flipy = 0;
	int code = vram1[tile_index] + 256 * vram2[tile_index];
	int color = cram[tile_index];
	int flags = 0;
	int priority = 0;
	int bank = m_charrombank[(color & 0x0c) >> 2];
	if (m_has_extra_video_ram)
		bank = (color & 0x0c) >> 2; /* kludge for X-Men */

	color = (color & 0xf3) | ((bank & 0x03) << 2);
	bank >>= 2;

	flipy = color & 0x02;

	m_k052109_cb(layer, bank, &code, &color, &flags, &priority);

	/* if the callback set flip X but it is not enabled, turn it off */
	if (!(m_tileflip_enable & 1))
		flags &= ~TILE_FLIPX;

	/* if flip Y is enabled and the attribute but is set, turn it on */
	if (flipy && (m_tileflip_enable & 2))
		flags |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(0,
			code,
			color,
			flags);

	tileinfo.category = priority;
}

TILE_GET_INFO_MEMBER(k052109_device::get_tile_info0)
{
	get_tile_info(tileinfo, tile_index, 0, m_colorram_F, m_videoram_F, m_videoram2_F);
}

TILE_GET_INFO_MEMBER(k052109_device::get_tile_info1)
{
	get_tile_info(tileinfo, tile_index, 1, m_colorram_A, m_videoram_A, m_videoram2_A);
}

TILE_GET_INFO_MEMBER(k052109_device::get_tile_info2)
{
	get_tile_info(tileinfo, tile_index, 2, m_colorram_B, m_videoram_B, m_videoram2_B);
}


void k052109_device::tileflip_reset()
{
	int data = m_ram[0x1e80];
	m_tilemap[0]->set_flip((data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_tilemap[1]->set_flip((data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_tilemap[2]->set_flip((data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_tileflip_enable = ((data & 0x06) >> 1);
}
