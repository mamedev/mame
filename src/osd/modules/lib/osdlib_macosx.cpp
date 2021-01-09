// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// MAME headers
#include "osdcore.h"
#include "osdlib.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <memory>

#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <Carbon/Carbon.h>


//============================================================
//  osd_getenv
//============================================================

const char *osd_getenv(const char *name)
{
	return getenv(name);
}

//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
	return setenv(name, value, overwrite);
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill()
{
	kill(getpid(), SIGKILL);
}

//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
	pid_t const mypid = getpid();
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, int(mypid) };
	struct kinfo_proc info;
	info.kp_proc.p_flag = 0;
	std::size_t infosz = sizeof(info);
	sysctl(mib, ARRAY_LENGTH(mib), &info, &infosz, nullptr, 0);
	if (info.kp_proc.p_flag & P_TRACED)
	{
		printf("MAME exception: %s\n", message);
		printf("Attempting to fall into debugger\n");
		kill(mypid, SIGTRAP);
	}
	else
	{
		printf("Ignoring MAME exception: %s\n", message);
	}
}


//============================================================
//  osd_get_clipboard_text
//============================================================

std::string osd_get_clipboard_text(void)
{
	std::string result;
	bool has_result = false;
	OSStatus err;

	PasteboardRef pasteboard_ref;
	err = PasteboardCreate(kPasteboardClipboard, &pasteboard_ref);
	if (err)
		return result;

	PasteboardSynchronize(pasteboard_ref);

	ItemCount item_count;
	err = PasteboardGetItemCount(pasteboard_ref, &item_count);

	for (UInt32 item_index = 1; (item_index <= item_count) && !has_result; item_index++)
	{
		PasteboardItemID item_id;
		err = PasteboardGetItemIdentifier(pasteboard_ref, item_index, &item_id);
		if (err)
			continue;

		CFArrayRef flavor_type_array;
		err = PasteboardCopyItemFlavors(pasteboard_ref, item_id, &flavor_type_array);
		if (err)
			continue;

		CFIndex const flavor_count = CFArrayGetCount(flavor_type_array);
		for (CFIndex flavor_index = 0; (flavor_index < flavor_count) && !has_result; flavor_index++)
		{
			CFStringRef const flavor_type = (CFStringRef)CFArrayGetValueAtIndex(flavor_type_array, flavor_index);

			CFStringEncoding encoding;
			if (UTTypeConformsTo(flavor_type, kUTTypeUTF16PlainText))
				encoding = kCFStringEncodingUTF16;
			else if (UTTypeConformsTo (flavor_type, kUTTypeUTF8PlainText))
				encoding = kCFStringEncodingUTF8;
			else if (UTTypeConformsTo (flavor_type, kUTTypePlainText))
				encoding = kCFStringEncodingMacRoman;
			else
				continue;

			CFDataRef flavor_data;
			err = PasteboardCopyItemFlavorData(pasteboard_ref, item_id, flavor_type, &flavor_data);

			if (!err)
			{
				CFStringRef string_ref = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, flavor_data, encoding);
				CFDataRef data_ref = CFStringCreateExternalRepresentation (kCFAllocatorDefault, string_ref, kCFStringEncodingUTF8, '?');
				CFRelease(string_ref);
				CFRelease(flavor_data);

				CFIndex const length = CFDataGetLength(data_ref);
				CFRange const range = CFRangeMake(0, length);

				result.resize(length);
				CFDataGetBytes(data_ref, range, reinterpret_cast<unsigned char *>(&result[0]));
				has_result = true;

				CFRelease(data_ref);
			}
		}

		CFRelease(flavor_type_array);
	}

	CFRelease(pasteboard_ref);

	return result;
}

//============================================================
//  osd_getpid
//============================================================

int osd_getpid(void)
{
	return getpid();
}


namespace osd {

namespace {

class dynamic_module_posix_impl : public dynamic_module
{
public:
	dynamic_module_posix_impl(std::vector<std::string> &&libraries) : m_libraries(std::move(libraries))
	{
	}

	virtual ~dynamic_module_posix_impl() override
	{
		if (m_module)
			dlclose(m_module);
	}

protected:
	virtual generic_fptr_t get_symbol_address(char const *symbol) override
	{
		/*
		 * given a list of libraries, if a first symbol is successfully loaded from
		 * one of them, all additional symbols will be loaded from the same library
		 */
		if (m_module)
			return reinterpret_cast<generic_fptr_t>(dlsym(m_module, symbol));

		for (auto const &library : m_libraries)
		{
			void *const module = dlopen(library.c_str(), RTLD_LAZY);

			if (module != nullptr)
			{
				generic_fptr_t const function = reinterpret_cast<generic_fptr_t>(dlsym(module, symbol));

				if (function)
				{
					m_module = module;
					return function;
				}
				else
				{
					dlclose(module);
				}
			}
		}

		return nullptr;
	}

private:
	std::vector<std::string> m_libraries;
	void *                   m_module = nullptr;
};

} // anonymous namespace


bool invalidate_instruction_cache(void const *start, std::size_t size)
{
	char const *const begin(reinterpret_cast<char const *>(start));
	char const *const end(begin + size);
	__builtin___clear_cache(const_cast<char *>(begin), const_cast<char *>(end));
	return true;
}


void *virtual_memory_allocation::do_alloc(std::initializer_list<std::size_t> blocks, std::size_t &size, std::size_t &page_size)
{
	long const p(sysconf(_SC_PAGE_SIZE));
	if (0 >= p)
		return nullptr;
	std::size_t s(0);
	for (std::size_t b : blocks)
		s += (b + p - 1) / p;
	s *= p;
	if (!s)
		return nullptr;
	void *const result(mmap(nullptr, s, PROT_NONE, MAP_ANON | MAP_SHARED, -1, 0));
	if (result == (void *)-1)
		return nullptr;
	size = s;
	page_size = p;
	return result;
}

void virtual_memory_allocation::do_free(void *start, std::size_t size)
{
	munmap(start, size);
}

bool virtual_memory_allocation::do_set_access(void *start, std::size_t size, unsigned access)
{
	int prot((NONE == access) ? PROT_NONE : 0);
	if (access & READ)
		prot |= PROT_READ;
	if (access & WRITE)
		prot |= PROT_WRITE;
	if (access & EXECUTE)
		prot |= PROT_EXEC;
	return mprotect(start, size, prot) == 0;
}


dynamic_module::ptr dynamic_module::open(std::vector<std::string> &&names)
{
	return std::make_unique<dynamic_module_posix_impl>(std::move(names));
}

} // namespace osd
