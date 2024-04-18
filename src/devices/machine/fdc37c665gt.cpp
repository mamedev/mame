// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli, windyfairy
/***************************************************************************

FDC37C665GT.h

SMSC FDC37C665GT High Performance Multi-Mode Parallel Port Super I/O Floppy Disk Controllers

***************************************************************************/

#include "emu.h"
#include "fdc37c665gt.h"

#define LOG_CONFIG (1U << 1) // Show global configuration changes

#define VERBOSE (LOG_GENERAL | LOG_CONFIG)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGCONFIG(...) LOGMASKED(LOG_CONFIG, __VA_ARGS__)

DEFINE_DEVICE_TYPE(FDC37C665GT, fdc37c665gt_device, "fdc37c665gt", "FDC37C665GT")

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
}

uint8_t fdc37c665gt_device::read(offs_t offset)
{
	// TODO: IDE not implemented

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

	return 0;
}

void fdc37c665gt_device::write(offs_t offset, uint8_t data)
{
	// TODO: IDE not implemented

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
		enabled_logical[LogicalDevice::IDE] = BIT(configuration_registers[index], 1);
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
		m_serial[1]->set_unscaled_clock(clock() / (13 - BIT(configuration_registers[5], 5)));
	} else if (index == 5) {
		auto fdc_port = BIT(configuration_registers[index], 0);
		if (fdc_port == 0) {
			device_addresses[LogicalDevice::FDC] = 0x3f0;
		} else if (fdc_port == 1) {
			device_addresses[LogicalDevice::FDC] = 0x370;
		}
	}
}

void fdc37c665gt_device::irq_floppy_w(int state)
{
	if (!enabled_logical[LogicalDevice::FDC]) {
		return;
	}

	m_fintr_callback(state);
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
