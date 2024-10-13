// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics IP12 systems.
 *
 *   Year  Model   Board  Type  CPU    Clock  I/D Cache    Code Name(s)
 *   1991  4D/30   IP14   IP12  R3000  30MHz  64KiB/64KiB  Magnum
 *   1991  4D/35   IP12   IP12  R3000  36MHz  64KiB/64KiB  Magnum
 *   1991  Indigo  IP12   IP12  R3000  33MHz  32KiB/32KiB  Hollywood, 4DRPC, HP1
 *
 * TODO:
 *  - VME-based V30/V35
 *  - DSP
 *
 */

/*
 * WIP
 * ---
 * setenv bootmode d  # enable diagnostic output (bootmode c is normal)
 *
 * ide usage: report=5; <test>
 *
 * installing IRIX 4.0.5/5.3 (IRIX CDROM at SCSI ID 4):
 *  - boot -f dksc(0,4,8)sashIP12
 *  - boot -f dksc(0,4,7)stand/fx.IP12 --x
 *  - create root partition and label disk
 *  - use firmware option 2 and inst to install OS from CDROM
 *
 * after installation, disable windowing system (until graphics work):
 *  - shroot (or sh and prefix commands with chroot /root)
 *  - /etc/chkconfig -f windowsystem off
 *  - /etc/chkconfig -f xdm off
 *  - for IRIX 5.3, also edit /etc/inittab to spawn getty on console
 *
 */

#include "emu.h"
#include "cpu/mips/mips1.h"
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
#include "pic1.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class ip12_state : public driver_device
{
public:
	ip12_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_pic(*this, "pic")
		, m_hpc(*this, "hpc")
		, m_int(*this, "int")
		, m_nvram(*this, "nvram")
		, m_rtc(*this, "rtc")
		, m_scsi(*this, "scsi:0:wd33c93a")
		, m_eth(*this, "eth")
		, m_scc(*this, "scc%u", 0U)
		, m_scc_irq(*this, "scc_irq")
		, m_dsp(*this, "dsp")
		, m_gfx(*this, "gfx")
	{
	}

	void indigo(machine_config &config);
	void pi4d30(machine_config &config);
	void pi4d35(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void ip12(machine_config &config);
	void pi4d3x(machine_config &config);

	void ip12_map(address_map &map) ATTR_COLD;
	void pi4d3x_map(address_map &map) ATTR_COLD;
	void pbus_map(address_map &map) ATTR_COLD;

	required_device<r3000a_device> m_cpu;
	required_device<sgi_pic1_device> m_pic;
	required_device<hpc1_device> m_hpc;
	required_device<sgi_int2_device> m_int;
	required_device<eeprom_serial_93cxx_device> m_nvram;
	required_device<dp8572a_device> m_rtc;
	required_device<wd33c93a_device> m_scsi;
	required_device<seeq8003_device> m_eth;
	required_device_array<scc85c30_device, 3> m_scc;
	required_device<input_merger_any_high_device> m_scc_irq;
	required_device<dsp56001_device> m_dsp;
	required_device<sgi_lg1_device> m_gfx;

	/*
	 * board revisions according to NetBSD source:
	 *   0x0000-0x6000 -> IP12 4D/3x
	 *   0x7000        -> IP12 VIP12
	 *   0x8000-0xd000 -> IP12 HP1
	 *   0xe000-0xf000 -> IP12 HPLC
	 */
	u32 m_brdrev;
};

void ip12_state::machine_start()
{
}

void ip12_state::machine_reset()
{
}

void ip12_state::ip12_map(address_map &map)
{
	// 0x0000'0000-0fff'ffff // local memory
	// 0x1000'0000-1eff'ffff // vme
	// 0x1f00'0000-1fbf'ffff // local i/o
	// 0x1fc0'0000-1fff'ffff // boot prom

	// 0x2000'0000-3fff'ffff // vme (normally inaccessible)
	// 0x4000'0000-ffff'ffff // unused (normally inaccessible), bus error (read) interrupt (write)

	map(0x1000'0000, 0x1eff'ffff).lr32([this]() { m_cpu->berr_w(1); return 0; }, "buserror"); // vme

	map(0x1f3f'0000, 0x1f3f'7fff).m(m_gfx, FUNC(sgi_lg1_device::map));
	// 1f3f'8000-1f3f'ffff // lg2x2 second head

	// 0x1f40'0000-0x1f5'ffff gio slot 0
	// 0x1f60'0000-0x1f7'ffff gio slot 1

	//map(0x1f90'0000, 0x1f97'ffff); // hpc 3
	//map(0x1f98'0000, 0x1f9f'ffff); // hpc 2
	map(0x1fa0'0000, 0x1faf'ffff).m(m_pic, FUNC(sgi_pic1_device::map));
	//map(0x1fb0'0000, 0x1fb7'ffff); // hpc 1
	//map(0x1fb8'0000, 0x1fbf'ffff); // hpc 0

	map(0x1fb8'0000, 0x1fb8'ffff).m(m_hpc, FUNC(hpc1_device::map));
	map(0x1fb8'0100, 0x1fb8'011f).m(m_eth, FUNC(seeq8003_device::map)).umask32(0xff);
	map(0x1fb8'0120, 0x1fb8'0127).rw(m_scsi, FUNC(wd33c93a_device::indir_r), FUNC(wd33c93a_device::indir_w)).umask32(0xff00);
	map(0x1fb8'01c0, 0x1fb8'01ff).m(m_int, FUNC(sgi_int2_device::map)).umask32(0xff);
	map(0x1fb8'0d00, 0x1fb8'0d0f).rw(m_scc[0], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w)).umask32(0xff);
	map(0x1fb8'0d10, 0x1fb8'0d1f).rw(m_scc[1], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w)).umask32(0xff);
	map(0x1fb8'0d30, 0x1fb8'0d3f).noprw(); // TODO: left/right headphone gain mdacs
	map(0x1fb8'0e00, 0x1fb8'0e7f).rw(m_rtc, FUNC(dp8572a_device::read), FUNC(dp8572a_device::write)).umask32(0xff);

	map(0x1fbd'0000, 0x1fbd'0003).lr32([this]() { return m_brdrev; }, "board_rev"); // board revision register

	map(0x1fbe'0000, 0x1fbf'ffff).ram().share("dsp_ram"); // 3xTC55328J-35 (24 bits x 32k)

	map(0x1fc0'0000, 0x1fc3'ffff).rom().region("prom", 0);
}

void ip12_state::pi4d3x_map(address_map &map)
{
	ip12_map(map);

	map(0x1fb8'0d20, 0x1fb8'0d2f).rw(m_scc[2], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w)).umask32(0xff);
}

// HPC1 PBUS decode?
// 00100-0011f == 00040-00047 -> net
// 00120-00127 == 00048-00049 -> scsi
// 001c0-001ff == 00070-0007f -> int2
// 00d00-00d3f == 00340-0034f -> fff0-ffff  duart
// 00e00-00e7f == 00380-0039f -> ffc0-ffdf  rtc
// 60000-7ffff == 18000-1ffff -> 8000-ffff? ram
//
// Actel/AUD1 and audio adc/dac via DSP SCI
//
void ip12_state::pbus_map(address_map &map)
{
	map(0xffc0, 0xffdf).rw(m_rtc, FUNC(dp8572a_device::read), FUNC(dp8572a_device::write));

	map(0xfff0, 0xfff3).rw(m_scc[0], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w));
	map(0xfff4, 0xfff7).rw(m_scc[1], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w));
	map(0xfff8, 0xfffb).rw(m_scc[2], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w));
	//map(0xfffc, 0xffff).rw(m_scc[3], FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w));
}

static DEVICE_INPUT_DEFAULTS_START(pi4d3x_pic1)
	DEVICE_INPUT_DEFAULTS("VALID", 0x0f, 0x0f)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(indigo_pic1)
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

void ip12_state::ip12(machine_config &config)
{
	SGI_PIC1(config, m_pic, 0);
	m_pic->set_bus(m_cpu, AS_PROGRAM);

	SGI_HPC1(config, m_hpc, 0);
	m_hpc->set_addrmap(0, &ip12_state::pbus_map);
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
	m_int->write_intr<1>().set_inputline(m_cpu, INPUT_LINE_IRQ1);
	m_int->write_intr<2>().set_inputline(m_cpu, INPUT_LINE_IRQ2);
	m_int->write_intr<3>().set_inputline(m_cpu, INPUT_LINE_IRQ3);
	m_int->write_intr<4>().set_inputline(m_cpu, INPUT_LINE_IRQ4);
	m_int->write_intr<5>().set_inputline(m_cpu, INPUT_LINE_IRQ5);
	// TODO: write_led and write_poweroff outputs
	//
	//  0  scc.clk.sel      external clock on for hp1 port 0
	//  1  ser0.sel         rts/tx+ for hp1 port 0 (rts=1)
	//  2  ser1.sel         rts/tx+ for hp1 port 1 (rts=1)
	//  3  clr.retrace-     disable/enable vert. retrace intr.
	//  4  poweroff

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

	// 24.576MHz
	// 22.5792MHz
	// 3.6720MHz
	input_merger_any_high_device &scc_irq(INPUT_MERGER_ANY_HIGH(config, "scc_irq"));
	scc_irq.output_handler().set(m_int, FUNC(sgi_int2_device::lio0_w<sgi_int2_device::LIO0_DUART>));

	// duart 0: keyboard/mouse ports
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

	// duart 1: serial ports
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

	// duart 2: "Apple" RS-422 serial ports (4D/PI only)
	SCC85C30(config, m_scc[2], 10_MHz_XTAL); // Z8513010VSC ESCC
	m_scc[2]->configure_channels(3'686'400, 0, 3'686'400, 0);
	m_scc[2]->out_int_callback().set(scc_irq, FUNC(input_merger_any_high_device::in_w<1>));

	DSP56001(config, m_dsp, 20_MHz_XTAL);
	//m_dsp->moda_w(1);
	//m_dsp->modb_w(0);

	SGI_LG1(config, m_gfx);
	m_gfx->write_vblank().set(m_int, FUNC(sgi_int2_device::lio1_w<sgi_int2_device::LIO1_GIO2>));
}

void ip12_state::pi4d3x(machine_config &config)
{
	ip12(config);

	m_pic->set_input_default(DEVICE_INPUT_DEFAULTS_NAME(pi4d3x_pic1));

	// serial port 3
	// FIXME: HSKO/HSKI/GPI
	rs232_port_device &rs232c(RS232_PORT(config, "rs232c", default_rs232_devices, nullptr));
	m_scc[2]->out_txda_callback().set(rs232c, FUNC(rs232_port_device::write_txd));
	rs232c.rxd_handler().set(m_scc[2], FUNC(z80scc_device::rxa_w));

	// serial port 4
	// FIXME: HSKO/HSKI/GPI
	rs232_port_device &rs232d(RS232_PORT(config, "rs232d", default_rs232_devices, nullptr));
	m_scc[2]->out_txdb_callback().set(rs232d, FUNC(rs232_port_device::write_txd));
	rs232d.rxd_handler().set(m_scc[2], FUNC(z80scc_device::rxb_w));
}

void ip12_state::indigo(machine_config &config)
{
	R3000A(config, m_cpu, 66.0_MHz_XTAL / 2, 32768, 32768);
	m_cpu->set_addrmap(AS_PROGRAM, &ip12_state::ip12_map);
	// FIXME: route fpu interrupt via int2
	m_cpu->set_fpu(r3000a_device::MIPS_R3010A, INPUT_LINE_IRQ0);

	ip12(config);

	m_pic->set_input_default(DEVICE_INPUT_DEFAULTS_NAME(indigo_pic1));

	m_brdrev = 0x8000;
}

void ip12_state::pi4d30(machine_config &config)
{
	R3000A(config, m_cpu, 30_MHz_XTAL, 65536, 65536);
	m_cpu->set_addrmap(AS_PROGRAM, &ip12_state::pi4d3x_map);
	// FIXME: route fpu interrupt via int2
	m_cpu->set_fpu(r3000a_device::MIPS_R3010A, INPUT_LINE_IRQ0);

	pi4d3x(config);

	m_brdrev = 0x0000;
}

void ip12_state::pi4d35(machine_config &config)
{
	R3000A(config, m_cpu, 36_MHz_XTAL, 65536, 65536);
	m_cpu->set_addrmap(AS_PROGRAM, &ip12_state::pi4d3x_map);
	// FIXME: route fpu interrupt via int2
	m_cpu->set_fpu(r3000a_device::MIPS_R3010A, INPUT_LINE_IRQ0);

	pi4d3x(config);

	// FIXME: higher revisions fail scsi power fuse diagnostic
	m_brdrev = 0x0000;
}

ROM_START(indigo)
	ROM_REGION32_BE(0x40000, "prom", 0) // Am27C2048 128Kx16

	// dumped over serial connection from boot monitor and swapped
	ROM_SYSTEM_BIOS(0, "401-rev-c", "SGI Version 4.0.1 Rev C LG1/GR2, Jul 9, 1992")
	ROMX_LOAD("ip12prom.070-8088-xxx.u56", 0x000000, 0x040000, CRC(25ca912f) SHA1(94b3753d659bfe50b914445cef41290122f43880), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0))

	// dumped with EPROM programmer
	ROM_SYSTEM_BIOS(1, "401-rev-d", "SGI Version 4.0.1 Rev D LG1/GR2, Mar 24, 1992")
	ROMX_LOAD("ip12prom.070-8088-002.u56", 0x000000, 0x040000, CRC(ea4329ef) SHA1(b7d67d0e30ae8836892f7170dd4757732a0a3fd6), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "sni", "SGI Version 4.0.1 Rev C LG1/GR2,  Feb 14, 1992")
	ROMX_LOAD("070-8088-001__a.u56", 0x000000, 0x040000, CRC(40eae7a0) SHA1(d60ef74cf04a16d9dad6b9594a66724d944b1208), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(2))
ROM_END

ROM_START(pi4d30)
	ROM_REGION32_BE(0x80000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "4.0.1c", "SGI Version 4.0.1 Rev C GR1/GR2/LG1,  Feb 14, 1992")
	ROMX_LOAD("ip14prom.bin", 0x000000, 0x080000, NO_DUMP, ROM_BIOS(0))
ROM_END

ROM_START(pi4d35)
	ROM_REGION32_BE(0x80000, "prom", 0) // TC574096D-120 (262,144x16-bit EEPROM)

	ROM_SYSTEM_BIOS(0, "4.0.1d", "SGI Version 4.0.1 Rev D LG1/GR2,  Mar 24, 1992")
	ROMX_LOAD("ip12prom.002be.u61", 0x000000, 0x040000, CRC(d35f105c) SHA1(3d08dfb961d7512bd8ed41cb6e01e89d14134f09), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "4.0.1c", "SGI Version 4.0.1 Rev C GR1/GR2/LG1,  Feb 14, 1992")
	ROMX_LOAD("ip12prom.070-8086-002.u61", 0x000000, 0x080000, CRC(543cfc3f) SHA1(a7331876f11bff40c960f822923503eca17191c5), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "4.0a", "SGI Version 4.0 Rev A IP12,  Aug 22, 1991")
	ROMX_LOAD("ip12prom.070-8045-002.u61", 0x000000, 0x040000, CRC(fe999bae) SHA1(eb054c365a6e018be3b9ae44169c0ffc6447c6f0), ROM_BIOS(2))
ROM_END

} // anonymous namespace

//   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY             FULLNAME               FLAGS
COMP(1991, pi4d30,  0,      0,      pi4d30,  0,     ip12_state, empty_init, "Silicon Graphics", "Personal IRIS 4D/30", MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS)
COMP(1991, pi4d35,  0,      0,      pi4d35,  0,     ip12_state, empty_init, "Silicon Graphics", "Personal IRIS 4D/35", MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS)
COMP(1991, indigo,  0,      0,      indigo,  0,     ip12_state, empty_init, "Silicon Graphics", "IRIS Indigo",         MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS)
