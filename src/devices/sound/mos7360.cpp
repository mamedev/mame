// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 7360/8360 Text Edit Device (TED) emulation

**********************************************************************/

#include "emu.h"
#include "mos7360.h"

#include "screen.h"

#include <algorithm>
#include <utility>


namespace {

enum : uint32_t
{
	EV_REFRESH_ON       = 1U << 0,
	EV_REFRESH_OFF      = 1U << 1,
	EV_INC_LINE         = 1U << 2,
	EV_LINE_REGS        = 1U << 3,
	EV_STOP_INC_DMA     = 1U << 4,
	EV_LATCH_DMA        = 1U << 5,
	EV_LATCH_CHARPOS    = 1U << 6,
	EV_INIT_DISPLAY     = 1U << 7,
	EV_INC_LINE_2       = 1U << 8,
	EV_INC_VSUB         = 1U << 9,
	EV_VSUB_REG         = 1U << 10,
	EV_COLOR            = 1U << 11,
	EV_RELOAD_CHARPOS   = 1U << 16,
	EV_SINGLE_ON        = 1U << 17,
	EV_DMA_1            = 1U << 18,
	EV_DMA_2            = 1U << 19,
	EV_DMA_3            = 1U << 20,
	EV_DMA_4            = 1U << 21,
	EV_DMA_5            = 1U << 22,
	EV_DMA_ABORT        = 1U << 23,
	EV_T2_START         = 1U << 24,
	EV_VSCROLL          = 1U << 25,
	EV_HSCROLL          = 1U << 26,
	EV_SELECT_RENDER    = 1U << 27,
	EV_FORCE_SINGLE     = 1U << 28,
	EV_CHARPOS_REGS     = 1U << 29,
	EV_RESET_VSUB       = 1U << 30,
	EV_FLASH            = 1U << 31,

	EV_DMA_MASK         = EV_DMA_1 | EV_DMA_2 | EV_DMA_3 | EV_DMA_4 | EV_DMA_5 | EV_DMA_ABORT
};

enum : uint8_t
{
	MODE_MCM = 0x01,
	MODE_BMM = 0x02,
	MODE_ECM = 0x04,
	MODE_RVS_OFF = 0x08
};

constexpr uint8_t REGISTER_INIT[0x20] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x08, 0xff, 0x04, 0xa0, 0x00, 0xff, 0xff, 0x00, 0x00,
	0x7c, 0x00, 0xc4, 0x01, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0xff, 0xe8, 0xfe, 0xe0, 0xc8, 0x80
};

// palettes measured from VICE's default internal palettes
constexpr rgb_t PALETTE_PAL[128] =
{
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x20, 0x20, 0x20), rgb_t(0x5e, 0x07, 0x00), rgb_t(0x00, 0x39, 0x40),
	rgb_t(0x56, 0x00, 0x6c), rgb_t(0x00, 0x4a, 0x00), rgb_t(0x15, 0x0a, 0xae), rgb_t(0x2b, 0x36, 0x00),
	rgb_t(0x54, 0x16, 0x00), rgb_t(0x42, 0x26, 0x00), rgb_t(0x12, 0x43, 0x00), rgb_t(0x5f, 0x00, 0x38),
	rgb_t(0x00, 0x45, 0x08), rgb_t(0x00, 0x1a, 0x9a), rgb_t(0x2e, 0x00, 0xac), rgb_t(0x00, 0x4a, 0x00),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x30, 0x30, 0x30), rgb_t(0x6e, 0x17, 0x10), rgb_t(0x00, 0x49, 0x50),
	rgb_t(0x66, 0x06, 0x7c), rgb_t(0x00, 0x5a, 0x00), rgb_t(0x25, 0x1a, 0xbe), rgb_t(0x3b, 0x46, 0x00),
	rgb_t(0x64, 0x26, 0x00), rgb_t(0x52, 0x36, 0x00), rgb_t(0x22, 0x53, 0x00), rgb_t(0x6f, 0x0b, 0x48),
	rgb_t(0x00, 0x55, 0x18), rgb_t(0x0e, 0x2a, 0xaa), rgb_t(0x3e, 0x0d, 0xbc), rgb_t(0x0b, 0x5a, 0x00),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x40, 0x40, 0x40), rgb_t(0x7e, 0x27, 0x20), rgb_t(0x02, 0x59, 0x60),
	rgb_t(0x76, 0x16, 0x8c), rgb_t(0x0a, 0x6a, 0x00), rgb_t(0x35, 0x2a, 0xce), rgb_t(0x4b, 0x56, 0x00),
	rgb_t(0x74, 0x36, 0x00), rgb_t(0x62, 0x46, 0x00), rgb_t(0x32, 0x63, 0x00), rgb_t(0x7f, 0x1b, 0x58),
	rgb_t(0x01, 0x65, 0x28), rgb_t(0x1e, 0x3a, 0xba), rgb_t(0x4e, 0x1d, 0xcc), rgb_t(0x1b, 0x6a, 0x00),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x50, 0x50, 0x50), rgb_t(0x8e, 0x37, 0x30), rgb_t(0x12, 0x69, 0x70),
	rgb_t(0x86, 0x26, 0x9c), rgb_t(0x1a, 0x7a, 0x04), rgb_t(0x45, 0x3a, 0xde), rgb_t(0x5b, 0x66, 0x00),
	rgb_t(0x84, 0x46, 0x00), rgb_t(0x72, 0x56, 0x00), rgb_t(0x42, 0x73, 0x00), rgb_t(0x8f, 0x2b, 0x68),
	rgb_t(0x11, 0x75, 0x38), rgb_t(0x2e, 0x4a, 0xca), rgb_t(0x5e, 0x2d, 0xdc), rgb_t(0x2b, 0x7a, 0x00),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x78, 0x78, 0x78), rgb_t(0xb6, 0x5f, 0x58), rgb_t(0x3a, 0x91, 0x98),
	rgb_t(0xae, 0x4e, 0xc4), rgb_t(0x42, 0xa2, 0x2c), rgb_t(0x6d, 0x62, 0xff), rgb_t(0x83, 0x8e, 0x00),
	rgb_t(0xac, 0x6e, 0x25), rgb_t(0x9a, 0x7e, 0x00), rgb_t(0x6a, 0x9b, 0x00), rgb_t(0xb7, 0x53, 0x90),
	rgb_t(0x39, 0x9d, 0x60), rgb_t(0x56, 0x72, 0xf2), rgb_t(0x86, 0x55, 0xff), rgb_t(0x53, 0xa2, 0x03),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x90, 0x90, 0x90), rgb_t(0xce, 0x77, 0x70), rgb_t(0x52, 0xa9, 0xb0),
	rgb_t(0xc6, 0x66, 0xdc), rgb_t(0x5a, 0xba, 0x44), rgb_t(0x85, 0x7a, 0xff), rgb_t(0x9b, 0xa6, 0x02),
	rgb_t(0xc4, 0x86, 0x3d), rgb_t(0xb2, 0x96, 0x16), rgb_t(0x82, 0xb3, 0x04), rgb_t(0xcf, 0x6b, 0xa8),
	rgb_t(0x51, 0xb5, 0x78), rgb_t(0x6e, 0x8a, 0xff), rgb_t(0x9e, 0x6d, 0xff), rgb_t(0x6b, 0xba, 0x1b),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0xc0, 0xc0, 0xc0), rgb_t(0xfe, 0xa7, 0xa0), rgb_t(0x82, 0xd9, 0xe0),
	rgb_t(0xf6, 0x96, 0xff), rgb_t(0x8a, 0xea, 0x74), rgb_t(0xb5, 0xaa, 0xff), rgb_t(0xcb, 0xd6, 0x32),
	rgb_t(0xf4, 0xb6, 0x6d), rgb_t(0xe2, 0xc6, 0x46), rgb_t(0xb2, 0xe3, 0x34), rgb_t(0xff, 0x9b, 0xd8),
	rgb_t(0x81, 0xe5, 0xa8), rgb_t(0x9e, 0xba, 0xff), rgb_t(0xce, 0x9d, 0xff), rgb_t(0x9b, 0xea, 0x4b),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0xff, 0xff, 0xff), rgb_t(0xff, 0xe7, 0xe0), rgb_t(0xc2, 0xff, 0xff),
	rgb_t(0xff, 0xd6, 0xff), rgb_t(0xca, 0xff, 0xb4), rgb_t(0xf5, 0xea, 0xff), rgb_t(0xff, 0xff, 0x72),
	rgb_t(0xff, 0xf6, 0xad), rgb_t(0xff, 0xff, 0x86), rgb_t(0xf2, 0xff, 0x74), rgb_t(0xff, 0xdb, 0xff),
	rgb_t(0xc1, 0xff, 0xe8), rgb_t(0xde, 0xfa, 0xff), rgb_t(0xff, 0xdd, 0xff), rgb_t(0xdb, 0xff, 0x8b)
};

constexpr rgb_t PALETTE_NTSC[128] =
{
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x20, 0x20, 0x20), rgb_t(0x62, 0x0b, 0x0e), rgb_t(0x00, 0x35, 0x32),
	rgb_t(0x5d, 0x09, 0x23), rgb_t(0x00, 0x37, 0x1d), rgb_t(0x12, 0x15, 0x6f), rgb_t(0x2e, 0x2b, 0x00),
	rgb_t(0x61, 0x12, 0x00), rgb_t(0x4e, 0x1e, 0x00), rgb_t(0x00, 0x37, 0x00), rgb_t(0x61, 0x0b, 0x12),
	rgb_t(0x00, 0x37, 0x20), rgb_t(0x00, 0x26, 0x64), rgb_t(0x49, 0x09, 0x48), rgb_t(0x00, 0x38, 0x02),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x30, 0x30, 0x30), rgb_t(0x72, 0x1b, 0x1e), rgb_t(0x00, 0x45, 0x42),
	rgb_t(0x6d, 0x19, 0x33), rgb_t(0x00, 0x47, 0x2d), rgb_t(0x22, 0x25, 0x7f), rgb_t(0x3e, 0x3b, 0x00),
	rgb_t(0x71, 0x22, 0x02), rgb_t(0x5e, 0x2e, 0x00), rgb_t(0x09, 0x47, 0x05), rgb_t(0x71, 0x1b, 0x22),
	rgb_t(0x00, 0x47, 0x30), rgb_t(0x00, 0x36, 0x74), rgb_t(0x59, 0x19, 0x58), rgb_t(0x00, 0x48, 0x12),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x40, 0x40, 0x40), rgb_t(0x82, 0x2b, 0x2e), rgb_t(0x00, 0x55, 0x52),
	rgb_t(0x7d, 0x29, 0x43), rgb_t(0x03, 0x57, 0x3d), rgb_t(0x32, 0x35, 0x8f), rgb_t(0x4e, 0x4b, 0x00),
	rgb_t(0x81, 0x32, 0x12), rgb_t(0x6e, 0x3e, 0x00), rgb_t(0x19, 0x57, 0x15), rgb_t(0x81, 0x2b, 0x32),
	rgb_t(0x02, 0x57, 0x40), rgb_t(0x0a, 0x46, 0x84), rgb_t(0x69, 0x29, 0x68), rgb_t(0x10, 0x58, 0x22),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x50, 0x50, 0x50), rgb_t(0x92, 0x3b, 0x3e), rgb_t(0x0e, 0x65, 0x62),
	rgb_t(0x8d, 0x39, 0x53), rgb_t(0x13, 0x67, 0x4d), rgb_t(0x42, 0x45, 0x9f), rgb_t(0x5e, 0x5b, 0x01),
	rgb_t(0x91, 0x42, 0x22), rgb_t(0x7e, 0x4e, 0x06), rgb_t(0x29, 0x67, 0x25), rgb_t(0x91, 0x3b, 0x42),
	rgb_t(0x12, 0x67, 0x50), rgb_t(0x1a, 0x56, 0x94), rgb_t(0x79, 0x39, 0x78), rgb_t(0x20, 0x68, 0x32),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x78, 0x78, 0x78), rgb_t(0xba, 0x63, 0x66), rgb_t(0x36, 0x8d, 0x8a),
	rgb_t(0xb5, 0x61, 0x7b), rgb_t(0x3b, 0x8f, 0x75), rgb_t(0x6a, 0x6d, 0xc7), rgb_t(0x86, 0x83, 0x29),
	rgb_t(0xb9, 0x6a, 0x4a), rgb_t(0xa6, 0x76, 0x2e), rgb_t(0x51, 0x8f, 0x4d), rgb_t(0xb9, 0x63, 0x6a),
	rgb_t(0x3a, 0x8f, 0x78), rgb_t(0x42, 0x7e, 0xbc), rgb_t(0xa1, 0x61, 0xa0), rgb_t(0x48, 0x90, 0x5a),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x90, 0x90, 0x90), rgb_t(0xd2, 0x7b, 0x7e), rgb_t(0x4e, 0xa5, 0xa2),
	rgb_t(0xcd, 0x79, 0x93), rgb_t(0x53, 0xa7, 0x8d), rgb_t(0x82, 0x85, 0xdf), rgb_t(0x9e, 0x9b, 0x41),
	rgb_t(0xd1, 0x82, 0x62), rgb_t(0xbe, 0x8e, 0x46), rgb_t(0x69, 0xa7, 0x65), rgb_t(0xd1, 0x7b, 0x82),
	rgb_t(0x52, 0xa7, 0x90), rgb_t(0x5a, 0x96, 0xd4), rgb_t(0xb9, 0x79, 0xb8), rgb_t(0x60, 0xa8, 0x72),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0xc0, 0xc0, 0xc0), rgb_t(0xff, 0xab, 0xae), rgb_t(0x7e, 0xd5, 0xd2),
	rgb_t(0xfd, 0xa9, 0xc3), rgb_t(0x83, 0xd7, 0xbd), rgb_t(0xb2, 0xb5, 0xff), rgb_t(0xce, 0xcb, 0x71),
	rgb_t(0xff, 0xb2, 0x92), rgb_t(0xee, 0xbe, 0x76), rgb_t(0x99, 0xd7, 0x95), rgb_t(0xff, 0xab, 0xb2),
	rgb_t(0x82, 0xd7, 0xc0), rgb_t(0x8a, 0xc6, 0xff), rgb_t(0xe9, 0xa9, 0xe8), rgb_t(0x90, 0xd8, 0xa2),
	rgb_t(0x00, 0x00, 0x00), rgb_t(0xff, 0xff, 0xff), rgb_t(0xff, 0xeb, 0xee), rgb_t(0xbe, 0xff, 0xff),
	rgb_t(0xff, 0xe9, 0xff), rgb_t(0xc3, 0xff, 0xfd), rgb_t(0xf2, 0xf5, 0xff), rgb_t(0xff, 0xff, 0xb1),
	rgb_t(0xff, 0xf2, 0xd2), rgb_t(0xff, 0xfe, 0xb6), rgb_t(0xd9, 0xff, 0xd5), rgb_t(0xff, 0xeb, 0xf2),
	rgb_t(0xc2, 0xff, 0xff), rgb_t(0xca, 0xff, 0xff), rgb_t(0xff, 0xe9, 0xff), rgb_t(0xd0, 0xff, 0xe2)
};

// output level for volume (bits 0-3), voice 1 (bit 4) and voice 2 (bit 5)
constexpr int16_t TED_SOUND_LEVEL[64] =
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x024a, 0x064a, 0x0a4a, 0x0e4a, 0x124a, 0x164a, 0x1a4a,
	0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a,
	0x0000, 0x024a, 0x064a, 0x0a4a, 0x0e4a, 0x124a, 0x164a, 0x1a4a,
	0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a, 0x1e4a,
	0x0000, 0x0494, 0x0cd4, 0x1596, 0x1f30, 0x29a2, 0x34ec, 0x410e,
	0x4e08, 0x4e08, 0x4e08, 0x4e08, 0x4e08, 0x4e08, 0x4e08, 0x4e08
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(MOS7360, mos7360_device, "mos7360", "MOS 7360 TED")


void mos7360_device::mos7360_videoram_map(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00000, 0x0ffff).mirror(0x10000).ram();
}


device_memory_interface::space_config_vector mos7360_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_videoram_space_config)
	};
}


mos7360_device::mos7360_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MOS7360, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_sound_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 17, 0, address_map_constructor(FUNC(mos7360_device::mos7360_videoram_map), this)),
	m_write_irq(*this),
	m_write_ba(*this),
	m_read_k(*this, 0xff),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_bus_tap_installed(false),
	m_stream(nullptr),
	m_palette(PALETTE_PAL),
	m_catching_up(false)
{
}


//-------------------------------------------------
//  configure_screen - raw parameters in beam
//  coordinates (x from the end of horizontal
//  blanking, y from the start of vertical sync)
//-------------------------------------------------

void mos7360_device::configure_screen(screen_device &screen, bool ntsc) const
{
	if (ntsc)
		screen.set_raw(clock() / 2, HTOTAL, 0, HVISIBLE, NTSC_LINES, NTSC_VBEND, NTSC_VBSTART);
	else
		screen.set_raw(clock() * 2 / 5, HTOTAL, 0, HVISIBLE, PAL_LINES, PAL_VBEND, PAL_VBSTART);
}


void mos7360_device::device_start()
{
	m_bitmap.allocate(HTOTAL, PAL_LINES);

	m_stream = stream_alloc(0, 1, clock() / 80);
	m_ba_timer = timer_alloc(FUNC(mos7360_device::ba_tick), this);

	save_item(NAME(m_slot));
	save_item(NAME(m_cpu_prev_slot));
	save_item(NAME(m_ba));
	save_item(NAME(m_ba_armed));
	save_item(NAME(m_ba_slot));
	save_item(NAME(m_ba_rise_slot));
	save_item(NAME(m_cpu_base));
	save_item(NAME(m_pre_done));
	save_item(NAME(m_reg));
	save_item(NAME(m_bus));
	save_item(NAME(m_rom));
	save_item(NAME(m_ntsc));
	save_item(NAME(m_col));
	save_item(NAME(m_line));
	save_item(NAME(m_saved_line));
	save_item(NAME(m_line_d1));
	save_item(NAME(m_char_line));
	save_item(NAME(m_prev_char_line));
	save_item(NAME(m_char_pos));
	save_item(NAME(m_saved_char_pos));
	save_item(NAME(m_char_pos_reload));
	save_item(NAME(m_char_col));
	save_item(NAME(m_dma_pos));
	save_item(NAME(m_dma_pos_reload));
	save_item(NAME(m_dma_base));
	save_item(NAME(m_bitmap_base));
	save_item(NAME(m_charset_base));
	save_item(NAME(m_cursor_pos));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_refresh));
	save_item(NAME(m_frozen));
	save_item(NAME(m_frozen_half));
	save_item(NAME(m_flash));
	save_item(NAME(m_render_window));
	save_item(NAME(m_inc_char_line));
	save_item(NAME(m_bitmap_disable));
	save_item(NAME(m_display_window));
	save_item(NAME(m_display_active));
	save_item(NAME(m_hblank));
	save_item(NAME(m_vblank));
	save_item(NAME(m_prev_raster_irq));
	save_item(NAME(m_irq));
	save_item(NAME(m_dma_enabled));
	save_item(NAME(m_single));
	save_item(NAME(m_ext_single));
	save_item(NAME(m_dma_flags));
	save_item(NAME(m_dma_active));
	save_item(NAME(m_inc_dma_pos));
	save_item(NAME(m_inc_char_pos));
	save_item(NAME(m_cpu_running));
	save_item(NAME(m_cpu_halted));
	save_item(NAME(m_events_even));
	save_item(NAME(m_events_odd));
	save_item(NAME(m_t1_run));
	save_item(NAME(m_t2_run));
	save_item(NAME(m_t3_run));
	save_item(NAME(m_t1));
	save_item(NAME(m_t1_reload));
	save_item(NAME(m_t2));
	save_item(NAME(m_t3));
	save_item(NAME(m_attr_buf));
	save_item(NAME(m_attr_tmp));
	save_item(NAME(m_char_buf));
	save_item(NAME(m_shift_enable));
	save_item(NAME(m_mode));
	save_item(NAME(m_render_mode));
	save_item(NAME(m_char_mask));
	save_item(STRUCT_MEMBER(m_shift, attr));
	save_item(STRUCT_MEMBER(m_shift, ch));
	save_item(STRUCT_MEMBER(m_shift, bitmap));
	save_item(STRUCT_MEMBER(m_shift, flags));
	save_item(STRUCT_MEMBER(m_current, attr));
	save_item(STRUCT_MEMBER(m_current, ch));
	save_item(STRUCT_MEMBER(m_current, bitmap));
	save_item(STRUCT_MEMBER(m_current, flags));
	save_item(STRUCT_MEMBER(m_next, attr));
	save_item(STRUCT_MEMBER(m_next, ch));
	save_item(STRUCT_MEMBER(m_next, bitmap));
	save_item(STRUCT_MEMBER(m_next, flags));
	save_item(NAME(m_color_latch));
	save_item(NAME(m_hscroll));
	save_item(NAME(m_vscroll));
	save_item(NAME(m_beam_x));
	save_item(NAME(m_beam_y));
	save_item(NAME(m_vsync_pending));
	save_item(NAME(m_osc_accu));
	save_item(NAME(m_osc_reload));
	save_item(NAME(m_osc_sign));
	save_item(NAME(m_osc_out));
	save_item(NAME(m_noise_sr));
}


void mos7360_device::device_reset()
{
	if (!m_bus_tap_installed)
	{
		m_bus_tap = m_cpu->space(AS_PROGRAM).install_readwrite_tap(0x0000, 0xffff, "ted_bus",
				[this] (offs_t offset, uint8_t &data, uint8_t mem_mask)
				{
					if (m_cpu->executing())
						cpu_access(data, false);
				},
				[this] (offs_t offset, uint8_t &data, uint8_t mem_mask)
				{
					if (m_cpu->executing())
						cpu_access(data, true);
				});
		m_bus_tap_installed = true;
	}

	std::copy(std::begin(REGISTER_INIT), std::end(REGISTER_INIT), std::begin(m_reg));
	m_bus = 0xff;
	m_rom = true;

	m_col = 100;
	m_line = 224;
	m_saved_line = 224;
	m_line_d1 = 223;
	m_char_line = 0;
	m_prev_char_line = 0;
	m_char_pos = 0;
	m_saved_char_pos = 0;
	m_char_pos_reload = 0;
	m_char_col = 0;
	m_dma_pos = 0x3ff;
	m_dma_pos_reload = 0x3ff;
	m_dma_base = 0;
	m_bitmap_base = 0;
	m_charset_base = 0;
	m_cursor_pos = 0x3ff;
	m_irq_line = 0;
	m_refresh = 0;

	m_frozen = false;
	m_frozen_half = false;
	m_flash = 0;
	m_render_window = false;
	m_inc_char_line = false;
	m_bitmap_disable = 0x03;
	m_display_window = false;
	m_display_active = false;
	m_hblank = true;
	m_vblank = true;
	m_prev_raster_irq = false;
	m_dma_enabled = false;
	m_single = 0;
	m_ext_single = true;
	m_dma_flags = 0;
	m_dma_active = false;
	m_inc_dma_pos = false;
	m_inc_char_pos = false;
	m_cpu_running = true;
	m_cpu_halted = false;
	m_events_even = 0;
	m_events_odd = 0;
	m_events_now = 0;

	m_t1_run = m_t2_run = m_t3_run = true;
	m_t1 = m_t1_reload = m_t2 = m_t3 = 0;

	std::fill(std::begin(m_attr_buf), std::end(m_attr_buf), 0);
	std::fill(std::begin(m_attr_tmp), std::end(m_attr_tmp), 0);
	std::fill(std::begin(m_char_buf), std::end(m_char_buf), 0);
	m_shift_enable = false;
	m_mode = 0;
	m_render_mode = 0;
	m_char_mask = 0x7f;
	m_shift = m_current = m_next = character{ 0, 0, 0, 0 };
	std::fill(std::begin(m_color_latch), std::end(m_color_latch), 0x80);
	m_hscroll = 0;
	m_vscroll = 3;
	m_beam_x = 0;
	m_beam_y = 0;
	m_vsync_pending = false;

	std::fill(std::begin(m_osc_accu), std::end(m_osc_accu), 0);
	std::fill(std::begin(m_osc_reload), std::end(m_osc_reload), 1);
	std::fill(std::begin(m_osc_sign), std::end(m_osc_sign), 0);
	std::fill(std::begin(m_osc_out), std::end(m_osc_out), 0);
	m_noise_sr = 0xff;

	m_ba_armed = false;
	m_ba_timer->adjust(attotime::never);

	m_ntsc = false;
	update_timing();

	m_irq = false;
	m_write_irq(0);

	m_slot = 0;
	m_pre_done = false;
	m_cpu_waiting = false;
	m_cpu_access = false;
	m_cpu_data = 0xff;
	m_cpu_base = m_cpu->total_cycles();
	m_cpu_prev_slot = 0;

	m_ba = true;
	m_ba_slot = 0;
	m_ba_rise_slot = 0;
	m_write_ba(1);
}


void mos7360_device::device_pre_save()
{
	sync();
}


void mos7360_device::device_post_load()
{
	update_timing();
}


//**************************************************************************
//  SCHEDULING
//**************************************************************************

uint64_t mos7360_device::cpu_slot() const
{
	return m_cpu->total_cycles() - m_cpu_base;
}


//-------------------------------------------------
//  slot_delay - time from now until the CPU is
//  half a cycle past the start of 'slot'
//-------------------------------------------------

attotime mos7360_device::slot_delay(uint64_t slot) const
{
	uint64_t const now = cpu_slot();
	uint64_t const cycles = (slot > now) ? (slot - now) : 0;

	attotime const when = m_cpu->local_time() + m_cpu->cycles_to_attotime(cycles) + m_cpu->cycles_to_attotime(1) / 2;
	attotime const base = machine().time();
	if (when > base)
		return when - base;

	// the CPU can lag machine time; let it run up to the slot rather than spin here
	return (slot > now) ? (m_cpu->cycles_to_attotime(1) / 2) : attotime::zero;
}


void mos7360_device::sync()
{
	catch_up(cpu_slot());
}


//-------------------------------------------------
//  catch_up - finish every half-cycle before
//  'slot' and run the part of 'slot' that comes
//  before the CPU's bus access
//-------------------------------------------------

void mos7360_device::catch_up(uint64_t slot)
{
	if (m_catching_up)
		return;

	m_catching_up = true;

	while (m_slot < slot)
	{
		if (!m_pre_done)
		{
			half_cycle_pre();
			m_pre_done = true;
		}

		if (!m_cpu_access && !(m_col & 1) && !m_frozen_half && !m_cpu_running && !m_cpu_halted)
			break;

		half_cycle_post();
		m_slot++;
		m_pre_done = false;
	}

	if (!m_pre_done)
	{
		half_cycle_pre();
		m_pre_done = true;
	}

	update_ba();

	m_catching_up = false;
}


uint64_t mos7360_device::next_even_slot() const
{
	return m_slot + ((m_col & 1) ? 1 : 2);
}


//-------------------------------------------------
//  update_ba - drive BA (wired to the CPU's RDY)
//  and schedule its next edge; a release is
//  predicted from column 76 so that the halted
//  read cycles are only run once the CPU access
//  that owns them supplies its address
//-------------------------------------------------

void mos7360_device::update_ba()
{
	if (m_ba)
	{
		if (!m_cpu_running)
		{
			if (m_slot >= m_ba_rise_slot)
			{
				m_ba = false;
				m_write_ba(0);
			}
		}
		else if (!m_frozen && (m_events_even & EV_DMA_2))
		{
			arm_ba(next_even_slot());
		}
	}

	if (!m_ba)
	{
		if (m_cpu_running)
		{
			arm_ba(m_slot);
		}
		else if (m_frozen || (m_events_even & EV_DMA_ABORT))
		{
			arm_ba(next_even_slot());
		}
		else
		{
			unsigned const delta = (76 + 114 - m_col) % 114;
			arm_ba(m_slot + (delta ? delta : 114));
		}
	}
}


void mos7360_device::arm_ba(uint64_t slot)
{
	if (m_ba_armed && (slot == m_ba_slot))
		return;

	m_ba_armed = true;
	m_ba_slot = slot;
	m_ba_timer->adjust(slot_delay(slot));
}


TIMER_CALLBACK_MEMBER(mos7360_device::ba_tick)
{
	m_ba_armed = false;

	if (m_ba)
	{
		catch_up(cpu_slot());
	}
	else
	{
		m_ba = true;
		m_ba_rise_slot = m_ba_slot;
		m_write_ba(1);
	}
}


bool mos7360_device::cpu_slot_free(bool write) const
{
	if (m_col & 1)
		return !m_single;

	return m_cpu_running || (write && !m_cpu_halted);
}


//-------------------------------------------------
//  cpu_cycle - run up to the CPU's bus cycle and
//  hold the CPU while its half-cycle isn't free
//-------------------------------------------------

void mos7360_device::cpu_cycle(uint8_t data, bool write)
{
	uint64_t const slot = cpu_slot();
	if (m_catching_up || (slot == m_cpu_prev_slot))
		return;

	m_cpu_access = true;
	m_cpu_data = data;

	if (slot > m_cpu_prev_slot + 1)
	{
		catch_up(m_cpu_prev_slot + 1);
		m_cpu_waiting = !write;
	}

	catch_up(slot);

	int stall = 0;
	while (!cpu_slot_free(write))
	{
		m_cpu_waiting = !write;
		catch_up(m_slot + 1);
		stall++;
	}

	m_cpu_waiting = false;
	m_cpu_access = false;
	m_cpu_prev_slot = m_slot;

	if (stall)
		m_cpu->adjust_icount(-stall);
}


//-------------------------------------------------
//  cpu_access - the CPU's bus cycle has finished;
//  in single clock mode the following odd
//  half-cycle is withheld from the CPU, so it is
//  charged now and the next access lands after it
//-------------------------------------------------

void mos7360_device::cpu_access(uint8_t data, bool write)
{
	if (machine().side_effects_disabled())
		return;

	cpu_cycle(data, write);
	m_bus = data;

	if (!(m_col & 1) && m_single && (cpu_slot() == m_slot))
	{
		m_cpu->adjust_icount(-1);
		m_cpu_prev_slot = m_slot + 1;
	}
}


//**************************************************************************
//  HALF-CYCLE STATE MACHINE
//**************************************************************************

void mos7360_device::half_cycle_pre()
{
	m_frozen_half = m_frozen;
	m_beam_x = ((m_col + 114 - 106) % 114) * 4;

	if (!(m_col & 1))
	{
		m_char_pos = 0x3ff;

		if (m_inc_char_pos)
		{
			m_saved_char_pos = (m_saved_char_pos + 1) & 0x3ff;
			m_char_pos = m_saved_char_pos;
		}

		if (!m_frozen)
		{
			m_events_now = m_events_even;
			m_events_even = 0;
			even_events();
			if (m_events_now)
				process_events(std::exchange(m_events_now, 0));
		}
		else
		{
			m_events_even &= ~EV_DMA_MASK;
			m_dma_active = false;
			m_cpu_halted = false;
			m_cpu_running = true;
			if (m_events_even)
				process_events(std::exchange(m_events_even, 0));
		}
	}
	else
	{
		if (m_events_odd)
			process_events(std::exchange(m_events_odd, 0));

		if (!m_frozen)
			odd_events();
	}
}


void mos7360_device::half_cycle_post()
{
	if (!(m_col & 1))
	{
		if (!m_frozen_half)
		{
			if (!m_cpu_running)
			{
				if (m_cpu_halted)
					m_bus = fetch_dma(m_dma_base | m_dma_pos);
				else if (m_cpu_waiting)
					m_bus = m_cpu_data;

				if (m_char_col < 40)
				{
					if (m_dma_flags & 0x01)
						m_attr_tmp[m_char_col] = m_bus;
					if (m_dma_flags & 0x02)
						m_char_buf[m_char_col] = m_bus;
				}
			}

			if (m_inc_dma_pos)
				m_dma_pos = (m_dma_pos + 1) & 0x3ff;

			render(m_hscroll, false);

			if (m_t1_run)
			{
				if (!m_t1)
				{
					m_reg[0x09] |= 0x08;
					update_irq();
					m_t1 = m_t1_reload;
				}

				m_t1 = (m_t1 - 1) & 0xffff;
			}

			if (!m_t2 && m_t2_run)
			{
				m_reg[0x09] |= 0x10;
				update_irq();
			}

			if (!m_t3 && m_t3_run)
			{
				m_reg[0x09] |= 0x40;
				update_irq();
			}
		}
		else
		{
			if (m_inc_dma_pos)
				m_dma_pos = (m_dma_pos + 1) & 0x3ff;

			render(m_hscroll, true);
		}

		m_reg[0x1e] = m_col;
		m_col |= 1;
	}
	else
	{
		if (!m_frozen_half)
		{
			if (m_single)
			{
				if (!m_bitmap_disable)
				{
					offs_t addr = m_char_line;

					if (!(m_reg[0x06] & 0x80))
					{
						if (!(m_reg[0x06] & 0x20))
							addr |= m_charset_base | ((m_char_buf[m_char_col] & m_char_mask) << 3);
						else
							addr |= m_bitmap_base | (m_char_pos << 3);
					}
					else
					{
						if (!(m_reg[0x06] & 0x20))
							addr |= (m_attr_tmp[m_char_col] << 3) | 0xf800;
						else
							addr |= m_bitmap_base | ((m_char_pos & (m_attr_tmp[m_char_col] | 0xff00)) << 3);
					}

					m_bus = fetch_bitmap(addr & 0xffff);
				}
				else
				{
					fetch_idle();
				}
			}

			render(m_hscroll - 4, false);

			m_current.bitmap = 0;
			if (m_shift_enable)
				m_current = m_next;

			if (m_char_col != 40)
			{
				m_next.attr = m_attr_buf[m_char_col];
				m_attr_buf[m_char_col] = m_attr_tmp[m_char_col];
				m_next.ch = m_char_buf[m_char_col];
				if (++m_char_col >= 64)
					m_char_col = m_render_window ? 0 : 40;
			}
			else
			{
				m_next.attr = 0;
				m_next.ch = 0;
			}

			m_next.bitmap = m_bus;
			m_next.flags = ((m_char_pos == m_cursor_pos) ? 0xff : 0x0f) ^ m_mode;

			m_t2 = (m_t2 - (m_t2_run ? 1 : 0)) & 0xffff;
			m_t3 = (m_t3 - (m_t3_run ? 1 : 0)) & 0xffff;
		}
		else
		{
			if (m_single)
				fetch_idle();

			render(m_hscroll - 4, true);
		}

		m_reg[0x1e] = m_col;

		uint8_t const prev_col = m_col & 0x7e;

		if (!m_frozen)
		{
			if (m_col == 113)
				m_col = 0;
			else
				m_col++;
		}

		m_col &= 0x7e;

		if ((m_col == 106) && (prev_col != 106))
		{
			if (m_vsync_pending)
			{
				m_vsync_pending = false;
				m_beam_y = 0;
			}
			else
			{
				m_beam_y = (m_beam_y + 1) % PAL_LINES;
			}
		}
	}
}


//-------------------------------------------------
//  even_events - events at the start of an even
//  half-cycle, before the CPU
//-------------------------------------------------

void mos7360_device::even_events()
{
	bool const col40 = BIT(m_reg[0x07], 3);

	switch (m_col)
	{
	case 0:
		if (m_display_window && col40)
			m_display_active = true;
		break;

	case 2:
		if (m_display_window && !col40)
			m_display_active = true;
		break;

	case 72:
		if (m_inc_dma_pos)
			m_events_even |= EV_STOP_INC_DMA;
		if (m_render_window)
		{
			if (m_char_line == 6)
				m_events_even |= EV_LATCH_DMA;
			if ((m_prev_char_line == 6) && !(m_bitmap_disable & 0x02))
				m_events_even |= EV_LATCH_CHARPOS;
		}
		break;

	case 74:
		m_events_even |= EV_REFRESH_ON;
		m_ext_single = false;
		m_dma_flags &= 0x03;
		m_inc_char_pos = false;
		break;

	case 76:
		m_bitmap_disable |= 0x01;
		m_events_now &= ~EV_DMA_MASK;
		m_events_even &= ~EV_DMA_MASK;
		m_dma_active = false;
		m_cpu_halted = false;
		m_cpu_running = true;
		break;

	case 78:
		if (!col40)
			m_display_active = false;
		m_shift_enable = false;
		break;

	case 80:
		if (col40)
			m_display_active = false;
		break;

	case 84:
		m_events_even |= EV_REFRESH_OFF;
		break;

	case 88:
		m_hblank = true;
		break;

	case 96:
		m_events_even |= EV_INC_LINE;
		break;

	case 98:
		if (m_line_d1 == (m_ntsc ? 261 : 311))
		{
			m_line = 0x1ff;
			m_char_pos = 0;
			m_saved_char_pos = 0;
			m_char_pos_reload = 0;
			m_events_odd |= EV_CHARPOS_REGS;
			m_refresh = 0;
		}
		if (m_inc_char_line)
			m_events_even |= EV_INC_VSUB;
		break;

	case 100:
		m_ext_single = true;
		if (m_render_window)
			m_events_even |= EV_SINGLE_ON;
		break;

	case 102:
		m_dma_flags |= 0x80;
		if (m_char_col == 40)
			m_char_col = 0x3c;
		if (m_dma_enabled && ((m_line_d1 & 7) == m_vscroll))
		{
			m_events_even &= ~EV_DMA_MASK;
			m_dma_active = true;
			m_events_now = (m_events_now & ~EV_DMA_MASK) | EV_DMA_1;
			m_dma_flags |= 0x01;
			m_dma_base &= 0xf800;
		}
		else if (m_dma_flags & 0x02)
		{
			m_events_even &= ~EV_DMA_MASK;
			m_dma_active = true;
			m_events_now = (m_events_now & ~EV_DMA_MASK) | EV_DMA_1;
			m_dma_base |= 0x400;
		}
		break;

	case 106:
		m_hblank = false;
		break;

	case 108:
		if (m_render_window)
		{
			if (m_inc_dma_pos)
				m_dma_pos_reload &= m_dma_pos;
			m_dma_pos = m_dma_pos_reload;
			m_inc_dma_pos = true;
		}
		m_events_even |= EV_RELOAD_CHARPOS;
		break;

	case 110:
		m_bitmap_disable &= 0x02;
		if (!m_bitmap_disable)
			m_inc_char_pos = true;
		break;

	case 112:
		if (m_render_window || m_display_window || m_display_active)
			m_shift_enable = true;
		break;
	}
}


void mos7360_device::odd_events()
{
	if ((m_col == 87) && (m_line == 205))
		m_events_odd |= EV_FLASH;
}


void mos7360_device::process_events(uint32_t ev)
{
	if (ev & EV_REFRESH_ON)
		m_single = (m_single & 0x02) | 0x80;

	if (ev & EV_REFRESH_OFF)
		m_single &= 0x03;

	if (ev & EV_INC_LINE)
	{
		m_dma_flags &= 0x80;
		if (((m_line_d1 & 7) == m_vscroll) && m_dma_enabled)
			m_dma_flags |= 0x02;
		else if (m_dma_active)
			m_events_even |= EV_DMA_ABORT;

		m_saved_line = (m_line + 1) & 0x1ff;
		m_render_window = m_render_window || m_dma_enabled;

		bool const rows25 = BIT(m_reg[0x06], 3);

		if ((m_saved_line >= 226) && (m_saved_line <= 269))
		{
			vertical_events();
		}
		else if (m_saved_line <= 8)
		{
			if (BIT(m_reg[0x06], 4))
			{
				if (m_saved_line == 0)
					m_events_even |= EV_INIT_DISPLAY;
				if (m_saved_line == (rows25 ? 4 : 8))
					m_display_window = true;
			}
		}
		else if ((m_saved_line >= 200) && (m_saved_line <= 204))
		{
			if (m_saved_line == (rows25 ? 204 : 200))
				m_display_window = false;
			if (m_saved_line == 204)
			{
				m_render_window = false;
				m_inc_char_line = m_inc_char_line && m_dma_enabled;
			}
		}

		m_line = m_saved_line;
		m_prev_char_line = m_char_line;
		m_events_odd |= EV_LINE_REGS;
		m_events_even |= EV_INC_LINE_2;

		if (m_line == (m_ntsc ? 229 : 254))
			m_vsync_pending = true;
	}

	if (ev & EV_LINE_REGS)
	{
		check_dma_reset();
		m_reg[0x1d] = m_line & 0xff;
		m_reg[0x1c] = (m_line >> 8) & 0x01;
		check_raster_irq();
	}

	if (ev & EV_STOP_INC_DMA)
	{
		m_inc_dma_pos = false;
		if (m_line == 205)
			m_dma_pos_reload = 0x3ff;
	}

	if (ev & EV_LATCH_DMA)
		m_dma_pos_reload = m_dma_pos;

	if (ev & EV_LATCH_CHARPOS)
	{
		m_char_pos_reload = (m_char_pos + 1) & 0x3ff;
		m_events_odd |= EV_CHARPOS_REGS;
	}

	if (ev & EV_INIT_DISPLAY)
	{
		m_dma_enabled = true;
		if (!m_render_window)
		{
			m_render_window = true;
			m_events_odd |= EV_RESET_VSUB;
			if (m_ext_single)
				m_events_even |= EV_SINGLE_ON;
		}
	}

	if (ev & EV_INC_LINE_2)
	{
		m_line_d1 = m_saved_line;
		if (m_saved_line == 203)
			m_dma_enabled = false;

		if (m_saved_line == 204)
		{
			m_bitmap_disable |= 0x02;
			m_single &= 0x82;
		}
		else if (m_render_window)
		{
			if ((((m_line_d1 ^ m_reg[0x06]) & 7) == 0) && m_dma_enabled)
				m_inc_char_line = true;
			if (m_dma_flags & 0x02)
				m_bitmap_disable &= 0x01;
		}

		if (m_col != 100)
			ev |= EV_VSCROLL;
	}

	if (ev & EV_INC_VSUB)
	{
		m_char_line = (m_char_line + 1) & 7;
		m_events_odd |= EV_VSUB_REG;
	}

	if (ev & EV_VSUB_REG)
		m_reg[0x1f] = (m_reg[0x1f] & 0xf8) | m_char_line;

	for (int i = 0; i < 5; i++)
		if (ev & (EV_COLOR << i))
			m_color_latch[i] = m_reg[0x15 + i];

	if (ev & EV_RELOAD_CHARPOS)
	{
		m_saved_char_pos = m_char_pos_reload;
		m_char_pos = m_saved_char_pos;
	}

	if (ev & EV_SINGLE_ON)
		m_single |= 0x01;

	if (ev & EV_DMA_1)
	{
		if (m_render_window)
			m_single |= 0x01;
		m_events_even |= EV_DMA_2;
	}

	if (ev & EV_DMA_2)
	{
		m_cpu_running = false;
		m_single |= 0x01;
		m_events_even |= EV_DMA_3;
	}

	if (ev & EV_DMA_3)
		m_events_even |= EV_DMA_4;

	if (ev & EV_DMA_4)
		m_events_even |= EV_DMA_5;

	if (ev & EV_DMA_5)
		m_cpu_halted = true;

	if (ev & EV_DMA_ABORT)
	{
		m_events_even &= ~EV_DMA_MASK;
		m_dma_active = false;
		m_cpu_halted = false;
		m_cpu_running = true;
	}

	if (ev & EV_T2_START)
		m_t2_run = true;

	if (ev & EV_VSCROLL)
	{
		m_vscroll = m_reg[0x06] & 7;

		if (m_render_window)
		{
			if ((m_line_d1 & 7) == m_vscroll)
			{
				if (m_dma_enabled)
				{
					m_inc_char_line = true;
					if (m_col == 98)
						m_events_even |= EV_INC_VSUB;
					if (m_dma_flags & 0x80)
					{
						if (!m_dma_active)
						{
							m_dma_active = true;
							m_single |= 0x01;
							m_events_even |= EV_DMA_2;
						}
						m_dma_flags |= 0x01;
						m_dma_base &= 0xf800;
					}
				}
			}
			else
			{
				m_dma_flags &= 0x82;
				if (!(m_dma_flags & 0x03))
				{
					if (m_dma_active)
						m_events_even |= EV_DMA_ABORT;
				}
				else
				{
					m_dma_base |= 0x400;
				}
			}
		}
	}

	if (ev & EV_HSCROLL)
		m_hscroll = m_reg[0x07] & 7;

	if (ev & EV_SELECT_RENDER)
		m_render_mode = m_mode;

	if (ev & EV_FORCE_SINGLE)
		m_single = (m_single & 0x81) | (m_reg[0x13] & 0x02);

	if (ev & EV_CHARPOS_REGS)
	{
		m_reg[0x1a] = m_char_pos_reload >> 8;
		m_reg[0x1b] = m_char_pos_reload & 0xff;
	}

	if (ev & EV_RESET_VSUB)
	{
		m_char_line = 7;
		m_prev_char_line = 7;
		m_events_even |= EV_VSUB_REG;
	}

	if (ev & EV_FLASH)
	{
		m_reg[0x1f] = (m_reg[0x1f] & 0x7f) + 0x08;
		if (m_reg[0x1f] & 0x80)
			m_flash ^= 0xff;
	}
}


void mos7360_device::vertical_events()
{
	int const vblank_start = m_ntsc ? 226 : 251;
	int const vblank_end = m_ntsc ? 244 : 269;

	if (m_saved_line == vblank_start)
		m_vblank = true;
	else if (m_saved_line == vblank_end)
		m_vblank = false;
}


void mos7360_device::check_raster_irq()
{
	if (m_line == m_irq_line)
	{
		if (!m_prev_raster_irq)
		{
			m_prev_raster_irq = true;
			m_reg[0x09] |= 0x02;
			update_irq();
		}
	}
	else
	{
		m_prev_raster_irq = false;
	}
}


void mos7360_device::check_dma_reset()
{
	if ((m_line == 205) && !m_inc_dma_pos)
		m_dma_pos_reload = 0x3ff;
}


void mos7360_device::update_irq()
{
	bool const irq = m_reg[0x09] & m_reg[0x0a];

	if (irq != m_irq)
	{
		m_irq = irq;
		m_write_irq(irq ? 1 : 0);
	}
}


void mos7360_device::update_mode()
{
	switch (m_mode & 0x0e)
	{
	case 0x00:
		m_charset_base = (m_reg[0x13] & 0xfc) << 8;
		m_char_mask = 0x7f;
		break;

	case 0x04:
	case 0x0c:
		m_charset_base = (m_reg[0x13] & 0xf8) << 8;
		m_char_mask = 0x3f;
		break;

	case 0x08:
		m_charset_base = (m_reg[0x13] & 0xf8) << 8;
		m_char_mask = 0xff;
		break;

	default:
		m_charset_base = 0;
		m_char_mask = 0;
		break;
	}
}


void mos7360_device::set_ntsc(bool ntsc)
{
	if (ntsc == m_ntsc)
		return;

	m_ntsc = ntsc;
	update_timing();
}


void mos7360_device::update_timing()
{
	m_palette = m_ntsc ? PALETTE_NTSC : PALETTE_PAL;

	uint32_t const half_cycle = m_ntsc ? (clock() / 8) : (clock() / 10);
	m_cpu->set_unscaled_clock(half_cycle);
	m_stream->set_sample_rate(half_cycle / 8);

	if (m_ntsc)
		screen().configure(HTOTAL, NTSC_LINES, rectangle(0, HVISIBLE - 1, NTSC_VBEND, NTSC_VBSTART - 1), attotime::from_ticks(uint64_t(HTOTAL) * NTSC_LINES, half_cycle * 4));
	else
		screen().configure(HTOTAL, PAL_LINES, rectangle(0, HVISIBLE - 1, PAL_VBEND, PAL_VBSTART - 1), attotime::from_ticks(uint64_t(HTOTAL) * PAL_LINES, half_cycle * 4));
}


//**************************************************************************
//  MEMORY FETCHES
//**************************************************************************

uint8_t mos7360_device::fetch_dma(offs_t addr)
{
	if (m_rom && (addr >= 0x8000))
		return space(0).read_byte(0x10000 | addr);

	return space(0).read_byte(addr);
}


uint8_t mos7360_device::fetch_bitmap(offs_t addr)
{
	if (!BIT(m_reg[0x12], 2))
	{
		if ((addr & 0xffe0) == 0xff00)
			return reg_r(addr & 0x1f);

		return space(0).read_byte(addr);
	}

	if (addr < 0x8000 || ((addr >= 0xfd00) && (addr < 0xff00)))
		return m_bus;

	if ((addr & 0xffe0) == 0xff00)
		return reg_r(addr & 0x1f);

	return space(0).read_byte(0x10000 | addr);
}


void mos7360_device::fetch_idle()
{
	offs_t addr = 0xffff;

	if (m_single & 0x80)
	{
		addr = 0xff00 | m_refresh;
		m_refresh++;
	}

	if ((addr & 0xffe0) == 0xff00)
		m_bus = reg_r(addr & 0x1f);
	else
		m_bus = fetch_dma(addr);
}


//**************************************************************************
//  VIDEO OUTPUT
//**************************************************************************

void mos7360_device::render(int load, bool force_blank)
{
	uint32_t *const dest = &m_bitmap.pix(m_beam_y, m_beam_x);

	bool const blank = force_blank || m_hblank || m_vblank;
	uint8_t const mode = m_render_mode & (MODE_MCM | MODE_BMM | MODE_ECM);
	bool const active = !blank && m_display_active && (mode < 0x05);

	for (int p = 0; p < 4; p++)
	{
		if (p == load)
		{
			m_shift = m_current;

			if (active && (mode == 0x00) && (m_shift.attr & 0x80))
				m_shift.bitmap &= BIT(m_shift.flags, 7) ? 0xff : m_flash;
		}

		uint8_t const a = m_shift.attr;
		uint8_t const c = m_shift.ch;
		uint8_t const b = m_shift.bitmap;
		uint8_t color = 0;
		bool multicolor;

		if (active)
		{
			multicolor = (mode == (MODE_MCM | MODE_BMM)) || ((mode == MODE_MCM) && (a & 0x08));

			switch (mode)
			{
			case 0x00:
			{
				int bit = BIT(b, 7);
				if (BIT(c, 7) && BIT(m_shift.flags, 3))
					bit ^= 1;
				if (BIT(m_shift.flags, 7))
					bit ^= BIT(m_flash, 7);
				color = bit ? a : (p ? m_reg[0x15] : m_color_latch[0]);
				break;
			}

			case MODE_MCM:
				if (multicolor)
				{
					int const pair = b >> 6;
					color = (pair == 3) ? (a & 0x77) : (p ? m_reg[0x15 + pair] : m_color_latch[pair]);
				}
				else
				{
					color = BIT(b, 7) ? a : (p ? m_reg[0x15] : m_color_latch[0]);
				}
				break;

			case MODE_BMM:
				color = BIT(b, 7) ? (((a & 0x07) << 4) | (c >> 4)) : ((a & 0x70) | (c & 0x0f));
				break;

			case MODE_MCM | MODE_BMM:
				switch (b >> 6)
				{
				case 0: color = p ? m_reg[0x15] : m_color_latch[0]; break;
				case 1: color = ((a & 0x07) << 4) | (c >> 4); break;
				case 2: color = (a & 0x70) | (c & 0x0f); break;
				default: color = p ? m_reg[0x16] : m_color_latch[1]; break;
				}
				break;

			default:
			{
				int const bg = c >> 6;
				color = BIT(b, 7) ? a : (p ? m_reg[0x15 + bg] : m_color_latch[bg]);
				break;
			}
			}
		}
		else
		{
			multicolor = (m_mode & MODE_MCM) && !(((m_mode & (MODE_MCM | MODE_BMM)) == MODE_MCM) && !(a & 0x08));

			if (!blank && !m_display_active)
				color = p ? m_reg[0x19] : m_color_latch[4];
		}

		dest[p] = m_palette[color & 0x7f];

		if (!multicolor)
			m_shift.bitmap <<= 1;
		else if ((p - load) & 1)
			m_shift.bitmap <<= 2;
	}
}


uint32_t mos7360_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}


//**************************************************************************
//  REGISTER INTERFACE
//**************************************************************************

uint8_t mos7360_device::reg_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00: return m_t1 & 0xff;
	case 0x01: return m_t1 >> 8;
	case 0x02: return m_t2 & 0xff;
	case 0x03: return m_t2 >> 8;
	case 0x04: return m_t3 & 0xff;
	case 0x05: return m_t3 >> 8;

	case 0x09:
	{
		uint8_t data = (m_reg[0x09] & 0x7f) | 0x25;
		if (data & m_reg[0x0a] & 0x5e)
			data |= 0x80;
		return data;
	}

	case 0x0a: return m_reg[0x0a] | 0xa0;
	case 0x0c: return m_reg[0x0c] | 0xfc;
	case 0x10: return (m_reg[0x10] | 0x7c) & 0x7f;
	case 0x12: return m_reg[0x12] | 0xc0;
	case 0x13: return (m_reg[0x13] & 0xfe) | (m_rom ? 1 : 0);
	case 0x14: return m_reg[0x14] | 0x07;
	case 0x1a: return (m_reg[0x1a] & (m_char_pos_reload >> 8)) | 0xfc;
	case 0x1b: return m_reg[0x1b] & (m_char_pos_reload & 0xff);
	case 0x1c: return m_reg[0x1c] | 0xfe;
	case 0x1e: return m_reg[0x1e] << 1;
	case 0x1f: return (m_reg[0x1f] | 0x80) & (m_char_line | 0xf8);
	default: return m_reg[offset];
	}
}


uint8_t mos7360_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_cpu->executing())
			cpu_cycle(reg_r(offset & 0x1f), false);
		else
			sync();
	}

	return reg_r(offset & 0x1f);
}


void mos7360_device::write(offs_t offset, uint8_t data)
{
	if (!m_cpu->executing())
		sync();

	bool const even = !(m_col & 1);

	switch (offset)
	{
	case 0x00:
		m_t1_run = false;
		m_t1 = (m_t1 & 0xff00) | data;
		m_t1_reload = (m_t1_reload & 0xff00) | data;
		break;

	case 0x01:
		m_t1_run = true;
		m_t1 = (m_t1 & 0x00ff) | (data << 8);
		m_t1_reload = (m_t1_reload & 0x00ff) | (data << 8);
		break;

	case 0x02:
		m_t2_run = false;
		m_events_even &= ~EV_T2_START;
		m_t2 = (m_t2 & 0xff00) | data;
		break;

	case 0x03:
		if (!m_t2_run)
		{
			if (even)
				m_t2_run = true;
			else
				m_events_even |= EV_T2_START;
		}
		m_t2 = (m_t2 & 0x00ff) | (data << 8);
		break;

	case 0x04:
		m_t3_run = false;
		m_t3 = (m_t3 & 0xff00) | data;
		break;

	case 0x05:
		m_t3_run = true;
		m_t3 = (m_t3 & 0x00ff) | (data << 8);
		break;

	case 0x06:
	{
		uint8_t const changed = data ^ m_reg[0x06];
		m_reg[0x06] = data;
		m_mode = (m_mode & (MODE_MCM | MODE_RVS_OFF)) | ((data >> 4) & (MODE_BMM | MODE_ECM));

		if (changed & 0x07)
			m_events_even |= EV_VSCROLL;

		if (changed & 0x18)
		{
			switch (m_saved_line)
			{
			case 0:
				if (data & 0x10)
					m_events_even |= EV_INIT_DISPLAY | EV_VSCROLL;
				break;
			case 4:
				if ((data & 0x18) == 0x18)
					m_display_window = true;
				break;
			case 8:
				if ((data & 0x18) == 0x10)
					m_display_window = true;
				break;
			case 200:
				if (!(data & 0x08))
					m_display_window = false;
				break;
			case 204:
				if (data & 0x08)
					m_display_window = false;
				break;
			}
		}

		if (changed & 0x60)
		{
			update_mode();
			if (even)
				m_events_odd |= EV_SELECT_RENDER;
			else
				m_events_even |= EV_SELECT_RENDER;
		}
		break;
	}

	case 0x07:
	{
		uint8_t const changed = data ^ m_reg[0x07];
		m_reg[0x07] = data;
		m_frozen = BIT(data, 5);
		m_mode = (m_mode & (MODE_BMM | MODE_ECM)) | ((data >> 4) & (MODE_MCM | MODE_RVS_OFF));

		if (changed & 0x07)
		{
			if (even)
				m_events_even |= EV_HSCROLL;
			else
				m_events_odd |= EV_HSCROLL;
		}

		if ((changed & 0x20) && m_frozen && even)
		{
			m_events_even |= EV_REFRESH_ON;
			m_hblank = true;
		}

		if (changed & 0x40)
		{
			set_ntsc(BIT(data, 6));
			if ((m_saved_line >= 226) && (m_saved_line <= 269))
				vertical_events();
		}

		if (changed & 0x90)
		{
			update_mode();
			if (even)
				m_events_odd |= EV_SELECT_RENDER;
			else
				m_events_even |= EV_SELECT_RENDER;
		}
		break;
	}

	case 0x08:
		m_reg[0x08] = m_read_k(data);
		break;

	case 0x09:
		m_reg[0x09] = (m_reg[0x09] & ~data) | 0x04;
		update_irq();
		break;

	case 0x0a:
		m_reg[0x0a] = data;
		update_irq();
		m_irq_line = (m_irq_line & 0xff) | ((data & 0x01) << 8);
		check_raster_irq();
		break;

	case 0x0b:
		m_reg[0x0b] = data;
		m_irq_line = (m_irq_line & 0x100) | data;
		check_raster_irq();
		break;

	case 0x0c:
		m_reg[0x0c] = data;
		m_cursor_pos = (m_cursor_pos & 0xff) | ((data & 0x03) << 8);
		break;

	case 0x0d:
		m_reg[0x0d] = data;
		m_cursor_pos = (m_cursor_pos & 0x300) | data;
		break;

	case 0x0e:
	case 0x0f:
	case 0x10:
	case 0x11:
		soundport_w(offset, data);
		break;

	case 0x12:
		soundport_w(offset, data);
		m_reg[0x12] = data;
		m_bitmap_base = (data & 0x38) << 10;
		break;

	case 0x13:
		if ((data ^ m_reg[0x13]) & 0x02)
			m_events_even |= EV_FORCE_SINGLE;
		m_reg[0x13] = data;
		update_mode();
		break;

	case 0x14:
		m_reg[0x14] = data;
		m_dma_base = (m_dma_base & 0x400) | ((data & 0xf8) << 8);
		break;

	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	{
		int const n = offset - 0x15;
		m_reg[offset] = data | 0x80;
		m_color_latch[n] = 0xff;
		if (even)
			m_events_odd |= EV_COLOR << n;
		else
			m_events_even |= EV_COLOR << n;
		break;
	}

	case 0x1a:
		m_reg[0x1a] = data;
		m_char_pos_reload = ((data & 0x03) << 8) | m_reg[0x1b];
		break;

	case 0x1b:
		m_reg[0x1b] = data;
		m_char_pos_reload = ((m_reg[0x1a] & 0x03) << 8) | data;
		break;

	case 0x1c:
		m_reg[0x1c] = data;
		m_line = m_reg[0x1d] | ((data & 0x01) << 8);
		check_dma_reset();
		check_raster_irq();
		break;

	case 0x1d:
		m_reg[0x1d] = data;
		m_line = data | ((m_reg[0x1c] & 0x01) << 8);
		check_dma_reset();
		check_raster_irq();
		break;

	case 0x1e:
		m_col = (m_col & 0x01) | ((~data & 0xfc) >> 1);
		m_reg[0x1e] = m_col;
		break;

	case 0x1f:
		if (((m_reg[0x1f] ^ data) & 0x78) && ((m_reg[0x1f] & 0x78) == 0x78))
			m_flash ^= 0xff;
		m_reg[0x1f] = data;
		m_char_line = data & 0x07;
		break;

	case 0x3e:
		m_rom = true;
		break;

	case 0x3f:
		m_rom = 0;
		break;
	}
}


//-------------------------------------------------
//  bus_r - floating data bus read
//-------------------------------------------------

uint8_t mos7360_device::bus_r()
{
	if (!machine().side_effects_disabled())
	{
		if (m_cpu->executing())
			cpu_cycle(m_bus, false);
		else
			sync();
	}

	return m_bus;
}


//**************************************************************************
//  SOUND
//**************************************************************************

#define TONE1_VALUE     (m_reg[0x0e] | ((m_reg[0x12] & 3) << 8))
#define TONE2_VALUE     (m_reg[0x0f] | ((m_reg[0x10] & 3) << 8))
#define VOLUME          (m_reg[0x11] & 0x0f)
#define VOICE1_ON       (m_reg[0x11] & 0x10)
#define VOICE2_ON       (m_reg[0x11] & 0x20)
#define NOISE_ON        ((m_reg[0x11] & 0x60) == 0x40)
#define DIGITAL_ON      (m_reg[0x11] & 0x80)


void mos7360_device::sound_stream_update(sound_stream &stream)
{
	if (DIGITAL_ON)
	{
		stream.fill(0, TED_SOUND_LEVEL[m_reg[0x11] & 0x3f] / 32768.0);
		return;
	}

	for (int i = 0; i < stream.samples(); i++)
	{
		for (int voice = 0; voice < 2; voice++)
		{
			if ((m_osc_reload[voice] != 0x3ff) && (++m_osc_accu[voice] >= 0x400))
			{
				m_osc_accu[voice] = m_osc_reload[voice];
				m_osc_sign[voice] ^= 1;

				if (voice)
					m_noise_sr = (m_noise_sr << 1) | (BIT(m_noise_sr, 7) ^ BIT(m_noise_sr, 5) ^ BIT(m_noise_sr, 4) ^ BIT(m_noise_sr, 1));

				update_sound_outputs();
			}
		}

		stream.put_int(0, i, TED_SOUND_LEVEL[m_osc_out[0] | m_osc_out[1]], 32768);
	}
}


void mos7360_device::soundport_w(int offset, int data)
{
	m_stream->update();

	switch (offset)
	{
	case 0x0e:
	case 0x12:
		if (offset == 0x12)
			m_reg[offset] = (m_reg[offset] & ~3) | (data & 3);
		else
			m_reg[offset] = data;

		if (TONE1_VALUE == 0x3fe)
			m_osc_sign[0] = 1;

		m_osc_reload[0] = (TONE1_VALUE + 1) & 0x3ff;
		break;

	case 0x0f:
	case 0x10:
		m_reg[offset] = data;

		if (TONE2_VALUE == 0x3fe)
			m_osc_sign[1] = 1;

		m_osc_reload[1] = (TONE2_VALUE + 1) & 0x3ff;
		break;

	case 0x11:
		m_reg[offset] = data;

		if (DIGITAL_ON)
		{
			m_osc_sign[0] = m_osc_sign[1] = 1;
			m_osc_accu[0] = m_osc_reload[0];
			m_osc_accu[1] = m_osc_reload[1];
			m_noise_sr = 0xff;
		}
		break;
	}

	update_sound_outputs();
}


void mos7360_device::update_sound_outputs()
{
	m_osc_out[0] = VOLUME | ((m_osc_sign[0] && VOICE1_ON) ? 0x10 : 0);

	if (NOISE_ON)
		m_osc_out[1] = VOLUME | (BIT(m_noise_sr, 0) ? 0x20 : 0);
	else
		m_osc_out[1] = VOLUME | ((m_osc_sign[1] && VOICE2_ON) ? 0x20 : 0);
}
