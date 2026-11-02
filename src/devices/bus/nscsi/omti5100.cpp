// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
/*********************************************************************

    OMTI 5100 SASI Winchester disk controller

    Two drives, addressed as LUN 0 and 1.  The Z8 firmware is not
    dumped; the controller is emulated at the SASI command level.

*********************************************************************/

#include "emu.h"
#include "omti5100.h"

#include "multibyte.h"

#include <algorithm>

#define LOG_COMMAND (1U << 1)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_OMTI5100, nscsi_omti5100_device, "scsi_omti5100", "OMTI 5100 Winchester Disk Controller")

nscsi_omti5100_device::nscsi_omti5100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_full_device(mconfig, NSCSI_OMTI5100, tag, owner, clock),
	m_image(*this, "image%u", 0U),
	m_heads{ 1, 1 },
	m_sectors{ 1, 1 },
	m_param{ },
	m_lun(0),
	m_lba(0),
	m_block{ }
{
}

void nscsi_omti5100_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, m_image[0]).set_interface("scsi_hdd,hdd");
	HARDDISK(config, m_image[1]).set_interface("scsi_hdd,hdd");
}

void nscsi_omti5100_device::device_start()
{
	nscsi_full_device::device_start();

	save_item(NAME(m_heads));
	save_item(NAME(m_sectors));
	save_item(NAME(m_param));
	save_item(NAME(m_lun));
	save_item(NAME(m_lba));
	save_item(NAME(m_cur_lba));
	save_item(NAME(m_block));
}

void nscsi_omti5100_device::device_reset()
{
	nscsi_full_device::device_reset();

	// with no drives attached the controller doesn't answer selection
	if (!m_image[0]->exists() && !m_image[1]->exists())
	{
		m_scsi_id = -1;
	}

	// until ASSIGN DISK PARAMETERS, use the image geometry
	for (int lun = 0; lun < 2; lun++)
	{
		if (m_image[lun]->exists())
		{
			const auto &info = m_image[lun]->get_info();
			m_heads[lun] = std::max<uint32_t>(info.heads, 1);
			m_sectors[lun] = std::max<uint32_t>(info.sectors, 1);
		}
	}

	// SASI sense, not the extended format the base sets up
	std::fill_n(m_scsi_sense_buffer, 4, 0);

	m_cur_lba = ~0U;
}

bool nscsi_omti5100_device::lun_ready(int lun) const
{
	return (lun < 2) && m_image[lun]->exists() && (m_image[lun]->get_info().sectorbytes <= sizeof(m_block));
}

// translate through the assigned geometry and check it against the drive
bool nscsi_omti5100_device::address_valid(uint32_t lba) const
{
	const auto &info = m_image[m_lun]->get_info();
	const uint32_t track = lba / m_sectors[m_lun];

	return ((track % m_heads[m_lun]) < info.heads) && ((track / m_heads[m_lun]) < info.cylinders);
}

void nscsi_omti5100_device::sasi_error(uint8_t code, bool valid, uint32_t lba)
{
	m_scsi_sense_buffer[0] = (valid ? 0x80 : 0x00) | code;
	m_scsi_sense_buffer[1] = (m_lun << 5) | ((lba >> 16) & 0x1f);
	m_scsi_sense_buffer[2] = (lba >> 8) & 0xff;
	m_scsi_sense_buffer[3] = lba & 0xff;
	scsi_status_complete(SS_CHECK_CONDITION);
}

void nscsi_omti5100_device::fill_blocks(uint32_t lba, uint32_t count)
{
	const auto &info = m_image[m_lun]->get_info();
	const uint32_t end = std::min(lba + count, info.cylinders * info.heads * info.sectors);

	std::fill_n(m_block, info.sectorbytes, 0xe5);
	for ( ; lba < end; lba++)
	{
		if (!m_image[m_lun]->write(lba, m_block))
		{
			LOG("write error at LBA %d\n", lba);
		}
	}
}

bool nscsi_omti5100_device::scsi_command_done(uint8_t command, uint8_t length)
{
	// the vendor-specific groups use 6-byte commands
	switch (command >> 5)
	{
		case 6:
		case 7:
			return length == 6;

		default:
			return nscsi_full_device::scsi_command_done(command, length);
	}
}

void nscsi_omti5100_device::scsi_command()
{
	const uint8_t command = m_scsi_cmdbuf[0];

	LOGMASKED(LOG_COMMAND, "command %02x %02x %02x %02x %02x %02x\n",
			m_scsi_cmdbuf[0], m_scsi_cmdbuf[1], m_scsi_cmdbuf[2],
			m_scsi_cmdbuf[3], m_scsi_cmdbuf[4], m_scsi_cmdbuf[5]);

	m_lun = get_lun(m_scsi_cmdbuf[1] >> 5);
	m_cur_lba = ~0U;

	if (command == SC_REQUEST_SENSE)
	{
		scsi_data_in(SBUF_SENSE, 4);
		scsi_status_complete(SS_GOOD);
		return;
	}

	std::fill_n(m_scsi_sense_buffer, 4, 0);

	if (!lun_ready(m_lun))
	{
		sasi_error(ERR_NOT_READY);
		return;
	}

	const auto &info = m_image[m_lun]->get_info();

	switch (command)
	{
		case SC_TEST_UNIT_READY:
		case SC_REZERO_UNIT:
		case SC_CHECK_TRACK_FORMAT:
			scsi_status_complete(SS_GOOD);
			break;

		case SC_FORMAT_UNIT:
			m_lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
			fill_blocks(m_lba, info.cylinders * info.heads * info.sectors);
			scsi_status_complete(SS_GOOD);
			break;

		case SC_FORMAT_TRACK:
			m_lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
			if (!address_valid(m_lba))
			{
				sasi_error(ERR_NO_SEEK_COMPLETE, true, m_lba);
			}
			else
			{
				fill_blocks(m_lba - (m_lba % m_sectors[m_lun]), m_sectors[m_lun]);
				scsi_status_complete(SS_GOOD);
			}
			break;

		case SC_READ_6:
		case SC_WRITE_6:
		case SC_SEEK_6:
		{
			m_lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
			const uint32_t blocks = (command == SC_SEEK_6) ? 1 : (m_scsi_cmdbuf[4] ? m_scsi_cmdbuf[4] : 256);

			if (!address_valid(m_lba) || !address_valid(m_lba + blocks - 1))
			{
				sasi_error(ERR_NO_SEEK_COMPLETE, true, m_lba);
				break;
			}

			if (command == SC_READ_6)
			{
				scsi_data_in(BUF_DISK, blocks * info.sectorbytes);
			}
			else if (command == SC_WRITE_6)
			{
				scsi_data_out(BUF_DISK, blocks * info.sectorbytes);
			}
			scsi_status_complete(SS_GOOD);
			break;
		}

		case SC_ASSIGN_DISK_PARAMETERS:
			scsi_data_out(BUF_PARAM, sizeof(m_param));
			scsi_status_complete(SS_GOOD);
			break;

		case SC_READ_DATA_BUFFER:
			scsi_data_in(BUF_BUFFER, 512);
			scsi_status_complete(SS_GOOD);
			break;

		default:
			logerror("command %02x unknown\n", command);
			sasi_error(ERR_INVALID_COMMAND);
			break;
	}
}

uint8_t nscsi_omti5100_device::scsi_get_data(int id, int pos)
{
	switch (id)
	{
		case BUF_DISK:
		{
			const int bytes = m_image[m_lun]->get_info().sectorbytes;
			const uint32_t lba = m_lba + pos / bytes;

			if (lba != m_cur_lba)
			{
				m_cur_lba = lba;
				if (!m_image[m_lun]->read(lba, m_block))
				{
					LOG("read error at LBA %d\n", lba);
					std::fill_n(m_block, bytes, 0);
				}
			}
			return m_block[pos % bytes];
		}

		case BUF_BUFFER:
		{
			static constexpr char ID[] = "5100";
			return (pos < 4) ? ID[pos] : 0;
		}

		default:
			return nscsi_full_device::scsi_get_data(id, pos);
	}
}

void nscsi_omti5100_device::scsi_put_data(int id, int pos, uint8_t data)
{
	switch (id)
	{
		case BUF_DISK:
		{
			const int bytes = m_image[m_lun]->get_info().sectorbytes;
			const int offset = pos % bytes;

			m_block[offset] = data;
			if (offset == bytes - 1)
			{
				m_cur_lba = m_lba + pos / bytes;
				if (!m_image[m_lun]->write(m_cur_lba, m_block))
				{
					LOG("write error at LBA %d\n", m_cur_lba);
				}
			}
			break;
		}

		case BUF_PARAM:
			m_param[pos] = data;
			if (pos == sizeof(m_param) - 1)
			{
				m_heads[m_lun] = m_param[3] + 1;
				if (m_param[8])
				{
					m_sectors[m_lun] = m_param[8] + 1;
				}
				else
				{
					// zero selects the standard format for the sector size
					switch (m_image[m_lun]->get_info().sectorbytes)
					{
						case 128:
							m_sectors[m_lun] = 53;
							break;

						case 256:
							m_sectors[m_lun] = 32;
							break;

						case 512:
							m_sectors[m_lun] = 18; // TODO: verify
							break;

						case 1024:
							m_sectors[m_lun] = 9;
							break;
					}
				}
				LOG("LUN %d: %d cylinders, %d heads, %d sectors/track\n", m_lun, get_u16be(&m_param[4]) + 1, m_heads[m_lun], m_sectors[m_lun]);
			}
			break;

		default:
			nscsi_full_device::scsi_put_data(id, pos, data);
			break;
	}
}
