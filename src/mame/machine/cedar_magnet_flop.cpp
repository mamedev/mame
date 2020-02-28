// license:BSD-3-Clause
// copyright-holders:David Haywood

// todo, scrap this and use the core SAB 2797B emulation
// (FM - MFM type) @ 1.25mhz (oscillates 1.250~1.251)


#include "emu.h"
#include "cedar_magnet_flop.h"

DEFINE_DEVICE_TYPE(CEDAR_MAGNET_FLOP, cedar_magnet_flop_device, "cedmag_flop", "Cedar Floppy Simulation")

cedar_magnet_flop_device::cedar_magnet_flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CEDAR_MAGNET_FLOP, tag, owner, clock)
	, m_disk(*this, "disk")
{
}


void cedar_magnet_flop_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "floppy_nvram", nvram_device::DEFAULT_NONE);
}


void cedar_magnet_flop_device::device_start()
{
	subdevice<nvram_device>("floppy_nvram")->set_base(memregion("disk")->base(), 0xf0000);
}

void cedar_magnet_flop_device::device_reset()
{
	m_flopdat = 0;
	m_flopcmd = 0;
	m_flopsec = 0;
	m_flopstat = 0;
	m_floptrk = 0;
}

u8 cedar_magnet_flop_device::port60_r()
{
	u8 ret = m_flopstat;
	return ret;
}

u8 cedar_magnet_flop_device::port61_r()
{
	u8 ret = m_curtrack;
	return ret;
}

u8 cedar_magnet_flop_device::port63_r()
{
	u8 ret = machine().rand();

	// printf("%s: port63_r (DATA) (%02x)\n", machine().describe_context().c_str(), ret);

	if ((m_flopcmd&0xf0) == 0x90) // reading data
	{
		int side = (m_flopcmd & 0x02)>>1;
		int read_offset_base = (m_flopsec * 0x400) + (m_curtrack * 0x3000) + (side * 0x1800);

		int sector_size = 1024;

		if (m_secoffs < sector_size)
		{
			m_flopstat |= 0x05;

			int read_offset = read_offset_base + m_secoffs;
			ret = m_disk[read_offset];

			if (m_secoffs == 0)
			{
				// this is weird data, protection??
				if(read_offset_base==0xea400)
					printf("reading sector %d offset %d (from disk image at %04x) (cur track %02x cur side %02x cur sector %02x)\n", m_flopsec, m_secoffs, read_offset, m_curtrack, side, m_flopsec);
			}

			m_secoffs++;

			if (m_secoffs == sector_size)
			{
				//printf("finished sector read\n");
				m_flopstat &= ~0x05;

				m_secoffs = 0;
				m_flopsec++;
			}
		}
		else
		{
			printf("read past sector!! %d\n", m_secoffs);
			m_secoffs++;
		}

	}
	else
	{
		fatalerror("read data in non-read mode?\n");
	}
//
	return ret;
}

void cedar_magnet_flop_device::port60_w(u8 data)
{
	//printf("%s: port60_w (COMMAND) %02x\n", machine().describe_context().c_str(), data);
	m_flopcmd = data;


	switch (m_flopcmd & 0xf0)
	{
	case 0x00:
		//printf("restore\n");
		m_flopstat = 0x06;
		m_curtrack = 0x00;
		break;

	case 0x10:
		//printf("seek track\n");
		m_curtrack = m_flopdat;
		break;


	case 0x90:
		//printf("read sector\n");
		m_flopstat |= 0x07;
		m_secoffs = 0;
		break;

	case 0xb0:
		//printf("write sector\n");
		m_flopstat |= 0x07;
		m_secoffs = 0;
		break;

	case 0xd0:
		//printf("force interrupt?\n");
	//  m_flopstat = 0x06;
	//  m_flopstat &= ~0x07;
	//  m_maincpu->set_input_line(0, HOLD_LINE);

		break;


	default:
		printf("unhandled disk command %02x\n", m_flopcmd);
	}

}

void cedar_magnet_flop_device::port62_w(u8 data)
{
	//printf("%s: port62_w (SECTOR) %02x\n", machine().describe_context().c_str(), data);
	m_flopsec = data;

	if (m_flopsec < 200)
	{
		printf("sector specified <200!\n");
	}

	m_flopsec -= 200;
}

void cedar_magnet_flop_device::port63_w(u8 data)
{
	//printf("%s: port63_w (DATA) %02x\n", machine().describe_context().c_str(), data);
	m_flopdat = data;

	if ((m_flopcmd & 0xf0) == 0xb0) // writing data
	{
		int side = (m_flopcmd & 0x02)>>1;
		int read_offset_base = (m_flopsec * 0x400) + (m_curtrack * 0x3000) + (side * 0x1800);

		int sector_size = 1024;

		if (m_secoffs < sector_size)
		{
			m_flopstat |= 0x05;

			int read_offset = read_offset_base + m_secoffs;
			m_disk[read_offset] = data;

			if (m_secoffs == 0)
			{
				printf("writing sector %d offset %d (from disk image at %04x) (cur track %02x cur side %02x cur sector %02x)\n", m_flopsec, m_secoffs, read_offset, m_curtrack, side, m_flopsec);
			}

			m_secoffs++;

			if (m_secoffs == sector_size)
			{
				//printf("finished sector read\n");
				m_flopstat &= ~0x05;

				m_secoffs = 0;
				m_flopsec++;
			}
		}
	}

}

WRITE8_MEMBER(cedar_magnet_flop_device::write)
{
	switch (offset & 3)
	{
		case 0x00:port60_w(data);break;
	//  case 0x01:port61_w(space, offset, data);break;
		case 0x02:port62_w(data);break;
		case 0x03:port63_w(data);break;
		default:break;
	}
}

READ8_MEMBER(cedar_magnet_flop_device::read)
{
	switch (offset & 3)
	{
		case 0x00: return port60_r();
		case 0x01: return port61_r();
		//case 0x02: return port62_r(space, offset);
		case 0x03: return port63_r();
		default: return 0x00;
	}

	return 0x00;
}
