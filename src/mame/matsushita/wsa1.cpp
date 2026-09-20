// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-WSA1R

    A 1995 rack-mounted "Acoustic Modeling Synthesis" synthesizer module:
    64 notes over up to 32 parts, 256 preset sounds, 16 preset drum kits and
    128 preset combinations, a 320 x 240 dot LCD, two sets of MIDI IN/OUT/THRU
    and a built-in 3.5 inch floppy drive.

    Two TLCS-900/H processors share the work.  CPU 1 drives the panel, the
    floppy and MIDI; CPU 2 drives the tone generator and the modeling LSI.

    Hardware inventory below is taken from the SX-WSA1R service manual,
    ORDER NO. EMiD951604, (c) 1995 Matsushita Electric Industrial, which covers
    the rack module only.  The scan available here is photocopy grade: where
    the schematic sheets and the parts list give different spellings both are
    shown, and a designator that was not legible is marked (derived) or
    (unknown).

    A keyboard version of this instrument exists, the SX-WSA1, and it is not
    declared here: the only claim that it runs the same ROM set is the
    redistributor's, and no SX-WSA1 material was available to check it against.

    The four images below are not chip reads.  They are the firmware set that
    has been publicly redistributed for this machine; the copy used here came
    from dbwbp.com in August 2026, and its uploader states it was read from a
    rack SX-WSA1R.  They carry no dump-quality flag because nothing suggests
    the bytes are wrong -- what is second-hand is the provenance, not the
    integrity.  The set is self-consistent: three of the four images end with
    their own build tag ("wsaa_822", "wsac_230", "wsad_54"), matching the AX,
    CX and DX factory part numbers in the manual, and the firmware's ROM
    VERSION screen has exactly three slots, WSA-A/WSA-C/WSA-D, with no WSA-B
    line for the fourth image.

    TODO:
      - dump the six 16 Mbit wave mask ROMs, the AM29F400T flash, and the
        internal ROM of the control panel microcontroller
      - devices with no MAME implementation yet: the L7A1429 modeling LSI, the
        uPD6383GF-3BA DSP
      - map the flash at 0xE80000 on CPU 2.  The firmware probes it with the
        AMD autoselect sequence (0xAAAA/0x5554 unlock, 0x90, then reads
        0xE80000 and 0xE80002) at prom_c 0xFC85BD, so the part is almost
        certainly the AM29F400T the parts list names.  MAME has AMD_29F400T
        but only as an 8-bit device, and the accepted-device table the
        firmware compares against has not been decoded yet.

***************************************************************************/

#include "emu.h"

#include "wsa1r_cpanel.h"

#include "bus/midi/midi.h"
#include "cpu/tlcs900/tmp95c061.h"
#include "imagedev/floppy.h"
#include "machine/eepromser.h"
#include "machine/upd765.h"
#include "video/sed1330.h"

#include "diserial.h"
#include "emupal.h"
#include "screen.h"


// A byte/bit shim between the TMP95C061's serial channel 0 and MAME's
// bit-serial MIDI ports: 31250 baud, 8N1, with a small transmit ring so a
// burst from the firmware is not lost between stop bits.
class wsa1_midi_uart_device : public device_t, public device_serial_interface
{
public:
	wsa1_midi_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto rx_cb() { return m_rx_cb.bind(); }   // each received MIDI byte
	auto tx_cb() { return m_tx_cb.bind(); }   // each transmitted bit

	void tx_byte(uint8_t data);
	void rx_line_w(int state) { rx_w(state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override
	{
		receive_register_extract();
		m_rx_cb(get_received_char());
	}
	virtual void tra_callback() override { m_tx_cb(transmit_register_get_data_bit()); }
	virtual void tra_complete() override { start_next_tx(); }

private:
	void start_next_tx();

	static constexpr unsigned TX_FIFO = 256;   // power of two

	devcb_write8      m_rx_cb;
	devcb_write_line  m_tx_cb;
	uint8_t           m_tx_ring[TX_FIFO];
	unsigned          m_tx_head = 0;
	unsigned          m_tx_tail = 0;
	bool              m_tx_busy = false;
};

DEFINE_DEVICE_TYPE(WSA1_MIDI_UART, wsa1_midi_uart_device, "wsa1_midi_uart", "SX-WSA1R MIDI UART")

wsa1_midi_uart_device::wsa1_midi_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WSA1_MIDI_UART, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_rx_cb(*this),
	m_tx_cb(*this)
{
}

void wsa1_midi_uart_device::device_start()
{
	save_item(NAME(m_tx_ring));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_tail));
	save_item(NAME(m_tx_busy));
}

void wsa1_midi_uart_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(31250);
	transmit_register_reset();
	receive_register_reset();
	m_tx_head = m_tx_tail = 0;
	m_tx_busy = false;
	m_tx_cb(1);                                // TXD idles high
}

void wsa1_midi_uart_device::tx_byte(uint8_t data)
{
	unsigned const next = (m_tx_head + 1) & (TX_FIFO - 1);
	if (next != m_tx_tail)                     // drop on overflow rather than corrupt
	{
		m_tx_ring[m_tx_head] = data;
		m_tx_head = next;
	}
	if (!m_tx_busy)
		start_next_tx();
}

void wsa1_midi_uart_device::start_next_tx()
{
	if (m_tx_head == m_tx_tail)
	{
		m_tx_busy = false;
		return;
	}
	m_tx_busy = true;
	uint8_t const b = m_tx_ring[m_tx_tail];
	m_tx_tail = (m_tx_tail + 1) & (TX_FIFO - 1);
	transmit_register_setup(b);
}


namespace {
class wsa1_state : public driver_device
{
public:
	wsa1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu1(*this, "cpu1")
		, m_cpu2(*this, "cpu2")
		, m_lcdc(*this, "lcdc")
		, m_fdc(*this, "fdc")
		, m_eeprom(*this, "eeprom")
		, m_cpanel(*this, "cpanel")
		, m_midi_uart(*this, "midi_uart")
	{ }

	void wsa1r(machine_config &config);

private:
	required_device<tmp95c061_device> m_cpu1;
	required_device<tmp95c061_device> m_cpu2;
	required_device<sed1330_device> m_lcdc;
	required_device<upd765a_device> m_fdc;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<wsa1r_cpanel_device> m_cpanel;
	required_device<wsa1_midi_uart_device> m_midi_uart;

	void midi1_rx(uint8_t data) { m_cpu1->sc0_rxd(data); }

	uint8_t m_cpu1_p8 = 0;
	uint8_t m_cpu1_pb = 0;
	int m_panel_sclk = 1;   // P8.5, idle high
	int m_panel_busy = 0;   // PB.4, idle low

	uint8_t cpu1_p8_r();
	void cpu1_p8_w(uint8_t data);
	uint8_t cpu1_pb_r();
	void cpu1_pb_w(uint8_t data);

	void cpu2_p6_w(uint8_t data);
	uint8_t cpu2_p8_r();
	void cpu2_p8_w(uint8_t data);

	void palette_init(palette_device &palette) ATTR_COLD;

	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
	void lcdc_map(address_map &map) ATTR_COLD;
};


// CPU 1's P8 and PB carry the panel's serial clock and busy lines alongside
// the floppy's terminal count.  PB.0 low identifies the rack model.
uint8_t wsa1_state::cpu1_p8_r()
{
	uint8_t data = (m_cpu1_p8 & 0x09) | 0xd6;
	if (m_panel_sclk)
		data |= 0x20;
	return data;
}

void wsa1_state::cpu1_p8_w(uint8_t data)
{
	m_cpu1_p8 = data;
}

uint8_t wsa1_state::cpu1_pb_r()
{
	uint8_t data = (m_cpu1_pb & 0x0c) | 0xe3;
	data &= ~0x01;                  // PB.0 low: the rack
	if (m_panel_busy)
		data |= 0x10;
	return data;
}

void wsa1_state::cpu1_pb_w(uint8_t data)
{
	m_cpu1_pb = data;
	m_fdc->tc_w(BIT(data, 3));
}


// The calibration EEPROM is bit-banged from CPU 2: P6.5 is chip select, P8.4
// data in, P8.3 clock, P8.5 data out.  64 x 16 with a 6-bit address, so a
// 93C46 class part; the identification is from the protocol at prom_c
// EepromBitbangProtocol, not from a part number, and the device itself is not dumped.
void wsa1_state::cpu2_p6_w(uint8_t data)
{
	m_eeprom->cs_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t wsa1_state::cpu2_p8_r()
{
	return m_eeprom->do_read() << 5;
}

void wsa1_state::cpu2_p8_w(uint8_t data)
{
	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->clk_write(BIT(data, 3));
}


void wsa1_state::palette_init(palette_device &palette)
{
	// A driver choice, not a measurement: the pen pair ympsr2000.cpp uses for
	// its own SED1330 panel of the same geometry.  The real module's appearance
	// is not established.
	palette.set_pen_color(0, rgb_t(0x36, 0x41, 0xcf));
	palette.set_pen_color(1, rgb_t(0xdb, 0xe9, 0xff));
}


void wsa1_state::lcdc_map(address_map &map)
{
	// 32 KiB of display RAM: the power-on clear at LcdRamPowerOnClear writes 0x800 x 16
	// bytes, and the highest address any layer reaches is SAD3 + 240 * AP.
	map(0x0000, 0x7fff).ram();
}


// CPU 1 fetches prom_a and prom_b; CPU 2 fetches prom_c.  Which of them is IC1
// "MICROCOMPUTER (MAIN)" and which is IC2 "(SUB)" is not established, so the
// tags are neutral.
void wsa1_state::cpu1_map(address_map &map)
{
	// static RAM on CS1 (MSAR1 = 0x00 at InitMSAR1_CS1).  Boot clears from 0x000080
	// and the checksum furniture at 0x007FCA-0x007FD4 sizes the chip at 32 KiB.
	map(0x000080, 0x007fff).ram();

	// work DRAM on CS3 (MSAR3 = 0x60 at InitMSAR3_CS3; P6FC = 0x1F at InitP6FC_LCAS makes
	// the CS3 pin LCAS).  First stack is 0x60EB80.
	map(0x600000, 0x67ffff).ram();

	map(0x790000, 0x790000).rw(m_lcdc, FUNC(sed1330_device::status_r), FUNC(sed1330_device::data_w));
	map(0x790001, 0x790001).rw(m_lcdc, FUNC(sed1330_device::data_r),   FUNC(sed1330_device::command_w));

	// FDC data port, reached by programmed I/O and by micro-DMA channel 0.
	map(0x7a0000, 0x7a0000).rw(m_fdc, FUNC(upd765a_device::dma_r),
	                                  FUNC(upd765a_device::dma_w));

	// Main status / data rate select at +4, data at +5.  Read off the callers:
	// 0x7B0004 is only ever tested for RQM and DIO, never stored, while every
	// read of 0x7B0005 goes straight into the result buffer.
	map(0x7b0004, 0x7b0004).r(m_fdc, FUNC(upd765a_device::msr_r))
	                       .w(m_fdc, FUNC(upd765a_device::dsr_w));
	map(0x7b0005, 0x7b0005).rw(m_fdc, FUNC(upd765a_device::fifo_r),
	                                  FUNC(upd765a_device::fifo_w));

	map(0xf00000, 0xf7ffff).rom().region("prom_ab", 0x000000);   // IC13
	map(0xf80000, 0xffffff).rom().region("prom_ab", 0x080000);   // IC12
}

void wsa1_state::cpu2_map(address_map &map)
{
	map(0x000080, 0x01ffff).ram();
	map(0xf00000, 0xf7ffff).rom().region("prom_d", 0);           // IC21, tone database
	map(0xf80000, 0xffffff).rom().region("prom_c", 0);           // IC28
}


static void wsa1_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}


static INPUT_PORTS_START(wsa1r)
INPUT_PORTS_END


void wsa1_state::wsa1r(machine_config &config)
{
	// fc = 28 MHz: the firmware stores it as a byte, prom_c[FcClockByte] = 0x1C
	// read at SerialDivisorFromFc, and computes its own serial divisor from it.
	TMP95C061(config, m_cpu1, 28_MHz_XTAL);
	m_cpu1->set_addrmap(AS_PROGRAM, &wsa1_state::cpu1_map);
	m_cpu1->port8_read().set(FUNC(wsa1_state::cpu1_p8_r));
	m_cpu1->port8_write().set(FUNC(wsa1_state::cpu1_p8_w));
	m_cpu1->portb_read().set(FUNC(wsa1_state::cpu1_pb_r));
	m_cpu1->portb_write().set(FUNC(wsa1_state::cpu1_pb_w));
	m_cpu1->sc1_txd().set(m_cpanel, FUNC(wsa1r_cpanel_device::tx_byte));
	m_cpu1->sc1_mod().set([this] (uint8_t data) { m_cpanel->rx_enable(BIT(data, 5)); });

	TMP95C061(config, m_cpu2, 28_MHz_XTAL);
	m_cpu2->set_addrmap(AS_PROGRAM, &wsa1_state::cpu2_map);
	m_cpu2->port6_write().set(FUNC(wsa1_state::cpu2_p6_w));
	m_cpu2->port8_read().set(FUNC(wsa1_state::cpu2_p8_r));
	m_cpu2->port8_write().set(FUNC(wsa1_state::cpu2_p8_w));

	auto &palette = PALETTE(config, "palette", FUNC(wsa1_state::palette_init), 2);

	screen_device &screen = SCREEN(config, "screen").set_lcd();
	screen.set_refresh_hz(60);
	screen.set_screen_update(m_lcdc, FUNC(sed1330_device::screen_update));
	screen.set_size(320, 240);
	screen.set_visarea_full();
	screen.set_palette(palette);

	SED1330(config, m_lcdc, 8'000'000);   // IC7's X2
	m_lcdc->set_screen("screen");
	m_lcdc->set_addrmap(0, &wsa1_state::lcdc_map);

	// The parts list names a uPD72070; upd765a stands in for it.  Interrupt
	// lines are the schematic's: IC1 pin 37 INT5 = FDINT, pin 41 INT7 = FDDRQ.
	UPD765A(config, m_fdc, 24'000'000, true, true);   // IC8's X5
	m_fdc->intrq_wr_callback().set_inputline(m_cpu1, TLCS900_INT5);
	m_fdc->drq_wr_callback().set_inputline(m_cpu1, TLCS900_INT7);

	// PC formats: the geometries the firmware programmes are the IBM ones down
	// to the gap lengths.  Untested against real media -- no disk is dumped.
	FLOPPY_CONNECTOR(config, "fdc:0", wsa1_floppies, "35hd",
		floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	EEPROM_93C46_16BIT(config, m_eeprom);

	WSA1R_CPANEL(config, m_cpanel);
	m_cpanel->atn().set([this] (int state) {
			m_cpu1->set_input_line(TLCS900_INT6, state ? ASSERT_LINE : CLEAR_LINE); });
	m_cpanel->busy().set([this] (int state) { m_panel_busy = state; });
	m_cpanel->sclk().set([this] (int state) { m_panel_sclk = state; });
	m_cpanel->rxd().set([this] (uint8_t data) { m_cpu1->sc1_rxd(data); });

	// The rear MIDI1 jack, on CPU 1's serial channel 0.
	WSA1_MIDI_UART(config, m_midi_uart, 0);
	m_midi_uart->rx_cb().set(FUNC(wsa1_state::midi1_rx));
	m_cpu1->sc0_txd().set(m_midi_uart, FUNC(wsa1_midi_uart_device::tx_byte));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(
			m_midi_uart, FUNC(wsa1_midi_uart_device::rx_line_w));
	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_midi_uart->tx_cb().set("mdout", FUNC(midi_port_device::write_txd));
}


/***************************************************************************

    Hardware inventory, from the SX-WSA1R service manual, ORDER NO. EMiD951604,
    (c) 1995 Matsushita Electric Industrial, which covers the rack module only.
    The scan is photocopy grade: where the schematic and the parts list give
    different spellings both are shown, and a designator that was not legible
    is marked (derived) or (unknown).

    MAIN board

    IC1, IC2     TMP95C061AF    Toshiba TLCS-900/H, "MICROCOMPUTER (MAIN)" and
                                "(SUB)" respectively
    IC3          L7A1429        "MODELING LSI"
    IC4          TC183C230002   "TONE GENELATOR LSI" [sic] (schematic:
                                TC1830230002; designator derived)
    IC5, IC6,    D6383GF-3BA    NEC digital signal processor, three of them
    IC30                        (only IC30 is printed cleanly)
    IC7          SED1330FBA     LCD controller for the 320 x 240 panel;
                                src/devices/video/sed1330.h
    IC12         QSIGCWSA1AX    4 Mbit programmed EPROM, chip select PROMACS
    IC13         QSIGCWSA1BX    4 Mbit programmed EPROM, chip select PROMBCS
    IC28         QSIGCWSA1CX    4 Mbit programmed EPROM, chip select PROMCCS
    (derived)    QSIGCWSA1DX    4 Mbit programmed EPROM, chip select PROMDCS.
                                Parts-list row order puts it at IC21, and the
                                redistributed set names the file that way, but
                                the scan does not confirm it.
    (derived)    AM29F400T      4 Mbit flash memory; row order puts it at IC22
    IC14, IC15   M5256CFP70LL,  256 kbit static RAM and 4 Mbit dynamic RAM; the
                 M5M44170AJ7S   self-diagnostic calls the pair "RAM (IC14, 15)"
                                (parts list: M5M44170AN7S)
    IC23, IC31,  LC321664AJ80   1 Mbit dynamic RAM
    IC32, IC51,
    IC61
    IC27         D74HC139GS     decoder; generates PROMCCS and PROMDCS
    IC33, IC34   M5M44260AJ7S   4 Mbit dynamic RAM (parts list: M5M44260AJN7S)
    IC43         QSIGH3C16DT8   16 Mbit wave mask ROM (schematic: ...DT3)
    IC44         QSIGH3C16EA0   16 Mbit wave mask ROM (schematic: ...EA9)
    IC45         QSIGH3C16EA2   16 Mbit wave mask ROM (parts list: QSIGH38C...)
    IC47         QSIGH3C16DT7   16 Mbit wave mask ROM
    IC48         QSIGH3C16DT9   16 Mbit wave mask ROM
    IC49         QSIGH3C16EA1   16 Mbit wave mask ROM
    IC52 - IC54, PCM1702U       D/A converter, four of them
    IC59
    IC55 - IC58  M5218AFP       operational amplifier
    IC71         LH5P832N-10    256 kbit RAM (schematic: pseudo static)
    (unknown)    D72070GF3BE    NEC floppy disk controller, 3.5 inch 2HD
                                1.44 MB / 2DD 720 KB

    CONTROL PANEL 1 board

    (unknown)    M37471M2196S   Mitsubishi panel microcontroller, the same part
                                as the two MCUs in kn5000_cpanel.cpp.  Its mask
                                ROM is undumped and no region is declared for
                                it, because the manual does not give a capacity.
    (unknown)    HD74LS07P      hex buffer

    Capacities are the manual's own.  It never says what any of the four EPROMs
    holds; the roles in the ROM definitions below were read out of the images.
    Only devices with a legible part number are listed; discrete logic, the
    power supply and the SY-EW1 / SY-ES1 option boards are omitted.

***************************************************************************/

ROM_START(wsa1r)
	// The OS v2.0 set.  A v1 OS shipped and is not dumped; whether it used
	// different factory part numbers is not known, so the revision is carried
	// in the file names rather than left to the part number.
	// Regions are named after the chip-select nets, not after processors:
	// which of IC1 ("MAIN") and IC2 ("SUB") fetches which pair is not
	// established.

	// A and B share one address space.  B at PromB_JumpTable opens with a five entry
	// jump table and holds the user interface text (English, German, French)
	// and the service test screens.  A at PromC_Base puts its vector table at
	// PromC_Vectors, where a TMP95C061 fetches vectors; reset is PromA_Reset.
	ROM_REGION16_LE(0x100000, "prom_ab", 0)
	ROM_LOAD("qsigcwsa1bx_v2.ic13", 0x000000, 0x080000, CRC(f3f84441) SHA1(93adec2a04b7d93a2ec2bfb059227ff3959906e0)) // B, at 0xf00000
	ROM_LOAD("qsigcwsa1ax_v2.ic12", 0x080000, 0x080000, CRC(5f34af46) SHA1(90a2369f8e4d2fcdf26875272267624b07bc200d)) // A, at 0xf80000

	// C is the other processor's program, at PromC_Base in its own space, with
	// an independent vector table at PromC_Vectors and reset PromC_Reset.  It also
	// holds data: 0x000000-0x0165BF is a "ZZZZ" headed block with a 16 entry
	// category table and 128 combinations of 704 bytes; code runs 0x018000 to
	// 0x0621E4; boot code and vectors from 0x07F000.
	ROM_REGION16_LE(0x080000, "prom_c", 0)
	ROM_LOAD("qsigcwsa1cx_v2.ic28", 0x000000, 0x080000, CRC(855c8ac4) SHA1(9b2911e4b21a08d9744b91844630489f54dde856)) // at 0xf80000

	// D holds no executable content.  It is a tone bank: 32-bit image-relative
	// offsets at 0x000000, a 274 entry pointer directory at 0x000B80 (256
	// sounds then 18 drum kit records, each headed by a 16 byte printable
	// name), payload ending at 0x050B08.  A region of its own because it is a
	// separate chip on a separate chip select, IC27's 2Y2 = PROMDCS.  Sheet
	// II-11/II-12 draws IC21 QSIGCWSA1DX beside that decoder.
	ROM_REGION16_LE(0x080000, "prom_d", 0)
	ROM_LOAD("qsigcwsa1dx_v2.ic21", 0x000000, 0x080000, CRC(735ae465) SHA1(82df50816c20cd8f2d29551326d2633e7791f306))

	// Wave ROMs, undumped.  The self-diagnostic covers IC43-IC45 and IC47-IC49;
	// there is no IC46.  The manual gives 16 Mbit but not the organisation, and
	// which device sits on which tone-generator bus is not resolved, so each
	// gets its own region rather than being concatenated into a bank.
	ROM_REGION(0x200000, "waveform_ic43", 0)
	ROM_LOAD("qsigh3c16dt8.ic43", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic44", 0)
	ROM_LOAD("qsigh3c16ea0.ic44", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic45", 0)
	ROM_LOAD("qsigh3c16ea2.ic45", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic47", 0)
	ROM_LOAD("qsigh3c16dt7.ic47", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic48", 0)
	ROM_LOAD("qsigh3c16dt9.ic48", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic49", 0)
	ROM_LOAD("qsigh3c16ea1.ic49", 0x000000, 0x200000, NO_DUMP)

	// A 4 Mbit flash device that the block diagram puts on the same address
	// group as the program EPROMs.  Not held here, and the manual does not say
	// what it holds.  Named after the part number because its designator is
	// not legible either.
	ROM_REGION(0x080000, "flash", 0)
	ROM_LOAD("am29f400t.bin", 0x000000, 0x080000, NO_DUMP)
ROM_END

} // anonymous namespace


//   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY     FULLNAME    FLAGS
SYST(1995, wsa1r, 0,      0,      wsa1r,   wsa1r, wsa1_state, empty_init, "Technics", "SX-WSA1R", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
