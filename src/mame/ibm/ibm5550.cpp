// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

IBM 5550

http://ibm5550.na.coocan.jp/architec-e.html

TODO:
- No available documentation
\- it's barely known that it uses a specific "JDA" video adapter, and that it uses a specific
   incompatible bus slot;

POST codes (to port $a1):
- 0xe2: ROM check
[- 0xfc:] goes high if first $a1 check = 1 at PC=0xfc0d8
- 0xfa: PIC check #1
- 0xf6: undefined boundary check at $4c000-$ec000
- 0xe4: RAM check
- 0xe8: PIC check #2
- 0xf0: PIT check
- 0xf8: $3d0-$3d1 check (JDA adapter?)
- 0xf2: VRAM check

**************************************************************************************************/

#include "emu.h"

//#include "bus/isa/isa.h"
//#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i86/i86.h"
#include "machine/am9517a.h"
#include "machine/i8251.h"
//#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"

class ibm5550_state : public driver_device
{
public:
	ibm5550_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit")
		, m_pic(*this, "pic")
		, m_dma(*this, "dma")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
		, m_gfxram(*this, "gfxram")
		, m_palette(*this, "palette")
	{ }

	void ibm5550(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;

	required_device<i8086_cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<pic8259_device> m_pic;
	required_device<am9517a_device> m_dma;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_vram;
	required_shared_ptr<uint16_t> m_gfxram;
	required_device<palette_device> m_palette;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 m_a0_unk = 0;
};

/*
static const gfx_layout kanji_layout =
{
    32, 32,
    RGN_FRAC(1,1),
    1,
    { 0 },
    { STEP16(0,1), STEP16(16, 16) },
    { STEP16(0,16), STEP16(16*16, 16) },
    32*32
};
*/

uint32_t ibm5550_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u32 pitch = 82;
	bitmap.fill(m_palette->black_pen(), cliprect);

	for(int y = 0; y < 24; y++)
	{
		for(int x = 0; x < pitch; x++)
		{
			const u16 tile = m_vram[x + y * pitch] & 0x1f;

			for (int yi = 0; yi < 32; yi ++)
			{
				u16 color = m_gfxram[tile * 64 + yi];
				for (int xi = 0; xi < 16; xi ++)
				{
					const int res_x = x * 32 + xi;
					const int res_y = y * 32 + yi;
					if(cliprect.contains(res_x, res_y))
						bitmap.pix(res_y, res_x) = m_palette->pen(BIT(color, 15 - xi) ? 2 : 0);
				}
				color = m_gfxram[tile * 64 + yi + 32];
				for (int xi = 0; xi < 16; xi ++)
				{
					const int res_x = x * 32 + xi + 16;
					const int res_y = y * 32 + yi;
					if(cliprect.contains(res_x, res_y))
						bitmap.pix(res_y, res_x) = m_palette->pen(BIT(color, 15 - xi) ? 2 : 0);
				}
			}
		}
	}

	return 0;
}

void ibm5550_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x3ffff).ram(); // 256 or 512KB
	// POST test $f6 expects that all the blocks between $4c000-$ec000 returns 0xff
	map(0xd8000, 0xd8fff).ram().share("gfxram");
	map(0xe0000, 0xe0fff).ram().share("vram");
	map(0xfc000, 0xfffff).rom().region("ipl", 0);
}

void ibm5550_state::main_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x00?0, 0x00?7).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));

	// tested later, with bit 6 irq from PIC
//  map(0x0040, 0x0047).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));

//  map(0x0060, 0x0060) / map(0x0064, 0x0064) standard XT keyboard

	// bit 0 on will punt before testing for $20-$21,
	// but will be required on after $4c-$ec RAM holes above
	// ... RAM protection?
	map(0x00a0, 0x00a0).lrw8(
		NAME([this] (offs_t offset) { return m_a0_unk; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("$a0 %02x\n", data);
			m_a0_unk = BIT(data, 6);
			if (data == 0xc0)
			{
				// attached to bus error?
				m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			}
		})
	);
//  map(0x00a1, 0x00a1) LED write?
	map(0x00a1, 0x00a1).lr8(
		NAME([] (offs_t offset) {
			// read thru NMI trap above, bit 3-0 must be low
			return 0;
		})
	);
	map(0x00a2, 0x00a2).lw8(
		NAME([this] (offs_t offset, u8 data) {
			logerror("$a2 %02x\n", data);
			m_a0_unk = 0;
		})
	);

	// TODO: not right, definitely an address/data of some sort (extended regs?)
	//map(0x3d0, 0x3d0).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	//map(0x3d1, 0x3d1).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	// TODO: vblank
	//map(0x3da, 0x3da).lr8(
}

static INPUT_PORTS_START( ibm5550 )
INPUT_PORTS_END

void ibm5550_state::machine_reset()
{
	m_a0_unk = 0;
}

void ibm5550_state::ibm5550(machine_config &config)
{
	I8086(config, m_maincpu, XTAL(16'000'000) / 2); // driven by a TD308C/TD1100C @ 16MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &ibm5550_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &ibm5550_state::main_io);
	m_maincpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

	// i8087 socket

	// iDB284A
	// MN2364SPM
	// MN2364SPN
	// IBM6343868 / MN50015SPE
	// IBM6343871 / MN50015SPH
	// IBM6343866 / MN50015SPC
	// IBM6343867 / MN50015SPD
	// D8039LC
	// iP8237A-5
	AM9517A(config, m_dma, XTAL(5'000'000));

	// D765AC + SED9420C
	// HM6116LP-2
	// D7261AD
	// D8259AC-2
	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	// D8253C-2
	PIT8253(config, m_pit, 0);

	// Parallel port, 36 pins
	// keyboard connector
	// TEST switch + LED

	// JDA display board

	// CX0-043C @ 40 MHz
	// HD46505SP-2, unknown pixel clock
	HD6845S(config, m_crtc, XTAL(40'000'000) / 10);
	m_crtc->set_screen(m_screen);
//  m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(16);

	// IBM6343870 / MN50015SPG
	// IBM6343869 / MN50007SPF
	// x2 HM6116P-2

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(ibm5550_state::screen_update));
	m_screen->set_size(1280, 1024);
	m_screen->set_visarea(0, 1023, 0, 767);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

//  ibm5160_mb_device &mb(IBM5160_MOTHERBOARD(config, "mb"));
//  mb.set_cputag(m_maincpu);
//  mb.int_callback().set_inputline(m_maincpu, 0);
//  mb.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
//  mb.kbdclk_callback().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
//  mb.kbddata_callback().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));
//  mb.set_input_default(DEVICE_INPUT_DEFAULTS_NAME(pccga));

	// FIXME: determine ISA bus clock
//  ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, "cga", false);
//  ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, "fdc_xt", false);
//  ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, "lpt", false);
//  ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, "com", false);

	/* keyboard */
//  pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83));
//  kbd.out_clock_cb().set("mb", FUNC(ibm5160_mb_device::keyboard_clock_w));
//  kbd.out_data_cb().set("mb", FUNC(ibm5160_mb_device::keyboard_data_w));

	/* internal ram */
//  RAM(config, RAM_TAG).set_default_size("256K").set_extra_options("512K");
}

ROM_START( ibm5550 )
	ROM_REGION16_LE(0x4000, "ipl", 0)
	ROM_LOAD("ipl5550.rom", 0x0000, 0x4000, CRC(40cf34c9) SHA1(d41f77fdfa787b0e97ed311e1c084b8699a5b197))

	// integrated for later models
	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)
	ROM_LOAD("chargen.rom", 0x00000, 0x20000, NO_DUMP )
ROM_END

COMP( 1983, ibm5550, 0, 0, ibm5550, ibm5550, ibm5550_state, empty_init, "International Business Machines", "Multistation 5550", MACHINE_IS_SKELETON )
