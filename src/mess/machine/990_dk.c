/*
    990_dk.c: emulation of a TI FD800 'Diablo' floppy disk controller
    controller, for use with any TI990 system (and possibly any system which
    implements the CRU bus).

    This floppy disk controller supports IBM-format 8" SSSD and DSSD floppies.

    Raphael Nabet 2003
*/

#include "emu.h"

#include "formats/basicdsk.h"
#include "imagedev/flopdrv.h"
#include "990_dk.h"

#define MAX_FLOPPIES 4

enum buf_mode_t {
	bm_off, bm_read, bm_write
};
static struct
{
	running_machine *machine;
	UINT16 recv_buf;
	UINT16 stat_reg;
	UINT16 xmit_buf;
	UINT16 cmd_reg;

	int interrupt_f_f;
	void (*interrupt_callback)(running_machine &, int state);

	UINT8 buf[128];
	int buf_pos;
	buf_mode_t buf_mode;
	int unit;
	int head;
	int sector;
	/*int non_seq_mode;*/
	int ddam;

	struct
	{
		device_image_interface *img;
		int phys_cylinder;
		int log_cylinder[2];
		int seclen;
	} drv[MAX_FLOPPIES];
} fd800;

/* status bits */
enum
{
	status_OP_complete	= 1 << 0,
	status_XFER_ready	= 1 << 1,
	status_drv_not_ready= 1 << 2,
	status_dat_chk_err	= 1 << 3,
	status_seek_err		= 1 << 4,
	status_invalid_cmd	= 1 << 5,
	status_no_addr_mark	= 1 << 6,
	status_equ_chk_err	= 1 << 7,
	status_ID_chk_err	= 1 << 8,
	status_ID_not_found	= 1 << 9,
	status_ctlr_busy	= 1 << 10,
	status_write_prot	= 1 << 11,
	status_del_sector	= 1 << 12,
	status_interrupt	= 1 << 15,

	status_unit_shift	= 13
};

LEGACY_FLOPPY_OPTIONS_START(fd800)
#if 1
	/* SSSD 8" */
	LEGACY_FLOPPY_OPTION(fd800, "dsk", "TI990 8\" SSSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([77])
		SECTORS([26])
		SECTOR_LENGTH([128])
		FIRST_SECTOR_ID([1]))
#elif 0
	/* DSSD 8" */
	LEGACY_FLOPPY_OPTION(fd800, "dsk", "TI990 8\" DSSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([77])
		SECTORS([26])
		SECTOR_LENGTH([128])
		FIRST_SECTOR_ID([1]))
#endif
LEGACY_FLOPPY_OPTIONS_END

static void fd800_field_interrupt(void)
{
	if (fd800.interrupt_callback)
		(*fd800.interrupt_callback)(*fd800.machine, (fd800.stat_reg & status_interrupt) && ! fd800.interrupt_f_f);
}

static void fd800_unload_proc(device_image_interface &image)
{
	int unit = floppy_get_drive(&image.device());

	fd800.drv[unit].log_cylinder[0] = fd800.drv[unit].log_cylinder[1] = -1;
}

void fd800_machine_init(running_machine &machine, void (*interrupt_callback)(running_machine &machine, int state))
{
	int i;

	fd800.machine = &machine;
	fd800.interrupt_callback = interrupt_callback;

	fd800.stat_reg = 0;
	fd800.interrupt_f_f = 1;

	fd800.buf_pos = 0;
	fd800.buf_mode = bm_off;

	for (i=0; i<MAX_FLOPPIES; i++)
	{
		fd800.drv[i].img = dynamic_cast<device_image_interface *>(floppy_get_device(machine, i));
		fd800.drv[i].phys_cylinder = -1;
		fd800.drv[i].log_cylinder[0] = fd800.drv[i].log_cylinder[1] = -1;
		fd800.drv[i].seclen = 64;
		floppy_install_unload_proc(&fd800.drv[i].img->device(), fd800_unload_proc);
	}

	fd800_field_interrupt();
}

/*
    Read the first id field that can be found on the floppy disk.

    unit: floppy drive index
    head: selected head
    cylinder_id: cylinder ID read
    sector_id: sector ID read

    Return TRUE if an ID was found
*/
static int fd800_read_id(int unit, int head, int *cylinder_id, int *sector_id)
{
	/*UINT8 revolution_count;*/
	chrn_id id;

	/*revolution_count = 0;*/

	/*while (revolution_count < 2)*/
	/*{*/
		if (floppy_drive_get_next_id(&fd800.drv[unit].img->device(), head, &id))
		{
			if (cylinder_id)
				*cylinder_id = id.C;
			if (sector_id)
				*sector_id = id.R;
			return TRUE;
		}
	/*}*/

	return FALSE;
}

/*
    Find a sector by id.

    unit: floppy drive index
    head: selected head
    sector: sector ID to search
    data_id: data ID to be used when calling sector read/write functions

    Return TRUE if the given sector ID was found
*/
static int fd800_find_sector(int unit, int head, int sector, int *data_id)
{
	UINT8 revolution_count;
	chrn_id id;

	revolution_count = 0;

	while (revolution_count < 2)
	{
		if (floppy_drive_get_next_id(&fd800.drv[unit].img->device(), head, &id))
		{
			/* compare id */
			if ((id.R == sector) && (id.N == 0))
			{
				*data_id = id.data_id;
				/* get ddam status */
				/*w->ddam = id.flags & ID_FLAG_DELETED_DATA;*/
				return TRUE;
			}
		}
	}

	return FALSE;
}

/*
    Perform seek command

    unit: floppy drive index
    cylinder: track to seek for
    head: head for which the seek is performed

    Return FALSE if the seek was successful
*/
static int fd800_do_seek(int unit, int cylinder, int head)
{
	int retries;

	if (cylinder > 76)
	{
		fd800.stat_reg |= status_invalid_cmd;
		return TRUE;
	}

	if (!fd800.drv[unit].img->exists())
	{
		fd800.stat_reg |= status_drv_not_ready;	/* right??? */
		return TRUE;
	}

	if (fd800.drv[unit].log_cylinder[head] == -1)
	{	/* current track ID is unknown: read it */
		if (!fd800_read_id(unit, head, &fd800.drv[unit].log_cylinder[head], NULL))
		{
			fd800.stat_reg |= status_ID_not_found;
			return TRUE;
		}
	}
	/* exit if we are already at the requested track */
	if (fd800.drv[unit].log_cylinder[head] == cylinder)
	{
		/*fd800.stat_reg |= status_OP_complete;*/
		return FALSE;
	}
	for (retries=0; retries<10; retries++)
	{	/* seek to requested track */
		floppy_drive_seek(&fd800.drv[unit].img->device(), cylinder-fd800.drv[unit].log_cylinder[head]);
		/* update physical track position */
		if (fd800.drv[unit].phys_cylinder != -1)
			fd800.drv[unit].phys_cylinder += cylinder-fd800.drv[unit].log_cylinder[head];
		/* read new track ID */
		if (!fd800_read_id(unit, head, &fd800.drv[unit].log_cylinder[head], NULL))
		{
			fd800.drv[unit].log_cylinder[head] = -1;
			fd800.stat_reg |= status_ID_not_found;
			return TRUE;
		}
		/* exit if we have reached the requested track */
		if (fd800.drv[unit].log_cylinder[head] == cylinder)
		{
			/*fd800.stat_reg |= status_OP_complete;*/
			return FALSE;
		}
	}
	/* track not found */
	fd800.stat_reg |= status_seek_err;
	return TRUE;
}

/*
    Perform restore command

    unit: floppy drive index

    Return FALSE if the restore was successful
*/
static int fd800_do_restore(int unit)
{
	int seek_count = 0;
	int seek_complete;

	if (!fd800.drv[unit].img->exists())
	{
		fd800.stat_reg |= status_drv_not_ready;	/* right??? */
		return TRUE;
	}

	/* limit iterations to 76 to prevent an endless loop if the disc is locked */
	while (!(seek_complete = !floppy_tk00_r(&fd800.drv[unit].img->device())) && (seek_count < 76))
	{
		floppy_drive_seek(&fd800.drv[unit].img->device(), -1);
		seek_count++;
	}
	if (! seek_complete)
	{
		fd800.drv[unit].phys_cylinder = -1;
		fd800.stat_reg |= status_seek_err;
	}
	else
	{
		fd800.drv[unit].phys_cylinder = 0;
		/*fd800.stat_reg |= status_OP_complete;*/
	}

	return ! seek_complete;
}

/*
    Perform a read operation for one sector
*/
static void fd800_do_read(void)
{
	int data_id;

	if ((fd800.sector == 0) || (fd800.sector > 26))
	{
		fd800.stat_reg |= status_invalid_cmd;
		return;
	}

	if (!fd800_find_sector(fd800.unit, fd800.head, fd800.sector, &data_id))
	{
		fd800.stat_reg |= status_ID_not_found;
		return;
	}

	floppy_drive_read_sector_data(&fd800.drv[fd800.unit].img->device(), fd800.head, data_id, fd800.buf, 128);
	fd800.buf_pos = 0;
	fd800.buf_mode = bm_read;
	fd800.recv_buf = (fd800.buf[fd800.buf_pos<<1] << 8) | fd800.buf[(fd800.buf_pos<<1)+1];

	fd800.stat_reg |= status_XFER_ready;
	fd800.stat_reg |= status_OP_complete;	/* right??? */
}

/*
    Perform a write operation for one sector
*/
static void fd800_do_write(void)
{
	int data_id;

	if (fd800.drv[fd800.unit].seclen < 64)
		/* fill with 0s */
		memset(fd800.buf+(fd800.drv[fd800.unit].seclen<<1), 0, (64-fd800.drv[fd800.unit].seclen)<<1);

	if (!fd800_find_sector(fd800.unit, fd800.head, fd800.sector, &data_id))
	{
		fd800.stat_reg |= status_ID_not_found;
		return;
	}

	floppy_drive_write_sector_data(&fd800.drv[fd800.unit].img->device(), fd800.head, data_id, fd800.buf, 128, fd800.ddam);
	fd800.buf_pos = 0;
	fd800.buf_mode = bm_write;

	fd800.stat_reg |= status_XFER_ready;
	fd800.stat_reg |= status_OP_complete;	/* right??? */
}

/*
    Execute a fdc command
*/
static void fd800_do_cmd(void)
{
	int unit;
	int cylinder;
	int head;
	int seclen;
	int sector;


	if (fd800.buf_mode != bm_off)
	{	/* All commands in the midst of read or write are interpreted as Stop */
		unit = (fd800.cmd_reg >> 10) & 3;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		fd800.buf_pos = 0;
		fd800.buf_mode = bm_off;

		fd800.stat_reg |= status_OP_complete;

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();

		return;
	}

	switch (fd800.cmd_reg >> 12)
	{
	case 0:		/* select
                    bits 16-25: 0s
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		if (!fd800.drv[unit].img->exists())
			fd800.stat_reg |= status_drv_not_ready;	/* right??? */
		else if (fd800.drv[unit].img->is_readonly())
			fd800.stat_reg |= status_write_prot;
		else
			fd800.stat_reg |= status_OP_complete;

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 1:		/* seek
                    bits 16-22: cylinder number (0-76)
                    bits 23-24: 0s
                    bits 25: head number (1=upper)
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;
		head = (fd800.cmd_reg >> 9) & 1;
		cylinder = fd800.cmd_reg & 0x7f;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		if (!fd800_do_seek(unit, cylinder, head))
			fd800.stat_reg |= status_OP_complete;

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 2:		/* restore
                    bits 16-25: 0s
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		if (!fd800_do_restore(unit))
			fd800.stat_reg |= status_OP_complete;

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 3:		/* sector length
                    bits 16-22: sector word count (0-64)
                    bits 23-25: 0s
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;
		seclen = fd800.cmd_reg & 0x7f;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		if ((seclen > 64) || (seclen == 0))
		{
			fd800.stat_reg |= status_invalid_cmd;
		}
		else
		{
			fd800.drv[unit].seclen = seclen;
			fd800.stat_reg |= status_OP_complete;
		}

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 4:		/* read
                    bits 16-20: sector number (1-26)
                    bits 21-23: 0s
                    bit 24: no sequential sectoring (1=active)
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;
		head = (fd800.cmd_reg >> 9) & 1;
		/*non_seq_mode = (fd800.cmd_reg >> 8) & 1;*/
		sector = fd800.cmd_reg & 0x1f;

		fd800.unit = unit;
		fd800.head = head;
		fd800.sector = sector;
		/*fd800.non_seq_mode = non_seq_mode;*/

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		fd800_do_read();

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 5:		/* read ID
                    bits 16-24: 0s
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;
		head = (fd800.cmd_reg >> 9) & 1;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		if (!fd800_read_id(unit, head, &cylinder, &sector))
		{
			fd800.stat_reg |= status_ID_not_found;
		}
		else
		{
			fd800.recv_buf = (cylinder << 8) | sector;
			fd800.stat_reg |= status_OP_complete;
		}

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 6:		/* read unformatted
                    bits 16-20: sector number (1-26)
                    bits 21-24: 0s
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3) */
		/* ... */
		break;

	case 7:		/* write
                    bits 16-20: sector number (1-26)
                    bits 21-24: 0s
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;
		head = (fd800.cmd_reg >> 9) & 1;
		sector = fd800.cmd_reg & 0x1f;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		if ((fd800.sector == 0) || (fd800.sector > 26))
		{
			fd800.stat_reg |= status_invalid_cmd;
		}
		else
		{
			fd800.unit = unit;
			fd800.head = head;
			fd800.sector = sector;
			fd800.ddam = 0;

			fd800.buf_pos = 0;
			fd800.buf_mode = bm_write;
			fd800.stat_reg |= status_XFER_ready;
			fd800.stat_reg |= status_OP_complete;	/* right??? */
		}

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 8:		/* write delete
                    bits 16-20: sector number (1-26)
                    bits 21-24: 0s
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;
		head = (fd800.cmd_reg >> 9) & 1;
		sector = fd800.cmd_reg & 0x1f;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		if ((fd800.sector == 0) || (fd800.sector > 26))
		{
			fd800.stat_reg |= status_invalid_cmd;
		}
		else
		{
			fd800.unit = unit;
			fd800.head = head;
			fd800.sector = sector;
			fd800.ddam = 1;

			fd800.buf_pos = 0;
			fd800.buf_mode = bm_write;
			fd800.stat_reg |= status_XFER_ready;
			fd800.stat_reg |= status_OP_complete;	/* right??? */
		}

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 9:		/* format track
                    bits 16-23: track ID (0-255, normally current cylinder index, or 255 for bad track)
                    bit 24: verify only (1 - verify, 0 - format & verify)
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3) */
		/* ... */
		break;

	case 10:	/* load int mask
                    bit 16: bad mask for interrupt (0 = unmask or enable interrupt)
                    bits 17-27: 0s */
		fd800.interrupt_f_f = fd800.cmd_reg & 1;
		fd800_field_interrupt();
		break;

	case 11:	/* stop
                    bits 16-25: 0s
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;

		/* reset status */
		fd800.stat_reg = unit << status_unit_shift;

		fd800.stat_reg |= status_OP_complete;

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 12:	/* step head
                    bits 16-22: track number (0-76)
                    bits 23-25: 0s
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;
		cylinder = fd800.cmd_reg & 0x7f;

		if (cylinder > 76)
		{
			fd800.stat_reg |= status_invalid_cmd;
		}
		else if ((fd800.drv[unit].phys_cylinder != -1) || (!fd800_do_restore(unit)))
		{
			floppy_drive_seek(&fd800.drv[unit].img->device(), cylinder-fd800.drv[unit].phys_cylinder);
			fd800.stat_reg |= status_OP_complete;
		}

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 13:	/* maintenance commands
                    bits 16-23: according to extended command code
                    bits 24-27: extended command code (0-7) */
		switch ((fd800.cmd_reg >> 8) & 15)
		{
		case 0:	/* reset
                    bits 16-23: 0s */
			/* ... */
			break;
		case 1:	/* retry inhibit
                    bits 16-23: 0s */
			/* ... */
			break;
		case 2:	/* LED test
                    bit 16: 1
                    bits 17-19: 0s
                    bit 20: LED #2 enable
                    bit 21: LED #3 enable
                    bit 22: LED #4 enable
                    bit 23: enable LEDs */
			/* ... */
			break;
		case 3:	/* program error (a.k.a. invalid command)
                    bits 16-23: 0s */
			/* ... */
			break;
		case 4:	/* memory read
                    bits 16-20: controller memory address (shifted left by 8 to generate 9900 address)
                    bits 21-23: 0s */
			/* ... */
			break;
		case 5:	/* RAM load
                    bit 16: 0
                    bits 17-23: RAM offset (shifted left by 1 and offset by >1800 to generate 9900 address) */
			/* ... */
			break;
		case 6:	/* RAM run
                    bit 16: 0
                    bits 17-23: RAM offset (shifted left by 1 and offset by >1800 to generate 9900 address) */
			/* ... */
			break;
		case 7:	/* power up simulation
                    bits 16-23: 0s */
			/* ... */
			break;
		}
		/* ... */
		break;

	case 14:	/* IPL
                    bits 16-22: track number (0-76)
                    bit 23: 0
                    bit 24: no sequential sectoring (1=active)
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3) */
		unit = (fd800.cmd_reg >> 10) & 3;
		head = (fd800.cmd_reg >> 9) & 1;
		/*non_seq_mode = (fd800.cmd_reg >> 8) & 1;*/
		cylinder = fd800.cmd_reg & 0x7f;

		if (!fd800_do_seek(unit, cylinder, head))
		{
			fd800.unit = unit;
			fd800.head = head;
			fd800.sector = 1;
			/*fd800.non_seq_mode = non_seq_mode;*/

			fd800_do_read();
		}

		fd800.stat_reg |= status_interrupt;
		fd800_field_interrupt();
		break;

	case 15:	/* Clear Status port
                    bits 16-27: 0s */
		fd800.stat_reg = 0;
		fd800_field_interrupt();
		break;
	}
}

/*
    read one CRU bit

    0-15: receive buffer
    16-31: status:
        16: OP complete (1 -> complete???)
        17: Xfer ready (XFER) (1 -> ready???)
        18: drive not ready
        19: data check error
        20: seek error/??????
        21 invalid command/??????
        22: no address mark found/??????
        23: equipment check error/??????
        24: ID check error
        25: ID not found
        26: Controller busy (CTLBSY) (0 -> controller is ready)
        27: write protect
        28: deleted sector detected
        29: unit LSB
        30: unit MSB
        31: Interrupt (CBUSY???) (1 -> controller is ready)
*/
 READ8_HANDLER(fd800_cru_r)
{
	int reply = 0;

	switch (offset)
	{
	case 0:
	case 1:
		/* receive buffer */
		reply = fd800.recv_buf >> (offset*8);
		break;

	case 2:
	case 3:
		/* status register */
		reply = fd800.stat_reg >> ((offset-2)*8);
		break;
	}

	return reply;
}

/*
    write one CRU bit

    0-15: controller data word (transmit buffer)
    16-31: controller command word (command register)
    16-23: parameter value
    24: flag bit/extended command code
    25: head select/extended command code
    26: FD unit number LSB/extended command code
    27: FD unit number MSB/extended command code
    28-31: command code
*/
WRITE8_HANDLER(fd800_cru_w)
{
	switch (offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		/* transmit buffer */
		if (data)
			fd800.xmit_buf |= 1 << offset;
		else
			fd800.xmit_buf &= ~(1 << offset);
		if (offset == 15)
		{
			switch (fd800.buf_mode)
			{
			case bm_off:
				break;
			case bm_read:
				fd800.buf_pos++;
				if (fd800.buf_pos == fd800.drv[fd800.unit].seclen)
				{	/* end of sector */
					if (fd800.sector == 26)
					{	/* end of track -> end command (right???) */
						fd800.stat_reg &= ~status_XFER_ready;
						fd800.stat_reg |= status_OP_complete;
						fd800.stat_reg |= status_interrupt;
						fd800.buf_mode = bm_off;
						fd800_field_interrupt();
					}
					else
					{	/* read next sector */
						fd800.sector++;
						fd800.stat_reg &= ~status_XFER_ready | status_OP_complete | status_interrupt;
						fd800_do_read();
						fd800.stat_reg |= status_interrupt;
						fd800_field_interrupt();
					}
				}
				else
					fd800.recv_buf = (fd800.buf[fd800.buf_pos<<1] << 8) | fd800.buf[(fd800.buf_pos<<1)+1];
				break;

			case bm_write:
				fd800.buf[fd800.buf_pos<<1] = fd800.xmit_buf >> 8;
				fd800.buf[(fd800.buf_pos<<1)+1] = fd800.xmit_buf & 0xff;
				fd800.buf_pos++;
				if (fd800.buf_pos == fd800.drv[fd800.unit].seclen)
				{	/* end of sector */
					fd800_do_write();
					if (fd800.sector == 26)
					{
						/* end of track -> end command (right???) */
						fd800.stat_reg &= ~status_XFER_ready;
						fd800.stat_reg |= status_OP_complete;
						fd800.stat_reg |= status_interrupt;
						fd800.buf_mode = bm_off;
						fd800_field_interrupt();
					}
					else
					{	/* increment to next sector */
						fd800.sector++;
						fd800.stat_reg |= status_interrupt;
						fd800_field_interrupt();
					}
				}
				break;
			}
		}
		break;

	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
		/* command register */
		if (data)
			fd800.cmd_reg |= 1 << (offset-16);
		else
			fd800.cmd_reg &= ~(1 << (offset-16));
		if (offset == 31)
			fd800_do_cmd();
		break;
	}
}
