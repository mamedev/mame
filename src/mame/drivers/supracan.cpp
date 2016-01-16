// license:LGPL-2.1+
// copyright-holders:Angelo Salese,Ryan Holtz
/***************************************************************************


    Funtech Super A'Can
    -------------------

    Preliminary driver by Angelo Salese
    Improvements by Ryan Holtz


*******************************************************************************

INFO:

    The system unit contains a reset button.

    Controllers:
    - 4 directional buttons
    - A, B, X, Y, buttons
    - Start, select buttons
    - L, R shoulder buttons

STATUS:

    The driver is begging for a re-write or at least a split into video/supracan.c.  It will happen eventually.

    Sound CPU comms and sound chip are completely unknown.

    There are 6 interrupt sources on the 6502 side, all of which use the IRQ line.
    The register at 0x411 is bitmapped to indicate what source(s) are active.
    In priority order from most to least important, they are:

    411 value  How acked                     Notes
    0x40       read reg 0x16 of sound chip   likely timer. snd regs 0x16/0x17 are time constant, write 0 to reg 0x9f to start
    0x04       read at 0x405                 latch 1?  0xcd is magic value
    0x08       read at 0x404                 latch 2?  0xcd is magic value
    0x10       read at 0x409                 unknown, dispatched but not used in startup 6502 code
    0x20       read at 0x40a                 possible periodic like vblank?
    0x80       read reg 0x14 of sound chip   depends on reg 0x14 of sound chip & 0x40: if not set writes 0x8f to reg 0x14,
                                             otherwise writes 0x4f to reg 0x14 and performs additional processing

    Known unemulated graphical effects and issues:
    - All: Sprite sizing is still imperfect.
    - All: Sprites need to be converted to use scanline rendering for proper clipping.
    - All: Improperly-emulated 1bpp ROZ mode, used by the Super A'Can BIOS logo.
    - All: Unimplemented ROZ scaling tables, used by the Super A'Can BIOS logo and Speedy Dragon intro, among others.
    - All: Priorities are largely unknown.
    - C.U.G.: Gameplay backgrounds are broken.
    - Sango Fighter: Possible missing masking on the upper edges of the screen during gameplay.
    - Sango Fighter: Raster effects off by 1 line
    - Sango Fighter: Specifies tiles out of range of video ram??
    - Speedy Dragon: Backgrounds are broken (wrong tile bank/region).
    - Super Taiwanese Baseball League: Does not boot, uses an unemulated DMA type
    - Super Taiwanese Baseball League: Missing window effect applied on tilemaps?
    - The Son of Evil: Many graphical issues.
    - Visible area, looks like it should be 224 pixels high at most, most games need 8 off the top and 8 off the bottom (or a global scroll)
      Sango looks like it needs 16 off the bottom instead
      Visible area is almost certainly 224 as Son of Evil has an explicit check in the vblank handler

    - All: are ALL the layers ROZ capable??

DEBUG TRICKS:

    baseball game debug trick:
    wpset e90020,1f,w
    do pc=5ac40
    ...
    do pc=5acd4
    wpclear
    bp 0269E4
    [ff7be4] <- 0x269ec
    bpclear

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

#define SOUNDCPU_BOOT_HACK      (1)

#define DRAW_DEBUG_ROZ          (0)

#define DRAW_DEBUG_UNK_SPRITE   (0)

#define DEBUG_PRIORITY          (0)
#define DEBUG_PRIORITY_INDEX    (0) // 0-3

#define VERBOSE_LEVEL   (3)

#define ENABLE_VERBOSE_LOG (1)

struct acan_dma_regs_t
{
	UINT32 source[2];
	UINT32 dest[2];
	UINT16 count[2];
	UINT16 control[2];
};

struct acan_sprdma_regs_t
{
	UINT32 src;
	UINT16 src_inc;
	UINT32 dst;
	UINT16 dst_inc;
	UINT16 count;
	UINT16 control;
};

class supracan_state : public driver_device
{
public:
	supracan_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_cart(*this, "cartslot"),
			m_vram(*this, "vram"),
			m_soundram(*this, "soundram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette")
	{
		m_m6502_reset = 0;
	}

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<generic_slot_device> m_cart;

	required_shared_ptr<UINT16> m_vram;
	required_shared_ptr<UINT8> m_soundram;

	DECLARE_READ16_MEMBER(_68k_soundram_r);
	DECLARE_WRITE16_MEMBER(_68k_soundram_w);
	DECLARE_READ8_MEMBER(_6502_soundmem_r);
	DECLARE_WRITE8_MEMBER(_6502_soundmem_w);

	void dma_w(address_space &space, int offset, UINT16 data, UINT16 mem_mask, int ch);
	DECLARE_WRITE16_MEMBER(dma_channel0_w);
	DECLARE_WRITE16_MEMBER(dma_channel1_w);

	DECLARE_READ16_MEMBER(sound_r);
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_READ16_MEMBER(video_r);
	DECLARE_WRITE16_MEMBER(video_w);
	DECLARE_WRITE16_MEMBER(vram_w);
	acan_dma_regs_t m_acan_dma_regs;
	acan_sprdma_regs_t m_acan_sprdma_regs;

	UINT16 m_m6502_reset;
	UINT8 m_soundlatch;
	UINT8 m_soundcpu_irq_src;
	UINT8 m_sound_irq_enable_reg;
	UINT8 m_sound_irq_source_reg;
	UINT8 m_sound_cpu_68k_irq_reg;

	emu_timer *m_video_timer;
	emu_timer *m_hbl_timer;
	emu_timer *m_line_on_timer;
	emu_timer *m_line_off_timer;

	dynamic_buffer m_vram_addr_swapped;

	UINT16 *m_pram;

	UINT16 m_sprite_count;
	UINT32 m_sprite_base_addr;
	UINT8 m_sprite_flags;

	UINT32 m_tilemap_base_addr[3];
	int m_tilemap_scrollx[3];
	int m_tilemap_scrolly[3];
	UINT16 m_video_flags;
	UINT16 m_tilemap_flags[3];
	UINT16 m_tilemap_mode[3];
	UINT16 m_irq_mask;
	UINT16 m_hbl_mask;

	UINT32 m_roz_base_addr;
	UINT16 m_roz_mode;
	UINT32 m_roz_scrollx;
	UINT32 m_roz_scrolly;
	UINT16 m_roz_tile_bank;
	UINT32 m_roz_unk_base0;
	UINT32 m_roz_unk_base1;
	UINT32 m_roz_unk_base2;
	UINT16 m_roz_coeffa;
	UINT16 m_roz_coeffb;
	UINT16 m_roz_coeffc;
	UINT16 m_roz_coeffd;
	INT32 m_roz_changed;
	INT32 m_roz_cx;
	INT32 m_roz_cy;
	UINT16 m_unk_1d0;

	UINT16 m_video_regs[256];

	bool m_hack_68k_to_6502_access;

	tilemap_t *m_tilemap_sizes[4][4];
	bitmap_ind16 m_sprite_final_bitmap;
	void write_swapped_byte(int offset, UINT8 byte);
	TILE_GET_INFO_MEMBER(get_supracan_tilemap0_tile_info);
	TILE_GET_INFO_MEMBER(get_supracan_tilemap1_tile_info);
	TILE_GET_INFO_MEMBER(get_supracan_tilemap2_tile_info);
	TILE_GET_INFO_MEMBER(get_supracan_roz_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(supracan);
	UINT32 screen_update_supracan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(supracan_irq);
	INTERRUPT_GEN_MEMBER(supracan_sound_irq);
	TIMER_CALLBACK_MEMBER(supracan_hbl_callback);
	TIMER_CALLBACK_MEMBER(supracan_line_on_callback);
	TIMER_CALLBACK_MEMBER(supracan_line_off_callback);
	TIMER_CALLBACK_MEMBER(supracan_video_callback);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(supracan_cart);
	inline void verboselog(std::string tag, int n_level, const char *s_fmt, ...) ATTR_PRINTF(4,5);
	int supracan_tilemap_get_region(int layer);
	void supracan_tilemap_get_info_common(int layer, tile_data &tileinfo, int count);
	void supracan_tilemap_get_info_roz(int layer, tile_data &tileinfo, int count);
	int get_tilemap_dimensions(int &xsize, int &ysize, int layer);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mark_active_tilemap_all_dirty(int layer);
	void supracan_suprnova_draw_roz(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound/*, int columnscroll, UINT32* scrollram*/, int transmask);
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



inline void supracan_state::verboselog(std::string tag, int n_level, const char *s_fmt, ...)
{
#if ENABLE_VERBOSE_LOG
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%06x: %s: %s", machine().device(tag)->safe_pc(), tag.c_str(), buf );
	}
#endif
}

int supracan_state::supracan_tilemap_get_region(int layer)
{
	// HACK!!!
	if (layer==2)
	{
		return 2;
	}


	if (layer==3)
	{
		// roz layer

		int gfx_mode = (m_roz_mode & 3);

		switch(gfx_mode)
		{
			case 0: return 4;
			case 1: return 2;
			case 2: return 1;
			case 3: return 0;
		}
		return 1;
	}
	else
	{
		// normal layers
		int gfx_mode = (m_tilemap_mode[layer] & 0x7000) >> 12;

		switch(gfx_mode)
		{
			case 7: return 2;
			case 4: return 1;
			case 2: return 1;
			case 0: return 1;
		}

		return 1;
	}

}

void supracan_state::supracan_tilemap_get_info_common(int layer, tile_data &tileinfo, int count)
{
	UINT16* supracan_vram = m_vram;

	UINT32 base = (m_tilemap_base_addr[layer]);
	int gfx_mode = (m_tilemap_mode[layer] & 0x7000) >> 12;
	int region = supracan_tilemap_get_region(layer);

	count += base;

	UINT16 tile_bank = 0;
	UINT16 palette_bank = 0;
	switch(gfx_mode)
	{
		case 7:
			tile_bank = 0x1c00;
			palette_bank = 0x00;
			break;

		case 6: // gambling lord
			tile_bank = 0x0c00;
			palette_bank = 0x00;
			break;

		case 4:
			tile_bank = 0x800;
			palette_bank = 0x00;
			break;

		case 2:
			tile_bank = 0x400;
			palette_bank = 0x00;
			break;

		case 0:
			tile_bank = 0;
			palette_bank = 0x00;
			break;

		default:
			verboselog("maincpu", 0, "Unsupported tilemap mode: %d\n", (m_tilemap_mode[layer] & 0x7000) >> 12);
			break;
	}


	if(layer == 2)
	{
		tile_bank = 0x1000;
	}

	int tile = (supracan_vram[count] & 0x03ff) + tile_bank;
	int flipxy = (supracan_vram[count] & 0x0c00)>>10;
	int palette = ((supracan_vram[count] & 0xf000) >> 12) + palette_bank;

	SET_TILE_INFO_MEMBER(region, tile, palette, TILE_FLIPXY(flipxy));
}

// I wonder how different this really is.. my guess, not at all.
void supracan_state::supracan_tilemap_get_info_roz(int layer, tile_data &tileinfo, int count)
{
	UINT16* supracan_vram = m_vram;

	UINT32 base = m_roz_base_addr;


	int region = 1;
	UINT16 tile_bank = 0;
	UINT16 palette_bank = 0;

	region = supracan_tilemap_get_region(layer);

	switch(m_roz_mode & 3) //FIXME: fix gfx bpp order
	{
		case 0:
			// hack: case for startup logo
			// this isn't understood properly, it's rendering a single 64x64 tile, which for convenience we've rearranged and decoded as 8x8 for the tilemaps
			{
				int tile = 0x880 + ((count & 7)*2);
			//  tile += (count & 0x070) >> 2;

				if (count & 0x20) tile ^= 1;
				tile |= (count & 0xc0)>>2;

				SET_TILE_INFO_MEMBER(region, tile, 0, 0);
				return;
			}


		case 1:
			tile_bank = (m_roz_tile_bank & 0xf000) >> 3;
			break;

		case 2:
			tile_bank = (m_roz_tile_bank & 0xf000) >> 3;
			break;

		case 3:
			tile_bank = (m_roz_tile_bank & 0xf000) >> 3;
			break;
	}

	count += base;

	int tile = (supracan_vram[count] & 0x03ff) + tile_bank;
	int flipxy = (supracan_vram[count] & 0x0c00)>>10;
	int palette = ((supracan_vram[count] & 0xf000) >> 12) + palette_bank;

	SET_TILE_INFO_MEMBER(region, tile, palette, TILE_FLIPXY(flipxy));
}



TILE_GET_INFO_MEMBER(supracan_state::get_supracan_tilemap0_tile_info)
{
	supracan_tilemap_get_info_common(0, tileinfo, tile_index);
}

TILE_GET_INFO_MEMBER(supracan_state::get_supracan_tilemap1_tile_info)
{
	supracan_tilemap_get_info_common(1, tileinfo, tile_index);
}

TILE_GET_INFO_MEMBER(supracan_state::get_supracan_tilemap2_tile_info)
{
	supracan_tilemap_get_info_common(2, tileinfo, tile_index);
}

TILE_GET_INFO_MEMBER(supracan_state::get_supracan_roz_tile_info)
{
	supracan_tilemap_get_info_roz(3, tileinfo, tile_index);
}


void supracan_state::video_start()
{
	m_sprite_final_bitmap.allocate(1024, 1024, BITMAP_FORMAT_IND16);

	m_vram_addr_swapped.resize(0x20000); // hack for 1bpp layer at startup
	m_gfxdecode->gfx(4)->set_source(&m_vram_addr_swapped[0]);
	m_gfxdecode->gfx(4)->set_xormask(0);

	m_tilemap_sizes[0][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap_sizes[0][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap_sizes[0][2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_tilemap_sizes[0][3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap_sizes[1][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap_sizes[1][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap_sizes[1][2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_tilemap_sizes[1][3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap_sizes[2][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap_sizes[2][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap_sizes[2][2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_tilemap_sizes[2][3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_tilemap2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap_sizes[3][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_roz_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap_sizes[3][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_roz_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap_sizes[3][2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_roz_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_tilemap_sizes[3][3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supracan_state::get_supracan_roz_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
}

int supracan_state::get_tilemap_dimensions(int &xsize, int &ysize, int layer)
{
	int select;

	xsize = 32;
	ysize = 32;

	if (layer==3) select = (m_roz_mode & 0x0f00);
	else select = m_tilemap_flags[layer] & 0x0f00;

	switch(select)
	{
		case 0x600:
			xsize = 64;
			ysize = 32;
			return 1;

		case 0xa00:
			xsize = 128;
			ysize = 32;
			return 2;

		case 0xc00:
			xsize = 64;
			ysize = 64;
			return 3;

		default:
			verboselog("maincpu", 0, "Unsupported tilemap size for layer %d: %04x\n", layer, select);
			return 0;
	}
}




void supracan_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *supracan_vram = m_vram;

//      [0]
//      -e-- ---- ---- ---- sprite enable?
//      ---h hhh- ---- ---- Y size (not always right)
//      ---- ---y yyyy yyyy Y position
//      [1]
//      bbbb ---- ---- ---- Tile bank
//      ---- h--- ---- ---- Horizontal flip
//      ---- -v-- ---- ---- Vertical flip
//      ---- --mm ---- ---- Masking mode
//      ---- ---- ---- -www X size
//      [2]
//      zzzz ---- ---- ---- X scale
//      ---- ---x xxxx xxxx X position
//      [3]
//      d--- ---- ---- ---- Direct Sprite (use details from here, not looked up in vram)
//      -ooo oooo oooo oooo Sprite address

	UINT32 skip_count = 0;
	UINT32 start_word = (m_sprite_base_addr >> 1) + skip_count * 4;
	UINT32 end_word = start_word + (m_sprite_count - skip_count) * 4;
	int region = (m_sprite_flags & 1) ? 0 : 1; //8bpp : 4bpp

//  printf("frame\n");
	#define VRAM_MASK (0xffff)

	for(int i = start_word; i < end_word; i += 4)
	{
		int x = supracan_vram[i+2] & 0x01ff;
		int y = supracan_vram[i+0] & 0x01ff;

		int sprite_offset = (supracan_vram[i+3])<< 1;

		int bank = (supracan_vram[i+1] & 0xf000) >> 12;
		//int mask = (supracan_vram[i+1] & 0x0300) >> 8;
		int sprite_xflip = (supracan_vram[i+1] & 0x0800) >> 11;
		int sprite_yflip = (supracan_vram[i+1] & 0x0400) >> 10;
		//int xscale = (supracan_vram[i+2] & 0xf000) >> 12;
		gfx_element *gfx = m_gfxdecode->gfx(region);




		// wraparound
		if (y>=0x180) y-=0x200;
		if (x>=0x180) x-=0x200;

		if((supracan_vram[i+0] & 0x4000))
		{
		#if 0
			printf("%d (unk %02x) (enable %02x) (unk Y2 %02x, %02x) (y pos %02x) (bank %01x) (flip %01x) (unknown %02x) (x size %02x) (xscale %01x) (unk %01x) (xpos %02x) (code %04x)\n", i,
				(supracan_vram[i+0] & 0x8000) >> 15,
				(supracan_vram[i+0] & 0x4000) >> 14,
				(supracan_vram[i+0] & 0x2000) >> 13,
				(supracan_vram[i+0] & 0x1e00) >> 8,
				(supracan_vram[i+0] & 0x01ff),
				(supracan_vram[i+1] & 0xf000) >> 12,
				(supracan_vram[i+1] & 0x0c00) >> 10,
				(supracan_vram[i+1] & 0x03f0) >> 4,
				(supracan_vram[i+1] & 0x000f),
				(supracan_vram[i+2] & 0xf000) >> 12,
				(supracan_vram[i+2] & 0x0e00) >> 8,
				(supracan_vram[i+2] & 0x01ff) >> 0,
				(supracan_vram[i+3] & 0xffff));
		#endif


			if (supracan_vram[i+3] &0x8000)
			{
				UINT16 data = supracan_vram[i+3];
				int tile = (bank * 0x200) + (data & 0x03ff);

				int palette = (data & 0xf000) >> 12; // this might not be correct, due to the &0x8000 condition above this would force all single tile sprites to be using palette >=0x8 only

				//printf("sprite data %04x %04x %04x %04x\n", supracan_vram[i+0] , supracan_vram[i+1] , supracan_vram[i+2] ,supracan_vram[i+3]  );

				gfx->transpen(bitmap,cliprect,tile,palette,sprite_xflip,sprite_yflip,
					x,
					y,
					0);

			}
			else
			{
				int xsize = 1 << (supracan_vram[i+1] & 7);
				int ysize = ((supracan_vram[i+0] & 0x1e00) >> 9) + 1;

				// I think the xsize must influence the ysize somehow, there are too many conflicting cases otherwise
				// there don't appear to be any special markers in the actual looked up tile data to indicate skip / end of list

				for(int ytile = 0; ytile < ysize; ytile++)
				{
					for(int xtile = 0; xtile< xsize; xtile++)
					{
						UINT16 data = supracan_vram[(sprite_offset+ytile*xsize+xtile)&VRAM_MASK];
						int tile = (bank * 0x200) + (data & 0x03ff);
						int palette = (data & 0xf000) >> 12;

						int xpos, ypos;

						if (!sprite_yflip)
						{
							ypos = y + ytile*8;
						}
						else
						{
							ypos = y - (ytile+1)*8;
							ypos += ysize*8;
						}

						if (!sprite_xflip)
						{
							xpos = x + xtile*8;
						}
						else
						{
							xpos = x - (xtile+1)*8;
							xpos += xsize*8;
						}

						int tile_xflip = sprite_xflip ^ ((data & 0x0800)>>11);
						int tile_yflip = sprite_yflip ^ ((data & 0x0400)>>10);

						gfx->transpen(bitmap,cliprect,tile,palette,tile_xflip,tile_yflip,xpos,ypos,0);
					}
				}
			}

#if 0
			if(xscale == 0) continue;
			UINT32 delta = (1 << 17) / xscale;
			for(int sy = 0; sy < ysize*8; sy++)
			{
				UINT16 *src = &sprite_bitmap->pix16(sy);
				UINT16 *dst = &bitmap.pix16(y + sy);
				UINT32 dx = x << 16;
				for(int sx = 0; sx < xsize*8; sx++)
				{
					dst[dx >> 16] = src[sx];
					dx += delta;
				}
			}
#endif

		}
	}
}



void supracan_state::mark_active_tilemap_all_dirty(int layer)
{
	int xsize = 0;
	int ysize = 0;

	int which_tilemap_size;

	which_tilemap_size = get_tilemap_dimensions(xsize, ysize, layer);
//  for (int i=0;i<4;i++)
//    tilemap_mark_all_tiles_dirty(m_tilemap_sizes[layer][i]);
	m_tilemap_sizes[layer][which_tilemap_size]->mark_all_dirty();
}



/* draws ROZ with linescroll OR columnscroll to 16-bit indexed bitmap */
void supracan_state::supracan_suprnova_draw_roz(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound/*, int columnscroll, UINT32* scrollram*/, int transmask)
{
	//bitmap_ind16 *destbitmap = bitmap;
	bitmap_ind16 &srcbitmap = tmap->pixmap();
	//bitmap_ind16 &srcbitmapflags = tmap->flagsmap();
	const int xmask = srcbitmap.width()-1;
	const int ymask = srcbitmap.height()-1;
	const int widthshifted = srcbitmap.width() << 16;
	const int heightshifted = srcbitmap.height() << 16;
	UINT32 cx;
	UINT32 cy;
	int x;
	int sx;
	int sy;
	int ex;
	int ey;
	UINT16 *dest;
//  UINT8* destflags;
//  UINT8 *pri;
	//const UINT16 *src;
	//const UINT8 *maskptr;
	//int destadvance = destbitmap->bpp / 8;

	/* pre-advance based on the cliprect */
	startx += cliprect.min_x * incxx + cliprect.min_y * incyx;
	starty += cliprect.min_x * incxy + cliprect.min_y * incyy;

	/* extract start/end points */
	sx = cliprect.min_x;
	sy = cliprect.min_y;
	ex = cliprect.max_x;
	ey = cliprect.max_y;

	{
		/* loop over rows */
		while (sy <= ey)
		{
			/* initialize X counters */
			x = sx;
			cx = startx;
			cy = starty;

			/* get dest and priority pointers */
			dest = &bitmap.pix16(sy, sx);
			//destflags = &bitmapflags->pix8(sy, sx);

			/* loop over columns */
			while (x <= ex)
			{
				if ((wraparound) || (cx < widthshifted && cy < heightshifted)) // not sure how this will cope with no wraparound, but row/col scroll..
				{
					#if 0
					if (columnscroll)
					{
						int scroll = 0;//scrollram[(cx>>16)&0x3ff]);


						UINT16 data = &srcbitmap.pix16(
												((cy >> 16) - scroll) & ymask,
												(cx >> 16) & xmask)[0];

						if ((data & transmask)!=0)
							dest[0] = data;

						//destflags[0] = &srcbitmapflags.pix8(((cy >> 16) - scrollram[(cx>>16)&0x3ff]) & ymask, (cx >> 16) & xmask)[0];
					}
					else
					#endif
					{
						int scroll = 0;//scrollram[(cy>>16)&0x3ff]);
						UINT16 data =  srcbitmap.pix16(
												(cy >> 16) & ymask,
												((cx >> 16) - scroll) & xmask);


						if ((data & transmask)!=0)
							dest[0] = data;

						//destflags[0] = &srcbitmapflags.pix8((cy >> 16) & ymask, ((cx >> 16) - scrollram[(cy>>16)&0x3ff]) & xmask)[0];
					}
				}

				/* advance in X */
				cx += incxx;
				cy += incxy;
				x++;
				dest++;
//            destflags++;
//            pri++;
			}

			/* advance in Y */
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}
}


// VIDEO FLAGS                  ROZ MODE            TILEMAP FLAGS
//
//  Bit                         Bit                 Bit
// 15-9: Unknown                15-13: Priority?    15-13: Priority?
//    8: X ht. (256/320)        12: Unknown         12: Unknown
//    7: Tilemap 0 enable       11-8: Dims          11-8: Dims
//    6: Tilemap 1 enable       7-6: Unknown        7-6: Unknown
//    5: Tilemap 2 enable?      5: Wrap             5: Wrap
//    3: Sprite enable          4-2: Unknown        4-2: Mosaic
//    2: ROZ enable             1-0: Bit Depth      1-0: Bit Depth
//  1-0: Unknown

//                      Video Flags                 ROZ Mode                    Tilemap 0   Tilemap 1   Tilemap 2   VF Unk0
// A'Can logo:          120e: 0001 0010 0000 1110   4020: 0100 0000 0010 0000   4620        ----        ----        0x09
// Boomzoo Intro:       9a82: 1001 1010 1000 0010   0402: 0000 0100 0000 0010   6400        6400        4400        0x4d
// Boomzoo Title:       9acc: 1001 1010 1100 1100   0402: 0000 0100 0000 0010   6400        6400        4400        0x4d
// C.U.G. Intro:        11c8: 0001 0001 1100 1000   0402: 0000 0100 0000 0010   2400        4400        6400        0x08
// C.U.G. Title:        11cc: 0001 0001 1100 1100   0602: 0000 0110 0000 0010   2600        4600        ----        0x08
// Speedy Dragon Logo:  0388: 0000 0011 1000 1000   4020: 0100 0000 0010 0000   6c20        6c20        2600        0x01
// Speedy Dragon Title: 038c: 0000 0011 1000 1100   2603: 0010 0110 0000 0011   6c20        2c20        2600        0x01
// Sango Fighter Intro: 03c8: 0000 0011 1100 1000   ----: ---- ---- ---- ----   6c20        4620        ----        0x01
// Sango Fighter Game:  03ce: 0000 0011 1100 1110   0622: 0000 0110 0010 0010   2620        4620        ----        0x01

UINT32 supracan_state::screen_update_supracan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// treat the sprites as frame-buffered and only update the buffer when drawing scanline 0 - this might not be true!

	if (0)
	{
		if (cliprect.min_y == 0x00)
		{
			const rectangle &visarea = screen.visible_area();

			m_sprite_final_bitmap.fill(0x00, visarea);
			bitmap.fill(0x80, visarea);

			draw_sprites( m_sprite_final_bitmap, visarea);
		}
	}
	else
	{
		m_sprite_final_bitmap.fill(0x00, cliprect);
		bitmap.fill(0x80, cliprect);

		draw_sprites(m_sprite_final_bitmap, cliprect);
	}



	// mix screen
	int xsize = 0, ysize = 0;
//  int tilemap_num;
	int which_tilemap_size;
	int priority = 0;


	for (int pri=7;pri>=0;pri--)
	{
		for (int layer = 3; layer >=0; layer--)
		{
		//  popmessage("%04x\n",m_video_flags);
			int enabled = 0;

			if(m_video_flags & 0x04)
				if (layer==3) enabled = 1;

			if(m_video_flags & 0x80)
				if (layer==0) enabled = 1;

			if(m_video_flags & 0x40)
				if (layer==1) enabled = 1;

			if(m_video_flags & 0x20)
				if (layer==2) enabled = 1;


			if (layer==3) priority = ((m_roz_mode >> 13) & 7); // roz case
			else priority = ((m_tilemap_flags[layer] >> 13) & 7); // normal cases


			if (priority==pri)
			{
//            tilemap_num = layer;
				which_tilemap_size = get_tilemap_dimensions(xsize, ysize, layer);
				bitmap_ind16 &src_bitmap = m_tilemap_sizes[layer][which_tilemap_size]->pixmap();
				int gfx_region = supracan_tilemap_get_region(layer);
				int transmask = 0xff;

				switch (gfx_region)
				{
					case 0:transmask = 0xff; break;
					case 1:transmask = 0x0f; break;
					case 2:transmask = 0x03; break;
					case 3:transmask = 0x01; break;
					case 4:transmask = 0x01; break;
				}

				if (enabled)
				{
					if (layer != 3) // standard layers, NOT roz
					{
						int wrap = (m_tilemap_flags[layer] & 0x20);

						int scrollx = m_tilemap_scrollx[layer];
						int scrolly = m_tilemap_scrolly[layer];

						if (scrollx&0x8000) scrollx-= 0x10000;
						if (scrolly&0x8000) scrolly-= 0x10000;

						int mosaic_count = (m_tilemap_flags[layer] & 0x001c) >> 2;
						int mosaic_mask = 0xffffffff << mosaic_count;

						int y,x;
						// yes, it will draw a single line if you specify a cliprect as such (partial updates...)

						for (y=cliprect.min_y;y<=cliprect.max_y;y++)
						{
							// these will have to change to ADDR32 etc. once alpha blending is supported
							UINT16* screen = &bitmap.pix16(y);

							int actualy = y&mosaic_mask;

							int realy = actualy+scrolly;

							if (!wrap)
								if (scrolly+y < 0 || scrolly+y > ((ysize*8)-1))
									continue;


							UINT16* src = &src_bitmap.pix16((realy)&((ysize*8)-1));

							for (x=cliprect.min_x;x<=cliprect.max_x;x++)
							{
								int actualx = x & mosaic_mask;
								int realx = actualx+scrollx;

								if (!wrap)
									if (scrollx+x < 0 || scrollx+x > ((xsize*8)-1))
										continue;

								UINT16 srcpix = src[(realx)&((xsize*8)-1)];

								if ((srcpix & transmask) != 0)
									screen[x] = srcpix;
							}
						}
					}
					else
					{
						int wrap = m_roz_mode & 0x20;

						int incxx = (m_roz_coeffa);
						int incyy = (m_roz_coeffd);

						int incxy = (m_roz_coeffc);
						int incyx = (m_roz_coeffb);

						int scrollx = (m_roz_scrollx);
						int scrolly = (m_roz_scrolly);






						if (incyx & 0x8000) incyx -= 0x10000;
						if (incxy & 0x8000) incxy -= 0x10000;

						if (incyy & 0x8000) incyy -= 0x10000;
						if (incxx & 0x8000) incxx -= 0x10000;

						//popmessage("%04x %04x\n",m_video_flags, m_roz_mode);

						// roz mode..
						//4020 = enabled speedyd
						//6c22 = enabled speedyd
						//2c22 = enabled speedyd
						//4622 = disabled jttlaugh
						//2602 = disabled monopoly
						//0402 = disabled (sango title)
						// or is it always enabled, and only corrupt because we don't clear ram properly?
						// (probably not this register?)

						if (!(m_roz_mode & 0x0200) && (m_roz_mode&0xf000) ) // HACK - Not Trusted, Acan Logo, Speedy Dragon Intro ,Speed Dragon Bonus stage need it.  Monopoly and JTT *don't* causes graphical issues
						{
							// NOT accurate, causes issues when the attract mode loops and the logo is shown the 2nd time in some games - investigate
							for (int y=cliprect.min_y;y<=cliprect.max_y;y++)
							{
								rectangle clip(cliprect.min_x, cliprect.max_x, y, y);

								scrollx = (m_roz_scrollx);
								scrolly = (m_roz_scrolly);
								incxx = (m_roz_coeffa);

								incxx += m_vram[m_roz_unk_base0/2 + y];

								scrollx += m_vram[m_roz_unk_base1/2 + y*2] << 16;
								scrollx += m_vram[m_roz_unk_base1/2 + y*2 + 1];

								scrolly += m_vram[m_roz_unk_base2/2 + y*2] << 16;
								scrolly += m_vram[m_roz_unk_base2/2 + y*2 + 1];

								if (incxx & 0x8000) incxx -= 0x10000;


								if (m_vram[m_roz_unk_base0/2 + y]) // incxx = 0, no draw?
									supracan_suprnova_draw_roz(bitmap, clip, m_tilemap_sizes[layer][which_tilemap_size], scrollx<<8, scrolly<<8, incxx<<8, incxy<<8, incyx<<8, incyy<<8, wrap, transmask);
							}
						}
						else
						{
							supracan_suprnova_draw_roz(bitmap, cliprect, m_tilemap_sizes[layer][which_tilemap_size], scrollx<<8, scrolly<<8, incxx<<8, incxy<<8, incyx<<8, incyy<<8, wrap, transmask);
						}
					}
				}
			}
		}
	}


	// just draw the sprites on top for now
	if(m_video_flags & 0x08)
	{
		for (int y=cliprect.min_y;y<=cliprect.max_y;y++)
		{
			UINT16* src = &m_sprite_final_bitmap.pix16(y);
			UINT16* dst = &bitmap.pix16(y);

			for (int x=cliprect.min_x;x<=cliprect.max_x;x++)
			{
				UINT16 dat = src[x];
				if (dat) dst[x] = dat;
			}
		}
	}

	return 0;
}


void supracan_state::dma_w(address_space &space, int offset, UINT16 data, UINT16 mem_mask, int ch)
{
	acan_dma_regs_t *acan_dma_regs = &m_acan_dma_regs;
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	switch(offset)
	{
		case 0x00/2: // Source address MSW
			verboselog("maincpu", 0, "dma_w: source msw %d: %04x\n", ch, data);
			acan_dma_regs->source[ch] &= 0x0000ffff;
			acan_dma_regs->source[ch] |= data << 16;
			break;
		case 0x02/2: // Source address LSW
			verboselog("maincpu", 0, "dma_w: source lsw %d: %04x\n", ch, data);
			acan_dma_regs->source[ch] &= 0xffff0000;
			acan_dma_regs->source[ch] |= data;
			break;
		case 0x04/2: // Destination address MSW
			verboselog("maincpu", 0, "dma_w: dest msw %d: %04x\n", ch, data);
			acan_dma_regs->dest[ch] &= 0x0000ffff;
			acan_dma_regs->dest[ch] |= data << 16;
			break;
		case 0x06/2: // Destination address LSW
			verboselog("maincpu", 0, "dma_w: dest lsw %d: %04x\n", ch, data);
			acan_dma_regs->dest[ch] &= 0xffff0000;
			acan_dma_regs->dest[ch] |= data;
			break;
		case 0x08/2: // Byte count
			verboselog("maincpu", 0, "dma_w: count %d: %04x\n", ch, data);
			acan_dma_regs->count[ch] = data;
			break;
		case 0x0a/2: // Control
			verboselog("maincpu", 0, "dma_w: control %d: %04x\n", ch, data);
			if(data & 0x8800)
			{
//            if(data & 0x2000)
//                acan_dma_regs->source-=2;
				logerror("dma_w: Kicking off a DMA from %08x to %08x, %d bytes (%04x)\n", acan_dma_regs->source[ch], acan_dma_regs->dest[ch], acan_dma_regs->count[ch] + 1, data);

				for(int i = 0; i <= acan_dma_regs->count[ch]; i++)
				{
					if(data & 0x1000)
					{
						mem.write_word(acan_dma_regs->dest[ch], mem.read_word(acan_dma_regs->source[ch]));
						acan_dma_regs->dest[ch]+=2;
						acan_dma_regs->source[ch]+=2;
						if(data & 0x0100)
							if((acan_dma_regs->dest[ch] & 0xf) == 0)
								acan_dma_regs->dest[ch]-=0x10;
					}
					else
					{
						mem.write_byte(acan_dma_regs->dest[ch], mem.read_byte(acan_dma_regs->source[ch]));
						acan_dma_regs->dest[ch]++;
						acan_dma_regs->source[ch]++;
					}
				}
			}
			else if(data != 0x0000) // fake DMA, used by C.U.G.
			{
				verboselog("maincpu", 0, "dma_w: Unknown DMA kickoff value of %04x (other regs %08x, %08x, %d)\n", data, acan_dma_regs->source[ch], acan_dma_regs->dest[ch], acan_dma_regs->count[ch] + 1);
				fatalerror("dma_w: Unknown DMA kickoff value of %04x (other regs %08x, %08x, %d)\n",data, acan_dma_regs->source[ch], acan_dma_regs->dest[ch], acan_dma_regs->count[ch] + 1);
			}
			break;
		default:
			verboselog("maincpu", 0, "dma_w: Unknown register: %08x = %04x & %04x\n", 0xe90020 + (offset << 1), data, mem_mask);
			break;
	}
}

WRITE16_MEMBER( supracan_state::dma_channel0_w )
{
	dma_w(space, offset, data, mem_mask, 0);
}

WRITE16_MEMBER( supracan_state::dma_channel1_w )
{
	dma_w(space, offset, data, mem_mask, 1);
}


#if 0
WRITE16_MEMBER( supracan_state::supracan_pram_w )
{
	m_pram[offset] &= ~mem_mask;
	m_pram[offset] |= data & mem_mask;
}
#endif

// swap address around so that 64x64 tile can be decoded as 8x8 tiles..
void supracan_state::write_swapped_byte( int offset, UINT8 byte )
{
	int swapped_offset = BITSWAP32(offset, 31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,2,1,0,6,5,4,3);

	m_vram_addr_swapped[swapped_offset] = byte;
}

WRITE16_MEMBER( supracan_state::vram_w )
{
	COMBINE_DATA(&m_vram[offset]);

	// hack for 1bpp layer at startup
	write_swapped_byte(offset*2, (data & 0xff00)>>8);
	write_swapped_byte(offset*2+1, (data & 0x00ff));

	// mark tiles of each depth as dirty
	m_gfxdecode->gfx(0)->mark_dirty((offset*2)/(64));
	m_gfxdecode->gfx(1)->mark_dirty((offset*2)/(32));
	m_gfxdecode->gfx(2)->mark_dirty((offset*2)/(16));
	m_gfxdecode->gfx(3)->mark_dirty((offset*2)/(512));
	m_gfxdecode->gfx(4)->mark_dirty((offset*2)/(8));

}

static ADDRESS_MAP_START( supracan_mem, AS_PROGRAM, 16, supracan_state )
	//AM_RANGE( 0x000000, 0x3fffff )        // mapped by the cartslot
	AM_RANGE( 0xe80200, 0xe80201 ) AM_READ_PORT("P1")
	AM_RANGE( 0xe80202, 0xe80203 ) AM_READ_PORT("P2")
	AM_RANGE( 0xe80208, 0xe80209 ) AM_READ_PORT("P3")
	AM_RANGE( 0xe8020c, 0xe8020d ) AM_READ_PORT("P4")
	AM_RANGE( 0xe80000, 0xe8ffff ) AM_READWRITE(_68k_soundram_r, _68k_soundram_w)
	AM_RANGE( 0xe90000, 0xe9001f ) AM_READWRITE(sound_r, sound_w)
	AM_RANGE( 0xe90020, 0xe9002f ) AM_WRITE(dma_channel0_w)
	AM_RANGE( 0xe90030, 0xe9003f ) AM_WRITE(dma_channel1_w)

	AM_RANGE( 0xf00000, 0xf001ff ) AM_READWRITE(video_r, video_w)
	AM_RANGE( 0xf00200, 0xf003ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0xf40000, 0xf5ffff ) AM_RAM_WRITE(vram_w) AM_SHARE("vram")
	AM_RANGE( 0xfc0000, 0xfdffff ) AM_MIRROR(0x30000) AM_RAM /* System work ram */
ADDRESS_MAP_END

READ8_MEMBER( supracan_state::_6502_soundmem_r )
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	UINT8 data = m_soundram[offset];

	switch(offset)
	{
#if SOUNDCPU_BOOT_HACK
		case 0x300: // HACK to make games think the sound CPU is always reporting 'OK'.
			return 0xff;
#endif

		case 0x410: // Sound IRQ enable
			data = m_sound_irq_enable_reg;
			if(!mem.debugger_access()) verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 0, "supracan_soundreg_r: IRQ enable: %04x\n", data);
			if(!mem.debugger_access())
			{
				if(m_sound_irq_enable_reg & m_sound_irq_source_reg)
				{
					m_soundcpu->set_input_line(0, ASSERT_LINE);
				}
				else
				{
					m_soundcpu->set_input_line(0, CLEAR_LINE);
				}
			}
			break;
		case 0x411: // Sound IRQ source
			data = m_sound_irq_source_reg;
			m_sound_irq_source_reg = 0;
			if(!mem.debugger_access()) verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 3, "supracan_soundreg_r: IRQ source: %04x\n", data);
			if(!mem.debugger_access())
			{
				m_soundcpu->set_input_line(0, CLEAR_LINE);
			}
			break;
		case 0x420:
			if(!mem.debugger_access()) verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 3, "supracan_soundreg_r: Sound hardware status? (not yet implemented): %02x\n", 0);
			break;
		case 0x422:
			if(!mem.debugger_access()) verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 3, "supracan_soundreg_r: Sound hardware data? (not yet implemented): %02x\n", 0);
			break;
		case 0x404:
		case 0x405:
		case 0x409:
		case 0x414:
		case 0x416:
			// Intentional fall-through
		default:
			if(offset >= 0x300 && offset < 0x500)
			{
				if(!mem.debugger_access()) verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 0, "supracan_soundreg_r: Unknown register %04x\n", offset);
			}
			break;
	}

	return data;
}

WRITE8_MEMBER( supracan_state::_6502_soundmem_w )
{
	switch(offset)
	{
		case 0x407:
			if(m_sound_cpu_68k_irq_reg &~ data)
			{
				verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 0, "supracan_soundreg_w: sound_cpu_68k_irq_reg: %04x: Triggering M68k IRQ\n", data);
				m_maincpu->set_input_line(7, HOLD_LINE);
			}
			else
			{
				verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 0, "supracan_soundreg_w: sound_cpu_68k_irq_reg: %04x\n", data);
			}
			m_sound_cpu_68k_irq_reg = data;
			break;
		case 0x410:
			m_sound_irq_enable_reg = data;
			verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 0, "supracan_soundreg_w: IRQ enable: %02x\n", data);
			break;
		case 0x420:
			verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 3, "supracan_soundreg_w: Sound hardware reg data? (not yet implemented): %02x\n", data);
			break;
		case 0x422:
			verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 3, "supracan_soundreg_w: Sound hardware reg addr? (not yet implemented): %02x\n", data);
			break;
		default:
			if(offset >= 0x300 && offset < 0x500)
			{
				verboselog(m_hack_68k_to_6502_access ? "maincpu" : "soundcpu", 0, "supracan_soundreg_w: Unknown register %04x = %02x\n", offset, data);
			}
			m_soundram[offset] = data;
			break;
	}
}

static ADDRESS_MAP_START( supracan_sound_mem, AS_PROGRAM, 8, supracan_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(_6502_soundmem_r, _6502_soundmem_w) AM_SHARE("soundram")
ADDRESS_MAP_END

static INPUT_PORTS_START( supracan )
	PORT_START("P1")
	PORT_DIPNAME( 0x01, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Button R")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Button L")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Button Y")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Button X")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 Joypad Right")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_NAME("P1 Joypad Left")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_NAME("P1 Joypad Down")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_NAME("P1 Joypad Up")
	PORT_DIPNAME( 0x1000, 0x0000, "SYSTEM" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Button B")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Button A")

	PORT_START("P2")
	PORT_DIPNAME( 0x01, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Button R")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Button L")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Button Y")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Button X")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_NAME("P2 Joypad Right")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_NAME("P2 Joypad Left")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_NAME("P2 Joypad Down")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_NAME("P2 Joypad Up")
	PORT_DIPNAME( 0x1000, 0x0000, "SYSTEM" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Button B")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Button A")

	PORT_START("P3")
	PORT_DIPNAME( 0x01, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(3) PORT_NAME("P3 Button R")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(3) PORT_NAME("P3 Button L")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3) PORT_NAME("P3 Button Y")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Button X")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_NAME("P3 Joypad Right")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_NAME("P3 Joypad Left")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_NAME("P3 Joypad Down")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_NAME("P3 Joypad Up")
	PORT_DIPNAME( 0x1000, 0x0000, "SYSTEM" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Button B")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Button A")

	PORT_START("P4")
	PORT_DIPNAME( 0x01, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(4) PORT_NAME("P4 Button R")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(4) PORT_NAME("P4 Button L")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("P4 Button Y")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 Button X")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_NAME("P4 Joypad Right")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_NAME("P4 Joypad Left")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_NAME("P4 Joypad Down")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_NAME("P4 Joypad Up")
	PORT_DIPNAME( 0x1000, 0x0000, "SYSTEM" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 Button B")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 Button A")
INPUT_PORTS_END

PALETTE_INIT_MEMBER(supracan_state, supracan)
{
	// Used for debugging purposes for now
	//#if 0
	int i, r, g, b;

	for( i = 0; i < 32768; i++ )
	{
		r = (i & 0x1f) << 3;
		g = ((i >> 5) & 0x1f) << 3;
		b = ((i >> 10) & 0x1f) << 3;
		palette.set_pen_color( i, r, g, b );
	}
	//#endif
}

WRITE16_MEMBER( supracan_state::_68k_soundram_w )
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	m_soundram[offset*2 + 1] = data & 0xff;
	m_soundram[offset*2 + 0] = data >> 8;

	if(offset*2 < 0x500 && offset*2 >= 0x300)
	{
		if(mem_mask & 0xff00)
		{
			m_hack_68k_to_6502_access = true;
			_6502_soundmem_w(mem, offset*2, data >> 8);
			m_hack_68k_to_6502_access = false;
		}
		if(mem_mask & 0x00ff)
		{
			m_hack_68k_to_6502_access = true;
			_6502_soundmem_w(mem, offset*2 + 1, data & 0xff);
			m_hack_68k_to_6502_access = false;
		}
	}
}

READ16_MEMBER( supracan_state::_68k_soundram_r )
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	UINT16 val = m_soundram[offset*2 + 0] << 8;
	val |= m_soundram[offset*2 + 1];

	if(offset*2 >= 0x300 && offset*2 < 0x500)
	{
		val = 0;
		if(mem_mask & 0xff00)
		{
			m_hack_68k_to_6502_access = true;
			val |= _6502_soundmem_r(mem, offset*2) << 8;
			m_hack_68k_to_6502_access = false;
		}
		if(mem_mask & 0x00ff)
		{
			m_hack_68k_to_6502_access = true;
			val |= _6502_soundmem_r(mem, offset*2 + 1);
			m_hack_68k_to_6502_access = false;
		}
	}

	return val;
}

READ16_MEMBER( supracan_state::sound_r )
{
	UINT16 data = 0;

	switch( offset )
	{
		default:
			verboselog("maincpu", 0, "sound_r: Unknown register: (%08x) & %04x\n", 0xe90000 + (offset << 1), mem_mask);
			break;
	}

	return data;
}

WRITE16_MEMBER( supracan_state::sound_w )
{
	switch ( offset )
	{
		case 0x000a/2:  /* Sound cpu IRQ request. */
			m_soundcpu->set_input_line(0, ASSERT_LINE);
			break;
		case 0x001c/2:  /* Sound cpu control. Bit 0 tied to sound cpu RESET line */
			if(data & 0x01)
			{
				if(!m_m6502_reset)
				{
					/* Reset and enable the sound cpu */
#if !(SOUNDCPU_BOOT_HACK)
					m_soundcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
					m_soundcpu->reset();
#endif
				}
				m_m6502_reset = data & 0x01;
			}
			else
			{
				/* Halt the sound cpu */
				m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			}
			verboselog("maincpu", 0, "sound cpu ctrl: %04x\n", data);
			break;
		default:
			verboselog("maincpu", 0, "sound_w: Unknown register: %08x = %04x & %04x\n", 0xe90000 + (offset << 1), data, mem_mask);
			break;
	}
}


READ16_MEMBER( supracan_state::video_r )
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	UINT16 data = m_video_regs[offset];

	switch(offset)
	{
		case 0x00/2: // Video IRQ flags
			if(!mem.debugger_access())
			{
				//verboselog("maincpu", 0, "read video IRQ flags (%04x)\n", data);
				m_maincpu->set_input_line(7, CLEAR_LINE);
			}
			break;
		case 0x02/2: // Current scanline
			break;
		case 0x08/2: // Unknown (not video flags!) - gambling lord disagrees, it MUST read back what it wrote because it reads it before turning on/off layers and writes it back
			//data = 0;
			break;
		case 0x100/2:
			if(!mem.debugger_access()) verboselog("maincpu", 0, "read tilemap_flags[0] (%04x)\n", data);
			break;
		case 0x106/2:
			if(!mem.debugger_access()) verboselog("maincpu", 0, "read tilemap_scrolly[0] (%04x)\n", data);
			break;
		case 0x120/2:
			if(!mem.debugger_access()) verboselog("maincpu", 0, "read tilemap_flags[1] (%04x)\n", data);
			break;
		default:
			if(!mem.debugger_access()) verboselog("maincpu", 0, "video_r: Unknown register: %08x (%04x & %04x)\n", 0xf00000 + (offset << 1), data, mem_mask);
			break;
	}

	return data;
}

TIMER_CALLBACK_MEMBER(supracan_state::supracan_hbl_callback)
{
	m_maincpu->set_input_line(3, HOLD_LINE);

	m_hbl_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(supracan_state::supracan_line_on_callback)
{
	m_maincpu->set_input_line(5, HOLD_LINE);

	m_line_on_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(supracan_state::supracan_line_off_callback)
{
	m_maincpu->set_input_line(5, CLEAR_LINE);

	m_line_on_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(supracan_state::supracan_video_callback)
{
	int vpos = machine().first_screen()->vpos();

	m_video_regs[0] &= ~0x0002;

	switch( vpos )
	{
	case 0:
		m_video_regs[0] &= 0x7fff;

		// we really need better management of this
		mark_active_tilemap_all_dirty(0);
		mark_active_tilemap_all_dirty(1);
		mark_active_tilemap_all_dirty(2);
		mark_active_tilemap_all_dirty(3);


		break;

	case 224://FIXME: Son of Evil is pretty picky about this one, a timing of 240 makes it to crash
		m_video_regs[0] |= 0x8000;
		break;

	case 240:
		if(m_irq_mask & 1)
		{
			verboselog("maincpu", 0, "Triggering VBL IRQ\n\n");
			m_maincpu->set_input_line(7, HOLD_LINE);
		}
		break;
	}

	m_video_regs[1] = machine().first_screen()->vpos()-16; // for son of evil, wants vblank active around 224 instead...

	m_hbl_timer->adjust( machine().first_screen()->time_until_pos( vpos, 320 ) );
	m_video_timer->adjust( machine().first_screen()->time_until_pos( ( vpos + 1 ) % 256, 0 ) );
}

WRITE16_MEMBER( supracan_state::video_w )
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	acan_sprdma_regs_t *acan_sprdma_regs = &m_acan_sprdma_regs;
	int i;

	// if any of this changes we need a partial update (see sango fighters intro)
	machine().first_screen()->update_partial(machine().first_screen()->vpos());

	COMBINE_DATA(&m_video_regs[offset]);
	data = m_video_regs[offset];

	switch(offset)
	{
		case 0x10/2: // Byte count
			verboselog("maincpu", 0, "sprite dma word count: %04x\n", data);
			acan_sprdma_regs->count = data;
			break;
		case 0x12/2: // Destination address MSW
			acan_sprdma_regs->dst &= 0x0000ffff;
			acan_sprdma_regs->dst |= data << 16;
			verboselog("maincpu", 0, "sprite dma dest msw: %04x\n", data);
			break;
		case 0x14/2: // Destination address LSW
			acan_sprdma_regs->dst &= 0xffff0000;
			acan_sprdma_regs->dst |= data;
			verboselog("maincpu", 0, "sprite dma dest lsw: %04x\n", data);
			break;
		case 0x16/2: // Source word increment
			verboselog("maincpu", 0, "sprite dma dest word inc: %04x\n", data);
			acan_sprdma_regs->dst_inc = data;
			break;
		case 0x18/2: // Source address MSW
			acan_sprdma_regs->src &= 0x0000ffff;
			acan_sprdma_regs->src |= data << 16;
			verboselog("maincpu", 0, "sprite dma src msw: %04x\n", data);
			break;
		case 0x1a/2: // Source address LSW
			verboselog("maincpu", 0, "sprite dma src lsw: %04x\n", data);
			acan_sprdma_regs->src &= 0xffff0000;
			acan_sprdma_regs->src |= data;
			break;
		case 0x1c/2: // Source word increment
			verboselog("maincpu", 0, "sprite dma src word inc: %04x\n", data);
			acan_sprdma_regs->src_inc = data;
			break;
		case 0x1e/2:
			logerror( "video_w: Kicking off a DMA from %08x to %08x, %d bytes (%04x)\n", acan_sprdma_regs->src, acan_sprdma_regs->dst, acan_sprdma_regs->count, data);

			/* TODO: what's 0x2000 and 0x4000 for? */
			if(data & 0x8000)
			{
				if(data & 0x2000 || data & 0x4000)
				{
					acan_sprdma_regs->dst |= 0xf40000;
				}

				if(data & 0x2000)
				{
					//acan_sprdma_regs->count <<= 1;
				}

				for(i = 0; i <= acan_sprdma_regs->count; i++)
				{
					if(data & 0x0100) //dma 0x00 fill (or fixed value?)
					{
						mem.write_word(acan_sprdma_regs->dst, 0);
						acan_sprdma_regs->dst+=2 * acan_sprdma_regs->dst_inc;
						//memset(supracan_vram,0x00,0x020000);
					}
					else
					{
						mem.write_word(acan_sprdma_regs->dst, mem.read_word(acan_sprdma_regs->src));
						acan_sprdma_regs->dst+=2 * acan_sprdma_regs->dst_inc;
						acan_sprdma_regs->src+=2 * acan_sprdma_regs->src_inc;
					}
				}
			}
			else
			{
				verboselog("maincpu", 0, "dma_w: Attempting to kick off a DMA without bit 15 set! (%04x)\n", data);
			}
			break;
		case 0x08/2:
			{
				verboselog("maincpu", 3, "video_flags = %04x\n", data);
				m_video_flags = data;

				rectangle visarea = machine().first_screen()->visible_area();

				visarea.set(0, ((m_video_flags & 0x100) ? 320 : 256) - 1, 8, 232 - 1);
				machine().first_screen()->configure(348, 256, visarea, machine().first_screen()->frame_period().attoseconds());
			}
			break;
		case 0x0a/2:
			{
				// raster interrupt
				verboselog("maincpu", 0, "IRQ Trigger? = %04x\n", data);
				if(data & 0x8000)
				{
					m_line_on_timer->adjust(machine().first_screen()->time_until_pos((data & 0x00ff), 0));
				}
				else
				{
					m_line_on_timer->adjust(attotime::never);
				}
			}
			break;

		case 0x0c/2:
			{
				verboselog("maincpu", 0, "IRQ De-Trigger? = %04x\n", data);
				if(data & 0x8000)
				{
					m_line_off_timer->adjust(machine().first_screen()->time_until_pos(data & 0x00ff, 0));
				}
				else
				{
					m_line_off_timer->adjust(attotime::never);
				}
			}
			break;

		/* Sprites */
		case 0x20/2: m_sprite_base_addr = data << 2; verboselog("maincpu", 0, "sprite_base_addr = %04x\n", data); break;
		case 0x22/2: m_sprite_count = data+1; verboselog("maincpu", 0, "sprite_count = %d\n", data+1); break;
		case 0x26/2: m_sprite_flags = data; verboselog("maincpu", 0, "sprite_flags = %04x\n", data); break;

		/* Tilemap 0 */
		case 0x100/2: m_tilemap_flags[0] = data; verboselog("maincpu", 3, "tilemap_flags[0] = %04x\n", data); break;
		case 0x104/2: m_tilemap_scrollx[0] = data; verboselog("maincpu", 3, "tilemap_scrollx[0] = %04x\n", data); break;
		case 0x106/2: m_tilemap_scrolly[0] = data; verboselog("maincpu", 3, "tilemap_scrolly[0] = %04x\n", data); break;
		case 0x108/2: m_tilemap_base_addr[0] = (data) << 1; verboselog("maincpu", 3, "tilemap_base_addr[0] = %05x\n", data << 2); break;
		case 0x10a/2: m_tilemap_mode[0] = data; verboselog("maincpu", 3, "tilemap_mode[0] = %04x\n", data); break;

		/* Tilemap 1 */
		case 0x120/2: m_tilemap_flags[1] = data; verboselog("maincpu", 3, "tilemap_flags[1] = %04x\n", data); break;
		case 0x124/2: m_tilemap_scrollx[1] = data; verboselog("maincpu", 3, "tilemap_scrollx[1] = %04x\n", data); break;
		case 0x126/2: m_tilemap_scrolly[1] = data; verboselog("maincpu", 3, "tilemap_scrolly[1] = %04x\n", data); break;
		case 0x128/2: m_tilemap_base_addr[1] = (data) << 1; verboselog("maincpu", 3, "tilemap_base_addr[1] = %05x\n", data << 2); break;
		case 0x12a/2: m_tilemap_mode[1] = data; verboselog("maincpu", 3, "tilemap_mode[1] = %04x\n", data); break;

		/* Tilemap 2? */
		case 0x140/2: m_tilemap_flags[2] = data; verboselog("maincpu", 0, "tilemap_flags[2] = %04x\n", data); break;
		case 0x144/2: m_tilemap_scrollx[2] = data; verboselog("maincpu", 0, "tilemap_scrollx[2] = %04x\n", data); break;
		case 0x146/2: m_tilemap_scrolly[2] = data; verboselog("maincpu", 0, "tilemap_scrolly[2] = %04x\n", data); break;
		case 0x148/2: m_tilemap_base_addr[2] = (data) << 1; verboselog("maincpu", 0, "tilemap_base_addr[2] = %05x\n", data << 2); break;
		case 0x14a/2: m_tilemap_mode[2] = data; verboselog("maincpu", 0, "tilemap_mode[2] = %04x\n", data); break;

		/* ROZ */
		case 0x180/2: m_roz_mode = data; verboselog("maincpu", 3, "roz_mode = %04x\n", data); break;
		case 0x184/2: m_roz_scrollx = (data << 16) | (m_roz_scrollx & 0xffff); m_roz_changed |= 1; verboselog("maincpu", 3, "roz_scrollx = %08x\n", m_roz_scrollx); break;
		case 0x186/2: m_roz_scrollx = (data) | (m_roz_scrollx & 0xffff0000); m_roz_changed |= 1; verboselog("maincpu", 3, "roz_scrollx = %08x\n", m_roz_scrollx); break;
		case 0x188/2: m_roz_scrolly = (data << 16) | (m_roz_scrolly & 0xffff); m_roz_changed |= 2; verboselog("maincpu", 3, "roz_scrolly = %08x\n", m_roz_scrolly); break;
		case 0x18a/2: m_roz_scrolly = (data) | (m_roz_scrolly & 0xffff0000); m_roz_changed |= 2; verboselog("maincpu", 3, "roz_scrolly = %08x\n", m_roz_scrolly); break;
		case 0x18c/2: m_roz_coeffa = data; verboselog("maincpu", 3, "roz_coeffa = %04x\n", data); break;
		case 0x18e/2: m_roz_coeffb = data; verboselog("maincpu", 3, "roz_coeffb = %04x\n", data); break;
		case 0x190/2: m_roz_coeffc = data; verboselog("maincpu", 3, "roz_coeffc = %04x\n", data); break;
		case 0x192/2: m_roz_coeffd = data; verboselog("maincpu", 3, "roz_coeffd = %04x\n", data); break;
		case 0x194/2: m_roz_base_addr = (data) << 1; verboselog("maincpu", 3, "roz_base_addr = %05x\n", data << 2); break;
		case 0x196/2: m_roz_tile_bank = data; verboselog("maincpu", 3, "roz_tile_bank = %04x\n", data); break; //tile bank
		case 0x198/2: m_roz_unk_base0 = data << 2; verboselog("maincpu", 3, "roz_unk_base0 = %05x\n", data << 2); break;
		case 0x19a/2: m_roz_unk_base1 = data << 2; verboselog("maincpu", 3, "roz_unk_base1 = %05x\n", data << 2); break;
		case 0x19e/2: m_roz_unk_base2 = data << 2; verboselog("maincpu", 3, "roz_unk_base2 = %05x\n", data << 2); break;

		case 0x1d0/2: m_unk_1d0 = data; verboselog("maincpu", 3, "unk_1d0 = %04x\n", data); break;




		case 0x1f0/2: //FIXME: this register is mostly not understood
			m_irq_mask = data;//(data & 8) ? 0 : 1;
#if 0
			if(!m_irq_mask && !m_hbl_mask)
			{
				m_maincpu->set_input_line(7, CLEAR_LINE);
			}
#endif
			verboselog("maincpu", 3, "irq_mask = %04x\n", data);
			break;
		default:
			verboselog("maincpu", 0, "video_w: Unknown register: %08x = %04x & %04x\n", 0xf00000 + (offset << 1), data, mem_mask);
			break;
	}
//  m_video_regs[offset] = data;
}


DEVICE_IMAGE_LOAD_MEMBER( supracan_state, supracan_cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	if (size > 0x400000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}


void supracan_state::machine_start()
{
	m_video_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supracan_state::supracan_video_callback),this));
	m_hbl_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supracan_state::supracan_hbl_callback),this));
	m_line_on_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supracan_state::supracan_line_on_callback),this));
	m_line_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supracan_state::supracan_line_off_callback),this));

	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x000000, 0x3fffff, read16_delegate(FUNC(generic_slot_device::read16_rom),(generic_slot_device*)m_cart));
}


void supracan_state::machine_reset()
{
	m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_video_timer->adjust( machine().first_screen()->time_until_pos( 0, 0 ) );
	m_irq_mask = 0;
}

/* gfxdecode is retained for reference purposes but not otherwise used by the driver */
static const gfx_layout supracan_gfx8bpp =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ STEP8(0,8*8) },
	8*8*8
};



static const gfx_layout supracan_gfx4bpp =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout supracan_gfx2bpp =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,1 },
	{ 0*2, 1*2, 2*2, 3*2, 4*2, 5*2, 6*2, 7*2 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const UINT32 xtexlayout_xoffset[64] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
												24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
												45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 };

static const UINT32 xtexlayout_yoffset[64] = {  0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,8*64,
												9*64,10*64,11*64,12*64,13*64,14*64,15*64,
												16*64,17*64,18*64,19*64,20*64,21*64,22*64,23*64,
												24*64,25*64,26*64,27*64,28*64,29*64,30*64,31*64,
												32*64,33*64,34*64,35*64,36*64,37*64,38*64,39*64,
												40*64,41*64,42*64,43*64,44*64,45*64,46*64,47*64,
						48*64,49*64,50*64,51*64,52*64,53*64,54*64,55*64,
												56*64,57*64,58*64,59*64,60*64,61*64,62*64,63*64 };
static const gfx_layout supracan_gfx1bpp =
{
	64,64,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	64*64,
	xtexlayout_xoffset,
	xtexlayout_yoffset
};

static const gfx_layout supracan_gfx1bpp_alt =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( supracan )
	GFXDECODE_RAM( "vram",  0, supracan_gfx8bpp,   0, 1 )
	GFXDECODE_RAM( "vram",  0, supracan_gfx4bpp,   0, 0x10 )
	GFXDECODE_RAM( "vram",  0, supracan_gfx2bpp,   0, 0x40 )
	GFXDECODE_RAM( "vram",  0, supracan_gfx1bpp,   0, 0x80 )
	GFXDECODE_RAM( "vram",  0, supracan_gfx1bpp_alt,   0, 0x80 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(supracan_state::supracan_irq)
{
#if 0

	if(m_irq_mask)
	{
		device.execute().set_input_line(7, HOLD_LINE);
	}
#endif
}

INTERRUPT_GEN_MEMBER(supracan_state::supracan_sound_irq)
{
	m_sound_irq_source_reg |= 0x80;

	if(m_sound_irq_enable_reg & m_sound_irq_source_reg)
	{
		m_soundcpu->set_input_line(0, ASSERT_LINE);
	}
	else
	{
		m_soundcpu->set_input_line(0, CLEAR_LINE);
	}
}

static MACHINE_CONFIG_START( supracan, supracan_state )

	MCFG_CPU_ADD( "maincpu", M68000, XTAL_10_738635MHz )        /* Correct frequency unknown */
	MCFG_CPU_PROGRAM_MAP( supracan_mem )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", supracan_state,  supracan_irq)

	MCFG_CPU_ADD( "soundcpu", M6502, XTAL_3_579545MHz )     /* TODO: Verify actual clock */
	MCFG_CPU_PROGRAM_MAP( supracan_sound_mem )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", supracan_state,  supracan_sound_irq)

#if !(SOUNDCPU_BOOT_HACK)
	MCFG_QUANTUM_PERFECT_CPU("maincpu")
	MCFG_QUANTUM_PERFECT_CPU("soundcpu")
#endif

	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_738635MHz/2, 348, 0, 256, 256, 0, 240 )  /* No idea if this is correct */
	MCFG_SCREEN_UPDATE_DRIVER(supracan_state, screen_update_supracan)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD( "palette", 32768 )
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_PALETTE_INIT_OWNER(supracan_state, supracan)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", supracan)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "supracan_cart")
	MCFG_GENERIC_WIDTH(GENERIC_ROM16_WIDTH)
	MCFG_GENERIC_ENDIAN(ENDIANNESS_BIG)
	MCFG_GENERIC_LOAD(supracan_state, supracan_cart)

	MCFG_SOFTWARE_LIST_ADD("cart_list","supracan")
MACHINE_CONFIG_END


ROM_START( supracan )
ROM_END


/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT     INIT    COMPANY                  FULLNAME        FLAGS */
CONS( 1995, supracan,   0,      0,      supracan,   supracan, driver_device, 0,      "Funtech Entertainment", "Super A'Can",  MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
