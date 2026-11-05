// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SMSC FDC37C669 PC98/PC99 complaint Super I/O

Differences between this and '665:
- capability of setting addresses/IRQ/DRQ lines for the various devices;
- auto unmap at lower 0x000-0x0ff I/O range;
- an extra game port;
- different enable mechanisms;
- aggressive <reserved> masks when writing to the CR00~CR0C base registers;

TODO:
- IRQ and DRQ selects (more akin to FDC37C93X, with custom lines named IRQ_A ~ IRQ_H and
  DMA_A ~ DMA_C). UART can share irq by setting COM2 to 0xf;
- FDD drive type;
- PLL at CR10 (used by BIOSes);
- GAMECS, GPIO and PWRGD;

**************************************************************************************************/

#include "emu.h"
#include "fdc37c669.h"

#include "formats/naslite_dsk.h"

#define LOG_MAP    (1U << 1) // log remaps (verbose)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(FDC37C669, fdc37c669_device, "fdc37c669", "SMSC FDC37C669 Super I/O")

fdc37c669_device::fdc37c669_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDC37C669, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 6, 0, address_map_constructor(FUNC(fdc37c669_device::config_map), this))
	, m_fdc(*this, "fdc")
	, m_lpt(*this, "lpt")
	, m_com(*this, "com%d", 1U)
	, m_fintr_callback(*this)
	, m_pintr1_callback(*this)
	, m_irq3_callback(*this)
	, m_irq4_callback(*this)
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
{ }

ALLOW_SAVE_TYPE(fdc37c669_device::config_phase_t);

void fdc37c669_device::device_start()
{
	set_isa_device();
	//m_isa->set_dma_channel(0, this, true);
	//m_isa->set_dma_channel(1, this, true);
	m_isa->set_dma_channel(2, this, true);
	//m_isa->set_dma_channel(3, this, true);

	save_item(NAME(m_config_phase));
	save_item(NAME(m_index));
	save_item(NAME(m_cr));
	save_item(NAME(m_fdc_address));
	save_item(NAME(m_ide_address));
	save_item(NAME(m_lpt_address));
	save_item(NAME(m_com_address));
	save_item(NAME(m_gamecs_address));
	save_item(NAME(m_gamecs_config));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_lpt_drq));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_lpt_irq));
	save_item(NAME(m_com_irq));
	save_item(NAME(m_irqin));
}

void fdc37c669_device::device_reset()
{
	m_config_phase = config_phase_t::LOCK_55_0;
	m_cr[0] = 0x28;
	m_cr[1] = 0x9c;
	m_cr[2] = 0x88;
	m_cr[3] = 0x78;
	m_cr[4] = 0x00;
	m_cr[5] = 0x00;
	m_cr[6] = 0xff;
	m_cr[7] = 0x00;
	m_cr[8] = 0x00;
	m_cr[9] = 0x00;
	m_cr[0xa] = 0x00;
	// "TBD" in documentation
	m_cr[0xb] = m_cr[0xc] = 0;

	m_fdc->set_mode(n82077aa_device::mode_t::AT);
	m_fdc->set_rate(500000);

	m_fdc_address = 0x3c << 2;
	m_ide_address[0] = m_ide_address[1] = 0x3c << 2;
	m_lpt_address = 0x00 << 2;
	m_com_address[0] = m_com_address[1] = 0x00 << 2;
	m_gamecs_address = 0x3c << 2;
	m_gamecs_config = 0;
	m_fdd_dt = 0;
	m_fdc_drq = m_lpt_drq = 0;
	m_fdc_irq = m_lpt_irq = 0;
	m_com_irq[0] = m_com_irq[1] = 0;
	m_irqin = 0;

	remap(AS_IO, 0, 0x400);
}

device_memory_interface::space_config_vector fdc37c669_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("35ed", FLOPPY_35_ED);
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void fdc37c669_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
}


void fdc37c669_device::device_add_mconfig(machine_config &config)
{
	N82077AA(config, m_fdc, XTAL(24'000'000), n82077aa_device::mode_t::AT);
	m_fdc->intrq_wr_callback().set(FUNC(fdc37c669_device::irq_floppy_w));
	m_fdc->drq_wr_callback().set(FUNC(fdc37c669_device::drq_floppy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", fdc37c669_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", fdc37c669_device::floppy_formats);

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(fdc37c669_device::irq_parallel_w));

	NS16550(config, m_com[0], XTAL(24'000'000) / 13);
	m_com[0]->out_int_callback().set(FUNC(fdc37c669_device::irq_serial1_w));
	m_com[0]->out_tx_callback().set(FUNC(fdc37c669_device::txd_serial1_w));
	m_com[0]->out_dtr_callback().set(FUNC(fdc37c669_device::dtr_serial1_w));
	m_com[0]->out_rts_callback().set(FUNC(fdc37c669_device::rts_serial1_w));

	NS16550(config, m_com[1], XTAL(24'000'000) / 13);
	m_com[1]->out_int_callback().set(FUNC(fdc37c669_device::irq_serial2_w));
	m_com[1]->out_tx_callback().set(FUNC(fdc37c669_device::txd_serial2_w));
	m_com[1]->out_dtr_callback().set(FUNC(fdc37c669_device::dtr_serial2_w));
	m_com[1]->out_rts_callback().set(FUNC(fdc37c669_device::rts_serial2_w));
}

void fdc37c669_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// FDC
		if (BIT(m_cr[0], 3) && m_fdc_address & 0xf00)
		{
			const u16 fdc_start = m_fdc_address;
			const u16 fdc_end = fdc_start + 7;
			LOGMASKED(LOG_MAP, "Map FDC: %04x-%04x\n", fdc_start, fdc_end);
			m_isa->install_device(fdc_start, fdc_end, *m_fdc, &n82077aa_device::map);
		}

		// Map Super I/O registers after FDC, in case it's enabled
		if (m_config_phase != config_phase_t::LOCK_CR)
		{
			const u16 superio_base = 0x03f0;
			m_isa->install_device(superio_base, superio_base + 1, read8sm_delegate(*this, FUNC(fdc37c669_device::read)), write8sm_delegate(*this, FUNC(fdc37c669_device::write)));
		}

		// (*) IDE
		if (BIT(m_cr[0], 1) && (m_ide_address[0] & 0xf00 || m_ide_address[1] & 0xf00))
		{
			const bool alt_function = BIT(m_cr[0], 0);
			const u16 ide_start_cs0 = m_ide_address[0];
			const u16 ide_end_cs0 = ide_start_cs0 + 7;
			const u16 ide_start_cs1 = m_ide_address[1];
			const u16 ide_end_cs1 = ide_start_cs1 + 1;

			LOGMASKED(LOG_MAP, "Map IDE: %04x-%04x ~ %04x-%04x (alt: %d)\n", ide_start_cs0, ide_end_cs0, ide_start_cs1, ide_end_cs1, alt_function);
			// ...
		}

		// LPT
		if (BIT(m_cr[1], 2) && m_lpt_address & 0xf00)
		{
			const bool lpt_mode = BIT(m_cr[1], 3);
			const u16 lpt_start = m_lpt_address;
			const u16 lpt_end = lpt_start + 3;
			LOGMASKED(LOG_MAP, "Map LPT: %04x-%04x (mode %d)\n", lpt_start, lpt_end, lpt_mode);
			m_isa->install_device(lpt_start, lpt_end, *m_lpt, &pc_lpt_device::isa_map);
		}

		// COM1
		if (BIT(m_cr[2], 3) && m_com_address[0] & 0xf00)
		{
			const u16 com_start = m_com_address[0];
			const u16 com_end = com_start + 7;
			LOGMASKED(LOG_MAP, "Map COM1: %04x-%04x\n", com_start, com_end);
			m_isa->install_device(com_start, com_end + 7, read8sm_delegate(*m_com[0], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_com[0], FUNC(ns16450_device::ins8250_w)));
		}

		// COM2
		if (BIT(m_cr[2], 7) && m_com_address[1] & 0xf00)
		{
			const u16 com_start = m_com_address[1];
			const u16 com_end = com_start + 7;
			LOGMASKED(LOG_MAP, "Map COM2: %04x-%04x\n", com_start, com_end);
			m_isa->install_device(com_start, com_end + 7, read8sm_delegate(*m_com[1], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_com[1], FUNC(ns16450_device::ins8250_w)));
		}

		// TODO: PWRGD / GAMECS map
		// CR3 bit 0 selects type
		// CR08 ~ CR09 "ADRx" for GPIO
	}
}

u8 fdc37c669_device::read(offs_t offset)
{
	// TODO: outside of configuration mode should read FDC (if enabled)
	if (m_config_phase != config_phase_t::UNLOCK_DATA)
	{
		if (!machine().side_effects_disabled())
			LOG("Invalid config phase (%d) read %d\n", (u8)m_config_phase, offset);

		return 0xff;
	}

	return (offset == 0) ? m_index : space().read_byte(m_index);
}

// NOTE: same as 665GT/IR
void fdc37c669_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		switch(m_config_phase)
		{
			case config_phase_t::LOCK_55_0:
				if (data == 0x55)
					m_config_phase = config_phase_t::UNLOCK_55_1;
				else
					LOG("Invalid config phase (%d) write %02x\n", (u8)m_config_phase, data);
				break;

			case config_phase_t::UNLOCK_55_1:
				if (data == 0x55)
					m_config_phase = config_phase_t::UNLOCK_DATA;
				else
					LOG("Invalid config phase (%d) write %02x\n", (u8)m_config_phase, data);
				break;

			case config_phase_t::UNLOCK_DATA:
				if (data == 0xaa)
					m_config_phase = config_phase_t::LOCK_55_0;
				else if ((data & 0xc0) == 0)
					m_index = data;
				else
					LOG("Invalid config phase (%d) [%d] write %02x (register out of bounds)\n", (u8)m_config_phase, offset, data);
				break;

			case config_phase_t::LOCK_CR:
				LOG("Invalid config phase (%d) [%d] write %02x\n", (u8)m_config_phase, offset, data);
				break;
		}
	}
	else
	{
		if (m_config_phase == config_phase_t::UNLOCK_DATA)
			space().write_byte(m_index, data);
		else
			LOG("Invalid config phase (%d) [%d] write %02x\n", m_config_phase, offset, data);
	}
}

u8 fdc37c669_device::cr_setup_r(offs_t offset)
{
	return m_cr[offset];
}

void fdc37c669_device::cr_setup_w(offs_t offset, u8 data)
{
	const u8 reg_masks[0x0d] = {
		0x8b, 0x8c, 0x88, 0xf7,
		0xff, 0x7c, 0xff, 0xfc,
		0xf0, 0xc7, 0x0f, 0xff,
		0xff
	};
	const u8 mask = reg_masks[offset];
	LOG("CR%02X write %02x & %02x\n", offset, data, mask);
	m_cr[offset] = data & mask;

	if (offset == 3)
	{
		switch((m_cr[offset] >> 5) & 3)
		{
			case 0: m_fdc->set_mode(upd765_family_device::mode_t::M30); break;
			case 1: m_fdc->set_mode(upd765_family_device::mode_t::PS2); break;
			case 3: m_fdc->set_mode(upd765_family_device::mode_t::AT);  break;
			default:
				popmessage("FDD: select <reserved> mode 2");
				break;
		}
	}
	else if (offset <= 2)
	{
		if (offset == 1 && !BIT(data, 7))
		{
			LOG("CR1: Issued LOCK_CRx %02x\n", data);
			m_config_phase = config_phase_t::LOCK_CR;
		}

		remap(AS_IO, 0, 0x400);
	}
}

void fdc37c669_device::config_map(address_map &map)
{
	map(0x00, 0x0c).rw(FUNC(fdc37c669_device::cr_setup_r), FUNC(fdc37c669_device::cr_setup_w));
	map(0x0d, 0x0d).lr8(NAME([] () { return 0x03; }));
	map(0x0e, 0x0e).lr8(NAME([] () { return 0x02; }));
	// map(0x0f, 0x0f) Test block
	// map(0x10, 0x10) PLL control
	// map(0x11, 0x11) Test 10ms and IR loopback

	map(0x1e, 0x1e).lrw8(
		NAME([this] () {
			return (m_gamecs_address >> 2) | (m_gamecs_config & 3);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_gamecs_config = data & 3;
			m_gamecs_address = (data & 0xfc) << 2;
			remap(AS_IO, 0, 0x400);
		})
	);
	map(0x1f, 0x1f).lrw8(
		NAME([this] () {
			return m_fdd_dt;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fdd_dt = data;
			LOG("CR1F: %02x (Drive Type)\n", data);
			// TODO: set DT per drive
		})
	);
	map(0x20, 0x20).lrw8(
		NAME([this] () {
			return (m_fdc_address >> 2);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fdc_address = (data & 0xfc) << 2;
			LOG("CR20: %02x (FDC %04x)\n", data, m_fdc_address);
			remap(AS_IO, 0, 0x400);
		})
	);
	map(0x21, 0x22).lrw8(
		NAME([this] (offs_t offset) {
			const u8 which = offset ^ 1;
			return (m_ide_address[which] >> 2) | which;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 which = offset ^ 1;
			m_ide_address[which] = (data & 0xfc) << 2;
			LOG("CR%02X: %02x (IDE%d %04x)\n", which + 0x21, data, offset, m_ide_address[which]);
			remap(AS_IO, 0, 0x400);
		})
	);
	map(0x23, 0x23).lrw8(
		NAME([this] () {
			return m_lpt_address;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_lpt_address = data << 2;
			LOG("CR23: %02x (LPT %04x)\n", data, m_lpt_address);
			remap(AS_IO, 0, 0x400);
		})
	);
	map(0x24, 0x25).lrw8(
		NAME([this] (offs_t offset) {
			return (m_com_address[offset] >> 2) | offset;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_com_address[offset] = (data & 0xfe) << 2;
			LOG("CR%02X: %02x (COM%d %04x)\n", offset + 0x24, data, offset, m_com_address[offset]);
			remap(AS_IO, 0, 0x400);
		})
	);
	// CR26 ~ CR29 DRQ / IRQ section (TBD)
	map(0x26, 0x26).lrw8(
		NAME([this] () {
			return (m_fdc_drq << 4) | (m_lpt_drq);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fdc_drq = data >> 4;
			m_lpt_drq = data & 0xf;
			LOG("CR26: %02x (FDC DRQ %01x LPT DRQ %01x)\n", data, m_fdc_drq, m_lpt_drq);
		})
	);
	map(0x27, 0x27).lrw8(
		NAME([this] () {
			return (m_fdc_irq << 4) | (m_lpt_irq);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fdc_irq = data >> 4;
			m_lpt_irq = data & 0xf;
			LOG("CR27: %02x (FDC IRQ %01x LPT IRQ %01x)\n", data, m_fdc_irq, m_lpt_irq);
		})
	);
	map(0x28, 0x28).lrw8(
		NAME([this] () {
			return (m_com_irq[0] << 4) | (m_com_irq[1]);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_com_irq[0] = data >> 4;
			m_com_irq[1] = data & 0xf;
			LOG("CR28: %02x (COM1 IRQ %01x COM2 IRQ %01x)\n", data, m_com_irq[0], m_com_irq[1]);
		})
	);
	map(0x29, 0x29).lrw8(
		NAME([this] () {
			return m_irqin;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_irqin = data & 0xf;
			LOG("CR29: %02x (IRQIN %01x)\n", data, m_irqin);
		})
	);
}

/*
 * IRQ section
 */

// TODO: emulate properly
void fdc37c669_device::irq_floppy_w(int state)
{
	if (!BIT(m_cr[0], 3))
		return;

	m_fintr_callback(state);
}

void fdc37c669_device::drq_floppy_w(int state)
{
	if (!BIT(m_cr[0], 3))
		return;

	m_isa->drq2_w(state);
}

void fdc37c669_device::irq_parallel_w(int state)
{
	if (!BIT(m_cr[1], 2))
		return;

	m_pintr1_callback(state);
}

void fdc37c669_device::irq_serial1_w(int state)
{
	if (!BIT(m_cr[2], 3))
		return;

//  if (BIT(m_cr[2], 0))
//  {
//      // COM2 ~ COM4
//      m_irq3_callback(state);
//  }
//
//  else
//  {
//      // COM1 ~ COM3
//      m_irq4_callback(state);
//  }
}

void fdc37c669_device::irq_serial2_w(int state)
{
	if (!BIT(m_cr[2], 7))
		return;

//  if (BIT(m_cr[2], 4))
//  {
//      // COM1 ~ COM3
//      m_irq3_callback(state);
//  }
//  else
//  {
//      // COM2 ~ COM4
//      m_irq4_callback(state);
//  }
}

void fdc37c669_device::txd_serial1_w(int state)
{
	m_txd1_callback(state);
}

void fdc37c669_device::dtr_serial1_w(int state)
{
	m_ndtr1_callback(state);
}

void fdc37c669_device::rts_serial1_w(int state)
{
	m_nrts1_callback(state);
}

void fdc37c669_device::txd_serial2_w(int state)
{
	m_txd2_callback(state);
}

void fdc37c669_device::dtr_serial2_w(int state)
{
	m_ndtr2_callback(state);
}

void fdc37c669_device::rts_serial2_w(int state)
{
	m_nrts2_callback(state);
}

void fdc37c669_device::rxd1_w(int state)
{
	m_com[0]->rx_w(state);
}

void fdc37c669_device::ndcd1_w(int state)
{
	m_com[0]->dcd_w(state);
}

void fdc37c669_device::ndsr1_w(int state)
{
	m_com[0]->dsr_w(state);
}

void fdc37c669_device::nri1_w(int state)
{
	m_com[0]->ri_w(state);
}

void fdc37c669_device::ncts1_w(int state)
{
	m_com[0]->cts_w(state);
}

void fdc37c669_device::rxd2_w(int state)
{
	m_com[1]->rx_w(state);
}

void fdc37c669_device::ndcd2_w(int state)
{
	m_com[1]->dcd_w(state);
}

void fdc37c669_device::ndsr2_w(int state)
{
	m_com[1]->dsr_w(state);
}

void fdc37c669_device::nri2_w(int state)
{
	m_com[1]->ri_w(state);
}

void fdc37c669_device::ncts2_w(int state)
{
	m_com[1]->cts_w(state);
}
