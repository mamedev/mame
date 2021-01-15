// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  osdlib.h
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  - Common low level routines
//  - Source files also provide the following from osdcore.h
//
//    - osd_ticks
//    - osd_sleep
//============================================================

#ifndef __OSDLIB__
#define __OSDLIB__

#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>
#include <memory>

/*-----------------------------------------------------------------------------
    osd_process_kill: kill the current process

    Parameters:

        None.

    Return value:

        None.
-----------------------------------------------------------------------------*/

void osd_process_kill(void);


/*-----------------------------------------------------------------------------
    osd_setenv: set environment variable

    Parameters:

        name  - name of environment variable
        value - value to write
        overwrite - overwrite if it exists

    Return value:

        0 on success
-----------------------------------------------------------------------------*/

int osd_setenv(const char *name, const char *value, int overwrite);


/*-----------------------------------------------------------------------------
    osd_get_clipboard_text: retrieves text from the clipboard
-----------------------------------------------------------------------------*/
std::string osd_get_clipboard_text(void);

namespace osd {

bool invalidate_instruction_cache(void const *start, std::size_t size);


class virtual_memory_allocation
{
public:
	enum : unsigned
	{
		NONE = 0x00,
		READ = 0x01,
		WRITE = 0x02,
		EXECUTE = 0x04,
		READ_WRITE = READ | WRITE,
		READ_EXECUTE = READ | EXECUTE
	};

	virtual_memory_allocation(virtual_memory_allocation const &) = delete;
	virtual_memory_allocation &operator=(virtual_memory_allocation const &) = delete;

	virtual_memory_allocation() { }
	virtual_memory_allocation(std::initializer_list<std::size_t> blocks)
	{
		m_memory = do_alloc(blocks, m_size, m_page_size);
	}
	virtual_memory_allocation(virtual_memory_allocation &&that) : m_memory(that.m_memory), m_size(that.m_size), m_page_size(that.m_page_size)
	{
		that.m_memory = nullptr;
		that.m_size = that.m_page_size = 0U;
	}
	~virtual_memory_allocation()
	{
		if (m_memory)
			do_free(m_memory, m_size);
	}

	explicit operator bool() const { return bool(m_memory); }
	void *get() { return m_memory; }
	std::size_t size() const { return m_size; }
	std::size_t page_size() const { return m_page_size; }

	bool set_access(std::size_t start, std::size_t size, unsigned access)
	{
		if ((start % m_page_size) || (size % m_page_size) || (start > m_size) || ((m_size - start) < size))
			return false;
		else
			return do_set_access(reinterpret_cast<std::uint8_t *>(m_memory) + start, size, access);
	}

	virtual_memory_allocation &operator=(std::nullptr_t)
	{
		if (m_memory)
			do_free(m_memory, m_size);
		m_memory = nullptr;
		m_size = m_page_size = 0U;
		return *this;
	}

	virtual_memory_allocation &operator=(virtual_memory_allocation &&that)
	{
		if (&that != this)
		{
			if (m_memory)
				do_free(m_memory, m_size);
			m_memory = that.m_memory;
			m_size = that.m_size;
			m_page_size = that.m_page_size;
			that.m_memory = nullptr;
			that.m_size = that.m_page_size = 0U;
		}
		return *this;
	}

private:
	static void *do_alloc(std::initializer_list<std::size_t> blocks, std::size_t &size, std::size_t &page_size);
	static void do_free(void *start, std::size_t size);
	static bool do_set_access(void *start, std::size_t size, unsigned access);

	void *m_memory = nullptr;
	std::size_t m_size = 0U, m_page_size = 0U;
};


/*-----------------------------------------------------------------------------
    dynamic_module: load functions from optional shared libraries

    Notes:

        - Supports Mac OS X, Unix and Windows (both desktop and Windows
          Store universal applications)
        - A symbol can be searched in a list of libraries (e.g. more
          revisions of a same library)
-----------------------------------------------------------------------------*/

class dynamic_module
{
public:
	typedef std::unique_ptr<dynamic_module> ptr;

	static ptr open(std::vector<std::string> &&libraries);

	virtual ~dynamic_module() { };

	template <typename T>
	typename std::enable_if_t<std::is_pointer_v<T>, T> bind(char const *symbol)
	{
		return reinterpret_cast<T>(get_symbol_address(symbol));
	}

protected:
	typedef void (*generic_fptr_t)();

	virtual generic_fptr_t get_symbol_address(char const *symbol) = 0;
};

} // namespace osd

//=========================================================================================================
// Dynamic API helpers. Useful in creating a class members that expose dynamically bound API functions.
//
// OSD_DYNAMIC_API(dxgi, "dxgi.dll")
// DYNAMIC_API_FN(dxgi, DWORD, WINAPI, CreateDXGIFactory1, REFIID, void**)
//
// Calling then looks like: DYNAMIC_CALL(CreateDXGIFactory1, p1, p2, etc)
//=========================================================================================================

#if !defined(OSD_UWP)

#define OSD_DYNAMIC_API(apiname, ...) osd::dynamic_module::ptr m_##apiname##module = osd::dynamic_module::open( { __VA_ARGS__ } )
#define OSD_DYNAMIC_API_FN(apiname, ret, conv, fname, ...) ret(conv *m_##fname##_pfn)( __VA_ARGS__ ) = m_##apiname##module->bind<ret(conv *)( __VA_ARGS__ )>(#fname)
#define OSD_DYNAMIC_CALL(fname, ...) (*m_##fname##_pfn) ( __VA_ARGS__ )
#define OSD_DYNAMIC_API_TEST(fname) (m_##fname##_pfn != nullptr)

#else

#define OSD_DYNAMIC_API(apiname, ...)
#define OSD_DYNAMIC_API_FN(apiname, ret, conv, fname, ...)
#define OSD_DYNAMIC_CALL(fname, ...) fname( __VA_ARGS__ )
#define OSD_DYNAMIC_API_TEST(fname) (true)

#endif

#endif  /* __OSDLIB__ */
