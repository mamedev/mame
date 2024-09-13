// license:BSD-3-Clause
// copyright-holders:Chris Hanson
/*
 * ms68k.cpp - Marion Systems MS68K
 *
 * Documentation:
 *   http://bitsavers.org/pdf/marionSystems/Marion_Systems_MS68K_Single_Board_Computer_Users_Manual.pdf
 *
 *  The Marion Systems MS68K was a single-board MC68000 system designed by Tom
 *  Oberheim in 1985 with two serial ports, a parallel port, WD1772 floppy,
 *  optional SCSI, and up to 512KB of RAM and ROM each onboard, plus an
 *  expansion connector providing full access to the 68000 bus. It was used in
 *  a number of university-level computer systems classes, and a variant of
 *  Peter Stark’s SK*DOS was also available.
 *
 *  The default ROM contains a port of Tiny BASIC 1.2 at 0xF04900, and a
 *  loader for it at 0xF04800. Thus while there is no direct command to start
 *  it, a simple GF04800 will start Tiny BASIC with it.
 *
 * Specifications:
 * - 8MHz MC68000 CPU
 * - 512KB maximum onboard RAM
 * - 128KB maximum onboard EPROM
 * - MC68681 DUART
 * - TTL-supported parallel port
 * - NCR53C80 SCSI controller
 * - Expansion bus
 *
 * TODO:
 * - Expansion bus
 *
 */

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/floppy.h"
#include "machine/mc68681.h"
#include "machine/ncr5380.h"
#include "machine/wd_fdc.h"


namespace {

class ms68k_state : public driver_device
{
public:
	ms68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bootvect(*this, "bootvect")
		, m_maincpu(*this, "maincpu")
		, m_duart(*this, "duart")
		, m_terminal(*this, "terminal")
		, m_modem(*this, "modem")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0)
		, m_floppy_selected_drive(0)
		, m_scsi(*this, "scsibus:7:ncr5380")
		, m_scsibus(*this, "scsibus")
		, m_printer_conn(*this, "prn")
		, m_printer_out(*this, "prn_out")
	{ }

	void ms68k(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(nmi_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	memory_view m_bootvect;

	required_device<cpu_device> m_maincpu;

	required_device<mc68681_device> m_duart;
	required_device<rs232_port_device> m_terminal;
	required_device<rs232_port_device> m_modem;

	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	uint8_t m_floppy_selected_drive;

	required_device<ncr5380_device> m_scsi;
	required_device<nscsi_bus_device> m_scsibus;

	required_device<centronics_device> m_printer_conn;
	required_device<output_latch_device> m_printer_out;

	void mem_map(address_map &map) ATTR_COLD;

	uint8_t duart_r(offs_t offset);
	void duart_w(offs_t offset, uint8_t data);

	// Floppy
	static void floppy_types(device_slot_interface &device) ATTR_COLD;
	static void floppy_formats(format_registration &fr) ATTR_COLD;
	void floppy_side_w(int state);
	void floppy_drive0_w(int state);
	void floppy_drive1_w(int state);
	void floppy_drive_select(int drive);

	// Printer
	uint8_t printer_r(offs_t offset);
	void printer_w(offs_t offset, uint8_t data);
};

/* Input ports */
static INPUT_PORTS_START( ms68k )
	PORT_START("ABORT")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("NMI button") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, ms68k_state, nmi_button, 0)
INPUT_PORTS_END


void ms68k_state::ms68k(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ms68k_state::mem_map);

	// Set up DUART.

	MC68681(config, m_duart, 16_MHz_XTAL / 2);
	// DUART Interrupt = 3
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_3);
	m_duart->a_tx_cb().set(m_terminal, FUNC(rs232_port_device::write_txd));
	// DUART OP0 = RTS A
	m_duart->outport_cb().append(m_terminal, FUNC(rs232_port_device::write_rts)).bit(0);
	m_duart->b_tx_cb().set(m_modem, FUNC(rs232_port_device::write_txd));
	// DUART OP1 = RTS B
	m_duart->outport_cb().append(m_modem, FUNC(rs232_port_device::write_rts)).bit(1);
	// DUART OP3 = Floppy Side 1 Select
	m_duart->outport_cb().append(FUNC(ms68k_state::floppy_side_w)).bit(3);
	// DUART OP4 = Floppy Drive Select 0
	m_duart->outport_cb().append(FUNC(ms68k_state::floppy_drive0_w)).bit(4);
	// DUART OP5 = Floppy Drive Select 1
	m_duart->outport_cb().append(FUNC(ms68k_state::floppy_drive1_w)).bit(5);
	// DUART OP6 = Floppy Double Density
	m_duart->outport_cb().append(m_fdc, FUNC(wd1772_device::dden_w)).bit(6);
	// DUART OP2 = Printer Initialize
	m_duart->outport_cb().append(m_printer_conn, FUNC(centronics_device::write_init)).bit(2);
	// DUART OP7 = Printer Data Strobe
	m_duart->outport_cb().append(m_printer_conn, FUNC(centronics_device::write_strobe)).bit(7);

	// Set up terminal RS-232.

	RS232_PORT(config, m_terminal, default_rs232_devices, "terminal");
	m_terminal->rxd_handler().set(m_duart, FUNC(mc68681_device::rx_a_w));
	// DUART IP0 = CTS
	m_terminal->cts_handler().set(m_duart, FUNC(mc68681_device::ip0_w));

	// Set up modem RS-232.

	RS232_PORT(config, m_modem, default_rs232_devices, nullptr);
	m_modem->rxd_handler().set(m_duart, FUNC(mc68681_device::rx_b_w));
	// DUART IP1 = CTS
	m_modem->cts_handler().set(m_duart, FUNC(mc68681_device::ip1_w));

	// Set up SCSI.

	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsibus:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsibus:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:7").option_set("ncr5380", NCR53C80)
		.machine_config([this](device_t *device) {
			// SCSI Interrupt = 5
			downcast<ncr53c80_device &>(*device).irq_handler().set_inputline(m_maincpu, M68K_IRQ_5);
		});

	// Set up FDC.

	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	// FDC Interrupt = 4
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, M68K_IRQ_3);
	FLOPPY_CONNECTOR(config, m_floppy[0], floppy_types, "525sd", floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], floppy_types, "525sd", floppy_formats);

	// Set up printer.

	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);
	// Printer Interrupt = 2
	m_printer_conn->ack_handler().append_inputline(m_maincpu, M68K_IRQ_2);
	// DUART IP2 = Busy
	m_printer_conn->busy_handler().set(m_duart, FUNC(mc68681_device::ip2_w));
	// DUART IP3 = Paper Out
	m_printer_conn->perror_handler().set(m_duart, FUNC(mc68681_device::ip3_w));
	// DUART IP4 = Device Selected
	m_printer_conn->select_handler().set(m_duart, FUNC(mc68681_device::ip4_w));
	// DUART IP5 = Acknowledge
	m_printer_conn->ack_handler().set(m_duart, FUNC(mc68681_device::ip5_w));

	// Set up Expansion.
	// EXT Interrupt = 6
	// TODO: Implement expansion bus.
}

void ms68k_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x07ffff).ram().share("ram"); // 512KB RAM onboard maximum
	map(0x000008, 0x07ffff).ram();
	map(0xd00000, 0xd7ffff).rw(m_scsi, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)).umask16(0x00ff);
	map(0xd80000, 0xdfffff).rw(FUNC(ms68k_state::printer_r), FUNC(ms68k_state::printer_w)).umask16(0x00ff);
	map(0xe00000, 0xe7ffff).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0xe80000, 0xefffff).r(m_duart, FUNC(mc68681_device::read)).w(FUNC(ms68k_state::duart_w)).umask16(0x00ff);
	map(0xf00000, 0xf1ffff).rom().region("roms", 0); // 128KB ROM onboard maximum

	// ROM is mapped to 0 on reset, remapped on first DUART write
	map(0x000000, 0x000007).view(m_bootvect);
	m_bootvect[0](0x000000, 0x000007).rom().region("roms", 0);
}

void ms68k_state::machine_start()
{
	save_item(NAME(m_floppy_selected_drive));
}

void ms68k_state::machine_reset()
{
	// Reset pointer to bootvector in ROM for bootvector view
	m_bootvect.select(0);
}

/* DUART */

void ms68k_state::duart_w(offs_t offset, uint8_t data)
{
	m_duart->write(offset, data);

	// The first write to the DUART also swaps ROM and RAM.
	if (m_bootvect.entry().has_value()) {
		m_bootvect.disable(); // stop mapping ROM until reset
	}
}

/* Floppy */

void ms68k_state::floppy_types(device_slot_interface &device)
{
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}

void ms68k_state::floppy_formats(format_registration &fr)
{
	floppy_image_device::default_mfm_floppy_formats(fr);
}

void ms68k_state::floppy_side_w(int state)
{
	floppy_image_device *floppy = nullptr;
	floppy = m_floppy[m_floppy_selected_drive]->get_device();
	if (floppy) {
		floppy->ss_w(state);
	}
}

void ms68k_state::floppy_drive0_w(int state)
{
	if (state)
		floppy_drive_select(0);
}

void ms68k_state::floppy_drive1_w(int state)
{
	if (state)
		floppy_drive_select(1);
}

void ms68k_state::floppy_drive_select(int drive)
{
	m_floppy_selected_drive = drive;
	floppy_image_device *floppy = nullptr;
	floppy = m_floppy[m_floppy_selected_drive]->get_device();
	m_fdc->set_floppy(floppy);
}


/* Printer */
uint8_t ms68k_state::printer_r(offs_t offset)
{
	return 0;
}

void ms68k_state::printer_w(offs_t offset, uint8_t data)
{
	m_printer_out->write(data);
}


/* NMI button */
INPUT_CHANGED_MEMBER(ms68k_state::nmi_button)
{
	m_maincpu->set_input_line(M68K_IRQ_7, newval ? ASSERT_LINE : CLEAR_LINE);
}


/* ROM definition */
ROM_START( ms68k )
	ROM_REGION16_BE(0x20000, "roms", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("msmon")

	ROM_SYSTEM_BIOS(0, "msmon", "MSMON")
	ROMX_LOAD("msmonu27.bin", 0x000000, 0x010000, CRC(b278ea2a) SHA1(383911c213448d41198f2b44494376c1dc26e897), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD("msmonu26.bin", 0x000001, 0x010000, CRC(7ff029cf) SHA1(4338ef716d455fd86e1c32c04e146d2be1603dff), ROM_SKIP(1) | ROM_BIOS(0) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME   PARENT COMPAT MACHINE INPUT  CLASS        INIT        COMPANY           FULLNAME FLAGS */
COMP( 1981, ms68k, 0,     0,     ms68k,  ms68k, ms68k_state, empty_init, "Marion Systems", "MS68K", MACHINE_NO_SOUND_HW )
