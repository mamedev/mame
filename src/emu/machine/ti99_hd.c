// license:MAME|LGPL-2.1+
// copyright-holders:Michael Zapf
/*************************************************************************

    Hard disk support

    This device wraps the plain image device as that device does not allow
    for internal states (like head position)
    The plain device is a subdevice ("drive") of this device, so we
    get names like "mfmhd0:drive"

    Michael Zapf
    April 2010
    February 2012: Rewritten as class

**************************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "harddisk.h"
#include "smc92x4.h"

#include "ti99_hd.h"

#define TI99HD_BLOCKNOTFOUND -1

#define LOG logerror
#define VERBOSE 0

#define GAP1 16
#define GAP2 8
#define GAP3 15
#define GAP4 340
#define SYNC 13

mfm_harddisk_device::mfm_harddisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, TI99_MFMHD, "MFM Harddisk", tag, owner, clock, "mfm_harddisk", __FILE__)
{
}

/*
    Calculate the ident byte from the cylinder. The specification does not
    define idents beyond cylinder 1023, but formatting programs seem to
    continue with 0xfd for cylinders between 1024 and 2047.
*/
UINT8 mfm_harddisk_device::cylinder_to_ident(int cylinder)
{
	if (cylinder < 256) return 0xfe;
	if (cylinder < 512) return 0xff;
	if (cylinder < 768) return 0xfc;
	if (cylinder < 1024) return 0xfd;
	return 0xfd;
}

/*
    Returns the linear sector number, given the CHS data.
*/
bool mfm_harddisk_device::harddisk_chs_to_lba(hard_disk_file *hdfile, int cylinder, int head, int sector, UINT32 *lba)
{
	const hard_disk_info *info;

	if ( hdfile != NULL)
	{
		info = hard_disk_get_info(hdfile);

		if (    (cylinder >= info->cylinders) ||
				(head >= info->heads) ||
				(sector >= info->sectors))
		return false;

		*lba = (cylinder * info->heads + head)
				* info->sectors
				+ sector;
		return true;
	}
	return false;
}

/* Accessor functions */
void mfm_harddisk_device::read_sector(int cylinder, int head, int sector, UINT8 *buf)
{
	UINT32 lba;
	if (VERBOSE>5) LOG("ti99_hd: read_sector(%d, %d, %d)\n", cylinder, head, sector);
	hard_disk_file *file = m_drive->get_hard_disk_file();

	if (cylinder != m_current_cylinder)
	{
		return;
	}

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	if (!harddisk_chs_to_lba(file, cylinder, head, sector, &lba))
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	if (!hard_disk_read(file, lba, buf))
	{
		m_status &= ~MFMHD_READY;
		return;
	}
	/* printf("ti99_hd read sector  c=%04d h=%02d s=%02d\n", cylinder, head, sector); */
	m_status |= MFMHD_READY;
}

void mfm_harddisk_device::write_sector(int cylinder, int head, int sector, UINT8 *buf)
{
	UINT32 lba;
	if (VERBOSE>5) LOG("ti99_hd: write_sector(%d, %d, %d)\n", cylinder, head, sector);
	hard_disk_file *file = m_drive->get_hard_disk_file();

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	if (!harddisk_chs_to_lba(file, cylinder, head, sector, &lba))
	{
		m_status &= ~MFMHD_READY;
		m_status |= MFMHD_WRFAULT;
		return;
	}

	if (!hard_disk_write(file, lba, buf))
	{
		m_status &= ~MFMHD_READY;
		m_status |= MFMHD_WRFAULT;
		return;
	}

	m_status |= MFMHD_READY;
}

/*
    Searches a block containing number * byte, starting at the given
    position. Returns the position of the first byte of the block.
*/
int mfm_harddisk_device::find_block(const UINT8 *buffer, int start, int stop, UINT8 byte, size_t number)
{
	int i = start;
	size_t current = number;
	while (i < stop && current > 0)
	{
		if (buffer[i++] != byte)
		{
			current = number;
		}
		else
		{
			current--;
		}
	}
	if (current==0)
	{
		return i - number;
	}
	else
		return TI99HD_BLOCKNOTFOUND;
}

int mfm_harddisk_device::get_track_length()
{
	int count;
	int size;
	hard_disk_file *file = m_drive->get_hard_disk_file();
	const hard_disk_info *info;

	if (file==NULL) return 0;
	info = hard_disk_get_info(file);

	count = info->sectors;
	size = info->sectorbytes/128;
	return GAP1 + count*(SYNC+12+GAP2+SYNC+size*128+GAP3)+GAP4;
}

/*
    Reads a complete track. We have to rebuild the gaps. This is basically
    done in the same way as in the SDF format in ti99_dsk.

    WARNING: This function is untested! We need to create a suitable
    application program for the TI which makes use of it.
*/
void mfm_harddisk_device::read_track(int head, UINT8 *trackdata)
{
	/* We assume an interleave of 3 for 32 sectors. */
	int step = 3;
	UINT32 lba;
	int i;

	int size;
	int position = 0;
	int sector;
	int crc;
	int count;

	const hard_disk_info *info;
	hard_disk_file *file = m_drive->get_hard_disk_file();

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	info = hard_disk_get_info(file);

	count = info->sectors;
	size = info->sectorbytes/128;

	/* Write lead-in. */
	memset(trackdata, 0x4e, GAP1);

	position += GAP1;
	for (i=0; i < count; i++)
	{
		sector = (i*step) % info->sectors;
		memset(&trackdata[position], 0x00, SYNC);
		position += SYNC;

		/* Write sync */
		trackdata[position++] = 0xa1;

		/* Write IDAM */
		trackdata[position++] = cylinder_to_ident(m_current_cylinder);
		trackdata[position++] = m_current_cylinder;
		trackdata[position++] = head;
		trackdata[position++] = sector;
		trackdata[position++] = (size==1)? 0x00 : (0x01 << (size-1));

		/* Set CRC16 */
		crc = ccitt_crc16(0xffff, &trackdata[position-5], 5);
		trackdata[position++] = (crc>>8)&0xff;
		trackdata[position++] = crc & 0xff;

		/* Write Gap2 */
		memset(&trackdata[position], 0x4e, GAP2);
		position += GAP2;

		/* Write sync */
		memset(&trackdata[position], 0x00, SYNC);
		position += SYNC;
		trackdata[position++] = 0xa1;

		/* Write DAM */
		trackdata[position++] = 0xfb;

		/* Locate the sector content in the image and load it. */

		if (!harddisk_chs_to_lba(file, m_current_cylinder, head, sector, &lba))
		{
			m_status &= ~MFMHD_READY;
			return;
		}

		if (!hard_disk_read(file, lba, &trackdata[position]))
		{
			m_status &= ~MFMHD_READY;
			return;
		}

		position += info->sectorbytes;

		/* Set CRC16. Includes the address mark. */
		crc = ccitt_crc16(0xffff, &trackdata[position-size-1], size+1);
		trackdata[position++] = (crc>>8)&0xff;
		trackdata[position++] = crc & 0xff;

		/* Write remaining 3 bytes which would have been used for ECC. */
		memset(&trackdata[position], 0x00, 3);
		position += 3;

		/* Write Gap3 */
		memset(&trackdata[position], 0x4e, GAP3);
		position += GAP3;
	}
	/* Write Gap 4 */
	memset(&trackdata[position], 0x4e, GAP4);
	position += GAP4;

	m_status |= MFMHD_READY;
}

/*
    Writes a track to the image. We need to isolate the sector contents.
    This is basically done in the same way as in the SDF format in ti99_dsk.
*/
void mfm_harddisk_device::write_track(int head, UINT8 *track_image, int data_count)
{
	int current_pos = 0;
	bool found;
	// UINT8 wident;
	UINT8 whead = 0, wsector = 0;
	// UINT8 wsize;
	UINT16 wcyl = 0;
	int state;

	/* Only search in the first 100 bytes for the start. */

	UINT32 lba;
	hard_disk_file *file = m_drive->get_hard_disk_file();

	/* printf("ti99_hd write track c=%d h=%d\n", m_current_cylinder, head); */

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	current_pos = find_block(track_image, 0, 100, 0x4e, GAP1);

	// In case of defect formats, we continue as far as possible. This
	// may lead to sectors not being written. */
	if (current_pos==TI99HD_BLOCKNOTFOUND)
	{
		logerror("ti99_hd error: write track: Cannot find GAP1 for cylinder %d, head %d.\n", m_current_cylinder, head);
		/* What now? The track data are illegal, so we refuse to continue. */
		m_status |= MFMHD_WRFAULT;
		return;
	}

	/* Get behind GAP1 */
	current_pos += GAP1;
	found = false;

	while (current_pos < data_count)
	{
		/* We must find the address block to determine the sector. */
		int new_pos = find_block(track_image, current_pos, data_count, 0x00, SYNC);
		if (new_pos==TI99HD_BLOCKNOTFOUND)
		{
			/* Forget about the rest; we're done. */
			if (found) break;  /* we were already successful, so all ok */
			logerror("ti99_hd error: write track: Cannot find sync for track %d, head %d.\n", m_current_cylinder, head);
			m_status |= MFMHD_WRFAULT;
			return;
		}
		found = true;

		new_pos = new_pos + SYNC; /* skip sync bytes */

		if (track_image[new_pos]==0xa1)
		{
			/* IDAM found. */
			current_pos = new_pos + 1;
			// wident = track_image[current_pos];  // unused
			wcyl = track_image[current_pos+1] + ((track_image[current_pos+2]&0x70)<<4);
			whead = track_image[current_pos+2]&0x0f;
			wsector = track_image[current_pos+3];
			// wsize = track_image[current_pos+4];  // unused

			if (wcyl == m_current_cylinder && whead == head)
			{
				if (!harddisk_chs_to_lba(file, wcyl, whead, wsector, &lba))
				{
					m_status |= MFMHD_WRFAULT;
					return;
				}

				/* Skip to the sector content. */
				new_pos = find_block(track_image, current_pos, data_count, 0x00, SYNC);
				current_pos = new_pos + SYNC;
				if (track_image[current_pos]==0xa1)
				{
					current_pos += 2;
					state = hard_disk_write(file, lba, track_image+current_pos);
					if (state==0)
					{
						logerror("ti99_hd error: write track: Write error during formatting cylinder %d, head %d\n", wcyl, whead);
						m_status |= MFMHD_WRFAULT;
						return;
					}
					current_pos = current_pos+256+2; /* Skip contents and CRC */
				}
				else
				{
					logerror("ti99_hd error: write track: Cannot find DAM for cylinder %d, head %d, sector %d.\n", wcyl, whead, wsector);
					m_status |= MFMHD_WRFAULT;
					return;
				}
			}
			else
			{
				logerror("ti99_hd error: write track: Cylinder/head mismatch. Drive is on cyl=%d, head=%d, track data say cyl=%d, head=%d\n", m_current_cylinder, head, wcyl, whead);
				m_status |= MFMHD_WRFAULT;
			}
		}
		else
		{
			logerror("ti99_hd error: write track: Invalid track image for cyl=%d, head=%d. Cannot find any IDAM in track data.\n",  m_current_cylinder, head);
			m_status |= MFMHD_WRFAULT;
			return;
		}
	}
}

UINT8 mfm_harddisk_device::get_status()
{
	UINT8 status = 0;
	hard_disk_file *file = m_drive->get_hard_disk_file();
	if (file!=NULL)
		status |= MFMHD_READY;

	if (m_current_cylinder==0)
		status |= MFMHD_TRACK00;

	if (!m_seeking)
		status |= MFMHD_SEEKCOMP;

	if (m_id_index == 0)
		status |= MFMHD_INDEX;

	m_status = status;
	if (VERBOSE>5) LOG("ti99_hd: request status reply = %02x\n", status);
	return status;
}

void mfm_harddisk_device::seek(int direction)
{
	const hard_disk_info *info;
	hard_disk_file *file = m_drive->get_hard_disk_file();

	if (file==NULL) return;

	info = hard_disk_get_info(file);

	m_seeking = true;

	if (direction<0)
	{
		if (m_current_cylinder>0)
			m_current_cylinder--;
	}
	else
	{
		if (m_current_cylinder < info->cylinders)
			m_current_cylinder++;
	}

	// TODO: Requires timer

	m_seeking = false;
}

void mfm_harddisk_device::get_next_id(int head, chrn_id_hd *id)
{
	const hard_disk_info *info;
	hard_disk_file *file;
	int interleave = 3;

	file = m_drive->get_hard_disk_file();

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	info = hard_disk_get_info(file);

	m_current_head = head;

	/* TODO: implement an interleave suitable for the number of sectors in the track. */
	m_id_index = (m_id_index + interleave) % info->sectors;

	/* build a new info block. */
	id->C = m_current_cylinder;
	id->H = m_current_head;
	id->R = m_id_index;
	id->N = 1;
	id->data_id = m_id_index;
	id->flags = 0;
}

void mfm_harddisk_device::device_start()
{
	m_current_cylinder = 0;
	m_current_head = 0;
}

void mfm_harddisk_device::device_reset()
{
	m_drive = static_cast<harddisk_image_device *>(subdevice("drive"));
	m_seeking = false;
	m_status = 0;
	m_id_index = 0;
}

MACHINE_CONFIG_FRAGMENT( mfmhd )
	MCFG_HARDDISK_ADD("drive")
MACHINE_CONFIG_END

machine_config_constructor mfm_harddisk_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mfmhd );
}

const device_type TI99_MFMHD = &device_creator<mfm_harddisk_device>;
