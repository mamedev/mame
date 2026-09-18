// license:BSD-3-Clause
// copyright-holders:Mark McDougall, Angelo Salese

/*
 *    Yamaha YGV608 - PVDC2 Pattern mode Video Display Controller 2
 *    - Mark McDougall
 *
 *    Notes:
 *    ======
 *
 *    This implementation is far from complete.
 *    There's enough here to emulate Namco ND-1 games.
 *    Some functionality is missing, some is incomplete.
 *    Also missing for ND-1 is rotation and scaling (cosmetic only).
 *
 *    It could also do with some optimisation for speed!
 *
 *    (Still lots of debugging info/options in here!)
 *
 *    TODO: soon
 *    create tilemaps at vh_start time and switch between at runtime
 *    mark tiles dirty when VRAM is written to instead of dirtying entire
 *       screen each frame
 *
 *    T.B.D. (not critical to ND-1)
 *    ======
 *
 *    Rotation
 *    Scaling
 *    Split-screen scrolling by row finer than one tile (tilemap is 1 X per tile row)
 *    Everything else! :)
 *
 *    TODO (2017-2018 edition):
 *    - move ports into device_address_map (done);
 *    - add registers into own space, improve naming and variable usage;
 *    - remove code repetition in tilemap drawing functions;
 *    - add crtc section (done partially);
 *    - fix garbage tiles in Mappy Arrange (done)
 *    - fix tile encryption for Abnormal Check (sets extra bit in cuskey);
 *      nopping bit 0 writes to 0x40081e makes gfxs to draw better!?
 *    - fix Gynotai row scroll glitches (pixmap blit; SLH/SLV screen strips; HDS/RLSC wrap);
 *    - fix attract mode garbage for Namco Collection Vol. 2 (either transparent or page banking select registers) (done);
 *    - fix tilemap dirty flags, move tilemap data in own space probably helps;
 *    - DMA from/to ROM;
 *    - color palette accessors presumably accesses an internal RAMDAC with controllable auto-increment, convert to that;
 *    - fix char getting cut off from GAME SELECT msg in NCV2 (done, sprite wraparound for sx & sy);
 *    - clean-ups & documentation;
 *
 *
 */


#include "emu.h"
#include "ygv608.h"

#include "screen.h"



// TODO: move these into enums
// R#7(md)
static constexpr u8 MD_2PLANE_8BIT      = 0x00;
static constexpr u8 MD_2PLANE_16BIT     = 0x01;
static constexpr u8 MD_1PLANE_16COLOUR  = 0x02;
static constexpr u8 MD_1PLANE_256COLOUR = 0x03;
static constexpr u8 MD_1PLANE           = (MD_1PLANE_16COLOUR & MD_1PLANE_256COLOUR);
static constexpr u8 MD_SHIFT            = 0;
static constexpr u8 MD_MASK             = 0x03;

// R#8
static constexpr u8 PGS_64X32 = 0x0;
static constexpr u8 PGS_32X64 = 0x1;
static constexpr u8 PGS_SHIFT = 0;
static constexpr u8 PGS_MASK  = 0x01;

// R#9
static constexpr u8 SLV_SCREEN = 0x00;
static constexpr u8 SLV_8      = 0x04;
static constexpr u8 SLV_16     = 0x05;
static constexpr u8 SLV_32     = 0x06;
static constexpr u8 SLV_64     = 0x07;
static constexpr u8 SLH_SCREEN = 0x00;
static constexpr u8 SLH_8      = 0x04;
static constexpr u8 SLH_16     = 0x05;
static constexpr u8 SLH_32     = 0x06;
static constexpr u8 SLH_64     = 0x07;
static constexpr u8 PTS_8X8    = 0x00;
static constexpr u8 PTS_16X16  = 0x01;
static constexpr u8 PTS_32X32  = 0x02;
static constexpr u8 PTS_64X64  = 0x03;

// R#10
static constexpr bool SPAS_SPRITESIZE = false;
static constexpr bool SPAS_SPRITEREVERSE = true;

// R#10(spas)=1
static constexpr u8 SZ_8X8   = 0x00;
static constexpr u8 SZ_16X16 = 0x01;
static constexpr u8 SZ_32X32 = 0x02;
static constexpr u8 SZ_64X64 = 0x03;

// R#10(spas)=0
static constexpr u8 SZ_NOREVERSE    = 0x00;
static constexpr u8 SZ_VERTREVERSE  = 0x01;
static constexpr u8 SZ_HORIZREVERSE = 0x02;
static constexpr u8 SZ_BOTHREVERSE  = 0x03;

// R#11(prm)
static constexpr u8 PRM_SABDEX = 0x00;
static constexpr u8 PRM_ASBDEX = 0x01;
static constexpr u8 PRM_SEABDX = 0x02;
static constexpr u8 PRM_ASEBDX = 0x03;

// R#40
static constexpr u8 HDW_SHIFT = 0;
static constexpr u8 HDW_MASK  = 0x3f;

// R#44
static constexpr u8 VDW_SHIFT = 0;
static constexpr u8 VDW_MASK  = 0x3f;

static constexpr bool ENABLE_SPRITES = true;
//#define _SHOW_VIDEO_DEBUG

static constexpr u8 GFX_8X8_4BIT   = 0;
static constexpr u8 GFX_16X16_4BIT = 1;
static constexpr u8 GFX_32X32_4BIT = 2;
static constexpr u8 GFX_64X64_4BIT = 3;
static constexpr u8 GFX_8X8_8BIT   = 4;
static constexpr u8 GFX_16X16_8BIT = 5;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(YGV608, ygv608_device, "ygv608", "Yamaha YGV608 PVDC2")

/* text-layer characters */

static const u32 pts_4bits_layout_xoffset[64] =
{
	STEP8(0, 4), STEP8(8*8*4, 4), STEP8(4*8*8*4, 4), STEP8(5*8*8*4, 4),
	STEP8(16*8*8*4, 4), STEP8(17*8*8*4, 4), STEP8(20*8*8*4, 4), STEP8(21*8*8*4, 4)
};

static const u32 pts_4bits_layout_yoffset[64] =
{
	STEP8(0, 8*4), STEP8(2*8*8*4, 8*4), STEP8(8*8*8*4, 8*4), STEP8(10*8*8*4, 8*4),
	STEP8(32*8*8*4, 8*4), STEP8(34*8*8*4, 8*4), STEP8(40*8*8*4, 8*4), STEP8(42*8*8*4, 8*4)
};

static const gfx_layout pts_32x32_4bits_layout =
{
	32,32,        /* 32*32 pixels */
	RGN_FRAC(1,1),         /* 4096 patterns */
	4,            /* 4 bits per pixel */
	{ STEP4(0, 1) },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	32*32*4,
	pts_4bits_layout_xoffset,
	pts_4bits_layout_yoffset
};

static const gfx_layout pts_64x64_4bits_layout =
{
	64,64,        /* 32*32 pixels */
	RGN_FRAC(1,1),         /* 1024 patterns */
	4,            /* 4 bits per pixel */
	{ STEP4(0, 1) },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	64*64*4,
	pts_4bits_layout_xoffset,
	pts_4bits_layout_yoffset
};

static const gfx_layout pts_16x16_8bits_layout =
{
	16,16,        /* 16*16 pixels */
	RGN_FRAC(1,1),         /* 8192 patterns */
	8,            /* 8 bits per pixel */
	{ STEP8(0, 1) },
	{ STEP8(0, 8), STEP8(8*8*8, 8) },
	{ STEP8(0, 8*8), STEP8(2*8*8*8, 8*8) },
	16*16*8
};

static GFXDECODE_START(gfx_ygv608)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_8x8x4_packed_msb,               0, 16)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_8x8x4_row_2x2_group_packed_msb, 0, 16)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, pts_32x32_4bits_layout,             0, 16)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, pts_64x64_4bits_layout,             0, 16)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_8x8x8_raw,                      0,  1)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, pts_16x16_8bits_layout,             0,  1)
GFXDECODE_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

/***************************************
 *
 * Internal I/O register structure
 *
 ***************************************/

 // we use decimals here to match documentation
void ygv608_device::regs_map(address_map &map)
{
	// address pointers
	map(0, 0).rw(FUNC(ygv608_device::pattern_name_table_y_r), FUNC(ygv608_device::pattern_name_table_y_w));
	map(1, 1).rw(FUNC(ygv608_device::pattern_name_table_x_r), FUNC(ygv608_device::pattern_name_table_x_w));

	map(2, 2).rw(FUNC(ygv608_device::ram_access_ctrl_r), FUNC(ygv608_device::ram_access_ctrl_w));

	map(3, 3).rw(FUNC(ygv608_device::sprite_address_r), FUNC(ygv608_device::sprite_address_w));
	map(4, 4).rw(FUNC(ygv608_device::scroll_address_r), FUNC(ygv608_device::scroll_address_w));
	map(5, 5).rw(FUNC(ygv608_device::palette_address_r), FUNC(ygv608_device::palette_address_w));
	map(6, 6).rw(FUNC(ygv608_device::sprite_bank_r), FUNC(ygv608_device::sprite_bank_w));

	// screen control
	map(7, 7).rw(FUNC(ygv608_device::screen_ctrl_7_r), FUNC(ygv608_device::screen_ctrl_7_w));
	map(8, 8).rw(FUNC(ygv608_device::screen_ctrl_8_r), FUNC(ygv608_device::screen_ctrl_8_w));
	map(9, 9).rw(FUNC(ygv608_device::screen_ctrl_9_r), FUNC(ygv608_device::screen_ctrl_9_w));
	map(10, 10).rw(FUNC(ygv608_device::screen_ctrl_10_r), FUNC(ygv608_device::screen_ctrl_10_w));
	map(11, 11).rw(FUNC(ygv608_device::screen_ctrl_11_r), FUNC(ygv608_device::screen_ctrl_11_w));
	map(12, 12).rw(FUNC(ygv608_device::screen_ctrl_12_r), FUNC(ygv608_device::screen_ctrl_12_w));

	map(13, 13).w(FUNC(ygv608_device::border_color_w));
	// interrupt section
	map(14, 14).rw(FUNC(ygv608_device::irq_mask_r), FUNC(ygv608_device::irq_mask_w));
	map(15, 16).rw(FUNC(ygv608_device::irq_ctrl_r), FUNC(ygv608_device::irq_ctrl_w));
	// base address
	map(17, 24).w(FUNC(ygv608_device::base_address_w));

	// ROZ parameters
	map(25, 27).w(FUNC(ygv608_device::roz_ax_w));
	map(28, 29).w(FUNC(ygv608_device::roz_dx_w));
	map(30, 31).w(FUNC(ygv608_device::roz_dxy_w));
	map(32, 34).w(FUNC(ygv608_device::roz_ay_w));
	map(35, 36).w(FUNC(ygv608_device::roz_dy_w));
	map(37, 38).w(FUNC(ygv608_device::roz_dyx_w));

	// CRTC
	map(39, 46).w(FUNC(ygv608_device::crtc_w));
//  47-48 ROM transfer control - DMA source address
//  49 ROM transfer control - DMA size
}

/***************************************
 *
 * Port Interface map
 *
 ***************************************/

void ygv608_device::port_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(ygv608_device::pattern_name_table_r), FUNC(ygv608_device::pattern_name_table_w));
	map(0x01, 0x01).rw(FUNC(ygv608_device::sprite_data_r), FUNC(ygv608_device::sprite_data_w));
	map(0x02, 0x02).rw(FUNC(ygv608_device::scroll_data_r), FUNC(ygv608_device::scroll_data_w));
	map(0x03, 0x03).rw(FUNC(ygv608_device::palette_data_r), FUNC(ygv608_device::palette_data_w));
	map(0x04, 0x04).rw(FUNC(ygv608_device::register_data_r), FUNC(ygv608_device::register_data_w));
	map(0x05, 0x05).nopr().w(FUNC(ygv608_device::register_select_w));
	map(0x06, 0x06).rw(FUNC(ygv608_device::status_port_r), FUNC(ygv608_device::status_port_w));
	map(0x07, 0x07).rw(FUNC(ygv608_device::system_control_r), FUNC(ygv608_device::system_control_w));
}


//-------------------------------------------------
//  ygv608_device - constructor
//-------------------------------------------------

ygv608_device::ygv608_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, YGV608, tag, owner, clock),
	device_gfx_interface(mconfig, *this, gfx_ygv608, DEVICE_SELF),
	device_memory_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_io_space_config("io", ENDIANNESS_BIG, 8, 6, 0, address_map_constructor(FUNC(ygv608_device::regs_map), this)),
	m_gfxbank_cb(*this),
	m_tilemap_cache_8{{nullptr, nullptr, nullptr}, {nullptr, nullptr, nullptr}},
	m_tilemap_cache_16{{nullptr, nullptr, nullptr}, {nullptr, nullptr, nullptr}},
	m_tilemap{nullptr, nullptr},
	m_work_bitmap(0),
	m_bits16(0),
	m_page_x(0),
	m_page_y(0),
	m_pny_shift(0),
	m_na8_mask(0),
	m_col_shift(31),
	m_row_shift(31),
	m_base_y_shift(0),
	m_screen_resize(false),
	m_tilemap_resize(false),
	m_color_state_r(0),
	m_color_state_w(0),
	m_p0_state(0),
	m_pattern_name_base_r(0),
	m_pattern_name_base_w(0),
	m_screen_status(0),
	m_dma_status(0),
	m_register_address(0),
	m_register_autoinc_r(false),
	m_register_autoinc_w(false),
	m_raster_irq_mask(false),
	m_vblank_irq_mask(false),
	m_raster_irq_hpos(0),
	m_raster_irq_vpos(0),
	m_raster_irq_mode(false),
	m_scroll_address(0),
	m_palette_address(0),
	m_sprite_address(0),
	m_sprite_bank(0),
	m_xtile_ptr(0),
	m_ytile_ptr(0),
	m_xtile_autoinc(false),
	m_ytile_autoinc(false),
	m_plane_select_access(false),
	m_mosaic_plane{0},
	m_sprite_disable(0),
	m_sprite_aux_mode(0),
	m_sprite_aux_reg(0),
	m_border_color(0),
	m_saar(false),
	m_saaw(false),
	m_scar(false),
	m_scaw(false),
	m_cpar(false),
	m_cpaw(false),
	m_ba_plane_scroll_select(false),
	m_dspe(false),
	m_md(0),
	m_zron(false),
	m_flip(false),
	m_dckm(false),
	m_page_size(false),
	m_h_display_size(0),
	m_v_display_size(0),
	m_roz_wrap_disable(false),
	m_scroll_wrap_disable(false),
	m_pattern_size(0),
	m_h_div_size(0),
	m_v_div_size(0),
	m_plane_trans_enable{false},
	m_priority_mode(0),
	m_cbdr(false),
	m_yse(false),
	m_scm(0),
	m_plane_color_fetch{0},
	m_sprite_color_fetch(0),
	m_vblank_handler(*this),
	m_raster_handler(*this),
	m_vblank_timer(nullptr),
	m_raster_timer(nullptr),
	m_ax(0),
	m_dx(0),
	m_dxy(0),
	m_ay(0),
	m_dy(0),
	m_dyx(0),
	m_raw_ax(0),
	m_raw_dx(0),
	m_raw_dxy(0),
	m_raw_ay(0),
	m_raw_dy(0),
	m_raw_dyx(0)
{
	std::fill(std::begin(m_pattern_name_table), std::end(m_pattern_name_table), 0);

	for (int i = 0; i < 2; i++)
	{
		std::fill(std::begin(m_scroll_data_table[i]), std::end(m_scroll_data_table[i]), 0);
		std::fill(std::begin(m_base_addr[i]), std::end(m_base_addr[i]), 0);
	}

	for (int i = 0; i < 256; i++)
		std::fill(std::begin(m_colour_palette[i]), std::end(m_colour_palette[i]), 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void ygv608_device::device_start()
{
	m_gfxbank_cb.resolve();

//  memset(&m_ports, 0, sizeof(m_ports));
//  memset(&m_regs, 0, sizeof(m_regs));
	memset(&m_pattern_name_table, 0, sizeof(m_pattern_name_table));
	memset(&m_sprite_attribute_table, 0, sizeof(m_sprite_attribute_table));

	memset(&m_scroll_data_table, 0, sizeof(m_scroll_data_table));
	memset(&m_colour_palette, 0, sizeof(m_colour_palette));

	m_bits16 = 0;
	m_page_x = 0;
	m_page_y = 0;
	m_pny_shift = 0;
	m_na8_mask = 0;
	m_col_shift = 31;
	m_row_shift = 31;

	m_ax = 0; m_dx = 0; m_dxy = 0; m_ay = 0; m_dy = 0; m_dyx = 0;

	memset(&m_base_addr, 0, sizeof(m_base_addr));
	m_base_y_shift = 0;

	// flag rebuild of the tilemaps
	m_screen_resize = true;
	m_tilemap_resize = true;

	/* create tilemaps of all sizes and combinations */
	m_tilemap_cache_8[0][0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_A_8)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  8,8, 32,32);
	m_tilemap_cache_8[0][1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_A_8)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  8,8, 64,32);
	m_tilemap_cache_8[0][2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_A_8)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  8,8, 32,64);

	m_tilemap_cache_16[0][0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_A_16)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  16,16, 32,32);
	m_tilemap_cache_16[0][1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_A_16)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  16,16, 64,32);
	m_tilemap_cache_16[0][2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_A_16)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  16,16, 32,64);

	m_tilemap_cache_8[1][0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_B_8)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  8,8, 32,32);
	m_tilemap_cache_8[1][1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_B_8)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  8,8, 64,32);
	m_tilemap_cache_8[1][2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_B_8)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  8,8, 32,64);

	m_tilemap_cache_16[1][0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_B_16)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  16,16, 32,32);
	m_tilemap_cache_16[1][1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_B_16)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  16,16, 64,32);
	m_tilemap_cache_16[1][2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(ygv608_device::get_tile_info_B_16)), tilemap_mapper_delegate(*this, FUNC(ygv608_device::get_tile_offset)),  16,16, 32,64);

	m_tilemap[0] = nullptr;
	m_tilemap[1] = nullptr;

	m_iospace = &space(AS_IO);

	// TODO: tagging configuration
	m_vblank_timer = timer_alloc(FUNC(ygv608_device::update_vblank_flag), this);
	m_raster_timer = timer_alloc(FUNC(ygv608_device::update_raster_flag), this);

	register_state_save();
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ygv608_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_IO, &m_io_space_config)
	};
}

inline void ygv608_device::vblank_irq_check()
{
	if (m_vblank_irq_mask && m_screen_status & 8)
		m_vblank_handler(ASSERT_LINE);
}

inline void ygv608_device::raster_irq_check()
{
	if (m_raster_irq_mask && m_screen_status & 0x10)
		m_raster_handler(ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(ygv608_device::update_vblank_flag)
{
	m_screen_status |= 8; // FV
	vblank_irq_check();
}

TIMER_CALLBACK_MEMBER(ygv608_device::update_raster_flag)
{
	// Latch the lines drawn so far before the handler changes the scroll tables.
	// ncv1's scroll test ($E5E08) walks IV by 2 and retargets SCA $80 on each hit.
	const int y = screen().vpos() - 1;
	if (y >= 0)
		screen().update_partial(y);

	m_screen_status |= 0x10; // FP
	raster_irq_check();

	// adjust for next one shot
	m_raster_timer->reset();
	m_raster_timer->adjust(raster_sync_offset(), 0);
}

void ygv608_device::set_tilemap_dirty()
{
	m_tilemap_resize = true;
}

int ygv608_device::scroll_table_shift(u8 div_size) const
{
	// R#9 SLH/SLV: 0 (and the undefined 1-3) = entire screen; 4/5/6/7 = 8/16/32/64
	// dot strips.  The strip boundaries are in *display dots*, not in tiles, and the
	// blit samples the pixmap in screen space, so the table index is a plain shift of
	// the screen coordinate and does not involve PTS at all:
	//     4 -> >>3 (8 dots) ... 7 -> >>6 (64 dots)
	// The old tile-unit form (div-4-PTS) could not express a strip finer than one
	// tile (e.g. abcheck's SLH=8px helper running while PTS=16x16) and silently fell
	// back to slot 0 for the whole layer.
	if ((div_size & 4) == 0)
		return 31;                  // unused; get_*_division() short-circuits first

	return int(div_size) - 1;
}

// screen_x / screen_y are display dots, not tiles.
inline int ygv608_device::get_col_division(int screen_x)
{
	if ((m_v_div_size & 4) == 0)
		return 0;

	// Column table is 64 x 12-bit entries at $00-$7F.
	return ((screen_x >> m_col_shift) * 2) & 0x7f;
}

inline int ygv608_device::get_row_division(int screen_y)
{
	if ((m_h_div_size & 4) == 0)
		return 0;

	// Row table is 32 x 12-bit entries at $80-$BF (not page_y/2).
	return ((screen_y >> m_row_shift) * 2) & 0x3f;
}

TILEMAP_MAPPER_MEMBER(ygv608_device::get_tile_offset)
{
	// this optimisation is not much good to us,
	// since we really need row,col in the get_tile_info() routines
	// - so just pack them into a u32

	return ((col << 6) | row);
}

TILE_GET_INFO_MEMBER(ygv608_device::get_tile_info_A_8)
{
	// extract row,col packed into tile_index
	const int col = tile_index >> 6;
	const int row = tile_index & 0x3f;
	const int translated_column = get_col_division(col * 8);

	u8 attr = 0;
	const int pattern_name_base = 0;
	const int set = (m_md == MD_1PLANE_256COLOUR) ? GFX_8X8_8BIT : GFX_8X8_4BIT;
	const int base = row >> m_base_y_shift;

	if (col >= m_page_x)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else if (row >= m_page_y)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else
	{
		int page;
		const int i = pattern_name_base + (((row << m_pny_shift) + col) << m_bits16);
		int j = m_pattern_name_table[i];
		int f = 0;

		if (m_bits16)
		{
			j += ((int)(m_pattern_name_table[i + 1] & m_na8_mask)) << 8;
			// attribute only valid in 16 color mode
			if (set == GFX_8X8_4BIT)
				attr = m_pattern_name_table[i + 1] >> 4;

			if (m_flip)
			{
				f = TILE_FLIPXY(m_pattern_name_table[i + 1] >> 2);
			}
		}

		/* calculate page according to scroll data */
		/* Slot 0 is only the whole-screen camera.  Once SLH/SLV strips are on,
		   each strip carries its own X/Y, so baking a page from slot 0 here and
		   then adding the per-strip scroll in the blit would shift twice. */
		if ((m_v_div_size | m_h_div_size) & 4)
		{
			page = 0;
		}
		else
		{
			const int sy = (int)m_scroll_data_table[0][translated_column] +
					(((int)m_scroll_data_table[0][translated_column + 1] & 0x0f) << 8);
			const int sx = (int)m_scroll_data_table[0][0x80] +
					(((int)m_scroll_data_table[0][0x81] & 0x0f) << 8);

			if (m_md == MD_2PLANE_16BIT)
			{
				page = ((sx + col * 8) & 0x3ff) >> 8;
				page += (((sy + row * 8) & 0x7ff) >> 8) * 4;
			}
			else if (m_page_size)
			{
				page = ((sx + col * 8) & 0x7ff) >> 9;
				page += (((sy + row * 8) & 0x7ff) >> 8) * 4;
			}
			else
			{
				page = ((sx + col * 8) & 0x7ff) >> 8;
				page += (((sy + row * 8) & 0x7ff) >> 9) * 8;
			}
		}

		page &= 0x1f;

		/* add page, base address to pattern name */
		j += ((int)m_scroll_data_table[0][0xc0 + page] << 10);
		j += (m_base_addr[0][base] << 8);
		const int addr_shift = (set == GFX_8X8_4BIT) ? 5 : 6;
		if (!m_gfxbank_cb.isnull())
			j = m_gfxbank_cb((j << addr_shift) & 0x1fffff) >> addr_shift;

		if (j >= gfx(set)->elements())
		{
			logerror("A_8X8: tilemap=%d\n", j);
			j = 0;
		}
		if (m_plane_color_fetch[0] != 0)
		{
			// attribute only valid in 16 color mode
			if (set == GFX_8X8_4BIT)
				attr = (j >> ((m_plane_color_fetch[0] - 1) * 2)) & 0x0f;
		}

		tileinfo.set(set, j, attr & 0x0F, f);
	}
}

TILE_GET_INFO_MEMBER(ygv608_device::get_tile_info_B_8)
{
	// extract row,col packed into tile_index
	const int col = tile_index >> 6;
	const int row = tile_index & 0x3f;
	const int translated_column = get_col_division(col * 8);

	u8 attr = 0;
	const int pattern_name_base = ((m_page_y << m_pny_shift) << m_bits16);
	const int set = GFX_8X8_4BIT;
	const int base = row >> m_base_y_shift;

	if (m_md & MD_1PLANE)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else if (col >= m_page_x)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else if (row >= m_page_y)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else
	{
		int page;
		const int i = pattern_name_base + (((row << m_pny_shift) + col) << m_bits16);
		int j = m_pattern_name_table[i];
		int f = 0;

		if (m_bits16)
		{
			j += ((int)(m_pattern_name_table[i + 1] & m_na8_mask)) << 8;
			attr = m_pattern_name_table[i + 1] >> 4; /*& 0x00; 0xf0;*/

			if (m_flip)
			{
				f = TILE_FLIPXY(m_pattern_name_table[i + 1] >> 2);
			}
		}

		/* calculate page according to scroll data */
		/* Slot 0 is only the whole-screen camera.  Once SLH/SLV strips are on,
		   each strip carries its own X/Y, so baking a page from slot 0 here and
		   then adding the per-strip scroll in the blit would shift twice. */
		if ((m_v_div_size | m_h_div_size) & 4)
		{
			page = 0;
		}
		else
		{
			const int sy = (int)m_scroll_data_table[1][translated_column] +
					(((int)m_scroll_data_table[1][translated_column + 1] & 0x0f) << 8);
			const int sx = (int)m_scroll_data_table[1][0x80] +
					(((int)m_scroll_data_table[1][0x81] & 0x0f) << 8);

			if (m_md == MD_2PLANE_16BIT)
			{
				page = ((sx + col * 8) & 0x3ff) >> 8;
				page += (((sy + row * 8) & 0x7ff) >> 8) * 4;
			}
			else if (m_page_size)
			{
				page = ((sx + col * 8) & 0x7ff) >> 9;
				page += (((sy + row * 8) & 0x7ff) >> 8) * 4;
			}
			else
			{
				page = ((sx + col * 8) & 0x7ff) >> 8;
				page += (((sy + row * 8) & 0x7ff) >> 9) * 8;
			}
		}

		page &= 0x1f;

		/* add page, base address to pattern name */
		j += ((int)m_scroll_data_table[1][0xc0 + page] << 10);
		j += (m_base_addr[1][base] << 8);
		const int addr_shift = (set == GFX_8X8_4BIT) ? 5 : 6;
		if (!m_gfxbank_cb.isnull())
			j = m_gfxbank_cb((j << addr_shift) & 0x1fffff) >> addr_shift;

		if (j >= gfx(set)->elements())
		{
			logerror("B_8X8: tilemap=%d\n", j);
			j = 0;
		}
		if (m_plane_color_fetch[1] != 0)
		{
			/* assume 16 colour mode for now... */
			attr = (j >> ((m_plane_color_fetch[1] - 1) * 2)) & 0x0f;
		}

		tileinfo.set(set, j, attr, f);
	}
}

TILE_GET_INFO_MEMBER(ygv608_device::get_tile_info_A_16)
{
	// extract row,col packed into tile_index
	const int col = tile_index >> 6;
	const int row = tile_index & 0x3f;
	const int translated_column = get_col_division(col * 16);

	u8 attr = 0;
	const int pattern_name_base = 0;
	const int set = (m_md == MD_1PLANE_256COLOUR) ? GFX_16X16_8BIT : GFX_16X16_4BIT;
	const int base = row >> m_base_y_shift;

	if (col >= m_page_x)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else if (row >= m_page_y)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else
	{
		int page;
		const int i = pattern_name_base + (((row << m_pny_shift) + col) << m_bits16);
		int j = m_pattern_name_table[i];
		int f = 0;

		if (m_bits16)
		{
			j += ((int)(m_pattern_name_table[i + 1] & m_na8_mask)) << 8;
			// attribute only valid in 16 color mode
			if (set == GFX_16X16_4BIT)
				attr = m_pattern_name_table[i + 1] >> 4;

			if (m_flip)
			{
				f = TILE_FLIPXY(m_pattern_name_table[i + 1] >> 2);
			}
		}

		/* calculate page according to scroll data */
		/* Slot 0 is only the whole-screen camera.  Once SLH/SLV strips are on,
		   each strip carries its own X/Y, so baking a page from slot 0 here and
		   then adding the per-strip scroll in the blit would shift twice. */
		if ((m_v_div_size | m_h_div_size) & 4)
		{
			page = 0;
		}
		else
		{
			const int sy = (int)m_scroll_data_table[0][translated_column] +
					(((int)m_scroll_data_table[0][translated_column + 1] & 0x0f) << 8);
			const int sx = (int)m_scroll_data_table[0][0x80] +
					(((int)m_scroll_data_table[0][0x81] & 0x0f) << 8);

			if (m_md == MD_2PLANE_16BIT)
			{
				page = ((sx + col * 16) & 0x7ff) >> 9;
				page += ((sy + row * 16) >> 9) * 4;
			}
			else if (m_page_size)
			{
				page = (sx + col * 16) >> 9;
				page += ((sy + row * 16) >> 10) * 8;
			}
			else
			{
				page = (sx + col * 16) >> 10;
				page += ((sy + row * 16) >> 9) * 4;
			}
		}

		page &= 0x1f;

		/* add page, base address to pattern name */
		j += ((int)m_scroll_data_table[0][0xc0 + page] << 8);
		j += (m_base_addr[0][base] << 8);
		const int addr_shift = (set == GFX_16X16_4BIT) ? 7 : 8;
		if (!m_gfxbank_cb.isnull())
			j = m_gfxbank_cb((j << addr_shift) & 0x1fffff) >> addr_shift;

		if (j >= gfx(set)->elements())
		{
			logerror("A_16X16: tilemap=%d\n", j);
			j = 0;
		}
		if (m_plane_color_fetch[0] != 0)
		{
			// attribute only valid in 16 color mode
			if (set == GFX_16X16_4BIT)
				attr = (j >> (m_plane_color_fetch[0] * 2)) & 0x0f;
		}

		tileinfo.set(set, j, attr, f);
	}
}

TILE_GET_INFO_MEMBER(ygv608_device::get_tile_info_B_16)
{
	// extract row,col packed into tile_index
	const int col = tile_index >> 6;
	const int row = tile_index & 0x3f;
	const int translated_column = get_col_division(col * 16);

	u8 attr = 0;
	const int pattern_name_base = ((m_page_y << m_pny_shift) << m_bits16);
	const int set = GFX_16X16_4BIT;
	const int base = row >> m_base_y_shift;

	if (m_md & MD_1PLANE)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	if (col >= m_page_x)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else if (row >= m_page_y)
	{
		tileinfo.set(set, 0, 0, 0);
	}
	else
	{
		int page;
		const int i = pattern_name_base + (((row << m_pny_shift) + col) << m_bits16);
		int j = m_pattern_name_table[i];
		int f = 0;

		if (m_bits16)
		{
			j += ((int)(m_pattern_name_table[i + 1] & m_na8_mask)) << 8;
			attr = m_pattern_name_table[i + 1] >> 4; /*& 0x00; 0xf0;*/

			if (m_flip)
			{
				f = TILE_FLIPXY(m_pattern_name_table[i + 1] >> 2);
			}
		}

		/* calculate page according to scroll data */
		/* Slot 0 is only the whole-screen camera.  Once SLH/SLV strips are on,
		   each strip carries its own X/Y, so baking a page from slot 0 here and
		   then adding the per-strip scroll in the blit would shift twice. */
		if ((m_v_div_size | m_h_div_size) & 4)
		{
			page = 0;
		}
		else
		{
			const int sy = (int)m_scroll_data_table[1][translated_column] +
					(((int)m_scroll_data_table[1][translated_column + 1] & 0x0f) << 8);
			const int sx = (int)m_scroll_data_table[1][0x80] +
					(((int)m_scroll_data_table[1][0x81] & 0x0f) << 8);

			if (m_md == MD_2PLANE_16BIT)
			{
				page = ((sx + col * 16) & 0x7ff) >> 9;
				page += ((sy + row * 16) >> 9) * 4;
			}
			else if (m_page_size)
			{
				page = (sx + col * 16) >> 9;
				page += ((sy + row * 16) >> 10) * 8;
			}
			else
			{
				page = (sx + col * 16) >> 10;
				page += ((sy + row * 16) >> 9) * 4;
			}
		}

		page &= 0x1f;

		/* add page, base address to pattern name */
		j += ((int)m_scroll_data_table[1][0xc0 + page] << 8);
		j += (m_base_addr[1][base] << 8);
		const int addr_shift = (set == GFX_16X16_4BIT) ? 7 : 8;
		if (!m_gfxbank_cb.isnull())
			j = m_gfxbank_cb((j << addr_shift) & 0x1fffff) >> addr_shift;

		if (j >= gfx(set)->elements())
		{
			logerror("B_16X16: tilemap=%d\n", j);
			j = 0;
		}
		if (m_plane_color_fetch[1] != 0)
		{
			/* assume 16 colour mode for now... */
			attr = (j >> (m_plane_color_fetch[1] * 2)) & 0x0f;
		}

		tileinfo.set(set, j, attr, f);
	}
}

void ygv608_device::device_post_load()
{
	m_screen_resize = true;
	m_tilemap_resize = true;
}

void ygv608_device::register_state_save()
{
//  save_item(NAME(m_ports.b));
//  save_item(NAME(m_regs.b));
	save_item(NAME(m_pattern_name_table));
	save_item(NAME(m_sprite_attribute_table.b));
	save_item(NAME(m_scroll_data_table));
	save_item(NAME(m_colour_palette));
	save_item(NAME(m_color_state_r));
	save_item(NAME(m_color_state_w));
	save_item(NAME(m_bits16));
	save_item(NAME(m_page_x));
	save_item(NAME(m_page_y));
	save_item(NAME(m_pny_shift));
	save_item(NAME(m_na8_mask));
	save_item(NAME(m_col_shift));
	save_item(NAME(m_row_shift));
	save_item(NAME(m_base_addr));
	save_item(NAME(m_base_y_shift));
	save_item(NAME(m_screen_resize));
	save_item(NAME(m_tilemap_resize));
	save_item(NAME(m_p0_state));
	save_item(NAME(m_pattern_name_base_r));
	save_item(NAME(m_pattern_name_base_w));
	save_item(NAME(m_screen_status));
	save_item(NAME(m_dma_status));
	save_item(NAME(m_register_address));
	save_item(NAME(m_register_autoinc_r));
	save_item(NAME(m_register_autoinc_w));
	save_item(NAME(m_raster_irq_mask));
	save_item(NAME(m_vblank_irq_mask));
	save_item(NAME(m_raster_irq_hpos));
	save_item(NAME(m_raster_irq_vpos));
	save_item(NAME(m_raster_irq_mode));
	save_item(NAME(m_scroll_address));
	save_item(NAME(m_palette_address));
	save_item(NAME(m_sprite_address));
	save_item(NAME(m_sprite_bank));
	save_item(NAME(m_xtile_ptr));
	save_item(NAME(m_ytile_ptr));
	save_item(NAME(m_xtile_autoinc));
	save_item(NAME(m_ytile_autoinc));
	save_item(NAME(m_plane_select_access));
	save_item(NAME(m_mosaic_plane));
	save_item(NAME(m_sprite_disable));
	save_item(NAME(m_sprite_aux_mode));
	save_item(NAME(m_sprite_aux_reg));
	save_item(NAME(m_border_color));
	save_item(NAME(m_saar));
	save_item(NAME(m_saaw));
	save_item(NAME(m_scar));
	save_item(NAME(m_scaw));
	save_item(NAME(m_cpar));
	save_item(NAME(m_cpaw));
	save_item(NAME(m_ba_plane_scroll_select));
	save_item(NAME(m_dspe));
	save_item(NAME(m_md));
	save_item(NAME(m_zron));
	save_item(NAME(m_flip));
	save_item(NAME(m_dckm));
	save_item(NAME(m_page_size));
	save_item(NAME(m_h_display_size));
	save_item(NAME(m_v_display_size));
	save_item(NAME(m_roz_wrap_disable));
	save_item(NAME(m_scroll_wrap_disable));
	save_item(NAME(m_pattern_size));
	save_item(NAME(m_h_div_size));
	save_item(NAME(m_v_div_size));
	save_item(NAME(m_plane_trans_enable));
	save_item(NAME(m_priority_mode));
	save_item(NAME(m_cbdr));
	save_item(NAME(m_yse));
	save_item(NAME(m_scm));
	save_item(NAME(m_plane_color_fetch));
	save_item(NAME(m_sprite_color_fetch));
	save_item(NAME(m_crtc.htotal));
	save_item(NAME(m_crtc.vtotal));
	save_item(NAME(m_crtc.display_hstart));
	save_item(NAME(m_crtc.display_vstart));
	save_item(NAME(m_crtc.display_width));
	save_item(NAME(m_crtc.display_height));
	save_item(NAME(m_crtc.display_hsync));
	save_item(NAME(m_crtc.display_vsync));
	save_item(NAME(m_crtc.border_width));
	save_item(NAME(m_crtc.border_height));
	save_item(NAME(m_ax));
	save_item(NAME(m_dx));
	save_item(NAME(m_dxy));
	save_item(NAME(m_ay));
	save_item(NAME(m_dy));
	save_item(NAME(m_dyx));
	save_item(NAME(m_raw_ax));
	save_item(NAME(m_raw_dx));
	save_item(NAME(m_raw_dxy));
	save_item(NAME(m_raw_ay));
	save_item(NAME(m_raw_dy));
	save_item(NAME(m_raw_dyx));
}


void ygv608_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!ENABLE_SPRITES)
		return;

	static constexpr int sprite_limits[4] = { 512-8, 512-16, 512-32, 512-64 };
	static constexpr u8 bank_shift[4] = { 5, 7, 9, 11 };
	static constexpr int sprite_shift[4] = { 8, 6, 4, 2 };
	static constexpr int sprite_mask[4] = { 0xff, 0xfc, 0xf0, 0xc0 };
	static constexpr int spf_shift[4] = { -1, 0, +1, +2 };
	/* ensure that sprites are enabled */
	if (!m_dspe || m_sprite_disable)
		return;

	// sprites are always clipped to 512x512
	// - regardless of the visible display dimensions
	rectangle sprite_clip(0, 512, 0, 512);

	/* draw sprites */
	sprite_clip &= cliprect;
	SPRITE_ATTR *sa = &m_sprite_attribute_table.s[MAX_SPRITES - 1];
	for (int i = 0; i < MAX_SPRITES; i++, sa--)
	{
		int color = (sa->attr >> 4) & 0x0f;
		const int sx = ((int)(sa->attr & 0x02) << 7) | (int)sa->sx;
		const int sy = ((((int)(sa->attr & 0x01) << 8) | (int)sa->sy) + 1) & 0x1ff;
		const int attr = (sa->attr & 0x0c) >> 2;
		const int g_attr = m_sprite_aux_reg & 3;
		const int spf = m_sprite_color_fetch;

		bool flipx = false, flipy = false;
		int size = 0;
		if (m_sprite_aux_mode == SPAS_SPRITESIZE)
		{
			size = g_attr;
			flipx = (attr & SZ_HORIZREVERSE) != 0;
			flipy = (attr & SZ_VERTREVERSE) != 0;
		}
		else
		{
			size = attr;
			flipx = (g_attr & SZ_HORIZREVERSE) != 0;
			flipy = (g_attr & SZ_VERTREVERSE) != 0;
		}

		// calculate code and apply sprite base address
		int code = ((int)(m_sprite_bank & sprite_mask[size]) << sprite_shift[size]) | (int)sa->sn;
		// apply spf to color (invalidates individual attribute bits for color)
		if (spf != 0)
			color = (code >> ((spf + spf_shift[size]) * 2)) & 0x0f;
		if (!m_gfxbank_cb.isnull())
			code = m_gfxbank_cb((code << bank_shift[size]) & 0x1fffff) >> bank_shift[size];
		// check code boundary (TODO: do we really need this?)
		if (code >= gfx(size)->elements())
		{
			logerror("SZ_%d: sprite=%d\n", size, code);
			code = 0;
		}
		// draw the sprite
		gfx(size)->transpen(bitmap, sprite_clip,
			code,
			color,
			flipx, flipy,
			sx, sy, 0x00);
		// draw with wraparound
		if (sx > sprite_limits[size] || sy > sprite_limits[size])
		{
			gfx(size)->transpen(bitmap, sprite_clip,
					code,
					color,
					flipx, flipy,
					sx - 512, sy, 0x00);
			gfx(size)->transpen(bitmap, sprite_clip,
					code,
					color,
					flipx, flipy,
					sx, sy - 512, 0x00);
			gfx(size)->transpen(bitmap, sprite_clip,
					code,
					color,
					flipx, flipy,
					sx - 512, sy - 512, 0x00);
		}

	}
}

/***************************************
 *
 *  Tilemap blitting
 *
 *  Both planes are sampled by hand rather than through tilemap_t::draw() or
 *  draw_roz(), because the chip's coordinate space is not the pixmap's.  A
 *  sample takes two steps:
 *
 *    1) HDS/VDS (R#8) give the camera a 4096/2048/1024/512 dot space, and the
 *       scroll registers index THAT.  gynotai runs HDS=3, a 512 dot camera over
 *       a 1024 dot page, so a scroll of 480 has to wrap back to the left half;
 *    2) the result is then wrapped onto the page, or clipped when the game asks
 *       for that (RLSC for the scroll path, RLRT for ROZ).
 *
 *  tilemap_t only knows step 2.  Step 1 is what is hand-rolled here - along
 *  with everything tilemap_t used to do before its inner loop, the enable flag
 *  above all, which screen_update() drives from DSPE.
 *
 ***************************************/

namespace {

// R#8 HDS/VDS: width of the chip's coordinate space, in display dots.
static constexpr int k_camera_domain[4] = { 4096, 2048, 1024, 512 };

// Step 1: fold a coordinate into the HDS/VDS camera space.
inline int wrap_camera(int v, int domain)
{
	if (domain <= 0)
		return 0;

	v %= domain;
	return (v < 0) ? (v + domain) : v;
}

// Step 2: camera coordinate -> pixmap coordinate.  wrap == false means the chip
// clips instead of repeating, so off-map returns -1 and the caller leaves the
// pixel alone.
inline int camera_to_pixmap(int v, int map, int pix, bool wrap)
{
	if (map <= 0)
		return -1;

	if (wrap)
	{
		v %= map;
		if (v < 0)
			v += map;
	}
	else if (v < 0 || v >= map)
	{
		return -1;
	}

	return (v >= pix) ? -1 : v;
}

} // anonymous namespace

inline void ygv608_device::draw_layer_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int ba_select)
{
	tilemap_t *source_tilemap = m_tilemap[ba_select];

	const u32 sy = (int)m_scroll_data_table[ba_select][0x00] +
					(((int)m_scroll_data_table[ba_select][0x01] & 0x0f) << 8);
	const u32 sx = (int)m_scroll_data_table[ba_select][0x80] +
					(((int)m_scroll_data_table[ba_select][0x81] & 0x0f) << 8);

	if (m_zron)
	{
		// No 9-bit fold here. An earlier pass subtracted $200 from sx/sy whenever
		// SLV named a real strip size, to pull a runaway camera back on screen. That
		// is the same arithmetic the HDS/VDS camera wrap below now does - $200 IS the
		// HDS=3 domain - only keyed on a register that has nothing to do with ROZ.
		// ncv1 shows the damage: the game-select menu leaves SLV=5 behind ($01CA9E),
		// and the Xevious Arrangement title turns ZRON on ($0784AC) without clearing
		// it, so the fold displaced the logo by 512 dots until VBLANK2 got around to
		// zeroing SLV. Let the camera wrap handle it; sx/sy stay plain 12-bit.

		// tilemap_t::draw_roz() opens with "if (!m_enable) return;" and so must
		// this.  ncv1 holds DSPE low while it sets the Xevious Arrangement title
		// up, and the matrix is stale through that window - $2C86, the game's own
		// "identity" reset, clears DXY twice and never DYX - so drawing anyway put
		// a sheared logo on screen.
		if (!source_tilemap->enabled())
			return;

		const bitmap_ind16 &src = source_tilemap->pixmap();
		const bitmap_ind8 &flags = source_tilemap->flagsmap();
		const int tile_size = 8 << m_pattern_size;
		const int map_w = m_page_x * tile_size;
		const int map_h = m_page_y * tile_size;
		const int pix_w = src.width();
		const int pix_h = src.height();
		const int x_dom = k_camera_domain[m_h_display_size & 3];
		const int y_dom = k_camera_domain[m_v_display_size & 3];
		const bool wrap = (!m_roz_wrap_disable);   // R#8 RLRT

		const u32 startx = m_ax + (sx << 16);
		const u32 starty = m_ay + (sy << 16);

		// R#25-38 affine step, in the order tilemap_t::draw_roz() uses it:
		//     srcx = startx + x * DX  + y * DXY
		//     srcy = starty + x * DYX + y * DY
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			u32 curx = startx + u32(cliprect.min_x) * m_dx + u32(y) * m_dxy;
			u32 cury = starty + u32(cliprect.min_x) * m_dyx + u32(y) * m_dy;
			u16 *dest = &bitmap.pix(y, cliprect.min_x);

			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				int px = s32(curx) >> 16;
				int py = s32(cury) >> 16;

				if (wrap)
				{
					px = wrap_camera(px, x_dom);
					py = wrap_camera(py, y_dom);
				}

				px = camera_to_pixmap(px, map_w, pix_w, wrap);
				py = camera_to_pixmap(py, map_h, pix_h, wrap);

				if (px >= 0 && py >= 0 && (flags.pix(py, px) & TILEMAP_PIXEL_LAYER0))
					*dest = src.pix(py, px);

				dest++;
				curx += m_dx;
				cury += m_dyx;
			}
		}
	}
	else
		draw_layer_scroll(screen, bitmap, cliprect, ba_select);
}

// Non-ROZ planes.  Sampled in screen space so that the SLH row-X table and the
// SLV column-Y table apply independently, including both at once - a split a
// tilemap_t scroll row/column setup cannot express.
//
// Note the wrap is driven by RLSC (R#8 bit 2) alone.  Comparing the map size
// against screen().width() instead does not work: that is htotal/2, not HDW/2,
// so a 256 dot MD=1 page against a 288 dot visible area never wrapped and the
// 12-bit cameras clipped the whole layer away.
void ygv608_device::draw_layer_scroll(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int ba)
{
	tilemap_t *source_tilemap = m_tilemap[ba];
	if (!source_tilemap->enabled())
		return;

	const bitmap_ind16 &src = source_tilemap->pixmap();
	const bitmap_ind8 &flags = source_tilemap->flagsmap();
	const int tile_size = 8 << m_pattern_size;
	const int map_w = m_page_x * tile_size;
	const int map_h = m_page_y * tile_size;
	const int pix_w = src.width();
	const int pix_h = src.height();
	const int x_dom = k_camera_domain[m_h_display_size & 3];
	const int y_dom = k_camera_domain[m_v_display_size & 3];
	const bool wrap = (!m_scroll_wrap_disable);   // R#8 RLSC

	const u8 *const table = m_scroll_data_table[ba];
	auto scroll12 = [table](int idx) -> int
	{
		return int(table[idx]) + ((int(table[idx + 1]) & 0x0f) << 8);
	};

	// The two tables are not indexed off the same coordinate:
	//
	//   row X ($80-$BF) is a raster effect - the chip latches it at the start of
	//         the display line, so the DISPLAY line selects the entry;
	//   col Y ($00-$7F) scrolls a column of the pattern plane, so the PLANE x
	//         that the row offset has already produced selects the entry.
	//
	// Index the column table off the display x instead and every horizontally
	// offset band drags the columns of whatever sits under it.  gynotai's result
	// banner is the clean case: $3ED30 writes rowX[0] = $120, so the banner band
	// reads the plane from x = 288, plus nine column entries in three groups of
	// three for the three ball lanes at plane x 0-287.  Taken off the plane x the
	// banner lands on entries 9..17, which are zero, and stays put.
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const int scrollx = scroll12(0x80 + get_row_division(y));
		u16 *dest = &bitmap.pix(y, cliprect.min_x);
		int last_col_index = -1;
		int sy = -1;

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const int plane_x = wrap_camera(x + scrollx, x_dom);
			const int col_index = get_col_division(plane_x);

			// y is fixed in this loop, so sy only moves when the strip does.
			if (col_index != last_col_index)
			{
				last_col_index = col_index;
				sy = camera_to_pixmap(wrap_camera(y + scroll12(col_index), y_dom), map_h, pix_h, wrap);
			}

			const int sx = camera_to_pixmap(plane_x, map_w, pix_w, wrap);

			if (sx >= 0 && sy >= 0 && (flags.pix(sy, sx) & TILEMAP_PIXEL_LAYER0))
				*dest = src.pix(sy, sx);

			dest++;
		}
	}
}

void ygv608_device::draw_mosaic(bitmap_ind16 &bitmap, const rectangle &cliprect, int n)
{
	if (n <= 0)
		return;

	// mask to drop the lowest n-bits
	const int mask = ~((1 << n) - 1);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bitmap.pix(y, x) = bitmap.pix(y & mask, x & mask);
		}
	}
}

u32 ygv608_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle finalclip;
	const rectangle &visarea = screen.visible_area();

	// clip to the current bitmap
	finalclip.set(0, screen.width() - 1, 0, screen.height() - 1);
	finalclip &= cliprect;
	// TODO: black/transparent pen if CBDR is 1 and border color is 0
	bitmap.fill(m_border_color, finalclip);

	// punt if not initialized
	if (m_page_x == 0 || m_page_y == 0)
	{
		return 0;
	}

	if (m_screen_resize)
	{
		m_work_bitmap.resize(screen.width(), screen.height());

		// reset resize flag
		m_screen_resize = false;
	}

	if (m_tilemap_resize)
	{
		int index;

		/* based on the page sizes, pick an index */
		if (m_page_x == 64)
			index = 1;
		else if (m_page_y == 64)
			index = 2;
		else
			index = 0;

		if (m_pattern_size == PTS_8X8)
			m_tilemap[0] = m_tilemap_cache_8[0][index];
		else
			m_tilemap[0] = m_tilemap_cache_16[0][index];
		m_tilemap[0]->mark_all_dirty();

		m_tilemap[0]->set_transparent_pen(m_border_color);

		if (m_pattern_size == PTS_8X8)
			m_tilemap[1] = m_tilemap_cache_8[1][index];
		else
			m_tilemap[1] = m_tilemap_cache_16[1][index];
		m_tilemap[1]->mark_all_dirty();

		// now clear the screen in case we change to 1-plane mode
		m_work_bitmap.fill(0, finalclip);

		// reset resize flag
		m_tilemap_resize = false;
	}

	m_tilemap[0]->enable(m_dspe);
	if (m_md & MD_1PLANE)
		m_tilemap[1]->enable(0);
	else
		m_tilemap[1]->enable(m_dspe);

	// Rebuild tiles once per frame only: with raster interrupts on, this runs
	// again for every update_partial() strip.
	if (cliprect.min_y <= visarea.min_y)
	{
		m_tilemap[0]->mark_all_dirty();
		m_tilemap[1]->mark_all_dirty();
	}


	/*
	 *    now we can render the screen
	 */

	// LBO - need to implement proper pen marking for sprites as well as set aside a non-transparent
	// pen to be used for background fills when plane B is disabled.
	if (m_md & MD_1PLANE)
	{
		// If the background tilemap is disabled, we need to clear the bitmap to black
		m_work_bitmap.fill(0, finalclip);
//      m_work_bitmap.fill(1, *visarea);
	}
	else
	{
		// Plane B needs the same pre-clear as plane A: draw_layer_scroll() only
		// writes the pixels the tilemap marks opaque, so whatever it skips keeps
		// the previous frame's content.  With CTPB=0 that work bitmap is copied to
		// the screen *opaquely* (gynotai's HUD runs CTPB=0), so a rectangle of an
		// earlier scene survives on screen.  draw_roz() used to touch every pixel,
		// which is why this was not needed before the pixmap blit.
		m_work_bitmap.fill(0, finalclip);

		draw_layer_roz(screen, m_work_bitmap, finalclip, 1);
		if (m_mosaic_plane[1] > 0)
			draw_mosaic(m_work_bitmap, finalclip, m_mosaic_plane[1]);

		if (m_plane_trans_enable[1])
			copybitmap_trans(bitmap, m_work_bitmap, 0, 0, 0, 0, finalclip, 0);
		else
			copybitmap(bitmap, m_work_bitmap, 0, 0, 0, 0, finalclip);
	}

	// for some reason we can't use an opaque m_tilemap[0]
	// so use a transparent but clear the work bitmap first
	// - look at why this is the case?!?
	m_work_bitmap.fill(0, finalclip);

	if (m_priority_mode == PRM_ASBDEX ||
		m_priority_mode == PRM_ASEBDX)
		draw_sprites(bitmap, finalclip);

	draw_layer_roz(screen, m_work_bitmap, finalclip, 0);
	if (m_mosaic_plane[0] > 0)
		draw_mosaic(m_work_bitmap, finalclip, m_mosaic_plane[0]);

	if (m_plane_trans_enable[0])
		copybitmap_trans(bitmap, m_work_bitmap, 0, 0, 0, 0, finalclip, 0);
	else
		copybitmap(bitmap, m_work_bitmap, 0, 0, 0, 0, finalclip);

	if (m_priority_mode == PRM_SABDEX ||
		m_priority_mode == PRM_SEABDX)
		draw_sprites(bitmap,finalclip);


#ifdef _SHOW_VIDEO_DEBUG
	/* show screen control information */
	static const char *const mode[] = {
		"2PLANE_8BIT",
		"2PLANE_16BIT",
		"1PLANE_16COLORS",
		"1PLANE_256COLORS"
	};

	static const char *const psize[] = { "8x8", "16x16", "32x32", "64x64" };

	char buffer[64];
	ui_draw_text(mode[m_md], 0, 0);
	snprintf(buffer, std::size(buffer), "%02ux%02u", m_page_x, m_page_y);
	ui_draw_text(buffer, 0, 16);
	ui_draw_text(psize[m_pattern_size], 0, 32);
	sprintf(buffer, "A: SX:%d SY:%d",
			(int)m_scroll_data_table[0][0x80] +
			(((int)m_scroll_data_table[0][0x81] & 0x0f) << 8),
			(int)m_scroll_data_table[0][0x00] +
			(((int)m_scroll_data_table[0][0x01] & 0x0f) << 8));
	ui_draw_text(buffer, 0, 48);
	snprintf(buffer, std::size(buffer), "B: SX:%d SY:%d",
			(int)m_scroll_data_table[1][0x80] +
			(((int)m_scroll_data_table[1][0x81] & 0x0f) << 8),
			(int)m_scroll_data_table[1][0x00] +
			(((int)m_scroll_data_table[1][0x01] & 0x0f) << 8));
	ui_draw_text(buffer, 0, 64);
#endif
	return 0;
}

/***************************************
 *
 * Port Interface routines
 *
 ****************************************/

 // P#0R - pattern name table data port
u8 ygv608_device::pattern_name_table_r()
{
	int pn = 0;

	switch (m_p0_state)
	{
		case 0:
			/* Are we reading from plane B? */
			if (!machine().side_effects_disabled())
			{
				if (!(m_md & MD_1PLANE) && (m_plane_select_access))
					m_pattern_name_base_r = ((m_page_y << m_pny_shift) << m_bits16);

			}
			/* read character from ram */
			pn = m_pattern_name_base_r + (((m_ytile_ptr << m_pny_shift) + m_xtile_ptr) << m_bits16);
			break;

		case 1:
			/* read character from ram */
			pn = m_pattern_name_base_r + (((m_ytile_ptr << m_pny_shift) + m_xtile_ptr) << m_bits16) + 1;
			break;
	}

	if (pn > 4095)
	{
		if (!machine().side_effects_disabled())
		{
			logerror("attempt (%d) to read pattern name %d\n"
					"mode = %d, pgs = %d (%dx%d)\n"
					"m_pattern_name_base_r = %d\n"
					"pnx = %d, pny = %d, pny_shift = %d, bits16 = %d\n",
					m_p0_state,
					pn, m_md, m_page_size,
					m_page_x, m_page_y,
					m_pattern_name_base_r,
					m_xtile_ptr, m_ytile_ptr, m_pny_shift,
					m_bits16);
		}
		pn = 0;
	}

	if (!machine().side_effects_disabled())
	{
		m_p0_state++;
		if (m_md == MD_2PLANE_8BIT)
			m_p0_state++;

		if (m_p0_state == 2)
		{
			pattern_name_autoinc_check();
			m_p0_state = 0;
			m_pattern_name_base_r = 0;
		}
	}

	return m_pattern_name_table[pn];
}

// P#1R - sprite data port
u8 ygv608_device::sprite_data_r()
{
	const u8 res = m_sprite_attribute_table.b[m_sprite_address];

	if (!machine().side_effects_disabled())
	{
		if (m_saar)
			m_sprite_address++;
	}

	return res;
}

// P#2R - scroll data port
u8 ygv608_device::scroll_data_r()
{
	const u8 res = m_scroll_data_table[m_ba_plane_scroll_select][m_scroll_address];

	if (!machine().side_effects_disabled())
	{
		if (m_scar)
		{
			m_scroll_address++;
			/* handle wrap to next plane */
			if (m_scroll_address == 0)
				m_ba_plane_scroll_select ^= 1;
		}
	}

	return res;
}

// P#3 - color palette data port
u8 ygv608_device::palette_data_r()
{
	const u8 res = m_colour_palette[m_palette_address][m_color_state_r];

	if (!machine().side_effects_disabled())
	{
		if (++m_color_state_r == 3)
		{
			m_color_state_r = 0;

			if (m_cpar)
				m_palette_address++;
		}
	}

	return res;
}

// P#4R - register data port
u8 ygv608_device::register_data_r()
{
	const u8 regnum = m_register_address & 0x3f;
	const u8 res = m_iospace->read_byte(regnum);

	if (!machine().side_effects_disabled())
	{
		if (m_register_autoinc_r)
		{
			m_register_address ++;
			m_register_address &= 0x3f;
#if 0
			// we'll catch this in the logerror anyway
			if (regnum == 50)
			{
				regnum = 0;
				logerror("warning: rn=50 after read increment\n");
			}
#endif
		}
	}

	return res;
}

// P#6R - status port
/***
 * ---x ---- FP Specified display position flag (R#15 & 16), reset by writing '1'
 * ---- x--- FV Vertical border interval start, reset by writing '1'
 * ---- -x-- FC Sprite collision flag, reset by writing '1'
 * ---- --x- HB 1 when horizontal border or retrace is in progress (read only)
 * ---- ---x VB 1 when vertical border or retrace is in progress (read only)
 ***/
u8 ygv608_device::status_port_r()
{
	// TODO: we need to use h/vpos in case of border support instead due of how MAME framework works here.
	return (m_screen_status & 0x1c) | (screen().hblank() << 1) | screen().vblank();
}

// P#7R - system control port
u8 ygv608_device::system_control_r()
{
	return m_dma_status;
}

// P#0W - pattern name table data write
void ygv608_device::pattern_name_table_w(u8 data)
{
	int pn = 0;

	switch (m_p0_state)
	{
		case 0:
			/* Are we reading from plane B? */
			if (!(m_md & MD_1PLANE) && (m_plane_select_access))
				m_pattern_name_base_w = ((m_page_y << m_pny_shift) << m_bits16);

			/* read character from ram */
			pn = m_pattern_name_base_w + (((m_ytile_ptr << m_pny_shift) + m_xtile_ptr) << m_bits16);
			break;

		case 1:
			/* read character from ram */
			pn = m_pattern_name_base_w + (((m_ytile_ptr << m_pny_shift) + m_xtile_ptr) << m_bits16) + 1;
			break;
	}

	if (pn > 4095)
	{
		logerror("attempt (%d) to write pattern name %d\n"
				"mode = %d, pgs = %d (%dx%d)\n"
				"m_pattern_name_base_w = %d\n"
				"pnx = %d, pny = %d, pny_shift = %d, bits16 = %d\n",
				m_p0_state,
				pn, m_md, m_page_size,
				m_page_x, m_page_y,
				m_pattern_name_base_w,
				m_xtile_ptr, m_ytile_ptr, m_pny_shift,
				m_bits16);
		pn = 0;
	}

	m_pattern_name_table[pn] = data;

	m_p0_state++;
	if (m_md == MD_2PLANE_8BIT)
		m_p0_state++;

	if (m_p0_state == 2)
	{
		pattern_name_autoinc_check();
		m_p0_state = 0;
		m_pattern_name_base_w = 0;
	}
}

inline void ygv608_device::pattern_name_autoinc_check()
{
	u8 xtile = m_xtile_ptr;
	u8 ytile = m_ytile_ptr;

	if (m_ytile_autoinc)
	{
		// we are incrementing in Y direction
		if (ytile++ == (m_page_y - 1))
		{
			ytile = 0;
			if (xtile++ == (m_page_x - 1))
			{
				xtile = 0;
				m_plane_select_access ^= 1; // flip A/B plane
			}
		}
		m_ytile_ptr = ytile;
		m_xtile_ptr = xtile;
	}
	else if (m_xtile_autoinc)
	{
		// we are incrementing in X direction
		if (xtile++ == (m_page_x - 1))
		{
			xtile = 0;
			if (ytile++ == (m_page_y - 1))
			{
				ytile = 0;
				m_plane_select_access ^= 1; // flip A/B plane
			}
		}
		m_ytile_ptr = ytile;
		m_xtile_ptr = xtile;
	}
}

// P#1W - sprite data port
void ygv608_device::sprite_data_w(u8 data)
{
	m_sprite_attribute_table.b[m_sprite_address] = data;

	if (m_saaw)
		m_sprite_address++;
}

// P#2W - scroll data port
void ygv608_device::scroll_data_w(u8 data)
{
	m_scroll_data_table[m_ba_plane_scroll_select][m_scroll_address] = data;

	if (m_scaw)
	{
		m_scroll_address++;
		/* handle wrap to next plane */
		if (m_scroll_address == 0)
			m_ba_plane_scroll_select ^= 1;
	}
}

// P#3W - colour palette data port
void ygv608_device::palette_data_w(u8 data)
{
	m_colour_palette[m_palette_address][m_color_state_w] = data;
	if (++m_color_state_w == 3)
	{
		m_color_state_w = 0;
		//if (m_colour_palette[m_palette_address][0] & 0x80) // Transparency designation, none of the Namco games enables it?

		set_pen_color(m_palette_address,
				pal6bit(m_colour_palette[m_palette_address][0]),
				pal6bit(m_colour_palette[m_palette_address][1]),
				pal6bit(m_colour_palette[m_palette_address][2]));

		if (m_cpaw)
			m_palette_address++;
	}
}

// P#4W - register data port
void ygv608_device::register_data_w(u8 data)
{
	const u8 regnum = m_register_address & 0x3f;
	//logerror("R#%d = $%02X\n", regnum, data);

	m_iospace->write_byte(regnum, data);

	if (m_register_autoinc_w)
	{
		m_register_address++;
		m_register_address &= 0x3f;

#if 0
		// we'll catch this in the logerror anyway
		if (regnum == 50)
		{
			regnum = 0;
			logerror("warning: rn=50 after write increment\n");
		}
#endif
	}
}

// P#5W - register select port
void ygv608_device::register_select_w(u8 data)
{
	m_register_address = data & 0x3f;
	m_register_autoinc_r = BIT(data, 6);
	m_register_autoinc_w = BIT(data, 7);
}

// P#6W - status port
void ygv608_device::status_port_w(u8 data)
{
	/* writing a '1' resets that bit */
	m_screen_status &= ~data;

	// send an irq ack to the delegates accordingly
	if (BIT(data, 3))
		m_vblank_handler(CLEAR_LINE);
	if (BIT(data, 4))
		m_raster_handler(CLEAR_LINE);
}

// P#7W - system control port
void ygv608_device::system_control_w(u8 data)
{
	m_dma_status = data;
	if (m_dma_status & 0x3e)
		handle_rom_transfers(data & 0x3e);
	if (m_dma_status & 0x01)
		handle_reset();
}


// TODO: actual timing of this
void ygv608_device::handle_reset()
{
	/* Clear ports #0-7 */
	//memset(&m_ports.b[0], 0, 8);
	// most likely variables to be reset here from ports, there might be more
	m_pattern_name_base_w = 0;
	m_pattern_name_base_r = 0;
	m_register_address = 0;
	m_register_autoinc_r = false;
	m_register_autoinc_w = false;

	/* Clear registers #0-38, #47-49 */
	for (int i = 0; i < 39; i++)
		m_iospace->write_byte(i, 0x00);
	for (int i = 47; i < 50; i++)
		m_iospace->write_byte(i, 0x00);

	//memset(&m_regs.b[0], 0, 39);
	//memset(&m_regs.b[47], 0, 3);

	/* Clear internal ram */
	memset(m_pattern_name_table, 0, 4096);
	memset(m_sprite_attribute_table.b, 0, SPRITE_ATTR_TABLE_SIZE);
	memset(m_scroll_data_table, 0, 2*256);
	memset(m_colour_palette, 0, 256*3);
}

/*
    The YGV608 has a function to block-move data from the rom into
    internal tables. This function is not used in NCV1, but I used
    it for testing trojan ROM software.
    - So leave it in!
 */
void ygv608_device::handle_rom_transfers(u8 type)
{
	popmessage("ROM DMA used %02x",type);

#if 0
	// TODO: eventually update this code to latest
	static u8 *sdt = (u8 *)m_scroll_data_table;
	static u8 *sat = (u8 *)m_sprite_attribute_table.b;

	/* fudge copy from sprite data for now... */
	u8 *RAM = machine.memory_region[0];
	int i;

	int src = (((int)m_regs.s.tb13 << 8) + (int)m_regs.s.tb5) << 5;
	int bytes = (int)m_regs.s.tn4 << 4;

	logerror("Transferring data from rom...\n");

	/* pattern name table */
	if (m_ports.s.tn)
	{
	}

	/* scroll table */
	if (m_ports.s.tl)
	{
		int dest = (int)m_regs.s.sca;
		if (m_regs.s.p2_b_a)
			dest += 0x100;

		/* fudge a transfer for now... */
		for (i=0; i<bytes; i++)
		{
			sdt[(dest+i)%512] = RAM[src+(i^0x01)];
		}

		/* flag as finished */
		m_ports.s.tl = 0;
	}

	/* sprite attribute table */
	if (m_ports.s.ts)
	{
		int dest = (int)m_sprite_address;

		/* fudge a transfer for now... */
		for (i=0; i<bytes; i++)
		{
			sat[(dest+i)%256] = RAM[src+(i^0x01)];
		}

		/* flag as finished */
		m_ports.s.ts = 0;
	}
#endif
}

/***************************************
 *
 * Register Interface routines
 *
 ****************************************/

 // R#0R - Pattern Name Table Access pointer Y
u8 ygv608_device::pattern_name_table_y_r()
{
	return (m_ytile_autoinc << 7) | (m_plane_select_access << 6) | m_ytile_ptr;
}

 // R#0W - Pattern Name Table Access pointer Y
void ygv608_device::pattern_name_table_y_w(u8 data)
{
	m_ytile_ptr = data & 0x3f;
	//if (ytile >= m_page_y)
	//  logerror ("%s:setting pny(%d) >= page_y(%d)\n", machine().describe_context(),
	//      ytile, m_page_y);
	m_ytile_ptr &= m_page_y - 1;
	m_ytile_autoinc = BIT(data, 7);
	m_plane_select_access = BIT(data, 6);
	// TODO: done by Dig Dug Original
	if (m_ytile_autoinc && m_xtile_autoinc)
		logerror("%s: Warning both X/Y Tiles autoinc enabled!\n",this->tag());
}

 // R#1R - Pattern Name Table Access pointer X
u8 ygv608_device::pattern_name_table_x_r()
{
	return (m_xtile_autoinc << 7) | m_xtile_ptr;
}

 // R#1W - Pattern Name Table Access pointer X
void ygv608_device::pattern_name_table_x_w(u8 data)
{
	m_xtile_ptr = data & 0x3f;
	//if (xtile >= m_page_x)
	//  logerror ("%s:setting pnx(%d) >= page_x(%d)\n", machine().describe_context(),
	//      xtile, m_page_x);
	m_xtile_ptr &= m_page_x - 1;
	m_xtile_autoinc = BIT(data, 7);
	// TODO: done by Dig Dug Original
	if (m_ytile_autoinc && m_xtile_autoinc)
		logerror("%s: Warning both X/Y Tiles autoinc enabled!\n",this->tag());
}

// R#2R - Built in RAM access control
/***
 * x--- ---- CPAW Address autoincrements after color palette write
 * -x-- ---- CPAR Address autoincrements after color palette read
 * ---x ---- B/(A) P#2 plane access select (1=B Plane)
 * ---- x--- SCAW Address autoincrements after scroll data write
 * ---- -x-- SCAR Address autoincrements after scroll data read
 * ---- --x- SAAW Address autoincrements after sprite attribute table write
 * ---- ---x SAAR Address autoincrements after sprite attribute table read
 ***/
u8 ygv608_device::ram_access_ctrl_r()
{
	return (m_cpaw << 7) | (m_cpar << 6) |
			(m_ba_plane_scroll_select << 4) |
			(m_scaw << 3) | (m_scar << 2) | (m_saaw << 1) | (m_saar << 0);
}

// R#2W - Built in RAM access control
void ygv608_device::ram_access_ctrl_w(u8 data)
{
	m_saar = BIT(data, 0);
	m_saaw = BIT(data, 1);
	m_scar = BIT(data, 2);
	m_scaw = BIT(data, 3);
	m_ba_plane_scroll_select = BIT(data, 4);
	m_cpar = BIT(data, 6);
	m_cpaw = BIT(data, 7);
}


// R#3R - sprite attribute table access pointer
u8 ygv608_device::sprite_address_r()
{
	return m_sprite_address;
}

// R#3W - sprite attribute table access pointer
void ygv608_device::sprite_address_w(u8 data)
{
	m_sprite_address = data;
}


 // R#4R - scroll table access pointer
u8 ygv608_device::scroll_address_r()
{
	return m_scroll_address;
}

 // R#4W - scroll table access pointer
void ygv608_device::scroll_address_w(u8 data)
{
	m_scroll_address = data;
}

 // R#5R - color palette access pointer
u8 ygv608_device::palette_address_r()
{
	return m_palette_address;
}

 // R#5W - color palette access pointer
void ygv608_device::palette_address_w(u8 data)
{
	m_palette_address = data;
}

// R#6R - sprite generator base address
u8 ygv608_device::sprite_bank_r()
{
	return m_sprite_bank;
}

// R#6W - sprite generator base address
void ygv608_device::sprite_bank_w(u8 data)
{
	m_sprite_bank = data;
}

// R#7R - screen control 7
/***
 * x--- ---- DCKM dot clock frequency (0 = 1/2 1 = 1/4)
 * -x-- ---- FLIP reverse display in pattern name
 * ---- x--- ZRON enables ROZ features
 * ---- -xx- MDx  planes display mode
 * ---- -11-      1 plane/256 colors (16 bits)
 * ---- -10-      1 plane/16 colors (16 bits)
 * ---- -01-      2 planes/16 bits
 * ---- -00-      2 planes/8 bits
 * ---- ---x DSPE display permission of pattern planes (screen blanked if 0)
 ***/
u8 ygv608_device::screen_ctrl_7_r()
{
	return (m_dckm << 7) | (m_flip << 6) | 
			(m_zron << 3) | ((m_md & 3) << 1) | (m_dspe << 0);
}

// R#7W - screen control 7
void ygv608_device::screen_ctrl_7_w(u8 data)
{
	const u8 new_md = (data >> 1) & 3;
	if (new_md != m_md)
		m_tilemap_resize = true;

	m_dckm = BIT(data, 7);
	m_flip = BIT(data, 6);
	m_zron = BIT(data, 3);
	m_md = new_md;
	m_dspe = BIT(data, 0);

	m_na8_mask = m_flip ? 0x03 : 0x0f;

	// changing mode resets the pattern name table states (Mappy Arrange)
	m_p0_state = 0;
	pattern_mode_setup();
	// TODO: add dot clock into CRTC
	//screen_configure();
}

inline void ygv608_device::pattern_mode_setup()
{
	const u32 old_page_x = m_page_x;
	const u32 old_page_y = m_page_y;

	m_bits16 = (m_md == MD_2PLANE_8BIT) ? 0 : 1;

	if (m_md == MD_2PLANE_16BIT)
	{
		m_page_x = m_page_y = 32;
	}
	else
	{
		if (!m_page_size)
		{
			m_page_x = 64;
			m_page_y = 32;
		}
		else
		{
			m_page_x = 32;
			m_page_y = 64;
		}
	}
	m_pny_shift = (m_page_x == 32) ? 5 : 6;

	/* bits to shift pattern y coordinate to extract base */
	m_base_y_shift = (m_page_y == 32) ? 2 : 3;

	// The tilemap cache is picked by page size, so a MD (R#7) or PGS (R#8) change
	// that resizes the page has to re-select it - screen_ctrl_9_w only does this on
	// a PTS change. Without it the blit keeps sampling the previous page's pixmap
	// while get_tile_info_*() already indexes the name table with the new geometry.
	if (m_page_x != old_page_x || m_page_y != old_page_y)
		m_tilemap_resize = true;
}

// R#8R - screen control 8
/***
 * xx-- ---- HDS horizontal display domain size (0=4096, 3=512)
 * --xx ---- VDS vertical display domain size (0=4096, 3=512)
 * ---- x--- RLRT ROZ wraparound disable
 * ---- -x-- RLSC scroll wraparound disable
 * ---- ---x PGS page size (0=64x32, 1=32x64; Mode 2=32x32)
 ***/
u8 ygv608_device::screen_ctrl_8_r()
{
	return (m_h_display_size << 6) | (m_v_display_size << 4) | 
			(m_roz_wrap_disable << 3) | (m_scroll_wrap_disable << 2) | 
			(m_page_size << 0);
}

// R#8W - screen control 8
void ygv608_device::screen_ctrl_8_w(u8 data)
{
	if ((data & 1) != m_page_size)
		m_tilemap_resize = true;

	m_h_display_size = (data >> 6) & 3;
	m_v_display_size = (data >> 4) & 3;
	m_roz_wrap_disable = BIT(data, 3);
	m_scroll_wrap_disable = BIT(data, 2);
	m_page_size = BIT(data, 0);

	pattern_mode_setup();
}

// R#9R - screen control 9
/***
 * xx-- ---- PTS: pattern size in pattern planes (8x8, 16x16, 32x32, 64x64)
 * --xx x--- SLH: size of horizontal division in screen division scrolling
 * ---- -xxx SLV: size of vertical division in screen division scrolling
 * ---- -111 64 dots division
 * ---- -110 32 dots division
 * ---- -101 16 dots division
 * ---- -100 8 dots division
 * ---- -000 entire screen
 ***/
u8 ygv608_device::screen_ctrl_9_r()
{
	return (m_pattern_size << 6) | 
			(m_h_div_size << 3) | (m_v_div_size << 0);
}

void ygv608_device::screen_ctrl_9_w(u8 data)
{
	const u8 new_pts = (data >> 6) & 3;

	if (new_pts != m_pattern_size)
		m_tilemap_resize = true;

	m_pattern_size = new_pts;
	m_h_div_size = (data >> 3) & 7;
	m_v_div_size = (data >> 0) & 7;

	m_col_shift = scroll_table_shift(m_v_div_size);
	m_row_shift = scroll_table_shift(m_h_div_size);
}

// R#10R - screen control 10: mosaic & sprite
/***
 * xx-- ---- SPAx: Auxiliary bits of sprite attribute table (0=8x8 or no flip, 1=16x16 or flipy, 2=32x32 or flipx, 3=64x64 or flipx & y)
 * --x- ---- SPAS: Auxiliary function select (0=SPAx selects sprite size, 1=SPAx selects flipping)
 * ---x ---- SPRD: Sprite display disable
 * ---- xx-- MCBx: Mosaic enable on plane B
 * ---- --xx MCAx: Mosaic enable on plane A
 ***/
u8 ygv608_device::screen_ctrl_10_r()
{
	return (m_sprite_aux_reg << 6) | (m_sprite_aux_mode << 5) | (m_sprite_disable << 4)
			| (m_mosaic_plane[1] << 2) | (m_mosaic_plane[0] & 3);
}

// R#10W - screen control: mosaic & sprite
void ygv608_device::screen_ctrl_10_w(u8 data)
{
	m_sprite_aux_reg = (data & 0xc0) >> 6;
	m_sprite_aux_mode = BIT(data, 5);
	m_sprite_disable = BIT(data, 4);

	// check mosaic
	m_mosaic_plane[1] = (data & 0xc) >> 2;
	m_mosaic_plane[0] = data & 3;
//  if (m_mosaic_plane[0] || m_mosaic_plane[1])
//      popmessage("Mosaic effect %02x %02x",m_mosaic_plane[0],m_mosaic_plane[1]);
}

// R#11R - screen control 11
u8 ygv608_device::screen_ctrl_11_r()
{
	return (m_scm << 6) | (m_yse << 5) | (m_cbdr << 4) | 
			(m_priority_mode << 2) | (m_plane_trans_enable[1] << 1) | (m_plane_trans_enable[0] << 0);
}

// R#11W - screen control 11
void ygv608_device::screen_ctrl_11_w(u8 data)
{
/**/m_scm = (data >> 6) & 3;
/**/m_yse = BIT(data, 5);
/**/m_cbdr = BIT(data, 4);
	m_priority_mode = (data >> 2) & 3;
	m_plane_trans_enable[1] = BIT(data, 1);
	m_plane_trans_enable[0] = BIT(data, 0);
}

// R#12R - screen control 12: color fetch modes
u8 ygv608_device::screen_ctrl_12_r()
{
	return (m_sprite_color_fetch << 6) | (m_plane_color_fetch[1] << 3) | (m_plane_color_fetch[0] << 0);
}

// R#12W - screen control 12: color fetch modes
void ygv608_device::screen_ctrl_12_w(u8 data)
{
	m_sprite_color_fetch = (data >> 6) & 3;
	m_plane_color_fetch[1] = (data >> 3) & 7;
	m_plane_color_fetch[0] = (data >> 0) & 7;
}

// R#13W - border color
void ygv608_device::border_color_w(u8 data)
{
	m_border_color = data;
}

// R#14R interrupt mask control
u8 ygv608_device::irq_mask_r()
{
	return (m_raster_irq_mask << 1) | (m_vblank_irq_mask << 0);
}

// R#14W interrupt mask control
void ygv608_device::irq_mask_w(u8 data)
{
	m_vblank_irq_mask = BIT(data, 0);
	m_raster_irq_mask = BIT(data, 1);

	// check if we have an irq in the queue
	vblank_irq_check();
	raster_irq_check();
}

// R#15R / R#16R raster interrupt control
u8 ygv608_device::irq_ctrl_r(offs_t offset)
{
	u8 res;

	if (offset == 0) // R#15
		res = m_raster_irq_vpos & 0xff;
	else // R#16
	{
		res = (m_raster_irq_mode << 7);

		res |= (BIT(m_raster_irq_vpos, 8) << 6);

		res |= (m_raster_irq_hpos >> 5) & 0x1f;
	}

	return res;
}

// R#15W / R#16W raster interrupt control
void ygv608_device::irq_ctrl_w(offs_t offset, u8 data)
{
	if (offset == 0) // R#15
	{
		m_raster_irq_vpos &= ~0xff;
		m_raster_irq_vpos |= data & 0xff;
	}
	else // R#16
	{
		m_raster_irq_mode = BIT(data, 7);

		m_raster_irq_vpos &= ~0x100;
		m_raster_irq_vpos |= BIT(data, 6) << 8;

		m_raster_irq_hpos = (data & 0x1f) * 32;
	}

	// reset raster timer
	m_raster_timer->reset();
	m_raster_timer->adjust(raster_sync_offset(), 0);

	//printf("%d %d %d %d %d\n",m_raster_irq_hpos,m_raster_irq_vpos,m_raster_irq_mode,m_crtc.htotal,m_crtc.vtotal);
}

// helper for validating and convert to screen position
attotime ygv608_device::raster_sync_offset()
{
	// don't care if h/v pos is higher than CRTC params (NCV2 POST)
	if (m_raster_irq_hpos >  m_crtc.htotal || m_raster_irq_vpos > m_crtc.vtotal)
		return attotime::never;

	// Horizontal CRTC dots are stored at full HDW; the MAME screen is HDW/2.
	const int hpos = m_raster_irq_hpos / 2;

	if (m_raster_irq_mode)
	{
		// FPM: INT1 every line at IH. IV is ignored.
		int y = screen().vpos() + 1;
		if (y > int(m_crtc.vtotal))
			y = 0;
		return screen().time_until_pos(y, hpos);
	}

	// TODO: actual sync not taken into account, needs a better test than NCV2 limited case
	return screen().time_until_pos(m_raster_irq_vpos, hpos);
}

// R#17 / R#24 - base address
/*
 * offset & 4 selects plane B
 * -xxx ---- write to base address + 1
 * ---- -xxx write to base address
 */
void ygv608_device::base_address_w(offs_t offset, u8 data)
{
	const int plane = offset >> 2;
	const int addr = (offset << 1) & 0x07;
	m_base_addr[plane][addr] = data & 0x07;
	m_base_addr[plane][addr + 1] = (data >> 4) & 0x7;

	m_tilemap_resize = true;
}

// R#25W - R#27W - X coordinate of initial value
void ygv608_device::roz_ax_w(offs_t offset, u8 data)  { m_ax = roz_convert_raw24(m_raw_ax, offset, data); }

// R#28W - R#29W - increment of coordinate in X direction
void ygv608_device::roz_dx_w(offs_t offset, u8 data)  { m_dx = roz_convert_raw16(m_raw_dx, offset, data); }

// R#30W - R#31W - increment of coordinate in X direction in movement toward Y direction
void ygv608_device::roz_dxy_w(offs_t offset, u8 data) { m_dxy = roz_convert_raw16(m_raw_dxy, offset, data); }

// R#32W - R#34W - Y coordinate of initial value
void ygv608_device::roz_ay_w(offs_t offset, u8 data)  { m_ay = roz_convert_raw24(m_raw_ay, offset, data); }

// R#35W - R#36W - increment of coordinate in Y direction
void ygv608_device::roz_dy_w(offs_t offset, u8 data)  { m_dy = roz_convert_raw16(m_raw_dy, offset, data); }

// R#37W - R#38W - increment of coordinate in Y direction in movement toward X direction
void ygv608_device::roz_dyx_w(offs_t offset, u8 data) { m_dyx = roz_convert_raw16(m_raw_dyx, offset, data); }

// ROZ assign helpers
inline u32 ygv608_device::roz_convert_raw24(u32 &raw_reg, u8 offset, u8 data)
{
	const u32 roz_data_mask24 = 0x1fffff;
	const u32 mem_mask = (0xff << (offset * 8)) ^ ~0;

	// substitute the new byte value into the raw register
	raw_reg &= mem_mask;
	raw_reg |= data << (offset * 8);

	// convert raw to the given register
	u32 res = raw_reg & roz_data_mask24;
	res = util::sext(res << 7, 28);

	return res;
}

inline u32 ygv608_device::roz_convert_raw16(u16 &raw_reg, u8 offset, u8 data)
{
	const u16 roz_data_mask16 = 0x1fff;
	const u16 mem_mask = (0xff << (offset * 8)) ^ ~0;

	// substitute the new byte value into the raw register
	raw_reg &= mem_mask;
	raw_reg |= data << (offset * 8);

	// convert raw to the given register
	u32 res = raw_reg & roz_data_mask16;
	res = util::sext(res << 7, 20);

	return res;
}

// R#39W - R#46W display scan control write
void ygv608_device::crtc_w(offs_t offset, u8 data)
{
	//printf("[%d] <- %02x\n",offset+39,data);

	switch (offset + 39)
	{
		case 39:
		{
			m_crtc.display_hsync = ((data >> 5) & 7) * 16;
			m_crtc.border_width = (data & 0x1f) * 16;
			break;
		}

		case 40:
		{
			const int new_display_width = (data & 0x3f) * 16;

			m_crtc.htotal &= ~0x600;
			m_crtc.htotal |= ((data & 0xc0) << 3);

			if (new_display_width != m_crtc.display_width)
				m_screen_resize = true;

			m_crtc.display_width = new_display_width;
			break;
		}

		case 41:
		{
			m_crtc.display_hstart &= ~0x1fe;
			m_crtc.display_hstart |= (data & 0xff) << 1;
			break;
		}

		case 42:
		{
			m_crtc.htotal &= ~0x1fe;
			m_crtc.htotal |= ((data & 0xff) << 1);

			//printf("H %d %d %d %d %d\n",m_crtc.htotal,m_crtc.display_hstart,m_crtc.display_width,m_crtc.display_hsync,m_crtc.border_width);
			break;
		}

		case 43:
		{
			m_crtc.display_vsync = (data >> 5) & 7;
			m_crtc.border_height = (data & 0x1f) * 8;
			break;
		}

		case 44:
		{
			const int new_display_height = (data & 0x3f) * 8;

			// TODO: VSLS, bit 6
			if (new_display_height != m_crtc.display_height)
				m_screen_resize = true;

			m_crtc.display_height = new_display_height;
			break;
		}

		case 45:
		{
			m_crtc.vtotal &= ~0x100;
			m_crtc.vtotal |= BIT(data,7) << 8;

			// TODO: TRES, bit 6

			m_crtc.display_vstart = data & 0x3f;
			break;
		}

		case 46:
		{
			m_crtc.vtotal &= ~0xff;
			m_crtc.vtotal |= data & 0xff;

			// TODO: call it for all mods in the CRTC, add sanity checks
			screen_configure();

			//printf("V %d %d %d %d %d\n",m_crtc.vtotal,m_crtc.display_vstart,m_crtc.display_height,m_crtc.display_vsync,m_crtc.border_height);
			break;
		}
	}
}

// TODO: all horizontal values needs to be divided by 2, presumably some other register?
// TODO: h/vstart not taken into account (needs video mods)
void ygv608_device::screen_configure()
{
//  const int display_hend = (m_crtc.display_hstart + (m_crtc.display_width / 2)) - 1;
	const int display_hend = (m_crtc.display_width / 2) - 1;
//  const int display_vend = (m_crtc.display_vstart + m_crtc.display_height) - 1;
	const int display_vend = (m_crtc.display_height) - 1;

	//rectangle visarea(m_crtc.display_hstart, display_hend, m_crtc.display_vstart, display_vend);
	rectangle visarea(0, display_hend, 0, display_vend);

	// TODO: Dig Dug Original wants this to be 60.60 Hz (like original Namco HW), lets compensate somehow
	//      (clock is really 6144000 x 8 = 49152000, so it must have same parameters in practice)
	attotime period = attotime::from_hz(screen().clock()) * (m_crtc.vtotal + m_crtc.display_vsync) * ((m_crtc.htotal + 12 - m_crtc.display_hsync) / 2);

	screen().configure(m_crtc.htotal / 2, m_crtc.vtotal, visarea, period);

	// reset vblank timer
	m_vblank_timer->reset();
	//m_vblank_timer->adjust(screen().time_until_pos(m_crtc.display_vstart+m_crtc.display_height,0), 0, screen().frame_period());
	m_vblank_timer->adjust(screen().time_until_pos(m_crtc.display_height,0), 0, screen().frame_period());
}
