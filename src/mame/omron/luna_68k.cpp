// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Omron Luna M68K systems.
 *
 * Sources:
 *   - https://wiki.netbsd.org/ports/luna68k/
 *
 * TODO:
 *   - skeleton only
 */
/*
 * WIP
 *
 * This driver is currently based on a VME-based Luna with a 25MHz 68030, which
 * differs from the systems supported by NetBSD. Boards installed are:
 *
 *  C25   CPU, FPU, serial, RTC, 68030 + 68882 @ 25MHz?
 *  IOC2  I/O controller (floppy, SCSI, serial), 68000 @ 10MHz?
 *  GPU8  graphics processor + serial, 68020 @ 20MHz? + +68881 @ 16MHz?
 *  DPU8  video/framebuffer, Bt458 @ 108MHz
 *  CMC   communications (GPIB, Ethernet, serial), 68020 @ 12.5MHz?
 *
 * This specific machine may be an SX-9100 Model 90?
 */
#include "emu.h"

#include "cpu/m68000/m68030.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/mc146818.h"
#include "machine/z80sio.h"
#include "machine/am9513.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/nscsi/hd.h"

// ioc2
#include "cpu/m68000/m68000.h"
#include "machine/hd63450.h"
#include "machine/mb87030.h"
#include "machine/wd_fdc.h"
#include "machine/z80scc.h"
#include "machine/z8536.h"

// gpu
#include "cpu/m68000/m68020.h"
#include "machine/mc68681.h"
#include "machine/mc68901.h"
#include "video/bt45x.h"
#include "machine/nvram.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class luna_68k_state : public driver_device
{
public:
	luna_68k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_rtc(*this, "rtc")
		, m_sio(*this, "sio")
		, m_stc(*this, "stc")
		, m_serial(*this, "serial%u", 0U)
		, m_eprom(*this, "eprom")
		// ioc2
		, m_ioc_cpu(*this, "ioc")
		, m_ioc_dma(*this, "dma%u", 0U)
		, m_ioc_spc(*this, "scsi%u:7:spc", 0U)
		, m_ioc_fdc(*this, "fdc")
		, m_ioc_scc(*this, "scc")
		, m_ioc_cio(*this, "cio")
		, m_ioc_ram(*this, "ioc_ram")
		, m_ioc_boot(*this, "ioc_boot")
		// gpu
		, m_gpu_cpu(*this, "gpu")
		, m_gpu_dac(*this, "dac")
		, m_gpu_mfp(*this, "mfp")
		, m_gpu_tty(*this, "tty")
		, m_gpu_duart(*this, "duart%u", 0U)
	{
	}

	// machine config
	void luna(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void cpu_map(address_map &map) ATTR_COLD;
	void cpu_autovector_map(address_map &map) ATTR_COLD;

	void ioc_cpu_map(address_map &map);
	void gpu_cpu_map(address_map &map);

private:
	u32 bus_error_r(offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			LOG("bus_error_r 0x%x (%s)\n", offset << 2, machine().describe_context());
			m_cpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		}

		return 0;
	}

	u16 ioc_ram_r(offs_t offset)
	{
		return m_ioc_ram[offset];
	}

	void ioc_ram_w(offs_t offset, u16 data, u16 mem_mask)
	{
		m_ioc_ram[offset] = (m_ioc_ram[offset] & ~mem_mask) | (data & mem_mask);
	}

	void ioc_boot_disable_w(offs_t offset, u16 data)
	{
		if (!machine().side_effects_disabled())
			m_ioc_boot.disable();
		m_ioc_ram[offset & 0x1ffff] = data;
	}

	void ram_size_w(u32 data)
	{
		// FIXME: ram size/enable?
		if (data != m_ram_size)
		{
			m_ram_size = data;
			m_cpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
		}
	}

	// devices
	required_device<m68030_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<ds1287_device> m_rtc;
	required_device<upd7201_device> m_sio;
	required_device<am9513_device> m_stc;
	required_device_array<rs232_port_device, 2> m_serial;

	required_region_ptr<u32> m_eprom;

	// ioc2
	required_device<m68000_device> m_ioc_cpu;
	required_device_array<hd63450_device, 2> m_ioc_dma;
	required_device_array<mb89352_device, 2> m_ioc_spc;
	required_device<mb8877_device> m_ioc_fdc;
	required_device<z80scc_device> m_ioc_scc;
	required_device<z8536_device> m_ioc_cio;
	required_shared_ptr<u16> m_ioc_ram;
	memory_view m_ioc_boot;

	// gpu
	required_device<m68020fpu_device> m_gpu_cpu;
	required_device<bt458_device> m_gpu_dac;
	required_device<mc68901_device> m_gpu_mfp;
	required_device<rs232_port_device> m_gpu_tty;
	required_device_array<mc68681_device, 2> m_gpu_duart;

	u32 m_ram_size = 0U;
};

void luna_68k_state::machine_start()
{
}

void luna_68k_state::machine_reset()
{
	// mirror eprom at reset
	m_cpu->space(AS_PROGRAM).install_rom(0, m_eprom.bytes() - 1, m_eprom);

	m_ioc_boot.select(0);
}

void luna_68k_state::cpu_map(address_map &map)
{
	map(0x20280000, 0x202bffff).rw(FUNC(luna_68k_state::ioc_ram_r), FUNC(luna_68k_state::ioc_ram_w));

	map(0x30000000, 0x3fffffff).r(FUNC(luna_68k_state::bus_error_r));
	if (0) // FIXME: won't boot to monitor if this is enabled
	{
		map(0x30000d00, 0x30000d1f).m(m_ioc_spc[0], FUNC(mb89352_device::map)).umask32(0x00ff00ff);
		map(0x30000d20, 0x30000d3f).m(m_ioc_spc[1], FUNC(mb89352_device::map)).umask32(0x00ff00ff);
	}

	map(0x40000000, 0x4001ffff).rom().region("eprom", 0);

	map(0x50000000, 0x50000007).rw(m_sio, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask32(0xff00ff00);
	map(0x58000000, 0x5800007f).rw(m_rtc, FUNC(ds1287_device::read_direct), FUNC(ds1287_device::write_direct)).umask32(0xff00ff00);
	map(0x60000000, 0x60000003).rw(m_stc, FUNC(am9513_device::read16), FUNC(am9513_device::write16));

	map(0x70000000, 0x70000003).lr32([]() { return 0x000000fc; }, "sw3"); // FIXME: possibly CPU board DIP switch? (1=UP)
	map(0x78000000, 0x78000003).w(FUNC(luna_68k_state::ram_size_w));

	map(0xd01f8000, 0xd01f8003).r(FUNC(luna_68k_state::bus_error_r)); // acrtc graphics
	map(0xe1f00038, 0xe1f0003b).nopr(); // jrc graphics
}

void luna_68k_state::cpu_autovector_map(address_map &map)
{
	map(0xfffffff3, 0xfffffff3).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
	map(0xfffffff5, 0xfffffff5).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xfffffff7, 0xfffffff7).lr8(NAME([]() { return m68000_base_device::autovector(3); }));
	map(0xfffffff9, 0xfffffff9).lr8(NAME([]() { return m68000_base_device::autovector(4); }));
	map(0xfffffffb, 0xfffffffb).lr8(NAME([]() { return m68000_base_device::autovector(5); }));
	map(0xfffffffd, 0xfffffffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
	map(0xffffffff, 0xffffffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}

void luna_68k_state::ioc_cpu_map(address_map &map)
{
	// am8530h-6pc scc @ 4.9152MHz
	// mb89352 x 2 scsi
	// mb8877a
	// hd63450ps10 x 2 dma
	// z0853606psc cio

	map(0x000000, 0x03ffff).ram().share(m_ioc_ram).mirror(0x100000); // HM62256LP-10x8 (32768x8) - 256KB
	map(0x000000, 0x000fff).view(m_ioc_boot);
	m_ioc_boot[0](0x000000, 0x000fff).rom().region("ioc", 0).w(FUNC(luna_68k_state::ioc_boot_disable_w));
	map(0xfc0000, 0xfcffff).rom().region("ioc", 0);
	map(0xfe0000, 0xfe0fff).rom().region("ioc", 0);

	map(0xfef400, 0xfef400).rw(m_ioc_scc, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w));
	map(0xfef401, 0xfef401).rw(m_ioc_scc, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w));
	map(0xfef402, 0xfef402).rw(m_ioc_scc, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w));
	map(0xfef403, 0xfef403).rw(m_ioc_scc, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w));
}

void luna_68k_state::gpu_cpu_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom().region("gpu", 0);

	map(0x80000000, 0x80bfffff).ram(); // M5M41000BJ  1mb  (1m x 1) dynamic RAM (8x12) - 12MB

	map(0xb0080000, 0xb008001f).rw(m_gpu_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write));
	map(0xb0081000, 0xb008100f).rw(m_gpu_duart[0], FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0xb0082000, 0xb008200f).rw(m_gpu_duart[1], FUNC(mc68681_device::read), FUNC(mc68681_device::write));

	map(0xb0090000, 0xb00900ff).ram().share("gpu_nvram"); // MBM2212-20 256x4 NVRAM x 2 - 256B

	map(0xc0000000, 0xc000ffff).ram(); // M5M5178P-55 64kb (8k x 8) static RAM (2x4)   -  64kB
	map(0xf0000000, 0xf003ffff).ram(); // M5M5258P-35 256kb (64k x 4) static RAM (x8)  - 256kB
}

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_19200)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_19200)
DEVICE_INPUT_DEFAULTS_END

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void luna_68k_state::luna(machine_config &config)
{
	M68030(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &luna_68k_state::cpu_autovector_map);

	// 8 SIMMs for RAM arranged as two groups of 4, soldered
	RAM(config, m_ram);
	m_ram->set_default_size("16M");

	DS1287(config, m_rtc, 32'768);

	UPD7201(config, m_sio, 9'830'000); // D9.83B0
	m_sio->out_int_callback().set_inputline(m_cpu, M68K_IRQ_6);

	// console
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	m_serial[0]->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	m_sio->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_serial[0]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxa_w));
	m_serial[0]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsa_w));

	// keyboard
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_sio->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_serial[1]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxb_w));
	m_serial[1]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsb_w));

	AM9513(config, m_stc, 9'830'000); // FIXME: clock? sources?
	// TODO: clock interrupt 5?
	// TODO: soft interrupt 1?
	m_stc->fout_cb().set(m_stc, FUNC(am9513_device::gate1_w)); // assumption based on a common configuration
	m_stc->out1_cb().set_inputline(m_cpu, M68K_IRQ_7);
	m_stc->out4_cb().set(m_sio, FUNC(upd7201_device::rxca_w));
	m_stc->out4_cb().append(m_sio, FUNC(upd7201_device::txca_w));
	m_stc->out5_cb().set(m_sio, FUNC(upd7201_device::rxcb_w));
	m_stc->out5_cb().append(m_sio, FUNC(upd7201_device::txcb_w));

	// IOC2
	M68000(config, m_ioc_cpu, 10_MHz_XTAL);
	m_ioc_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::ioc_cpu_map);

	HD63450(config, m_ioc_dma[0], 20'000'000 / 2, "ioc");
	HD63450(config, m_ioc_dma[1], 20'000'000 / 2, "ioc");

	// internal SCSI
	NSCSI_BUS(config, "scsi0");
	NSCSI_CONNECTOR(config, "scsi0:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:6", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("spc", MB89352).machine_config(
		[](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(10'000'000);
			//spc.out_irq_callback().set(spc_irq, FUNC(input_merger_any_high_device::in_w<0>));
		});

	// external SCSI
	NSCSI_BUS(config, "scsi1");
	NSCSI_CONNECTOR(config, "scsi1:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("spc", MB89352).machine_config(
		[](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(10'000'000);
			//spc.out_irq_callback().set(spc_irq, FUNC(input_merger_any_high_device::in_w<1>));
		});

	MB8877(config, m_ioc_fdc, 0);
	SCC8530(config, m_ioc_scc, 4.9152_MHz_XTAL); // AM8530H-6PC
	m_ioc_scc->configure_channels(4'915'200, 4'915'200, 4'915'200, 4'915'200);
	Z8536(config, m_ioc_cio, 10'000'000);

	// GPU
	M68020FPU(config, m_gpu_cpu, 33'340'000 / 2);
	m_gpu_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::gpu_cpu_map);

	BT458(config, m_gpu_dac, 108'000'000);

	MC68901(config, m_gpu_mfp, 3.6864_MHz_XTAL);
	m_gpu_mfp->set_timer_clock(3.6864_MHz_XTAL);
	m_gpu_mfp->out_tdo_cb().set(m_gpu_mfp, FUNC(mc68901_device::tc_w));
	m_gpu_mfp->out_tdo_cb().append(m_gpu_mfp, FUNC(mc68901_device::rc_w));
	//m_gpu_mfp->out_irq_cb().set_inputline(m_gpu_cpu, M68K_IRQ_7);

	RS232_PORT(config, m_gpu_tty, default_rs232_devices, nullptr);
	m_gpu_mfp->out_so_cb().set(m_gpu_tty, FUNC(rs232_port_device::write_txd));
	m_gpu_tty->rxd_handler().set(m_gpu_mfp, FUNC(mc68901_device::si_w));

	MC68681(config, m_gpu_duart[0], 3.6864_MHz_XTAL);
	MC68681(config, m_gpu_duart[1], 3.6864_MHz_XTAL);

	NVRAM(config, "gpu_nvram");
}

ROM_START(luna)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_LOAD16_WORD_SWAP("0283__ac__8117__1.05.ic88", 0x00000, 0x20000, CRC(c46dec54) SHA1(22ef9274f4ef85d446d56cce13a68273dc55f10a))

	// HACK: force firmware to reinitialize nvram at first boot
	ROM_REGION(64, "rtc", 0)
	ROM_FILL(0, 64, 0xff)

	ROM_REGION16_BE(0x10000, "ioc", 0)
	ROM_LOAD16_BYTE("8145__h__3.24.ic108", 0x0000, 0x8000, CRC(d2dde582) SHA1(e34c15e43869be573272503d1f47e9e244536396))
	ROM_LOAD16_BYTE("8145__l__3.24.ic100", 0x0001, 0x8000, CRC(4863329b) SHA1(881623c3a64260f5cc1be066dbb47799d1f2ce14))

	ROM_REGION32_BE(0x40000, "gpu", 0)
	ROM_LOAD("jaw-2500__rom0__v1.21.rom0", 0x00000, 0x20000, CRC(915e0e86) SHA1(1115a8d3101f6d16e397016ae02fc64202edfc3a))
	ROM_LOAD("jaw-2500__rom1__v1.21.rom1", 0x20000, 0x20000, CRC(b4c21f3f) SHA1(577833dfbbceba8ee32fd2ac5b1809f860143d44))
ROM_END

} // anonymous namespace

/*   YEAR   NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS           INIT         COMPANY  FULLNAME  FLAGS */
COMP(1989?, luna, 0,      0,      luna,    0,     luna_68k_state, empty_init,  "Omron", "Luna",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
