// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Universal Peripheral Controller 82C711 emulation

**********************************************************************/

#include "emu.h"
#include "machine/upc82c711.h"

#define LOG_CFG (1 << 0)
#define LOG_FDC (1 << 1)
#define LOG_IDE (1 << 2)
#define LOG_LPT (1 << 3)
#define LOG_SER (1 << 4)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(UPC82C711, upc82c711_device, "upc82c711", "Universal Peripheral Controller 82C711")


upc82c711_device::upc82c711_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPC82C711, tag, owner, clock)
	, m_ide(*this, "ide")
	, m_fdc(*this, "fdc")
	, m_lpt(*this, "parallel")
	, m_serial(*this, "serial%u", 1U)
	, m_fintr_callback(*this)
	, m_fdrq_callback(*this)
	, m_pintr_callback(*this)
	, m_irq3_callback(*this)
	, m_irq4_callback(*this)
	, m_txd1_callback(*this)
	, m_dtr1_callback(*this)
	, m_rts1_callback(*this)
	, m_txd2_callback(*this)
	, m_dtr2_callback(*this)
	, m_rts2_callback(*this)
	, m_cfg_mode(0)
{
}


void upc82c711_device::device_add_mconfig(machine_config &config)
{
	// ide interface
	ATA_INTERFACE(config, m_ide);

	// floppy disc controller
	UPD765A(config, m_fdc, clock() / 3, false, false); // uPD72065B
	m_fdc->intrq_wr_callback().set(FUNC(upc82c711_device::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(upc82c711_device::fdc_drq_w));

	// parallel port
	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set([this](int state) { m_pintr_callback(state); });

	// serial ports
	NS16450(config, m_serial[0], clock() / 13);
	m_serial[0]->out_int_callback().set([this](int state) { m_irq4_callback(state); });
	m_serial[0]->out_tx_callback().set([this](int state) { m_txd1_callback(state); });
	m_serial[0]->out_dtr_callback().set([this](int state) { m_dtr1_callback(state); });
	m_serial[0]->out_rts_callback().set([this](int state) { m_rts1_callback(state); });

	NS16450(config, m_serial[1], clock() / 13);
	m_serial[1]->out_int_callback().set([this](int state) { m_irq3_callback(state); });
	m_serial[1]->out_tx_callback().set([this](int state) { m_txd2_callback(state); });
	m_serial[1]->out_dtr_callback().set([this](int state) { m_dtr2_callback(state); });
	m_serial[1]->out_rts_callback().set([this](int state) { m_rts2_callback(state); });
}


void upc82c711_device::device_start()
{
	for (int i=0; i<4; i++)
	{
		char name[2] = {static_cast<char>('0'+i), 0};
		floppy_connector *conn = m_fdc->subdevice<floppy_connector>(name);
		floppy[i] = conn ? conn->get_device() : nullptr;
	}

	irq = drq = false;
	fdc_irq = fdc_drq = false;

	m_fintr_callback.resolve_safe();
	m_fdrq_callback.resolve_safe();
	m_pintr_callback.resolve_safe();
	m_irq3_callback.resolve_safe();
	m_irq4_callback.resolve_safe();
	m_txd1_callback.resolve_safe();
	m_dtr1_callback.resolve_safe();
	m_rts1_callback.resolve_safe();
	m_txd2_callback.resolve_safe();
	m_dtr2_callback.resolve_safe();
	m_rts2_callback.resolve_safe();

	// default addresses
	com_address[0] = 0x3f8;
	com_address[1] = 0x2f8;
	com_address[2] = 0x338;
	com_address[3] = 0x238;

	device_address[DEVICE_IDE] = 0x1f0;
	device_address[DEVICE_FDC] = 0x3f0;
	device_address[DEVICE_LPT] = 0x278;
	device_address[DEVICE_SER1] = com_address[0];
	device_address[DEVICE_SER2] = com_address[1];

	const u8 cfg_regs_defaults[] = { 0x3f, 0x9f, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// Set the value first and then use write_config because some flags
	// rely on other flags being initialized properly first
	std::copy(std::begin(cfg_regs_defaults), std::end(cfg_regs_defaults), std::begin(m_cfg_regs));
	for (int i = 0; i < std::size(cfg_regs_defaults); i++)
	{
		write_cfg(i, cfg_regs_defaults[i]);
	}

	save_item(NAME(m_cfg_regs));
	save_item(NAME(m_cfg_indx));
	save_item(NAME(m_cfg_mode));
}

void upc82c711_device::device_reset()
{
	dor_w(0x00);
}


u16 upc82c711_device::io_r(offs_t offset, u16 mem_mask)
{
	u16 data = 0xffff;

	// configuration
	if (offset == 0x3f1 && m_cfg_mode == 2)
	{
		data = m_cfg_regs[m_cfg_indx];
		LOGMASKED(LOG_CFG, "CFG read %04x -> %02x\n", offset, data);
	}

	// ide
	if (device_enabled[DEVICE_IDE] && (offset & ~7) == 0x1f0)
	{
		data = m_ide->cs0_r(offset & 7);
		LOGMASKED(LOG_IDE, "IDE read %04x -> %04x\n", offset, data);
	}
	if (device_enabled[DEVICE_IDE] && offset == 0x3f6)
	{
		data = m_ide->cs1_r(offset & 7);
		LOGMASKED(LOG_IDE, "IDE read %04x -> %04x\n", offset, data);
	}

	// fdc
	if (device_enabled[DEVICE_FDC] && (offset & ~7) == 0x3f0 && offset != 0x3f0 && offset != 0x3f1 && offset != 0x3f6)
	{
		switch (offset & 7)
		{
		case 3:
			data = 0x20;
			break;
		case 4:
			data = m_fdc->msr_r();
			break;
		case 5:
			data = m_fdc->fifo_r();
			break;
		case 7:
			data = m_fdc->dir_r() | (m_ide->cs1_r(7) & 0x7f);
			break;
		}
		LOGMASKED(LOG_FDC, "FDC read %04x -> %02x\n", offset, data);
	}

	// parallel
	if (device_enabled[DEVICE_LPT] && (offset & ~3) == device_address[DEVICE_LPT])
	{
		data = m_lpt->read(offset & 3);
		LOGMASKED(LOG_LPT, "LPT read %04x -> %02x\n", offset, data);
	}

	// serial 1
	if (device_enabled[DEVICE_SER1] && (offset & ~7) == device_address[DEVICE_SER1])
	{
		data = m_serial[0]->ins8250_r(offset & 7);
		LOGMASKED(LOG_SER, "SER1 read %04x -> %02x\n", offset, data);
	}

	// serial 2
	if (device_enabled[DEVICE_SER2] && (offset & ~7) == device_address[DEVICE_SER2])
	{
		data = m_serial[1]->ins8250_r(offset & 7);
		LOGMASKED(LOG_SER, "SER2 read %04x -> %02x\n", offset, data);
	}

	return data;
}

void upc82c711_device::io_w(offs_t offset, u16 data, u16 mem_mask)
{
	// configuration
	if (offset == 0x3f0)
	{
		LOGMASKED(LOG_CFG, "CFG write %04x <- %02x\n", offset, data);
		if (m_cfg_mode == 2)
		{
			if (data == 0xaa) // escape configuration
			{
				m_cfg_mode = 0;
				LOGMASKED(LOG_CFG, "CFG -- mode off --\n");
			}
			else
			{
				m_cfg_indx = data & 0x0f;
			}
		}
		else
		{
			if (data == 0x55) // enter configuration
			{
				m_cfg_mode++;
				if (m_cfg_mode == 2)
					LOGMASKED(LOG_CFG, "CFG -- mode on --\n");
			}
			else
			{
				m_cfg_mode = 0;
			}
		}
	}
	else if (offset == 0x3f1)
	{
		LOGMASKED(LOG_CFG, "CFG write %04x <- %02x\n", offset, data);
		if (m_cfg_mode == 2)
		{
			write_cfg(m_cfg_indx, data);
		}
	}

	// ide
	if (device_enabled[DEVICE_IDE] && (offset & ~7) == 0x1f0)
	{
		m_ide->cs0_w(offset & 7, data);
		LOGMASKED(LOG_IDE, "IDE write %04x <- %04x\n", offset, data);
	}
	if (device_enabled[DEVICE_IDE] && offset == 0x3f6)
	{
		m_ide->cs1_w(offset & 7, data);
		LOGMASKED(LOG_IDE, "IDE write %04x <- %04x\n", offset, data);
	}

	// fdc
	if (device_enabled[DEVICE_FDC] && (offset & ~7) == 0x3f0 && offset != 0x3f0 && offset != 0x3f1 && offset != 0x3f6)
	{
		switch (offset & 7)
		{
		case 2:
			dor_w(data);
			break;
		case 5:
			m_fdc->fifo_w(data);
			break;
		case 7:
			m_fdc->ccr_w(data);
			break;
		}
		LOGMASKED(LOG_FDC, "FDC write %04x <- %02x\n", offset, data);
	}

	// parallel
	if (device_enabled[DEVICE_LPT] && (offset & ~3) == device_address[DEVICE_LPT])
	{
		m_lpt->write(offset & 3, data);
		LOGMASKED(LOG_LPT, "LPT write %04x <- %02x\n", offset, data);
	}

	// serial 1
	if (device_enabled[DEVICE_SER1] && (offset & ~7) == device_address[DEVICE_SER1])
	{
		m_serial[0]->ins8250_w(offset & 7, data);
		LOGMASKED(LOG_SER, "SER1 write %04x <- %02x\n", offset, data);
	}

	// serial 2
	if (device_enabled[DEVICE_SER2] && (offset & ~7) == device_address[DEVICE_SER2])
	{
		m_serial[1]->ins8250_w(offset & 7, data);
		LOGMASKED(LOG_SER, "SER2 write %04x <- %02x\n", offset, data);
	}
}


u8 upc82c711_device::dack_r()
{
	return m_fdc->dma_r();
}

void upc82c711_device::dack_w(u8 data)
{
	m_fdc->dma_w(data);
}

void upc82c711_device::tc_w(bool state)
{
	m_fdc->tc_w(state);
}


void upc82c711_device::dor_w(uint8_t data)
{
	dor = data;

	for (int i=0; i<4; i++)
		if (floppy[i])
			floppy[i]->mon_w(!(dor & (0x10 << i)));

	int fid = dor & 3;
	if (dor & (0x10 << fid))
		m_fdc->set_floppy(floppy[fid]);
	else
		m_fdc->set_floppy(nullptr);

	check_irq();
	check_drq();
	m_fdc->reset_w(!BIT(dor, 2));
}

WRITE_LINE_MEMBER(upc82c711_device::fdc_irq_w)
{
	fdc_irq = state;
	check_irq();
}

WRITE_LINE_MEMBER(upc82c711_device::fdc_drq_w)
{
	fdc_drq = state;
	check_drq();
}

void upc82c711_device::check_irq()
{
	bool pirq = irq;
	irq = fdc_irq && (dor & 4) && (dor & 8);
	if (irq != pirq)
		m_fintr_callback(irq);
}

void upc82c711_device::check_drq()
{
	bool pdrq = drq;
	drq = fdc_drq && (dor & 4) && (dor & 8);
	if (drq != pdrq)
		m_fdrq_callback(drq);
}


void upc82c711_device::write_cfg(int index, u8 data)
{
	m_cfg_regs[index] = data;
	LOGMASKED(LOG_CFG, "CR[%02x] = %02x\n", index, data);

	switch (index)
	{
	case 0x00:
		device_enabled[DEVICE_IDE] = BIT(m_cfg_regs[index], 0);
		device_enabled[DEVICE_FDC] = BIT(m_cfg_regs[index], 3) && BIT(m_cfg_regs[index], 4);
		LOGMASKED(LOG_CFG, "IDE %s\n", device_enabled[DEVICE_IDE] ? "enabled" : "disabled");
		LOGMASKED(LOG_CFG, "FDC %s\n", device_enabled[DEVICE_FDC] ? "enabled" : "disabled");
		break;

	case 0x01:
	{
		device_enabled[DEVICE_LPT] = BIT(m_cfg_regs[index], 2);

		auto lpt_port = BIT(m_cfg_regs[index], 0, 2);
		switch (lpt_port)
		{
		case 0: // Disabled
			device_enabled[DEVICE_LPT] = false;
			break;
		case 1:
			device_address[DEVICE_LPT] = 0x3bc;
			break;
		case 2:
			device_address[DEVICE_LPT] = 0x378;
			break;
		case 3: // Default
			device_address[DEVICE_LPT] = 0x278;
			break;
		}
		LOGMASKED(LOG_CFG, "LPT %04x %s\n", device_address[DEVICE_LPT], device_enabled[DEVICE_LPT] ? "enabled" : "disabled");
		auto com34 = BIT(m_cfg_regs[index], 5, 2);
		switch (com34)
		{
		case 0: // Default
			com_address[2] = 0x338;
			com_address[3] = 0x238;
			break;
		case 1:
			com_address[2] = 0x3e8;
			com_address[3] = 0x2e8;
			break;
		case 2:
			com_address[2] = 0x2e8;
			com_address[3] = 0x2e0;
			break;
		case 3:
			com_address[2] = 0x220;
			com_address[3] = 0x228;
			break;
		}
		break;
	}
	case 0x02:
		device_enabled[DEVICE_SER1] = BIT(m_cfg_regs[index], 2) && BIT(m_cfg_regs[index], 3);
		device_address[DEVICE_SER1] = com_address[BIT(m_cfg_regs[index], 0, 2)];

		device_enabled[DEVICE_SER2] = BIT(m_cfg_regs[index], 6) && BIT(m_cfg_regs[index], 7);
		device_address[DEVICE_SER2] = com_address[BIT(m_cfg_regs[index], 4, 2)];

		LOGMASKED(LOG_CFG, "SER1 %04x %s\n", device_address[DEVICE_SER1], device_enabled[DEVICE_SER1] ? "enabled" : "disabled");
		LOGMASKED(LOG_CFG, "SER2 %04x %s\n", device_address[DEVICE_SER2], device_enabled[DEVICE_SER2] ? "enabled" : "disabled");
		break;
	}
}


WRITE_LINE_MEMBER(upc82c711_device::rxd1_w)
{
	m_serial[0]->rx_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::dcd1_w)
{
	m_serial[0]->dcd_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::dsr1_w)
{
	m_serial[0]->dsr_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::ri1_w)
{
	m_serial[0]->ri_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::cts1_w)
{
	m_serial[0]->cts_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::rxd2_w)
{
	m_serial[1]->rx_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::dcd2_w)
{
	m_serial[1]->dcd_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::dsr2_w)
{
	m_serial[1]->dsr_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::ri2_w)
{
	m_serial[1]->ri_w(state);
}

WRITE_LINE_MEMBER(upc82c711_device::cts2_w)
{
	m_serial[1]->cts_w(state);
}
