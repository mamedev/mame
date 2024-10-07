// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor Series 32000 DB32016 Development Board
 *
 * Sources:
 *  - Series 32000, DB32016 Development Board User's Manual, National Semiconductor Corporation May 1985 (420310111-001A)
 *
 * TODO:
 *  - clock/timer routing
 *  - nmi/reset switches, layout
 *  - multibus interface
 */

#include "emu.h"

// cpus and memory
#include "cpu/ns32000/ns32000.h"

// various hardware
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"

#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"

#include "machine/clock.h"
#include "machine/74157.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "speaker.h"
#include "imagedev/cassette.h"

#include "db32016.lh"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class ns32kdb_state : public driver_device
{
public:
	ns32kdb_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_mmu(*this, "mmu")
		, m_icu(*this, "icu")
		, m_pci(*this, "pci%u", 0U)
		, m_ppi(*this, "ppi")
		, m_pit(*this, "pit")
		, m_sdm(*this, "sdm%u", 0U)
		, m_cass(*this, "cassette")
		, m_cfg(*this, "S3")
		, m_led(*this, "DS%u", 1U)
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;

public:
	// machine config
	void db32016(machine_config &config);

protected:
	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;

	required_device_array<i8251_device, 2> m_pci;
	required_device<i8255_device> m_ppi;
	required_device<pit8253_device> m_pit;

	// serial port diagnostic multiplexers
	required_device_array<ls157_device, 2> m_sdm;
	required_device<cassette_image_device> m_cass;

	required_ioport m_cfg;
	output_finder<4> m_led;
private:
	u8 pa_r();
	void pb_w(u8);
};

void ns32kdb_state::machine_start()
{
	m_led.resolve();
}

void ns32kdb_state::machine_reset()
{
	m_led[0] = 0;
	m_led[1] = 0;
	m_led[2] = 0;
	m_led[3] = 0;

	// FIXME: tied to TCU bus timeout
	m_led[3] = 1;
}

template <unsigned ST> void ns32kdb_state::cpu_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("eprom", 0);

	map(0x008000, 0x027fff).ram();
	//map(0x028000, 0x7fffff); // off-board RAM
	//map(0x800000, 0x9fffff); // off-board I/O ports

	map(0xc00000, 0xc00003).rw(m_pci[0], FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);

	map(0xc00020, 0xc00027).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);

	map(0xc00030, 0xc00030).lr8([this]() { return m_cfg->read(); }, "cfg_r");
	map(0xc00032, 0xc00037).lw8([this](offs_t offset, u8 data) { m_led[2 - offset] = data; }, "led_w").umask16(0x00ff);
	map(0xc00038, 0xc00038).lw8([this](u8 data) { m_sdm[0]->select_w(data); m_sdm[1]->select_w(data); }, "sdm_w");

	map(0xc00040, 0xc00043).rw(m_pci[1], FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);

	map(0xc00050, 0xc00057).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);

	//map(0xc00060, 0xc0006f).umask16(0x00ff); // BLX J4 MCS0/
	//map(0xc00070, 0xc0007f).umask16(0x00ff); // BLX j4 MCS1/
	//map(0xc00060, 0xc0006f).umask16(0xff00); // BLX j4 MCS1

	//map(0xd00000, 0xd0ffff); // on-board ROM/EPROM expansion

	map(0xfffe00, 0xfffeff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>)).umask16(0x00ff);
}

static INPUT_PORTS_START(db32016)
	PORT_START("S3")

	PORT_DIPUNUSED_DIPLOC(0x80, 0x00, "S3:8")

	PORT_DIPNAME(0x40, 0x00, "PPI Test") PORT_DIPLOCATION("S3:7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))

	PORT_DIPNAME(0x20, 0x00, "MMU") PORT_DIPLOCATION("S3:6")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x20, DEF_STR(Off))

	PORT_DIPNAME(0x10, 0x00, "FPU") PORT_DIPLOCATION("S3:5")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x10, DEF_STR(Off))

	PORT_DIPNAME(0x0f, 0x01, "Baud Rate") PORT_DIPLOCATION("S3:4,3,2,1")
	PORT_DIPSETTING(0x00, "19200")
	PORT_DIPSETTING(0x01, "9600")
	PORT_DIPSETTING(0x02, "7200")
	PORT_DIPSETTING(0x03, "4800")
	PORT_DIPSETTING(0x04, "3600")
	PORT_DIPSETTING(0x05, "2400")
	PORT_DIPSETTING(0x06, "2000")
	PORT_DIPSETTING(0x07, "1800")
	PORT_DIPSETTING(0x08, "1200")
	PORT_DIPSETTING(0x09, "600")
	PORT_DIPSETTING(0x0a, "300")
	PORT_DIPSETTING(0x0b, "150")
	PORT_DIPSETTING(0x0c, "134")
	PORT_DIPSETTING(0x0d, "110")
	PORT_DIPSETTING(0x0e, "75")
	PORT_DIPSETTING(0x0f, "50")
INPUT_PORTS_END

void ns32kdb_state::pb_w(u8 data)
{
	m_cass->output(BIT(data, 0) ? 1.0 : -1.0);
}

u8 ns32kdb_state::pa_r()
{
	return (m_cass->input() > 0.03) ? 0xfe : 0xff;
}

void ns32kdb_state::db32016(machine_config &config)
{
	NS32016(config, m_cpu, 10_MHz_XTAL);
	m_cpu->set_addrmap(0, &ns32kdb_state::cpu_map<0>);
	m_cpu->set_addrmap(4, &ns32kdb_state::cpu_map<4>);

	NS32081(config, m_fpu, 10_MHz_XTAL);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 10_MHz_XTAL);
	m_cpu->set_mmu(m_mmu);

	NS32202(config, m_icu, 18.432_MHz_XTAL / 10);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(ns32kdb_state::pa_r));
	m_ppi->out_pb_callback().set(FUNC(ns32kdb_state::pb_w));

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(18.432_MHz_XTAL / 150);
	m_pit->set_clk<1>(18.432_MHz_XTAL / 15);
	m_pit->set_clk<2>(18.432_MHz_XTAL / 15);

	// HACK: serial clock should be configurable from ICU/pit
	clock_device &clk(CLOCK(config, "clock", 18.432_MHz_XTAL / 120));
	clk.signal_handler().set(m_pci[0], FUNC(i8251_device::write_txc));
	clk.signal_handler().append(m_pci[0], FUNC(i8251_device::write_rxc));
	clk.signal_handler().append(m_pci[1], FUNC(i8251_device::write_txc));
	clk.signal_handler().append(m_pci[1], FUNC(i8251_device::write_rxc));

	// serial port 0 is DCE
	I8251(config, m_pci[0], 18.432_MHz_XTAL / 10);
	rs232_port_device &j2(RS232_PORT(config, "j2", default_rs232_devices, "terminal"));
	LS157(config, m_sdm[0]);
	m_pci[0]->rts_handler().set(j2, FUNC(rs232_port_device::write_rts));
	m_pci[0]->rts_handler().append(m_sdm[0], FUNC(ls157_device::b0_w));
	m_pci[0]->txd_handler().set(j2, FUNC(rs232_port_device::write_txd));
	m_pci[0]->txd_handler().append(m_sdm[0], FUNC(ls157_device::b1_w));
	m_pci[0]->dtr_handler().set(j2, FUNC(rs232_port_device::write_dtr));

	j2.dsr_handler().set(m_pci[0], FUNC(i8251_device::write_dsr));
	j2.cts_handler().set(m_sdm[0], FUNC(ls157_device::a0_w));
	j2.rxd_handler().set(m_sdm[0], FUNC(ls157_device::a1_w));

	m_sdm[0]->out_callback().set(m_pci[0], FUNC(i8251_device::write_cts)).bit(0);
	m_sdm[0]->out_callback().append(m_pci[0], FUNC(i8251_device::write_rxd)).bit(1);
	//m_sdm[0]->out_callback().append(m_pci[0], FUNC(i8251_device::write_txc)).bit(2);
	//m_sdm[0]->out_callback().append(m_pci[0], FUNC(i8251_device::write_rxc)).bit(3);

	// serial port 1 is DTE
	I8251(config, m_pci[1], 18.432_MHz_XTAL / 10);
	rs232_port_device &j3(RS232_PORT(config, "j3", default_rs232_devices, nullptr));
	LS157(config, m_sdm[1]);
	m_pci[1]->rts_handler().set(j3, FUNC(rs232_port_device::write_rts));
	m_pci[1]->rts_handler().append(m_sdm[1], FUNC(ls157_device::b0_w));
	m_pci[1]->txd_handler().set(j3, FUNC(rs232_port_device::write_txd));
	m_pci[1]->txd_handler().append(m_sdm[1], FUNC(ls157_device::b1_w));
	m_pci[1]->dtr_handler().set(j3, FUNC(rs232_port_device::write_dtr));

	j3.cts_handler().set(m_sdm[1], FUNC(ls157_device::a0_w));
	j3.rxd_handler().set(m_sdm[1], FUNC(ls157_device::a1_w));

	m_sdm[1]->out_callback().set(m_pci[1], FUNC(i8251_device::write_cts)).bit(0);
	m_sdm[1]->out_callback().append(m_pci[1], FUNC(i8251_device::write_rxd)).bit(1);
	//m_sdm[1]->out_callback().append(m_pci[1], FUNC(i8251_device::write_txc)).bit(2);
	//m_sdm[1]->out_callback().append(m_pci[1], FUNC(i8251_device::write_rxc)).bit(3);

	// cassette
	SPEAKER(config, "mono").front_center();
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	config.set_default_layout(layout_db32016);
}

ROM_START(db32016)
	ROM_REGION16_LE(0x8000, "eprom", 0)

	ROM_SYSTEM_BIOS(0, "tds16", "Rev 2.00 24-NOV-83  DB32016  Version")
	ROMX_LOAD("007346__0025.u18", 0x0000, 0x4000, CRC(3d36eff5) SHA1(0a935e6299934a597e2ac24775cb1f082a38c8b3), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("007346__0018.u15", 0x0001, 0x4000, CRC(58ea003c) SHA1(62d81ff35c3eba8efa60d80326eb8264904676ec), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "1.1", "National DB16000 Monitor (Rev. 1.1) (tsang) Thu Sep 22 17:16:02 PDT 1983")
	ROMX_LOAD("u18.bin", 0x0000, 0x1000, CRC(05d0c876) SHA1(3e94589bbf30f41b0a704473ad15cffa08997f37), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("u15.bin", 0x0001, 0x1000, CRC(a9955f20) SHA1(2b9780f68c33ee72741472cde7104fb69baabc40), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(2, "v200", "VERSION_2.00_10-FEB-83")
	ROMX_LOAD("950308221_001__rev_b.u9",  0x0001, 0x1000, CRC(28036c3f) SHA1(d4942cabac779855936b1b448630699bf83768fd), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("950308221_002__rev_b.u11", 0x0000, 0x1000, CRC(e04edaf7) SHA1(e7173b916029c2f4b45c08e7b29b34c55e94c46f), ROM_BIOS(2) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(3, "vi03", "VERSION_I.03_25-DEC-81")
	ROMX_LOAD("6573_001.bin", 0x0001, 0x1000, CRC(fee2e343) SHA1(c8fa82a59372a304b62c5beb56b067dcb7a10c3c), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("6573_002.bin", 0x0000, 0x1000, CRC(623c76c5) SHA1(4d088c2175456536c1db886b81aec70360ca9991), ROM_BIOS(3) | ROM_SKIP(1))
ROM_END

}

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                   FULLNAME   FLAGS */
COMP(1984, db32016, 0,      0,      db32016, db32016, ns32kdb_state, empty_init, "National Semiconductor", "DB32016", MACHINE_NO_SOUND_HW)
