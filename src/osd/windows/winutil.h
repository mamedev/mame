// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutil.h - Win32 OSD core utility functions
//
//============================================================

#ifndef __WINUTIL__
#define __WINUTIL__

#include "osdcore.h"
#include <string>
#include <vector>

// Shared code
osd_dir_entry_type win_attributes_to_entry_type(DWORD attributes);
BOOL win_is_gui_application(void);
HMODULE WINAPI GetModuleHandleUni();

//-----------------------------------------------------------
//  Lazy loaded function using LoadLibrary / GetProcAddress
//-----------------------------------------------------------

class lazy_loaded_function
{
private:
	std::string                 m_name;
	std::vector<std::wstring>   m_dll_names;
	HMODULE                     m_module;
	bool                        m_initialized;

protected:
	void check_init();
	FARPROC m_pfn;

public:
	lazy_loaded_function(const char * name, const wchar_t* dll_name);
	lazy_loaded_function(const char * name, const wchar_t** dll_names, int dll_count);
	~lazy_loaded_function();
	int initialize();
	const char * name() { return m_name.c_str(); }
};

// No parameters
template <class TRet>
class lazy_loaded_function_ret : public lazy_loaded_function
{
public:
	lazy_loaded_function_ret(const char * name, const wchar_t* dll_name)
		: lazy_loaded_function(name, &dll_name, 1)
	{
	}

	lazy_loaded_function_ret(const char * name, const wchar_t** dll_names, int dll_count)
		: lazy_loaded_function(name, dll_names, dll_count)
	{
	}

	TRet operator ()()
	{
		check_init();
		return ((TRet(__stdcall *) ())m_pfn)();
	}
};

// One parameter
template <class TRet, class P1>
class lazy_loaded_function_p1 : public lazy_loaded_function
{
public:
	lazy_loaded_function_p1(const char * name, const wchar_t* dll_name)
		: lazy_loaded_function(name, &dll_name, 1)
	{
	}

	lazy_loaded_function_p1(const char * name, const wchar_t** dll_names, int dll_count)
		: lazy_loaded_function(name, dll_names, dll_count)
	{
	}

	TRet operator ()(P1 p1)
	{
		check_init();
		return ((TRet(__stdcall *) (P1))m_pfn)(p1);
	}
};

// Two parameters
template <class TRet, class P1, class P2>
class lazy_loaded_function_p2 : public lazy_loaded_function
{
public:
	lazy_loaded_function_p2(const char * name, const wchar_t* dll_name)
		: lazy_loaded_function(name, &dll_name, 1)
	{
	}

	lazy_loaded_function_p2(const char * name, const wchar_t** dll_names, int dll_count)
		: lazy_loaded_function(name, dll_names, dll_count)
	{
	}

	TRet operator ()(P1 p1, P2 p2)
	{
		check_init();
		return ((TRet(__stdcall *) (P1, P2))m_pfn)(p1, p2);
	}
};

// Three parameters
template <class TRet, class P1, class P2, class P3>
class lazy_loaded_function_p3 : public lazy_loaded_function
{
public:
	lazy_loaded_function_p3(const char * name, const wchar_t* dll_name)
		: lazy_loaded_function(name, &dll_name, 1)
	{
	}

	lazy_loaded_function_p3(const char * name, const wchar_t** dll_names, int dll_count)
		: lazy_loaded_function(name, dll_names, dll_count)
	{
	}

	TRet operator ()(P1 p1, P2 p2, P3 p3)
	{
		check_init();
		return ((TRet(__stdcall *) (P1, P2, P3))m_pfn)(p1, p2, p3);
	}
};

// Four parameters
template <class TRet, class P1, class P2, class P3, class P4>
class lazy_loaded_function_p4 : public lazy_loaded_function
{
public:
	lazy_loaded_function_p4(const char * name, const wchar_t* dll_name)
		: lazy_loaded_function(name, &dll_name, 1)
	{
	}

	lazy_loaded_function_p4(const char * name, const wchar_t** dll_names, int dll_count)
		: lazy_loaded_function(name, dll_names, dll_count)
	{
	}

	TRet operator ()(P1 p1, P2 p2, P3 p3, P4 p4)
	{
		check_init();
		return ((TRet(__stdcall *) (P1, P2, P3, P4))m_pfn)(p1, p2, p3, p4);
	}
};

// Five parameters
template <class TRet, class P1, class P2, class P3, class P4, class P5>
class lazy_loaded_function_p5 : public lazy_loaded_function
{
public:
	lazy_loaded_function_p5(const char * name, const wchar_t* dll_name)
		: lazy_loaded_function(name, &dll_name, 1)
	{
	}

	lazy_loaded_function_p5(const char * name, const wchar_t** dll_names, int dll_count)
		: lazy_loaded_function(name, dll_names, dll_count)
	{
	}

	TRet operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		check_init();
		return ((TRet(__stdcall *) (P1, P2, P3, P4, P5))m_pfn)(p1, p2, p3, p4, p5);
	}
};

#endif // __WINUTIL__
