// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
/***************************************************************************

    Round Up 5                                          ATC-011

    TODO:

    - Finish road layer.
      Tunnel sections are borderline unplayable, plus slopes are ugly to watch.

    - Always boots with a coin inserted
      $5152 is the coin counter, gets an explicit 1 at boot.
      There are other two buffers read from 68k before that, written to $5156 and $515a
      If these are 0xffff by then game boots normally ...

***************************************************************************/

#include "emu.h"
#include "tatsumi.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"

#include "screen.h"
#include "speaker.h"

#include "roundup5.lh"

namespace {

class roundup5_state : public tatsumi_state
{
public:
	roundup5_state(const machine_config &mconfig, device_type type, const char *tag)
		: tatsumi_state(mconfig, type, tag)
		, m_vregs(*this, "vregs")
		, m_bg_scrollx(*this, "bg_scrollx")
		, m_bg_scrolly(*this, "bg_scrolly")
		, m_road_ctrl_ram(*this, "road_ctrl_ram")
		, m_road_pixel_ram(*this, "road_pixel_ram")
		, m_road_color_ram(*this, "road_color_ram")
		, m_road_yclip(*this, "road_yclip")
		, m_road_vregs(*this, "road_vregs")
	{
	}

	void roundup5(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	uint16_t roundup_v30_z80_r(offs_t offset);
	void roundup_v30_z80_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void roundup5_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void road_vregs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t gfxdata_r(offs_t offset);
	void gfxdata_w(offs_t offset, uint8_t data);
	void output_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void roundup5_68000_map(address_map &map) ATTR_COLD;
	void roundup5_v30_map(address_map &map) ATTR_COLD;
	void roundup5_z80_map(address_map &map) ATTR_COLD;

	void draw_road(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_landscape(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t type);

	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_bg_scrollx;
	required_shared_ptr<uint16_t> m_bg_scrolly;
	required_shared_ptr<uint16_t> m_road_ctrl_ram;
	required_shared_ptr<uint16_t> m_road_pixel_ram;
	required_shared_ptr<uint16_t> m_road_color_ram;
	required_shared_ptr<uint16_t> m_road_yclip;
	required_shared_ptr<uint16_t> m_road_vregs;

	std::unique_ptr<uint8_t[]> m_tx_gfxram;
	std::unique_ptr<uint8_t[]> m_bg_gfxram;
};


uint8_t roundup5_state::gfxdata_r(offs_t offset)
{
	if((m_control_word & 0x200) == 0x200)
	{
		offset += (m_control_word & 0x6000) << 2;

		return m_bg_gfxram[offset];
	}

	offset+=((m_control_word&0x0c00)>>10) * 0x8000;
	return m_tx_gfxram[offset];
}

void roundup5_state::gfxdata_w(offs_t offset, uint8_t data)
{
	if((m_control_word & 0x200) == 0x200)
	{
		offset += (m_control_word & 0x6000) << 2;
		m_bg_gfxram[offset] = data;
		return;
	}

	offset+=((m_control_word&0x0c00)>>10) * 0x8000;

	if (offset>=0x18000 && data)
		logerror("effective write to vram %06x %02x (control %04x)\n",offset,data,m_control_word);

	m_tx_gfxram[offset] = data;

	offset=offset%0x8000;

	m_gfxdecode->gfx(0)->mark_dirty(offset/8);
}

void roundup5_state::video_start()
{
	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(roundup5_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 128,64);
	m_tx_gfxram = std::make_unique<uint8_t[]>(0x20000);
	m_bg_gfxram = std::make_unique<uint8_t[]>(0x20000);

	m_tx_layer->set_transparent_pen(0);

	m_gfxdecode->gfx(0)->set_source(m_tx_gfxram.get());

	save_pointer(NAME(m_tx_gfxram), 0x20000);
	save_pointer(NAME(m_bg_gfxram), 0x20000);
}

void roundup5_state::draw_road(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
/*
0xf980 0x0008 0x8c80 0x4a00 - road right to below, width unknown (32 pixels guess)
0xfa80 0x0008 0x8c80 0x4a00 - road right to below, width unknown (32 pixels guess)

0xfb80 0x0008 0x8c80 0x4a00 - road in middle of screen, width unknown (32 pixels guess)

0xfc80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xfd80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xfe80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xff80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0x0001                      - road half/width above to left, (ie, xpos - 16)
0x0081                      - road width to left as usual (xpos-16 from above, or 32 from above2)

0xfb0b 0x210b 0x8cf5 0x0dea - blue & left & right, with  blue|----|----|----|----|blue
in this mode changing columns 2 & 3 have no apparent effect
0xfb0b 0x7b09 0x8cf5 0x0dea - as above, but scaled up - perhaps 18 pixels shifted (twice that overall size)
0xfb0b 0x6c07 0x8cf5 0x0dea - as above, but scaled up - perhaps 40 pixels shifted from above
0xfb0b 0xaa06 0x8cf5 0x0dea - as above, but scaled up - perhaps 16 pixels shifted from above
0xfb0b 0xb005 0x8cf5 0x0dea - as above, but scaled up - perhaps 38 pixels shifted from above

b21 diff is 1a6
97b         20f
76c         c2
6aa         fa
5b0


0x0000 0x0008 0xxxxx 0xxxx - line starting at 0 for 128 pixels - 1 to 1 with road pixel data
0xff00 0x0008 0xxxxx 0xxxx - line starting at 32 for 128 pixels - 1 to 1 with road pixel data
0xfe00 0x0008 0xxxxx 0xxxx - line starting at 64 for 128 pixels - 1 to 1 with road pixel data



at standard zoom (0x800)
shift of 0x100 moves 32 pixels
so shift of 8 is assumed to move 1 pixel

at double zoom (0x1000)
assume shift of 0x100 only moves 16 pixels
so

0x100 * 0x400 => 0x40
0x100 * step 0x800 = must come out at 0x20
0x100 * step 0x1000 = must come out at 0x10
0x100 * step 0x2000 = 0x5

pos is 11.5 fixed point

-0x580 = middle
-0x180
-0x080
0
0x80

*/
	const uint16_t *data = m_road_ctrl_ram;

	// Road layer enable (?)
	if ((m_vregs[0x1]&0x1)==0)
		return;

	// Road data bank select (double buffered)
	if (m_road_vregs[0]&0x10)
		data+=0x400;

	// Apply clipping: global screen + local road y offsets
	int y = 256 - ((m_vregs[0xa/2] >> 8) + m_road_yclip[0]);
	data+=y*4;

	int visible_line=0;

	for ( ; y<cliprect.max_y+1; y++)
	{
		// TODO: tunnels road drawing has a different format?
		// shift is always 0x88** while data[3] is a variable argument with bit 15 always on
		int shift=data[0];
		int shift2=data[2];
		int pal = 4; //(data[3]>>8)&0xf;
		int step = swapendian_int16(data[1]);
		int samplePos=0;
		uint16_t const *const linedata=m_road_pixel_ram;// + (0x100 * pal);
		int startPos=0, endPos=0;

		int palette_byte;//=m_road_color_ram[visible_line/8];

		/*
		    Each road line consists of up to two sets of 128 pixel data that can be positioned
		    on the x-axis and stretched/compressed on the x-axis.  Any screen pixels to the left
		    of the first set are drawn with pen 0 of the road pixel data.  Any screen pixels to the
		    right of the second set line are drawn with pen 127 of the road pixel data.

		    The road control data is laid out as follows (4 words per screen line, with 2 banks):

		    Word 0: Line shift for 1st set - 13.3 signed fixed point value.
		    Word 1: Line scale - 5.11 fixed point value.  So 0x800 is 1:1, 0x400 is 1:2, etc
		    Word 2: Line shift for 2nd set - 13.3 signed fixed point value.
		    Word 3: ?

		    The scale is shared between both pixel sets.  The 2nd set is only used when the road
		    forks into two between stages.  The 2nd line shift is an offset from the last pixel
		    of the 1st set.  The 2nd line shift uses a different palette bank.

2nd road uses upper palette - confirmed by water stage.
offset is from last pixel of first road segment?
//last pixel of first road is really colour from 2nd road line?

		*/

		palette_byte=m_road_color_ram[visible_line/8];
		pal = 4 + ((palette_byte>>(visible_line%8))&1);

		visible_line++;

		if (shift&0x8000)
			shift=-(0x10000 - shift);
		shift=-shift;

		if (step)
			startPos=((shift<<8) + 0x80 )/ step;

		int x;

		/* Fill in left of road segment */
		for (x=0; (x < startPos) && (x < cliprect.max_x+1); x++)
		{
			int col = linedata[0]&0xf;
			bitmap.pix(y, x) = m_palette->pen(256 + pal*16 + col);
		}

		/* If startpos is negative, clip it and adjust the sampling position accordingly */
		if (startPos<0)
		{
			samplePos=step*(0-startPos);
			startPos=0;
		}
		else
		{
			samplePos=0;
		}

		/* Fill in main part of road, then right-hand side edge */
		for (x=startPos; x < (cliprect.max_x + 1) && ((samplePos>>11)<0x80); x++)
		{
			// look up colour
			int col = linedata[(samplePos>>11)&0x7f]&0xf;

			/* Clamp if we have reached the end of the pixel data */
			//if ((samplePos>>11) > 0x7f)
			//  col=linedata[0x7f]&0xf;

			bitmap.pix(y, x) = m_palette->pen(256 + pal*16 + col);

			samplePos+=step;
		}

		/* Now work out how many pixels until start of 2nd segment */
		startPos=x;

		if (shift2&0x8000)
			shift2=-(0x10000 - shift2);
		shift2=-shift2;

		if (step)
			endPos=((shift2<<8) + 0x80) / step;
		else
			endPos=0;
		endPos-=128;
		endPos=startPos+endPos;

		/* Fill pixels */
		for (x=startPos; x < (cliprect.max_x+1) && (x < endPos); x++)
		{
			int col = linedata[0x80]&0xf;

			/* Clamp if we have reached the end of the pixel data */
			//if ((samplePos>>11) > 0x7f)
			//  col=linedata[0x7f]&0xf;

			bitmap.pix(y, x) = m_palette->pen(256 + pal*16 + col + 32);
		}

		if (endPos<0)
		{
			// end of left intersection (taking right turn)
			samplePos=step*(0-startPos);
		}
		else if (endPos<x)
		{
			// start of right intersection
			samplePos=step*(x-endPos);
		}
		else
		{
			// end of right intersection (taking right turn)
			samplePos=0; // todo
		}

		for (/*x=endPos*/; x < cliprect.max_x+1; x++)
		{
			// look up colour
			int col = linedata[((samplePos>>11)&0x7f) + 0x200]&0xf;

			/* Clamp if we have reached the end of the pixel data */
			if ((samplePos>>11) > 0x7f)
				col=linedata[0x7f + 0x200]&0xf;

			bitmap.pix(y, x) = m_palette->pen(256 + pal*16 + col + 32);

			samplePos+=step;
		}
		data+=4;
	}
}

// background layer landscape for Round Up 5
// two bitmap layers, back layer is 512 x 128, the other one is 512 x 64
// it's safe to assume that three monitor version will have a different arrangement here ...
void roundup5_state::draw_landscape(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t type)
{
	// TODO: guess, assume back layer having less scroll increment than front for parallax scrolling.
	// also notice that m_vregs[8/2] >> 8 is identical to [0x0c/2], always?
	uint16_t x_base = type ? m_bg_scrollx[0] : m_vregs[0xc/2];
	// TODO: maybe [0xa/2] applies here as well?
	uint16_t y_base = m_bg_scrolly[0] & 0x1ff;
	uint16_t y_scroll = 0x180 - y_base;
	uint32_t base_offset;
	uint16_t color_base = type ? 0x100 : 0x110;
	int ysize = type ? 64 : 128;

	base_offset = 0x10000 + type * 0x8000;
	if(type)
		y_scroll += 64;

	//popmessage("%04x %04x %04x",m_vregs[8/2],m_vregs[0xc/2],m_bg_scrollx[0]);

	for(int y = 0; y < ysize; y++)
	{
		for(int x = 0; x < 512; x++)
		{
			int res_x = (x_base + x) & 0x1ff;
			uint32_t color = m_bg_gfxram[(res_x >> 1)+y*256+base_offset];

			if(res_x & 1)
				color >>= 4;

			color &= 0xf;

			if(cliprect.contains(x, y+y_scroll) && color)
				bitmap.pix(y+y_scroll, x) = m_palette->pen(color+color_base);
		}
	}
}

uint32_t roundup5_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int tx_start_addr;

	tx_start_addr = (m_hd6445_reg[0xc] << 8) | (m_hd6445_reg[0xd]);
	tx_start_addr &= 0x3fff;

	m_sprites->update_cluts();

	m_tx_layer->set_scrollx(0,24);
	m_tx_layer->set_scrolly(0,(tx_start_addr >> 4) | m_hd6445_reg[0x1d]);

	bitmap.fill(m_palette->pen(384), cliprect); // todo
	screen.priority().fill(0, cliprect);
	m_sprites->draw_sprites(screen.priority(),cliprect,1,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Alpha pass only
	draw_landscape(bitmap,cliprect,0);
	draw_landscape(bitmap,cliprect,1);
	draw_road(bitmap,cliprect);
	apply_shadow_bitmap(bitmap,cliprect,screen.priority(), 0);
	if(m_control_word & 0x80) // enabled on map screen after a play
	{
		m_tx_layer->draw(screen, bitmap, cliprect, 0,0);
		m_sprites->draw_sprites(bitmap,cliprect,0,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Full pass
	}
	else
	{
		m_sprites->draw_sprites(bitmap,cliprect,0,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Full pass
		m_tx_layer->draw(screen, bitmap, cliprect, 0,0);
	}
	return 0;
}

uint16_t roundup5_state::roundup_v30_z80_r(offs_t offset)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	/* Each Z80 byte maps to a V30 word */
	if (m_control_word & 0x20)
		offset += 0x8000; /* Upper half */

	return 0xff00 | targetspace.read_byte(offset);
}

void roundup5_state::roundup_v30_z80_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	/* Only 8 bits of the V30 data bus are connected - ignore writes to the other half */
	if (ACCESSING_BITS_0_7)
	{
		if (m_control_word & 0x20)
			offset += 0x8000; /* Upper half of Z80 address space */

		targetspace.write_byte(offset, data & 0xff);
	}
}


void roundup5_state::roundup5_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_control_word);

	if (m_control_word & 0x10)
		m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	else
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	if (m_control_word & 0x4)
		m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	else
		m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

//  if (offset == 1 && (tatsumi_control_w & 0xfeff) != (last_bank & 0xfeff))
//      logerror("%s:  Changed bank to %04x (%d)\n", m_maincpu->pc(), tatsumi_control_w,offset);

//todo - watchdog

	/* Bank:

	    0x0017  :   OBJ banks
	    0x0018  :   68000 RAM       mask 0x0380 used to save bits when writing
	    0x0c10  :   68000 ROM

	    0x0040  :   Z80 rom (lower half) mapped to 0x10000
	    0x0060  :   Z80 rom (upper half) mapped to 0x10000

	    0x0080  :   enabled when showing map screen after a play
	                (switches video priority between text layer and sprites)

	    0x0100  :   watchdog.

	    0x0c00  :   vram bank (bits 0x7000 also set when writing vram)

	    0x8000  :   set whenever writing to palette ram?

	    Changed bank to 0060 (0)
	*/

	if ((m_control_word & 0x8) == 0 && !(m_last_control & 0x8))
		m_subcpu->set_input_line(INPUT_LINE_IRQ4, ASSERT_LINE);
//  if (tatsumi_control_w&0x200)
//      cpu_set_reset_line(1, CLEAR_LINE);
//  else
//      cpu_set_reset_line(1, ASSERT_LINE);

//  if ((tatsumi_control_w&0x200) && (last_bank&0x200)==0)
//      logerror("68k irq\n");
//  if ((tatsumi_control_w&0x200)==0 && (last_bank&0x200)==0x200)
//      logerror("68k reset\n");

	if (m_control_word == 0x3a00)
	{
//      cpu_set_reset_line(1, CLEAR_LINE);
//      logerror("68k on\n");
	}

	m_last_control = m_control_word;
}

void roundup5_state::road_vregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	    ---- ---x ---- ---- enabled when there's a road slope of any kind, unknown purpose
	    ---- ---- -xx- ---- enables alternatively in tunnels sometimes, color mods?
	    ---- ---- ---x ---- road bank select
	    ---- ---- ---- xxxx various values written during POST while accessing road pixel ram,
	                        otherwise 0xb at the start of irq service
	*/

	COMBINE_DATA(&m_road_vregs[offset]);

	m_subcpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE); // guess, probably wrong
//  logerror("d_68k_e0000_w %s %04x\n", m_maincpu->pc(), data);
}

void roundup5_state::output_w(uint8_t data)
{
	/*
	    ---- x--- depending on Output Mode dipswitch:
	              A Mode: enables when police siren is on
	              B Mode: enables when player collides with objects or go offroad
	    ---- -x-- start button light
	    ---- --xx coin counters
	*/
	// avoid spurious write to coin counters
	if(data == 0xff)
		return;

	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);

	if(data & 0xf0)
		logerror("output_w = %02x\n",data);
}


void roundup5_state::roundup5_v30_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x08000, 0x0bfff).ram().w(FUNC(roundup5_state::text_w)).share("videoram");
	map(0x0c000, 0x0c003).w(FUNC(roundup5_state::hd6445_crt_w)).umask16(0x00ff);
	map(0x0d000, 0x0d001).portr("DSW");
	map(0x0d400, 0x0d40f).ram().share("vregs");
	map(0x0d800, 0x0d801).writeonly().share("bg_scrollx");
	map(0x0dc00, 0x0dc01).writeonly().share("bg_scrolly");
	map(0x0e000, 0x0e001).w(FUNC(roundup5_state::roundup5_control_w));
	map(0x0f000, 0x0ffff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x10000, 0x1ffff).rw(FUNC(roundup5_state::roundup_v30_z80_r), FUNC(roundup5_state::roundup_v30_z80_w));
	map(0x20000, 0x2ffff).rw(FUNC(roundup5_state::tatsumi_v30_68000_r), FUNC(roundup5_state::tatsumi_v30_68000_w));
	map(0x30000, 0x3ffff).rw(FUNC(roundup5_state::gfxdata_r), FUNC(roundup5_state::gfxdata_w)).umask16(0x00ff);
	map(0x80000, 0xfffff).rom().region("master_rom", 0);
}

void roundup5_state::roundup5_68000_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("slave_rom", 0);
	map(0x80000, 0x83fff).ram().share("sharedram");
	map(0x90000, 0x93fff).ram().share("spriteram");
	map(0x9a000, 0x9a1ff).rw(FUNC(roundup5_state::tatsumi_sprite_control_r), FUNC(roundup5_state::tatsumi_sprite_control_w)).share("obj_ctrl_ram");
	map(0xa0000, 0xa0fff).ram().share("road_ctrl_ram"); // Road control data
	map(0xb0000, 0xb0fff).ram().share("road_pixel_ram"); // Road pixel data
	map(0xc0000, 0xc0fff).ram().share("road_color_ram"); // Road colour data
	map(0xd0000, 0xd0001).ram().share("road_yclip");
	map(0xe0000, 0xe0001).ram().w(FUNC(roundup5_state::road_vregs_w)).share("road_vregs");
}

void roundup5_state::roundup5_z80_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xffef).ram();
	map(0xfff0, 0xfff1).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xfff4, 0xfff4).r(m_oki, FUNC(okim6295_device::read)).w(m_oki, FUNC(okim6295_device::write));
	map(0xfff8, 0xfffb).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfffc, 0xfffc).portr("STICKX");
}


static INPUT_PORTS_START( roundup5 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Accelerator")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Brake")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Shift") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Turbo")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	// Tested in service mode, probably unused
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-3:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-3:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-3:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("DSW") /* Verified by Manual & in Game service menu */
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW-1:1,2")
	PORT_DIPSETTING(      0x0003, "Shortest" )
	PORT_DIPSETTING(      0x0002, "Short" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0001, "Long" )
	PORT_DIPNAME( 0x000c, 0x0000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW-1:3,4")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW-1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW-1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Stage 5 Continue" )  PORT_DIPLOCATION("SW-1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Output Mode" )       PORT_DIPLOCATION("SW-1:8")
	PORT_DIPSETTING(      0x0000, "A (Light)" )
	PORT_DIPSETTING(      0x0080, "B (Vibration)" )
	// TODO: Coinage was all wrong, maybe manual refers to an undumped version?
	PORT_DIPNAME( 0x0700, 0x0000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW-2:1,2,3")
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x0000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW-2:4,5,6")
	PORT_DIPSETTING(      0x3000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-2:7") /* Manual only shows nothing for this one */
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// putting this and sw-2:6 ON position after POST to enable debugging info
	PORT_DIPNAME( 0x8000, 0x8000, "Hardware Test Mode" ) PORT_DIPLOCATION("SW-2:8") /* Manual only shows nothing for this one */
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout roundup5_vramlayout =
{
	8,8,
	4096,
	3,
	{ 0x10000 * 8, 0x8000 * 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_roundup5 )
	GFXDECODE_RAM(   nullptr,   0, roundup5_vramlayout, 0,  16)
GFXDECODE_END


void roundup5_state::roundup5(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, roundup5_state::CLOCK_1 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &roundup5_state::roundup5_v30_map);
	m_maincpu->set_vblank_int("screen", FUNC(roundup5_state::v30_interrupt));

	M68000(config, m_subcpu, roundup5_state::CLOCK_2 / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &roundup5_state::roundup5_68000_map);

	Z80(config, m_audiocpu, roundup5_state::CLOCK_1 / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &roundup5_state::roundup5_z80_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.out_pc_callback().set(FUNC(roundup5_state::output_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(roundup5_state::CLOCK_2 / 8, 400, 0, 320, 272, 0, 240); // TODO: Hook up CRTC
	screen.set_screen_update(FUNC(roundup5_state::screen_update));

	TZB315_SPRITES(config, m_sprites, 0, 0x800); // confirmed TZB315, even if it has the smaller CLUT like Apache 3 / TZB215
	m_sprites->set_sprite_palette_base(512);
	m_sprites->set_palette("sprites:palette_clut");
	m_sprites->set_basepalette(m_palette);
	m_sprites->set_spriteram(m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_roundup5);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024); // 1024 real colours
	m_palette->set_membits(8).set_endianness(ENDIANNESS_BIG);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, roundup5_state::CLOCK_1 / 4);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	m_ym2151->add_route(0, "speaker", 0.45, 0);
	m_ym2151->add_route(1, "speaker", 0.45, 1);

	OKIM6295(config, m_oki, roundup5_state::CLOCK_1 / 4 / 2, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.75, 0);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.75, 1);
}


ROM_START( roundup5 )
	ROM_REGION16_LE( 0x80000, "master_rom", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ru-23s",   0x000000, 0x20000, CRC(2dc8c521) SHA1(b78de101db3ef00fc4375ae32a7871e0da2dac6c) )
	ROM_LOAD16_BYTE( "ru-26s",   0x000001, 0x20000, CRC(1e16b531) SHA1(d7badef29cf1c4a9bd262933ecd1ca3343ea94bd) )
	ROM_LOAD16_BYTE( "ru-22t",   0x040000, 0x20000, CRC(9611382e) SHA1(c99258782dbad6d69ba7f54115ee3aa218f9b6ee) )
	ROM_LOAD16_BYTE( "ru-25t",   0x040001, 0x20000, CRC(b6cd0f2d) SHA1(61925c2346d79baaf9bce3d19a7dfc45b8232f92) )

	ROM_REGION16_BE( 0x80000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "ru-20s",   0x000000, 0x20000, CRC(c5524558) SHA1(a94e7e4548148c83a332524ab4e06607732e13d5) )
	ROM_LOAD16_BYTE( "ru-18s",   0x000001, 0x20000, CRC(163ef03d) SHA1(099ac2d74164bdc6402b08efb521f49275780858) )
	ROM_LOAD16_BYTE( "ru-21s",   0x040000, 0x20000, CRC(b9f91b70) SHA1(43c5d9dafb60ed3e5c3eb0e612c2dbc5497f8a6c) )
	ROM_LOAD16_BYTE( "ru-19s",   0x040001, 0x20000, CRC(e3953800) SHA1(28fbc6bf154b512fcefeb04fe12db598b1b20cfe) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "ru-28d",   0x000000, 0x10000, CRC(df36c6c5) SHA1(c046482043f6b54c55696ba3d339ffb11d78f674) )

	ROM_REGION( 0x0c0000, "sprites:sprites_l", 0)
	ROM_LOAD32_BYTE( "ru-00b",   0x000000, 0x20000, CRC(388a0647) SHA1(e4ab43832872f44c0fe1aaede4372cc00ca7d32b) )
	ROM_LOAD32_BYTE( "ru-02b",   0x000001, 0x20000, CRC(eff33945) SHA1(3f4c3aaa11ccf945c2f898dfdf815705d8539e21) )
	ROM_LOAD32_BYTE( "ru-04b",   0x000002, 0x20000, CRC(40fda247) SHA1(f5fbc07fda024baedf35ac209210e94df9f15065) )
	ROM_LOAD32_BYTE( "ru-06b",   0x000003, 0x20000, CRC(cd2484f3) SHA1(a23a4d36a8b913104bcc75228317b2979afec888) )
	ROM_LOAD32_BYTE( "ru-01b",   0x080000, 0x10000, CRC(5e91f401) SHA1(df976c5ba0f14b14f5642b5ca35b996bca64e369) )
	ROM_LOAD32_BYTE( "ru-03b",   0x080001, 0x10000, CRC(2fb109de) SHA1(098c103e6bae0f52ec66f0cdda2da60bd7108736) )
	ROM_LOAD32_BYTE( "ru-05b",   0x080002, 0x10000, CRC(23dd10e1) SHA1(f30ff1a8c7ed9bc567b901cbdd202028fffb9f80) )
	ROM_LOAD32_BYTE( "ru-07b",   0x080003, 0x10000, CRC(bb40f46e) SHA1(da694e16d19f60a0dee47551f00f3e50b2d5dcaf) )

	ROM_REGION( 0x0c0000, "sprites:sprites_h", 0)
	ROM_LOAD32_BYTE( "ru-08b",   0x000000, 0x20000, CRC(01729e3c) SHA1(1445287fde0b993d053aab73efafc902a6b7e2cc) )
	ROM_LOAD32_BYTE( "ru-10b",   0x000001, 0x20000, CRC(cd2357a7) SHA1(313460a74244325ce2c659816f2b738f3dc5358a) )
	ROM_LOAD32_BYTE( "ru-12b",   0x000002, 0x20000, CRC(ca63b1f8) SHA1(a50ef8259745dc166eb0a1b2c812ff620818a755) )
	ROM_LOAD32_BYTE( "ru-14b",   0x000003, 0x20000, CRC(dde79bfc) SHA1(2d5888189a6f954801f248a3365e328370fed837) )
	ROM_LOAD32_BYTE( "ru-09b",   0x080000, 0x10000, CRC(629ac0a6) SHA1(c3eeccd6c07be7455cf180c9c7d5efcd6d08c0b5) )
	ROM_LOAD32_BYTE( "ru-11b",   0x080001, 0x10000, CRC(fe3fbf53) SHA1(7400c088025ac22e5d9db816792533fc02f2dcf5) )
	ROM_LOAD32_BYTE( "ru-13b",   0x080002, 0x10000, CRC(d0f6e747) SHA1(ef15ed41124b2d37bc6e92254138690dd644e50f) )
	ROM_LOAD32_BYTE( "ru-15b",   0x080003, 0x10000, CRC(6ee6b22e) SHA1(a28edaf23ca6c7231264de962d5ea37bad39f996) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "ru-17b",   0x000000, 0x20000, CRC(82391b47) SHA1(6b1977522c6e906503abc50bdd24c4c38cdc9bdb) )
	ROM_LOAD( "ru-16b",   0x020000, 0x10000, CRC(374fe170) SHA1(5d190a2735698b0384948bfdb1a900f56f0d7ebc) )
ROM_END

} // anonymous namespace

GAMEL(1989, roundup5,  0,        roundup5,  roundup5, roundup5_state, init_tatsumi,  ROT0, "Tatsumi", "Round Up 5 - Super Delta Force", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING, layout_roundup5 )
