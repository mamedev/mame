// license:BSD-3-Clause
// copyright-holders:Robbbert
/*****************************************************************************

Super80.cpp written by Robbbert, 2005-2010.

2010-12-19: Added V3.7 bios freshly dumped today.
2014-04-28: Added disk system and did cleanups

See the MAME sysinfo and wiki for usage
documentation. Below for the most technical bits:

= Architecture (super80):

  * Z80 @ 2MHz
  * 16k, 32k or 48k RAM (0000-BFFF)
  * 12k ROM (C000-EFFF)
  * 3.5k RAM (F000-FDFF), comes with the "64k ram" modification
  * 0.5k RAM (FE00-FFFF) for Chipspeed colour board

= Architecture (super80v):

  * Z80 @ 2MHz
  * 16k, 32k or 48k RAM (0000-BFFF)
  * 12k ROM (C000-EFFF)
  * 2k Video RAM (F000-F7FF) banked with Colour RAM (modified Chipspeed board)
  * 2k PCG RAM (F800-FFFF) banked with Character Generator ROM

= Super80 ports:

  port $F0: General Purpose output port
    Bit 0 - cassette output
    Bit 1 - cassette relay control; 0=relay on
    Bit 2 - turns screen on and off;0=screen off
    Bit 3 - Available for user projects [We will use it for sound]
    Bit 4 - Available for user projects
    Bit 5 - cassette LED; 0=LED on
    Bit 6/7 - not decoded

  port $F1: Video page output port
    Bit 0 - not decoded [we will use it for video switching]
    Bits 1 to 7 - choose video page to display
    Bit 1 controls A9, bit 2 does A10, etc

  port $F2: General purpose input port
    Bit 0 - cassette input
    Bit 1 - Available for user projects
    Bit 2 - Available for user projects
    Bit 3 - not decoded
    Bit 4 - Switch A [These switches are actual DIP switches on the motherboard]
    Bit 5 - Switch B
    Bit 6 - Switch C
    Bit 7 - Switch D

= Super80v ports:

  port $10: MC6845 control port

  port $11: MC6845 data port

  port $F0: General Purpose output port
    Bit 0 - cassette output
    Bit 1 - Cassette relay control; 0=relay on
    Bit 2 - Colour banking (0 = Colour Ram, 1 = Video Ram)
    Bit 3 - Sound
    Bit 4 - PCG banking (0 = PROM, 1 = PCG)
    Bit 5 - cassette LED; 0=LED on
    Bit 6/7 - not decoded

  port $F2: General purpose input port - same as for Super80.

= Cassette information:

The standard cassette system uses Kansas City format. Data rates available are 300, 400, 600, and 1200 baud.
The user has to adjust some bytes in memory in order to select a different baud rate.

  BDF8  BDF9    Baud
  ---------------------
  F8    04       300
  BA    03       400
  7C    02       600
  3E    01      1200

The enhanced Monitor roms (those not supplied by Dick Smith) have extra commands to change the rates
without the tedium of manually modifying memory.

When saving, the OS toggles the cassette bit (bit 0 of port F0) at the required frequencies directly.

When loading, the signal passes through a filter, then into a 4046 PLL (Phase-Locked-Loop) chip.
This acts as a frequency-to-voltage converter. The output of this device is passed to the "+" input
of a LM311 op-amp. The "-" input is connected to a 10-turn trimpot, which is adjusted by the owner
at construction time. It sets the switching midpoint voltage. Voltages above a certain level become
a "1" level at the output, while voltages below become a "0" level. This output in turn connects
to the cassette input bit of port F2.

The monitor loading routine (at C066 in most monitor roms), waits for a high-to-low transition
(the low is the beginning of the start bit), then waits for half a bit, checks it is still low,
waits for a full bit, then reads the state (this is the first bit), then cycles between waiting
a bit and reading the next, until a full byte has been constructed. Lastly, the stop bit is
checked that it is at a high level.

This means that we cannot attempt to convert frequency to voltage ourselves, since the OS only
"looks" once a bit. The solution is to use a mame timer running at a high enough rate (40 kHz)
to read the wave state. While the wave state stays constant, a counter is incremented. When the
state changes, the output is set according to how far the counter has progressed. The counter is
then reset ready for the next wave state. The code for this is in the TIMER_CALLBACK.

A kit was produced by ETI magazine, which plugged into the line from your cassette player earphone
socket. The computer line was plugged into this box instead of the cassette player. The box was
fitted with a speaker and a volume control. You could listen to the tones, to assist with head
alignment, and with debugging. In MAME, a config switch has been provided so that you can turn
this sound on or off as needed.

= About the 1 MHz / 2 MHz switching:

The original hardware runs with a 2 MHz clock, but operates in an unusual way. There is no video
processor chip, just a huge bunch of TTL. The system spends half the time running the CPU,
and half the time displaying the picture. The timing will activate the BUSREQ line, and the CPU will
finish its current instruction, activate the BUSACK line, and go to sleep. The video circuits will
read the video RAM and show the picture. At the end, the BUSREQ line is released, and processing can
continue.

The processing time occurs during the black parts of the screen, therefore half the screen will be
black unless you expand the image with the monitor's controls. This method ensures that there will
be no memory contention, and thus, no snow. The processor will run at 2 MHz pulsed at 48.8 Hz, which
is an effective speed of 1 MHz.

When saving or loading a cassette file, this pulsing would cause the save tone to be modulated with
a loud hum. When loading, the synchronisation to the start bit could be missed, causing errors.
Therefore the screen can be turned off via an output bit. This disables the BUSREQ control, which
in turn prevents screen refresh, and gives the processor a full uninterrupted 2 MHz speed.

To obtain accurate timing, the video update routine will toggle the HALT line on alternate frames.
Although physically incorrect, it is the only way to accurately emulate the speed change function.
The video update routine emulates the blank screen by filling it with spaces.

For the benefit of those who like to experiment, config switches have been provided in MAME to
allow you to leave the screen on at all times, and to always run at 2 MHz if desired. These options
cannot exist in real hardware.

= Quickload:

This was not a standard feature. It is a hardware facility I added to my machine when age threatened
to kill off my cassette player and tapes. The tapes were loaded up one last time, and transferred to
a hard drive on a surplus 386 PC, in binary format (NOT a wave file). Special roms were made to allow
loading and saving to the S-100 board and its ports. These ports were plugged into the 386 via cables.
A QBASIC program on the 386 monitored the ports and would save and load files when requested by the
Super-80. This worked (and still works) very well, and is a huge improvement over the cassette, both
speedwise and accuracy-wise.

The modified rom had the autorun option built in. Autorun was never available for cassettes.

In MAME, the same file format is used - I can transfer files between MAME and the 386 seamlessly.
MAME has one difference - the program simply appears in memory without the processor being aware
of it. To accomplish autorun therefore requires that the processor pc register be set to the start
address of the program. BASIC programs may need some preprocessing before they can be started. This
is not necessary on a Super-80 or a Microbee, but is needed on any system running Microsoft BASIC,
such as the Exidy Sorcerer or the VZ-200.

In MAME, quickload is available for all Super80 variants (otherwise you would not have any games
to play). MAME features a config switch so the user can turn autorun on or off as desired.


= Start of Day circuit:

When the computer is turned on or reset, the Z80 will want to start executing at 0000. Since this is
RAM, this is not a good idea. The SOD circuit forcibly disables the RAM and enables the ROMs, so they
will appear to be at 0000. Thus, the computer will boot.

The Master Reset signal (power-on or Reset button pushed), triggers a flipflop that disables RAM and
causes C000 to FFFF to appear at 0000 to 3FFF. This will be reset (back to normal) when A14 and A15
are high, and /M1 is active. This particular combination occurs on all ROM variants, when reading the
fifth byte in the ROM. In reality, the switchover can take place any time between the 4th byte until
the computer has booted up. This is because the low RAM does not contain any system areas.


Super80 disk WD2793, Z80DMA:

Port(hex)  Role       Comment
---------  -----      --------
30         DMA I/O    Z80A DMA Controller Command Register
38         WDSTCM     WD2793 Command Status
39         WDTRK      WD2793 Track Register
3A         WDSEC      WD2793 Sector Register
3B         WDDATA     WD2793 Data Register

3E         UFDSTAT    FDD and WD2793 Status input
   Bit0    INTRQ      1 = WD2793 Interrupt Request
   Bit1    DATARQ     1 = Data Ready to Receive or Send
   Bit2    MOTOR ON   1 = FDD Motor On

3F         UFDCOM     WD2793 Command Register output
   Bit0    5/8        1 = 8" Drive Selected
   Bit1    ENMF*      0 = 1MHz, 1 = 2MHz (for 8" double density)
   Bit2    DR0SEL     1 = Drive 0 (Drive A) selected
   Bit3    DR1SEL     1 = Drive 1 (Drive B) selected
   Bit4    DR2SEL     1 = Drive 2 (Drive C) selected
   Bit5    DR3SEL     1 = Drive 3 (Drive D) selected
   Bit6    SIDESEL    0 = Select Disk side 0, 1 = Select Disk side 1
   Bit7    DDEN*      0 = MFM (Double Density), 1 = FM (Single Density)

ToDo:
- Fix Paste: Shift operates randomly (only super80m is suitable, the others drop characters because
       of the horrible inline editor they use)


***********************************************************************************************************/

#include "emu.h"
#include "super80.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"

#include "super80.lh"


#define MASTER_CLOCK    (12_MHz_XTAL)
#define PIXEL_CLOCK (MASTER_CLOCK/2)
#define HTOTAL      (384)
#define HBEND       (0)
#define HBSTART     (256)
#define VTOTAL      (240)
#define VBEND       (0)
#define VBSTART     (160)

#define SUPER80V_SCREEN_WIDTH       (560)
#define SUPER80V_SCREEN_HEIGHT      (300)
#define SUPER80V_DOTS           (7)


/**************************** MEMORY AND I/O MAPPINGS *****************************************************************/

void super80_state::super80_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share("mainram");
	map(0xc000, 0xefff).rom().region("maincpu", 0).nopw();
	map(0xf000, 0xffff).lr8(NAME([] () { return 0xff; })).nopw();
}

void super80_state::super80m_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0xc000, 0xefff).rom().region("maincpu", 0).nopw();
}

void super80v_state::super80v_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share("mainram");
	map(0xc000, 0xefff).rom().region("maincpu", 0).nopw();
	map(0xf000, 0xf7ff).rw(FUNC(super80v_state::low_r),  FUNC(super80v_state::low_w));
	map(0xf800, 0xffff).rw(FUNC(super80v_state::high_r), FUNC(super80v_state::high_w));
}

void super80r_state::super80r_map(address_map &map)
{
	super80v_map(map);
	map(0xf000, 0xf7ff).rw(FUNC(super80r_state::low_r),  FUNC(super80r_state::low_w));
	map(0xf800, 0xffff).rw(FUNC(super80r_state::high_r), FUNC(super80r_state::high_w));
}

void super80_state::super80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xdc, 0xdc).r("cent_status_in", FUNC(input_buffer_device::read));
	map(0xdc, 0xdc).w(FUNC(super80_state::portdc_w));
	map(0xe0, 0xe0).mirror(0x14).w(FUNC(super80_state::portf0_w));
	map(0xe1, 0xe1).mirror(0x14).w(FUNC(super80_state::portf1_w));
	map(0xe2, 0xe2).mirror(0x14).r(FUNC(super80_state::portf2_r));
	map(0xf8, 0xfb).mirror(0x04).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}

void super80_state::super80e_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xbc, 0xbc).r("cent_status_in", FUNC(input_buffer_device::read));
	map(0xbc, 0xbc).w(FUNC(super80_state::portdc_w));
	map(0xe0, 0xe0).mirror(0x14).w(FUNC(super80_state::portf0_w));
	map(0xe1, 0xe1).mirror(0x14).w(FUNC(super80_state::portf1_w));
	map(0xe2, 0xe2).mirror(0x14).r(FUNC(super80_state::portf2_r));
	map(0xf8, 0xfb).mirror(0x04).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}


void super80v_state::super80v_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x10, 0x10).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x11, 0x11).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x30, 0x30).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x38, 0x3b).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x3e, 0x3e).r(FUNC(super80v_state::port3e_r));
	map(0x3f, 0x3f).w(FUNC(super80v_state::port3f_w));
	map(0xdc, 0xdc).r("cent_status_in", FUNC(input_buffer_device::read));
	map(0xdc, 0xdc).w(FUNC(super80v_state::portdc_w));
	map(0xe0, 0xe0).mirror(0x14).w(FUNC(super80v_state::portf0_w));
	map(0xe2, 0xe2).mirror(0x14).r(FUNC(super80v_state::portf2_r));
	map(0xf8, 0xfb).mirror(0x04).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}

/**************************** DIPSWITCHES, KEYBOARD, HARDWARE CONFIGURATION ****************************************/

	/* Enhanced options not available on real hardware */
static INPUT_PORTS_START( super80_cfg )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Autorun on Quickload")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x01, DEF_STR(Yes))
	PORT_CONFNAME( 0x02, 0x02, "2 MHz always")
	PORT_CONFSETTING(    0x02, DEF_STR(No))
	PORT_CONFSETTING(    0x00, DEF_STR(Yes))
	PORT_CONFNAME( 0x04, 0x04, "Screen on always")
	PORT_CONFSETTING(    0x04, DEF_STR(No))
	PORT_CONFSETTING(    0x00, DEF_STR(Yes))
	PORT_CONFNAME( 0x08, 0x08, "Cassette Speaker")
	PORT_CONFSETTING(    0x08, DEF_STR(On))
	PORT_CONFSETTING(    0x00, DEF_STR(Off))
	PORT_CONFNAME( 0x60, 0x40, "Colour")
	PORT_CONFSETTING(    0x60, "White")
	PORT_CONFSETTING(    0x40, "Green")
INPUT_PORTS_END

static INPUT_PORTS_START( super80 )
	PORT_START("DSW")
	PORT_BIT( 0xf, 0xf, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Switch A") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "Switch B") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "Switch C") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, "Switch D") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("KEY.0")  // The physical keys show some shifted characters that are not actually available
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@  `") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REPT") PORT_CODE(KEYCODE_LALT)
	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('Q') PORT_CHAR(0x11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('Y') PORT_CHAR(0x19)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('R') PORT_CHAR(0x12)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(0x09)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(Fire)") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('K') PORT_CHAR(0x0b)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[  {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINEFEED") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(0x0a)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('L') PORT_CHAR(0x0c)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('T') PORT_CHAR(0x14)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\  |")PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('\\') PORT_CHAR(0x1c)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BRK") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(0x03)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('U') PORT_CHAR(0x15)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]  }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR(']') PORT_CHAR(0x1d)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^  ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('^') PORT_CHAR(0x1e)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  \'") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x7f) PORT_CHAR(0x7f) PORT_CHAR(0x1f) // natural kbd, press ctrl-backspace to DEL
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('G') PORT_CHAR(0x07)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('O') PORT_CHAR(0x0f)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_INCLUDE( super80_cfg )
INPUT_PORTS_END

static INPUT_PORTS_START( super80d )
	PORT_START("DSW")
	PORT_BIT( 0xf, 0xf, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Switch A") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "Switch B") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "Switch C") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, "Switch D") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@  `") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p') PORT_CHAR(0x10)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x') PORT_CHAR(0x18)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REPT") PORT_CODE(KEYCODE_LALT)
	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a') PORT_CHAR(0x01)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q') PORT_CHAR(0x11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y') PORT_CHAR(0x19)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b') PORT_CHAR(0x02)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r') PORT_CHAR(0x12)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z') PORT_CHAR(0x1a)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(0x09)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(Fire)") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k') PORT_CHAR(0x0b)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s') PORT_CHAR(0x13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[  {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINEFEED") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(0x0a)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d') PORT_CHAR(0x04)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l') PORT_CHAR(0x0c)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t') PORT_CHAR(0x14)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\  |")PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BRK") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(0x03)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e') PORT_CHAR(0x05)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u') PORT_CHAR(0x15)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]  }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f') PORT_CHAR(0x06)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n') PORT_CHAR(0x0e)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v') PORT_CHAR(0x16)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^  ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~') PORT_CHAR(0x1e)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  \'") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x7f) PORT_CHAR(0x5f) PORT_CHAR(0x1f) // natural kbd, press ctrl-backspace to DEL
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g') PORT_CHAR(0x07)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o') PORT_CHAR(0x0f)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w') PORT_CHAR(0x17)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_INCLUDE( super80_cfg )
INPUT_PORTS_END

static INPUT_PORTS_START( super80e )
	PORT_INCLUDE( super80d )
	PORT_MODIFY("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@  `") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`') // fix mistake in super80d matrix
INPUT_PORTS_END

static INPUT_PORTS_START( super80m )
	PORT_INCLUDE( super80e )

	PORT_MODIFY("CONFIG")

	/* Enhanced options not available on real hardware */
	PORT_CONFNAME( 0x10, 0x10, "Swap CharGens")
	PORT_CONFSETTING(    0x10, DEF_STR(No))
	PORT_CONFSETTING(    0x00, DEF_STR(Yes))
	PORT_CONFNAME( 0x60, 0x40, "Colour")
	PORT_CONFSETTING(    0x60, "White")
	PORT_CONFSETTING(    0x40, "Green")
	PORT_CONFSETTING(    0x00, "Composite")
	PORT_CONFSETTING(    0x20, "RGB")
INPUT_PORTS_END

static INPUT_PORTS_START( super80v )
	PORT_INCLUDE( super80m )

	PORT_MODIFY("CONFIG")
	PORT_BIT( 0x16, 0x16, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( super80r )
	PORT_INCLUDE( super80d )

	PORT_MODIFY("CONFIG")
	PORT_BIT( 0x16, 0x16, IPT_UNUSED )
INPUT_PORTS_END


/**************************** F4 CHARACTER DISPLAYER ***********************************************************/

static const gfx_layout super80_charlayout =
{
	8,10,                   /* 8 x 10 characters */
	64,                 /* 64 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  2*8,  4*8,  6*8,  8*8, 10*8, 12*8, 14*8, 1*8,  3*8,  5*8,  7*8,  9*8, 11*8, 13*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static const gfx_layout super80d_charlayout =
{
	8,10,                   /* 8 x 10 characters */
	128,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  2*8,  4*8,  6*8,  8*8, 10*8, 12*8, 14*8, 1*8,  3*8,  5*8,  7*8,  9*8, 11*8, 13*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static const gfx_layout super80e_charlayout =
{
	8,10,                   /* 8 x 10 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  2*8,  4*8,  6*8,  8*8, 10*8, 12*8, 14*8, 1*8,  3*8,  5*8,  7*8,  9*8, 11*8, 13*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static const gfx_layout super80v_charlayout =
{
	8,16,                   /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_super80 )
	GFXDECODE_ENTRY( "chargen", 0x0000, super80_charlayout, 16, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_super80d )
	GFXDECODE_ENTRY( "chargen", 0x0000, super80d_charlayout, 16, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_super80e )
	GFXDECODE_ENTRY( "chargen", 0x0000, super80e_charlayout, 16, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_super80m )
	GFXDECODE_ENTRY( "chargen", 0x0000, super80e_charlayout, 2, 6 )
	GFXDECODE_ENTRY( "chargen", 0x1000, super80d_charlayout, 2, 6 )
GFXDECODE_END

static GFXDECODE_START( gfx_super80v )
	GFXDECODE_ENTRY( "chargen", 0x0000, super80v_charlayout, 2, 6 )
GFXDECODE_END



/**************************** BASIC MACHINE CONSTRUCTION ***********************************************************/


static const z80_daisy_config super80_daisy_chain[] =
{
	{ "z80pio" },
	{ nullptr }
};

//-------------------------------------------------
//  Z80DMA
//-------------------------------------------------

uint8_t super80v_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void super80v_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

uint8_t super80v_state::io_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

void super80v_state::io_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

static void super80_floppies(device_slot_interface &device)
{
	device.option_add("s80flop", FLOPPY_525_QD);
}


static const char *const relay_sample_names[] =
{
	"*relay",
	"relayoff",
	"relayon",
	nullptr
};


void super80_state::super80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);        /* 2 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &super80_state::super80_map);
	m_maincpu->set_addrmap(AS_IO, &super80_state::super80_io);
	m_maincpu->set_daisy_config(super80_daisy_chain);

	Z80PIO(config, m_pio, MASTER_CLOCK/6);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->out_pa_callback().set([this](u8 data){ super80_state::pio_port_a_w(data); });
	m_pio->in_pb_callback().set([this](){ return super80_state::pio_port_b_r(); });

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(48.8);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(super80_state::screen_update_super80));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(super80_state::super80m_palette), 32);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_super80);

	config.set_default_layout(layout_super80);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(relay_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit7));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	INPUT_BUFFER(config, "cent_status_in", 0);

	/* quickload */
	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "bin", attotime::from_seconds(3)));
	quickload.set_load_callback(FUNC(super80_state::quickload_cb));
	quickload.set_interface("super80_quik");

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("super80_cass");

	TIMER(config, "kansas_r").configure_periodic(FUNC(super80_state::kansas_r), attotime::from_hz(40000)); // cass read
	TIMER(config, "timer_k").configure_periodic(FUNC(super80_state::timer_k), attotime::from_hz(300)); // keyb scan
	TIMER(config, "timer_h").configure_periodic(FUNC(super80_state::timer_h), attotime::from_hz(100)); // half-speed

	// software list
	SOFTWARE_LIST(config, "cass_list").set_original("super80_cass").set_filter("DEF");
	SOFTWARE_LIST(config, "quik_list").set_original("super80_quik").set_filter("DEF");
}

void super80_state::super80d(machine_config &config)
{
	super80(config);
	m_gfxdecode->set_info(gfx_super80d);
	m_screen->set_screen_update(FUNC(super80_state::screen_update_super80d));

	// software list
	SOFTWARE_LIST(config.replace(), "cass_list").set_original("super80_cass").set_filter("D");
	SOFTWARE_LIST(config.replace(), "quik_list").set_original("super80_quik").set_filter("D");
}

void super80_state::super80e(machine_config &config)
{
	super80(config);
	m_maincpu->set_addrmap(AS_IO, &super80_state::super80e_io);
	m_gfxdecode->set_info(gfx_super80e);
	m_screen->set_screen_update(FUNC(super80_state::screen_update_super80e));

	// software list
	SOFTWARE_LIST(config.replace(), "cass_list").set_original("super80_cass").set_filter("E");
	SOFTWARE_LIST(config.replace(), "quik_list").set_original("super80_quik").set_filter("E");
}

void super80_state::super80m(machine_config &config)
{
	super80(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &super80_state::super80m_map);

	m_gfxdecode->set_info(gfx_super80m);

	m_screen->set_screen_update(FUNC(super80_state::screen_update_super80m));
	m_screen->screen_vblank().set([this](bool state) { super80_state::screen_vblank_super80m(state); });

	// software list
	SOFTWARE_LIST(config.replace(), "cass_list").set_original("super80_cass").set_filter("M");
	SOFTWARE_LIST(config.replace(), "quik_list").set_original("super80_quik").set_filter("M");
}

void super80v_state::super80v(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);        /* 2 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &super80v_state::super80v_map);
	m_maincpu->set_addrmap(AS_IO, &super80v_state::super80v_io);
	m_maincpu->set_daisy_config(super80_daisy_chain);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	Z80PIO(config, m_pio, MASTER_CLOCK/6);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->out_pa_callback().set([this](u8 data){ super80v_state::pio_port_a_w(data); });
	m_pio->in_pb_callback().set([this](){ return super80v_state::pio_port_b_r(); });

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_size(SUPER80V_SCREEN_WIDTH, SUPER80V_SCREEN_HEIGHT);
	m_screen->set_visarea(0, SUPER80V_SCREEN_WIDTH-1, 0, SUPER80V_SCREEN_HEIGHT-1);
	m_screen->set_screen_update(FUNC(super80v_state::screen_update_super80v));
	m_screen->screen_vblank().set([this](bool state) { super80v_state::screen_vblank_super80m(state); });

	PALETTE(config, m_palette, FUNC(super80v_state::super80m_palette), 32);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_super80v);

	MC6845(config, m_crtc, MASTER_CLOCK / SUPER80V_DOTS);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(SUPER80V_DOTS);
	m_crtc->set_update_row_callback(FUNC(super80v_state::crtc_update_row));

	config.set_default_layout(layout_super80);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(relay_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit7));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	INPUT_BUFFER(config, "cent_status_in", 0);

	/* quickload */
	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "bin", attotime::from_seconds(3)));
	quickload.set_load_callback(FUNC(super80v_state::quickload_cb));
	quickload.set_interface("super80_quik");

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("super80_cass");

	TIMER(config, "kansas_r").configure_periodic(FUNC(super80v_state::kansas_r), attotime::from_hz(40000)); // cass read
	TIMER(config, "timer_k").configure_periodic(FUNC(super80v_state::timer_k), attotime::from_hz(300)); // keyb scan

	Z80DMA(config, m_dma, MASTER_CLOCK/6);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	//ba0 - not connected
	m_dma->in_mreq_callback().set(FUNC(super80v_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(super80v_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(super80v_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(super80v_state::io_write_byte));

	WD2793(config, m_fdc, 2_MHz_XTAL);
	m_fdc->drq_wr_callback().set(m_dma, FUNC(z80dma_device::rdy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", super80_floppies, "s80flop", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", super80_floppies, "s80flop", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", super80_floppies, "s80flop", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", super80_floppies, "s80flop", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// software list
	SOFTWARE_LIST(config, "cass_list").set_original("super80_cass").set_filter("V");
	SOFTWARE_LIST(config, "quik_list").set_original("super80_quik").set_filter("V");
	SOFTWARE_LIST(config, "flop_list").set_original("super80_flop");
}

void super80r_state::super80r(machine_config &config)
{
	super80v(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &super80r_state::super80r_map);

	// software list
	SOFTWARE_LIST(config.replace(), "cass_list").set_original("super80_cass").set_filter("R");
	SOFTWARE_LIST(config.replace(), "quik_list").set_original("super80_quik").set_filter("R");
}

/**************************** ROMS *****************************************************************/

ROM_START( super80 )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("super80.u26",   0x0000, 0x1000, CRC(6a6a9664) SHA1(2c4fcd943aa9bf7419d58fbc0e28ffb89ef22e0b) )
	ROM_LOAD("super80.u33",   0x1000, 0x1000, CRC(cf8020a8) SHA1(2179a61f80372cd49e122ad3364773451531ae85) )
	ROM_LOAD("super80.u42",   0x2000, 0x1000, CRC(a1c6cb75) SHA1(d644ca3b399c1a8902f365c6095e0bbdcea6733b) )

	ROM_REGION(0x0400, "chargen", 0)    // 2513 prom
	ROM_LOAD("super80.u27",   0x0000, 0x0400, CRC(d1e4b3c6) SHA1(3667b97c6136da4761937958f281609690af4081) )
ROM_END

ROM_START( super80d )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "super80d", "V2.2")
	ROMX_LOAD("super80d.u26", 0x0000, 0x1000, CRC(cebd2613) SHA1(87b94cc101a5948ce590211c68272e27f4cbe95a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "super80f", "MDS (original)")
	ROMX_LOAD("super80f.u26", 0x0000, 0x1000, CRC(d39775f0) SHA1(b47298ee028924612e9728bb2debd0f47399add7), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "super80g", "MDS (upgraded)")
	ROMX_LOAD("super80g.u26", 0x0000, 0x1000, CRC(7386f507) SHA1(69d7627033d62bd4e886ccc136e89f1524d38f47), ROM_BIOS(2))
	ROM_LOAD("super80.u33",   0x1000, 0x1000, CRC(cf8020a8) SHA1(2179a61f80372cd49e122ad3364773451531ae85) )
	ROM_LOAD("super80.u42",   0x2000, 0x1000, CRC(a1c6cb75) SHA1(d644ca3b399c1a8902f365c6095e0bbdcea6733b) )

	ROM_REGION(0x0800, "chargen", 0)    // 2716 eprom
	ROM_LOAD("super80d.u27",  0x0000, 0x0800, CRC(cb4c81e2) SHA1(8096f21c914fa76df5d23f74b1f7f83bd8645783) )
ROM_END

ROM_START( super80e )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("super80e.u26",  0x0000, 0x1000, CRC(bdc668f8) SHA1(3ae30b3cab599fca77d5e461f3ec1acf404caf07) )
	ROM_LOAD("super80.u33",   0x1000, 0x1000, CRC(cf8020a8) SHA1(2179a61f80372cd49e122ad3364773451531ae85) )
	ROM_LOAD("super80.u42",   0x2000, 0x1000, CRC(a1c6cb75) SHA1(d644ca3b399c1a8902f365c6095e0bbdcea6733b) )

	ROM_REGION(0x1000, "chargen", 0)    // 2732 eprom
	ROM_LOAD("super80e.u27",  0x0000, 0x1000, CRC(ebe763a7) SHA1(ffaa6d6a2c5dacc5a6651514e6707175a32e83e8) )
ROM_END

ROM_START( super80m )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "8r0", "8R0")
	ROMX_LOAD("s80-8r0.u26",  0x0000, 0x1000, CRC(48d410d8) SHA1(750d984abc013a3344628300288f6d1ba140a95f), ROM_BIOS(0) )
	ROMX_LOAD("s80-8r0.u33",  0x1000, 0x1000, CRC(9765793e) SHA1(4951b127888c1f3153004cc9fb386099b408f52c), ROM_BIOS(0) )
	ROMX_LOAD("s80-8r0.u42",  0x2000, 0x1000, CRC(5f65d94b) SHA1(fe26b54dec14e1c4911d996c9ebd084a38dcb691), ROM_BIOS(0) )
#if 0
	/* Temporary patch to fix crash when lprinting a tab */
	ROM_FILL(0x0c44,1,0x46)
	ROM_FILL(0x0c45,1,0xc5)
	ROM_FILL(0x0c46,1,0x06)
	ROM_FILL(0x0c47,1,0x20)
	ROM_FILL(0x0c48,1,0xcd)
	ROM_FILL(0x0c49,1,0xc7)
	ROM_FILL(0x0c4a,1,0xcb)
	ROM_FILL(0x0c4b,1,0xc1)
	ROM_FILL(0x0c4c,1,0x10)
	ROM_FILL(0x0c4d,1,0xf7)
	ROM_FILL(0x0c4e,1,0x00)
	ROM_FILL(0x0c4f,1,0x00)
#endif
	ROM_SYSTEM_BIOS(1, "v37", "V3.7")
	ROMX_LOAD("s80-v37.u26",  0x0000, 0x1000, CRC(46043035) SHA1(1765105df4e4af83d56cafb88e158ed462d4709e), ROM_BIOS(1) )
	ROMX_LOAD("s80-v37.u33",  0x1000, 0x1000, CRC(afb52b15) SHA1(0a2c25834074ce44bf12ac8532b4add492bcf950), ROM_BIOS(1) )
	ROMX_LOAD("s80-v37.u42",  0x2000, 0x1000, CRC(7344b27a) SHA1(f43fc47ddb5c12bffffa63488301cd5eb386cc9a), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "8r2", "8R2")
	ROMX_LOAD("s80-8r2.u26",  0x0000, 0x1000, CRC(1e166c8c) SHA1(15647614be9300cdd2956da913e83234c36b36a9), ROM_BIOS(2) )
	ROMX_LOAD("s80-8r0.u33",  0x1000, 0x1000, CRC(9765793e) SHA1(4951b127888c1f3153004cc9fb386099b408f52c), ROM_BIOS(2) )
	ROMX_LOAD("s80-8r0.u42",  0x2000, 0x1000, CRC(5f65d94b) SHA1(fe26b54dec14e1c4911d996c9ebd084a38dcb691), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(3, "8r3", "8R3")
	ROMX_LOAD("s80-8r3.u26",  0x0000, 0x1000, CRC(ee7dd90b) SHA1(c53f8eef82e8f943642f6ddfc2cb1bfdc32d25ca), ROM_BIOS(3) )
	ROMX_LOAD("s80-8r0.u33",  0x1000, 0x1000, CRC(9765793e) SHA1(4951b127888c1f3153004cc9fb386099b408f52c), ROM_BIOS(3) )
	ROMX_LOAD("s80-8r0.u42",  0x2000, 0x1000, CRC(5f65d94b) SHA1(fe26b54dec14e1c4911d996c9ebd084a38dcb691), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS(4, "8r4", "8R4")
	ROMX_LOAD("s80-8r4.u26",  0x0000, 0x1000, CRC(637d001d) SHA1(f26b5ecc33fd44b05b1f199d79e0f072ec8d0e23), ROM_BIOS(4) )
	ROMX_LOAD("s80-8r0.u33",  0x1000, 0x1000, CRC(9765793e) SHA1(4951b127888c1f3153004cc9fb386099b408f52c), ROM_BIOS(4) )
	ROMX_LOAD("s80-8r0.u42",  0x2000, 0x1000, CRC(5f65d94b) SHA1(fe26b54dec14e1c4911d996c9ebd084a38dcb691), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS(5, "8r5", "8R5")
	ROMX_LOAD("s80-8r5.u26",  0x0000, 0x1000, CRC(294f217c) SHA1(f352d54e84e94bf299726dc3af4eb7b2d06d317c), ROM_BIOS(5) )
	ROMX_LOAD("s80-8r0.u33",  0x1000, 0x1000, CRC(9765793e) SHA1(4951b127888c1f3153004cc9fb386099b408f52c), ROM_BIOS(5) )
	ROMX_LOAD("s80-8r0.u42",  0x2000, 0x1000, CRC(5f65d94b) SHA1(fe26b54dec14e1c4911d996c9ebd084a38dcb691), ROM_BIOS(5) )

	ROM_REGION(0x1800, "chargen", 0)
	ROM_LOAD("super80e.u27",  0x0000, 0x1000, CRC(ebe763a7) SHA1(ffaa6d6a2c5dacc5a6651514e6707175a32e83e8) )
	ROM_LOAD("super80d.u27",  0x1000, 0x0800, CRC(cb4c81e2) SHA1(8096f21c914fa76df5d23f74b1f7f83bd8645783) )
ROM_END

ROM_START( super80r )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "super80r", "MCE (original)")
	ROMX_LOAD("super80r.u26", 0x0000, 0x1000, CRC(01bb6406) SHA1(8e275ecf5141b93f86e45ff8a735b965ea3e8d44), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "super80s", "MCE (upgraded)")
	ROMX_LOAD("super80s.u26", 0x0000, 0x1000, CRC(3e29d307) SHA1(b3f4667633e0a4eb8577e39b5bd22e1f0bfbc0a9), ROM_BIOS(1))

	ROM_LOAD("super80.u33",   0x1000, 0x1000, CRC(cf8020a8) SHA1(2179a61f80372cd49e122ad3364773451531ae85) )
	ROM_LOAD("super80.u42",   0x2000, 0x1000, CRC(a1c6cb75) SHA1(d644ca3b399c1a8902f365c6095e0bbdcea6733b) )

	ROM_REGION(0x0800, "chargen", 0)    // 2716 eprom
	ROM_LOAD("s80hmce.ic24",  0x0000, 0x0800, CRC(a6488a1e) SHA1(7ba613d70a37a6b738dcd80c2bb9988ff1f011ef) )
ROM_END

ROM_START( super80v )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD("s80-v37v.u26",  0x0000, 0x1000, CRC(01e0c0dd) SHA1(ef66af9c44c651c65a21d5bda939ffa100078c08) )
	ROM_LOAD("s80-v37v.u33",  0x1000, 0x1000, CRC(812ad777) SHA1(04f355bea3470a7d9ea23bb2811f6af7d81dc400) )
	ROM_LOAD("s80-v37v.u42",  0x2000, 0x1000, CRC(e02e736e) SHA1(57b0264c805da99234ab5e8e028fca456851a4f9) )

	ROM_REGION(0x0800, "chargen", 0)    // 2716 eprom
	ROM_LOAD("s80hmce.ic24",  0x0000, 0x0800, CRC(a6488a1e) SHA1(7ba613d70a37a6b738dcd80c2bb9988ff1f011ef) )
ROM_END

/*    YEAR  NAME      PARENT COMPAT MACHINE   INPUT     CLASS          INIT          COMPANY                   FULLNAME */
COMP( 1981, super80,  0,       0,   super80,  super80,  super80_state,  empty_init, "Dick Smith Electronics", "Super-80 (V1.2)" , MACHINE_SUPPORTS_SAVE )
COMP( 1981, super80d, super80, 0,   super80d, super80d, super80_state,  empty_init, "Dick Smith Electronics", "Super-80 (V2.2)" , MACHINE_SUPPORTS_SAVE )
COMP( 1981, super80e, super80, 0,   super80e, super80e, super80_state,  empty_init, "Dick Smith Electronics", "Super-80 (El Graphix 4)" , MACHINE_UNOFFICIAL | MACHINE_SUPPORTS_SAVE )
COMP( 1981, super80m, super80, 0,   super80m, super80m, super80_state,  empty_init, "Dick Smith Electronics", "Super-80 (with colour)" , MACHINE_UNOFFICIAL | MACHINE_SUPPORTS_SAVE )
COMP( 1981, super80r, super80, 0,   super80r, super80r, super80r_state, empty_init, "Dick Smith Electronics", "Super-80 (with VDUEB)" , MACHINE_UNOFFICIAL | MACHINE_SUPPORTS_SAVE )
COMP( 1981, super80v, super80, 0,   super80v, super80v, super80v_state, empty_init, "Dick Smith Electronics", "Super-80 (with enhanced VDUEB)" , MACHINE_UNOFFICIAL | MACHINE_SUPPORTS_SAVE )

