// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Rewrite of Sega 315-5313 MD VDP (tied to Teradrive)

Notes:
- Teradrive actually has 128 KiB compared to stock 64, this means that d_titov2 won't possibly work
here;
- Should eventually derive from 315-5124 (the SMS VDP);

**************************************************************************************************/

#include "emu.h"
#include "ym7101.h"

#define LOG_REGS        (1U << 1)
#define LOG_REGSDMA     (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_REGS)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGREGS(...)       LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGREGSDMA(...)    LOGMASKED(LOG_REGSDMA, __VA_ARGS__)

DEFINE_DEVICE_TYPE(YM7101, ym7101_device, "ym7101", "Yamaha YM7101 VDP")

ym7101_device::ym7101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, YM7101, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, device_gfx_interface(mconfig, *this, nullptr, "palette")
	, device_mixer_interface(mconfig, *this)
	, m_space_vram_config("vram", ENDIANNESS_BIG, 16, 16 + 1, 0, address_map_constructor(FUNC(ym7101_device::vram_map), this))
	, m_space_cram_config("cram", ENDIANNESS_BIG, 16, 7, 0, address_map_constructor(FUNC(ym7101_device::cram_map), this))
	, m_space_vsram_config("vsram", ENDIANNESS_BIG, 16, 8, 0, address_map_constructor(FUNC(ym7101_device::vsram_map), this))
	, m_space_regs_config("regs", ENDIANNESS_BIG, 8, 6, 0, address_map_constructor(FUNC(ym7101_device::regs_map), this))
	, m_vram(*this, "vram")
	, m_cram(*this, "cram")
	, m_vsram(*this, "vsram")
	, m_vint_callback(*this)
	, m_hint_callback(*this)
	, m_sint_callback(*this)
	, m_mreq_cb(*this, 0)
	, m_dtack_cb(*this)
	, m_palette(*this, "palette")
	, m_psg(*this, "psg")
{
}

//const u16 dot_a = m_vram[(tile_a << 4) + BIT(xi ^ flipx_a, 2) + ((y_char_a ^ flipy_a) << 1)] >> (((flipx_a ^ xi) & 3) * 4) & 0xf;
// NOTE: can't use RGN_FRAC(1,1) in devices
static const gfx_layout layout_8x8x4 =
{
	8,8,
	0x1000,
	4,
	{ STEP4(0,1) },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ STEP8(0,4*8) },
	4*8*8
};

void ym7101_device::device_start()
{
	if (m_palette && !m_palette->started())
		throw device_missing_dependencies();

	screen().register_screen_bitmap(m_bitmap);

	// TODO: eventually merge these
	m_scan_timer = timer_alloc(FUNC(ym7101_device::scan_timer_callback), this);
	m_vint_on_timer = timer_alloc(FUNC(ym7101_device::vint_trigger_callback), this);
	m_hint_on_timer = timer_alloc(FUNC(ym7101_device::hint_trigger_callback), this);
	m_dma_timer = timer_alloc(FUNC(ym7101_device::dma_callback), this);
	// NOTE: DMA still runs on reset
	m_dma_timer->adjust(attotime::never);
	m_dma.source_address = 0;
	m_dma.length = 0;
	m_dma.fill = 0;
	m_dma.mode = MEMORY_TO_VRAM;
	m_dma.active = false;
	m_sprite_cache = std::make_unique<u16[]>(80 * 4);
	m_sprite_line = std::make_unique<u8[]>(320);

	set_gfx(0, std::make_unique<gfx_element>(
		m_palette,
		layout_8x8x4,
		(u8 *)(m_vram.target()),
		0, 4, 0
	));

	save_pointer(NAME(m_sprite_cache), 80 * 4);
}

void ym7101_device::device_reset()
{
	m_command.write_state = command_write_state_t::FIRST_WORD;
	m_command.latch = 0;
	m_command.address = 0;
	m_command.code = 0;

	m_de = false;
	m_ie0 = false;
	m_vint_pending = 0;
	m_plane_a_name_table = 0;
	m_plane_b_name_table = 0;
	m_background_color = 0;
	m_vcounter = 0;
	m_vint_on_timer->adjust(attotime::never);
	m_hint_on_timer->adjust(attotime::never);
	m_scan_timer->adjust(screen().time_until_pos(224), 224);
}

void ym7101_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette);
	m_palette->set_entries(0x40 + 0x40 * 2);

	SEGAPSG(config, m_psg, DERIVED_CLOCK(4, 15)).add_route(ALL_OUTPUTS, *this, 0.5, 0);
}

device_memory_interface::space_config_vector ym7101_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_VDP_VRAM, &m_space_vram_config),
		std::make_pair(AS_VDP_CRAM, &m_space_cram_config),
		std::make_pair(AS_VDP_VSRAM, &m_space_vsram_config),
		std::make_pair(AS_VDP_IO,   &m_space_regs_config)
	};
}

void ym7101_device::update_command_state()
{
	m_command.address = (m_command.latch & 0x3fff) | ((m_command.latch & 0x07'0000) >> 2);
	m_command.code = ((m_command.latch & 0xc000) >> 14) | ((m_command.latch & 0xf0'0000) >> 18);
}

u16 ym7101_device::control_port_r(offs_t offset, u16 mem_mask)
{
	// other bits returns open bus, tbd
	// FIFO empty << 9
	// FIFO full << 8
	return (m_vint_pending << 7)
//      | sprite_overflow << 6
//      | sprite_collision << 5
//      | odd << 4
		| (screen().vblank() << 3)
		| (screen().hblank() << 2)
		| (m_dma.active << 1);
		// is_pal << 0
}

void ym7101_device::control_port_w(offs_t offset, u16 data, u16 mem_mask)
{
	//printf("%04x %04x %d\n", data, mem_mask, m_command.write_state);
	if (ACCESSING_BITS_0_15)
	{
		if (m_command.write_state == command_write_state_t::SECOND_WORD)
		{
			m_command.latch = (data << 16) | (m_command.latch & 0xffff);
			update_command_state();
			m_dma.active = !!(m_m1 && BIT(m_command.code, 5));

			if (m_dma.active &&
				(
					(m_dma.mode == MEMORY_TO_VRAM && BIT(m_command.code, 0))
					// supdaisn writes a code of 0x30
					|| (m_dma.mode == VRAM_COPY && BIT(m_command.code, 4))
				)
			)
			{
				m_dma_timer->adjust(attotime::from_ticks(8, clock()));
			}

			m_command.write_state = command_write_state_t::FIRST_WORD;
			return;
		}

		m_command.latch = data | (m_command.latch & 0xffff0000);
		update_command_state();

		if ((data & 0xc000) == 0x8000)
		{
			space(AS_VDP_IO).write_byte((data >> 8) & 0x3f, data & 0xff);
		}
		else
		{
			m_command.write_state = command_write_state_t::SECOND_WORD;
		}
	}
}


u16 ym7101_device::data_port_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
		return 0xffff;

	m_command.write_state = command_write_state_t::FIRST_WORD;

	if (BIT(m_command.code, 0))
	{
		LOG("data_port_r: illegal read on write code %d & %04x\n", m_command.code, mem_mask);
		return 0xffff;
	}

	u16 res = 0;

	switch (m_command.code >> 1)
	{
		case 0:
			res = space(AS_VDP_VRAM).read_word(m_command.address, mem_mask);
			break;
		case 2:
			res = space(AS_VDP_VSRAM).read_word(m_command.address, mem_mask);
			break;
		case 4:
			res = space(AS_VDP_CRAM).read_word(m_command.address, mem_mask);
			break;

		case 6:
			LOG("data_port_r: undocumented vram 8-bit & %04x\n", mem_mask);
			break;
		default:
			LOG("data_port_r: illegal code %d & %04x\n", m_command.code >> 1, mem_mask);
			break;
	}

	m_command.address += m_auto_increment;
	m_command.address &= 0x1ffff;

	return res;
}

void ym7101_device::data_port_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_command.write_state = command_write_state_t::FIRST_WORD;

	if (!BIT(m_command.code, 0))
	{
		LOG("data_port_w: illegal write on read code %d data %04x & %04x\n", m_command.code, data, mem_mask);
		return;
	}

	if (m_dma.active && m_dma.mode == VRAM_FILL && BIT(m_command.code, 0))
	{
		m_dma.fill = data;
		m_dma_timer->adjust(attotime::from_ticks(8, clock()));
		return;
	}

	switch (m_command.code >> 1)
	{
		case 0:
			space(AS_VDP_VRAM).write_word(m_command.address, data, mem_mask);
			break;
		case 1:
			space(AS_VDP_CRAM).write_word(m_command.address, data, mem_mask);
			break;
		case 2:
			space(AS_VDP_VSRAM).write_word(m_command.address, data, mem_mask);
			break;
		default:
			LOG("data_port_w: illegal code %d data %04x & %04x\n", m_command.code >> 1, data, mem_mask);
			break;
	}

	m_command.address += m_auto_increment;
	m_command.address &= 0x1ffff;
}

// https://gendev.spritesmind.net/forum/viewtopic.php?t=768
u16 ym7101_device::hv_counter_r(offs_t offset, u16 mem_mask)
{
//	int const hpos = screen().hpos();
	int const vpos = screen().vpos();
	u8 vcount = vpos > 234 ? vpos - 0xea + 0xe4 : vpos;

	return (vcount << 8);
}

void ym7101_device::if16_map(address_map &map)
{
	map(0x00, 0x01).mirror(2).rw(FUNC(ym7101_device::data_port_r), FUNC(ym7101_device::data_port_w));
	// Control Port
	map(0x04, 0x05).mirror(2).rw(FUNC(ym7101_device::control_port_r), FUNC(ym7101_device::control_port_w));
	map(0x08, 0x09).mirror(6).r(FUNC(ym7101_device::hv_counter_r));
	map(0x11, 0x11).mirror(6).w(m_psg, FUNC(segapsg_device::write));
	// TODO: debug port at $18/$1c
}

void ym7101_device::if8_map(address_map &map)
{
	map(0x11, 0x11).mirror(6).w(m_psg, FUNC(segapsg_device::write));
}


void ym7101_device::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	gfx(0)->mark_dirty(offset >> 4);

	const u32 sprite_table = m_sprite_attribute_table >> 1;

	// TODO: check akumajo Stage 6-3
	// TODO: check segacd:snatcheru (H32, puts sprite_table at $fe00)
	// TODO: only Y & Link parameter is cached
	if (offset == std::clamp(offset, sprite_table, (sprite_table + 80 * 4) - 1))
	{
		m_sprite_cache[offset - sprite_table] = m_vram[offset];
	}
}

void ym7101_device::vram_map(address_map &map)
{
	// TODO: configurable way to make it stock 64 KiB
//  if (!has_configured_map(AS_VDP_VRAM))
	map(0x00000, 0x1ffff).ram().share("vram").w(FUNC(ym7101_device::vram_w));
}

void ym7101_device::cram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_cram[offset]);
	static constexpr u8 level[15] = {0,29,52,70,87,101,116,130,144,158,172,187,206,228,255};

	int r = (m_cram[offset] & 0x000e) >> 1;
	int g = (m_cram[offset] & 0x00e0) >> 5;
	int b = (m_cram[offset] & 0x0e00) >> 9;

	// normal
	m_palette->set_pen_color(offset, level[r << 1], level[g << 1], level[b << 1]);
	// TODO: shadow & highlight
}

void ym7101_device::cram_map(address_map &map)
{
	map(0x00, 0x7f).ram().share("cram").w(FUNC(ym7101_device::cram_w));
}

void ym7101_device::vsram_map(address_map &map)
{
	map(0x00, 0x4f).ram().share("vsram");
}

static const char *const size_names[] = { "256 pixels/32 cells", "512 pixels/64 cells", "<invalid>", "1024 pixels/128 cells" };

// https://plutiedev.com/vdp-registers
// https://segaretro.org/Sega_Mega_Drive/VDP_registers
// NOTE: in decimal units, classic Yamaha
void ym7101_device::regs_map(address_map &map)
{
	map(0, 0).lw8(NAME([this] (u8 data) {
		LOGREGS("#00: Mode Register 1 %02x\n", data);
		m_ie1 = !!BIT(data, 4);
		if (m_hint_pending && m_ie1)
		{
			m_hint_on_timer->adjust(attotime::from_ticks(16, clock()));
		}
		else
			m_hint_callback(0);

		LOGREGS("\tL: %d IE1: %d M4: %d M3: %d DE: %d\n"
			, BIT(data, 5)
			, m_ie1
			, BIT(data, 2)
			, BIT(data, 1)
			, BIT(data, 0)
		);
	}));
	map(1, 1).lw8(NAME([this] (u8 data) {
		LOGREGS("#01: Mode Register 2 %02x\n", data);
		m_vr = !!BIT(data, 7);
		m_de = !!BIT(data, 6);
		m_ie0 = !!BIT(data, 5);
		m_m1 = !!BIT(data, 4);
		//m_dma.active = !!(m_m1 && BIT(m_command.code, 5));

		if (m_ie0 && m_vint_pending)
		{
			m_vint_on_timer->adjust(attotime::from_ticks(16, clock()));
		}
		else
		{
			m_vint_callback(0);
		}
		LOGREGS("\tVR: %d DE: %d IE0: %d M1: %d M2: %d M5: %d\n"
			, m_vr
			, m_de
			, m_ie0
			, m_m1
			, BIT(data, 3)
			, BIT(data, 2)
		);
	}));
	map(2, 2).lw8(NAME([this] (u8 data) {
		m_plane_a_name_table = (data & 0x78) << 10;
		LOGREGS("#02: Plane A Name Table %02x (%05x)\n"
			, data
			, m_plane_a_name_table
		);
	}));
	map(3, 3).lw8(NAME([this] (u8 data) {
		m_window_name_table = (data & 0x7e) << 10;
		LOGREGS("#03: Window Name Table %02x (%05x)\n"
			, data
			, m_window_name_table
		);
	}));
	map(4, 4).lw8(NAME([this] (u8 data) {
		m_plane_b_name_table = (data & 0xf) << 13;
		LOGREGS("#04: Plane B Name Table %02x (%05x)\n"
			, data
			, m_plane_b_name_table
		);
	}));
	map(5, 5).lw8(NAME([this] (u8 data) {
		m_sprite_attribute_table = data << 9;
		LOGREGS("#05: Sprite Table %02x (%05x)\n"
			, data
			, data << 9
		);
	}));
	map(6, 6).lw8(NAME([this] (u8 data) {
		LOGREGS("#06: 128kB Sprite Table %02x (%d)\n", data , BIT(data, 5));
	}));
	map(7, 7).lw8(NAME([this] (u8 data) {
		LOGREGS("#07: Background Color %02x & 0x3f\n", data);
		m_background_color = data & 0x3f;
	}));
//  map(8, 8) Master System h scroll
//  map(9, 9) Master System v scroll
	map(10, 10).lw8(NAME([this] (u8 data) {
		m_hit = data + 1;
		LOGREGS("#10: Horizontal Interrupt Counter %02x (%u)\n", data, m_hit);
	}));
	// <-- mode 4 ignores everything beyond this point
	map(11, 11).lw8(NAME([this] (u8 data) {
		LOGREGS("#11: Mode Register 3 %02x\n", data);
		m_vs = BIT(data, 2);
		m_hs = data & 3;
		LOGREGS("\tIE2: %d VS: %d HS: %d\n"
			, BIT(data, 3)
			, m_vs
			, m_hs
		);
	}));
	map(12, 12).lw8(NAME([this] (u8 data) {
		LOGREGS("#12: Mode Register 4 %02x\n", data);
		// VS-HS-EP are undocumented
		// VS/HS: (external?) Sync
		// EP: External Pixel bus enable (32x?)
		LOGREGS("\tRSx: %d (%s) VS: %d HS: %d EP: %d S/H: %d LSx: %d\n"
			, BIT(data, 7)
			, BIT(data, 7) ? "H40" : "H32"
			, BIT(data, 6)
			, BIT(data, 5)
			, BIT(data, 4)
			, BIT(data, 3)
			, (data & 6) >> 1
		);
		// https://plutiedev.com/mirror/kabuto-hardware-notes#h40-mode-tricks
		if (BIT(data, 7) != BIT(data, 0))
			popmessage("ym7101.cpp: invalid RS setting %02x", data & 0x81);
	}));
	map(13, 13).lw8(NAME([this] (u8 data) {
		m_hscroll_address = (data & 0x7f) << 10;
		LOGREGS("#13: Horizontal Scroll address %02x (%05x)\n", data, (data & 0x7f) << 10);
	}));
	map(14, 14).lw8(NAME([this] (u8 data) {
		// TODO: extra address bits for 128 KiB mode?
		LOGREGS("#14: 128 KiB plane address %02x (%02x)\n", data, data & 0x11);
	}));
	map(15, 15).lw8(NAME([this] (u8 data) {
		m_auto_increment = data;
		//LOGREGS("#15: Auto Increment %02x\n", data);
	}));
	map(16, 16).lw8(NAME([this] (u8 data) {
		m_vsz = (data & 0x30) >> 4;
		m_hsz = data & 3;
		LOGREGS("#16: Plane Size %02x\n", data);
		LOGREGS("\tH %d (%s) x V %d (%s)\n"
			, m_hsz
			, size_names[m_hsz]
			, m_vsz
			, size_names[m_vsz]
		);
		if (m_hsz == 2 || m_vsz == 2 || (m_vsz == 3 && m_hsz != 0) || (m_hsz == 3 && m_vsz != 0))
			popmessage("ym7101.cpp: illegal plane size set %d %d", m_hsz, m_vsz);
	}));
	map(17, 17).lw8(NAME([this] (u8 data) {
		m_rigt = !!BIT(data, 7);
		m_whp = data & 0x1f;
		LOGREGS("#17: Window Plane Horizontal position %02x\n", data);
		LOGREGS("\tRIGT %d WHP %d\n"
			, m_rigt
			, m_whp
		);
	}));
	map(18, 18).lw8(NAME([this] (u8 data) {
		m_down = !!BIT(data, 7);
		m_wvp = data & 0x1f;
		LOGREGS("#18: Window Plane Vertical position %02x\n", data);
		LOGREGS("\tDOWN %d WVP %d\n"
			, m_down
			, m_wvp
		);
	}));
	map(19, 19).lw8(NAME([this] (u8 data) {
		LOGREGSDMA("#19: DMA length low %02x\n", data);
		m_dma.length = data | (m_dma.length & 0xff00);
	}));
	map(20, 20).lw8(NAME([this] (u8 data) {
		LOGREGSDMA("#20: DMA length high %02x\n", data);
		m_dma.length = (data << 8) | (m_dma.length & 0xff);
	}));
	map(21, 21).lw8(NAME([this] (u8 data) {
		LOGREGSDMA("#21: DMA source low %02x\n", data);
		m_dma.source_address = (data << 1) | (m_dma.source_address & 0xfffe00);
	}));
	map(22, 22).lw8(NAME([this] (u8 data) {
		LOGREGSDMA("#22: DMA source mid %02x\n", data);
		m_dma.source_address = (data << 9) | (m_dma.source_address & 0xfe01fe);
	}));
	map(23, 23).lw8(NAME([this] (u8 data) {
		LOGREGSDMA("#23: DMA source high %02x | data %02x mode %02x\n", data, data & 0x3f, data >> 6);
		m_dma.source_address = ((data & 0x3f) << 17) | (m_dma.source_address & 0x01fffe);
		if (BIT(data, 7))
		{
			m_dma.source_address &= 0x7fffff;
			m_dma.mode = BIT(data, 6) ? VRAM_COPY : VRAM_FILL;
		}
		else
		{
			m_dma.mode = MEMORY_TO_VRAM;
			m_dma.source_address |= (BIT(data, 6) << 23);
		}
	}));

}

TIMER_CALLBACK_MEMBER(ym7101_device::vint_trigger_callback)
{
	if (m_ie0)
		m_vint_callback(1);
}

TIMER_CALLBACK_MEMBER(ym7101_device::hint_trigger_callback)
{
	m_hint_callback(1);
}


void ym7101_device::prepare_sprite_line(int scanline)
{
	// TODO: configure for H32
	std::fill_n(&m_sprite_line[0], 320, 0);

	int num_sprites = 20;
	int num_pixels = 320;
	u16 link = 0;
	u16 offset = 0;
	int y, x;
	u16 height, width;

	do {
		const u16 *cache = &m_sprite_cache[offset];
		const u16 *vram = &m_vram[(m_sprite_attribute_table >> 1) + offset];

		y = (cache[0] & 0x1ff) - 128;
		height = (((cache[1] >> 8) & 0x3) + 1) * 8;

		if (scanline == std::clamp(scanline, y, y + height - 1))
		{
			num_sprites --;
			width = (((cache[1] >> 10) & 0x3) + 1) * 8;
			x = (vram[3] & 0x1ff) - 128;

			const u16 id_flags = vram[2];
			const u16 tile = id_flags & 0x7ff;
			const bool flipx = !!BIT(id_flags, 11);
			const bool flipy = !!BIT(id_flags, 12);
			const u8 color = ((id_flags >> 13) & 3) << 4;
			const bool high_priority = !!BIT(id_flags, 15);

			const int yi = scanline - y;

			for (int xi = 0; xi < width; xi ++)
			{
				num_pixels --;

				if (x + xi < 0 || x + xi >= 320 || num_pixels < 0)
					continue;

				const int x_offs = flipx ? width - xi - 1 : xi;
				const int y_offs = flipy ? height - yi - 1 : yi;
				const u8 x_char = x_offs & 7;
				const u8 y_char = y_offs & 7;
				// https://plutiedev.com/sprites#sprite-format
				// column first
				const u32 sprite_offs = ((x_offs >> 3) * (height >> 3) + (y_offs >> 3)) << 4;

				const u8 dot = m_vram[(tile << 4) + BIT(x_char, 2) + (y_char << 1) + sprite_offs] >> (((3 ^ x_char) & 3) * 4) & 0xf;

				if ((m_sprite_line[x + xi] & 0xf) == 0)
					m_sprite_line[x + xi] = (color) | (dot & 0xf) | (high_priority << 6);
				// sprite collision
				// else if (dot)
			}
		}

		link = cache[1] & 0x7f;

		// TODO: https://plutiedev.com/mirror/kabuto-hardware-notes#sprite-rendering-beyond-80
		if (link >= 80)
		{
			// - teradrive pzlcnst game.exe tends to use a link = 0x7f (127) on piece removals,
			//   with X and Y at max values (0xffff).
			//   Looks just a quick way to draw nothing that works by chance.
			// - rambo3 references link = 80 during attract.
			if (link != 0x7f)
				popmessage("ym7101: attempt to access link $%d, aborted", link);
			break;
		}

		// jumping on the same offset just aborts the chain.
		// rambo3 depends on this during intro (will deadlock otherwise)
		// implicitly hitting 64/80 sprites per screen max?
		if (offset == link * 4)
		{
			break;
		}

		offset = link * 4;

	} while(num_sprites > 0 && num_pixels > 0 && link != 0);

	// sprite overflow, here
}

bool ym7101_device::render_line(int scanline)
{
	if (scanline >= 224)
		return false;

	uint32_t *p = &m_bitmap.pix(scanline);

	//int y = scanline >> 3;
	int yi = scanline & 7;

	if (!m_de)
	{
		const rectangle scanclip(0, 320, scanline, scanline);
		m_bitmap.fill(m_palette->pen(m_background_color), scanclip);
		return true;
	}

	const u16 vram_mask = 0x7ff;
	//const u16 page_mask[] = { 0x7ff, 0x1fff, 0x1fff, 0x1fff };

	const u16 page_masks[] = { 32, 64, 1, 128 };

	const u16 h_page = page_masks[m_hsz];
	const u16 v_page = page_masks[m_vsz];

	prepare_sprite_line(scanline);

	// TODO: smasters wants full screen window for text layer to work
	const int min_y = m_down ? m_wvp * 8 : 0;
	const int max_y = m_down ? 223 : (m_wvp * 8) - 1;

	const bool is_window_y_layer = scanline == std::clamp(scanline, min_y, max_y);

	int min_x = -1;
	int max_x = -1;

	if (is_window_y_layer)
	{
		// TODO: this is intentionally reversed, until I understand what's happening here
		min_x = m_rigt ? 0 : m_whp * 2;
		max_x = m_rigt ? m_whp * 2 : 40;
	}

//	if (min_x != -1 && max_x != -1)
//		popmessage("X %d %d Y %d %d", min_x, max_x, min_y, max_y);

	// mode 1 is <prohibited>, used by d_titov2
	const u16 scroll_x_mode_masks[] = { 0, 0x7, 0xf8, 0xff };
	const u16 scroll_x_base = (scanline & scroll_x_mode_masks[m_hs]) << 1;

	// gynoug, mushaj, btlmanid
	// TODO: buggy with first column if HS also enabled (gynoug)
	const u8 scroll_y_mask = m_vs ? 0x7e : 0;

	for (int x = 0; x < 40; x ++)
	{
		// TODO: prettify, shouldn't need scrolly in branch
		u16 id_flags_a, tile_a;
		u8 flipx_a, flipy_a, color_a;
		bool high_priority_a;
		const u16 scrollx_a_frac = 0; //scrollx_a & 7;
		u16 scrolly_a_frac;

		if (is_window_y_layer && x == std::clamp(x, min_x, max_x))
		{
			const u16 vcolumn_a = scanline & ((v_page * 8) - 1);
			const u32 tile_offset_a = (x & ((h_page * 1) - 1)) + ((vcolumn_a >> 3) * (h_page >> 0));
			scrolly_a_frac = 0;
			id_flags_a = m_vram[((m_window_name_table >> 1) + tile_offset_a) & 0x1ffff];
			tile_a = id_flags_a & vram_mask;
			flipx_a = BIT(id_flags_a, 11) ? 4 : 3;
			flipy_a = BIT(id_flags_a, 12) ? 7 : 0;
			color_a = ((id_flags_a >> 13) & 3) << 4;
			high_priority_a = !!BIT(id_flags_a, 15);
		}
		else
		{
			const u16 scrollx_a = m_vram[(m_hscroll_address >> 1) + scroll_x_base];
			const u16 scrolly_a = m_vsram[x & scroll_y_mask];
			const u16 vcolumn_a = (scrolly_a + scanline) & ((v_page * 8) - 1);
			const u32 tile_offset_a = ((x - (scrollx_a >> 3)) & ((h_page * 1) - 1)) + ((vcolumn_a >> 3) * (h_page >> 0));
			scrolly_a_frac = scrolly_a & 7;

			id_flags_a = m_vram[((m_plane_a_name_table >> 1) + tile_offset_a) & 0x1ffff];
			tile_a = id_flags_a & vram_mask;
			flipx_a = BIT(id_flags_a, 11) ? 4 : 3;
			flipy_a = BIT(id_flags_a, 12) ? 7 : 0;
			color_a = ((id_flags_a >> 13) & 3) << 4;
			high_priority_a = !!BIT(id_flags_a, 15);
		}

		const u16 scrollx_b = m_vram[(m_hscroll_address >> 1) + 1 + scroll_x_base];
		const u16 scrollx_b_frac = 0; //scrollx_b & 7;
		const u16 scrolly_b = m_vsram[(x & scroll_y_mask) + 1];
		const u16 scrolly_b_frac = scrolly_b & 7;
		const u16 vcolumn_b = (scrolly_b + scanline) & ((v_page * 8) - 1);
		const u32 tile_offset_b = ((x - (scrollx_b >> 3)) & ((h_page * 1) - 1)) + ((vcolumn_b >> 3) * (h_page >> 0));
		const u16 id_flags_b = m_vram[((m_plane_b_name_table >> 1) + tile_offset_b) & 0x1ffff];
		const u16 tile_b = id_flags_b & vram_mask;
		const u8 flipx_b = BIT(id_flags_b, 11) ? 4 : 3;
		const u8 flipy_b = BIT(id_flags_b, 12) ? 7 : 0;
		const u8 color_b = ((id_flags_b >> 13) & 3) << 4;
		const bool high_priority_b = !!BIT(id_flags_b, 15);

		for (int xi = 0; xi < 8; xi++)
		{
			u8 pen = m_background_color;

			const u8 x_char_a = (xi + scrollx_a_frac) & 7;
			const u8 y_char_a = (yi + scrolly_a_frac) & 7;
			const u16 dot_a = m_vram[(tile_a << 4) + BIT(x_char_a ^ flipx_a, 2) + ((y_char_a ^ flipy_a) << 1)] >> (((flipx_a ^ x_char_a) & 3) * 4) & 0xf;

			const u8 x_char_b = (xi + scrollx_b_frac) & 7;
			const u8 y_char_b = (yi + scrolly_b_frac) & 7;
			const u16 dot_b = m_vram[(tile_b << 4) + BIT(x_char_b ^ flipx_b, 2) + ((y_char_b ^ flipy_b) << 1)] >> (((flipx_b ^ x_char_b) & 3) * 4) & 0xf;

			for (int pri = 0; pri < 2; pri ++)
			{
				if (dot_b && high_priority_b == pri)
					pen = (dot_b) | color_b;

				if (dot_a && high_priority_a == pri)
					pen = (dot_a) | color_a;

				if (m_sprite_line[x * 8 + xi] & 0xf && BIT(m_sprite_line[x * 8 + xi], 6) == pri)
					pen = m_sprite_line[x * 8 + xi] & 0x3f;
			}

			p[(x << 3) + xi] = m_palette->pen(pen);
		}
	}

	return true;
}

TIMER_CALLBACK_MEMBER(ym7101_device::scan_timer_callback)
{
	int scanline = param;

	// TODO: to execution pipeline
	if (scanline == 224)
	{
		m_vint_on_timer->adjust(attotime::from_ticks(16, clock()));
		m_vint_pending = 1;
	}

	// TODO: correct?
	if (scanline == 240)
		m_sint_callback(1);
	if (scanline == 241)
		m_sint_callback(0);

	const bool active_scan = render_line(scanline);

	// TODO: should trigger at the end of current display pixel
	if (active_scan)
	{
		m_vcounter ++;
		if (m_vcounter >= m_hit)
		{
			m_vcounter = 0;
			m_hint_pending = 1;
			if (m_ie1)
			{
				m_hint_on_timer->adjust(attotime::from_ticks(16, clock()));
			}
			else
				m_hint_callback(0);
		}
	}
	else
	{
		// NOTE: V counter is not running during vertical border and onward
		m_vcounter = 0;
	}

	scanline ++;
	scanline %= screen().height();

	m_scan_timer->adjust(screen().time_until_pos(scanline), scanline);
}

TIMER_CALLBACK_MEMBER(ym7101_device::dma_callback)
{
	if (!m_dma.active)
	{
		m_dtack_cb(0);
		return;
	}

	const u8 code_dest = m_dma.mode == VRAM_COPY ? AS_VDP_VRAM : (m_command.code >> 1) & 3;
	assert(code_dest != 3);

	const u32 mask_values[] = { 0x1ffff, 0x7f, 0x7f, 0 };
	u32 mask = mask_values[code_dest];
	const u32 src = m_dma.source_address;
	const u32 dst = m_command.address;

	u16 res = 0;

	switch (m_dma.mode)
	{
		case MEMORY_TO_VRAM: res = m_mreq_cb(src); m_dtack_cb(1); break;
		case VRAM_FILL:      res = m_dma.fill; break;
		case VRAM_COPY:      res = space(AS_VDP_VRAM).read_word((src >> 1) & mask, 0xffff); break;
	}

	space(code_dest).write_word((dst >> 0) & mask, res, 0xffff);

	m_dma.source_address = (m_dma.source_address & 0xfe0000) | ((m_dma.source_address + 2) & 0x1ffff);
	m_command.address += m_auto_increment;
	m_command.address &= 0x1ffff;

	m_dma.length --;
	if (m_dma.length == 0)
	{
		m_dtack_cb(0);
		m_dma.active = false;
		m_dma_timer->adjust(attotime::never);
	}
	else
	{
		m_dma_timer->adjust(attotime::from_ticks(8, clock()));
	}
}

u32 ym7101_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


