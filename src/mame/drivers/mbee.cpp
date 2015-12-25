// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Driver completely rewritten by Robbbert, in a process begun on 2009-02-24.
Assistance/advice was gratefully received from:
    E.J.Wordsworth (owner of Microbee Systems), nama, ChickenMan,
    and the author of the "ubee512" emulator.

Previous driver was written by Juergen Buchmueller, Jan 2000 with assistance
from Brett Selwood and Andrew Davies.


    Keyboard notes are in video/microbee.c

    256tc: The 1.31 rom version which appears to fit the 256TC is actually
    part of the Z80 emulation in the Matilda model. If you fit this rom into a real
    256TC, the floppy disk will not be detected.

    The unemulated Matilda is a IBM XT clone fitted with a NEC V40, and has the
    ability to emulate the 256TC as mentioned above.

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
      from the MESS config menu, then enter NET CLOCK to enable it. NET TIME hhmm to set
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

    - 256tc: Paste ignores shift key
    - All others: Paste drops most characters.

    - various fdc issues:
        - B drive doesn't work with most disks.
        - some disks cause MESS to freeze.
        - ENMF pin missing from wd_fdc.
        - incorrect timing for track register causes 256tc failure to boot a disk.

    - Simply Write has keyboard problems (in 128k, no keys work).

    - 256tc: At the menu, if F2 pressed to activate the Monitor, the emulated machine
      crashes due to a bug in z80pio emulation.

    - 256tc: the Intro disk doesn't work

    - 256tc, Teleterm: Keyboard CPU inbuilt ROM needs to be dumped.
    - 128k, 64k: PALs need to be dumped for the bankswitching.

    - Teleterm: keyboard has problems. The schematic shows it using the old-style keyboard,
                however it actually uses the new keyboard with interrupts.

    - Mouse: a few programs support the use of a serial mouse which interfaced
             directly to the Z80PIO. However there's little info to be found.
             PIO B3 to ground activates the mouse pointer in Shell v3.01.

    - Hard drive (10MB) & controller

*******************************************************************************/

#include "includes/mbee.h"
#include "formats/mbee_cas.h"

#define XTAL_13_5MHz 13500000

/********** NOTE !!! ***********************************************************
    The microbee uses lots of bankswitching and the memory maps are still
    being determined. Please don't merge memory maps !!
********************************************************************************/


static ADDRESS_MAP_START(mbee_mem, AS_PROGRAM, 8, mbee_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_READWRITE(video_low_r, video_low_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(video_high_r, video_high_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbeeic_mem, AS_PROGRAM, 8, mbee_state)
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("pak")
	AM_RANGE(0xe000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_READWRITE(video_low_r, video_low_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(video_high_r, video_high_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbeepc_mem, AS_PROGRAM, 8, mbee_state)
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("pak")
	AM_RANGE(0xe000, 0xefff) AM_ROMBANK("telcom")
	AM_RANGE(0xf000, 0xf7ff) AM_READWRITE(video_low_r, video_low_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(video_high_r, video_high_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbeeppc_mem, AS_PROGRAM, 8, mbee_state)
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("basic")
	AM_RANGE(0xa000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("pak")
	AM_RANGE(0xe000, 0xefff) AM_ROMBANK("telcom")
	AM_RANGE(0xf000, 0xf7ff) AM_READWRITE(video_low_r, video_low_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(video_high_r, video_high_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbee56_mem, AS_PROGRAM, 8, mbee_state)
	AM_RANGE(0x0000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_READWRITE(video_low_r, video_low_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(video_high_r, video_high_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbee256_mem, AS_PROGRAM, 8, mbee_state)
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x1000, 0x1fff) AM_READ_BANK("bankr1") AM_WRITE_BANK("bankw1")
	AM_RANGE(0x2000, 0x2fff) AM_READ_BANK("bankr2") AM_WRITE_BANK("bankw2")
	AM_RANGE(0x3000, 0x3fff) AM_READ_BANK("bankr3") AM_WRITE_BANK("bankw3")
	AM_RANGE(0x4000, 0x4fff) AM_READ_BANK("bankr4") AM_WRITE_BANK("bankw4")
	AM_RANGE(0x5000, 0x5fff) AM_READ_BANK("bankr5") AM_WRITE_BANK("bankw5")
	AM_RANGE(0x6000, 0x6fff) AM_READ_BANK("bankr6") AM_WRITE_BANK("bankw6")
	AM_RANGE(0x7000, 0x7fff) AM_READ_BANK("bankr7") AM_WRITE_BANK("bankw7")
	AM_RANGE(0x8000, 0x8fff) AM_READ_BANK("bankr8") AM_WRITE_BANK("bankw8")
	AM_RANGE(0x9000, 0x9fff) AM_READ_BANK("bankr9") AM_WRITE_BANK("bankw9")
	AM_RANGE(0xa000, 0xafff) AM_READ_BANK("bankr10") AM_WRITE_BANK("bankw10")
	AM_RANGE(0xb000, 0xbfff) AM_READ_BANK("bankr11") AM_WRITE_BANK("bankw11")
	AM_RANGE(0xc000, 0xcfff) AM_READ_BANK("bankr12") AM_WRITE_BANK("bankw12")
	AM_RANGE(0xd000, 0xdfff) AM_READ_BANK("bankr13") AM_WRITE_BANK("bankw13")
	AM_RANGE(0xe000, 0xefff) AM_READ_BANK("bankr14") AM_WRITE_BANK("bankw14")
	AM_RANGE(0xf000, 0xffff) AM_READ_BANK("bankr15") AM_WRITE_BANK("bankw15")
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbeett_mem, AS_PROGRAM, 8, mbee_state)
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("pak")
	AM_RANGE(0xe000, 0xefff) AM_ROMBANK("telcom")
	AM_RANGE(0xf000, 0xf7ff) AM_READWRITE(video_low_r, video_low_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(video_high_r, video_high_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbee_io, AS_IO, 8, mbee_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x10) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0b, 0x0b) AM_MIRROR(0x10) AM_WRITE(port0b_w)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0x10) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(m6545_index_w)
	AM_RANGE(0x0d, 0x0d) AM_MIRROR(0x10) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(m6545_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbeeic_io, AS_IO, 8, mbee_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x10) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0x10) AM_READWRITE(port08_r, port08_w)
	AM_RANGE(0x09, 0x09) AM_WRITENOP /* Listed as "Colour Wait Off" or "USART 2651" but doesn't appear in the schematics */
	AM_RANGE(0x0a, 0x0a) AM_MIRROR(0x10) AM_WRITE(port0a_w)
	AM_RANGE(0x0b, 0x0b) AM_MIRROR(0x10) AM_WRITE(port0b_w)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0x10) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(m6545_index_w)
	AM_RANGE(0x0d, 0x0d) AM_MIRROR(0x10) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(m6545_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbeepc_io, AS_IO, 8, mbee_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0003) AM_MIRROR(0xff10) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0008, 0x0008) AM_MIRROR(0xff10) AM_READWRITE(port08_r, port08_w)
	AM_RANGE(0x0009, 0x0009) AM_MIRROR(0xff00) AM_WRITENOP
	AM_RANGE(0x000b, 0x000b) AM_MIRROR(0xff10) AM_WRITE(port0b_w)
	AM_RANGE(0x000c, 0x000c) AM_MIRROR(0xff10) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(m6545_index_w)
	AM_RANGE(0x000d, 0x000d) AM_MIRROR(0xff10) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(m6545_data_w)
	AM_RANGE(0x000a, 0x000a) AM_MIRROR(0xfe10) AM_READWRITE(telcom_low_r, port0a_w)
	AM_RANGE(0x010a, 0x010a) AM_MIRROR(0xfe10) AM_READWRITE(telcom_high_r, port0a_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbeeppc_io, AS_IO, 8, mbee_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0003) AM_MIRROR(0xff10) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0008, 0x0008) AM_MIRROR(0xff10) AM_READWRITE(port08_r, port08_w)
	AM_RANGE(0x0009, 0x0009) AM_MIRROR(0xff00) AM_WRITENOP
	AM_RANGE(0x000b, 0x000b) AM_MIRROR(0xff10) AM_WRITE(port0b_w)
	AM_RANGE(0x000c, 0x000c) AM_MIRROR(0xff00) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(m6545_index_w)
	AM_RANGE(0x000d, 0x000d) AM_MIRROR(0xff10) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(m6545_data_w)
	AM_RANGE(0x001c, 0x001c) AM_MIRROR(0xff00) AM_READWRITE(port1c_r, port1c_w)
	AM_RANGE(0x000a, 0x000a) AM_MIRROR(0xfe10) AM_READWRITE(telcom_low_r, port0a_w)
	AM_RANGE(0x010a, 0x010a) AM_MIRROR(0xfe10) AM_READWRITE(telcom_high_r, port0a_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbeett_io, AS_IO, 8, mbee_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0003) AM_MIRROR(0xff00) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0004, 0x0004) AM_MIRROR(0xff00) AM_WRITE(port04_w)
	AM_RANGE(0x0006, 0x0006) AM_MIRROR(0xff00) AM_WRITE(port06_w)
	AM_RANGE(0x0007, 0x0007) AM_MIRROR(0xff00) AM_READ(port07_r)
	AM_RANGE(0x0008, 0x0008) AM_MIRROR(0xff00) AM_READWRITE(port08_r, port08_w)
	AM_RANGE(0x000b, 0x000b) AM_MIRROR(0xff00) AM_WRITE(port0b_w)
	AM_RANGE(0x000c, 0x000c) AM_MIRROR(0xff00) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(m6545_index_w)
	AM_RANGE(0x000d, 0x000d) AM_MIRROR(0xff00) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(m6545_data_w)
	AM_RANGE(0x0018, 0x001b) AM_MIRROR(0xff00) AM_READ(port18_r)
	AM_RANGE(0x001c, 0x001f) AM_MIRROR(0xff00) AM_READWRITE(port1c_r, port1c_w)
	AM_RANGE(0x0009, 0x0009) AM_MIRROR(0xfe00) AM_READ(speed_low_r)
	AM_RANGE(0x0109, 0x0109) AM_MIRROR(0xfe00) AM_READ(speed_high_r)
	AM_RANGE(0x000a, 0x000a) AM_MIRROR(0xfe00) AM_READWRITE(telcom_low_r, port0a_w)
	AM_RANGE(0x010a, 0x010a) AM_MIRROR(0xfe00) AM_READWRITE(telcom_high_r, port0a_w)
	AM_RANGE(0x0068, 0x006f) AM_MIRROR(0xff00) AM_DEVREADWRITE("scc", scc8530_t, reg_r, reg_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbee56_io, AS_IO, 8, mbee_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x08, 0x08) AM_READWRITE(port08_r, port08_w)
	AM_RANGE(0x09, 0x09) AM_WRITENOP
	AM_RANGE(0x0b, 0x0b) AM_WRITE(port0b_w)
	AM_RANGE(0x0c, 0x0c) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(m6545_index_w)
	AM_RANGE(0x0d, 0x0d) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(m6545_data_w)
	AM_RANGE(0x44, 0x47) AM_DEVREADWRITE("fdc", wd2793_t, read, write)
	AM_RANGE(0x48, 0x4f) AM_READWRITE(fdc_status_r, fdc_motor_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbee128_io, AS_IO, 8, mbee_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x04, 0x04) AM_WRITE(port04_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(port06_w)
	AM_RANGE(0x07, 0x07) AM_READ(port07_r)
	AM_RANGE(0x08, 0x08) AM_READWRITE(port08_r, port08_w)
	AM_RANGE(0x09, 0x09) AM_WRITENOP
	AM_RANGE(0x0b, 0x0b) AM_WRITE(port0b_w)
	AM_RANGE(0x0c, 0x0c) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(m6545_index_w)
	AM_RANGE(0x0d, 0x0d) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(m6545_data_w)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE(port1c_r, port1c_w)
	AM_RANGE(0x44, 0x47) AM_DEVREADWRITE("fdc", wd2793_t, read, write)
	AM_RANGE(0x48, 0x4f) AM_READWRITE(fdc_status_r, fdc_motor_w)
	AM_RANGE(0x50, 0x57) AM_WRITE(mbee128_50_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mbee256_io, AS_IO, 8, mbee_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0003) AM_MIRROR(0xff00) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0004, 0x0004) AM_MIRROR(0xff00) AM_WRITE(port04_w)
	AM_RANGE(0x0006, 0x0006) AM_MIRROR(0xff00) AM_WRITE(port06_w)
	AM_RANGE(0x0007, 0x0007) AM_MIRROR(0xff00) AM_READ(port07_r)
	AM_RANGE(0x0008, 0x0008) AM_MIRROR(0xff00) AM_READWRITE(port08_r, port08_w)
	AM_RANGE(0x0009, 0x0009) AM_MIRROR(0xfd00) AM_READ(speed_low_r)
	AM_RANGE(0x0209, 0x0209) AM_MIRROR(0xfd00) AM_READ(speed_high_r)
	AM_RANGE(0x0009, 0x0009) AM_MIRROR(0xff00) AM_WRITENOP
	AM_RANGE(0x000b, 0x000b) AM_MIRROR(0xff00) AM_WRITE(port0b_w)
	AM_RANGE(0x000c, 0x000c) AM_MIRROR(0xff00) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(m6545_index_w)
	AM_RANGE(0x000d, 0x000d) AM_MIRROR(0xff00) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(m6545_data_w)
	// AM_RANGE(0x0010, 0x0013) AM_MIRROR(0xff00) Optional SN76489AN audio chip
	AM_RANGE(0x0018, 0x001b) AM_MIRROR(0xff00) AM_READ(port18_r)
	AM_RANGE(0x001c, 0x001f) AM_MIRROR(0xff00) AM_READWRITE(port1c_r, port1c_w)
	AM_RANGE(0x0044, 0x0047) AM_MIRROR(0xff00) AM_DEVREADWRITE("fdc", wd2793_t, read, write)
	AM_RANGE(0x0048, 0x004f) AM_MIRROR(0xff00) AM_READWRITE(fdc_status_r, fdc_motor_w)
	AM_RANGE(0x0050, 0x0057) AM_MIRROR(0xff00) AM_WRITE(mbee256_50_w)
	// AM_RANGE(0x0058, 0x005f) AM_MIRROR(0xff00) External options: floppy drive, hard drive and keyboard
	// AM_RANGE(0x0060, 0x0067) AM_MIRROR(0xff00) Reserved for file server selection (unused)
	// AM_RANGE(0x0068, 0x006f) AM_MIRROR(0xff00) Reserved for 8530 SCC (unused)
ADDRESS_MAP_END

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
	PORT_CONFNAME( 0xc0, 0x40, "PIO B7")
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
	PORT_CONFNAME( 0xc0, 0x40, "PIO B7")
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
static const gfx_layout mbee_charlayout =
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

static GFXDECODE_START( mono )
	GFXDECODE_ENTRY( "gfx", 0x0000, mbee_charlayout, 96, 1 )
GFXDECODE_END

static GFXDECODE_START( standard )
	GFXDECODE_ENTRY( "gfx", 0x0000, mbee_charlayout, 0, 48 )
GFXDECODE_END

static GFXDECODE_START( premium )
	GFXDECODE_ENTRY( "gfx", 0x0000, mbee_charlayout, 0, 8 )
GFXDECODE_END

static SLOT_INTERFACE_START( mbee_floppies )
	SLOT_INTERFACE( "drive3a", FLOPPY_35_DD )
	SLOT_INTERFACE( "drive3b", FLOPPY_35_DD )
	SLOT_INTERFACE( "drive5a", FLOPPY_525_QD )
	SLOT_INTERFACE( "drive5b", FLOPPY_525_QD )
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( mbee, mbee_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz / 6)         /* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(mbee_mem)
	MCFG_CPU_IO_MAP(mbee_io)
	MCFG_CPU_CONFIG(mbee_daisy_chain)

	MCFG_MACHINE_RESET_OVERRIDE(mbee_state, mbee)

	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_12MHz / 6)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(mbee_state, pio_ardy))
	MCFG_Z80PIO_IN_PB_CB(READ8(mbee_state, pio_port_b_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(mbee_state, pio_port_b_w))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250)) /* not accurate */
	MCFG_SCREEN_SIZE(64*8, 19*16)           /* need at least 17 lines for NET */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0, 19*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(mbee_state, screen_update_mbee)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mono)

	MCFG_PALETTE_ADD("palette", 100)
	MCFG_PALETTE_INIT_OWNER(mbee_state, standard)

	MCFG_VIDEO_START_OVERRIDE(mbee_state, mono)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_MC6845_ADD("crtc", SY6545_1, "screen", XTAL_12MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(mbee_state, crtc_update_row)
	MCFG_MC6845_ADDR_CHANGED_CB(mbee_state, crtc_update_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(mbee_state, crtc_vs))

	MCFG_QUICKLOAD_ADD("quickload", mbee_state, mbee, "mwb,com,bee", 3)
	MCFG_QUICKLOAD_ADD("quickload2", mbee_state, mbee_z80bin, "bin", 3)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("z80pio", z80pio_device, strobe_a))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(mbee_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( mbeeic, mbee_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_13_5MHz / 4)         /* 3.37500 MHz */
	MCFG_CPU_PROGRAM_MAP(mbeeic_mem)
	MCFG_CPU_IO_MAP(mbeeic_io)
	MCFG_CPU_CONFIG(mbee_daisy_chain)

	MCFG_MACHINE_RESET_OVERRIDE(mbee_state, mbee)

	MCFG_DEVICE_ADD("z80pio", Z80PIO, 3375000)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(mbee_state, pio_ardy))
	MCFG_Z80PIO_IN_PB_CB(READ8(mbee_state, pio_port_b_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(mbee_state, pio_port_b_w))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250)) /* not accurate */
	MCFG_SCREEN_SIZE(80*8, 310)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 19*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(mbee_state, screen_update_mbee)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", standard)

	MCFG_PALETTE_ADD("palette", 100)
	MCFG_PALETTE_INIT_OWNER(mbee_state, standard)

	MCFG_VIDEO_START_OVERRIDE(mbee_state, standard)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_MC6845_ADD("crtc", SY6545_1, "screen", XTAL_13_5MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(mbee_state, crtc_update_row)
	MCFG_MC6845_ADDR_CHANGED_CB(mbee_state, crtc_update_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(mbee_state, crtc_vs))

	MCFG_QUICKLOAD_ADD("quickload", mbee_state, mbee, "mwb,com,bee", 2)
	MCFG_QUICKLOAD_ADD("quickload2", mbee_state, mbee_z80bin, "bin", 2)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("z80pio", z80pio_device, strobe_a))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(mbee_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mbeepc, mbeeic )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(mbeepc_mem)
	MCFG_CPU_IO_MAP(mbeepc_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mbeeppc, mbeeic )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(mbeeppc_mem)
	MCFG_CPU_IO_MAP(mbeeppc_io)
	MCFG_VIDEO_START_OVERRIDE(mbee_state, premium)
	MCFG_GFXDECODE_MODIFY("gfxdecode", premium)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(mbee_state, premium)
	MCFG_MC146818_ADD( "rtc", XTAL_32_768kHz )
	MCFG_MC146818_IRQ_HANDLER(WRITELINE(mbee_state, rtc_irq_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mbee56, mbeeic )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(mbee56_mem)
	MCFG_CPU_IO_MAP(mbee56_io)
	MCFG_MACHINE_RESET_OVERRIDE(mbee_state, mbee56)
	MCFG_WD2793_ADD("fdc", XTAL_4MHz / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(mbee_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(mbee_state, fdc_drq_w))
	MCFG_WD_FDC_ENMF_CALLBACK(GND)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", mbee_floppies, "drive5a", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", mbee_floppies, "drive5b", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mbee128, mbee56 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(mbee256_mem)
	MCFG_CPU_IO_MAP(mbee128_io)
	MCFG_MACHINE_RESET_OVERRIDE(mbee_state, mbee128)
	MCFG_MC146818_ADD( "rtc", XTAL_32_768kHz )
	MCFG_MC146818_IRQ_HANDLER(WRITELINE(mbee_state, rtc_irq_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mbee128p, mbeeppc )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(mbee256_mem)
	MCFG_CPU_IO_MAP(mbee128_io)
	MCFG_MACHINE_RESET_OVERRIDE(mbee_state, mbee128)
	MCFG_WD2793_ADD("fdc", XTAL_4MHz / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(mbee_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(mbee_state, fdc_drq_w))
	MCFG_WD_FDC_ENMF_CALLBACK(GND)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", mbee_floppies, "drive5a", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", mbee_floppies, "drive5b", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mbee256, mbee128p )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(mbee256_mem)
	MCFG_CPU_IO_MAP(mbee256_io)
	MCFG_MACHINE_RESET_OVERRIDE(mbee_state, mbee256)

	MCFG_DEVICE_REMOVE("fdc:0")
	MCFG_DEVICE_REMOVE("fdc:1")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", mbee_floppies, "drive3a", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", mbee_floppies, "drive3b", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mbeett, mbeeppc )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(mbeett_mem)
	MCFG_CPU_IO_MAP(mbeett_io)
	MCFG_MACHINE_RESET_OVERRIDE(mbee_state, mbeett)
	MCFG_DEVICE_REMOVE("quickload")
	MCFG_DEVICE_REMOVE("quickload2")
	MCFG_DEVICE_ADD("scc", SCC8530, 4000000) // clock unknown
MACHINE_CONFIG_END

/* Unused roms:
    ROM_LOAD_OPTIONAL("net.rom", 0xe000,  0x1000, CRC(e14aac4c) SHA1(330902cf47f53c22c85003a620f7d7d3261ebb67) )
    customised for a certain high school. Requires pulsing at bit 3 port 0 before it runs.

    ROM_LOAD_OPTIONAL("chip8_22.rom", 0xe000,  0x1000, CRC(11fbb547) SHA1(7bd9dc4b67b33b8e1be99beb6a0ddff25bdbd3f7) )
    Dreamcards Chip-8 V2.2 rom

    ROM_LOAD_OPTIONAL("telcom11.rom", 0xe000,  0x1000, CRC(15516499) SHA1(2d4953f994b66c5d3b1d457b8c92d9a0a69eb8b8) )
    Telcom 1.1 for the mbeeic (It could have 1.0, 1.1 or 1.2)

*/


/* gfxram:
    0000 = normal characters in charrom
    0800 = small characters in charrom
    1000 = characters selected
    1800 = pcg */

ROM_START( mbee )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bas510a.ic25",          0x8000,  0x1000, CRC(2ca47c36) SHA1(f36fd0afb3f1df26edc67919e78000b762b6cbcb) )
	ROM_LOAD("bas510b.ic27",          0x9000,  0x1000, CRC(a07a0c51) SHA1(dcbdd9df78b4b6b2972de2e4050dabb8ae9c3f5a) )
	ROM_LOAD("bas510c.ic28",          0xa000,  0x1000, CRC(906ac00f) SHA1(9b46458e5755e2c16cdb191a6a70df6de9fe0271) )
	ROM_LOAD("bas510d.ic30",          0xb000,  0x1000, CRC(61727323) SHA1(c0fea9fd0e25beb9faa7424db8efd07cf8d26c1b) )
	ROM_LOAD_OPTIONAL("edasma.ic31",  0xc000,  0x1000, CRC(120c3dea) SHA1(32c9bb6e54dd50d5218bb43cc921885a0307161d) )
	ROM_LOAD_OPTIONAL("edasmb.ic33",  0xd000,  0x1000, CRC(a23bf3c8) SHA1(73a57c2800a1c744b527d0440b170b8b03351753) )
	ROM_LOAD_OPTIONAL("telcom10.rom", 0xe000,  0x1000, CRC(cc9ac94d) SHA1(6804b5ff54d16f8e06180751d8681c44f351e0bb) )

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("charrom.ic13",          0x1000,  0x0800, CRC(b149737b) SHA1(a3cd4f5d0d3c71137cd1f0f650db83333a2e3597) )
	ROM_RELOAD( 0x0000, 0x0800 )
	ROM_RELOAD( 0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0000,  0x0020, CRC(4e779985) SHA1(cd2579cf65032c30b3fe7d6d07b89d4633687481) )   /* video switching prom, not needed for emulation purposes */

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
ROM_END

ROM_START( mbeeic )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bas522a.rom",           0x8000,  0x2000, CRC(7896a696) SHA1(a158f7803296766160e1f258dfc46134735a9477) )
	ROM_LOAD("bas522b.rom",           0xa000,  0x2000, CRC(b21d9679) SHA1(332844433763331e9483409cd7da3f90ac58259d) )
	ROM_LOAD_OPTIONAL("telcom12.rom", 0xe000,  0x1000, CRC(0231bda3) SHA1(be7b32499034f985cc8f7865f2bc2b78c485585c) )

	/* PAK option roms */
	ROM_REGION(0x20000, "pakrom", ROMREGION_ERASEFF)
	ROM_LOAD_OPTIONAL("edasm.rom",    0x0000,  0x2000, CRC(1af1b3a9) SHA1(d035a997c2dbbb3918b3395a3a5a1076aa203ee5) ) // 0
	ROM_LOAD_OPTIONAL("wbee12.rom",   0x2000,  0x2000, CRC(0fc21cb5) SHA1(33b3995988fc51ddef1568e160dfe699867adbd5) ) // 1
	ROM_LOAD_OPTIONAL("forth11.rom",  0x4000,  0x2000, CRC(f0fc2358) SHA1(b7303b94abe647d5a6ffb2fba5d205412f970c16) ) // 2

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(4e779985) SHA1(cd2579cf65032c30b3fe7d6d07b89d4633687481) )   /* video switching prom, not needed for emulation purposes */

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
ROM_END

ROM_START( mbeepc )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bas522a.rom",           0x8000,  0x2000, CRC(7896a696) SHA1(a158f7803296766160e1f258dfc46134735a9477) )
	ROM_LOAD("bas522b.rom",           0xa000,  0x2000, CRC(b21d9679) SHA1(332844433763331e9483409cd7da3f90ac58259d) )

	ROM_REGION(0x2000, "telcomrom", 0)
	ROM_LOAD_OPTIONAL("telcom31.rom", 0x0000,  0x2000, CRC(5a904a29) SHA1(3120fb65ccefeb180ab80d8d35440c70dc8452c8) )

	/* PAK option roms */
	ROM_REGION(0x20000, "pakrom", ROMREGION_ERASEFF)
	ROM_LOAD_OPTIONAL("edasm.rom",    0x0000,  0x2000, CRC(1af1b3a9) SHA1(d035a997c2dbbb3918b3395a3a5a1076aa203ee5) ) // 0
	ROM_LOAD_OPTIONAL("wbee12.rom",   0x2000,  0x2000, CRC(0fc21cb5) SHA1(33b3995988fc51ddef1568e160dfe699867adbd5) ) // 1
	ROM_LOAD_OPTIONAL("mwbhelp.rom",  0x4000,  0x2000, CRC(d34fae54) SHA1(5ed30636f48e9d208ce2da367ba4425782a5bce3) ) // 2

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(4e779985) SHA1(cd2579cf65032c30b3fe7d6d07b89d4633687481) )   /* video switching prom, not needed for emulation purposes */

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
ROM_END

ROM_START( mbeepc85 )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("bas525a.rom",           0x8000,  0x2000, CRC(a6e02afe) SHA1(0495308c7e1d84b5989a3af6d3b881f4580b2641) )
	ROM_LOAD("bas525b.rom",           0xa000,  0x2000, CRC(245dd36b) SHA1(dd288f3e6737627f50d3d2a49df3e57c423d3118) )

	ROM_REGION(0x2000, "telcomrom", 0)
	ROM_LOAD_OPTIONAL("telco321.rom", 0x0000,  0x2000, CRC(36852a11) SHA1(c45b8d03629e86231c6b256a7435abd87d8872a4) )

	/* PAK option roms - Wordbee must be in slot 0 and Shell must be in slot 5. */
	ROM_REGION(0x20000, "pakrom", ROMREGION_ERASEFF)
	ROM_LOAD("wbee13.rom",            0x0000,  0x2000, CRC(d7c58b7b) SHA1(5af1b8d21a0f21534ed1833ae919dbbc6ca973e2) ) // 0
	ROM_LOAD_OPTIONAL("cmdhelp.rom",  0x2000,  0x2000, CRC(a4f1fa90) SHA1(1456abc6ed0501a3b15a99b4302750843293ae5f) ) // 1
	ROM_LOAD_OPTIONAL("edasm.rom",    0x4000,  0x2000, CRC(1af1b3a9) SHA1(d035a997c2dbbb3918b3395a3a5a1076aa203ee5) ) // 2
	ROM_LOAD_OPTIONAL("forth.rom",    0x6000,  0x2000, CRC(c0795c2b) SHA1(8faa0a46fbbdb8a1019d706a40cd4431a5063f8c) ) // 3
	ROM_LOAD("shell.rom",             0xa000,  0x2000, CRC(5a2c7cd6) SHA1(8edc086710cb558f2146d660eddc8a18ba6a141c) ) // 5
	ROM_LOAD_OPTIONAL("ozlogo.rom",   0xc000,  0x2000, CRC(47c3ef69) SHA1(8274d27c323ca4a6cc9e7d24946ae9c0531c3112) ) // 6
	ROM_LOAD_OPTIONAL("chess.rom",    0xe000,  0x2000, CRC(fe9ee9d0) SHA1(a316559414e68c0101af5f00755db551e7c5788e) ) // 7

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(4e779985) SHA1(cd2579cf65032c30b3fe7d6d07b89d4633687481) )   /* video switching prom, not needed for emulation purposes */

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
ROM_END

ROM_START( mbeepc85b )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("bas525a.rom",           0x8000,  0x2000, CRC(a6e02afe) SHA1(0495308c7e1d84b5989a3af6d3b881f4580b2641) )
	ROM_LOAD("bas525b.rom",           0xa000,  0x2000, CRC(245dd36b) SHA1(dd288f3e6737627f50d3d2a49df3e57c423d3118) )

	ROM_REGION(0x2000, "telcomrom", 0)
	ROM_LOAD_OPTIONAL("telco321.rom", 0x0000,  0x2000, CRC(36852a11) SHA1(c45b8d03629e86231c6b256a7435abd87d8872a4) )

	/* PAK option roms - Wordbee must be in slot 0 and Shell must be in slot 5. */
	ROM_REGION(0x20000, "pakrom", ROMREGION_ERASEFF)
	ROM_LOAD("wbee13.rom",            0x0000,  0x2000, CRC(d7c58b7b) SHA1(5af1b8d21a0f21534ed1833ae919dbbc6ca973e2) ) // 0
	ROM_LOAD_OPTIONAL("cmdhelp.rom",  0x2000,  0x2000, CRC(a4f1fa90) SHA1(1456abc6ed0501a3b15a99b4302750843293ae5f) ) // 1
	ROM_LOAD_OPTIONAL("busy.rom",     0x4000,  0x2000, CRC(56255f60) SHA1(fd2e37209fd49290be6875bc460cfc05392938ba) ) // 2
	ROM_CONTINUE( 0x14000, 0x2000 )
	ROM_LOAD_OPTIONAL("graphics.rom", 0x6000,  0x2000, CRC(9e9d327c) SHA1(aebf60ed153004380b9f271f2212376910a6cef9) ) // 3
	ROM_CONTINUE( 0x16000, 0x2000 )
	ROM_LOAD_OPTIONAL("viatel.rom",   0x8000,  0x2000, CRC(2da2411f) SHA1(d3cfa978165feef0a96e28197f6a762aa6604799) ) // 4
	ROM_LOAD("shell-b.rom",           0xa000,  0x2000, CRC(17bf2d58) SHA1(ae22a5fc5783f37066ba5555497e40945272ca3d) ) // 5

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(4e779985) SHA1(cd2579cf65032c30b3fe7d6d07b89d4633687481) )   /* video switching prom, not needed for emulation purposes */

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
ROM_END

ROM_START( mbeepc85s )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("bas524a.rom",           0x8000,  0x2000, CRC(ec9c7a60) SHA1(a4021bcedc8da8c0eb0bda036a1d457619a175b0) )
	ROM_LOAD("bas524b.rom",           0xa000,  0x2000, CRC(17d3eac7) SHA1(d40d376cc5e751d257d951909a34445e70506c7b) )

	ROM_REGION(0x2000, "telcomrom", 0)
	ROM_LOAD_OPTIONAL("telco321s.rom", 0x0000, 0x2000, CRC(00f8fde1) SHA1(eb881bbab90c85fd6e29540decd25e884c67f738) )

	/* PAK roms - These are not optional and will only work in the correct slots. */
	ROM_REGION(0x20000, "pakrom", ROMREGION_ERASEFF)
	ROM_LOAD("wbee20-s.rom",          0x0000,  0x2000, CRC(6a0fe57f) SHA1(a101b588b1872e19382b9e9ea50fabb0fd060aa6) ) // 0
	ROM_LOAD("db-s.rom",              0x2000,  0x2000, CRC(e2094771) SHA1(62d7fb66c91d2bd24523bc84e4f005cf2c4480bb) ) // 1
	ROM_LOAD("kalk-s.rom",            0x4000,  0x2000, CRC(08dd71ee) SHA1(c9d506d8bb56f602c3481b253d4cac226f545d98) ) // 2
	ROM_LOAD("bg-s.rom",              0x6000,  0x2000, CRC(5aa4813e) SHA1(a8638e9046bfb9d5a98c878322295ce408bd879d) ) // 3
	ROM_LOAD("videotex-s.rom",        0x8000,  0x2000, CRC(67592b3f) SHA1(7f1d23ded34781ccda5f36b4a4fa118a8c0e44ec) ) // 4
	ROM_LOAD("shell-s.rom",           0xa000,  0x2000, CRC(bdf1768f) SHA1(4385351d07288cf94947ac63131eeed98572caa1) ) // 5

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("charrom-s.bin",         0x1000,  0x1000, CRC(1bcbf083) SHA1(6438649b8b5fc20dd772ec7195e69a5bbe016b09) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(4e779985) SHA1(cd2579cf65032c30b3fe7d6d07b89d4633687481) )   /* video switching prom, not needed for emulation purposes */

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
ROM_END

ROM_START( mbeett )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("kernel_106.rom",        0x8000,  0x2000, CRC(5ab9cb1d) SHA1(a1fb971622f85c4d866b91cb4bec6d75757e8c5f) )

	ROM_REGION(0x2000, "telcomrom", 0)
	ROM_LOAD("wm_106.rom",            0x0000,  0x2000, CRC(77e0b355) SHA1(1db6769cd6b12e1c335c83f17f8c139986c87758) )

	ROM_REGION(0x20000, "pakrom", ROMREGION_ERASEFF)
	ROM_LOAD("tv_470311.rom",         0x2000,  0x2000, CRC(2c4c2dcb) SHA1(77cd75166a389cb2d1d8abf00b1ddd077ce98354) ) // 1
	ROM_CONTINUE( 0x12000, 0x2000 )
	ROM_LOAD("tw_103.rom",            0x4000,  0x2000, CRC(881edb4b) SHA1(f6e30a12b1537bd55b69d1319799b150e80a471b) ) // 2
	ROM_CONTINUE( 0x14000, 0x2000 )
	ROM_LOAD("oside_107.rom",         0x6000,  0x2000, CRC(05d99aba) SHA1(4f88d63025f99bcc54d6f2abc20a699c97384f68) ) // 3
	ROM_CONTINUE( 0x16000, 0x1000 )
	ROM_LOAD("test_105.rom",          0x8000,  0x2000, CRC(b69aa618) SHA1(49de8cbad59f549c7ad9f8efc9beee0cfcd901fe) ) // 4

	ROM_REGION(0x9800, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "attrib", ROMREGION_ERASE00 )
ROM_END

ROM_START( mbeeppc )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("bas529b.rom",           0xa000,  0x2000, CRC(a1bd986b) SHA1(5d79f210c9042db5aefc85a0bdf45210cb9e9899) )

	ROM_REGION(0x4000, "basicrom", 0)
	ROM_LOAD("bas529a.rom",           0x0000,  0x4000, CRC(fe8242e1) SHA1(ff790edf4fcc7a134d451dbad7779157b07f6abf) )

	ROM_REGION(0x2000, "telcomrom", 0)
	ROM_LOAD_OPTIONAL("telco321.rom", 0x0000,  0x2000, CRC(36852a11) SHA1(c45b8d03629e86231c6b256a7435abd87d8872a4) )

	/* PAK option roms - Wordbee must be in slot 0 and Shell must be in slot 5. */
	ROM_REGION(0x20000, "pakrom", ROMREGION_ERASEFF)
	ROM_LOAD("wbee13.rom",            0x0000,  0x2000, CRC(d7c58b7b) SHA1(5af1b8d21a0f21534ed1833ae919dbbc6ca973e2) ) // 0
	ROM_LOAD_OPTIONAL("cmdhelp.rom",  0x2000,  0x2000, CRC(a4f1fa90) SHA1(1456abc6ed0501a3b15a99b4302750843293ae5f) ) // 1
	ROM_LOAD_OPTIONAL("busy-p.rom",   0x4000,  0x2000, CRC(f2897427) SHA1(b4c351bdac72d89589980be6d654f9b931bcba6b) ) // 2
	ROM_CONTINUE( 0x14000, 0x2000 )
	ROM_LOAD_OPTIONAL("graphics.rom", 0x6000,  0x2000, CRC(9e9d327c) SHA1(aebf60ed153004380b9f271f2212376910a6cef9) ) // 3
	ROM_CONTINUE( 0x16000, 0x2000 )
	ROM_LOAD_OPTIONAL("vtex235.rom",  0x8000,  0x2000, CRC(8c30ecb2) SHA1(cf068462d7def885bdb5d3a265851b88c727c0d7) ) // 4
	ROM_LOAD("ppcshell.rom",          0xa000,  0x2000, CRC(1e793555) SHA1(ddeaa081ec4408e80e3fb192865d87daa035c701) ) // 5

	ROM_REGION(0x9800, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
	ROM_REGION( 0x0800, "attrib", ROMREGION_ERASE00 )
ROM_END

ROM_START( mbee56 )
	ROM_REGION(0x10000,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("56kb.rom",              0xe000,  0x1000, CRC(28211224) SHA1(b6056339402a6b2677b0e6c57bd9b78a62d20e4f) )

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(4e779985) SHA1(cd2579cf65032c30b3fe7d6d07b89d4633687481) )   /* video switching prom, not needed for emulation purposes */

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
ROM_END

ROM_START( mbee128 ) // Standard 128k (CIAB is the same thing with half the ram)
	ROM_REGION(0x20000, "rams", ROMREGION_ERASEFF)

	ROM_REGION(0x8000, "roms", 0)
	ROM_LOAD("bn54.bin",              0x0000,  0x2000, CRC(995c53db) SHA1(46e1a5cfd5795b8cf528bacf9dc79398ff7d64af) )

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION(0x4000, "pals", 0) // undumped; using prom from 256tc for now
	ROM_LOAD( "silver.u39", 0x0000, 0x4000, BAD_DUMP CRC(c34aab64) SHA1(781fe648488dec90185760f8e081e488b73b68bf) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.ic7",           0x0000,  0x0020, CRC(61b9c16c) SHA1(0ee72377831c21339360c376f7248861d476dc20) )
	ROM_LOAD_OPTIONAL( "82s123.ic16", 0x0020,  0x0020, CRC(4e779985) SHA1(cd2579cf65032c30b3fe7d6d07b89d4633687481) )   /* video switching prom, not needed for emulation purposes */

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
ROM_END

ROM_START( mbee128p ) // Premium 128K
	ROM_REGION(0x20000, "rams", ROMREGION_ERASEFF)

	ROM_REGION(0x8000, "roms", 0) // rom plus optional undumped roms plus dummy area
	ROM_SYSTEM_BIOS( 0, "bn56", "bn56" )
	ROMX_LOAD("bn56.rom",     0x0000, 0x2000, CRC(3f76769d) SHA1(cfae2069d739c26fe39f734d9f705a3c965d1e6f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "bn59", "Version 2.02" )
	ROMX_LOAD("bn59.rom",     0x0000, 0x2000, CRC(97384116) SHA1(87f2c4ab1a1f2964ba4f2bb60e62dc9c163831ba), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "bn60", "Version 2.03" )
	ROMX_LOAD("bn60.rom",     0x0000, 0x2000, CRC(ed15d4ee) SHA1(3ea42b63d42b9a4c5402676dee8912ad1f906bda), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "bn55", "bn55" )
	ROMX_LOAD("bn55.rom",     0x0000, 0x2000, CRC(ca2c1073) SHA1(355d90d181de899cc7af892df96305fead9c81b4), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "bn54", "bn54" )
	ROMX_LOAD("bn54.rom",     0x0000, 0x2000, CRC(995c53db) SHA1(46e1a5cfd5795b8cf528bacf9dc79398ff7d64af), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5, "hd18", "Hard Disk System" )
	ROMX_LOAD("hd18.rom",     0x0000, 0x2000, CRC(ed53ace7) SHA1(534e2e00cc527197c76b3c106b3c9ff7f1328487), ROM_BIOS(6) )

	ROM_REGION(0x4000, "pals", 0) // undumped; using prom from 256tc for now
	ROM_LOAD( "silver.u39", 0x0000, 0x4000, BAD_DUMP CRC(c34aab64) SHA1(781fe648488dec90185760f8e081e488b73b68bf) )

	ROM_REGION(0x9800, "gfx", 0)
	ROM_LOAD("charrom.bin",           0x1000,  0x1000, CRC(1f9fcee4) SHA1(e57ac94e03638075dde68a0a8c834a4f84ba47b0) )
	ROM_RELOAD( 0x0000, 0x1000 )

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
	ROM_REGION( 0x0800, "attrib", ROMREGION_ERASE00 )
ROM_END

ROM_START( mbee256 ) // 256tc
	ROM_REGION(0x40000, "rams", ROMREGION_ERASEFF)

	ROM_REGION(0x5000, "roms", 0) // rom plus dummy area
	ROM_SYSTEM_BIOS( 0, "1.20", "Version 1.20" )
	ROMX_LOAD("256tc_boot_1.20.u38", 0x0000, 0x4000, CRC(fe8d6a84) SHA1(a037a1b90b18a2180e9f5f216b829fcd480449a4), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "1.15", "Version 1.15" )
	ROMX_LOAD("256tc_boot_1.15.u38", 0x0000, 0x4000, CRC(1902062d) SHA1(e4a1c0b3f4996e313da0bac0edb6d34e3270723e), ROM_BIOS(2) )

	ROM_REGION(0x4000, "pals", 0)
	ROM_LOAD( "silver.u39", 0x0000, 0x4000, CRC(c34aab64) SHA1(781fe648488dec90185760f8e081e488b73b68bf) )

	ROM_REGION(0x9800, "gfx", 0)
	ROM_LOAD("char256.u53", 0x1000, 0x1000, CRC(9372af3c) SHA1(a63591822c0504de2fed52e88d64e1dbd6124b74) )
	ROM_IGNORE( 0x1000 )    // throw away swedish characters for now
	ROM_COPY( "gfx", 0x1000, 0x0000, 0x1000 )

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASE00 )
	ROM_REGION( 0x0800, "colorram", ROMREGION_ERASEVAL(2))
	ROM_REGION( 0x0800, "attrib", ROMREGION_ERASE00 )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS        INIT         COMPANY                   FULLNAME */
COMP( 1982, mbee,     0,        0,      mbee,     mbee,     mbee_state,  mbee,       "Applied Technology",  "Microbee 16 Standard" , 0 )
COMP( 1982, mbeeic,   mbee,     0,      mbeeic,   mbee,     mbee_state,  mbeeic,     "Applied Technology",  "Microbee 32 IC" , 0 )
COMP( 1982, mbeepc,   mbee,     0,      mbeepc,   mbee,     mbee_state,  mbeepc,     "Applied Technology",  "Microbee Personal Communicator" , 0 )
COMP( 1985, mbeepc85, mbee,     0,      mbeepc,   mbee,     mbee_state,  mbeepc85,   "Applied Technology",  "Microbee PC85" , 0 )
COMP( 1985, mbeepc85b,mbee,     0,      mbeepc,   mbee,     mbee_state,  mbeepc85,   "Applied Technology",  "Microbee PC85 (New version)" , 0 )
COMP( 1985, mbeepc85s,mbee,     0,      mbeepc,   mbee,     mbee_state,  mbeepc85,   "Applied Technology",  "Microbee PC85 (Swedish)" , 0 )
COMP( 1986, mbeeppc,  mbee,     0,      mbeeppc,  mbee,     mbee_state,  mbeeppc,    "Applied Technology",  "Microbee Premium PC85" , 0 )
COMP( 1986, mbeett,   mbee,     0,      mbeett,   mbee256,  mbee_state,  mbeett,     "Applied Technology",  "Microbee Teleterm" , MACHINE_NOT_WORKING )
COMP( 1986, mbee56,   mbee,     0,      mbee56,   mbee,     mbee_state,  mbee56,     "Applied Technology",  "Microbee 56k" , MACHINE_NOT_WORKING )
COMP( 1986, mbee128,  mbee,     0,      mbee128,  mbee128,  mbee_state,  mbee128,    "Applied Technology",  "Microbee 128k Standard" , MACHINE_NOT_WORKING )
COMP( 1986, mbee128p, mbee,     0,      mbee128p, mbee128,  mbee_state,  mbee128,    "Applied Technology",  "Microbee 128k Premium" , MACHINE_NOT_WORKING )
COMP( 1987, mbee256,  mbee,     0,      mbee256,  mbee256,  mbee_state,  mbee256,    "Applied Technology",  "Microbee 256TC" , MACHINE_NOT_WORKING )
