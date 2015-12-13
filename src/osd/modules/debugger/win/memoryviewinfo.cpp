// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  memoryviewinfo.c - Win32 debug window handling
//
//============================================================

#include "memoryviewinfo.h"

#include "debug/dvmemory.h"


memoryview_info::memoryview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent) :
	debugview_info(debugger, owner, parent, DVT_MEMORY)
{
}


memoryview_info::~memoryview_info()
{
}


UINT8 memoryview_info::data_format() const
{
	return view<debug_view_memory>()->get_data_format();
}


UINT32 memoryview_info::chunks_per_row() const
{
	return view<debug_view_memory>()->chunks_per_row();
}


bool memoryview_info::reverse() const
{
	return view<debug_view_memory>()->reverse();
}


bool memoryview_info::physical() const
{
	return view<debug_view_memory>()->physical();
}


void memoryview_info::set_expression(char const *string)
{
	view<debug_view_memory>()->set_expression(string);
}

void memoryview_info::set_data_format(UINT8 dataformat)
{
	view<debug_view_memory>()->set_data_format(dataformat);
}

void memoryview_info::set_chunks_per_row(UINT32 rowchunks)
{
	view<debug_view_memory>()->set_chunks_per_row(rowchunks);
}

void memoryview_info::set_reverse(bool reverse)
{
	view<debug_view_memory>()->set_reverse(reverse);
}

void memoryview_info::set_physical(bool physical)
{
	view<debug_view_memory>()->set_physical(physical);
}
