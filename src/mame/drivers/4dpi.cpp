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
 *   - http://www.futuretech.blinkenlights.nl/pitechrep.html
 *   - https://hardware.majix.org/computers/sgi.pi/index.shtml
 *   - http://archive.irix.cc/sgistuff/hardware/systems/personal.html
 *   - https://github.com/NetBSD/src/tree/trunk/sys/arch/sgimips/
 *
 * TODO:
 *   - IOC1 and CTL1
 *   - graphics, keyboard, mouse, audio
 *
 * Status:
 *   - hangs waiting for wd33c93 reset interrupt
 *   - if bypassed, hangs waiting on a timer(?) at bfa40004
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
		, m_net(*this, "net")
		, m_duart(*this, "duart%u", 0U)
		, m_serial(*this, "serial%u", 0U)
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
	required_device<am7990_device> m_net;
	required_device_array<scn2681_device, 2> m_duart;
	required_device_array<rs232_port_device, 2> m_serial;

	void map(address_map &map);

	u8 m_ctl_sysid;
	u8 m_aux_cpuctrl;
	u16 m_ctl_cpuctrl;

	u8 m_int_mask;
};

void ip6_state::map(address_map &map)
{
	//map(0x10000000, 0x1bffffff); // a32 kernel vme bus, vme modifier 0x09
	//map(0x1c000000, 0x1cffffff); // a24 kernel vme bus, vme modifier 0x3d
	//map(0x1e000000, 0x1effffff); // a24 kernel vme bus, vme modifier 0x39

	//map(0x1f900000, 0x1f900000); // i/o controller (also lance and scsi)

	map(0x1f800000, 0x1f800003).lr8("ctl_sysid", [this]() { return m_ctl_sysid; }).umask32(0x00ff0000);

	map(0x1f840008, 0x1f84000b).ram().umask32(0x000000ff); // vme mask?

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
			//BIT(data, 14); // watchdog enable

			m_ctl_cpuctrl = data;
		}).umask32(0x0000ffff);
	map(0x1f8e0000, 0x1f8e0003).lrw8("aux_cpuctrl",
		[this]()
		{
			return m_aux_cpuctrl;
		},
		[this](u8 data)
		{
			//BIT(data, 1); // heartbeat
			//BIT(data, 4); // enable console led(?)/eeprom program enable?
			m_eeprom->cs_write(BIT(data, 5));
			m_eeprom->clk_write(BIT(data, 6));

			m_aux_cpuctrl = data;
		}).umask32(0xff000000);

	map(0x1f950000, 0x1f9501ff).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w)).umask32(0xffff0000);

	// local interrupt goes to cpu interrupt 1
	//map(0x1f980000, 0x1f980003).r().umask32(0x0000ffff);  // int1_local_status
	map(0x1f980008, 0x1f98000b).lrw8("int_mask", [this]() { return m_int_mask; }, [this](u8 data) { m_int_mask = data; }).umask32(0x000000ff); // int1_local_mask

	map(0x1fa00000, 0x1fa00003).lr8("timer1_ack", [this]() { m_cpu->set_input_line(INPUT_LINE_IRQ4, 0); return 0; }).umask32(0xff000000);
	map(0x1fa20000, 0x1fa20003).lr8("timer0_ack", [this]() { m_cpu->set_input_line(INPUT_LINE_IRQ2, 0); return 0; }).umask32(0xff000000);
	//map(0x1fa40000, 0x1fa4000f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask32(0xff000000); // NetBSD says 0x1fb40000?

	map(0x1fa80000, 0x1fa80007).lr32("scsi_reset", [this](offs_t offset) { m_scsi->reset_w(!!offset); return 0; });

	// TODO: firmware resets wd33c93 and expects it to produce an interrupt - device emulation is currently wrong
	map(0x1fb00000, 0x1fb00003).rw(m_scsi, FUNC(wd33c93_device::indir_addr_r), FUNC(wd33c93_device::indir_addr_w)).umask32(0x00ff0000);
	map(0x1fb00100, 0x1fb00103).rw(m_scsi, FUNC(wd33c93_device::indir_reg_r), FUNC(wd33c93_device::indir_reg_w)).umask32(0x00ff0000);

	map(0x1fb40000, 0x1fb4000f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask32(0xff000000);

	map(0x1fb80000, 0x1fb800ff).lrw8("duart0",
		[this](offs_t offset)
		{
			return m_duart[0]->read(offset >> 2);
		},
		[this](offs_t offset, u8 data)
		{
			m_duart[0]->write(offset >> 2, data);
		}).umask32(0xff000000);

	map(0x1fbc0000, 0x1fbc00ff).rw(m_rtc, FUNC(dp8573_device::read), FUNC(dp8573_device::write)).umask32(0xff000000);

	map(0x1fc00000, 0x1fc3ffff).rom().region("prom", 0);
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
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
	m_eeprom->do_callback().set(
		[this](int state)
		{
			if (state)
				m_ctl_sysid |= 0x01;
			else
				m_ctl_sysid &= ~0x01;
		});

	DP8573(config, m_rtc);

	PIT8254(config, m_pit);
	m_pit->set_clk<2>(3.6864_MHz_XTAL);
	m_pit->out_handler<0>().set_inputline(m_cpu, INPUT_LINE_IRQ2);
	m_pit->out_handler<1>().set_inputline(m_cpu, INPUT_LINE_IRQ4);
	m_pit->out_handler<2>().set(m_pit, FUNC(pit8254_device::write_clk0));
	m_pit->out_handler<2>().append(m_pit, FUNC(pit8254_device::write_clk1));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0").option_set("wd33c93", WD33C93).machine_config(
		[](device_t *device)
		{
			device->set_clock(10000000);

			// TODO: connect irq and drq
		});
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, "cdrom", false);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, nullptr, false);

	AM7990(config, m_net);
	// TODO: irq, drq, memory accessors (via ioc)

	// duarts
	// NOTE: one of these is has 40 pins, while the other has only 24 pins; most
	// likely the keyboard/mouse one is without and doesn't have any flow control
	SCN2681(config, m_duart[0], 3.6864_MHz_XTAL);
	SCN2681(config, m_duart[1], 3.6864_MHz_XTAL); // TODO: probably for the keyboard and mouse
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	// duart 0 outputs
	// TODO: irq
	m_duart[0]->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_duart[0]->outport_cb().set(
		[this](u8 data)
		{
			m_serial[0]->write_rts(BIT(data, 0));
			m_serial[1]->write_rts(BIT(data, 1));
			m_serial[0]->write_dtr(BIT(data, 2));
			m_serial[1]->write_dtr(BIT(data, 3));

			// TODO: bit 4-7
		});

	// duart inputs (guessed for now)
	m_serial[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_serial[0]->cts_handler().set(m_duart[0], FUNC(scn2681_device::ip0_w));
	m_serial[0]->dsr_handler().set(m_duart[0], FUNC(scn2681_device::ip3_w));
	m_serial[0]->dcd_handler().set(m_duart[0], FUNC(scn2681_device::ip4_w));

	m_serial[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));
	m_serial[1]->cts_handler().set(m_duart[0], FUNC(scn2681_device::ip1_w));
	m_serial[1]->dsr_handler().set(m_duart[0], FUNC(scn2681_device::ip5_w));
	m_serial[1]->dcd_handler().set(m_duart[0], FUNC(scn2681_device::ip6_w));
}

void ip6_state::initialize()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());

	m_ctl_sysid = 0x02; // fpu present
}

ROM_START(4d20)
	ROM_REGION32_BE(0x40000, "prom", 0)
	ROM_LOAD("4d202031.bin", 0x000000, 0x040000, CRC(065a290a) SHA1(6f5738e79643f94901e6efe3612468d14177f65b))
ROM_END

//   YEAR  NAME  PARENT  COMPAT  MACHINE    INPUT  CLASS      INIT        COMPANY                 FULLNAME  FLAGS
COMP(1988, 4d20, 0,      0,      configure, 0,     ip6_state, initialize, "Silicon Graphics Inc", "4D/20",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
