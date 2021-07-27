// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot XEN

    Codename "Candyfloss". The system is not IBM compatible but can run
    MS-DOS (and Windows) as well as Xenix.

    Models:
    - XEN FD (512 KB RAM, 2x 720 KB floppy)
    - XEN HD (1 MB RAM, 1x 720 KB or 1.2 MB floppy, 20M HDD)
    - XEN WS (1 MB RAM, no drives)

    TODO:
    - Everything

    Notes:
    - Two graphics cards: Mono and colour boards

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i286.h"
#include "machine/pic8259.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "video/mc6845.h"
#include "imagedev/floppy.h"
#include "formats/apridisk.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class apxen_state : public driver_device
{
public:
	apxen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic%u", 0U),
		m_crtc(*this, "crtc"),
		m_mono_palette(*this, "mono_palette")
	{ }

	void apxen(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i80286_cpu_device> m_maincpu;
	required_device_array<pic8259_device, 2> m_pic;
	required_device<hd6845s_device> m_crtc;
	required_device<palette_device> m_mono_palette;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	MC6845_UPDATE_ROW(crtc_update_row);

	uint8_t get_slave_ack(offs_t offset);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void apxen_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram();
	map(0x0f0000, 0x0fffff).rom().region("bios", 0);
	map(0xff0000, 0xffffff).rom().region("bios", 0);
}

void apxen_state::io_map(address_map &map)
{
	map(0x068, 0x068).w(m_crtc, FUNC(hd6845s_device::address_w));
	map(0x06a, 0x06a).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
//  map(0xc00, 0xc00) leds
//  map(0xc20, 0xc20) printer
//  map(0xc30, 0xc31) SN76489
//  map(0xc40, 0xc47) CIO
//  map(0xc50, 0xc57) SIO
//  map(0xc60, 0xc6f) MM5827 RTC
//  map(0xc80, 0xc87) FDC
//  map(0xc90, 0xc93) uPD7261 HDC
	map(0xca0, 0xca3).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xca4, 0xca7).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
//  map(0xcb0, 0xcb7) PIT
//  map(0xcc0, 0xccd) uPD71071 DMA 1
//  map(0xcd0, 0xcdd) uPD71071 DMA 2
//  map(0xce0, 0xce0) cpu control
//  map(0xcf0, 0xcf0) cpu reset
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( apxen )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t apxen_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

MC6845_UPDATE_ROW( apxen_state::crtc_update_row )
{
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void apxen_state::machine_start()
{
}

void apxen_state::machine_reset()
{
}

uint8_t apxen_state::get_slave_ack(offs_t offset)
{
	if (offset == 2)
		return m_pic[1]->acknowledge();

	return 0x00;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void apxen_state::apxen(machine_config &config)
{
	I80286(config, m_maincpu, 15_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &apxen_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &apxen_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_pic[0], FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic[0], 0);
	m_pic[0]->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->read_slave_ack_callback().set(FUNC(apxen_state::get_slave_ack));

	PIC8259(config, m_pic[1], 0);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir2_w));
	m_pic[1]->in_sp_callback().set_constant(0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_size(800, 400);
	screen.set_visarea(0, 800-1, 0, 400-1);
	screen.set_refresh_hz(72);
	screen.set_screen_update(FUNC(apxen_state::screen_update));

	PALETTE(config, m_mono_palette, palette_device::MONOCHROME_HIGHLIGHT);

	HD6845S(config, m_crtc, 24_MHz_XTAL / 20); // actually MB89321B
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(10);
	m_crtc->set_update_row_callback(FUNC(apxen_state::crtc_update_row));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( apxen )
	ROM_REGION16_LE(0x10000, "bios", 0)
	// LO-XEN  3.1.3 7143 (checksum matches)
	ROM_LOAD16_BYTE("lo-xen_313.ic80", 0x0000, 0x8000, CRC(c2a1fd6e) SHA1(8dfc711dd910bc3d43c1120978ba199a13463068))
	// HI-XEN  3.1.3 9BF0 (checksum matches)
	ROM_LOAD16_BYTE("hi-xen_313.ic37", 0x0001, 0x8000, CRC(72ee2f09) SHA1(da11043d40a694802f6d3d27a4359067dd19c8e6))

	// should probably be moved elsewhere
	ROM_REGION(0x2000, "hdd", 0)
	ROM_LOAD("rodime_ro3055.bin", 0x0000, 0x2000, CRC(61d1544a) SHA1(2177a4c6409c0ee3d3e3e6c659085adf236f8726))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME       FLAGS
COMP( 1985, apxen, 0,      0,      apxen,   apxen, apxen_state, empty_init, "ACT",   "Apricot XEN", MACHINE_IS_SKELETON )
