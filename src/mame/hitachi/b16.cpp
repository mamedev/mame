// license: BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Mike Stedman
/**************************************************************************************************

Hitachi B(asic Master?) 16000 series?

TODO:
- Barely anything is known about the HW;
- Requires a compatible system disk to make it go further;
- hookup proper keyboard;
- FDC throws disk error, mon_w hookup is unconfirmed, doesn't seem to select a drive ...
- b16: hangs for checking bit 7 in vblank fashion at $80, either the DMA is at the
  wrong spot or the earlier variant effectively don't have it.
- b16ex2: "error message 1101", bypassed between ports $80 and $48
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

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void b16_map(address_map &map) ATTR_COLD;
	void b16_io(address_map &map) ATTR_COLD;
	void b16ex2_map(address_map &map) ATTR_COLD;

private:
	uint8_t m_crtc_vreg[0x100]{}, m_crtc_index = 0;
	uint8_t m_port78 = 0;
	uint8_t m_keyb_scancode = 0;

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<pic8259_device> m_intm;
	required_device<pic8259_device> m_ints;
	required_device<i8257_device> m_dma;
	required_device<mc6845_device> m_crtc;
	required_shared_ptr<uint16_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_kanji_rom;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	std::unique_ptr<u8[]> m_ig_ram;

	u8 ig_ram_r(offs_t offset);
	void ig_ram_w(offs_t offset, uint8_t data);
	void crtc_address_w(uint8_t data);
	void crtc_data_w(uint8_t data);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER( tc_tick_cb );
	emu_timer *m_timer_tc;

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
				// TODO: incorrect select (will print gibberish after the system bootup message)
				// system doesn't bother to clear kanji upper addresses, may opt-out thru a global register.
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
	map(0xb0000, 0xb7fff).ram().share("vram");
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
	map(0x28, 0x28).lr8(NAME([this]() {return m_keyb_scancode; }));
	map(0x2a, 0x2a).lr8(NAME([]() { return 0x02; }));
	// Jumper block?
//  map(0x40, 0x40)
//  map(0x42, 0x42)
//  map(0x44, 0x44)
	// b16ex2: jumps to $e0000 if bit 7 high, noisy on bit 4
	map(0x48, 0x48).lr8(NAME([] () { return 0; }));
	map(0x70, 0x73).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x74, 0x74).lw8(
		NAME([this] (u8 data) {
			floppy_image_device *floppy = m_floppy[0]->get_device();

			// motor on strobe?
			if (floppy != nullptr)
			{
				floppy->mon_w(1);
				floppy->mon_w(0);
			}
		})
	);
	map(0x78, 0x78).lrw8(
		NAME([this] () {
			return m_port78;
		}),
		NAME([this] (u8 data) {
			logerror("Port $78 %02x\n", data);
			// bit 0: TC strobe?
			// bit 2: FDC reset?
			m_port78 = data;
			if (BIT(data, 0))
			{
				m_fdc->tc_w(true);
				m_timer_tc->adjust(attotime::zero);
			}
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
	map(0x80, 0x89).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write)).umask16(0x00ff);
//  map(0x00a0, 0x00bf) DMA upper segments or video clut
//  map(0x8020, 0x8023) external FDC? Can branch at FDC irq
}

INPUT_CHANGED_MEMBER(b16_state::key_pressed)
{
	m_ints->ir0_w(!newval);
	m_keyb_scancode = (u8)param | (newval << 7);
}


static INPUT_PORTS_START( b16 )
	// question marks denotes unknown keys, asterisks for empty chars
	// KEY0
	// ?1234567
	// ? = left arrow (backspace?)

	// KEY1
	// 890-<backslash>***

	PORT_START("KEY2")
	// ?qwertyu
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, b16_state, key_pressed, 0x10 | 6)

	// KEY3
	// iop@[***

	PORT_START("KEY4")
	// asdfghjk
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, b16_state, key_pressed, 0x20)

	// KEY5
	// ?;:]****

	PORT_START("KEY6")
	// zxcvbnm,
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, b16_state, key_pressed, 0x30)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, b16_state, key_pressed, 0x31)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, b16_state, key_pressed, 0x32)
//    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, b16_state, key_pressed, 0x33)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, b16_state, key_pressed, 0x34)
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
	GFXDECODE_ENTRY( nullptr, 0x0000, charlayout, 0, 1 )
GFXDECODE_END

uint8_t b16_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void b16_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

static void b16_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	// TODO: at least PC-98 3.5" x 1.2MB format
}

void b16_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
//  fr.add(FLOPPY_PC98_FORMAT);
//  fr.add(FLOPPY_PC98FDI_FORMAT);
//  fr.add(FLOPPY_FDD_FORMAT);
//  fr.add(FLOPPY_DCP_FORMAT);
//  fr.add(FLOPPY_DIP_FORMAT);
//  fr.add(FLOPPY_NFD_FORMAT);
}

TIMER_CALLBACK_MEMBER( b16_state::tc_tick_cb )
{
//  logerror("tc off\n");
	m_fdc->tc_w(false);
}

void b16_state::machine_start()
{
	m_timer_tc = timer_alloc(FUNC(b16_state::tc_tick_cb), this);
}

void b16_state::machine_reset()
{
	floppy_image_device *floppy = m_floppy[0]->get_device();

	if (floppy != nullptr)
		floppy->set_rpm(300);
	m_fdc->set_rate(250000);

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

	PIC8259(config, m_intm);
	m_intm->out_int_callback().set_inputline(m_maincpu, 0);
	m_intm->in_sp_callback().set_constant(1);
	m_intm->read_slave_ack_callback().set(m_ints, FUNC(pic8259_device::acknowledge));

	PIC8259(config, m_ints, 0);
	m_ints->out_int_callback().set(m_intm, FUNC(pic8259_device::ir6_w));
	m_ints->in_sp_callback().set_constant(0);

	// clock unconfirmed, definitely want the ready line on
	// would stop at `SEARCH_ADDRESS_MARK_HEADER` otherwise
	UPD765A(config, m_fdc, XTAL(16'000'000) / 2, true, false);
	m_fdc->intrq_wr_callback().set(m_intm, FUNC(pic8259_device::ir1_w));
	m_fdc->drq_wr_callback().set([this] (int state) { logerror("drq %d\n", state);});
	FLOPPY_CONNECTOR(config, "fdc:0", b16_floppies, "525dd", b16_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", b16_floppies, "525dd", b16_state::floppy_formats).enable_sound(true);

	/* unknown variant, unknown clock, hand tuned to get ~60 fps */
	MC6845(config, m_crtc, XTAL(16'000'000)/6);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
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
