// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Digilog 320

    Protocol analyzer

    Hardware:

    PCB 24-0710-02B:
    - Speaker
    - XTAL 1.344 MHz
    - R80186-10
    - XTAL 20 MHz
    - DS1241
    - HM62256LP-15 x4
    - AT27C512R x6

    PCB 24-0706-02C:
    - MB8877A
    - FDC9229BT
    - XTAL 16 MHz
    - P8251A
    - XTAL 3.6864 MHz
    - SCN2681AC1N40
    - HD46505SP-2
    - HM6264ALP-12 x2
    - AM27128DC "24-1140-00 A"
    - XTAL 5.659200 MHz
    - PAL labeled "20-1110-1 A"

    PCB 24-0709-02D:
    - XTAL 16 MHz
    - Z0840008PSC Z80 CPU
    - HM62256LP-12
    - Z8530H-8PC x2
    - AT27C256 "24-0196-01 C"
    - AM9519A-1PC
    - XTAL 2.688 MHz
    - XTAL 3.6864 MHz

    TODO:
    - Finish floppy hookup
    - SCC interrupts
    - Unknown DUART inputs/outputs
    - RS232
    - RTC
    - Artwork with LEDs

    Notes:
    - Everything here guessed, no manuals available

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/z80/z80.h"
#include "machine/am9519.h"
#include "machine/digilog_kbd.h"
#include "machine/i8251.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "machine/wd_fdc.h"
#include "machine/z80scc.h"
#include "video/mc6845.h"
#include "imagedev/floppy.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class digilog320_state : public driver_device
{
public:
	digilog320_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_uic(*this, "uic"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_duart(*this, "duart"),
		m_scc(*this, "scc%u", 0U),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:0"),
		m_vram(*this, "vram"),
		m_chargen(*this, "chargen")
	{ }

	void digilog320(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<z80_device> m_subcpu;
	required_device<am9519_device> m_uic;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<scn2681_device> m_duart;
	required_device_array<scc8530_device, 2> m_scc;
	required_device<mb8877_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_shared_ptr<uint16_t> m_vram;
	required_region_ptr<uint8_t> m_chargen;

	void main_mem_map(address_map &map);
	void main_io_map(address_map &map);
	void sub_mem_map(address_map &map);
	void sub_io_map(address_map &map);

	MC6845_UPDATE_ROW(update_row);

	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	void fdc_ctrl_w(uint8_t data);

	uint8_t subcpu_to_maincpu_r();
	void maincpu_to_subcpu_w(uint8_t data);
	uint8_t maincpu_to_subcpu_r();
	void subcpu_to_maincpu_w(uint8_t data);
	uint8_t maincpu_status_r();

	uint8_t m_subcpu_to_maincpu;
	uint8_t m_maincpu_to_subcpu;
	uint8_t m_maincpu_status;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void digilog320_state::main_mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0x80000, 0x83fff).ram().share("vram");
	map(0x90000, 0x91fff).ram().share("nvram");
	map(0xa0000, 0xfffff).rom().region("maincpu", 0);
}

void digilog320_state::main_io_map(address_map &map)
{
	map(0x000, 0x003).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x080, 0x09f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x100, 0x107).rw(FUNC(digilog320_state::fdc_r), FUNC(digilog320_state::fdc_w)).umask16(0x00ff);
	map(0x180, 0x180).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x182, 0x182).w(m_crtc, FUNC(mc6845_device::register_w));
	map(0x200, 0x200).w(FUNC(digilog320_state::fdc_ctrl_w));
	map(0x280, 0x280).rw(FUNC(digilog320_state::subcpu_to_maincpu_r), FUNC(digilog320_state::maincpu_to_subcpu_w));
}

void digilog320_state::sub_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("subcpu", 0);
	map(0x8000, 0xffff).ram();
}

void digilog320_state::sub_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_uic, FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x01, 0x01).rw(m_uic, FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
	map(0x04, 0x07).rw(m_scc[0], FUNC(z80scc_device::dc_ab_r), FUNC(z80scc_device::dc_ab_w));
	map(0x08, 0x0b).rw(m_scc[1], FUNC(z80scc_device::dc_ab_r), FUNC(z80scc_device::dc_ab_w));
	map(0x10, 0x10).rw(FUNC(digilog320_state::maincpu_to_subcpu_r), FUNC(digilog320_state::subcpu_to_maincpu_w));
	map(0x11, 0x11).r(FUNC(digilog320_state::maincpu_status_r));
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

MC6845_UPDATE_ROW( digilog320_state::update_row )
{
	pen_t const *const pen = m_palette->pens();

	for (int x = 0; x < x_count; x++)
	{
		// attributes
		// 76------  unknown
		// --5-----  half intensity
		// ---4----  unknown
		// ----3---  reverse
		// -----2--  unknown
		// ------10  chargen high bits

		uint16_t const data = (m_vram[ma + x]);
		uint8_t gfx = m_chargen[((data & 0x3ff) << 4) | ra];
		uint8_t attr = data >> 8;

		if (x == cursor_x)
			gfx ^= 0xff;

		if (BIT(attr, 3))
			gfx ^= 0xff;

		// foreground/background colors
		rgb_t fg = BIT(attr, 5) ? pen[1] : pen[2];
		rgb_t bg = pen[0];

		// draw 8 pixels of the character
		for (int i = 0; i < 8; i++)
			bitmap.pix(y, x * 8 + i) = BIT(gfx, i) ? fg : bg;
	}
}

static const gfx_layout char_layout =
{
	8, 12,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  FLOPPY
//**************************************************************************

// either cpu too fast or fdc too slow, assume some waitstates to pass check
uint8_t digilog320_state::fdc_r(offs_t offset)
{
	m_maincpu->adjust_icount(-2);
	return m_fdc->read(offset);
}

void digilog320_state::fdc_w(offs_t offset, uint8_t data)
{
	m_maincpu->adjust_icount(-2);
	m_fdc->write(offset, data);
}

void digilog320_state::fdc_ctrl_w(uint8_t data)
{
	// 7654----  unknown (not used?)
	// ----3---  unknown (motor on?)
	// -----2--  side select
	// ------10  unknown (drive select?)

	logerror("fdc_ctrl_w: %02x\n", data);

	floppy_image_device *floppy = m_floppy->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->ss_w(BIT(data, 2));

		// motor always on for now
		floppy->mon_w(0);
	}

	// set to mfm
	m_fdc->dden_w(0);
}

static void digilog320_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t digilog320_state::subcpu_to_maincpu_r()
{
	logerror("subcpu_to_maincpu_r: %02x\n", m_subcpu_to_maincpu);

	m_maincpu_status &= ~0x40;
	m_duart->ip3_w(0);

	return m_subcpu_to_maincpu;
}

void digilog320_state::maincpu_to_subcpu_w(uint8_t data)
{
	logerror("maincpu_to_subcpu_w: %02x\n", data);

	m_maincpu_to_subcpu = data;

	m_duart->ip0_w(1);
	m_uic->ireq4_w(0);
}

uint8_t digilog320_state::maincpu_to_subcpu_r()
{
	logerror("maincpu_to_subcpu_r: %02x\n", m_maincpu_to_subcpu);

	m_duart->ip0_w(0);
	m_uic->ireq4_w(1);

	return m_maincpu_to_subcpu;
}

void digilog320_state::subcpu_to_maincpu_w(uint8_t data)
{
	logerror("subcpu_to_maincpu_w: %02x\n", data);

	m_subcpu_to_maincpu = data;

	m_maincpu_status |= 0x40;
	m_duart->ip3_w(1);
}

uint8_t digilog320_state::maincpu_status_r()
{
	// 7-------  unknown
	// -6------  maincpu ready to receive data (0 = ready)
	// --543210  unknown

	return m_maincpu_status;
}

void digilog320_state::machine_start()
{
	// register for save states
	save_item(NAME(m_subcpu_to_maincpu));
	save_item(NAME(m_maincpu_to_subcpu));
	save_item(NAME(m_maincpu_status));
}

void digilog320_state::machine_reset()
{
	m_maincpu_status = 0x00;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void digilog320_state::digilog320(machine_config &config)
{
	I80186(config, m_maincpu, 20_MHz_XTAL); // R80186-10
	m_maincpu->set_addrmap(AS_PROGRAM, &digilog320_state::main_mem_map);
	m_maincpu->set_addrmap(AS_IO, &digilog320_state::main_io_map);

	Z80(config, m_subcpu, 16_MHz_XTAL / 2); // Z0840008PSC
	m_subcpu->set_addrmap(AS_PROGRAM, &digilog320_state::sub_mem_map);
	m_subcpu->set_addrmap(AS_IO, &digilog320_state::sub_io_map);
	m_subcpu->set_irq_acknowledge_callback(m_uic, FUNC(am9519_device::iack_cb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	AM9519(config, m_uic, 0);
	m_uic->out_int_callback().set_inputline(m_subcpu, INPUT_LINE_IRQ0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(5.6592_MHz_XTAL, 320, 0, 256, 262, 0, 192);
	screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	MC6845(config, m_crtc, 5.6592_MHz_XTAL / 8); // HD46505SP-2
	m_crtc->set_char_width(8);
	m_crtc->set_show_border_area(false);
	m_crtc->set_update_row_callback(FUNC(digilog320_state::update_row));

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));
	m_duart->outport_cb().set("usart", FUNC(i8251_device::write_txc)).bit(3);
	m_duart->outport_cb().append("usart", FUNC(i8251_device::write_rxc)).bit(3);

	I8251(config, "usart", 0);

	SCC8530N(config, m_scc[0], 3.6864_MHz_XTAL);

	SCC8530N(config, m_scc[1], 3.6864_MHz_XTAL);

	MB8877(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::int3_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq1_w));
	FLOPPY_CONNECTOR(config, "fdc:0", digilog320_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);

	SOFTWARE_LIST(config, "floppy_list").set_original("digilog320");

	digilog_kbd_device &kbd(DIGILOG_KBD(config, "kbd"));
	kbd.tx_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( digilog320 )
	ROM_REGION(0x60000, "maincpu", 0)
	ROM_LOAD16_BYTE("24-0197-01_e.u33", 0x00000, 0x10000, CRC(d1be8c1f) SHA1(5cef9d54e341da4cfa6ca50939c889c6d502240a))
	ROM_LOAD16_BYTE("24-0197-02_e.u24", 0x00001, 0x10000, CRC(1bbe293d) SHA1(69d22456e6b20fc3ce1c5208af169310e414e955))
	ROM_LOAD16_BYTE("24-0197-03_e.u34", 0x20000, 0x10000, CRC(d492b6fd) SHA1(474220128e63133eeb8591401edb3ad65d71e027))
	ROM_LOAD16_BYTE("24-0197-04_e.u25", 0x20001, 0x10000, CRC(a5667746) SHA1(ebecd69ebf394365e9bedfcbe98d71ce4ec7feb1))
	ROM_LOAD16_BYTE("24-0197-05_e.u35", 0x40000, 0x10000, CRC(17273a19) SHA1(0c28b304e1447d4afd821b3b1a4fb6029bf2a24d))
	ROM_LOAD16_BYTE("24-0197-06_e.u26", 0x40001, 0x10000, CRC(cb29ca28) SHA1(8827de70c006250ba2989c4dc57e402ed222f948))

	ROM_REGION(0x8000, "subcpu", 0)
	ROM_LOAD("24-0196-01_c.u12", 0x0000, 0x8000, CRC(20291d21) SHA1(1151414531040af59f4f692aab31bfc04e7fc56c))

	ROM_REGION(0x4000, "chargen", 0)
	ROM_LOAD("24-1140-00_a.bin", 0x0000, 0x4000, CRC(7a4d0b82) SHA1(15952655cef77918a76c0c268b749be34b28634b))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT  CLASS             INIT        COMPANY    FULLNAME  FLAGS
COMP( 1988, digilog320, 0,          0,      digilog320, 0,     digilog320_state, empty_init, "Digilog", "320",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
