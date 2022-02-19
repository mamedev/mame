// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************

        SPG290 CD Servo

*****************************************************************************/

#include "emu.h"
#include "spg290_cdservo.h"
#include "coreutil.h"


#define SPG290_LEADIN_LEN      7500
#define SPG290_LEADOUT_LEN     6750


DEFINE_DEVICE_TYPE(SPG290_CDSERVO, spg290_cdservo_device, "spg290_cdservo", "SPG290 CDServo HLE")


spg290_cdservo_device::spg290_cdservo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPG290_CDSERVO, tag, owner, clock)
	, m_cdrom(*this, finder_base::DUMMY_TAG)
	, m_irq_cb(*this)
	, m_space_write_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spg290_cdservo_device::device_start()
{
	m_irq_cb.resolve_safe();
	m_space_write_cb.resolve_safe();

	m_cdtimer = timer_alloc();
	m_dsp_memory = std::make_unique<uint32_t[]>(0x10000);

	save_item(NAME(m_addr));
	save_item(NAME(m_data));
	save_item(NAME(m_buf_start));
	save_item(NAME(m_buf_end));
	save_item(NAME(m_buf_ptr));
	save_item(NAME(m_speed));
	save_item(NAME(m_seek_min));
	save_item(NAME(m_seek_sec));
	save_item(NAME(m_seek_frm));
	save_item(NAME(m_seek_lba));
	save_item(NAME(m_sector_size));
	save_item(NAME(m_dsp_data));
	save_item(NAME(m_frame_found));
	save_item(NAME(m_control1));
	save_item(NAME(m_skip));
	save_item(NAME(m_dsp_regs));
	save_item(NAME(m_tot_sectors));
	save_item(NAME(m_cur_sector));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spg290_cdservo_device::device_reset()
{
	m_addr = 0;
	m_data = 0;
	m_buf_start = 0;
	m_buf_end = 0;
	m_buf_ptr = 0;
	m_speed = 1;
	m_seek_min = 0;
	m_seek_sec = 0;
	m_seek_frm = 0;
	m_seek_lba = 0;
	m_sector_size = 0;
	m_dsp_data = 0;
	m_frame_found = false;
	m_control1 = 0;
	m_skip = 0;

	std::fill(std::begin(m_dsp_regs), std::end(m_dsp_regs), 0);
	std::fill_n(m_dsp_memory.get(), 0x10000, 0);

	m_tot_sectors = 0;
	m_cur_sector = 0;
	m_qsub = nullptr;

	m_irq_cb(CLEAR_LINE);

	// generate Q subchannel
	if (m_cdrom.found())
	{
		auto *cdrom = m_cdrom->get_cdrom_file();
		if (cdrom != nullptr)
			generate_qsub(cdrom);
	}

	change_status();
}

void spg290_cdservo_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (!(m_control1 & 0x04) && m_cur_sector == m_seek_lba + SPG290_LEADIN_LEN)
	{
		uint8_t cdbuf[2448] = { 0 };

		if (BIT(m_control0, 15))        // CDDA
		{
			cdrom_read_data(m_cdrom->get_cdrom_file(), m_cur_sector - 150 - SPG290_LEADIN_LEN, cdbuf, CD_TRACK_AUDIO);

			for (int i=0; i<2352; i++)
			{
				m_space_write_cb(m_buf_ptr++, cdbuf[i]);
				if (m_buf_ptr > m_buf_end)
					m_buf_ptr = m_buf_start;
			}
		}
		else
		{
			cdrom_read_data(m_cdrom->get_cdrom_file(), m_cur_sector - 150 - SPG290_LEADIN_LEN, cdbuf, CD_TRACK_MODE1_RAW);

			// FIXME: this is required for load iso images
			if (cdrom_get_track_type(m_cdrom->get_cdrom_file(), m_qsub[m_cur_sector * 12 + 1] - 1) == CD_TRACK_MODE1)
			{
				int lba = (bcd_2_dec(cdbuf[12]) * 60 + bcd_2_dec(cdbuf[13])) * 75 + bcd_2_dec(cdbuf[14]);
				uint32_t msf = lba_to_msf(lba + 150);
				cdbuf[12] = (msf >> 16) & 0xff;
				cdbuf[13] = (msf >> 8) & 0xff;
				cdbuf[14] = (msf >> 0) & 0xff;
			}

			for (int i=0; i<m_sector_size; i++)
			{
				m_space_write_cb(m_buf_ptr++, cdbuf[i]);
				if (m_buf_ptr > m_buf_end)
					m_buf_ptr = m_buf_start;
			}
		}

		m_seek_lba++;
		m_frame_found = true;
		m_irq_cb(ASSERT_LINE);
	}

	if ((m_cur_sector < m_seek_lba + SPG290_LEADIN_LEN) && (m_cur_sector < m_tot_sectors))
		m_cur_sector++;
}

uint32_t spg290_cdservo_device::read(offs_t offset, uint32_t mem_mask)
{
	switch (offset << 2)
	{
	case 0x08:
		return m_data;  // CD servo DSP reply
	case 0x0c:
		return 0;       // CD servo ready
	case 0x40:
		return m_control0;
	case 0x44:
		return m_control1;
	case 0x48:
		return m_seek_min;
	case 0x4c:
		return m_seek_sec;
	case 0x50:
		return m_seek_frm;
	case 0x54:
		return 0;       // sector CRC error flags
	case 0x60:
		return m_buf_start;
	case 0x64:
		return m_buf_end;
	case 0x68:
		return m_buf_ptr;
	case 0x6c:
		return m_sector_size;
	default:
		logerror("[%s] %s: unknown read %x\n", tag(), machine().describe_context(), offset << 2);
	}

	return 0;
}

void spg290_cdservo_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset << 2)
	{
	case 0x04:              // CD servo DSP command
		COMBINE_DATA(&m_addr);
		break;
	case 0x08:              // CD servo DSP data
		COMBINE_DATA(&m_data);
		break;
	case 0x0c:              // CD servo DSP exec
		if (ACCESSING_BITS_0_7)
		{
			if (data & 1)
				servo_cmd_r();
			else
				servo_cmd_w();
		}
		break;
	case 0x40:
		COMBINE_DATA(&m_control0);
		break;
	case 0x44:
		COMBINE_DATA(&m_control1);
		m_seek_lba = (bcd_2_dec(m_seek_min) * 60 + bcd_2_dec(m_seek_sec)) * 75 + bcd_2_dec(m_seek_frm);
		m_cur_sector = SPG290_LEADIN_LEN + m_seek_lba;  // TODO: seek time
		break;
	case 0x48:
		if (ACCESSING_BITS_0_7)
			m_seek_min = data;
		break;
	case 0x4c:
		if (ACCESSING_BITS_0_7)
			m_seek_sec = data;
		break;
	case 0x50:
		if (ACCESSING_BITS_0_7)
			m_seek_frm = data;
		break;
	case 0x60:
		COMBINE_DATA(&m_buf_start);
		break;
	case 0x54:
		break;
	case 0x64:
		COMBINE_DATA(&m_buf_end);
		break;
	case 0x68:
		COMBINE_DATA(&m_buf_ptr);
		break;
	case 0x6c:
		COMBINE_DATA(&m_sector_size);
		break;
	default:
		logerror("[%s] %s: unknown write %x = %x\n", tag(), machine().describe_context(), offset << 2, data);
	}
}

void spg290_cdservo_device::servo_cmd_r()
{
	if (m_addr == 0x079)
		m_data = 0xe3;          // servo status
	else if (m_addr == 0x07b)
		m_data = 0x06;          // unknown, should be in the range 4-7
	else if (m_addr == 0x07c)
		m_data = m_dsp_data >> 8;
	else if (m_addr == 0x07d)
		m_data = m_dsp_data;
	else if (m_addr == 0x082)
		m_data = 0x04;
	else if (m_addr == 0x083)
		m_data = machine().rand() & 0xff;
	else if (m_addr == 0x307)
		m_data = m_frame_found;
	else if (m_addr >= 0x340 && m_addr <= 0x349)    // read Q subchannel
		m_data = m_qsub[m_cur_sector * 12 + (m_addr & 0x0f)];
	else if (m_addr == 0x34c)
		m_data = 0x40;          // Q subchannel ready
	else if (m_addr  == 0x506)
		m_data = 0x04;          // DSP ready
	else if (m_addr  >= 0x500 && m_addr < 0x50a)
		m_data = m_dsp_regs[m_addr & 0x0f];
	else if (m_addr  == 0x41f)
		m_data = 0xff;          // if zero cause servo timeout
	else
		m_data = 0;
}


void spg290_cdservo_device::servo_cmd_w()
{
	if (m_addr == 0x013)    // skip n sectors
	{
		if (m_data & 0x01)
			m_cur_sector += (m_data & 0x10) ? -m_skip : m_skip;

		if (m_cur_sector < 0)                   m_cur_sector = 0;
		if (m_cur_sector >= m_tot_sectors)      m_cur_sector = m_tot_sectors - 1;
	}
	else if (m_addr == 0x020)       // speed
	{
		if (m_speed != 1 << (m_data & 0x03))
		{
			m_speed = 1 << (m_data & 0x03);
			change_status();
		}
	}
	else if (m_addr == 0x030)       // Disc ID
	{
		// < 10 no disc
		// < 97 CD-RW
		// >= 97 CD-ROM
		if (m_data == 0x13)
			m_dsp_data = 8;
		else
			m_dsp_data = m_qsub ? 100 : 0;
	}
	else if (m_addr == 0x032)       // DSP version
		m_dsp_data = 0x0102;
	else if (m_addr == 0x19a)
		m_skip = (m_skip & 0x00ff) | (m_data << 8);
	else if (m_addr == 0x29a)
		m_skip = (m_skip & 0xff00) | m_data;
	else if (m_addr == 0x307)
		m_frame_found = false;
	else if (m_addr == 0x505)               // DSP memory
	{
		if ((m_data & 0x0f) == 0x02)    // write
		{
			// maincpu upload the DSP code in 3 byte chunks
			uint16_t addr = (m_dsp_regs[0] << 8) | m_dsp_regs[1];
			m_dsp_memory[addr] = (m_dsp_regs[2] << 16) | (m_dsp_regs[3] << 8) | m_dsp_regs[4];
		}
		else if ((m_data & 0x0f) == 0x03)       // read
		{
			uint16_t addr = (m_dsp_regs[0] << 8) | m_dsp_regs[1];
			m_dsp_regs[7] = (m_dsp_memory[addr] >> 16) & 0xff;
			m_dsp_regs[8] = (m_dsp_memory[addr] >> 8) & 0xff;
			m_dsp_regs[9] = (m_dsp_memory[addr] >> 0) & 0xff;
		}
	}
	else if (m_addr  >= 0x500 && m_addr < 0x50a)
		m_dsp_regs[m_addr & 0x0f] = m_data;
}


void spg290_cdservo_device::change_status()
{
	if (m_speed == 0 || !m_cdrom.found() || !m_cdrom->get_cdrom_file())
		m_cdtimer->adjust(attotime::never);
	else
		m_cdtimer->adjust(attotime::from_hz(75 * m_speed), 0, attotime::from_hz(75 * m_speed));
}

void spg290_cdservo_device::add_qsub(int sector, uint8_t addrctrl, uint8_t track, uint8_t index, uint32_t rel_msf, uint32_t abs_msf, uint16_t crc16)
{
	uint8_t *dest = m_qsub.get() + sector * 12;
	dest[0] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
	dest[1] = track;
	dest[2] = index;
	dest[3] = rel_msf >> 16;
	dest[4] = rel_msf >> 8;
	dest[5] = rel_msf;
	dest[6] = 0;
	dest[7] = abs_msf >> 16;
	dest[8] = abs_msf >> 8;
	dest[9] = abs_msf;
	dest[10] = crc16 >> 8;
	dest[11] = crc16;
}

void spg290_cdservo_device::generate_qsub(cdrom_file *cdrom)
{
	const cdrom_toc *toc = cdrom_get_toc(cdrom);
	int numtracks = cdrom_get_last_track(cdrom);
	uint32_t total_sectors = cdrom_get_track_start(cdrom, numtracks - 1) + toc->tracks[numtracks - 1].frames + 150;

	m_tot_sectors = SPG290_LEADIN_LEN + total_sectors + SPG290_LEADOUT_LEN;
	m_qsub = std::make_unique<uint8_t[]>(m_tot_sectors * 12);

	int lba = 0;

	// 7500 sectors lead-in
	for (int s=0; s < SPG290_LEADIN_LEN; s += numtracks + 3)
	{
		if (lba < SPG290_LEADIN_LEN)     add_qsub(lba++, 0x14, 0, 0xa0, lba_to_msf(s + 0), 1 << 16);                      // first track number
		if (lba < SPG290_LEADIN_LEN)     add_qsub(lba++, 0x14, 0, 0xa1, lba_to_msf(s + 1), numtracks << 16);              // last track number
		if (lba < SPG290_LEADIN_LEN)     add_qsub(lba++, 0x14, 0, 0xa2, lba_to_msf(s + 2), lba_to_msf(total_sectors));    // start time of lead-out

		for(int track = 0; track < numtracks; track++)
		{
			uint32_t track_start = cdrom_get_track_start(cdrom, track) + 150;
			if (lba < SPG290_LEADIN_LEN)
				add_qsub(lba++, cdrom_get_adr_control(cdrom, track), 0, dec_2_bcd(track + 1), lba_to_msf(s + 3 + track), lba_to_msf(track_start));
		}
	}

	// data tracks
	for(int track = 0; track < numtracks; track++)
	{
		uint32_t control = cdrom_get_adr_control(cdrom, track);
		uint32_t track_start = cdrom_get_track_start(cdrom, track);

		// pregap
		uint32_t pregap = toc->tracks[track].pregap;

		// first track should have a 150 frames pregap
		if (track == 0 && toc->tracks[0].pregap == 0)
			pregap = 150;
		else if (track != 0 && toc->tracks[0].pregap == 0)
			track_start += 150;

		for(int s = 0; s < pregap; s++)
			add_qsub(lba++, control, dec_2_bcd(track + 1), 0, lba_to_msf(s), lba_to_msf(track_start + s));

		track_start += pregap;

		for(int s = 0; s < toc->tracks[track].frames; s++)
		{
			// TODO: if present use subcode from CHD
			add_qsub(lba++, control, dec_2_bcd(track + 1), 1, lba_to_msf(s), lba_to_msf(track_start + s));
		}

		track_start += toc->tracks[track].frames;

		// postgap
		for(int s = 0; s < toc->tracks[track].postgap; s++)
			add_qsub(lba++, control, dec_2_bcd(track + 1), 2, lba_to_msf(s), lba_to_msf(track_start + s));

		track_start += toc->tracks[track].postgap;
	}

	// 6750 sectors lead-out
	for(int s = 0; s < SPG290_LEADOUT_LEN; s++)
		add_qsub(lba++, 0x14, 0xaa, 1, lba_to_msf(s), lba_to_msf(total_sectors + s));
}
