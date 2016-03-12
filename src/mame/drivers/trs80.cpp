// license:BSD-3-Clause
// copyright-holders:Robbbert and unknown others
/***************************************************************************
TRS80 memory map

0000-2fff ROM                 R   D0-D7
3000-37ff ROM on Model III        R   D0-D7
      unused on Model I
37de      UART status             R/W D0-D7
37df      UART data           R/W D0-D7
37e0      interrupt latch address (lnw80 = for the realtime clock)
37e1      select disk drive 0         W
37e2      cassette drive latch address    W
37e3      select disk drive 1         W
37e4      select which cassette unit      W   D0-D1 (D0 selects unit 1, D1 selects unit 2)
37e5      select disk drive 2         W
37e7      select disk drive 3         W
37e0-37e3 floppy motor            W   D0-D3
      or floppy head select   W   D3
37e8      send a byte to printer          W   D0-D7
37e8      read printer status             R   D7
37ec-37ef FDC WD179x              R/W D0-D7
37ec      command             W   D0-D7
37ec      status              R   D0-D7
37ed      track               R/W D0-D7
37ee      sector              R/W D0-D7
37ef      data                R/W D0-D7
3800-38ff keyboard matrix         R   D0-D7
3900-3bff unused - kbd mirrored
3c00-3fff video RAM               R/W D0-D5,D7 (or D0-D7)
4000-ffff RAM

Interrupts:
IRQ mode 1
NMI

Printer: Level II usually 37e8; System80 uses port FD; Model 4 uses port F8.
Uart: TR1602, equivalent to the uart used in the Exidy Sorcerer.
System80 has non-addressable dip switches to set the UART control register.
System80 and LNW80 have non-addressable links to set the baud rate. Receive and Transmit clocks are tied together.
It is assumed that the TRS80L2 UART setup is identical to the System80, apart from the address ports used.
Due to the above, the only working emulated UART is for the Model 3.

Cassette baud rates:    Model I level I - 250 baud
        Model I level II and all clones - 500 baud
        Model III/4 - 500 and 1500 baud selectable at boot time
        - When it says "Cass?" press L for 500 baud, or Enter otherwise.
        LNW-80 - 500 baud @1.77MHz and 1000 baud @4MHz.

I/O ports
FF:
- bits 0 and 1 are for writing a cassette
- bit 2 must be high to turn the cassette motor on, enables cassette data paths on a system-80
- bit 3 switches the display between 64 or 32 characters per line
- bit 6 remembers the 32/64 screen mode (inverted)
- bit 7 is for reading from a cassette

FE:
- bit 0 is for selecting inverse video of the whole screen on a lnw80
- bit 2 enables colour on a lnw80
- bit 3 is for selecting roms (low) or 16k hires area (high) on a lnw80
- bit 4 selects internal cassette player (low) or external unit (high) on a system-80

FD:
- Read printer status on a system-80
- Write to printer on a system-80

F9:
- UART data (write) status (read) on a system-80

F8:
- UART data (read) status (write) on a system-80
- Write to printer (Model III/4)
- Read printer status (Model III/4)

EB:
- UART data (read and write) on a Model III/4

EA:
- UART status (read and write) on a Model III/4

E9:
- UART Configuration jumpers (read) on a Model III/4
- Set baud rate (Model III/4)

E8:
- UART Modem Status register (read) on a Model III/4
- UART Master Reset (write) on a Model III/4

Model 4 - C0-CF = hard drive (optional)
    - 90-93 write sound (optional)
    - 80-8F hires graphics (optional)

About the ht1080z - This was made for schools in Hungary. Each comes with a BASIC extension roms
    which activated Hungarian features. To activate - start emulation - enter SYSTEM
    Enter /12288 and the extensions will be installed and you are returned to READY.
    The ht1080z is identical to the System 80, apart from the character rom.
    The ht1080z2 has a modified extension rom and character generator.

About the RTC - The time is incremented while ever the cursor is flashing. It is stored in a series
    of bytes in the computer's work area. The bytes are in a certain order, this is:
    seconds, minutes, hours, year, day, month. On a model 1, the seconds are stored at
    0x4041, while on the model 4 it is 0x4217. A reboot always sets the time to zero.

Model 4 memory organisation -
    Mode 0: ROM=0-37E7 and 37EA-3FFF; Printer=37E8-37E9; Keyboard=3800-3BFF; Video=3C00-3FFF
    Mode 1: Keyboard and Video as above; 0-3FFF read=ROM and write=RAM
    Mode 2: Keyboard=F400-F7FF; Video=F800-FFFF; the rest is RAM
    Mode 3: All RAM
    In the "maincpu" memory map, the first 64k is given to the ROM, keyboard, printer and video,
        while the second 64k is RAM that is switched in as needed. The area from 4800-FFFF
        is considered a "black hole", any writes to there will disappear.
    The video is organised as 2 banks of 0x400 bytes, except in Mode 2 where it becomes contiguous.

Model 4P - is the same as Model 4 except:
    - ROM is only 0000-0FFF, while 1000-37FF is given over to RAM
    - There is no cassette support in hardware.

***************************************************************************

Not dumped (to our knowledge):
 TRS80 Japanese bios
 TRS80 Katakana Character Generator
 TRS80 Small English Character Generator
 TRS80 Model III old version Character Generator
 TRS80 Model II bios and boot disk

Not emulated:
 TRS80 Japanese kana/ascii switch and alternate keyboard
 TRS80 Model III/4 Hard drive, Graphics board, Alternate Character set
 LNW80 1.77 / 4.0 MHz switch (this is a physical switch)

Virtual floppy disk formats are JV1, JV3, and DMK. Only the JV1 is emulated.
There don't seem to be any JV1 boot disks for Model III/4.

***************************************************************************/

#include "includes/trs80.h"
#include "formats/trs80_dsk.h"
#include "formats/dmk_dsk.h"


static ADDRESS_MAP_START( trs80_map, AS_PROGRAM, 8, trs80_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x3800, 0x38ff) AM_READ(trs80_keyboard_r)
	AM_RANGE(0x3c00, 0x3fff) AM_READWRITE(trs80_videoram_r, trs80_videoram_w) AM_SHARE("p_videoram")
	AM_RANGE(0x4000, 0x7fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( trs80_io, AS_IO, 8, trs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xff, 0xff) AM_READWRITE(trs80_ff_r, trs80_ff_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( model1_map, AS_PROGRAM, 8, trs80_state )
	AM_RANGE(0x0000, 0x377f) AM_ROM // sys80,ht1080 needs up to 375F
	AM_RANGE(0x37de, 0x37de) AM_READWRITE(sys80_f9_r, sys80_f8_w)
	AM_RANGE(0x37df, 0x37df) AM_READWRITE(trs80m4_eb_r, trs80m4_eb_w)
	AM_RANGE(0x37e0, 0x37e3) AM_READWRITE(trs80_irq_status_r, trs80_motor_w)
	AM_RANGE(0x37e4, 0x37e7) AM_WRITE(trs80_cassunit_w)
	AM_RANGE(0x37e8, 0x37eb) AM_READWRITE(trs80_printer_r, trs80_printer_w)
	AM_RANGE(0x37ec, 0x37ec) AM_READ(trs80_wd179x_r)
	AM_RANGE(0x37ec, 0x37ec) AM_DEVWRITE("fdc", fd1793_t, cmd_w)
	AM_RANGE(0x37ed, 0x37ed) AM_DEVREADWRITE("fdc", fd1793_t, track_r, track_w)
	AM_RANGE(0x37ee, 0x37ee) AM_DEVREADWRITE("fdc", fd1793_t, sector_r, sector_w)
	AM_RANGE(0x37ef, 0x37ef) AM_DEVREADWRITE("fdc", fd1793_t, data_r, data_w)
	AM_RANGE(0x3800, 0x38ff) AM_MIRROR(0x300) AM_READ(trs80_keyboard_r)
	AM_RANGE(0x3c00, 0x3fff) AM_READWRITE(trs80_videoram_r, trs80_videoram_w) AM_SHARE("p_videoram")
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( model1_io, AS_IO, 8, trs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xff, 0xff) AM_READWRITE(trs80_ff_r, trs80_ff_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sys80_io, AS_IO, 8, trs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf8, 0xf8) AM_READWRITE(trs80m4_eb_r, sys80_f8_w)
	AM_RANGE(0xf9, 0xf9) AM_READWRITE(sys80_f9_r, trs80m4_eb_w)
	AM_RANGE(0xfd, 0xfd) AM_READWRITE(trs80_printer_r, trs80_printer_w)
	AM_RANGE(0xfe, 0xfe) AM_WRITE(sys80_fe_w)
	AM_RANGE(0xff, 0xff) AM_READWRITE(trs80_ff_r, trs80_ff_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lnw80_map, AS_PROGRAM, 8, trs80_state )
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( lnw80_io, AS_IO, 8, trs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xe8, 0xe8) AM_READWRITE(trs80m4_e8_r, trs80m4_e8_w)
	AM_RANGE(0xe9, 0xe9) AM_READ_PORT("E9")
	AM_RANGE(0xea, 0xea) AM_READWRITE(trs80m4_ea_r, trs80m4_ea_w)
	AM_RANGE(0xeb, 0xeb) AM_READWRITE(trs80m4_eb_r, trs80m4_eb_w)
	AM_RANGE(0xfe, 0xfe) AM_READWRITE(lnw80_fe_r, lnw80_fe_w)
	AM_RANGE(0xff, 0xff) AM_READWRITE(trs80_ff_r, trs80_ff_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( model3_map, AS_PROGRAM, 8, trs80_state )
ADDRESS_MAP_END

static ADDRESS_MAP_START( model3_io, AS_IO, 8, trs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xe0, 0xe3) AM_READWRITE(trs80m4_e0_r, trs80m4_e0_w)
	AM_RANGE(0xe4, 0xe4) AM_READWRITE(trs80m4_e4_r, trs80m4_e4_w)
	AM_RANGE(0xe8, 0xe8) AM_READWRITE(trs80m4_e8_r, trs80m4_e8_w)
	AM_RANGE(0xe9, 0xe9) AM_READ_PORT("E9") AM_WRITE(trs80m4_e9_w)
	AM_RANGE(0xea, 0xea) AM_READWRITE(trs80m4_ea_r, trs80m4_ea_w)
	AM_RANGE(0xeb, 0xeb) AM_READWRITE(trs80m4_eb_r, trs80m4_eb_w)
	AM_RANGE(0xec, 0xef) AM_READWRITE(trs80m4_ec_r, trs80m4_ec_w)
	AM_RANGE(0xf0, 0xf0) AM_READ(trs80_wd179x_r)
	AM_RANGE(0xf0, 0xf0) AM_DEVWRITE("fdc", fd1793_t, cmd_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREADWRITE("fdc", fd1793_t, track_r, track_w)
	AM_RANGE(0xf2, 0xf2) AM_DEVREADWRITE("fdc", fd1793_t, sector_r, sector_w)
	AM_RANGE(0xf3, 0xf3) AM_DEVREADWRITE("fdc", fd1793_t, data_r, data_w)
	AM_RANGE(0xf4, 0xf4) AM_WRITE(trs80m4_f4_w)
	AM_RANGE(0xf8, 0xfb) AM_READWRITE(trs80_printer_r, trs80_printer_w)
	AM_RANGE(0xfc, 0xff) AM_READWRITE(trs80m4_ff_r, trs80m4_ff_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( model4_io, AS_IO, 8, trs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x84, 0x87) AM_WRITE(trs80m4_84_w)
	AM_RANGE(0x88, 0x89) AM_WRITE(trs80m4_88_w)
	AM_RANGE(0x90, 0x93) AM_WRITE(trs80m4_90_w)
	AM_RANGE(0xe0, 0xe3) AM_READWRITE(trs80m4_e0_r, trs80m4_e0_w)
	AM_RANGE(0xe4, 0xe4) AM_READWRITE(trs80m4_e4_r, trs80m4_e4_w)
	AM_RANGE(0xe8, 0xe8) AM_READWRITE(trs80m4_e8_r, trs80m4_e8_w)
	AM_RANGE(0xe9, 0xe9) AM_READ_PORT("E9") AM_WRITE(trs80m4_e9_w)
	AM_RANGE(0xea, 0xea) AM_READWRITE(trs80m4_ea_r, trs80m4_ea_w)
	AM_RANGE(0xeb, 0xeb) AM_READWRITE(trs80m4_eb_r, trs80m4_eb_w)
	AM_RANGE(0xec, 0xef) AM_READWRITE(trs80m4_ec_r, trs80m4_ec_w)
	AM_RANGE(0xf0, 0xf0) AM_READ(trs80_wd179x_r)
	AM_RANGE(0xf0, 0xf0) AM_DEVWRITE("fdc", fd1793_t, cmd_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREADWRITE("fdc", fd1793_t, track_r, track_w)
	AM_RANGE(0xf2, 0xf2) AM_DEVREADWRITE("fdc", fd1793_t, sector_r, sector_w)
	AM_RANGE(0xf3, 0xf3) AM_DEVREADWRITE("fdc", fd1793_t, data_r, data_w)
	AM_RANGE(0xf4, 0xf4) AM_WRITE(trs80m4_f4_w)
	AM_RANGE(0xf8, 0xfb) AM_READWRITE(trs80_printer_r, trs80_printer_w)
	AM_RANGE(0xfc, 0xff) AM_READWRITE(trs80m4_ff_r, trs80m4_ff_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( model4p_io, AS_IO, 8, trs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x84, 0x87) AM_WRITE(trs80m4_84_w)
	AM_RANGE(0x88, 0x89) AM_WRITE(trs80m4_88_w)
	AM_RANGE(0x90, 0x93) AM_WRITE(trs80m4_90_w)
	AM_RANGE(0x9c, 0x9f) AM_WRITE(trs80m4p_9c_w)
	AM_RANGE(0xe0, 0xe3) AM_READWRITE(trs80m4_e0_r, trs80m4_e0_w)
	AM_RANGE(0xe4, 0xe4) AM_READWRITE(trs80m4_e4_r, trs80m4_e4_w)
	AM_RANGE(0xe8, 0xe8) AM_READWRITE(trs80m4_e8_r, trs80m4_e8_w)
	AM_RANGE(0xe9, 0xe9) AM_READ_PORT("E9") AM_WRITE(trs80m4_e9_w)
	AM_RANGE(0xea, 0xea) AM_READWRITE(trs80m4_ea_r, trs80m4_ea_w)
	AM_RANGE(0xeb, 0xeb) AM_READWRITE(trs80m4_eb_r, trs80m4_eb_w)
	AM_RANGE(0xec, 0xef) AM_READWRITE(trs80m4_ec_r, trs80m4_ec_w)
	AM_RANGE(0xf0, 0xf0) AM_READ(trs80_wd179x_r)
	AM_RANGE(0xf0, 0xf0) AM_DEVWRITE("fdc", fd1793_t, cmd_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREADWRITE("fdc", fd1793_t, track_r, track_w)
	AM_RANGE(0xf2, 0xf2) AM_DEVREADWRITE("fdc", fd1793_t, sector_r, sector_w)
	AM_RANGE(0xf3, 0xf3) AM_DEVREADWRITE("fdc", fd1793_t, data_r, data_w)
	AM_RANGE(0xf4, 0xf4) AM_WRITE(trs80m4_f4_w)
	AM_RANGE(0xf8, 0xfb) AM_READWRITE(trs80_printer_r, trs80_printer_w)
	AM_RANGE(0xfc, 0xff) AM_READWRITE(trs80m4_ff_r, trs80m4_ff_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritum_map, AS_PROGRAM, 8, trs80_state )
	AM_RANGE(0x0000, 0x37ff) AM_ROM
	AM_RANGE(0x3800, 0x38ff) AM_MIRROR(0x300) AM_READ(trs80_keyboard_r)
	AM_RANGE(0x3c00, 0x3fff) AM_READWRITE(trs80_videoram_r, trs80_videoram_w) AM_SHARE("p_videoram")
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritum_io, AS_IO, 8, trs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// There are specific writes to ports 03, F3, F7, F8, FA, FB, FD
	// so perhaps this system uses devices at these locations.
	// The disk input expects values that are different to the usual,
	// eg. port F0 should be 5, port F2 should have bit 3 set.
	//AM_RANGE(0x03, 0x03) unknown
	AM_RANGE(0xf0, 0xf0) AM_READ(trs80_wd179x_r)
	AM_RANGE(0xf0, 0xf0) AM_DEVWRITE("fdc", fd1793_t, cmd_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREADWRITE("fdc", fd1793_t, track_r, track_w)
	AM_RANGE(0xf2, 0xf2) AM_DEVREADWRITE("fdc", fd1793_t, sector_r, sector_w)
	AM_RANGE(0xf3, 0xf3) AM_DEVREADWRITE("fdc", fd1793_t, data_r, data_w)
	AM_RANGE(0xf4, 0xf4) AM_WRITE(trs80m4_f4_w)
	AM_RANGE(0xf8, 0xfb) AM_READWRITE(trs80_printer_r, trs80_printer_w)
	//AM_RANGE(0xfc, 0xfd) unknown
	AM_RANGE(0xff, 0xff) AM_READWRITE(trs80_ff_r, trs80_ff_w)
ADDRESS_MAP_END

/**************************************************************************
   w/o SHIFT                             with SHIFT
   +-------------------------------+     +-------------------------------+
   | 0   1   2   3   4   5   6   7 |     | 0   1   2   3   4   5   6   7 |
+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
|0 | @ | A | B | C | D | E | F | G |  |0 | ` | a | b | c | d | e | f | g |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|1 | H | I | J | K | L | M | N | O |  |1 | h | i | j | k | l | m | n | o |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|2 | P | Q | R | S | T | U | V | W |  |2 | p | q | r | s | t | u | v | w |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|3 | X | Y | Z | [ | \ | ] | ^ | _ |  |3 | x | y | z | { | | | } | ~ |   |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|4 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |  |4 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|5 | 8 | 9 | : | ; | , | - | . | / |  |5 | 8 | 9 | * | + | < | = | > | ? |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|  |6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|7 |SHF|ALT|PUP|PDN|INS|DEL|CTL|END|  |7 |SHF|ALT|PUP|PDN|INS|DEL|CTL|END|
+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
NB: row 7 contains some originally unused bits
    only the shift bit was there in the TRS80

2008-05 FP:
NB2: 3:3 -> 3:7 have no correspondent keys (see
    below) in the usual 53-keys keyboard ( +
    12-keys keypad) for Model I, III and 4. Where
    are the symbols above coming from?
NB3: the 12-keys keypad present in later models
    is mapped to the corrispondent keys of the
    keyboard: '0' -> '9', 'Enter', '.'
NB4: when it was added a 15-key keypad, there were
    three functions key 'F1', 'F2', 'F3'. I found no
    doc about their position in the matrix above,
    but the schematics of the clone System-80 MkII
    (which had 4 function keys) put these keys in
    3:4 -> 3:7. Right now they're not implemented
    below.

***************************************************************************/

static INPUT_PORTS_START( trs80 )
	PORT_START("CONFIG") /* IN0 */
	PORT_CONFNAME(    0x80, 0x00,   "Floppy Disc Drives")
	PORT_CONFSETTING(   0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(   0x80, DEF_STR( On ) )
	PORT_BIT(0x7f, 0x7f, IPT_UNUSED)

	PORT_START("LINE0") /* KEY ROW 0 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE)      PORT_CHAR('@')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)              PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("LINE1") /* KEY ROW 1 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("LINE2") /* KEY ROW 2 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("LINE3") /* KEY ROW 3 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	/* on Model I and Model III keyboards, there are only 53 keys (+ 12 keypad keys) and these are not connected:
	on Model I, they produce arrows and '_', on Model III either produce garbage or overlap with other keys;
	on Model 4 (which has a 15-key with 3 function keys) here are mapped 'F1', 'F2', 'F3'    */
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("(n/c)")
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("(n/c)")
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("(n/c)")
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("(n/c)")
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("(n/c)")

	PORT_START("LINE4") /* KEY ROW 4 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)              PORT_CHAR('0') PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&') PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'') PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("LINE5") /* KEY ROW 5 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(') PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')') PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_MINUS)        PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR('>') PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6") /* KEY ROW 6 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)                          PORT_CHAR(13) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(F8)) // 3rd line, 1st key from right
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(F9)) // 1st line, 1st key from right
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)                PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	/* backspace do the same as cursor left */
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)              PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)              PORT_CHAR(' ')

	PORT_START("LINE7") /* KEY ROW 7 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)                PORT_CHAR(UCHAR_SHIFT_1)
	/* These keys are only on a Model 4. The one marked CTL seems to be another shift key (4 in total). */
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("CTL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x80, 0x00, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( trs80m3 )
	PORT_INCLUDE (trs80)
	PORT_START("E9")    // these are the power-on uart settings
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x88, 0x08, "Parity")
	PORT_DIPSETTING(    0x08, DEF_STR(None))
	PORT_DIPSETTING(    0x00, "Odd")
	PORT_DIPSETTING(    0x80, "Even")
	PORT_DIPNAME( 0x10, 0x10, "Stop Bits")
	PORT_DIPSETTING(    0x10, "2")
	PORT_DIPSETTING(    0x00, "1")
	PORT_DIPNAME( 0x60, 0x60, "Bits")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x20, "6")
	PORT_DIPSETTING(    0x40, "7")
	PORT_DIPSETTING(    0x60, "8")
INPUT_PORTS_END


/**************************** F4 CHARACTER DISPLAYER ***********************************************************/
static const gfx_layout trs80_charlayout =
{
	8, 8,           /* 8 x 8 characters */
	128,            /* 128 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8        /* every char takes 8 bytes */
};

static const gfx_layout ht1080z_charlayout =
{
	5, 12,          /* 5 x 12 characters */
	128,            /* 128 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16           /* every char takes 16 bytes */
};

static const gfx_layout trs80m4_charlayout =
{
	8, 8,           /* 8 x 8 characters */
	256,            /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8        /* every char takes 8 bytes */
};

static const gfx_layout lnw80_charlayout =
{
	8, 8,           /* 8 x 8 characters */
	128,            /* 128 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 7, 5, 6, 1, 0, 2, 4, 3 },
	/* y offsets */
	{  0*8, 512*8, 256*8, 768*8, 1*8, 513*8, 257*8, 769*8 },
	8*2        /* every char takes 8 bytes */
};

static const gfx_layout radionic_charlayout =
{
	8, 16,          /* 8 x 16 characters */
	256,            /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 2048*8, 2049*8, 2050*8, 2051*8, 2052*8, 2053*8, 2054*8, 2055*8 },
	8*8        /* every char takes 16 bytes */
};

static const gfx_layout meritum_charlayout =
{
	6, 11,          /* 8 x 16 characters */
	256,            /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8 },
	8*16           /* every char takes 16 bytes (unused scanlines are blank) */
};

static GFXDECODE_START(trs80)
	GFXDECODE_ENTRY( "chargen", 0, trs80_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START(ht1080z)
	GFXDECODE_ENTRY( "chargen", 0, ht1080z_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START(trs80m4)
	GFXDECODE_ENTRY( "chargen", 0, trs80m4_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START(lnw80)
	GFXDECODE_ENTRY( "chargen", 0, lnw80_charlayout, 0, 4 )
GFXDECODE_END

static GFXDECODE_START(radionic)
	GFXDECODE_ENTRY( "chargen", 0, radionic_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START(meritum)
	GFXDECODE_ENTRY( "chargen", 0, meritum_charlayout, 0, 1 )
GFXDECODE_END


FLOPPY_FORMATS_MEMBER( trs80_state::floppy_formats )
	FLOPPY_TRS80_FORMAT,
	FLOPPY_DMK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( trs80_floppies )
	SLOT_INTERFACE("sssd", FLOPPY_525_SSSD)
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( trs80, trs80_state )       // the original model I, level I, with no extras
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 1796000)        /* 1.796 MHz */
	MCFG_CPU_PROGRAM_MAP(trs80_map)
	MCFG_CPU_IO_MAP(trs80_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*6, 16*12)
	MCFG_SCREEN_VISIBLE_AREA(0,64*6-1,0,16*12-1)
	MCFG_SCREEN_UPDATE_DRIVER(trs80_state, screen_update_trs80)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", trs80)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_CASSETTE_ADD( "cassette" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( model1, trs80 )      // model I, level II
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( model1_map)
	MCFG_CPU_IO_MAP( model1_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(trs80_state, trs80_rtc_interrupt,  40)

	/* devices */
	MCFG_CASSETTE_MODIFY( "cassette" )
	MCFG_CASSETTE_FORMATS(trs80l2_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)

	MCFG_QUICKLOAD_ADD("quickload", trs80_state, trs80_cmd, "cmd", 0.5)

	MCFG_FD1793_ADD("fdc", XTAL_1MHz) // todo: should be fd1771
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(trs80_state,trs80_fdc_intrq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", trs80_floppies, "sssd", trs80_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", trs80_floppies, "sssd", trs80_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", trs80_floppies, "", trs80_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", trs80_floppies, "", trs80_state::floppy_formats)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit7))
	MCFG_CENTRONICS_PERROR_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit6))
	MCFG_CENTRONICS_SELECT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit5))
	MCFG_CENTRONICS_FAULT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit4))

	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_DEVICE_ADD( "tr1602", AY31015, 0 )
	MCFG_AY31015_RX_CLOCK(0.0)
	MCFG_AY31015_TX_CLOCK(0.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( model3, model1 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( model3_map)
	MCFG_CPU_IO_MAP( model3_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(trs80_state, trs80_rtc_interrupt,  30)

	MCFG_MACHINE_RESET_OVERRIDE(trs80_state, trs80m4 )

	MCFG_GFXDECODE_MODIFY("gfxdecode",trs80m4)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(trs80_state, screen_update_trs80m4)
	MCFG_SCREEN_SIZE(80*8, 240)
	MCFG_SCREEN_VISIBLE_AREA(0,80*8-1,0,239)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( model4, model3 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_IO_MAP( model4_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( model4p, model3 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_IO_MAP( model4p_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sys80, model1 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_IO_MAP( sys80_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ht1080z, sys80 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(trs80_state, screen_update_ht1080z)
	MCFG_GFXDECODE_MODIFY("gfxdecode", ht1080z)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lnw80, model1 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( lnw80_map)
	MCFG_CPU_IO_MAP( lnw80_io)
	MCFG_MACHINE_RESET_OVERRIDE(trs80_state, lnw80 )

	MCFG_GFXDECODE_MODIFY("gfxdecode",lnw80)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(8)
	MCFG_PALETTE_INIT_OWNER(trs80_state,lnw80)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(80*6, 16*12)
	MCFG_SCREEN_VISIBLE_AREA(0,80*6-1,0,16*12-1)
	MCFG_SCREEN_UPDATE_DRIVER(trs80_state, screen_update_lnw80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( radionic, model1 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(64*8, 16*16)
	MCFG_SCREEN_VISIBLE_AREA(0,64*8-1,0,16*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(trs80_state, screen_update_radionic)
	MCFG_GFXDECODE_MODIFY("gfxdecode", radionic)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( meritum, sys80 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( meritum_map)
	MCFG_CPU_IO_MAP( meritum_io)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(trs80_state, screen_update_meritum)
	MCFG_GFXDECODE_MODIFY("gfxdecode", meritum)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(trs80)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD("level1.rom",   0x0000, 0x1000, CRC(70d06dff) SHA1(20d75478fbf42214381e05b14f57072f3970f765))

	ROM_REGION(0x00400, "chargen",0)
	ROM_LOAD("trs80m1.chr",  0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7))
ROM_END

ROM_START(trs80l2)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_SYSTEM_BIOS(0, "level2", "Radio Shack Level II Basic")
	ROMX_LOAD("trs80.z33",   0x0000, 0x1000, CRC(37c59db2) SHA1(e8f8f6a4460a6f6755873580be6ff70cebe14969), ROM_BIOS(1))
	ROMX_LOAD("trs80.z34",   0x1000, 0x1000, CRC(05818718) SHA1(43c538ca77623af6417474ca5b95fb94205500c1), ROM_BIOS(1))
	ROMX_LOAD("trs80.zl2",   0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rsl2", "R/S L2 Basic")
	ROMX_LOAD("trs80alt.z33",0x0000, 0x1000, CRC(be46faf5) SHA1(0e63fc11e207bfd5288118be5d263e7428cc128b), ROM_BIOS(2))
	ROMX_LOAD("trs80alt.z34",0x1000, 0x1000, CRC(6c791c2d) SHA1(2a38e0a248f6619d38f1a108eea7b95761cf2aee), ROM_BIOS(2))
	ROMX_LOAD("trs80alt.zl2",0x2000, 0x1000, CRC(55b3ad13) SHA1(6279f6a68f927ea8628458b278616736f0b3c339), ROM_BIOS(2))

	ROM_REGION(0x00400, "chargen",0)
	ROM_LOAD("trs80m1.chr",  0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7))
ROM_END

ROM_START(radionic)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD("ep1.bin",      0x0000, 0x1000, CRC(e8908f44) SHA1(7a5a60c3afbeb6b8434737dd302332179a7fca59))
	ROM_LOAD("ep2.bin",      0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155))
	ROM_LOAD("ep3.bin",      0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369))
	ROM_LOAD("ep4.bin",      0x3000, 0x0400, CRC(70f90f26) SHA1(cbee70da04a3efac08e50b8e3a270262c2440120))
	ROM_CONTINUE(0x3000, 0x400)
	ROM_CONTINUE(0x3000, 0x600)
	ROM_IGNORE(0x200)

	ROM_REGION(0x01000, "chargen",0)
	ROM_LOAD("trschar.bin",  0x0000, 0x1000, CRC(02e767b6) SHA1(c431fcc6bd04ce2800ca8c36f6f8aeb2f91ce9f7))
ROM_END

ROM_START(sys80)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD("sys80rom.1",   0x0000, 0x1000, CRC(8f5214de) SHA1(d8c052be5a2d0ec74433043684791d0554bf203b))
	ROM_LOAD("sys80rom.2",   0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155))
	ROM_LOAD("trs80.zl2",    0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369))
	/* This rom turns the system80 into the "blue label" version. SYSTEM then /12288 to activate. */
	ROM_LOAD("sys80.ext",    0x3000, 0x0800, CRC(2a851e33) SHA1(dad21ec60973eb66e499fe0ecbd469118826a715))

	ROM_REGION(0x00400, "chargen",0)
	ROM_LOAD("trs80m1.chr",  0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7))
ROM_END

ROM_START(lnw80)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD("lnw_a.bin",    0x0000, 0x0800, CRC(e09f7e91) SHA1(cd28e72efcfebde6cf1c7dbec4a4880a69e683da))
	ROM_LOAD("lnw_a1.bin",   0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD("lnw_b.bin",    0x1000, 0x0800, CRC(c4303568) SHA1(13e3d81c6f0de0e93956fa58c465b5368ea51682))
	ROM_LOAD("lnw_b1.bin",   0x1800, 0x0800, CRC(3a5ea239) SHA1(8c489670977892d7f2bfb098f5df0b4dfa8fbba6))
	ROM_LOAD("lnw_c.bin",    0x2000, 0x0800, CRC(2ba025d7) SHA1(232efbe23c3f5c2c6655466ebc0a51cf3697be9b))
	ROM_LOAD("lnw_c1.bin",   0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))

	ROM_REGION(0x00800, "chargen",0)
	ROM_LOAD("lnw_chr.bin",  0x0000, 0x0800, CRC(c89b27df) SHA1(be2a009a07e4378d070002a558705e9a0de59389))

	ROM_REGION(0x04400, "gfx2", ROMREGION_ERASEFF) /* 0x4000 for trs80_gfxram + 0x400 for videoram */
ROM_END

ROM_START(trs80m3)
/* ROMS we have and are missing:
HAVE    TRS-80 Model III Level 1 ROM (U104)
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM A (U104) ver. CRC BBC4
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM A (U104) ver. CRC DA75
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM A (U104) ver. CRC 9639
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM B (U105) ver. CRC 407C
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM C (U106) ver. CRC 2B91 - early mfg. #80040316
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM C (U106) ver. CRC 278A - no production REV A
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM C (U106) ver. CRC 2EF8 - Manufacturing #80040316 REV B
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM C (U106) ver. CRC 2F84 - Manufacturing #80040316 REV C
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM C ver. CRC 2764 - Network III v1
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM C ver. CRC 276A - Network III v2
MISSING TRS-80 Model III Level 2 (BELGIUM) CRC ????
Note: Be careful when dumping rom C: if dumped on the trs-80 m3 with software, bytes 0x7e8 and 0x7e9 (addresses 0x37e8, 0x0x37e9)
      will read as 0xFF 0xFF; on the original rom, these bytes are 0x00 0x00 (for eproms) or 0xAA 0xAA (for mask roms), those two bytes are used for printer status on the trs-80 and are mapped on top of the rom; This problem should be avoided by pulling the rom chips and dumping them directly.
*/
	ROM_REGION(0x20000, "maincpu",0)
	ROM_SYSTEM_BIOS(0, "trs80m3_revc", "Level 2 bios, RomC Rev C")
	ROMX_LOAD("8041364.u104", 0x0000, 0x2000, CRC(ec0c6daa) SHA1(257cea6b9b46912d4681251019ec2b84f1b95fc8), ROM_BIOS(1)) // Label: "SCM91248C // Tandy (c) 80 // 8041364 // 8134" (Level 2 bios ROM A '9639')
	ROMX_LOAD("8040332.u105", 0x2000, 0x1000, CRC(ed4ee921) SHA1(ec0a19d4b72f71e51965de63250009c3c4e4cab3), ROM_BIOS(1)) // Label: "SCM91619P // Tandy (c) 80 // 8040332 // QQ8117", (Level 2 bios ROM B '407c')
	ROMX_LOAD("8040316c.u106", 0x3000, 0x0800, CRC(c8f79433) SHA1(6f395bba822d39d3cd2b73c8ea25aab4c4c26da7), ROM_BIOS(1)) // Label: "SCM91692P // Tandy (c) 81 // 8040316-C // QQ8220" (Level 2 bios ROM C REV C '2f84')
	ROM_SYSTEM_BIOS(1, "trs80m3_revb", "Level 2 bios, RomC Rev B")
	ROMX_LOAD("8041364.u104", 0x0000, 0x2000, CRC(ec0c6daa) SHA1(257cea6b9b46912d4681251019ec2b84f1b95fc8), ROM_BIOS(2)) // Label: "SCM91248C // Tandy (c) 80 // 8041364 // 8134" (Level 2 bios ROM A '9639')
	ROMX_LOAD("8040332.u105", 0x2000, 0x1000, CRC(ed4ee921) SHA1(ec0a19d4b72f71e51965de63250009c3c4e4cab3), ROM_BIOS(2)) // Label: "SCM91619P // Tandy (c) 80 // 8040332 // QQ8117", (Level 2 bios ROM B '407c')
	ROMX_LOAD("8040316b.u106", 0x3000, 0x0800, CRC(84a5702d) SHA1(297dca756a9d3c6fd13e0fa6f93d172ff795b520), ROM_BIOS(2)) // Label: "SCM91692P // Tandy (c) 80 // 8040316B // QQ8040" (Level 2 bios ROM C REV B '2ef8')
	ROM_SYSTEM_BIOS(2, "trs80m3_n3v2", "Level 2 bios, Network III v2 (student)")
	ROMX_LOAD("8041364.u104", 0x0000, 0x2000, CRC(ec0c6daa) SHA1(257cea6b9b46912d4681251019ec2b84f1b95fc8), ROM_BIOS(3)) // Label: "SCM91248C // Tandy (c) 80 // 8041364 // 8134" (Level 2 bios ROM A '9639')
	ROMX_LOAD("8040332.u105", 0x2000, 0x1000, CRC(ed4ee921) SHA1(ec0a19d4b72f71e51965de63250009c3c4e4cab3), ROM_BIOS(3)) // Label: "SCM91619P // Tandy (c) 80 // 8040332 // QQ8117" (Level 2 bios ROM B '407c')
	ROMX_LOAD("276a.u106", 0x3000, 0x0800, CRC(7d38720a) SHA1(bef621e5ae2a8c1f9e7f6325b7841f5ab8ab7e6a), ROM_BIOS(3)) // 2716 EPROM Label: "MOD.III // ROM C // (276A)" (Network III v2 ROM C '276a')
	ROM_SYSTEM_BIOS(3, "trs80m3_l1", "Level 1 bios")
	ROMX_LOAD("8040032.u104", 0x0000, 0x1000, CRC(6418d641) SHA1(f823ab6ceb102588d27e5f5c751e31175289291c), ROM_BIOS(4) ) // Label: "8040032 // (M) QQ8028 // SCM91616P"; Silkscreen: "TANDY // (C) '80"; (Level 1 bios)

	ROM_REGION(0x00800, "chargen",0)    /* correct for later systems; the trs80m3_l1 bios uses the non-a version of this rom, dump is pending */
	ROM_LOAD("8044316.u36", 0x0000, 0x0800, NO_DUMP) // Label: "(M) // SCM91665P // 8044316 // QQ8029" ('no-letter' revision)
	ROM_LOAD("8044316a.u36", 0x0000, 0x0800, CRC(444c8b60) SHA1(c52ee41439bd5e57c3b113ebfd61c951e2af4446)) // Label: "Tandy (C) 81 // 8044316A // 8206" (rev A)
ROM_END

// for model 4 and 4p info, see http://vt100.net/mirror/harte/Radio%20Shack/TRS-80%20Model%204_4P%20Soft%20Tech%20Ref.pdf
ROM_START(trs80m4)
	ROM_REGION(0x20000, "maincpu",0)
	ROM_LOAD("trs80m4.rom",  0x0000, 0x3800, BAD_DUMP CRC(1a92d54d) SHA1(752555fdd0ff23abc9f35c6e03d9d9b4c0e9677b)) // should be split into 3 roms, roms A, B, C, exactly like trs80m3; in fact, roms A and B are shared between both systems.

	ROM_REGION(0x00800, "chargen",0)
	ROM_LOAD("8044316a.u36", 0x0000, 0x0800, CRC(444c8b60) SHA1(c52ee41439bd5e57c3b113ebfd61c951e2af4446)) // according to parts catalog, this is the correct rom for both model 3 and 4
ROM_END

ROM_START(trs80m4p) // uses a completely different memory map scheme to the others; the trs-80 model 3 roms are loaded from a boot disk, the only rom on the machine is a bootloader; bootloader can be banked out of 0x0000-0x1000 space which is replaced with ram; see the tech ref pdf, pdf page 62
	ROM_REGION(0x20000, "maincpu",0)
	ROM_SYSTEM_BIOS(0, "trs80m4p", "Level 2 bios, gate array machine")
	ROMX_LOAD("8075332.u69", 0x0000, 0x1000, CRC(3a738aa9) SHA1(6393396eaa10a84b9e9f0cf5930aba73defc5c52), ROM_BIOS(1)) // Label: "SCM95060P // 8075332 // TANDY (C) 1983 // 8421" at location U69 (may be located at U70 on some pcb revisions)
	ROM_SYSTEM_BIOS(1, "trs80m4p_hack", "Disk loader hack")
	ROMX_LOAD("trs80m4p_loader_hack.rom", 0x0000, 0x01f8, CRC(7ff336f4) SHA1(41184f5240b4b54f3804f5a22b4d78bbba52ed1d), ROM_BIOS(2))

	ROM_REGION(0x00800, "chargen",0)
	ROM_LOAD("8049007.u103", 0x0000, 0x0800, CRC(1ac44bea) SHA1(c9426ab2b2aa5380dc97a7b9c048ccd1bbde92ca)) // Label: "SCM95987P // 8049007 // TANDY (C) 1983 // 8447" at location U103 (may be located at U43 on some pcb revisions)
ROM_END

ROM_START(ht1080z)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD("ht1080z.rom",  0x0000, 0x3000, CRC(2bfef8f7) SHA1(7a350925fd05c20a3c95118c1ae56040c621be8f))
	ROM_LOAD("sys80.ext",    0x3000, 0x0800, CRC(2a851e33) SHA1(dad21ec60973eb66e499fe0ecbd469118826a715))

	ROM_REGION(0x00800, "chargen",0)
	ROM_LOAD("ht1080z.chr",  0x0000, 0x0800, CRC(e8c59d4f) SHA1(a15f30a543e53d3e30927a2e5b766fcf80f0ae31))
ROM_END

ROM_START(ht1080z2)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD("ht1080z.rom",  0x0000, 0x3000, CRC(2bfef8f7) SHA1(7a350925fd05c20a3c95118c1ae56040c621be8f))
	ROM_LOAD("ht1080z2.ext", 0x3000, 0x0800, CRC(07415ac6) SHA1(b08746b187946e78c4971295c0aefc4e3de97115))

	ROM_REGION(0x00800, "chargen",0)
	ROM_LOAD("ht1080z2.chr", 0x0000, 0x0800, CRC(6728f0ab) SHA1(1ba949f8596f1976546f99a3fdcd3beb7aded2c5))
ROM_END

ROM_START(ht108064)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD("ht108064.rom", 0x0000, 0x3000, CRC(48985a30) SHA1(e84cf3121f9e0bb9e1b01b095f7a9581dcfaaae4))
	ROM_LOAD("ht108064.ext", 0x3000, 0x0800, CRC(fc12bd28) SHA1(0da93a311f99ec7a1e77486afe800a937778e73b))

	ROM_REGION(0x00800, "chargen",0)
	ROM_LOAD("ht108064.chr", 0x0000, 0x0800, CRC(e76b73a4) SHA1(6361ee9667bf59d50059d09b0baf8672fdb2e8af))
ROM_END

ROM_START( meritum)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD( "01.bin", 0x0000, 0x0800, CRC(ed705a47) SHA1(dae8b14eb2ddb2a8b4458215180ebc0fb781816a))
	ROM_LOAD( "02.bin", 0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD( "03.bin", 0x1000, 0x0800, CRC(a21d0d62) SHA1(6dfdf3806ed2b6502e09a1b6922f21494134cc05))
	ROM_LOAD( "04.bin", 0x1800, 0x0800, CRC(3610bdda) SHA1(602f0ba1e1267f24620f993acac019ac6342a594))
	ROM_LOAD( "05.bin", 0x2000, 0x0800, CRC(461fbf0d) SHA1(bd19187dd992168af43bd68055343d515f152624))
	ROM_LOAD( "06.bin", 0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))
	ROM_LOAD( "07.bin", 0x3000, 0x0800, CRC(044b1459) SHA1(faace7353ffbef6587b1b9e7f8b312e0892e3427))
	ROM_REGION(0x1000, "chargen", ROMREGION_INVERT)
	ROM_LOAD( "chargen.bin", 0x0000, 0x1000, CRC(3dfc6439) SHA1(6e45a27f68c3491c403b4eafe45a108f348dd2fd))
ROM_END

ROM_START( meritum_net )
	ROM_REGION(0x10000, "maincpu",0)
	ROM_LOAD( "01_447_m07_015m.bin", 0x0000, 0x0800, CRC(6d30cb49) SHA1(558241340a84eebcbbf8d92540e028e9164b6f8a))
	ROM_LOAD( "02_440_m08_01.bin",   0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD( "03_440_m09_015m.bin", 0x1000, 0x0800, CRC(88e267da) SHA1(9cb8626801f8e969f35291de43c1b643c809a3c3))
	ROM_LOAD( "04_447_m10_015m.bin", 0x1800, 0x0800, CRC(e51991e4) SHA1(a7d42436da1af405970f9f99ab34b6d9abd05adf))
	ROM_LOAD( "05_440_m11_02.bin",   0x2000, 0x0800, CRC(461fbf0d) SHA1(bd19187dd992168af43bd68055343d515f152624))
	ROM_LOAD( "06_440_m12_01.bin",   0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))
	ROM_LOAD( "07_447_m13_015m.bin", 0x3000, 0x0800, CRC(789f6964) SHA1(9b2231ca7ffd82bbca1f53988a7df833290ddbf2))
	ROM_REGION(0x1000, "chargen", ROMREGION_INVERT)
	ROM_LOAD( "char.bin", 0x0000, 0x1000, CRC(2c09a5a7) SHA1(146891b3ddfc2de95e6a5371536394a657880054))
ROM_END

DRIVER_INIT_MEMBER(trs80_state,trs80)
{
	m_mode = 0;
	m_model4 = 0;
}

DRIVER_INIT_MEMBER(trs80_state,trs80l2)
{
	m_mode = 2;
	m_model4 = 0;
}

DRIVER_INIT_MEMBER(trs80_state,trs80m4)
{
	m_mode = 0;
	m_model4 = 2;
	m_p_videoram.set_target(memregion("maincpu")->base()+0x4000,m_p_videoram.bytes());
}

DRIVER_INIT_MEMBER(trs80_state,trs80m4p)
{
	m_mode = 0;
	m_model4 = 4;
	m_p_videoram.set_target(memregion("maincpu")->base()+0x4000,m_p_videoram.bytes());
}

DRIVER_INIT_MEMBER(trs80_state,lnw80)
{
	m_mode = 0;
	m_model4 = 0;
	m_p_gfxram = memregion("gfx2")->base();
	m_p_videoram.set_target(memregion("maincpu")->base()+0x4000,m_p_videoram.bytes());
}

/*    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT    INIT         COMPANY           FULLNAME */
COMP( 1977, trs80,    0,      0,      trs80,      trs80, trs80_state,   trs80,    "Tandy Radio Shack", "TRS-80 Model I (Level I Basic)", 0 )
COMP( 1978, trs80l2,  trs80,  0,      model1,     trs80, trs80_state,   trs80l2,  "Tandy Radio Shack", "TRS-80 Model I (Level II Basic)", 0 )
COMP( 1983, radionic, trs80,  0,      radionic,   trs80, trs80_state,   trs80,    "Komtek", "Radionic", 0 )
COMP( 1980, sys80,    trs80,  0,      sys80,      trs80, trs80_state,   trs80l2,  "EACA Computers Ltd", "System-80", 0 )
COMP( 1981, lnw80,    trs80,  0,      lnw80,      trs80m3, trs80_state, lnw80,    "LNW Research", "LNW-80", 0 )
COMP( 1980, trs80m3,  trs80,  0,      model3,     trs80m3, trs80_state, trs80m4,  "Tandy Radio Shack", "TRS-80 Model III", 0 )
COMP( 1980, trs80m4,  trs80,  0,      model4,     trs80m3, trs80_state, trs80m4,  "Tandy Radio Shack", "TRS-80 Model 4", 0 )
COMP( 1983, trs80m4p, trs80,  0,      model4p,    trs80m3, trs80_state, trs80m4p, "Tandy Radio Shack", "TRS-80 Model 4P", 0 )
COMP( 1983, ht1080z,  trs80,  0,      ht1080z,    trs80, trs80_state,   trs80l2,  "Hiradastechnika Szovetkezet", "HT-1080Z Series I", 0 )
COMP( 1984, ht1080z2, trs80,  0,      ht1080z,    trs80, trs80_state,   trs80l2,  "Hiradastechnika Szovetkezet", "HT-1080Z Series II", 0 )
COMP( 1985, ht108064, trs80,  0,      ht1080z,    trs80, trs80_state,   trs80,    "Hiradastechnika Szovetkezet", "HT-1080Z/64", 0 )
COMP( 1985, meritum,  trs80,  0,      meritum,    trs80, trs80_state,   trs80l2,  "Mera-Elzab", "Meritum I (Model 2)", 0 )
COMP( 1985, meritum_net, trs80,  0,   meritum,    trs80, trs80_state,   trs80l2,  "Mera-Elzab", "Meritum I (Model 2) (network)", 0 )
