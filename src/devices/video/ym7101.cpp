// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Rewrite of Sega 315-5315 MD VDP (tied to Teradrive)

Notes:
- Teradrive actually has 128 KiB compared to stock 64, this means that d_titov2 won't possibly work
here;
- Should eventually derive from 315-5124 (the SMS VDP);

**************************************************************************************************/

#include "emu.h"
#include "ym7101.h"

#define LOG_REGS        (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_REGS)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGREGS(...)    LOGMASKED(LOG_REGS, __VA_ARGS__)


DEFINE_DEVICE_TYPE(YM7101, ym7101_device, "ym7101", "Yamaha YM7101")

ym7101_device::ym7101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, YM7101, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, m_space_vram_config("videoram", ENDIANNESS_BIG, 8, 17, 0, address_map_constructor(FUNC(ym7101_device::vram_map), this))
	, m_space_regs_config("regs", ENDIANNESS_BIG, 8, 6, 0, address_map_constructor(FUNC(ym7101_device::regs_map), this))
	, m_vint_callback(*this)
//  , m_hint_callback(*this)
	, m_sint_callback(*this)
	, m_psg(*this, "psg")
{
}

void ym7101_device::device_start()
{
	m_scan_timer = timer_alloc(FUNC(ym7101_device::scan_timer_callback), this);
}

void ym7101_device::device_reset()
{
	m_ie0 = false;
	m_scan_timer->adjust(screen().time_until_pos(224), 224);
}

void ym7101_device::device_add_mconfig(machine_config &config)
{
	SEGAPSG(config, m_psg, DERIVED_CLOCK(4, 15)).add_route(ALL_OUTPUTS, *this, 1.0, 0);
}

device_memory_interface::space_config_vector ym7101_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_DATA, &m_space_vram_config),
		std::make_pair(AS_IO, &m_space_regs_config)
	};
}


void ym7101_device::if_map(address_map &map)
{
	// Control Port
	map(0x04, 0x05).mirror(2).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			return screen().vblank() << 3;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			//printf("%04x %04x\n", data, mem_mask);
			if (ACCESSING_BITS_0_15)
			{
				// TODO: as FIFO
				if ((data & 0xc000) == 0x8000)
				{
					space(AS_IO).write_byte((data >> 8) & 0x3f, data & 0xff);
				}
			}
		})
	);
	map(0x11, 0x11).mirror(6).w(m_psg, FUNC(segapsg_device::write));
}

void ym7101_device::vram_map(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00000, 0x1ffff).ram();
}

// https://plutiedev.com/vdp-registers
// https://segaretro.org/Sega_Mega_Drive/VDP_registers
// NOTE: in decimal units, classic Yamaha
static const char *const size_names[] = { "256/32", "512/64", "<invalid>", "1024/128" };

void ym7101_device::regs_map(address_map &map)
{
	map(0, 0).lw8(NAME([this] (u8 data) {
		LOGREGS("#00: Mode Register 1 %02x\n", data);
		LOGREGS("\tL %d IE1 %d M4: %d M3: %d DE: %d\n"
			, BIT(data, 5)
			, BIT(data, 4)
			, BIT(data, 2)
			, BIT(data, 1)
			, BIT(data, 0)
		);
	}));
	map(1, 1).lw8(NAME([this] (u8 data) {
		LOGREGS("#01: Mode Register 2 %02x\n", data);
		m_ie0 = !!BIT(data, 5);
		LOGREGS("\tVR %d DE %d IE0: %d M1: %d M2: %d M5: %d\n"
			, BIT(data, 7)
			, BIT(data, 6)
			, m_ie0
			, BIT(data, 4)
			, BIT(data, 3)
			, BIT(data, 2)
		);
	}));
	map(2, 2).lw8(NAME([this] (u8 data) {
		LOGREGS("#02: Plane A Name Table %02x (%05x)\n"
			, data
			, (data & 0x78) << 10
		);
	}));
	map(3, 3).lw8(NAME([this] (u8 data) {
		LOGREGS("#03: Window Name Table %02x (%05x)\n"
			, data
			, (data & 0x7e) << 10
		);
	}));
	map(4, 4).lw8(NAME([this] (u8 data) {
		LOGREGS("#04: Plane B Name Table %02x (%05x)\n"
			, data
			, (data & 0xf) << 13
		);
	}));
	map(5, 5).lw8(NAME([this] (u8 data) {
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
	}));
//  map(8, 8) Master System h scroll
//  map(9, 9) Master System v scroll
	map(10, 10).lw8(NAME([this] (u8 data) {
		LOGREGS("#10: Horizontal Interrupt Counter %02x (%d)\n", data, data);
	}));
	// <-- mode 4 ignores everything beyond this point
	map(11, 11).lw8(NAME([this] (u8 data) {
		LOGREGS("#11: Mode Register 3 %02x\n", data);
		LOGREGS("\tIE2 %d VS %d HS %d\n"
			, BIT(data, 3)
			, BIT(data, 2)
			, data & 3
		);
	}));
	map(12, 12).lw8(NAME([this] (u8 data) {
		LOGREGS("#12: Mode Register 4 %02x\n", data);
		LOGREGS("\tRSx %d (%s) VS %d HS %d EP %d SH %d LSx %d\n"
			, BIT(data, 7)
			, BIT(data, 7) ? "H320" : "H256"
			, BIT(data, 6)
			, BIT(data, 5)
			, BIT(data, 4)
			, BIT(data, 3)
			, (data & 6) >> 1
		);
		if (BIT(data, 7) != BIT(data, 0))
			popmessage("ym7101.cpp: invalid RS setting %02x", data & 0x81);
	}));
	map(13, 13).lw8(NAME([this] (u8 data) {
		LOGREGS("#13: Horizontal Scroll address %02x (%05x)\n", data, (data & 0x7f) << 10);
	}));
	map(14, 14).lw8(NAME([this] (u8 data) {
		// TODO: extra address bits for 128 KiB mode?
		LOGREGS("#14: 128 KiB plane address %02x (%02x)\n", data, data & 0x11);
	}));
	map(15, 15).lw8(NAME([this] (u8 data) {
		LOGREGS("#15: autoincrement %02x\n", data);
	}));
	map(16, 16).lw8(NAME([this] (u8 data) {
		LOGREGS("#16: Plane Size %02x\n", data);
		LOGREGS("\tH %d (%s) V %d (%s)\n"
			, (data & 0x30) >> 4
			, size_names[(data & 0x30) >> 4]
			, (data & 3)
			, size_names[(data & 0x3)]
		);
	}));
	map(17, 17).lw8(NAME([this] (u8 data) {
		LOGREGS("#17: Window Plane Horizontal position %02x\n", data);
		LOGREGS("\tR %d HP %d\n"
			, BIT(data, 7)
			, (data & 0x1f) * 8
		);
	}));
	map(18, 18).lw8(NAME([this] (u8 data) {
		LOGREGS("#18: Window Plane Vertical position %02x\n", data);
		LOGREGS("\tD %d VP %d\n"
			, BIT(data, 7)
			, (data & 0x1f) * 8
		);
	}));
	map(19, 19).lw8(NAME([this] (u8 data) {
		LOGREGS("#19: DMA length low %02x\n", data);
	}));
	map(20, 20).lw8(NAME([this] (u8 data) {
		LOGREGS("#20: DMA length high %02x\n", data);
	}));
	map(21, 21).lw8(NAME([this] (u8 data) {
		LOGREGS("#21: DMA source low %02x\n", data);
	}));
	map(22, 22).lw8(NAME([this] (u8 data) {
		LOGREGS("#22: DMA source mid %02x\n", data);
	}));
	map(23, 23).lw8(NAME([this] (u8 data) {
		LOGREGS("#23: DMA source high %02x data %02x mode %02x\n", data, data & 0x3f, data >> 6);
	}));

}

TIMER_CALLBACK_MEMBER(ym7101_device::scan_timer_callback)
{
	int scanline = param;

	// TODO: to execution pipeline
	if (scanline == 224 && m_ie0)
	{
		m_vint_callback(true);
	}

	// TODO: correct?
	if (scanline == 240)
		m_sint_callback(true);
	if (scanline == 241)
		m_sint_callback(false);

	scanline ++;
	scanline %= screen().height();

	m_scan_timer->adjust(screen().time_until_pos(scanline), scanline);
}

u32 ym7101_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


