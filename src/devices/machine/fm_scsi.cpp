// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, R. Belmont
/*
 *
 *  Implementation of Fujitsu FMR-50, FMR-60 and FM-Towns SCSI hardware
 *
 *

 info from Toshiya Takeda's e-FMR50 source (the Towns uses the same controller?)
 0xc30 = Data register
 0xc32 = Status register (read)
         bit 7 = REQ
         bit 6 = I/O
         bit 5 = MSG
         bit 4 = C/D
         bit 3 = BUSY
         bit 1 = INT
         bit 0 = PERR
 0xc32 = Control register (write)
         bit 7 = WEN
         bit 6 = IMSK
         bit 4 = ATN
         bit 2 = SEL
         bit 1 = DMAE
         bit 0 = RST
 0xc34 = Word transfer support (read, Towns II)

 Accessing the data register acknowledges a pending REQ.  INT is set by
 each REQ outside the data phases and cleared by a data register access;
 IMSK set enables the interrupt.  The data phases use DMA while DMAE is set.
*/
#include "emu.h"
#include "fm_scsi.h"

#define LOG_CONTROL (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CONTROL)
#include "logmacro.h"

namespace {

enum : uint8_t
{
	STAT_PERR = 0x01,
	STAT_INT  = 0x02,
	STAT_BSY  = 0x08,
	STAT_CD   = 0x10,
	STAT_MSG  = 0x20,
	STAT_IO   = 0x40,
	STAT_REQ  = 0x80
};

enum : uint8_t
{
	CTRL_RST  = 0x01,
	CTRL_DMAE = 0x02,
	CTRL_SEL  = 0x04,
	CTRL_ATN  = 0x10,
	CTRL_IMSK = 0x40,
	CTRL_WEN  = 0x80
};

} // anonymous namespace

DEFINE_DEVICE_TYPE(FMSCSI, fmscsi_device, "fmscsi", "FM-SCSI")

fmscsi_device::fmscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, FMSCSI, tag, owner, clock),
	nscsi_device_interface(mconfig, *this),
	m_irq_handler(*this),
	m_drq_handler(*this),
	m_data(0),
	m_control(0),
	m_ctrl(0),
	m_ack(false),
	m_int(false),
	m_irq(false),
	m_drq(false)
{
}

void fmscsi_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_control));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_ack));
	save_item(NAME(m_int));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
}

void fmscsi_device::device_reset()
{
	m_data = 0;
	m_control = 0;
	m_ack = false;
	m_int = false;

	m_scsi_bus->data_w(m_scsi_refid, 0);
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_ALL, S_ALL);
	m_ctrl = m_scsi_bus->ctrl_r();

	m_irq = false;
	m_irq_handler(CLEAR_LINE);
	m_drq = false;
	m_drq_handler(CLEAR_LINE);
}

void fmscsi_device::scsi_ctrl_changed()
{
	// targets can change the bus again from inside our own line changes, so
	// everything after those reads m_ctrl rather than a local copy
	const uint32_t ctrl = m_scsi_bus->ctrl_r();
	const uint32_t changed = ctrl ^ m_ctrl;
	m_ctrl = ctrl;

	if (changed & S_INP)
	{
		update_data();
	}

	if (changed & S_REQ)
	{
		if (ctrl & S_REQ)
		{
			if (ctrl & (S_CTL | S_MSG))
			{
				set_int(true);
			}
		}
		else if (m_ack)
		{
			ack_w(false);
		}
	}

	update_drq();
}

uint8_t fmscsi_device::data_r()
{
	const uint8_t data = m_scsi_bus->data_r();

	if (!machine().side_effects_disabled())
	{
		set_int(false);
		if (((m_ctrl & (S_REQ | S_INP)) == (S_REQ | S_INP)) && !m_ack)
		{
			ack_w(true);
		}
	}

	return data;
}

void fmscsi_device::data_w(uint8_t data)
{
	m_data = data;
	update_data();

	set_int(false);
	if (((m_ctrl & (S_REQ | S_INP)) == S_REQ) && !m_ack)
	{
		ack_w(true);
	}
}

uint8_t fmscsi_device::status_r()
{
	const uint32_t ctrl = m_scsi_bus->ctrl_r();

	return ((ctrl & S_REQ) ? STAT_REQ : 0) |
		((ctrl & S_INP) ? STAT_IO : 0) |
		((ctrl & S_MSG) ? STAT_MSG : 0) |
		((ctrl & S_CTL) ? STAT_CD : 0) |
		((ctrl & S_BSY) ? STAT_BSY : 0) |
		(m_int ? STAT_INT : 0);
}

void fmscsi_device::control_w(uint8_t data)
{
	LOGMASKED(LOG_CONTROL, "control %02x\n", data);

	m_control = data;
	if (data & CTRL_RST)
	{
		m_ack = false;
		m_int = false;
	}

	const uint32_t lines = ((data & CTRL_RST) ? S_RST : 0) |
		((data & CTRL_SEL) ? S_SEL : 0) |
		((data & CTRL_ATN) ? S_ATN : 0) |
		(m_ack ? S_ACK : 0);
	m_scsi_bus->ctrl_w(m_scsi_refid, lines, S_RST | S_SEL | S_ATN | S_ACK);

	update_irq();
	update_drq();
}

void fmscsi_device::ack_w(bool state)
{
	// drop DRQ first: the target can raise REQ for the next byte inside the ACK
	m_ack = state;
	update_drq();
	m_scsi_bus->ctrl_w(m_scsi_refid, state ? S_ACK : 0, S_ACK);
}

void fmscsi_device::set_int(bool state)
{
	m_int = state;
	update_irq();
}

void fmscsi_device::update_data()
{
	// the data drivers are off while the target drives I/O
	m_scsi_bus->data_w(m_scsi_refid, (m_ctrl & S_INP) ? 0 : m_data);
}

void fmscsi_device::update_irq()
{
	const bool irq = m_int && (m_control & CTRL_IMSK);
	if (irq != m_irq)
	{
		m_irq = irq;
		m_irq_handler(irq ? ASSERT_LINE : CLEAR_LINE);
	}
}

void fmscsi_device::update_drq()
{
	const bool drq = (m_control & CTRL_DMAE) && (m_ctrl & S_REQ) && !(m_ctrl & (S_CTL | S_MSG)) && !m_ack;
	if (drq != m_drq)
	{
		m_drq = drq;
		m_drq_handler(drq ? ASSERT_LINE : CLEAR_LINE);
	}
}

uint8_t fmscsi_device::read(offs_t offset)
{
	switch (offset & 0x03)
	{
	case 0x00:
		return data_r();
	case 0x01:
		return status_r();
	case 0x02:
		return 0x80;  // Linux uses this port to detect the ability to do word transfers.  We'll tell it that it doesn't for now.
	default:
		if (!machine().side_effects_disabled())
		{
			LOG("unknown read at offset %d\n", offset);
		}
		return 0;
	}
}

void fmscsi_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x03)
	{
	case 0x00:
		data_w(data);
		break;
	case 0x01:
		control_w(data);
		break;
	default:
		LOG("unknown write %02x at offset %d\n", data, offset);
		break;
	}
}
