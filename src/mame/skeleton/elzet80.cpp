// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

ELZET/K
ELZET/P
BCS

All documentation is in German.

The ELZET/P is a luggable CP/M computer that from the outside looks very
much like a Kaypro II. The floppy drives could be one under the other like
the Kaypro, or could be vertically-orientated side-by-side. The drives are
80 track quad-density with 800KB capacity.
Behind the drives is a cage that can hold up to 8 slots that plug into a
small motherboard.
CARDS: (some are optional)
- Floppy Disk Controller: choice of FDC 2: (basic) or FDC3 (also has 128K RAM)
    (main chips are PIO, DMA, MB8877A/SAB1793)
- Dynamic RAM: choice of 64k or 256k
- Centronics (contains PIO, CTC)
- EIC (contains PIO, DART, Z80B)
- Experimenter board (contains PIO, CTC)
- Video (contains MC6845, 2k vram, 2k attr-ram, chargen roms, 15MHz xtal)
- CPU (contains Z80, DART, PIO, 4MHz xtal, 4/8K ROM, 2K RAM)

The keyboard plugs into the front by using a stereo audio plug, like you
have on a modern computer's line-out jack. It's using a serial interface.


The ELZET/K is a similar computer but is a slightly smaller form factor.

The BCS is a box that can be used to convert over 300 floppy-disk formats.

***************************************************************************


TODO: FDC card, Centronics card, EIC card, Experimenter board, video RAM visibility, keyboard
Status: Video works, otherwise skeleton


***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"


namespace {

class elzet80_state : public driver_device
{
public:
	elzet80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "pio")
		, m_dart(*this, "dart")
		, m_pctc(*this, "pctc")
		, m_ppio(*this, "ppio")
		, m_crtc(*this, "crtc")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_p_colorram(*this, "colorram")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_fdma(*this, "fdma")
		, m_fdc(*this, "fdc")
		, m_fpio(*this, "fpio")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		//, m_io_keyboard(*this, "LINE%d", 0U)
	{ }

	void elzet80(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(update_row);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	floppy_image_device *m_floppy = nullptr;
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<z80dart_device> m_dart;
	required_device<z80ctc_device> m_pctc;
	required_device<z80pio_device> m_ppio;
	required_device<mc6845_device> m_crtc;
	required_region_ptr<uint8_t> m_p_chargen;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_shared_ptr<uint8_t> m_p_colorram;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<z80dma_device> m_fdma;
	required_device<fd1793_device> m_fdc;
	required_device<z80pio_device> m_fpio;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	//required_ioport_array<8> m_io_keyboard;
};

MC6845_UPDATE_ROW( elzet80_state::update_row )
{
	rgb_t const *const pens = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_p_videoram[mem];

		/* get pattern of pixels for that character scanline */
		uint16_t gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0x1ff : 0);

		/* Display a scanline of a character (9 pixels) */
		*p++ = pens[BIT(gfx, 8)];
		*p++ = pens[BIT(gfx, 7)];
		*p++ = pens[BIT(gfx, 6)];
		*p++ = pens[BIT(gfx, 5)];
		*p++ = pens[BIT(gfx, 4)];
		*p++ = pens[BIT(gfx, 3)];
		*p++ = pens[BIT(gfx, 2)];
		*p++ = pens[BIT(gfx, 1)];
		*p++ = pens[BIT(gfx, 0)];
	}
}

void elzet80_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom(); // CPU card ROM
	map(0x2000, 0xffff).ram(); // 64K RAM card
	map(0xe000, 0xe7ff).ram().share("colorram"); // video attribute RAM
	map(0xe800, 0xefff).ram().share("videoram"); // video character RAM
}

void elzet80_state::io_map(address_map &map)
{
	map(0x00, 0x03).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x04, 0x07).rw(m_dart, FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w));

	// printer card
	map(0x20, 0x23).rw(m_pctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x24, 0x27).rw(m_ppio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));

	// video card
	map(0x28, 0x28).nopw(); // toggle video memory access
	map(0x29, 0x29).noprw(); // video card (unused)
	map(0x2a, 0x2a).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x2b, 0x2b).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	// fdc card
	map(0x60, 0x63).rw(m_fdma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x68, 0x6b).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x6c, 0x6f).rw(m_fpio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));

	map.global_mask(0xff);
	map.unmap_value_high();
}

static INPUT_PORTS_START( elzet80 )
INPUT_PORTS_END

static const gfx_layout elzet_charlayout =
{
	8, 12,                  /* 8 x 12 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_elzet )
	GFXDECODE_ENTRY( "chargen", 0x0000, elzet_charlayout, 0, 1 )
GFXDECODE_END

void elzet80_state::machine_start()
{
}

void elzet80_state::machine_reset()
{
	m_floppy = nullptr;
}

static void elzet80_floppies(device_slot_interface &device)
{
	device.option_add("fdd", FLOPPY_525_QD);
}


void elzet80_state::elzet80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &elzet80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &elzet80_state::io_map);

	Z80PIO(config, m_pio, 4_MHz_XTAL);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80DART(config, m_dart, 6144000); // FET-oscillator
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	rs232a.rxc_handler().set(m_dart, FUNC(z80dart_device::rxca_w));
	rs232a.cts_handler().set(m_dart, FUNC(z80dart_device::ctsa_w));
	rs232a.dcd_handler().set(m_dart, FUNC(z80dart_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_dart, FUNC(z80dart_device::rxb_w));
	rs232b.dcd_handler().set(m_dart, FUNC(z80dart_device::dcdb_w));

	Z80CTC(config, m_pctc, 4_MHz_XTAL);
	Z80PIO(config, m_ppio, 4_MHz_XTAL);

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(15_MHz_XTAL, 512, 0, 320, 326, 0, 240);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 15_MHz_XTAL);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(elzet80_state::update_row));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_elzet);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// floppy
	Z80DMA(config, m_fdma, 4_MHz_XTAL);
	m_fdma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	FD1793(config, m_fdc, 4_MHz_XTAL);
	Z80PIO(config, m_fpio, 4_MHz_XTAL);
	m_fpio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	//m_fpio->out_pa_callback().set(DRIVE SELECT);
	FLOPPY_CONNECTOR(config, "fdc:0", elzet80_floppies, "fdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", elzet80_floppies, "fdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START(elzet80k)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "elzet80-k_cpu.bin",                  0x0000, 0x1000, CRC(e5300137) SHA1(5a9fcb52756a2e9008d53b734b874d33b069efb5) )

	ROM_REGION(0x2000, "chargen", 0) // same roms as P?
	ROM_LOAD( "elzet80-p_video80_ic1_prom3.bin",    0x0000, 0x1000, CRC(70008d71) SHA1(048efed186861050f0e6b47fec57149319de22b6) )
	ROM_LOAD( "elzet80-p_video80_ic2_prom4.bin",    0x1000, 0x1000, CRC(8db78b54) SHA1(7d0ce2811ee1f6c179295411fd8c3c2e67ea2842) )

	ROM_REGION(0x1000, "kbd", 0)
	ROM_LOAD( "elzet80-k_tastatur_d8748.bin",       0x0000, 0x0400, CRC(78bf6f1a) SHA1(95919363e73779899cbd7c143d10c245c35ca789) )

	ROM_REGION(0x1000, "bcs", 0)
	ROM_LOAD( "elzet80-bcs_cpuiec.bin",             0x0000, 0x1000, CRC(724fe45a) SHA1(b4b01c9bf11b35c48b0a4839d7530a18259b1ae9) )
ROM_END

ROM_START(elzet80p)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "elzet80-p_cpu.bin",                  0x0000, 0x1000, CRC(8b876e12) SHA1(0fb4dfe267a3fbc033fbce430d13fb22a4ad16ed) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "elzet80-p_video80_ic1_prom3.bin",    0x0000, 0x1000, CRC(70008d71) SHA1(048efed186861050f0e6b47fec57149319de22b6) )
	ROM_LOAD( "elzet80-p_video80_ic2_prom4.bin",    0x1000, 0x1000, CRC(8db78b54) SHA1(7d0ce2811ee1f6c179295411fd8c3c2e67ea2842) )

	ROM_REGION(0x1000, "kbd", 0)
	ROM_LOAD( "elzet80-p_tastatur_jk_ti2732.bin",  0x0000, 0x1000, CRC(88be9b29) SHA1(b83603dadce9d7e7367d1782ee78189156fe9a60) )

	ROM_REGION(0x1000, "bcs", 0)
	ROM_LOAD( "elzet80-bcs_cpuiec.bin",             0x0000, 0x1000, CRC(724fe45a) SHA1(b4b01c9bf11b35c48b0a4839d7530a18259b1ae9) )
ROM_END

} // Anonymous namespace

//    YEAR  NAME        PARENT    COMPAT    MACHINE     INPUT        CLASS           INIT             COMPANY          FULLNAME               FLAGS
COMP( 1982, elzet80k,   elzet80p, 0,        elzet80,   elzet80,    elzet80_state, empty_init,     "Giesler & Danne GmbH & Co. KG",  "Elzet/K 80",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 1982, elzet80p,   0,        0,        elzet80,   elzet80,    elzet80_state, empty_init,     "Giesler & Danne GmbH & Co. KG",  "Elzet/P 80",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
