// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    x86log.c

    x86/x64 code logging helpers.

***************************************************************************/

#include "emu.h"
#include "x86log.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* comment parameters */
#define MAX_COMMENTS        4000
#define MAX_DATA_RANGES     1000
#define COMMENT_POOL_SIZE   (MAX_COMMENTS * 40)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* code logging info */
struct log_comment
{
	x86code *       base;
	const char *    string;
};


/* data ranges */
struct data_range_t
{
	x86code *       base;
	x86code *       end;
	int             size;
};


/* the code logging context */
struct x86log_context
{
	std::string     filename;                       /* name of the file */
	FILE *          file;                           /* file we are logging to */

	data_range_t    data_range[MAX_DATA_RANGES];    /* list of data ranges */
	int             data_range_count;               /* number of data ranges */

	log_comment     comment_list[MAX_COMMENTS];     /* list of comments */
	int             comment_count;                  /* number of live comments */

	char            comment_pool[COMMENT_POOL_SIZE];/* string pool to hold comments */
	char *          comment_pool_next;              /* pointer to next string pool location */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void reset_log(x86log_context *log);
extern int i386_dasm_one_ex(char *buffer, UINT64 eip, const UINT8 *oprom, int mode);



/***************************************************************************
    EXTERNAL INTERFACES
***************************************************************************/

/*-------------------------------------------------
    x86log_create_context - create a new context
-------------------------------------------------*/

x86log_context *x86log_create_context(const char *filename)
{
	x86log_context *log;

	/* allocate the log */
	log = global_alloc_clear(x86log_context);

	/* allocate the filename */
	log->filename.assign(filename);

	/* reset things */
	reset_log(log);
	return log;
}


/*-------------------------------------------------
    x86log_free_context - release a context
-------------------------------------------------*/

void x86log_free_context(x86log_context *log)
{
	/* close any open files */
	if (log->file != NULL)
		fclose(log->file);

	/* free the structure */
	global_free(log);
}


/*-------------------------------------------------
    x86log_add_comment - add a comment associated
    with a given code pointer
-------------------------------------------------*/

void x86log_add_comment(x86log_context *log, x86code *base, const char *format, ...)
{
	char *string = log->comment_pool_next;
	log_comment *comment;
	va_list va;

	assert(log->comment_count < MAX_COMMENTS);
	assert(log->comment_pool_next + strlen(format) + 256 < log->comment_pool + COMMENT_POOL_SIZE);

	/* we assume comments are registered in order; enforce this */
	assert(log->comment_count == 0 || base >= log->comment_list[log->comment_count - 1].base);

	/* if we exceed the maxima, skip it */
	if (log->comment_count >= MAX_COMMENTS)
		return;
	if (log->comment_pool_next + strlen(format) + 256 >= log->comment_pool + COMMENT_POOL_SIZE)
		return;

	/* do the printf to the string pool */
	va_start(va, format);
	log->comment_pool_next += vsprintf(log->comment_pool_next, format, va) + 1;
	va_end(va);

	/* fill in the new comment */
	comment = &log->comment_list[log->comment_count++];
	comment->base = base;
	comment->string = string;
}


/*-------------------------------------------------
    x86log_mark_as_data - mark a given range as
    data for logging purposes
-------------------------------------------------*/

void x86log_mark_as_data(x86log_context *log, x86code *base, x86code *end, int size)
{
	data_range_t *data;

	assert(log->data_range_count < MAX_DATA_RANGES);
	assert(end >= base);
	assert(size == 1 || size == 2 || size == 4 || size == 8);

	/* we assume data ranges are registered in order; enforce this */
	assert(log->data_range_count == 0 || base > log->data_range[log->data_range_count - 1].end);

	/* if we exceed the maxima, skip it */
	if (log->data_range_count >= MAX_DATA_RANGES)
		return;

	/* fill in the new range */
	data = &log->data_range[log->data_range_count++];
	data->base = base;
	data->end = end;
	data->size = size;
}


/*-------------------------------------------------
    x86log_disasm_code_range - disassemble a range
    of code and reset accumulated information
-------------------------------------------------*/

void x86log_disasm_code_range(x86log_context *log, const char *label, x86code *start, x86code *stop)
{
	const log_comment *lastcomment = &log->comment_list[log->comment_count];
	const log_comment *curcomment = &log->comment_list[0];
	const data_range_t *lastdata = &log->data_range[log->data_range_count];
	const data_range_t *curdata = &log->data_range[0];
	x86code *cur = start;

	/* print the optional label */
	if (label != NULL)
		x86log_printf(log, "\n%s\n", label);

	/* loop from the start until the cache top */
	while (cur < stop)
	{
		char buffer[100];
		int bytes;

		/* skip past any past data ranges */
		while (curdata < lastdata && cur > curdata->end)
			curdata++;

		/* skip past any past comments */
		while (curcomment < lastcomment && cur > curcomment->base)
			curcomment++;

		/* if we're in a data range, output the next chunk and continue */
		if (cur >= curdata->base && cur <= curdata->end)
		{
			bytes = curdata->size;
			switch (curdata->size)
			{
				default:
				case 1:     sprintf(buffer, "db      %02X", *cur);              break;
				case 2:     sprintf(buffer, "dw      %04X", *(UINT16 *)cur);    break;
				case 4:     sprintf(buffer, "dd      %08X", *(UINT32 *)cur);    break;
				case 8:     sprintf(buffer, "dq      %08X%08X", ((UINT32 *)cur)[1], ((UINT32 *)cur)[0]);    break;
			}
		}

		/* if we're not in the data range, skip filler opcodes */
		else if (*cur == 0xcc)
		{
			cur++;
			continue;
		}

		/* otherwise, do a disassembly of the current instruction */
		else
		{
#ifdef PTR64
			bytes = i386_dasm_one_ex(buffer, (FPTR)cur, cur, 64) & DASMFLAG_LENGTHMASK;
#else
			bytes = i386_dasm_one_ex(buffer, (FPTR)cur, cur, 32) & DASMFLAG_LENGTHMASK;
#endif
		}

		/* if we have a matching comment, output it */
		if (curcomment < lastcomment && cur == curcomment->base)
		{
			/* if we have additional matching comments at the same address, output them first */
			for ( ; curcomment + 1 < lastcomment && cur == curcomment[1].base; curcomment++)
				x86log_printf(log, "%p: %-50s; %s\n", cur, "", curcomment->string);
			x86log_printf(log, "%p: %-50s; %s\n", cur, buffer, curcomment->string);
		}

		/* if we don't, just print the disassembly and move on */
		else
			x86log_printf(log, "%p: %s\n", cur, buffer);

		/* advance past this instruction */
		cur += bytes;
	}

	/* reset our state */
	reset_log(log);
}


/*-------------------------------------------------
    x86log_printf - manually printf information to
    the log file
-------------------------------------------------*/

void x86log_printf(x86log_context *log, const char *format, ...)
{
	va_list va;

	/* open the file, creating it if necessary */
	if (log->file == NULL)
		log->file = fopen(log->filename.c_str(), "w");
	if (log->file == NULL)
		return;

	/* do the printf */
	va_start(va, format);
	vfprintf(log->file, format, va);
	va_end(va);

	/* flush the file */
	fflush(log->file);
}



/***************************************************************************
    LOCAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    reset_log - reset the state of the log
-------------------------------------------------*/

static void reset_log(x86log_context *log)
{
	log->data_range_count = 0;
	log->comment_count = 0;
	log->comment_pool_next = log->comment_pool;
}
