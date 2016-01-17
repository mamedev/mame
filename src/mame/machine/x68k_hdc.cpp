// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    X68000 custom SASI Hard Disk controller

 0xe96001 (R/W) - SASI data I/O
 0xe96003 (W)   - SEL signal high (0)
 0xe96003 (R)   - SASI status
                  - bit 4 = MSG - if 1, content of data line is a message
                  - bit 3 = Command / Data - if 1, content of data line is a command or status, otherwise it is data.
                  - bit 2 = I/O - if 0, Host -> Controller (Output), otherwise Controller -> Host (Input).
                  - bit 1 = BSY - if 1, HD is busy.
                  - bit 0 = REQ - if 1, host is demanding data transfer to the host.
 0xe96005 (W/O) - data is arbitrary (?)
 0xe96007 (W/O) - SEL signal low (1)

*/

#include "x68k_hdc.h"
#include "imagedev/harddriv.h"
#include "image.h"

const device_type X68KHDC = &device_creator<x68k_hdc_image_device>;

x68k_hdc_image_device::x68k_hdc_image_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, X68KHDC, "SASI Hard Disk", tag, owner, clock, "x68k_hdc_image", __FILE__),
		device_image_interface(mconfig, *this)
{
}

void x68k_hdc_image_device::device_config_complete()
{
	update_names(X68KHDC, "sasihd", "sasi");
}

void x68k_hdc_image_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_req = 1;
	m_status_port |= 0x01;
}

void x68k_hdc_image_device::device_start()
{
	m_status = 0x00;
	m_status_port = 0x00;
	m_phase = SASI_PHASE_BUSFREE;
}

bool x68k_hdc_image_device::call_create(int format_type, option_resolution *format_options)
{
	// create 20MB HD
	int x;
	int ret;
	unsigned char sectordata[256];  // empty block data

	memset(sectordata,0,sizeof(sectordata));
	for(x=0;x<0x013c98;x++)  // 0x13c98 = number of blocks on a 20MB HD
	{
		ret = fwrite(sectordata,256);
		if(ret < 256)
			return IMAGE_INIT_FAIL;
	}

	return IMAGE_INIT_PASS;
}

WRITE16_MEMBER( x68k_hdc_image_device::hdc_w )
{
	unsigned int lba = 0;
	std::vector<char> blk;
	switch(offset)
	{
	case 0x00:  // data I/O
		if(m_phase == SASI_PHASE_WRITE)
		{
			if(m_transfer_byte_count == 0)
			{
				switch(m_command[0])
				{
				case SASI_CMD_SPECIFY:
					m_transfer_byte_total = 10;
					break;
				case SASI_CMD_WRITE:
					m_transfer_byte_total = (0x100 * m_command[4]);
					break;
				default:
					m_transfer_byte_total = 0x100;
				}
			}

			if(m_command[0] == SASI_CMD_SPECIFY)
			{
				logerror("SPECIFY: wrote 0x%02x\n",data);
			}

			if(m_command[0] == SASI_CMD_WRITE)
			{
				if(!exists())
				{
					m_phase = SASI_PHASE_STATUS;
					m_io = 1;  // Output
					m_status_port |= 0x04;  // C/D remains the same
					m_status = 0x02;
					logerror("SASI: No HD connected.\n");
				}
				else
				{
					fwrite(&data,1);
				}
			}

			m_req = 0;
			m_status_port &= ~0x01;
			timer_set(attotime::from_nsec(450));
			m_transfer_byte_count++;
			if(m_transfer_byte_count >= m_transfer_byte_total)
			{
				// End of transfer
				m_phase = SASI_PHASE_STATUS;
				m_io = 1;
				m_status_port |= 0x04;
				m_cd = 1;
				m_status_port |= 0x08;
				logerror("SASI: Write transfer complete\n");
			}
		}
		if(m_phase == SASI_PHASE_COMMAND)
		{
			if(m_command_byte_count == 0)
			{
				// first command byte
				m_current_command = data;
				switch(data >> 5)  // high 3 bits determine command class, and therefore, length
				{
				case 0:
					m_command_byte_total = 6;
					break;
				case 1:
					m_command_byte_total = 10;
					break;
				case 2:
					m_command_byte_total = 8;
					break;
				default:
					m_command_byte_total = 6;
				}
			}
			m_command[m_command_byte_count] = data;
			// reset REQ temporarily
			m_req = 0;
			m_status_port &= ~0x01;
			timer_set(attotime::from_nsec(450));

			m_command_byte_count++;
			if(m_command_byte_count >= m_command_byte_total)
			{
				// End of command

				switch(m_command[0])
				{
				case SASI_CMD_REZERO_UNIT:
					m_phase = SASI_PHASE_STATUS;
					m_io = 1;  // Output
					m_status_port |= 0x04;  // C/D remains the same
					logerror("SASI: REZERO UNIT\n");
					break;
				case SASI_CMD_REQUEST_SENSE:
					m_phase = SASI_PHASE_READ;
					m_io = 1;
					m_status_port |= 0x04;
					m_cd = 0;
					m_status_port &= ~0x08;
					m_transfer_byte_count = 0;
					m_transfer_byte_total = 0;
					logerror("SASI: REQUEST SENSE\n");
					break;
				case SASI_CMD_SPECIFY:
					m_phase = SASI_PHASE_WRITE;
					m_io = 0;
					m_status_port &= ~0x04;
					m_cd = 0;  // Data
					m_status_port &= ~0x08;
					m_transfer_byte_count = 0;
					m_transfer_byte_total = 0;
					logerror("SASI: SPECIFY\n");
					break;
				case SASI_CMD_READ:
					if(!exists())
					{
						m_phase = SASI_PHASE_STATUS;
						m_io = 1;  // Output
						m_status_port |= 0x04;  // C/D remains the same
						m_cd = 1;
						m_status_port |= 0x08;
						m_status = 0x02;
						logerror("SASI: No HD connected\n");
					}
					else
					{
						m_phase = SASI_PHASE_READ;
						m_io = 1;
						m_status_port |= 0x04;
						m_cd = 0;
						m_status_port &= ~0x08;
						m_transfer_byte_count = 0;
						m_transfer_byte_total = 0;
						lba = m_command[3];
						lba |= m_command[2] << 8;
						lba |= (m_command[1] & 0x1f) << 16;
						fseek(lba * 256,SEEK_SET);
						logerror("SASI: READ (LBA 0x%06x, blocks = %i)\n",lba,m_command[4]);
					}
					break;
				case SASI_CMD_WRITE:
					if(!exists())
					{
						m_phase = SASI_PHASE_STATUS;
						m_io = 1;  // Output
						m_status_port |= 0x04;  // C/D remains the same
						m_cd = 1;
						m_status_port |= 0x08;
						m_status = 0x02;
						logerror("SASI: No HD connected\n");
					}
					else
					{
						m_phase = SASI_PHASE_WRITE;
						m_io = 0;
						m_status_port &= ~0x04;
						m_cd = 0;
						m_status_port &= ~0x08;
						m_transfer_byte_count = 0;
						m_transfer_byte_total = 0;
						lba = m_command[3];
						lba |= m_command[2] << 8;
						lba |= (m_command[1] & 0x1f) << 16;
						fseek(lba * 256,SEEK_SET);
						logerror("SASI: WRITE (LBA 0x%06x, blocks = %i)\n",lba,m_command[4]);
					}
					break;
				case SASI_CMD_SEEK:
						m_phase = SASI_PHASE_STATUS;
						m_io = 1;  // Output
						m_status_port |= 0x04;  // C/D remains the same
						m_cd = 1;
						m_status_port |= 0x08;
						logerror("SASI: SEEK (LBA 0x%06x)\n",lba);
					break;
				case SASI_CMD_FORMAT_UNIT:
				case SASI_CMD_FORMAT_UNIT_06:
					/*
					    Format Unit command format  (differs from SASI spec?)
					    0 |   0x06
					    1 |   Unit number (0-7) | LBA MSB (high 5 bits)
					    2 |   LBA
					    3 |   LBA LSB
					    4 |   ??  (usually 0x01)
					    5 |   ??
					*/
						m_phase = SASI_PHASE_STATUS;
						m_io = 1;  // Output
						m_status_port |= 0x04;  // C/D remains the same
						m_cd = 1;
						m_status_port |= 0x08;
						lba = m_command[3];
						lba |= m_command[2] << 8;
						lba |= (m_command[1] & 0x1f) << 16;
						fseek(lba * 256,SEEK_SET);
						blk.resize(256*33);
						memset(&blk[0], 0, 256*33);
						// formats 33 256-byte blocks
						fwrite(&blk[0],256*33);
						logerror("SASI: FORMAT UNIT (LBA 0x%06x)\n",lba);
					break;
				default:
					m_phase = SASI_PHASE_STATUS;
					m_io = 1;  // Output
					m_status_port |= 0x04;  // C/D remains the same
					m_status = 0x02;
					logerror("SASI: Invalid or unimplemented SASI command (0x%02x) received.\n",m_command[0]);
				}
			}
		}
		break;
	case 0x01:
		if(data == 0)
		{
			if(m_phase == SASI_PHASE_SELECTION)
			{
				// Go to Command phase
				m_phase = SASI_PHASE_COMMAND;
				m_cd = 1;   // data port expects a command or status
				m_status_port |= 0x08;
				m_command_byte_count = 0;
				m_command_byte_total = 0;
				timer_set(attotime::from_nsec(45));
			}
		}
		break;
	case 0x02:
		break;
	case 0x03:
		if(data != 0)
		{
			if(m_phase == SASI_PHASE_BUSFREE)
			{
				// Go to Selection phase
				m_phase = SASI_PHASE_SELECTION;
				m_bsy = 1;  // HDC is now busy
				m_status_port |= 0x02;
			}
		}
		break;
	}

//  logerror("SASI: write to HDC, offset %04x, data %04x\n",offset,data);
}

READ16_MEMBER( x68k_hdc_image_device::hdc_r )
{
	int retval = 0xff;

	switch(offset)
	{
	case 0x00:
		if(m_phase == SASI_PHASE_MESSAGE)
		{
			m_phase = SASI_PHASE_BUSFREE;
			m_msg = 0;
			m_cd = 0;
			m_io = 0;
			m_bsy = 0;
			m_req = 0;
			m_status = 0;
			m_status_port = 0;  // reset all status bits to 0
			return 0x00;
		}
		if(m_phase == SASI_PHASE_STATUS)
		{
			m_phase = SASI_PHASE_MESSAGE;
			m_msg = 1;
			m_status_port |= 0x10;
			// reset REQ temporarily
			m_req = 0;
			m_status_port &= ~0x01;
			timer_set(attotime::from_nsec(450));

			return m_status;
		}
		if(m_phase == SASI_PHASE_READ)
		{
			if(m_transfer_byte_count == 0)
			{
				switch(m_command[0])
				{
				case SASI_CMD_REQUEST_SENSE:
					// set up sense bytes
					m_sense[0] = 0x01;  // "No index signal"
					m_sense[1] = 0;
					m_sense[2] = 0;
					m_sense[3] = 0;
					if(m_command[3] == 0)
						m_transfer_byte_total = 4;
					else
						m_transfer_byte_total = m_command[3];
					break;
				case SASI_CMD_READ:
					m_transfer_byte_total = (0x100 * m_command[4]);
					m_transfer_byte_count = 0;
					break;
				default:
					m_transfer_byte_total = 0;
				}
			}

			switch(m_command[0])
			{
			case SASI_CMD_REQUEST_SENSE:
				retval = m_sense[m_transfer_byte_count];
				logerror("REQUEST SENSE: read value 0x%02x\n",retval);
				break;
			case SASI_CMD_READ:
				if(!exists())
				{
					m_phase = SASI_PHASE_STATUS;
					m_io = 1;  // Output
					m_status_port |= 0x04;  // C/D remains the same
					m_status = 0x02;
					logerror("SASI: No HD connected.\n");
				}
				else
				{
					unsigned char val;
					fread(&val,1);
					retval = val;
				}
				break;
			default:
				retval = 0;
			}

			m_req = 0;
			m_status_port &= ~0x01;
			timer_set(attotime::from_nsec(450));
			m_transfer_byte_count++;
			if(m_transfer_byte_count >= m_transfer_byte_total)
			{
				// End of transfer
				m_phase = SASI_PHASE_STATUS;
				m_io = 1;
				m_status_port |= 0x04;
				m_cd = 1;
				m_status_port |= 0x08;
				logerror("SASI: Read transfer complete\n");
			}

			return retval;
		}
		return 0x00;
	case 0x01:
//      logerror("SASI: [%08x] read from status port, read 0x%02x\n",activecpu_get_pc(),m_status_port);
		return m_status_port;
	case 0x02:
		return 0xff;  // write-only
	case 0x03:
		return 0xff;  // write-only
	default:
		return 0xff;
	}
}
