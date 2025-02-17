// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Winbond IT8705F LPC Super I/O

TODO:
- Move stuff from sis950_lpc;
- shutms11 fails detecting FDC;

**************************************************************************************************/

#include "emu.h"
#include "it8705f.h"

#include "formats/naslite_dsk.h"

#include <algorithm>

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(IT8705F, it8705f_device, "it8705f", "ITE IT8705F LPC Super I/O")

it8705f_device::it8705f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IT8705F, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(it8705f_device::config_map), this))
	, m_pc_fdc(*this, "fdc")
	, m_pc_com(*this, "uart%d", 0U)
	, m_pc_lpt(*this, "lpta")
	, m_logical_view(*this, "logical_view")
	, m_irq1_callback(*this)
	, m_irq8_callback(*this)
	, m_irq9_callback(*this)
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
	, m_index(0)
	, m_logical_index(0)
	, m_lock_sequence_index(0)
{
	std::fill(std::begin(m_activate), std::end(m_activate), false);
}

it8705f_device::~it8705f_device()
{
}

void it8705f_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(0, this, true);
	m_isa->set_dma_channel(1, this, true);
	m_isa->set_dma_channel(2, this, true);
	m_isa->set_dma_channel(3, this, true);
	remap(AS_IO, 0, 0x400);

}

void it8705f_device::device_reset()
{
	m_index = 0;
	m_lock_sequence_index = 0;

	m_pc_fdc_irq_line = 6;
	m_pc_fdc_drq_line = 2;
//  m_pc_fdc_mode = ;
	m_pc_fdc_address = 0x03f0;

	m_pc_lpt_address = 0x0378;
	m_pc_lpt_irq_line = 7;
	m_pc_lpt_drq_line = 4; // disabled
//  m_pc_lpt_mode = 0x3f;

	m_pc_fdc->set_rate(500000);
}

device_memory_interface::space_config_vector it8705f_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void it8705f_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
}

void it8705f_device::device_add_mconfig(machine_config &config)
{
	// 82077 compatible
	N82077AA(config, m_pc_fdc, XTAL(24'000'000), upd765_family_device::mode_t::AT);
	m_pc_fdc->intrq_wr_callback().set(FUNC(it8705f_device::irq_floppy_w));
	m_pc_fdc->drq_wr_callback().set(FUNC(it8705f_device::drq_floppy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", it8705f_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", it8705f_device::floppy_formats);

	NS16550(config, m_pc_com[0], XTAL(24'000'000) / 13);
	m_pc_com[0]->out_int_callback().set(FUNC(it8705f_device::irq_serial1_w));
	m_pc_com[0]->out_tx_callback().set(FUNC(it8705f_device::txd_serial1_w));
	m_pc_com[0]->out_dtr_callback().set(FUNC(it8705f_device::dtr_serial1_w));
	m_pc_com[0]->out_rts_callback().set(FUNC(it8705f_device::rts_serial1_w));

	NS16550(config, m_pc_com[1], XTAL(24'000'000) / 13);
	m_pc_com[1]->out_int_callback().set(FUNC(it8705f_device::irq_serial2_w));
	m_pc_com[1]->out_tx_callback().set(FUNC(it8705f_device::txd_serial2_w));
	m_pc_com[1]->out_dtr_callback().set(FUNC(it8705f_device::dtr_serial2_w));
	m_pc_com[1]->out_rts_callback().set(FUNC(it8705f_device::rts_serial2_w));

	PC_LPT(config, m_pc_lpt);
	m_pc_lpt->irq_handler().set(FUNC(it8705f_device::irq_parallel_w));

}


void it8705f_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// TODO: apparently installs two ports, an alias at 0x4e/0x4f
		m_isa->install_device(0x002e, 0x002f, read8sm_delegate(*this, FUNC(it8705f_device::read)), write8sm_delegate(*this, FUNC(it8705f_device::write)));

		if (m_activate[0])
		{
			m_isa->install_device(m_pc_fdc_address, m_pc_fdc_address + 7, *m_pc_fdc, &n82077aa_device::map);
		}

		for (int i = 0; i < 2; i++)
		{
			if (m_activate[i + 1])
			{
				const u16 uart_addr = m_pc_com_address[i];
				m_isa->install_device(uart_addr, uart_addr + 7, read8sm_delegate(*m_pc_com[i], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_pc_com[i], FUNC(ns16450_device::ins8250_w)));
			}
		}

		// can't map below 0x100
		if (m_activate[3] & 1 && m_pc_lpt_address & 0xf00)
		{
			m_isa->install_device(m_pc_lpt_address, m_pc_lpt_address + 3, read8sm_delegate(*m_pc_lpt, FUNC(pc_lpt_device::read)), write8sm_delegate(*m_pc_lpt, FUNC(pc_lpt_device::write)));
		}
	}
}

uint8_t it8705f_device::read(offs_t offset)
{
	if (m_lock_sequence_index != 4)
		return 0;

	if (offset == 0)
		return m_index;

	return space().read_byte(m_index);
}

void it8705f_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		if (m_lock_sequence_index == 4)
			m_index = data;
		else
		{
			// TODO: 0xaa for lock_seq[3] with 0x4e alias
			const u8 lock_seq[4] = { 0x87, 0x01, 0x55, 0x55 };
			if (data == lock_seq[m_lock_sequence_index])
				m_lock_sequence_index ++;
			else
				m_lock_sequence_index = 0;
		}
	}
	else
	{
		if (m_lock_sequence_index == 4)
			space().write_byte(m_index, data);
	}
}

void it8705f_device::config_map(address_map &map)
{
	map(0x02, 0x02).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(data, 1))
				m_lock_sequence_index = 0;
			// TODO: bit 0 for global reset
		})
	);
	map(0x07, 0x07).lr8(NAME([this] () { return m_logical_index; })).w(FUNC(it8705f_device::logical_device_select_w));
	map(0x20, 0x20).lr8(NAME([] () { return 0x87; })); // device ID
	map(0x21, 0x21).lr8(NAME([] () { return 0x05; })); // revision
//  map(0x22, 0x22) Configuration Select and Chip Version
//  map(0x23, 0x23) Software Suspend
//  map(0x24, 0x24) Clock Selection and Flash ROM I/F control
//  map(0x25, 0x2a) LDN5 GPIO set multi-function pins
//  map(0x2b, 0x2b) LDN4 alternate GPIO set multi-function pins
//  map(0x2e, 0x2f) LDNF4 Test Modes

	map(0x30, 0xff).view(m_logical_view);
	// FDC
	m_logical_view[0](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<0>), FUNC(it8705f_device::activate_w<0>));
	m_logical_view[0](0x60, 0x61).lrw8(
		NAME([this] (offs_t offset) {
			return (m_pc_fdc_address >> (offset * 8)) & 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset * 8;
			m_pc_fdc_address &= 0xff << shift;
			m_pc_fdc_address |= data << (shift ^ 8);
			m_pc_fdc_address &= ~0xf007;
			LOG("LDN0 (FDC): remap %04x ([%d] %02x)\n", m_pc_fdc_address, offset, data);

			remap(AS_IO, 0, 0x400);
		})
	);
	m_logical_view[0](0x70, 0x70).lrw8(
		NAME([this] () {
			return m_pc_fdc_irq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pc_fdc_irq_line = data & 0xf;
			LOG("LDN0 (FDC): irq routed to %02x\n", m_pc_lpt_irq_line);
		})
	);
	m_logical_view[0](0x74, 0x74).lrw8(
		NAME([this] () {
			return m_pc_lpt_drq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pc_fdc_drq_line = data & 0x7;
			LOG("LDN0 (FDC): drq %s (%02x)\n", BIT(m_pc_lpt_drq_line, 2) ? "disabled" : "enabled", data);
		})
	);
	// TODO: m_logical_view[0](0xf0, 0xf1) FDC config

	// UART1
	m_logical_view[1](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<1>), FUNC(it8705f_device::activate_w<1>));
	m_logical_view[1](0x60, 0x61).rw(FUNC(it8705f_device::uart_address_r<0>), FUNC(it8705f_device::uart_address_w<0>));
	m_logical_view[1](0x70, 0x70).rw(FUNC(it8705f_device::uart_irq_r<0>), FUNC(it8705f_device::uart_irq_w<0>));
	m_logical_view[1](0xf0, 0xf0).rw(FUNC(it8705f_device::uart_config_r<0>), FUNC(it8705f_device::uart_config_w<0>));

	// UART2
	m_logical_view[2](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<2>), FUNC(it8705f_device::activate_w<2>));
	m_logical_view[2](0x60, 0x61).rw(FUNC(it8705f_device::uart_address_r<1>), FUNC(it8705f_device::uart_address_w<1>));
	m_logical_view[2](0x70, 0x70).rw(FUNC(it8705f_device::uart_irq_r<1>), FUNC(it8705f_device::uart_irq_w<1>));
	m_logical_view[2](0xf0, 0xf0).rw(FUNC(it8705f_device::uart_config_r<1>), FUNC(it8705f_device::uart_config_w<1>));

	// LPT
	m_logical_view[3](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<3>), FUNC(it8705f_device::activate_w<3>));
	m_logical_view[3](0x60, 0x61).lrw8(
		NAME([this] (offs_t offset) {
			return (m_pc_lpt_address >> (offset * 8)) & 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset * 8;
			m_pc_lpt_address &= 0xff << shift;
			m_pc_lpt_address |= data << (shift ^ 8);
			m_pc_lpt_address &= ~0xf003;
			LOG("LDN3 (LPT): remap %04x ([%d] %02x)\n", m_pc_lpt_address, offset, data);

			remap(AS_IO, 0, 0x400);
		})
	);
	//m_logical_view[3](0x62, 0x63) secondary base address
	//m_logical_view[3](0x64, 0x65) POST data port base address
	m_logical_view[3](0x70, 0x70).lrw8(
		NAME([this] () {
			return m_pc_lpt_irq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pc_lpt_irq_line = data & 0xf;
			LOG("LDN3 (LPT): irq routed to %02x\n", m_pc_lpt_irq_line);
		})
	);
	m_logical_view[3](0x74, 0x74).lrw8(
		NAME([this] () {
			return m_pc_lpt_drq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pc_lpt_drq_line = data & 0x7;
			LOG("LDN3 (LPT): drq %s (%02x)\n", BIT(m_pc_lpt_drq_line, 2) ? "disabled" : "enabled", data);
		})
	);

	// Environment controller / HW monitor
	m_logical_view[4](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<4>), FUNC(it8705f_device::activate_w<4>));
	m_logical_view[4](0x31, 0xff).unmaprw();

	// GPIO
	m_logical_view[5](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<5>), FUNC(it8705f_device::activate_w<5>));
	m_logical_view[5](0x31, 0xff).unmaprw();

	// Game port
	m_logical_view[6](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<6>), FUNC(it8705f_device::activate_w<6>));
	m_logical_view[6](0x31, 0xff).unmaprw();

	// Consumer IR
	m_logical_view[7](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<7>), FUNC(it8705f_device::activate_w<7>));
	m_logical_view[7](0x31, 0xff).unmaprw();

	// MIDI port
	m_logical_view[8](0x30, 0x30).rw(FUNC(it8705f_device::activate_r<8>), FUNC(it8705f_device::activate_w<8>));
	m_logical_view[8](0x31, 0xff).unmaprw();
}

/*
 * Global register space
 */

void it8705f_device::logical_device_select_w(offs_t offset, u8 data)
{
	m_logical_index = data;
	if (m_logical_index <= 0x8)
		m_logical_view.select(m_logical_index);
	else
		LOG("Attempt to select an unmapped device with %02x\n", data);
}

template <unsigned N> u8 it8705f_device::activate_r(offs_t offset)
{
	return m_activate[N];
}

template <unsigned N> void it8705f_device::activate_w(offs_t offset, u8 data)
{
	m_activate[N] = data & 1;
	LOG("LDN%d Device %s\n", N, data & 1 ? "enabled" : "disabled");
	remap(AS_IO, 0, 0x400);
}

void it8705f_device::request_irq(int irq, int state)
{
	switch (irq)
	{
	case 1:
		m_irq1_callback(state);
		break;
	case 3:
		m_isa->irq3_w(state);
		break;
	case 4:
		m_isa->irq4_w(state);
		break;
	case 5:
		m_isa->irq5_w(state);
		break;
	case 6:
		m_isa->irq6_w(state);
		break;
	case 7:
		m_isa->irq7_w(state);
		break;
	case 8:
		m_irq8_callback(state);
		break;
	case 9:
		m_irq9_callback(state);
		break;
	case 10:
		m_isa->irq10_w(state);
		break;
	case 11:
		m_isa->irq11_w(state);
		break;
	case 12:
		m_isa->irq12_w(state);
		break;
	case 14:
		m_isa->irq14_w(state);
		break;
	case 15:
		m_isa->irq15_w(state);
		break;
	}
}

void it8705f_device::request_dma(int dreq, int state)
{
	switch (dreq)
	{
	case 0:
		m_isa->drq0_w(state);
		break;
	case 1:
		m_isa->drq1_w(state);
		break;
	case 2:
		m_isa->drq2_w(state);
		break;
	case 3:
		m_isa->drq3_w(state);
		break;
	}
}

/*
 * Device #0 (FDC)
 */

void it8705f_device::irq_floppy_w(int state)
{
	if (!m_activate[0])
		return;
	request_irq(m_pc_fdc_irq_line, state ? ASSERT_LINE : CLEAR_LINE);
}

void it8705f_device::drq_floppy_w(int state)
{
	if (!m_activate[0])
		return;
	request_dma(m_pc_fdc_drq_line, state ? ASSERT_LINE : CLEAR_LINE);
}

/*
 * Device #1/#2 (UART)
 */

template <unsigned N> u8 it8705f_device::uart_address_r(offs_t offset)
{
	return (m_pc_com_address[N] >> (offset * 8)) & 0xff;
}

template <unsigned N> void it8705f_device::uart_address_w(offs_t offset, u8 data)
{
	const u8 shift = offset * 8;
	m_pc_com_address[N] &= 0xff << shift;
	m_pc_com_address[N] |= data << (shift ^ 8);
	m_pc_com_address[N] &= ~0xf007;
	LOG("LDN%d (COM%d): remap %04x ([%d] %02x)\n", N, N + 1, m_pc_com_address[N], offset, data);

	remap(AS_IO, 0, 0x400);
}

template <unsigned N> u8 it8705f_device::uart_irq_r(offs_t offset)
{
	return m_pc_com_irq_line[N];
}

template <unsigned N> void it8705f_device::uart_irq_w(offs_t offset, u8 data)
{
	m_pc_com_irq_line[N] = data & 0xf;
	LOG("LDN%d (UART): irq routed to %02x\n", N, m_pc_com_irq_line[N]);
}

template <unsigned N> u8 it8705f_device::uart_config_r(offs_t offset)
{
	return m_pc_com_control[N];
}

/*
 * ---- -xx- Clock Source
 * ---- -00- 24 MHz/13
 * ---- -??- <reserved>
 * ---- ---x IRQ sharing enable
 */
template <unsigned N> void it8705f_device::uart_config_w(offs_t offset, u8 data)
{
	m_pc_com_control[N] = data;
	LOG("LDN%d (UART): control %02x\n", N, m_pc_com_control[N]);
}

void it8705f_device::irq_serial1_w(int state)
{
	if (!m_activate[1])
		return;
	request_irq(m_pc_com_irq_line[0], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8705f_device::irq_serial2_w(int state)
{
	if (!m_activate[2])
		return;
	request_irq(m_pc_com_irq_line[1], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8705f_device::txd_serial1_w(int state)
{
	if (!m_activate[1])
		return;
	m_txd1_callback(state);
}

void it8705f_device::txd_serial2_w(int state)
{
	if (!m_activate[2])
		return;
	m_txd2_callback(state);
}

void it8705f_device::dtr_serial1_w(int state)
{
	if (!m_activate[1])
		return;
	m_ndtr1_callback(state);
}

void it8705f_device::dtr_serial2_w(int state)
{
	if (!m_activate[2])
		return;
	m_ndtr2_callback(state);
}

void it8705f_device::rts_serial1_w(int state)
{
	if (!m_activate[1])
		return;
	m_nrts1_callback(state);
}

void it8705f_device::rts_serial2_w(int state)
{
	if (!m_activate[2])
		return;
	m_nrts2_callback(state);
}

void it8705f_device::rxd1_w(int state)
{
	m_pc_com[0]->rx_w(state);
}

void it8705f_device::ndcd1_w(int state)
{
	m_pc_com[0]->dcd_w(state);
}

void it8705f_device::ndsr1_w(int state)
{
	m_pc_com[0]->dsr_w(state);
}

void it8705f_device::nri1_w(int state)
{
	m_pc_com[0]->ri_w(state);
}

void it8705f_device::ncts1_w(int state)
{
	m_pc_com[0]->cts_w(state);
}

void it8705f_device::rxd2_w(int state)
{
	m_pc_com[1]->rx_w(state);
}

void it8705f_device::ndcd2_w(int state)
{
	m_pc_com[1]->dcd_w(state);
}

void it8705f_device::ndsr2_w(int state)
{
	m_pc_com[1]->dsr_w(state);
}

void it8705f_device::nri2_w(int state)
{
	m_pc_com[1]->ri_w(state);
}

void it8705f_device::ncts2_w(int state)
{
	m_pc_com[1]->cts_w(state);
}

/*
 * Device #3 (Parallel)
 */

void it8705f_device::irq_parallel_w(int state)
{
	if (m_activate[3] == false)
		return;
	request_irq(m_pc_lpt_irq_line, state ? ASSERT_LINE : CLEAR_LINE);
}
