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
- b16ex2: confirm kanji hookup;

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
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "imagedev/floppy.h"
#include "machine/i8257.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "video/mc6845.h"

#include "formats/pc98_dsk.h"
#include "formats/pc98fdi_dsk.h"
#include "formats/dcp_dsk.h"
#include "formats/dip_dsk.h"
#include "formats/fdd_dsk.h"
#include "formats/nfd_dsk.h"

#include "emupal.h"
#include "screen.h"
//#include "softlist_dev.h"

namespace {

class b16_state : public driver_device
{
public:
	b16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit")
		, m_intm(*this, "intm")
		, m_ints(*this, "ints")
		, m_dma(*this, "dma")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_kanji_rom(*this, "kanji")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
	{ }

	void b16(machine_config &config);
	void b16ex2(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(key_pressed);
	DECLARE_INPUT_CHANGED_MEMBER(key_modifier);
	DECLARE_INPUT_CHANGED_MEMBER(key_lock);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void b16_map(address_map &map) ATTR_COLD;
	void b16_io(address_map &map) ATTR_COLD;
	void b16ex2_map(address_map &map) ATTR_COLD;

private:
	uint8_t m_crtc_vreg[0x100]{}, m_crtc_index = 0;
	u16 m_tvram_attr_latch = 0;
	uint8_t m_port78 = 0;
	uint8_t m_port80 = 0;
	u8 m_dma_page = 0;
	uint8_t m_keyb_scancode = 0;
	std::array<u8, 32> m_keyb_fifo{};
	u8 m_keyb_head = 0, m_keyb_count = 0;
	u8 m_keyb_modifiers = 0, m_keyb_locks = 0;
	u32 m_fdc_rate = 250'000;
	u8 m_port80_vblank_mask = 0xb0;

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<pic8259_device> m_intm;
	required_device<pic8259_device> m_ints;
	required_device<i8257_device> m_dma;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_kanji_rom;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	std::unique_ptr<u8[]> m_ig_ram;

	u16 tvram_r(offs_t offset);
	void tvram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 ig_ram_r(offs_t offset);
	void ig_ram_w(offs_t offset, uint8_t data);
	void crtc_address_w(uint8_t data);
	void crtc_data_w(uint8_t data);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	u8 keyboard_data_r();
	void keyboard_data_w(u8 data);
	void keyboard_push(u8 data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static void floppy_formats(format_registration &fr);
};

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))


void b16_state::video_start()
{
	m_ig_ram = make_unique_clear<u8[]>(0x4000);
	m_gfxdecode->gfx(1)->set_source_and_total(m_ig_ram.get(), 0x200 * 2);

	save_item(NAME(m_crtc_vreg));
	save_item(NAME(m_crtc_index));
	save_item(NAME(m_tvram_attr_latch));
}


uint32_t b16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO: convert to scanline renderer
	for(int y = 0; y < mc6845_v_display; y++)
	{
		for(int x = 0; x < mc6845_h_display; x++)
		{
			const u32 tile_offset = x + y * mc6845_h_display;
			const u16 lo_vram = m_vram[tile_offset];
			const u16 hi_vram = m_vram[tile_offset + 0x4000 / 2];
			const u16 tile = lo_vram & 0x00ff;
			const u8 color = (lo_vram & 0x0700) >> 8;

			for(int yi = 0; yi < mc6845_tile_height; yi++)
			{
				u8 gfx_data = 0;
				// TODO: confirm global character/kanji display control.
				if (BIT(hi_vram, 5))
				{
					// TODO: apply bitswap at device_init time, move this calculation out of this inner loop.
					const u16 lr = BIT(hi_vram, 7) ^ 1;
					const u16 kanji_bank = (hi_vram & 0x1f);
					const u32 kanji_offset = bitswap<13>(tile + (kanji_bank << 8), 12, 7, 6, 5, 11, 10, 9, 8, 4, 3, 2, 1, 0) << 4;
					gfx_data = m_kanji_rom[((kanji_offset + yi) << 1) + lr];
				}
				else
				{
					gfx_data = m_ig_ram[(tile << 4) + yi];
				}

				for(int xi = 0; xi < 8; xi++)
				{
					const int res_x = x * 8 + xi;
					const int res_y = y * mc6845_tile_height + yi;
					if (!cliprect.contains(res_x, res_y))
						continue;

					int const pen = BIT(gfx_data, 7 - xi) ? color : 0;
					bitmap.pix(res_y, res_x) = m_palette->pen(pen);
				}
			}
		}
	}

	return 0;
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

u8 b16_state::ig_ram_r(offs_t offset)
{
	// swap bit 0 for now, so we can see a setup thru debugger later on.
	// SW does mov ah,0 -> stosw when uploading.
	const u16 ig_offset = bitswap<13>(offset, 0, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
	return m_ig_ram[ig_offset];
}

void b16_state::ig_ram_w(offs_t offset, uint8_t data)
{
	const u16 ig_offset = bitswap<13>(offset, 0, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
	m_ig_ram[ig_offset] = data;

	m_gfxdecode->gfx(1)->mark_dirty(ig_offset >> 4);
}

void b16_state::b16_map(address_map &map)
{
	map.unmap_value_high();
	// TODO: amount of work RAM depends on model type
	map(0x00000, 0x9ffff).ram();
	map(0xa0000, 0xaffff).ram(); // bitmap?
	map(0xb0000, 0xb7fff).ram().rw(FUNC(b16_state::tvram_r), FUNC(b16_state::tvram_w)).share("vram");
	map(0xb8000, 0xbbfff).rw(FUNC(b16_state::ig_ram_r), FUNC(b16_state::ig_ram_w)).umask16(0xffff);
	map(0xfc000, 0xfffff).rom().region("ipl", 0);
}

void b16_state::b16ex2_map(address_map &map)
{
	b16_state::b16_map(map);
//  map(0x0e****) bus slot ROM?
	map(0x0f8000, 0x0fffff).rom().region("ipl", 0);
	map(0xff8000, 0xffffff).rom().region("ipl", 0);
}

void b16_state::crtc_address_w(uint8_t data)
{
	m_crtc_index = data;
	m_crtc->address_w(data);
}

void b16_state::crtc_data_w(uint8_t data)
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(data);
}

void b16_state::b16_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x00, 0x07).umask16(0x00ff); // PIT mirror?
	map(0x08, 0x0f).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x10, 0x13).rw(m_intm, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x18, 0x1b).rw(m_ints, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x20, 0x20).w(FUNC(b16_state::crtc_address_w));
	map(0x22, 0x22).w(FUNC(b16_state::crtc_data_w));
//  map(0x28, 0x2b) keyboard, likely thru USART
	map(0x28, 0x28).rw(FUNC(b16_state::keyboard_data_r), FUNC(b16_state::keyboard_data_w));
	// USART ready bits: transmitter ready/empty, receiver ready while a byte is pending.
	// TODO: serial framing and keyboard command responses.
	map(0x2a, 0x2a).lr8(NAME([this] () { return u8(0x05 | (m_keyb_count ? 0x02 : 0x00)); }));
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
	map(0x60, 0x6f).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write)).umask16(0x00ff);
	map(0x70, 0x73).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
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
			return (m_port80 & ~m_port80_vblank_mask) | (m_screen->vblank() ? m_port80_vblank_mask : 0x00);
		}),
		NAME([this] (u8 data) { m_port80 = data; })
	);
//  map(0x00a0, 0x00bf) DMA upper segments or video clut
//  map(0x8020, 0x8023) external FDC? Can branch at FDC irq
}

INPUT_CHANGED_MEMBER(b16_state::key_pressed)
{
	if (newval)
		keyboard_push(u8(param));
}

INPUT_CHANGED_MEMBER(b16_state::key_modifier)
{
	if (newval)
		m_keyb_modifiers |= u8(param);
	else
		m_keyb_modifiers &= ~u8(param);

	keyboard_push(0x80 | (m_keyb_modifiers & 0x03)
		| ((m_keyb_modifiers & 0x0c) ? 0x04 : 0x00)
		| ((m_keyb_modifiers & 0x30) ? 0x08 : 0x00));
}

INPUT_CHANGED_MEMBER(b16_state::key_lock)
{
	if (!newval)
		return;
	m_keyb_locks ^= u8(param);

	// FA834/FA83C: 90 = alpha, 92 = kana, with bit 2 selecting Caps Lock.
	keyboard_push(0x90 | (BIT(m_keyb_locks, 0) ? 0x04 : 0x00)
		| (BIT(m_keyb_locks, 1) ? 0x02 : 0x00));
}

void b16_state::keyboard_data_w(u8 data)
{
	if (data == 0x10)
		keyboard_push(0xf0);
}

void b16_state::keyboard_push(u8 data)
{
	if (m_keyb_count == m_keyb_fifo.size())
	{
		logerror("Keyboard receive buffer full, dropping %02x\n", data);
		return;
	}

	m_keyb_fifo[(m_keyb_head + m_keyb_count) % m_keyb_fifo.size()] = data;
	if (!m_keyb_count++)
		m_ints->ir0_w(1);
}

u8 b16_state::keyboard_data_r()
{
	if (!m_keyb_count)
		return m_keyb_scancode;

	u8 const data = m_keyb_fifo[m_keyb_head];
	if (!machine().side_effects_disabled())
	{
		m_keyb_scancode = data;
		m_keyb_head = (m_keyb_head + 1) % m_keyb_fifo.size();
		--m_keyb_count;
		m_ints->ir0_w(0);
		if (m_keyb_count)
			m_ints->ir0_w(1);
	}
	return data;
}


static INPUT_PORTS_START( b16 )
	PORT_START("KBD0MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_modifier), 0x01)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_modifier), 0x02)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_modifier), 0x04)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Ctrl") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_modifier), 0x08)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Graph (Left Alt)") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_modifier), 0x10)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Graph (Right Alt)") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_modifier), 0x20)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KBD1LOCKS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_lock), 0x01)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Kana (Num Lock)") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_lock), 0x02)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY0") // 00..07
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x00)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x01)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x02)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x03)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x04)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x05)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x06)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x07)

	PORT_START("KEY1") // 08..0f
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x08)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x09)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x0a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x0b)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x0c)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x0d)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x0e)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY2") // 10..17
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(0x09) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x10)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x11)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x12)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x13)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x14)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x15)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x16)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x17)

	PORT_START("KEY3") // 18..1f
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x18)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x19)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x1a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x1b)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x1c)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x1d)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY4") // 20..27
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x20)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x21)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x22)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x23)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x24)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x25)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x26)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x27)

	PORT_START("KEY5") // 28..2f
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x28)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x29)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x2a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x2b)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x2c)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY6") // 30..37
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x30)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x31)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x32)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x33)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x34)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x35)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x36)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x37)

	PORT_START("KEY7") // 38..3f
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x38)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x39)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ro / _") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0) PORT_CHAR('_') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x3a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Insert") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x3b)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x3c)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x3d)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x3e)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY8") // 40..47
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x43)
	PORT_BIT(0xf7, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY9") // 48..4f
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x48)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x49)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x4a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x4b)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x4e)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Return") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(0x0d) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x4f)
	PORT_BIT(0x30, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYA") // 50..57
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYB") // 58..5f
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x5d)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x5e)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x5f)
	PORT_BIT(0x1f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYC") // 60..67
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x60)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x61)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x62)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x63)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x64)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x65)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x66)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x67)

	PORT_START("KEYD") // 68..6f
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x68)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x69)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x6a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x6b)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYE") // 70..77
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(PAUSE)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x71)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Print Screen") PORT_CODE(KEYCODE_PRTSCR) PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x73)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad /") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x74)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x75)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x76)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x77)
	PORT_BIT(0x05, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYF") // 78..7f
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x78)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x79)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x7a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad =") PORT_CHAR('=') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x7b)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x7c)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x7d)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x7e)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad ,") PORT_CHAR(',') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(b16_state::key_pressed), 0x7f)
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
	save_item(NAME(m_dma_page));
	save_item(NAME(m_keyb_scancode));
	save_item(NAME(m_keyb_fifo));
	save_item(NAME(m_keyb_head));
	save_item(NAME(m_keyb_count));
	save_item(NAME(m_keyb_modifiers));
	save_item(NAME(m_keyb_locks));
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
	m_tvram_attr_latch = 0;
	m_dma->dreq0_w(0);
	m_fdc->tc_w(false);
	m_keyb_scancode = 0;
	m_keyb_head = m_keyb_count = 0;
	m_keyb_modifiers = m_keyb_locks = 0;
	m_ints->ir0_w(0);
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

	screen_device &screen(SCREEN(config, m_screen));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(b16_state::screen_update));
	screen.set_size(640, 400);
	screen.set_visarea_full();
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_b16);
	// TODO: palette format is a guess
	PALETTE(config, m_palette, palette_device::BRG_3BIT).set_entries(8);
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

	m_palette->set_entries(16);

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

COMP( 1983?, b16ex2,  0,      0,      b16ex2,  b16,   b16_state, empty_init, "Hitachi", "B16 EX-II",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// B16 MX-II, 1~2MB RAM + 640x494 extra video mode + support for M[ultiuser?]DOS

// B16 EX-III, 386 based? <fill me>

// B16 LX, LCD variants
// Ricoh Mr. My Tool (Mr.マイツール), sister variants with 3.5 2HD floppies
