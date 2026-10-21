// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/****************************************************************************

    Preliminary driver for the Spare Time Gizmos SBC6120.

    Hardware documentation, firmware sources and GAL equations:
    https://github.com/SpareTimeGizmos/SBC6120

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/pdp8/hd6120.h"
#include "machine/ay31015.h"
#include "machine/clock.h"


namespace {

class sbc6120_state : public driver_device
{
public:
	sbc6120_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_brg(*this, "brg")
		, m_eprom(*this, "eprom")
		, m_jumpers(*this, "JUMPERS")
		, m_panel_iview(*this, "paneli")
		, m_panel_dview(*this, "paneld")
		, m_slu_ioview(*this, "slui")
		, m_slu_ctlview(*this, "sluc")
		, m_kbd_flag(false)
		, m_prt_flag(false)
		, m_prt_eoc(true)
	{
	}

	void sbc6120(machine_config &config);
	void init_sbc6120();

	DECLARE_INPUT_CHANGED_MEMBER(baud_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void memory_map_w(offs_t offset, u16 data);

	void kcc_w(u16 data);
	u16 krs_r();
	void tcf_w(u16 data);
	void tpc_w(u16 data);
	void tls_w(u16 data);
	u8 ksf_devctl_r();
	u8 tsf_devctl_r();
	void sslucm_w(u16 data);
	void sslusm_w(u16 data);

	void keyboard_dav_w(int state);
	void printer_eoc_w(int state);
	void framing_error_w(int state);
	void update_interrupt();
	void ioclr_w(int state);

	void inst_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void devctl_map(address_map &map) ATTR_COLD;

	required_device<hd6120_device> m_maincpu;
	required_device<ay31015_device> m_uart;
	required_device<clock_device> m_brg;
	required_region_ptr<u16> m_eprom;
	required_ioport m_jumpers;
	memory_view m_panel_iview;
	memory_view m_panel_dview;
	memory_view m_slu_ioview;
	memory_view m_slu_ctlview;

	bool m_kbd_flag;
	bool m_prt_flag;
	bool m_prt_eoc;
};

void sbc6120_state::init_sbc6120()
{
	for (offs_t addr = 0; addr < m_eprom.length(); addr++)
		m_eprom[addr] = (m_eprom[addr] >> 8 & 077) << 6 | (m_eprom[addr] & 077);
}

void sbc6120_state::machine_start()
{
	// U16 is strapped for eight data bits, no parity and one stop bit
	m_uart->write_np(1);
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_tsb(0);
	m_uart->write_eps(0);
	m_uart->write_cs(1);
	m_uart->write_swe(0);
	m_uart->write_rdav(1);

	save_item(NAME(m_kbd_flag));
	save_item(NAME(m_prt_flag));
	save_item(NAME(m_prt_eoc));
}

void sbc6120_state::machine_reset()
{
	m_panel_iview.select(0);
	m_panel_dview.select(0);
	m_slu_ioview.select(0);
	m_slu_ctlview.select(0);

	m_brg->set_unscaled_clock(4.9152_MHz_XTAL / (m_jumpers->read() & 0x7ff));
}

void sbc6120_state::memory_map_w(offs_t offset, u16 data)
{
	m_panel_iview.select(offset);
	m_panel_dview.select(offset);
}

//  The console is a proper subset of the KL8E
void sbc6120_state::kcc_w(u16 data)
{
	m_uart->write_rdav(0);
	m_uart->write_rdav(1);
}

u16 sbc6120_state::krs_r()
{
	return m_uart->receive();
}

void sbc6120_state::tcf_w(u16 data)
{
	m_prt_flag = false;
	update_interrupt();
}

void sbc6120_state::tpc_w(u16 data)
{
	m_uart->transmit(data & 0377);
}

void sbc6120_state::tls_w(u16 data)
{
	tcf_w(data);
	tpc_w(data);
}

u8 sbc6120_state::ksf_devctl_r()
{
	return m_kbd_flag ? hd6120_device::SKIP : 0;
}

// The printer IOTs never transfer data to the AC and, unlike a real KL8E, never clear it either
u8 sbc6120_state::tsf_devctl_r()
{
	return m_prt_flag ? hd6120_device::SKIP : 0;
}

void sbc6120_state::sslucm_w(u16 data)
{
	m_slu_ioview.select(0);
	m_slu_ctlview.select(0);
}

void sbc6120_state::sslusm_w(u16 data)
{
	m_slu_ioview.select(1);
	m_slu_ctlview.select(1);
}

void sbc6120_state::keyboard_dav_w(int state)
{
	m_kbd_flag = state;
	update_interrupt();
}

void sbc6120_state::printer_eoc_w(int state)
{
	// TODO: edge trigger
	if (state && !m_prt_eoc)
	{
		m_prt_flag = true;
		update_interrupt();
	}
	m_prt_eoc = state;
}

// With J10 installed, a framing error - which is what the BREAK key looks like to the UART - requests panel mode
void sbc6120_state::framing_error_w(int state)
{
	if (BIT(m_jumpers->read(), 15))
		m_maincpu->set_input_line(hd6120_device::CPREQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void sbc6120_state::update_interrupt()
{
	m_maincpu->set_input_line(hd6120_device::INTREQ_LINE, (m_kbd_flag || m_prt_flag) ? ASSERT_LINE : CLEAR_LINE);
}

void sbc6120_state::ioclr_w(int state)
{
	m_uart->write_xr(!state);

	if (!state)
	{
		m_prt_flag = false;
		update_interrupt();
	}
}

INPUT_CHANGED_MEMBER(sbc6120_state::baud_changed)
{
	m_brg->set_unscaled_clock(4.9152_MHz_XTAL / (newval & 0x7ff));
}

//  Memory map
//
//      map  direct      indirect
//      ---  ----------  ---------
//       0   EPROM       RAM        selected by the hardware at reset
//       1   RAM         EPROM      used to copy the EPROM into RAM
//       2   RAM         RAM        the normal mapping
//       3   RAM         RAM disk
//
void sbc6120_state::inst_map(address_map &map)
{
	map(0000000, 0077777).ram().share("mainram");
	map(0100000, 0177777).ram().share("panelram");
	map(0100000, 0177777).view(m_panel_iview);
	// the EPROM is never enabled for writes, so those fall through to the SRAM
	m_panel_iview[0](0100000, 0177777).rom().region("eprom", 0);
	m_panel_iview[1]; // RAM
	m_panel_iview[2]; // RAM
	m_panel_iview[3]; // RAM
}

void sbc6120_state::data_map(address_map &map)
{
	map(0000000, 0077777).ram().share("mainram");
	map(0100000, 0177777).ram().share("panelram");
	map(0100000, 0177777).view(m_panel_dview);
	m_panel_dview[0]; // RAM
	m_panel_dview[1](0100000, 0177777).rom().region("eprom", 0);
	m_panel_dview[2]; // RAM
	m_panel_dview[3](0100000, 0177777).lr16(NAME([]() { return 0377; })).nopw();
}

// TODO: remaining I/O devices
// 41: IDE interface (82C55 PPI) and RAM disk daughter board
// 44: POST code display
// 643x: FP6120 front panel
void sbc6120_state::io_map(address_map &map)
{
	map(0030, 0377).view(m_slu_ioview);
	m_slu_ioview[0](0032, 0032).w(FUNC(sbc6120_state::kcc_w));
	m_slu_ioview[0](0034, 0034).r(FUNC(sbc6120_state::krs_r));
	m_slu_ioview[0](0036, 0036).rw(FUNC(sbc6120_state::krs_r), FUNC(sbc6120_state::kcc_w));
	m_slu_ioview[0](0042, 0042).w(FUNC(sbc6120_state::tcf_w));
	m_slu_ioview[0](0044, 0044).w(FUNC(sbc6120_state::tpc_w));
	m_slu_ioview[0](0046, 0046).w(FUNC(sbc6120_state::tls_w));
	m_slu_ioview[1](0362, 0362).w(FUNC(sbc6120_state::kcc_w));
	m_slu_ioview[1](0364, 0364).r(FUNC(sbc6120_state::krs_r));
	m_slu_ioview[1](0366, 0366).rw(FUNC(sbc6120_state::krs_r), FUNC(sbc6120_state::kcc_w));
	m_slu_ioview[1](0372, 0372).w(FUNC(sbc6120_state::tcf_w));
	m_slu_ioview[1](0374, 0374).w(FUNC(sbc6120_state::tpc_w));
	m_slu_ioview[1](0376, 0376).w(FUNC(sbc6120_state::tls_w));
	map(0400, 0403).w(FUNC(sbc6120_state::memory_map_w));
	map(0412, 0412).w(FUNC(sbc6120_state::sslucm_w));
	map(0413, 0413).w(FUNC(sbc6120_state::sslusm_w));
}

// Anything not decoded here leaves C0, C1 and SKIP inactive, so unused IOTs neither skip nor disturb the AC
void sbc6120_state::devctl_map(address_map &map)
{
	map(0030, 0377).view(m_slu_ctlview);
	m_slu_ctlview[0](0031, 0031).r(FUNC(sbc6120_state::ksf_devctl_r));
	m_slu_ctlview[0](0032, 0032).lr8(NAME([]() { return hd6120_device::C0; }));
	m_slu_ctlview[0](0034, 0034).lr8(NAME([]() { return hd6120_device::C1; }));
	m_slu_ctlview[0](0036, 0036).lr8(NAME([]() { return hd6120_device::C0 | hd6120_device::C1; }));
	m_slu_ctlview[0](0041, 0041).r(FUNC(sbc6120_state::tsf_devctl_r));
	m_slu_ctlview[1](0361, 0361).r(FUNC(sbc6120_state::ksf_devctl_r));
	m_slu_ctlview[1](0362, 0362).lr8(NAME([]() { return hd6120_device::C0; }));
	m_slu_ctlview[1](0364, 0364).lr8(NAME([]() { return hd6120_device::C1; }));
	m_slu_ctlview[1](0366, 0366).lr8(NAME([]() { return hd6120_device::C0 | hd6120_device::C1; }));
	m_slu_ctlview[1](0371, 0371).r(FUNC(sbc6120_state::tsf_devctl_r));
	map(0412, 0412).lr8(NAME([]() { return hd6120_device::C0; })); // SSLUCM
}

static INPUT_PORTS_START(sbc6120)
	PORT_START("JUMPERS")
	PORT_CONFNAME(0x07ff, 0x0020, "Console Baud Rate") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sbc6120_state::baud_changed), 0)
	PORT_CONFSETTING(0x0008, "38400 (J11)")
	PORT_CONFSETTING(0x0020, "9600 (J12)")
	PORT_CONFSETTING(0x0100, "1200 (J13)")
	PORT_CONFSETTING(0x0400, "300 (J14)")
	PORT_CONFNAME(0x8000, 0x8000, "Console Break Requests Panel Mode (J10)")
	PORT_CONFSETTING(0x0000, DEF_STR(No))
	PORT_CONFSETTING(0x8000, DEF_STR(Yes))
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

// XTALs: 5.000 MHz, 4.9152 MHz
void sbc6120_state::sbc6120(machine_config &config)
{
	HD6120(config, m_maincpu, 5_MHz_XTAL); // U1 oscillator, 8 MHz parts also work
	m_maincpu->set_addrmap(AS_PROGRAM, &sbc6120_state::inst_map);
	m_maincpu->set_addrmap(AS_DATA, &sbc6120_state::data_map);
	m_maincpu->set_addrmap(AS_IO, &sbc6120_state::io_map);
	m_maincpu->set_addrmap(hd6120_device::AS_DEVCTL, &sbc6120_state::devctl_map);
	m_maincpu->ioclr_callback().set(FUNC(sbc6120_state::ioclr_w));
	m_maincpu->strtup_callback().set_constant(0); // STRTUP is tied low, so the CPU traps to panel memory first

	AY31015(config, m_uart); // U16 is an IM6402
	m_uart->write_so_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->write_dav_callback().set(FUNC(sbc6120_state::keyboard_dav_w));
	m_uart->write_eoc_callback().set(FUNC(sbc6120_state::printer_eoc_w));
	m_uart->write_fe_callback().set(FUNC(sbc6120_state::framing_error_w));
	m_uart->set_auto_rdav(false);

	// U23 and the U15 counter divide 4.9152 MHz down to sixteen times the jumpered baud rate
	CLOCK(config, m_brg, 4.9152_MHz_XTAL / 32);
	m_brg->signal_handler().set(m_uart, FUNC(ay31015_device::write_rcp));
	m_brg->signal_handler().append(m_uart, FUNC(ay31015_device::write_tcp));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_uart, FUNC(ay31015_device::write_si));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

ROM_START(sbc6120)
	ROM_REGION16_BE(0200000, "eprom", ROMREGION_ERASE00) // U9 and U10
	ROM_DEFAULT_BIOS("v271")

	ROM_SYSTEM_BIOS(0, "v271", "BTS6120 V271 (04-NOV-04)") // last release for the original SBC6120
	ROMX_LOAD("271h.bin", 0x0000, 0x3000, CRC(89c548b3) SHA1(6f8715558bd59895ea901f0b548135f0af8a467f), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("271l.bin", 0x0001, 0x3000, CRC(e12afc09) SHA1(f0baa111d6dc8751c7fc2bc7199029e55f891be5), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v320", "BTS6120 V320 (09-APR-10)") // released for the SBC6120-RC, but runs here too
	ROMX_LOAD("320h.bin", 0x0000, 0x3000, CRC(5cb4aa14) SHA1(dae8d51914470e74fe8809696957333858fb8953), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("320l.bin", 0x0001, 0x3000, CRC(ff7ed516) SHA1(e3f5a7a1f997eaac2913417e128de59276048c6a), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v266", "BTS6120 V266 (03-JUL-03)")
	ROMX_LOAD("266h.bin", 0x0000, 0x3000, CRC(441a4126) SHA1(711cafc26dc4a9bb57b94c6a11c4a4c62a95c978), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("266l.bin", 0x0001, 0x3000, CRC(dc8d437e) SHA1(6e445abf97b3b93b1bd0fbf12cee3f12b4174746), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "v215", "BTS6120 V215 (02-FEB-03)")
	ROMX_LOAD("215h.bin", 0x0000, 0x2000, CRC(3e294240) SHA1(f9383362f155af29e0ee499de7fab6637ff3cc42), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("215l.bin", 0x0001, 0x2000, CRC(7c3f836f) SHA1(8d6525a9e88a75a84e0afcaead1f515ab5607a6c), ROM_SKIP(1) | ROM_BIOS(3))
ROM_END

} // anonymous namespace


COMP(2001, sbc6120, 0, 0, sbc6120, sbc6120, sbc6120_state, init_sbc6120, "Spare Time Gizmos", "SBC6120", MACHINE_NO_SOUND_HW)
