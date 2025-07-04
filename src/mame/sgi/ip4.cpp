// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics Professional IRIS 4D/50 and 4D/70 CPU board.
 */

/*
 * WIP:
 *
 * bios 0: configuration is already set in the provided nvram, otherwise configure monitor as follows:
 *  setenv bootfile dksc(0,1,8)sash         # configure boot device
 *  setenv netaddr 192.168.137.2            # configure network address
 *  setenv console d                        # configure terminal I/O
 *
 * label/partition disk using fx:
 *  boot -f dksc(0,4,8)sash.IP4
 *  boot -f dksc(0,4,7)stand/fx.IP4         # start fx, enable advanced options
 *                                          # label all, sync, exit
 *
 * install irix from cdrom:
 *  boot -f dksc(0,4,8)sash.IP4 --m         # start sash from CDROM, and copy/boot miniroot from swap
 *  install audio.data.sounds               # optionally install sound data
 *  go                                      # launch installation
 *  quit                                    # exit installer, wait for post-installation processing
 *
 * use "auto" to boot from monitor
 *
 * bios 1 fpu diagnostic fails at 0x9fc0499c:
 *  fcr31=0
 *  f2=0x7fe52c4d'716169fe
 *  cvt.w.s $f20,$f2  # result: f20=0xffffffff'80000000 fcr31=0x00010040 == "invalid operation" cause + flag
 *  cvt.s.d $f20,$f2  # result: f20=0xffffffff'7f800000 fcr31=0x00015054 == "invalid operation" cause + flag + overflow/inexact
 *  ...
 *  expect fcr31 == 0x00020000 == "unimplemented operation"
 *
 * bios 1 also recognizes cdrom as floppy, requiring installation workarounds:
 *  setenv tapedevice dksc(0,4,8)
 *  setenv root dks0d1s0
 *  boot -f dksc(0,4,8)sash.IP4 --m
 *  sh: mkdir /mnt; mount /dev/dks0d4s7 /mnt; exit
 *  from /mnt/dist
 *  go
 *
 * TODO:
 *  - fpu/cpu id, switches and leds
 *  - fix parity diagnostic error
 *  - fpu unimplemented operations
 *
 */

#include "emu.h"
#include "ip4.h"

#include "machine/nscsi_bus.h"
#include "machine/nvram.h"

#include "bus/nscsi/hd.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/tape.h"
#include "bus/rs232/hlemouse.h"

#include "kbd.h"

#include "speaker.h"

#define LOG_PARITY (1U << 1)
#define LOG_VME    (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_IP4, sgi_ip4_device, "sgi_ip4", "SGI IP4")

sgi_ip4_device::sgi_ip4_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_IP4, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_rtc(*this, "rtc")
	, m_pit(*this, "pit")
	, m_scsi(*this, "scsi:0:wd33c93")
	, m_duart(*this, "duart%u", 0U)
	, m_serial(*this, "serial%u", 0U)
	, m_saa(*this, "saa")
	, m_nvram(*this, "nvram", 0x800, ENDIANNESS_BIG)
	, m_ram(*this, "ram")
	, m_leds(*this, "led%u", 0U)
	, m_dma_hi{}
	, m_dma_page(0)
{
}

enum cpucfg_mask : u16
{
	CPUCFG_LEDS = 0x003f,
	CPUCFG_S01  = 0x0040, // enable serial ports 0,1
	CPUCFG_S23  = 0x0080, // enable serial ports 2,3
	CPUCFG_MAIL = 0x0100, // enable mailbox interrupts
	CPUCFG_SIN  = 0x0200, // VME sysreset (reset)
	CPUCFG_RPAR = 0x0400, // enable parity checking
	CPUCFG_SLA  = 0x0800, // enable slave accesses
	CPUCFG_ARB  = 0x1000, // enable VME arbiter
	CPUCFG_BAD  = 0x2000, // write bad parity
	CPUCFG_DOG  = 0x4000, // enable watchdog timout
	CPUCFG_AUX2 = 0x8000, // unused
};

enum parerr_mask : u8
{
	PAR_LAN = 0x01,
	PAR_DMA = 0x02,
	PAR_CPU = 0x04,
	PAR_VME = 0x08,
	PAR_B3  = 0x10, // parity error byte 3
	PAR_B2  = 0x20, // parity error byte 2
	PAR_B1  = 0x40, // parity error byte 1
	PAR_B0  = 0x80, // parity error byte 0
	PAR_ALL = 0xf0, // parity error all bytes
};

enum lio_int_number : unsigned
{
	LIO_D0   = 0, // duart 0
	LIO_D1   = 1, // duart 1
	LIO_D2   = 2, // duart 2
				  // unused
	LIO_SCSI = 4, // scsi
				  // unused
	LIO_MAIL = 6, // VME mailbox
	LIO_AC   = 7, // VME acfail
};

void sgi_ip4_device::map(address_map &map)
{
	// TODO: 4 banks of 4 SIMMs with parity
	map(0x0000'0000, 0x007f'ffff).ram().share("ram");

	map(0x1000'0000, 0x1bff'ffff).rw(&device_vme_card_interface::vme_read32<vme::AM_09,0x1000'0000>, "read", &device_vme_card_interface::vme_write32<vme::AM_09,0x1000'0000>, "write");
	map(0x1c00'0000, 0x1cff'ffff).rw(FUNC(device_vme_card_interface::vme_read32<vme::AM_3d>), FUNC(device_vme_card_interface::vme_write32<vme::AM_3d>));
	map(0x1d00'0000, 0x1d00'ffff).rw(FUNC(device_vme_card_interface::vme_read32<vme::AM_2d>), FUNC(device_vme_card_interface::vme_write32<vme::AM_2d>));
	map(0x1d10'0000, 0x1d10'ffff).rw(FUNC(device_vme_card_interface::vme_read32<vme::AM_29>), FUNC(device_vme_card_interface::vme_write32<vme::AM_29>));
	map(0x1df0'0000, 0x1df0'000f).lr16(NAME([this](offs_t offset) { return vme_iack_r(offset << 1); })).mirror(0xf'fff0);
	map(0x1e00'0000, 0x1eff'ffff).rw(FUNC(device_vme_card_interface::vme_read32<vme::AM_39>), FUNC(device_vme_card_interface::vme_write32<vme::AM_39>));
	map(0x2000'0000, 0x2fff'ffff).rw(FUNC(device_vme_card_interface::vme_read32<vme::AM_09>), FUNC(device_vme_card_interface::vme_write32<vme::AM_09>));

	map(0x1f60'0000, 0x1f60'0003).umask32(0xff00'0000).w(m_saa, FUNC(saa1099_device::data_w));
	map(0x1f60'0010, 0x1f60'0013).umask32(0xff00'0000).w(m_saa, FUNC(saa1099_device::control_w));

	map(0x1f80'0000, 0x1f80'0003).umask32(0x00ff'0000).lr8(NAME([]() { return 0; })); // TODO: system id prom/coprocessor present (2=no fpu)

	map(0x1f84'0000, 0x1f84'0003).umask32(0x0000'00ff).lrw8(NAME([this]() { LOG("vme_isr_r 0x%02x\n", m_vme_isr); return m_vme_isr; }), NAME([this](u8 data) { LOG("vme_isr_w 0x%02x\n", data); m_vme_isr = data; }));
	map(0x1f84'0008, 0x1f84'000b).umask32(0x0000'00ff).lrw8(NAME([this]() { LOG("vme_imr_r 0x%02x\n", m_vme_imr); return m_vme_imr; }), NAME([this](u8 data) { LOG("vme_imr_w 0x%02x\n", data); m_vme_imr = data; }));

	map(0x1f88'0000, 0x1f88'0003).umask32(0x0000'ffff).rw(FUNC(sgi_ip4_device::cpucfg_r), FUNC(sgi_ip4_device::cpucfg_w));

	map(0x1f90'0000, 0x1f90'0003).umask32(0x0000'ffff).lrw16(
		NAME([this]() { return m_dma_lo; }),
		NAME([this](u16 data) { m_dma_lo = data; m_dma_page = 0; }));
	map(0x1f92'0000, 0x1f92'003f).umask32(0x0000'ffff).lrw16(
		NAME([this](offs_t offset) { return m_dma_hi[offset]; }),
		NAME([this](offs_t offset, u16 data) { m_dma_hi[offset] = data; }));
	map(0x1f94'0000, 0x1f94'0003).nopw(); // dma flush

	map(0x1f98'0000, 0x1f98'0003).umask32(0x0000'00ff).lr8(NAME([this]() { return m_lio_isr; }));

	map(0x1f9a'0000, 0x1f9a'0003).umask32(0x0000'ffff).lr16([]() { return 0; }, "switch_r"); // TODO: switches (0x84 == nodiag)

	map(0x1fa0'0000, 0x1fa0'0003).umask32(0xff00'0000).lr8(NAME([this]() { m_cpu->set_input_line(INPUT_LINE_IRQ4, 0); return 0; }));
	map(0x1fa2'0000, 0x1fa2'0003).umask32(0xff00'0000).lr8(NAME([this]() { m_cpu->set_input_line(INPUT_LINE_IRQ2, 0); return 0; }));
	map(0x1fa4'0000, 0x1fa4'0003).lr32(NAME([this]() { m_cpu->set_input_line(INPUT_LINE_IRQ5, 0); return m_erradr; }));
	map(0x1fa8'0000, 0x1fa8'0003).umask32(0xff00'0000).lr8(NAME([this]() { m_scsi->reset_w(0); return 0; }));
	map(0x1fa8'0004, 0x1fa8'0007).umask32(0xff00'0000).lr8(NAME([this]() { m_scsi->reset_w(1); return 0; }));

	//map(0x1fa60000, 0x1fa60003).umask32(0xff00'0000); // TODO: vme rmw
	map(0x1faa0000, 0x1faa0003).lrw8(
		NAME([this](offs_t offset) { m_parerr &= ~(PAR_ALL | (1U << offset)); return 0; }),
		NAME([this](offs_t offset, u8 data) { m_parerr &= ~(PAR_ALL | (1U << offset)); }));
	map(0x1faa0004, 0x1faa0007).umask32(0x00ff'0000).lr8(NAME([this]() { return m_parerr ^ PAR_ALL; }));

	map(0x1fae'0000, 0x1fae'001f).rom().region("idprom", 0);

	map(0x1fb0'0000, 0x1fb0'0003).umask32(0x00ff'0000).rw(m_scsi, FUNC(wd33c93_device::indir_addr_r), FUNC(wd33c93_device::indir_addr_w));
	map(0x1fb0'0100, 0x1fb0'0103).umask32(0x00ff'0000).rw(m_scsi, FUNC(wd33c93_device::indir_reg_r), FUNC(wd33c93_device::indir_reg_w));
	map(0x1fb4'0000, 0x1fb4'000f).umask32(0xff00'0000).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));

	map(0x1fb8'0000, 0x1fb8'00ff).umask32(0xff00'0000).lrw8(
		NAME([this](offs_t offset) { return m_duart[BIT(offset, 0, 2)]->read(offset >> 2); }),
		NAME([this](offs_t offset, u8 data) { m_duart[BIT(offset, 0, 2)]->write(offset >> 2, data); }));

	map(0x1fbc'0000, 0x1fbc'1fff).umask32(0xff00'0000).rw(FUNC(sgi_ip4_device::nvram_r), FUNC(sgi_ip4_device::nvram_w));

	map(0x1fc0'0000, 0x1fc3'ffff).rom().region("boot", 0);
}

static DEVICE_INPUT_DEFAULTS_START(ip4_ctl1)
	DEVICE_INPUT_DEFAULTS("VALID", 0x000f, 0x000f)
DEVICE_INPUT_DEFAULTS_END

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_SGI).machine_config(
		[](device_t *device)
		{
			downcast<nscsi_cdrom_device &>(*device).set_block_size(512);
		});
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tape", NSCSI_TAPE);
}

void sgi_ip4_device::device_add_mconfig(machine_config &config)
{
	R2000(config, m_cpu, clock() / 2, 65536, 32768);
	m_cpu->set_fpu(mips1_device_base::MIPS_R2010);
	m_cpu->set_addrmap(AS_PROGRAM, &sgi_ip4_device::map);
	m_cpu->in_brcond<0>().set([]() { return 1; }); // writeback complete

	DS1215(config, m_rtc); // DS1216B?

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CXK5816PN-15L

	PIT8253(config, m_pit);
	m_pit->set_clk<2>(3.6864_MHz_XTAL);
	m_pit->out_handler<0>().set([this](int state) { if (state) m_cpu->set_input_line(INPUT_LINE_IRQ2, 1); });
	m_pit->out_handler<1>().set([this](int state) { if (state) m_cpu->set_input_line(INPUT_LINE_IRQ4, 1); });
	m_pit->out_handler<2>().set(m_pit, FUNC(pit8254_device::write_clk0));
	m_pit->out_handler<2>().append(m_pit, FUNC(pit8254_device::write_clk1));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0").option_set("wd33c93", WD33C93).machine_config(
		[this](device_t *device)
		{
			wd33c9x_base_device &wd33c93(downcast<wd33c9x_base_device &>(*device));

			wd33c93.set_clock(10'000'000);
			wd33c93.irq_cb().set(*this, FUNC(sgi_ip4_device::lio_irq<LIO_SCSI>)).invert();
			wd33c93.drq_cb().set(*this, FUNC(sgi_ip4_device::scsi_drq));
		});
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, "cdrom", false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, "tape", false);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, nullptr, false);

	// duart 0 (keyboard/mouse)
	SCN2681(config, m_duart[0], 3.6864_MHz_XTAL); // SCN2681AC1N24

	sgi_kbd_port_device &keyboard_port(SGI_KBD_PORT(config, "keyboard_port", default_sgi_kbd_devices, nullptr));
	rs232_port_device &mouse_port(RS232_PORT(config, "mouse_port",
		[](device_slot_interface &device)
		{
			device.option_add("mouse", SGI_HLE_SERIAL_MOUSE);
		},
		nullptr));

	// duart 0 outputs
	m_duart[0]->irq_cb().set(FUNC(sgi_ip4_device::lio_irq<LIO_D0>)).invert();
	m_duart[0]->a_tx_cb().set(keyboard_port, FUNC(sgi_kbd_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(mouse_port, FUNC(rs232_port_device::write_txd));

	// duart 0 inputs
	keyboard_port.rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	mouse_port.rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));

	// duart 1 (serial ports 0,1)
	SCN2681(config, m_duart[1], 3.6864_MHz_XTAL); // SCN2681AC1N40
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	// duart 1 outputs
	m_duart[1]->irq_cb().set(FUNC(sgi_ip4_device::lio_irq<LIO_D1>)).invert();
	m_duart[1]->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_duart[1]->outport_cb().set(
		[this](u8 data)
		{
			m_serial[0]->write_rts(BIT(data, 0));
			m_serial[1]->write_rts(BIT(data, 1));
			m_duart[1]->ip5_w(BIT(data, 3));
			m_duart[1]->ip6_w(BIT(data, 3));
			m_serial[0]->write_dtr(BIT(data, 4));
			m_serial[1]->write_dtr(BIT(data, 5));
		});

	// duart 1 inputs
	m_serial[0]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_serial[0]->cts_handler().set(m_duart[1], FUNC(scn2681_device::ip0_w));
	m_serial[0]->dcd_handler().set(m_duart[1], FUNC(scn2681_device::ip3_w));

	m_serial[1]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));
	m_serial[1]->cts_handler().set(m_duart[1], FUNC(scn2681_device::ip1_w));
	m_serial[1]->dcd_handler().set(m_duart[1], FUNC(scn2681_device::ip2_w));

	// duart 2 (serial ports 2,3)
	SCN2681(config, m_duart[2], 3.6864_MHz_XTAL); // SCN2681AC1N40
	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[3], default_rs232_devices, nullptr);

	// duart 2 outputs
	m_duart[2]->irq_cb().set(FUNC(sgi_ip4_device::lio_irq<LIO_D2>)).invert();
	m_duart[2]->a_tx_cb().set(m_serial[2], FUNC(rs232_port_device::write_txd));
	m_duart[2]->b_tx_cb().set(m_serial[3], FUNC(rs232_port_device::write_txd));
	m_duart[2]->outport_cb().set(
		[this](u8 data)
		{
			m_serial[2]->write_rts(BIT(data, 0));
			m_serial[3]->write_rts(BIT(data, 1));
			m_duart[2]->ip5_w(BIT(data, 3));
			m_duart[2]->ip6_w(BIT(data, 3));
			m_serial[2]->write_dtr(BIT(data, 4));
			m_serial[3]->write_dtr(BIT(data, 5));
		});

	// duart 2 inputs
	m_serial[2]->rxd_handler().set(m_duart[2], FUNC(scn2681_device::rx_a_w));
	m_serial[2]->cts_handler().set(m_duart[2], FUNC(scn2681_device::ip0_w));
	m_serial[2]->dcd_handler().set(m_duart[2], FUNC(scn2681_device::ip3_w));

	m_serial[3]->rxd_handler().set(m_duart[2], FUNC(scn2681_device::rx_b_w));
	m_serial[3]->cts_handler().set(m_duart[2], FUNC(scn2681_device::ip1_w));
	m_serial[3]->dcd_handler().set(m_duart[2], FUNC(scn2681_device::ip2_w));

	// TODO: move speakers to host
	SPEAKER(config, "speaker", 2).front();

	SAA1099(config, m_saa, 8_MHz_XTAL);
	m_saa->add_route(0, "speaker", 0.5, 0);
	m_saa->add_route(1, "speaker", 0.5, 1);

	// TODO: ACFAIL -> vme_irq<0>
	device_vme_card_interface::vme_irq<1>().set(*this, FUNC(sgi_ip4_device::vme_irq<1>));
	device_vme_card_interface::vme_irq<2>().set(*this, FUNC(sgi_ip4_device::vme_irq<2>));
	device_vme_card_interface::vme_irq<3>().set(*this, FUNC(sgi_ip4_device::vme_irq<3>));
	device_vme_card_interface::vme_irq<4>().set(*this, FUNC(sgi_ip4_device::vme_irq<4>));
	device_vme_card_interface::vme_irq<5>().set(*this, FUNC(sgi_ip4_device::vme_irq<5>));
	device_vme_card_interface::vme_irq<6>().set(*this, FUNC(sgi_ip4_device::vme_irq<6>));
	device_vme_card_interface::vme_irq<7>().set(*this, FUNC(sgi_ip4_device::vme_irq<7>));

	vme_berr().set_inputline(m_cpu, INPUT_LINE_IRQ5).invert();
}

void sgi_ip4_device::device_start()
{
	m_leds.resolve();

	save_item(NAME(m_cpucfg));
	save_item(NAME(m_dma_lo));
	save_item(NAME(m_dma_hi));
	save_item(NAME(m_dma_page));
	save_item(NAME(m_lio_isr));
	save_item(NAME(m_vme_isr));
	save_item(NAME(m_vme_imr));
	save_item(NAME(m_parerr));
	save_item(NAME(m_erradr));

	save_item(NAME(m_lio_irq));
	save_item(NAME(m_vme_irq));

	m_cpucfg = 0;
	m_dma_lo = 0;
	m_lio_isr = 0xff;
	m_vme_isr = 0;
	m_vme_imr = 0;

	m_lio_irq = false;
	m_vme_irq = false;

	m_parity_bad = 0;
}

void sgi_ip4_device::device_reset()
{
	m_parerr = 0;
	m_erradr = 0;
}

void sgi_ip4_device::lio_irq(unsigned number, int state)
{
	// record interrupt state
	if (state)
		m_lio_isr |= 1U << number;
	else
		m_lio_isr &= ~(1U << number);

	// update interrupt line
	bool const lio_irq = (m_lio_isr ^ 0xff);
	if (m_lio_irq ^ lio_irq)
	{
		m_lio_irq = lio_irq;
		m_cpu->set_input_line(INPUT_LINE_IRQ1, m_lio_irq);
	}
}

void sgi_ip4_device::vme_irq(unsigned number, int state)
{
	// record interrupt state
	if (!state)
		m_vme_isr |= 1U << number;
	else
		m_vme_isr &= ~(1U << number);

	// update interrupt line
	bool const vme_irq = m_vme_isr;// &m_vme_imr;
	if (m_vme_irq ^ vme_irq)
	{
		m_vme_irq = vme_irq;
		m_cpu->set_input_line(INPUT_LINE_IRQ0, m_vme_irq);
	}
}

void sgi_ip4_device::scsi_drq(int state)
{
	if (state)
	{
		u32 const addr = (u32(m_dma_hi[m_dma_page]) << 12) | (m_dma_lo & 0x0fff);

		if (m_dma_lo & 0x8000)
			m_cpu->space(0).write_byte(addr, m_scsi->dma_r());
		else
			m_scsi->dma_w(m_cpu->space(0).read_byte(addr));

		m_dma_lo = (m_dma_lo + 1) & 0x8fff;

		if (!(m_dma_lo & 0xfff))
			m_dma_page = (m_dma_page + 1) & 0xf;
	}
}

void sgi_ip4_device::cpucfg_w(u16 data)
{
	//LOG("cpucfg_w 0x%04x (%s)\n", data, machine().describe_context());

	// update leds
	for (unsigned i = 0; i < 6; i++)
		m_leds[i] = BIT(data, i);

	if ((m_cpucfg & CPUCFG_MAIL) && !BIT(data, 8))
		lio_irq<LIO_MAIL>(1);

	if (BIT(data, 9))
		machine().schedule_soft_reset();

	if ((m_cpucfg ^ data) & CPUCFG_RPAR)
		LOGMASKED(LOG_PARITY, "parity checking %d\n", BIT(data, 10));

	if (!(m_cpucfg & CPUCFG_SLA) && (data & CPUCFG_SLA))
	{
		LOGMASKED(LOG_VME, "vme slave access enabled\n");
		vme_space(vme::AM_29).install_write_handler(0x1000, 0x13ff, emu::rw_delegate(*this, FUNC(sgi_ip4_device::mailbox_w)));

		vme_space(vme::AM_39).install_ram(0x0000'0000, 0x007f'ffff, m_ram.target());
		vme_space(vme::AM_3a).install_ram(0x0000'0000, 0x007f'ffff, m_ram.target());
		vme_space(vme::AM_3b).install_ram(0x0000'0000, 0x007f'ffff, m_ram.target());
		vme_space(vme::AM_3d).install_ram(0x0000'0000, 0x007f'ffff, m_ram.target());
		vme_space(vme::AM_3e).install_ram(0x0000'0000, 0x007f'ffff, m_ram.target());
		vme_space(vme::AM_3f).install_ram(0x0000'0000, 0x007f'ffff, m_ram.target());

		vme_space(vme::AM_09).install_ram(0x0000'0000, m_ram.bytes() - 1, m_ram.target());
		vme_space(vme::AM_0a).install_ram(0x0000'0000, m_ram.bytes() - 1, m_ram.target());
	}

	if ((m_cpucfg ^ data) & CPUCFG_BAD)
	{
		LOGMASKED(LOG_PARITY, "write bad parity %d\n", BIT(data, 13));

		if ((data & CPUCFG_BAD) && !m_parity)
		{
			unsigned const ram_size = 8;

			LOGMASKED(LOG_PARITY, "bad parity activated %dM\n", ram_size);

			m_parity = std::make_unique<u8[]>(ram_size << (20 - 3));
			m_parity_mph = m_cpu->space(0).install_readwrite_tap(0, (ram_size << 20) - 1, "parity",
				std::bind(&sgi_ip4_device::parity_r, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
				std::bind(&sgi_ip4_device::parity_w, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}
	}

	m_cpucfg = data;
}

void sgi_ip4_device::parity_r(offs_t offset, u32 &data, u32 mem_mask)
{
	if (m_cpucfg & CPUCFG_RPAR)
	{
		bool error = false;
		for (unsigned byte = 0; byte < 4; byte++)
		{
			if (BIT(mem_mask, 24 - byte * 8, 8) && BIT(m_parity[offset >> 3], BIT(offset, 2) * 4 + byte))
			{
				m_parerr |= (PAR_B0 >> byte) | PAR_CPU;
				error = true;

				LOGMASKED(LOG_PARITY, "bad parity err 0x%08x byte %d count %d\n", offset, byte, m_parity_bad);
			}
		}

		if (error)
		{
			m_erradr = offset;
			m_cpu->berr_w(1);
		}
	}
}

void sgi_ip4_device::parity_w(offs_t offset, u32 &data, u32 mem_mask)
{
	if (m_cpucfg & CPUCFG_BAD)
	{
		for (unsigned byte = 0; byte < 4; byte++)
		{
			if (BIT(mem_mask, 24 - byte * 8, 8) && !BIT(m_parity[offset >> 3], BIT(offset, 2) * 4 + byte))
			{
				m_parity[offset >> 3] |= 1U << (BIT(offset, 2) * 4 + byte);
				m_parity_bad++;

				LOGMASKED(LOG_PARITY, "bad parity set 0x%08x byte %d count %d\n", offset, byte, m_parity_bad);
			}
		}
	}
	else
	{
		for (unsigned byte = 0; byte < 4; byte++)
		{
			if (BIT(mem_mask, 24 - byte * 8, 8) && BIT(m_parity[offset >> 3], BIT(offset, 2) * 4 + byte))
			{
				m_parity[offset >> 3] &= ~(1U << (BIT(offset, 2) * 4 + byte));
				m_parity_bad--;

				LOGMASKED(LOG_PARITY, "bad parity clr 0x%08x byte %d count %d\n", offset, byte, m_parity_bad);
			}
		}

		if (m_parity_bad == 0)
		{
			LOGMASKED(LOG_PARITY, "bad parity deactivated\n");

			m_parity_mph.remove();
			m_parity.reset();
		}
	}
}

void sgi_ip4_device::mailbox_w(offs_t offset, u8 data)
{
	if (m_cpucfg & CPUCFG_MAIL)
	{
		LOGMASKED(LOG_VME, "vme mailbox interrupt (%s)\n", machine().describe_context());

		lio_irq<LIO_MAIL>(0);
	}
}

u8 sgi_ip4_device::nvram_r(offs_t offset)
{
	if (offset == 0x7ff && !machine().side_effects_disabled())
	{
		// return SmartWatch data if /CEO is negated
		if (m_rtc->ceo_r())
			return m_rtc->read();
		else
			m_rtc->read();
	}

	return m_nvram[offset];
}

void sgi_ip4_device::nvram_w(offs_t offset, u8 data)
{
	// write to NVRAM if SmartWatch /CEO is asserted
	if (offset != 0x7ff || !m_rtc->ceo_r())
		m_nvram[offset] = data;

	if (offset == 0x7ff)
		m_rtc->write(data);
}

ROM_START(ip4)
	ROM_REGION32_BE(0x40000, "boot", 0)

	ROM_SYSTEM_BIOS(0, "4d1v30", "Version 4D1-3.0 PROM IP4 Mon Jan  4 20:29:51 PST 1988 SGI")
	ROMX_LOAD("070-0093-009.bin", 0x000000, 0x010000, CRC(261b0a4c) SHA1(59f73d0e022a502dc5528289e388700b51b308da), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070-0094-009.bin", 0x000001, 0x010000, CRC(8c05f591) SHA1(d4f86ad274f9dfe10c38551f3b6b9ba73570747f), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070-0095-009.bin", 0x000002, 0x010000, CRC(2dacfcb7) SHA1(0149274a11d61e3ada0f7b055e79d884a65481d3), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070-0096-009.bin", 0x000003, 0x010000, CRC(72dd0246) SHA1(6df99bdf7afaded8ef68a9644dd06ca69a996db0), ROM_BIOS(0) | ROM_SKIP(3))

	ROM_SYSTEM_BIOS(1, "4d1v31", "Version 4D1-3.1 PROM IP4 OPT Thu Dec  8 16:12:10 PST 1988 SGI")
	ROMX_LOAD("070-0093-010.bin", 0x000000, 0x010000, CRC(f46871bf) SHA1(bda07b083dbb350d90a9145539c389180c800fb4), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("070-0094-010.bin", 0x000001, 0x010000, CRC(7b183793) SHA1(b5471a990f755333378f77e7a4d1102479ccfdc1), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("070-0095-010.bin", 0x000002, 0x010000, CRC(af2a2f0b) SHA1(beda300a5b1633f7682fdf3b8b0650bfea253567), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("070-0096-010.bin", 0x000003, 0x010000, CRC(eaa2bdc3) SHA1(8f1f37ec8875f36b45d335412ae1c298af84f20f), ROM_BIOS(1) | ROM_SKIP(3))

	ROM_REGION32_BE(0x20, "idprom", 0)
	ROM_LOAD("idprom.bin", 0, 0x20, NO_DUMP)

	ROM_REGION(0x800, "nvram", 0)
	ROM_LOAD("nvram", 0, 0x800, CRC(333443d0) SHA1(aceb115a2f4d2e3a229b2c2c3b20314674113705))
ROM_END

static INPUT_PORTS_START(ip4)
INPUT_PORTS_END

tiny_rom_entry const *sgi_ip4_device::device_rom_region() const
{
	return ROM_NAME(ip4);
}

ioport_constructor sgi_ip4_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ip4);
}
