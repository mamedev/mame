// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for E-mu Emulator Three (EIII) synthesizer.


    WIP
     - scsi scan fails, selection code seems buggy
     - debug port serial parameters aren't correct

***********************************************************************************************************************************/

#include "emu.h"

#include "cpu/ns32000/ns32000.h"

// various hardware
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "machine/z80scc.h"
#include "machine/ncr5380.h"
#include "machine/6850acia.h"
#include "machine/clock.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/terminal.h"

#include "imagedev/floppy.h"

// video
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"


namespace {

class emu3_state : public driver_device
{
public:
	emu3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdc:0:35dd")
		, m_hdc(*this, "scsi:0:ncr5380")
		, m_pit(*this, "pit")
		, m_scc(*this, "scc")
		, m_ddt(*this, "ddt")
		, m_ddt_port(*this, "ddt_port")
		, m_led(*this, "led%u", 0U)
	{
	}

	void emu3(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(nmi_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void emu3_map(address_map &map) ATTR_COLD;

	void palette_init(palette_device &palette);

	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<wd1772_device> m_fdc;
	required_device<floppy_image_device> m_fdd;
	required_device<ncr5380_device> m_hdc;
	required_device<pit8254_device> m_pit;
	required_device<z80scc_device> m_scc;
	optional_device<acia6850_device> m_ddt;
	optional_device<rs232_port_device> m_ddt_port;

	output_finder<17> m_led;

	enum irq_number : unsigned
	{
		SCNINT = 0, // scanner interrupt (active low)
		SCCINT = 1, // serial controller interrupt (active low)
		FDCINT = 2, // flopper disk controller interrupt (active high)
		STINT  = 3, // software timer interrupt (active low)
		TGINT  = 4, // transient generator interrupt (active high)
		HDINT  = 5, // hard disk interrupt (active high)
					// unused (tied high)
					// unused (tied low)
	};

	enum misc_latch : u8
	{
		MISC_MET     = 0, // metronome?
		MISC_MIDIOF  = 1, // MIDI/RS422 select (+MIDIOF)
		MISC_SCNDATA = 2, // scanner cpu serial data (+SCNDATA)
		MISC_SCNCLK  = 3, // scanner cpu serial clock (+SCNCLK)
		MISC_SIDE    = 4, // floppy disk side select (-SIDE)
		MISC_MTR     = 5, // floppy disk motor control (-MTR)
		MISC_SYNCON  = 6, // ext. sync on (+SYNCON)
		MISC_LED     = 7, // front panel Erase LED (CN6 60)
	};

	template <irq_number IRQ> void irq_w(int state)
	{
		if (state)
			m_irq_latch |= 1U << IRQ;
		else
			m_irq_latch &= ~(1U << IRQ);

		bool const irq_state = (m_irq_latch & 0x34) || (~m_irq_latch & 0x0b);
		if (irq_state != m_irq_state)
		{
			m_irq_state = irq_state;
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_irq_state);
		}
	}

	u8 m_irq_latch = 0;
	bool m_irq_state = false;
};

void emu3_state::machine_start()
{
	m_led.resolve();

	m_irq_latch = 0x4b;
	m_irq_state = false;
}

void emu3_state::machine_reset()
{
	m_fdc->set_floppy(m_fdd);
	m_fdc->dden_w(0);
}

void emu3_state::emu3_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("bootprom", 0);
	map(0x008000, 0x027fff).ram();
	map(0x2c0000, 0x2c0000).rw(m_hdc, FUNC(ncr5380_device::dma_r), FUNC(ncr5380_device::dma_w));
	map(0x300000, 0x30000f).rw(m_hdc, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)).umask16(0x00ff);
	map(0x390000, 0x390007).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	map(0x400000, 0xbfffff).ram();

	map(0xc00000, 0xc00000).lw8([this](u8 data) { irq_w<STINT>(1); }, "stint_w");
	map(0xc40000, 0xc40000).lw8([this](u8 data) { irq_w<TGINT>(0); }, "tgint_w");
	map(0xd70000, 0xd70000).lw8(
		[this](u8 data)
		{
			m_fdd->ss_w(!BIT(data, MISC_SIDE));
			m_fdd->mon_w(BIT(data, MISC_MTR));
			m_led[16] = BIT(data, MISC_LED);
		}, "misc_w");
	map(0xdb0000, 0xdb0001).lw16(
		[this](u16 data)
		{
			for (unsigned i = 0; i < 16; i++)
				m_led[i] = BIT(data, i);
		}, "led_w");

	map(0xde0060, 0xde007f).nopw(); // temporarily mute logging

	map(0xeb0000, 0xeb0000).w(m_lcdc, FUNC(hd44780_device::control_w));
	map(0xeb0002, 0xeb0002).r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0xeb0004, 0xeb0004).w(m_lcdc, FUNC(hd44780_device::data_w));
	map(0xeb0006, 0xeb0006).r(m_lcdc, FUNC(hd44780_device::data_r));

	map(0xef0001, 0xef0001).w(m_ddt, FUNC(acia6850_device::control_w));
	map(0xef0003, 0xef0003).r(m_ddt, FUNC(acia6850_device::status_r));
	map(0xef0005, 0xef0005).w(m_ddt, FUNC(acia6850_device::data_w));
	map(0xef0007, 0xef0007).r(m_ddt, FUNC(acia6850_device::data_r));

	map(0xf50000, 0xf50007).w(m_fdc, FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0xf50008, 0xf5000f).r(m_fdc, FUNC(wd1772_device::read)).umask16(0x00ff);

	map(0xfffe00, 0xfffe00).lr8([this]() { return m_irq_latch; }, "irq_latch_r");
}

void emu3_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::white());
	palette.set_pen_color(1, rgb_t::black());
}

static void emu_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
}

static void emu3_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static void emu3_rs232(device_slot_interface &device)
{
	device.option_add("terminal", SERIAL_TERMINAL);
}

void emu3_state::emu3(machine_config &config)
{
	NS32016(config, m_maincpu, 20_MHz_XTAL / 2); // 32016-10 CPU + 32001-10 TCU
	m_maincpu->set_addrmap(AS_PROGRAM, &emu3_state::emu3_map);

	//NS32081(config, "fpu", 20_MHz_XTAL / 2);

	//R6500_11(config, "scannercpu", 16_MHz_XTAL / 4);

	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	m_fdc->intrq_wr_callback().set(*this, FUNC(emu3_state::irq_w<FDCINT>));
	m_fdc->set_disable_motor_control(true);

	FLOPPY_CONNECTOR(config, "fdc:0", emu3_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	PIT8254(config, m_pit); // 8254-2
	m_pit->set_clk<0>(20_MHz_XTAL / 2);
	m_pit->set_clk<1>(16_MHz_XTAL / 16);
	m_pit->out_handler<0>().set(*this, FUNC(emu3_state::irq_w<TGINT>));
	//pit.out_handler<1>().set(); // -SWTIME
	//pit.out_handler<2>().set(); // -SMPTIME

	// scsi bus and devices
	NSCSI_BUS(config, "scsi");

	// scsi host adapter
	NSCSI_CONNECTOR(config, "scsi:0").option_set("ncr5380", NCR5380).machine_config(
		[this](device_t *device)
		{
			ncr5380_device &adapter = downcast<ncr5380_device &>(*device);

			adapter.irq_handler().set(*this, FUNC(emu3_state::irq_w<HDINT>));
		});

	NSCSI_CONNECTOR(config, "scsi:1", emu_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:2", emu_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", emu_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", emu_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", emu_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", emu_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", emu_scsi_devices, nullptr);

	SCC85230(config, m_scc, 16_MHz_XTAL / 4);
	m_scc->out_int_callback().set(*this, FUNC(emu3_state::irq_w<SCCINT>)).invert();

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(4, 20);

	PALETTE(config, "palette", FUNC(emu3_state::palette_init), 2);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60); // no idea
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(20 * 6, 4 * 9); // 20x4 columns/rows, 5x8 character cells
	screen.set_visarea_full();
	screen.set_palette("palette");

	// debug board
	ACIA6850(config, m_ddt);
	clock_device &ddt_clock(CLOCK(config, "ddt_clock", 10_MHz_XTAL / 64));
	ddt_clock.signal_handler().set(m_ddt, FUNC(acia6850_device::write_txc));
	ddt_clock.signal_handler().append(m_ddt, FUNC(acia6850_device::write_rxc));

	RS232_PORT(config, m_ddt_port, emu3_rs232, nullptr);
	m_ddt_port->rxd_handler().set(m_ddt, FUNC(acia6850_device::write_rxd)).invert();
	m_ddt->rts_handler().set(m_ddt_port, FUNC(rs232_port_device::write_rts)).invert();
	m_ddt->txd_handler().set(m_ddt_port, FUNC(rs232_port_device::write_txd)).invert();
}

INPUT_CHANGED_MEMBER(emu3_state::nmi_button)
{
	if (newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

static INPUT_PORTS_START(emu3)
	PORT_START("ddt")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, emu3_state, nmi_button, 0)
INPUT_PORTS_END

ROM_START(emu3)
	ROM_REGION16_LE(0x8000, "bootprom", 0)
	ROM_LOAD16_BYTE("e3-lsboot_ip381a_emu_systems_4088.ic3", 0x0000, 0x4000, CRC(34e5283f) SHA1(902c2a9a2b37b34331fb57d45b88ffabc1f12a53)) // 27128B
	ROM_LOAD16_BYTE("e3-msboot_ip380a_emu_systems_4088.ic4", 0x0001, 0x4000, CRC(1302c054) SHA1(28b7e8991e72cc111ee0067c58bddafca70f3824)) // 27128B

	ROM_REGION(0xc00, "scannercpu", 0)
	ROM_LOAD("im368.ic31", 0x000, 0xc00, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1987, emu3, 0, 0, emu3, emu3, emu3_state, empty_init, "E-mu Systems", "Emulator Three Digital Sound Production System", MACHINE_IS_SKELETON)
