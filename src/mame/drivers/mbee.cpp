// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Driver completely rewritten by Robbbert, in a process begun on 2009-02-24.
Assistance/advice was gratefully received from:
    E.J.Wordsworth (owner of Microbee Systems), nama, ChickenMan,
    and the author of the "ubee512" emulator.

Previous driver was written by Juergen Buchmueller, Jan 2000 with assistance
from Brett Selwood and Andrew Davies.


    Keyboard notes are in video/microbee.cpp

    256tc: The 1.31 rom version which appears to fit the 256TC is actually
    part of the Z80 emulation in the Matilda model. If you fit this rom into a real
    256TC, the floppy disk will not be detected.

    The unemulated Matilda is a IBM XT clone fitted with a NEC V40, and has the
    ability to emulate the 256TC as mentioned above.

    The Premium Plus was a limited-edition kit from Microbee Systems, but we don't
    have any technical info or schematic as yet. It starts up, keyboard works, disks
    work much the same as a 128k or 256tc. It has 1024k of RAM.
    The kit itself has an extra custom FPGA CPU board with memory-card slot, but there's
    no info on it yet. We just emulate the Z80 portion.

    ICs on schematics, pcbs, and manuals which never made it into production machines:
    - Z80SCC;
    - SN76489A;
    - 2651A;
    - B & C roms on disk-based machines.

    Floppy formats:
    - All disks are the standard CPCEMU 'dsk' format.
    - Types are 9/13cm 40/80 track (single or double sided)
    - 13cm has been specified as QD to prevent a nasty crash if an 80-track disk was mounted
    - The tracks/sector layout is the same regardless of size
    - Although certain models came with particular drives as standard, users could add
      the other size if they wished. We support both sizes on any model.

    Early machines have 'standard' video (128 hires characters).
    Later machines had the option of 'premium' video which
    provides thousands of hires characters, enough to simulate
    bit-mapped graphics.

    Commands to call up built-in roms (depends on the model):
    NET - Jump to E000, usually the Telcom communications program.
          This rom can be replaced with the Dreamdisk Chip-8 rom.
        Note that Telcom 3.21 is 8k, it uses a rombank switch
        (by reading port 0A) to swap between the two halves.
        See Telcom notes below.

    MEM - same as NET.

    EDASM - Jump to C000, usually the Editor/Assembler package.

    MENU - Do a rombank switch to bank 5 and jump to C000 to start the Shell

    PAK n - Do a rombank switch (write to port 0A) to bank "n" and jump to C000.

    These early colour computers have a PROM to create the foreground palette.

    Notes about the printer:
    - Older models default to a 1200 baud serial printer, which we do not support.
    - You need to change it to parallel by entering OUTL#1 while in Basic.
    - After you mount/create a printfile, you can LPRINT and LLIST in Basic,
      or by using the printing option in other apps.

    Notes about Telcom:
    - On the older models, Telcom is called up by entering NET from within Basic. Models
      from the pc85 onwards have it as a menu option.
    - To exit, press Enter without any input. Disk versions, enter CPM or press ^C.
    - After being used, version 3 and up will enable the use of OUT#7 in Basic, which
      changes the screen to 80x24. Enter OUT#0 to revert to normal.
    - Most versions of Telcom can have their parameters adjusted directly from Basic,
      without needing to enter the Telcom program.
    - Most versions of Telcom have an optional clock. In older models firstly select VS
      from the MAME config menu, then enter NET CLOCK to enable it. NET TIME hhmm to set
      the time (24hour format). NET CLOCKD is supposed to remove the status line, but it
      doesn't, although the clock stops updating. NET CLOCK and NET CLOCKD are toggles.
    - Telcom 1.2 (used in mbeeic) has a bug. If you enter NET CLOCK, the status line is
      filled with inverse K. You can fix this from Basic by doing NET CLOCK 3 times.

    Notes about Disk System
    - Ports 44 to 47 are for standard connection to FD2793.
    - Port 48 is used for drive/side/density select on write, and intrq/drq on read.
      intrq and drq are OR'd together, then gated to bit 7 of the data bus whenever
      port 48 is activated on read. There are no interrupts used.

    How to use the config switch for PIO B7:
    - Teleterm: Must use RTC. Anything else makes teleterm go crazy.
    - 256TC, 128, 128p: Doesn't seem to matter, leave at the default.
    - Standard: Has no effect, best left at "Tied High"
    - Other rom-based models: "VS" to make the Telcom clock work, or "Tied high".
    - 56k: not sure yet, leave as "Tied high" until more is known.

***************************************************************************

    TODO/not working:

    Old CRTC-based keyboard:
    - Paste drops many characters.
    - Typing can drop the occasional character.

    FDC:   (TODO: see if these bugs still exist)
    - B drive doesn't work with most disks.
    - some disks cause MAME to freeze.

    - 128k: Simply Write has no keyboard.

    - 256tc: At the menu, if F2 pressed to activate the Monitor, the emulated machine
      crashes due to a bug in z80pio emulation.

    - 256tc: the Intro disk doesn't work

    - 256tc, Teleterm: Keyboard CPU inbuilt ROM needs to be dumped.
    - 128k, 64k: PALs need to be dumped for the bankswitching.

    - Mouse: a few programs support the use of a serial mouse which interfaced
             directly to the Z80PIO. However there's little info to be found.
             PIO B3 to ground activates the mouse pointer in Shell v3.01.

    - Hard drive (10MB) & controller

*******************************************************************************/

#include "emu.h"
#include "includes/mbee.h"
#include "formats/mbee_cas.h"
#include "sound/sn76496.h"
#include "softlist_dev.h"
#include "speaker.h"

void mbee_state::mbee_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	map(0x8000, 0xbfff).rom().region("maincpu",0);
	map(0xc000, 0xdfff).r(FUNC(mbee_state::pak_r));
	map(0xe000, 0xefff).r(FUNC(mbee_state::net_r));
	map(0xf000, 0xf7ff).rw(FUNC(mbee_state::video_low_r), FUNC(mbee_state::video_low_w));
	map(0xf800, 0xffff).rw(FUNC(mbee_state::video_high_r), FUNC(mbee_state::video_high_w));
}

void mbee_state::mbeeic_mem(address_map &map)
{
	map(0x0000, 0x7fff).ram();
	map(0x8000, 0xbfff).rom().region("maincpu",0);
	map(0xc000, 0xdfff).r(FUNC(mbee_state::pak_r));
	map(0xe000, 0xefff).r(FUNC(mbee_state::net_r));
	map(0xf000, 0xf7ff).rw(FUNC(mbee_state::video_low_r), FUNC(mbee_state::video_low_w));
	map(0xf800, 0xffff).rw(FUNC(mbee_state::video_high_r), FUNC(mbee_state::video_high_w));
}

void mbee_state::mbeeppc_mem(address_map &map)
{
	map(0x0000, 0x7fff).ram();
	map(0x8000, 0x9fff).bankr("basic");
	map(0xa000, 0xbfff).rom().region("maincpu",0);
	map(0xc000, 0xdfff).r(FUNC(mbee_state::pak_r));
	map(0xe000, 0xefff).r(FUNC(mbee_state::net_r));
	map(0xf000, 0xf7ff).rw(FUNC(mbee_state::video_low_r), FUNC(mbee_state::video_low_w));
	map(0xf800, 0xffff).rw(FUNC(mbee_state::video_high_r), FUNC(mbee_state::video_high_w));
}

void mbee_state::mbeett_mem(address_map &map)
{
	map(0x0000, 0x7fff).ram();
	map(0x8000, 0x9fff).rom().region("maincpu",0);
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).r(FUNC(mbee_state::pak_r));
	map(0xe000, 0xefff).r(FUNC(mbee_state::net_r));
	map(0xf000, 0xf7ff).rw(FUNC(mbee_state::video_low_r), FUNC(mbee_state::video_low_w));
	map(0xf800, 0xffff).rw(FUNC(mbee_state::video_high_r), FUNC(mbee_state::video_high_w));
}

void mbee_state::mbee56_mem(address_map &map)
{
	map(0x0000, 0xdfff).ram();
	map(0xe000, 0xefff).rom().region("maincpu",0);
	map(0xf000, 0xf7ff).rw(FUNC(mbee_state::video_low_r), FUNC(mbee_state::video_low_w));
	map(0xf800, 0xffff).rw(FUNC(mbee_state::video_high_r), FUNC(mbee_state::video_high_w));
}

void mbee_state::mbee256_mem(address_map &map)
{
	map(0x0000, 0x0fff).bankr("bankr0").bankw("bankw0");
	map(0x1000, 0x1fff).bankr("bankr1").bankw("bankw1");
	map(0x2000, 0x2fff).bankr("bankr2").bankw("bankw2");
	map(0x3000, 0x3fff).bankr("bankr3").bankw("bankw3");
	map(0x4000, 0x4fff).bankr("bankr4").bankw("bankw4");
	map(0x5000, 0x5fff).bankr("bankr5").bankw("bankw5");
	map(0x6000, 0x6fff).bankr("bankr6").bankw("bankw6");
	map(0x7000, 0x7fff).bankr("bankr7").bankw("bankw7");
	map(0x8000, 0x8fff).bankr("bankr8").bankw("bankw8");
	map(0x9000, 0x9fff).bankr("bankr9").bankw("bankw9");
	map(0xa000, 0xafff).bankr("bankr10").bankw("bankw10");
	map(0xb000, 0xbfff).bankr("bankr11").bankw("bankw11");
	map(0xc000, 0xcfff).bankr("bankr12").bankw("bankw12");
	map(0xd000, 0xdfff).bankr("bankr13").bankw("bankw13");
	map(0xe000, 0xefff).bankr("bankr14").bankw("bankw14");
	map(0xf000, 0xffff).bankr("bankr15").bankw("bankw15");
}

void mbee_state::mbee_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).mirror(0x10).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0b, 0x0b).mirror(0x10).w(FUNC(mbee_state::port0b_w));
	map(0x0c, 0x0c).mirror(0x10).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(mbee_state::m6545_index_w));
	map(0x0d, 0x0d).mirror(0x10).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(mbee_state::m6545_data_w));
}

void mbee_state::mbeeic_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0003).mirror(0xff10).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0008, 0x0008).mirror(0xff10).rw(FUNC(mbee_state::port08_r), FUNC(mbee_state::port08_w));
	map(0x0009, 0x0009).mirror(0xff00).nopw();
	map(0x000a, 0x000a).select(0xff10).rw(FUNC(mbee_state::telcom_r), FUNC(mbee_state::port0a_w));
	map(0x000b, 0x000b).mirror(0xff10).w(FUNC(mbee_state::port0b_w));
	map(0x000c, 0x000c).mirror(0xff10).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(mbee_state::m6545_index_w));
	map(0x000d, 0x000d).mirror(0xff10).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(mbee_state::m6545_data_w));
}

void mbee_state::mbeeppc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0003).mirror(0xff10).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0008, 0x0008).mirror(0xff10).rw(FUNC(mbee_state::port08_r), FUNC(mbee_state::port08_w));
	map(0x0009, 0x0009).mirror(0xff00).nopw();
	map(0x000a, 0x000a).select(0xff10).rw(FUNC(mbee_state::telcom_r), FUNC(mbee_state::port0a_w));
	map(0x000b, 0x000b).mirror(0xff10).w(FUNC(mbee_state::port0b_w));
	map(0x000c, 0x000c).mirror(0xff00).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(mbee_state::m6545_index_w));
	map(0x000d, 0x000d).mirror(0xff10).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(mbee_state::m6545_data_w));
	map(0x001c, 0x001c).mirror(0xff00).rw(FUNC(mbee_state::port1c_r), FUNC(mbee_state::port1c_w));
}

void mbee_state::mbeett_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0003).mirror(0xff00).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0004, 0x0004).mirror(0xff00).w(FUNC(mbee_state::port04_w));
	map(0x0006, 0x0006).mirror(0xff00).w(FUNC(mbee_state::port06_w));
	map(0x0007, 0x0007).mirror(0xff00).r(FUNC(mbee_state::port07_r));
	map(0x0008, 0x0008).mirror(0xff00).rw(FUNC(mbee_state::port08_r), FUNC(mbee_state::port08_w));
	map(0x000a, 0x000a).select(0xff10).rw(FUNC(mbee_state::telcom_r), FUNC(mbee_state::port0a_w));
	map(0x000b, 0x000b).mirror(0xff00).w(FUNC(mbee_state::port0b_w));
	map(0x000c, 0x000c).mirror(0xff00).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(mbee_state::m6545_index_w));
	map(0x000d, 0x000d).mirror(0xff00).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(mbee_state::m6545_data_w));
	map(0x0018, 0x001b).mirror(0xff00).r(FUNC(mbee_state::port18_r));
	map(0x001c, 0x001f).mirror(0xff00).rw(FUNC(mbee_state::port1c_r), FUNC(mbee_state::port1c_w));
	map(0x0009, 0x0009).select(0xff00).r(FUNC(mbee_state::speed_r));
	map(0x0068, 0x006f).mirror(0xff00).noprw();    // swallow i/o to SCC which was never fitted to production machines
}

void mbee_state::mbee56_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x08, 0x08).rw(FUNC(mbee_state::port08_r), FUNC(mbee_state::port08_w));
	map(0x09, 0x09).nopw();
	map(0x0b, 0x0b).w(FUNC(mbee_state::port0b_w));
	map(0x0c, 0x0c).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(mbee_state::m6545_index_w));
	map(0x0d, 0x0d).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(mbee_state::m6545_data_w));
	map(0x44, 0x47).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x48, 0x4f).rw(FUNC(mbee_state::fdc_status_r), FUNC(mbee_state::fdc_motor_w));
}

void mbee_state::mbee128_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x04, 0x04).w(FUNC(mbee_state::port04_w));
	map(0x06, 0x06).w(FUNC(mbee_state::port06_w));
	map(0x07, 0x07).r(FUNC(mbee_state::port07_r));
	map(0x08, 0x08).rw(FUNC(mbee_state::port08_r), FUNC(mbee_state::port08_w));
	map(0x09, 0x09).nopw();
	map(0x0b, 0x0b).w(FUNC(mbee_state::port0b_w));
	map(0x0c, 0x0c).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(mbee_state::m6545_index_w));
	map(0x0d, 0x0d).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(mbee_state::m6545_data_w));
	map(0x1c, 0x1f).rw(FUNC(mbee_state::port1c_r), FUNC(mbee_state::port1c_w));
	map(0x44, 0x47).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x48, 0x4f).rw(FUNC(mbee_state::fdc_status_r), FUNC(mbee_state::fdc_motor_w));
	map(0x50, 0x57).w(FUNC(mbee_state::port50_w));
}

void mbee_state::mbee128p_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	mbee128_io(map);
	map(0x10, 0x13).w("sn1", FUNC(sn76489a_device::write));
}

void mbee_state::mbee256_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0003).mirror(0xff00).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0004, 0x0004).mirror(0xff00).w(FUNC(mbee_state::port04_w));
	map(0x0006, 0x0006).mirror(0xff00).w(FUNC(mbee_state::port06_w));
	map(0x0007, 0x0007).mirror(0xff00).r(FUNC(mbee_state::port07_r));
	map(0x0008, 0x0008).mirror(0xff00).rw(FUNC(mbee_state::port08_r), FUNC(mbee_state::port08_w));
	map(0x0009, 0x0009).select(0xff00).r(FUNC(mbee_state::speed_r));
	map(0x0009, 0x0009).mirror(0xff00).nopw();
	map(0x000b, 0x000b).mirror(0xff00).w(FUNC(mbee_state::port0b_w));
	map(0x000c, 0x000c).mirror(0xff00).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(mbee_state::m6545_index_w));
	map(0x000d, 0x000d).mirror(0xff00).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(mbee_state::m6545_data_w));
	map(0x0010, 0x0013).mirror(0xff00).w("sn1", FUNC(sn76489a_device::write));
	map(0x0018, 0x001b).mirror(0xff00).r(FUNC(mbee_state::port18_r));
	map(0x001c, 0x001f).mirror(0xff00).rw(FUNC(mbee_state::port1c_r), FUNC(mbee_state::port1c_w));
	map(0x0044, 0x0047).mirror(0xff00).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x0048, 0x004f).mirror(0xff00).rw(FUNC(mbee_state::fdc_status_r), FUNC(mbee_state::fdc_motor_w));
	map(0x0050, 0x0057).mirror(0xff00).w(FUNC(mbee_state::port50_w));
	// map(0x0058, 0x005f).mirror(0xff00); External options: floppy drive, hard drive and keyboard
	// map(0x0060, 0x0067).mirror(0xff00); Reserved for file server selection (unused)
	// map(0x0068, 0x006f).mirror(0xff00); Reserved for 8530 SCC (never used)
}

static INPUT_PORTS_START( oldkb )
	PORT_START("X.0") /* IN0 KEY ROW 0 [000] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)

	PORT_START("X.1") /* IN1 KEY ROW 1 [080] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f)

	PORT_START("X.2") /* IN2 KEY ROW 2 [100] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)

	PORT_START("X.3") /* IN3 KEY ROW 3 [180] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHAR(0x1b)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("^") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~') PORT_CHAR(0x1e)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL) PORT_CHAR(8) PORT_CHAR(0x5f) PORT_CHAR(0x1f)  // port_char not working - hijacked

	PORT_START("X.4") /* IN4 KEY ROW 4 [200] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 '") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("X.5") /* IN5 KEY ROW 5 [280] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(": *") PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("; +") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X.6") /* IN6 KEY ROW 6 [300] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Linefeed") PORT_CODE(KEYCODE_HOME) PORT_CHAR(10)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_END) PORT_CHAR(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("X.7") /* IN7 KEY ROW 7 [380] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("(Up)") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("(Down)") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("(Left)") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("(Right)") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

static INPUT_PORTS_START( mbee )

	PORT_INCLUDE( oldkb )

	// Autorun on quickload
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Autorun on Quickload")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x01, DEF_STR(Yes))
	// monochrome monitor could be used
	PORT_CONFNAME( 0x30, 0x00, "Monitor type")
	PORT_CONFSETTING(    0x00, "Colour")
	PORT_CONFSETTING(    0x10, "Green")
	PORT_CONFSETTING(    0x20, "Amber")
	PORT_CONFSETTING(    0x30, "White")
	// Wire links on motherboard
	PORT_CONFNAME( 0xc0, 0x00, "PIO B7")
	PORT_CONFSETTING(    0x00, "VS") // sync pulse to enable telcom clock
	PORT_CONFSETTING(    0x40, "RTC") // optional board usually not fitted
	PORT_CONFSETTING(    0x80, "Tied high") // default resistor to vcc
	PORT_CONFSETTING(    0xc0, "Reserved for net")
INPUT_PORTS_END

static INPUT_PORTS_START( mbee128 )

	PORT_INCLUDE( oldkb )

	// Autorun on quickload
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Autorun on Quickload")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x01, DEF_STR(Yes))
	// monochrome monitor could be used
	PORT_CONFNAME( 0x30, 0x00, "Monitor type")
	PORT_CONFSETTING(    0x00, "Colour")
	PORT_CONFSETTING(    0x10, "Green")
	PORT_CONFSETTING(    0x20, "Amber")
	PORT_CONFSETTING(    0x30, "White")
	// Wire links on motherboard
	PORT_CONFNAME( 0xc0, 0x80, "PIO B7")
	PORT_CONFSETTING(    0x00, "VS") // sync pulse to enable telcom clock
	PORT_CONFSETTING(    0x40, "RTC") // RTC IRQ for clock
	PORT_CONFSETTING(    0x80, "Tied high") // default resistor to vcc
	PORT_CONFSETTING(    0xc0, "Reserved for net")
INPUT_PORTS_END

static INPUT_PORTS_START( mbee256 )
	PORT_START("Y.0") /* IN0 KEY ROW 0 [+00] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_END) PORT_CHAR(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 (num)") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL (num)") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("Y.1") /* IN1 KEY ROW 1 [+08] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Linefeed") PORT_CODE(KEYCODE_HOME) PORT_CHAR(10)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Insert") PORT_CODE(KEYCODE_INSERT)

	PORT_START("Y.2") /* IN2 KEY ROW 2 [+10] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+ (num)") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 (num)") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 (num)") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)

	PORT_START("Y.3") /* IN3 KEY ROW 3 [+18] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("- (num)") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 (num)") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 (num)") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)

	PORT_START("Y.4") /* IN4 KEY ROW 4 [+20] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("* (num)") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 (num)") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 (num)") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)

	PORT_START("Y.5") /* IN5 KEY ROW 5 [+28] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 (num)") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 (num)") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 (num)") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)

	PORT_START("Y.6") /* IN6 KEY ROW 6 [+30] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/ (num)") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("(Down)") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("(Right)") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)

	PORT_START("Y.7") /* IN7 KEY ROW 7 [+38] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("(Left)") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)

	PORT_START("Y.8") /* IN0 KEY ROW 0 [+40] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("(Up)") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d)

	PORT_START("Y.9") /* IN1 KEY ROW 1 [+48] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F10") PORT_CODE(KEYCODE_F10)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("Y.10") /* IN2 KEY ROW 2 [+50] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F11") PORT_CODE(KEYCODE_F11)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL) PORT_CHAR(127)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("Y.11") /* IN3 KEY ROW 3 [+58] */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F12") PORT_CODE(KEYCODE_F12)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHAR(0x1b)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("Y.12") /* IN4 KEY ROW 4 [+60] */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y.13") /* IN5 KEY ROW 5 [+68] */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("Y.14") /* IN6 KEY ROW 6 [+70] */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)

	// Autorun on quickload
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Autorun on Quickload")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x01, DEF_STR(Yes))
	// Wire links on motherboard
	PORT_CONFNAME( 0xc0, 0x80, "PIO B7")
	PORT_CONFSETTING(    0x00, "VS") // sync pulse to enable telcom clock
	PORT_CONFSETTING(    0x40, "RTC") // RTC IRQ must be used on teleterm
	PORT_CONFSETTING(    0x80, "Tied high") // default resistor to vcc
	PORT_CONFSETTING(    0xc0, "Reserved for net")
INPUT_PORTS_END

static const z80_daisy_config mbee_daisy_chain[] =
{
	{ "z80pio" },
	{ nullptr }
};

/**************************** F4 CHARACTER DISPLAYER */
static const gfx_layout charlayout =
{
	8,16,                   /* 8 x 16 characters */
	RGN_FRAC(1,1),
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_mono )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 96, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_standard )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 48 )
GFXDECODE_END

static GFXDECODE_START( gfx_premium )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 8 )
GFXDECODE_END

static void mbee_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}


void mbee_state::remove_carts(machine_config &config)
{
	config.device_remove("cart_list");
	config.device_remove("optrom1");
	config.device_remove("optrom2");
	config.device_remove("optrom3");
	config.device_remove("optrom4");
	config.device_remove("optrom5");
	config.device_remove("optrom6");
	config.device_remove("optrom7");
}

void mbee_state::remove_quick(machine_config &config)
{
	config.device_remove("cass_list");
	config.device_remove("quickload");
	config.device_remove("quik_list");
}

void mbee_state::mbee(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12_MHz_XTAL / 6);         /* 2 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mbee_state::mbee_mem);
	m_maincpu->set_addrmap(AS_IO, &mbee_state::mbee_io);
	m_maincpu->set_daisy_config(mbee_daisy_chain);

	Z80PIO(config, m_pio, 12_MHz_XTAL / 6);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_pio->out_ardy_callback().set(FUNC(mbee_state::pio_ardy));
	m_pio->in_pb_callback().set(FUNC(mbee_state::pio_port_b_r));
	m_pio->out_pb_callback().set(FUNC(mbee_state::pio_port_b_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(250)); /* not accurate */
	m_screen->set_size(64*8, 19*16);           /* need at least 17 lines for NET */
	m_screen->set_visarea(0*8, 64*8-1, 0, 19*16-1);
	m_screen->set_screen_update(FUNC(mbee_state::screen_update_mbee));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_mono);

	PALETTE(config, m_palette, FUNC(mbee_state::standard_palette), 100);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	SY6545_1(config, m_crtc, 12_MHz_XTAL / 8);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(mbee_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(mbee_state::crtc_update_addr));
	m_crtc->out_vsync_callback().set(FUNC(mbee_state::crtc_vs));

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "mwb,com,bee,bin", attotime::from_seconds(3)));
	quickload.set_load_callback(FUNC(mbee_state::quickload_cb));
	quickload.set_interface("mbee_quik");

	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->ack_handler().set(m_pio, FUNC(z80pio_device::strobe_a));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(mbee_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("mbee_cass");

	generic_slot_device &optrom1(GENERIC_SOCKET(config, "optrom1", generic_plain_slot, "mbee_net", "mbn")); // net
	optrom1.set_device_load(FUNC(mbee_state::pak_load<1U>));
	optrom1.set_device_unload(FUNC(mbee_state::pak_unload<1U>));
	generic_slot_device &optrom2(GENERIC_SOCKET(config, "optrom2", generic_plain_slot, "mbee_pak", "mbp")); // edasm
	optrom2.set_device_load(FUNC(mbee_state::pak_load<2U>));
	optrom2.set_device_unload(FUNC(mbee_state::pak_unload<2U>));

	SOFTWARE_LIST(config, "cass_list").set_original("mbee_cass").set_filter("1");
	SOFTWARE_LIST(config, "quik_list").set_original("mbee_quik").set_filter("1");
	SOFTWARE_LIST(config, "cart_list").set_original("mbee_cart").set_filter("1");
}


void mbee_state::mbeeic(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 13.5_MHz_XTAL / 4);         /* 3.37500 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mbee_state::mbeeic_mem);
	m_maincpu->set_addrmap(AS_IO, &mbee_state::mbeeic_io);
	m_maincpu->set_daisy_config(mbee_daisy_chain);

	Z80PIO(config, m_pio, 13.5_MHz_XTAL / 4);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_pio->out_ardy_callback().set(FUNC(mbee_state::pio_ardy));
	m_pio->in_pb_callback().set(FUNC(mbee_state::pio_port_b_r));
	m_pio->out_pb_callback().set(FUNC(mbee_state::pio_port_b_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(250)); /* not accurate */
	m_screen->set_size(80*8, 310);
	m_screen->set_visarea(0, 80*8-1, 0, 19*16-1);
	m_screen->set_screen_update(FUNC(mbee_state::screen_update_mbee));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_standard);

	PALETTE(config, m_palette, FUNC(mbee_state::standard_palette), 100);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	SY6545_1(config, m_crtc, 13.5_MHz_XTAL / 8);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(mbee_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(mbee_state::crtc_update_addr));
	m_crtc->out_vsync_callback().set(FUNC(mbee_state::crtc_vs));

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "mwb,com,bee,bin", attotime::from_seconds(3)));
	quickload.set_load_callback(FUNC(mbee_state::quickload_cb));
	quickload.set_interface("mbee_quik");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_pio, FUNC(z80pio_device::strobe_a));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(mbee_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("mbee_cass");

	generic_slot_device &optrom1(GENERIC_SOCKET(config, "optrom1", generic_plain_slot, "mbee_net", "mbn")); // net
	optrom1.set_device_load(FUNC(mbee_state::pak_load<1U>));
	optrom1.set_device_unload(FUNC(mbee_state::pak_unload<1U>));
	generic_slot_device &optrom2(GENERIC_SOCKET(config, "optrom2", generic_plain_slot, "mbee_pak", "mbp")); // pak0
	optrom2.set_device_load(FUNC(mbee_state::pak_load<2U>));
	optrom2.set_device_unload(FUNC(mbee_state::pak_unload<2U>));
	generic_slot_device &optrom3(GENERIC_SOCKET(config, "optrom3", generic_plain_slot, "mbee_pak", "mbp")); // pak1
	optrom3.set_device_load(FUNC(mbee_state::pak_load<3U>));
	optrom3.set_device_unload(FUNC(mbee_state::pak_unload<3U>));
	generic_slot_device &optrom4(GENERIC_SOCKET(config, "optrom4", generic_plain_slot, "mbee_pak", "mbp")); // pak2
	optrom4.set_device_load(FUNC(mbee_state::pak_load<4U>));
	optrom4.set_device_unload(FUNC(mbee_state::pak_unload<4U>));
	generic_slot_device &optrom5(GENERIC_SOCKET(config, "optrom5", generic_plain_slot, "mbee_pak", "mbp")); // pak3
	optrom5.set_device_load(FUNC(mbee_state::pak_load<5U>));
	optrom5.set_device_unload(FUNC(mbee_state::pak_unload<5U>));
	generic_slot_device &optrom6(GENERIC_SOCKET(config, "optrom6", generic_plain_slot, "mbee_pak", "mbp")); // pak4
	optrom6.set_device_load(FUNC(mbee_state::pak_load<6U>));
	optrom6.set_device_unload(FUNC(mbee_state::pak_unload<6U>));
	generic_slot_device &optrom7(GENERIC_SOCKET(config, "optrom7", generic_plain_slot, "mbee_pak", "mbp")); // pak5
	optrom7.set_device_load(FUNC(mbee_state::pak_load<7U>));
	optrom7.set_device_unload(FUNC(mbee_state::pak_unload<7U>));

	SOFTWARE_LIST(config, "cass_list").set_original("mbee_cass").set_filter("2");
	SOFTWARE_LIST(config, "quik_list").set_original("mbee_quik").set_filter("2");
	SOFTWARE_LIST(config, "cart_list").set_original("mbee_cart").set_filter("2");
}

void mbee_state::mbeepc85(machine_config &config)
{
	mbeeic(config);
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("mbee_cart").set_filter("3");
}

void mbee_state::mbeeppc(machine_config &config)
{
	mbeepc85(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mbee_state::mbeeppc_mem);
	m_maincpu->set_addrmap(AS_IO, &mbee_state::mbeeppc_io);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_premium);
	m_palette->set_init(FUNC(mbee_state::premium_palette));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(FUNC(mbee_state::rtc_irq_w));
}

void mbee_state::mbee56(machine_config &config)
{
	mbeeic(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mbee_state::mbee56_mem);
	m_maincpu->set_addrmap(AS_IO, &mbee_state::mbee56_io);

	WD2793(config, m_fdc, 4_MHz_XTAL / 2);
	m_fdc->intrq_wr_callback().set(FUNC(mbee_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(mbee_state::fdc_drq_w));
	m_fdc->enmf_rd_callback().set_constant(0);
	FLOPPY_CONNECTOR(config, m_floppy0, mbee_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, mbee_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("mbee_flop").set_filter("1");
	remove_carts(config);
	remove_quick(config);
}

void mbee_state::mbee128(machine_config &config)
{
	mbee56(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mbee_state::mbee256_mem);
	m_maincpu->set_addrmap(AS_IO, &mbee_state::mbee128_io);

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(FUNC(mbee_state::rtc_irq_w));

	SOFTWARE_LIST(config.replace(), "flop_list").set_original("mbee_flop").set_filter("2");
}

void mbee_state::mbee128p(machine_config &config)
{
	mbeeppc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mbee_state::mbee256_mem);
	m_maincpu->set_addrmap(AS_IO, &mbee_state::mbee128p_io);

	WD2793(config, m_fdc, 4_MHz_XTAL / 2);
	m_fdc->intrq_wr_callback().set(FUNC(mbee_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(mbee_state::fdc_drq_w));
	m_fdc->enmf_rd_callback().set_constant(0);
	FLOPPY_CONNECTOR(config, m_floppy0, mbee_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, mbee_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	SN76489A(config, "sn1", 13.5_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.50);

	SOFTWARE_LIST(config, "flop_list").set_original("mbee_flop").set_filter("3");
	remove_carts(config);
	remove_quick(config);
}

void mbee_state::mbeepp(machine_config &config)
{
	mbee128p(config);
	SOFTWARE_LIST(config.replace(), "flop_list").set_original("mbee_flop").set_filter("2,3");
}

void mbee_state::mbee256(machine_config &config)
{
	mbee128p(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mbee_state::mbee256_mem);
	m_maincpu->set_addrmap(AS_IO, &mbee_state::mbee256_io);

	TIMER(config, "newkb_timer").configure_periodic(FUNC(mbee_state::newkb_timer), attotime::from_hz(50));

	SOFTWARE_LIST(config.replace(), "flop_list").set_original("mbee_flop").set_filter("4");
}

void mbee_state::mbeett(machine_config &config)
{
	mbeeppc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mbee_state::mbeett_mem);
	m_maincpu->set_addrmap(AS_IO, &mbee_state::mbeett_io);
	TIMER(config, "newkb_timer").configure_periodic(FUNC(mbee_state::newkb_timer), attotime::from_hz(50));
	remove_quick(config);
	config.device_remove("optrom3");
	config.device_remove("optrom4");
	config.device_remove("optrom5");
	config.device_remove("optrom6");
	config.device_remove("optrom7");
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("mbee_cart").set_filter("TT");
}

// This represents the Series 1: Kit computer, 16K, 32K, 16K Plus, and 32K plus.
ROM_START( mbee )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD("bas510a.ic25",          0x0000,  0x1000, CRC(2ca47c36) SHA1(f36fd0afb3f1df26edc67919e78000b762b6cbcb) )
	ROM_LOAD("bas510b.ic27",          0x1000,  0x1000, CRC(a07a0c51) SHA1(dcbdd9df78b4b6b2972de2e4050dabb8ae9c3f5a) )
	ROM_LOAD("bas510c.ic28",          0x2000,  0x1000, CRC(906ac00f) SHA1(9b46458e5755e2c16cdb191a6a70df6de9fe0271) )
	ROM_LOAD("bas510d.ic30",          0x3000,  0x1000, CRC(61727323) SHA1(c0fea9fd0e25beb9faa7424db8efd07cf8d26c1b) )

	// first 0x800 for normal chars, 2nd 0x800 for small chars. Some roms don't have small chars so normal ones loaded twice.
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.ic13",          0x0000,  0x0800, CRC(b149737b) SHA1(a3cd4f5d0d3c71137cd1f0f650db83333a2e3597) )
	ROM_RELOAD( 0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0000,  0x0020, CRC(79fa1e9d) SHA1(0454051697b23e4561744466fb31e7a133d02246) ) // video switching prom, not needed for emulation purposes
ROM_END

// This represents the Series 1: IC (Integrated Computer), and the Series 2: Experimenter, the Educator, and the Personal Communicator
ROM_START( mbeeic )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "522e", "Basic 5.22e" )
	ROMX_LOAD("bas522e.ic5",           0x0000,  0x2000, CRC(7896a696) SHA1(a158f7803296766160e1f258dfc46134735a9477), ROM_BIOS(0) )
	ROMX_LOAD("bas522e.ic10",          0x2000,  0x2000, CRC(b21d9679) SHA1(332844433763331e9483409cd7da3f90ac58259d), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "522d", "Basic 5.22d" )
	ROMX_LOAD("bas522d.ic5",           0x0000,  0x2000, CRC(7896a696) SHA1(a158f7803296766160e1f258dfc46134735a9477), ROM_BIOS(1) )
	ROMX_LOAD("bas522d.ic10",          0x2000,  0x2000, CRC(523a38ff) SHA1(a5383067bc712123849710d8b69cbd879d17a61f), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "netdef", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL( "telcom10.mbn", 0x0000, 0x1000, CRC(d1617e4f) SHA1(c73dc4dcf4c69419842fa4b52aa92e86924a2e2b) ) // net

	/* PAK option roms */
	ROM_REGION( 0x20000, "pakdef", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL("wbee12.mbp",   0x0000,  0x2000, CRC(0fc21cb5) SHA1(33b3995988fc51ddef1568e160dfe699867adbd5) ) // pak0
	ROM_LOAD_OPTIONAL("help1.mbp",    0x2000,  0x2000, CRC(d34fae54) SHA1(5ed30636f48e9d208ce2da367ba4425782a5bce3) ) // pak1

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(79fa1e9d) SHA1(0454051697b23e4561744466fb31e7a133d02246) ) // video switching prom, not needed for emulation purposes
ROM_END

// This represents the Series 3: 16K Educator, and the 32K Communicator
ROM_START( mbeepc )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "522e", "Basic 5.22e" )
	ROMX_LOAD("bas522e.ic5",           0x0000,  0x2000, CRC(7896a696) SHA1(a158f7803296766160e1f258dfc46134735a9477), ROM_BIOS(0) )
	ROMX_LOAD("bas522e.ic10",          0x2000,  0x2000, CRC(b21d9679) SHA1(332844433763331e9483409cd7da3f90ac58259d), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "524e", "Basic 5.24e" )
	ROMX_LOAD("bas524e.ic5",           0x0000,  0x2000, CRC(ec9c7a60) SHA1(a4021bcedc8da8c0eb0bda036a1d457619a175b0), ROM_BIOS(1) )
	ROMX_LOAD("bas524e.ic10",          0x2000,  0x2000, CRC(9621cfc8) SHA1(81ab332d366466ae84cff2e8b8596dd86c6b6f63), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "netdef", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL( "telcom10.mbn", 0x0000, 0x1000, CRC(d1617e4f) SHA1(c73dc4dcf4c69419842fa4b52aa92e86924a2e2b) ) // net

	/* PAK option roms */
	ROM_REGION( 0x20000, "pakdef", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL("wbee12.mbp",   0x0000,  0x2000, CRC(0fc21cb5) SHA1(33b3995988fc51ddef1568e160dfe699867adbd5) ) // pak0
	ROM_LOAD_OPTIONAL("help1.mbp",    0x2000,  0x2000, CRC(d34fae54) SHA1(5ed30636f48e9d208ce2da367ba4425782a5bce3) ) // pak1

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(79fa1e9d) SHA1(0454051697b23e4561744466fb31e7a133d02246) ) // video switching prom, not needed for emulation purposes
ROM_END

ROM_START( mbeepc85 )  // The PC85 with the earlier menu
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("bas525a.rom",           0x0000,  0x2000, CRC(a6e02afe) SHA1(0495308c7e1d84b5989a3af6d3b881f4580b2641) )
	ROM_LOAD("bas525b.rom",           0x2000,  0x2000, CRC(245dd36b) SHA1(dd288f3e6737627f50d3d2a49df3e57c423d3118) )

	ROM_REGION( 0x2000, "netdef", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL("telcom321a.mbn", 0x0000,  0x2000, CRC(36852a11) SHA1(c45b8d03629e86231c6b256a7435abd87d8872a4) )

	// PAK option roms - Wordbee must be in slot 0 and Shell must be in slot 5.
	ROM_REGION( 0x20000, "pakdef", ROMREGION_ERASEFF )
	ROM_LOAD("wbee13r3.mbp",          0x0000,  0x2000, CRC(d7c58b7b) SHA1(5af1b8d21a0f21534ed1833ae919dbbc6ca973e2) ) // 0
	ROM_LOAD("help2.mbp",             0x2000,  0x2000, CRC(a4f1fa90) SHA1(1456abc6ed0501a3b15a99b4302750843293ae5f) ) // 1
	ROM_LOAD("shell.mbp",             0xa000,  0x2000, CRC(5a2c7cd6) SHA1(8edc086710cb558f2146d660eddc8a18ba6a141c) ) // 5

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(79fa1e9d) SHA1(0454051697b23e4561744466fb31e7a133d02246) ) // video switching prom, not needed for emulation purposes
ROM_END

ROM_START( mbeepc85b ) // The PC85 with the later menu
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("bas525a.rom",           0x0000,  0x2000, CRC(a6e02afe) SHA1(0495308c7e1d84b5989a3af6d3b881f4580b2641) )
	ROM_LOAD("bas525b.rom",           0x2000,  0x2000, CRC(245dd36b) SHA1(dd288f3e6737627f50d3d2a49df3e57c423d3118) )

	ROM_REGION( 0x2000, "netdef", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL("telcom321a.mbn", 0x0000,  0x2000, CRC(36852a11) SHA1(c45b8d03629e86231c6b256a7435abd87d8872a4) )

	// PAK option roms - Wordbee must be in slot 0 and Shell must be in slot 5.
	ROM_REGION( 0x20000, "pakdef", ROMREGION_ERASEFF )
	ROM_LOAD("wbee13r3.mbp",          0x0000,  0x2000, CRC(d7c58b7b) SHA1(5af1b8d21a0f21534ed1833ae919dbbc6ca973e2) ) // 0
	ROM_LOAD("help2.mbp",             0x2000,  0x2000, CRC(a4f1fa90) SHA1(1456abc6ed0501a3b15a99b4302750843293ae5f) ) // 1
	ROM_LOAD("busy.mbp",              0x4000,  0x2000, CRC(56255f60) SHA1(fd2e37209fd49290be6875bc460cfc05392938ba) ) // 2
	ROM_CONTINUE( 0x14000, 0x2000 )
	ROM_LOAD("graphics.mbp",          0x6000,  0x2000, CRC(9e9d327c) SHA1(aebf60ed153004380b9f271f2212376910a6cef9) ) // 3
	ROM_CONTINUE( 0x16000, 0x2000 )
	ROM_LOAD("viatel23.mbp",          0x8000,  0x2000, CRC(2da2411f) SHA1(d3cfa978165feef0a96e28197f6a762aa6604799) ) // 4
	ROM_LOAD("shell-b.mbp",           0xa000,  0x2000, CRC(17bf2d58) SHA1(ae22a5fc5783f37066ba5555497e40945272ca3d) ) // 5

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(79fa1e9d) SHA1(0454051697b23e4561744466fb31e7a133d02246) ) // video switching prom, not needed for emulation purposes
ROM_END

ROM_START( mbeepc85s ) // The Swedish version of the PC85, with custom localised software paks
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("bas524a.rom",           0x0000,  0x2000, CRC(ec9c7a60) SHA1(a4021bcedc8da8c0eb0bda036a1d457619a175b0) )
	ROM_LOAD("bas524b.rom",           0x2000,  0x2000, CRC(17d3eac7) SHA1(d40d376cc5e751d257d951909a34445e70506c7b) )

	ROM_REGION( 0x2000, "netdef", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL("telcom321s.mbp", 0x0000, 0x2000, CRC(00f8fde1) SHA1(eb881bbab90c85fd6e29540decd25e884c67f738) )

	// PAK roms - These are not optional and will only work in the correct slots.
	ROM_REGION( 0x20000, "pakdef", ROMREGION_ERASEFF )
	ROM_LOAD("wbee20s.mbp",           0x0000,  0x2000, CRC(6a0fe57f) SHA1(a101b588b1872e19382b9e9ea50fabb0fd060aa6) ) // 0
	ROM_LOAD("db-s.mbp",              0x2000,  0x2000, CRC(e2094771) SHA1(62d7fb66c91d2bd24523bc84e4f005cf2c4480bb) ) // 1
	ROM_LOAD("kalk-s.mbp",            0x4000,  0x2000, CRC(08dd71ee) SHA1(c9d506d8bb56f602c3481b253d4cac226f545d98) ) // 2
	ROM_LOAD("bg-s.mbp",              0x6000,  0x2000, CRC(5aa4813e) SHA1(a8638e9046bfb9d5a98c878322295ce408bd879d) ) // 3
	ROM_LOAD("vtex11s.mbp",           0x8000,  0x2000, CRC(67592b3f) SHA1(7f1d23ded34781ccda5f36b4a4fa118a8c0e44ec) ) // 4
	ROM_LOAD("shell-s.mbp",           0xa000,  0x2000, CRC(bdf1768f) SHA1(4385351d07288cf94947ac63131eeed98572caa1) ) // 5

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom-s.bin",         0x0000,  0x1000, CRC(1bcbf083) SHA1(6438649b8b5fc20dd772ec7195e69a5bbe016b09) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(79fa1e9d) SHA1(0454051697b23e4561744466fb31e7a133d02246) ) // video switching prom, not needed for emulation purposes
ROM_END

ROM_START( mbeett ) // The Teleterm
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("kernel_106.rom",        0x0000,  0x2000, CRC(5ab9cb1d) SHA1(a1fb971622f85c4d866b91cb4bec6d75757e8c5f) )

	ROM_REGION( 0x2000, "netdef", ROMREGION_ERASEFF )
	ROM_LOAD("wm_106.mbn",            0x0000,  0x2000, CRC(77e0b355) SHA1(1db6769cd6b12e1c335c83f17f8c139986c87758) )

	ROM_REGION( 0x20000, "pakdef", ROMREGION_ERASEFF )
	ROM_LOAD("tv_470311.mbp",         0x2000,  0x2000, CRC(2c4c2dcb) SHA1(77cd75166a389cb2d1d8abf00b1ddd077ce98354) ) // 1
	ROM_CONTINUE( 0x12000, 0x2000 )
	ROM_LOAD("tw_103.mbp",            0x4000,  0x2000, CRC(881edb4b) SHA1(f6e30a12b1537bd55b69d1319799b150e80a471b) ) // 2
	ROM_CONTINUE( 0x14000, 0x2000 )
	ROM_LOAD("oside_107.mbp",         0x6000,  0x2000, CRC(05d99aba) SHA1(4f88d63025f99bcc54d6f2abc20a699c97384f68) ) // 3
	ROM_CONTINUE( 0x16000, 0x1000 )
	ROM_LOAD("test_105.mbp",          0x8000,  0x2000, CRC(b69aa618) SHA1(49de8cbad59f549c7ad9f8efc9beee0cfcd901fe) ) // 4

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
ROM_END

ROM_START( mbeeppc ) // The Premium PC85
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("bas529b.rom",           0x0000,  0x2000, CRC(a1bd986b) SHA1(5d79f210c9042db5aefc85a0bdf45210cb9e9899) )

	ROM_REGION( 0x4000, "basicrom", 0 )
	ROM_LOAD("bas529a.rom",           0x0000,  0x4000, CRC(fe8242e1) SHA1(ff790edf4fcc7a134d451dbad7779157b07f6abf) )

	ROM_REGION( 0x2000, "netdef", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL("telco321.rom", 0x0000,  0x2000, CRC(36852a11) SHA1(c45b8d03629e86231c6b256a7435abd87d8872a4) )

	/* PAK option roms - Wordbee must be in slot 0 and Shell must be in slot 5. */
	ROM_REGION( 0x20000, "pakdef", ROMREGION_ERASEFF )
	ROM_LOAD("wbee13r3.mbp",          0x0000,  0x2000, CRC(d7c58b7b) SHA1(5af1b8d21a0f21534ed1833ae919dbbc6ca973e2) ) // 0
	ROM_LOAD("help2.mbp",             0x2000,  0x2000, CRC(a4f1fa90) SHA1(1456abc6ed0501a3b15a99b4302750843293ae5f) ) // 1
	ROM_LOAD("busy-p.mbp",            0x4000,  0x2000, CRC(f2897427) SHA1(b4c351bdac72d89589980be6d654f9b931bcba6b) ) // 2
	ROM_CONTINUE( 0x14000, 0x2000 )
	ROM_LOAD("graphics.mbp",          0x6000,  0x2000, CRC(9e9d327c) SHA1(aebf60ed153004380b9f271f2212376910a6cef9) ) // 3
	ROM_CONTINUE( 0x16000, 0x2000 )
	ROM_LOAD("vtex235.mbp",           0x8000,  0x2000, CRC(8c30ecb2) SHA1(cf068462d7def885bdb5d3a265851b88c727c0d7) ) // 4
	ROM_LOAD("ppcshell.mbp",          0xa000,  0x2000, CRC(1e793555) SHA1(ddeaa081ec4408e80e3fb192865d87daa035c701) ) // 5

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
ROM_END

// This represents the Series 1: 64K, and 64K Plus
ROM_START( mbee56 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("56kb.rom",              0x0000,  0x1000, CRC(28211224) SHA1(b6056339402a6b2677b0e6c57bd9b78a62d20e4f) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(79fa1e9d) SHA1(0454051697b23e4561744466fb31e7a133d02246) ) // video switching prom, not needed for emulation purposes
ROM_END

// This represents the Series 3: 64K CIAB (Computer-in-a-Book), and the standard 128K Small Business Computer
ROM_START( mbee128 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("bn54.bin",              0x0000,  0x2000, CRC(995c53db) SHA1(46e1a5cfd5795b8cf528bacf9dc79398ff7d64af) )
	//ROM_SYSTEM_BIOS( 1, "bn55", "bn55" )
	//ROMX_LOAD("bn55.rom",           0x0000, 0x2000, CRC(ca2c1073) SHA1(355d90d181de899cc7af892df96305fead9c81b4), ROM_BIOS(1) )  // not working - meant for standard CIAB

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )

	ROM_REGION( 0x4000, "pals", 0 ) // undumped; using prom from 256tc for now
	ROM_LOAD( "silver.u39", 0x0000, 0x4000, BAD_DUMP CRC(c34aab64) SHA1(781fe648488dec90185760f8e081e488b73b68bf) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(79fa1e9d) SHA1(0454051697b23e4561744466fb31e7a133d02246) ) // video switching prom, not needed for emulation purposes
ROM_END

// This represents the 64K Premium CIAB, the 128K Premium Small Business Computer, and the 128K Overdrive
ROM_START( mbee128p )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bn60", "Version 2.03" )
	ROMX_LOAD("bn60.rom",           0x0000, 0x2000, CRC(ed15d4ee) SHA1(3ea42b63d42b9a4c5402676dee8912ad1f906bda), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "bn59", "Version 2.02" )
	ROMX_LOAD("bn59.rom",           0x0000, 0x2000, CRC(97384116) SHA1(87f2c4ab1a1f2964ba4f2bb60e62dc9c163831ba), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "bn58", "Version 2.01" )
	ROMX_LOAD("bn58.rom",           0x0000, 0x2000, CRC(2f3757a6) SHA1(37158da0e8609fea50382f6b941fe473eaaf20cf), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "bn56", "bn56" )
	ROMX_LOAD("bn56.rom",           0x0000, 0x2000, CRC(3f76769d) SHA1(cfae2069d739c26fe39f734d9f705a3c965d1e6f), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "bn54", "bn54" )
	ROMX_LOAD("bn54.rom",           0x0000, 0x2000, CRC(995c53db) SHA1(46e1a5cfd5795b8cf528bacf9dc79398ff7d64af), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "bn54s", "bn54_Swedish" )
	ROMX_LOAD("bn54_swedish.rom",   0x0000, 0x2000, CRC(694179a6) SHA1(1d465330845cd7878c236a0c84a85b5512ccfd65), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "bn56s", "bn56_Swedish" )
	ROMX_LOAD("bn56_swedish.rom",   0x0000, 0x2000, CRC(dad4a515) SHA1(9c1e0faaccd8d2062bb3b99c8600b515ed460479), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "hd18", "Hard Disk System" )
	ROMX_LOAD("hd18.rom",           0x0000, 0x2000, CRC(ed53ace7) SHA1(534e2e00cc527197c76b3c106b3c9ff7f1328487), ROM_BIOS(7) )

	ROM_REGION( 0x4000, "pals", 0 ) // undumped; using prom from 256tc for now
	ROM_LOAD( "silver.u39", 0x0000, 0x4000, BAD_DUMP CRC(c34aab64) SHA1(781fe648488dec90185760f8e081e488b73b68bf) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("charrom.bin",           0x0000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
ROM_END

ROM_START( mbee256 ) // The 256K Telecomputer
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "1.20", "Version 1.20" )
	ROMX_LOAD("256tc_boot_1.20.u38", 0x0000, 0x4000, CRC(fe8d6a84) SHA1(a037a1b90b18a2180e9f5f216b829fcd480449a4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "1.15", "Version 1.15" )
	ROMX_LOAD("256tc_boot_1.15.u38", 0x0000, 0x4000, CRC(1902062d) SHA1(e4a1c0b3f4996e313da0bac0edb6d34e3270723e), ROM_BIOS(1) )

	ROM_REGION( 0x4000, "pals", 0 )
	ROM_LOAD( "silver.u39", 0x0000, 0x4000, CRC(c34aab64) SHA1(781fe648488dec90185760f8e081e488b73b68bf) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD("char256.u53", 0x0000, 0x2000, CRC(9372af3c) SHA1(a63591822c0504de2fed52e88d64e1dbd6124b74) )
ROM_END

// Premium Plus - Note: The bios rom is the only one confirmed to be in the machine. IC position numbers are unknown.
// No technical information has been released.
ROM_START( mbeepp ) // Premium Plus
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "pp.bin", 0x0000, 0x4000, CRC(33292300) SHA1(8ba32123ef1b3beffa797855a1de0ea2078d652a) ) // ver 1.0

	ROM_REGION( 0x4000, "pals", 0 )
	ROM_LOAD( "silver.u39", 0x0000, 0x4000, BAD_DUMP CRC(c34aab64) SHA1(781fe648488dec90185760f8e081e488b73b68bf) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD("char256.u53", 0x0000, 0x2000, BAD_DUMP CRC(9372af3c) SHA1(a63591822c0504de2fed52e88d64e1dbd6124b74) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT    CLASS       INIT           COMPANY               FULLNAME
COMP( 1982, mbee,      0,      0,      mbee,     mbee,    mbee_state, init_mbee,     "Applied Technology", "Microbee 16k Standard",       MACHINE_SUPPORTS_SAVE )
COMP( 1983, mbeeic,    mbee,   0,      mbeeic,   mbee,    mbee_state, init_mbeeic,   "Applied Technology", "Microbee 32k IC",             MACHINE_SUPPORTS_SAVE )
COMP( 1983, mbee56,    mbee,   0,      mbee56,   mbee,    mbee_state, init_mbee56,   "Applied Technology", "Microbee 64k",                MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1984, mbeepc,    mbee,   0,      mbeeic,   mbee,    mbee_state, init_mbeeic,   "Applied Technology", "Microbee 32k Communicator",   MACHINE_SUPPORTS_SAVE )
COMP( 1984, mbee128,   mbee,   0,      mbee128,  mbee128, mbee_state, init_mbee128,  "Applied Technology", "Microbee 128k Standard",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1985, mbeepc85,  mbee,   0,      mbeepc85, mbee,    mbee_state, init_mbeeic,   "Applied Technology", "Microbee PC85",               MACHINE_SUPPORTS_SAVE )
COMP( 1985, mbeepc85s, mbee,   0,      mbeepc85, mbee,    mbee_state, init_mbeeic,   "Applied Technology", "Microbee PC85 (Swedish)",     MACHINE_SUPPORTS_SAVE )
COMP( 1985, mbeepc85b, mbee,   0,      mbeepc85, mbee,    mbee_state, init_mbeeic,   "Microbee Systems",   "Microbee PC85 (New version)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, mbeeppc,   mbee,   0,      mbeeppc,  mbee,    mbee_state, init_mbeeppc,  "Microbee Systems",   "Microbee Premium PC85",       MACHINE_SUPPORTS_SAVE )
COMP( 1986, mbeett,    mbee,   0,      mbeett,   mbee256, mbee_state, init_mbeett,   "Microbee Systems",   "Microbee Teleterm",           MACHINE_SUPPORTS_SAVE )
COMP( 1986, mbee128p,  mbee,   0,      mbee128p, mbee128, mbee_state, init_mbee128p, "Microbee Systems",   "Microbee 128k Premium",       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1987, mbee256,   mbee,   0,      mbee256,  mbee256, mbee_state, init_mbee256,  "Microbee Systems",   "Microbee 256TC",              MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 2012, mbeepp,    mbee,   0,      mbeepp,   mbee128, mbee_state, init_mbeepp,   "Microbee Systems",   "Microbee Premium Plus+",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
