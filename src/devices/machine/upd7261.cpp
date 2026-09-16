// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * NEC μPD7261A/μPD7261B Hard-Disk Controller
 *
 * Sources:
 *  - μPD7261A/μPD7261B Hard-Disk Controllers, © NEC Electronics Inc.
 *
 * TODO:
 *  - unemulated commands
 *  - hard sectored drives
 *  - concurrent recalibrate/seek commands
 *  - terminal count
 *  - drive/head mapping for 3b2
 */

#include "emu.h"
#include "upd7261.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REGR    (1U << 1)
#define LOG_REGW    (1U << 2)
#define LOG_COMMAND (1U << 3)
#define LOG_STATE   (1U << 4)
#define LOG_DMA     (1U << 5)

//#define VERBOSE (LOG_GENERAL|LOG_COMMAND)
#include "logmacro.h"

unsigned constexpr BUF_SIZE = 4096;

enum state : u32
{
	IDLE = 0,

	EXECUTE_READ,
	EXECUTE_WRITE,

	SEEK_POLLED0,  // recalibrate/seek with polling
	SEEK_POLLED1,  // recalibrate/seek with polling

	RESULTS_4,     // detect error
	RESULTS_56,    // results: recalibrate, seek
	RESULTS_bcdef, // results: read data, check, scan, verify data, write data
	RESULTS_789,   // results: format, verify id, read id

	COMPLETE,
	ERROR,
};

enum status_mask : u8
{
	S_DRQ = 0x01, // data request
	S_NCI = 0x02, // not coincident
	S_IER = 0x04, // ID error
	S_RRQ = 0x08, // reset request
	S_SRQ = 0x10, // sense interrupt status request
	S_CEL = 0x20, // command end lo
	S_CEH = 0x40, // command end hi
	S_CB  = 0x80, // controller busy
};

enum ist_mask : u8
{
	IST_UA  = 0x07, // unit address
	IST_NR  = 0x08, // not ready
	IST_EQC = 0x10, // equipment check
	IST_SER = 0x20, // seek error
	IST_RC  = 0x40, // ready change
	IST_SEN = 0x80, // seek end
};

enum ust_mask : u8
{
	UST_D0 = 0x01, // fault/write fault
	UST_D1 = 0x02, // seek error/ready
	UST_D2 = 0x04, // on cylinder/track 000
	UST_D3 = 0x08, // unit ready/seek complete
	UST_D4 = 0x10, // -/drive selected
	UST_D5 = 0x20, // write protected/-
	UST_D6 = 0x40, // seek end/-
	UST_D7 = 0x80, // unit selected/-
};

enum est_mask : u8
{
	EST_MAM = 0x01, // missing address mark
	EST_NWR = 0x02, // not writable
	EST_ND  = 0x04, // no data
	EST_NR  = 0x08, // not ready
	EST_EQC = 0x10, // equipment check
	EST_DER = 0x20, // data error
	EST_OVR = 0x40, // overrun
	EST_ENC = 0x80, // end of cylinder
};

enum specify_mode_mask : u8
{
	SM_STP  = 0x0f, // stepping rate
	SM_SSEC = 0x10, // soft-sector disk
	SM_CRCS = 0x20, // generator polynomial
	SM_ECC  = 0x40, // ecc is appended in data field
};

enum specify_dtlh_mask : u8
{
	DTLH_DTLH = 0x0f, // data length high bits
	DTLH_NPOL = 0x10, // non-polling mode
	DTLH_PAD  = 0x20, // ID/data pad (0=0x00, 1=0x4e)
	DTLH_CRC  = 0x40, // initial polynomial counter
};

DEFINE_DEVICE_TYPE(UPD7261, upd7261_device, "upd7261", "NEC uPD7261 Hard-Disk Controller")

upd7261_device::upd7261_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD7261, tag, owner, clock)
	, m_drive(*this, "%u", 0)
	, m_dreq(*this)
	, m_int(*this)
	, m_head(0)
	, m_specify{}
	, m_transfer{}
	, m_dreq_state(false)
	, m_int_state(false)
	, m_tc_state(false)
{
}

void upd7261_device::device_start()
{
	save_item(NAME(m_state));
	save_item(NAME(m_head));

	save_item(NAME(m_status));
	save_item(NAME(m_est));
	save_item(NAME(m_ist));
	save_item(NAME(m_ua));
	save_item(NAME(m_pcn));

	save_item(STRUCT_MEMBER(m_specify, mode));
	save_item(STRUCT_MEMBER(m_specify, dtlh));
	save_item(STRUCT_MEMBER(m_specify, dtll));
	save_item(STRUCT_MEMBER(m_specify, etn));
	save_item(STRUCT_MEMBER(m_specify, esn));
	save_item(STRUCT_MEMBER(m_specify, gpl2));
	save_item(STRUCT_MEMBER(m_specify, rwch));
	save_item(STRUCT_MEMBER(m_specify, rwcl));

	save_item(STRUCT_MEMBER(m_transfer, phn));
	save_item(STRUCT_MEMBER(m_transfer, lcnh));
	save_item(STRUCT_MEMBER(m_transfer, lcnl));
	save_item(STRUCT_MEMBER(m_transfer, lhn));
	save_item(STRUCT_MEMBER(m_transfer, lsn));
	save_item(STRUCT_MEMBER(m_transfer, scnt));

	save_item(NAME(m_buf_index));
	save_item(NAME(m_buf_count));
	save_pointer(NAME(m_buf), BUF_SIZE);

	save_item(NAME(m_dreq_state));
	save_item(NAME(m_int_state));
	save_item(NAME(m_tc_state));

	m_state_timer = timer_alloc(timer_expired_delegate(FUNC(upd7261_device::state_timer), this));

	m_buf = std::make_unique<u8[]>(BUF_SIZE);
}

void upd7261_device::device_reset()
{
	m_state = IDLE;

	m_status = 0;
	m_est = 0;
	m_ist = 0;
	m_ua = 0;

	for (u16 &pcn : m_pcn)
		pcn = 0;

	m_buf_index = 0;
	m_buf_count = 0;

	set_dreq(false);
	set_int(false);
}

void upd7261_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(upd7261_device::data_r), FUNC(upd7261_device::data_w));
	map(0x1, 0x1).rw(FUNC(upd7261_device::status_r), FUNC(upd7261_device::command_w));
}

void upd7261_device::set_dreq(int state)
{
	if (state != m_dreq_state)
	{
		LOGMASKED(LOG_STATE, "dreq %d\n", state);

		if (state)
		{
			m_status |= S_DRQ;
			m_buf_count = m_specify.dtl();
		}
		else
		{
			m_status &= ~S_DRQ;
			m_buf_count = 0;
		}

		m_dreq_state = state;
		m_dreq(m_dreq_state);
	}
}

void upd7261_device::set_int(bool state)
{
	if (state != m_int_state)
	{
		LOGMASKED(LOG_STATE, "int %d\n", state);
		m_int_state = state;
		m_int(m_int_state);
	}
}

void upd7261_device::tc_w(int state)
{
	LOGMASKED(LOG_STATE, "tc %d\n", state);

	m_tc_state = !state;
}

u8 upd7261_device::data_r()
{
	if (m_buf_index == m_buf_count)
		fatalerror("%s: buffer underrun\n", tag());

	u8 const data = m_buf[m_buf_index++];

	if (m_status & S_DRQ)
		LOGMASKED(LOG_DMA, "data_r 0x%02x (dma)\n", data);
	else
		LOGMASKED(LOG_REGR, "data_r 0x%02x (%s)\n", data, machine().describe_context());

	if ((m_status & S_DRQ) && (m_buf_index == m_buf_count))
		m_state_timer->adjust(attotime::zero);

	return data;
}

void upd7261_device::data_w(u8 data)
{
	if (m_status & S_DRQ)
		LOGMASKED(LOG_DMA, "data_w 0x%02x (dma)\n", data);
	else
		LOGMASKED(LOG_REGW, "data_w 0x%02x (%s)\n", data, machine().describe_context());

	if (m_buf_index == BUF_SIZE)
		fatalerror("%s: buffer overrun\n", tag());

	m_buf[m_buf_index++] = data;

	if ((m_status & S_DRQ) && (m_buf_index == m_buf_count))
		m_state_timer->adjust(attotime::zero);
}

u8 upd7261_device::status_r()
{
	u8 data = m_status;

	LOGMASKED(LOG_REGR, "status_r 0x%02x (%s)\n", data, machine().describe_context());

	return data;
}

void upd7261_device::command_w(u8 data)
{
	LOGMASKED(LOG_REGW, "command_w 0x%02x (%s)\n", data, machine().describe_context());
	static constexpr attotime execute = attotime::from_nsec(400);

	if (BIT(data, 4, 4))
	{
		m_status &= ~(S_CEH | S_CEL);
		m_status |= S_CB;

		m_ua = BIT(data, 0, (m_specify.mode & SM_SSEC) ? 2 : 3);

		m_est = 0;

		switch (BIT(data, 4, 4))
		{
		case 0x1: // sense interrupt status
			LOGMASKED(LOG_COMMAND, "sense interrupt status 0x%02x\n", m_ist);
			m_buf_index = 0;
			m_buf_count = 0;

			if (m_status & S_SRQ)
			{
				m_buf[m_buf_count++] = m_ist;
				m_status &= ~S_SRQ;
				m_state = COMPLETE;
			}
			else
				m_state = ERROR;

			m_ist = 0;
			m_state_timer->adjust(execute);
			break;
		case 0x2: // specify
			m_specify.mode = m_buf[0];
			m_specify.dtlh = m_buf[1];
			m_specify.dtll = m_buf[2];
			m_specify.etn = m_buf[3];
			m_specify.esn = m_buf[4];
			m_specify.gpl2 = m_buf[5];
			m_specify.rwch = m_buf[6];
			m_specify.rwcl = m_buf[7];

			m_buf_index = 0;
			m_buf_count = 0;

			LOGMASKED(LOG_COMMAND, "specify mode 0x%02x dtl 0x%04x etn 0x%02x esn 0x%02x gpl2 0x%02x rwch 0x%02x rwcl 0x%02x\n",
				m_specify.mode, m_specify.dtl(), m_specify.etn, m_specify.esn, m_specify.gpl2, m_specify.rwch, m_specify.rwcl);

			if (!(m_specify.mode & SM_SSEC))
				fatalerror("%s: hard sectored mode is not emulated\n", tag());

			m_state = COMPLETE;
			m_state_timer->adjust(execute);
			break;
		case 0x3: // sense unit status
			LOGMASKED(LOG_COMMAND, "sense unit status\n");
			m_buf_index = 0;
			m_buf_count = 0;

			if (m_drive[m_ua] && m_drive[m_ua]->exists())
				m_buf[m_buf_count++] = UST_D4 | UST_D1;
			else
				m_buf[m_buf_count++] = 0;

			m_state = COMPLETE;
			m_state_timer->adjust(execute);
			break;
		case 0x4: // detect error
			LOGMASKED(LOG_COMMAND, "detect error (not emulated)\n");
			m_buf_index = 0;
			m_buf_count = 0;

			m_state = RESULTS_4;
			m_state_timer->adjust(execute);
			break;
		case 0x5: // recalibrate
			LOGMASKED(LOG_COMMAND, "recalibrate (%s mode with polling %s)\n",
				BIT(data, 3) ? "buffered" : "normal", (m_specify.dtlh & DTLH_NPOL) ? "disabled" : "enabled");
			m_buf_index = 0;
			m_buf_count = 0;

			if (m_specify.mode & SM_SSEC)
			{
				if (m_specify.dtlh & DTLH_NPOL)
				{
					// non-polled mode
					m_buf[m_buf_count++] = IST_SEN | m_ua;
					m_state = COMPLETE;

					m_state_timer->adjust(attotime::from_ticks(m_specify.stp(m_pcn[m_ua]), clock()));
					m_pcn[m_ua] = 0;
				}
				else
				{
					// polled mode
					m_state = SEEK_POLLED0;
					m_state_timer->adjust(execute, 0);
				}
			}
			break;
		case 0x6: // seek
			LOGMASKED(LOG_COMMAND, "seek pcn 0x%04x (%s mode with polling %s)\n",
				u16(m_buf[0]) << 8 | m_buf[1], BIT(data, 3) ? "buffered" : "normal",
				(m_specify.dtlh & DTLH_NPOL) ? "disabled" : "enabled");

			m_buf_index = 0;
			m_buf_count = 0;
			if (m_specify.mode & SM_SSEC)
			{
				u16 const pcn = u16(m_buf[0]) << 8 | m_buf[1];

				if (m_specify.dtlh & DTLH_NPOL)
				{
					// non-polled mode
					m_buf[m_buf_count++] = IST_SEN | m_ua;
					m_state = COMPLETE;

					m_state_timer->adjust(attotime::from_ticks(m_specify.stp((pcn > m_pcn[m_ua]) ? pcn - m_pcn[m_ua] : m_pcn[m_ua] - pcn), clock()));
					m_pcn[m_ua] = pcn;
				}
				else
				{
					m_state = SEEK_POLLED0;
					m_state_timer->adjust(execute, pcn);
				}
			}
			break;
		case 0x7: // format
			LOGMASKED(LOG_COMMAND, "format (not emulated)\n");
			break;
		case 0x8: // verify id
			LOGMASKED(LOG_COMMAND, "verify id (not emulated)\n");
			break;
		case 0x9: // read id
			LOGMASKED(LOG_COMMAND, "read id (not emulated)\n");
			break;
		case 0xa: // read diagnostic
			LOGMASKED(LOG_COMMAND, "read diagnostic (not emulated)\n");
			break;
		case 0xb: // read data
			if (m_specify.mode & SM_SSEC)
			{
				m_transfer.phn = m_buf[0];
				m_transfer.lcnh = m_buf[1];
				m_transfer.lcnl = m_buf[2];
				m_transfer.lhn = m_buf[3];
				m_transfer.lsn = m_buf[4];
				m_transfer.scnt = m_buf[5];

				m_buf_index = 0;
				m_buf_count = 0;

				LOGMASKED(LOG_COMMAND, "read data phn 0x%02x lcn 0x%04x lhn 0x%02x lsn 0x%02x scnt 0x%02x\n",
					m_transfer.phn, m_transfer.lcn(), m_transfer.lhn, m_transfer.lsn, m_transfer.scnt);

				m_state = EXECUTE_READ;
				m_state_timer->adjust(execute);
			}
			break;
		case 0xc: // check
			LOGMASKED(LOG_COMMAND, "check (not emulated)\n");
			break;
		case 0xd: // scan
			LOGMASKED(LOG_COMMAND, "scan (not emulated)\n");
			break;
		case 0xe: // verify data
			LOGMASKED(LOG_COMMAND, "verify data (not emulated)\n");
			break;
		case 0xf: // write data
			if (m_specify.mode & SM_SSEC)
			{
				m_transfer.phn = m_buf[0];
				m_transfer.lcnh = m_buf[1];
				m_transfer.lcnl = m_buf[2];
				m_transfer.lhn = m_buf[3];
				m_transfer.lsn = m_buf[4];
				m_transfer.scnt = m_buf[5];

				m_buf_index = 0;
				m_buf_count = 0;

				LOGMASKED(LOG_COMMAND, "write data phn 0x%02x lcn 0x%04x lhn 0x%02x lsn 0x%02x scnt 0x%02x\n",
					m_transfer.phn, m_transfer.lcn(), m_transfer.lhn, m_transfer.lsn, m_transfer.scnt);

				m_state = EXECUTE_WRITE;
				set_dreq(true);
			}
			break;
		}
	}
	else
	{
		u8 mask = (S_CEH | S_CEL | S_SRQ);

		LOGMASKED(LOG_COMMAND, "auxiliary command%s%s%s%s\n",
			BIT(data, 0) ? ", chip reset" : "",
			BIT(data, 1) ? ", clear data" : "",
			BIT(data, 2) ? ", clear srq" : "",
			BIT(data, 3) ? ", clear ce" : "");

		if (BIT(data, 0))
			reset();
		if (BIT(data, 1))
		{
			m_buf_index = 0;
			m_buf_count = 0;
		}
		if (BIT(data, 2))
			mask &= ~S_SRQ;
		if (BIT(data, 3))
			m_status &= ~(S_CEH | S_CEL);

		set_int(m_status & mask);
	}
}

void upd7261_device::state_timer(s32 param)
{
	// step state machine
	attotime const delay = state_step(param);

	// check for data stall
	if (delay.is_never())
		return;

	// repeat until idle
	if (m_state != IDLE)
		m_state_timer->adjust(delay);
}

attotime upd7261_device::state_step(s32 param)
{
	attotime delay = attotime::zero;

	switch (m_state)
	{
	case IDLE:
		break;

	case EXECUTE_READ:
		// check unit address is valid
		if (!m_drive[m_ua] && m_drive[m_ua]->exists())
			m_est |= EST_NR;

		if (m_transfer.scnt && !m_est)
		{
			// HACK: The MG-1 has an additional external head select bit, and
			// expects the controller to report "no data" when the sector to be
			// transferred can't be found on the track. This hack uses a fake
			// external head register to check whether the current head matches
			// before reading the disk.
			if (m_transfer.lhn == ((m_head & ~7) | (m_transfer.lhn & 7)))
			{
				// read a sector
				harddisk_image_device &hid(*m_drive[m_ua]);
				hard_disk_file::info const &i = hid.get_info();

				u32 const lba = ((m_transfer.lcn() * i.heads) + m_transfer.lhn) * i.sectors + m_transfer.lsn;

				hid.read(lba, m_buf.get());
				m_buf_index = 0;

				m_transfer.scnt--;

				if (m_transfer.lsn++ == m_specify.esn)
				{
					m_transfer.lsn = 0;

					if (m_transfer.lhn++ == m_specify.etn)
					{
						m_transfer.lhn = 0;
						m_transfer.lcnl++;
						if (m_transfer.lcnl == 0)
							m_transfer.lcnh++;

						if (m_transfer.scnt)
							m_est |= EST_ENC;
					}
				}

				delay = attotime::never;
			}
			else
			{
				m_est = EST_ND;
				m_state = RESULTS_bcdef;
				m_buf_index = 0;
			}
		}
		else
		{
			m_state = RESULTS_bcdef;
			m_buf_index = 0;
		}

		set_dreq(m_state == EXECUTE_READ);
		break;

	case EXECUTE_WRITE:
		// check unit address is valid
		if (!m_drive[m_ua] && m_drive[m_ua]->exists())
			m_est |= EST_NR;

		if (m_transfer.scnt && !m_est)
		{
			// HACK: as above for reading
			if (m_transfer.lhn == ((m_head & ~7) | (m_transfer.lhn & 7)))
			{
				// write a sector
				harddisk_image_device &hid(*m_drive[m_ua]);
				hard_disk_file::info const &i = hid.get_info();

				u32 const lba = ((m_transfer.lcn() * i.heads) + m_transfer.lhn) * i.sectors + m_transfer.lsn;

				hid.write(lba, m_buf.get());
				m_buf_index = 0;

				m_transfer.scnt--;

				if (m_transfer.lsn++ == m_specify.esn)
				{
					m_transfer.lsn = 0;

					if (m_transfer.lhn++ == m_specify.etn)
					{
						m_transfer.lhn = 0;
						m_transfer.lcnl++;
						if (m_transfer.lcnl == 0)
							m_transfer.lcnh++;

						if (m_transfer.scnt)
						{
							m_est |= EST_ENC;
							m_state = RESULTS_bcdef;
						}
					}
				}

				if (!m_transfer.scnt)
					m_state = RESULTS_bcdef;

				if (m_state == EXECUTE_WRITE)
					delay = attotime::never;
			}
			else
			{
				m_est = EST_ND;
				m_state = RESULTS_bcdef;
				m_buf_index = 0;
			}
		}
		else
		{
			m_state = RESULTS_bcdef;
			m_buf_index = 0;
		}

		set_dreq(m_state == EXECUTE_WRITE);
		break;

	case SEEK_POLLED0:
		m_status &= ~S_CB;
		m_status |= S_CEH;
		m_state = SEEK_POLLED1;

		delay = attotime::from_ticks(m_specify.stp(std::abs(m_pcn[m_ua] - param)), clock());
		m_pcn[m_ua] = param;

		set_int(true);
		break;

	case SEEK_POLLED1:
		m_ist |= IST_SEN | m_ua;
		m_status |= S_SRQ;
		m_state = IDLE;

		set_int(true);
		break;

	case RESULTS_4:
		m_buf[m_buf_count++] = 0; // eadh
		m_buf[m_buf_count++] = 0; // eadl
		m_buf[m_buf_count++] = 0; // ept1
		m_buf[m_buf_count++] = 0; // ept2
		m_buf[m_buf_count++] = 0; // ept3

		m_state = COMPLETE;
		break;

	case RESULTS_789:
		m_buf[m_buf_count++] = m_est;
		m_buf[m_buf_count++] = m_transfer.scnt;

		m_state = COMPLETE;
		break;

	case RESULTS_bcdef:
		m_buf[m_buf_count++] = m_est;
		m_buf[m_buf_count++] = m_transfer.phn;
		// (flag)
		m_buf[m_buf_count++] = m_transfer.lcnh;
		m_buf[m_buf_count++] = m_transfer.lcnl;
		m_buf[m_buf_count++] = m_transfer.lhn;
		m_buf[m_buf_count++] = m_transfer.lsn;
		m_buf[m_buf_count++] = m_transfer.scnt;

		m_state = COMPLETE;
		break;

	case COMPLETE:
		if (m_est)
			m_status |= S_CEL;
		else
			m_status |= S_CEH;
		m_status &= ~S_CB;

		set_int(true);

		m_state = IDLE;
		break;

	case ERROR:
		m_status |= S_CEH | S_CEL;
		m_status &= ~S_CB;

		set_int(true);

		m_state = IDLE;
		break;
	}

	return delay;
}

u16 upd7261_device::specify::dtl() const
{
	return u16(dtlh & DTLH_DTLH) << 8 | dtll;
}

u16 upd7261_device::transfer::lcn() const
{
	return u16(lcnh) << 8 | lcnl;
}

unsigned upd7261_device::specify::stp(unsigned cylinders) const
{
	/*
	 * Datasheet formula for ST506 interface stepping is given as:
	 *
	 *   (16 - stp) * 2110 * tCY
	 *
	 * Example for a 10MHz clock gives a range of stepping values from
	 * 2.11ms to 33.76ms; formula seems incorrect by a factor of 10?
	 */
	return (16 - (mode & SM_STP)) * 21100 * cylinders;
}
