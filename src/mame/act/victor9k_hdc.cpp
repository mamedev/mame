// license:BSD-3-Clause
// copyright-holders:D. Donohoe

#include "emu.h"
#include "victor9k_hdc.h"

#include "logmacro.h"

/*
 * Emulation of the Victor 9000 DMA Interface Board, which interfaces to the
 * Xebec S1410 Hard Disk controller.
 *
 * Source: https://bitsavers.org/pdf/victor/victor9000/Victor_9000_Sirius_1_Hard_Disk_Subsystem.pdf
 *
 * Compared to the typical SCSI/SASI controller device, the DMA Interface Board
 * acts more like a passthrough, and the usual state machine is primarily
 * implemented on the main CPU.
 */

DEFINE_DEVICE_TYPE(VICTOR_9000_HDC, victor_9000_hdc_device, "victor_9000_hdc", "Victor 9000 Hard Disk Controller")

victor_9000_hdc_device::victor_9000_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	victor_9000_hdc_device(mconfig, VICTOR_9000_HDC, tag, owner, clock)
{
}

victor_9000_hdc_device::victor_9000_hdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_device(mconfig, type, tag, owner, clock),
	nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF),
	m_dma_r(*this, 0xff),
	m_dma_w(*this),
	m_irq_handler(*this),
	m_dma_on(false),
	m_dma_write(false),
	m_dma_addr(0),
	m_asserting_ack(false),
	m_non_dma_req(false),
	m_ctrl(0),
	m_data(0),
	m_bus_ctrl(0),
	m_irq_state(false)
{
}

uint8_t victor_9000_hdc_device::read(offs_t offset)
{
	// Lower address bits are ignored
	if (offset >= 0x80)
	{
		offset &= ~0x1f;
	}
	else
	{
		offset &= ~0xf;
	}

	uint8_t data = 0xff;

	switch (offset)
	{
		case R_CONTROL:
			// Write only
			break;
		case R_DATA:
			if (m_non_dma_req && !machine().side_effects_disabled())
			{
				m_non_dma_req = false;
				scsi_bus->data_w(scsi_refid, 0);
				m_data = scsi_bus->data_r();
				m_asserting_ack = true;
				scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
			}
			data = m_data;
			break;
		case R_STATUS:
			if (!machine().side_effects_disabled())
			{
				// Reading status register clears interrupt
				update_ints(false);
			}

			data = ((m_bus_ctrl & S_INP) ? R_S_INP : 0) |
				   ((m_bus_ctrl & S_CTL) ? R_S_CMD : 0) |
				   ((m_bus_ctrl & S_BSY) ? R_S_BSY : 0) |
				   ((m_bus_ctrl & S_REQ) ? R_S_REQ : 0) |
				   ((m_bus_ctrl & S_MSG) ? R_S_MSG : 0);
			break;
		case R_ADDR_L:
			data = m_dma_addr & 0xff;
			break;
		case R_ADDR_M:
			data = (m_dma_addr >> 8) & 0xff;
			break;
		case R_ADDR_H:
			// Full DMA address is 20 bits
			data = (m_dma_addr >> 16) & 0x0f;
			break;
		default:
			logerror("Read from unknown register offset (0x%x)\n", offset);
			break;
	}

	return data;
}

void victor_9000_hdc_device::write(offs_t offset, uint8_t data)
{
	// Lower address bits are ignored
	if (offset >= 0x80)
	{
		offset &= ~0x1f;
	}
	else
	{
		offset &= ~0xf;
	}

	switch (offset)
	{
		case R_CONTROL:
			m_ctrl = data;
			if (data & R_C_RESET)
			{
				device_reset();
			}
			else
			{
				if (data & R_C_DMAON_LATCH)
				{
					m_dma_on = (data & R_C_DMAON_VALUE);
				}

				m_dma_write = (data & R_C_WMODE);

				uint32_t sel = ((data & R_C_SELECT) ? S_SEL : 0);
				scsi_bus->ctrl_w(scsi_refid, sel, S_SEL);
			}
			break;
		case R_DATA:
			m_data = data;

			if (m_ctrl & R_C_SELECT)
			{
				// Later versions of HDSETUP.EXE don't always clear the select
				// bit before asserting a new target ID on the data bus during
				// selection.  This is in violation of the SCSI spec (the data
				// lines are supposed to be asserted first, then SEL should be
				// asserted after a delay).  nscsi_bus doesn't react if the
				// data lines change without a change in the SEL control line.
				// Workaround is to ensure SEL isn't asserted whenever the data
				// bus is written.
				scsi_bus->ctrl_w(scsi_refid, 0, S_SEL);
				m_ctrl &= ~R_C_SELECT;
			}

			scsi_bus->data_w(scsi_refid, data);
			if (m_non_dma_req)
			{
				m_non_dma_req = false;
				m_asserting_ack = true;
				scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
			}
			break;
		case R_STATUS:
			// Read only
			break;
		case R_ADDR_L:
			m_dma_addr = (m_dma_addr & ~0xff) | data;
			break;
		case R_ADDR_M:
			m_dma_addr = (m_dma_addr & ~0xff00) | data << 8;
			break;
		case R_ADDR_H:
			m_dma_addr = (m_dma_addr & ~0x0f0000) | (data & 0x0f) << 16;
			break;
		default:
			logerror("Write to unknown register offset (0x%x)\n", offset);
			break;
	}
}

void victor_9000_hdc_device::device_reset()
{
	m_dma_on = false;
	m_dma_write = false;
	m_dma_addr = 0;
	m_asserting_ack = false;
	m_non_dma_req = false;
	m_data = 0;
	m_bus_ctrl = 0;

	scsi_bus->ctrl_wait(scsi_refid, S_SEL|S_BSY|S_REQ|S_RST, S_ALL);
	scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
	scsi_bus->data_w(scsi_refid, 0);

	update_ints(false);
	m_irq_handler(false);
}

void victor_9000_hdc_device::device_start()
{
	m_ctrl_timer = timer_alloc(FUNC(victor_9000_hdc_device::ctrl_change_handler), this);

	save_item(NAME(m_dma_on));
	save_item(NAME(m_dma_write));
	save_item(NAME(m_dma_addr));
	save_item(NAME(m_asserting_ack));
	save_item(NAME(m_non_dma_req));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_data));
	save_item(NAME(m_bus_ctrl));
	save_item(NAME(m_irq_state));
}

TIMER_CALLBACK_MEMBER(victor_9000_hdc_device::ctrl_change_handler)
{
	m_ctrl_timer->reset();

	if (m_bus_ctrl & S_REQ)
	{
		// Only data (not control or status) transfers can use DMA
		if (m_dma_on && !(m_bus_ctrl & S_CTL))
		{
			if (m_dma_write)
			{
				// Write to system memory
				scsi_bus->data_w(scsi_refid, 0);
				m_data = scsi_bus->data_r();
				m_dma_w(m_dma_addr, m_data);
			}
			else
			{
				// Read from system memory
				m_data = m_dma_r(m_dma_addr);
				scsi_bus->data_w(scsi_refid, m_data);
			}
			m_dma_addr++;

			m_asserting_ack = true;
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		}
		else
		{
			// ACK will be generated upon direct data register access
			m_non_dma_req = true;
		}
	}
	else if (m_asserting_ack)
	{
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		m_asserting_ack = false;
	}

	if ((m_bus_ctrl & (S_REQ|S_CTL)) == (S_REQ|S_CTL))
	{
		// Command or status request from controller generates interrupt
		update_ints(true);
	}
}

void victor_9000_hdc_device::scsi_ctrl_changed()
{
	m_bus_ctrl = scsi_bus->ctrl_r();
	attotime delay;

	if (m_bus_ctrl & S_REQ)
	{
		if (m_dma_on && !(m_bus_ctrl & S_CTL))
		{
			// How long a DMA transaction takes to complete depends on what
			// the 8088 is doing.  The right thing to do here is to assert
			// HOLD, then wait for HLDA, but the 8088 emulation doesn't
			// seem to implement these signals currently.
			delay = clocks_to_attotime(2);
		}
		else
		{
			delay = clocks_to_attotime(0);
		}
	}
	else if (m_asserting_ack)
	{
		delay = clocks_to_attotime(1);
	}
	else
	{
		delay = attotime::never;
	}

	m_ctrl_timer->adjust(delay);
}

void victor_9000_hdc_device::update_ints(bool irq_state)
{
	if (irq_state != m_irq_state)
	{
		m_irq_state = irq_state;
		m_irq_handler(m_irq_state);
	}
}
