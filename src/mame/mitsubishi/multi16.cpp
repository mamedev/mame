// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Dirk Best
/***************************************************************************

    Mitsubishi MULTI-16 series

    MULTI-16 (1982, 1983 for S models)
    - 8088, 4 MHz
    - 8041 (MCU), 8253 (PIT), 8259A (PIC), MB8866 (FDC)
    - MP-1601 (128 KB RAM, 32 KB VRAM, monochrome, 1x 5.25")
    - MP-1602 (192 KB RAM, 32 KB VRAM, monochrome, 2x 5.25")
    - MP-1605 (256 KB RAM, 96 KB VRAM, 8 colors, 2x 5.25")
    - MP-1601S (8087, 128 KB RAM, 32 KB VRAM, monochrome, 1x 5.25")
    - MP-1602S (8087, 192 KB RAM, 32 KB VRAM, monochrome, 2x 5.25")
    - MP-1605S (8087, 256 KB RAM, 96 KB VRAM, 8 colors, 2x 5.25")
    - MP-1622 (8087, 192 KB RAM, 32 KB VRAM, monochrome, 2x 8")
    - MP-1625 (8087, 256 KB RAM, 96 KB VRAM, 8 colors, 2x 8")

    MULTI-16 II (1984)
    - 8086, 8 MHz
    - MP-1642 (256 KB RAM, 64 KB VRAM, monochrome, 2x 5.25")
    - MP-1645 (256 KB RAM, 192 KB VRAM, 8 colors, 2x 5.25")

    MULTI-16 IV (1986)
    - 80286, 8 MHz
    - MP-1652-A20 (512 KB RAM, 128 KB VRAM, monochrome, 2x 5.25")
    - MP-1652-A22 (512 KB RAM, 128 KB VRAM, monochrome, 2x 5.25", 20 MB HDD)
    - MP-1655-A20 (512 KB RAM, 384 KB VRAM, 8 colors, 2x 5.25")
    - MP-1655-A22 (512 KB RAM, 384 KB VRAM, 8 colors, 2x 5.25", 20 MB HDD)

    The IPL dump we currently have seems to be from the MP-1645.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "imagedev/floppy.h"
#include "emupal.h"
#include "screen.h"


namespace {

class multi16_state : public driver_device
{
public:
	multi16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic"),
		m_pit(*this, "pit"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0U),
		m_vram(*this, "vram")
	{ }

	void multi16(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<hd6845s_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<mb8866_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_shared_ptr<uint16_t> m_vram;

	MC6845_UPDATE_ROW(crtc_update_row);

	void multi16_io(address_map &map) ATTR_COLD;
	void multi16_map(address_map &map) ATTR_COLD;
};

MC6845_UPDATE_ROW(multi16_state::crtc_update_row)
{
	uint8_t const *const vram = reinterpret_cast<uint8_t *>(m_vram.target());

	for (int i = 0; i < x_count; i++)
	{
		offs_t offs = ((ra & 1) << 14) + (ma << 3) + ((ra >> 1) * 80) + i;

		// vram is from c0000 to effff, the ipl uses the last half
		uint8_t data_r = vram[3 * 0x8000 + offs];
		uint8_t data_g = vram[4 * 0x8000 + offs];
		uint8_t data_b = vram[5 * 0x8000 + offs];

		// draw 8 pixels of the character
		for (int x = 0; x < 8; x++)
		{
			int color = 0;
			color |= BIT(data_r, 7 - x) << 0;
			color |= BIT(data_g, 7 - x) << 1;
			color |= BIT(data_b, 7 - x) << 2;

			bitmap.pix(y, x + i * 8) = m_palette->pens()[color];
		}
	}
}

void multi16_state::multi16_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x3ffff).ram();
	map(0x40000, 0x7ffff).noprw();
	map(0xc0000, 0xeffff).ram().share("vram");
	map(0xf0000, 0xf3fff).mirror(0xc000).rom().region("ipl", 0);
}

void multi16_state::multi16_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x02, 0x03) // ?
//  map(0x06, 0x06) // ?
//  map(0x07, 0x07) // ?
//  map(0x08, 0x0b) // ?
	map(0x0c, 0x0f).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x40, 0x40).w(m_crtc, FUNC(hd6845s_device::address_w));
	map(0x41, 0x41).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
//  map(0x42, 0x44) // ?
//  map(0x48, 0x48) // ?
//  map(0x4e, 0x4e) // ?
//  map(0x82, 0x82) // ?
//  map(0x92, 0x92) // ?
}

/* Input ports */
static INPUT_PORTS_START( multi16 )
INPUT_PORTS_END

void multi16_state::machine_start()
{
}


void multi16_state::machine_reset()
{
}

static void multi16_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void multi16_state::multi16(machine_config &config)
{
	I8086(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &multi16_state::multi16_map);
	m_maincpu->set_addrmap(AS_IO, &multi16_state::multi16_io);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	PIT8253(config, m_pit);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16000000, 848, 0, 640, 518, 0, 400); // unknown clock
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	HD6845S(config, m_crtc, 2000000); // unknown clock
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(multi16_state::crtc_update_row));

	// floppy
	MB8866(config, m_fdc, 2000000);

	FLOPPY_CONNECTOR(config, m_floppy[0], multi16_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], multi16_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
}

/* ROM definition */

// MULTI16MODEL2-24
ROM_START( multi16 )
	ROM_REGION16_LE( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD("ipl.rom", 0x0000, 0x4000, CRC(5beb5e94) SHA1(d3b9dc9a08995a0f26af9671893417e795370306))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("kanji.rom", 0x00000, 0x20000, NO_DUMP)
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY       FULLNAME    FLAGS
COMP( 1984, multi16, 0,      0,      multi16, multi16, multi16_state, empty_init, "Mitsubishi", "MULTI 16-II MP-1645", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
