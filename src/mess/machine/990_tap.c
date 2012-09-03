/*
    990_tap.c: emulation of a generic ti990 tape controller, for use with
    TILINE-based TI990 systems (TI990/10, /12, /12LR, /10A, Business system 300
    and 300A).

    This core will emulate the common feature set found in every tape controller.
    Most controllers support additional features, but are still compatible with
    the basic feature set.  I have a little documentation on two specific
    tape controllers (MT3200 and WD800/WD800A), but I have not tried to emulate
    controller-specific features.


    Long description: see 2234398-9701 and 2306140-9701.


    Raphael Nabet 2002
*/
/*
    Image encoding:


    2 bytes: record len - little-endian
    2 bytes: always 0s (length MSBs?)
    len bytes: data
    2 bytes: record len - little-endian
    2 bytes: always 0s (length MSBs?)

    4 0s: EOF mark
*/

#include "emu.h"
#include "990_tap.h"


static void update_interrupt(device_t *device);

#define MAX_TAPE_UNIT 4

typedef struct tape_unit_t
{
	device_image_interface *img;		/* image descriptor */
	unsigned int bot : 1;	/* TRUE if we are at the beginning of tape */
	unsigned int eot : 1;	/* TRUE if we are at the end of tape */
	unsigned int wp : 1;	/* TRUE if tape is write-protected */
} tape_unit_t;

typedef struct _tap_990_t tap_990_t;
struct _tap_990_t
{
	UINT16 w[8];

	const ti990_tpc_interface *intf;

	tape_unit_t t[MAX_TAPE_UNIT];
};

typedef struct _ti990_tape_t ti990_tape_t;
struct _ti990_tape_t
{
	int dummy;
};

enum
{
	w0_offline			= 0x8000,
	w0_BOT				= 0x4000,
	w0_EOR				= 0x2000,
	w0_EOF				= 0x1000,
	w0_EOT				= 0x0800,
	w0_write_ring		= 0x0400,
	w0_tape_rewinding	= 0x0200,
	w0_command_timeout	= 0x0100,

	w0_rewind_status	= 0x00f0,
	w0_rewind_mask		= 0x000f,

	w6_unit0_sel		= 0x8000,
	w6_unit1_sel		= 0x4000,
	w6_unit2_sel		= 0x2000,
	w6_unit3_sel		= 0x1000,
	w6_command			= 0x0f00,

	w7_idle				= 0x8000,
	w7_complete			= 0x4000,
	w7_error			= 0x2000,
	w7_int_enable		= 0x1000,
	w7_PE_format		= 0x0200,
	w7_abnormal_completion	= 0x0100,
	w7_interface_parity_err	= 0x0080,
	w7_err_correction_enabled	= 0x0040,
	w7_hard_error			= 0x0020,
	w7_tiline_parity_err	= 0x0010,
	w7_tiline_timing_err	= 0x0008,
	w7_tiline_timeout_err	= 0x0004,
	/*w7_format_error       = 0x0002,*/
	w7_tape_error		= 0x0001
};

static const UINT16 w_mask[8] =
{
	0x000f,		/* Controllers should prevent overwriting of w0 status bits, and I know
                that some controllers do so. */
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xf3ff		/* Don't overwrite reserved bits */
};

static int tape_get_id(device_t *image)
{
	int drive =0;
	if (strcmp(image->tag(), ":tape0") == 0) drive = 0;
	if (strcmp(image->tag(), ":tape1") == 0) drive = 1;
	if (strcmp(image->tag(), ":tape2") == 0) drive = 2;
	if (strcmp(image->tag(), ":tape3") == 0) drive = 3;
	return drive;
}

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/
INLINE tap_990_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TI990_TAPE_CTRL);

	return (tap_990_t *)downcast<tap_990_device *>(device)->token();
}


/*
    Parse the tape select lines, and return the corresponding tape unit.
    (-1 if none)
*/
static int cur_tape_unit(device_t *device)
{
	int reply;
	tap_990_t *tpc = get_safe_token(device);

	if (tpc->w[6] & w6_unit0_sel)
		reply = 0;
	else if (tpc->w[6] & w6_unit1_sel)
		reply = 1;
	else if (tpc->w[6] & w6_unit2_sel)
		reply = 2;
	else if (tpc->w[6] & w6_unit3_sel)
		reply = 3;
	else
		reply = -1;

	if (reply >= MAX_TAPE_UNIT)
		reply = -1;

	return reply;
}

/*
    Update interrupt state
*/
static void update_interrupt(device_t *device)
{
	tap_990_t *tpc = get_safe_token(device);
	if (tpc->intf->interrupt_callback)
		(*tpc->intf->interrupt_callback)(device->machine(), (tpc->w[7] & w7_idle)
									&& (((tpc->w[7] & w7_int_enable) && (tpc->w[7] & (w7_complete | w7_error)))
										|| ((tpc->w[0] & ~(tpc->w[0] >> 4)) & w0_rewind_mask)));
}

/*
    Handle the read binary forward command: read the next record on tape.
*/
static void cmd_read_binary_forward(device_t *device)
{
	UINT8 buffer[256];
	int reclen;

	int dma_address;
	int char_count;
	int read_offset;

	int rec_count = 0;
	int chunk_len;
	int bytes_to_read;
	int bytes_read;
	int i;
	tap_990_t *tpc = get_safe_token(device);
	int tap_sel = cur_tape_unit(device);

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		return;
	}
	else if (! tpc->t[tap_sel].img->exists())
	{	/* offline */
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#if 0
	else if (0)
	{	/* rewind in progress */
		tpc->w[0] |= 0x80 >> tap_sel;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#endif

	tpc->t[tap_sel].bot = 0;

	dma_address = ((((int) tpc->w[6]) << 16) | tpc->w[5]) & 0x1ffffe;
	char_count = tpc->w[4];
	read_offset = tpc->w[3];

	bytes_read = tpc->t[tap_sel].img->fread(buffer, 4);
	if (bytes_read != 4)
	{
		if (bytes_read == 0)
		{	/* legitimate EOF */
			tpc->t[tap_sel].eot = 1;
			tpc->w[0] |= w0_EOT;	/* or should it be w0_command_timeout? */
			tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
			update_interrupt(device);
			goto update_registers;
		}
		else
		{	/* illegitimate EOF */
			/* No idea what to report... */
			/* eject tape to avoid catastrophes */
			logerror("Tape error\n");
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}
	}
	reclen = (((int) buffer[1]) << 8) | buffer[0];
	if (buffer[2] || buffer[3])
	{	/* no idea what these bytes mean */
		logerror("Tape error\n");
		logerror("Tape format looks gooofy\n");
		/* eject tape to avoid catastrophes */
		tpc->t[tap_sel].img->unload();
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		goto update_registers;
	}

	/* test for EOF mark */
	if (reclen == 0)
	{
		logerror("read binary forward: found EOF, requested %d\n", char_count);
		tpc->w[0] |= w0_EOF;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		goto update_registers;
	}

	logerror("read binary forward: rec length %d, requested %d\n", reclen, char_count);

	rec_count = reclen;

	/* skip up to read_offset bytes */
	chunk_len = (read_offset > rec_count) ? rec_count : read_offset;

	if (tpc->t[tap_sel].img->fseek(chunk_len, SEEK_CUR))
	{	/* eject tape */
		logerror("Tape error\n");
		tpc->t[tap_sel].img->unload();
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		goto update_registers;
	}

	rec_count -= chunk_len;
	read_offset -= chunk_len;
	if (read_offset)
	{
		tpc->w[0] |= w0_EOR;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		goto skip_trailer;
	}

	/* read up to char_count bytes */
	chunk_len = (char_count > rec_count) ? rec_count : char_count;

	for (; chunk_len>0; )
	{
		bytes_to_read = (chunk_len < sizeof(buffer)) ? chunk_len : sizeof(buffer);
		bytes_read = tpc->t[tap_sel].img->fread(buffer, bytes_to_read);

		if (bytes_read & 1)
		{
			buffer[bytes_read] = 0xff;
		}

		/* DMA */
		for (i=0; i<bytes_read; i+=2)
		{
			device->machine().device("maincpu")->memory().space(AS_PROGRAM)->write_word(dma_address, (((int) buffer[i]) << 8) | buffer[i+1]);
			dma_address = (dma_address + 2) & 0x1ffffe;
		}

		rec_count -= bytes_read;
		char_count -= bytes_read;
		chunk_len -= bytes_read;

		if (bytes_read != bytes_to_read)
		{	/* eject tape */
			logerror("Tape error\n");
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}
	}

	if (char_count)
	{
		tpc->w[0] |= w0_EOR;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		goto skip_trailer;
	}

	if (rec_count)
	{	/* skip end of record */
		if (tpc->t[tap_sel].img->fseek(rec_count, SEEK_CUR))
		{	/* eject tape */
			logerror("Tape error\n");
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}
	}

skip_trailer:
	if (tpc->t[tap_sel].img->fread(buffer, 4) != 4)
	{	/* eject tape */
		logerror("Tape error\n");
		tpc->t[tap_sel].img->unload();
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		goto update_registers;
	}

	if (reclen != ((((int) buffer[1]) << 8) | buffer[0]))
	{	/* eject tape */
		logerror("Tape error\n");
		tpc->t[tap_sel].img->unload();
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		goto update_registers;
	}
	if (buffer[2] || buffer[3])
	{	/* no idea what these bytes mean */
		logerror("Tape error\n");
		logerror("Tape format looks gooofy\n");
		/* eject tape to avoid catastrophes */
		tpc->t[tap_sel].img->unload();
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		goto update_registers;
	}

	if (! (tpc->w[7] & w7_error))
	{
		tpc->w[7] |= w7_idle | w7_complete;
		update_interrupt(device);
	}

update_registers:
	tpc->w[1] = rec_count & 0xffff;
	tpc->w[2] = (rec_count >> 8) & 0xff;
	tpc->w[3] = read_offset;
	tpc->w[4] = char_count;
	tpc->w[5] = (dma_address >> 1) & 0xffff;
	tpc->w[6] = (tpc->w[6] & 0xffe0) | ((dma_address >> 17) & 0xf);
}

/*
    Handle the record skip forward command: skip a specified number of records.
*/
static void cmd_record_skip_forward(device_t *device)
{
	UINT8 buffer[4];
	int reclen;

	int record_count;
	int bytes_read;
	tap_990_t *tpc = get_safe_token(device);
	int tap_sel = cur_tape_unit(device);

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		return;
	}
	else if (! tpc->t[tap_sel].img->exists())
	{	/* offline */
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#if 0
	else if (0)
	{	/* rewind in progress */
		tpc->w[0] |= 0x80 >> tap_sel;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#endif

	record_count = tpc->w[4];

	if (record_count)
		tpc->t[tap_sel].bot = 0;

	while (record_count > 0)
	{
		bytes_read = tpc->t[tap_sel].img->fread(buffer, 4);
		if (bytes_read != 4)
		{
			if (bytes_read == 0)
			{	/* legitimate EOF */
				tpc->t[tap_sel].eot = 1;
				tpc->w[0] |= w0_EOT;	/* or should it be w0_command_timeout? */
				tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
				update_interrupt(device);
				goto update_registers;
			}
			else
			{	/* illegitimate EOF */
				/* No idea what to report... */
				/* eject tape to avoid catastrophes */
				tpc->t[tap_sel].img->unload();
				tpc->w[0] |= w0_offline;
				tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
				update_interrupt(device);
				goto update_registers;
			}
		}
		reclen = (((int) buffer[1]) << 8) | buffer[0];
		if (buffer[2] || buffer[3])
		{	/* no idea what these bytes mean */
			logerror("Tape format looks gooofy\n");
			/* eject tape to avoid catastrophes */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}

		/* test for EOF mark */
		if (reclen == 0)
		{
			logerror("record skip forward: found EOF\n");
			tpc->w[0] |= w0_EOF;
			tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
			update_interrupt(device);
			goto update_registers;
		}

		/* skip record data */
		if (tpc->t[tap_sel].img->fseek(reclen, SEEK_CUR))
		{	/* eject tape */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}

		if (tpc->t[tap_sel].img->fread(buffer, 4) != 4)
		{	/* eject tape */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}

		if (reclen != ((((int) buffer[1]) << 8) | buffer[0]))
		{	/* eject tape */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}
		if (buffer[2] || buffer[3])
		{	/* no idea what these bytes mean */
			logerror("Tape format looks gooofy\n");
			/* eject tape to avoid catastrophes */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}

		record_count--;
	}

	tpc->w[7] |= w7_idle | w7_complete;
	update_interrupt(device);

update_registers:
	tpc->w[4] = record_count;
}

/*
    Handle the record skip reverse command: skip a specified number of records backwards.
*/
static void cmd_record_skip_reverse(device_t *device)
{
	UINT8 buffer[4];
	int reclen;

	int record_count;

	int bytes_read;
	tap_990_t *tpc = get_safe_token(device);
	int tap_sel = cur_tape_unit(device);

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		return;
	}
	else if (! tpc->t[tap_sel].img->exists())
	{	/* offline */
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#if 0
	else if (0)
	{	/* rewind in progress */
		tpc->w[0] |= 0x80 >> tap_sel;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#endif

	record_count = tpc->w[4];

	if (record_count)
		tpc->t[tap_sel].eot = 0;

	while (record_count > 0)
	{
		if (tpc->t[tap_sel].img->ftell() == 0)
		{	/* bot */
			tpc->t[tap_sel].bot = 1;
			tpc->w[0] |= w0_BOT;
			tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
			update_interrupt(device);
			goto update_registers;
		}
		if (tpc->t[tap_sel].img->fseek(-4, SEEK_CUR))
		{	/* eject tape */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}
		bytes_read = tpc->t[tap_sel].img->fread(buffer, 4);
		if (bytes_read != 4)
		{
			/* illegitimate EOF */
			/* No idea what to report... */
			/* eject tape to avoid catastrophes */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}
		reclen = (((int) buffer[1]) << 8) | buffer[0];
		if (buffer[2] || buffer[3])
		{	/* no idea what these bytes mean */
			logerror("Tape format looks gooofy\n");
			/* eject tape to avoid catastrophes */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}

		/* look for EOF mark */
		if (reclen == 0)
		{
			logerror("record skip reverse: found EOF\n");
			if (tpc->t[tap_sel].img->fseek(-4, SEEK_CUR))
			{	/* eject tape */
				tpc->t[tap_sel].img->unload();
				tpc->w[0] |= w0_offline;
				tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
				update_interrupt(device);
				goto update_registers;
			}
			tpc->w[0] |= w0_EOF;
			tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
			update_interrupt(device);
			goto update_registers;
		}

		if (tpc->t[tap_sel].img->fseek(-reclen-8, SEEK_CUR))
		{	/* eject tape */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}

		if (tpc->t[tap_sel].img->fread(buffer, 4) != 4)
		{	/* eject tape */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}
		if (reclen != ((((int) buffer[1]) << 8) | buffer[0]))
		{	/* eject tape */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}
		if (buffer[2] || buffer[3])
		{	/* no idea what these bytes mean */
			logerror("Tape format looks gooofy\n");
			/* eject tape to avoid catastrophes */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}

		if (tpc->t[tap_sel].img->fseek(-4, SEEK_CUR))
		{	/* eject tape */
			tpc->t[tap_sel].img->unload();
			tpc->w[0] |= w0_offline;
			tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt(device);
			goto update_registers;
		}

		record_count--;
	}

	tpc->w[7] |= w7_idle | w7_complete;
	update_interrupt(device);

update_registers:
	tpc->w[4] = record_count;
}

/*
    Handle the rewind command: rewind to BOT.
*/
static void cmd_rewind(device_t *device)
{
	tap_990_t *tpc = get_safe_token(device);
	int tap_sel = cur_tape_unit(device);

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		return;
	}
	else if (! tpc->t[tap_sel].img->exists())
	{	/* offline */
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#if 0
	else if (0)
	{	/* rewind in progress */
		tpc->w[0] |= 0x80 >> tap_sel;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#endif

	tpc->t[tap_sel].eot = 0;

	if (tpc->t[tap_sel].img->fseek(0, SEEK_SET))
	{	/* eject tape */
		tpc->t[tap_sel].img->unload();
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		return;
	}
	tpc->t[tap_sel].bot = 1;

	tpc->w[7] |= w7_idle | w7_complete;
	update_interrupt(device);
}

/*
    Handle the rewind and offline command: disable the tape unit.
*/
static void cmd_rewind_and_offline(device_t *device)
{
	tap_990_t *tpc = get_safe_token(device);
	int tap_sel = cur_tape_unit(device);

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		return;
	}
	else if (! tpc->t[tap_sel].img->exists())
	{	/* offline */
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#if 0
	else if (0)
	{	/* rewind in progress */
		tpc->w[0] |= 0x80 >> tap_sel;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
		return;
	}
#endif

	/* eject tape */
	tpc->t[tap_sel].img->unload();

	tpc->w[7] |= w7_idle | w7_complete;
	update_interrupt(device);
}

/*
    Handle the read transport status command: return the current tape status.
*/
static void read_transport_status(device_t *device)
{
	tap_990_t *tpc = get_safe_token(device);
	int tap_sel = cur_tape_unit(device);

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
	}
	else if (! tpc->t[tap_sel].img->exists())
	{	/* offline */
		tpc->w[0] |= w0_offline;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
	}
#if 0
	else if (0)
	{	/* rewind in progress */
		tpc->w[0] |= /*...*/;
		tpc->w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt(device);
	}
#endif
	else
	{	/* no particular error condition */
		if (tpc->t[tap_sel].bot)
			tpc->w[0] |= w0_BOT;
		if (tpc->t[tap_sel].eot)
			tpc->w[0] |= w0_EOT;
		if (tpc->t[tap_sel].wp)
			tpc->w[0] |= w0_write_ring;
		tpc->w[7] |= w7_idle | w7_complete;
		update_interrupt(device);
	}
}

/*
    Parse command code and execute the command.
*/
static void execute_command(device_t *device)
{
	/* hack */
	tap_990_t *tpc = get_safe_token(device);
	tpc->w[0] &= 0xff;

	switch ((tpc->w[6] & w6_command) >> 8)
	{
	case 0x00:
	case 0x0C:
	case 0x0E:
		/* NOP */
		logerror("NOP\n");
		tpc->w[7] |= w7_idle | w7_complete;
		update_interrupt(device);
		break;
	case 0x01:
		/* buffer sync: means nothing under emulation */
		logerror("buffer sync\n");
		tpc->w[7] |= w7_idle | w7_complete;
		update_interrupt(device);
		break;
	case 0x02:
		/* write EOF - not emulated */
		logerror("write EOF\n");
		/* ... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		break;
	case 0x03:
		/* record skip reverse - not fully tested */
		logerror("record skip reverse\n");
		cmd_record_skip_reverse(device);
		break;
	case 0x04:
		/* read binary forward */
		logerror("read binary forward\n");
		cmd_read_binary_forward(device);
		break;
	case 0x05:
		/* record skip forward - not tested */
		logerror("record skip forward\n");
		cmd_record_skip_forward(device);
		break;
	case 0x06:
		/* write binary forward - not emulated */
		logerror("write binary forward\n");
		/* ... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		break;
	case 0x07:
		/* erase - not emulated */
		logerror("erase\n");
		/* ... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		break;
	case 0x08:
	case 0x09:
		/* read transport status */
		logerror("read transport status\n");
		read_transport_status(device);
		break;
	case 0x0A:
		/* rewind - not tested */
		logerror("rewind\n");
		cmd_rewind(device);
		break;
	case 0x0B:
		/* rewind and offline - not tested */
		logerror("rewind and offline\n");
		cmd_rewind_and_offline(device);
		break;
	case 0x0F:
		/* extended control and status - not emulated */
		logerror("extended control and status\n");
		/* ... */
		tpc->w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt(device);
		break;
	}
}


/*
    Read one register in TPCS space
*/
READ16_DEVICE_HANDLER(ti990_tpc_r)
{
	tap_990_t *tpc = get_safe_token(device);
	if (offset < 8)
		return tpc->w[offset];
	else
		return 0;
}

/*
    Write one register in TPCS space.  Execute command if w7_idle is cleared.
*/
WRITE16_DEVICE_HANDLER(ti990_tpc_w)
{
	tap_990_t *tpc = get_safe_token(device);
	if (offset < 8)
	{
		/* write protect if a command is in progress */
		if (tpc->w[7] & w7_idle)
		{
			UINT16 old_data = tpc->w[offset];

			/* Only write writable bits AND honor byte accesses (ha!) */
			tpc->w[offset] = (tpc->w[offset] & ((~w_mask[offset]) | mem_mask)) | (data & w_mask[offset] & ~mem_mask);

			if ((offset == 0) || (offset == 7))
				update_interrupt(device);

			if ((offset == 7) && (old_data & w7_idle) && ! (data & w7_idle))
			{	/* idle has been cleared: start command execution */
				execute_command(device);
			}
		}
	}
}

class ti990_tape_image_device :	public device_t,
									public device_image_interface
{
public:
	// construction/destruction
	ti990_tape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual iodevice_t image_type() const { return IO_MAGTAPE; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const { return "tap"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	virtual bool call_load();
	virtual void call_unload();
protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();
};

const device_type TI990_TAPE = &device_creator<ti990_tape_image_device>;

ti990_tape_image_device::ti990_tape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, TI990_TAPE, "TI990 Magnetic Tape", tag, owner, clock),
	  device_image_interface(mconfig, *this)
{
}

void ti990_tape_image_device::device_config_complete()
{
	update_names();
}

void ti990_tape_image_device::device_start()
{
	tape_unit_t *t;
	tap_990_t *tpc = get_safe_token(owner());
	int id = tape_get_id(this);

	t = &tpc->t[id];
	memset(t, 0, sizeof(*t));

	t->img = this;
	t->wp = 1;
	t->bot = 0;
	t->eot = 0;
}

/*
    Open a tape image
*/
bool ti990_tape_image_device::call_load()
{
	tape_unit_t *t;
	tap_990_t *tpc = get_safe_token(owner());
	int id = tape_get_id(this);

	t = &tpc->t[id];
	memset(t, 0, sizeof(*t));

	/* tell whether the image is writable */
	t->wp = is_readonly();

	t->bot = 1;

	return IMAGE_INIT_PASS;
}

/*
    Close a tape image
*/
void ti990_tape_image_device::call_unload()
{
	tape_unit_t *t;
	tap_990_t *tpc = get_safe_token(owner());
	int id = tape_get_id(this);

	t = &tpc->t[id];
	t->wp = 1;
	t->bot = 0;
	t->eot = 0;
}

#define MCFG_TI990_TAPE_ADD(_tag)	\
	MCFG_DEVICE_ADD((_tag),  TI990_TAPE, 0)


static MACHINE_CONFIG_FRAGMENT( tap_990 )
	MCFG_TI990_TAPE_ADD("tape0")
	MCFG_TI990_TAPE_ADD("tape1")
	MCFG_TI990_TAPE_ADD("tape2")
	MCFG_TI990_TAPE_ADD("tape3")
MACHINE_CONFIG_END

/*
    Init the tape controller core
*/
static DEVICE_START(tap_990)
{
	tap_990_t *tpc = get_safe_token(device);
	/* verify that we have an interface assigned */
	assert(device->static_config() != NULL);

	/* copy interface pointer */
	tpc->intf = (const ti990_tpc_interface*)device->static_config();

	memset(tpc->w, 0, sizeof(tpc->w));
	/* The PE bit is always set for the MT3200 (but not MT1600) */
	/* According to MT3200 manual, w7 bit #4 (reserved) is always set */
	tpc->w[7] = w7_idle /*| w7_PE_format*/ | 0x0800;

	update_interrupt(device);
}


DEVICE_GET_INFO( tap_990 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(tap_990_t);								break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_MACHINE_CONFIG:				info->machine_config = MACHINE_CONFIG_NAME(tap_990);		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(tap_990);					break;
		case DEVINFO_FCT_STOP:							/* Nothing */												break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Generic TI990 Tape Controller");								break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "TI990 Tape Controller");								break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");										break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);									break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright the MESS Team"); 				break;
	}
}

const device_type TI990_TAPE_CTRL = &device_creator<tap_990_device>;

tap_990_device::tap_990_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TI990_TAPE_CTRL, "Generic TI990 Tape Controller", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(tap_990_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tap_990_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tap_990_device::device_start()
{
	DEVICE_START_NAME( tap_990 )(this);
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor tap_990_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tap_990  );
}


