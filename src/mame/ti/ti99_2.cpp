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
    at 10.7 MHz. Also, the CPU has on-chip RAM, unlike the version in the 99/8.

    At many places, the tight price constraint shows up.
    - 2 or 4 KiB of RAM
    - Video memory is part of the CPU RAM
    - Video controller is black/white only and has a fixed character set, no sprites
    - No sound
    - No GROMs (as the significant TI technology circuit)
    - No P-Box connection

    Main board
    ----------
    1 CPU @ 10.7 MHz (contrary to specs which state 5.35 MHz)
    2 RAM circuits   (4 KiB instead of 2 KiB in specs)
    3 or 4 EPROMs
    1 TAL-004 custom gate array as the video controller
    1 TAL-004 custom gate array as the I/O controller
    and six selector or latch circuits

    Connectors
    ----------
    - Built-in RF modulator, switch allows for setting the channel to VHF 3 or 4
    - Two jacks for cassette input and output, no motor control
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
    less than 4 KiB by an unmapped memory access. Also, the CPU is not clocked
    by 5.35 MHz as specified, but by the undivided 10.7 MHz; this was proved
    by running test programs on the real consoles.

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
    interface. The 24K version seems to have no proper Hexbus support for
    floppy drives; it always starts the cassette I/O instead.

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
    - During cassette transfer, the screen is blanked using the VIDENA line.
      This is expected and not a bug.
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
    Although these machines are extremely rare, we were lucky to get in
    contact with users of both console variants and got dumps from
    their machines.

    The ROMs contain a stripped-down version of TI BASIC, but without
    the specific graphics subprograms. Programs written on the 99/2 should
    run on the 99/4A, but the opposite is not true.

    Original implementation: Raphael Nabet; December 1999, 2000

    Michael Zapf, May 2018

    References :
    [1] TI-99/2 main logic board schematics, 83/03/28-30
    [2] BYTE magazine, June 1983, pp. 128-134
*/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "bus/ti99/internal/992board.h"
#include "machine/ram.h"
#include "imagedev/cassette.h"
#include "bus/hexbus/hexbus.h"

#define TI992_IO_TAG       "io"
#define TI992_RAM_TAG      "ram_region"
#define TI992_ROM          "rom_region"
#define TI992_SCREEN_TAG   "screen"

#define LOG_WARN           (1U << 1)   // Warnings
#define LOG_CRU            (1U << 2)   // CRU activities
#define LOG_SIGNALS        (1U << 3)   // Signals like HOLD/HOLDA

// Minimum log should be config and warnings
#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"


namespace {

class ti99_2_state : public driver_device
{
public:
	ti99_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoctrl(*this, TI992_VDC_TAG),
		m_io992(*this, TI992_IO_TAG),
		m_cassette(*this, TI992_CASSETTE),
		m_ram(*this, TI992_RAM_TAG),
		m_expport(*this, TI992_EXPPORT_TAG),
		m_otherbank(false),
		m_rom(*this, TI992_ROM),
		m_ram_start(0xf000),
		m_first_ram_page(0)
		{ }

	void ti99_2(machine_config &config);
	void ti99_224(machine_config &config);
	void ti99_232(machine_config &config);

	// Lifecycle
	void driver_start() override;
	void driver_reset() override;

private:
	void intflag_write(offs_t offset, uint8_t data);

	uint8_t mem_read(offs_t offset);
	void mem_write(offs_t offset, uint8_t data);

	void hold(int state);
	void holda(int state);
	void interrupt(int state);
	void cassette_output(int state);

	void rombank_set(int state);

	void crumap(address_map &map) ATTR_COLD;
	void memmap(address_map &map) ATTR_COLD;

	required_device<tms9995_device> m_maincpu;
	required_device<bus::ti99::internal::video992_device> m_videoctrl;
	required_device<bus::ti99::internal::io992_device>    m_io992;

	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	required_device<bus::ti99::internal::ti992_expport_device> m_expport;

	bool m_otherbank;

	required_region_ptr<uint8_t> m_rom;
	int m_ram_start;
	int m_first_ram_page;
};

void ti99_2_state::driver_start()
{
	m_ram_start = 0xf000 - m_ram->default_size();
	m_first_ram_page = m_ram_start >> 12;
}

void ti99_2_state::driver_reset()
{
	m_otherbank = false;

	// Configure CPU to insert 1 wait state for each external memory access
	// by lowering the READY line on reset
	// This has been verified with the real machine, running test loops.
	m_maincpu->ready_line(CLEAR_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

void ti99_2_state::rombank_set(int state)
{
	m_otherbank = (state==ASSERT_LINE);
}

/*
    Memory map - see description above
*/

void ti99_2_state::memmap(address_map &map)
{
	// 3 or 4 ROMs of 8 KiB. The 32K version has two ROMs mapped into 4000-5fff
	// Part of the ROM is accessed by the video controller for the
	// character definitions (1c00-1fff)
	map(0x0000, 0xffff).rw(FUNC(ti99_2_state::mem_read), FUNC(ti99_2_state::mem_write));
}

/*
    CRU map - see description above
*/
void ti99_2_state::crumap(address_map &map)
{
	map(0x0000, 0xffff).rw(m_io992, FUNC(bus::ti99::internal::io992_device::cruread), FUNC(bus::ti99::internal::io992_device::cruwrite));

	// Mirror of CPU-internal flags (1ee0-1efe). Don't read. Write is OK.
	map(0x1ee0, 0x1eff).nopr().w(FUNC(ti99_2_state::intflag_write));
}

/*
    These CRU addresses are actually inside the 9995 CPU, but they are
    propagated to the outside world, so we can watch the changes.
*/
void ti99_2_state::intflag_write(offs_t offset, uint8_t data)
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
uint8_t ti99_2_state::mem_read(offs_t offset)
{
	uint8_t value = 0;
	if (m_maincpu->is_onchip(offset)) return m_maincpu->debug_read_onchip_memory(offset&0xff);

	int page = offset >> 12;

	if (page>=0 && page<4)
	{
		// ROM, unbanked
		value = m_rom[offset];
	}
	if (page>=4 && page<6)
	{
		// ROM, banked on 32K version
		if (m_otherbank) offset = (offset & 0x1fff) | 0x10000;
		value = m_rom[offset];
	}

	if ((page >= m_first_ram_page) && (page < 15))
	{
		value = m_ram->pointer()[offset - m_ram_start];
	}

	m_expport->readz(offset, &value);

	return value;
}

void ti99_2_state::mem_write(offs_t offset, uint8_t data)
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

	m_expport->write(offset, data);
}

/*
    Called by the VDC as a vblank interrupt
*/
void ti99_2_state::interrupt(int state)
{
	LOGMASKED(LOG_SIGNALS, "Interrupt: %d\n", state);
	m_maincpu->set_input_line(INT_9995_INT4, state);
}

/*
    Called by the VDC to HOLD the CPU
*/
void ti99_2_state::hold(int state)
{
	LOGMASKED(LOG_SIGNALS, "HOLD: %d\n", state);
	m_maincpu->hold_line(state);
}

/*
    Called by the CPU to ack the HOLD
*/
void ti99_2_state::holda(int state)
{
	LOGMASKED(LOG_SIGNALS, "HOLDA: %d\n", state);
}

void ti99_2_state::ti99_224(machine_config& config)
{
	ti99_2(config);
	// Video hardware
	VIDEO99224(config, m_videoctrl, XTAL(10'738'635));
	m_videoctrl->readmem_cb().set(FUNC(ti99_2_state::mem_read));
	m_videoctrl->hold_cb().set(FUNC(ti99_2_state::hold));
	m_videoctrl->int_cb().set(FUNC(ti99_2_state::interrupt));

	using namespace bus::ti99::internal;
	screen_device& screen(SCREEN(config, TI992_SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(10'738'635) / 2,
			video992_device::TOTAL_HORZ,
			video992_device::HORZ_DISPLAY_START-12,
			video992_device::HORZ_DISPLAY_START + 256 + 12,
			video992_device::TOTAL_VERT_NTSC,
			video992_device::VERT_DISPLAY_START_NTSC - 12,
			video992_device::VERT_DISPLAY_START_NTSC + 192 + 12 );
	screen.set_screen_update(TI992_VDC_TAG, FUNC(video992_device::screen_update));

	// I/O interface circuit. No banking callback.
	IO99224(config, m_io992, 0);
}

void ti99_2_state::ti99_232(machine_config& config)
{
	ti99_2(config);
	// Video hardware
	VIDEO99232(config, m_videoctrl, XTAL(10'738'635));
	m_videoctrl->readmem_cb().set(FUNC(ti99_2_state::mem_read));
	m_videoctrl->hold_cb().set(FUNC(ti99_2_state::hold));
	m_videoctrl->int_cb().set(FUNC(ti99_2_state::interrupt));

	using namespace bus::ti99::internal;
	screen_device& screen(SCREEN(config, TI992_SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(10'738'635) / 2,
			video992_device::TOTAL_HORZ,
			video992_device::HORZ_DISPLAY_START-12,
			video992_device::HORZ_DISPLAY_START + 256 + 12,
			video992_device::TOTAL_VERT_NTSC,
			video992_device::VERT_DISPLAY_START_NTSC - 12,
			video992_device::VERT_DISPLAY_START_NTSC + 192 + 12 );
	screen.set_screen_update(TI992_VDC_TAG, FUNC(video992_device::screen_update));

	// I/O interface circuit
	IO99232(config, m_io992, 0).rombank_cb().set(FUNC(ti99_2_state::rombank_set));
}

void ti99_2_state::ti99_2(machine_config& config)
{
	// TMS9995, standard variant
	// Documents state that there is a divider by 2 for the clock rate
	// Experiments with real consoles proved them wrong.
	TMS9995(config, m_maincpu, XTAL(10'738'635));
	m_maincpu->set_addrmap(AS_PROGRAM, &ti99_2_state::memmap);
	m_maincpu->set_addrmap(AS_IO, &ti99_2_state::crumap);
	m_maincpu->holda_cb().set(FUNC(ti99_2_state::holda));

	// RAM 4 KiB
	// Early documents indicate 2 KiB RAM, but this does not work with
	// either ROM version, so we have to assume that the 2 KiB were never
	// sufficient, not even in the initial design
	RAM(config, TI992_RAM_TAG).set_default_size("4096").set_default_value(0);

	// Cassette drives
	// There is no route from the cassette to some audio input,
	// so we don't hear it.
	CASSETTE(config, "cassette", 0);

	// Hexbus
	HEXBUS(config, TI992_HEXBUS_TAG, 0, hexbus_options, nullptr);

	// Expansion port (backside)
	TI992_EXPPORT(config, m_expport, 0, ti992_expport_options, nullptr);
}

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

} // Anonymous namespace


//      YEAR    NAME      PARENT    COMPAT  MACHINE   INPUT   CLASS         INIT        COMPANY              FULLNAME                               FLAGS
COMP(   1983,   ti99_224, 0,        0,      ti99_224, 0, ti99_2_state, empty_init, "Texas Instruments", "TI-99/2 BASIC Computer (24 KiB ROM)" , MACHINE_NO_SOUND_HW )
COMP(   1983,   ti99_232, 0,        0,      ti99_232, 0, ti99_2_state, empty_init, "Texas Instruments", "TI-99/2 BASIC Computer (32 KiB ROM)" , MACHINE_NO_SOUND_HW )
