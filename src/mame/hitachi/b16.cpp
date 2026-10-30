// license: BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Mike Stedman
/**************************************************************************************************

Hitachi B(asic Master?) 16000 series?

TODO:
- Barely anything is known about the HW;
- confirm keyboard command responses and typematic;
- Confirm exact $80 video/status signals (bit 4 on EX-II, bit 5 on the earlier ROM);
- Confirm whether the motor-enable line is shared by both floppy drives;
- b16ex2: improve kanji hookup (needs more use cases);

===================================================================================================

B16 EX-II PCB contents:

TIM uPD8253C-5 @ 16B
DMAC uPD8257C-5 @ 18B
INTM / INTS 2x uPD8259AC @ 13D / 15D
Labelless CRTC, Fujitsu 6845 package
FDC is unpopulated on board, should be a 765 coupled with a SED9420C/AC
USART is unpopulated on board, assume i8251
PPI is unpopulated on board
BDC HN65013G025 @ 2A
CAL HN6223S @ 12K  - Near bus slots
CA HN6022B @ 12L   /
MPX HG61H06R29F @ 9J
RAC HN60236 - 81005V @ 9H
A HN60230 - 81007V @ 9F
PAC HG61H20B12F @ 9D
DECO HG61H15B19F @ 11D
KAM HN671105AU @ 16A
DECI HG61H15B19F @ 2F
VAP NEC uPD65030G035 @ 4K
GN NEC uPD65021G030 @ 2K
4x 32x3 slots, labeled SLOT0 to 3
11 Jumpers "SH1"
Several connectors scattered across the board
Centronics port CN9, Serial port CN8
2 empty sockets for CN10 / CN11 (DE-9 options?)

===================================================================================================

irq vectors

intm
ir0 pit timer (does work RAM logic only)
ir1 FDC irq
ir2 ?
ir3 vblank?
ir4 ?
ir5 ?
ir6 slave ack
ir7 ?

ints
ir0 keyboard
ir1 ?
ir2 ?
ir3 device at I/O $38 / $3a
ir4 device at I/O $4c / $4e
ir5 ?
ir6 ?
ir7 ?

===================================================================================================

Error codes (TODO: RE them all)
0301 PIT failure
1101 b16ex2 bus config error?

**************************************************************************************************/

#include "emu.h"
#include "b16_kbd.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "imagedev/floppy.h"
#include "machine/i8257.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

#include "formats/pc98_dsk.h"
#include "formats/pc98fdi_dsk.h"
#include "formats/dcp_dsk.h"
#include "formats/dip_dsk.h"
#include "formats/fdd_dsk.h"
#include "formats/nfd_dsk.h"


namespace {

class b16_state : public driver_device
{
public:
	b16_state(const machine_config &mconfig, device_type type, const char *tag, unsigned graphics_planes = 3)
		: driver_device(mconfig, type, tag)
		, m_graphics_planes(graphics_planes)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit")
		, m_intm(*this, "intm")
		, m_ints(*this, "ints")
		, m_keyboard(*this, "keyboard")
		, m_dma(*this, "dma")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
		, m_gvram(*this, "gvram", graphics_planes * 0x10000, ENDIANNESS_LITTLE)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_kanji_rom(*this, "kanji")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
	{ }

	void b16(machine_config &config) ATTR_COLD;
	void b16ex2(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void b16_map(address_map &map) ATTR_COLD;
	void b16_io(address_map &map) ATTR_COLD;
	void b16ex2_map(address_map &map) ATTR_COLD;

	void palette_init(palette_device &palette) const ATTR_COLD;

	u8 m_port80_config_mask = 0;
	u8 m_port80_config = 0;

private:
	const unsigned m_graphics_planes;
	u8 m_graphics_mask = 0xff;
	u8 m_graphics_palette[16]{};
	u8 m_graphics_data[4]{};
	u8 m_graphics_latch[4]{};
	u8 m_graphics_read_plane = 0;
	u8 m_graphics_read_count = 4;
	u8 m_graphics_write_count = 4;
	u8 m_graphics_copy = 0;
	u8 m_graphics_display_page = 0;
	u16 m_tvram_attr_latch = 0;
	u8 m_port84 = 0;
	uint8_t m_port78 = 0;
	uint8_t m_port80 = 0;
	u8 m_dma_page = 0;
	u32 m_fdc_rate = 250'000;
	u8 m_port80_vblank_mask = 0xb0;

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<pic8259_device> m_intm;
	required_device<pic8259_device> m_ints;
	required_device<b16_kbd_device> m_keyboard;
	required_device<i8257_device> m_dma;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_vram;
	memory_share_creator<u8> m_gvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_kanji_rom;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	std::unique_ptr<u8[]> m_ig_ram;

	u16 tvram_r(offs_t offset);
	void tvram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 graphics_r(offs_t offset);
	void graphics_w(offs_t offset, u8 data);
	void graphics_ctrl_w(offs_t offset, u8 data);
	u8 graphics_palette_r(offs_t offset);
	void graphics_palette_w(offs_t offset, u8 data);
	u8 kanji_r(offs_t offset);
	u8 ig_ram_r(offs_t offset);
	void ig_ram_w(offs_t offset, uint8_t data);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	MC6845_UPDATE_ROW(crtc_update_row);

	static void floppy_formats(format_registration &fr);
};

// The legacy C interface uses three planes; EX-II adds a fourth via D.
class b16ex2_state : public b16_state
{
public:
	b16ex2_state(const machine_config &mconfig, device_type type, const char *tag)
		: b16_state(mconfig, type, tag, 4)
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
};

// Text and bitmap colours use BGR components. Intensity levels are provisional.
void b16_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, pal1bit(BIT(i, 2)), pal1bit(BIT(i, 1)), pal1bit(BIT(i, 0)));
		palette.set_pen_color(i + 8, BIT(i, 2) * 0xaa + 0x55, BIT(i, 1) * 0xaa + 0x55, BIT(i, 0) * 0xaa + 0x55);
	}
}

void b16_state::video_start()
{
	m_ig_ram = make_unique_clear<u8[]>(0x2000);
	m_gfxdecode->gfx(1)->set_source_and_total(m_ig_ram.get(), 0x200);
	save_pointer(NAME(m_ig_ram), 0x2000);
	std::fill(m_gvram.begin(), m_gvram.end(), 0);
	save_item(NAME(m_graphics_mask));
	save_item(NAME(m_graphics_palette));
	save_item(NAME(m_graphics_data));
	save_item(NAME(m_graphics_latch));
	save_item(NAME(m_graphics_read_plane));
	save_item(NAME(m_graphics_read_count));
	save_item(NAME(m_graphics_write_count));
	save_item(NAME(m_graphics_copy));
	save_item(NAME(m_graphics_display_page));

	save_item(NAME(m_tvram_attr_latch));
}


uint32_t b16_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	return m_crtc->screen_update(screen, bitmap, cliprect);
}

MC6845_UPDATE_ROW(b16_state::crtc_update_row)
{
	if (!de)
		return;

	const pen_t *const palette = m_palette->pens();
	// TODO: confirm the text attribute blink rate from the video circuitry.
	const bool blink_off = BIT(m_screen->frame_number(), 4);
	// Raster rows within each plane are 0800h bytes apart. CC/DC uses
	// 16 KiB units; the BIOS selects value 2 for the second 640x400 page.
	const u32 graphics_row = (u32(m_graphics_display_page) << 14) + (u32(ra) << 11) + ma;
	auto *const dest = &bitmap.pix(y);
	for (unsigned x = 0; x < x_count; x++)
	{
		const u32 tile_offset = (ma + x) & 0x1fff;
		const u16 lo_vram = m_vram[tile_offset];
		const u16 hi_vram = m_vram[tile_offset + 0x4000 / 2];
		const u16 tile = lo_vram & 0x00ff;
		const u8 fg_color = (lo_vram >> 8) & 0x0f;
		const u8 bg_color = (lo_vram >> 12) & 0x07;
		u8 gfx_data;
		// Character generators contain sixteen raster rows per glyph.
		if (BIT(hi_vram, 5))
		{
			const u16 lr = BIT(hi_vram, 7) ^ 1;
			const u16 kanji_bank = hi_vram & 0x1f;
			const u32 kanji_offset = bitswap<13>(tile + (kanji_bank << 8), 12, 7, 6, 5, 11, 10, 9, 8, 4, 3, 2, 1, 0) << 4;
			gfx_data = m_kanji_rom[((kanji_offset + (ra & 0x0f)) << 1) + lr];
		}
		else
		{
			gfx_data = m_ig_ram[(tile << 4) + (ra & 0x0f)];
		}
		if (BIT(lo_vram, 15) && blink_off)
			gfx_data = 0;

		// CRTC supplies the cursor position, raster shape and blink phase.
		// Keep it visible even over the black attributes left by BIOS/BASIC.
		// TODO: confirm the cursor colour from the video circuitry.
		const bool cursor = int(x) == cursor_x;
		const u32 address = (graphics_row + x) & 0xffff;
		for (unsigned xi = 0; xi < 8; xi++)
		{
			const int res_x = x * 8 + xi;
			if (!cliprect.containsx(res_x))
				continue;

			u8 graphics_color = 0;
			for (unsigned plane = 0; plane < m_graphics_planes; plane++)
				graphics_color |= BIT(m_gvram[plane * 0x10000 + address], 7 - xi) << plane;
			dest[res_x] = palette[16 + graphics_color];
			if (cursor)
				dest[res_x] = palette[7];
			else if (BIT(gfx_data, 7 - xi))
				dest[res_x] = palette[fg_color];
			else if (bg_color)
				// Background zero is transparent; coloured key labels are opaque.
				dest[res_x] = palette[bg_color];
		}
	}
}

u16 b16_state::tvram_r(offs_t offset)
{
	if (offset < 0x2000 && BIT(m_port78, 2) && !machine().side_effects_disabled())
		m_tvram_attr_latch = m_vram[offset + 0x2000];
	return m_vram[offset];
}

void b16_state::tvram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	if (offset < 0x2000 && BIT(m_port78, 2))
		m_vram[offset + 0x2000] = m_tvram_attr_latch;
}

u8 b16_state::graphics_r(offs_t offset)
{
	const u8 data = m_gvram[m_graphics_read_plane * 0x10000 + offset];
	if (!machine().side_effects_disabled())
	{
		// The BIOS uses a read followed by a write to move all planes together.
		for (unsigned plane = 0; plane < m_graphics_planes; plane++)
			m_graphics_latch[plane] = m_gvram[plane * 0x10000 + offset];
		m_graphics_read_plane = (m_graphics_read_plane + 1) % m_graphics_read_count;
	}
	return data;
}

void b16_state::graphics_w(offs_t offset, u8)
{
	// CPU write data is a strobe. Pixel data comes from the plane registers
	// (or the read latches during CEh/DEh transfers), selected by C0h's mask.
	for (unsigned plane = 0; plane < m_graphics_write_count; plane++)
	{
		u8 &dest = m_gvram[plane * 0x10000 + offset];
		const u8 source = m_graphics_copy ? m_graphics_latch[plane] : m_graphics_data[plane];
		dest = (dest & ~m_graphics_mask) | (source & m_graphics_mask);
	}
}

void b16_state::graphics_ctrl_w(offs_t offset, u8 data)
{
	// The C register set is used by the B16 BIOS;
	// EX-II firmware uses the four-plane D register set and the same mask.
	const unsigned reg = offset * 2;
	const unsigned count = std::min(m_graphics_planes, reg < 0x10 ? 3U : 4U);
	switch (reg)
	{
	case 0x00: m_graphics_mask = data; break;
	case 0x02: case 0x04: case 0x06:
	case 0x12: case 0x14: case 0x16:
		m_graphics_data[((reg & 0x0f) - 2) / 2] = data;
		m_graphics_write_count = count;
		break;
	case 0x10:
		m_graphics_data[3] = data;
		m_graphics_write_count = m_graphics_planes;
		break;
	case 0x08: case 0x18:
		m_graphics_read_count = count;
		m_graphics_read_plane = (data & 3) % count;
		break;
	case 0x0a: case 0x1a:
		for (unsigned plane = 0; plane < count; plane++)
			m_graphics_data[plane] = BIT(data, plane) ? 0xff : 0x00;
		m_graphics_write_count = count;
		break;
	case 0x0c: case 0x1c:
		m_graphics_display_page = data;
		break;
	case 0x0e: case 0x1e:
		m_graphics_copy = BIT(data, 0);
		m_graphics_write_count = count;
		break;
	}
}

u8 b16_state::graphics_palette_r(offs_t offset)
{
	// CTM reads the four colour bits inverted; unused bits remain high.
	return m_graphics_palette[offset] ^ 0xff;
}

void b16_state::graphics_palette_w(offs_t offset, u8 data)
{
	m_graphics_palette[offset] = data & 0x0f;
	// BGR component order agrees with the B16 colour display reference.
	// TODO: confirm physical intensity levels.
	const u8 low = BIT(data, 3) ? 0x55 : 0x00;
	const u8 high = BIT(data, 3) ? 0xff : 0xaa;
	m_palette->set_pen_color(16 + offset, rgb_t(
		BIT(data, 2) ? high : low, BIT(data, 1) ? high : low, BIT(data, 0) ? high : low));
}

u8 b16_state::kanji_r(offs_t offset)
{
	// 84h.bit0 selects the left/right byte of a glyph row; bit1 selects
	// the upper ROM half. The CPU window is on the low byte lane.
	return m_kanji_rom[(offset << 1) | ((m_port84 & 2) << 16) | BIT(m_port84, 0)];
}

u8 b16_state::ig_ram_r(offs_t offset)
{
	return m_ig_ram[offset];
}

void b16_state::ig_ram_w(offs_t offset, uint8_t data)
{
	// 8 KiB CGRAM uses the low byte lane of B8000h-BBFFFh. CPU A13
	// separates the ordinary glyphs from the external glyphs at BA000h.
	m_ig_ram[offset] = data;
	m_gfxdecode->gfx(1)->mark_dirty(offset >> 4);
}

void b16_state::b16_map(address_map &map)
{
	map.unmap_value_high();
	// TODO: amount of work RAM depends on model type
	map(0x00000, 0x9ffff).ram();
	map(0xa0000, 0xaffff).umask16(0xffff).rw(FUNC(b16_state::graphics_r), FUNC(b16_state::graphics_w));
	map(0xb0000, 0xb7fff).ram().rw(FUNC(b16_state::tvram_r), FUNC(b16_state::tvram_w)).share("vram");
	map(0xb8000, 0xbbfff).umask16(0x00ff).rw(FUNC(b16_state::ig_ram_r), FUNC(b16_state::ig_ram_w));
	map(0xc0000, 0xdffff).umask16(0x00ff).r(FUNC(b16_state::kanji_r));
	map(0xfc000, 0xfffff).rom().region("ipl", 0);
}

void b16_state::b16ex2_map(address_map &map)
{
	b16_state::b16_map(map);
//  map(0x0e****) bus slot ROM?
	map(0x0f8000, 0x0fffff).rom().region("ipl", 0);
	map(0xff8000, 0xffffff).rom().region("ipl", 0);
}

void b16_state::b16_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x00, 0x07).umask16(0x00ff); // PIT mirror?
	map(0x08, 0x0f).umask16(0x00ff).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x10, 0x13).umask16(0x00ff).rw(m_intm, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x18, 0x1b).umask16(0x00ff).rw(m_ints, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x20, 0x20).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x22, 0x22).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
//  map(0x28, 0x2b) keyboard, likely thru USART
	map(0x28, 0x28).rw(m_keyboard, FUNC(b16_kbd_device::data_r), FUNC(b16_kbd_device::data_w));
	map(0x2a, 0x2a).r(m_keyboard, FUNC(b16_kbd_device::status_r));
	// Jumper block?
//  map(0x40, 0x40)
//  map(0x42, 0x42)
//  map(0x44, 0x44)
	// b16ex2: jumps to $e0000 if bit 7 high, noisy on bit 4
	map(0x48, 0x48).lr8(NAME([] () { return 0; }));
	map(0x50, 0x50).lrw8(
		NAME([this] () { return m_dma->read(8); }),
		NAME([this] (u8 data) { m_dma->write(8, data); })
	);
	map(0x60, 0x6f).umask16(0x00ff).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x70, 0x73).umask16(0x00ff).m(m_fdc, FUNC(upd765a_device::map));
	map(0x74, 0x74).lw8(NAME([this] (u8 data) { m_dma_page = data & 0x0f; }));
	map(0x78, 0x78).lrw8(
		NAME([this] () {
			return m_port78;
		}),
		NAME([this] (u8 data) {
			logerror("Port $78 %02x\n", data);
			// bit 0: TC strobe?
			// bit 2: FDC reset?
			m_port78 = data;
			// Bit 0 is set before spin-up and cleared by the BIOS motor timeout.
			// Bit 2 enables character/attribute transfer.
			// A common motor-enable line for the two drives is assumed.
			for (auto &connector : m_floppy)
				if (floppy_image_device *const floppy = connector->get_device())
					floppy->mon_w(!BIT(data, 0));
		})
	);
	map(0x79, 0x79).lrw8(
		NAME([] () {
			// bit 0 read at POST
			return 0;
		}),
		NAME([this] (u8 data) {
			logerror("Port $79 write %02x\n", data);
		})
	);
	// Configuration/video control latch; timing status is not writable.
	// B16 ROM polls bit 5; B16EXII ROM polls bit 4.
	// TODO: confirm the source/polarity of these vblank-derived status bits.
	map(0x80, 0x80).lrw8(
		NAME([this] () {
			return (m_port80 & ~(m_port80_vblank_mask | m_port80_config_mask)) | m_port80_config
				| (m_screen->vblank() ? m_port80_vblank_mask : 0x00);
		}),
		NAME([this] (u8 data) { m_port80 = data; })
	);
	map(0x84, 0x84).lw8(NAME([this] (u8 data) { m_port84 = data & 3; }));
	map(0xa0, 0xbf).umask16(0x00ff).rw(FUNC(b16_state::graphics_palette_r), FUNC(b16_state::graphics_palette_w));
	map(0xc0, 0xdf).umask16(0x00ff).w(FUNC(b16_state::graphics_ctrl_w));
//  map(0x8020, 0x8023) external FDC? Can branch at FDC irq
}

static INPUT_PORTS_START( b16 )
	// Keep this shared entry for future system inputs.
INPUT_PORTS_END




static const gfx_layout charlayout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static const gfx_layout kanjilayout =
{
	16, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,8*2) },
	16*16
};


static GFXDECODE_START( gfx_b16 )
	GFXDECODE_ENTRY( "kanji", 0x0000, kanjilayout, 0, 1 )
	GFXDECODE_RAM( nullptr, 0x0000, charlayout, 0, 1 )
GFXDECODE_END

uint8_t b16_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte((u32(m_dma_page) << 16) | offset);
}

void b16_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte((u32(m_dma_page) << 16) | offset, data);
}

static void b16_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525hd", FLOPPY_525_HD); // Y-E Data YD-380 (B16 EX onward)
	device.option_add("35hd", FLOPPY_35_HD); // Optional 3.5" HD drive
}

void b16_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_PC98_FORMAT);
	fr.add(FLOPPY_PC98FDI_FORMAT);
	fr.add(FLOPPY_FDD_FORMAT);
	fr.add(FLOPPY_DCP_FORMAT);
	fr.add(FLOPPY_DIP_FORMAT);
	fr.add(FLOPPY_NFD_FORMAT);
}

void b16_state::machine_start()
{
	save_item(NAME(m_port78));
	save_item(NAME(m_port80));
	save_item(NAME(m_port84));
	save_item(NAME(m_dma_page));
}

void b16ex2_state::machine_start()
{
	b16_state::machine_start();
	// EX-II BIOS reads bit 2 as the colour-display capability input.
	// Control writes must not turn it into a monochrome configuration.
	// TODO: confirm its physical source and configuration on other models.
	m_port80_config_mask = 0x04;
}

void b16_state::machine_reset()
{
	// Keep the selected drive's native speed (300 RPM DD or 360 RPM HD).
	// Japanese 1.23MB mode uses 360 RPM
	m_fdc->set_rate(m_fdc_rate);
	if (m_fdc_rate == 500'000)
	{
		for (auto &connector : m_floppy)
		{
			if (auto *floppy = connector->get_device(); floppy && floppy->get_form_factor() == floppy_image::FF_35)
				floppy->set_rpm(360);
		}
	}
	m_dma_page = 0;
	m_port78 = 0;
	m_port80 = 0;
	m_port80_config = 0;
	m_tvram_attr_latch = 0;
	m_port84 = 0;
	m_graphics_mask = 0xff;
	std::fill(std::begin(m_graphics_data), std::end(m_graphics_data), 0);
	std::fill(std::begin(m_graphics_latch), std::end(m_graphics_latch), 0);
	m_graphics_read_plane = 0;
	m_graphics_read_count = m_graphics_write_count = m_graphics_planes;
	m_graphics_copy = m_graphics_display_page = 0;
	for (unsigned i = 0; i < 16; i++)
		graphics_palette_w(i, i);
	m_dma->dreq0_w(0);
	m_fdc->tc_w(false);
}

void b16ex2_state::machine_reset()
{
	b16_state::machine_reset();
	m_port80_config = 0x04;
}

void b16_state::b16(machine_config &config)
{
	// TODO: attached ROM may be B16 EX instead, with 8086-2 (8 MHz)
	// Original B16 should be I8088
	I8086(config, m_maincpu, XTAL(16'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &b16_state::b16_map);
	m_maincpu->set_addrmap(AS_IO, &b16_state::b16_io);
	m_maincpu->set_irq_acknowledge_callback(m_intm, FUNC(pic8259_device::inta_cb));

	PIT8253(config, m_pit);
	// TODO: unconfirmed, just enough to make it surpass POST checks
	m_pit->set_clk<0>(XTAL(16'000'000) / 8);
	m_pit->out_handler<0>().set(m_intm, FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(XTAL(16'000'000) / 4);
//  m_pit->out_handler<1>()
	m_pit->set_clk<2>(XTAL(16'000'000) / 4);
//  m_pit->out_handler<2>()

	I8257(config, m_dma, XTAL(16'000'000));
	m_dma->in_memr_cb().set(FUNC(b16_state::memory_read_byte));
	m_dma->out_memw_cb().set(FUNC(b16_state::memory_write_byte));
	m_dma->out_hrq_cb().set([this] (int state) {
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_dma->hlda_w(state);
	});
	m_dma->in_ior_cb<0>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dma->out_iow_cb<0>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dma->out_tc_cb().set(m_fdc, FUNC(upd765a_device::tc_line_w));

	PIC8259(config, m_intm);
	m_intm->out_int_callback().set_inputline(m_maincpu, 0);
	m_intm->in_sp_callback().set_constant(1);
	m_intm->read_slave_ack_callback().set(m_ints, FUNC(pic8259_device::acknowledge));

	PIC8259(config, m_ints);
	m_ints->out_int_callback().set(m_intm, FUNC(pic8259_device::ir6_w));
	m_ints->in_sp_callback().set_constant(0);

	B16_KBD(config, m_keyboard);
	m_keyboard->irq_callback().set(m_ints, FUNC(pic8259_device::ir0_w));

	// clock unconfirmed, definitely want the ready line on
	// would stop at `SEARCH_ADDRESS_MARK_HEADER` otherwise
	UPD765A(config, m_fdc, XTAL(16'000'000) / 2, true, true);
	m_fdc->intrq_wr_callback().set(m_intm, FUNC(pic8259_device::ir1_w));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq0_w));
	FLOPPY_CONNECTOR(config, "fdc:0", b16_floppies, "525dd", b16_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", b16_floppies, "525dd", b16_state::floppy_formats).enable_sound(true);

	/* unknown variant, unknown clock, hand tuned to get ~60 fps */
	MC6845(config, m_crtc, XTAL(16'000'000)/6);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(b16_state::crtc_update_row));

	screen_device &screen(SCREEN(config, m_screen));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(b16_state::screen_update));
	screen.set_size(640, 400);
	screen.set_visarea_full();

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_b16);
	PALETTE(config, m_palette, FUNC(b16_state::palette_init)).set_entries(32);

	SOFTWARE_LIST(config, "disk_list").set_original("b16");
}

void b16_state::b16ex2(machine_config &config)
{
	b16_state::b16(config);
	m_fdc->subdevice<floppy_connector>("0")->set_default_option("525hd");
	m_fdc->subdevice<floppy_connector>("1")->set_default_option("525hd");
	m_fdc_rate = 500'000;
	m_port80_vblank_mask = 0x90;
	I80286(config.replace(), m_maincpu, XTAL(16'000'000) / 2); // A80286-8 / S
	m_maincpu->set_addrmap(AS_PROGRAM, &b16_state::b16ex2_map);
	m_maincpu->set_addrmap(AS_IO, &b16_state::b16_io);
	m_maincpu->set_irq_acknowledge_callback(m_intm, FUNC(pic8259_device::inta_cb));

	// TODO: as above
	m_pit->set_clk<0>(XTAL(16'000'000) / 4);
//  m_pit->out_handler<0>()
	m_pit->set_clk<1>(XTAL(16'000'000) / 4);
//  m_pit->out_handler<1>()
	m_pit->set_clk<2>(XTAL(16'000'000) / 2);
//  m_pit->out_handler<2>()

	// Sixteen text pens remain independent of sixteen bitmap pens.

	// 512~1.5M RAM
	// 5.25" FDD x 2
}


ROM_START( b16 )
	ROM_REGION16_LE( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(7c1c93d5) SHA1(2a1e63a689c316ff836f21646166b38714a18e03) )

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
ROM_END

ROM_START( b16ex2 )
	ROM_REGION16_LE( 0x8000, "ipl", ROMREGION_ERASEFF )
	// M27128-L
	ROM_LOAD16_BYTE( "j4c-0072.4b", 0x0001, 0x4000, CRC(7f6e4143) SHA1(afe126639b7161562d93e955c1fc720a93e1596b))
	// M27128-H
	ROM_LOAD16_BYTE( "j5c-0072.6b", 0x0000, 0x4000, CRC(5f3c85ca) SHA1(4988d8e1e763268a62f2a86104db4d106babd242))

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASE00 )
	// Both HN62301AP, sockets labeled "KANJI1" and "KANJI2"
	ROM_LOAD16_BYTE( "7l1.11f", 0x00001, 0x20000, CRC(a31b3efd) SHA1(818b9fbb5109779bb3d66f76a93cc087c3b21698) )
	ROM_LOAD16_BYTE( "7m1.12f", 0x00000, 0x20000, CRC(9eff324e) SHA1(2dfe14cc4b884eb6bb3c953d0ba9677d663b256e) )
ROM_END

} // anonymous namespace


// TODO: pinpoint MB SKU for all
// Original would be MB-16001, flyer shows a "Basic Master" subtitle
COMP( 1982?, b16,     0,      0,      b16,     b16,   b16_state, empty_init, "Hitachi", "B16",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// B16 EX 8086-2
// B16 SX superimpose option, 3.5" x 1.2MB floppies

COMP( 1983?, b16ex2,  0,      0,      b16ex2,  b16,   b16ex2_state, empty_init, "Hitachi", "B16 EX-II",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// B16 MX-II, 1~2MB RAM + 640x494 extra video mode + support for M[ultiuser?]DOS

// B16 EX-III, 386 based? <fill me>

// B16 LX, LCD variants
// Ricoh Mr. My Tool (Mr.マイツール), sister variants with 3.5" 2HD floppies
