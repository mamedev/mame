// license:GPL-2.0+
// copyright-holders: Michael Zapf
/*
    TI-99/2 driver
    --------------

    Drivers: ti99_224 - 24 KiB version
             ti99_232 - 32 KiB version

    Codenamed "Ground Squirrel", the TI-99/2 was designed to be an extremely
    low-cost, downstripped version of the TI-99/4A, competing with systems
    like the ZX81 or the Times/Sinclair 1000. The targeted price was below $100.

    The 99/2 was equipped with a TMS9995, the same CPU as used in the envisioned
    flag ship 99/8 and later in the Geneve 9640. In the 99/2 it is clocked
    at 5.35 MHz. Also, the CPU has on-chip RAM, unlike the version in the 99/8.

    At many places, the tight price constraint shows up.
    - 2 or 4 KiB of RAM
    - Video memory is part of the CPU RAM
    - Video controller is black/white only and has a fixed character set, no sprites
    - No sound
    - No GROMs (as the significant TI technology circuit)
    - No P-Box connection

    Main board
    ----------
    1 CPU @ 5.35 MHz
    2 RAM circuits
    3 or 4 EPROMs
    1 TAL-004 custom gate array as the video controller
    1 TAL-004 custom gate array as the I/O controller
    and six selector or latch circuits

    Connectors
    ----------
    - Built-in RF modulator, switch allows for setting the channel to VHF 3 or 4
    - Cassette connector (compatible to 99/4A), one unit only
    - Hexbus connector
    - System expansion slot for cartridges (ROM or RAM), 60 pins, on the back

    Keyboard: 48 keys, no Alpha Lock, two separate Shift keys, additional break key.

    Description
    -----------
    Prototypes were developed in 1983. Despite a late marketing campaign,
    including well-known faces, no devices were ever sold. A few of them
    survived in the hands of the engineers, and were later sold to private
    users.

    The Ground Squirrel underwent several design changes. In the initial
    design, only 2 KiB RAM were planned, and the included ROM was 24 KiB.
    Later, the 2 KiB were obviously expanded to 4 KiB, while the ROM remained
    the same in size. This can be proved here, since the console crashes with
    less than 4 KiB by an unmapped memory access.

    The next version showed an additional 8 KiB of ROM. Possibly in order
    to avoid taking away too much address space, two EPROMs are mapped to the
    same address space block, selectable by a CRU bit. ROM may be added as
    cartridges to be plugged into the expansion slot, and the same is true
    for RAM. Actually, since the complete bus is available on that connector,
    almost any kind of peripheral device could be added. Too bad, none were
    developed.

    However, the Hexbus seemed to have matured in the meantime, which became
    the standard peripheral bus system for the TI-99/8, and for smaller
    systems like the CC-40 and the TI-74. The TI-99/2 also offers a Hexbus
    interface so that any kind of Hexbus device can be connected, like, for
    example, the HX5102 floppy drive, a Wafertape drive, or the RS232 serial
    interface.

    The address space layout looks like this:

    0000 - 3FFF:    ROM, unbanked
    4000 - 5FFF:    ROM, banked for 32K version
    6000 - DFFF:    ROM or RAM; ROM growing upwards, RAM growing downwards
    E000 - EFFF:    4K RAM
      EC00 - EEFF:  Area used by video controller (24 rows, 32 columns)
      EF00       :  Control byte for colors (black/white) for backdrop/border
    EF01 - EFFF:    RAM
    F000 - F0FB:    CPU-internal RAM
    F0FC - FFF9:    empty
    FFFA - FFFF:    CPU-internal decrementer and NMI vector

    RAM expansions may be offered via the cartridge port, but they must be
    contiguous with the built-in RAM, which means that it must grow
    downwards from E000. The space from F0FC up to FFF9 may also be covered
    by expansion RAM.

    From the other side, ROM or other memory-mapped devices may occupy
    address space, growing up from 6000.

    One peculiar detail is the memory-mapped video RAM. The video controller
    shares an area of RAM with the CPU. To avoid congestion, the video
    controller must HOLD the CPU while it accesses the video RAM area.

    This decreases processing speed of the CPU, of course. For that reason,
    a special character may be placed in every row after which the row will
    be filled with blank pixels, and the CPU will be released early. This
    means that with a screen full of characters, the processing speed is
    definitely slower than with a clear screen.

    To be able to temporarily get full CPU power, there is a pin VIDENA at
    the video controller which causes the screen to be blank when asserted,
    and to be restored as before when cleared. This is used during cassette
    transfer.

    Technical details
    -----------------
    - HOLD is asserted in every scanline when characters are drawn that are
      not the "Blank End of line" (BEOL). Once encountered, the remaining
      scanline remains blank.
    - When a frame has been completed, the INT4 interrupt of the 9995 is
      triggered as a vblank interrupt.
    - All CRU operations are handled by the second gate array. Unfortunately,
      there is no known documentation about this circuit.
    - The two banks of the last 16 KiB of ROM are selected with the same
      line that is used for selecting keyboard row 0. This should mean that
      you cannot read the keyboard from the second ROM bank.

    I/O map (CRU map)
    -----------------
    0000 - 1DFE: unmapped
    1E00 - 1EFE: TMS9995-internal CRU addresses
    1F00 - 1FD8: unmapped
    1FDA:        TMS9995 MID flag
    1FDC - 1FFF: unmapped
    2000 - DFFE: unmapped
    E000 - E00E: Read: Keyboard column input
    E000 - E00A: Write: Keyboard row selection
    E00C:        Write: unmapped
    E00E:        Write: Video enable (VIDENA)
    E010 - E7FE: Mirrors of the above
    E800 - E80C: Hexbus
       E800 - E806: Data lines
       E808: HSK line
       E80A: BAV line
       E80C: Inhibit (Write only)
    E80E: Cassette

    ROM dumps
    ---------
    Hard to believe, but we have access to two people with one 24K and
    one 32K version, and we were able to dumps the ROMs correctly.

    The ROMs contain a stripped-down version of TI BASIC, but without
    the specific graphics subprograms. Programs written on the 99/2 should
    run on the 99/4A, but the opposite is not true.

    TODO
    ----
    * Fix cassette
    * Add Hexbus

    Original implementation: Raphael Nabet; December 1999, 2000

    Michael Zapf, May 2018

    References :
    [1] TI-99/2 main logic board schematics, 83/03/28-30
    [2] BYTE magazine, June 1983, pp. 128-134
*/

#include "emu.h"
#include "machine/tms9901.h"
#include "cpu/tms9900/tms9995.h"
#include "bus/ti99/internal/992board.h"
#include "machine/ram.h"
#include "imagedev/cassette.h"

#define TI_VDC_TAG         "vdc"
#define TI_SCREEN_TAG      "screen"
#define TI992_ROM          "rom_region"
#define TI992_RAM_TAG      "ram_region"

#define LOG_WARN           (1U<<1)   // Warnings
#define LOG_HEXBUS         (1U<<2)   // Hexbus operation
#define LOG_KEYBOARD       (1U<<3)   // Keyboard operation
#define LOG_SIGNALS        (1U<<4)   // Signals like HOLD/HOLDA
#define LOG_CRU            (1U<<5)   // CRU activities
#define LOG_BANK           (1U<<6)   // Change ROM banks

// Minimum log should be config and warnings
#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"

class ti99_2_state : public driver_device
{
public:
	ti99_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoctrl(*this, "vdc"),
		m_cassette(*this, "cassette"),
		m_ram(*this, TI992_RAM_TAG),
		m_have_banked_ROM(true),
		m_otherbank(false),
		m_keyRow(0),
		m_rom(nullptr),
		m_ram_start(0xf000),
		m_first_ram_page(0)
		{ }


	void machine_start_ti99_224() ATTR_COLD;
	void machine_start_ti99_232() ATTR_COLD;
	void machine_reset_ti99_2();

	DECLARE_WRITE8_MEMBER(intflag_write);

	DECLARE_READ8_MEMBER(read_e00x);
	DECLARE_READ8_MEMBER(read_e80x);
	DECLARE_WRITE8_MEMBER(write_e00x);
	DECLARE_WRITE8_MEMBER(write_e80x);

	DECLARE_READ8_MEMBER(mem_read);
	DECLARE_WRITE8_MEMBER(mem_write);

	DECLARE_WRITE_LINE_MEMBER(hold);
	DECLARE_WRITE_LINE_MEMBER(holda);
	DECLARE_WRITE_LINE_MEMBER(interrupt);
	DECLARE_WRITE_LINE_MEMBER(cassette_output);

	void crumap(address_map &map);
	void memmap(address_map &map);

	void ti99_2(machine_config &config);
	void ti99_224(machine_config &config);
	void ti99_232(machine_config &config);

private:
	required_device<tms9995_device> m_maincpu;
	required_device<bus::ti99::internal::video992_device> m_videoctrl;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	bool m_have_banked_ROM;
	bool m_otherbank;
	int m_keyRow;

	uint8_t*   m_rom;
	int m_ram_start;
	int m_first_ram_page;
};

void ti99_2_state::machine_start_ti99_224()
{
	m_rom = memregion(TI992_ROM)->base();
	m_ram_start = 0xf000 - m_ram->default_size();
	m_first_ram_page = m_ram_start >> 12;
	m_have_banked_ROM = false;
}

void ti99_2_state::machine_start_ti99_232()
{
	m_rom = memregion(TI992_ROM)->base();
	m_ram_start = 0xf000 - m_ram->default_size();
	m_first_ram_page = m_ram_start >> 12;
	m_have_banked_ROM = true;
}

void ti99_2_state::machine_reset_ti99_2()
{
	m_otherbank = false;

	// Configure CPU to insert 1 wait state for each external memory access
	// by lowering the READY line on reset
	// TODO: Check with specs
	m_maincpu->ready_line(CLEAR_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

/*
    Memory map - see description above
*/

void ti99_2_state::memmap(address_map &map)
{
	// 3 or 4 ROMs of 8 KiB. The 32K version has two ROMs mapped into 4000-5fff
	// Part of the ROM is accessed by the video controller for the
	// character definitions (1c00-1fff)
	map(0x0000, 0xffff).rw(this, FUNC(ti99_2_state::mem_read), FUNC(ti99_2_state::mem_write));
}

/*
    CRU map - see description above
    Note that the CRU address space has only even numbers, and the
    read addresses in the emulation gather 8 bits in one go, so the address
    is the bit number times 16.
*/
void ti99_2_state::crumap(address_map &map)
{
	// Mirror of CPU-internal flags (1ee0-1efe). Don't read. Write is OK.
	map(0x01ee, 0x01ef).nopr();
	map(0x0f70, 0x0F7F).w(this, FUNC(ti99_2_state::intflag_write));

	// CRU E000-E7fE: Keyboard
	//  Read: 0000 1110 0*** **** (mirror 007f)
	// Write: 0111 00** **** *XXX (mirror 03f8)
	map(0x0e00, 0x0e00).mirror(0x007f).r(this, FUNC(ti99_2_state::read_e00x));
	map(0x7000, 0x7007).mirror(0x03f8).w(this, FUNC(ti99_2_state::write_e00x));

	// CRU E800-EFFE: Hexbus and other functions
	//  Read: 0000 1110 1*** **** (mirror 007f)
	// Write: 0111 01** **** *XXX (mirror 03f8)
	map(0x0e80, 0x0e80).mirror(0x007f).r(this, FUNC(ti99_2_state::read_e80x));
	map(0x7400, 0x7407).mirror(0x03f8).w(this, FUNC(ti99_2_state::write_e80x));
}


/*
    Select the current keyboard row. Also, bit 0 is used to switch the
    ROM bank. Suppose that means we won't be able to read the keyboard
    when processing that ROM area.
*/
WRITE8_MEMBER(ti99_2_state::write_e00x)
{
	// logerror("offset=%d, data=%d\n", offset, data);
	switch (offset)
	{
	case 0:
		if (m_have_banked_ROM)
		{
			LOGMASKED(LOG_BANK, "set bank = %d\n", data);
			m_otherbank = (data==1);
		}
		// no break
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if (data == 0) m_keyRow = offset;
		break;
	case 6:
		LOGMASKED(LOG_WARN, "Unmapped CRU write to address e00c\n");
		break;
	case 7:
		LOGMASKED(LOG_CRU, "VIDENA = %d\n", data);
		m_videoctrl->videna(data);
		break;
	}
}

WRITE8_MEMBER(ti99_2_state::write_e80x)
{
	switch (offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		LOGMASKED(LOG_CRU, "Hexbus I/O (%d) = %d\n", offset, data);
		break;
	case 4:
		LOGMASKED(LOG_CRU, "Hexbus HSK = %d\n", data);
		break;
	case 5:
		LOGMASKED(LOG_CRU, "Hexbus BAV = %d\n", data);
		break;
	case 6:
		LOGMASKED(LOG_CRU, "Hexbus inhibit = %d\n", data);
		break;
	case 7:
		LOGMASKED(LOG_CRU, "Cassette output = %d\n", data);
		cassette_output((data==1)? ASSERT_LINE : CLEAR_LINE);
		break;
	}
}

/* read keys in the current row */
READ8_MEMBER(ti99_2_state::read_e00x)
{
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7" };
	return ioport(keynames[m_keyRow])->read();
}

READ8_MEMBER(ti99_2_state::read_e80x)
{
	uint8_t value = 0;
	if (m_cassette->input() > 0)
	{
		value |= 0x80;
	}
	return value;
}

/*
    Tape output. See also ti99_4x.cpp where this is taken from.
*/
WRITE_LINE_MEMBER( ti99_2_state::cassette_output )
{
	m_cassette->output(state==ASSERT_LINE? +1 : -1);
}

/*
    These CRU addresses are actually inside the 9995 CPU, but they are
    propagated to the outside world, so we can watch the changes.
*/
WRITE8_MEMBER(ti99_2_state::intflag_write)
{
	int addr = 0x1ee0 | (offset<<1);
	switch (addr)
	{
	case 0x1ee0:
		LOGMASKED(LOG_CRU, "Setting 9995 decrementer to %s mode\n", (data==1)? "event" : "timer");
		break;
	case 0x1ee2:
		LOGMASKED(LOG_CRU, "Setting 9995 decrementer to %s\n", (data==1)? "enabled" : "disabled");
		break;
	case 0x1ee4:
		if (data==0) LOGMASKED(LOG_CRU, "Clear INT1 latch\n");
		break;
	case 0x1ee6:
		if (data==0) LOGMASKED(LOG_CRU, "Clear INT3 latch\n");
		break;
	case 0x1ee8:
		if (data==0) LOGMASKED(LOG_CRU, "Clear INT4 latch\n");
		break;
	case 0x1eea:
		LOGMASKED(LOG_CRU, "Switch to bank %d\n", data);
		break;

	default:
		LOGMASKED(LOG_CRU, "Writing internal flag %04x = %d\n", addr, data);
		break;
	}
}

/*
    Called by CPU and video controller.
*/
READ8_MEMBER(ti99_2_state::mem_read)
{
	int page = offset >> 12;

	if (page>=0 && page<4)
	{
		// ROM, unbanked
		return m_rom[offset];
	}
	if (page>=4 && page<6)
	{
		// ROM, banked on 32K version
		if (m_otherbank) offset = (offset & 0x1fff) | 0x10000;
		return m_rom[offset];
	}

	if ((page >= m_first_ram_page) && (page < 15))
	{
		return m_ram->pointer()[offset - m_ram_start];
	}

	LOGMASKED(LOG_WARN, "Unmapped read access at %04x\n", offset);
	return 0;
}

WRITE8_MEMBER(ti99_2_state::mem_write)
{
	int page = offset >> 12;

	if (page>=0 && page<4)
	{
		// ROM, unbanked
		LOGMASKED(LOG_WARN, "Writing to ROM at %04x ignored (data=%02x)\n", offset, data);
		return;
	}

	if (page>=4 && page<6)
	{
		LOGMASKED(LOG_WARN, "Writing to ROM at %04x (bank %d) ignored (data=%02x)\n", offset, m_otherbank? 1:0, data);
		return;
	}

	if ((page >= m_first_ram_page) && (page < 15))
	{
		m_ram->pointer()[offset - m_ram_start] = data;
		return;
	}

	LOGMASKED(LOG_WARN, "Unmapped write access at %04x\n", offset);
}

/*
    Called by the VDC as a vblank interrupt
*/
WRITE_LINE_MEMBER(ti99_2_state::interrupt)
{
	LOGMASKED(LOG_SIGNALS, "Interrupt: %d\n", state);
	m_maincpu->set_input_line(INT_9995_INT4, state);
}

/*
    Called by the VDC to HOLD the CPU
*/
WRITE_LINE_MEMBER(ti99_2_state::hold)
{
	LOGMASKED(LOG_SIGNALS, "HOLD: %d\n", state);
	m_maincpu->hold_line(state);
}

/*
    Called by the CPU to ack the HOLD
*/
WRITE_LINE_MEMBER(ti99_2_state::holda)
{
	LOGMASKED(LOG_SIGNALS, "HOLDA: %d\n", state);
}

/*
    54-key keyboard
*/
static INPUT_PORTS_START(ti99_2)

	PORT_START("LINE0")    /* col 0 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 ! DEL") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @ INS") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $ CLEAR") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 % BEGIN") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^ PROC'D") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 & AID") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 * REDO") PORT_CODE(KEYCODE_8)

	PORT_START("LINE1")    /* col 1 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w W ~") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e E (UP)") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r R [") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t T ]") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i I ?") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 ( BACK") PORT_CODE(KEYCODE_9)

	PORT_START("LINE2")    /* col 2 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s S (LEFT)") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d D (RIGHT)") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f F {") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u U _") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o O '") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0)

	PORT_START("LINE3")    /* col 3 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z Z \\") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x X (DOWN)") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c C `") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g G }") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p P \"") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= + QUIT") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("LINE4")    /* col 4 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT/*KEYCODE_CAPSLOCK*/)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ -") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE5")    /* col 5 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(SPACE)") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FCTN") PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)

	PORT_START("LINE6")    /* col 6 */
		PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE7")    /* col 7 */
		PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

MACHINE_CONFIG_START(ti99_2_state::ti99_224)
	ti99_2(config);
	// Video hardware
	MCFG_DEVICE_ADD( TI_VDC_TAG, VIDEO99224, XTAL(10'738'635) )
	MCFG_VIDEO992_MEM_ACCESS_CB( READ8( *this, ti99_2_state, mem_read ) )
	MCFG_VIDEO992_HOLD_CB( WRITELINE( *this, ti99_2_state, hold ) )
	MCFG_VIDEO992_INT_CB( WRITELINE( *this, ti99_2_state, interrupt ) )
	MCFG_VIDEO992_SCREEN_ADD( TI_SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TI_VDC_TAG, bus::ti99::internal::video992_device, screen_update )

	set_machine_start_cb(config, driver_callback_delegate(&machine_start_ti99_224, this));
MACHINE_CONFIG_END

MACHINE_CONFIG_START(ti99_2_state::ti99_232)
	ti99_2(config);
	// Video hardware
	MCFG_DEVICE_ADD( TI_VDC_TAG, VIDEO99232, XTAL(10'738'635) )
	MCFG_VIDEO992_MEM_ACCESS_CB( READ8( *this, ti99_2_state, mem_read ) )
	MCFG_VIDEO992_HOLD_CB( WRITELINE( *this, ti99_2_state, hold ) )
	MCFG_VIDEO992_INT_CB( WRITELINE( *this, ti99_2_state, interrupt ) )
	MCFG_VIDEO992_SCREEN_ADD( TI_SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TI_VDC_TAG, bus::ti99::internal::video992_device, screen_update )

	set_machine_start_cb(config, driver_callback_delegate(&machine_start_ti99_232, this));

MACHINE_CONFIG_END

MACHINE_CONFIG_START(ti99_2_state::ti99_2)
	// TMS9995, standard variant
	// There is a divider by 2 for the clock rate
	MCFG_TMS99xx_ADD("maincpu", TMS9995, XTAL(10'738'635) / 2, memmap, crumap)
	MCFG_TMS9995_HOLDA_HANDLER( WRITELINE(*this, ti99_2_state, holda) )

	// RAM 4 KiB
	// Early documents indicate 2 KiB RAM, but this does not work with
	// either ROM version, so we have to assume that the 2 KiB were never
	// sufficient, not even in the initial design
	MCFG_RAM_ADD(TI992_RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4096")
	MCFG_RAM_DEFAULT_VALUE(0)

	set_machine_reset_cb(config, driver_callback_delegate(&machine_reset_ti99_2, this));

	// Cassette drives
	// There is no route from the cassette to some audio output, so we don't hear it.
	MCFG_CASSETTE_ADD( "cassette" )

MACHINE_CONFIG_END

/*
  ROM loading
*/
ROM_START(ti99_224)
	// The 24K version is an early design; the PCB does not have any socket names
	ROM_REGION(0x6000, TI992_ROM ,0)
	ROM_LOAD("rom0000.bin", 0x0000, 0x2000, CRC(c57436f1) SHA1(71d9048fed0317cfcc4cd966dcbc3bc163080cf9))
	ROM_LOAD("rom2000.bin", 0x2000, 0x2000, CRC(be22c6c4) SHA1(931931d61732bacdab1da227c01b8045ca860f0b))
	ROM_LOAD("rom4000.bin", 0x4000, 0x2000, CRC(926ca20e) SHA1(91624a16aa2c62c7ebc23128308709efdebddca3))
ROM_END

ROM_START(ti99_232)
	ROM_REGION(0x12000, TI992_ROM, 0)
	// The 32K version is a more elaborate design; the ROMs for addresses 0000-1fff
	// and the second bank for 4000-5fff are stacked on the socket U2
	ROM_LOAD("rom0000.u2a", 0x0000, 0x2000, CRC(01b94f06) SHA1(ef2e0c5f0492d7d024ebfe3fad29c2b57ea849e1))
	ROM_LOAD("rom2000.u12", 0x2000, 0x2000, CRC(0a32f80a) SHA1(32ed98481998be295e637eaa2117337cfa4a7984))
	ROM_LOAD("rom4000a.u3", 0x4000, 0x2000, CRC(10c11fab) SHA1(d43e0952538e66e2cedc307b71b65cb388cbe8e3))
	ROM_LOAD("rom4000b.u2b", 0x10000, 0x2000, CRC(34dd52ed) SHA1(e01892b1b110d7d592a7e7f1f39f9f46ea0818db))
ROM_END

//      YEAR    NAME      PARENT    COMPAT  MACHINE   INPUT   CLASS         INIT        COMPANY              FULLNAME                               FLAGS
COMP(   1983,   ti99_224, 0,        0,      ti99_224, ti99_2, ti99_2_state, empty_init, "Texas Instruments", "TI-99/2 BASIC Computer (24 KiB ROM)" , MACHINE_NO_SOUND_HW )
COMP(   1983,   ti99_232, ti99_224, 0,      ti99_232, ti99_2, ti99_2_state, empty_init, "Texas Instruments", "TI-99/2 BASIC Computer (32 KiB ROM)" , MACHINE_NO_SOUND_HW )
