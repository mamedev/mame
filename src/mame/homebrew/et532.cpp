// license:BSD-3-Clause
// copyright-holders:Dave Rand

/*
 * George Scolaro's ET532 — an Ethernet/serial variant of the pc532 (532 Baby AT).
 *
 * The ET532 was designed (rev 1A, 1989) but never built.  It is a pc532 superset:
 * the same NS32532 CPU cluster + DP8490 SCSI, plus an on-board DP8390 Ethernet
 * controller (BNC/Cheapernet + AUI, MAC in a 93C46) and two SCC2698 octal UARTs
 * giving 16 serial lines.  It can run stand-alone (jumper J4) or plug into a 532
 * system over the 532SC bus (CONN1).  This driver models the stand-alone board.
 *
 * This is a fresh bring-up of a board that was never built.  There is no original
 * ET532 firmware, so the EPROM here is a purpose-built bring-up monitor written
 * from scratch with the Definicon DSI-32 NS32k toolchain: it boots, prints a
 * banner, echoes the console, and offers examine/deposit/go plus console and
 * block-0 SCSI loaders with a signature+checksum autoboot.  Marked
 * MACHINE_NOT_WORKING — the DP8390 Ethernet and 15 of the 16 serial channels are
 * not yet wired, and the board never physically existed.
 *
 * The address decode and peripheral map below come from George's original PAL
 * sources (DEC32.TDL et al., recovered from the design archive) and the schematic.
 *
 * Sources:
 *  - ET532 schematic (PROJECT:ET532, rev 1A, (C) 1988-90 George Scolaro)
 *  - PAL equations: KSER/PALS/{DEC32,DRAMC,DRAMEN,WAIT,COPETH}.TDL
 *  - pc532.cpp (Patrick Mackinlay) — the device-wiring template this is based on
 *
 * Memory map (DEC32 PAL):
 *   0000'0000-03ff'ffff  EPROM (read)                      single 8-bit 27C512 on BD0-7
 *   0400'0000-07ff'ffff  DRAM
 *   0800'0000-0fff'ffff  COPS (93C46 serial EEPROM, MAC)
 *   1000'0000-17ff'ffff  on-board peripherals (PERBD), sub-decoded by the 74F138 (U30)
 *                        from A22-A24:  10000000 OCT0  10400000 OCT1
 *                                       11000000 SCSI  11400000 SCSID
 *                                       11800000 ETHER 11c00000 ETHERD
 *   1800'0000-1fff'ffff  off-board peripherals (PEROBD, 532SC bus)
 *   2000'0000-27ff'ffff  SCSI/Ethernet CPU-driven pseudo-DMA (IPERDMA)
 *
 * TODO:
 *  - no NS32202 on the ET532: interrupts OR into the NS32532 /INT (non-vectored),
 *    software polls device status to find the source (no interrupt-status register
 *    exists in the rev-1A schematic)
 *  - confirm the 74F138 sub-decode offsets against silicon
 *  - wire the remaining 15 serial channels (only the console, chip0:A, is wired)
 *  - 532SC expansion-bus (PEROBD + /IR1-3) modelling for plug-in boards
 */

#include "emu.h"

// cpu cluster
#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"

// other devices
#include "machine/ncr5380.h"   // dp8490
#include "machine/nscsi_bus.h"
#include "machine/scc2698b.h"  // octal UARTs
#include "machine/dp8390.h"    // ethernet
#include "machine/eepromser.h"
#include "machine/input_merger.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class et532_state : public driver_device
{
public:
	et532_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_scsi(*this, "dp8490")
		, m_net(*this, "dp8390")
		, m_eeprom(*this, "eeprom")
		, m_duart(*this, "duart%u", 0U)
		, m_serial(*this, "serial%u", 0U)
		, m_irqs(*this, "irqs")
	{
	}

	void et532(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;

	required_device<ns32532_device> m_cpu;
	required_device<ns32381_device> m_fpu;

	required_device<dp8490_device> m_scsi;
	required_device<dp8390d_device> m_net;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	required_device_array<scc2698b_device, 2> m_duart;
	required_device_array<rs232_port_device, 16> m_serial;
	required_device<input_merger_device> m_irqs;

private:
	// SCSI CPU-driven pseudo-DMA (identical mechanism to the pc532)
	void drq_w(int state);
	void irq_w(int state);
	u32 dma_r(offs_t offset, u32 mem_mask);
	void dma_w(offs_t offset, u32 data, u32 mem_mask);

	// DP8390 packet-buffer SRAM (the 6264) and the COPS serial EEPROM
	u8 net_ram_r(offs_t offset);
	void net_ram_w(offs_t offset, u8 data);
	u8 cops_r(offs_t offset);
	void cops_w(offs_t offset, u8 data);

	std::unique_ptr<u8[]> m_net_ram; // 8KB DP8390 packet buffer

	bool m_drq;
	bool m_irq;
	u32 m_dma;
	enum dma_state : unsigned
	{
		IDLE,
		WR1, WR2, WR3,
		RD1, RD2, RD3, RD4,
	}
	m_state;
};

void et532_state::machine_start()
{
	m_net_ram = std::make_unique<u8[]>(0x2000);

	save_pointer(NAME(m_net_ram), 0x2000);
	save_item(NAME(m_drq));
	save_item(NAME(m_irq));
	save_item(NAME(m_dma));
	// m_state is transient pseudo-DMA burst state; not saved (resets to IDLE)
}

void et532_state::machine_reset()
{
	m_drq = false;
	m_irq = false;
	m_dma = 0;
	m_state = IDLE;
}

// DP8390 local packet RAM (the 6264 SRAM), accessed by the NIC's DMA engine
u8 et532_state::net_ram_r(offs_t offset)
{
	return m_net_ram[offset & 0x1fff];
}

void et532_state::net_ram_w(offs_t offset, u8 data)
{
	m_net_ram[offset & 0x1fff] = data;
}

// COPS: the 93C46 serial EEPROM holding the Ethernet MAC, bit-banged via the COPETH
// GAL (U19).  Bit assignment from COPETH.TDL: write bit0=CS, bit1=SK, bit2=DI; read bit0=DO.
u8 et532_state::cops_r(offs_t offset)
{
	return m_eeprom->do_read();
}

void et532_state::cops_w(offs_t offset, u8 data)
{
	m_eeprom->cs_write(BIT(data, 0));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->di_write(BIT(data, 2));
}

/*
 * CPU-driven pseudo-DMA for the SCSI controller, using the NS32532 dynamic bus
 * sizing and cycle extension, exactly as on the pc532.  See pc532.cpp for the
 * detailed description of the protocol.
 */
void et532_state::drq_w(int state)
{
	if (state)
	{
		switch (m_state)
		{
		case RD1: m_dma |= u32(m_scsi->dma_r()) << 8; m_state = RD2; break;
		case RD2: m_dma |= u32(m_scsi->dma_r()) << 16; m_state = RD3; break;
		case RD3: m_dma |= u32(m_scsi->dma_r()) << 24; m_state = RD4; break;

		case WR3: m_scsi->dma_w(m_dma >> 8); m_state = WR2; break;
		case WR2: m_scsi->dma_w(m_dma >> 16); m_state = WR1; break;
		case WR1: m_scsi->dma_w(m_dma >> 24); m_state = IDLE; break;

		default:
			break;
		}
	}

	m_drq = state;
}

void et532_state::irq_w(int state)
{
	if (state)
	{
		switch (m_state)
		{
		case RD1:
		case RD2:
		case RD3:
			m_state = RD4;
			break;

		case WR3:
		case WR2:
		case WR1:
			m_state = IDLE;
			break;

		default:
			break;
		}
	}

	m_irq = state;
}

u32 et532_state::dma_r(offs_t offset, u32 mem_mask)
{
	u32 data = 0;

	if (!machine().side_effects_disabled())
	{
		switch (m_state)
		{
		case IDLE:
			if (m_drq && !m_irq)
			{
				m_dma = m_scsi->dma_r();
				m_state = RD1;

				m_cpu->rdy_w(1);
			}
			break;

		case RD1:
		case RD2:
		case RD3:
			m_cpu->rdy_w(1);
			break;

		case RD4:
			data = m_dma;
			m_state = IDLE;
			break;

		default:
			break;
		}
	}

	return data;
}

void et532_state::dma_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_state == IDLE)
	{
		if (m_drq)
		{
			m_dma = data;
			m_scsi->dma_w(m_dma >> 0);
			m_state = WR3;
		}
	}
	else
		m_cpu->rdy_w(1);
}

template <unsigned ST> void et532_state::cpu_map(address_map &map)
{
	// EPROM at reset (the NS32532 fetches from 0); single 8-bit device on the
	// peripheral bus, read-only.  DRAM follows at 0400'0000.
	map(0x0000'0000, 0x0000'ffff).rom().region("eprom", 0);
	map(0x0400'0000, 0x047f'ffff).ram().share("ram"); // 8MB DRAM (size TBD)

	// COPS: 93C46 serial EEPROM (Ethernet MAC)
	map(0x0800'0000, 0x0800'0000).rw(FUNC(et532_state::cops_r), FUNC(et532_state::cops_w));

	// on-board peripherals (PERBD), 74F138 (U30) sub-decode of A22-A24
	map(0x1000'0000, 0x1000'003f).m(m_duart[0], FUNC(scc2698b_device::map)); // OCT0: serial 0-7
	map(0x1040'0000, 0x1040'003f).m(m_duart[1], FUNC(scc2698b_device::map)); // OCT1: serial 8-15
	map(0x1100'0000, 0x1100'0007).m(m_scsi, FUNC(dp8490_device::map));       // SCSI registers
	map(0x1180'0000, 0x1180'000f).rw(m_net, FUNC(dp8390d_device::cs_read), FUNC(dp8390d_device::cs_write)); // ETHER
	// ETHERD: byte-wide remote-DMA data port (DP8390 is on the 8-bit peripheral bus)
	map(0x11c0'0000, 0x11c0'0000).lrw8(
			NAME([this](offs_t offset) { return u8(m_net->remote_read()); }),
			NAME([this](offs_t offset, u8 data) { m_net->remote_write(data); }));

	// SCSI/Ethernet CPU-driven pseudo-DMA (IPERDMA)
	map(0x2000'0000, 0x27ff'ffff).rw(FUNC(et532_state::dma_r), FUNC(et532_state::dma_w));

	// No ICU: the ET532 has no NS32202.  Interrupt sources are wired together to
	// the NS32532 /INT (non-vectored); software polls the device status registers
	// (DP8490, DP8390, the two SCC2698s) to find the source.
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tape", NSCSI_TAPE);
}

void et532_state::et532(machine_config &config)
{
	NS32532(config, m_cpu, 40_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &et532_state::cpu_map<0>);
	m_cpu->set_addrmap(ns32000::ST_IAM, &et532_state::cpu_map<ns32000::ST_IAM>);
	m_cpu->set_addrmap(ns32000::ST_ODT, &et532_state::cpu_map<ns32000::ST_ODT>);

	NS32381(config, m_fpu, 40_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	// The ET532 has no NS32202 ICU.  OR all maskable interrupt sources to the
	// NS32532 /INT input; the CPU runs non-vectored and software polls to resolve.
	INPUT_MERGER_ANY_HIGH(config, m_irqs);
	m_irqs->output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	// SCSI — DP8490 on the (shared) 532SC bus; CPU-driven pseudo-DMA
	auto &scsi(NSCSI_BUS(config, "scsi"));
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, "tape", false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);

	DP8490(config, m_scsi);
	scsi.set_external_device(7, m_scsi);
	m_scsi->drq_handler().set(DEVICE_SELF, FUNC(et532_state::drq_w));
	m_scsi->irq_handler().append(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_scsi->irq_handler().append(DEVICE_SELF, FUNC(et532_state::irq_w));

	// Ethernet — DP8390 + 8KB packet buffer (6264) + 93C46 MAC EEPROM
	DP8390D(config, m_net);
	m_net->irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_net->mem_read_callback().set(FUNC(et532_state::net_ram_r));
	m_net->mem_write_callback().set(FUNC(et532_state::net_ram_w));

	EEPROM_93C46_16BIT(config, m_eeprom);

	// two SCC2698 octal UARTs = 16 serial lines.  Each chip has four interrupt
	// outputs (one per channel pair), all OR'd into the NS32532 /INT.
	SCC2698B(config, m_duart[0], 3.6864_MHz_XTAL);
	m_duart[0]->intr_A().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_duart[0]->intr_B().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_duart[0]->intr_C().set(m_irqs, FUNC(input_merger_device::in_w<4>));
	m_duart[0]->intr_D().set(m_irqs, FUNC(input_merger_device::in_w<5>));

	SCC2698B(config, m_duart[1], 3.6864_MHz_XTAL);
	m_duart[1]->intr_A().set(m_irqs, FUNC(input_merger_device::in_w<6>));
	m_duart[1]->intr_B().set(m_irqs, FUNC(input_merger_device::in_w<7>));
	m_duart[1]->intr_C().set(m_irqs, FUNC(input_merger_device::in_w<8>));
	m_duart[1]->intr_D().set(m_irqs, FUNC(input_merger_device::in_w<9>));

	for (unsigned i = 0; i < 16; i++)
		RS232_PORT(config, m_serial[i], default_rs232_devices, (i == 0) ? "terminal" : nullptr);

	// console = chip 0, channel A <-> serial0.
	// TODO: wire the remaining 15 channels (chip0 b-h, chip1 a-h) the same way.
	m_duart[0]->tx_callback<'a'>().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_serial[0]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_a_rx_w));
}

ROM_START(et532)
	// The ET532 was never built, so no EPROM was ever programmed.  This is a
	// minimal bring-up monitor (NS32k reset + SCC2698 console, examine/deposit/
	// go); built with the Definicon DSI-32 toolchain; will grow into the
	// console + block-0 SCSI loaders.  Single 27C512 (64KB).
	ROM_REGION32_LE(0x10000, "eprom", 0)
	ROM_LOAD("et532_monitor.bin", 0x0000, 0x10000, CRC(250253af) SHA1(415104ebee9b8acf25d1e0b5ff25b31de81dc54b))
ROM_END

} // anonymous namespace

/*   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY           FULLNAME                       FLAGS */
COMP(1989, et532, 0,      0,      et532,   0,     et532_state, empty_init, "George Scolaro", "ET532 (NS32532 + Ethernet)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
