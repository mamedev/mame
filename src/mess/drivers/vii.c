/******************************************************************************


    Sport Vii / The Batman
    ----------------------

    MESS driver by Harmony
    Based largely off of Unununium, by Segher


*******************************************************************************

    Short Description:

        Systems run on the SPG243 SoC

    Status:

        Mostly working

    To-Do:

        Audio (SPG243)
        Motion controls (Vii)

    Known u'nSP-Based Systems:

        ND - SPG243 - Some form of Leapfrog "edutainment" system
        ND - SPG243 - Star Wars: Clone Wars
        ND - SPG243 - Toy Story
        ND - SPG243 - Animal Art Studio
        ND - SPG243 - Finding Nemo
         D - SPG243 - The Batman
         D - SPG243 - Wall-E
         D - SPG243 - Chintendo / KenSingTon / Siatronics / Jungle Soft Vii
 Partial D - SPG200 - V-Tech V-Smile
        ND - Likely many more


Similar Systems: ( from http://en.wkikpedia.org/wiki/V.Smile )
- V.Smile by Vtech, a system designed for children under the age of 10
- V.Smile Pocket (2 versions)
- V.SMile Cyber Pocket
- V.Smile PC Pal
- V-Motion Active Learning System
- Leapster
- V.Smile Baby Infant Development System
- V.Flash

Detailed list of bugs:
- When loading a cart from file manager, sometimes it will crash
- On 'vii_vc1' & 'vii_vc2' cart, the left-right keys are transposed with the up-down keys
- The game 'Jewel Master' on both above carts displays a priority error at top of screen
- In the default bios (no cart loaded):
-- In the menu, when 'Come On!' is selected, a graphics error appears
-- Catch Fish, black screen
-- Come On! freezes at the high score screen, controls seem haywire
-- Bird Knight, no controls
-- Lucky Dice, the dice never stop spinning
-- Fever Move, after pressing A it freezes
-- Alacrity Golf, black screen
-- Smart Dart, black screen
-- Happy Tennis, controls are haywire
-- Bowling, freezes at the high score screen


*******************************************************************************/

#include "emu.h"
#include "cpu/unsp/unsp.h"
#include "imagedev/cartslot.h"
#include "machine/i2cmem.h"

#define PAGE_ENABLE_MASK        0x0008

#define PAGE_DEPTH_FLAG_MASK    0x3000
#define PAGE_DEPTH_FLAG_SHIFT   12
#define PAGE_TILE_HEIGHT_MASK   0x00c0
#define PAGE_TILE_HEIGHT_SHIFT  6
#define PAGE_TILE_WIDTH_MASK    0x0030
#define PAGE_TILE_WIDTH_SHIFT   4
#define TILE_X_FLIP             0x0004
#define TILE_Y_FLIP             0x0008

class vii_state : public driver_device
{
public:
	vii_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
		m_p_ram(*this, "p_ram"),
		m_p_rowscroll(*this, "p_rowscroll"),
		m_p_palette(*this, "p_palette"),
		m_p_spriteram(*this, "p_spriteram"),
		m_p_cart(*this, "p_cart"){ }

	required_device<cpu_device> m_maincpu;
	DECLARE_READ16_MEMBER(vii_video_r);
	DECLARE_WRITE16_MEMBER(vii_video_w);
	DECLARE_READ16_MEMBER(vii_audio_r);
	DECLARE_WRITE16_MEMBER(vii_audio_w);
	DECLARE_READ16_MEMBER(vii_io_r);
	DECLARE_WRITE16_MEMBER(vii_io_w);
	DECLARE_WRITE16_MEMBER(vii_rowscroll_w);
	DECLARE_WRITE16_MEMBER(vii_spriteram_w);
	required_shared_ptr<UINT16> m_p_ram;
	required_shared_ptr<UINT16> m_p_rowscroll;
	required_shared_ptr<UINT16> m_p_palette;
	required_shared_ptr<UINT16> m_p_spriteram;
	required_shared_ptr<UINT16> m_p_cart;

	UINT32 m_current_bank;

	UINT16 m_video_regs[0x100];
	UINT32 m_centered_coordinates;

	struct
	{
		UINT8 r, g, b;
	}
	m_screen[320*240];

	UINT16 m_io_regs[0x200];
	UINT16 m_uart_rx_count;
	UINT8 m_controller_input[8];
	UINT32 m_spg243_mode;

	emu_timer *m_tmb1;
	emu_timer *m_tmb2;
	void vii_do_dma(UINT32 len);
	void vii_do_gpio(UINT32 offset);
	void vii_switch_bank(UINT32 bank);
	void vii_do_i2c();
	void spg_do_dma(UINT32 len);
	DECLARE_DRIVER_INIT(vsmile);
	DECLARE_DRIVER_INIT(walle);
	DECLARE_DRIVER_INIT(vii);
	DECLARE_DRIVER_INIT(batman);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_vii(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vii_vblank);
	TIMER_CALLBACK_MEMBER(tmb1_tick);
	TIMER_CALLBACK_MEMBER(tmb2_tick);
};

enum
{
	SPG243_VII = 0,
	SPG243_BATMAN,
	SPG243_VSMILE,

	SPG243_MODEL_COUNT,
};


#define VII_CTLR_IRQ_ENABLE m_io_regs[0x21]
#define VII_VIDEO_IRQ_ENABLE    m_video_regs[0x62]
#define VII_VIDEO_IRQ_STATUS    m_video_regs[0x63]


#define VERBOSE_LEVEL   (3)

#define ENABLE_VERBOSE_LOG (1)

#if ENABLE_VERBOSE_LOG
INLINE void verboselog(running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%04x: %s", machine.device("maincpu")->safe_pc(), buf );
	}
}
#else
#define verboselog(x,y,z,...)
#endif

/*************************
*     Video Hardware     *
*************************/

void vii_state::video_start()
{
}

INLINE UINT8 expand_rgb5_to_rgb8(UINT8 val)
{
	UINT8 temp = val & 0x1f;
	return (temp << 3) | (temp >> 2);
}

// Perform a lerp between a and b
INLINE UINT8 vii_mix_channel(vii_state *state, UINT8 a, UINT8 b)
{
	UINT8 alpha = state->m_video_regs[0x1c] & 0x00ff;
	return ((64 - alpha) * a + alpha * b) / 64;
}

static void vii_mix_pixel(vii_state *state, UINT32 offset, UINT16 rgb)
{
	state->m_screen[offset].r = vii_mix_channel(state, state->m_screen[offset].r, expand_rgb5_to_rgb8(rgb >> 10));
	state->m_screen[offset].g = vii_mix_channel(state, state->m_screen[offset].g, expand_rgb5_to_rgb8(rgb >> 5));
	state->m_screen[offset].b = vii_mix_channel(state, state->m_screen[offset].b, expand_rgb5_to_rgb8(rgb));
}

static void vii_set_pixel(vii_state *state, UINT32 offset, UINT16 rgb)
{
	state->m_screen[offset].r = expand_rgb5_to_rgb8(rgb >> 10);
	state->m_screen[offset].g = expand_rgb5_to_rgb8(rgb >> 5);
	state->m_screen[offset].b = expand_rgb5_to_rgb8(rgb);
}

static void vii_blit(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT32 xoff, UINT32 yoff, UINT32 attr, UINT32 ctrl, UINT32 bitmap_addr, UINT16 tile)
{
	vii_state *state = machine.driver_data<vii_state>();
	address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	UINT32 h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	UINT32 w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	UINT32 yflipmask = attr & TILE_Y_FLIP ? h - 1 : 0;
	UINT32 xflipmask = attr & TILE_X_FLIP ? w - 1 : 0;

	UINT32 nc = ((attr & 0x0003) + 1) << 1;

	UINT32 palette_offset = (attr & 0x0f00) >> 4;
	palette_offset >>= nc;
	palette_offset <<= nc;

	UINT32 m = bitmap_addr + nc*w*h/16*tile;
	UINT32 bits = 0;
	UINT32 nbits = 0;

	UINT32 x, y;

	for(y = 0; y < h; y++)
	{
		UINT32 yy = (yoff + (y ^ yflipmask)) & 0x1ff;

		for(x = 0; x < w; x++)
		{
			UINT32 xx = (xoff + (x ^ xflipmask)) & 0x1ff;
			UINT32 pal;

			bits <<= nc;
			if(nbits < nc)
			{
				UINT16 b = space.read_word((m++ & 0x3fffff) << 1);
				b = (b << 8) | (b >> 8);
				bits |= b << (nc - nbits);
				nbits += 16;
			}
			nbits -= nc;

			pal = palette_offset | (bits >> 16);
			bits &= 0xffff;

			if((ctrl & 0x0010) && yy < 240)
			{
				xx = (xx - (INT16)state->m_p_rowscroll[yy]) & 0x01ff;
			}

			if(xx < 320 && yy < 240)
			{
				UINT16 rgb = state->m_p_palette[pal];
				if(!(rgb & 0x8000))
				{
					if (attr & 0x4000)
					{
						vii_mix_pixel(state, xx + 320*yy, rgb);
					}
					else
					{
						vii_set_pixel(state, xx + 320*yy, rgb);
					}
				}
			}
		}
	}
}

static void vii_blit_page(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, UINT32 bitmap_addr, UINT16 *regs)
{
	UINT32 x0, y0;
	UINT32 xscroll = regs[0];
	UINT32 yscroll = regs[1];
	UINT32 attr = regs[2];
	UINT32 ctrl = regs[3];
	UINT32 tilemap = regs[4];
	UINT32 palette_map = regs[5];
	UINT32 h, w, hn, wn;
	address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	if(!(ctrl & PAGE_ENABLE_MASK))
	{
		return;
	}

	if(((attr & PAGE_DEPTH_FLAG_MASK) >> PAGE_DEPTH_FLAG_SHIFT) != depth)
	{
		return;
	}

	h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	hn = 256 / h;
	wn = 512 / w;

	for(y0 = 0; y0 < hn; y0++)
	{
		for(x0 = 0; x0 < wn; x0++)
		{
			UINT16 tile = space.read_word((tilemap + x0 + wn * y0) << 1);
			UINT16 palette = 0;
			UINT32 xx, yy;

			if(!tile)
			{
				continue;
			}

			palette = space.read_word((palette_map + (x0 + wn * y0) / 2) << 1);
			if(x0 & 1)
			{
				palette >>= 8;
			}

			UINT32 tileattr = attr;
			UINT32 tilectrl = ctrl;
			if ((ctrl & 2) == 0)
			{   // -(1) bld(1) flip(2) pal(4)
				tileattr &= ~0x000c;
				tileattr |= (palette >> 2) & 0x000c;    // flip

				tileattr &= ~0x0f00;
				tileattr |= (palette << 8) & 0x0f00;    // palette

				tileattr &= ~0x0100;
				tileattr |= (palette << 2) & 0x0100;    // blend
			}

			yy = ((h*y0 - yscroll + 0x10) & 0xff) - 0x10;
			xx = (w*x0 - xscroll) & 0x1ff;

			vii_blit(machine, bitmap, cliprect, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
		}
	}
}

static void vii_blit_sprite(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, UINT32 base_addr)
{
	vii_state *state = machine.driver_data<vii_state>();
	address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT16 tile, attr;
	INT16 x, y;
	UINT32 h, w;
	UINT32 bitmap_addr = 0x40 * state->m_video_regs[0x22];

	tile = space.read_word((base_addr + 0) << 1);
	x = space.read_word((base_addr + 1) << 1);
	y = space.read_word((base_addr + 2) << 1);
	attr = space.read_word((base_addr + 3) << 1);

	if(!tile)
	{
		return;
	}

	if(((attr & PAGE_DEPTH_FLAG_MASK) >> PAGE_DEPTH_FLAG_SHIFT) != depth)
	{
		return;
	}

	if(state->m_centered_coordinates)
	{
		x = 160 + x;
		y = 120 - y;

		h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
		w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

		x -= (w/2);
		y -= (h/2) - 8;
	}

	x &= 0x01ff;
	y &= 0x01ff;

	vii_blit(machine, bitmap, cliprect, x, y, attr, 0, bitmap_addr, tile);
}

static void vii_blit_sprites(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth)
{
	vii_state *state = machine.driver_data<vii_state>();
	UINT32 n;

	if (!(state->m_video_regs[0x42] & 1))
	{
		return;
	}

	for(n = 0; n < 256; n++)
	{
		//if(space.read_word((0x2c00 + 4*n) << 1))
		{
			vii_blit_sprite(machine, bitmap, cliprect, depth, 0x2c00 + 4*n);
		}
	}
}

UINT32 vii_state::screen_update_vii(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, x, y;

	bitmap.fill(0, cliprect);

	memset(m_screen, 0, sizeof(m_screen));

	for(i = 0; i < 4; i++)
	{
		vii_blit_page(machine(), bitmap, cliprect, i, 0x40 * m_video_regs[0x20], m_video_regs + 0x10);
		vii_blit_page(machine(), bitmap, cliprect, i, 0x40 * m_video_regs[0x21], m_video_regs + 0x16);
		vii_blit_sprites(machine(), bitmap, cliprect, i);
	}

	for(y = 0; y < 240; y++)
	{
		for(x = 0; x < 320; x++)
		{
			bitmap.pix32(y, x) = (m_screen[x + 320*y].r << 16) | (m_screen[x + 320*y].g << 8) | m_screen[x + 320*y].b;
		}
	}

	return 0;
}

/*************************
*    Machine Hardware    *
*************************/

void vii_state::vii_do_dma(UINT32 len)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	UINT32 src = m_video_regs[0x70];
	UINT32 dst = m_video_regs[0x71] + 0x2c00;
	UINT32 j;

	for(j = 0; j < len; j++)
	{
		mem.write_word((dst+j) << 1, mem.read_word((src+j) << 1));
	}

	m_video_regs[0x72] = 0;
	m_video_regs[0x63] |= 4;
}

READ16_MEMBER( vii_state::vii_video_r )
{
	switch(offset)
	{
		case 0x62: // Video IRQ Enable
			verboselog(machine(), 0, "vii_video_r: Video IRQ Enable: %04x\n", VII_VIDEO_IRQ_ENABLE);
			return VII_VIDEO_IRQ_ENABLE;

		case 0x63: // Video IRQ Status
			verboselog(machine(), 0, "vii_video_r: Video IRQ Status: %04x\n", VII_VIDEO_IRQ_STATUS);
			return VII_VIDEO_IRQ_STATUS;

		default:
			verboselog(machine(), 0, "vii_video_r: Unknown register %04x = %04x\n", 0x2800 + offset, m_video_regs[offset]);
			break;
	}
	return m_video_regs[offset];
}

WRITE16_MEMBER( vii_state::vii_video_w )
{
	switch(offset)
	{
		case 0x10: case 0x16:   // page 1,2 X scroll
			data &= 0x01ff;
			COMBINE_DATA(&m_video_regs[offset]);
			break;

		case 0x11: case 0x17:   // page 1,2 Y scroll
			data &= 0x00ff;
			COMBINE_DATA(&m_video_regs[offset]);
			break;
		case 0x36:      // IRQ pos V
		case 0x37:      // IRQ pos H
			data &= 0x01ff;
			COMBINE_DATA(&m_video_regs[offset]);
			break;
		case 0x62: // Video IRQ Enable
			verboselog(machine(), 0, "vii_video_w: Video IRQ Enable = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&VII_VIDEO_IRQ_ENABLE);
			break;

		case 0x63: // Video IRQ Acknowledge
			verboselog(machine(), 0, "vii_video_w: Video IRQ Acknowledge = %04x (%04x)\n", data, mem_mask);
			VII_VIDEO_IRQ_STATUS &= ~data;
			if(!VII_VIDEO_IRQ_STATUS)
			{
				machine().device("maincpu")->execute().set_input_line(UNSP_IRQ0_LINE, CLEAR_LINE);
			}
			break;

		case 0x70: // Video DMA Source
			verboselog(machine(), 0, "vii_video_w: Video DMA Source = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_video_regs[offset]);
			break;

		case 0x71: // Video DMA Dest
			verboselog(machine(), 0, "vii_video_w: Video DMA Dest = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_video_regs[offset]);
			break;

		case 0x72: // Video DMA Length
			verboselog(machine(), 0, "vii_video_w: Video DMA Length = %04x (%04x)\n", data, mem_mask);
			vii_do_dma(data);
			break;

		default:
			verboselog(machine(), 0, "vii_video_w: Unknown register %04x = %04x (%04x)\n", 0x2800 + offset, data, mem_mask);
			COMBINE_DATA(&m_video_regs[offset]);
			break;
	}
}

READ16_MEMBER( vii_state::vii_audio_r )
{
	switch(offset)
	{
		default:
			verboselog(machine(), 4, "vii_audio_r: Unknown register %04x\n", 0x3000 + offset);
			break;
	}
	return 0;
}

WRITE16_MEMBER( vii_state::vii_audio_w )
{
	switch(offset)
	{
		default:
			verboselog(machine(), 4, "vii_audio_w: Unknown register %04x = %04x (%04x)\n", 0x3000 + offset, data, mem_mask);
			break;
	}
}

void vii_state::vii_switch_bank(UINT32 bank)
{
	UINT8 *cart = memregion("cart")->base();

	if(bank != m_current_bank)
	{
		m_current_bank = bank;

		memcpy(m_p_cart, cart + 0x400000 * bank * 2 + 0x4000*2, (0x400000 - 0x4000) * 2);
	}
}

void vii_state::vii_do_gpio(UINT32 offset)
{
	UINT32 index  = (offset - 1) / 5;
	UINT16 buffer = m_io_regs[5*index + 2];
	UINT16 dir    = m_io_regs[5*index + 3];
	UINT16 attr   = m_io_regs[5*index + 4];
	UINT16 special= m_io_regs[5*index + 5];

	UINT16 push   = dir;
	UINT16 pull   = (~dir) & (~attr);
	UINT16 what   = (buffer & (push | pull));
	what ^= (dir & ~attr);
	what &= ~special;

	if(m_spg243_mode == SPG243_VII)
	{
		if(index == 1)
		{
			UINT32 bank = ((what & 0x80) >> 7) | ((what & 0x20) >> 4);
			vii_switch_bank(bank);
		}
	}
	else if(m_spg243_mode == SPG243_BATMAN)
	{
		if(index == 0)
		{
			UINT16 temp = ioport("P1")->read();
			what |= (temp & 0x0001) ? 0x8000 : 0;
			what |= (temp & 0x0002) ? 0x4000 : 0;
			what |= (temp & 0x0004) ? 0x2000 : 0;
			what |= (temp & 0x0008) ? 0x1000 : 0;
			what |= (temp & 0x0010) ? 0x0800 : 0;
			what |= (temp & 0x0020) ? 0x0400 : 0;
			what |= (temp & 0x0040) ? 0x0200 : 0;
			what |= (temp & 0x0080) ? 0x0100 : 0;
		}

		if(index == 2)
		{
		}
	}

	m_io_regs[5*index + 1] = what;
}

void vii_state::vii_do_i2c()
{
}

void vii_state::spg_do_dma(UINT32 len)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	UINT32 src = ((m_io_regs[0x101] & 0x3f) << 16) | m_io_regs[0x100];
	UINT32 dst = m_io_regs[0x103] & 0x3fff;
	UINT32 j;

	for(j = 0; j < len; j++)
		mem.write_word((dst+j) << 1, mem.read_word((src+j) << 1));

	m_io_regs[0x102] = 0;
}

READ16_MEMBER( vii_state::vii_io_r )
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[] = { 'A', 'B', 'C' };

	offset -= 0x500;

	UINT16 val = m_io_regs[offset];

	switch(offset)
	{
		case 0x01: case 0x06: case 0x0b: // GPIO Data Port A/B/C
			vii_do_gpio(offset);
			verboselog(machine(), 3, "vii_io_r: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset], mem_mask);
			val = m_io_regs[offset];
			break;

		case 0x02: case 0x03: case 0x04: case 0x05:
		case 0x07: case 0x08: case 0x09: case 0x0a:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Other GPIO regs
			verboselog(machine(), 3, "vii_io_r: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset], mem_mask);
			break;

		case 0x1c: // Random
			val = machine().rand() & 0x00ff;
			verboselog(machine(), 3, "vii_io_r: Random = %04x (%04x)\n", val, mem_mask);
			break;

		case 0x21: // IRQ Control
			verboselog(machine(), 3, "vii_io_r: Controller IRQ Control = %04x (%04x)\n", val, mem_mask);
			break;

		case 0x22: // IRQ Status
			verboselog(machine(), 3, "vii_io_r: Controller IRQ Status = %04x (%04x)\n", val, mem_mask);
			break;

		case 0x2c: case 0x2d: // Timers?
			val = machine().rand() & 0x0000ffff;
			verboselog(machine(), 3, "vii_io_r: Unknown Timer %d Register = %04x (%04x)\n", offset - 0x2c, val, mem_mask);
			break;

		case 0x2f: // Data Segment
			val = machine().device("maincpu")->state().state_int(UNSP_SR) >> 10;
			verboselog(machine(), 3, "vii_io_r: Data Segment = %04x (%04x)\n", val, mem_mask);
			break;

		case 0x31: // Unknown, UART Status?
			verboselog(machine(), 3, "vii_io_r: Unknown (UART Status?) = %04x (%04x)\n", 3, mem_mask);
			val = 3;
			break;

		case 0x36: // UART RX Data
			val = m_controller_input[m_uart_rx_count];
			m_uart_rx_count = (m_uart_rx_count + 1) % 8;
			verboselog(machine(), 3, "vii_io_r: UART RX Data = %04x (%04x)\n", val, mem_mask);
			break;

		case 0x59: // I2C Status
			verboselog(machine(), 3, "vii_io_r: I2C Status = %04x (%04x)\n", val, mem_mask);
			break;

		case 0x5e: // I2C Data In
			verboselog(machine(), 3, "vii_io_r: I2C Data In = %04x (%04x)\n", val, mem_mask);
			break;

		default:
			verboselog(machine(), 3, "vii_io_r: Unknown register %04x\n", 0x3d00 + offset);
			break;
	}

	return val;
}

WRITE16_MEMBER( vii_state::vii_io_w )
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[3] = { 'A', 'B', 'C' };

	UINT16 temp = 0;

	offset -= 0x500;

	switch(offset)
	{
		case 0x00: // GPIO special function select
			verboselog(machine(), 3, "vii_io_w: GPIO Function Select = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x01: case 0x06: case 0x0b: // GPIO data, port A/B/C
			offset++;
			// Intentional fallthrough

		case 0x02: case 0x03: case 0x04: case 0x05: // Port A
		case 0x07: case 0x08: case 0x09: case 0x0a: // Port B
		case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Port C
			verboselog(machine(), 3, "vii_io_w: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			vii_do_gpio(offset);
			break;

		case 0x10:      // timebase control
			if ((m_io_regs[offset] & 0x0003) != (data & 0x0003)) {
				UINT16 hz = 8 << (data & 0x0003);
				verboselog(machine(), 3, "*** TMB1 FREQ set to %dHz\n", hz);
				m_tmb1->adjust(attotime::zero, 0, attotime::from_hz( hz ));
			}
			if ((m_io_regs[offset] & 0x000c) != (data & 0x000c)) {
				UINT16 hz = 128 << ((data & 0x000c) >> 2);
				verboselog(machine(), 3, "*** TMB2 FREQ set to %dHz\n", hz);
				m_tmb2->adjust(attotime::zero, 0, attotime::from_hz( hz ));
			}
			COMBINE_DATA(&m_io_regs[offset]);
			break;
		case 0x21: // IRQ Enable
			verboselog(machine(), 3, "vii_io_w: Controller IRQ Control = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&VII_CTLR_IRQ_ENABLE);
			if(!VII_CTLR_IRQ_ENABLE)
			{
				machine().device("maincpu")->execute().set_input_line(UNSP_IRQ3_LINE, CLEAR_LINE);
			}
			break;

		case 0x22: // IRQ Acknowledge
			verboselog(machine(), 3, "vii_io_w: Controller IRQ Acknowledge = %04x (%04x)\n", data, mem_mask);
			m_io_regs[0x22] &= ~data;
			if(!m_io_regs[0x22])
			{
				machine().device("maincpu")->execute().set_input_line(UNSP_IRQ3_LINE, CLEAR_LINE);
			}
			break;

		case 0x2f: // Data Segment
			temp = machine().device("maincpu")->state().state_int(UNSP_SR);
			machine().device("maincpu")->state().set_state_int(UNSP_SR, (temp & 0x03ff) | ((data & 0x3f) << 10));
			verboselog(machine(), 3, "vii_io_w: Data Segment = %04x (%04x)\n", data, mem_mask);
			break;

		case 0x31: // Unknown UART
			verboselog(machine(), 3, "vii_io_w: Unknown UART = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x32: // UART Reset
			verboselog(machine(), 3, "vii_io_r: UART Reset\n");
			break;

		case 0x33: // UART Baud Rate
			verboselog(machine(), 3, "vii_io_w: UART Baud Rate = %u\n", 27000000 / 16 / (0x10000 - (m_io_regs[0x34] << 8) - data));
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x35: // UART TX Data
			verboselog(machine(), 3, "vii_io_w: UART Baud Rate = %u\n", 27000000 / 16 / (0x10000 - (data << 8) - m_io_regs[0x33]));
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x5a: // I2C Access Mode
			verboselog(machine(), 3, "vii_io_w: I2C Access Mode = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x5b: // I2C Device Address
			verboselog(machine(), 3, "vii_io_w: I2C Device Address = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x5c: // I2C Sub-Address
			verboselog(machine(), 3, "vii_io_w: I2C Sub-Address = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x5d: // I2C Data Out
			verboselog(machine(), 3, "vii_io_w: I2C Data Out = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x5e: // I2C Data In
			verboselog(machine(), 3, "vii_io_w: I2C Data In = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x5f: // I2C Controller Mode
			verboselog(machine(), 3, "vii_io_w: I2C Controller Mode = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x58: // I2C Command
			verboselog(machine(), 3, "vii_io_w: I2C Command = %04x (%04x)\n", data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			vii_do_i2c();
			break;

		case 0x59: // I2C Status / IRQ Acknowledge(?)
			verboselog(machine(), 3, "vii_io_w: I2C Status / Ack = %04x (%04x)\n", data, mem_mask);
			m_io_regs[offset] &= ~data;
			break;

		case 0x100: // DMA Source (L)
		case 0x101: // DMA Source (H)
		case 0x103: // DMA Destination
			COMBINE_DATA(&m_io_regs[offset]);
			break;

		case 0x102: // DMA Length
			spg_do_dma(data);
			break;

		default:
			verboselog(machine(), 3, "vii_io_w: Unknown register %04x = %04x (%04x)\n", 0x3d00 + offset, data, mem_mask);
			COMBINE_DATA(&m_io_regs[offset]);
			break;
	}
}

/*
WRITE16_MEMBER( vii_state::vii_rowscroll_w )
{
    switch(offset)
    {
        default:
            verboselog(machine(), 0, "vii_rowscroll_w: %04x = %04x (%04x)\n", 0x2900 + offset, data, mem_mask);
            break;
    }
}

WRITE16_MEMBER( vii_state::vii_spriteram_w )
{
    switch(offset)
    {
        default:
            verboselog(machine(), 0, "vii_spriteram_w: %04x = %04x (%04x)\n", 0x2c00 + offset, data, mem_mask);
            break;
    }
}
*/

static ADDRESS_MAP_START( vii_mem, AS_PROGRAM, 16, vii_state )
	AM_RANGE( 0x000000, 0x004fff ) AM_RAM AM_SHARE("p_ram")
	AM_RANGE( 0x005000, 0x0051ff ) AM_READWRITE(vii_video_r, vii_video_w)
	AM_RANGE( 0x005200, 0x0055ff ) AM_RAM AM_SHARE("p_rowscroll")
	AM_RANGE( 0x005600, 0x0057ff ) AM_RAM AM_SHARE("p_palette")
	AM_RANGE( 0x005800, 0x005fff ) AM_RAM AM_SHARE("p_spriteram")
	AM_RANGE( 0x006000, 0x006fff ) AM_READWRITE(vii_audio_r, vii_audio_w)
	AM_RANGE( 0x007000, 0x007fff ) AM_READWRITE(vii_io_r,    vii_io_w)
	AM_RANGE( 0x008000, 0x7fffff ) AM_ROM AM_SHARE("p_cart")
ADDRESS_MAP_END

static INPUT_PORTS_START( vii )
	PORT_START("P1")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("Button A")
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Button B")
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Button C")
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("Button D")
INPUT_PORTS_END

static INPUT_PORTS_START( batman )
	PORT_START("P1")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Menu")
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("B Button")
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("X Button")
INPUT_PORTS_END

static INPUT_PORTS_START( vsmile )
	PORT_START("P1")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Menu")
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("B Button")
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("X Button")
INPUT_PORTS_END

static INPUT_PORTS_START( walle )
	PORT_START("P1")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
INPUT_PORTS_END

static DEVICE_IMAGE_LOAD( vii_cart )
{
	vii_state *state = image.device().machine().driver_data<vii_state>();
	UINT8 *cart = state->memregion( "cart" )->base();
	if (image.software_entry() == NULL)
	{
		int size = image.length();

		if( image.fread(cart, size ) != size )
		{
			image.seterror( IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file" );
			return IMAGE_INIT_FAIL;
		}
	} else {
		int filesize = image.get_software_region_length("rom");
		memcpy(cart, image.get_software_region("rom"), filesize);
	}

	memcpy(state->m_p_cart, cart + 0x4000*2, (0x400000 - 0x4000) * 2);

	if( cart[0x3cd808] == 0x99 &&
		cart[0x3cd809] == 0x99 &&
		cart[0x3cd80a] == 0x83 &&
		cart[0x3cd80b] == 0x5e &&
		cart[0x3cd80c] == 0x52 &&
		cart[0x3cd80d] == 0x6b &&
		cart[0x3cd80e] == 0x78 &&
		cart[0x3cd80f] == 0x7f )
	{
		state->m_centered_coordinates = 0;
	}
	return IMAGE_INIT_PASS;
}

static DEVICE_IMAGE_LOAD( vsmile_cart )
{
	vii_state *state = image.device().machine().driver_data<vii_state>();
	UINT8 *cart = state->memregion( "cart" )->base();
	if (image.software_entry() == NULL)
	{
		int size = image.length();

		if( image.fread( cart, size ) != size )
		{
			image.seterror( IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file" );
			return IMAGE_INIT_FAIL;
		}
	}
	else
	{
		int filesize = image.get_software_region_length("rom");
		memcpy(cart, image.get_software_region("rom"), filesize);
	}
	memcpy(state->m_p_cart, cart + 0x4000*2, (0x400000 - 0x4000) * 2);
	return IMAGE_INIT_PASS;
}

TIMER_CALLBACK_MEMBER(vii_state::tmb1_tick)
{
	m_io_regs[0x22] |= 1;
}

TIMER_CALLBACK_MEMBER(vii_state::tmb2_tick)
{
	m_io_regs[0x22] |= 2;
}

void vii_state::machine_start()
{
	memset(m_video_regs, 0, 0x100 * sizeof(UINT16));
	memset(m_io_regs, 0, 0x100 * sizeof(UINT16));
	m_current_bank = 0;

	m_controller_input[0] = 0;
	m_controller_input[4] = 0;
	m_controller_input[6] = 0xff;
	m_controller_input[7] = 0;

	UINT8 *rom = memregion( "cart" )->base();
	if (rom)
	{ // to prevent batman crash
		memcpy(m_p_cart, rom + 0x4000*2, (0x400000 - 0x4000) * 2);
	}

	m_video_regs[0x36] = 0xffff;
	m_video_regs[0x37] = 0xffff;

	m_tmb1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vii_state::tmb1_tick),this));
	m_tmb2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vii_state::tmb2_tick),this));
	m_tmb1->reset();
	m_tmb2->reset();
}

void vii_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(vii_state::vii_vblank)
{
	UINT32 x = machine().rand() & 0x3ff;
	UINT32 y = machine().rand() & 0x3ff;
	UINT32 z = machine().rand() & 0x3ff;


	m_controller_input[0] = ioport("P1")->read();
	m_controller_input[1] = (UINT8)x;
	m_controller_input[2] = (UINT8)y;
	m_controller_input[3] = (UINT8)z;
	m_controller_input[4] = 0;
	x >>= 8;
	y >>= 8;
	z >>= 8;
	m_controller_input[5] = (z << 4) | (y << 2) | x;
	m_controller_input[6] = 0xff;
	m_controller_input[7] = 0;

	m_uart_rx_count = 0;

	VII_VIDEO_IRQ_STATUS = VII_VIDEO_IRQ_ENABLE & 1;
	if(VII_VIDEO_IRQ_STATUS)
	{
		verboselog(machine(), 0, "Video IRQ\n");
		machine().device("maincpu")->execute().set_input_line(UNSP_IRQ0_LINE, ASSERT_LINE);
	}

//  {
//      verboselog(machine(), 0, "audio 1 IRQ\n");
//      machine().device("maincpu")->execute().set_input_line(UNSP_IRQ1_LINE, ASSERT_LINE);
//  }
	if(m_io_regs[0x22] & m_io_regs[0x21] & 0x0c00)
	{
		verboselog(machine(), 0, "timerA, timer B IRQ\n");
		machine().device("maincpu")->execute().set_input_line(UNSP_IRQ2_LINE, ASSERT_LINE);
	}

	//if(m_io_regs[0x22] & m_io_regs[0x21] & 0x2100)
	// For now trigger always if any enabled
	if(VII_CTLR_IRQ_ENABLE)
	{
		verboselog(machine(), 0, "UART, ADC IRQ\n");
		machine().device("maincpu")->execute().set_input_line(UNSP_IRQ3_LINE, ASSERT_LINE);
	}
//  {
//      verboselog(machine(), 0, "audio 4 IRQ\n");
//      machine().device("maincpu")->execute().set_input_line(UNSP_IRQ4_LINE, ASSERT_LINE);
//  }

	if(m_io_regs[0x22] & m_io_regs[0x21] & 0x1200)
	{
		verboselog(machine(), 0, "External IRQ\n");
		machine().device("maincpu")->execute().set_input_line(UNSP_IRQ5_LINE, ASSERT_LINE);
	}
	if(m_io_regs[0x22] & m_io_regs[0x21] & 0x0070)
	{
		verboselog(machine(), 0, "1024Hz, 2048HZ, 4096HZ IRQ\n");
		machine().device("maincpu")->execute().set_input_line(UNSP_IRQ6_LINE, ASSERT_LINE);
	}
	if(m_io_regs[0x22] & m_io_regs[0x21] & 0x008b)
	{
		verboselog(machine(), 0, "TMB1, TMB2, 4Hz, key change IRQ\n");
		machine().device("maincpu")->execute().set_input_line(UNSP_IRQ7_LINE, ASSERT_LINE);
	}

}

static MACHINE_CONFIG_START( vii, vii_state )

	MCFG_CPU_ADD( "maincpu", UNSP, XTAL_27MHz)
	MCFG_CPU_PROGRAM_MAP( vii_mem )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vii_state,  vii_vblank)


	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(vii_state, screen_update_vii)
	MCFG_PALETTE_LENGTH(32768)

	MCFG_CARTSLOT_ADD( "cart" )
	MCFG_CARTSLOT_EXTENSION_LIST( "bin" )
	MCFG_CARTSLOT_LOAD( vii_cart )
	MCFG_CARTSLOT_INTERFACE("vii_cart")

	MCFG_SOFTWARE_LIST_ADD("vii_cart","vii")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( vsmile, vii_state )

	MCFG_CPU_ADD( "maincpu", UNSP, XTAL_27MHz)
	MCFG_CPU_PROGRAM_MAP( vii_mem )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vii_state,  vii_vblank)


	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(vii_state, screen_update_vii)
	MCFG_PALETTE_LENGTH(32768)

	MCFG_CARTSLOT_ADD( "cart" )
	MCFG_CARTSLOT_EXTENSION_LIST( "bin" )
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_LOAD( vsmile_cart )
MACHINE_CONFIG_END

static const i2cmem_interface i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, 0, 0x200
};

static MACHINE_CONFIG_START( batman, vii_state )

	MCFG_CPU_ADD( "maincpu", UNSP, XTAL_27MHz)
	MCFG_CPU_PROGRAM_MAP( vii_mem )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vii_state,  vii_vblank)


	MCFG_I2CMEM_ADD("i2cmem",i2cmem_interface)

	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(vii_state, screen_update_vii)
	MCFG_PALETTE_LENGTH(32768)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(vii_state,vii)
{
	m_spg243_mode = SPG243_VII;
	m_centered_coordinates = 1;
}

DRIVER_INIT_MEMBER(vii_state,batman)
{
	m_spg243_mode = SPG243_BATMAN;
	m_centered_coordinates = 1;
}

DRIVER_INIT_MEMBER(vii_state,vsmile)
{
	m_spg243_mode = SPG243_VSMILE;
	m_centered_coordinates = 1;
}

DRIVER_INIT_MEMBER(vii_state,walle)
{
	m_spg243_mode = SPG243_BATMAN;
	m_centered_coordinates = 0;
}

ROM_START( vii )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for u'nSP */

	ROM_REGION( 0x2000000, "cart", ROMREGION_ERASE00 )
	ROM_LOAD( "vii.bin", 0x0000, 0x2000000, CRC(04627639) SHA1(f883a92d31b53c9a5b0cdb112d07cd793c95fc43))
	ROM_CART_LOAD("cart", 0x0000, 0x2000000, ROM_MIRROR)
ROM_END

ROM_START( batmantv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for u'nSP */
	ROM_LOAD16_WORD_SWAP( "batman.bin", 0x000000, 0x400000, CRC(46f848e5) SHA1(5875d57bb3fe0cac5d20e626e4f82a0e5f9bb94c) )
ROM_END

ROM_START( vsmile )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for u'nSP */

	ROM_REGION( 0x2000000, "cart", ROMREGION_ERASE00 )
	ROM_CART_LOAD("cart", 0x0000, 0x2000000, ROM_MIRROR)
ROM_END

ROM_START( walle )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for u'nSP */
	ROM_LOAD16_WORD_SWAP( "walle.bin", 0x000000, 0x400000, BAD_DUMP CRC(bd554cba) SHA1(6cd06a036ab12e7b0e1fd8003db873b0bb783868) )
	// Alternate dump, we need to decide which one is correct.
	//ROM_LOAD16_WORD_SWAP( "walle.bin", 0x000000, 0x400000, CRC(6bc90b16) SHA1(184d72de059057aae7800da510fcf05ed1da9ec9))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY                                              FULLNAME      FLAGS */
CONS( 2004, batmantv, vii,      0,        batman,   batman, vii_state,   batman,   "JAKKS Pacific Inc / HotGen Ltd",                    "The Batman", GAME_NO_SOUND )
CONS( 2005, vsmile,   0,        0,        vsmile,   vsmile, vii_state,   vsmile,   "V-Tech",                                            "V-Smile",    GAME_NO_SOUND | GAME_NOT_WORKING )
CONS( 2007, vii,      0,        0,        vii,      vii, vii_state,      vii,      "Jungle Soft / KenSingTon / Chintendo / Siatronics", "Vii",        GAME_NO_SOUND )
CONS( 2008, walle,    vii,      0,        batman,   walle, vii_state,    walle,    "JAKKS Pacific Inc",                                 "Wall-E",     GAME_NO_SOUND )
