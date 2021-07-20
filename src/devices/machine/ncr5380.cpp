// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * NCR 5380 and 53C80, aka Zilog Z5380, AMD Am5380, Sony CXD1180 and others.
 *
 * Sources:
 *  - http://bitsavers.org/components/ncr/scsi/SP-1051_NCR_5380-53C80_SCSI_Interface_Chip_Design_Manual_Mar86.pdf
 *
 * TODO:
 *  - target mode
 *  - cxd1180 enhancements
 */

#include "emu.h"
#include "ncr5380.h"

#define LOG_GENERAL  (1U << 0)
#define LOG_REGW     (1U << 1)
#define LOG_REGR     (1U << 2)
#define LOG_SCSI     (1U << 3)
#define LOG_ARB      (1U << 4)
#define LOG_DMA      (1U << 5)

//#define VERBOSE (LOG_GENERAL|LOG_REGW|LOG_REGR|LOG_SCSI|LOG_ARB|LOG_DMA)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NCR5380,  ncr5380_device,  "ncr5380",  "NCR 5380 SCSI")
DEFINE_DEVICE_TYPE(NCR53C80, ncr53c80_device, "ncr53c80", "NCR 53C80 SCSI")
DEFINE_DEVICE_TYPE(CXD1180,  cxd1180_device,  "cxd1180",  "Sony CXD1180")

ALLOW_SAVE_TYPE(ncr5380_device::state);

ncr5380_device::ncr5380_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, bool has_lbs)
	: nscsi_device(mconfig, type, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_irq_handler(*this)
	, m_drq_handler(*this)
	, m_has_lbs(has_lbs)
{
}

ncr5380_device::ncr5380_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ncr5380_device(mconfig, NCR5380, tag, owner, clock)
{
}

ncr53c80_device::ncr53c80_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ncr5380_device(mconfig, NCR53C80, tag, owner, clock, true)
{
}

cxd1180_device::cxd1180_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ncr5380_device(mconfig, CXD1180, tag, owner, clock, true)
{
}

void ncr5380_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(ncr5380_device::csdata_r), FUNC(ncr5380_device::odata_w));
	map(0x1, 0x1).rw(FUNC(ncr5380_device::icmd_r), FUNC(ncr5380_device::icmd_w));
	map(0x2, 0x2).rw(FUNC(ncr5380_device::mode_r), FUNC(ncr5380_device::mode_w));
	map(0x3, 0x3).rw(FUNC(ncr5380_device::tcmd_r), FUNC(ncr5380_device::tcmd_w));
	map(0x4, 0x4).rw(FUNC(ncr5380_device::csstat_r), FUNC(ncr5380_device::selen_w));
	map(0x5, 0x5).rw(FUNC(ncr5380_device::bas_r), FUNC(ncr5380_device::sds_w));
	map(0x6, 0x6).rw(FUNC(ncr5380_device::idata_r), FUNC(ncr5380_device::sdtr_w));
	map(0x7, 0x7).rw(FUNC(ncr5380_device::rpi_r), FUNC(ncr5380_device::sdir_w));
}

void ncr5380_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_drq_handler.resolve_safe();

	m_state_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ncr5380_device::state_timer), this));

	save_item(NAME(m_state));

	save_item(NAME(m_odata));
	save_item(NAME(m_icmd));
	save_item(NAME(m_mode));
	save_item(NAME(m_tcmd));
	save_item(NAME(m_bas));
	save_item(NAME(m_idata));

	save_item(NAME(m_scsi_ctrl));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_drq_state));
}

void ncr5380_device::device_reset()
{
	m_state = IDLE;

	// clear registers
	m_odata = 0;
	m_icmd = 0;
	m_mode = 0;
	m_tcmd = 0;
	m_bas = 0;
	m_idata = 0;

	// clear scsi bus
	scsi_bus->data_w(scsi_refid, 0);
	scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);

	// monitor all control lines
	m_scsi_ctrl = 0;
	scsi_bus->ctrl_wait(scsi_refid, S_ALL, S_ALL);

	// clear output lines
	set_irq(false);
	set_drq(false);
}

void ncr5380_device::scsi_ctrl_changed()
{
	u32 const ctrl = scsi_bus->ctrl_r();

	if (VERBOSE & LOG_SCSI)
	{
		static char const *const nscsi_phase[] = { "DATA OUT", "DATA IN", "COMMAND", "STATUS", "*", "*", "MESSAGE OUT", "MESSAGE IN" };

		if (ctrl & S_RST)
			LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x BUS RESET\n", ctrl);
		else if ((ctrl & S_BSY) && !(ctrl & S_SEL))
			LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x phase %s%s%s\n", ctrl, nscsi_phase[ctrl & S_PHASE_MASK],
				ctrl & S_REQ ? " REQ" : "", ctrl & S_ACK ? " ACK" : "");
		else if (ctrl & S_BSY)
			LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x arbitration/selection\n", ctrl);
		else
			LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x BUS FREE\n", ctrl);
	}

	m_bas &= ~BAS_BUSYERROR;

	if (ctrl & S_RST)
	{
		LOG("scsi reset received\n");
		device_reset();

		set_irq(true);
	}
	else if (!(m_mode & MODE_TARGET) && (m_scsi_ctrl & S_BSY) && !(ctrl & S_BSY))
	{
		LOG("target disconnected\n");
		m_icmd &= (IC_RST | IC_AIP);

		if (m_mode & MODE_DMA)
		{
			// stop dma
			m_mode &= ~MODE_DMA;
			m_bas &= ~BAS_ENDOFDMA;

			set_drq(false);
		}

		if (m_mode & MODE_BSYIRQ)
		{
			m_bas |= BAS_BUSYERROR;

			set_irq(true);
		}

		m_state = IDLE;
		m_state_timer->enable(false);

		// clear scsi bus
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
	}
	else if (!(m_scsi_ctrl & S_REQ) && (ctrl & S_REQ))
	{
		// target asserted REQ
		if (m_mode & MODE_DMA)
		{
			if ((ctrl & S_PHASE_MASK) == (m_tcmd & TC_PHASE))
			{
				// transfer cycle
				if (m_state != IDLE && !m_state_timer->enabled())
					m_state_timer->adjust(attotime::zero);
			}
			else
			{
				LOG("phase mismatch %d != %d\n", (ctrl & S_PHASE_MASK), (m_tcmd & TC_PHASE));

				m_state = IDLE;
				m_state_timer->enable(false);

				set_drq(true);
				set_irq(true);
			}
		}
	}

	m_scsi_ctrl = ctrl;
}

u8 ncr5380_device::csdata_r()
{
	u8 const data = scsi_bus->data_r();
	LOGMASKED(LOG_REGR, "csdata_r 0x%02x (%s)\n", data, machine().describe_context());

	return data;
}

void ncr5380_device::odata_w(u8 data)
{
	LOGMASKED(LOG_REGW, "odata_w 0x%02x (%s)\n", data, machine().describe_context());

	// drive scsi data
	if (m_icmd & IC_DBUS)
		scsi_data_w(data);

	m_odata = data;
}

u8 ncr5380_device::icmd_r()
{
	LOGMASKED(LOG_REGR, "icmd_r 0x%02x (%s)\n", m_icmd, machine().describe_context());

	return m_icmd;
}

void ncr5380_device::icmd_w(u8 data)
{
	LOGMASKED(LOG_REGW, "icmd_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!(data & IC_RST))
	{
		// drive scsi data
		if ((data ^ m_icmd) & IC_DBUS)
			scsi_data_w((data & IC_DBUS) ? m_odata : 0);

		// check for control line changes
		if ((data & IC_PHASE) ^ (m_icmd & IC_PHASE))
		{
			u32 const mask = (m_mode & MODE_TARGET) ?
				(S_RST | S_BSY | S_SEL) :
				(S_RST | S_ACK | S_BSY | S_SEL | S_ATN);

			// translate to nscsi
			u32 const ctrl =
				(data & IC_RST ? S_RST : 0) |
				(data & IC_ACK ? S_ACK : 0) |
				(data & IC_BSY ? S_BSY : 0) |
				(data & IC_SEL ? S_SEL : 0) |
				(data & IC_ATN ? S_ATN : 0);

			LOGMASKED(LOG_SCSI, "changing control lines 0x%04x\n", ctrl);
			scsi_bus->ctrl_w(scsi_refid, ctrl, mask);
		}
	}
	else
	{
		LOG("scsi reset issued\n");
		device_reset();
		scsi_bus->ctrl_w(scsi_refid, S_RST, S_RST);

		set_irq(true);
	}

	m_icmd = (m_icmd & ~IC_WRITE) | (data & IC_WRITE);
}

u8 ncr5380_device::mode_r()
{
	LOGMASKED(LOG_REGR, "mode_r 0x%02x (%s)\n", m_mode, machine().describe_context());

	return m_mode;
}

void ncr5380_device::mode_w(u8 data)
{
	LOGMASKED(LOG_REGW, "mode_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!(data & MODE_BSYIRQ))
		m_bas &= ~BAS_BUSYERROR;

	// disable dma
	if ((m_mode & MODE_DMA) && !(data & MODE_DMA))
	{
		m_state = IDLE;
		m_state_timer->enable(false);

		m_bas &= ~BAS_ENDOFDMA;

		if (m_has_lbs)
			m_tcmd &= ~TC_LBS;

		set_drq(false);

		// clear ACK
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
	}

	// start/stop arbitration
	if ((m_mode ^ data) & MODE_ARBITRATE)
	{
		if (data & MODE_ARBITRATE)
		{
			// start arbitration
			m_icmd &= ~IC_LA;
			m_state = ARB_BUS_FREE;
			m_state_timer->adjust(attotime::zero);
		}
		else
		{
			// stop arbitration
			m_state = IDLE;
			m_icmd &= ~(IC_AIP | IC_LA);
		}
	}

	m_mode = data;
}

u8 ncr5380_device::tcmd_r()
{
	LOGMASKED(LOG_REGR, "tcmd_r 0x%02x (%s)\n", m_tcmd, machine().describe_context());

	return m_tcmd;
}

void ncr5380_device::tcmd_w(u8 data)
{
	LOGMASKED(LOG_REGW, "tcmd_w 0x%02x (%s)\n", data, machine().describe_context());

	if (m_has_lbs)
		m_tcmd = (m_tcmd & TC_LBS) | (data & ~TC_LBS);
	else
		m_tcmd = data;
}

u8 ncr5380_device::csstat_r()
{
	u32 const ctrl = scsi_bus->ctrl_r();
	u8 const data =
		(ctrl & S_RST ? ST_RST : 0) |
		(ctrl & S_BSY ? ST_BSY : 0) |
		(ctrl & S_REQ ? ST_REQ : 0) |
		(ctrl & S_MSG ? ST_MSG : 0) |
		(ctrl & S_CTL ? ST_CD  : 0) |
		(ctrl & S_INP ? ST_IO  : 0) |
		(ctrl & S_SEL ? ST_SEL : 0);

	LOGMASKED(LOG_REGR, "csstat_r 0x%02x (%s)\n", data, machine().describe_context());
	return data;
}

void ncr5380_device::selen_w(u8 data)
{
	LOGMASKED(LOG_REGW, "selen_w 0x%02x (%s)\n", data, machine().describe_context());
}

u8 ncr5380_device::bas_r()
{
	u32 const ctrl = scsi_bus->ctrl_r();
	u8 const data = m_bas |
		(((ctrl & S_PHASE_MASK) == (m_tcmd & TC_PHASE)) ? BAS_PHASEMATCH : 0) |
		(ctrl & S_ATN ? BAS_ATN : 0) |
		(ctrl & S_ACK ? BAS_ACK : 0);

	LOGMASKED(LOG_REGR, "bas_r 0x%02x (%s)\n", data, machine().describe_context());

	return data;
}

void ncr5380_device::sds_w(u8 data)
{
	LOGMASKED(LOG_REGW, "sds_w 0x%02x (%s)\n", data, machine().describe_context());

	if (m_mode & MODE_DMA)
	{
		m_state = DMA_OUT_DRQ;
		m_state_timer->adjust(attotime::zero);
	}
}

u8 ncr5380_device::idata_r()
{
	LOGMASKED(LOG_REGR, "idata_r 0x%02x (%s)\n", m_idata, machine().describe_context());

	return m_idata;
}

void ncr5380_device::sdtr_w(u8 data)
{
	LOGMASKED(LOG_REGW, "sdtr_w 0x%02x (%s)\n", data, machine().describe_context());
}

u8 ncr5380_device::rpi_r()
{
	LOGMASKED(LOG_REGR, "rpi_r (%s)\n", machine().describe_context());

	m_bas &= ~(BAS_PARITYERROR | BAS_BUSYERROR);
	set_irq(false);

	return 0;
}

void ncr5380_device::sdir_w(u8 data)
{
	LOGMASKED(LOG_REGW, "sdir_w 0x%02x (%s)\n", data, machine().describe_context());

	if ((m_mode & MODE_DMA) && !(m_mode & MODE_TARGET))
	{
		m_state = DMA_IN_REQ;
		m_state_timer->adjust(attotime::zero);
	}
}

void ncr5380_device::state_timer(void *ptr, s32 param)
{
	// step state machine
	int const delay = state_step();

	// check for data stall
	if (delay < 0)
		return;

	// repeat until idle
	if (m_state != IDLE)
		m_state_timer->adjust(attotime::from_nsec(delay));
}

int ncr5380_device::state_step()
{
	u32 const ctrl = scsi_bus->ctrl_r();
	int delay = 0;

	switch (m_state)
	{
	case IDLE:
		break;

	case ARB_BUS_FREE:
		if (!(ctrl & (S_SEL | S_BSY | S_RST)))
		{
			LOGMASKED(LOG_ARB, "arbitration: bus free\n");
			// FIXME: AIP should only be set when arbitration begins
			m_icmd |= IC_AIP;
			m_state = ARB_START;
			delay = 1700;
		}
		else
		{
			LOGMASKED(LOG_ARB, "arbitration: bus not free\n");
			m_state = IDLE;
		}
		break;
	case ARB_START:
		LOGMASKED(LOG_ARB, "arbitration: started\n");
		m_icmd |= IC_BSY;
		m_state = ARB_EVALUATE;
		delay = 2200;

		// assert own ID and BSY
		scsi_bus->data_w(scsi_refid, m_odata);
		scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
		break;
	case ARB_EVALUATE:
		// check if SEL asserted, or if there's a higher ID on the bus
		if ((ctrl & S_SEL) || (scsi_bus->data_r() & ~((m_odata - 1) | m_odata)))
		{
			LOGMASKED(LOG_ARB, "arbitration: lost\n");
			m_icmd &= ~IC_BSY;
			m_icmd |= IC_LA;

			m_state = IDLE;

			// clear data and BSY
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
		}
		else
		{
			LOGMASKED(LOG_ARB, "arbitration: won\n");
			m_state = IDLE;
		}
		break;

	case DMA_IN_REQ:
		if (ctrl & S_REQ)
		{
			if ((ctrl & S_PHASE_MASK) == (m_tcmd & TC_PHASE))
			{
				m_idata = scsi_bus->data_r();
				LOGMASKED(LOG_DMA, "dma in: 0x%02x\n", m_idata);

				m_state = DMA_IN_ACK;
				set_drq(true);

				// assert ACK
				scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
			}

			delay = -1;
		}
		break;
	case DMA_IN_ACK:
		if (!(ctrl & S_REQ))
		{
			m_state = (m_bas & BAS_ENDOFDMA) ? IDLE : DMA_IN_REQ;

			// clear ACK
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		break;

	case DMA_OUT_DRQ:
		m_state = DMA_OUT_REQ;
		set_drq(true);

		delay = -1;
		break;
	case DMA_OUT_REQ:
		if (ctrl & S_REQ)
		{
			if ((ctrl & S_PHASE_MASK) == (m_tcmd & TC_PHASE))
			{
				LOGMASKED(LOG_DMA, "dma out: 0x%02x\n", m_odata);

				m_state = DMA_OUT_ACK;

				// assert data and ACK
				scsi_bus->data_w(scsi_refid, m_odata);
				scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
			}
		}
		break;
	case DMA_OUT_ACK:
		if (!(ctrl & S_REQ))
		{
			if (m_bas & BAS_ENDOFDMA)
			{
				m_state = IDLE;

				if (m_has_lbs)
					m_tcmd |= TC_LBS;
			}
			else
				m_state = DMA_OUT_DRQ;

			// clear data and ACK
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		break;
	}

	return delay;
}

void ncr5380_device::eop_w(int state)
{
	LOGMASKED(LOG_DMA, "eop_w %d\n", state);
	if (state && (m_mode & MODE_DMA))
	{
		m_bas |= BAS_ENDOFDMA;

		if (m_mode & MODE_EOPIRQ)
		{
			// FIXME: should only trigger when combined with dma_r/dma_w
			LOG("eop irq asserted\n");

			set_irq(true);
		}
	}
}

void ncr5380_device::dma_w(u8 data)
{
	set_drq(false);

	if (m_mode & MODE_DMA)
	{
		m_odata = data;

		m_state_timer->adjust(attotime::zero);
	}
}

u8 ncr5380_device::dma_r()
{
	set_drq(false);

	if (m_mode & MODE_DMA)
		m_state_timer->adjust(attotime::zero);

	return m_idata;
}

void ncr5380_device::scsi_data_w(u8 data)
{
	// TODO: release data bus when any of the prerequisite conditions expire
	u32 const ctrl = scsi_bus->ctrl_r();

	if ((m_mode & MODE_TARGET) || (!(ctrl & S_INP) && (ctrl & S_PHASE_MASK) == (m_tcmd & S_PHASE_MASK)))
	{
		LOGMASKED(LOG_SCSI, "scsi data 0x%02x\n", data);
		scsi_bus->data_w(scsi_refid, data);
	}
}

void ncr5380_device::set_irq(bool irq_state)
{
	if (irq_state != m_irq_state)
	{
		LOG("set_irq %d\n", irq_state);

		if (irq_state)
			m_bas |= BAS_IRQACTIVE;
		else
			m_bas &= ~BAS_IRQACTIVE;

		m_irq_state = irq_state;
		m_irq_handler(m_irq_state);
	}
}

void ncr5380_device::set_drq(bool drq_state)
{
	if (drq_state != m_drq_state)
	{
		LOGMASKED(LOG_DMA, "set_drq %d\n", drq_state);

		if (drq_state)
			m_bas |= BAS_DMAREQUEST;
		else
			m_bas &= ~BAS_DMAREQUEST;

		m_drq_state = drq_state;
		m_drq_handler(m_drq_state);
	}
}

u8 ncr5380_device::read(offs_t offset)
{
	switch (offset & 7)
	{
	case 0: return csdata_r();
	case 1: return icmd_r();
	case 2: return mode_r();
	case 3: return tcmd_r();
	case 4: return csstat_r();
	case 5: return bas_r();
	case 6: return idata_r();
	case 7: return rpi_r();
	}

	// can't happen
	return 0;
}

void ncr5380_device::write(offs_t offset, u8 data)
{
	switch (offset & 7)
	{
	case 0: odata_w(data); break;
	case 1: icmd_w(data); break;
	case 2: mode_w(data); break;
	case 3: tcmd_w(data); break;
	case 4: selen_w(data); break;
	case 5: sds_w(data); break;
	case 6: sdtr_w(data); break;
	case 7: sdir_w(data); break;
	}
}
