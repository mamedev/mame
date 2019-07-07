// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/****************************************************************************

    drivers/4dpi.cpp
    SGI Personal IRIS family skeleton driver

    by Ryan Holtz

        0x1fc00000 - 0x1fc3ffff     ROM

    Interrupts:
        R2000:
            NYI

    Year  Model  Board  CPU    Clock    I/D Cache
    1988  4D/20  IP6    R2000  12.5MHz  16KiB/8KiB
          4D/25  IP10   R3000  20MHz    64KiB/32KiB
          4D/30  IP14   R3000  30MHz
    1991  4D/35  IP12   R3000  36MHz

****************************************************************************/
/*
 * Sources:
 *   - http://www.bitsavers.org/pdf/sgi/personal_iris/SGI_IP-6_Schematic.pdf
 *   - http://www.futuretech.blinkenlights.nl/pitechrep.html
 *   - https://hardware.majix.org/computers/sgi.pi/index.shtml
 *   - http://archive.irix.cc/sgistuff/hardware/systems/personal.html
 *   - http://archive.irix.cc/developerforum95/Silicon_Graphics_Developer_Forum_95_The_CD_Volume_2/documents/hw_handbook_html/handbook.html
 *   - https://github.com/NetBSD/src/tree/trunk/sys/arch/sgimips/
 *
 * TODO:
 *   - IOC1 and CTL1
 *   - graphics, audio
 *
 * Status:
 *   - parity and cache diagnostics fail
 *   - boots monitor and fx/sash from cdrom or network
 *   - hangs after booting irix from miniroot
 */

#include "emu.h"

// cpu and memory
#include "cpu/mips/mips1.h"
#include "machine/ram.h"
#include "machine/eepromser.h"

// other devices
#include "machine/wd33c9x.h"
#include "machine/am79c90.h"
#include "machine/mc68681.h"
#include "machine/pit8253.h"
#include "machine/dp8573.h"

// buses and connectors
#include "machine/nscsi_bus.h"
#include "machine/nscsi_hd.h"
#include "machine/nscsi_cd.h"
#include "bus/rs232/rs232.h"
#include "bus/sgikbd/sgikbd.h"
#include "bus/rs232/hlemouse.h"

// video and audio
#include "screen.h"

#define LOG_GENERAL (1U << 0)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

class ip6_state : public driver_device
{
public:
	ip6_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_eeprom(*this, "eeprom")
		, m_rtc(*this, "rtc")
		, m_pit(*this, "pit")
		, m_scsi(*this, "scsi:0:wd33c93")
		, m_enet(*this, "enet")
		, m_duart(*this, "duart%u", 0U)
		, m_serial(*this, "serial%u", 1U)
		, m_leds(*this, "led%u", 0U)
	{
	}

	void configure(machine_config &config);
	void initialize();

private:
	required_device<mips1_device_base> m_cpu;
	required_device<ram_device> m_ram;
	required_device<eeprom_serial_93c56_16bit_device> m_eeprom;

	required_device<dp8573_device> m_rtc;
	required_device<pit8254_device> m_pit;
	required_device<wd33c93_device> m_scsi;
	required_device<am7990_device> m_enet;
	required_device_array<scn2681_device, 2> m_duart;
	required_device_array<rs232_port_device, 2> m_serial;

	enum leds : unsigned
	{
		LED_HBT = 0, // heartbeat (1Hz)
		LED_CPU = 1, // cpu activity
		LED_GFX = 2, // graphics
		LED_FPU = 3, // fpu present
	};
	output_finder<4> m_leds;

	void map(address_map &map);

	template <unsigned N> void lio_interrupt(int state) { lio_interrupt(N, state); }
	void lio_interrupt(unsigned number, int state);
	void scsi_drq(int state);

	u8 ctl_sid_r();

	enum ctl_sid_mask : u8
	{
		SID_SERDATA = 0x01, // serial memory data output state
		SID_FPPRES  = 0x02, // floating point processor present
		SID_SERCLK  = 0x04, // serial memory clock
		SID_GDMAERR = 0x08, // error in graphics dma
		SID_GDMAEN  = 0x10, // graphics dma busy
		SID_GDMARDY = 0x20, // asserted at end of graphics dma
		SID_GDMARST = 0x40, // asserted in reset of graphics dma
		SID_VMERMW  = 0x80, // asserted in vme read-modify-write
	};

	enum lio_int_number : unsigned
	{
		LIO_D0     = 0, // duart 0 interrupt
		LIO_D1     = 1, // duart 1 interrupt
		LIO_VR     = 2, // vertical retrace interrupt
		LIO_CENTR  = 3, // parallel port interrupt
		LIO_SCSI   = 4, // scsi interrupt
		LIO_ENET   = 5, // ethernet interrupt
		LIO_GE     = 6, // ge interrupt
		LIO_FIFO   = 7, // fifo full interrupt
		LIO_AC     = 8, // vme ac fail interrupt
		LIO_VRSTAT = 9, // vert retrace status: no interrupt
	};

	enum mem_cfg_mask : u8
	{
		MCF_4MRAM    = 0x10,
		MCF_MEMSIZE  = 0x1f,
		MCF_TIMERDIS = 0x20, // reduce peripheral r/w strobe
		MCF_FMEM     = 0x40, // reduce cas pulse on reads
		MCF_REFDIS   = 0x80, // disable memory refresh
	};

	enum aux_ctl_mask : u8
	{
		AUX_LED = 0x0f, // diagnostic leds
		AUX_PE  = 0x10, // console led/eeprom program enable
		AUX_CS  = 0x20, // eeprom chip select
		AUX_CLK = 0x40, // serial clock
		AUX_GR  = 0x80, // graphics reset
	};

	u8 m_mem_cfg;
	u8 m_ctl_sid;
	u8 m_vme_isr;
	u8 m_vme_imr;
	u8 m_aux_ctl;
	u16 m_ctl_cpuctrl;
	attotime m_refresh_timer;

	u16 m_lio_isr;
	u8 m_lio_imr;
	bool m_lio_int;

	u16 m_scsi_dmalo;
	unsigned m_scsi_dmapage;
	std::unique_ptr<u16 []> m_dma_map;
};

void ip6_state::map(address_map &map)
{
	//map(0x10000000, 0x1bffffff); // vme a32 modifier 0x09 non-privileged
	//map(0x1c000000, 0x1cffffff); // vme a24 modifier 0x3d privileged
	//map(0x1e000000, 0x1effffff); // vme a24 modifier 0x39 non-privileged
	//map(0x1d000000, 0x1d00ffff); // vme a16 modifier 0x2d privileged
	//map(0x1d100000, 0x1d10ffff); // vme a16 modifier 0x29 non-privileged

	//map(0x1df00000, 0x1df00003).umask32(0x0000ff00); // VME_IACK: vme interrupt acknowledge

	map(0x1f800000, 0x1f800003).lw8("mem_cfg", [this](u8 data) { m_mem_cfg = data; }).umask32(0xff000000);
	map(0x1f800000, 0x1f800003).r(FUNC(ip6_state::ctl_sid_r)).umask32(0x00ff0000);

	map(0x1f840000, 0x1f840003).lrw8("vme_isr", [this]() { return m_vme_isr; }, [this](u8 data) { m_vme_isr = data; }).umask32(0x000000ff);
	map(0x1f840008, 0x1f84000b).lrw8("vme_imr", [this]() { return m_vme_imr; }, [this](u8 data) { m_vme_imr = data; }).umask32(0x000000ff);

	map(0x1f880000, 0x1f880003).lrw16("ctl_cpuctrl",
		[this]()
		{
			return m_ctl_cpuctrl;
		},
		[this](u16 data)
		{
			m_eeprom->di_write(BIT(data, 8));

			//BIT(data, 9); // reset system
			//BIT(data, 10); // enable parity checking
			//BIT(data, 11); // enable slave accesses
			//BIT(data, 12); // enable vme arbiter
			//BIT(data, 13); // write bad parity
			//BIT(data, 14); // watchdog enable
			//BIT(data, 15); // unused/fast peripheral cycle?

			m_ctl_cpuctrl = data;
		}).umask32(0x0000ffff);
	//map(0x1f8c0000, 0x1f8c0003); // lca readback trigger (b)
	map(0x1f8e0000, 0x1f8e0003).lrw8("aux_ctl",
		[this]()
		{
			return m_aux_ctl;
		},
		[this](u8 data)
		{
			// cpu leds
			m_leds[LED_HBT] = BIT(data, 0);
			m_leds[LED_CPU] = BIT(data, 1);
			m_leds[LED_GFX] = BIT(data, 2);
			m_leds[LED_FPU] = BIT(data, 3);

			//BIT(data, 4); // console led(?) & eeprom program enable
			m_eeprom->cs_write(BIT(data, 5));
			m_eeprom->clk_write(BIT(data, 6));
			//BIT(data, 7); // gfx_reset: reset graphics subsystem

			m_aux_ctl = data;
		}).umask32(0xff000000);

		map(0x1f900000, 0x1f900003).lrw16("scsi_dmalo_addr", [this]() { return m_scsi_dmalo; }, [this](u16 data) { m_scsi_dmalo = data; m_scsi_dmapage = 0; }).umask32(0x0000ffff);

	/*
	 * DMA address mapping table is a pair of CY7C128-35PC 2048x8 SRAMs which
	 * read/write to data bus D27-12. A10 is tied high, giving 1024 entries.
	 */
	map(0x1f920000, 0x1f920fff).lrw16("dma_map",
		[this](offs_t offset)
		{
			return m_dma_map[offset];
		},
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			mem_mask &= 0x0fff;
			COMBINE_DATA(&m_dma_map[offset]);
		}).umask32(0x0000ffff);

	//map(0x1f940000, 0x1f940003).umask32(?); // scsi_flush_addr

	map(0x1f950000, 0x1f9501ff).rw(m_enet, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w)).umask32(0xffff0000);
	map(0x1f960000, 0x1f960007).lr8("enet_reset", [this](offs_t offset) { m_enet->reset_w(!offset); return 0; }).umask32(0xff000000);

	map(0x1f980000, 0x1f980003).lr16("lio_isr", [this]() { return m_lio_isr; }).umask32(0x0000ffff);
	map(0x1f980008, 0x1f98000b).lrw8("lio_imr", [this]() { return m_lio_imr; }, [this](u8 data) { m_lio_imr = data; }).umask32(0x000000ff);

	map(0x1fa00000, 0x1fa00003).lr8("timer1_ack", [this]() { m_cpu->set_input_line(INPUT_LINE_IRQ4, 0); return 0; }).umask32(0xff000000);
	map(0x1fa20000, 0x1fa20003).lr8("timer0_ack", [this]() { m_cpu->set_input_line(INPUT_LINE_IRQ2, 0); return 0; }).umask32(0xff000000);

	//map(0x1fa40000, 0x1fa40000); // system bus error address
	map(0x1fa40004, 0x1fa40007).lrw32("refresh_timer",
		[this]()
		{
			return u32((machine().time() - m_refresh_timer).as_attoseconds() / ATTOSECONDS_PER_NANOSECOND);
		},
		[this](u32 data)
		{
			m_refresh_timer = machine().time() - attotime::from_nsec(data);
		});

	//map(0x1fa40008, 0x1fa4000b); // GDMA_DABR_PHYS descriptor array base register
	//map(0x1fa4000c, 0x1fa4000f); // GDMA_BUFADR_PHYS buffer address register
	//map(0x1fa40010, 0x1fa400013).umask32(0xffff0000); // GDMA_BURST_PHYS burst/delay register
	//map(0x1fa40010, 0x1fa400013).umask32(0x0000ffff); // GDMA_BUFLEN_PHYS buffer length register

	//map(0x1fa60000, 0x1fa600003); // VMA_RMW_ADDR

	map(0x1fa80000, 0x1fa80007).lr32("scsi_reset", [this](offs_t offset) { m_scsi->reset_w(!!offset); return 0; });

	//map(0x1fa80008, 0x1fa8000b); // IOC2 configuration register, bus error on IOC1

	//map(0x1faa0000, 0x1faa0003).umask32(0xff000000); // clear lan access bit
	//map(0x1faa0000, 0x1faa0003).umask32(0x00ff0000); // clear dma access bit
	//map(0x1faa0000, 0x1faa0003).umask32(0x0000ff00); // clear cpu access bit
	//map(0x1faa0000, 0x1faa0003).umask32(0x000000ff); // clear vme access bit
	//map(0x1faa0004, 0x1faa0007).umask32(0x00ff0000); // parity error register

	map(0x1fb00000, 0x1fb00003).rw(m_scsi, FUNC(wd33c93_device::indir_addr_r), FUNC(wd33c93_device::indir_addr_w)).umask32(0x00ff0000);
	map(0x1fb00100, 0x1fb00103).rw(m_scsi, FUNC(wd33c93_device::indir_reg_r), FUNC(wd33c93_device::indir_reg_w)).umask32(0x00ff0000);

	map(0x1fb40000, 0x1fb4000f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask32(0xff000000);

	map(0x1fb80000, 0x1fb800ff).lrw8("duarts",
		[this](offs_t offset)
		{
			return m_duart[BIT(offset, 0)]->read(offset >> 2);
		},
		[this](offs_t offset, u8 data)
		{
			m_duart[BIT(offset, 0)]->write(offset >> 2, data);
		}).umask32(0xff000000);

	map(0x1fbc0000, 0x1fbc007f).rw(m_rtc, FUNC(dp8573_device::read), FUNC(dp8573_device::write)).umask32(0xff000000);

	map(0x1fc00000, 0x1fc3ffff).rom().region("prom", 0);
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_SGI).machine_config(
		[](device_t *device)
		{
			downcast<nscsi_cdrom_device &>(*device).set_block_size(512);
		});
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void ip6_state::configure(machine_config &config)
{
	R2000A(config, m_cpu, 25_MHz_XTAL / 2, 16384, 8192);
	m_cpu->set_endianness(ENDIANNESS_BIG);
	m_cpu->set_addrmap(AS_PROGRAM, &ip6_state::map);
	m_cpu->set_fpu(mips1_device_base::MIPS_R2010A);
	m_cpu->in_brcond<0>().set([]() { return 1; }); // writeback complete

	// 16 SIMM slots with 1, 2 or 4MB SIMMs installed in sets of 4
	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("4M,8M,12M,32M,48M,64M");
	m_ram->set_default_value(0);

	EEPROM_93C56_16BIT(config, m_eeprom);

	DP8573(config, m_rtc); // DP8572AN

	PIT8254(config, m_pit);
	m_pit->set_clk<2>(3.6864_MHz_XTAL);
	m_pit->out_handler<0>().set_inputline(m_cpu, INPUT_LINE_IRQ2);
	m_pit->out_handler<1>().set_inputline(m_cpu, INPUT_LINE_IRQ4);
	m_pit->out_handler<2>().set(m_pit, FUNC(pit8254_device::write_clk0));
	m_pit->out_handler<2>().append(m_pit, FUNC(pit8254_device::write_clk1));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0").option_set("wd33c93", WD33C93).machine_config(
		[this](device_t *device)
		{
			wd33c93_device &wd33c93(downcast<wd33c93_device &>(*device));

			wd33c93.set_clock(10000000);
			wd33c93.irq_cb().set(*this, FUNC(ip6_state::lio_interrupt<LIO_SCSI>)).invert();
			wd33c93.drq_cb().set(*this, FUNC(ip6_state::scsi_drq));
		});
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, "cdrom", false);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, nullptr, false);

	AM7990(config, m_enet);
	m_enet->intr_out().set(FUNC(ip6_state::lio_interrupt<LIO_ENET>));
	m_enet->dma_in().set(
		[this](offs_t offset)
		{
			unsigned const page = 0x200 + (offset >> 12);
			u32 const address = (u32(m_dma_map[page]) << 12) | (offset & 0xfff);

			return m_cpu->space(0).read_word(address);
		});
	m_enet->dma_out().set(
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			unsigned const page = 0x200 + (offset >> 12);
			u32 const address = (u32(m_dma_map[page]) << 12) | (offset & 0xfff);

			m_cpu->space(0).write_word(address, data, mem_mask);
		});

	// duart 0 (keyboard/mouse)
	SCN2681(config, m_duart[0], 3.6864_MHz_XTAL); // SCN2681AC1N24
	sgi_keyboard_port_device &keyboard_port(SGIKBD_PORT(config, "keyboard_port", default_sgi_keyboard_devices, "hlekbd"));
	rs232_port_device &mouse_port(RS232_PORT(config, "mouse_port",
		[](device_slot_interface &device)
		{
			device.option_add("mouse", SGI_HLE_SERIAL_MOUSE);
		},
		"mouse"));

	// duart 0 outputs
	m_duart[0]->irq_cb().set(FUNC(ip6_state::lio_interrupt<LIO_D0>)).invert();
	m_duart[0]->a_tx_cb().set(keyboard_port, FUNC(sgi_keyboard_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(mouse_port, FUNC(rs232_port_device::write_txd));

	// duart 0 inputs
	keyboard_port.rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	mouse_port.rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));

	// duart 1 (serial ports)
	SCN2681(config, m_duart[1], 3.6864_MHz_XTAL); // SCN2681AC1N40
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	// duart 1 outputs
	m_duart[1]->irq_cb().set(FUNC(ip6_state::lio_interrupt<LIO_D1>)).invert();
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
}

void ip6_state::initialize()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());

	m_ctl_sid = SID_FPPRES;

	m_lio_isr = 0x3ff;
	m_lio_imr = 0;
	m_lio_int = false;

	m_refresh_timer = machine().time();

	m_dma_map = make_unique_clear<u16 []>(2048);

	m_leds.resolve();
}

void ip6_state::lio_interrupt(unsigned number, int state)
{
	u16 const mask = 1 << number;

	// record interrupt state
	if (!state)
		m_lio_isr &= ~mask;
	else
		m_lio_isr |= mask;

	// update interrupt line
	bool const lio_int = ~m_lio_isr & m_lio_imr;
	if (m_lio_imr ^ lio_int)
	{
		m_lio_int = lio_int;
		m_cpu->set_input_line(INPUT_LINE_IRQ1, m_lio_int);
	}
}

void ip6_state::scsi_drq(int state)
{
	if (state)
	{
		u32 const address = (u32(m_dma_map[m_scsi_dmapage]) << 12) | (m_scsi_dmalo & 0x0fff);

		if (m_scsi_dmalo & 0x8000)
			m_cpu->space(0).write_byte(address, m_scsi->dma_r());
		else
			m_scsi->dma_w(m_cpu->space(0).read_byte(address));

		m_scsi_dmalo = (m_scsi_dmalo & 0xf000) | ((m_scsi_dmalo + 1) & 0x0fff);
		if (!(m_scsi_dmalo & 0x0fff))
			m_scsi_dmapage++;
	}
}

u8 ip6_state::ctl_sid_r()
{
	u8 data = m_ctl_sid;

	if (m_eeprom->do_read())
		data |= SID_SERDATA;

	if (m_aux_ctl & AUX_CLK)
		data |= SID_SERCLK;

	return data;
}

// FIXME: boot prom is in fact four 27C512 eproms
ROM_START(4d20)
	ROM_REGION32_BE(0x40000, "prom", 0)
	ROM_LOAD("4d202031.bin", 0x000000, 0x040000, CRC(065a290a) SHA1(6f5738e79643f94901e6efe3612468d14177f65b))
ROM_END

//   YEAR  NAME  PARENT  COMPAT  MACHINE    INPUT  CLASS      INIT        COMPANY                 FULLNAME  FLAGS
COMP(1988, 4d20, 0,      0,      configure, 0,     ip6_state, initialize, "Silicon Graphics Inc", "4D/20",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
