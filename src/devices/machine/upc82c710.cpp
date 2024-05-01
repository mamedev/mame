// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Universal Peripheral Controller 82C710 emulation

**********************************************************************/

#include "emu.h"
#include "upc82c710.h"

#define LOG_CFG (1U << 1)
#define LOG_FDC (1U << 2)
#define LOG_IDE (1U << 3)
#define LOG_LPT (1U << 4)
#define LOG_SER (1U << 5)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(UPC82C710, upc82c710_device, "upc82c710", "Universal Peripheral Controller 82C710")


upc82c710_device::upc82c710_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPC82C710, tag, owner, clock)
	, m_ide(*this, "ide")
	, m_fdc(*this, "fdc")
	, m_lpt(*this, "parallel")
	, m_serial(*this, "serial")
	, m_fintr_callback(*this)
	, m_fdrq_callback(*this)
	, m_pintr_callback(*this)
	, m_sintr_callback(*this)
	, m_txd_callback(*this)
	, m_dtr_callback(*this)
	, m_rts_callback(*this)
	, m_cfg_mode(0)
{
}


void upc82c710_device::device_add_mconfig(machine_config &config)
{
	// ide interface
	ATA_INTERFACE(config, m_ide);

	// floppy disc controller
	UPD765A(config, m_fdc, clock() / 3, false, false); // uPD72065B
	m_fdc->intrq_wr_callback().set(FUNC(upc82c710_device::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(upc82c710_device::fdc_drq_w));

	// parallel port
	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set([this](int state) { m_pintr_callback(state); });

	// serial port
	NS16450(config, m_serial, clock() / 13);
	m_serial->out_int_callback().set([this](int state) { m_sintr_callback(state); });
	m_serial->out_tx_callback().set([this](int state) { m_txd_callback(state); });
	m_serial->out_dtr_callback().set([this](int state) { m_dtr_callback(state); });
	m_serial->out_rts_callback().set([this](int state) { m_rts_callback(state); });
}


void upc82c710_device::device_start()
{
	for (int i=0; i<4; i++)
	{
		char name[2] = {static_cast<char>('0'+i), 0};
		floppy_connector *conn = m_fdc->subdevice<floppy_connector>(name);
		floppy[i] = conn ? conn->get_device() : nullptr;
	}

	irq = drq = false;
	fdc_irq = fdc_drq = false;

	// default addresses
	device_address[DEVICE_CFG] = 0x390;
	device_address[DEVICE_IDE] = 0x1f0;
	device_address[DEVICE_FDC] = 0x3f0;
	device_address[DEVICE_LPT] = 0x278;
	device_address[DEVICE_SER] = 0x3f8;

	const u8 cfg_regs_defaults[] = { 0x0e, 0x00, 0x08, 0x00, 0xfe, 0xbe, 0x9e, 0x00, 0xec, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

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

void upc82c710_device::device_reset()
{
	dor_w(0x00);
}


u16 upc82c710_device::io_r(offs_t offset, u16 mem_mask)
{
	u16 data = 0xffff;

	// configuration
	if (offset == 0x391 && m_cfg_mode == 5)
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

	// serial
	if (device_enabled[DEVICE_SER] && (offset & ~7) == device_address[DEVICE_SER])
	{
		data = m_serial->ins8250_r(offset & 7);
		LOGMASKED(LOG_SER, "SER read %04x -> %02x\n", offset, data);
	}

	return data;
}

void upc82c710_device::io_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (m_cfg_mode)
	{
	case 0:
		if (offset == 0x2fa && data == 0x55)
		{
			m_cfg_mode++;
			LOGMASKED(LOG_CFG, "CFG step %d write %04x <- %02x\n", m_cfg_mode, offset, data);
		}
		break;
	case 1:
		if (offset == 0x3fa && data == 0xaa)
		{
			m_cfg_mode++;
			LOGMASKED(LOG_CFG, "CFG step %d write %04x <- %02x\n", m_cfg_mode, offset, data);
		}
		break;
	case 2:
		if (offset == 0x3fa && data == 0x36)
		{
			m_cfg_mode++;
			LOGMASKED(LOG_CFG, "CFG step %d write %04x <- %02x\n", m_cfg_mode, offset, data);
		}
		break;
	case 3:
		if (offset == 0x3fa)
		{
			m_cfg_mode++;
			LOGMASKED(LOG_CFG, "CFG step %d write %04x <- %02x\n", m_cfg_mode, offset, data);
			m_cfg_regs[0x0f] = data;
			device_address[DEVICE_CFG] = data << 2;
			LOGMASKED(LOG_CFG, "CFG address %04x\n", device_address[DEVICE_CFG]);
		}
		break;
	case 4:
		// enter configuration
		if (offset == 0x2fa && data == (~m_cfg_regs[0x0f] & 0xff))
		{
			m_cfg_mode++;
			LOGMASKED(LOG_CFG, "CFG step %d write %04x <- %02x\n", m_cfg_mode, offset, data);
			LOGMASKED(LOG_CFG, "CFG -- mode on --\n");
		}
		break;
	}

	// configuration
	if (offset == device_address[DEVICE_CFG])
	{
		LOGMASKED(LOG_CFG, "CFG write %04x <- %02x\n", offset, data);
		if (m_cfg_mode == 5)
		{
			m_cfg_indx = data & 0x0f;
		}
		else
		{
			m_cfg_mode = 0;
		}
	}
	else if (offset == device_address[DEVICE_CFG] + 1)
	{
		LOGMASKED(LOG_CFG, "CFG write %04x <- %02x\n", offset, data);
		if (m_cfg_mode == 5)
		{
			write_cfg(m_cfg_indx, data);
		}
		else
		{
			m_cfg_mode = 0;
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

	// serial
	if (device_enabled[DEVICE_SER] && (offset & ~7) == device_address[DEVICE_SER])
	{
		m_serial->ins8250_w(offset & 7, data);
		LOGMASKED(LOG_SER, "SER write %04x <- %02x\n", offset, data);
	}
}


u8 upc82c710_device::dack_r()
{
	return m_fdc->dma_r();
}

void upc82c710_device::dack_w(u8 data)
{
	m_fdc->dma_w(data);
}

void upc82c710_device::tc_w(bool state)
{
	m_fdc->tc_w(state);
}


void upc82c710_device::dor_w(uint8_t data)
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

void upc82c710_device::fdc_irq_w(int state)
{
	fdc_irq = state;
	check_irq();
}

void upc82c710_device::fdc_drq_w(int state)
{
	fdc_drq = state;
	check_drq();
}

void upc82c710_device::check_irq()
{
	bool pirq = irq;
	irq = fdc_irq && (dor & 4) && (dor & 8);
	if (irq != pirq)
		m_fintr_callback(irq);
}

void upc82c710_device::check_drq()
{
	bool pdrq = drq;
	drq = fdc_drq && (dor & 4) && (dor & 8);
	if (drq != pdrq)
		m_fdrq_callback(drq);
}


void upc82c710_device::write_cfg(int index, u8 data)
{
	m_cfg_regs[index] = data;
	LOGMASKED(LOG_CFG, "CR[%02x] = %02x\n", index, data);

	switch (index)
	{
	case 0x00: // Chip Selects and Enable
		device_enabled[DEVICE_LPT]= BIT(m_cfg_regs[index], 3);
		device_enabled[DEVICE_SER]= BIT(m_cfg_regs[index], 2);
		LOGMASKED(LOG_CFG, "LPT %s\n", device_enabled[DEVICE_LPT] ? "enabled" : "disabled");
		LOGMASKED(LOG_CFG, "SER %s\n", device_enabled[DEVICE_SER] ? "enabled" : "disabled");
		break;

	case 0x04: // Serial Port Address
		device_address[DEVICE_SER] = (data & 0xfe) << 2;
		LOGMASKED(LOG_CFG, "SER address %04x\n", device_address[DEVICE_SER]);
		break;

	case 0x06: // Parallel Port Address
		device_address[DEVICE_LPT] = data << 2;
		LOGMASKED(LOG_CFG, "LPT address %04x\n", device_address[DEVICE_LPT]);
		break;

	case 0x0c:
		device_enabled[DEVICE_IDE] = BIT(m_cfg_regs[index], 7);
		device_enabled[DEVICE_FDC] = BIT(m_cfg_regs[index], 5);
		LOGMASKED(LOG_CFG, "IDE %s\n", device_enabled[DEVICE_IDE] ? "enabled" : "disabled");
		LOGMASKED(LOG_CFG, "FDC %s\n", device_enabled[DEVICE_FDC] ? "enabled" : "disabled");
		break;

	case 0x0f:
		m_cfg_mode = 0;
		LOGMASKED(LOG_CFG, "CFG -- mode off --\n");
		break;
	}
}


void upc82c710_device::rxd_w(int state)
{
	m_serial->rx_w(state);
}

void upc82c710_device::dcd_w(int state)
{
	m_serial->dcd_w(state);
}

void upc82c710_device::dsr_w(int state)
{
	m_serial->dsr_w(state);
}

void upc82c710_device::ri_w(int state)
{
	m_serial->ri_w(state);
}

void upc82c710_device::cts_w(int state)
{
	m_serial->cts_w(state);
}
