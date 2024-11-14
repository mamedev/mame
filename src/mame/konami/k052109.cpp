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
- character "color" (COL0-COL7); used for color but also bank switching and tile
  flipping. Exact meaning depends on external connections. COL2 and COL3 select
  the tile bank, and may be replaced with the low 2 bits from the bank register.
  The top 2 bits of the register go to CAB1-CAB2.
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
1000-17ff: layer B tilemap (attributes)
180c-1833: A y scroll
1a00-1bff: A x scroll
1c00     : Maps the three 8kB RAM chips to memory addresses.
            ------xx select the configuration from this table
               RAM0 RAM1 RAM2
            00 A~B  6~7  8~9  Reset state
            01 8~9  4~5  6~7
            10 6~7  2~3  4~5
            11 4~5  0~1  2~3  TMNT setting
            ---xxx-- affects how RAMs are accessed
            -x------
                     0 = replace bits 5:4 of color attribute by bits 1:0
                     1 = do not alter color attribute (gradius3,xmen)
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
           suratk sets this register to 70 during the second boss to produce rotating star field, using X and Y scroll in the same time.
           not emulated due to MAME's tilemaps restrictions, currently handled in hacky way.
           glfgreat sets it to 30 when showing the leader board
           mariorou sets it to 36 when ingame, while actually does per-row scroll for layer A and per-collumn scroll for layer B.
           such usage not supported by current implementation, hacked in game driver instead.
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

The main CPU doesn't have direct access to the RAM used by the 052109; it has
to go through the chip (8 bits at a time, even on 68000-based systems).
*/

#include "emu.h"
#include "k052109.h"

#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(K052109, k052109_device, "k052109", "K052109 Tilemap Generator")

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


k052109_device::k052109_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K052109, tag, owner, clock),
	device_gfx_interface(mconfig, *this, gfxinfo),
	device_video_interface(mconfig, *this, false),
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
	m_dx(0),
	m_dy(0),
	m_char_rom(*this, DEVICE_SELF),
	m_k052109_cb(*this),
	m_irq_handler(*this),
	m_firq_handler(*this),
	m_nmi_handler(*this)
{
}


void k052109_device::set_char_ram(bool ram)
{
	if (ram)
		set_info(gfxinfo_ram);
	else
		set_info(gfxinfo);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k052109_device::device_start()
{
	// assumes it can make an address mask with m_char_rom.length() - 1
	assert(!m_char_rom.found() || !(m_char_rom.length() & (m_char_rom.length() - 1)));

	if (has_screen())
	{
		// make sure our screen is started
		if (!screen().started())
			throw device_missing_dependencies();

		// and register a callback for vblank state
		screen().register_vblank_callback(vblank_state_delegate(&k052109_device::vblank_callback, this));
	}

	// resolve delegates
	m_k052109_cb.resolve();

	decode_gfx();
	gfx(0)->set_colors(palette().entries() / gfx(0)->depth());

	m_ram = make_unique_clear<uint8_t[]>(0x6000);

	m_colorram_F = &m_ram[0x0000];
	m_colorram_A = &m_ram[0x0800];
	m_colorram_B = &m_ram[0x1000];
	m_videoram_F = &m_ram[0x2000];
	m_videoram_A = &m_ram[0x2800];
	m_videoram_B = &m_ram[0x3000];
	m_videoram2_F = &m_ram[0x4000];
	m_videoram2_A = &m_ram[0x4800];
	m_videoram2_B = &m_ram[0x5000];

	m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k052109_device::get_tile_info0)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k052109_device::get_tile_info1)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k052109_device::get_tile_info2)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);

	m_tilemap[0]->set_scrolldx(m_dx, m_dx);
	m_tilemap[1]->set_scrolldx(m_dx+6, m_dx+6);
	m_tilemap[2]->set_scrolldx(m_dx+6, m_dx+6);
	m_tilemap[0]->set_scrolldy(m_dy, m_dy);
	m_tilemap[1]->set_scrolldy(m_dy, m_dy);
	m_tilemap[2]->set_scrolldy(m_dy, m_dy);

	save_pointer(NAME(m_ram), 0x6000);
	save_item(NAME(m_rmrd_line));
	save_item(NAME(m_romsubbank));
	save_item(NAME(m_scrollctrl));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_charrombank));
	save_item(NAME(m_charrombank_2));
	save_item(NAME(m_addrmap));
	save_item(NAME(m_has_extra_video_ram));
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
	m_addrmap    = 0;
	m_has_extra_video_ram = 0;

	for (int i = 0; i < 4; i++)
	{
		m_charrombank[i] = 0;
		m_charrombank_2[i] = 0;
	}
}

//-------------------------------------------------
//  device_post_load - device-specific postload
//-------------------------------------------------

void k052109_device::device_post_load()
{
	tileflip_reset();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k052109_device::set_xy_offset(int dx, int dy)
{
	m_dx = dx;
	m_dy = dy;
}

void k052109_device::vblank_callback(screen_device &screen, bool state)
{
	if (state && m_irq_enabled)
		m_irq_handler(ASSERT_LINE);
}

u8 k052109_device::read(offs_t offset)
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
			//else logerror("%s: read from unknown 052109 address %04x\n",machine().describe_context(),offset);
		}

		return m_ram[offset];
	}
	else    /* Punk Shot and TMNT read from 0000-1fff, Aliens from 2000-3fff */
	{
		assert (m_char_rom.found());

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
		addr &= m_char_rom.length() - 1;

		//logerror("%s: off = %04x sub = %02x (bnk = %x) adr = %06x\n", machine().describe_context(), offset, m_romsubbank, bank, addr);

		return m_char_rom[addr];
	}
}

void k052109_device::write(offs_t offset, u8 data)
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
		else if (offset == 0x1c00)
		{
			m_addrmap = data;
		}
		else if (offset == 0x1c80)
		{
			if (m_scrollctrl != data)
			{
				//popmessage("scrollcontrol = %02x", data);
				//logerror("%s: rowscrollcontrol = %02x\n", machine().describe_context(), data);
				m_scrollctrl = data;
			}
		}
		else if (offset == 0x1d00)
		{
			//logerror("%s: 052109 register 1d00 = %02x\n", machine().describe_context(), data);
			/* bit 2 = irq enable */
			/* the custom chip can also generate NMI and FIRQ, for use with a 6809 */
			m_irq_enabled = data & 0x04;
			if (!m_irq_enabled)
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
			//logerror("%s: 052109 register 1e00 = %02x\n",machine().describe_context(),data);
			m_romsubbank = data;
		}
		else if (offset == 0x1e80)
		{
			//if ((data & 0xfe)) logerror("%s: 052109 register 1e80 = %02x\n",machine().describe_context(),data);
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
		//else logerror("%s: write %02x to unknown 052109 address %04x\n",machine().describe_context(),data,offset);
	}
}

void k052109_device::set_rmrd_line( int state )
{
	m_rmrd_line = state;
}

int k052109_device::get_rmrd_line( )
{
	return m_rmrd_line;
}


void k052109_device::tilemap_update( )
{
	int xscroll, yscroll, offs;

#if 0
	popmessage("%x %x %x %x",
			m_charrombank[0],
			m_charrombank[1],
			m_charrombank[2],
			m_charrombank[3]);
	//popmessage("%x",m_addrmap);
#endif

	// note: this chip can do both per-column and per-row scroll in the same time, currently not emulated.

	if ((m_scrollctrl & 0x03) == 0x02)
	{
		uint8_t *scrollram = &m_ram[0x1a00];

		m_tilemap[1]->set_scroll_rows(32);
		m_tilemap[1]->set_scroll_cols(1);
		yscroll = m_ram[0x180c];
		m_tilemap[1]->set_scrolly(0, yscroll);
		yscroll /= 8;
		for (offs = 0; offs < 32; offs++)
		{
			xscroll = scrollram[16 * offs + 0] + 256 * scrollram[16 * offs + 1];
			m_tilemap[1]->set_scrollx((offs + yscroll) & 31, xscroll);
		}
	}
	else if ((m_scrollctrl & 0x03) == 0x03)
	{
		uint8_t *scrollram = &m_ram[0x1a00];

		m_tilemap[1]->set_scroll_rows(256);
		m_tilemap[1]->set_scroll_cols(1);
		yscroll = m_ram[0x180c];
		m_tilemap[1]->set_scrolly(0, yscroll);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * offs + 0] + 256 * scrollram[2 * offs + 1];
			m_tilemap[1]->set_scrollx((offs + yscroll) & 0xff, xscroll);
		}
	}
	else if ((m_scrollctrl & 0x04) == 0x04)
	{
		uint8_t *scrollram = &m_ram[0x1800];

		m_tilemap[1]->set_scroll_rows(1);
		m_tilemap[1]->set_scroll_cols(64);
		xscroll = m_ram[0x1a00] + 256 * m_ram[0x1a01];
		m_tilemap[1]->set_scrollx(0, xscroll);
		xscroll /= 8;
		for (offs = 0; offs < 64; offs++)
		{
			yscroll = scrollram[offs];
			m_tilemap[1]->set_scrolly((offs + xscroll) & 63, yscroll);
		}
	}
	else
	{
		uint8_t *scrollram = &m_ram[0x1a00];

		m_tilemap[1]->set_scroll_rows(1);
		m_tilemap[1]->set_scroll_cols(1);
		xscroll = scrollram[0] + 256 * scrollram[1];
		yscroll = m_ram[0x180c];
		m_tilemap[1]->set_scrollx(0, xscroll);
		m_tilemap[1]->set_scrolly(0, yscroll);
	}

	if ((m_scrollctrl & 0x18) == 0x10)
	{
		uint8_t *scrollram = &m_ram[0x3a00];

		m_tilemap[2]->set_scroll_rows(32);
		m_tilemap[2]->set_scroll_cols(1);
		yscroll = m_ram[0x380c];
		//
		if (m_scrollctrl == 0x70) yscroll = m_ram[0x3823]; // hack for suratk 2nd boss rotating star field
		//
		m_tilemap[2]->set_scrolly(0, yscroll);
		yscroll /= 8;
		for (offs = 0; offs < 32; offs++)
		{
			xscroll = scrollram[16 * offs + 0] + 256 * scrollram[16 * offs + 1];
			m_tilemap[2]->set_scrollx((offs + yscroll) & 31, xscroll);
		}
	}
	else if ((m_scrollctrl & 0x18) == 0x18)
	{
		uint8_t *scrollram = &m_ram[0x3a00];

		m_tilemap[2]->set_scroll_rows(256);
		m_tilemap[2]->set_scroll_cols(1);
		yscroll = m_ram[0x380c];
		m_tilemap[2]->set_scrolly(0, yscroll);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * offs + 0] + 256 * scrollram[2 * offs + 1];
			m_tilemap[2]->set_scrollx((offs + yscroll) & 0xff, xscroll);
		}
	}
	else if ((m_scrollctrl & 0x20) == 0x20)
	{
		uint8_t *scrollram = &m_ram[0x3800];

		m_tilemap[2]->set_scroll_rows(1);
		m_tilemap[2]->set_scroll_cols(64);
		xscroll = m_ram[0x3a00] + 256 * m_ram[0x3a01];
		m_tilemap[2]->set_scrollx(0, xscroll);
		xscroll /= 8;
		for (offs = 0; offs < 64; offs++)
		{
			yscroll = scrollram[offs];
			m_tilemap[2]->set_scrolly((offs + xscroll) & 63, yscroll);
		}
	}
	else
	{
		uint8_t *scrollram = &m_ram[0x3a00];

		m_tilemap[2]->set_scroll_rows(1);
		m_tilemap[2]->set_scroll_cols(1);
		xscroll = scrollram[0] + 256 * scrollram[1];
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

void k052109_device::tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, uint32_t flags, uint8_t priority )
{
	m_tilemap[tmap_num]->draw(screen, bitmap, cliprect, flags, priority);
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

void k052109_device::get_tile_info( tile_data &tileinfo, int tile_index, int layer, uint8_t *cram, uint8_t *vram1, uint8_t *vram2 )
{
	int flipy = 0;
	int code = vram1[tile_index] + 256 * vram2[tile_index];
	int color = cram[tile_index];
	int flags = 0;
	int priority = 0;
	int bank = m_charrombank[(color & 0x0c) >> 2];
	if (!BIT(m_addrmap,6))
	{
		color = (color & 0xf3) | ((bank & 0x03) << 2);
	}

	bank >>= 2;

	flipy = color & 0x02;

	m_k052109_cb(layer, bank, &code, &color, &flags, &priority);

	/* if the callback set flip X but it is not enabled, turn it off */
	if (!(m_tileflip_enable & 1))
		flags &= ~TILE_FLIPX;

	/* if flip Y is enabled and the attribute but is set, turn it on */
	if (flipy && (m_tileflip_enable & 2))
		flags |= TILE_FLIPY;

	tileinfo.set(0,
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
