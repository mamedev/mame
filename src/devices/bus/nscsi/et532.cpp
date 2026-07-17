// license:BSD-3-Clause
// copyright-holders:Dave Rand

/*
 * ET532 board core + SCSI slot card.
 *
 * et532_board_device is the whole ET532 machine (NS32532 cluster, DRAM, EPROM,
 * COPS MAC EEPROM, two SCC2698 octal UARTs = 16 serial lines, DP8390 Ethernet,
 * DP8490 SCSI with CPU-driven pseudo-DMA, and the non-vectored /INT merger).  It
 * is shared by the stand-alone `et532' system driver (homebrew/et532.cpp) and by
 * et532_scsi_device below.  The board's DP8490 (tag "dp8490") is left for the
 * owner to place on a bus:
 *   - stand-alone: a private nscsi bus with disk/tape, board = initiator (J4);
 *   - slot card:   a host nscsi bus, board = target (the in-system 532SC role).
 * Which role the DP8490 plays is a firmware choice (MODE_TARGET), not wiring.
 *
 * et532_scsi_device wraps the board as an nscsi slot card so it can drop into an
 * NSCSI_CONNECTOR on a host bus (the PC532 expansion SCSI bus).  The on-board
 * NS32532 runs the SFP (Serial/SCSI Frame Protocol) firmware that answers the host.
 *
 * See docs/et532/ET532-SLAVE-LOG.txt for the slave-role design.
 */

#include "emu.h"
#include "et532.h"

DEFINE_DEVICE_TYPE(ET532_BOARD, et532_board_device, "et532_board", "ET532 board")
DEFINE_DEVICE_TYPE(ET532_SCSI,  et532_scsi_device,  "et532_scsi",  "ET532 SCSI coprocessor card")

namespace {

ROM_START(et532_board)
	ROM_REGION32_LE(0x10000, "eprom", 0)
	ROM_LOAD("et532_monitor.bin", 0x0000, 0x10000, CRC(9c024fab) SHA1(c8f3651768efe8282ceb914a99c2355735be8976))
ROM_END

} // anonymous namespace


//**************************************************************************
//  ET532 BOARD CORE
//**************************************************************************

ALLOW_SAVE_TYPE(et532_board_device::dma_state)

et532_board_device::et532_board_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ET532_BOARD, tag, owner, clock)
	, m_cpu(*this, "cpu")
	, m_fpu(*this, "fpu")
	, m_scsi(*this, "dp8490")
	, m_net(*this, "dp8390")
	, m_eeprom(*this, "eeprom")
	, m_duart(*this, "duart%u", 0U)
	, m_serial(*this, "serial%u", 0U)
	, m_irqs(*this, "irqs")
	, m_net_ram{}
	, m_drq(false)
	, m_irq(false)
	, m_dma(0)
	, m_state(IDLE)
{
}

void et532_board_device::device_start()
{
	save_item(NAME(m_net_ram));
	save_item(NAME(m_drq));
	save_item(NAME(m_irq));
	save_item(NAME(m_dma));
	save_item(NAME(m_state));
}

void et532_board_device::device_reset()
{
	m_drq = false;
	m_irq = false;
	m_dma = 0;
	m_state = IDLE;
}

const tiny_rom_entry *et532_board_device::device_rom_region() const
{
	return ROM_NAME(et532_board);
}

// DP8390 local packet RAM (the 6264 SRAM)
u8 et532_board_device::net_ram_r(offs_t offset)
{
	return m_net_ram[offset & 0x1fff];
}

void et532_board_device::net_ram_w(offs_t offset, u8 data)
{
	m_net_ram[offset & 0x1fff] = data;
}

// COPS: 93C46 serial EEPROM holding the Ethernet MAC (COPETH GAL).  write bit0=CS,
// bit1=SK, bit2=DI; read bit0=DO.
u8 et532_board_device::cops_r(offs_t offset)
{
	return m_eeprom->do_read();
}

void et532_board_device::cops_w(offs_t offset, u8 data)
{
	m_eeprom->cs_write(BIT(data, 0));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->di_write(BIT(data, 2));
}

// CPU-driven pseudo-DMA for the SCSI controller (NS32532 dynamic bus sizing + cycle
// extension), exactly as on the pc532.
void et532_board_device::drq_w(int state)
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

void et532_board_device::irq_w(int state)
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

u32 et532_board_device::dma_r(offs_t offset, u32 mem_mask)
{
	u32 data = 0;

	switch (m_state)
	{
	case IDLE:
		if (!machine().side_effects_disabled())
		{
			if (m_drq && !m_irq)
			{
				m_dma = m_scsi->dma_r();
				m_state = RD1;

				m_cpu->rdy_w(1);
			}
		}
		break;

	case RD1:
	case RD2:
	case RD3:
		if (!machine().side_effects_disabled())
			m_cpu->rdy_w(1);
		break;

	case RD4:
		data = m_dma;
		if (!machine().side_effects_disabled())
			m_state = IDLE;
		break;

	default:
		break;
	}

	return data;
}

void et532_board_device::dma_w(offs_t offset, u32 data, u32 mem_mask)
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

template <unsigned ST> void et532_board_device::cpu_map(address_map &map)
{
	map(0x0000'0000, 0x0000'ffff).rom().region("eprom", 0);
	map(0x0400'0000, 0x047f'ffff).ram().share("ram"); // 8MB DRAM

	map(0x0800'0000, 0x0800'0000).rw(FUNC(et532_board_device::cops_r), FUNC(et532_board_device::cops_w));

	map(0x1000'0000, 0x1000'003f).m(m_duart[0], FUNC(scc2698b_device::map)); // OCT0: serial 0-7
	map(0x1040'0000, 0x1040'003f).m(m_duart[1], FUNC(scc2698b_device::map)); // OCT1: serial 8-15
	map(0x1100'0000, 0x1100'0007).m(m_scsi, FUNC(dp8490_device::map));       // SCSI registers
	map(0x1180'0000, 0x1180'000f).rw(m_net, FUNC(dp8390d_device::cs_read), FUNC(dp8390d_device::cs_write));
	map(0x11c0'0000, 0x11c0'0000).lrw8(
			NAME([this](offs_t offset) { return u8(m_net->remote_read()); }),
			NAME([this](offs_t offset, u8 data) { m_net->remote_write(data); }));

	map(0x2000'0000, 0x27ff'ffff).rw(FUNC(et532_board_device::dma_r), FUNC(et532_board_device::dma_w));
}

void et532_board_device::device_add_mconfig(machine_config &config)
{
	NS32532(config, m_cpu, 40_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &et532_board_device::cpu_map<0>);
	m_cpu->set_addrmap(ns32000::ST_IAM, &et532_board_device::cpu_map<ns32000::ST_IAM>);
	m_cpu->set_addrmap(ns32000::ST_ODT, &et532_board_device::cpu_map<ns32000::ST_ODT>);

	NS32381(config, m_fpu, 40_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	// no NS32202 ICU: OR all maskable interrupt sources to the NS32532 /INT
	INPUT_MERGER_ANY_HIGH(config, m_irqs);
	m_irqs->output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	// DP8490 SCSI; the owner attaches it to a bus (private = initiator, host = target)
	DP8490(config, m_scsi);
	m_scsi->drq_handler().set(FUNC(et532_board_device::drq_w));
	m_scsi->irq_handler().append(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_scsi->irq_handler().append(FUNC(et532_board_device::irq_w));

	// Ethernet
	DP8390D(config, m_net);
	m_net->irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_net->mem_read_callback().set(FUNC(et532_board_device::net_ram_r));
	m_net->mem_write_callback().set(FUNC(et532_board_device::net_ram_w));

	EEPROM_93C46_16BIT(config, m_eeprom);

	// two SCC2698 octal UARTs = 16 serial lines
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

	m_duart[0]->tx_callback<'a'>().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_serial[0]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_a_rx_w));
	m_duart[0]->tx_callback<'b'>().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_serial[1]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_b_rx_w));
	m_duart[0]->tx_callback<'c'>().set(m_serial[2], FUNC(rs232_port_device::write_txd));
	m_serial[2]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_c_rx_w));
	m_duart[0]->tx_callback<'d'>().set(m_serial[3], FUNC(rs232_port_device::write_txd));
	m_serial[3]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_d_rx_w));
	m_duart[0]->tx_callback<'e'>().set(m_serial[4], FUNC(rs232_port_device::write_txd));
	m_serial[4]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_e_rx_w));
	m_duart[0]->tx_callback<'f'>().set(m_serial[5], FUNC(rs232_port_device::write_txd));
	m_serial[5]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_f_rx_w));
	m_duart[0]->tx_callback<'g'>().set(m_serial[6], FUNC(rs232_port_device::write_txd));
	m_serial[6]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_g_rx_w));
	m_duart[0]->tx_callback<'h'>().set(m_serial[7], FUNC(rs232_port_device::write_txd));
	m_serial[7]->rxd_handler().set(m_duart[0], FUNC(scc2698b_device::port_h_rx_w));

	m_duart[1]->tx_callback<'a'>().set(m_serial[8], FUNC(rs232_port_device::write_txd));
	m_serial[8]->rxd_handler().set(m_duart[1], FUNC(scc2698b_device::port_a_rx_w));
	m_duart[1]->tx_callback<'b'>().set(m_serial[9], FUNC(rs232_port_device::write_txd));
	m_serial[9]->rxd_handler().set(m_duart[1], FUNC(scc2698b_device::port_b_rx_w));
	m_duart[1]->tx_callback<'c'>().set(m_serial[10], FUNC(rs232_port_device::write_txd));
	m_serial[10]->rxd_handler().set(m_duart[1], FUNC(scc2698b_device::port_c_rx_w));
	m_duart[1]->tx_callback<'d'>().set(m_serial[11], FUNC(rs232_port_device::write_txd));
	m_serial[11]->rxd_handler().set(m_duart[1], FUNC(scc2698b_device::port_d_rx_w));
	m_duart[1]->tx_callback<'e'>().set(m_serial[12], FUNC(rs232_port_device::write_txd));
	m_serial[12]->rxd_handler().set(m_duart[1], FUNC(scc2698b_device::port_e_rx_w));
	m_duart[1]->tx_callback<'f'>().set(m_serial[13], FUNC(rs232_port_device::write_txd));
	m_serial[13]->rxd_handler().set(m_duart[1], FUNC(scc2698b_device::port_f_rx_w));
	m_duart[1]->tx_callback<'g'>().set(m_serial[14], FUNC(rs232_port_device::write_txd));
	m_serial[14]->rxd_handler().set(m_duart[1], FUNC(scc2698b_device::port_g_rx_w));
	m_duart[1]->tx_callback<'h'>().set(m_serial[15], FUNC(rs232_port_device::write_txd));
	m_serial[15]->rxd_handler().set(m_duart[1], FUNC(scc2698b_device::port_h_rx_w));
}


//**************************************************************************
//  ET532 SCSI SLOT CARD
//**************************************************************************

et532_scsi_device::et532_scsi_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ET532_SCSI, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "board:dp8490")
	, m_board(*this, "board")
{
}

void et532_scsi_device::device_start()
{
}

void et532_scsi_device::device_add_mconfig(machine_config &config)
{
	ET532_BOARD(config, m_board, 40_MHz_XTAL);
}
