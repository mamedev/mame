// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics Indigo R4000/R4400 aka 4DRPC-50/Blackjack/HP2 (also VME-based V50)
 *
 * INT2    local I/O interrupt multiplexor
 * HPC1.5  high-performance peripheral controller
 * MC      memory controller
 *
 */
 /* board revs of IP20 are: 1 & 2 were early blackjack spins
  * 10 decimal == 0x0a == 1010binary == VME IP 20 board (V50)
  * V50 board lacks audio hardware
  * The upper bit used to change the standard (rev 2) board
  * to the V50 designation (i.e the 0x8 bit) used to be known
  * internally as the 'Hollywood processor 1 bit' or hp1 bit
  */

#include "emu.h"
#include "cpu/mips/r4000.h"
#include "cpu/dsp56000/dsp56000.h"

#include "machine/dp8573a.h"
#include "machine/edlc.h"
#include "machine/eepromser.h"
#include "machine/input_merger.h"
#include "machine/nscsi_bus.h"
#include "machine/wd33c9x.h"
#include "machine/z80scc.h"

#include "bus/rs232/rs232.h"
#include "bus/rs232/hlemouse.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"

#include "hpc1.h"
#include "int2.h"
#include "kbd.h"
#include "light.h"
#include "mc.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class ip20_state : public driver_device
{
public:
	ip20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_eerom(*this, "eerom")
		, m_mc(*this, "mc")
		, m_hpc(*this, "hpc")
		, m_int(*this, "int")
		, m_nvram(*this, "nvram")
		, m_rtc(*this, "rtc")
		, m_scsi(*this, "scsi:0:wd33c93a")
		, m_eth(*this, "eth")
		, m_scc(*this, "scc%u", 0U)
		, m_dsp(*this, "dsp")
		, m_gfx(*this, "gfx")
	{
	}

	void indigo_r4000(machine_config &config);
	void indigo_r4400(machine_config &config);

protected:
	void ip20(machine_config &config);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cpu_map(address_map &map) ATTR_COLD;

	required_device<r4000_base_device> m_cpu;
	required_device<eeprom_serial_93cxx_device> m_eerom;
	required_device<sgi_mc_device> m_mc;
	required_device<hpc1_device> m_hpc;
	required_device<sgi_int2_device> m_int;
	required_device<eeprom_serial_93cxx_device> m_nvram;
	required_device<dp8572a_device> m_rtc;
	required_device<wd33c93a_device> m_scsi;
	required_device<seeq8003_device> m_eth;
	required_device_array<scc85c30_device, 3> m_scc;
	required_device<dsp56001_device> m_dsp;
	required_device<sgi_lg1_device> m_gfx;
};

void ip20_state::machine_start()
{
}

void ip20_state::machine_reset()
{
}

void ip20_state::cpu_map(address_map &map)
{
	//map(0x1fbd9000, 0x1fbd903f).rw(FUNC(indigo_state::int_r), FUNC(indigo_state::int_w));

	//map(0x0000'0000, 0x0007'ffff); // system memory alias (512K)
	//map(0x0008'0000, 0x0008'ffff); // EISA I/O space (64K)
	//map(0x0009'0000, 0x0009'ffff); // EISA I/O space alias (64K)
	//map(0x000a'0000, 0x07ff'ffff); // EISA memory (128M)
	//map(0x0800'0000, 0x17ff'ffff); // physical memory segment 0 (256M)
	//map(0x1800'0000, 0x1eff'ffff); // reserved (future GIO space) (112M)
	//map(0x1f00'0000, 0x1f3f'ffff); // graphics system (4M)
	//map(0x1f40'0000, 0x1f5f'ffff); // gio64 expansion slot 0 (2M)
	//map(0x1f60'0000, 0x1f9f'ffff); // gio64 expansion slot 1 (4M)
	//map(0x1fa0'0000, 0x1faf'ffff); // mc registers (1M)
	//map(0x1fb0'0000, 0x1fbf'ffff); // hpc and i/o devices (1M)
	//map(0x1fc0'0000, 0x1fff'ffff); // boot prom (4M)
	//map(0x2000'0000, 0x2fff'ffff); // physical memory segment 1 (256M)
	//map(0x3000'0000, 0x7fff'ffff); // reserved (1.25G)
	//map(0x8000'0000, 0xffff'ffff); // EISA memory (2G)

	//map(0x1000'0000, 0x1eff'ffff).lr32([this]() { m_cpu->berr_w(1); return 0; }, "buserror"); // vme

	map(0x1f3f'0000, 0x1f3f'7fff).m(m_gfx, FUNC(sgi_lg1_device::map));

	map(0x1fa0'0000, 0x1fa1'ffff).rw(m_mc, FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));

	map(0x1fb8'0000, 0x1fb8'ffff).m(m_hpc, FUNC(hpc1_device::map));
	map(0x1fb8'0100, 0x1fb8'011f).m(m_eth, FUNC(seeq8003_device::map)).umask32(0xff);
	map(0x1fb8'0120, 0x1fb8'0127).rw(m_scsi, FUNC(wd33c93a_device::indir_r), FUNC(wd33c93a_device::indir_w)).umask32(0xff00);
	map(0x1fb8'01c0, 0x1fb8'01ff).m(m_int, FUNC(sgi_int2_device::map)).umask32(0xff);
	map(0x1fb8'0d00, 0x1fb8'0d0f).rw(m_scc[0], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w)).umask32(0xff);
	map(0x1fb8'0d10, 0x1fb8'0d1f).rw(m_scc[1], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w)).umask32(0xff);
	//map(0x1fb8'0d20, 0x1fb8'0d2f).rw(m_scc[2], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w)).umask32(0xff); // V50 only?
	map(0x1fb8'0d30, 0x1fb8'0d37).noprw(); // TODO: left/right headphone gain mdacs
	map(0x1fb8'0e00, 0x1fb8'0e7f).rw(m_rtc, FUNC(dp8572a_device::read), FUNC(dp8572a_device::write)).umask32(0xff);
	map(0x1fbd'0000, 0x1fbd'0003).lr32([]() { return 0x0000'8000; }, "board_rev"); // board revision register

	map(0x1fbe'0000, 0x1fbf'ffff).ram().share("dsp_ram"); // 3xTC55328J-35 (24 bits x 32k)

	map(0x1fc0'0000, 0x1fc7'ffff).rom().region("prom", 0);
}

static DEVICE_INPUT_DEFAULTS_START(ip20_mc)
	DEVICE_INPUT_DEFAULTS("VALID", 0x0f, 0x07)
DEVICE_INPUT_DEFAULTS_END

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_SGI).machine_config(
		[](device_t *device)
		{
			downcast<nscsi_cdrom_device &>(*device).set_block_size(512);
		});
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void ip20_state::indigo_r4000(machine_config &config)
{
	R4000(config, m_cpu, 50'000'000);

	ip20(config);
}

void ip20_state::indigo_r4400(machine_config &config)
{
	R4400(config, m_cpu, 75'000'000);

	ip20(config);
}

void ip20_state::ip20(machine_config &config)
{
	m_cpu->set_addrmap(AS_PROGRAM, &ip20_state::cpu_map);

	EEPROM_93C56_16BIT(config, m_eerom);

	SGI_MC(config, m_mc, m_cpu, m_eerom, 50'000'000);
	m_mc->eisa_present().set_constant(0);
	m_mc->set_input_default(DEVICE_INPUT_DEFAULTS_NAME(ip20_mc));

	SGI_HPC1(config, m_hpc, 0);
	m_hpc->set_gio(m_cpu, AS_PROGRAM);
	m_hpc->set_enet(m_eth);
	m_hpc->int_w().set(m_int, FUNC(sgi_int2_device::lio0_w<sgi_int2_device::LIO0_ETHERNET>));
	m_hpc->dma_r_cb<0>().set(m_scsi, FUNC(wd33c93a_device::dma_r));
	m_hpc->dma_w_cb<0>().set(m_scsi, FUNC(wd33c93a_device::dma_w));
	m_hpc->eeprom_dati().set(m_nvram, FUNC(eeprom_serial_93cxx_device::do_read));
	m_hpc->eeprom_out().set(
		[this](u8 data)
		{
			// TODO: bit 0 console led
			m_nvram->di_write(BIT(data, 3));
			m_nvram->cs_write(BIT(data, 1));
			m_nvram->clk_write(BIT(data, 2));
		});

	SGI_INT2(config, m_int, 10_MHz_XTAL);
	m_int->write_intr<1>().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	m_int->write_intr<2>().set_inputline(m_cpu, INPUT_LINE_IRQ1);
	// TODO: write_led and write_poweroff outputs
	//
	//  0   cache parity error?
	//  1   1Hz heartbeat
	//  2   off when cpu is idle
	//  3   graphics
	//  4   poweroff?

	EEPROM_93C56_16BIT(config, m_nvram);

	DP8572A(config, m_rtc, 32.768_kHz_XTAL).set_use_utc(true);

	NSCSI_BUS(config, "scsi", 0);
	NSCSI_CONNECTOR(config, "scsi:0").option_set("wd33c93a", WD33C93A).machine_config(
		[this](device_t *device)
		{
			wd33c93a_device &scsi = downcast<wd33c93a_device &>(*device);

			scsi.set_clock(10_MHz_XTAL);
			scsi.irq_cb().set(m_int, FUNC(sgi_int2_device::lio0_w<sgi_int2_device::LIO0_SCSI>));
			scsi.drq_cb().set(m_hpc, FUNC(hpc1_device::write_drq<0>));
		});
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, "cdrom", false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, nullptr, false);

	SEEQ8003(config, m_eth, 0);
	m_eth->out_int_cb().set(m_hpc, FUNC(hpc1_device::write_int));
	m_eth->out_rxrdy_cb().set(m_hpc, FUNC(hpc1_device::write_drq<1>));
	m_hpc->dma_r_cb<1>().set(m_eth, FUNC(seeq8003_device::fifo_r));
	m_hpc->dma_w_cb<1>().set(m_eth, FUNC(seeq8003_device::fifo_w));

	input_merger_any_high_device &scc_irq(INPUT_MERGER_ANY_HIGH(config, "scc_irq"));
	scc_irq.output_handler().set(m_int, FUNC(sgi_int2_device::lio0_w<sgi_int2_device::LIO0_DUART>));

	// duart 0
	SCC85C30(config, m_scc[0], 10_MHz_XTAL);
	m_scc[0]->configure_channels(3'686'400, 0, 3'686'400, 0);
	m_scc[0]->out_int_callback().set(scc_irq, FUNC(input_merger_any_high_device::in_w<0>));

	sgi_kbd_port_device &kbd_port(SGI_KBD_PORT(config, "keyboard_port"));
	kbd_port.option_set("keyboard", SGI_KBD);
	rs232_port_device &mouse_port(RS232_PORT(config, "mouse_port", 0));
	mouse_port.option_set("mouse", SGI_HLE_SERIAL_MOUSE);
	m_scc[0]->out_txda_callback().set(kbd_port, FUNC(sgi_kbd_port_device::write_txd));
	kbd_port.rxd_handler().set(m_scc[0], FUNC(scc85c30_device::rxa_w));
	mouse_port.rxd_handler().set(m_scc[0], FUNC(scc85c30_device::rxb_w));

	// duart 1
	SCC85C30(config, m_scc[1], 10_MHz_XTAL);
	m_scc[1]->configure_channels(3'686'400, 0, 3'686'400, 0);
	m_scc[1]->out_int_callback().set(scc_irq, FUNC(input_merger_any_high_device::in_w<1>));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	m_scc[1]->out_txda_callback().set(rs232a, FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtra_callback().set(rs232a, FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsa_callback().set(rs232a, FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_txdb_callback().set(rs232b, FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtrb_callback().set(rs232b, FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsb_callback().set(rs232b, FUNC(rs232_port_device::write_rts));
	rs232a.cts_handler().set(m_scc[1], FUNC(scc85c30_device::ctsa_w));
	rs232a.dcd_handler().set(m_scc[1], FUNC(scc85c30_device::dcda_w));
	rs232a.rxd_handler().set(m_scc[1], FUNC(scc85c30_device::rxa_w));
	rs232b.cts_handler().set(m_scc[1], FUNC(scc85c30_device::ctsb_w));
	rs232b.dcd_handler().set(m_scc[1], FUNC(scc85c30_device::dcdb_w));
	rs232b.rxd_handler().set(m_scc[1], FUNC(scc85c30_device::rxb_w));

	// duart 2
	SCC85C30(config, m_scc[2], 10_MHz_XTAL);
	m_scc[2]->configure_channels(3'686'400, 0, 3'686'400, 0);
	m_scc[2]->out_int_callback().set(scc_irq, FUNC(input_merger_any_high_device::in_w<2>));

	DSP56001(config, m_dsp, 20_MHz_XTAL);

	SGI_LG1(config, m_gfx);
	m_gfx->write_vblank().set(m_int, FUNC(sgi_int2_device::lio1_w<sgi_int2_device::LIO1_GIO2>));
}

ROM_START(indigo_r4000)
	ROM_REGION64_BE(0x80000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "405g-rev-b", "SGI Version 4.0.5G Rev B IP20, Nov 10, 1992") // dumped over serial connection from boot monitor and swapped
	ROMX_LOAD("ip20prom.070-8116-005.bin", 0x000000, 0x080000, CRC(1875b645) SHA1(52f5d7baea3d1bc720eb2164104c177e23504345), ROM_GROUPDWORD | ROM_REVERSE | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "405d-rev-a", "SGI Version 4.0.5D Rev A IP20, Aug 19, 1992")
	ROMX_LOAD("ip20prom.070-8116-004.bin", 0x000000, 0x080000, CRC(940d960e) SHA1(596aba530b53a147985ff3f6f853471ce48c866c), ROM_GROUPDWORD | ROM_REVERSE | ROM_BIOS(1))

	// hand-made content sets eaddr 08:00:69:12:34:56 and netaddr 192.168.137.2
	ROM_REGION16_LE(0x100, "nvram", 0)
	ROM_LOAD("nvram.bin", 0x000, 0x100, CRC(b8367798) SHA1(61af4c9dba69e9f0552c10770f16044421730b6d))
ROM_END

#define rom_indigo_r4400 rom_indigo_r4000
} // anonymous namespace

//   YEAR  NAME          PARENT  COMPAT  MACHINE       INPUT  CLASS       INIT        COMPANY             FULLNAME             FLAGS
COMP(1992, indigo_r4000, 0,      0,      indigo_r4000, 0,     ip20_state, empty_init, "Silicon Graphics", "IRIS Indigo R4000", MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS)
COMP(1991, indigo_r4400, 0,      0,      indigo_r4400, 0,     ip20_state, empty_init, "Silicon Graphics", "IRIS Indigo R4400", MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS)
