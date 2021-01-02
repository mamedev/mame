// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <csignal>
#include <dlfcn.h>

#include <cstdio>
#include <iomanip>
#include <memory>


#include <mach/mach.h>
#include <mach/mach_time.h>
#include <Carbon/Carbon.h>

// MAME headers
#include "osdcore.h"
#include "osdlib.h"

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
//  osd_alloc_executable
//
//  allocates "size" bytes of executable memory.  this must take
//  things like NX support into account.
//============================================================

void *osd_alloc_executable(size_t size)
{
#if defined(SDLMAME_BSD) || defined(SDLMAME_MACOSX)
	#ifdef __aarch64__
	// $$$$HACK!  This assumes no DRC on Apple Silicon; making that work will be much more involved.
	return (void *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	#else
	return (void *)mmap(0, size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
	#endif
#elif defined(SDLMAME_UNIX)
	return (void *)mmap(0, size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, 0, 0);
#endif
}

//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
#ifdef SDLMAME_SOLARIS
	munmap((char *)ptr, size);
#else
	munmap(ptr, size);
#endif
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

//============================================================
//  dynamic_module_posix_impl
//============================================================

namespace osd {
class dynamic_module_posix_impl : public dynamic_module
{
public:
	dynamic_module_posix_impl(std::vector<std::string> &libraries)
		: m_module(nullptr)
	{
		m_libraries = libraries;
	}

	virtual ~dynamic_module_posix_impl() override
	{
		if (m_module != nullptr)
			dlclose(m_module);
	};

protected:
	virtual generic_fptr_t get_symbol_address(char const *symbol) override
	{
		/*
		 * given a list of libraries, if a first symbol is successfully loaded from
		 * one of them, all additional symbols will be loaded from the same library
		 */
		if (m_module)
		{
			return reinterpret_cast<generic_fptr_t>(dlsym(m_module, symbol));
		}

		for (auto const &library : m_libraries)
		{
			void *module = dlopen(library.c_str(), RTLD_LAZY);

			if (module != nullptr)
			{
				generic_fptr_t function = reinterpret_cast<generic_fptr_t>(dlsym(module, symbol));

				if (function != nullptr)
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
	void *                   m_module;
};

dynamic_module::ptr dynamic_module::open(std::vector<std::string> &&names)
{
	return std::make_unique<dynamic_module_posix_impl>(names);
}

} // namespace osd
