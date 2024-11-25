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
 - I/O board contains an i8031 CPU, an i8251A UART and serial port for a modem,
   and an AT-style keyboard interface and DIN connector.

   CPU board 68010 IRQs:
   IRQ1 = Internal Real Time Clock, Autovectored. Or, Can be used externally but requires manual vectoring.
   IRQ2 = I/O Card Incoming interrupt Request. Triggered when I/O card needs us to handle something.
   IRQ3 = Secondary Graphics Card Incoming interrupt request. Not used in single-card systems.
   IRQ4 = Primary Graphics Card Incoming interrupt request.
   IRQ5 = Data Card incoming interrupt request. Triggers when the Data card has something for us to do or handle.
   IRQ6 = Unused.
   IRQ7 = AC Fail, Battery Backup Input. Once triggered, Unrecoverable. Requires System Reset.

   Graphics board 68010 IRQs:
   IRQ5 = Vertical Blanking Interrupt. Autovectored. Used for timing graphics acceleration instructions.
   IRQ6 = Incoming request from the CPU Card. Used to signal us to go do something.
   IRQ7 = AC Fail, Battery Backup Input. Once triggered, Unrecoverable. Requires System Reset.

   Graphics board 8051 GPIO pins:
   P1.0(T2) = FIFO Empty Flag
   P1.1(T2EX) = Switch to Sat Video. (Active Low)(Drive high to shutdown genlock)
   P1.2 = Switch To Local Video. (Active High)
   P1.3 = Flag to 68K CPU (Control CPU Ready for Command, Active High)
   P1.4 = Sat Video Present/Odd-Even Frame Indicator
   P1.5 = Frame/Sync Control Register (Or Timer prescaler)
   P1.6 = Frame/Sync Control Register (Or Timer prescaler)
   P1.7 = Frame/Sync Control Register (Or Timer prescaler), Watchdog timer.
   P3.0 = RX from FPGA
   P3.1 = TX to FPGA
   P3.2(INT0) = Vertical Drive Interrupt
   P3.3(INT1) = Odd/Even Frame Sync Interrupt
   P3.4(T0) = Input from Frame/Sync Control Register
   P3.5(T1) = Input from Frame/Sync Control Register

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68010.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/icm7170.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "video/bt47x.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

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
		m_mainram(*this, "mainram"),
		m_extram(*this, "extram"),
		m_vram(*this, "vram"),
		m_rtc(*this, "rtc")
	{ }

	void wxstar4k(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cpubd_main(address_map &map) ATTR_COLD;
	void vidbd_main(address_map &map) ATTR_COLD;
	void vidbd_sub(address_map &map) ATTR_COLD;
	void vidbd_sub_io(address_map &map) ATTR_COLD;
	void databd_main(address_map &map) ATTR_COLD;
	void databd_main_io(address_map &map) ATTR_COLD;
	void iobd_main(address_map &map) ATTR_COLD;
	void iobd_main_io(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	optional_device<m68010_device> m_maincpu, m_gfxcpu;
	optional_device<mcs51_cpu_device> m_gfxsubcpu, m_datacpu, m_iocpu;
	optional_shared_ptr<uint16_t> m_mainram, m_extram, m_vram;
	required_device<icm7170_device> m_rtc;

	uint16_t buserr_r()
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
	map(0x000000, 0x1fffff).ram().share("mainram"); // private RAM
	map(0x200000, 0x3fffff).ram().share("extram");  // RAM accessible by other cards
	map(0x400000, 0x400003).r(FUNC(wxstar4k_state::buserr_r));
	// C00000 - I/O card control register
	// C00001-C001FF - I/O card UART buffer
	// C04000-C041FF - I/O card modem buffer
	// C0A000 - data card FIFO
	// C0A200 - write byte to data card CPU
	// C0A400 - read byte from data card CPU
	// C0A600 - write byte to audio control latch 1
	// C0A800 - write byte to audio control latch 2
	map(0xfd0000, 0xfd3fff).rom().region("eeprom", 0); // we'll make this writable later
	// FDF000 - cause IRQ 6 on graphics card
	// FDF004 - cause IRQ 6 on graphics card 2 (not used)
	// FDF008 - reset watchdog
	map(0xfdffc0, 0xfdffe3).rw(m_rtc, FUNC(icm7170_device::read), FUNC(icm7170_device::write)).umask16(0x00ff);
	map(0xfe0000, 0xffffff).rom().region("maincpu", 0);
}

void wxstar4k_state::vidbd_main(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("gfxcpu", 0);
	map(0x100000, 0x10ffff).ram();
	// 200000 - read bit 0=i8051 FIFO full, bit1=i8051 ready for command.  write: top 7 bits of VME address.
	map(0x200002, 0x200003).nopr(); // read: watchdog reset + bit0=Sat video present, bit1=local video present
	// write: interrupt vector when causing a main CPU interrupt
	// 200004 - write i8051 FIFO
	// 200006 - cause IRQ4 on main CPU
	// 300000-300003 - graphics control registers
	map(0x400000, 0x5fffff).ram().share("vram");  // framebuffer (16x M5M44256 = 16Mbit)
	// E00000-E1FFFF - lower 16 address bits of VME access
}

void wxstar4k_state::vidbd_sub(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("gfxsubcpu", 0);
}

void wxstar4k_state::vidbd_sub_io(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	// 1000-17FF - VRAM counter low byte
	// 1800-1FFF - VRAM counter high byte
	// 2000-27FF - read FIFO from 68010
	// 2800-7FFF - undecoded
	// 8000 - Bt471 palette address write (also at C000)
	// 8800 - Bt471 palette RAM (also C800)
	// 9000 - Bt471 pixel mask read (also D000)
	// 9800 - Bt471 palette address read (also D800)
	// A000 - Bt471 overlay write address (also E000)
	// A800 - Bt471 overlay register (also E800)
	// B800 - Bt471 overlay read address (also F800)
}

void wxstar4k_state::databd_main(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("datacpu", 0);
}

void wxstar4k_state::databd_main_io(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	// 0200 - UART data
	// 0201 - UART command
	// 8000 - PIO1 Command/Status register
	// 8001 - PIO1 Port A - VME address/data AD1-AD8
	// 8002 - PIO1 Port B - rear external switches
	// 8003 - PIO1 Port C
	// 8004 - PIO1 transfer count low
	// 8005 - PIO1 transfer count high
	// 8100 - PIO2 Command/Status register
	// 8101 - PIO2 Port A - indicator LEDs
	// 8102 - PIO2 Port B - DTMF dialer codes + 1 LED
	// 8103 - PIO2 Port C - bus clear, charging indicator
	// 8104 - PIO2 transfer count low
	// 8105 - PIO2 transfer count high
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

	I8344(config, m_datacpu, XTAL(7'372'800));  // 7.3728 MHz crystal connected directly to the CPU
	m_datacpu->set_addrmap(AS_PROGRAM, &wxstar4k_state::databd_main);
	m_datacpu->set_addrmap(AS_IO, &wxstar4k_state::databd_main_io);

	I8031(config, m_iocpu, XTAL(11'059'200));   // 11.0592 MHz crystal connected directly to the CPU
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

	ICM7170(config, m_rtc, XTAL(32'768));
}

ROM_START( wxstar4k )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* CPU board 68010 program */
	ROM_LOAD16_BYTE( "u79 rom.bin",  0x000001, 0x010000, CRC(11df2d70) SHA1(ac6cdb5290c90b043562464dc001fc5e3d26f7c6) )
	ROM_LOAD16_BYTE( "u80 rom.bin",  0x000000, 0x010000, CRC(23e15f22) SHA1(a630bda39c0beec7e7fc3834178ec8a6fece70c8) )

	ROM_REGION16_BE(0x4000, "eeprom", 0 ) /* CPU board EEPROM */
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

} // anonymous namespace


COMP( 1990, wxstar4k, 0, 0, wxstar4k, wxstar4k, wxstar4k_state, empty_init, "Applied Microelectronics Institute/The Weather Channel", "WeatherSTAR 4000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
