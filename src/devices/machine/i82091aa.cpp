// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "i82091aa.h"

#include "formats/naslite_dsk.h"

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(I82091AA, i82091aa_device, "i82091aa", "Intel 82091AA Advanced Integrated Peripheral")

i82091aa_device::i82091aa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, I82091AA, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(i82091aa_device::config_map), this))
	, m_fdc(*this, "fdc")
	, m_com(*this, "uart%d", 0U)
	, m_lpt(*this, "lpt")
	, m_ide(*this, finder_base::DUMMY_TAG)
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
	, m_config_address(0x22)
	, m_aipcfg1(0)
	, m_aipcfg2(0)
	, m_fdc_irq(false)
	, m_fdc_drq(false)
	, m_serial_irq{ false, false }
	, m_parallel_irq(false)
	, m_irq_out(0)
	, m_drq_out(false)
	, m_index(0)
	, m_fcfg1(0)
	, m_fcfg2(0)
	, m_pcfg1(0)
	, m_pcfg2(0)
	, m_sacfg1(0)
	, m_sacfg2(0)
	, m_sbcfg1(0)
	, m_sbcfg2(0)
	, m_idecfg(0)
{
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("35ed", FLOPPY_35_ED);
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void i82091aa_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
}

void i82091aa_device::device_add_mconfig(machine_config &config)
{
	// TODO: actually 82078
	N82077AA(config, m_fdc, XTAL(24'000'000), upd765_family_device::mode_t::AT);
	m_fdc->intrq_wr_callback().set(FUNC(i82091aa_device::irq_floppy_w));
	m_fdc->drq_wr_callback().set(FUNC(i82091aa_device::drq_floppy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", i82091aa_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", i82091aa_device::floppy_formats);

	NS16550(config, m_com[0], XTAL(24'000'000) / 13);
	m_com[0]->out_int_callback().set(FUNC(i82091aa_device::irq_serial1_w));
	m_com[0]->out_tx_callback().set(FUNC(i82091aa_device::txd_serial1_w));
	m_com[0]->out_dtr_callback().set(FUNC(i82091aa_device::dtr_serial1_w));
	m_com[0]->out_rts_callback().set(FUNC(i82091aa_device::rts_serial1_w));

	NS16550(config, m_com[1], XTAL(24'000'000) / 13);
	m_com[1]->out_int_callback().set(FUNC(i82091aa_device::irq_serial2_w));
	m_com[1]->out_tx_callback().set(FUNC(i82091aa_device::txd_serial2_w));
	m_com[1]->out_dtr_callback().set(FUNC(i82091aa_device::dtr_serial2_w));
	m_com[1]->out_rts_callback().set(FUNC(i82091aa_device::rts_serial2_w));

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(i82091aa_device::irq_parallel_w));
}

i82091aa_device::~i82091aa_device()
{
}

void i82091aa_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(2, this, true);

	save_item(NAME(m_index));
	save_item(NAME(m_aipcfg1));
	save_item(NAME(m_aipcfg2));
	save_item(NAME(m_fcfg1));
	save_item(NAME(m_fcfg2));
	save_item(NAME(m_pcfg1));
	save_item(NAME(m_pcfg2));
	save_item(NAME(m_sacfg1));
	save_item(NAME(m_sacfg2));
	save_item(NAME(m_sbcfg1));
	save_item(NAME(m_sbcfg2));
	save_item(NAME(m_idecfg));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_serial_irq));
	save_item(NAME(m_parallel_irq));
	save_item(NAME(m_irq_out));
	save_item(NAME(m_drq_out));
}

void i82091aa_device::device_reset()
{
	// Software motherboard/add-in defaults; hardware basic/extended straps are not implemented.
	m_index = 0;
	m_aipcfg1 = (m_config_address >= 0x100 ? 0x10 : 0) | ((m_config_address == 0x24 || m_config_address == 0x398) ? 8 : 0);
	m_aipcfg2 = 0;
	m_fcfg2 = m_pcfg2 = m_sacfg2 = m_sbcfg2 = 0;
	m_fcfg1 = 1;
	m_pcfg1 = 0;
	m_sacfg1 = 0x10;
	m_sbcfg1 = 0;
	m_idecfg = 1;

	m_fdc->set_mode(upd765_family_device::mode_t::AT);
	m_fdc->set_rate(500000);

	remap(AS_IO, 0, 0xffff);
	update_irq();
	update_drq();
}

void i82091aa_device::device_post_load()
{
	remap(AS_IO, 0, 0xffff);
}

device_memory_interface::space_config_vector i82091aa_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void i82091aa_device::unmap_io_map(address_map &map)
{
	// Byte handlers preserve neighboring devices on both 16- and 32-bit host spaces.
	for (const auto &[first, last] : m_io_ranges)
		map(first, last).lrw8(NAME([] () { return 0xff; }), NAME([] (u8 data) { }));
}

void i82091aa_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// Remove the preceding decode, including when restoring a different saved configuration.
		m_isa->install_device(0, 0xffff, *this, &i82091aa_device::unmap_io_map);
		m_io_ranges.clear();

		const u16 superio_base = m_config_address;
		m_io_ranges.emplace_back(superio_base, superio_base + 1);
		m_isa->install_device(superio_base, superio_base + 1, read8sm_delegate(*this, FUNC(i82091aa_device::read)), write8sm_delegate(*this, FUNC(i82091aa_device::write)));

		if (BIT(m_fcfg1, 0))
		{
			const u16 fdc_address = BIT(m_fcfg1, 1) ? 0x0370 : 0x03f0;
			m_io_ranges.emplace_back(fdc_address, fdc_address + 5);
			m_io_ranges.emplace_back(fdc_address + 7, fdc_address + 7);
			m_isa->install_device(fdc_address, fdc_address + 7, *m_fdc, &n82077aa_device::map);
		}

		if (m_ide && BIT(m_idecfg, 0))
		{
			for (u32 secondary = 0; secondary < 2; secondary++)
			{
				if (!BIT(m_idecfg, 2) && secondary != BIT(m_idecfg, 1))
					continue;
				const u16 base = secondary ? 0x170 : 0x1f0;
				m_io_ranges.emplace_back(base, base + 7);
				m_io_ranges.emplace_back(base + 0x206, base + 0x207);
				m_isa->install16_device(base, base + 7,
					read16s_delegate(*m_ide, FUNC(ide_controller_device::cs0_r)),
					write16s_delegate(*m_ide, FUNC(ide_controller_device::cs0_w)));
				m_isa->install_device(base + 0x206, base + 0x207,
					secondary ? read8sm_delegate(*this, FUNC(i82091aa_device::ide_alt_r<true>)) : read8sm_delegate(*this, FUNC(i82091aa_device::ide_alt_r<false>)),
					secondary ? write8sm_delegate(*this, FUNC(i82091aa_device::ide_alt_w<true>)) : write8sm_delegate(*this, FUNC(i82091aa_device::ide_alt_w<false>)));
			}
		}

		const u16 com_address_table[8] = { 0x3f8, 0x2f8, 0x220, 0x228, 0x238, 0x2e8, 0x338, 0x3e8 };
		if (BIT(m_sacfg1, 0))
		{
			const u16 uart_addr = com_address_table[(m_sacfg1 >> 1) & 7];
			m_io_ranges.emplace_back(uart_addr, uart_addr + 7);
			m_isa->install_device(uart_addr, uart_addr + 7, read8sm_delegate(*m_com[0], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_com[0], FUNC(ns16450_device::ins8250_w)));
		}
		if (BIT(m_sbcfg1, 0))
		{
			const u16 uart_addr = com_address_table[(m_sbcfg1 >> 1) & 7];
			m_io_ranges.emplace_back(uart_addr, uart_addr + 7);
			m_isa->install_device(uart_addr, uart_addr + 7, read8sm_delegate(*m_com[1], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_com[1], FUNC(ns16450_device::ins8250_w)));
		}


		// setting 3 is <reserved>
		const u8 lpt_address_select = (m_pcfg1 >> 1) & 3;
		if (BIT(m_pcfg1, 0) && lpt_address_select != 3)
		{
			const u16 lpt_address_table[3] = { 0x378, 0x278, 0x3bc };
			const u16 lpt_address = lpt_address_table[lpt_address_select];
			m_io_ranges.emplace_back(lpt_address, lpt_address + 2);
			m_isa->install_device(lpt_address, lpt_address + 3, *m_lpt, &pc_lpt_device::isa_map);
		}
	}
}

template <bool Secondary> u8 i82091aa_device::ide_alt_r(offs_t offset)
{
	// The IDE drive-address register shares its top bit with the floppy change line.
	const u8 data = m_ide->cs1_r(3, offset ? 0xff00 : 0x00ff) >> (offset * 8);
	return offset && BIT(m_fcfg1, 0) && BIT(m_fcfg1, 1) == Secondary ? (data & 0x7f) | (m_fdc->dir_r() & 0x80) : data;
}

template <bool Secondary> void i82091aa_device::ide_alt_w(offs_t offset, u8 data)
{
	m_ide->cs1_w(3, u16(data) << (offset * 8), offset ? 0xff00 : 0x00ff);
	if (offset && BIT(m_fcfg1, 0) && BIT(m_fcfg1, 1) == Secondary)
		m_fdc->ccr_w(data);
}

u8 i82091aa_device::read(offs_t offset)
{
	return (offset == 0) ? m_index : space().read_byte(m_index);
}

void i82091aa_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_index = data;
	}
	else
	{
		space().write_byte(m_index, data);
	}
}

void i82091aa_device::config_map(address_map &map)
{
	map(0x00, 0x00).lr8(
		NAME([this] () {
			if (!machine().side_effects_disabled())
				LOG("00h: AIPID Product Identification\n");
			return 0xa0;
		})
	);
	map(0x01, 0x01).lr8(
		NAME([this] () {
			if (!machine().side_effects_disabled())
				LOG("01h: AIPREV Revision Identification\n");
			return 0x00;
		})
	);
	map(0x02, 0x02).lrw8(
		NAME([this] () { return m_aipcfg1; }),
		NAME([this] (u8 data) { m_aipcfg1 = (m_aipcfg1 & 0x48) | (data & 0x31); })
	);
	// ISA callbacks carry asserted/deasserted state; electrical polarity is not modeled.
	map(0x03, 0x03).lrw8(
		NAME([this] () { return m_aipcfg2; }),
		NAME([this] (u8 data) { m_aipcfg2 = data & 0xf8; })
	);
	map(0x10, 0x10).lrw8(
		NAME([this] (offs_t offset) {
			return m_fcfg1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("10h: FCFG1 FDC Configuration %02x\n", data);
			m_fcfg1 = data & 0x83;
			remap(AS_IO, 0, 0xffff);
			update_irq();
			update_drq();
		})
	);
	map(0x11, 0x11).lrw8(
		NAME([this] (offs_t offset) {
			// TODO: bit 1 is FDC idle status (r/o)
			return m_fcfg2 | 2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("11h: FCFG2 FDC Power Management and Status %02x\n", data);
			if (BIT(data, 2) && !BIT(m_fcfg2, 2))
				m_fdc->reset();
			m_fcfg2 = data & 0xd;
		})
	);
	map(0x20, 0x20).lrw8(
		NAME([this] (offs_t offset) {
			return m_pcfg1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("20h: PCFG1 Parallel Port Configuration %02x\n", data);
			m_pcfg1 = data & 0xef;
			remap(AS_IO, 0, 0xffff);
			update_irq();
			update_drq();
		})
	);
	map(0x21, 0x21).lrw8(
		NAME([this] (offs_t offset) {
			return m_pcfg2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("21h: PCFG2 Parallel Port Power Management and Status %02x\n", data);
			if (BIT(data, 2) && !BIT(m_pcfg2, 2))
				m_lpt->reset();
			m_pcfg2 = data & 0x2f;
		})
	);
	map(0x30, 0x30).lrw8(
		NAME([this] (offs_t offset) {
			return m_sacfg1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("30h: SACFG1 Serial Port A Configuration %02x\n", data);
			m_sacfg1 = data & 0x9f;
			remap(AS_IO, 0, 0xffff);
			update_irq();
			update_drq();
		})
	);
	map(0x31, 0x31).lrw8(
		NAME([this] (offs_t offset) {
			return m_sacfg2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("31h: SACFG2 Serial Port A Power Management and Status %02x\n", data);
			if (BIT(data, 2) && !BIT(m_sacfg2, 2))
				m_com[0]->reset();
			m_sacfg2 = data & 0x1f;
		})
	);
	map(0x40, 0x40).lrw8(
		NAME([this] (offs_t offset) {
			return m_sbcfg1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("40h: SBCFG1 Serial Port B Configuration %02x\n", data);
			m_sbcfg1 = data & 0x9f;
			remap(AS_IO, 0, 0xffff);
			update_irq();
			update_drq();
		})
	);
	map(0x41, 0x41).lrw8(
		NAME([this] (offs_t offset) {
			return m_sbcfg2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("41h: SBCFG2 Serial Port B Power Management and Status %02x\n", data);
			if (BIT(data, 2) && !BIT(m_sbcfg2, 2))
				m_com[1]->reset();
			m_sbcfg2 = data & 0x1f;
		})
	);
	map(0x50, 0x50).lrw8(
		NAME([this] (offs_t offset) {
			return m_idecfg;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("50h: IDECFG IDE Configuration %02x\n", data);
			m_idecfg = data & 7;
			remap(AS_IO, 0, 0xffff);
			update_irq();
			update_drq();
		})
	);
}

void i82091aa_device::request_irq(int irq, int state)
{
	switch (irq)
	{
	// serial port at $2f8 or $2e8
	case 3:
		m_isa->irq3_w(state);
		break;
	// serial port at $3f8 or $3e8
	case 4:
		m_isa->irq4_w(state);
		break;
	// parallel port at $278
	case 5:
		m_isa->irq5_w(state);
		break;
	// FDC
	case 6:
		m_isa->irq6_w(state);
		break;
	// parallel port at $3bc or $378
	case 7:
		m_isa->irq7_w(state);
		break;
	}
}

void i82091aa_device::update_irq()
{
	// unrouted outputs are tri-stated, so only drive the lines that change
	const bool serial_a = BIT(m_sacfg1, 0) && m_serial_irq[0];
	const bool serial_b = BIT(m_sbcfg1, 0) && m_serial_irq[1];
	u8 irq = 0;
	if ((serial_a && !BIT(m_sacfg1, 4)) || (serial_b && !BIT(m_sbcfg1, 4)))
		irq |= 1 << 3;
	if ((serial_a && BIT(m_sacfg1, 4)) || (serial_b && BIT(m_sbcfg1, 4)))
		irq |= 1 << 4;
	if (BIT(m_pcfg1, 0) && !BIT(m_pcfg1, 3) && m_parallel_irq)
		irq |= 1 << 5;
	if (BIT(m_fcfg1, 0) && m_fdc_irq)
		irq |= 1 << 6;
	if (BIT(m_pcfg1, 0) && BIT(m_pcfg1, 3) && m_parallel_irq)
		irq |= 1 << 7;

	const u8 changed = irq ^ m_irq_out;
	m_irq_out = irq;
	for (int line = 3; line <= 7; line++)
	{
		if (BIT(changed, line))
			request_irq(line, BIT(irq, line));
	}
}

void i82091aa_device::update_drq()
{
	const bool drq = BIT(m_fcfg1, 0) && m_fdc_drq;
	if (drq != m_drq_out)
	{
		m_drq_out = drq;
		m_isa->drq2_w(drq);
	}
}

u8 i82091aa_device::dack_r(int line)
{
	return line == 2 && BIT(m_fcfg1, 0) ? m_fdc->dma_r() : 0xff;
}

void i82091aa_device::dack_w(int line, u8 data)
{
	if (line == 2 && BIT(m_fcfg1, 0))
		m_fdc->dma_w(data);
}

void i82091aa_device::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}

/*
 * FDC
 */

void i82091aa_device::irq_floppy_w(int state)
{
	m_fdc_irq = state;
	update_irq();
}

void i82091aa_device::drq_floppy_w(int state)
{
	m_fdc_drq = state;
	update_drq();
}

/*
 * COM1/2 Serial ports
 */

void i82091aa_device::irq_serial1_w(int state)
{
	m_serial_irq[0] = state;
	update_irq();
}

void i82091aa_device::irq_serial2_w(int state)
{
	m_serial_irq[1] = state;
	update_irq();
}

void i82091aa_device::txd_serial1_w(int state)
{
	if (!BIT(m_sacfg1, 0))
		return;
	m_txd1_callback(state);
}

void i82091aa_device::txd_serial2_w(int state)
{
	if (!BIT(m_sbcfg1, 0))
		return;
	m_txd2_callback(state);
}

void i82091aa_device::dtr_serial1_w(int state)
{
	if (!BIT(m_sacfg1, 0))
		return;
	m_ndtr1_callback(state);
}

void i82091aa_device::dtr_serial2_w(int state)
{
	if (!BIT(m_sbcfg1, 0))
		return;
	m_ndtr2_callback(state);
}

void i82091aa_device::rts_serial1_w(int state)
{
	if (!BIT(m_sacfg1, 0))
		return;
	m_nrts1_callback(state);
}

void i82091aa_device::rts_serial2_w(int state)
{
	if (!BIT(m_sbcfg1, 0))
		return;
	m_nrts2_callback(state);
}

void i82091aa_device::rxd1_w(int state)
{
	m_com[0]->rx_w(state);
}

void i82091aa_device::ndcd1_w(int state)
{
	m_com[0]->dcd_w(state);
}

void i82091aa_device::ndsr1_w(int state)
{
	m_com[0]->dsr_w(state);
}

void i82091aa_device::nri1_w(int state)
{
	m_com[0]->ri_w(state);
}

void i82091aa_device::ncts1_w(int state)
{
	m_com[0]->cts_w(state);
}

void i82091aa_device::rxd2_w(int state)
{
	m_com[1]->rx_w(state);
}

void i82091aa_device::ndcd2_w(int state)
{
	m_com[1]->dcd_w(state);
}

void i82091aa_device::ndsr2_w(int state)
{
	m_com[1]->dsr_w(state);
}

void i82091aa_device::nri2_w(int state)
{
	m_com[1]->ri_w(state);
}

void i82091aa_device::ncts2_w(int state)
{
	m_com[1]->cts_w(state);
}

/*
 * LPT Parallel Port
 */

void i82091aa_device::irq_parallel_w(int state)
{
	m_parallel_irq = state;
	update_irq();
}
