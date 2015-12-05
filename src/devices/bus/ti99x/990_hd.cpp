// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    990_hd.c: emulation of a generic ti990 hard disk controller, for use with
    TILINE-based TI990 systems (TI990/10, /12, /12LR, /10A, Business system 300
    and 300A).

    This core will emulate the common feature set found in every disk controller.
    Most controllers support additional features, but are still compatible with
    the basic feature set.  I have a little documentation on two specific
    disk controllers (WD900 and WD800/WD800A), but I have not tried to emulate
    controller-specific features.


    Long description: see 2234398-9701 and 2306140-9701.


    Raphael Nabet 2002-2003
*/

#include "emu.h"

#include "990_hd.h"

#include "harddisk.h"
#include "imagedev/harddriv.h"

/* Max sector length is bytes.  Generally 256, except for a few older disk
units which use 288-byte-long sectors, and SCSI units which generally use
standard 512-byte-long sectors. */
/* I chose a limit of 512.  No need to use more until someone writes CD-ROMs
for TI990. */
#define MAX_SECTOR_SIZE 512

/* Description of custom format */
/* We can use MAME's harddisk.c image format instead. */

/* machine-independent big-endian 32-bit integer */
struct UINT32BE
{
	UINT8 bytes[4];
};

INLINE UINT32 get_UINT32BE(UINT32BE word)
{
	return (word.bytes[0] << 24) | (word.bytes[1] << 16) | (word.bytes[2] << 8) | word.bytes[3];
}

#ifdef UNUSED_FUNCTION
INLINE void set_UINT32BE(UINT32BE *word, UINT32 data)
{
	word->bytes[0] = (data >> 24) & 0xff;
	word->bytes[1] = (data >> 16) & 0xff;
	word->bytes[2] = (data >> 8) & 0xff;
	word->bytes[3] = data & 0xff;
}
#endif

/* disk image header */
struct disk_image_header
{
	UINT32BE cylinders;         /* number of cylinders on hard disk (big-endian) */
	UINT32BE heads;             /* number of heads on hard disk (big-endian) */
	UINT32BE sectors_per_track; /* number of sectors per track on hard disk (big-endian) */
	UINT32BE bytes_per_sector;  /* number of bytes of data per sector on hard disk (big-endian) */
};

enum
{
	header_len = sizeof(disk_image_header)
};


/* masks for individual bits controller registers */
enum
{
	w0_offline          = 0x8000,
	w0_not_ready        = 0x4000,
	w0_write_protect    = 0x2000,
	w0_unsafe           = 0x1000,
	w0_end_of_cylinder  = 0x0800,
	w0_seek_incomplete  = 0x0400,
	/*w0_offset_active  = 0x0200,*/
	w0_pack_change      = 0x0100,

	w0_attn_lines       = 0x00f0,
	w0_attn_mask        = 0x000f,

	w1_extended_command = 0xc000,
	/*w1_strobe_early       = 0x2000,
	w1_strobe_late      = 0x1000,*/
	w1_transfer_inhibit = 0x0800,
	w1_command          = 0x0700,
	w1_offset           = 0x0080,
	w1_offset_forward   = 0x0040,
	w1_head_address     = 0x003f,

	w6_unit0_sel        = 0x0800,
	w6_unit1_sel        = 0x0400,
	w6_unit2_sel        = 0x0200,
	w6_unit3_sel        = 0x0100,

	w7_idle             = 0x8000,
	w7_complete         = 0x4000,
	w7_error            = 0x2000,
	w7_int_enable       = 0x1000,
	/*w7_lock_out           = 0x0800,*/
	w7_retry            = 0x0400,
	w7_ecc              = 0x0200,
	w7_abnormal_completion  = 0x0100,
	w7_memory_error         = 0x0080,
	w7_data_error           = 0x0040,
	w7_tiline_timeout_err   = 0x0020,
	w7_header_err           = 0x0010,
	w7_rate_err             = 0x0008,
	w7_command_time_out_err = 0x0004,
	w7_search_err           = 0x0002,
	w7_unit_err             = 0x0001
};

/* masks for computer-controlled bit in each controller register  */
static const UINT16 w_mask[8] =
{
	0x000f,     /* Controllers should prevent overwriting of w0 status bits, and I know
                that some controllers do so. */
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xf7ff      /* Don't overwrite reserved bits */
};


int ti990_hdc_device::get_id_from_device( device_t *device )
{
	int id = -1;

	if ( ! strcmp( ":harddisk1", device->tag() ) )
	{
		id = 0;
	}
	else if ( ! strcmp( ":harddisk2", device->tag() ) )
	{
		id = 1;
	}
	else if ( ! strcmp( ":harddisk3", device->tag() ) )
	{
		id = 2;
	}
	else if ( ! strcmp( ":harddisk4", device->tag() ) )
	{
		id = 3;
	}
	assert( id >= 0 );

	return id;
}


/*
    Initialize hard disk unit and open a hard disk image
*/
DEVICE_IMAGE_LOAD_MEMBER( ti990_hdc_device, ti990_hd )
{
	int id = get_id_from_device( &image.device() );
	hd_unit_t *d;
	hard_disk_file  *hd_file;

	d = &m_d[id];
	d->img = &image;

	hd_file = dynamic_cast<harddisk_image_device *>(&image)->get_hard_disk_file();

	if ( hd_file )
	{
		const hard_disk_info *standard_header;

		d->format = format_mame;
		d->hd_handle = hd_file;

		/* use standard hard disk image header. */
		standard_header = hard_disk_get_info(d->hd_handle);

		d->cylinders = standard_header->cylinders;
		d->heads = standard_header->heads;
		d->sectors_per_track = standard_header->sectors;
		d->bytes_per_sector = standard_header->sectorbytes;
	}
	else
	{
		/* older, custom format */
		disk_image_header custom_header;
		int bytes_read;

		/* set file descriptor */
		d->format = format_old;
		d->hd_handle = nullptr;

		/* use custom image header. */
		/* to convert old header-less images to this format, insert a 16-byte
		header as follow: 00 00 03 8f  00 00 00 05  00 00 00 21  00 00 01 00 */
		d->img->fseek(0, SEEK_SET);
		bytes_read = d->img->fread(&custom_header, sizeof(custom_header));
		if (bytes_read != sizeof(custom_header))
		{
			d->format = format_mame;    /* don't care */
			d->wp = 1;
			d->unsafe = 1;
			return IMAGE_INIT_FAIL;
		}

		d->cylinders = get_UINT32BE(custom_header.cylinders);
		d->heads = get_UINT32BE(custom_header.heads);
		d->sectors_per_track = get_UINT32BE(custom_header.sectors_per_track);
		d->bytes_per_sector = get_UINT32BE(custom_header.bytes_per_sector);
	}

	if (d->bytes_per_sector > MAX_SECTOR_SIZE)
	{
		d->format = format_mame;
		d->hd_handle = nullptr;
		d->wp = 1;
		d->unsafe = 1;
		return IMAGE_INIT_FAIL;
	}

	/* tell whether the image is writable */
	d->wp = image.is_readonly();

	d->unsafe = 1;
	/* set attention line */
	m_w[0] |= (0x80 >> id);

	return IMAGE_INIT_PASS;
}

/*
    close a hard disk image
*/
DEVICE_IMAGE_UNLOAD_MEMBER( ti990_hdc_device, ti990_hd )
{
	int id = get_id_from_device( image );
	hd_unit_t *d;

	d = &m_d[id];

	d->format = format_mame;    /* don't care */
	d->hd_handle = nullptr;
	d->wp = 1;
	d->unsafe = 1;

	/* clear attention line */
	m_w[0] &= ~ (0x80 >> id);
}

/*
    Return true if a HD image has been loaded
*/
int ti990_hdc_device::is_unit_loaded(int unit)
{
	int reply = 0;

	switch (m_d[unit].format)
	{
	case format_mame:
		reply = (m_d[unit].hd_handle != nullptr);
		break;

	case format_old:
		reply = (m_d[unit].img->exists() ? 1 : 0);
		break;
	}

	return reply;
}

/*
    Parse the disk select lines, and return the corresponding tape unit.
    (-1 if none)
*/
int ti990_hdc_device::cur_disk_unit(void)
{
	int reply;


	if (m_w[6] & w6_unit0_sel)
		reply = 0;
	else if (m_w[6] & w6_unit1_sel)
		reply = 1;
	else if (m_w[6] & w6_unit2_sel)
		reply = 2;
	else if (m_w[6] & w6_unit3_sel)
		reply = 3;
	else
		reply = -1;

	if (reply >= MAX_DISK_UNIT)
		reply = -1;

	return reply;
}

/*
    Update interrupt state
*/
void ti990_hdc_device::update_interrupt()
{
	if (!m_interrupt_callback.isnull())
		m_interrupt_callback((m_w[7] & w7_idle)
									&& (((m_w[7] & w7_int_enable) && (m_w[7] & (w7_complete | w7_error)))
										|| ((m_w[0] & (m_w[0] >> 4)) & w0_attn_mask)));
}

/*
    Check that a sector address is valid.

    Terminate current command and return non-zero if the address is invalid.
*/
int ti990_hdc_device::check_sector_address(int unit, unsigned int cylinder, unsigned int head, unsigned int sector)
{
	if ((cylinder > m_d[unit].cylinders) || (head > m_d[unit].heads) || (sector > m_d[unit].sectors_per_track))
	{   /* invalid address */
		if (cylinder > m_d[unit].cylinders)
		{
			m_w[0] |= w0_seek_incomplete;
			m_w[7] |= w7_idle | w7_error | w7_unit_err;
		}
		else if (head > m_d[unit].heads)
		{
			m_w[0] |= w0_end_of_cylinder;
			m_w[7] |= w7_idle | w7_error | w7_unit_err;
		}
		else if (sector > m_d[unit].sectors_per_track)
			m_w[7] |= w7_idle | w7_error | w7_command_time_out_err;
		update_interrupt();
		return 1;
	}

	return 0;
}

/*
    Seek to sector whose address is given
*/
int ti990_hdc_device::sector_to_lba(int unit, unsigned int cylinder, unsigned int head, unsigned int sector, unsigned int *lba)
{
	if (check_sector_address(unit, cylinder, head, sector))
		return 1;

	* lba = (cylinder*m_d[unit].heads + head)*m_d[unit].sectors_per_track + sector;

	return 0;
}

/*
    Read one given sector
*/
int ti990_hdc_device::read_sector(int unit, unsigned int lba, void *buffer, unsigned int bytes_to_read)
{
	unsigned long byte_position;
	unsigned int bytes_read;

	switch (m_d[unit].format)
	{
	case format_mame:
		bytes_read = m_d[unit].bytes_per_sector * hard_disk_read(m_d[unit].hd_handle, lba, buffer);
		if (bytes_read > bytes_to_read)
			bytes_read = bytes_to_read;
		break;

	case format_old:
		byte_position = lba*m_d[unit].bytes_per_sector + header_len;
		m_d[unit].img->fseek(byte_position, SEEK_SET);
		bytes_read = m_d[unit].img->fread(buffer, bytes_to_read);
		break;

	default:
		bytes_read = 0;
		break;
	}

	return bytes_read;
}

/*
    Write one given sector
*/
int ti990_hdc_device::write_sector(int unit, unsigned int lba, const void *buffer, unsigned int bytes_to_write)
{
	unsigned long byte_position;
	unsigned int bytes_written;

	switch (m_d[unit].format)
	{
	case format_mame:
		bytes_written = m_d[unit].bytes_per_sector * hard_disk_write(m_d[unit].hd_handle, lba, buffer);
		if (bytes_written > bytes_to_write)
			bytes_written = bytes_to_write;
		break;

	case format_old:
		byte_position = lba*m_d[unit].bytes_per_sector + header_len;
		m_d[unit].img->fseek(byte_position, SEEK_SET);
		bytes_written = m_d[unit].img->fwrite(buffer, bytes_to_write);
		break;

	default:
		bytes_written = 0;
		break;
	}

	return bytes_written;
}

/*
    Handle the store registers command: read the drive geometry.
*/
void ti990_hdc_device::store_registers()
{
	int dma_address;
	int byte_count;

	UINT16 buffer[3];
	int i, real_word_count;

	int dsk_sel = cur_disk_unit();


	if (dsk_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_abnormal_completion;
		update_interrupt();
		return;
	}
	else if (! is_unit_loaded(dsk_sel))
	{   /* offline */
		m_w[0] |= w0_offline | w0_not_ready;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}

	m_d[dsk_sel].unsafe = 0;      /* I think */

	dma_address = ((((int) m_w[6]) << 16) | m_w[5]) & 0x1ffffe;
	byte_count = m_w[4] & 0xfffe;

	/* formatted words per track */
	buffer[0] = (m_d[dsk_sel].sectors_per_track*m_d[dsk_sel].bytes_per_sector) >> 1;
	/* MSByte: sectors per track; LSByte: bytes of overhead per sector */
	buffer[1] = (m_d[dsk_sel].sectors_per_track << 8) | 0;
	/* bits 0-4: heads; bits 5-15: cylinders */
	buffer[2] = (m_d[dsk_sel].heads << 11) | m_d[dsk_sel].cylinders;

	real_word_count = byte_count >> 1;
	if (real_word_count > 3)
		real_word_count = 3;

	/* DMA */
	if (! (m_w[1] & w1_transfer_inhibit))
		for (i=0; i<real_word_count; i++)
		{
			machine().device("maincpu")->memory().space(AS_PROGRAM).write_word(dma_address, buffer[i]);
			dma_address = (dma_address + 2) & 0x1ffffe;
		}

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();
}

/*
    Handle the write format command: format a complete track on disk.

    The emulation just clears the track data in the disk image.
*/
void ti990_hdc_device::write_format()
{
	unsigned int cylinder, head, sector;
	unsigned int lba;

	UINT8 buffer[MAX_SECTOR_SIZE];
	int bytes_written;

	int dsk_sel = cur_disk_unit();


	if (dsk_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_abnormal_completion;
		update_interrupt();
		return;
	}
	else if (! is_unit_loaded(dsk_sel))
	{   /* offline */
		m_w[0] |= w0_offline | w0_not_ready;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}
	else if (m_d[dsk_sel].unsafe)
	{   /* disk in unsafe condition */
		m_w[0] |= w0_unsafe | w0_pack_change;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}
	else if (m_d[dsk_sel].wp)
	{   /* disk write-protected */
		m_w[0] |= w0_write_protect;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}

	cylinder = m_w[3];
	head = m_w[1] & w1_head_address;

	if (sector_to_lba(dsk_sel, cylinder, head, 0, &lba))
		return;

	memset(buffer, 0, m_d[dsk_sel].bytes_per_sector);

	for (sector=0; sector<m_d[dsk_sel].sectors_per_track; sector++)
	{
		bytes_written = write_sector(dsk_sel, lba, buffer, m_d[dsk_sel].bytes_per_sector);

		if (bytes_written != m_d[dsk_sel].bytes_per_sector)
		{
			m_w[0] |= w0_offline | w0_not_ready;
			m_w[7] |= w7_idle | w7_error | w7_unit_err;
			update_interrupt();
			return;
		}

		lba++;
	}

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();
}

/*
    Handle the read data command: read a variable number of sectors from disk.
*/
void ti990_hdc_device::read_data()
{
	int dma_address;
	int byte_count;

	unsigned int cylinder, head, sector;
	unsigned int lba;

	UINT8 buffer[MAX_SECTOR_SIZE];
	int bytes_to_read;
	int bytes_read;
	int i;

	int dsk_sel = cur_disk_unit();


	if (dsk_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_abnormal_completion;
		update_interrupt();
		return;
	}
	else if (! is_unit_loaded(dsk_sel))
	{   /* offline */
		m_w[0] |= w0_offline | w0_not_ready;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}
	else if (m_d[dsk_sel].unsafe)
	{   /* disk in unsafe condition */
		m_w[0] |= w0_unsafe | w0_pack_change;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}

	dma_address = ((((int) m_w[6]) << 16) | m_w[5]) & 0x1ffffe;
	byte_count = m_w[4] & 0xfffe;

	cylinder = m_w[3];
	head = m_w[1] & w1_head_address;
	sector = m_w[2] & 0xff;

	if (sector_to_lba(dsk_sel, cylinder, head, sector, &lba))
		return;

	while (byte_count)
	{   /* read data sector per sector */
		if (cylinder > m_d[dsk_sel].cylinders)
		{
			m_w[0] |= w0_seek_incomplete;
			m_w[7] |= w7_idle | w7_error | w7_unit_err;
			update_interrupt();
			return;
		}

		bytes_to_read = (byte_count < m_d[dsk_sel].bytes_per_sector) ? byte_count : m_d[dsk_sel].bytes_per_sector;
		bytes_read = read_sector(dsk_sel, lba, buffer, bytes_to_read);

		if (bytes_read != bytes_to_read)
		{   /* behave as if the controller could not found the sector ID mark */
			m_w[7] |= w7_idle | w7_error | w7_command_time_out_err;
			update_interrupt();
			return;
		}

		/* DMA */
		if (! (m_w[1] & w1_transfer_inhibit))
			for (i=0; i<bytes_read; i+=2)
			{
				machine().device("maincpu")->memory().space(AS_PROGRAM).write_word(dma_address, (((int) buffer[i]) << 8) | buffer[i+1]);
				dma_address = (dma_address + 2) & 0x1ffffe;
			}

		byte_count -= bytes_read;

		/* update sector address to point to next sector */
		lba++;
		sector++;
		if (sector == m_d[dsk_sel].sectors_per_track)
		{
			sector = 0;
			head++;
			if (head == m_d[dsk_sel].heads)
			{
				head = 0;
				cylinder++;
			}
		}
	}

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();
}

/*
    Handle the write data command: write a variable number of sectors from disk.
*/
void ti990_hdc_device::write_data()
{
	int dma_address;
	int byte_count;

	unsigned int cylinder, head, sector;
	unsigned int lba;

	UINT8 buffer[MAX_SECTOR_SIZE];
	UINT16 word;
	int bytes_written;
	int i;

	int dsk_sel = cur_disk_unit();


	if (dsk_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_abnormal_completion;
		update_interrupt();
		return;
	}
	else if (! is_unit_loaded(dsk_sel))
	{   /* offline */
		m_w[0] |= w0_offline | w0_not_ready;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}
	else if (m_d[dsk_sel].unsafe)
	{   /* disk in unsafe condition */
		m_w[0] |= w0_unsafe | w0_pack_change;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}
	else if (m_d[dsk_sel].wp)
	{   /* disk write-protected */
		m_w[0] |= w0_write_protect;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}

	dma_address = ((((int) m_w[6]) << 16) | m_w[5]) & 0x1ffffe;
	byte_count = m_w[4] & 0xfffe;

	cylinder = m_w[3];
	head = m_w[1] & w1_head_address;
	sector = m_w[2] & 0xff;

	if (sector_to_lba(dsk_sel, cylinder, head, sector, &lba))
		return;

	while (byte_count > 0)
	{   /* write data sector per sector */
		if (cylinder > m_d[dsk_sel].cylinders)
		{
			m_w[0] |= w0_seek_incomplete;
			m_w[7] |= w7_idle | w7_error | w7_unit_err;
			update_interrupt();
			return;
		}

		/* DMA */
		for (i=0; (i<byte_count) && (i<m_d[dsk_sel].bytes_per_sector); i+=2)
		{
			word = machine().device("maincpu")->memory().space(AS_PROGRAM).read_word(dma_address);
			buffer[i] = word >> 8;
			buffer[i+1] = word & 0xff;

			dma_address = (dma_address + 2) & 0x1ffffe;
		}
		/* fill with 0s if we did not reach sector end */
		for (; i<m_d[dsk_sel].bytes_per_sector; i+=2)
			buffer[i] = buffer[i+1] = 0;

		bytes_written = write_sector(dsk_sel, lba, buffer, m_d[dsk_sel].bytes_per_sector);

		if (bytes_written != m_d[dsk_sel].bytes_per_sector)
		{
			m_w[0] |= w0_offline | w0_not_ready;
			m_w[7] |= w7_idle | w7_error | w7_unit_err;
			update_interrupt();
			return;
		}

		byte_count -= bytes_written;

		/* update sector address to point to next sector */
		lba++;
		sector++;
		if (sector == m_d[dsk_sel].sectors_per_track)
		{
			sector = 0;
			head++;
			if (head == m_d[dsk_sel].heads)
			{
				head = 0;
				cylinder++;
			}
		}
	}

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();
}

/*
    Handle the unformatted read command: read drive geometry information.
*/
void ti990_hdc_device::unformatted_read()
{
	int dma_address;
	int byte_count;

	unsigned int cylinder, head, sector;

	UINT16 buffer[3];
	int i, real_word_count;

	int dsk_sel = cur_disk_unit();


	if (dsk_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_abnormal_completion;
		update_interrupt();
		return;
	}
	else if (! is_unit_loaded(dsk_sel))
	{   /* offline */
		m_w[0] |= w0_offline | w0_not_ready;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}
	else if (m_d[dsk_sel].unsafe)
	{   /* disk in unsafe condition */
		m_w[0] |= w0_unsafe | w0_pack_change;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}

	dma_address = ((((int) m_w[6]) << 16) | m_w[5]) & 0x1ffffe;
	byte_count = m_w[4] & 0xfffe;

	cylinder = m_w[3];
	head = m_w[1] & w1_head_address;
	sector = m_w[2] & 0xff;

	if (check_sector_address(dsk_sel, cylinder, head, sector))
		return;

	dma_address = ((((int) m_w[6]) << 16) | m_w[5]) & 0x1ffffe;
	byte_count = m_w[4] & 0xfffe;

	/* bits 0-4: head address; bits 5-15: cylinder address */
	buffer[0] = (head << 11) | cylinder;
	/* MSByte: sectors per record (1); LSByte: sector address */
	buffer[1] = (1 << 8) | sector;
	/* formatted words per record */
	buffer[2] = m_d[dsk_sel].bytes_per_sector >> 1;

	real_word_count = byte_count >> 1;
	if (real_word_count > 3)
		real_word_count = 3;

	/* DMA */
	if (! (m_w[1] & w1_transfer_inhibit))
		for (i=0; i<real_word_count; i++)
		{
			machine().device("maincpu")->memory().space(AS_PROGRAM).write_word(dma_address, buffer[i]);
			dma_address = (dma_address + 2) & 0x1ffffe;
		}

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();
}

/*
    Handle the restore command: return to track 0.
*/
void ti990_hdc_device::restore()
{
	int dsk_sel = cur_disk_unit();


	if (dsk_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_abnormal_completion;
		update_interrupt();
		return;
	}
	else if (! is_unit_loaded(dsk_sel))
	{   /* offline */
		m_w[0] |= w0_offline | w0_not_ready;
		m_w[7] |= w7_idle | w7_error | w7_unit_err;
		update_interrupt();
		return;
	}

	m_d[dsk_sel].unsafe = 0;      /* I think */

	/*if (seek_to_sector(dsk_sel, 0, 0, 0))
	    return;*/

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();
}

/*
    Parse command code and execute the command.
*/
void ti990_hdc_device::execute_command()
{
	/* hack */
	m_w[0] &= 0xff;

	if (m_w[1] & w1_extended_command)
		logerror("extended commands not supported\n");

	switch (/*((m_w[1] & w1_extended_command) >> 11) |*/ ((m_w[1] & w1_command) >> 8))
	{
	case 0x00: //0b000:
		/* store registers */
		logerror("store registers\n");
		store_registers();
		break;
	case 0x01: //0b001:
		/* write format */
		logerror("write format\n");
		write_format();
		break;
	case 0x02: //0b010:
		/* read data */
		logerror("read data\n");
		read_data();
		break;
	case 0x03: //0b011:
		/* write data */
		logerror("write data\n");
		write_data();
		break;
	case 0x04: //0b100:
		/* unformatted read */
		logerror("unformatted read\n");
		unformatted_read();
		break;
	case 0x05: //0b101:
		/* unformatted write */
		logerror("unformatted write\n");
		/* ... */
		m_w[7] |= w7_idle | w7_error | w7_abnormal_completion;
		update_interrupt();
		break;
	case 0x06: //0b110:
		/* seek */
		logerror("seek\n");
		/* This command can (almost) safely be ignored */
		m_w[7] |= w7_idle | w7_complete;
		update_interrupt();
		break;
	case 0x07: //0b111:
		/* restore */
		logerror("restore\n");
		restore();
		break;
	}
}

/*
    Read one register in TPCS space
*/
READ16_MEMBER(ti990_hdc_device::read)
{
	if (offset < 8)
		return m_w[offset];
	else
		return 0;
}

/*
    Write one register in TPCS space.  Execute command if w7_idle is cleared.
*/
WRITE16_MEMBER(ti990_hdc_device::write)
{
	if (offset < 8)
	{
		/* write protect if a command is in progress */
		if (m_w[7] & w7_idle)
		{
			UINT16 old_data = m_w[offset];

			/* Only write writable bits AND honor byte accesses (ha!) */
			m_w[offset] = (m_w[offset] & ((~w_mask[offset]) | mem_mask)) | (data & w_mask[offset] & ~mem_mask);

			if ((offset == 0) || (offset == 7))
				update_interrupt();

			if ((offset == 7) && (old_data & w7_idle) && ! (data & w7_idle))
			{   /* idle has been cleared: start command execution */
				execute_command();
			}
		}
	}
}


static MACHINE_CONFIG_FRAGMENT( ti990_hdc )
	MCFG_HARDDISK_ADD( "harddisk1" )
	MCFG_HARDDISK_LOAD(ti990_hdc_device, ti990_hd)
	MCFG_HARDDISK_UNLOAD(ti990_hdc_device, ti990_hd)
	MCFG_HARDDISK_ADD( "harddisk2" )
	MCFG_HARDDISK_LOAD(ti990_hdc_device, ti990_hd)
	MCFG_HARDDISK_UNLOAD(ti990_hdc_device, ti990_hd)
	MCFG_HARDDISK_ADD( "harddisk3" )
	MCFG_HARDDISK_LOAD(ti990_hdc_device, ti990_hd)
	MCFG_HARDDISK_UNLOAD(ti990_hdc_device, ti990_hd)
	MCFG_HARDDISK_ADD( "harddisk4" )
	MCFG_HARDDISK_LOAD(ti990_hdc_device, ti990_hd)
	MCFG_HARDDISK_UNLOAD(ti990_hdc_device, ti990_hd)
MACHINE_CONFIG_END

const device_type TI990_HDC = &device_creator<ti990_hdc_device>;

ti990_hdc_device::ti990_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TI990_HDC, "Generic TI-990 Hard Disk Controller", tag, owner, clock, "hdc_990", __FILE__),
	m_interrupt_callback(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ti990_hdc_device::device_start()
{
	int i;

	/* initialize harddisk information */
	/* attention lines will be set by DEVICE_IMAGE_LOD */
	for (i=0; i<MAX_DISK_UNIT; i++)
	{
		m_d[i].format = format_mame;
		m_d[i].hd_handle = nullptr;
		m_d[i].wp = 1;
		m_d[i].unsafe = 1;
	}
	memset(m_w, 0, sizeof(m_w));
	m_w[7] = w7_idle;

	/* get references to harddisk devices */
	m_d[0].img = dynamic_cast<device_image_interface *>(subdevice("harddisk1"));
	m_d[1].img = dynamic_cast<device_image_interface *>(subdevice("harddisk2"));
	m_d[2].img = dynamic_cast<device_image_interface *>(subdevice("harddisk3"));
	m_d[3].img = dynamic_cast<device_image_interface *>(subdevice("harddisk4"));

	m_interrupt_callback.resolve_safe();

	update_interrupt();
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor ti990_hdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti990_hdc  );
}
