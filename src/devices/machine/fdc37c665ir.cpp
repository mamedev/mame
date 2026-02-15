// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SMSC FDC37C665IR

TODO:
- merge with GT version (that core doesn't use ISA at current time);
- in turn homebrew/p112 doesn't use ISA but a custom bus tailored on Z180;
- IDE (is it even supported in this variant?)

**************************************************************************************************/

#include "emu.h"
#include "fdc37c665ir.h"

#include "formats/naslite_dsk.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(FDC37C665IR, fdc37c665ir_device, "fdc37c665ir", "SMSC FDC37C665IR Super I/O FDC with IR support")

fdc37c665ir_device::fdc37c665ir_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDC37C665IR, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(fdc37c665ir_device::config_map), this))
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

ALLOW_SAVE_TYPE(fdc37c665ir_device::config_phase_t);

void fdc37c665ir_device::device_start()
{
	set_isa_device();
	//m_isa->set_dma_channel(0, this, true);
	//m_isa->set_dma_channel(1, this, true);
	m_isa->set_dma_channel(2, this, true);
	//m_isa->set_dma_channel(3, this, true);

	save_item(NAME(m_config_phase));
	save_item(NAME(m_index));
	save_item(NAME(m_cr));
}

void fdc37c665ir_device::device_reset()
{
	m_config_phase = config_phase_t::LOCK_55_0;
	m_cr[0] = 0x3b;
	m_cr[1] = 0x9f;
	m_cr[2] = 0xdc;
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

	remap(AS_IO, 0, 0x400);
}

device_memory_interface::space_config_vector fdc37c665ir_device::memory_space_config() const
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

void fdc37c665ir_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
}


void fdc37c665ir_device::device_add_mconfig(machine_config &config)
{
	N82077AA(config, m_fdc, XTAL(24'000'000), n82077aa_device::mode_t::AT);
	m_fdc->intrq_wr_callback().set(FUNC(fdc37c665ir_device::irq_floppy_w));
	m_fdc->drq_wr_callback().set(FUNC(fdc37c665ir_device::drq_floppy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", fdc37c665ir_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", fdc37c665ir_device::floppy_formats);

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(fdc37c665ir_device::irq_parallel_w));

	NS16550(config, m_com[0], XTAL(24'000'000) / 13);
	m_com[0]->out_int_callback().set(FUNC(fdc37c665ir_device::irq_serial1_w));
	m_com[0]->out_tx_callback().set(FUNC(fdc37c665ir_device::txd_serial1_w));
	m_com[0]->out_dtr_callback().set(FUNC(fdc37c665ir_device::dtr_serial1_w));
	m_com[0]->out_rts_callback().set(FUNC(fdc37c665ir_device::rts_serial1_w));

	NS16550(config, m_com[1], XTAL(24'000'000) / 13);
	m_com[1]->out_int_callback().set(FUNC(fdc37c665ir_device::irq_serial2_w));
	m_com[1]->out_tx_callback().set(FUNC(fdc37c665ir_device::txd_serial2_w));
	m_com[1]->out_dtr_callback().set(FUNC(fdc37c665ir_device::dtr_serial2_w));
	m_com[1]->out_rts_callback().set(FUNC(fdc37c665ir_device::rts_serial2_w));
}

// getting the serial address is a bit convoluted
u16 fdc37c665ir_device::get_com_address(u8 setting)
{
	// regular COM1/COM2 setting
	if (!(setting & 2))
	{
		//                   COM2    COM1
		return setting & 1 ? 0x2f8 : 0x3f8;
	}

	// otherwise read COM3/COM4 register table and derive base from there
	const u16 com34_addresses[8] = {
		// COM3
		0x338, 0x3e8, 0x2e8, 0x220,
		// COM4
		0x238, 0x2e8, 0x2e0, 0x228
	};

	return com34_addresses[((m_cr[1] >> 5) & 3) | ((setting & 1) << 4)];
}

void fdc37c665ir_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// FDC
		if (BIT(m_cr[0], 4))
		{
			const u16 fdc_start = BIT(m_cr[5], 0) ? 0x370 : 0x3f0;
			const u16 fdc_end = fdc_start + 7;
			LOG("Map FDC: %04x-%04x\n", fdc_start, fdc_end);
			m_isa->install_device(fdc_start, fdc_end, *m_fdc, &n82077aa_device::map);
		}

		// Map Super I/O registers after FDC, in case it's enabled
		if (m_config_phase != config_phase_t::LOCK_CR)
		{
			const u16 superio_base = 0x03f0;
			m_isa->install_device(superio_base, superio_base + 1, read8sm_delegate(*this, FUNC(fdc37c665ir_device::read)), write8sm_delegate(*this, FUNC(fdc37c665ir_device::write)));
		}

		// IDE
		if (BIT(m_cr[0], 0))
		{
			const bool ide_is_secondary = BIT(m_cr[5], 1);
			const u16 ide_start_cs0 = ide_is_secondary ? 0x170 : 0x1f0;
			const u16 ide_end_cs0 = ide_start_cs0 + 7;
			const u16 ide_start_cs1 = ide_is_secondary ? 0x376 : 0x3f6;
			const u16 ide_end_cs1 = ide_start_cs1 + 1;

			LOG("Map IDE: %04x-%04x ~ %04x-%04x\n", ide_start_cs0, ide_end_cs0, ide_start_cs1, ide_end_cs1);
			// ...
		}

		// LPT: enabled if setting != 0
		if (m_cr[1] & 3)
		{
			const u16 lpt_addresses[3] = { 0x3bc, 0x378, 0x278 };
			const u16 lpt_start = lpt_addresses[(m_cr[1] & 3) - 1];
			const u16 lpt_end = lpt_start + 3;
			LOG("Map LPT: %04x-%04x\n", lpt_start, lpt_end);
			m_isa->install_device(lpt_start, lpt_end, read8sm_delegate(*m_lpt, FUNC(pc_lpt_device::read)), write8sm_delegate(*m_lpt, FUNC(pc_lpt_device::write)));
		}

		// COM1
		if (BIT(m_cr[2], 2))
		{
			const u16 com_start = get_com_address(m_cr[2] & 3);
			const u16 com_end = com_start + 7;
			LOG("Map COM1: %04x-%04x\n", com_start, com_end);
			m_isa->install_device(com_start, com_end + 7, read8sm_delegate(*m_com[0], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_com[0], FUNC(ns16450_device::ins8250_w)));
		}

		// COM2
		if (BIT(m_cr[2], 6))
		{
			const u16 com_start = get_com_address((m_cr[2] >> 4) & 3);
			const u16 com_end = com_start + 7;
			LOG("Map COM2: %04x-%04x\n", com_start, com_end);
			m_isa->install_device(com_start, com_end + 7, read8sm_delegate(*m_com[1], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_com[1], FUNC(ns16450_device::ins8250_w)));
		}

	}
}

u8 fdc37c665ir_device::read(offs_t offset)
{
	// TODO: outside of configuration mode should read FDC (if enabled)
	if (m_config_phase != config_phase_t::UNLOCK_DATA)
	{
		if (!machine().side_effects_disabled())
			LOG("Invalid config phase (%d) read %d\n", m_config_phase, offset);

		return 0xff;
	}

	return (offset == 0) ? m_index : space().read_byte(m_index);
}

void fdc37c665ir_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		switch(m_config_phase)
		{
			case config_phase_t::LOCK_55_0:
				if (data == 0x55)
					m_config_phase = config_phase_t::UNLOCK_55_1;
				else
					LOG("Invalid config phase (%d) write %02x\n", m_config_phase, data);
				break;

			case config_phase_t::UNLOCK_55_1:
				if (data == 0x55)
					m_config_phase = config_phase_t::UNLOCK_DATA;
				else
					LOG("Invalid config phase (%d) write %02x\n", m_config_phase, data);
				break;

			case config_phase_t::UNLOCK_DATA:
				if (data == 0xaa)
					m_config_phase = config_phase_t::LOCK_55_0;
				else if ((data & 0xf0) == 0)
					m_index = data;
				else
					LOG("Invalid config phase (%d) [%d] write %02x\n", m_config_phase, offset, data);
				break;

			case config_phase_t::LOCK_CR:
				LOG("Invalid config phase (%d) [%d] write %02x\n", m_config_phase, offset, data);
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

void fdc37c665ir_device::config_map(address_map &map)
{
	map(0x00, 0x0c).lrw8(
		NAME([this] (offs_t offset) { return m_cr[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("CR%01X write %02x\n", offset, data);
			m_cr[offset] = data;

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
			else if (offset == 0 || offset == 1 || offset == 2 || offset == 5)
			{
				if (offset == 1 && !BIT(data, 7))
				{
					LOG("CR1: Issued LOCK_CRx %02x\n", data);
					m_config_phase = config_phase_t::LOCK_CR;
				}

				remap(AS_IO, 0, 0x400);
			}
		})
	);
	map(0x0d, 0x0d).lr8(NAME([] () { return 0x65; })); // 0x66 on GT
	map(0x0e, 0x0e).lr8(NAME([] () { return 0x82; })); // 0x01 on GT
}

/*
 * IRQ section
 * Compared to other Super I/Os this has no way to configure irq lines
 */

void fdc37c665ir_device::irq_floppy_w(int state)
{
	if (!BIT(m_cr[0], 4))
		return;

	// TODO: m_cr[1] bit 4 for IRQ polarity
	m_fintr_callback(state);
}

void fdc37c665ir_device::drq_floppy_w(int state)
{
	if (!BIT(m_cr[0], 4))
		return;

	m_isa->drq2_w(state);
}

void fdc37c665ir_device::irq_parallel_w(int state)
{
	if ((m_cr[1] & 3) == 0)
		return;

	m_pintr1_callback(state);
}

void fdc37c665ir_device::irq_serial1_w(int state)
{
	if (!BIT(m_cr[2], 2))
		return;

	if (BIT(m_cr[2], 0))
	{
		// COM2 ~ COM4
		m_irq3_callback(state);
	}
	else
	{
		// COM1 ~ COM3
		m_irq4_callback(state);
	}
}

void fdc37c665ir_device::irq_serial2_w(int state)
{
	if (!BIT(m_cr[2], 6))
		return;

	if (BIT(m_cr[2], 4))
	{
		// COM1 ~ COM3
		m_irq3_callback(state);
	}
	else
	{
		// COM2 ~ COM4
		m_irq4_callback(state);
	}
}

void fdc37c665ir_device::txd_serial1_w(int state)
{
	m_txd1_callback(state);
}

void fdc37c665ir_device::dtr_serial1_w(int state)
{
	m_ndtr1_callback(state);
}

void fdc37c665ir_device::rts_serial1_w(int state)
{
	m_nrts1_callback(state);
}

void fdc37c665ir_device::txd_serial2_w(int state)
{
	m_txd2_callback(state);
}

void fdc37c665ir_device::dtr_serial2_w(int state)
{
	m_ndtr2_callback(state);
}

void fdc37c665ir_device::rts_serial2_w(int state)
{
	m_nrts2_callback(state);
}

void fdc37c665ir_device::rxd1_w(int state)
{
	m_com[0]->rx_w(state);
}

void fdc37c665ir_device::ndcd1_w(int state)
{
	m_com[0]->dcd_w(state);
}

void fdc37c665ir_device::ndsr1_w(int state)
{
	m_com[0]->dsr_w(state);
}

void fdc37c665ir_device::nri1_w(int state)
{
	m_com[0]->ri_w(state);
}

void fdc37c665ir_device::ncts1_w(int state)
{
	m_com[0]->cts_w(state);
}

void fdc37c665ir_device::rxd2_w(int state)
{
	m_com[1]->rx_w(state);
}

void fdc37c665ir_device::ndcd2_w(int state)
{
	m_com[1]->dcd_w(state);
}

void fdc37c665ir_device::ndsr2_w(int state)
{
	m_com[1]->dsr_w(state);
}

void fdc37c665ir_device::nri2_w(int state)
{
	m_com[1]->ri_w(state);
}

void fdc37c665ir_device::ncts2_w(int state)
{
	m_com[1]->cts_w(state);
}

