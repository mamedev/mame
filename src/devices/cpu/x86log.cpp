// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    x86log.c

    x86/x64 code logging helpers.

***************************************************************************/

#include <cstdint>
#include <cassert>
#include "emu.h"
#include "x86log.h"
#include "cpu/i386/i386dasm.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void reset_log(x86log_context *log) noexcept;



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
	log = global_alloc_clear<x86log_context>();

	/* allocate the filename */
	log->filename.assign(filename);

	/* reset things */
	reset_log(log);
	return log;
}


/*-------------------------------------------------
    x86log_free_context - release a context
-------------------------------------------------*/

void x86log_free_context(x86log_context *log) noexcept
{
	/* close any open files */
	if (log->file != nullptr)
		fclose(log->file);

	/* free the structure */
	global_free(log);
}


/*-------------------------------------------------
    x86log_mark_as_data - mark a given range as
    data for logging purposes
-------------------------------------------------*/

void x86log_mark_as_data(x86log_context *log, x86code *base, x86code *end, int size) noexcept
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

namespace {
	class x86_buf : public util::disasm_interface::data_buffer {
	public:
		x86_buf(offs_t _base_pc, const u8 *_buf) : base_pc(_base_pc), buf(_buf) {}
		~x86_buf() = default;

		// We know we're on a x86, so we can go short
		virtual u8  r8 (offs_t pc) const override { return *(u8  *)(buf + pc - base_pc); }
		virtual u16 r16(offs_t pc) const override { return *(u16 *)(buf + pc - base_pc); }
		virtual u32 r32(offs_t pc) const override { return *(u32 *)(buf + pc - base_pc); }
		virtual u64 r64(offs_t pc) const override { return *(u64 *)(buf + pc - base_pc); }

	private:
		offs_t base_pc;
		const u8 *buf;
	};

	class x86_config : public i386_disassembler::config {
	public:
		~x86_config() = default;
		virtual int get_mode() const override { return sizeof(void *) * 8; };
	};
}

void x86log_disasm_code_range(x86log_context *log, const char *label, x86code *start, x86code *stop)
{
	const log_comment *lastcomment = &log->comment_list[log->comment_count];
	const log_comment *curcomment = &log->comment_list[0];
	const data_range_t *lastdata = &log->data_range[log->data_range_count];
	const data_range_t *curdata = &log->data_range[0];
	x86code *cur = start;

	/* print the optional label */
	if (label != nullptr)
		x86log_printf(log, "\n%s\n", label);

	/* loop from the start until the cache top */
	while (cur < stop)
	{
		std::string buffer;
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
				case 1:     buffer = string_format("db      %02X", *cur);              break;
				case 2:     buffer = string_format("dw      %04X", *(uint16_t *)cur);    break;
				case 4:     buffer = string_format("dd      %08X", *(uint32_t *)cur);    break;
				case 8:     buffer = string_format("dq      %08X%08X", ((uint32_t *)cur)[1], ((uint32_t *)cur)[0]);    break;
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
			std::stringstream strbuffer;
			offs_t pc = (uintptr_t)cur;
			x86_buf buf(pc, cur);
			x86_config conf;
			i386_disassembler dis(&conf);
			bytes = dis.disassemble(strbuffer, pc, buf, buf) & util::disasm_interface::LENGTHMASK;
			buffer = strbuffer.str();
		}

		/* if we have a matching comment, output it */
		if (curcomment < lastcomment && cur == curcomment->base)
		{
			/* if we have additional matching comments at the same address, output them first */
			for ( ; curcomment + 1 < lastcomment && cur == curcomment[1].base; curcomment++)
				x86log_printf(log, "%p: %-50s; %s\n", cur, "", curcomment->string);
			x86log_printf(log, "%p: %-50s; %s\n", cur, buffer.c_str(), curcomment->string);
		}

		/* if we don't, just print the disassembly and move on */
		else
			x86log_printf(log, "%p: %s\n", cur, buffer.c_str());

		/* advance past this instruction */
		cur += bytes;
	}

	/* reset our state */
	reset_log(log);
}



/***************************************************************************
    LOCAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    reset_log - reset the state of the log
-------------------------------------------------*/

static void reset_log(x86log_context *log) noexcept
{
	log->data_range_count = 0;
	log->comment_count = 0;
	log->comment_pool_next = log->comment_pool;
}
