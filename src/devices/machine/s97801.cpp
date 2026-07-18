// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Siemens 97801 terminal (LLE) -- core device

    The serial console for SINIX on the Siemens PC-MX2 / MX300.  Low-level emulation:
      - SAB8031A microcontroller (no internal ROM; external EPROMs d26/d21)
      - SCN2672B AVDC + SCB2673-class attribute plane + d23 char-gen (512 glyphs)
      - SCN2661B EPCI -> host (SS97 = 38400 7O1 XON/XOFF)
      - detached keyboard (s97801_kbd LLE device) on the 8031 on-chip UART (TH1=0xD0
        -> f_cpu/18432 = 600.0 baud at 11.0592 MHz, matching the keyboard exactly)
    The D26/D21/D23 EPROMs were imaged from a W26361-D253 (second revision) board, whose
    clock modules are 22.1184 MHz (8031 = /2 = 11.0592 MHz, video dot clock) + 4.9152 MHz
    (EPCI baud); the matched K111-V1 keyboard of the same unit supplied the kbd dump.
    (The earlier D311 revision carries 24.000 MHz instead: 12 MHz CPU, 651-baud kbd link,
    same 1.92 crystal pairing satisfied by a 6.25 MHz keyboard.)
    References: Transdata 970/9780 Wartungshandbuch; SINIX Schnittstellen Benutzerhandbuch,
    U2300-J-Z95-1 (12/1985) and its 1987 edition; terminal board W26361-D253.

    Packaged as a device so it can serve as a serial terminal for any host (its real use is the
    PC-MX2 SERAD port): rxd_w() = host->terminal, txd_handler() = terminal->host.

***************************************************************************/

#include "emu.h"
#include "s97801.h"

#include "screen.h"

#include "s97801.lh"


DEFINE_DEVICE_TYPE(SIEMENS_97801, s97801_device, "s97801_device", "Siemens 97801 Terminal")

s97801_device::s97801_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SIEMENS_97801, tag, owner, clock)
	, m_cpu(*this, "maincpu")
	, m_avdc(*this, "avdc")
	, m_epci(*this, "epci")
	, m_kbd(*this, "kbd")
	, m_screen(*this, "screen")
	, m_chargen(*this, "chargen")
	, m_txd_cb(*this)
	, m_kbd_rxd(1)
{
}

void s97801_device::device_resolve_objects()
{
	m_kbd_rxd = 1; // the keyboard drives this line; it will update us if it changes on reset
}

void s97801_device::device_start()
{
	save_item(NAME(m_kbd_rxd));
	m_txd_cb(1); // mark the host line idle at power-up; the EPCI drives it thereafter
}

// host -> terminal serial line
void s97801_device::rxd_w(int state)
{
	m_epci->rxd_w(state);
}

// 8031 P3.0 = keyboard serial RX (the on-chip UART samples bit 0)
u8 s97801_device::cpu_p3_r()
{
	return 0xfe | (m_kbd_rxd & 1);
}

// E000/E001 controller status: E002 bit7=1 (normal, NOT loopback -- else the self-test at L0660
// puts the EPCI into local loopback and kills host RX); E003 bit0=0 (INT1 handler processes bytes).
u8 s97801_device::e00x_status_r(offs_t offset)
{
	return (offset == 0) ? 0x80 : 0x00;
}

void s97801_device::e00x_status_w(offs_t offset, u8 data)
{
}

// 8031 P3.1 = keyboard serial TX (the core's UART engine reports each output transition here)
void s97801_device::cpu_p3_w(u8 data)
{
	m_kbd->rxd_w(BIT(data, 1));
}

void s97801_device::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0); // d26
	map(0x2000, 0x3fff).rom().region("unk", 0);     // d21
}

void s97801_device::data_map(address_map &map)
{
	// 8031 external data bus (MOVX) -- firmware I/O map recovered from the d26/d21 RE
	map(0x0000, 0x7fff).ram();                      // HM62256 32Kx8 work RAM @ D34 (A15 = RAM/device select)
	map(0xc000, 0xc007).rw(m_avdc, FUNC(scn2672_device::read), FUNC(scn2672_device::write)); // AVDC
	// E000/E001 = the char/attr display-plane data latches; the AVDC write cmds (0xA2/0xAB/0xBB)
	// supply the RAM address + write cycle while these latches drive the data buses, so route them
	// to the AVDC char/attr buffer (the fix for "no visible firmware text").
	map(0xe000, 0xe000).rw(m_avdc, FUNC(scn2672_device::buffer_r), FUNC(scn2672_device::buffer_w));
	map(0xe001, 0xe001).rw(m_avdc, FUNC(scn2672_device::attr_buffer_r), FUNC(scn2672_device::attr_buffer_w));
	map(0xe002, 0xe003).rw(FUNC(s97801_device::e00x_status_r), FUNC(s97801_device::e00x_status_w));
	// host EPCI (SCN2661): only A0/A1 decoded (8000 RX / 8004 TX / 8001 status / 8006 mode / 8007 cmd)
	map(0x8000, 0x8003).mirror(0x0004).rw(m_epci, FUNC(scn2661b_device::read), FUNC(scn2661b_device::write));
}

void s97801_device::char_map(address_map &map)
{
	map(0x0000, 0x3fff).ram(); // Charakter refresh plane (D4364C @ D11); AVDC scans it
}

void s97801_device::attr_map(address_map &map)
{
	map(0x0000, 0x3fff).ram(); // Attribut plane (HM6116 D13/D20) -> SCB2673
}

SCN2672_DRAW_CHARACTER_MEMBER(s97801_device::draw_character)
{
	// SCB2673 attribute byte (this board's wiring, from the firmware SGR handler sub_129A; base 0x08):
	//   b0 blank, b1 blink, b2 underline, b3 intensity(1=normal/0=dim), b4 reverse,
	//   b5 alt/graphics charset bank -> d23 upper 256-glyph (line-draw "w" set, SO/SI via 0x0400 table).
	const bool a_blank = BIT(attrcode, 0);
	const bool a_blink = BIT(attrcode, 1);
	const bool a_ul    = BIT(attrcode, 2);
	const bool a_norm  = BIT(attrcode, 3);
	const bool a_rvid  = BIT(attrcode, 4);
	const bool a_bank  = BIT(attrcode, 5);

	const unsigned glyph = (a_bank ? 0x100 : 0) | charcode;
	u16 dots = (a_blank || (a_blink && blink)) ? 0 : u16(m_chargen[(glyph << 4) | linecount]) << 1;
	if (a_ul && ul)
		dots = 0x1ff;
	if (a_rvid)
		dots = ~dots;
	if (cursor)
		dots = ~dots;

	const rgb_t fg = a_norm ? rgb_t::white() : rgb_t(0x80, 0x80, 0x80);
	for (int i = 0; i < 9; i++)
	{
		bitmap.pix(y, x++) = BIT(dots, 8) ? fg : rgb_t::black();
		dots <<= 1;
	}
}

void s97801_device::device_add_mconfig(machine_config &config)
{
	// SAB8031 @ 22.1184/2 = 11.0592 MHz, per the ROM-source board (W26361-D253).  The fw
	// programs TH1=0xD0 (48 counts, SMOD=0) -> kbd link = f_cpu/18432 = 600.0 baud, matching
	// the 5.76 MHz keyboard's f_kbd/9600 exactly (the two firmwares pair crystals at a fixed
	// 1.92 ratio; the earlier 24.000 MHz D311 revision pairs with a 6.25 MHz keyboard at 651).
	I8031(config, m_cpu, 22.1184_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &s97801_device::prg_map);
	m_cpu->set_addrmap(AS_DATA, &s97801_device::data_map);
	m_cpu->port_in_cb<3>().set(FUNC(s97801_device::cpu_p3_r));  // P3.0 = keyboard RX
	m_cpu->port_out_cb<3>().set(FUNC(s97801_device::cpu_p3_w)); // P3.1 = keyboard TX

	SIEMENS_97801_KBD(config, m_kbd);
	m_kbd->txd_handler().set(FUNC(s97801_device::kbd_txd_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // white-phosphor tube
	m_screen->set_raw(22.1184_MHz_XTAL, 912, 0, 640, 423, 0, 400); // ~57.3 Hz on the D253 clocking
	m_screen->set_screen_update(m_avdc, FUNC(scn2672_device::screen_update));

	SCN2672(config, m_avdc, 22.1184_MHz_XTAL / 8);
	m_avdc->set_screen(m_screen);
	m_avdc->set_character_width(8);
	m_avdc->set_addrmap(0, &s97801_device::char_map);
	m_avdc->set_addrmap(1, &s97801_device::attr_map);
	m_avdc->set_display_callback(FUNC(s97801_device::draw_character));
	m_avdc->intr_callback().set_inputline(m_cpu, MCS51_INT0_LINE);

	SCN2661B(config, m_epci, 4.9152_MHz_XTAL);
	m_epci->rxrdy_handler().set_inputline(m_cpu, MCS51_INT1_LINE);
	m_epci->txd_handler().set(FUNC(s97801_device::epci_txd_w)); // terminal -> host

	// clickable keyboard under the screen; attached to the device so it is available both
	// standalone and when the terminal is used as an rs232 peripheral
	config.set_default_layout(layout_s97801);
}

ROM_START(s97801)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("010_d26__0118_04.d26", 0x0000, 0x2000, CRC(fcf045d7) SHA1(4a98e7d2d98272970d627ce5c10e9572b87293d1))

	ROM_REGION(0x2000, "unk", 0)
	ROM_LOAD("010_d21__0118_04.d21", 0x0000, 0x2000, CRC(b9b9df32) SHA1(9a3ba060ebcf00b1ed9112493a1d73212c04d8e5))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("010_d23__0118_03.d23", 0x0000, 0x2000, CRC(23b22a7d) SHA1(649abcfde9752f427ec7d1efdc013a4f01dc271c))
ROM_END

const tiny_rom_entry *s97801_device::device_rom_region() const
{
	return ROM_NAME(s97801);
}
