// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Siemens S26361-D333 CPUAP processor board.
 *
 * This Multibus card was in Siemens PC-MX2 systems. It has a 10MHz NS32016
 * CPU with the complete set of NS32000 support chips and 1MiB of on-board RAM.
 * The board connects to optional memory expansion boards through a dedicated
 * connector (not the Multibus). It has no on-board UART, so depends on a SERAD
 * or DUEAI board installed in the system to communicate with a console.
 *
 * Sources:
 *  - https://oldcomputers-ddns.org/public/pub/rechner/siemens/mx-rm/pc-mx2/manuals/pc-mx2_pc2000_9780_logik.pdf
 *  - https://mx300i.narten.de/view_board.cfm?5EF287A1ABC3F4DCAFEA2BC2FAB8C4000E50392BD499
 *
 * TODO:
 *  - led output
 *  - nmi generation
 *  - figure out how to pass-through multibus addresses
 */

#include "emu.h"
#include "cpuap.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CPUAP, cpuap_device, "cpuap", "Siemens S26361-D333 CPUAP")

cpuap_device::cpuap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CPUAP, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_fpu(*this, "fpu")
	, m_mmu(*this, "mmu")
	, m_icu(*this, "icu")
	, m_rtc(*this, "rtc")
	, m_s7(*this, "S7")
	, m_s8(*this, "S8")
	, m_boot(*this, "boot")
{
}

// processor diagnostic register
enum prdia_mask : u8
{
	LED1     = 0x01,
	LED2     = 0x02,
	LED3     = 0x04,
	LED4     = 0x08,
	LED5     = 0x10,
	LED6     = 0x20,
	ERRORLED = 0x40,
	ENNMI    = 0x80,
};

enum nmi_mask : u8
{
	PAR2    = 0x01,
	DEBUGI  = 0x02,
	ACF     = 0x04,
	PERLO   = 0x08, // parity error lo
	PERHI   = 0x10, // parity error hi
	BTIMOUT = 0x20, // bus timeout
	INT7    = 0x40,
};

ROM_START(cpuap)
	ROM_REGION16_LE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "rev9", "D333 Monitor Rev 9.0 16.06.1988")
	ROMX_LOAD("361d0333d053__e01735_ine.d53", 0x0000, 0x4000, CRC(b5eefb64) SHA1(a71a7daf9a8f0481d564bfc4d7ed5eb955f8665f), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("361d0333d054__e01725_ine.d54", 0x0001, 0x4000, CRC(3a3c6b6e) SHA1(5302fd79c89e0b4d164c639e2d73f4b9a279ddcb), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_FILL(0x8000, 0x8000, 0xff, ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "rev3", "D333 Monitor Rev 3 09.12.1985")
	ROMX_LOAD("d333__d56_g53__lb.d56", 0x0000, 0x2000, CRC(0892ff90) SHA1(e84ceb8eb3c13de3692297c46632dbfafaad675f), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("d333__d55_g53__hb.d55", 0x0001, 0x2000, CRC(821e1e41) SHA1(0800249eab8db490c1fb6fea6d65bc7e874c9a0c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_FILL(0x4000, 0xc000, 0xff, ROM_BIOS(1))
ROM_END

static INPUT_PORTS_START(cpuap)
	PORT_START("S7")

	// Offen: Ausgabe des Urladers über Diagnose-Stecker, keine SERAD/G Baugruppe gesteckt
	// Open: boot loader output via diagnostic plug, no SERAD/G module plugged in
	PORT_DIPNAME(0x80, 0x00, "Diagnostic") PORT_DIPLOCATION("S7:8")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x80, DEF_STR(On))

	// Offen: Monitor-Programm nach Testende
	// Open: enter monitor after test
	PORT_DIPNAME(0x40, 0x00, "Boot Option") PORT_DIPLOCATION("S7:7")
	PORT_DIPSETTING(0x00, "Disk")
	PORT_DIPSETTING(0x40, "Monitor")

	PORT_DIPNAME(0x20, 0x00, "S7:5") PORT_DIPLOCATION("S7:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, "S7:4") PORT_DIPLOCATION("S7:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x00, "S7:3") PORT_DIPLOCATION("S7:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "S7:2") PORT_DIPLOCATION("S7:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "S7:1") PORT_DIPLOCATION("S7:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))

	// Offen: kein Reboot nach Systemabsturz
	// Open: no reboot after system crash
	PORT_DIPNAME(0x01, 0x00, "S7:0") PORT_DIPLOCATION("S7:1") // S7:0 not allowed, causes crash in the dips menu
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	PORT_START("S8")
	PORT_DIPNAME(0x01, 0x00, "IRQ 7")
	PORT_DIPSETTING(0x00, "ICU IR11")
	PORT_DIPSETTING(0x01, "NMI")
INPUT_PORTS_END

const tiny_rom_entry *cpuap_device::device_rom_region() const
{
	return ROM_NAME(cpuap);
}

ioport_constructor cpuap_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cpuap);
}

void cpuap_device::device_start()
{
}

void cpuap_device::device_reset()
{
	m_boot.select(0);
	m_nmi = 0xff;
}

void cpuap_device::bus_timeout(u8 data)
{
	m_nmi &= ~BTIMOUT;

	if (m_prdia & ENNMI)
		m_cpu->set_input_line(INPUT_LINE_NMI, 1);
}

void cpuap_device::device_add_mconfig(machine_config &config)
{
	NS32016(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(0, &cpuap_device::cpu_map<0>);
	m_cpu->set_addrmap(4, &cpuap_device::cpu_map<4>);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
	m_cpu->set_mmu(m_mmu);

	NS32202(config, m_icu, 20_MHz_XTAL / 2);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();

	int_callback<0>().set(m_icu, FUNC(ns32202_device::ir_w<0>));
	int_callback<1>().set(m_icu, FUNC(ns32202_device::ir_w<1>));
	int_callback<2>().set(m_icu, FUNC(ns32202_device::ir_w<3>));
	int_callback<3>().set(m_icu, FUNC(ns32202_device::ir_w<4>));
	int_callback<4>().set(m_icu, FUNC(ns32202_device::ir_w<6>));
	int_callback<5>().set(m_icu, FUNC(ns32202_device::ir_w<7>));
	int_callback<6>().set(m_icu, FUNC(ns32202_device::ir_w<8>));
	int_callback<7>().set([this](int state) { m_s8->read() ? m_cpu->set_input_line(INPUT_LINE_NMI, !state) : m_icu->ir_w<11>(state); });

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
}

template <unsigned ST> void cpuap_device::cpu_map(address_map &map)
{
	if (ST == 0)
	{
		map(0x000000, 0x0fffff).view(m_boot);

		m_boot[0](0x000000, 0x00ffff).rom().region("eprom", 0);
		m_boot[1](0x000000, 0x0fffff).ram();

		//map(0x100000, 0x3fffff); // first memory expansion
		//map(0x400000, 0x6fffff); // second memory expansion

		map(0x700000, 0xdfffff).w(FUNC(cpuap_device::bus_timeout));

		map(0xe00000, 0xefffff).rw(FUNC(cpuap_device::bus_mem_r), FUNC(cpuap_device::bus_mem_w)); // multibus mem
		map(0xf00000, 0xf0ffff).rw(FUNC(cpuap_device::bus_pio_r), FUNC(cpuap_device::bus_pio_w)); // multibus i/o

		//map(0xf10000, 0xfdffff); // unused
		map(0xfe0000, 0xfeffff).rom().region("eprom", 0);

		//map(0xff0000, 0xf7ffff); // unused
		map(0xff8000, 0xff803f).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
		map(0xff8100, 0xff8100).lr8([this]() { return m_s7->read(); }, "s7_r");
		map(0xff8200, 0xff8200).lw8([this](u8 data) { LOG("prdia_w 0x%02x led 0x%x (%s)\n", data, ~data & 0x3f, machine().describe_context()); m_prdia = data; }, "prdia_w");
		map(0xff8300, 0xff8300).lrw8([this]() { return m_poff; }, "poff_r", [this](u8 data) { m_poff = data & 3; }, "poff_w"); // 3 pohopofi - power off interrupt?
		//map(0xff8400, 0xff840f); // 4 csuart
		map(0xff8500, 0xff8500).lw8([this](u8 data) { m_boot.select(BIT(data, 0)); }, "mapprom_w");
		map(0xff8600, 0xff863f).m(m_icu, FUNC(ns32202_device::map<0>)).umask16(0x00ff);
		map(0xff8700, 0xff8700).lrw8(
			[this]() { return m_nmi; }, "nmi_r",
			[this](u8 data) { LOG("nmi_w 0x%02x (%s)\n", data, machine().describe_context()); m_nmi = 0xff; }, "nmi_w");
	}

	map(0xfffe00, 0xfffeff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>)).umask16(0x00ff);
}
