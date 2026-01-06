// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    x86log.c

    x86/x64 code logging helpers.

***************************************************************************/

#include "emu.h"
#include "x86log.h"

#include "cpu/i386/i386dasm.h"

#include <sstream>


namespace {

class x86_buf : public util::disasm_interface::data_buffer
{
public:
	x86_buf(offs_t pc, const u8 *b) : base_pc(pc), buf(b) { }
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

class x86_config : public i386_disassembler::config
{
public:
	~x86_config() = default;
	virtual int get_mode() const override { return sizeof(void *) * 8; };
};

} // anonymous namespace



/***************************************************************************
    EXTERNAL INTERFACES
***************************************************************************/

/*-------------------------------------------------
    x86log_context::create- create a new context
-------------------------------------------------*/

x86log_context::ptr x86log_context::create(std::string_view filename)
{
	try
	{
		// allocate the log
		auto log = std::make_unique<x86log_context>();
		log->data_range.reserve(MAX_DATA_RANGES);
		log->comment_list.reserve(MAX_COMMENTS);

		// allocate the filename
		log->filename = filename;

		// reset things
		log->reset_log();
		return log;
	}
	catch (std::bad_alloc const &)
	{
		return nullptr;
	}
}


/*-------------------------------------------------
    ~x86log_context - release a context
-------------------------------------------------*/

x86log_context::~x86log_context()
{
	// close any open files
	if (file)
		std::fclose(file);
}


/*-------------------------------------------------
    x86log_context::mark_as_data - mark a given
    range as data for logging purposes
-------------------------------------------------*/

void x86log_context::mark_as_data(x86code const *base, x86code const *end, int size) noexcept
{
	assert(end >= base);
	assert((size == 1) || (size == 2) || (size == 4) || (size == 8));

	// we assume data ranges are registered in order; enforce this
	assert(data_range.empty() || (base > data_range.back().end));

	// fill in the new range
	try
	{
		data_range.emplace_back(data_range_t{ base, end, size });
	}
	catch (std::bad_alloc const &)
	{
	}
}


/*-------------------------------------------------
    x86log_context::disasm_code_range - disassemble
    a range of code and reset accumulated
    information
-------------------------------------------------*/

void x86log_context::disasm_code_range(const char *label, x86code const *start, x86code const *stop)
{
	auto const lastcomment = comment_list.cend();
	auto curcomment = comment_list.cbegin();
	auto const lastdata = data_range.cend();
	auto curdata = data_range.cbegin();
	x86code const *cur = start;

	// print the optional label
	if (label)
		printf("\n%s\n", label);

	// loop from the start until the cache top
	x86_config conf;
	i386_disassembler dis(&conf);
	std::stringstream strbuffer;
	while (cur < stop)
	{
		strbuffer.str("");
		int bytes;

		// skip past any past data ranges
		while ((curdata != lastdata) && (cur > curdata->end))
			++curdata;

		// skip past any past comments
		while ((curcomment != lastcomment) && (cur > curcomment->base))
			++curcomment;

		if ((cur >= curdata->base) && (cur <= curdata->end))
		{
			// if we're in a data range, output the next chunk and continue
			bytes = curdata->size;
			switch (curdata->size)
			{
				default:
				case 1: util::stream_format(strbuffer, "db      %02X", *cur); break;
				case 2: util::stream_format(strbuffer, "dw      %04X", *reinterpret_cast<uint16_t const *>(cur)); break;
				case 4: util::stream_format(strbuffer, "dd      %08X", *reinterpret_cast<uint32_t const *>(cur)); break;
				case 8: util::stream_format(strbuffer, "dq      %016X", *reinterpret_cast<uint64_t const *>(cur)); break;
			}
		}
		else if (*cur == 0xcc)
		{
			// if we're not in the data range, skip filler opcodes
			cur++;
			continue;
		}
		else
		{
			// otherwise, do a disassembly of the current instruction
			offs_t pc = uintptr_t(cur);
			x86_buf buf(pc, cur);
			bytes = dis.disassemble(strbuffer, pc, buf, buf) & util::disasm_interface::LENGTHMASK;
		}

		if ((curcomment != lastcomment) && (cur == curcomment->base))
		{
			// if we have a matching comment, output it
			// if we have additional matching comments at the same address, output them first
			for ( ; ((curcomment + 1) != lastcomment) && (cur == curcomment[1].base); curcomment++)
				printf("%p: %-50s; %s\n", cur, "", curcomment->string);
			printf("%p: %-50s; %s\n", cur, std::move(strbuffer).str().c_str(), curcomment->string);
		}
		else
		{
			// if we don't, just print the disassembly and move on
			printf("%p: %s\n", cur, std::move(strbuffer).str().c_str());
		}

		// advance past this instruction
		cur += bytes;
	}

	// reset our state
	reset_log();
}



/*-------------------------------------------------
    reset_log - reset the state of the log
-------------------------------------------------*/

void x86log_context::reset_log() noexcept
{
	data_range.clear();
	comment_list.clear();
	comment_pool_next = comment_pool;
}
