// license:GPL-2.0+
// copyright-holders:Raphael Nabet
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

#include "imagedev/magtape.h"

#include <algorithm>
#include <iterator>

enum
{
	w0_offline          = 0x8000,
	w0_BOT              = 0x4000,
	w0_EOR              = 0x2000,
	w0_EOF              = 0x1000,
	w0_EOT              = 0x0800,
	w0_write_ring       = 0x0400,
	w0_tape_rewinding   = 0x0200,
	w0_command_timeout  = 0x0100,

	w0_rewind_status    = 0x00f0,
	w0_rewind_mask      = 0x000f,

	w6_unit0_sel        = 0x8000,
	w6_unit1_sel        = 0x4000,
	w6_unit2_sel        = 0x2000,
	w6_unit3_sel        = 0x1000,
	w6_command          = 0x0f00,

	w7_idle             = 0x8000,
	w7_complete         = 0x4000,
	w7_error            = 0x2000,
	w7_int_enable       = 0x1000,
	w7_PE_format        = 0x0200,
	w7_abnormal_completion  = 0x0100,
	w7_interface_parity_err = 0x0080,
	w7_err_correction_enabled   = 0x0040,
	w7_hard_error           = 0x0020,
	w7_tiline_parity_err    = 0x0010,
	w7_tiline_timing_err    = 0x0008,
	w7_tiline_timeout_err   = 0x0004,
	/*w7_format_error       = 0x0002,*/
	w7_tape_error       = 0x0001
};

static const uint16_t w_mask[8] =
{
	0x000f,     /* Controllers should prevent overwriting of w0 status bits, and I know
	            that some controllers do so. */
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xffff,
	0xf3ff      /* Don't overwrite reserved bits */
};

/*
    Parse the tape select lines, and return the corresponding tape unit.
    (-1 if none)
*/
int tap_990_device::cur_tape_unit()
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

	if (reply >= MAX_TAPE_UNIT)
		reply = -1;

	return reply;
}

/*
    Update interrupt state
*/
void tap_990_device::update_interrupt()
{
	bool level = (m_w[7] & w7_idle)
				&& (((m_w[7] & w7_int_enable) && (m_w[7] & (w7_complete | w7_error)))
										|| ((m_w[0] & ~(m_w[0] >> 4)) & w0_rewind_mask));
	m_int_line(level);
}

/*
    Handle the read binary forward command: read the next record on tape.
*/
void tap_990_device::cmd_read_binary_forward()
{
	uint8_t buffer[256];
	int reclen;

	int dma_address;
	int char_count;
	int read_offset;

	int rec_count = 0;
	int chunk_len;
	int bytes_to_read;
	int bytes_read;
	int i;

	int tap_sel = cur_tape_unit();

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		return;
	}
	else if (!m_tape[tap_sel].img->exists())
	{   /* offline */
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#if 0
	else if (0)
	{   /* rewind in progress */
		m_w[0] |= 0x80 >> tap_sel;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#endif

	m_tape[tap_sel].bot = 0;

	dma_address = ((((int)m_w[6]) << 16) | m_w[5]) & 0x1ffffe;
	char_count = m_w[4];
	read_offset = m_w[3];

	bytes_read = m_tape[tap_sel].img->fread(buffer, 4);
	if (bytes_read != 4)
	{
		if (bytes_read == 0)
		{   /* legitimate EOF */
			m_tape[tap_sel].eot = 1;
			m_w[0] |= w0_EOT;    /* or should it be w0_command_timeout? */
			m_w[7] |= w7_idle | w7_error | w7_tape_error;
			update_interrupt();
			goto update_registers;
		}
		else
		{   /* illegitimate EOF */
			/* No idea what to report... */
			/* eject tape to avoid catastrophes */
			logerror("Tape error\n");
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}
	}
	reclen = (((int) buffer[1]) << 8) | buffer[0];
	if (buffer[2] || buffer[3])
	{   /* no idea what these bytes mean */
		logerror("Tape error\n");
		logerror("Tape format looks gooofy\n");
		/* eject tape to avoid catastrophes */
		m_tape[tap_sel].img->unload();
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		goto update_registers;
	}

	/* test for EOF mark */
	if (reclen == 0)
	{
		logerror("read binary forward: found EOF, requested %d\n", char_count);
		m_w[0] |= w0_EOF;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		goto update_registers;
	}

	logerror("read binary forward: rec length %d, requested %d\n", reclen, char_count);

	rec_count = reclen;

	/* skip up to read_offset bytes */
	chunk_len = (read_offset > rec_count) ? rec_count : read_offset;

	if (m_tape[tap_sel].img->fseek(chunk_len, SEEK_CUR))
	{   /* eject tape */
		logerror("Tape error\n");
		m_tape[tap_sel].img->unload();
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		goto update_registers;
	}

	rec_count -= chunk_len;
	read_offset -= chunk_len;
	if (read_offset)
	{
		m_w[0] |= w0_EOR;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		goto skip_trailer;
	}

	/* read up to char_count bytes */
	chunk_len = (char_count > rec_count) ? rec_count : char_count;

	for (; chunk_len>0; )
	{
		bytes_to_read = (chunk_len < sizeof(buffer)) ? chunk_len : sizeof(buffer);
		bytes_read = m_tape[tap_sel].img->fread(buffer, bytes_to_read);

		if (bytes_read & 1)
		{
			buffer[bytes_read] = 0xff;
		}

		/* DMA */
		for (i=0; i<bytes_read; i+=2)
		{
			m_memory_space->write_word(dma_address, (((int) buffer[i]) << 8) | buffer[i+1]);
			dma_address = (dma_address + 2) & 0x1ffffe;
		}

		rec_count -= bytes_read;
		char_count -= bytes_read;
		chunk_len -= bytes_read;

		if (bytes_read != bytes_to_read)
		{   /* eject tape */
			logerror("Tape error\n");
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}
	}

	if (char_count)
	{
		m_w[0] |= w0_EOR;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		goto skip_trailer;
	}

	if (rec_count)
	{   /* skip end of record */
		if (m_tape[tap_sel].img->fseek(rec_count, SEEK_CUR))
		{   /* eject tape */
			logerror("Tape error\n");
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}
	}

skip_trailer:
	if (m_tape[tap_sel].img->fread(buffer, 4) != 4)
	{   /* eject tape */
		logerror("Tape error\n");
		m_tape[tap_sel].img->unload();
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		goto update_registers;
	}

	if (reclen != ((((int) buffer[1]) << 8) | buffer[0]))
	{   /* eject tape */
		logerror("Tape error\n");
		m_tape[tap_sel].img->unload();
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		goto update_registers;
	}
	if (buffer[2] || buffer[3])
	{   /* no idea what these bytes mean */
		logerror("Tape error\n");
		logerror("Tape format looks gooofy\n");
		/* eject tape to avoid catastrophes */
		m_tape[tap_sel].img->unload();
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		goto update_registers;
	}

	if (! (m_w[7] & w7_error))
	{
		m_w[7] |= w7_idle | w7_complete;
		update_interrupt();
	}

update_registers:
	m_w[1] = rec_count & 0xffff;
	m_w[2] = (rec_count >> 8) & 0xff;
	m_w[3] = read_offset;
	m_w[4] = char_count;
	m_w[5] = (dma_address >> 1) & 0xffff;
	m_w[6] = (m_w[6] & 0xffe0) | ((dma_address >> 17) & 0xf);
}

/*
    Handle the record skip forward command: skip a specified number of records.
*/
void tap_990_device::cmd_record_skip_forward()
{
	uint8_t buffer[4];
	int reclen;

	int record_count;
	int bytes_read;

	int tap_sel = cur_tape_unit();

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		return;
	}
	else if (! m_tape[tap_sel].img->exists())
	{   /* offline */
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#if 0
	else if (0)
	{   /* rewind in progress */
		m_w[0] |= 0x80 >> tap_sel;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#endif

	record_count = m_w[4];

	if (record_count)
		m_tape[tap_sel].bot = 0;

	while (record_count > 0)
	{
		bytes_read = m_tape[tap_sel].img->fread(buffer, 4);
		if (bytes_read != 4)
		{
			if (bytes_read == 0)
			{   /* legitimate EOF */
				m_tape[tap_sel].eot = 1;
				m_w[0] |= w0_EOT;    /* or should it be w0_command_timeout? */
				m_w[7] |= w7_idle | w7_error | w7_tape_error;
				update_interrupt();
				goto update_registers;
			}
			else
			{   /* illegitimate EOF */
				/* No idea what to report... */
				/* eject tape to avoid catastrophes */
				m_tape[tap_sel].img->unload();
				m_w[0] |= w0_offline;
				m_w[7] |= w7_idle | w7_error | w7_hard_error;
				update_interrupt();
				goto update_registers;
			}
		}
		reclen = (((int) buffer[1]) << 8) | buffer[0];
		if (buffer[2] || buffer[3])
		{   /* no idea what these bytes mean */
			logerror("Tape format looks gooofy\n");
			/* eject tape to avoid catastrophes */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}

		/* test for EOF mark */
		if (reclen == 0)
		{
			logerror("record skip forward: found EOF\n");
			m_w[0] |= w0_EOF;
			m_w[7] |= w7_idle | w7_error | w7_tape_error;
			update_interrupt();
			goto update_registers;
		}

		/* skip record data */
		if (m_tape[tap_sel].img->fseek(reclen, SEEK_CUR))
		{   /* eject tape */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}

		if (m_tape[tap_sel].img->fread(buffer, 4) != 4)
		{   /* eject tape */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}

		if (reclen != ((((int) buffer[1]) << 8) | buffer[0]))
		{   /* eject tape */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}
		if (buffer[2] || buffer[3])
		{   /* no idea what these bytes mean */
			logerror("Tape format looks gooofy\n");
			/* eject tape to avoid catastrophes */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}

		record_count--;
	}

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();

update_registers:
	m_w[4] = record_count;
}

/*
    Handle the record skip reverse command: skip a specified number of records backwards.
*/
void tap_990_device::cmd_record_skip_reverse()
{
	uint8_t buffer[4];
	int reclen;

	int record_count;

	int bytes_read;

	int tap_sel = cur_tape_unit();

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		return;
	}
	else if (! m_tape[tap_sel].img->exists())
	{   /* offline */
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#if 0
	else if (0)
	{   /* rewind in progress */
		m_w[0] |= 0x80 >> tap_sel;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#endif

	record_count = m_w[4];

	if (record_count)
		m_tape[tap_sel].eot = 0;

	while (record_count > 0)
	{
		if (m_tape[tap_sel].img->ftell() == 0)
		{   /* bot */
			m_tape[tap_sel].bot = 1;
			m_w[0] |= w0_BOT;
			m_w[7] |= w7_idle | w7_error | w7_tape_error;
			update_interrupt();
			goto update_registers;
		}
		if (m_tape[tap_sel].img->fseek(-4, SEEK_CUR))
		{   /* eject tape */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}
		bytes_read = m_tape[tap_sel].img->fread(buffer, 4);
		if (bytes_read != 4)
		{
			/* illegitimate EOF */
			/* No idea what to report... */
			/* eject tape to avoid catastrophes */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}
		reclen = (((int) buffer[1]) << 8) | buffer[0];
		if (buffer[2] || buffer[3])
		{   /* no idea what these bytes mean */
			logerror("Tape format looks gooofy\n");
			/* eject tape to avoid catastrophes */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}

		/* look for EOF mark */
		if (reclen == 0)
		{
			logerror("record skip reverse: found EOF\n");
			if (m_tape[tap_sel].img->fseek(-4, SEEK_CUR))
			{   /* eject tape */
				m_tape[tap_sel].img->unload();
				m_w[0] |= w0_offline;
				m_w[7] |= w7_idle | w7_error | w7_hard_error;
				update_interrupt();
				goto update_registers;
			}
			m_w[0] |= w0_EOF;
			m_w[7] |= w7_idle | w7_error | w7_tape_error;
			update_interrupt();
			goto update_registers;
		}

		if (m_tape[tap_sel].img->fseek(-reclen-8, SEEK_CUR))
		{   /* eject tape */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}

		if (m_tape[tap_sel].img->fread(buffer, 4) != 4)
		{   /* eject tape */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}
		if (reclen != ((((int) buffer[1]) << 8) | buffer[0]))
		{   /* eject tape */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}
		if (buffer[2] || buffer[3])
		{   /* no idea what these bytes mean */
			logerror("Tape format looks gooofy\n");
			/* eject tape to avoid catastrophes */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}

		if (m_tape[tap_sel].img->fseek(-4, SEEK_CUR))
		{   /* eject tape */
			m_tape[tap_sel].img->unload();
			m_w[0] |= w0_offline;
			m_w[7] |= w7_idle | w7_error | w7_hard_error;
			update_interrupt();
			goto update_registers;
		}

		record_count--;
	}

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();

update_registers:
	m_w[4] = record_count;
}

/*
    Handle the rewind command: rewind to BOT.
*/
void tap_990_device::cmd_rewind()
{
	int tap_sel = cur_tape_unit();

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		return;
	}
	else if (! m_tape[tap_sel].img->exists())
	{   /* offline */
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#if 0
	else if (0)
	{   /* rewind in progress */
		m_w[0] |= 0x80 >> tap_sel;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#endif

	m_tape[tap_sel].eot = 0;

	if (m_tape[tap_sel].img->fseek(0, SEEK_SET))
	{   /* eject tape */
		m_tape[tap_sel].img->unload();
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		return;
	}
	m_tape[tap_sel].bot = 1;

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();
}

/*
    Handle the rewind and offline command: disable the tape unit.
*/
void tap_990_device::cmd_rewind_and_offline()
{
	int tap_sel = cur_tape_unit();

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		return;
	}
	else if (! m_tape[tap_sel].img->exists())
	{   /* offline */
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#if 0
	else if (0)
	{   /* rewind in progress */
		m_w[0] |= 0x80 >> tap_sel;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
		return;
	}
#endif

	/* eject tape */
	m_tape[tap_sel].img->unload();

	m_w[7] |= w7_idle | w7_complete;
	update_interrupt();
}

/*
    Handle the read transport status command: return the current tape status.
*/
void tap_990_device::read_transport_status()
{
	int tap_sel = cur_tape_unit();

	if (tap_sel == -1)
	{
		/* No idea what to report... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
	}
	else if (! m_tape[tap_sel].img->exists())
	{   /* offline */
		m_w[0] |= w0_offline;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
	}
#if 0
	else if (0)
	{   /* rewind in progress */
		m_w[0] |= /*...*/;
		m_w[7] |= w7_idle | w7_error | w7_tape_error;
		update_interrupt();
	}
#endif
	else
	{   /* no particular error condition */
		if (m_tape[tap_sel].bot)
			m_w[0] |= w0_BOT;
		if (m_tape[tap_sel].eot)
			m_w[0] |= w0_EOT;
		if (m_tape[tap_sel].wp)
			m_w[0] |= w0_write_ring;
		m_w[7] |= w7_idle | w7_complete;
		update_interrupt();
	}
}

/*
    Parse command code and execute the command.
*/
void tap_990_device::execute_command()
{
	/* hack */
	m_w[0] &= 0xff;

	switch ((m_w[6] & w6_command) >> 8)
	{
	case 0x00:
	case 0x0C:
	case 0x0E:
		/* NOP */
		logerror("NOP\n");
		m_w[7] |= w7_idle | w7_complete;
		update_interrupt();
		break;
	case 0x01:
		/* buffer sync: means nothing under emulation */
		logerror("buffer sync\n");
		m_w[7] |= w7_idle | w7_complete;
		update_interrupt();
		break;
	case 0x02:
		/* write EOF - not emulated */
		logerror("write EOF\n");
		/* ... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		break;
	case 0x03:
		/* record skip reverse - not fully tested */
		logerror("record skip reverse\n");
		cmd_record_skip_reverse();
		break;
	case 0x04:
		/* read binary forward */
		logerror("read binary forward\n");
		cmd_read_binary_forward();
		break;
	case 0x05:
		/* record skip forward - not tested */
		logerror("record skip forward\n");
		cmd_record_skip_forward();
		break;
	case 0x06:
		/* write binary forward - not emulated */
		logerror("write binary forward\n");
		/* ... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		break;
	case 0x07:
		/* erase - not emulated */
		logerror("erase\n");
		/* ... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		break;
	case 0x08:
	case 0x09:
		/* read transport status */
		logerror("read transport status\n");
		read_transport_status();
		break;
	case 0x0A:
		/* rewind - not tested */
		logerror("rewind\n");
		cmd_rewind();
		break;
	case 0x0B:
		/* rewind and offline - not tested */
		logerror("rewind and offline\n");
		cmd_rewind_and_offline();
		break;
	case 0x0F:
		/* extended control and status - not emulated */
		logerror("extended control and status\n");
		/* ... */
		m_w[7] |= w7_idle | w7_error | w7_hard_error;
		update_interrupt();
		break;
	}
}


/*
    Read one register in TPCS space
*/
uint16_t tap_990_device::read(offs_t offset)
{
	if (offset < 8)
		return m_w[offset];
	else
		return 0;
}

/*
    Write one register in TPCS space.  Execute command if w7_idle is cleared.
*/
void tap_990_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < 8)
	{
		/* write protect if a command is in progress */
		if (m_w[7] & w7_idle)
		{
			uint16_t old_data = m_w[offset];

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

class ti990_tape_image_device : public magtape_image_device
{
public:
	// construction/destruction
	ti990_tape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual const char *file_extensions() const noexcept override { return "tap"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	int tape_get_id();
};

DEFINE_DEVICE_TYPE(TI990_TAPE, ti990_tape_image_device, "ti990_tape_image", "TI-990 Magnetic Tape")

ti990_tape_image_device::ti990_tape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: magtape_image_device(mconfig, TI990_TAPE, tag, owner, clock)
{
}

void ti990_tape_image_device::device_start()
{
	tap_990_device* tpc = downcast<tap_990_device*>(owner());
	tpc->set_tape(tape_get_id(), this, true, false, false);
}

int ti990_tape_image_device::tape_get_id()
{
	int drive =0;
	if (strcmp(tag(), ":tape0") == 0) drive = 0;
	if (strcmp(tag(), ":tape1") == 0) drive = 1;
	if (strcmp(tag(), ":tape2") == 0) drive = 2;
	if (strcmp(tag(), ":tape3") == 0) drive = 3;
	return drive;
}

/*
    Open a tape image
*/
std::pair<std::error_condition, std::string> ti990_tape_image_device::call_load()
{
	tap_990_device* tpc = downcast<tap_990_device*>(owner());
	tpc->set_tape(tape_get_id(), this, true, false, is_readonly());

	return std::make_pair(std::error_condition(), std::string());
}

/*
    Close a tape image
*/
void ti990_tape_image_device::call_unload()
{
	tap_990_device* tpc = downcast<tap_990_device*>(owner());
	tpc->set_tape(tape_get_id(), this, false, false, true);
}


DEFINE_DEVICE_TYPE(TI990_TAPE_CTRL, tap_990_device, "ti990_tap", "Generic TI-900 Tape Controller")

tap_990_device::tap_990_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI990_TAPE_CTRL, tag, owner, clock)
	, m_memory_space(*this, finder_base::DUMMY_TAG, -1)
	, m_int_line(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tap_990_device::device_start()
{
	std::fill(std::begin(m_w), std::end(m_w), 0);

	// The PE bit is always set for the MT3200 (but not MT1600)
	// According to MT3200 manual, w7 bit #4 (reserved) is always set
	m_w[7] = w7_idle /*| w7_PE_format*/ | 0x0800;

	update_interrupt();
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tap_990_device::device_add_mconfig(machine_config &config)
{
	TI990_TAPE(config, "tape0", 0);
	TI990_TAPE(config, "tape1", 0);
	TI990_TAPE(config, "tape2", 0);
	TI990_TAPE(config, "tape3", 0);
}
