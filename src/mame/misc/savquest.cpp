// license:BSD-3-Clause
// copyright-holders:R. Belmont, Peter Ferrie, Matt Munroe
/***************************************************************************

    savquest.cpp

    "Savage Quest" (c) 1999 Interactive Light, developed by Angel Studios.
    Skeleton by R. Belmont

    TODO:
    - Needs proper AWE64 emulation defined as a slot option default, with
      fallbacks to AWE32 and SB16;
    \- to workaround: attrib -R C:\IMMERSIA\BIN\IMMOS.BAT then edit file to swap sound card

    - When switching gfx mode during boot routine it still sets a terminal debug mode (with cut down screen portions)
      instead of normal Voodoo drawing. Culprit may be an I/O port reading or a Voodoo bug;

    - Aforementioned debug mode shows that it can't find several assets on loading;

    - When game boots it does extensive CPUID checks, more copy protection tied to the CPU serial it has been installed on?

    - currently asserts by selecting a s3 video bank above 1M (register 0x6a) Update: fixed?

    - The version is labeled "SQ05" in the filesystem but has the 1999 release year.
      Other components are labeled "v0.5", but the game doesn't boot far enough to see if
      any graphics have version information. There appears to also be a "Savage Quest 2.1" which
      is undumped.

    PCI list:
    Bus no. Device No. Func No. Vendor ID Device ID Device Class          IRQ
    0       7          1        8086      7111      IDE Controller        14
    0       7          2        8086      7112      Serial Bus Controller 11
    0       9          0        5333      8901      Display Controller    10
    0       13         0        121a      0002      Multimedia Device     NA
    - First two are PIIX4/4E/4M IDE Controller / PIIX4/4E/4M USB Interface
      Third is S3 trio64uv+
      Fourth is Voodoo 2 3D Accelerator
      Sound Blaster is ISA/PnP

============================================================================
    H/W is a white-box PC consisting of:
    Pentium II 450 CPU
    DFI P2XBL motherboard (i440BX chipset)
    128 MB RAM
    Guillemot Maxi Gamer 3D2 Voodoo II
    Sound Blaster AWE64

    Protected by a HASP brand parallel port dongle.
    I/O board has a PIC17C43 which is not readable.

    On boot it reports: S3 86C775/86C705 Video BIOS. Version 2.04.11 Copyright 1996 S3 Incorporated.

- update by Peter Ferrie:
- split BIOS region into 16kb blocks and implement missing PAM registers

- HASP emulator by Peter Ferrie


***************************************************************************/

#include "emu.h"

#include "bus/isa/sblaster.h"
#include "bus/isa/isa_cards.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/fdc37c93x.h"
#include "machine/i82371eb_acpi.h"
#include "machine/i82371eb_ide.h"
#include "machine/i82371eb_isa.h"
#include "machine/i82371eb_usb.h"
#include "machine/i82371sb.h"
#include "machine/i82443bx_host.h"
#include "machine/pci.h"
#include "machine/w83977tf.h"
#include "video/voodoo_2.h"
#include "video/voodoo_pci.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>


DECLARE_DEVICE_TYPE(SAVQUEST_GCI, device_rs232_port_interface)

namespace {

class savquest_state : public driver_device
{
public:
	savquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_voodoo2(*this, "pci:0d.0")
	{
	}

	void savquest(machine_config &config);

protected:
	// driver_device overrides
//  virtual void video_start();


private:
	required_device<voodoo_2_pci_device> m_voodoo2;

	void savquest_io(address_map &map) ATTR_COLD;
	void savquest_map(address_map &map) ATTR_COLD;

	static void winbond_superio_config(device_t *device);
};


void savquest_state::savquest_map(address_map &map)
{
	map.unmap_value_high();
}

void savquest_state::savquest_io(address_map &map)
{
	map.unmap_value_high();
}

// cabinet controls, read by the GCI board on serport0 (kept in the driver so they stay player 1)
static INPUT_PORTS_START( savquest )
	// bits are GCI buttons 1-7, bit 7 is the coin input (the credit counter)
	PORT_START("PANEL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Roar")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Instant Credit")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Operator")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	// GCI buttons 11 and 12 (13-16 are the unused second stick and spares)
	PORT_START("STICK_BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Bite (Trigger)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Charge (Thumb)")

	PORT_START("STICK_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("STICK_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
INPUT_PORTS_END

/*
    Immersia GCI controls board (PIC17C43, not dumped), simulated from the protocol the game speaks.

    The game talks to it on COM1 at 115200 8N1. Frames are ASCII: 'Z', a command character, hex fields,
    then four lowercase hex digits of the 16-bit byte sum (the checksum) and CR.

    Game -> board:
      Z0      connect; reply "Z0" + version (the "REX" prefix selects the CoinSoft command set)
      Z9      firmware version; reply "Z9" + version string
      ZQc     is command c supported; reply "ZQcY" for the commands below, "ZQcN" otherwise
      ZLjjtt  automatic joystick reports every jj ms, 00 = off (trackball every tt ms, not fitted); reply "ZL"
      Z3abtt  outputs: lamps 1-4 (a), lamps 5-8 (b), tickets to dispense (tt); no reply
      ZC      toggle checksums
    Frames with a wrong checksum (while checksums are on) or the wrong length are dropped.
    The buttons are scanned every 16 ms once connected; changes are reported at once.
    Board -> game (the game acknowledges ZP and ZH with an empty frame of the same command):
      ZP      buttons 1-7, credits, ticket status; each button and credit digit holds a press count
              (bits 0-2, mod 8) and a held flag (bit 3 clear = held)
      ZH      buttons 11-16 in the same format (11 = stick trigger, 12 = stick thumb button)
      ZJ      joystick: x1 y1 x2 y2 poll counter, two hex digits each

    Button numbers follow the game's event map: 2 Roar, 4 Start, 6 Instant Credit, 7 Operator,
    11 Bite (trigger), 12 Charge (thumb).
    TODO: version strings are guesses (the game accepts them); joystick reports are sent when the stick moves,
    real board behaviour unknown; stick axis directions unverified.
*/
class savquest_gci_device : public buffered_rs232_device<64>
{
public:
	savquest_gci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void received_byte(u8 byte) override;

private:
	TIMER_CALLBACK_MEMBER(scan);
	TIMER_CALLBACK_MEMBER(joystick);

	void command(std::string_view body);
	void send(std::string_view msg);
	void send_panel();
	void send_stick_buttons();

	static char button_digit(u8 count, bool held) { return "0123456789ABCDEF"[(count & 7) | (held ? 0 : 8)]; }
	static int hex_digit(char c)
	{
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		return -1;
	}

	required_ioport m_panel;
	required_ioport m_stick_buttons;
	required_ioport m_stick_x;
	required_ioport m_stick_y;
	output_finder<8> m_lamps;

	emu_timer *m_scan_timer = nullptr;
	emu_timer *m_joystick_timer = nullptr;
	char m_line[64]{};
	u8 m_line_len = 0;
	bool m_line_overflow = false;
	bool m_checksum = true;
	bool m_autosend = false;
	u8 m_panel_state = 0;
	u8 m_panel_count[8]{};
	u8 m_stick_button_state = 0;
	u8 m_stick_button_count[2]{};
	int m_last_x = -1;
	int m_last_y = -1;
	u8 m_poll = 0;
};

savquest_gci_device::savquest_gci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	buffered_rs232_device<64>(mconfig, SAVQUEST_GCI, tag, owner, clock),
	m_panel(*this, ":PANEL"),
	m_stick_buttons(*this, ":STICK_BUTTONS"),
	m_stick_x(*this, ":STICK_X"),
	m_stick_y(*this, ":STICK_Y"),
	m_lamps(*this, "lamp%u", 1U)
{
}


void savquest_gci_device::device_start()
{
	buffered_rs232_device<64>::device_start();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(115'200);
	output_rxd(1);
	output_dcd(0);
	output_dsr(0);
	output_ri(0);
	output_cts(0);

	m_scan_timer = timer_alloc(FUNC(savquest_gci_device::scan), this);
	m_joystick_timer = timer_alloc(FUNC(savquest_gci_device::joystick), this);

	save_item(NAME(m_line));
	save_item(NAME(m_line_len));
	save_item(NAME(m_line_overflow));
	save_item(NAME(m_checksum));
	save_item(NAME(m_autosend));
	save_item(NAME(m_panel_state));
	save_item(NAME(m_panel_count));
	save_item(NAME(m_stick_button_state));
	save_item(NAME(m_stick_button_count));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
	save_item(NAME(m_poll));
}

void savquest_gci_device::device_reset()
{
	clear_fifo();
	receive_register_reset();
	transmit_register_reset();
	m_line_len = 0;
	m_line_overflow = false;
	m_checksum = true;
	m_autosend = false;
	m_panel_state = 0;
	std::fill(std::begin(m_panel_count), std::end(m_panel_count), 0);
	m_stick_button_state = 0;
	std::fill(std::begin(m_stick_button_count), std::end(m_stick_button_count), 0);
	m_last_x = m_last_y = -1;
	m_poll = 0;
	m_scan_timer->adjust(attotime::never);
	m_joystick_timer->adjust(attotime::never);
	for (int i = 0; i < 8; i++)
		m_lamps[i] = 0;
}

void savquest_gci_device::received_byte(u8 byte)
{
	if (byte == 'Z')
	{
		// a new frame restarts the buffer, as the game's own parser does
		m_line_len = 0;
		m_line_overflow = false;
	}
	if (byte != '\r')
	{
		if (m_line_len < std::size(m_line))
			m_line[m_line_len++] = char(byte);
		else
			m_line_overflow = true;
		return;
	}

	std::string_view body(m_line, m_line_len);
	m_line_len = 0;
	if (m_line_overflow || body.size() < 2 || body[0] != 'Z')
	{
		logerror("dropped malformed frame\n");
		return;
	}
	if (m_checksum)
	{
		unsigned sum = 0;
		bool valid = body.size() >= 6;
		for (size_t i = 0; valid && i < body.size() - 4; i++)
			sum += u8(body[i]);
		for (size_t i = 0; valid && i < 4; i++)
			valid = hex_digit(body[body.size() - 4 + i]) == int((sum >> (12 - 4 * i)) & 0x0f);
		if (!valid)
		{
			logerror("dropped frame with bad checksum: %s\n", std::string(body));
			return;
		}
		body.remove_suffix(4);
	}
	command(body);
}

void savquest_gci_device::command(std::string_view body)
{
	// commands this board implements, with their frame lengths (without checksum)
	static constexpr std::pair<char, size_t> s_commands[] = { { '0', 2 }, { '9', 2 }, { 'Q', 3 }, { 'L', 6 }, { '3', 6 }, { 'C', 2 } };
	auto const found = std::find_if(std::begin(s_commands), std::end(s_commands), [c = body[1]] (auto const &e) { return e.first == c; });
	if (found == std::end(s_commands))
		return; // acknowledgements of our reports (ZP, ZH), and commands this cabinet does not use
	if (body.size() != found->second)
	{
		logerror("dropped frame with wrong length: %s\n", std::string(body));
		return;
	}

	switch (body[1])
	{
	case '0': // connect: start scanning the controls
		send("Z0REX1.0");
		m_scan_timer->adjust(attotime::from_msec(16), 0, attotime::from_msec(16));
		break;

	case '9':
		send("Z91.0");
		break;

	case 'Q':
		{
			bool const supported = std::any_of(std::begin(s_commands), std::end(s_commands), [c = body[2]] (auto const &e) { return e.first == c; });
			send(std::string(body) + (supported ? "Y" : "N"));
		}
		break;

	case 'L':
		{
			if (hex_digit(body[2]) < 0 || hex_digit(body[3]) < 0)
				break;
			int const ms = (hex_digit(body[2]) << 4) | hex_digit(body[3]);
			send("ZL");
			m_autosend = ms != 0;
			m_last_x = m_last_y = -1; // report the current stick position on the first tick
			if (m_autosend)
				m_joystick_timer->adjust(attotime::from_msec(ms), 0, attotime::from_msec(ms));
			else
				m_joystick_timer->adjust(attotime::never);
		}
		break;

	case '3':
		if (hex_digit(body[2]) >= 0 && hex_digit(body[3]) >= 0)
		{
			u8 const lamps = hex_digit(body[2]) | (hex_digit(body[3]) << 4);
			for (int i = 0; i < 8; i++)
				m_lamps[i] = BIT(lamps, i);
		}
		break;

	case 'C':
		m_checksum = !m_checksum;
		break;
	}
}

TIMER_CALLBACK_MEMBER(savquest_gci_device::scan)
{
	u8 const panel = m_panel->read();
	if (panel != m_panel_state)
	{
		for (int i = 0; i < 8; i++)
			if (BIT(panel, i) && !BIT(m_panel_state, i))
				m_panel_count[i]++;
		m_panel_state = panel;
		send_panel();
	}

	u8 const stick_buttons = m_stick_buttons->read() & 0x03;
	if (stick_buttons != m_stick_button_state)
	{
		for (int i = 0; i < 2; i++)
			if (BIT(stick_buttons, i) && !BIT(m_stick_button_state, i))
				m_stick_button_count[i]++;
		m_stick_button_state = stick_buttons;
		send_stick_buttons();
	}
}

TIMER_CALLBACK_MEMBER(savquest_gci_device::joystick)
{
	int const x = m_stick_x->read();
	int const y = m_stick_y->read();
	if (x != m_last_x || y != m_last_y)
	{
		m_last_x = x;
		m_last_y = y;
		send(util::string_format("ZJ%02X%02X8080%02X", x, y, m_poll++));
	}
}

void savquest_gci_device::send_panel()
{
	std::string msg = "ZP";
	for (int i = 0; i < 8; i++)
		msg.push_back(button_digit(m_panel_count[i], BIT(m_panel_state, i)));
	msg.push_back('1'); // tickets OK
	send(msg);
}

void savquest_gci_device::send_stick_buttons()
{
	std::string msg = "ZH";
	for (int i = 0; i < 6; i++)
		msg.push_back((i < 2) ? button_digit(m_stick_button_count[i], BIT(m_stick_button_state, i)) : '8');
	send(msg);
}

void savquest_gci_device::send(std::string_view msg)
{
	std::string out(msg);
	if (m_checksum)
	{
		unsigned sum = 0;
		for (char c : msg)
			sum += u8(c);
		out += util::string_format("%04x", sum & 0xffff);
	}
	out.push_back('\r');
	for (char c : out)
		transmit_byte(u8(c));
}


static void savquest_com(device_slot_interface &device)
{
	default_rs232_devices(device);
	device.option_add("gci", SAVQUEST_GCI);
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("w83977tf", W83977TF);
}

void savquest_state::winbond_superio_config(device_t *device)
{
	w83977tf_device &fdc = *downcast<w83977tf_device *>(device);
//  fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set(":pci:07.0", FUNC(i82371eb_isa_device::a20gate_w));
	fdc.irq1().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq8n_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));

	auto *lpt = device->subdevice<pc_lpt_device>("lpt");
	auto *centronics = lpt->subdevice<centronics_device>("centronics");
	centronics->set_default_option("hasp_savquest");
	centronics->set_fixed(true);
}


void savquest_state::savquest(machine_config &config)
{
	pentium2_device &maincpu(PENTIUM2(config, "maincpu", 66'000'000)); // actually Pentium II @ 450 MHz
	// Execution is underclocked; count timestamps at an assumed nominal 450 MHz (board clock unverified)
	maincpu.set_tsc_cycle_ratio(75, 11);
	maincpu.set_addrmap(AS_PROGRAM, &savquest_state::savquest_map);
	maincpu.set_addrmap(AS_IO, &savquest_state::savquest_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82443BX_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);
	I82443BX_BRIDGE(config, "pci:01.0", 0 );

	i82371eb_isa_device &isa(I82371EB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);
	isa.a20m().set_inputline("maincpu", INPUT_LINE_A20);

	i82371eb_ide_device &ide(I82371EB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371eb_isa_device::pc_mirq0_w));

	I82371EB_USB (config, "pci:07.2", 0);
	I82371EB_ACPI(config, "pci:07.3", 0);
	ACPI_PIIX4   (config, "pci:07.3:acpi");
	SMBUS        (config, "pci:07.3:smbus", 0);

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "w83977tf", true).set_option_machine_config("w83977tf", winbond_superio_config);
	// TODO: awe64 by default
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, "sblaster_16", false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	// the GCI controls board talks to the PC on COM1
	rs232_port_device &serport0(RS232_PORT(config, "serport0", savquest_com, "gci"));
	serport0.rxd_handler().set("board4:w83977tf", FUNC(w83977tf_device::rxd1_w));
	serport0.dcd_handler().set("board4:w83977tf", FUNC(w83977tf_device::ndcd1_w));
	serport0.dsr_handler().set("board4:w83977tf", FUNC(w83977tf_device::ndsr1_w));
	serport0.ri_handler().set("board4:w83977tf", FUNC(w83977tf_device::nri1_w));
	serport0.cts_handler().set("board4:w83977tf", FUNC(w83977tf_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", default_rs232_devices, nullptr));
	serport1.rxd_handler().set("board4:w83977tf", FUNC(w83977tf_device::rxd2_w));
	serport1.dcd_handler().set("board4:w83977tf", FUNC(w83977tf_device::ndcd2_w));
	serport1.dsr_handler().set("board4:w83977tf", FUNC(w83977tf_device::ndsr2_w));
	serport1.ri_handler().set("board4:w83977tf", FUNC(w83977tf_device::nri2_w));
	serport1.cts_handler().set("board4:w83977tf", FUNC(w83977tf_device::ncts2_w));

	VOODOO_2_PCI(config, m_voodoo2, 0, "maincpu", "voodoo_screen");
	m_voodoo2->set_fbmem(4);
	m_voodoo2->set_tmumem(4, 4); /* this is the 12Mb card */
	m_voodoo2->set_status_cycles(1000); // optimization to consume extra cycles when polling status

//  m_voodoo2->vblank_callback().set(FUNC(savquest_state::vblank_assert));

	screen_device &screen(SCREEN(config, "voodoo_screen"));
	screen.set_refresh_hz(57);
	screen.set_size(800, 262);
	screen.set_visarea(0, 512 - 1, 0, 240 - 1);
	screen.set_screen_update("pci:0d.0", FUNC(voodoo_2_pci_device::screen_update));

	PCI_SLOT(config, "pci:01.0:0", agp_cards, 0, 0, 1, 2, 3, nullptr);

	PCI_SLOT(config, "pci:1", pci_cards, 9, 0, 1, 2, 3, "trio64dx").set_fixed(true);
	PCI_SLOT(config, "pci:2", pci_cards, 10, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 11, 2, 3, 0, 1, nullptr);
//  PCI_SLOT(config, "pci:4", pci_cards, 12, 3, 0, 1, 2, "voodoo2");
}

ROM_START( savquest )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD( "p2xbl_award_451pg.bin", 0x00000, 0x040000, CRC(37d0030e) SHA1(c6773d0e02325116f95c497b9953f59a9ac81317) )

//  ROM_REGION32_LE( 0x10000, "video_bios", 0 ) // 1st half is 2.04.14, second half is 2.01.11
//  ROM_LOAD( "vgabios.bin",   0x000000, 0x010000, CRC(a81423d6) SHA1(a099af621ce7fbaa55a2d9947d9f07e04f1b5fca) )

	ROM_REGION( 0x080, "rtc", 0 )    /* default NVRAM */
	ROM_LOAD( "savquest_ds12885.bin", 0x0000, 0x080, BAD_DUMP CRC(e9270019) SHA1(4d900ca317d93c915c80a9053528b741746f08a1) )

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "savquest", 0, SHA1(b7c8901172b66706a7ab5f5c91e6912855153fa9) )
ROM_END

} // Anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SAVQUEST_GCI, device_rs232_port_interface, savquest_gci_device, "savquest_gci", "Immersia GCI controls board (simulated)")

GAME(1999, savquest, 0, savquest, savquest, savquest_state, empty_init, ROT0, "Interactive Light", "Savage Quest", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
