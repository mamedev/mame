// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "corealloc.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <new>


#ifdef MAME_DEBUG

std::uint8_t g_mame_new_prefill_byte(0xcd);

void *operator new(std::size_t n)
{
	void *const result(std::malloc(n));
	if (result)
	{
		std::fill_n(reinterpret_cast<std::uint8_t *>(result), n, g_mame_new_prefill_byte);
		return result;
	}
	else
	{
		throw std::bad_alloc();
	}
}

void operator delete(void *ptr) noexcept
{
	std::free(ptr);
}

#endif // MAME_DEBUG
