// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//====================================================================
//
//  diagnostics_win32.cpp - Win32 implementation of diagnostic module
//
//====================================================================

#include "diagnostics_module.h"

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <debugger.h>

#include <psapi.h>
#include <dbghelp.h>

#include <memory>
#include <vector>

#include "modules/lib/osdlib.h"

#include <windows/winutil.h>

// Typedefs for dynamically loaded functions
typedef BOOL WINAPI (*StackWalk64_fn)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID, PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64, PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
typedef BOOL WINAPI (*SymInitialize_fn)(HANDLE, LPCTSTR, BOOL);
typedef PVOID WINAPI (*SymFunctionTableAccess64_fn)(HANDLE, DWORD64);
typedef DWORD64 WINAPI (*SymGetModuleBase64_fn)(HANDLE, DWORD64);
typedef BOOL WINAPI (*SymFromAddr_fn)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);
typedef BOOL WINAPI (*SymGetLineFromAddr64_fn)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
typedef PIMAGE_SECTION_HEADER WINAPI (*ImageRvaToSection_fn)(PIMAGE_NT_HEADERS, PVOID, ULONG);
typedef PIMAGE_NT_HEADERS WINAPI (*ImageNtHeader_fn)(PVOID);

class stack_walker
{
public:
	stack_walker();

	FPTR ip() const { return m_stackframe.AddrPC.Offset; }
	FPTR sp() const { return m_stackframe.AddrStack.Offset; }
	FPTR frame() const { return m_stackframe.AddrFrame.Offset; }

	bool reset();
	void reset(CONTEXT &context, HANDLE thread);
	bool unwind();

private:
	HANDLE          m_process;
	HANDLE          m_thread;
	STACKFRAME64    m_stackframe;
	CONTEXT         m_context;
	bool            m_first;

	osd_dynamic_bind<StackWalk64_fn> m_stack_walk_64;
	osd_dynamic_bind<SymInitialize_fn> m_sym_initialize;
	osd_dynamic_bind<SymFunctionTableAccess64_fn> m_sym_function_table_access_64;
	osd_dynamic_bind<SymGetModuleBase64_fn> m_sym_get_module_base_64;

	static bool     s_initialized;
};


class symbol_manager
{
public:
	// construction/destruction
	symbol_manager(const char *argv0);
	~symbol_manager();

	// getters
	FPTR last_base() const { return m_last_base; }

	// core symbol lookup
	const char *symbol_for_address(FPTR address);
	const char *symbol_for_address(PVOID address) { return symbol_for_address(reinterpret_cast<FPTR>(address)); }

	// force symbols to be cached
	void cache_symbols() { scan_file_for_address(0, true); }

	void reset_cache() { m_cache.reset(); }
private:
	// internal helpers
	bool query_system_for_address(FPTR address);
	void scan_file_for_address(FPTR address, bool create_cache);
	bool parse_sym_line(const char *line, FPTR &address, std::string &symbol);
	bool parse_map_line(const char *line, FPTR &address, std::string &symbol);
	void scan_cache_for_address(FPTR address);
	void format_symbol(const char *name, UINT32 displacement, const char *filename = nullptr, int linenumber = 0);

	static FPTR get_text_section_base();

	struct cache_entry
	{
		cache_entry(FPTR address, const char *symbol) :
			m_next(nullptr), m_address(address), m_name(symbol) { }
		cache_entry *next() const { return m_next; }

		cache_entry *   m_next;
		FPTR            m_address;
		std::string     m_name;
	};
	simple_list<cache_entry> m_cache;

	std::string     m_mapfile;
	std::string     m_symfile;
	std::string     m_buffer;
	HANDLE          m_process;
	FPTR            m_last_base;
	FPTR            m_text_base;

	osd_dynamic_bind<SymFromAddr_fn> m_sym_from_addr;
	osd_dynamic_bind<SymGetLineFromAddr64_fn> m_sym_get_line_from_addr_64;
};

class sampling_profiler
{
public:
	sampling_profiler(UINT32 max_seconds, UINT8 stack_depth);
	~sampling_profiler();

	void start();
	void stop();

	//  void reset();
	void print_results(symbol_manager &symbols);

private:
	static DWORD WINAPI thread_entry(LPVOID lpParameter);
	void thread_run();

	static int CLIB_DECL compare_address(const void *item1, const void *item2);
	static int CLIB_DECL compare_frequency(const void *item1, const void *item2);

	HANDLE          m_target_thread;

	HANDLE          m_thread;
	DWORD           m_thread_id;
	volatile bool   m_thread_exit;

	UINT8           m_stack_depth;
	UINT8           m_entry_stride;
	std::vector<FPTR>    m_buffer;
	FPTR *          m_buffer_ptr;
	FPTR *          m_buffer_end;
};


//**************************************************************************
//  STACK WALKER
//**************************************************************************

bool stack_walker::s_initialized = false;

//-------------------------------------------------
//  stack_walker - constructor
//-------------------------------------------------

stack_walker::stack_walker()
	: m_process(GetCurrentProcess()),
	m_thread(GetCurrentThread()),
	m_first(true),
	m_stack_walk_64               ("StackWalk64",              { L"dbghelp.dll" }),
	m_sym_initialize              ("SymInitialize",            { L"dbghelp.dll" }),
	m_sym_function_table_access_64("SymFunctionTableAccess64", { L"dbghelp.dll" }),
	m_sym_get_module_base_64      ("SymGetModuleBase64",       { L"dbghelp.dll" })
{
	// zap the structs
	memset(&m_stackframe, 0, sizeof(m_stackframe));
	memset(&m_context, 0, sizeof(m_context));

	// initialize the symbols
	if (!s_initialized && m_sym_initialize && m_stack_walk_64 && m_sym_function_table_access_64 && m_sym_get_module_base_64)
	{
		(*m_sym_initialize)(m_process, nullptr, TRUE);
		s_initialized = true;
	}
}


//-------------------------------------------------
//  reset - set up a new context
//-------------------------------------------------

bool stack_walker::reset()
{
	// set up the initial state
	RtlCaptureContext(&m_context);
	m_thread = GetCurrentThread();
	m_first = true;

	// initialize the stackframe
	memset(&m_stackframe, 0, sizeof(m_stackframe));
	m_stackframe.AddrPC.Mode = AddrModeFlat;
	m_stackframe.AddrFrame.Mode = AddrModeFlat;
	m_stackframe.AddrStack.Mode = AddrModeFlat;

	// pull architecture-specific fields from the context
#ifdef PTR64
	m_stackframe.AddrPC.Offset = m_context.Rip;
	m_stackframe.AddrFrame.Offset = m_context.Rsp;
	m_stackframe.AddrStack.Offset = m_context.Rsp;
#else
	m_stackframe.AddrPC.Offset = m_context.Eip;
	m_stackframe.AddrFrame.Offset = m_context.Ebp;
	m_stackframe.AddrStack.Offset = m_context.Esp;
#endif
	return true;
}

void stack_walker::reset(CONTEXT &initial, HANDLE thread)
{
	// set up the initial state
	m_context = initial;
	m_thread = thread;
	m_first = true;

	// initialize the stackframe
	memset(&m_stackframe, 0, sizeof(m_stackframe));
	m_stackframe.AddrPC.Mode = AddrModeFlat;
	m_stackframe.AddrFrame.Mode = AddrModeFlat;
	m_stackframe.AddrStack.Mode = AddrModeFlat;

	// pull architecture-specific fields from the context
#ifdef PTR64
	m_stackframe.AddrPC.Offset = m_context.Rip;
	m_stackframe.AddrFrame.Offset = m_context.Rsp;
	m_stackframe.AddrStack.Offset = m_context.Rsp;
#else
	m_stackframe.AddrPC.Offset = m_context.Eip;
	m_stackframe.AddrFrame.Offset = m_context.Ebp;
	m_stackframe.AddrStack.Offset = m_context.Esp;
#endif
}


//-------------------------------------------------
//  unwind - unwind a single level
//-------------------------------------------------

bool stack_walker::unwind()
{
	// if we were able to initialize, then we have everything we need
	if (s_initialized)
	{
#ifdef PTR64
		return (*m_stack_walk_64)(IMAGE_FILE_MACHINE_AMD64, m_process, m_thread, &m_stackframe, &m_context, nullptr, *m_sym_function_table_access_64, *m_sym_get_module_base_64, nullptr);
#else
		return (*m_stack_walk_64)(IMAGE_FILE_MACHINE_I386, m_process, m_thread, &m_stackframe, &m_context, nullptr, *m_sym_function_table_access_64, *m_sym_get_module_base_64, nullptr);
#endif
	}

	// otherwise, fake the first unwind, which will just return info from the context
	else
	{
		bool result = m_first;
		m_first = false;
		return result;
	}
}



//**************************************************************************
//  SYMBOL MANAGER
//**************************************************************************

//-------------------------------------------------
//  symbol_manager - constructor
//-------------------------------------------------

symbol_manager::symbol_manager(const char *argv0)
	: m_mapfile(argv0),
	m_symfile(argv0),
	m_process(GetCurrentProcess()),
	m_last_base(0),
	m_text_base(0),
	m_sym_from_addr            ("SymFromAddr",          { L"dbghelp.dll" }),
	m_sym_get_line_from_addr_64("SymGetLineFromAddr64", { L"dbghelp.dll" })
{
#ifdef __GNUC__
	// compute the name of the mapfile
	int extoffs = m_mapfile.find_last_of('.');
	if (extoffs != -1)
		m_mapfile.substr(0, extoffs);
	m_mapfile.append(".map");

	// and the name of the symfile
	extoffs = m_symfile.find_last_of('.');
	if (extoffs != -1)
		m_symfile = m_symfile.substr(0, extoffs);
	m_symfile.append(".sym");

	// figure out the base of the .text section
	m_text_base = get_text_section_base();
#endif

	// expand the buffer to be decently large up front
	m_buffer = string_format("%500s", "");
}


//-------------------------------------------------
//  ~symbol_manager - destructor
//-------------------------------------------------

symbol_manager::~symbol_manager()
{
}


//-------------------------------------------------
//  symbol_for_address - return a symbol by looking
//  it up either in the cache or by scanning the
//  file
//-------------------------------------------------

const char *symbol_manager::symbol_for_address(FPTR address)
{
	// default the buffer
	m_buffer.assign(" (not found)");
	m_last_base = 0;

	// first try to do it using system APIs
	if (!query_system_for_address(address))
	{
		// if that fails, scan the cache if we have one
		if (!m_cache.empty())
			scan_cache_for_address(address);

		// or else try to open a sym/map file and find it there
		else
			scan_file_for_address(address, false);
	}
	return m_buffer.c_str();
}


//-------------------------------------------------
//  query_system_for_address - ask the system to
//  look up our address
//-------------------------------------------------

bool symbol_manager::query_system_for_address(FPTR address)
{
	// need at least the sym_from_addr API
	if (!m_sym_from_addr)
		return false;

	BYTE info_buffer[sizeof(SYMBOL_INFO) + 256] = { 0 };
	SYMBOL_INFO &info = *reinterpret_cast<SYMBOL_INFO *>(&info_buffer[0]);
	DWORD64 displacement;

	// even through the struct says TCHAR, we actually get back an ANSI string here
	info.SizeOfStruct = sizeof(info);
	info.MaxNameLen = sizeof(info_buffer) - sizeof(info);
	if ((*m_sym_from_addr)(m_process, address, &displacement, &info))
	{
		// try to get source info as well; again we are returned an ANSI string
		IMAGEHLP_LINE64 lineinfo = { sizeof(lineinfo) };
		DWORD linedisp;
		if (m_sym_get_line_from_addr_64 && (*m_sym_get_line_from_addr_64)(m_process, address, &linedisp, &lineinfo))
			format_symbol(info.Name, displacement, lineinfo.FileName, lineinfo.LineNumber);
		else
			format_symbol(info.Name, displacement);

		// set the last base
		m_last_base = address - displacement;
		return true;
	}
	return false;
}


//-------------------------------------------------
//  scan_file_for_address - walk either the map
//  or symbol files and find the best match for
//  the given address, optionally creating a cache
//  along the way
//-------------------------------------------------

void symbol_manager::scan_file_for_address(FPTR address, bool create_cache)
{
	bool is_symfile = false;
	FILE *srcfile = nullptr;

#ifdef __GNUC__
	// see if we have a symbol file (gcc only)
	srcfile = fopen(m_symfile.c_str(), "r");
	is_symfile = (srcfile != nullptr);
#endif

	// if not, see if we have a map file
	if (srcfile == nullptr)
		srcfile = fopen(m_mapfile.c_str(), "r");

	// if not, fail
	if (srcfile == nullptr)
		return;

	// reset the best info
	std::string best_symbol;
	FPTR best_addr = 0;

	// parse the file, looking for valid entries
	std::string symbol;
	char line[1024];
	while (fgets(line, sizeof(line) - 1, srcfile))
	{
		// parse the line looking for an interesting symbol
		FPTR addr = 0;
		bool valid = is_symfile ? parse_sym_line(line, addr, symbol) : parse_map_line(line, addr, symbol);

		// if we got one, see if this is the best
		if (valid)
		{
			// if this is the best one so far, remember it
			if (addr <= address && addr > best_addr)
			{
				best_addr = addr;
				best_symbol = symbol;
			}

			// also create a cache entry if we can
			if (create_cache)
				m_cache.append(*global_alloc(cache_entry(addr, symbol.c_str())));
		}
	}

	// close the file
	fclose(srcfile);

	// format the symbol and remember the last base
	format_symbol(best_symbol.c_str(), address - best_addr);
	m_last_base = best_addr;
}


//-------------------------------------------------
//  scan_cache_for_address - walk the cache to
//  find the best match for the given address
//-------------------------------------------------

void symbol_manager::scan_cache_for_address(FPTR address)
{
	// reset the best info
	std::string best_symbol;
	FPTR best_addr = 0;

	// walk the cache, looking for valid entries
	for (cache_entry &entry : m_cache)

		// if this is the best one so far, remember it
		if (entry.m_address <= address && entry.m_address > best_addr)
		{
			best_addr = entry.m_address;
			best_symbol = entry.m_name;
		}

	// format the symbol and remember the last base
	format_symbol(best_symbol.c_str(), address - best_addr);
	m_last_base = best_addr;
}


//-------------------------------------------------
//  parse_sym_line - parse a line from a sym file
//  which is just the output of objdump
//-------------------------------------------------

bool symbol_manager::parse_sym_line(const char *line, FPTR &address, std::string &symbol)
{
#ifdef __GNUC__
	/*
	32-bit gcc symbol line:
	[271778](sec  1)(fl 0x00)(ty  20)(scl   3) (nx 0) 0x007df675 line_to_symbol(char const*, unsigned int&, bool)

	64-bit gcc symbol line:
	[271775](sec  1)(fl 0x00)(ty  20)(scl   3) (nx 0) 0x00000000008dd1e9 line_to_symbol(char const*, unsigned long long&, bool)
	*/

	// first look for a (ty) entry
	const char *type = strstr(line, "(ty  20)");
	if (type == nullptr)
		return false;

	// scan forward in the line to find the address
	bool in_parens = false;
	for (const char *chptr = type; *chptr != 0; chptr++)
	{
		// track open/close parentheses
		if (*chptr == '(')
			in_parens = true;
		else if (*chptr == ')')
			in_parens = false;

		// otherwise, look for an 0x address
		else if (!in_parens && *chptr == '0' && chptr[1] == 'x')
		{
			// make sure we can get an address
			void *temp;
			if (sscanf(chptr, "0x%p", &temp) != 1)
				return false;
			address = m_text_base + reinterpret_cast<FPTR>(temp);

			// skip forward until we're past the space
			while (*chptr != 0 && !isspace(*chptr))
				chptr++;

			// extract the symbol name
			strtrimspace(symbol.assign(chptr));
			return (symbol.length() > 0);
		}
	}
#endif
	return false;
}


//-------------------------------------------------
//  parse_map_line - parse a line from a linker-
//  generated map file
//-------------------------------------------------

bool symbol_manager::parse_map_line(const char *line, FPTR &address, std::string &symbol)
{
#ifdef __GNUC__
	/*
	32-bit gcc map line:
	0x0089cb00                nbmj9195_palette_r(_address_space const*, unsigned int)

	64-bit gcc map line:
	0x0000000000961afc                nbmj9195_palette_r(_address_space const*, unsigned int)
	*/

	// find a matching start
	if (strncmp(line, "                0x", 18) == 0)
	{
		// make sure we can get an address
		void *temp;
		if (sscanf(&line[16], "0x%p", &temp) != 1)
			return false;
		address = reinterpret_cast<FPTR>(temp);

		// skip forward until we're past the space
		const char *chptr = &line[16];
		while (*chptr != 0 && !isspace(*chptr))
			chptr++;

		// extract the symbol name
		strtrimspace(symbol.assign(chptr));
		return (symbol.length() > 0);
	}
#endif
	return false;
}


//-------------------------------------------------
//  format_symbol - common symbol formatting
//-------------------------------------------------

void symbol_manager::format_symbol(const char *name, UINT32 displacement, const char *filename, int linenumber)
{
	// start with the address and offset
	m_buffer = string_format(" (%s", name);
	if (displacement != 0)
		m_buffer.append(string_format("+0x%04x", (UINT32)displacement));

	// append file/line if present
	if (filename != nullptr)
		m_buffer.append(string_format(", %s:%d", filename, linenumber));

	// close up the string
	m_buffer.append(")");
}


//-------------------------------------------------
//  get_text_section_base - figure out the base
//  of the .text section
//-------------------------------------------------

FPTR symbol_manager::get_text_section_base()
{
	osd_dynamic_bind<ImageRvaToSection_fn> image_rva_to_section("ImageRvaToSection", { L"dbghelp.dll" });
	osd_dynamic_bind<ImageNtHeader_fn> image_nt_header         ("ImageNtHeader",     { L"dbghelp.dll" });

	// start with the image base
	PVOID base = reinterpret_cast<PVOID>(GetModuleHandleUni());
	assert(base != nullptr);

	// make sure we have the functions we need
	if (image_nt_header && image_rva_to_section)
	{
		// get the NT header
		PIMAGE_NT_HEADERS headers = (*image_nt_header)(base);
		assert(headers != nullptr);

		// look ourself up (assuming we are in the .text section)
		PIMAGE_SECTION_HEADER section = (*image_rva_to_section)(headers, base, reinterpret_cast<FPTR>(get_text_section_base) - reinterpret_cast<FPTR>(base));
		if (section != nullptr)
			return reinterpret_cast<FPTR>(base) + section->VirtualAddress;
	}

	// fallback to returning the image base (wrong)
	return reinterpret_cast<FPTR>(base);
}



//**************************************************************************
//  SAMPLING PROFILER
//**************************************************************************

//-------------------------------------------------
//  sampling_profiler - constructor
//-------------------------------------------------

sampling_profiler::sampling_profiler(UINT32 max_seconds, UINT8 stack_depth = 0)
	: m_target_thread(nullptr),
	m_thread(nullptr),
	m_thread_id(0),
	m_thread_exit(false),
	m_stack_depth(stack_depth),
	m_entry_stride(stack_depth + 2),
	m_buffer(max_seconds * 1000 * m_entry_stride),
	m_buffer_ptr(&m_buffer[0]),
	m_buffer_end(&m_buffer[0] + max_seconds * 1000 * m_entry_stride)
{
}


//-------------------------------------------------
//  sampling_profiler - destructor
//-------------------------------------------------

sampling_profiler::~sampling_profiler()
{
}


//-------------------------------------------------
//  start - begin gathering profiling information
//-------------------------------------------------

void sampling_profiler::start()
{
	// do the dance to get a handle to ourself
	BOOL result = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &m_target_thread,
		THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, 0);
	assert_always(result, "Failed to get thread handle for main thread");

	// reset the exit flag
	m_thread_exit = false;

	// start the thread
	m_thread = CreateThread(nullptr, 0, thread_entry, (LPVOID)this, 0, &m_thread_id);
	assert_always(m_thread != nullptr, "Failed to create profiler thread\n");

	// max out the priority
	SetThreadPriority(m_thread, THREAD_PRIORITY_TIME_CRITICAL);
}


//-------------------------------------------------
//  stop - stop gathering profiling information
//-------------------------------------------------

void sampling_profiler::stop()
{
	// set the flag and wait a couple of seconds (max)
	m_thread_exit = true;
	WaitForSingleObject(m_thread, 2000);

	// regardless, close the handle
	CloseHandle(m_thread);
}


//-------------------------------------------------
//  compare_address - compare two entries by their
//  bucket address
//-------------------------------------------------

int CLIB_DECL sampling_profiler::compare_address(const void *item1, const void *item2)
{
	const FPTR *entry1 = reinterpret_cast<const FPTR *>(item1);
	const FPTR *entry2 = reinterpret_cast<const FPTR *>(item2);
	int mincount = MIN(entry1[0], entry2[0]);

	// sort in order of: bucket, caller, caller's caller, etc.
	for (int index = 1; index <= mincount; index++)
		if (entry1[index] != entry2[index])
			return entry1[index] - entry2[index];

	// if we match to the end, sort by the depth of the stack
	return entry1[0] - entry2[0];
}


//-------------------------------------------------
//  compare_frequency - compare two entries by
//  their frequency of occurrence
//-------------------------------------------------

int CLIB_DECL sampling_profiler::compare_frequency(const void *item1, const void *item2)
{
	const FPTR *entry1 = reinterpret_cast<const FPTR *>(item1);
	const FPTR *entry2 = reinterpret_cast<const FPTR *>(item2);

	// sort by frequency, then by address
	if (entry1[0] != entry2[0])
		return entry2[0] - entry1[0];
	return entry1[1] - entry2[1];
}


//-------------------------------------------------
//  print_results - output the results
//-------------------------------------------------

void sampling_profiler::print_results(symbol_manager &symbols)
{
	// cache the symbols
	symbols.cache_symbols();

	// step 1: find the base of each entry
	for (FPTR *current = &m_buffer[0]; current < m_buffer_ptr; current += m_entry_stride)
	{
		assert(current[0] >= 1 && current[0] < m_entry_stride);

		// convert the sampled PC to its function base as a bucket
		symbols.symbol_for_address(current[1]);
		current[1] = symbols.last_base();
	}

	// step 2: sort the results
	qsort(&m_buffer[0], (m_buffer_ptr - &m_buffer[0]) / m_entry_stride, m_entry_stride * sizeof(FPTR), compare_address);

	// step 3: count and collapse unique entries
	UINT32 total_count = 0;
	for (FPTR *current = &m_buffer[0]; current < m_buffer_ptr; )
	{
		int count = 1;
		FPTR *scan;
		for (scan = current + m_entry_stride; scan < m_buffer_ptr; scan += m_entry_stride)
		{
			if (compare_address(current, scan) != 0)
				break;
			scan[0] = 0;
			count++;
		}
		current[0] = count;
		total_count += count;
		current = scan;
	}

	// step 4: sort the results again, this time by frequency
	qsort(&m_buffer[0], (m_buffer_ptr - &m_buffer[0]) / m_entry_stride, m_entry_stride * sizeof(FPTR), compare_frequency);

	// step 5: print the results
	UINT32 num_printed = 0;
	for (FPTR *current = &m_buffer[0]; current < m_buffer_ptr && num_printed < 30; current += m_entry_stride)
	{
		// once we hit 0 frequency, we're done
		if (current[0] == 0)
			break;

		// output the result
		printf("%4.1f%% - %6d : %p%s\n", (double)current[0] * 100.0 / (double)total_count, (UINT32)current[0], reinterpret_cast<void *>(current[1]), symbols.symbol_for_address(current[1]));
		for (int index = 2; index < m_entry_stride; index++)
		{
			if (current[index] == 0)
				break;
			printf("                 %p%s\n", reinterpret_cast<void *>(current[index]), symbols.symbol_for_address(current[index]));
		}
		printf("\n");
		num_printed++;
	}
	symbols.reset_cache();
}


//-------------------------------------------------
//  thread_entry - thread entry stub
//-------------------------------------------------

DWORD WINAPI sampling_profiler::thread_entry(LPVOID lpParameter)
{
	reinterpret_cast<sampling_profiler *>(lpParameter)->thread_run();
	return 0;
}


//-------------------------------------------------
//  thread_run - sampling thread
//-------------------------------------------------

void sampling_profiler::thread_run()
{
	CONTEXT context;
	memset(&context, 0, sizeof(context));

	// loop until done
	stack_walker walker;
	while (!m_thread_exit && m_buffer_ptr < m_buffer_end)
	{
		// pause the main thread and get its context
		SuspendThread(m_target_thread);
		context.ContextFlags = CONTEXT_FULL;
		GetThreadContext(m_target_thread, &context);

		// first entry is a count
		FPTR *count = m_buffer_ptr++;
		*count = 0;

		// iterate over the frames until we run out or hit an error
		walker.reset(context, m_target_thread);
		int frame;
		for (frame = 0; frame <= m_stack_depth && walker.unwind(); frame++)
		{
			*m_buffer_ptr++ = walker.ip();
			*count += 1;
		}

		// fill in any missing parts with nulls
		for (; frame <= m_stack_depth; frame++)
			*m_buffer_ptr++ = 0;

		// resume the thread
		ResumeThread(m_target_thread);

		// sleep for 1ms
		Sleep(1);
	}
}

/*-----------------------------------------------------------------------------
    Diagnostics module for Win32
-----------------------------------------------------------------------------*/

class diagnostics_win32 : public diagnostics_module
{
	friend diagnostics_module* diagnostics_module::get_instance();

private:
	std::unique_ptr<symbol_manager>    m_symbols;
	std::unique_ptr<sampling_profiler> m_sampling_profiler;
	LPTOP_LEVEL_EXCEPTION_FILTER m_pass_thru_filter;

	diagnostics_win32(): m_pass_thru_filter(nullptr)
	{
	}

public:
	int init_crash_diagnostics() override
	{
		ensure_symbols();

		// set up exception handling
		m_pass_thru_filter = SetUnhandledExceptionFilter(diagnostics_win32::exception_filter);
		SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

		return 0;
	}

	void start_profiler(std::uint32_t max_seconds, std::uint8_t stack_depth) override
	{
		m_sampling_profiler = std::make_unique<sampling_profiler>(max_seconds, stack_depth);
		m_sampling_profiler->start();
	}

	void stop_profiler() override
	{
		if (m_sampling_profiler != nullptr)
		{
			m_sampling_profiler->stop();
		}
	}

	void print_profiler_results() override
	{
		if (m_sampling_profiler != nullptr)
		{
			ensure_symbols();
			m_sampling_profiler->print_results(*m_symbols.get());
		}
	}

private:
	//============================================================
	//  exception_filter
	//============================================================

	static long __stdcall exception_filter(struct _EXCEPTION_POINTERS *info)
	{
		static const struct
		{
			DWORD code;
			const char *string;
		} exception_table[] =
		{
			{ EXCEPTION_ACCESS_VIOLATION,       "ACCESS VIOLATION" },
			{ EXCEPTION_DATATYPE_MISALIGNMENT,  "DATATYPE MISALIGNMENT" },
			{ EXCEPTION_BREAKPOINT,             "BREAKPOINT" },
			{ EXCEPTION_SINGLE_STEP,            "SINGLE STEP" },
			{ EXCEPTION_ARRAY_BOUNDS_EXCEEDED,  "ARRAY BOUNDS EXCEEDED" },
			{ EXCEPTION_FLT_DENORMAL_OPERAND,   "FLOAT DENORMAL OPERAND" },
			{ EXCEPTION_FLT_DIVIDE_BY_ZERO,     "FLOAT DIVIDE BY ZERO" },
			{ EXCEPTION_FLT_INEXACT_RESULT,     "FLOAT INEXACT RESULT" },
			{ EXCEPTION_FLT_INVALID_OPERATION,  "FLOAT INVALID OPERATION" },
			{ EXCEPTION_FLT_OVERFLOW,           "FLOAT OVERFLOW" },
			{ EXCEPTION_FLT_STACK_CHECK,        "FLOAT STACK CHECK" },
			{ EXCEPTION_FLT_UNDERFLOW,          "FLOAT UNDERFLOW" },
			{ EXCEPTION_INT_DIVIDE_BY_ZERO,     "INTEGER DIVIDE BY ZERO" },
			{ EXCEPTION_INT_OVERFLOW,           "INTEGER OVERFLOW" },
			{ EXCEPTION_PRIV_INSTRUCTION,       "PRIVILEGED INSTRUCTION" },
			{ EXCEPTION_IN_PAGE_ERROR,          "IN PAGE ERROR" },
			{ EXCEPTION_ILLEGAL_INSTRUCTION,    "ILLEGAL INSTRUCTION" },
			{ EXCEPTION_NONCONTINUABLE_EXCEPTION,"NONCONTINUABLE EXCEPTION" },
			{ EXCEPTION_STACK_OVERFLOW,         "STACK OVERFLOW" },
			{ EXCEPTION_INVALID_DISPOSITION,    "INVALID DISPOSITION" },
			{ EXCEPTION_GUARD_PAGE,             "GUARD PAGE VIOLATION" },
			{ EXCEPTION_INVALID_HANDLE,         "INVALID HANDLE" },
			{ 0,                                "UNKNOWN EXCEPTION" }
		};
		static int already_hit = 0;
		int i;

		// if we're hitting this recursively, just exit
		if (already_hit)
			return EXCEPTION_CONTINUE_SEARCH;
		already_hit = 1;

		// flush any debugging traces that were live
		debugger_flush_all_traces_on_abnormal_exit();

		// find our man
		for (i = 0; exception_table[i].code != 0; i++)
			if (info->ExceptionRecord->ExceptionCode == exception_table[i].code)
				break;

		// print the exception type and address
		fprintf(stderr, "\n-----------------------------------------------------\n");

		auto diagnostics = downcast<diagnostics_win32 *>(get_instance());

		fprintf(stderr, "Exception at EIP=%p%s: %s\n", info->ExceptionRecord->ExceptionAddress,
			diagnostics->m_symbols->symbol_for_address((FPTR)info->ExceptionRecord->ExceptionAddress), exception_table[i].string);

		// for access violations, print more info
		if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
			fprintf(stderr, "While attempting to %s memory at %p\n",
				info->ExceptionRecord->ExceptionInformation[0] ? "write" : "read",
				(void *)info->ExceptionRecord->ExceptionInformation[1]);

		// print the state of the CPU
		fprintf(stderr, "-----------------------------------------------------\n");
#ifdef PTR64
		fprintf(stderr, "RAX=%p RBX=%p RCX=%p RDX=%p\n",
			(void *)info->ContextRecord->Rax,
			(void *)info->ContextRecord->Rbx,
			(void *)info->ContextRecord->Rcx,
			(void *)info->ContextRecord->Rdx);
		fprintf(stderr, "RSI=%p RDI=%p RBP=%p RSP=%p\n",
			(void *)info->ContextRecord->Rsi,
			(void *)info->ContextRecord->Rdi,
			(void *)info->ContextRecord->Rbp,
			(void *)info->ContextRecord->Rsp);
		fprintf(stderr, " R8=%p  R9=%p R10=%p R11=%p\n",
			(void *)info->ContextRecord->R8,
			(void *)info->ContextRecord->R9,
			(void *)info->ContextRecord->R10,
			(void *)info->ContextRecord->R11);
		fprintf(stderr, "R12=%p R13=%p R14=%p R15=%p\n",
			(void *)info->ContextRecord->R12,
			(void *)info->ContextRecord->R13,
			(void *)info->ContextRecord->R14,
			(void *)info->ContextRecord->R15);
#else
		fprintf(stderr, "EAX=%p EBX=%p ECX=%p EDX=%p\n",
			(void *)info->ContextRecord->Eax,
			(void *)info->ContextRecord->Ebx,
			(void *)info->ContextRecord->Ecx,
			(void *)info->ContextRecord->Edx);
		fprintf(stderr, "ESI=%p EDI=%p EBP=%p ESP=%p\n",
			(void *)info->ContextRecord->Esi,
			(void *)info->ContextRecord->Edi,
			(void *)info->ContextRecord->Ebp,
			(void *)info->ContextRecord->Esp);
#endif

		// reprint the actual exception address
		fprintf(stderr, "-----------------------------------------------------\n");
		fprintf(stderr, "Stack crawl:\n");
		diagnostics->print_stacktrace(info->ContextRecord, GetCurrentThread());

		// flush stderr, so the data is actually written when output is being redirected
		fflush(stderr);

		// exit
		return EXCEPTION_CONTINUE_SEARCH;
	}

	void print_stacktrace(void* context, void* thread)
	{
		ensure_symbols();

		// set up the stack walker
		stack_walker walker;

		if (context != nullptr && thread != nullptr)
		{
			walker.reset(*static_cast<CONTEXT*>(context), thread);
		}
		else
		{
			if (!walker.reset())
				return;
		}

		// walk the stack
		while (walker.unwind())
			fprintf(
				stderr,
				"  %p: %p%s\n",
				reinterpret_cast<void *>(walker.frame()),
				reinterpret_cast<void *>(walker.ip()),
				m_symbols == nullptr ? "" : m_symbols->symbol_for_address(walker.ip()));
	}

	void ensure_symbols()
	{
		if (m_symbols == nullptr)
		{
			char exe_path[MAX_PATH];
			size_t len = GetModuleFileNameA(nullptr, exe_path, sizeof(exe_path));
			if (len == 0)
			{
				osd_printf_error("Failed to get the executable path.\n");
				fatalerror("Failed to get the executable path.");
			}

			exe_path[len] = '\0';
			m_symbols = std::make_unique<symbol_manager>(exe_path);
		}

		if (m_symbols == nullptr)
		{
			fatalerror("Could not initialize symbols.");
		}
	}
};

// Static accessor for the win32 diagnostic module
diagnostics_module * diagnostics_module::get_instance()
{
	static diagnostics_win32 s_instance;
	return &s_instance;
}

#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#endif
