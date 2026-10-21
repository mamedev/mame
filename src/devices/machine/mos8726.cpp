// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8726R1 DMA Controller emulation

**********************************************************************/

#include "emu.h"
#include "mos8726.h"

#define LOG_REGS    (1U << 1)
#define LOG_DMA     (1U << 2)

//#define VERBOSE (LOG_REGS | LOG_DMA)
#include "logmacro.h"

#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGDMA(...)  LOGMASKED(LOG_DMA, __VA_ARGS__)



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

namespace {

enum
{
	REG_STATUS = 0,
	REG_COMMAND,
	REG_C64_ADDR_LO,
	REG_C64_ADDR_HI,
	REG_REU_ADDR_LO,
	REG_REU_ADDR_HI,
	REG_REU_BANK,
	REG_LENGTH_LO,
	REG_LENGTH_HI,
	REG_IRQ_MASK,
	REG_ADDR_CTRL
};

enum
{
	STATUS_IRQ      = 0x80,
	STATUS_EOB      = 0x40,
	STATUS_FAULT    = 0x20,
	STATUS_SIZE     = 0x10
};

enum
{
	CMD_EXECUTE     = 0x80,
	CMD_AUTOLOAD    = 0x20,
	CMD_FF00        = 0x10,
	CMD_TYPE_MASK   = 0x03
};

enum
{
	TYPE_STASH = 0,
	TYPE_FETCH,
	TYPE_SWAP,
	TYPE_VERIFY
};

enum
{
	IMR_ENABLE      = 0x80,
	IMR_EOB         = 0x40,
	IMR_FAULT       = 0x20
};

enum
{
	ACR_FIX_C64     = 0x80,
	ACR_FIX_REU     = 0x40
};

} // anonymous namespace



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MOS8726, mos8726_device, "mos8726", "MOS 8726 DMA Controller")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8726_device - constructor
//-------------------------------------------------

mos8726_device::mos8726_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS8726, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_write_irq(*this)
	, m_write_dma(*this)
	, m_read_c64(*this, 0xff)
	, m_write_c64(*this)
	, m_read_reu(*this, 0xff)
	, m_write_reu(*this)
	, m_icount(0)
	, m_bs(1)
	, m_ba(true)
	, m_status(0)
	, m_command(0)
	, m_c64_addr(0)
	, m_c64_addr_shadow(0)
	, m_reu_addr(0)
	, m_reu_addr_shadow(0)
	, m_length(0)
	, m_length_shadow(0)
	, m_imr(0)
	, m_acr(0)
	, m_active(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos8726_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	// save state
	save_item(NAME(m_bs));
	save_item(NAME(m_ba));
	save_item(NAME(m_status));
	save_item(NAME(m_command));
	save_item(NAME(m_c64_addr));
	save_item(NAME(m_c64_addr_shadow));
	save_item(NAME(m_reu_addr));
	save_item(NAME(m_reu_addr_shadow));
	save_item(NAME(m_length));
	save_item(NAME(m_length_shadow));
	save_item(NAME(m_imr));
	save_item(NAME(m_acr));
	save_item(NAME(m_active));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos8726_device::device_reset()
{
	m_status = 0;
	m_command = CMD_FF00;
	m_c64_addr = m_c64_addr_shadow = 0;
	m_reu_addr = m_reu_addr_shadow = 0;
	m_length = m_length_shadow = 0xffff;
	m_imr = 0;
	m_acr = 0;
	m_active = false;

	m_write_irq(0);
	m_write_dma(0);

	suspend(SUSPEND_REASON_DISABLE, true);
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mos8726_device::execute_run()
{
	while (m_icount > 0)
	{
		if (!m_active)
		{
			m_icount = 0;
			break;
		}

		if (!m_ba)
		{
			m_icount--;
			continue;
		}

		transfer_byte();
	}
}


//-------------------------------------------------
//  reu_r - read expansion RAM
//-------------------------------------------------

uint8_t mos8726_device::reu_r(offs_t offset)
{
	return m_read_reu(offset & (m_bs ? 0x7ffff : 0x1ffff));
}


//-------------------------------------------------
//  reu_w - write expansion RAM
//-------------------------------------------------

void mos8726_device::reu_w(offs_t offset, uint8_t data)
{
	m_write_reu(offset & (m_bs ? 0x7ffff : 0x1ffff), data);
}


//-------------------------------------------------
//  transfer_byte -
//-------------------------------------------------

void mos8726_device::transfer_byte()
{
	bool fault = false;

	switch (m_command & CMD_TYPE_MASK)
	{
	case TYPE_STASH:
		reu_w(m_reu_addr, m_read_c64(m_c64_addr));
		m_icount--;
		break;

	case TYPE_FETCH:
		m_write_c64(m_c64_addr, reu_r(m_reu_addr));
		m_icount--;
		break;

	case TYPE_SWAP:
	{
		uint8_t const c64_data = m_read_c64(m_c64_addr);
		m_write_c64(m_c64_addr, reu_r(m_reu_addr));
		reu_w(m_reu_addr, c64_data);
		m_icount -= 2;
		break;
	}

	case TYPE_VERIFY:
		fault = m_read_c64(m_c64_addr) != reu_r(m_reu_addr);
		m_icount--;
		break;
	}

	if (!(m_acr & ACR_FIX_C64))
		m_c64_addr++;

	if (!(m_acr & ACR_FIX_REU))
	{
		m_reu_addr = (m_reu_addr + 1) & 0x7ffff;
		if (!m_bs && (m_reu_addr == 0x20000))
			m_reu_addr = 0;
	}

	if (m_length == 1)
	{
		m_status |= STATUS_EOB;
		if (fault)
			m_status |= STATUS_FAULT;
		end_transfer();
	}
	else
	{
		m_length--;
		if (fault)
		{
			m_status |= STATUS_FAULT;
			end_transfer();
		}
	}
}


//-------------------------------------------------
//  start_transfer -
//-------------------------------------------------

void mos8726_device::start_transfer()
{
	LOGDMA("%s: DMA %u C64 %04x REU %05x length %04x ACR %02x\n", machine().describe_context(), m_command & CMD_TYPE_MASK, m_c64_addr, m_reu_addr, m_length, m_acr);

	m_command = (m_command & ~CMD_EXECUTE) | CMD_FF00;
	m_active = true;

	m_write_dma(1);
	resume(SUSPEND_REASON_DISABLE);
}


//-------------------------------------------------
//  end_transfer -
//-------------------------------------------------

void mos8726_device::end_transfer()
{
	LOGDMA("DMA end status %02x C64 %04x REU %05x length %04x\n", m_status, m_c64_addr, m_reu_addr, m_length);

	if ((m_command & CMD_TYPE_MASK) == TYPE_FETCH)
		reu_r(m_reu_addr);

	if (m_command & CMD_AUTOLOAD)
	{
		m_c64_addr = m_c64_addr_shadow;
		m_reu_addr = m_reu_addr_shadow;
		m_length = m_length_shadow;
	}

	m_active = false;
	m_icount = 0;

	m_write_dma(0);
	suspend(SUSPEND_REASON_DISABLE, true);

	update_irq();
}


//-------------------------------------------------
//  update_irq -
//-------------------------------------------------

void mos8726_device::update_irq()
{
	if (!(m_status & STATUS_IRQ) && (m_imr & IMR_ENABLE) && (m_imr & m_status & (IMR_EOB | IMR_FAULT)))
	{
		m_status |= STATUS_IRQ;
		m_write_irq(1);
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos8726_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0x1f)
	{
	case REG_STATUS:
		data = m_status | (m_bs ? STATUS_SIZE : 0);

		if (!machine().side_effects_disabled())
		{
			bool const irq = m_status & STATUS_IRQ;
			m_status = 0;
			if (irq)
				m_write_irq(0);
		}
		break;

	case REG_COMMAND:
		data = m_command;
		break;

	case REG_C64_ADDR_LO:
		data = m_c64_addr & 0xff;
		break;

	case REG_C64_ADDR_HI:
		data = m_c64_addr >> 8;
		break;

	case REG_REU_ADDR_LO:
		data = m_reu_addr & 0xff;
		break;

	case REG_REU_ADDR_HI:
		data = (m_reu_addr >> 8) & 0xff;
		break;

	case REG_REU_BANK:
		data = 0xf8 | (m_reu_addr >> 16);
		break;

	case REG_LENGTH_LO:
		data = m_length & 0xff;
		break;

	case REG_LENGTH_HI:
		data = m_length >> 8;
		break;

	case REG_IRQ_MASK:
		data = m_imr | 0x1f;
		break;

	case REG_ADDR_CTRL:
		data = m_acr | 0x3f;
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos8726_device::write(offs_t offset, uint8_t data)
{
	LOGREGS("%s: write %02x = %02x\n", machine().describe_context(), offset & 0x1f, data);

	switch (offset & 0x1f)
	{
	case REG_COMMAND:
		m_command = data;

		if ((m_command & (CMD_EXECUTE | CMD_FF00)) == (CMD_EXECUTE | CMD_FF00))
			start_transfer();
		break;

	case REG_C64_ADDR_LO:
		m_c64_addr = m_c64_addr_shadow = (m_c64_addr_shadow & 0xff00) | data;
		break;

	case REG_C64_ADDR_HI:
		m_c64_addr = m_c64_addr_shadow = (m_c64_addr_shadow & 0x00ff) | (data << 8);
		break;

	case REG_REU_ADDR_LO:
		m_reu_addr = m_reu_addr_shadow = (m_reu_addr_shadow & 0x7ff00) | data;
		break;

	case REG_REU_ADDR_HI:
		m_reu_addr = m_reu_addr_shadow = (m_reu_addr_shadow & 0x700ff) | (data << 8);
		break;

	case REG_REU_BANK:
		m_reu_addr = m_reu_addr_shadow = (m_reu_addr_shadow & 0x0ffff) | ((data & 0x07) << 16);
		break;

	case REG_LENGTH_LO:
		m_length = m_length_shadow = (m_length_shadow & 0xff00) | data;
		break;

	case REG_LENGTH_HI:
		m_length = m_length_shadow = (m_length_shadow & 0x00ff) | (data << 8);
		break;

	case REG_IRQ_MASK:
		m_imr = data & 0xe0;
		break;

	case REG_ADDR_CTRL:
		m_acr = data & 0xc0;
		break;
	}
}


//-------------------------------------------------
//  ff00_w - CPU write to $FF00
//-------------------------------------------------

void mos8726_device::ff00_w()
{
	if (!m_active && (m_command & (CMD_EXECUTE | CMD_FF00)) == CMD_EXECUTE)
		start_transfer();
}


//-------------------------------------------------
//  bs_w - bank select write
//-------------------------------------------------

void mos8726_device::bs_w(int state)
{
	m_bs = state;
}


//-------------------------------------------------
//  romsel_r - ROM select read
//-------------------------------------------------

int mos8726_device::romsel_r(int roml, int romh)
{
	return roml && romh;
}
