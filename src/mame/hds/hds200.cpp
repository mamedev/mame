// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Human Designed Systems HDS200

    ANSI/DEC-compatible terminal

    Hardware:
    - Z80A (Z8400APS)
    - Z80A DMA (Z8410APS)
    - 2x SCN2681A
    - SCN2674B
    - 2x MB81416-12 DRAM (and two empty sockets)
    - 2x TMM2016BP-90 (2k)
    - 1x TMM2016AP-10 (2k)
    - MK48Z02B-25 (2k)
    - XTAL 3.6864 MHz (next to DUARTs)
    - XTAL 8 MHz (CPU)
    - XTAL 22.680 MHz and 35.640 MHz (video)

    TODO:
    - Fix SCN2674/Z80DMA hookup
    - Missing keyboard keys
    - RS232 control lines

    Notes:
    - The PCB has a large unpopulated area. Possibly this is used for the
      200G variant.

***************************************************************************/

#include "emu.h"

#include "hds200_kbd.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "machine/z80dma.h"
#include "video/scn2674.h"

#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class hds200_state : public driver_device
{
public:
	hds200_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dma(*this, "dma"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_avdc(*this, "avdc"),
		m_duart(*this, "duart%u", 0U),
		m_chargen(*this, "chargen"),
		m_vram(*this, "vram"),
		m_rombank(*this, "rombank"),
		m_nmi_enabled(false)
	{ }

	void hds200(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80dma_device> m_dma;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scn2674_device> m_avdc;
	required_device_array<scn2681_device, 2> m_duart;
	required_region_ptr<uint8_t> m_chargen;
	required_shared_ptr<uint8_t> m_vram;
	required_memory_bank m_rombank;

	uint8_t attr_r(offs_t offset);
	void attrram_map(address_map &map) ATTR_COLD;
	uint8_t char_r(offs_t offset);
	void charram_map(address_map &map) ATTR_COLD;

	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	uint8_t dma_mreq_r(offs_t offset);
	void dma_mreq_w(offs_t offset, uint8_t data);

	void nmi_w(int state);

	void duart0_out_w(uint8_t data);
	void duart1_out_w(uint8_t data);

	bool m_reverse_video;
	bool m_nmi_enabled;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void hds200_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x5fff).bankr(m_rombank);
//  map(0x6000, 0x6001) // unknown device here
	map(0x6800, 0x6fff).ram().share("nvram");
	map(0x7000, 0x77ff).ram().share("vram");
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xffff).noprw(); // expansion ram
}

void hds200_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x20, 0x2f).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x40, 0x4f).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x60, 0x67).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint8_t hds200_state::attr_r(offs_t offset)
{
	offs_t addr = ((offset << 1) + 0) & 0x7ff;
	return m_vram[addr];
}

void hds200_state::attrram_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(hds200_state::attr_r));
}

uint8_t hds200_state::char_r(offs_t offset)
{
	offs_t addr = 0;

	// there should be a better way
	if (((offset << 1) & 0x7000) == 0x7000)
	{
		// start address for row table
		addr = (offset << 1) & 0x7ff;
	}
	else
	{
		// char data
		addr = ((offset << 1) + 1) & 0x7ff;
	}

	return m_vram[addr];
}

void hds200_state::charram_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(hds200_state::char_r));
}

SCN2674_DRAW_CHARACTER_MEMBER( hds200_state::draw_character )
{
	uint16_t data = m_chargen[charcode << 4 | linecount];
	const pen_t *const pen = m_palette->pens();

	// 7-------  invert
	// -6------  bold
	// --5-----  unknown
	// ---4----  underline
	// ----3---  blink
	// -----2--  conceal
	// ------1-  unknown
	// -------0  unknown

	if (ul && BIT(attrcode, 4))
		data = 0x1ff;

	if (blink && BIT(attrcode, 3))
		data = 0x000;

	// invert
	if (BIT(attrcode, 7))
		data = ~data;

	if (cursor)
		data = ~data;

	// foreground/background colors
	rgb_t fg = BIT(attrcode, 6) ? pen[2] : pen[1];
	rgb_t bg = pen[0];

	if (m_reverse_video)
		std::swap(fg, bg);

	// draw 9 pixels of the character
	for (int i = 0; i < 9; i++)
		bitmap.pix(y, x + i) = BIT(data, 8 - i) ? fg : bg;
}

static const gfx_layout char_layout =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t hds200_state::dma_mreq_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void hds200_state::dma_mreq_w(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

void hds200_state::nmi_w(int state)
{
	if (state && m_nmi_enabled)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void hds200_state::duart0_out_w(uint8_t data)
{
	// 765-----  unknown
	// ---4----  80/132 column switch (1 = 80)
	// ----3---  reverse video
	// -----210  unknown

	//logerror("duart0_out_w: %02x\n", data);

	m_reverse_video = bool(BIT(data, 3));
}

void hds200_state::duart1_out_w(uint8_t data)
{
	// 765-----  unknown
	// ---4----  nmi enable
	// ----3---  unknown
	// -----2--  rombank
	// ------10  unknown

	//logerror("duart1_out_w: %02x\n", data);

	m_nmi_enabled = bool(BIT(data, 4));
	m_rombank->set_entry(!BIT(data, 2));
}

void hds200_state::machine_start()
{
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base() + 0x4000, 0x2000);

	// register for save states
	save_item(NAME(m_reverse_video));
	save_item(NAME(m_nmi_enabled));
}

void hds200_state::machine_reset()
{
	m_reverse_video = false;
	m_nmi_enabled = false;

	m_rombank->set_entry(0);

	// no duart irq active
	m_duart[0]->ip4_w(1);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void hds200_state::hds200(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &hds200_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hds200_state::io_map);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	input_merger_device &z80_irq(INPUT_MERGER_ANY_HIGH(config, "z80_irq"));
	z80_irq.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	Z80DMA(config, m_dma, 8_MHz_XTAL / 2); // divider not verified
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->in_mreq_callback().set(FUNC(hds200_state::dma_mreq_r));
	m_dma->out_mreq_callback().set(FUNC(hds200_state::dma_mreq_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::amber());
	m_screen->set_raw(22.680_MHz_XTAL, 1008, 0, 720, 375, 0, 350); // 80-column mode
	m_screen->set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	SCN2674(config, m_avdc, 22.680_MHz_XTAL / 9);
	m_avdc->intr_callback().set("z80_irq", FUNC(input_merger_device::in_w<0>));
	m_avdc->mbc_callback().set(FUNC(hds200_state::nmi_w));
	m_avdc->set_addrmap(0, &hds200_state::charram_map);
	m_avdc->set_addrmap(1, &hds200_state::attrram_map);
	m_avdc->set_character_width(9);
	m_avdc->set_screen("screen");
	m_avdc->set_display_callback(FUNC(hds200_state::draw_character));

	input_merger_device &duart_irq(INPUT_MERGER_ANY_HIGH(config, "duart_irq"));
	duart_irq.output_handler().set("z80_irq", FUNC(input_merger_device::in_w<1>));
	duart_irq.output_handler().append(m_duart[0], FUNC(scn2681_device::ip4_w)).invert();

	SCN2681(config, m_duart[0], 3.6864_MHz_XTAL);
	m_duart[0]->irq_cb().set("duart_irq", FUNC(input_merger_device::in_w<0>));
	m_duart[0]->outport_cb().set(FUNC(hds200_state::duart0_out_w));
	m_duart[0]->a_tx_cb().set("line1", FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set("line2", FUNC(rs232_port_device::write_txd));

	SCN2681(config, m_duart[1], 3.6864_MHz_XTAL);
	m_duart[1]->irq_cb().set("duart_irq", FUNC(input_merger_device::in_w<1>));
	m_duart[1]->outport_cb().set(FUNC(hds200_state::duart1_out_w));
	m_duart[1]->a_tx_cb().set("line3", FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set("kbd", FUNC(hds200_kbd_hle_device::rx_w));

	rs232_port_device &rs232_line1(RS232_PORT(config, "line1", default_rs232_devices, nullptr));
	rs232_line1.rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	rs232_line1.cts_handler().set(m_duart[0], FUNC(scn2681_device::ip0_w));

	rs232_port_device &rs232_line2(RS232_PORT(config, "line2", default_rs232_devices, nullptr));
	rs232_line2.rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));
	rs232_line2.cts_handler().set(m_duart[0], FUNC(scn2681_device::ip1_w));

	rs232_port_device &rs232_line3(RS232_PORT(config, "line3", default_rs232_devices, nullptr));
	rs232_line3.rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	rs232_line3.cts_handler().set(m_duart[1], FUNC(scn2681_device::ip0_w));

	hds200_kbd_hle_device &kbd(HDS200_KBD_HLE(config, "kbd"));
	kbd.tx_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( hds200 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("u78.bin", 0x0000, 0x2000, CRC(518cfeb7) SHA1(3c214dede545a2a991fdd77311b3474b01b2123f))
	ROM_LOAD("u79.bin", 0x2000, 0x2000, CRC(3a765c8a) SHA1(8ffb5fb07b086ac725f22c2643ecd2e061130b57))
	ROM_LOAD("u80.bin", 0x4000, 0x2000, CRC(f72dfeeb) SHA1(7e09b8f0df8384f6b5c4d29cd59fa31f743de8b8))
	ROM_LOAD("u81.bin", 0x6000, 0x2000, CRC(b3f430be) SHA1(dd5503de46c7f00f2e376104dff13224026f5870))

	ROM_REGION(0x2000, "chargen", ROMREGION_INVERT)
	ROM_LOAD("u56.bin", 0x0000, 0x2000, CRC(cd268bff) SHA1(42f2aa3f51ae53e5cbcb57f974e99b24bca5f56f))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY         FULLNAME            FLAGS
COMP( 1985, hds200, 0,       0,      hds200,  0,     hds200_state, empty_init, "Human Designed Systems", "HDS200", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
