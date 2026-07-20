// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple SCSI DMA ASIC (343S0064)
    Emulation by R. Belmont

    This ASIC consists of an NCR 53C80 standard cell plus custom Apple
    circuitry to do hardware handshaking on blind MOVE.L (A0), (A2)+
    style transfers.  True DMA to the full 32-bit address space is also
    available, along with automated SCSI bus arbitration.

    No shipped version of MacOS supports the full DMA mode.  But A/UX does.
*/

#include "emu.h"

#include "scsidma.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"

#include "endianness.h"

#define LOG_IRQ         (1U << 1)
#define LOG_DRQ         (1U << 2)
#define LOG_HANDSHAKE   (1U << 3)

#define VERBOSE (0)
#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

static constexpr u16 CTRL_DMAEN     = 0x0001;
static constexpr u16 CTRL_IRQEN     = 0x0002;
static constexpr u16 CTRL_HNDSHK    = 0x0008;
static constexpr u16 CTRL_SCIRQEN   = 0x0040;
static constexpr u16 CTRL_WDIRQ     = 0x0080;
static constexpr u16 CTRL_DMABERR   = 0x0100;
static constexpr u16 CTRL_ARBEN     = 0x1000;
static constexpr u16 CTRL_WONARB    = 0x2000;

DEFINE_DEVICE_TYPE(SCSIDMA, scsidma_device, "scsidma", "Apple SCSIDMA ASIC")

void scsidma_device::map(address_map &map)
{
	map(0x0000, 0x007f).rw(FUNC(scsidma_device::scsi_r), FUNC(scsidma_device::scsi_w));
	map(0x0000, 0x0003).rw(FUNC(scsidma_device::handshake_data_r), FUNC(scsidma_device::handshake_w));
	map(0x0060, 0x0063).r(FUNC(scsidma_device::handshake_r));
	map(0x0080, 0x01ff).rw(FUNC(scsidma_device::control_r), FUNC(scsidma_device::control_w));
}

void scsidma_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config([](device_t *device)
	{
		device->subdevice<cdda_device>("cdda")->add_route(0, "^^^speaker", 1.0, 0);
		device->subdevice<cdda_device>("cdda")->add_route(1, "^^^speaker", 1.0, 1);
	});
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, nullptr);

	NCR53C80(config, m_ncr);
	m_scsibus->set_external_device(7, m_ncr);
	m_ncr->irq_handler().set(DEVICE_SELF, FUNC(scsidma_device::scsi_irq_w));
	m_ncr->drq_handler().set(DEVICE_SELF, FUNC(scsidma_device::scsi_drq_w));
}

scsidma_device::scsidma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SCSIDMA, tag, owner, clock),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_scsibus(*this, "scsi"),
	m_ncr(*this, "ncr53c80"),
	m_irq(*this),
	m_scsi_irq(0),
	m_control(0),
	m_holding(0),
	m_dma_address(0),
	m_dma_count(0),
	m_holding_remaining(0),
	m_is_write(false),
	m_drq_completed(false),
	m_dma_direction(false)
{
}

void scsidma_device::device_start()
{
	m_maincpu->set_emmu_enable(true);

	save_item(NAME(m_control));
	save_item(NAME(m_holding));
	save_item(NAME(m_holding_remaining));
	save_item(NAME(m_is_write));
	save_item(NAME(m_drq_completed));
	save_item(NAME(m_dma_address));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_dma_direction));
}

void scsidma_device::device_reset()
{
	m_control = 0;
	m_holding = 0;
	m_holding_remaining = 0;
	m_drq_completed = false;
	m_dma_address = 0;
	m_dma_count = 0;
	m_dma_direction = false;
}

u32 scsidma_device::control_r(offs_t offset)
{
	switch (offset)
	{
		case (0x80 - 0x80) >> 2:
			return m_control;

		case (0xc0 - 0x80) >> 2:
			return m_dma_count;

		case (0x100 - 0x80) >> 2:
			return m_dma_address;
	}

	return 0;
}

void scsidma_device::control_w(offs_t offset, u32 data)
{
	LOGMASKED(LOG_GENERAL, "%s: control_w %08x @ %x (PC=%08x)\n", tag(), data, (offset << 2) + 0x80, m_maincpu->pc());

	switch (offset)
	{
		case (0x80 - 0x80) >> 2:
			// check for unemulated features
			if (data & CTRL_ARBEN)
			{
				fatalerror("%s: Auto-arbitration enabled, but not supported!\n", tag());
			}

			m_control &= CTRL_SCIRQEN | CTRL_WDIRQ | CTRL_DMABERR | CTRL_WONARB;
			m_control |= data & ~(CTRL_SCIRQEN | CTRL_WDIRQ | CTRL_DMABERR | CTRL_WONARB);
			break;

		case (0xc0 - 0x80) >> 2:
			m_dma_count = data;
			LOGMASKED(LOG_GENERAL, "%s: DMA count set to %08x\n", tag(), m_dma_count);
			break;

		case (0x100 - 0x80) >> 2:
			m_dma_address = data;
			LOGMASKED(LOG_GENERAL, "%s: DMA address set to %08x\n", tag(), m_dma_address);
			break;
	}
}

u8 scsidma_device::scsi_r(offs_t offset)
{
	return m_ncr->read(offset>>4);
}

void scsidma_device::scsi_w(offs_t offset, u8 data)
{
	const auto ncr_reg = offset >> 4;

	if (ncr_reg == 5)
	{
		m_dma_direction = false;
	}
	else if (ncr_reg == 7)
	{
		m_dma_direction = true;
	}

	m_ncr->write(ncr_reg, data);
}

u32 scsidma_device::handshake_r(offs_t offset, u32 mem_mask)
{
	// if the DRQ handler completed this transfer while we were out, just return the result now
	if (m_drq_completed)
	{
		LOGMASKED(LOG_HANDSHAKE, "%s: Completed read in DRQ, returning\n", tag());
		m_drq_completed = false;
		return m_holding;
	}

	if (mem_mask == 0xff000000)
	{
		if (m_control & CTRL_HNDSHK)
		{
			if (m_drq)
			{
				return m_ncr->dma_r() << 24;
			}
			else
			{
				LOGMASKED(LOG_HANDSHAKE, "Handshaking single byte, no DRQ\n");
				m_maincpu->restart_this_instruction();
				m_maincpu->suspend_until_trigger(1, true);
				return 0xffffffff;
			}
		}
		else
		{
			LOGMASKED(LOG_HANDSHAKE, "Non-handshake single byte\n");
			if (m_drq)
			{
				return m_ncr->dma_r() << 24;
			}
			else
			{
				return m_ncr->read(6) << 24;
			}
		}
	}
	else if ((mem_mask == 0xffffffff) || (mem_mask == 0xffff0000))
	{
		// are we here from a restart?
		if (!m_holding_remaining)
		{
			m_holding = 0;
			if (mem_mask == 0xffffffff)
			{
				m_holding_remaining = 4;
			}
			else if (mem_mask == 0xffff0000)
			{
				m_holding_remaining = 2;
			}
			m_is_write = false;
		}

		// is a new byte available?
		if (m_drq && m_holding_remaining)
		{
			m_holding <<= 8;
			m_holding |= m_ncr->dma_r();
			m_holding_remaining--;
			LOGMASKED(LOG_HANDSHAKE, "Holding %08x, remain %d\n", m_holding, m_holding_remaining);
		}

		if (!m_holding_remaining)
		{
			return m_holding;
		}

		LOGMASKED(LOG_HANDSHAKE, "Handshaking %d byte read\n", m_holding_remaining);
		m_maincpu->restart_this_instruction();
		m_maincpu->suspend_until_trigger(1, true);
		return 0xffffffff;
	}
	fatalerror("%s: Unhandled handshake read mask %08x\n", tag(), mem_mask);
	return 0xffffffff;
}

u32 scsidma_device::handshake_data_r(offs_t offset, u32 mem_mask)
{
	if (!m_drq_completed && (mem_mask == 0xff000000) && !(m_control & CTRL_HNDSHK) && !m_drq)
	{
		return u32(m_ncr->read(0)) << 24;
	}

	return handshake_r(offset, mem_mask);
}

void scsidma_device::handshake_w(offs_t offset, u32 data, u32 mem_mask)
{
	// if the DRQ handler completed this transfer while we were out, we're done
	if (m_drq_completed)
	{
		LOGMASKED(LOG_HANDSHAKE, "%s: Completed write in DRQ, returning\n", tag());
		m_drq_completed = false;
		return;
	}

	if (mem_mask == 0xff000000)
	{
		if (m_control & CTRL_HNDSHK)
		{
			if (m_drq)
			{
				m_ncr->dma_w(data >> 24);
				return;
			}
			else
			{
				LOGMASKED(LOG_HANDSHAKE, "Handshake single byte write\n");
				m_maincpu->restart_this_instruction();
				m_maincpu->suspend_until_trigger(1, true);
				return;
			}
		}
		else
		{
			LOGMASKED(LOG_HANDSHAKE, "Non-handshake single byte write, DRQ %d\n", m_drq);
			if (m_drq)
			{
				m_ncr->dma_w(data >> 24);
				return;
			}
			else
			{
				m_ncr->write(0, data >> 24);
				return;
			}
		}
	}
	else if ((mem_mask == 0xffffffff) || (mem_mask == 0xffff0000))
	{
		// are we here from a restart?
		if (!m_holding_remaining)
		{
			m_holding = data;
			if (mem_mask == 0xffffffff)
			{
				m_holding_remaining = 4;
			}
			else if (mem_mask == 0xffff0000)
			{
				m_holding_remaining = 2;
			}
			m_is_write = true;
		}

		// is a new byte available?
		while (m_drq && m_holding_remaining)
		{
			m_ncr->dma_w(m_holding >> 24);
			m_holding <<= 8;
			m_holding_remaining--;
			LOGMASKED(LOG_HANDSHAKE, "Holding write %08x, remain %d\n", m_holding, m_holding_remaining);
		}

		if (!m_holding_remaining)
		{
			return;
		}

		LOGMASKED(LOG_HANDSHAKE, "Handshaking %d byte write %08x\n", m_holding_remaining, m_holding);
		m_maincpu->restart_this_instruction();
		m_maincpu->suspend_until_trigger(1, true);
		return;
	}
	fatalerror("%s: Unhandled handshake write mask %08x\n", tag(), mem_mask);
}

void scsidma_device::scsi_irq_w(int state)
{
	LOGMASKED(LOG_IRQ, "%s: 53C80 IRQ: %d (was %d)\n", tag(), state, m_scsi_irq);
	m_scsi_irq = state;
	m_control &= ~CTRL_SCIRQEN;
	m_control |= state ? CTRL_SCIRQEN : 0;
	if (m_control & CTRL_IRQEN)
	{
		m_irq(state);
	}
}

void scsidma_device::scsi_drq_w(int state)
{
	LOGMASKED(LOG_DRQ, "%s: 53C80 DRQ %d (was %d) (remain %d write %d)\n", tag(), state, m_drq, m_holding_remaining, m_is_write);

	if ((state) && (m_control & CTRL_DMAEN) && m_dma_count)
	{
		if (m_dma_direction)
		{
			const u8 data = m_ncr->dma_r();
			m_ram_base[BYTE4_XOR_BE(m_dma_address)] = data;
			LOGMASKED(LOG_DRQ, "%s: DMA read byte %02x to address %08x\n", tag(), data, m_dma_address);
			m_dma_address++;
			m_dma_count--;
		}
		else
		{
			const u8 data = m_ram_base[BYTE4_XOR_BE(m_dma_address)];
			LOGMASKED(LOG_DRQ, "%s: DMA write byte %02x from address %08x\n", tag(), data, m_dma_address);
			m_ncr->dma_w(data);
			m_dma_address++;
			m_dma_count--;
		}

		if (m_dma_count == 0)
		{
			LOGMASKED(LOG_DRQ, "DMA completed\n");

			// we're done, now tell the 53C80 that.
			m_ncr->eop_w(1);
			m_ncr->eop_w(0);
		}

		return;
	}

	if ((state) && (m_holding_remaining > 0) && (m_control & CTRL_HNDSHK))
	{
		if (m_is_write)
		{
			m_ncr->dma_w(m_holding >> 24);
			m_holding <<= 8;
			m_holding_remaining--;
			LOGMASKED(LOG_HANDSHAKE, "DRQ: Holding write %08x, remain %d\n", m_holding, m_holding_remaining);
		}
		else
		{
			m_holding <<= 8;
			m_holding |= m_ncr->dma_r();
			m_holding_remaining--;
			LOGMASKED(LOG_HANDSHAKE, "DRQ: Holding %08x, remain %d\n", m_holding, m_holding_remaining);
		}

		if (m_holding_remaining == 0)
		{
			m_drq_completed = true;
			m_maincpu->trigger(1);
		}
	}
	m_drq = state;
}
