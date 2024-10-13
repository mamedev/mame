// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Data General AViiON M88k systems.
 *
 * Sources:
 *  - https://archive.org/details/Aviion530Docs/40020761
 *
 * TODO:
 *  - everything
 */

#include "emu.h"

// processors and memory
#include "cpu/m88000/m88000.h"
#include "machine/ram.h"
#include "machine/28fxxx.h"

// i/o devices
#include "machine/timekpr.h"
#include "machine/mc68681.h"
#include "machine/53c7xx.h"
#include "machine/scn_pci.h"
#include "machine/scnxx562.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"

#include "sound/spkrdev.h"
#include "speaker.h"

#include "debugger.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class aviion88k_state : public driver_device
{
public:
	aviion88k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_prom(*this, "prom%u", 0U)
		, m_ram(*this, "ram")
		, m_novram(*this, "novram")
		, m_uart(*this, "uart")
		, m_kbdc(*this, "kbdc")
		, m_duart(*this, "duart%u", 0U)
		, m_async(*this, { "console_port", "seriala", "mouse_port", "serialb" })
		, m_duscc(*this, "duscc")
		, m_scsibus(*this, "scsi")
		, m_scsi(*this, "scsi:7:ncr53c700")
		, m_speaker(*this, "speaker")
		, m_leds(*this, "CR%u", 1U)
		, m_mbus(*this, "mbus")
	{
	}

	// machine config
	void aviion_4600(machine_config &config);

	void init();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void cpu_map(address_map &map) ATTR_COLD;

	void pit_timer(s32 param) { LOG("pit_timer<%d> expired\n", param); }

	template <unsigned N> u32 pit_cnt_r() { return m_pit[N]->enabled() ? m_pit[N]->elapsed().as_ticks(m_cpu->clock()) : 0; }
	template <unsigned N> u32 pit_sts_r() { return m_pit_cmd[N]; }
	template <unsigned N> void pit_cnt_w(u32 data) { LOG("pit_cnt_w<%d> 0x%08x\n", N, data); m_pit_cnt[N] = data & 0xffffff00U; }
	template <unsigned N> void pit_cmd_w(u32 data)
	{
		LOG("pit_cmd_w<%d> 0x%x\n", N, data & 15); m_pit_cmd[N] = data & 15;

		// reset
		if (BIT(data, 0))
			m_pit[N]->adjust(attotime::from_ticks(-m_pit_cnt[N], m_cpu->clock()), N);

		// count enable
		if (BIT(data, 3) && !m_pit[N]->enabled())
			m_pit[N]->enable(true);
		else if (!BIT(data, 3) && m_pit[N]->enabled())
			m_pit[N]->enable(false);
	}

private:
	// processors and memory
	required_device<mc88100_device> m_cpu;
	required_device_array<intel_28f010_device, 4> m_prom;
	required_device<ram_device> m_ram;

	// i/o devices
	required_device<timekeeper_device> m_novram;
	required_device<scn_pci_device> m_uart;
	required_device<pc_kbdc_device> m_kbdc;
	required_device_array<scn2681_device, 2> m_duart;
	required_device_array<rs232_port_device, 4> m_async;
	required_device<duscc68562_device> m_duscc;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c7xx_device> m_scsi;
	required_device<speaker_sound_device> m_speaker;

	output_finder<3> m_leds;

	memory_view m_mbus;

	u32 m_ucs = 0;

	emu_timer *m_pit[4]{};

	u32 m_pit_cmd[4]{};
	u32 m_pit_cnt[4] = {};
};

void aviion88k_state::machine_start()
{
	m_leds.resolve();

	for (emu_timer *&pit : m_pit)
		pit = timer_alloc(FUNC(aviion88k_state::pit_timer), this);
}

void aviion88k_state::machine_reset()
{
	// disable mbus address decode
	m_mbus.select(0);

	m_ucs = 0b000'0110'0001'1011;
}

void aviion88k_state::init()
{
	// map the configured ram
	m_mbus[1].install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
}

void aviion88k_state::cpu_map(address_map &map)
{
	map(0x00000000, 0xffc7ffff).view(m_mbus);

	/*
	 * utility space (4M) 0xffc0'0000-ffff'ffff
	 *
	 * madv=0: only utility space available, mapped to 0x0000'0000-ffc7'ffff
	 * madv=1: utility space 0xffc7'0000-ffc7'ffff
	 */
	// mbus address decode disabled
	m_mbus[0](0x00000000, 0x0007ffff).rw(m_prom[0], FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).mirror(0xffc00000).umask32(0xff000000);
	m_mbus[0](0x00000000, 0x0007ffff).rw(m_prom[1], FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).mirror(0xffc00000).umask32(0x00ff0000);
	m_mbus[0](0x00000000, 0x0007ffff).rw(m_prom[2], FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).mirror(0xffc00000).umask32(0x0000ff00);
	m_mbus[0](0x00000000, 0x0007ffff).rw(m_prom[3], FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).mirror(0xffc00000).umask32(0x000000ff);

	// mbus address decode enabled
	m_mbus[1](0xffc00000, 0xffc7ffff).rw(m_prom[0], FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).umask32(0xff000000);
	m_mbus[1](0xffc00000, 0xffc7ffff).rw(m_prom[1], FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).umask32(0x00ff0000);
	m_mbus[1](0xffc00000, 0xffc7ffff).rw(m_prom[2], FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).umask32(0x0000ff00);
	m_mbus[1](0xffc00000, 0xffc7ffff).rw(m_prom[3], FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).umask32(0x000000ff);

	map(0xfff8'0000, 0xfff8'1fff).rw(m_novram, FUNC(mk48t12_device::read), FUNC(mk48t12_device::write)).umask32(0x000000ff);
	map(0xfff8'2000, 0xfff8'203f).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
	map(0xfff8'2040, 0xfff8'207f).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
	map(0xfff8'2800, 0xfff8'280f).rw(m_uart, FUNC(scn2661a_device::read), FUNC(scn2661a_device::write)).umask32(0x000000ff);

	map(0xfff8'3100, 0xfff8'3103).lw32(
		[this](u32 data)
		{
			if (!BIT(data, 0))
				m_duart[0]->reset();
			if (!BIT(data, 1))
				m_duart[1]->reset();

			// reset the keyboard or the uart?
			if (!BIT(data, 3))
			{
				LOG("uart reset\n");
				m_uart->reset();
			}
		}, "srst_w");

	map(0xfff8'7000, 0xfff8'7003).lr32([this]() { return m_ucs; }, "ucs_r");
	map(0xfff8'8000, 0xfff8'8003).lw32([this](u32 data) { LOG("madv %d\n", BIT(data, 1)); if (BIT(data, 1)) m_mbus.select(BIT(data, 1)); }, "ccs_w");
	map(0xfff8'8018, 0xfff8'801b).lr32([]() { return 0xa1; }, "whoami_r");

	map(0xfff8'f004, 0xfff8'f007).rw(FUNC(aviion88k_state::pit_cnt_r<0>), FUNC(aviion88k_state::pit_cnt_w<0>));
	map(0xfff8'f008, 0xfff8'f00b).rw(FUNC(aviion88k_state::pit_cnt_r<1>), FUNC(aviion88k_state::pit_cnt_w<1>));
	map(0xfff8'f010, 0xfff8'f013).rw(FUNC(aviion88k_state::pit_cnt_r<2>), FUNC(aviion88k_state::pit_cnt_w<2>));
	map(0xfff8'f020, 0xfff8'f023).rw(FUNC(aviion88k_state::pit_cnt_r<3>), FUNC(aviion88k_state::pit_cnt_w<3>));
	map(0xfff8'f044, 0xfff8'f047).rw(FUNC(aviion88k_state::pit_sts_r<0>), FUNC(aviion88k_state::pit_cmd_w<0>));
	map(0xfff8'f048, 0xfff8'f04b).rw(FUNC(aviion88k_state::pit_sts_r<1>), FUNC(aviion88k_state::pit_cmd_w<1>));
	map(0xfff8'f050, 0xfff8'f053).rw(FUNC(aviion88k_state::pit_sts_r<2>), FUNC(aviion88k_state::pit_cmd_w<2>));
	map(0xfff8'f060, 0xfff8'f063).rw(FUNC(aviion88k_state::pit_sts_r<3>), FUNC(aviion88k_state::pit_cmd_w<3>));

	map(0xfff8'ff00, 0xfff8'ff03).lrw32(
		[]() { return 0b1110'0000'0001'1000; }, "mds_r",
		[](u32 data) {}, "mcs_w");
}

static void aviion88k_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void aviion88k_state::aviion_4600(machine_config &config)
{
	MC88100(config, m_cpu, 33'333'333);
	m_cpu->set_addrmap(AS_PROGRAM, &aviion88k_state::cpu_map);

	INTEL_28F010(config, m_prom[0]);
	INTEL_28F010(config, m_prom[1]);
	INTEL_28F010(config, m_prom[2]);
	INTEL_28F010(config, m_prom[3]);

	// 8 SIMM slots (populated with pairs of 4M or 16M modules)
	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("16M,24M,32M,40M,48M,56M,64M,72M,80M,96M,104M,128M");
	m_ram->set_default_value(0);

	MK48T12(config, m_novram);

	// uart - keyboard interface
	SCN2661A(config, m_uart, 0);

	// keyboard connector
	PC_KBDC(config, m_kbdc, pc_at_keyboards, nullptr);
	m_kbdc->out_clock_cb().set(m_uart, FUNC(scn2661a_device::rxc_w));
	m_kbdc->out_data_cb().set(m_uart, FUNC(scn2661a_device::rxd_w));

	// duart 1
	SCN2681(config, m_duart[0], 14.7456_MHz_XTAL / 4); // SCC2692
	RS232_PORT(config, m_async[0], default_rs232_devices, "terminal"); // console: DCD,RXD,TXD,DTR,RTS,CTS
	RS232_PORT(config, m_async[1], default_rs232_devices, nullptr); // async A: DCD,RXD,TXD,DTR,DSR,RTS,CTS,RI

	m_duart[0]->a_tx_cb().set(m_async[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_async[1], FUNC(rs232_port_device::write_txd));
	m_async[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_async[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));

	// duart 2
	SCN2681(config, m_duart[1], 14.7456_MHz_XTAL / 4); // SCC2692
	RS232_PORT(config, m_async[2], default_rs232_devices, nullptr); // mouse: RTS,DTR,TXD,RXD
	RS232_PORT(config, m_async[3], default_rs232_devices, nullptr); // async B: DCD,RXD,TXD,DTR,DSR,RTS,CTS,RI

	m_duart[1]->a_tx_cb().set(m_async[2], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_async[3], FUNC(rs232_port_device::write_txd));
	m_async[2]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_async[3]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));

	// duscc
	DUSCC68562(config, m_duscc, 14.7456_MHz_XTAL);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", aviion88k_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", aviion88k_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", aviion88k_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", aviion88k_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", aviion88k_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", aviion88k_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", aviion88k_scsi_devices, nullptr);

	// scsi host adapter (NCR53C700)
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53c700", NCR53C7XX).clock(66'000'000);

	// TODO: ethernet (AM79C900)

	// speaker
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START(aviion_4600)
	ROM_REGION(0x20000, "prom0", 0)
	ROM_LOAD("11513__x02__92-05.bin", 0, 0x20000, CRC(7031d7d4) SHA1(c1ca7567b764b7f48e53b9bc8df40407464f9f67))
	ROM_REGION(0x20000, "prom1", 0)
	ROM_LOAD("11514__x02__92-06.bin", 0, 0x20000, CRC(4fcf85e6) SHA1(9afeec63cf8098d4518dc0712ba92614d44cd859))
	ROM_REGION(0x20000, "prom2", 0)
	ROM_LOAD("11515__x02__92-05.bin", 0, 0x20000, CRC(c9ce39d7) SHA1(fbdd3287b9f9eb6a621d7c10d900ccaff02660c5))
	ROM_REGION(0x20000, "prom3", 0)
	ROM_LOAD("11516__x02__92-05.bin", 0, 0x20000, CRC(71b6d338) SHA1(eb85bd16a25b6cd790272f007b8117fcf13b6b40))
ROM_END

} // anonymous namespace

/*   YEAR   NAME         PARENT  COMPAT  MACHINE      INPUT  CLASS            INIT  COMPANY         FULLNAME       FLAGS */
COMP(1991,  aviion_4600, 0,      0,      aviion_4600, 0,     aviion88k_state, init, "Data General", "AViiON 4600", MACHINE_IS_SKELETON)
