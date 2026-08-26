// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli, windyfairy
/***************************************************************************

FDC37C665GT.h

SMSC FDC37C665GT High Performance Multi-Mode Parallel Port Super I/O Floppy Disk Controllers

***************************************************************************/

#include "emu.h"
#include "fdc37c665gt.h"

//#include <iostream>

#define LOG_CONFIG (1U << 1) // Show global configuration changes
#define LOG_ACCESS (1U << 2) // Show read/write access (verbose)

#define VERBOSE (LOG_GENERAL | LOG_CONFIG)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGCONFIG(...) LOGMASKED(LOG_CONFIG, __VA_ARGS__)
#define LOGACCESS(...) LOGMASKED(LOG_ACCESS, __VA_ARGS__)

DEFINE_DEVICE_TYPE(FDC37C665GT, fdc37c665gt_device, "fdc37c665gt", "SMSC FDC37C665GT Super I/O")

fdc37c665gt_device::fdc37c665gt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, upd765_family_device::mode_t floppy_mode)
	: device_t(mconfig, FDC37C665GT, tag, owner, clock)
	, mode(OperatingMode::Run)
	, config_key_step(0)
	, config_index(0)
	, m_floppy_mode(floppy_mode)
	, m_fintr_callback(*this)
	, m_fdrq_callback(*this)
	, m_pintr1_callback(*this)
	, m_irq3_callback(*this)
	, m_irq4_callback(*this)
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
	, m_fdc(*this, "fdc")
	, m_serial(*this, "uart%u", 1)
	, m_lpt(*this, "lpt")
	, m_ide(*this, "ide%u", 1)
{
}

void fdc37c665gt_device::device_start()
{
	// Configuration registers and related bits aren't affected by soft resets
	// Default addresses
	com_addresses[0] = 0x3f8;
	com_addresses[1] = 0x2f8;
	com_addresses[2] = 0x338;
	com_addresses[3] = 0x238;

	device_addresses[LogicalDevice::IDE] = 0x1f0;
	device_addresses[LogicalDevice::FDC] = 0x3f0;
	device_addresses[LogicalDevice::Parallel] = 0x278;
	device_addresses[LogicalDevice::Serial1] = 0; // COM port
	device_addresses[LogicalDevice::Serial2] = 1; // COM port

	const uint8_t configuration_registers_defaults[] = {
		0x3b, 0x9f, 0xdc, 0x78, 0x00,
		0x00, 0xff, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x66, 0x01, 0x00
	};

	// Set the value first and then use write_configuration_register because some flags
	// rely on other flags being initialized properly first
	std::copy(std::begin(configuration_registers_defaults), std::end(configuration_registers_defaults), std::begin(configuration_registers));
	for (int i = 0; i < std::size(configuration_registers_defaults); i++) {
		write_configuration_register(i, configuration_registers_defaults[i]);
	}
}

void fdc37c665gt_device::device_add_mconfig(machine_config &config)
{
	// floppy disc controller
	N82077AA(config, m_fdc, 24_MHz_XTAL, m_floppy_mode);
	m_fdc->intrq_wr_callback().set(FUNC(fdc37c665gt_device::irq_floppy_w));
	m_fdc->drq_wr_callback().set(FUNC(fdc37c665gt_device::drq_floppy_w));

	// parallel port
	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(fdc37c665gt_device::irq_parallel_w));

	// serial ports
	NS16550(config, m_serial[0], clock() / 13);
	m_serial[0]->out_int_callback().set(FUNC(fdc37c665gt_device::irq_serial1_w));
	m_serial[0]->out_tx_callback().set(FUNC(fdc37c665gt_device::txd_serial1_w));
	m_serial[0]->out_dtr_callback().set(FUNC(fdc37c665gt_device::dtr_serial1_w));
	m_serial[0]->out_rts_callback().set(FUNC(fdc37c665gt_device::rts_serial1_w));

	NS16550(config, m_serial[1], clock() / 13);
	m_serial[1]->out_int_callback().set(FUNC(fdc37c665gt_device::irq_serial2_w));
	m_serial[1]->out_tx_callback().set(FUNC(fdc37c665gt_device::txd_serial2_w));
	m_serial[1]->out_dtr_callback().set(FUNC(fdc37c665gt_device::dtr_serial2_w));
	m_serial[1]->out_rts_callback().set(FUNC(fdc37c665gt_device::rts_serial2_w));

	// NOTE: irq(s) is client responsibility (no pins on Super I/O)
	ATA_INTERFACE(config, m_ide[0]).options(ata_devices, nullptr, nullptr, false);

	ATA_INTERFACE(config, m_ide[1]).options(ata_devices, nullptr, nullptr, false);
}

uint8_t fdc37c665gt_device::read(offs_t offset)
{
	LOGACCESS("[%04x]\n", offset);

	if (offset == 0x3f1 && mode == OperatingMode::Configuration) {
		u8 res = 0;
		switch(config_index)
		{
			case 0x0d:
				// read identifier (most if not all riscpc targets)
				// '666GT reads 0x66 here
				res = 0x65;
				break;
			case 0x0e:
				// chip revision level
				res = 0x02;
				break;
			default:
				if (config_index & 0xf0)
					LOG("Warning: read of register %02x with upper address bits set\n", config_index);
				res = configuration_registers[config_index & 0xf];
				break;
		}

		//printf("[%02x] -> %02x\n", config_index, res);

		return res;
	}

	// TODO: a7000p access at $70~$73, does it belong here or it's an external (ISA) GPIO?

	// Parallel port
	if (offset >= device_addresses[LogicalDevice::Parallel] && offset <= device_addresses[LogicalDevice::Parallel] + 2) {
		if (!enabled_logical[LogicalDevice::Parallel]) {
			return 0;
		}

		return m_lpt->read(offset - device_addresses[LogicalDevice::Parallel]);
	}

	// Serial 1
	if (offset >= com_addresses[device_addresses[LogicalDevice::Serial1]] && offset <= com_addresses[device_addresses[LogicalDevice::Serial1]] + 7) {
		if (!enabled_logical[LogicalDevice::Serial1]) {
			return 0;
		}

		return m_serial[0]->ins8250_r(offset - com_addresses[device_addresses[LogicalDevice::Serial1]]);
	}

	// Serial 2
	if (offset >= com_addresses[device_addresses[LogicalDevice::Serial2]] && offset <= com_addresses[device_addresses[LogicalDevice::Serial2]] + 7) {
		if (!enabled_logical[LogicalDevice::Serial2]) {
			return 0;
		}

		return m_serial[1]->ins8250_r(offset - com_addresses[device_addresses[LogicalDevice::Serial2]]);
	}

	// FDC, +6 is used by IDE
	if ((offset >= device_addresses[LogicalDevice::FDC] && offset <= device_addresses[LogicalDevice::FDC] + 5)
		|| offset == device_addresses[LogicalDevice::FDC] + 7) {
		if (!enabled_logical[LogicalDevice::FDC]) {
			return 0;
		}

		switch (offset - device_addresses[LogicalDevice::FDC]) {
			case 0: return m_fdc->sra_r();
			case 1: return m_fdc->srb_r();
			case 2: return m_fdc->dor_r();
			case 3: return m_fdc->tdr_r();
			case 4: return m_fdc->msr_r();
			case 5: return m_fdc->fifo_r();
			case 7: return m_fdc->dir_r();
		}
	}

	// IDE
	if ((offset & 0x178) == 0x170 && enabled_logical[LogicalDevice::IDE]) {
		// HACK: range $xx6~$xx7 looks sensitive on riscpc (and returns 0xff with current core)
		// Will hang even if no ATA device mounted
		if ((offset & 0x6) == 6)
			return machine().rand();

		// $1f0 IDE1 cs0
		// $3f0 IDE1 cs1
		// $170 IDE2 cs0
		// $370 IDE2 cs1
		const u8 cs_select = BIT(offset, 9);
		const u8 ide_target = !BIT(offset, 7);

		auto &ide_dev = m_ide[ide_target];

		return cs_select ? ide_dev->cs1_r(offset & 7, 0xff) : ide_dev->cs0_r(offset & 7, 0xff);
	}

	return 0;
}

void fdc37c665gt_device::write(offs_t offset, uint8_t data)
{
	LOGACCESS("[%04x] %02x\n", offset, data);

	// Parallel port
	if (offset >= device_addresses[LogicalDevice::Parallel] && offset <= device_addresses[LogicalDevice::Parallel] + 2) {
		if (!enabled_logical[LogicalDevice::Parallel]) {
			return;
		}

		m_lpt->write(offset - device_addresses[LogicalDevice::Parallel], data);
		return;
	}

	// Serial 1
	if (offset >= com_addresses[device_addresses[LogicalDevice::Serial1]] && offset <= com_addresses[device_addresses[LogicalDevice::Serial1]] + 7) {
		if (!enabled_logical[LogicalDevice::Serial1]) {
			return;
		}

		m_serial[0]->ins8250_w(offset - com_addresses[device_addresses[LogicalDevice::Serial1]], data);
		return;
	}

	// Serial 2
	if (offset >= com_addresses[device_addresses[LogicalDevice::Serial2]] && offset <= com_addresses[device_addresses[LogicalDevice::Serial2]] + 7) {
		if (!enabled_logical[LogicalDevice::Serial2]) {
			return;
		}

		m_serial[1]->ins8250_w(offset - com_addresses[device_addresses[LogicalDevice::Serial2]], data);
		return;
	}

	// FDC, +6 is used by IDE
	if ((offset >= device_addresses[LogicalDevice::FDC] && offset <= device_addresses[LogicalDevice::FDC] + 5)
		|| offset == device_addresses[LogicalDevice::FDC] + 7) {
		auto fdc_offset = offset - device_addresses[LogicalDevice::FDC];

		if ((!enabled_logical[LogicalDevice::FDC] && fdc_offset > 1)) {
			return;
		}

		switch (fdc_offset) {
			case 0: // FDC37C665GT Configuration
				if (mode == OperatingMode::Run) {
					if (data != 0x55) {
						config_key_step = 0;
						return;
					}

					config_key_step++;
					if (config_key_step > 1) {
						config_key_step = 0;
						mode = OperatingMode::Configuration;
					}
				} else {
					if (data == 0xaa) {
						mode = OperatingMode::Run;
						return;
					}

					config_index = data;
				}
				return;

			case 1: // FDC37C665GT Configuration
				if (mode == OperatingMode::Run) {
					config_key_step = 0;
					return;
				}

				write_configuration_register(config_index, data & 0xff);
				return;

			case 2: m_fdc->dor_w(data); return;
			case 3: m_fdc->tdr_w(data); return;
			case 4: m_fdc->dsr_w(data); return;
			case 5: m_fdc->fifo_w(data); return;
			case 7: m_fdc->ccr_w(data); return;
		}
	}

	// IDE
	if ((offset & 0x178) == 0x170 && enabled_logical[LogicalDevice::IDE]) {
		const u8 cs_select = BIT(offset, 9);
		const u8 ide_target = !BIT(offset, 7);

		auto &ide_dev = m_ide[ide_target];

		if(cs_select)
			ide_dev->cs1_w(offset & 7, data, 0xff);
		else
			ide_dev->cs0_w(offset & 7, data, 0xff);
	}
}

void fdc37c665gt_device::write_configuration_register(int index, int data)
{
	if (BIT(configuration_registers[1], 7) == 0) {
		// Bit 7 of CR1 is LOCK CRx
		// When this is set to 0, it can only be set back to 1 by a hard reset or power-up reset
		LOGCONFIG("IGNORED configuration register cr[%02x] = %02x\n", index, data);
		return;
	}

	configuration_registers[index] = data;
	LOGCONFIG("Modified configuration register cr[%02x] = %02x\n", index, data);

	if (index == 0) {
		enabled_logical[LogicalDevice::IDE] = BIT(configuration_registers[index], 0);
		// TODO: bit 1 IDE AT/XT mode
		enabled_logical[LogicalDevice::FDC] = BIT(configuration_registers[index], 3) && BIT(configuration_registers[index], 4);
	} else if (index == 1) {
		enabled_logical[LogicalDevice::Parallel] = BIT(configuration_registers[index], 2) && BIT(configuration_registers[index], 3);

		auto lpt_port = BIT(configuration_registers[index], 0, 2);
		if (lpt_port == 0) {
			enabled_logical[LogicalDevice::Parallel] = false; // Disabled
		} else if (lpt_port == 1) {
			device_addresses[LogicalDevice::Parallel] = 0x3bc;
		} else if (lpt_port == 2) {
			device_addresses[LogicalDevice::Parallel] = 0x378;
		} else if (lpt_port == 3) {
			device_addresses[LogicalDevice::Parallel] = 0x278; // Default
		}

		// TODO: bit 4 irq polarity

		auto com34 = BIT(configuration_registers[index], 5, 2);
		if (com34 == 0) {
			com_addresses[2] = 0x338;
			com_addresses[3] = 0x238;
		} else if (com34 == 1) {
			com_addresses[2] = 0x3e8;
			com_addresses[3] = 0x2e8;
		} else if (com34 == 2) {
			com_addresses[2] = 0x2e8;
			com_addresses[3] = 0x2e0;
		} else if (com34 == 3) {
			com_addresses[2] = 0x220;
			com_addresses[3] = 0x228;
		}
	} else if (index == 2) {
		enabled_logical[LogicalDevice::Serial1] = BIT(configuration_registers[index], 2) && BIT(configuration_registers[index], 3);
		device_addresses[LogicalDevice::Serial1] = BIT(configuration_registers[index], 0, 2);

		enabled_logical[LogicalDevice::Serial2] = BIT(configuration_registers[index], 6) && BIT(configuration_registers[index], 7);
		device_addresses[LogicalDevice::Serial2] = BIT(configuration_registers[index], 4, 2);
	} else if (index == 3) {
		// TODO: enhanced floppy mode 2 (bit 1), Drive Options (bits 3~4), PINTR (bit 2), ADRx/DRV2 EN/PINTR (bit 7)
		auto floppy_mode = BIT(configuration_registers[index], 5, 2);

		// 2 is reserved/unused
		if (floppy_mode == 3) {
			m_floppy_mode = upd765_family_device::mode_t::AT;
		} else if (floppy_mode == 1) {
			m_floppy_mode = upd765_family_device::mode_t::PS2;
		} else if (floppy_mode == 0) {
			m_floppy_mode = upd765_family_device::mode_t::M30;
		}

		m_fdc->set_mode(m_floppy_mode);
	} else if (index == 4) {
		// Set clock speeds for MIDI modes (clock divisor becomes 12 instead of 13)
		m_serial[0]->set_unscaled_clock(clock() / (13 - BIT(configuration_registers[4], 4)));
		m_serial[1]->set_unscaled_clock(clock() / (13 - BIT(configuration_registers[4], 5)));
		// TODO: PP EXT modes (bits 1~0), EPP Type (bit 6), Parallel Port FDC (bits 3~2)
	} else if (index == 5) {
		auto fdc_port = BIT(configuration_registers[index], 0);
		if (fdc_port == 0) {
			device_addresses[LogicalDevice::FDC] = 0x3f0;
		} else if (fdc_port == 1) {
			device_addresses[LogicalDevice::FDC] = 0x370;
		}
		// TODO: IDE Secondary (bit 1), FDC DMA Mode (bit 2), DenSel (bits 4~3), Swap drv 0,1 (bit 5), EXTx4 (bit 6), DS3 (bit 7)
	}

	// TODO: CR6 floppy disk type, reflected in FDC $3f3 bits 5~4
	// TODO: CR7 Media ID Polarity (bits 3~2) and Floppy Boot Drive (bits 1~0)
	// TODO: CR8~CR9 ADRx
	// TODO: CRA ECP FIFO Threshold (bits 3~0)
	// TODO: CRF Test Modes
}

void fdc37c665gt_device::irq_floppy_w(int state)
{
	if (!enabled_logical[LogicalDevice::FDC]) {
		return;
	}

	m_fintr_callback(state);
}

void fdc37c665gt_device::drq_floppy_w(int state)
{
	if (!enabled_logical[LogicalDevice::FDC]) {
		return;
	}

	m_fdrq_callback(state);
}

void fdc37c665gt_device::irq_parallel_w(int state)
{
	if (!enabled_logical[LogicalDevice::Parallel]) {
		return;
	}

	m_pintr1_callback(state);
}

void fdc37c665gt_device::irq_serial1_w(int state)
{
	if (!enabled_logical[LogicalDevice::Serial1]) {
		return;
	}

	m_irq4_callback(state);
}

void fdc37c665gt_device::txd_serial1_w(int state)
{
	if (!enabled_logical[LogicalDevice::Serial1]) {
		return;
	}

	m_txd1_callback(state);
}

void fdc37c665gt_device::dtr_serial1_w(int state)
{
	if (!enabled_logical[LogicalDevice::Serial1]) {
		return;
	}

	m_ndtr1_callback(state);
}

void fdc37c665gt_device::rts_serial1_w(int state)
{
	if (!enabled_logical[LogicalDevice::Serial1]) {
		return;
	}

	m_nrts1_callback(state);
}

void fdc37c665gt_device::irq_serial2_w(int state)
{
	if (!enabled_logical[LogicalDevice::Serial2]) {
		return;
	}

	m_irq3_callback(state);
}

void fdc37c665gt_device::txd_serial2_w(int state)
{
	if (!enabled_logical[LogicalDevice::Serial2]) {
		return;
	}

	m_txd2_callback(state);
}

void fdc37c665gt_device::dtr_serial2_w(int state)
{
	if (!enabled_logical[LogicalDevice::Serial2]) {
		return;
	}

	m_ndtr2_callback(state);
}

void fdc37c665gt_device::rts_serial2_w(int state)
{
	if (!enabled_logical[LogicalDevice::Serial2]) {
		return;
	}

	m_nrts2_callback(state);
}

void fdc37c665gt_device::rxd1_w(int state)
{
	m_serial[0]->rx_w(state);
}

void fdc37c665gt_device::ndcd1_w(int state)
{
	m_serial[0]->dcd_w(state);
}

void fdc37c665gt_device::ndsr1_w(int state)
{
	m_serial[0]->dsr_w(state);
}

void fdc37c665gt_device::nri1_w(int state)
{
	m_serial[0]->ri_w(state);
}

void fdc37c665gt_device::ncts1_w(int state)
{
	m_serial[0]->cts_w(state);
}

void fdc37c665gt_device::rxd2_w(int state)
{
	m_serial[1]->rx_w(state);
}

void fdc37c665gt_device::ndcd2_w(int state)
{
	m_serial[1]->dcd_w(state);
}

void fdc37c665gt_device::ndsr2_w(int state)
{
	m_serial[1]->dsr_w(state);
}

void fdc37c665gt_device::nri2_w(int state)
{
	m_serial[1]->ri_w(state);
}

void fdc37c665gt_device::ncts2_w(int state)
{
	m_serial[1]->cts_w(state);
}
