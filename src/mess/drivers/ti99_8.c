/****************************************************************************

    The MESS TI-99/8 emulation driver

    The TI-99/8 was the envisaged successor to the TI-99/4A but never passed
    its prototype state. Only a few consoles were built. The ROMs were not
    even finalized, so the few available consoles may have different
    operating system versions.

    There is some preliminary info:

Name: Texas Instruments Computer TI-99/8 (no "Home")

References:
    * machine room <http://...>
    * TI99/8 user manual
    * TI99/8 schematics
    * TI99/8 ROM source code
    * Message on TI99 yahoo group for CPU info

General:
    * a few dozen units were built in 1983, never released
    * CPU is a custom variant of tms9995 (part code MP9537): the 16-bit RAM and
      (presumably) the on-chip decrementer are disabled
    * 220kb(?) of ROM, including monitor, GPL interpreter, TI-extended basic
      II, and a P-code interpreter with a few utilities.  More specifically:
      - 32kb system ROM with GPL interpreter, TI-extended basic II and a few
        utilities (no dump, but 90% of source code is available and has been
        compiled)
      - 18kb system GROMs, with monitor and TI-extended basic II (no dump,
        but source code is available and has been compiled)
      - 4(???)kb DSR ROM for hexbus (no dump)
      - 32(?)kb speech ROM: contents are slightly different from the 99/4(a)
        speech ROMs, due to the use of a tms5220 speech synthesizer instead of
        the older tms0285 (no dump, but 99/4(a) speech ROMs should work mostly
        OK)
      - 12(???)kb ROM with PCode interpreter (no dump)
      - 2(3???)*48kb of GROMs with PCode data files (no dump)
    * 2kb SRAM (16 bytes of which are hidden), 64kb DRAM (expandable to almost
      16MBytes), 16kb vdp RAM
    * tms9118 vdp (similar to tms9918a, slightly different bus interface and
      timings)
    * I/O
      - 50-key keyboard, plus 2 optional joysticks
      - sound and speech (both ti99/4(a)-like)
      - Hex-Bus
      - Cassette
    * cartridge port on the top
    * 50-pin(?) expansion port on the back
    * Programs can enable/disable the ROM and memory mapped register areas.

Mapper:
    Mapper has 4kb page size (-> 16 pages per map file), 32 bits per page
    entry.  Address bits A0-A3 are the page index, whereas bits A4-A15 are the
    offset in the page.  Physical address space is 16Mbytes.  All pages are 4
    kBytes in length, and they can start anywhere in the 24-bit physical
    address space.  The mapper can load any of 4 map files from SRAM by DMA.
    Map file 0 is used by BIOS, file 1 by memory XOPs(?), file 2 by P-code
    interpreter(???).

    Format of map table entry:
    * bit 0: WTPROT: page is write protected if 1
    * bit 1: XPROT: page is execute protected if 1
    * bit 2: RDPROT: page is read protected if 1
    * bit 3: reserved, value is ignored
    * bits 4-7: reserved, always forced to 0
    * bits 8-23: page base address in 24-bit virtual address space

    Format of mapper control register:
    * bit 0-4: unused???
    * bit 5-6: map file to load/save (0 for file 0, 1 for file 1, etc.)
    * bit 7: 0 -> load map file from RAM, 1 -> save map file to RAM

    Format of mapper status register (cleared by read):
    * bit 0: WPE - Write-Protect Error
    * bit 1: XCE - eXeCute Error
    * bit 2: RPE - Read-Protect Error
    * bits 3-7: unused???

    Memory error interrupts are enabled by setting WTPROT/XPROT/RDPROT.  When
    an error occurs, the tms9901 INT1* pin is pulled low (active).  The pin
    remains low until the mapper status register is read.

24-bit address map:
    * >000000->00ffff: console RAM
    * >010000->feffff: expansion?
    * >ff0000->ff0fff: empty???
    * >ff1000->ff3fff: unused???
    * >ff4000->ff5fff: DSR space
    * >ff6000->ff7fff: cartridge space
    * >ff8000->ff9fff(???): >4000 ROM (normally enabled with a write to CRU >2700)
    * >ffa000->ffbfff(?): >2000 ROM
    * >ffc000->ffdfff(?): >6000 ROM


CRU map:
    Since the tms9995 supports full 15-bit CRU addresses, the >1000->17ff
    (>2000->2fff) range was assigned to support up to 16 extra expansion slot.
    The good thing with using >1000->17ff is the fact that older expansion
    cards that only decode 12 address bits will think that addresses
    >1000->17ff refer to internal TI99 peripherals (>000->7ff range), which
    suppresses any risk of bus contention.
    * >0000->001f (>0000->003e): tms9901
      - P4: 1 -> MMD (Memory Mapped Devices?) at >8000, ROM enabled
      - P5: 1 -> no P-CODE GROMs
    * >0800->17ff (>1000->2ffe): Peripheral CRU space
    * >1380->13ff (>2700->27fe): Internal DSR, with two output bits:
      - >2700: Internal DSR select (parts of Basic and various utilities)
      - >2702: SBO -> hardware reset


Memory map (TMS9901 P4 == 1):
    When TMS9901 P4 output is set, locations >8000->9fff are ignored by mapper.
    * >8000->83ff: SRAM (>8000->80ff is used by the mapper DMA controller
      to hold four map files) (r/w)
    * >8400: sound port (w)
    * >8410->87ff: SRAM (r/w)
    * >8800: VDP data read port (r)
    * >8802: VDP status read port (r)
    * >8810: memory mapper status and control registers (r/w)
    * >8c00: VDP data write port (w)
    * >8c02: VDP address and register write port (w)
    * >9000: speech synthesizer read port (r)
    * >9400: speech synthesizer write port (w)
    * >9800 GPL data read port (r)
    * >9802 GPL address read port (r)
    * >9c00 GPL data write port -- unused (w)
    * >9c02 GPL address write port (w)


Memory map (TMS9901 P5 == 0):
    When TMS9901 P5 output is cleared, locations >f840->f8ff(?) are ignored by
    mapper.
    * >f840: data port for P-code grom library 0 (r?)
    * >f880: data port for P-code grom library 1 (r?)
    * >f8c0: data port for P-code grom library 2 (r?)
    * >f842: address port for P-code grom library 0 (r/w?)
    * >f882: address port for P-code grom library 1 (r/w?)
    * >f8c2: address port for P-code grom library 2 (r/w?)


Cassette interface:
    Identical to ti99/4(a), except that the CS2 unit is not implemented.


Keyboard interface:
    The keyboard interface uses the console tms9901 PSI, but the pin assignment
    and key matrix are different from both 99/4 and 99/4a.
    - P0-P3: column select
    - INT6*-INT11*: row inputs (int6* is only used for joystick fire)

ROM file contents:
  0000-1fff ROM0                0x0000 (logical address)
  2000-3fff ROM1                0xffa000 - 0xffbfff
  4000-5fff DSR1                0xff4000 (TTS)
  6000-7fff ROM1a               0xffc000 - 0xffdfff
  8000-9fff DSR2                0xff4000 (missing; Hexbus?)

===========================================================================
Known Issues (MZ, 2010-11-07)

  KEEP IN MIND THAT TEXAS INSTRUMENTS NEVER RELEASED THE TI-99/8 AND THAT
  THERE ARE ONLY A FEW PROTOTYPES OF THE TI-99/8 AVAILABLE. ALL SOFTWARE
  MUST BE ASSUMED TO HAVE REMAINED IN A PRELIMINARY STATE.

- Extended Basic II does not start when a floppy controller is present. This is
  a problem of the prototypical XB II which we cannot solve. It seems as if only
  hexbus devices are properly supported, but we currently do not have an
  emulation for those. Thus you can currently only use cassette to load and
  save programs. You MUST not plug in any floppy controller when you intend to
  start XB II. Other cartridges (like Editor/Assembler)
  seem to be not affected by this problem and can make use of the floppy
  controllers.
    Technical detail: The designers of XB II seem to have decided to put PABs
    (Peripheral access block; contains pointers to buffers, the file name, and
    the access modes) into CPU RAM instead of the traditional storage in VDP
    RAM. The existing peripheral cards are hard-coded to interpret the given
    pointer to the PAB as pointing to a VDP RAM address. That is, as soon as
    the card is found, control is passed to the DSR (device service routine),
    the file name will not be found, and control returns with an error. It seems
    as if XB II does not properly handle this situation and may lock up
    (sometimes it starts up, but file access is still not possible).

- Multiple cartridges are not shown in the startup screen; only one
  cartridge is presented. You have to manually select the cartridges with the
  dip switch.

- SAVE and OLD MINIMEM do not work properly in XB II. It seems as if the
  mapper shadows the NVRAM of the cartridge. You will lose the contents when
  you turn off the machine.

    Raphael Nabet, 2003.

    February 2012: Rewritten as class
    Michael Zapf

*****************************************************************************/


#include "emu.h"
#include "cpu/tms9900/tms9995.h"

#include "sound/sn76496.h"
#include "sound/wave.h"
#include "machine/tms9901.h"
#include "imagedev/cassette.h"

#include "machine/ti99/videowrp.h"
#include "machine/ti99/speech8.h"

#include "machine/ti99/peribox.h"
#include "machine/ti99/mapper8.h"
#include "machine/ti99/grom.h"
#include "machine/ti99/gromport.h"
#include "machine/ti99/joyport.h"

#define VERBOSE 0
#define LOG logerror

class ti99_8 : public driver_device
{
public:
	ti99_8(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	// CRU (Communication Register Unit) handling
	DECLARE_READ8_MEMBER(cruread);
	DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_WRITE8_MEMBER(external_operation);

	// Forwarding interrupts to the CPU or CRU
	DECLARE_WRITE_LINE_MEMBER( console_ready );
	DECLARE_WRITE_LINE_MEMBER( console_ready_mapper );
	DECLARE_WRITE_LINE_MEMBER( console_reset );

	DECLARE_WRITE_LINE_MEMBER( set_tms9901_INT2 );
	DECLARE_WRITE_LINE_MEMBER( extint );
	DECLARE_WRITE_LINE_MEMBER( notconnected );

	// Connections with the system interface TMS9901
	DECLARE_READ8_MEMBER(read_by_9901);
	DECLARE_WRITE_LINE_MEMBER(keyC0);
	DECLARE_WRITE_LINE_MEMBER(keyC1);
	DECLARE_WRITE_LINE_MEMBER(keyC2);
	DECLARE_WRITE_LINE_MEMBER(keyC3);
	DECLARE_WRITE_LINE_MEMBER(CRUS);
	DECLARE_WRITE_LINE_MEMBER(PTGEN);
	DECLARE_WRITE_LINE_MEMBER(audio_gate);
	DECLARE_WRITE_LINE_MEMBER(cassette_output);
	DECLARE_WRITE_LINE_MEMBER(cassette_motor);
	DECLARE_WRITE8_MEMBER(tms9901_interrupt);

	DECLARE_WRITE_LINE_MEMBER( clock_out );
	virtual void machine_start();	
	virtual void machine_reset();

	// Some values to keep
	tms9995_device		*m_cpu;
	tms9901_device		*m_tms9901;
	gromport_device		*m_gromport;
	peribox_device		*m_peribox;
	ti998_mapper_device	*m_mapper;
	joyport_device* 	m_joyport;
	ti_video_device*	m_video;

	int 	m_firstjoy;			// First joystick. 14 for TI-99/8

	int		m_ready_line, m_ready_line1;

private:
	/* Keyboard support */
	void	set_keyboard_column(int number, int data);
	int		m_keyboard_column;
	int		m_alphalock_line;
};

/*
    Memory map. We have a configurable mapper, so we need to delegate the
    job to the mapper completely.
*/
static ADDRESS_MAP_START(memmap, AS_PROGRAM, 8, ti99_8)
	AM_RANGE(0x0000, 0xffff) AM_DEVREADWRITE(MAPPER_TAG, ti998_mapper_device, readm, writem )
ADDRESS_MAP_END

/*
    CRU map - see description above
    The TMS9901 is fully decoded according to the specification, so we only
    have 32 bits for it; the rest goes to the CRU bus
    (decoded by the "Vaquerro" chip, signal NNOICS*)
*/

static ADDRESS_MAP_START(crumap, AS_IO, 8, ti99_8)
	AM_RANGE(0x0000, 0x0003) AM_DEVREAD(TMS9901_TAG, tms9901_device, read)
	AM_RANGE(0x0000, 0x02ff) AM_READ(cruread)

	AM_RANGE(0x0000, 0x001f) AM_DEVWRITE(TMS9901_TAG, tms9901_device, write)
	AM_RANGE(0x0000, 0x17ff) AM_WRITE(cruwrite)
ADDRESS_MAP_END

/* ti99/8 : 54-key keyboard */
static INPUT_PORTS_START(ti99_8)

	/* 16 ports for keyboard and joystick */
	PORT_START("COL0")    /* col 0 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALPHA LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FCTN") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL1")    /* col 1 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 ! DEL") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL2")    /* col 2 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @ INS") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s S (LEFT)") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x X (DOWN)") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL3")    /* col 3 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e E (UP)") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d D (RIGHT)") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL4")    /* col 4 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $ CLEAR") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL5")    /* col 5 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 % BEGIN") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL6")    /* col 6 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^ PROC'D") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL7")    /* col 7 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 & AID") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL8")    /* col 8 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 * REDO") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL9")    /* col 9 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 ( BACK") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL10")    /* col 10 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL11")    /* col 11 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= + QUIT") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL12")    /* col 12 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(SPACE)") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL13")    /* col 13 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

/*****************************************************************************
    Components
******************************************************************************/

static GROM_CONFIG(grom0_config)
{
	false, 0, region_grom, 0x0000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ
};

static GROM_CONFIG(grom1_config)
{
	false, 1, region_grom, 0x2000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ
};

static GROM_CONFIG(grom2_config)
{
	false, 2, region_grom, 0x4000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ
};

/****************************************************
    PASCAL groms, 3 libraries @ 8 GROMs
    Do some macro tricks to keep writing effort low
*****************************************************/

#define pascal0_region "pascal0_region"
#define pascal12_region "pascal12_region"

#define MCFG_GROM_LIBRARY_ADD(_tag, _config)	\
	MCFG_DEVICE_ADD(#_tag "0", GROM, 0)	\
	MCFG_DEVICE_CONFIG(_config##0) \
	MCFG_DEVICE_ADD(#_tag "1", GROM, 0)	\
	MCFG_DEVICE_CONFIG(_config##1) \
	MCFG_DEVICE_ADD(#_tag "2", GROM, 0)	\
	MCFG_DEVICE_CONFIG(_config##2) \
	MCFG_DEVICE_ADD(#_tag "3", GROM, 0)	\
	MCFG_DEVICE_CONFIG(_config##3) \
	MCFG_DEVICE_ADD(#_tag "4", GROM, 0)	\
	MCFG_DEVICE_CONFIG(_config##4) \
	MCFG_DEVICE_ADD(#_tag "5", GROM, 0)	\
	MCFG_DEVICE_CONFIG(_config##5) \
	MCFG_DEVICE_ADD(#_tag "6", GROM, 0)	\
	MCFG_DEVICE_CONFIG(_config##6) \
	MCFG_DEVICE_ADD(#_tag "7", GROM, 0)	\
	MCFG_DEVICE_CONFIG(_config##7)

#define GROM_LIBRARY_CONFIG(_conf, _region) \
static GROM_CONFIG(_conf##0) \
{	false, 0, _region, 0x0000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ }; \
static GROM_CONFIG(_conf##1) \
{	false, 1, _region, 0x2000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ }; \
static GROM_CONFIG(_conf##2) \
{	false, 2, _region, 0x4000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ }; \
static GROM_CONFIG(_conf##3) \
{	false, 3, _region, 0x6000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ }; \
static GROM_CONFIG(_conf##4) \
{	false, 4, _region, 0x8000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ }; \
static GROM_CONFIG(_conf##5) \
{	false, 5, _region, 0xa000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ }; \
static GROM_CONFIG(_conf##6) \
{	false, 6, _region, 0xc000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ }; \
static GROM_CONFIG(_conf##7) \
{	false, 7, _region, 0xe000, 0x1800, DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready), GROMFREQ };

GROM_LIBRARY_CONFIG(pascal0, pascal0_region)
GROM_LIBRARY_CONFIG(pascal1, pascal12_region)
GROM_LIBRARY_CONFIG(pascal2, pascal12_region)

static GROMPORT_CONFIG(console_cartslot)
{
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready),
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_reset)
};

static PERIBOX_CONFIG( peribox_conf )
{
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, extint),			// INTA
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, notconnected),		// INTB
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready),	// READY
	0x70000												// Address bus prefix (AMA/AMB/AMC)
};

READ8_MEMBER( ti99_8::cruread )
{
//  if (VERBOSE>6) LOG("read access to CRU address %04x\n", offset << 4);
	UINT8 value = 0;

	// Similar to the bus8z_devices, just let the mapper, the gromport, and the p-box
	// decide whether they want to change the value at the CRU address
	// Also, we translate the bit addresses to base addresses
	m_mapper->crureadz(offset<<4, &value);
	m_gromport->crureadz(offset<<4, &value);
	m_peribox->crureadz(offset<<4, &value);

	if (VERBOSE>8) LOG("ti99_8: CRU %04x -> %02x\n", offset<<4, value);
	return value;
}

WRITE8_MEMBER( ti99_8::cruwrite )
{
	if (VERBOSE>8) LOG("ti99_8: CRU %04x <- %x\n", offset<<1, data);
	m_mapper->cruwrite(offset<<1, data);
	m_gromport->cruwrite(offset<<1, data);
	m_peribox->cruwrite(offset<<1, data);
}

/***************************************************************************
    TI99/8-specific tms9901 I/O handlers
    These methods are callbacks from the TMS9901 system interface. That is,
    they deliver the values queried via the TMS9901, and they represent
    console functions which are under control of the TMS9901 (like the
    keyboard column selection.)
***************************************************************************/

static const char *const column[] = {
	"COL0", "COL1", "COL2", "COL3", "COL4", "COL5", "COL6", "COL7",
	"COL8", "COL9", "COL10", "COL11", "COL12", "COL13"
};

READ8_MEMBER( ti99_8::read_by_9901 )
{
	int answer=0;
	UINT8 joyst;
	switch (offset & 0x03)
	{
	case TMS9901_CB_INT7:
		// Read pins INT3*-INT7* of TI99's 9901.
		//
		// (bit 1: INT1 status)
		// (bit 2: INT2 status)
		// bits 3-4: unused?
		// bit 5: ???
		// bit 6-7: keyboard status bits 0 through 1

		// |K|K|-|-|-|I2|I1|C|
		if (m_keyboard_column >= m_firstjoy)
		{
			// TI-99/8's wiring differs from the TI-99/4A
			joyst = m_joyport->read_port();
			answer = (joyst & 0x01) | ((joyst & 0x10)>>3);
		}
		else
		{
			answer = ioport(column[m_keyboard_column])->read();
		}
		answer = (answer << 6) & 0xc0;

		break;

	case TMS9901_INT8_INT15:
		// Read pins INT8*-INT15* of TI99's 9901.
		//
		// bit 0-2: keyboard status bits 2 to 4
		// bit 3: tape input mirror
		// bit 4: unused
		// bit 5-7: weird, not emulated

		// |0|0|0|0|0|K|K|K|

		if (m_keyboard_column >= m_firstjoy)
		{
			joyst = m_joyport->read_port();
			answer = joyst << 1;
		}
		else
		{
			answer = ioport(column[m_keyboard_column])->read();
		}
		answer = (answer >> 2) & 0x07;
		break;

	case TMS9901_P0_P7:
		// Read pins P0-P7 of TI99's 9901. None here.
		break;

	case TMS9901_P8_P15:
		// Read pins P8-P15 of TI99's 9901. (TI-99/8)
		//
		// bit 26: high
		// bit 27: tape input
		answer = 4;
		if ((machine().device<cassette_image_device>(CASSETTE_TAG))->input() > 0)
			answer |= 8;
		break;
	}
	return answer;
}

/*
    WRITE key column select (P2-P4), TI-99/8
*/
void ti99_8::set_keyboard_column(int number, int data)
{
	if (data != 0)		m_keyboard_column |= 1 << number;
	else				m_keyboard_column &= ~(1 << number);

	if (m_keyboard_column >= m_firstjoy)
	{
		m_joyport->write_port(m_keyboard_column - m_firstjoy + 1);
	}
}

WRITE_LINE_MEMBER( ti99_8::keyC0 )
{
	set_keyboard_column(0, state);
}

WRITE_LINE_MEMBER( ti99_8::keyC1 )
{
	set_keyboard_column(1, state);
}

WRITE_LINE_MEMBER( ti99_8::keyC2 )
{
	set_keyboard_column(2, state);
}

WRITE_LINE_MEMBER( ti99_8::keyC3 )
{
	set_keyboard_column(3, state);
}

/*
    Set 99/4A compatibility mode (CRUS=1)
*/
WRITE_LINE_MEMBER( ti99_8::CRUS )
{
	m_mapper->CRUS_set(state==ASSERT_LINE);
}

/*
    Set mapper /PTGEN. This is negative logic; we use PTGE as the positive logic signal.
*/
WRITE_LINE_MEMBER( ti99_8::PTGEN )
{
	m_mapper->PTGE_set(state==CLEAR_LINE);
}

/*
    Control cassette tape unit motor (P6)
*/
WRITE_LINE_MEMBER( ti99_8::cassette_motor )
{
	cassette_image_device *img = machine().device<cassette_image_device>(CASSETTE_TAG);
	img->change_state(state==ASSERT_LINE? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
}

/*
    Audio gate (P8)
    Set to 1 before using tape: this enables the mixing of tape input sound
    with computer sound.
    We do not really need to emulate this as the tape recorder generates sound
    on its own.
*/
WRITE_LINE_MEMBER( ti99_8::audio_gate )
{
}

/*
    Tape output (P9)
    I think polarity is correct, but don't take my word for it.
*/
WRITE_LINE_MEMBER( ti99_8::cassette_output )
{
	machine().device<cassette_image_device>(CASSETTE_TAG)->output(state==ASSERT_LINE? +1 : -1);
}

WRITE8_MEMBER( ti99_8::tms9901_interrupt )
{
	m_cpu->set_input_line(INPUT_LINE_99XX_INT1, data);
}

const tms9901_interface tms9901_wiring_ti99_8 =
{
	TMS9901_INT1 | TMS9901_INT2 | TMS9901_INTC,

	// read handler
	DEVCB_DRIVER_MEMBER(ti99_8, read_by_9901),

	// write handlers
	{
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, keyC0),
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, keyC1),
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, keyC2),
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, keyC3),
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, CRUS),
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, PTGEN),
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, cassette_motor),
		DEVCB_NULL,
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, audio_gate),
		DEVCB_DRIVER_LINE_MEMBER(ti99_8, cassette_output),
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL
	},

	DEVCB_DRIVER_MEMBER(ti99_8, tms9901_interrupt)
};

/*****************************************************************************/

/*
    set the state of TMS9901's INT2 (called by the tms9928 core)
*/
WRITE_LINE_MEMBER( ti99_8::set_tms9901_INT2 )
{
	if (VERBOSE>6) LOG("ti99_8: VDP int 2 on tms9901, level=%02x\n", state);
	m_tms9901->set_single_int(2, state);
}

/***********************************************************
    Links to external devices
***********************************************************/


WRITE_LINE_MEMBER( ti99_8::console_ready )
{
	if (VERBOSE>6) LOG("ti99_8: READY level=%02x\n", state);
	m_ready_line = state;

	m_cpu->set_ready((m_ready_line == ASSERT_LINE && m_ready_line1 == ASSERT_LINE)? ASSERT_LINE : CLEAR_LINE);
}

/*
    The RESET line leading to a reset of the CPU.
*/
WRITE_LINE_MEMBER( ti99_8::console_reset )
{
	if (machine().phase() != MACHINE_PHASE_INIT)
	{
		m_cpu->set_input_line(INPUT_LINE_99XX_RESET, state);
		m_video->reset_vdp(state);
	}
}

/*
    Memory access over the mapper also operates
    the READY line, and the mapper raises READY depending on the clock pulse.
    So we must make sure this does not interfere.
*/
WRITE_LINE_MEMBER( ti99_8::console_ready_mapper )
{
	if (VERBOSE>6) LOG("ti99_8: READY level (mapper) = %02x\n", state);
	m_ready_line1 = state;
	m_cpu->set_ready((m_ready_line == ASSERT_LINE && m_ready_line1 == ASSERT_LINE)? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( ti99_8::extint )
{
	if (VERBOSE>6) LOG("ti99_8: EXTINT level = %02x\n", state);
	if (m_tms9901 != NULL)
		m_tms9901->set_single_int(1, state);
}

WRITE_LINE_MEMBER( ti99_8::notconnected )
{
	if (VERBOSE>6) LOG("ti99_8: Setting a not connected line ... ignored\n");
}

static TMS9928A_INTERFACE(ti99_8_tms9118a_interface)
{
	SCREEN_TAG,
	0x4000,
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, set_tms9901_INT2)
};

WRITE8_MEMBER( ti99_8::external_operation )
{
	static const char* extop[8] = { "inv1", "inv2", "IDLE", "RSET", "inv3", "CKON", "CKOF", "LREX" };
	if (VERBOSE>1) LOG("External operation %s not implemented on TI-99 board\n", extop[offset]);
}

/*
    Clock line from the CPU. Used to control wait state generation.
*/
WRITE_LINE_MEMBER( ti99_8::clock_out )
{
	m_mapper->clock_in(state);
}

/*****************************************************************************/

/*
    MP9537 mask: This variant of the TMS9995 does not contain on-chip RAM
    Also, the overflow interrupt is disabled; in the available documentation
    this feature is said to be disabled and is announced for a later version.
*/
static TMS9995_CONFIG( ti99_8_processor_config )
{
	DEVCB_DRIVER_MEMBER(ti99_8, external_operation),
	DEVCB_NULL,		// Instruction acquisition
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, clock_out),
	DEVCB_NULL,		// wait
	DEVCB_NULL,		// HOLDA
	NO_INTERNAL_RAM,
	NO_OVERFLOW_INT
};

static TI_SOUND_CONFIG( sound_conf )
{
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready)	// READY
};

/*
    Format:
    Name, mode, stop, mask, select, write, read8z function, write8 function

    Multiple devices may have the same select pattern; as in the real hardware,
    care must be taken that only one device actually responds. In the case of
    GROMs, each chip has an internal address counter and an ID, and the chip
    only responds when the ID and the most significant 3 bits match.

    NATIVE <-> CRUS=0
    TI99EM <-> CRUS=1

    PATGEN <-> PTGEN=1

    CONT: Mapper continues iterating through devices
    STOP: Mapper stops iterating when found

    Access to the mapper registers is done directly in the mapper, not via
    this list.

    NOTE: The available system software contradicts the specification
    concerning the Pascal ports. While the specs define f840 as "Text-to-speech
    library" and f850 and f860 as two libraries for Pascal, the operating
    system defines f840 as Pascal lib 0, f880 as Pascal lib 1, and f8c0 as
    Pascal lib 2.

    TODO: This should (must) be improved in terms of performance. Every single
    memory access goes through the mapper. Either we use an ordered search list,
    or we order the entries according to their frequency.
    (I did this right now, putting the Pascal GROMs at the end.)
    We should think about a set entry where devices with the same address
    are collected as one single entry (think about the Pascal lib with 24 GROMs,
    every eight of them on the same address).
*/

#define PASCAL_GROM_LIB(_tag, _addr) \
	{ _tag "0",     PATGEN, CONT, _addr, 0xfff1, 0x0000    },	\
	{ _tag "1",     PATGEN, CONT, _addr, 0xfff1, 0x0000    },	\
	{ _tag "2",     PATGEN, CONT, _addr, 0xfff1, 0x0000    },	\
	{ _tag "3",     PATGEN, CONT, _addr, 0xfff1, 0x0000    },	\
	{ _tag "4",     PATGEN, CONT, _addr, 0xfff1, 0x0000    },	\
	{ _tag "5",     PATGEN, CONT, _addr, 0xfff1, 0x0000    },	\
	{ _tag "6",     PATGEN, CONT, _addr, 0xfff1, 0x0000    },	\
	{ _tag "7",     PATGEN, CONT, _addr, 0xfff1, 0x0000    }


static const mapper8_list_entry mapper_devices[] =
{
	// TI-99/4A mode (CRUS=1)
	// GROMs: According to the spec, the 99/8 supports up to 4 GROM libraries
	// (99/4A supports 256 libraries)
	// at 9800, 9804, 9808, 980c. Address counter access is at 9802,6,a,e. Write access +0400.
	{ ROM0NAME,			TI99EM, STOP, 0x0000, 0xe000, 0x0000	},	// 0000-1fff

	{ TISOUND_TAG,  	TI99EM, STOP, 0x8400, 0xfff0, 0x0000	},	// 8400-840f
	{ VIDEO_SYSTEM_TAG,	TI99EM, STOP, 0x8800, 0xfffd, 0x0400	},	// 8800,8802 / 8c00,8c02
	{ SPEECH_TAG,		TI99EM, STOP, 0x9000, 0xfff0, 0x0400	},	// 9000-900f / 9400-940f
	{ SRAMNAME,			TI99EM, STOP, 0x8000, 0xf800, 0x0000	},	// 8000-87ff; must follow the sound generator
	{ MAPPER_TAG,		TI99EM, STOP, 0x8810, 0xfff0, 0x0000	},

	{ GROM0_TAG,    	TI99EM, CONT, 0x9800, 0xfff1, 0x0400	},	// 9800,2,4,...e/9c00,2,4,...e
	{ GROM1_TAG,    	TI99EM, CONT, 0x9800, 0xfff1, 0x0400	},	// dto.
	{ GROM2_TAG,    	TI99EM, CONT, 0x9800, 0xfff1, 0x0400	},	// dto. (GROMs are connected in parallel,
	{ GROMPORT_TAG, 	TI99EM, CONT, 0x9800, 0xfff1, 0x0400	},	// dto.  use internal address counter and id)

	// TI-99/8 mode
	{ SRAMNAME,			NATIVE, STOP, 0xf000, 0xf800, 0x0000	},	// f000-f7ff
	{ TISOUND_TAG,  	NATIVE, STOP, 0xf800, 0xfff0, 0x0000	},	// f800-f80f
	{ VIDEO_SYSTEM_TAG,	NATIVE, STOP, 0xf810, 0xfffd, 0x0000	},	// f810,2 (unlike 99/4A, no different read/write ports)
	{ SPEECH_TAG,		NATIVE, STOP, 0xf820, 0xfff0, 0x0000	},	// f820-f82f
	{ MAPPER_TAG,		NATIVE, STOP, 0xf870, 0xfff0, 0x0000	},

	{ GROM0_TAG,	    NATIVE, CONT, 0xf830, 0xfff1, 0x0000	},	// f830-f83e (4 banks), no different read/write ports
	{ GROM1_TAG,		NATIVE, CONT, 0xf830, 0xfff1, 0x0000	},
	{ GROM2_TAG,		NATIVE, CONT, 0xf830, 0xfff1, 0x0000	},
	{ GROMPORT_TAG, 	NATIVE, CONT, 0xf830, 0xfff1, 0x0000	},

	PASCAL_GROM_LIB("pascal0_grom", 0xf840),
	PASCAL_GROM_LIB("pascal1_grom", 0xf880),		// lib1 and 2 are zeroed. We don't have good dumps for them yet.
	PASCAL_GROM_LIB("pascal2_grom", 0xf8c0),		// Anyway, we keep them in order to check whether/when they are accessed.

	// Physical (need to pack this in here as well to keep config simple)
	// but these lines will be put into a separate list
	{ DRAMNAME, 		PHYSIC, STOP, 0x000000, 0xff0000, 0x000000	},	// 000000-00ffff 64 KiB DRAM
	{ MAPPER_TAG,		PHYSIC, CONT, 0xff4000, 0xffe000, 0x000000	},	// ff4000-ff5fff Internal DSR
	{ GROMPORT_TAG,		PHYSIC, STOP, 0xff6000, 0xffe000, 0x000000	},	// ff6000-ff7fff Cartridge ROM space
	{ GROMPORT_TAG,		PHYSIC, STOP, 0xff8000, 0xffe000, 0x000000	},	// ff8000-ff9fff Cartridge ROM space
	{ ROM1NAME, 		PHYSIC, STOP, 0xffa000, 0xffe000, 0x000000	},	// ffa000-ffbfff ROM1
	{ ROM1ANAME,		PHYSIC, STOP, 0xffc000, 0xffe000, 0x000000	},	// ffc000-ffdfff ROM1
	{ INTSNAME, 		PHYSIC, STOP, 0xffe000, 0xfffff0, 0x000000	},	// ffe000-ffe00f Interrupt level sense
	{ PERIBOX_TAG,		PHYSIC, STOP, 0x000000, 0x000000, 0x000000	},	// Peripheral Expansion Box

	{ NULL, 0, 0, 0, 0, 0  }
};

static MAPPER8_CONFIG( mapper_conf )
{
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready_mapper),	// READY
	mapper_devices
};

static SPEECH8_CONFIG( speech_config )
{
	DEVCB_DRIVER_LINE_MEMBER(ti99_8, console_ready),	// READY
};

static JOYPORT_CONFIG( joyport8_60 )
{
	DEVCB_NULL,
	60
};

static JOYPORT_CONFIG( joyport8_50 )
{
	DEVCB_NULL,
	50
};

void ti99_8::machine_start()
{
	m_cpu = static_cast<tms9995_device*>(machine().device("maincpu"));
	m_tms9901 = static_cast<tms9901_device*>(machine().device(TMS9901_TAG));
	m_gromport = static_cast<gromport_device*>(machine().device(GROMPORT_TAG));
	m_peribox = static_cast<peribox_device*>(machine().device(PERIBOX_TAG));
	m_mapper = static_cast<ti998_mapper_device*>(machine().device(MAPPER_TAG));
	m_joyport = static_cast<joyport_device*>(machine().device(JOYPORT_TAG));
	m_video = static_cast<ti_video_device*>(machine().device(VIDEO_SYSTEM_TAG));

	m_peribox->senila(CLEAR_LINE);
	m_peribox->senilb(CLEAR_LINE);
	m_firstjoy = 14;
}

void ti99_8::machine_reset()
{

	m_cpu->set_hold(CLEAR_LINE);

	// Pulling down the line on RESET configures the CPU to insert one wait
	// state on external memory accesses

	// RN: enable automatic wait state generation
	// in January 83 99/8 schematics sheet 9: the delay logic
	// seems to keep READY low for one cycle when RESET* is
	// asserted, but the timings are completely wrong this way

	m_cpu->set_ready(CLEAR_LINE);

	// But we assert the line here so that the system starts running
	m_ready_line = m_ready_line1 = ASSERT_LINE;
}

static MACHINE_CONFIG_START( ti99_8_60hz, ti99_8 )
	/* basic machine hardware */
	/* TMS9995-MP9537 CPU @ 10.7 MHz */
	MCFG_TMS9995_ADD("maincpu", TMS9995, 10738635, memmap, crumap, ti99_8_processor_config)


	/* Video hardware */
	MCFG_TI998_ADD_NTSC(VIDEO_SYSTEM_TAG, TMS9118, ti99_8_tms9118a_interface)

	/* Main board */
	MCFG_TMS9901_ADD( TMS9901_TAG, tms9901_wiring_ti99_8, 2684658.75 )
	MCFG_MAPPER8_ADD( MAPPER_TAG, mapper_conf )
	MCFG_TI99_GROMPORT_ADD( GROMPORT_TAG, console_cartslot )

	/* Peripheral expansion box */
	MCFG_PERIBOX_ADD( PERIBOX_TAG, peribox_conf )

	/* Sound hardware */
	MCFG_TI_SOUND_76496_ADD( TISOUND_TAG, sound_conf )

	/* Cassette drives */
	MCFG_SPEAKER_STANDARD_MONO("cass_out")
	MCFG_CASSETTE_ADD( CASSETTE_TAG, default_cassette_interface )
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "cass_out", 0.25)

	/* Console GROMs */
	MCFG_GROM_ADD( GROM0_TAG, grom0_config )
	MCFG_GROM_ADD( GROM1_TAG, grom1_config )
	MCFG_GROM_ADD( GROM2_TAG, grom2_config )

	/* Pascal GROM libraries. */
	MCFG_GROM_LIBRARY_ADD(pascal0_grom, pascal0)
	MCFG_GROM_LIBRARY_ADD(pascal1_grom, pascal1)
	MCFG_GROM_LIBRARY_ADD(pascal2_grom, pascal2)

	/* Devices */
	MCFG_TISPEECH8_ADD(SPEECH_TAG, speech_config)

	// Joystick port
	MCFG_TI_JOYPORT4A_ADD( JOYPORT_TAG, joyport8_60 )
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ti99_8_50hz, ti99_8 )
	/* basic machine hardware */
	/* TMS9995-MP9537 CPU @ 10.7 MHz */
	MCFG_TMS9995_ADD("maincpu", TMS9995, 10738635, memmap, crumap, ti99_8_processor_config)

	/* Video hardware */
	MCFG_TI998_ADD_PAL(VIDEO_SYSTEM_TAG, TMS9129, ti99_8_tms9118a_interface)

	/* Main board */
	MCFG_TMS9901_ADD( TMS9901_TAG, tms9901_wiring_ti99_8, 2684658.75 )
	MCFG_MAPPER8_ADD( MAPPER_TAG, mapper_conf )
	MCFG_TI99_GROMPORT_ADD( GROMPORT_TAG, console_cartslot )

	/* Peripheral expansion box */
	MCFG_PERIBOX_ADD( PERIBOX_TAG, peribox_conf )

	/* Sound hardware */
	MCFG_TI_SOUND_76496_ADD( TISOUND_TAG, sound_conf )

	/* Cassette drives */
	MCFG_SPEAKER_STANDARD_MONO("cass_out")
	MCFG_CASSETTE_ADD( CASSETTE_TAG, default_cassette_interface )
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "cass_out", 0.25)

	/* Console GROMs */
	MCFG_GROM_ADD( GROM0_TAG, grom0_config )
	MCFG_GROM_ADD( GROM1_TAG, grom1_config )
	MCFG_GROM_ADD( GROM2_TAG, grom2_config )

	/* Pascal GROMs libraries. */
	MCFG_GROM_LIBRARY_ADD(pascal0_grom, pascal0)
	MCFG_GROM_LIBRARY_ADD(pascal1_grom, pascal1)
	MCFG_GROM_LIBRARY_ADD(pascal2_grom, pascal2)

	/* Devices */
	MCFG_TISPEECH8_ADD(SPEECH_TAG, speech_config)

	// Joystick port
	MCFG_TI_JOYPORT4A_ADD( JOYPORT_TAG, joyport8_50 )
MACHINE_CONFIG_END

/*
    ROM loading
*/
ROM_START(ti99_8)
	/*CPU memory space*/
	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("998rom.bin", 0x0000, 0x8000, CRC(b7a06ffd) SHA1(17dc8529fa808172fc47089982efb0bf0548c80c))		/* system ROMs */

	/*GROM memory space*/
	ROM_REGION(0x10000, region_grom, 0)
	ROM_LOAD("998grom.bin", 0x0000, 0x6000, CRC(c63806bc) SHA1(cbfa8b04b4aefbbd9a713c54267ad4dd179c13a3))	/* system GROMs */

	/* Pascal GROMs. Sadly, P-System fails to start. */
	ROM_REGION(0x10000, pascal0_region, 0)
	ROM_LOAD_OPTIONAL("998pascal.bin", 0x0000, 0x10000, CRC(1389589e) SHA1(42942b99ed355a2c091cc480b15f5329156e6b03))

	// Still need good dumps; so far, stay with 0
	ROM_REGION(0x10000, pascal12_region, 0)
	ROM_FILL(0x0000, 0x10000, 0x00)

	/* Built-in RAM */
	ROM_REGION(SRAM_SIZE, SRAM_TAG, 0)
	ROM_FILL(0x0000, SRAM_SIZE, 0x00)

	ROM_REGION(DRAM_SIZE, DRAM_TAG, 0)
	ROM_FILL(0x0000, DRAM_SIZE, 0x00)
ROM_END

#define rom_ti99_8e rom_ti99_8

/*      YEAR    NAME        PARENT  COMPAT  MACHINE     INPUT   INIT      COMPANY                 FULLNAME */
COMP(	1983,	ti99_8,		0,		0,	ti99_8_60hz,ti99_8, driver_device,	0,		"Texas Instruments",	"TI-99/8 Computer (US)" , 0)
COMP(	1983,	ti99_8e,	ti99_8,	0,	ti99_8_50hz,ti99_8, driver_device,	0,		"Texas Instruments",	"TI-99/8 Computer (Europe)" , 0 )
