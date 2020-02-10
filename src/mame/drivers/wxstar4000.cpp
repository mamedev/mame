// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    wxstar4000.cpp: WeatherSTAR 4000 cable head-end unit
    1990 Applied Microelectronics Institute (Amirix) / The Weather Channel
    Skeleton driver by R. Belmont

  This was used by cable companies in the US starting in 1990 to generate
  graphics and text for the local weather forecast during The Weather Channel's
  "Local on the 8s" segments.

 There are 4 PCBs on a VME backplane:

 - CPU board contains a 68010 CPU, RAM, and EEPROM
 - Graphics board contains a 68010 CPU and an i8051 which manages the palette
   and performs other functions.  Framebuffer is 8 bits per pixel and the
   framebuffer start position can be changed, possibly per-scanline.
   RAMDAC is a Bt471.
 - Data/Audio board contains an Intel P8344AH, which is an MCU containing an
   i8051 and an SDLC decoder.  The "audio" is a TTL circuit to create an
   alert tone for severe weather conditions.
 - I/O board contains an i8051 CPU, an i8251A UART and serial port for a modem,
   and an AT-style keyboard interface and DIN connector.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "video/bt47x.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class wxstar4k_state : public driver_device
{
public:
	wxstar4k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxcpu(*this, "gfxcpu"),
		m_gfxsubcpu(*this, "gfxsubcpu"),
		m_datacpu(*this, "datacpu"),
		m_iocpu(*this, "iocpu"),
		m_mainram(*this, "mainram")
	{ }

	void wxstar4k(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cpubd_main(address_map &map);
	void vidbd_main(address_map &map);
	void vidbd_sub(address_map &map);
	void vidbd_sub_io(address_map &map);
	void databd_main(address_map &map);
	void databd_main_io(address_map &map);
	void iobd_main(address_map &map);
	void iobd_main_io(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_device<m68010_device> m_maincpu, m_gfxcpu;
	required_device<i8051_device> m_gfxsubcpu, m_datacpu, m_iocpu;
	required_shared_ptr<uint16_t> m_mainram;

	DECLARE_READ16_MEMBER(buserr_r)
	{
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		return 0xffff;
	}
};

void wxstar4k_state::video_start()
{
}

uint32_t wxstar4k_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void wxstar4k_state::cpubd_main(address_map &map)
{
	// 4x M5M44400A fast-page DRAMs (1M x 4)
	map(0x000000, 0x1fffff).ram().share("mainram");
	map(0x100000, 0x100003).r(FUNC(wxstar4k_state::buserr_r));
	map(0xfe0000, 0xffffff).rom().region("maincpu", 0);
}

void wxstar4k_state::vidbd_main(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("gfxcpu", 0);
	map(0x100000, 0x10ffff).ram();  // shared with main?
	map(0x200002, 0x200003).nopr(); // probably resets the watchdog
	map(0x400000, 0x45ffff).ram();  // framebuffer?
}

void wxstar4k_state::vidbd_sub(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("gfxsubcpu", 0);
}

void wxstar4k_state::vidbd_sub_io(address_map &map)
{
	map(0x0000, 0x07ff).ram();
}

void wxstar4k_state::databd_main(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("datacpu", 0);
}

void wxstar4k_state::databd_main_io(address_map &map)
{
}

void wxstar4k_state::iobd_main(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("iocpu", 0);
}

void wxstar4k_state::iobd_main_io(address_map &map)
{
}

static INPUT_PORTS_START( wxstar4k )
INPUT_PORTS_END

void wxstar4k_state::machine_start()
{
}

void wxstar4k_state::machine_reset()
{
	u16 *RAM = (u16 *)m_mainram.target();
	u16 *ROM = &memregion("maincpu")->as_u16();
	memcpy(RAM, ROM, 0x400);
}

void wxstar4k_state::wxstar4k(machine_config &config)
{
	/* basic machine hardware */
	M68010(config, m_maincpu, XTAL(20'000'000)/2);  // 20 MHz crystal / 2 (QA output of a 74LS393)
	m_maincpu->set_addrmap(AS_PROGRAM, &wxstar4k_state::cpubd_main);

	M68010(config, m_gfxcpu, XTAL(20'000'000)/2);   // runs on the system clock from the CPU board, so also 10 MHz
	m_gfxcpu->set_addrmap(AS_PROGRAM, &wxstar4k_state::vidbd_main);

	I8051(config, m_gfxsubcpu, XTAL(12'000'000));   // 12 MHz crystal connected directly to the CPU
	m_gfxsubcpu->set_addrmap(AS_PROGRAM, &wxstar4k_state::vidbd_sub);
	m_gfxsubcpu->set_addrmap(AS_IO, &wxstar4k_state::vidbd_sub_io);

	I8051(config, m_datacpu, XTAL(7'372'800));  // 7.3728 MHz crystal connected directly to the CPU
	m_datacpu->set_addrmap(AS_PROGRAM, &wxstar4k_state::databd_main);
	m_datacpu->set_addrmap(AS_IO, &wxstar4k_state::databd_main_io);

	I8051(config, m_iocpu, XTAL(11'059'200));   // 11.0592 MHz crystal connected directly to the CPU
	m_iocpu->set_addrmap(AS_PROGRAM, &wxstar4k_state::iobd_main);
	m_iocpu->set_addrmap(AS_IO, &wxstar4k_state::iobd_main_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.62);  /* verified on pcb */
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(40, 400-1, 16, 240-1);
	screen.set_screen_update(FUNC(wxstar4k_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_888, 256);
}

ROM_START( wxstar4k )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* CPU board 68010 program */
	ROM_LOAD16_BYTE( "u79 rom.bin",  0x000001, 0x010000, CRC(11df2d70) SHA1(ac6cdb5290c90b043562464dc001fc5e3d26f7c6) )
	ROM_LOAD16_BYTE( "u80 rom.bin",  0x000000, 0x010000, CRC(23e15f22) SHA1(a630bda39c0beec7e7fc3834178ec8a6fece70c8) )

	ROM_REGION(0x2000, "eeprom", 0 ) /* CPU board EEPROM */
	ROM_LOAD( "u72 eeprom.bin", 0x000000, 0x002000, CRC(f775b4d6) SHA1(a0895177c381919f9bfd99ee35edde0dd5fa379c) )

	ROM_REGION(0x2000, "datacpu", 0) /* P8344 (i8051 plus SDLC decoder) on Data board */
	ROM_LOAD( "u12 rom.bin",  0x000000, 0x002000, CRC(f7d8432d) SHA1(0ff1dad65ecb4c3d8cb21feef56bbc6f06a2f712) )

	ROM_REGION(0x8000, "iocpu", 0) /* i8051 on I/O board */
	ROM_LOAD( "u11 rom.bin",  0x000000, 0x008000, CRC(f12cb28b) SHA1(3368f55717d8e9e7a06a4f241de02b7b2577b32b) )

	ROM_REGION(0x2000, "gfxsubcpu", 0) /* i8051 sub-CPU on Graphics board */
	ROM_LOAD( "u13 rom.bin",  0x000000, 0x002000, CRC(667b0a2b) SHA1(d60bcc271a73633544b0cf2f80589b2e5670b705) )

	ROM_REGION(0x10000, "gfxcpu", 0) /* Graphics board 68010 program */
	ROM_LOAD16_BYTE( "u42 rom low.bin", 0x000001, 0x008000, CRC(84038ca3) SHA1(b28a0d357d489fb06ff0d5d36ea11ebd1f9612a5) )
	ROM_LOAD16_BYTE( "u43 rom high.bin", 0x000000, 0x008000, CRC(6f2a7592) SHA1(1aa2394db42b6f28277e35a48a7cef348c213e05) )
ROM_END

COMP( 1990, wxstar4k, 0, 0, wxstar4k, wxstar4k, wxstar4k_state, empty_init, "Applied Microelectronics Institute/The Weather Channel", "WeatherSTAR 4000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
