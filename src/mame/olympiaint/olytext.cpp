// license:BSD-3-Clause
// copyright-holders: Robbbert
/*

    If you want to "own" this, go ahead, it's yours.

    Olympia Olytext 20 Screen-Typing System
    Made in Germany

    Major Chips: Z80A, Z80PIO x2, Z80SCC, WD1772, National Semiconductor NS405-B18N
    Crystals: 16.000, 18.000
    Piezo-Beeper
    RAM: 128k dynamic (16x 4164), 8k static (4x MB8128-10)
    Keyboard: 3870 with 4.000 crystal, 3 dipswitches

    The NS405 is a Terminal Management Processor which provides complete video functionality.
    It is modelled on the 8048, and requires an external rom. All other functions (cpu, timing,
    character generation, uart, dma, crtc, etc) are internal. Data is sent to it in parallel
    from the pio, something like a printer, but there's no handshake. Apparently the TMP is
    fast enough to accept all data as it arrives.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/keyboard.h"
#include "emupal.h"
#include "screen.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class olytext_state : public driver_device
{
public:
	olytext_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
//      , m_fdd0(*this, "fdc:0"),
//      , m_fdd1(*this, "fdc:1"),
//      , m_rom(*this, "mainrom"),
//      , m_lowram(*this, "lowram"),
	{ }

	void olytext(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void keyboard_put(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
//  required_device<floppy_connector> m_fdd0;
//  optional_device<floppy_connector> m_fdd1;
//  required_memory_region m_rom;
//  required_shared_ptr<u8> m_lowram;
};

void olytext_state::machine_reset()
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void olytext_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0xffff).ram();
}

void olytext_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x20, 0x23); // pio?  data, ctrl, data, ctrl // all text is sent as ascii (including control codes) to 0x20
	map(0x30, 0x30); // .w (device)  .r(get keyboard status, if bit 0 high then read keyboard data from port 0x31)
	map(0x32, 0x32); // .w (device)
	map(0x40, 0x40); // .w (banking?)
}


static INPUT_PORTS_START( olytext )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO
//**************************************************************************

uint32_t olytext_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	return 0;
}


//**************************************************************************
//  KEYBOARD
//**************************************************************************

void olytext_state::machine_start()
{
	//m_timer = timer_alloc();
	//const char *type = m_fdd0->get_device()->shortname();
	//if(!strncmp(type, "floppy_525_qd", 13))
		//m_fdctype = 0xa0;
	//else
		//m_fdctype = 0x80;
}

void olytext_state::keyboard_put(u8 data)
{
//  if (data)
//  {
//      m_keystroke=data;
//  }
}

//void olytext_state::fdcctrl_w(uint8_t data)
//{
//  m_fdcctrl = data;
//  m_romen = (m_fdcctrl & 1) ? false : true;
//  m_fdd0->get_device()->mon_w(!(data & 2));
//  if(m_fdd1)
//      m_fdd1->get_device()->mon_w(!(data & 4));
//}

static void olytext_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

void olytext_state::olytext(machine_config &config)
{
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &olytext_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &olytext_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(80*8, 28*11);
	screen.set_visarea(0, (80*8)-1, 0, (28*11)-1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(olytext_state::screen_update));
	//screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* devices */
	WD1772(config, "fdc", 16_MHz_XTAL / 8); // divisor guess
	FLOPPY_CONNECTOR(config, "fdc:0", olytext_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", olytext_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	/* keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(olytext_state::keyboard_put));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( olytext )
	ROM_REGION( 0x1000, "maincpu", 0)
	ROM_LOAD( "1220-040_700.bin",   0x0000, 0x1000, CRC(1df4b31f) SHA1(af3a275d9853d8d23ef091549d7c659cbdb257f3) )

	ROM_REGION( 0x1000, "chargen", 0) // this is firmware for the NS405 cpu
	ROM_LOAD( "1220-041_33702.bin", 0x0000, 0x1000, CRC(a6d39c2a) SHA1(b7a4c65edc7d46d1ab8b0b3aa52141c61c66bf32) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//   YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY                  FULLNAME               FLAGS
COMP(1985, olytext,  0,       0,      olytext,  olytext, olytext_state, empty_init, "Olympia International", "Olympia Olytext 20", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
