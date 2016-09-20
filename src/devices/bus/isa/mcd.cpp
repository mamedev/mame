// license:BSD-3-Clause
// copyright-holders:Carl

#include "mcd.h"
#include "coreutil.h"

DEVICE_ADDRESS_MAP_START(map, 16, mcd_isa_device)
	AM_RANGE(0x0, 0x1) AM_READWRITE8(data_r, cmd_w, 0x00ff)
	AM_RANGE(0x0, 0x1) AM_READWRITE8(flag_r, reset_w, 0xff00)
ADDRESS_MAP_END

static INPUT_PORTS_START( ide )
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_MCD = &device_creator<mcd_isa_device>;

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mcd_isa_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ide );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mcd_isa_device - constructor
//-------------------------------------------------

mcd_isa_device::mcd_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		cdrom_image_device(mconfig, ISA16_MCD, "Mitsumi ISA CDROM Adapter", tag, owner, clock, "mcd_isa", __FILE__),
		device_isa16_card_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mcd_isa_device::device_start()
{
	cdrom_image_device::device_start();
	set_isa_device();
	m_isa->install_device(0x0310, 0x0311, *this, &mcd_isa_device::map, 16);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mcd_isa_device::device_reset()
{
	m_data = false;
	m_stat = m_cdrom_handle ? STAT_READY : 0;
	m_flag = FLAG_STAT;
	m_rd_count = 0;
	m_buf_count = 0;
	m_curtoctrk = 1;
	m_dma = 0;
	m_irq = 0;
	m_locked = false;
}

READ8_MEMBER(mcd_isa_device::flag_r)
{
	UINT8 ret = m_flag;
	m_flag = 0;
	return ret;
}

READ8_MEMBER(mcd_isa_device::data_r)
{
	if(!m_data)
	{
		if(m_buf_count)
		{
			m_flag = FLAG_DATA;
			m_data = true;
		}
		return m_stat;
	}
	else if(m_buf_count)
	{
		UINT8 ret = m_buf[m_buf_idx++];
		m_buf_count--;
		if(m_buf_count)
			m_flag = FLAG_DATA;
		return ret;
	}
	return 0;
}

WRITE8_MEMBER(mcd_isa_device::reset_w)
{
	reset();
}

WRITE8_MEMBER(mcd_isa_device::cmd_w)
{
	if(m_rd_count)
	{
		m_rd_count--;
		switch(m_cmd)
		{
			case CMD_SET_MODE:
				m_mode = data;
				m_buf[0] = 0;
				m_buf_count = 1;
				break;
			case CMD_LOCK:
				m_locked = data & 1 ? true : false;
				m_buf[0] = 0;
				m_buf[1] = 0;
				m_buf_count = 2;
				break;
			case CMD_CONFIG:
				switch(m_rd_count)
				{
					case 1:
						m_conf = data;
						break;
					case 0:
						switch(m_conf)
						{
							case 0x10:
								m_irq = data;
								break;
							case 0x02:
								m_dma = data;
								break;
						}
						m_buf[0] = 0;
						m_buf_count = 1;
						break;
				}
		}
		if(!m_rd_count)
		{
			m_data = false;
			m_flag = FLAG_STAT;
			m_stat = m_cdrom_handle ? STAT_READY : 0;
		}
		return;
	}
	m_cmd = data;
	m_buf_idx = 0;
	m_rd_count = 0;
	m_buf_count = 0;
	m_stat = m_cdrom_handle ? STAT_READY : 0;
	switch(data)
	{
		case CMD_GET_INFO:
			if(m_cdrom_handle)
			{
				UINT32 first = lba_to_msf(150), last = lba_to_msf(cdrom_get_track_start(m_cdrom_handle, 0xaa));
				m_buf[0] = 1;
				m_buf[1] = dec_2_bcd(cdrom_get_last_track(m_cdrom_handle));
				m_buf[2] = dec_2_bcd((first >> 16) & 0xff);
				m_buf[3] = dec_2_bcd((first >> 8) & 0xff);
				m_buf[4] = dec_2_bcd(first & 0xff);
				m_buf[5] = dec_2_bcd((last >> 16) & 0xff);
				m_buf[6] = dec_2_bcd((last >> 8) & 0xff);
				m_buf[7] = dec_2_bcd(last & 0xff);
				m_buf[8] = 0;
				m_buf_count = 9;
			}
			else
			{
				m_buf_count = 1;
				m_buf[0] = 0;
				m_stat = STAT_CMD_CHECK;
			}
			break;
		case CMD_GET_Q:
			if(m_cdrom_handle)
			{
				int tracks = cdrom_get_last_track(m_cdrom_handle);
				UINT32 start = lba_to_msf(cdrom_get_track_start(m_cdrom_handle, m_curtoctrk));
				UINT32 end = lba_to_msf(cdrom_get_track_start(m_cdrom_handle, m_curtoctrk < tracks ? m_curtoctrk + 1 : 0xaa));
				m_buf[0] = 1; // track type?
				m_buf[1] = 0; // track num except when reading toc
				m_buf[2] = dec_2_bcd(m_curtoctrk); // index
				m_buf[3] = dec_2_bcd((start >> 16) & 0xff);
				m_buf[4] = dec_2_bcd((start >> 8) & 0xff);
				m_buf[5] = dec_2_bcd(start & 0xff);
				m_buf[6] = 0;
				m_buf[7] = dec_2_bcd((end >> 16) & 0xff);
				m_buf[8] = dec_2_bcd((end >> 8) & 0xff);
				m_buf[9] = dec_2_bcd(end & 0xff);
				if(m_curtoctrk >= tracks)
					m_curtoctrk = 1;
				m_buf_count = 10;
			}
			else
				m_stat = STAT_CMD_CHECK;
			break;
		case CMD_GET_STAT:
			m_buf[0] = m_stat;
			m_buf_count = 1;
			break;
		case CMD_SET_MODE:
			m_rd_count = 1;
			break;
		case CMD_STOPCDDA:
		case CMD_STOP:
			m_drvmode = DRV_MODE_STOP;
			m_curtoctrk = 1;
			break;
		case CMD_CONFIG:
			m_rd_count = 2;
			break;
		case CMD_READ:
			m_drvmode = DRV_MODE_READ;
			break;
		case CMD_READCDDA:
			m_drvmode = DRV_MODE_CDDA;
			break;
		case CMD_GET_VER:
			m_buf[0] = 1; // ?
			m_buf[1] = 'D';
			m_buf[2] = 0;
			m_buf_count = 3;
			break;
		case CMD_EJECT:
			break;
		case CMD_LOCK:
			m_rd_count = 1;
			break;
		default:
			m_stat |= STAT_CMD_CHECK;
			break;
	}
	if(!m_rd_count)
	{
		m_data = false;
		m_flag = FLAG_STAT;
	}
}
