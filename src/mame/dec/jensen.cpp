// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of the DECpc AXP/150 (code named Jensen), nearly identical
 * DEC 2000 Model 300 AXP, and DEC 2000 Model 500 AXP (code named Culzean).
 *
 * References:
 *
 *   ftp://ftp.netbsd.org/pub/NetBSD/misc/dec-docs/ek-a0638-td.pdf.gz
 *   http://www.decuslib.com/decus/vlt97a/everhart/jensen.info
 *
 * TODO
 *   - everything (skeleton only)
 *
 */
/*
 * WIP notes
 *
 * flash contains two compressed (deflate) blocks:
 *
 *   block 0 offset 0x71c00 compressed 0x031737 decompressed 0x72800
 *   block 1 offset 0xd2008 compressed 0x01c31b decompressed 0x47a00
 */
#include "emu.h"

#include "cpu/alpha/alpha.h"
#include "cpu/alpha/alphad.h"

 // memory
#include "machine/ram.h"
#include "machine/xc1700e.h"
#include "machine/intelfsh.h"

// various hardware
#include "machine/i82357.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class jensen_state : public driver_device
{
public:
	jensen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_srom(*this, "srom")
		, m_feprom(*this, "feprom%u", 0)
		, m_isp(*this, "isp")
	{
	}

	// machine config
	void jensen(machine_config &config);

	void d2k300axp(machine_config &config);
	void d2k500axp(machine_config &config);
	void dpcaxp150(machine_config &config);

	void init_common();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void local_memory(address_map &map) ATTR_COLD;
	void local_io(address_map &map) ATTR_COLD;
	void eisa_memory(address_map &map) ATTR_COLD;
	void eisa_io(address_map &map) ATTR_COLD;

private:
	// devices
	required_device<alpha_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<xc1765e_device> m_srom;
	required_device_array<intel_e28f008sa_device, 2> m_feprom;

	required_device<i82357_device> m_isp;

	// machine state
	u8 m_hae = 0;
	u8 m_sysctl = 0;
	u8 m_spare = 0;
};

void jensen_state::machine_start()
{
}

void jensen_state::machine_reset()
{
}

void jensen_state::init_common()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, 0x00000000 | m_ram->mask(), m_ram->pointer());
}

void jensen_state::local_memory(address_map &map)
{
	//map(0x00000000, 0x07ffffff); // valid range
}

void jensen_state::local_io(address_map &map)
{
	map(0x00000000, 0x0000001f); // EISA INTA cycle
	map(0x80000000, 0x9fffffff).lr8(NAME([this] (offs_t offset) { return m_feprom[0]->read(offset >> 9); }));
	map(0xa0000000, 0xbfffffff).lr8(NAME([this] (offs_t offset) { return m_feprom[1]->read(offset >> 9); }));

	//map(0xc0000000, 0xc1ffffff); // vl82c106 bits 24:9 -> vlsi 15:0 i.e. >> 9

	map(0xd0000000, 0xd0000000).lrw8(NAME([this] () { return m_hae; }), NAME([this] (u8 data) { m_hae = data & 0x7f; }));
	map(0xe0000000, 0xe0000000).lrw8(NAME([this] () { return m_sysctl; }), NAME([this] (u8 data) { m_sysctl = data; logerror("led %x\n", data & 0xf); }));
	map(0xf0000000, 0xf0000000).lrw8(NAME([this] () { return m_spare; }), NAME([this] (u8 data) { m_spare = data; }));
}

void jensen_state::eisa_memory(address_map &map)
{
	map(0x00000000, 0xffffffff).lrw8(
			NAME([this] (offs_t offset)
			{
				LOG("eisa_memory_r 0x%08x\n", (u32(m_hae) << 25) | (offset >> 7));

				return 0;
			}),
			NAME([this] (offs_t offset, u8 data)
			{
				LOG("eisa_memory_w 0x%08x data 0x%02x\n", (u32(m_hae) << 25) | (offset >> 7), data);
			}));
}

void jensen_state::eisa_io(address_map &map)
{
	map(0x00000000, 0xffffffff).lrw8(
			NAME([this] (offs_t offset)
			{
				LOG("eisa_io_r offset 0x%08x address 0x%08x count %d (%s)\n", offset,
					(u32(m_hae) << 25) | (offset >> 7), (offset >> 5) & 3, machine().describe_context());

				return 0;
			}),
			NAME([this] (offs_t offset, u8 data)
			{
				LOG("eisa_io_w offset 0x%08x address 0x%08x count %d data 0x%02x (%s)\n",
					offset, (u32(m_hae) << 25) | (offset >> 7), (offset >> 5) & 3, data, machine().describe_context());
			}));
}

void jensen_state::jensen(machine_config &config)
{
	DEC_21064(config, m_cpu, 300'000'000).set_clock_scale(0.5);
	m_cpu->set_dasm_type(alpha_disassembler::dasm_type::TYPE_NT);
	m_cpu->set_addrmap(0, &jensen_state::local_memory);
	m_cpu->set_addrmap(2, &jensen_state::local_io);
	m_cpu->set_addrmap(4, &jensen_state::eisa_memory);
	m_cpu->set_addrmap(6, &jensen_state::eisa_io);
	m_cpu->srom_oe_w().set(m_srom, FUNC(xc1765e_device::reset_w));
	m_cpu->srom_data_r().set(m_srom, FUNC(xc1765e_device::data_r));

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("32M,64M,80M,128M");
	m_ram->set_default_value(0);

	XC1765E(config, m_srom);

	// TODO: 1x1M + 1x256K flash?
	INTEL_E28F008SA(config, m_feprom[0]);
	INTEL_E28F008SA(config, m_feprom[1]);

	// keyboard connector
	[[maybe_unused]] pc_kbdc_device &kbd_con(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	//kbd_con.out_clock_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	//kbd_con.out_data_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::kbd_data_w));

	// TODO: VL82C106 (rtc, dual serial, parallel, dual ps/2)
	// TODO: 18.432 MHz crystal
#if 0
	rs232_port_device &com1(RS232_PORT(config, "com1", default_rs232_devices, nullptr));
	m_ace[0]->out_dtr_callback().set(serial0, FUNC(rs232_port_device::write_dtr));
	m_ace[0]->out_rts_callback().set(serial0, FUNC(rs232_port_device::write_rts));
	m_ace[0]->out_tx_callback().set(serial0, FUNC(rs232_port_device::write_txd));
	m_ace[0]->out_int_callback().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<8>));

	serial0.cts_handler().set(m_ace[0], FUNC(ns16550_device::cts_w));
	serial0.dcd_handler().set(m_ace[0], FUNC(ns16550_device::dcd_w));
	serial0.dsr_handler().set(m_ace[0], FUNC(ns16550_device::dsr_w));
	serial0.ri_handler().set(m_ace[0], FUNC(ns16550_device::ri_w));
	serial0.rxd_handler().set(m_ace[0], FUNC(ns16550_device::rx_w));

	rs232_port_device &com2(RS232_PORT(config, "com2", default_rs232_devices, nullptr));
	m_ace[1]->out_dtr_callback().set(serial1, FUNC(rs232_port_device::write_dtr));
	m_ace[1]->out_rts_callback().set(serial1, FUNC(rs232_port_device::write_rts));
	m_ace[1]->out_tx_callback().set(serial1, FUNC(rs232_port_device::write_txd));
	m_ace[1]->out_int_callback().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<9>));

	serial1.cts_handler().set(m_ace[1], FUNC(ns16550_device::cts_w));
	serial1.dcd_handler().set(m_ace[1], FUNC(ns16550_device::dcd_w));
	serial1.dsr_handler().set(m_ace[1], FUNC(ns16550_device::dsr_w));
	serial1.ri_handler().set(m_ace[1], FUNC(ns16550_device::ri_w));
	serial1.rxd_handler().set(m_ace[1], FUNC(ns16550_device::rx_w));

	PC_LPT(config, m_lpt, 0);
	m_lpt->irq_handler().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<0>));
#endif

	I82357(config, m_isp, 14.318181_MHz_XTAL);
	m_isp->out_int_cb().set_inputline(m_cpu, INPUT_LINE_IRQ2);
	m_isp->out_nmi_cb().set_inputline(m_cpu, INPUT_LINE_IRQ3);

	// TODO: six EISA slots

	// DECpc AXP 150 usually fitted with:
	// 30-40383-01 Adaptec AHA-1742A (SCSI + floppy)
	// 30-40385-01 Compaq QVision 1024/E (1MB VGA)

	// Optional
	// DE422-SA Digital EISA Ethernet Controller (10 Mbps TP and BNC connectors)
}

void jensen_state::d2k300axp(machine_config &config)
{
	jensen(config);
}

void jensen_state::d2k500axp(machine_config &config)
{
	jensen(config);
}

void jensen_state::dpcaxp150(machine_config &config)
{
	jensen(config);
}

ROM_START(d2k300axp)
	ROM_REGION32_LE(0x2000, "srom", 0)
	ROM_LOAD("srom.bin", 0x00000, 0x2000, NO_DUMP)

	ROM_REGION32_LE(0x100000, "feprom1", 0)
	ROM_SYSTEM_BIOS(0, "feprom", "Unknown version")
	ROMX_LOAD("feprom1.bin", 0x00000, 0x100000, NO_DUMP, ROM_BIOS(0))
ROM_END

ROM_START(d2k500axp)
	ROM_REGION32_LE(0x2000, "srom", 0)
	ROM_LOAD("srom.bin", 0x00000, 0x2000, NO_DUMP)

	ROM_REGION32_LE(0x100000, "feprom1", 0)
	ROM_SYSTEM_BIOS(0, "feprom", "Unknown version")
	ROMX_LOAD("feprom1.bin", 0x00000, 0x100000, NO_DUMP, ROM_BIOS(0))
ROM_END

ROM_START(dpcaxp150)
	/*
	 * This region corresponds to the 8KiB Xilinx XC1765 serial prom which is
	 * loaded into the Alpha primary instruction cache at reset. This dump is
	 * stored least-significant bit first (which suits the way it's loaded by
	 * the Alpha, and was the default treatment by the dumping device).
	 */
	ROM_REGION32_LE(0x2000, "srom", 0)
	ROM_LOAD("005v2__ax06.bin", 0x00000, 0x2000, CRC(d017ea39) SHA1(4d2d11ac94adebe797e749f6ef1e1e523bd1e9f3))

	/*
	 * This region corresponds to FEPROM1, the 1MiB Intel E28F008SA flash
	 * memory. Documentation indicates FEPROM0 may be either 256KiB or 1MiB,
	 * and may map to the unpopulated nearby PLCC socket. Some revisions of the
	 * main board and the documentation indicate a SIMM of some kind rather
	 * than directly mounted memory devices, but presumably have equivalent
	 * sizes and content.
	 */
	ROM_REGION32_LE(0x100000, "feprom1", 0)

	// source: extracted from ftp://ftp.hp.com/pub/alphaserver/firmware/retired_platforms/alphapc/dec2000_axp150/dec2000_v2_2.exe
	ROM_SYSTEM_BIOS(0, "v22", "Version 2.2, 12-FEB-1996")
	ROMX_LOAD("001z5__bl07.v22", 0x00000, 0x100000, CRC(1edb9c98) SHA1(a45f0dde236e189a57afc1ed354201180ab2f234), ROM_BIOS(0))

	// source: extracted from https://archive.org/download/ntrisc/alpha/ftp.alphant.com/Drivers/fw150v431.zip
	ROM_SYSTEM_BIOS(1, "v19", "Version 1.9, 22-JUN-1995")
	ROMX_LOAD("001z5__bl07.v19", 0x00000, 0x100000, CRC(53e981ee) SHA1(c726981441af88d8a224b1f81efee3a6ec95f227), ROM_BIOS(1))

	// source: extracted from https://archive.org/download/decpcaxp/DEC%20PC%20AXP%20Firmware%20Upgrade%20No9%20GXE.img
	ROM_SYSTEM_BIOS(2, "v13", "Version 1.3, 10-JUN-1994")
	ROMX_LOAD("001z5__bl07.v13", 0x00000, 0x100000, CRC(8035f370) SHA1(2ebd75267ab7373d344efe44e700645ed31b44cd), ROM_BIOS(2))

	// source: extracted from https://archive.org/download/ntrisc/alpha/ftp.alphant.com/Drivers/fw35-1.zip
	ROM_SYSTEM_BIOS(3, "vff", "Version f.f, 19-MAY-1994")
	ROMX_LOAD("001z5__bl07.vff", 0x00000, 0x100000, CRC(99238153) SHA1(ce543bd11937bdf24c4d9e898e917438c2163a20), ROM_BIOS(3))
ROM_END

}

/*    YEAR   NAME       PARENT  COMPAT  MACHINE    INPUT  CLASS         INIT         COMPANY  FULLNAME                  FLAGS */
COMP( 1993,  d2k300axp, 0,      0,      d2k300axp, 0,     jensen_state, init_common, "DEC",   "DEC 2000 Model 300 AXP", MACHINE_IS_SKELETON)
COMP( 1993,  d2k500axp, 0,      0,      d2k500axp, 0,     jensen_state, init_common, "DEC",   "DEC 2000 Model 500 AXP", MACHINE_IS_SKELETON)
COMP( 1993,  dpcaxp150, 0,      0,      dpcaxp150, 0,     jensen_state, init_common, "DEC",   "DECpc AXP 150",          MACHINE_IS_SKELETON)
