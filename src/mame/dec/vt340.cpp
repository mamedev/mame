// license:BSD-3-Clause
// copyright-holders:Eric Harley
/***************************************************************************

    Digital Equipment Corporation VT340

    ROM-level hardware reconstruction.  The original firmware drives the
    emulated processors, serial devices and Dragon graphics hardware; the
    driver does not replace terminal protocols or synthesize display text.

    Emulation status

    The available firmware completes POST, displays VT340 OK, enters Set-Up,
    and operates as an interactive text and graphics terminal through Comm 1
    or Comm 2.  Firmware-generated text, scrolling, ReGIS, Tektronix graphics,
    and color SIXEL exercise the native display command paths.

    Modeled:
      * Both 8031 processors, cartridge ROM reconstruction and CPU2 banking
      * Shared XDATA RAM and the established CPU1/CPU2 startup handshakes
      * ROM-driven LK201 keyboard and persistent E80 configuration EEPROM
      * SCN2681 Comm 1/Comm 2 UARTs with MAME RS-232 endpoints
      * SCN2641 register interface, interrupt, modem inputs and local loopback
      * DC323 command engine, four Viper planes, E49 character ROM and bitmap
      * E51/E52 output map, 16-entry color palette and an 800 by 500 display
      * Frame-synchronous scrolling and the established DC7051 raster timing

    Partial or compatibility behavior:
      * DC7051 timing uses fixed, driver-local frame/scanline waveforms rather
        than a complete programmable DC7051 device.
      * Four board-strobe and DAT9 observations remain instruction-qualified:
        CPU1 3941 records and 3940 consumes its MOVX read strobe, CPU2 1703
        identifies the startup DAT9 pulse, and CPU1 3925 consumes the retained
        pulse.  Lower-level bus and MCS-51 timing support is still required.
      * The separate bitmap POST aperture models the observed diagnostic state
        machine rather than general-purpose memory arbitration.
      * With the available V1.1 cartridge, CPU2 publishes a transient startup
        checkpoint before CPU1 starts waiting for it.  Native firmware then
        takes its approximately 100.8-second fallback path.  A default-off
        configuration option skips the missed checkpoint for interactive use.
      * DC323/Viper fidelity limits are documented in video/dc323.cpp.

    Not modeled:
      * Printer support.  E81's SCN2641 is sufficient for firmware POST, but
        its external TxD/RxD/RTS connector, printer-ready wiring, attached
        printer and Media Copy output path are not connected in this driver.
      * Mouse/locator support.  There is no VSXXX mouse or tablet device,
        locator-port signaling, coordinate/button input, or deterministic
        ReGIS locator event injection.
      * Shared-bus contention and cycle-exact arbitration between processors.
      * Terminal bell/speaker audio.

                              VT340 data flow

        LK201 ------> internal UART                         DC7051 timing
                         |                                      |
        Comm 1/2 <-> SCN2681                               frame/scanline
                         |                                      |
        E80 EEPROM <-> CPU1 <------ shared XDATA ------> CPU2   |
                         |             and DAT9              |   |
        SCN2641 registers|                                    v v
        (no printer I/O) +-----------------------------> DC323/Vipers
                                                               |
                                  E49 character ROM -----------+
                                                               v
                                 four bitmap planes -> E51/E52 -> screen

        VSXXX mouse/tablet/locator: not connected

    Terminal control module (54-16965-01):
      E57  Intel 8031, processor 1 (see reserved-SFR compatibility note)
      E24  Intel 8031, processor 2
      Y2   12 MHz oscillator feeding CPUACLK/CPUBCLK

    Cartridge program buses:
      CPU 1: E1 at 0000-7fff, E2 at 8000-ffff
      CPU 2: E3 at 0000-7fff, E4/E5 banked at 8000-ffff

    The published cartridge dumps lag the requested address by one byte:
    byte 1 contains EPROM address 0, while raw page-boundary bytes preserve
    otherwise lost xxFF locations.  Loading reconstructs that address stream,
    repairs one independently corrupt descriptor byte, and derives the
    uncaptured checksum bytes.  The lag is consistent with the original
    Arduino reader sampling immediately after changing its address outputs,
    without an EPROM access-time delay.

    The processor-2 INT1/P3.3 latch is ROMBANK1 L.  Controlled execution of
    the five-27256 cartridge establishes its effective mapping: E4 is selected
    when the latch is high and E5 when it is low.

    References:
      * DEC VT340 schematic drawing set and pocket service guide
        https://bitsavers.org/pdf/dec/terminal/vt340/
      * Malcolm Macleod's VT340 internals, cartridge wiring and ROM dumps
        https://avitech.com.au/?page_id=1964
      * DEC terminal ROM archive (E80 image 23-001N2)
        https://www.9track.net/roms/

***************************************************************************/

#include "emu.h"

#include "lk201.h"

#include "bus/rs232/rs232.h"
#include "cpu/mcs51/i8051.h"
#include "machine/eeprompar.h"
#include "machine/mc68681.h"
#include "machine/scn_pci.h"
#include "video/dc323.h"

#include "screen.h"

#define LOG_PORTS (1U << 1)
#define LOG_XDATA (1U << 2)
#define LOG_DC7051_SYNC (1U << 3)
#define LOG_DC7051_RESET (1U << 4)

// Enable LOG_PORTS | LOG_XDATA in a local diagnostic build.
#define VERBOSE (0)
#include "logmacro.h"


namespace {

// The DC323 device numbers Viper planes from zero upward, while terminal
// control module sheet BP0 connects their serialized outputs to the E51/E52
// output-map address latch in the opposite bit significance.  Keep the
// device's natural plane numbering internally and translate at scanout.
constexpr u8 output_map_address(u8 planes)
{
	return bitswap<4>(planes, 0, 1, 2, 3);
}

static_assert(output_map_address(0x1) == 0x8);
static_assert(output_map_address(0x6) == 0x6);
static_assert(output_map_address(0x9) == 0x9);
static_assert(output_map_address(0xe) == 0x7);

bool should_log_xdata(offs_t offset, u32 &trace_events, u32 &io_trace_events)
{
	bool const io_access = ((offset & 0xfff0) == 0x7ff0) || (offset >= 0xf000);
	bool const log_trace = trace_events < 256;
	if (trace_events != std::numeric_limits<u32>::max())
		++trace_events;

	if (log_trace || !io_access)
		return log_trace;

	bool const log_io = io_trace_events < 512;
	if (io_trace_events != std::numeric_limits<u32>::max())
		++io_trace_events;
	return log_io;
}

// Match the active communication setup produced by the recovered E80 image.
// These defaults apply to byte-stream endpoint devices; changing the
// terminal's Set-Up values also requires changing the attached endpoint to
// the same framing.
static DEVICE_INPUT_DEFAULTS_START(comm1_defaults)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_19200)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_19200)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
	// The firmware uses software receive flow control while it moves or redraws
	// large text/graphics regions, so the attached host must honor its XOFF.
	DEVICE_INPUT_DEFAULTS("FLOW_CONTROL", 0x07, 0x04)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(comm2_defaults)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
	DEVICE_INPUT_DEFAULTS("FLOW_CONTROL", 0x07, 0x00)
DEVICE_INPUT_DEFAULTS_END

class vt340_state : public driver_device
{
public:
	vt340_state(machine_config const &mconfig, device_type type, char const *tag) :
		driver_device(mconfig, type, tag),
		m_cpu1(*this, "cpu1"),
		m_cpu2(*this, "cpu2"),
		m_keyboard(*this, "keyboard"),
		m_duart(*this, "duart"),
		m_comm1(*this, "comm1"),
		m_comm2(*this, "comm2"),
		m_printer_uart(*this, "acia"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_dc323(*this, "dc323"),
		m_startup(*this, "STARTUP"),
		m_cpu2banks_rom(*this, "cpu2banks"),
		m_shared_ram(*this, "shared_ram", 0x10000, ENDIANNESS_LITTLE),
		m_bitmap_ram(*this, "bitmap_ram", BITMAP_POST_SIZE, ENDIANNESS_LITTLE)
	{
	}

	void vt340(machine_config &config);
	void init_vt340();

private:
	enum dc323_post_mode : u8
	{
		DC323_POST_INACTIVE,
		DC323_POST_ARMED,
		DC323_POST_PATTERN_AA,
		DC323_POST_EXPECT_PATTERN_55,
		DC323_POST_PATTERN_55,
		DC323_POST_VERIFY_CLEAR,
		DC323_POST_RELEASE_PENDING
	};

	enum dat9_bus_phase : u8
	{
		DAT9_WAIT_BRD_READ,
		DAT9_BRD_READ_LOW,
		DAT9_WAIT_STARTUP_PULSE,
		DAT9_STARTUP_RESPONSE_LOW,
		DAT9_WAIT_DC323_RESET,
		DAT9_WAIT_DC323_STATUS,
		DAT9_DC323_STATUS_HIGH,
		DAT9_DC323_STATUS_LOW,
		DAT9_RESOLVED_PORTS
	};

	static constexpr u8 MCS51_SFR_SPACE = 4;
	static constexpr u8 MCS51_IDATA_SPACE = 5;
	static constexpr u32 BITMAP_POST_SIZE = 0x10000;
	static constexpr u32 DC7051_DOT_CLOCK = 32'317'400;
	static constexpr u32 DC7051_HORIZONTAL_TOGGLE_RATE = 63'000;
	static constexpr u32 DC7051_HORIZONTAL_TOTAL = 1026;
	static constexpr u32 DC7051_VERTICAL_TOTAL = 526;
	static constexpr u32 DC7051_FRAME_CLOCKS = DC7051_HORIZONTAL_TOTAL * DC7051_VERTICAL_TOTAL;
	static constexpr u32 DC7051_VISIBLE_WIDTH = 800;
	static constexpr u32 DC7051_VISIBLE_HEIGHT = 500;
	static constexpr u32 DC7051_DIAGNOSTIC_FRAME_US = 16'698;
	static constexpr u32 DC7051_DIAGNOSTIC_PULSE_US = 27;
	static constexpr offs_t SERIAL_UART_MASK = 0xffc0;
	static constexpr offs_t BOARD_REGISTER_MASK = 0xffe0;
	static constexpr offs_t PRINTER_UART_BASE = 0xf000;
	static constexpr offs_t DUART_BASE = 0xf040;
	static constexpr offs_t DC323_BASE = 0xf000;
	static constexpr offs_t DC323_MASK = 0xf0c0;
	static constexpr offs_t VIPER_SELECT_BASE = 0xf060;
	static constexpr offs_t PALETTE_RG_BASE = 0xf080;
	static constexpr offs_t PALETTE_GB_BASE = 0xf0a0;

	static constexpr bool selects(offs_t address, offs_t base, offs_t mask)
	{
		return (address & mask) == base;
	}

	required_device<i8031_device> m_cpu1;
	required_device<i8031_device> m_cpu2;
	required_device<lk201_device> m_keyboard;
	required_device<scn2681_device> m_duart;
	required_device<rs232_port_device> m_comm1;
	required_device<rs232_port_device> m_comm2;
	required_device<scn2641_device> m_printer_uart;
	required_device<eeprom_parallel_2804_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<dc323_device> m_dc323;
	required_ioport m_startup;
	required_region_ptr<u8> m_cpu2banks_rom;
	memory_share_creator<u8> m_shared_ram;
	memory_share_creator<u8> m_bitmap_ram;

	// Processor port latches and board-level handshake state
	u8 m_cpu1_p3 = 0xff;
	u8 m_cpu2_p3 = 0xff;
	u8 m_cpu1_p1 = 0xff;
	u8 m_cpu2_p1 = 0xff;
	u8 m_cpu2_p2 = 0xff;
	u8 m_dat9_bus_phase = DAT9_WAIT_BRD_READ;
	bool m_board_status = false;
	u8 m_video_test_select = 0;
	bool m_cpu1_brd_strobe = false;
	bool m_cpu2_startup_pulse_pending = false;
	bool m_keyboard_rxd = true;

	// Graphics POST compatibility state
	u8 m_dc323_post_mode = DC323_POST_INACTIVE;
	bool m_dc323_post_done = false;
	u32 m_bitmap_test_reads = 0;

	// Board-owned timing and palette state around the DC323/Viper subsystem
	std::array<u8, 16> m_palette_rg{};
	std::array<u8, 16> m_palette_gb{};
	u16 m_palette_rg_valid = 0;
	u16 m_palette_gb_valid = 0;
	bool m_display_dirty = true;
	u64 m_display_dirty_through = 0;
	u32 m_cpu1_trace_events = 0;
	u32 m_cpu2_trace_events = 0;
	u32 m_cpu1_io_trace_events = 0;
	u32 m_cpu2_io_trace_events = 0;
	emu_timer *m_dc7051_horizontal_timer = nullptr;
	emu_timer *m_dc7051_scanline_timer = nullptr;
	emu_timer *m_dc7051_frame_timer = nullptr;
	bool m_dc7051_hsync_asserted = true;
	u64 m_dc7051_horizontal_transitions = 0;
	u64 m_dc7051_frame_events = 0;

	void cpu1_program_map(address_map &map) ATTR_COLD;
	void cpu2_program_map(address_map &map) ATTR_COLD;
	void cpu1_data_map(address_map &map) ATTR_COLD;
	void cpu2_data_map(address_map &map) ATTR_COLD;

	u8 cpu1_xdata_r(offs_t offset);
	void cpu1_xdata_w(offs_t offset, u8 data);
	bool eeprom_selected(offs_t offset) const;
	u8 cpu1_reserved_c2_r();
	u8 cpu1_p3_r();
	u8 cpu2_upper_r(offs_t offset);
	u8 cpu2_xdata_r(offs_t offset);
	void cpu2_xdata_w(offs_t offset, u8 data);
	void cpu1_p3_w(u8 data);
	void keyboard_rxd_w(int state);
	u8 duart_input_r();
	void cpu2_p3_w(u8 data);
	u8 cpu1_p1_r();
	void cpu1_p1_w(u8 data);
	u8 cpu2_p1_r();
	void cpu2_p1_w(u8 data);
	bool dat9_device_low() const;
	void update_dat9_input();
	u8 cpu2_p2_r();
	void cpu2_p2_w(u8 data);
	u8 cpu2_p3_r();
	bool dc323_post_bitmap_selected() const;
	void release_dc323_post();
	void apply_startup_rom_patch();
	int dc7051_diagnostic_vsync_r() const;
	u16 dc7051_raster_line_r() const;
	void dc7051_post_load();
	TIMER_CALLBACK_MEMBER(dc7051_horizontal_tick);
	TIMER_CALLBACK_MEMBER(dc7051_scanline_tick);
	TIMER_CALLBACK_MEMBER(dc7051_frame_tick);
	u32 screen_update(screen_device &, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void trace_exit();
};


void vt340_state::init_vt340()
{
	// The published Arduino dump changes the high address byte before the low
	// byte and samples without a settling delay.  Apart from byte zero, the
	// file is one byte behind the requested address.  At each page transition,
	// raw xx00 consequently preserves logical xxFF while the usual shifted
	// reconstruction incorrectly substitutes the following page's last byte.
	auto reconstruct = [this](char const *destination, u32 destination_offset, char const *source)
	{
		u8 *const dest = memregion(destination)->base() + destination_offset;
		u8 const *const raw = memregion(source)->base();

		std::copy_n(raw + 1, 0x7fff, dest);
		for (u32 page = 1; page < 0x7f; page++)
			dest[(page << 8) | 0xff] = raw[page << 8];
	};

	auto finish_checksum = [this](char const *region, u32 offset, u8 seed)
	{
		u8 *const data = memregion(region)->base() + offset;
		u8 checksum = seed;
		for (u32 address = 0; address < 0x7fff; address++)
			checksum = u8((data[address] << 1) | (data[address] >> 7)) ^ checksum;
		data[0x7fff] = u8((checksum >> 1) | (checksum << 7));
	};

	reconstruct("cpu1low", 0x0000, "e1raw");
	reconstruct("cpu1high", 0x0000, "e2raw");
	reconstruct("cpu2low", 0x0000, "e3raw");
	reconstruct("cpu2banks", 0x0000, "e4raw");
	reconstruct("cpu2banks", 0x8000, "e5raw");

	// This descriptor byte is an independent non-boundary dump defect.  CPU1
	// emits command 21 as FF 21 01 xx, so the E4 descriptor is length 02 and
	// handler C569; dumped 69 would consume the following packets as payload.
	memregion("cpu2banks")->base()[0x622d] = 0x02;

	// The last byte was not captured.  The firmware's rotate/XOR POST yields
	// its unique value after all recoverable page-boundary bytes are restored.
	finish_checksum("cpu1low", 0x0000, 0x07);
	finish_checksum("cpu1high", 0x0000, 0x08);
	finish_checksum("cpu2low", 0x0000, 0x07);
	finish_checksum("cpu2banks", 0x0000, 0x08);
	finish_checksum("cpu2banks", 0x8000, 0x09);
}


void vt340_state::cpu1_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("cpu1low", 0);
	map(0x8000, 0xffff).rom().region("cpu1high", 0);
}

void vt340_state::cpu2_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("cpu2low", 0);
	map(0x8000, 0xffff).r(FUNC(vt340_state::cpu2_upper_r));
}

u8 vt340_state::cpu2_upper_r(offs_t offset)
{
	// P3.3 is the ROMBANK1 L signal.  The firmware changes it with bit
	// instructions, which do not pass through the external port callback.
	// Select from the live port latch so opcode fetches see the same signal as
	// the cartridge decode.  On the dumped five-ROM cartridge, high selects E4
	// and low selects E5.
	u16 const bank = BIT(m_cpu2->state_int(MCS51_P3), 3) ? 0x0000 : 0x8000;
	return m_cpu2banks_rom[bank | offset];
}

void vt340_state::cpu1_data_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(vt340_state::cpu1_xdata_r), FUNC(vt340_state::cpu1_xdata_w));
}

void vt340_state::cpu2_data_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(vt340_state::cpu2_xdata_r), FUNC(vt340_state::cpu2_xdata_w));
}

u8 vt340_state::cpu1_xdata_r(offs_t offset)
{
	if ((VERBOSE & LOG_XDATA) && should_log_xdata(offset, m_cpu1_trace_events, m_cpu1_io_trace_events))
		LOGMASKED(LOG_XDATA, "%s CPU1 XDATA R %04X (PC=%04X)\n", machine().describe_context(), offset, m_cpu1->pc());
	// E80's A0-A8 inputs are driven directly by CPU1's multiplexed external
	// address bus.  E10 shifts P1.3 (NVRDS L) into the active-low NVRCS latch
	// under P1.7 (NVRCLK H); every firmware access performs that sequence at
	// 4156 immediately before MOVX.  The live port latch is required here
	// because MCS-51 bit instructions do not pass through the port callback.
	// While selected, E80 owns the bus even at addresses which overlap the
	// ordinary serial-device decode.
	if (eeprom_selected(offset))
		return m_eeprom->read(m_cpu1->space(AS_DATA), offset & 0x01ff);
	// E78 decodes the CPU1 I/O page into two 64-byte serial apertures.  E81
	// (SCN2641) occupies F000-F03F and E61 (SCN2681) occupies F040-F07F;
	// their register-select inputs use the low two and low four address bits.
	if (selects(offset, PRINTER_UART_BASE, SERIAL_UART_MASK))
		return m_printer_uart->read(offset & 0x03);
	if (selects(offset, DUART_BASE, SERIAL_UART_MASK))
		return m_duart->read(offset & 0x0f);
	// P1 BRD L reaches T0 during MOVX.  The core exposes neither that internal
	// bus phase nor a read-strobe callback, while POST samples its occurrence
	// in the 393D loop.  Retain one observation until the matching pin sample;
	// a general T0 pulse also clocks the counter and is not equivalent.
	if (m_cpu1->pc() == 0x3941)
		m_cpu1_brd_strobe = true;
	return m_shared_ram[offset];
}

u8 vt340_state::cpu1_reserved_c2_r()
{
	return 0xff;
}

void vt340_state::cpu1_xdata_w(offs_t offset, u8 data)
{
	if ((VERBOSE & LOG_XDATA) && should_log_xdata(offset, m_cpu1_trace_events, m_cpu1_io_trace_events))
		LOGMASKED(LOG_XDATA, "%s CPU1 XDATA W %04X=%02X (PC=%04X)\n", machine().describe_context(), offset, data, m_cpu1->pc());
	if (eeprom_selected(offset))
	{
		m_eeprom->write(offset & 0x01ff, data);
		return;
	}
	if (selects(offset, PRINTER_UART_BASE, SERIAL_UART_MASK))
	{
		m_printer_uart->write(offset & 0x03, data);
		return;
	}
	if (selects(offset, DUART_BASE, SERIAL_UART_MASK))
	{
		m_duart->write(offset & 0x0f, data);
		return;
	}
	m_shared_ram[offset] = data;
}

bool vt340_state::eeprom_selected(offs_t offset) const
{
	// E10 latches active-low NVRCS from P1.3 (NVRDS L) under P1.7
	// (NVRCLK H).  MCS-51 bit instructions bypass the port callback, so use
	// the live port latch for both reads and writes.
	return !BIT(m_cpu1->state_int(MCS51_P1), 3) && ((offset & 0xfe00) == 0xf000);
}

u8 vt340_state::cpu2_xdata_r(offs_t offset)
{
	if ((VERBOSE & LOG_XDATA) && should_log_xdata(offset, m_cpu2_trace_events, m_cpu2_io_trace_events))
		LOGMASKED(LOG_XDATA, "%s CPU2 XDATA R %04X (PC=%04X)\n", machine().describe_context(), offset, m_cpu2->pc());

	// The first external access after the three bitmap sweeps returns the bus
	// to normal decoding.  The intervening DAT9 handshakes do not use XDATA.
	if (m_dc323_post_mode == DC323_POST_RELEASE_PENDING)
		release_dc323_post();

	// DC323 POST selects bitmap memory for the two 0000-EFFF pattern sweeps
	// and the final 0000-FFFF verify/clear pass.  Firmware performs
	// write/read AA, write/read 55, then verify-55/write-zero/read-zero.  Track
	// phase completion from the value at the last decoded address rather than
	// from the program counter of the generic memory-test routine.
	if (dc323_post_bitmap_selected())
	{
		u8 const data = m_bitmap_ram[offset];
		++m_bitmap_test_reads;
		if (offset == 0xefff)
		{
			if ((m_dc323_post_mode == DC323_POST_PATTERN_AA) && (data == 0xaa))
				m_dc323_post_mode = DC323_POST_EXPECT_PATTERN_55;
			else if ((m_dc323_post_mode == DC323_POST_PATTERN_55) && (data == 0x55))
				m_dc323_post_mode = DC323_POST_VERIFY_CLEAR;
		}
		else if ((offset == 0xffff) && (m_dc323_post_mode == DC323_POST_VERIFY_CLEAR) && (data == 0x00))
			m_dc323_post_mode = DC323_POST_RELEASE_PENDING;
		return data;
	}

	// E16 decodes the two 16-entry output-map apertures.  BA4 is not decoded,
	// so F080/F090 address the red map and F0A0/F0B0 address the green/blue
	// map.  Firmware's output-map loader at 743c writes exactly these two
	// tables.  E51 stores red in its low nibble (its upper nibble is unused).
	// On E52, DA3-DA0 drive green and DB3-DB0 drive blue; these correspond to
	// the stored high and low nibbles respectively.
	if (selects(offset, PALETTE_RG_BASE, BOARD_REGISTER_MASK))
		return m_palette_rg[offset & 0x0f];
	if (selects(offset, PALETTE_GB_BASE, BOARD_REGISTER_MASK))
		return m_palette_gb[offset & 0x0f];

	// The E15 PAL decodes the first 64 DC323 Adder registers in the F000 I/O
	// page.  BA11-BA8 are not decoded, so the block is mirrored.  Register
	// identities and bit assignments match DEC's Dragon chipset specification.
	if (selects(offset, DC323_BASE, DC323_MASK))
	{
		unsigned const reg = offset & 0x3f;
		return m_dc323->read(reg);
	}

	// P2 BRD L is also presented on the multiplexed DAT9/TSTATUS net.  Retain
	// the low level from the bus cycle until CPU2 samples its port immediately
	// afterwards; subsequent ownership is advanced by the resolved bus model.
	if (m_dat9_bus_phase == DAT9_WAIT_BRD_READ)
	{
		m_dat9_bus_phase = DAT9_BRD_READ_LOW;
		update_dat9_input();
	}
	return m_shared_ram[offset];
}

bool vt340_state::dc323_post_bitmap_selected() const
{
	return (m_dc323_post_mode >= DC323_POST_PATTERN_AA) &&
		(m_dc323_post_mode <= DC323_POST_VERIFY_CLEAR);
}

void vt340_state::release_dc323_post()
{
	m_dc323_post_mode = DC323_POST_INACTIVE;
	m_dc323_post_done = true;
	m_dc323->operational_w(1);
}

void vt340_state::cpu2_xdata_w(offs_t offset, u8 data)
{
	if ((VERBOSE & LOG_XDATA) && should_log_xdata(offset, m_cpu2_trace_events, m_cpu2_io_trace_events))
		LOGMASKED(LOG_XDATA, "%s CPU2 XDATA W %04X=%02X (PC=%04X)\n", machine().describe_context(), offset, data, m_cpu2->pc());
	if (m_dc323_post_mode == DC323_POST_RELEASE_PENDING)
		release_dc323_post();

	// F072 arms the DC323 bitmap diagnostic.  The following write of AA to
	// address zero begins the first explicit test phase.
	if ((m_dc323_post_mode == DC323_POST_ARMED) && (offset == 0x0000) && (data == 0xaa))
	{
		m_dc323_post_mode = DC323_POST_PATTERN_AA;
		m_bitmap_test_reads = 0;
		m_palette_rg_valid = 0;
		m_palette_gb_valid = 0;
	}
	if ((m_dc323_post_mode == DC323_POST_EXPECT_PATTERN_55) &&
		(offset == 0x0000) && (data == 0x55))
		m_dc323_post_mode = DC323_POST_PATTERN_55;
	if (dc323_post_bitmap_selected())
	{
		m_bitmap_ram[offset] = data;
		return;
	}

	// E16 decodes F060-F07F as CNTLWR.  At that point in the multiplexed
	// 8031 bus cycle the write data appears on BAD.  Schematic sheet 8 wires
	// BAD3 through BAD0 to V0CS through V3CS respectively (BAD5 separately
	// drives OPADDS).
	if (selects(offset, VIPER_SELECT_BASE, BOARD_REGISTER_MASK))
	{
		m_dc323->viper_select_w(bitswap<4>(data, 0, 1, 2, 3));

		// Graphics initialization finishes its F072 control sequence with FF
		// just before exercising the separately addressed bitmap store.
		if (!m_dc323_post_done && (offset == 0xf072) && (data == 0xff))
		{
			m_dc323_post_mode = DC323_POST_ARMED;
			if (m_dat9_bus_phase == DAT9_WAIT_DC323_RESET)
				m_dat9_bus_phase = DAT9_WAIT_DC323_STATUS;
		}
		// The second board-control initialization terminates DC323 reset.  DAT9
		// is released high for one status sample, then the DC323 supplies the
		// low completion phase before leaving the shared net to the processors.
		else if ((m_dat9_bus_phase == DAT9_WAIT_DC323_STATUS) &&
			(offset == VIPER_SELECT_BASE) && (data == 0x00))
		{
			m_dat9_bus_phase = DAT9_DC323_STATUS_HIGH;
			update_dat9_input();
		}
		return;
	}

	// F0E0 selects a timing-generator signal onto P1.0 for the firmware's
	// measured horizontal/vertical timing diagnostics.
	if (m_board_status && (offset == 0xf0e0))
	{
		m_video_test_select = data;
		return;
	}

	if (selects(offset, PALETTE_RG_BASE, BOARD_REGISTER_MASK))
	{
		unsigned const index = offset & 0x0f;
		u16 const entry = 1U << index;
		bool const changed = !(m_palette_rg_valid & entry) || (m_palette_rg[index] != data);
		// Preserve colors already scanned before changing a complete E51/E52
		// entry.  An incomplete entry still uses the monochrome fallback, and a
		// repeated value cannot change the visible output.
		if ((m_palette_gb_valid & entry) && changed)
		{
			m_screen->update_partial(m_screen->vpos());
			m_display_dirty = true;
		}
		m_palette_rg[index] = data;
		m_palette_rg_valid |= entry;
		LOGMASKED(LOG_XDATA, "%s E51 palette[%X]=%02X (PC=%04X)\n", machine().describe_context(), offset & 0x0f, data, m_cpu2->pc());
		return;
	}
	if (selects(offset, PALETTE_GB_BASE, BOARD_REGISTER_MASK))
	{
		unsigned const index = offset & 0x0f;
		u16 const entry = 1U << index;
		bool const changed = !(m_palette_gb_valid & entry) || (m_palette_gb[index] != data);
		if ((m_palette_rg_valid & entry) && changed)
		{
			m_screen->update_partial(m_screen->vpos());
			m_display_dirty = true;
		}
		m_palette_gb[index] = data;
		m_palette_gb_valid |= entry;
		LOGMASKED(LOG_XDATA, "%s E52 palette[%X]=%02X (PC=%04X)\n", machine().describe_context(), offset & 0x0f, data, m_cpu2->pc());
		return;
	}

	if (selects(offset, DC323_BASE, DC323_MASK))
	{
		unsigned const reg = offset & 0x3f;
		u16 const value = (u16(m_cpu2_p1) << 8) | data;
		LOGMASKED(LOG_XDATA, "%s DC323 W %02X=%04X (PC=%04X)\n", machine().describe_context(), reg, value, m_cpu2->pc());
		m_dc323->write(reg, value);
		return;
	}

	m_shared_ram[offset] = data;
}

void vt340_state::cpu1_p3_w(u8 data)
{
	u8 const old = m_cpu1_p3;
	if (data != m_cpu1_p3)
		LOGMASKED(LOG_PORTS, "%s CPU1 P3 %02X -> %02X (PC=%04X)\n", machine().describe_context(), m_cpu1_p3, data, m_cpu1->pc());
	m_cpu1_p3 = data;
	update_dat9_input();
	// CPU1 P3.1/TXD is the terminal-to-keyboard half of the LK201 serial
	// connection.  Feed the live port level to the keyboard so the ROM's
	// native 4800-baud command stream is observed by the real device firmware.
	m_keyboard->rx_w(BIT(data, 1));
	if (BIT(old, 5) != BIT(data, 5))
		m_cpu1->abort_timeslice();
}

void vt340_state::keyboard_rxd_w(int state)
{
	m_keyboard_rxd = bool(state);
}

u8 vt340_state::duart_input_r()
{
	// E61 input pins, schematic sheet 5: Comm 1 supplies CTS, DSR, speed
	// indication and carrier detect.  The 25-pin RS-232 and six-pin DEC-423
	// connectors are alternate electrical interfaces for channel A, so the
	// abstract Comm 1 endpoint supplies both DSR inputs.  Comm 2 supplies DSR
	// only.  IP6 is TVSYNC and retains the previously established idle level.
	return BIT(m_comm1->cts_r(), 0)
		| (BIT(m_comm1->dsr_r(), 0) << 1)
		| (BIT(m_comm1->si_r(), 0) << 2)
		| (BIT(m_comm1->dcd_r(), 0) << 3)
		| (BIT(m_comm1->dsr_r(), 0) << 4)
		| (BIT(m_comm2->dsr_r(), 0) << 5)
		| 0x40;
}

u8 vt340_state::cpu1_p3_r()
{
	// CPU2 P1.1 and CPU1 P3.5/T1 share an open-collector-style status net.
	// Reflect the peer's live output latch in port reads as well as on the
	// core's external T1 input line.
	u8 data = m_cpu1_p3;
	// CPU1's internal UART receives the LK201 keyboard on P3.0/RXD.
	data = m_keyboard_rxd ? (data | 0x01) : (data & ~0x01);
	if (!BIT(m_cpu2_p1, 1))
		data &= ~0x20;
	if (dat9_device_low())
		data &= ~0x20;
	if (!machine().side_effects_disabled() &&
		(m_cpu1->pc() == 0x3925) && m_cpu2_startup_pulse_pending)
	{
		// Compatibility retention for the first CPU2 1701 pulse.  Without it,
		// CPU2 releases DAT9 about 8.55 ms before CPU1 begins sampling at 3922;
		// retain the event until the missing startup timing is understood.
		data &= ~0x20;
		m_cpu2_startup_pulse_pending = false;
	}
	if (!machine().side_effects_disabled() &&
		(m_cpu1->pc() == 0x3940) && m_cpu1_brd_strobe)
	{
		data &= ~0x10;
		m_cpu1_brd_strobe = false;
	}
	return data;
}

u8 vt340_state::cpu1_p1_r()
{
	// P1.4 is the NVREN output and P1.5 is MFGTEST/STATUS.  Both are driven by
	// CPU1 firmware during startup; do not replace either live output latch
	// with the separate graphics-diagnostic state tracked by the driver.
	//
	// CPU1 samples the resolved P1.7/P1.6 (NVRCLK/CPMRESET) pin levels during
	// startup.  The full VT340 configuration resolves P1.7 low, producing model
	// value 0; value 1 is the VT330 personality and value 2 is a reduced VT340
	// personality without the Color Map/Color Set-Up controls.  This machine is
	// the full VT340 color assembly, so expose that board-level identification
	// level continuously.
	// The exact off-sheet source of the resolved low level remains unknown.
	return m_cpu1_p1 & ~0x80;
}

void vt340_state::cpu1_p1_w(u8 data)
{
	if (data != m_cpu1_p1)
		LOGMASKED(LOG_PORTS, "%s CPU1 P1 %02X -> %02X (PC=%04X)\n", machine().describe_context(), m_cpu1_p1, data, m_cpu1->pc());
	m_cpu1_p1 = data;

	// CPU1 P1.0 is the active-high P1 RESET H net connected directly to
	// CPU2's RST pin (terminal-control-module schematic sheets 1 and 2).
	m_cpu2->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
}

u8 vt340_state::cpu2_p1_r()
{
	// P1 is the bidirectional P2 DAT8-15/status bus.  DAT9 is wired to CPU1's
	// T1/P3.5 pin, so either processor can pull the quasi-bidirectional net
	// low.  This is the power-on interprocessor handshake at CPU2 1701-1710.
	u8 data = m_cpu2_p1;
	if (!BIT(m_cpu1_p3, 5))
		data &= ~0x02;
	if (dat9_device_low())
		data &= ~0x02;

	// A port sample completes retained bus-turnaround phases.  The returned
	// value is from the phase that owned DAT9 for this sample; ownership changes
	// take effect on the resolved net after the read.
	if (!machine().side_effects_disabled())
	{
		switch (m_dat9_bus_phase)
		{
		case DAT9_BRD_READ_LOW:
			m_dat9_bus_phase = DAT9_WAIT_STARTUP_PULSE;
			break;
		case DAT9_STARTUP_RESPONSE_LOW:
			m_dat9_bus_phase = DAT9_WAIT_DC323_RESET;
			break;
		case DAT9_DC323_STATUS_HIGH:
			m_dat9_bus_phase = DAT9_DC323_STATUS_LOW;
			break;
		case DAT9_DC323_STATUS_LOW:
			m_dat9_bus_phase = DAT9_RESOLVED_PORTS;
			m_board_status = true;
			break;
		default:
			break;
		}
		update_dat9_input();
	}
	if (m_video_test_select == 0x70)
	{
		// E15/PAL board logic selects one DC7051 diagnostic output onto P1.0.
		// The selected waveform remains the established narrow compatibility
		// pulse until the physical test-output mux is fully decoded.
		data = dc7051_diagnostic_vsync_r() ? (data | 0x01) : (data & ~0x01);
	}
	return data;
}

void vt340_state::cpu2_p1_w(u8 data)
{
	u8 const old = m_cpu2_p1;
	if (data != m_cpu2_p1)
		LOGMASKED(LOG_PORTS, "%s CPU2 P1 %02X -> %02X (PC=%04X)\n", machine().describe_context(), m_cpu2_p1, data, m_cpu2->pc());
	m_cpu2_p1 = data;
	if ((m_dat9_bus_phase == DAT9_WAIT_STARTUP_PULSE) && !BIT(old, 1) && BIT(data, 1))
		m_dat9_bus_phase = DAT9_STARTUP_RESPONSE_LOW;
	if (BIT(old, 1) && !BIT(data, 1) && (m_cpu2->pc() == 0x1703))
		m_cpu2_startup_pulse_pending = true;
	// P1.1 is the board's P2 DAT9/STATUS H net and is wired directly to
	// processor 1's external T1 pin.  MAME input-line ASSERT means that the
	// active-low external pin is being pulled low.
	update_dat9_input();
	if (BIT(old, 1) != BIT(data, 1))
		m_cpu2->abort_timeslice();
}

bool vt340_state::dat9_device_low() const
{
	return (m_dat9_bus_phase == DAT9_BRD_READ_LOW) ||
		(m_dat9_bus_phase == DAT9_STARTUP_RESPONSE_LOW) ||
		(m_dat9_bus_phase == DAT9_DC323_STATUS_LOW);
}

void vt340_state::update_dat9_input()
{
	bool const high = BIT(m_cpu1_p3, 5) && BIT(m_cpu2_p1, 1) && !dat9_device_low();
	m_cpu1->set_input_line(MCS51_T1_LINE, high ? CLEAR_LINE : ASSERT_LINE);
}

u8 vt340_state::cpu2_p2_r()
{
	return m_cpu2_p2;
}

void vt340_state::cpu2_p2_w(u8 data)
{
	if (data != m_cpu2_p2)
		LOGMASKED(LOG_PORTS, "%s CPU2 P2 %02X -> %02X (PC=%04X)\n", machine().describe_context(), m_cpu2_p2, data, m_cpu2->pc());
	m_cpu2_p2 = data;
}

u8 vt340_state::cpu2_p3_r()
{
	return m_cpu2_p3;
}

void vt340_state::cpu2_p3_w(u8 data)
{
	u8 const old = m_cpu2_p3;
	if (data != m_cpu2_p3)
		LOGMASKED(LOG_PORTS, "%s CPU2 P3 %02X -> %02X (PC=%04X)\n", machine().describe_context(), m_cpu2_p3, data, m_cpu2->pc());

	m_cpu2_p3 = data;
	// Bitmap POST first verifies AA directly, then programs the graphics test
	// path and writes/readbacks zero with T1 high.  Returning T1 low exposes
	// the complemented 55 result that the final pass verifies and clears.
	if (dc323_post_bitmap_selected() && BIT(old, 5) && !BIT(data, 5))
		std::fill_n(&m_bitmap_ram[0], BITMAP_POST_SIZE, 0x55);
}

int vt340_state::dc7051_diagnostic_vsync_r() const
{
	// Preserve the measured 27 us high interval in a 16.698 ms period.  These
	// values are compatibility behavior, not decoded DC7051 register fields.
	u64 const phase = machine().time().as_ticks(1'000'000) % DC7051_DIAGNOSTIC_FRAME_US;
	return phase < DC7051_DIAGNOSTIC_PULSE_US;
}

u16 vt340_state::dc7051_raster_line_r() const
{
	if (!m_dc7051_frame_timer)
		return 0;
	u64 const remaining = std::min<u64>(m_dc7051_frame_timer->remaining().as_ticks(DC7051_DOT_CLOCK), DC7051_FRAME_CLOCKS);
	u64 const phase = (DC7051_FRAME_CLOCKS - remaining) % DC7051_FRAME_CLOCKS;
	return u16(phase / DC7051_HORIZONTAL_TOTAL);
}

void vt340_state::dc7051_post_load()
{
	// Timers are restored by the scheduler/save system.  Re-drive the one
	// persistent external level without replaying a frame event.
	m_cpu2->set_input_line(MCS51_T0_LINE, m_dc7051_hsync_asserted ? ASSERT_LINE : CLEAR_LINE);
	m_display_dirty = true;
}

TIMER_CALLBACK_MEMBER(vt340_state::dc7051_horizontal_tick)
{
	bool const old = m_dc7051_hsync_asserted;
	m_dc7051_hsync_asserted = !m_dc7051_hsync_asserted;
	++m_dc7051_horizontal_transitions;
	m_cpu2->set_input_line(MCS51_T0_LINE, m_dc7051_hsync_asserted ? ASSERT_LINE : CLEAR_LINE);
	LOGMASKED(LOG_DC7051_SYNC, "%s DC7051 hsync old=%u new=%u transition=%llu\n",
		machine().time().to_string(), old, m_dc7051_hsync_asserted,
		(unsigned long long)m_dc7051_horizontal_transitions);
}

TIMER_CALLBACK_MEMBER(vt340_state::dc7051_scanline_tick)
{
	// Scroll and pause comparisons use the Y1 dot-clock raster rather than the
	// provisional 63 kHz compatibility waveform supplied to CPU2 T0.
	m_dc323->scanline_event(dc7051_raster_line_r());
}

TIMER_CALLBACK_MEMBER(vt340_state::dc7051_frame_tick)
{
	++m_dc7051_frame_events;
	LOGMASKED(LOG_DC7051_SYNC, "%s DC7051 frame event=%llu\n",
		machine().time().to_string(), (unsigned long long)m_dc7051_frame_events);
	m_dc323->frame_event();
}

u32 vt340_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	u64 const frame = screen.frame_number();
	bool const changed = std::exchange(m_display_dirty, false) | m_dc323->consume_display_dirty();
	// Palette writes can request partial updates while MAME alternates two
	// screen bitmaps.  Refresh both complete frames after any change before
	// allowing either bitmap to be reused; refreshing only the change frame
	// leaves the other buffer stale and makes Set-Up feedback one keypress late.
	if (changed)
		m_display_dirty_through = frame + 2;
	else if (frame > m_display_dirty_through)
		return UPDATE_HAS_NOT_CHANGED;

	// Four serial bitmap planes form a 16-entry E51/E52 color-map index.  Use
	// genuine 12-bit RGB after firmware has programmed both halves of an entry.
	// Entries not programmed after graphics POST retain a monochrome fallback
	// so firmware-written pixels remain visible without inventing RGB values.
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u8 planes = 0;
			for (unsigned plane = 0; plane < dc323_device::BITMAP_PLANES; plane++)
				planes |= m_dc323->display_pixel_r(plane, x, y) << plane;
			u8 const color = output_map_address(planes);
			if (BIT(m_palette_rg_valid & m_palette_gb_valid, color))
			{
				u8 const red = (m_palette_rg[color] & 0x0f) * 0x11;
				u8 const green = (m_palette_gb[color] >> 4) * 0x11;
				u8 const blue = (m_palette_gb[color] & 0x0f) * 0x11;
				bitmap.pix(y, x) = rgb_t(red, green, blue);
			}
			else
				bitmap.pix(y, x) = color ? rgb_t::white() : rgb_t::black();
		}
	return 0;
}

void vt340_state::machine_start()
{
	m_dc7051_horizontal_timer = timer_alloc(FUNC(vt340_state::dc7051_horizontal_tick), this);
	m_dc7051_scanline_timer = timer_alloc(FUNC(vt340_state::dc7051_scanline_tick), this);
	m_dc7051_frame_timer = timer_alloc(FUNC(vt340_state::dc7051_frame_tick), this);
	machine().save().register_postload(save_prepost_delegate(FUNC(vt340_state::dc7051_post_load), this));

	// The base MCS-51 core leaves reserved SFR reads at zero.  CPU1 firmware
	// deliberately reads reserved SFR C2 and requires the all-ones value
	// observed for this unimplemented location on the target processor.
	m_cpu1->space(MCS51_SFR_SPACE).install_read_handler(0xc2, 0xc2,
		read8smo_delegate(*this, FUNC(vt340_state::cpu1_reserved_c2_r)));
	if (VERBOSE & LOG_PORTS)
		machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&vt340_state::trace_exit, this));

	save_item(NAME(m_cpu1_p3));
	save_item(NAME(m_cpu2_p3));
	save_item(NAME(m_cpu1_p1));
	save_item(NAME(m_cpu2_p1));
	save_item(NAME(m_cpu2_p2));
	save_item(NAME(m_dat9_bus_phase));
	save_item(NAME(m_board_status));
	save_item(NAME(m_video_test_select));
	save_item(NAME(m_cpu1_brd_strobe));
	save_item(NAME(m_cpu2_startup_pulse_pending));
	save_item(NAME(m_keyboard_rxd));
	save_item(NAME(m_dc323_post_mode));
	save_item(NAME(m_dc323_post_done));
	save_item(NAME(m_bitmap_test_reads));
	save_item(NAME(m_palette_rg));
	save_item(NAME(m_palette_gb));
	save_item(NAME(m_palette_rg_valid));
	save_item(NAME(m_palette_gb_valid));
	save_item(NAME(m_display_dirty));
	save_item(NAME(m_display_dirty_through));
	save_item(NAME(m_cpu1_trace_events));
	save_item(NAME(m_cpu2_trace_events));
	save_item(NAME(m_cpu1_io_trace_events));
	save_item(NAME(m_cpu2_io_trace_events));
	save_item(NAME(m_dc7051_hsync_asserted));
	save_item(NAME(m_dc7051_horizontal_transitions));
	save_item(NAME(m_dc7051_frame_events));
}

void vt340_state::trace_exit()
{
	LOGMASKED(LOG_PORTS, "VT340 stop: CPU1 PC=%04X IE=%02X TCON=%02X B=%02X ERR=%02X; "
		"CPU2 PC=%04X IE=%02X TCON=%02X A=%02X B=%02X R2=%02X R3=%02X ROM=%s bitmap=%u/%u video=%02X\n",
		m_cpu1->pc(), m_cpu1->state_int(MCS51_IE), m_cpu1->state_int(MCS51_TCON),
		m_cpu1->state_int(MCS51_B), m_cpu1->space(MCS51_IDATA_SPACE).read_byte(0x42),
		m_cpu2->pc(), m_cpu2->state_int(MCS51_IE), m_cpu2->state_int(MCS51_TCON),
		m_cpu2->state_int(MCS51_ACC), m_cpu2->state_int(MCS51_B),
		m_cpu2->state_int(MCS51_R2), m_cpu2->state_int(MCS51_R3),
		BIT(m_cpu2->state_int(MCS51_P3), 3) ? "E4" : "E5", dc323_post_bitmap_selected(),
		m_bitmap_test_reads, m_video_test_select);
}

void vt340_state::machine_reset()
{
	apply_startup_rom_patch();
	m_cpu1_p3 = 0xff;
	m_cpu2_p3 = 0xff;
	m_cpu1_p1 = 0xff;
	m_cpu2_p1 = 0xff;
	m_cpu2_p2 = 0xff;
	m_dat9_bus_phase = DAT9_WAIT_BRD_READ;
	m_board_status = true;
	m_video_test_select = 0;
	m_cpu1_brd_strobe = false;
	m_cpu2_startup_pulse_pending = false;
	m_keyboard_rxd = true;
	m_dc323_post_mode = DC323_POST_INACTIVE;
	m_dc323_post_done = false;
	m_bitmap_test_reads = 0;
	m_palette_rg.fill(0);
	m_palette_gb.fill(0);
	m_palette_rg_valid = 0;
	m_palette_gb_valid = 0;
	m_display_dirty = true;
	m_display_dirty_through = 0;
	m_dc323->operational_w(0);
	m_dc323->bitmap_clear();
	m_cpu1_trace_events = 0;
	m_cpu2_trace_events = 0;
	m_cpu1_io_trace_events = 0;
	m_cpu2_io_trace_events = 0;
	m_dc7051_hsync_asserted = true;
	m_dc7051_horizontal_transitions = 0;
	m_dc7051_frame_events = 0;
	m_cpu2->set_input_line(MCS51_T0_LINE, ASSERT_LINE);
	attotime const horizontal_period = attotime::from_hz(DC7051_HORIZONTAL_TOGGLE_RATE);
	m_dc7051_horizontal_timer->adjust(horizontal_period, 0, horizontal_period);
	m_dc7051_scanline_timer->adjust(attotime::zero, 0,
		attotime::from_ticks(DC7051_HORIZONTAL_TOTAL, DC7051_DOT_CLOCK));
	attotime const frame_period = attotime::from_ticks(DC7051_FRAME_CLOCKS, DC7051_DOT_CLOCK);
	m_dc7051_frame_timer->adjust(frame_period, 0, frame_period);
	LOGMASKED(LOG_DC7051_RESET, "%s DC7051 reset horizontal_toggle=%u frame_period=%s\n",
		machine().time().to_string(), DC7051_HORIZONTAL_TOGGLE_RATE, frame_period.to_string());
	// Processor 1 T0 is tied to the active-low P1 BRD bus-read strobe.  The
	// startup bus test samples it while performing its probe read.
	m_cpu1->set_input_line(MCS51_T0_LINE, CLEAR_LINE);
	update_dat9_input();
	m_cpu2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	// Unconnected serial inputs idle in the marking state.  Leaving these
	// device pins at their zero-initialized level presents a continuous break
	// to the firmware and can manufacture receive characters during startup.
	m_duart->rx_a_w(1);
	m_duart->rx_b_w(1);
	m_printer_uart->rxd_w(1);
	// E81's active-low CTS and DCD inputs are tied to ground on schematic
	// sheet 8P0.  Local loopback ignores both pins, but normal mode must see
	// the permanently asserted levels implemented by the board.
	m_printer_uart->cts_w(0);
	m_printer_uart->dcd_w(0);
	m_cpu2->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
	LOGMASKED(LOG_PORTS, "VT340 reset: CPU1 E1/E2, CPU2 E3/E4 selected\n");
}

void vt340_state::apply_startup_rom_patch()
{
	u8 *const rom = memregion("cpu1low")->base();

	// The original firmware writes pending value 10, then waits for a transient
	// zero checkpoint at 3A18.  With the available V1.1 ROM set, CPU2 publishes
	// that zero before CPU1 installs the sentinel, so the terminal takes the
	// 100.8-second fallback loop.  This optional ROM patch writes zero at the
	// same decision point instead.  The following 3A31 loop still waits for
	// CPU2's genuine nonzero result and 3A3A still requires success value 80.
	if (BIT(m_startup->read(), 0))
		rom[0x3a0c] = 0x00; // MOV A,00 at 3A0B
	else
		rom[0x3a0c] = 0x10; // MOV A,10 at 3A0B

	// Preserve the native rotate/XOR ROM checksum after either selection.
	u8 checksum = 0x07;
	for (u32 address = 0; address < 0x7fff; address++)
		checksum = u8((rom[address] << 1) | (rom[address] >> 7)) ^ checksum;
	rom[0x7fff] = u8((checksum >> 1) | (checksum << 7));
}

void vt340_state::vt340(machine_config &config)
{
	// E57 is an Intel 8031 on the terminal-control-module schematic.
	I8031(config, m_cpu1, 12_MHz_XTAL);
	m_cpu1->set_addrmap(AS_PROGRAM, &vt340_state::cpu1_program_map);
	m_cpu1->set_addrmap(AS_DATA, &vt340_state::cpu1_data_map);
	m_cpu1->port_in_cb<1>().set(FUNC(vt340_state::cpu1_p1_r));
	m_cpu1->port_out_cb<1>().set(FUNC(vt340_state::cpu1_p1_w));
	m_cpu1->port_in_cb<3>().set(FUNC(vt340_state::cpu1_p3_r));
	m_cpu1->port_out_cb<3>().set(FUNC(vt340_state::cpu1_p3_w));

	LK201(config, m_keyboard);
	m_keyboard->tx_handler().set(FUNC(vt340_state::keyboard_rxd_w));

	// E61 is the SCN2681 DUART.  Its active-low interrupt output drives the
	// processor-1 INT0 input.  The 3.6864 MHz clock is the standard crystal
	// frequency used by the part's baud-rate generator.
	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set_inputline(m_cpu1, MCS51_INT0_LINE);
	m_duart->a_tx_cb().set(m_comm1, FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_comm2, FUNC(rs232_port_device::write_txd));
	m_duart->inport_cb().set(FUNC(vt340_state::duart_input_r));
	m_duart->outport_cb().set(m_comm1, FUNC(rs232_port_device::write_rts)).bit(0);
	m_duart->outport_cb().append(m_comm1, FUNC(rs232_port_device::write_dtr)).bit(1);
	m_duart->outport_cb().append(m_comm1, FUNC(rs232_port_device::write_spds)).bit(2);
	m_duart->outport_cb().append(m_comm2, FUNC(rs232_port_device::write_dtr)).bit(7);

	RS232_PORT(config, m_comm1, default_rs232_devices, nullptr);
	m_comm1->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(comm1_defaults));
	m_comm1->set_option_device_input_defaults("pty", DEVICE_INPUT_DEFAULTS_NAME(comm1_defaults));
	m_comm1->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_comm1->cts_handler().set(m_duart, FUNC(scn2681_device::ip0_w));
	m_comm1->dsr_handler().set(m_duart, FUNC(scn2681_device::ip1_w));
	m_comm1->dsr_handler().append(m_duart, FUNC(scn2681_device::ip4_w));
	m_comm1->si_handler().set(m_duart, FUNC(scn2681_device::ip2_w));
	m_comm1->dcd_handler().set(m_duart, FUNC(scn2681_device::ip3_w));

	RS232_PORT(config, m_comm2, default_rs232_devices, nullptr);
	m_comm2->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(comm2_defaults));
	m_comm2->set_option_device_input_defaults("pty", DEVICE_INPUT_DEFAULTS_NAME(comm2_defaults));
	m_comm2->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
	m_comm2->dsr_handler().set(m_duart, FUNC(scn2681_device::ip5_w));

	// E81 is the SCN2641 printer communications interface.  Its interrupt
	// output is connected to processor-1 INT1; the keyboard instead uses the
	// processor's internal UART on P3.0/P3.1.  Only the chip-side interface is
	// modeled here: the external printer connector and its TxD/RxD/RTS wiring
	// are intentionally absent until their board behavior is integrated.
	SCN2641(config, m_printer_uart, 3.6864_MHz_XTAL);
	m_printer_uart->intr_handler().set_inputline(m_cpu1, MCS51_INT1_LINE);

	EEPROM_2804(config, m_eeprom); // E80, Xicor X2804AP-45

	I8031(config, m_cpu2, 12_MHz_XTAL);
	m_cpu2->set_addrmap(AS_PROGRAM, &vt340_state::cpu2_program_map);
	m_cpu2->set_addrmap(AS_DATA, &vt340_state::cpu2_data_map);
	m_cpu2->port_in_cb<1>().set(FUNC(vt340_state::cpu2_p1_r));
	m_cpu2->port_out_cb<1>().set(FUNC(vt340_state::cpu2_p1_w));
	m_cpu2->port_in_cb<2>().set(FUNC(vt340_state::cpu2_p2_r));
	m_cpu2->port_out_cb<2>().set(FUNC(vt340_state::cpu2_p2_w));
	m_cpu2->port_in_cb<3>().set(FUNC(vt340_state::cpu2_p3_r));
	m_cpu2->port_out_cb<3>().set(FUNC(vt340_state::cpu2_p3_w));

	DC323(config, m_dc323, 0);
	m_dc323->set_chargen_tag("^chargen");
	// Preserve the root share name used by existing debugger regressions while
	// making the DC323 the owner of the operational bitmap storage.
	m_dc323->set_bitmap_tag("^bitmap_planes");
	m_dc323->irq_handler().set_inputline(m_cpu2, MCS51_INT0_LINE);

	// Y1 is the physical DC7051 dot-clock source.  Driver-local timers model
	// the currently established board timing until its programming interface
	// and exact output waveforms are decoded.
	m_dc323->raster_line_handler().set(FUNC(vt340_state::dc7051_raster_line_r));
	// Y1 clocks the DC7051.  Its 32.3174 MHz rate divided by the currently fixed
	// 1026 clocks and 526 lines gives the measured 31.5 kHz / 59.89 Hz raster.
	// The 800x500 visible area includes the terminal's 25th status line.
	SCREEN(config, m_screen);
	m_screen->set_raw(XTAL(DC7051_DOT_CLOCK), DC7051_HORIZONTAL_TOTAL,
		0, DC7051_VISIBLE_WIDTH, DC7051_VERTICAL_TOTAL, 0, DC7051_VISIBLE_HEIGHT);
	m_screen->set_screen_update(FUNC(vt340_state::screen_update));
}


static INPUT_PORTS_START(vt340)
	PORT_START("STARTUP")
	PORT_CONFNAME(0x01, 0x00, "Startup Checkpoint Workaround")
	PORT_CONFSETTING(0x00, "Original ROM Wait (~120 Seconds)")
	PORT_CONFSETTING(0x01, "Skip Missed Checkpoint")
INPUT_PORTS_END


ROM_START(vt340)
	// Malcolm Macleod dumped E1-E5 from a physical VT340 cartridge using an
	// Arduino reader.  The published files have a one-byte address lag and do
	// not contain the final byte.  Keep them byte-for-byte intact and mark them
	// as bad dumps; init_vt340 reconstructs recoverable boundaries and derives
	// the firmware checksum bytes.  A fresh direct dump is needed for archival
	// verification.  E49 is a separate direct dump from the same source.
	ROM_REGION(0x8000, "e1raw", 0)
	ROM_LOAD("e1.bin", 0x0000, 0x8000, BAD_DUMP CRC(68b3cbde) SHA1(d51677c17e056c69a5864b1673f7887b24f50958))
	ROM_REGION(0x8000, "e2raw", 0)
	ROM_LOAD("e2.bin", 0x0000, 0x8000, BAD_DUMP CRC(19f788f0) SHA1(f4018501fee8c429bc1148014f2d9bb97e8cfdcf))
	ROM_REGION(0x8000, "e3raw", 0)
	ROM_LOAD("e3.bin", 0x0000, 0x8000, BAD_DUMP CRC(83919be1) SHA1(937b36e0475a8947a01d03c8e800c54a6461caf7))
	ROM_REGION(0x8000, "e4raw", 0)
	ROM_LOAD("e4.bin", 0x0000, 0x8000, BAD_DUMP CRC(d6eddc42) SHA1(df8097bcd04bd9fed5618d88cd0e0159f81c074b))
	ROM_REGION(0x8000, "e5raw", 0)
	ROM_LOAD("e5.bin", 0x0000, 0x8000, BAD_DUMP CRC(135c5c7c) SHA1(171d1582d6a62ec9fa8e42421693da6dac3b2a8a))

	ROM_REGION(0x8000, "cpu1low", ROMREGION_ERASEFF)
	ROM_REGION(0x8000, "cpu1high", ROMREGION_ERASEFF)
	ROM_REGION(0x8000, "cpu2low", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "cpu2banks", ROMREGION_ERASEFF)

	ROM_REGION(0x8000, "chargen", 0)
	ROM_LOAD("vt340e49.bin", 0x0000, 0x8000, CRC(38379339) SHA1(394e8511581abc796c8c612149eff280146b0ac8))

	// This factory/default EEPROM image is archived as 23-001N2.  It is known
	// to operate with the V1.1 cartridge but is not proven to come from the same
	// physical terminal as E1-E5 and E49.
	ROM_REGION(0x0200, "eeprom", ROMREGION_ERASEFF)
	ROM_LOAD("e80.bin", 0x0000, 0x0200, CRC(d1fd44bb) SHA1(ffbdeeffb55374211a371a77ed633b780cd0f526)) // Xicor X2804AP-45 (512 x 8 EEPROM)
ROM_END

} // anonymous namespace


COMP(1987, vt340, 0, 0, vt340, vt340, vt340_state, init_vt340, "Digital Equipment Corporation", "VT340 Video Terminal", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
