// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#ifndef MAME_CPU_I386_CACHE_H
#define MAME_CPU_I386_CACHE_H

#pragma once

/* To test it outside of Mame
#include <cstdlib>

typedef unsigned char u8;
typedef unsigned int u32;
*/

enum {
	CacheLineBytes16 = 4,
	CacheLineBytes32 = 5,
	CacheLineBytes64 = 6,
	CacheLineBytes128 = 7,
	CacheLineBytes256 = 8,
};

enum {
	CacheDirectMapped = 0,
	Cache2Way = 1,
	Cache4Way = 2,
	Cache8Way = 3,
	Cache16Way = 4
};

enum {
	CacheRead = 0,
	CacheWrite = 1
};

template<int TagBits, int SetBits, int WayBits, int LineBits>
class cpucache {
public:
	// Constructor
	cpucache();
	// Reset the cache
	void reset();
	// Find the cacheline containing data at address
	template <int ReadWrite> u8* search(u32 address);
	// Allocate a cacheline for data at address
	template <int ReadWrite> bool allocate(u32 address, u8 **data);
	// Get the address where the cacheline data should be written back to
	u32 old();
	// Get the address of the first byte of the cacheline that contains data at address
	u32 base(u32 address);
	// Compose the cacheline parameters into an address
	u32 address(u32 tag, u32 set, u32 offset);
	// Get the data of the first cacheline marked as dirty
	u8* first_dirty(u32 &base, bool clean);
	// Get the data of the next cacheline marked as dirty
	u8* next_dirty(u32 &base, bool clean);

private:
	static const int Ways = 1 << WayBits;
	static const int LineBytes = 1 << LineBits;
	static const int Sets = 1 << SetBits;
	static const u32 LineMask = (1 << LineBits) - 1;
	static const u32 SetMask = ((1 << SetBits) - 1) << LineBits;
	static const u32 WayMask = (1 << WayBits) - 1;
	static const int TagShift = LineBits + SetBits;

	struct cacheline {
		u8 data[LineBytes];
		bool allocated;
		bool dirty;
		u32 tag;
		u32 debug_address;
	};

	struct cacheset {
		cacheline lines[Ways];
		int nextway;
	};

	cacheset sets[Sets];
	u32 writeback_base;
	int last_set;
	int last_way;
};

template<int TagBits, int SetBits, int WayBits, int LineBits>
cpucache<TagBits, SetBits, WayBits, LineBits>::cpucache()
{
	reset();
}

template<int TagBits, int SetBits, int WayBits, int LineBits>
void cpucache<TagBits, SetBits, WayBits, LineBits>::reset()
{
	for (int s = 0; s < Sets; s++)
		for (int w = 0; w < Ways; w++)
		{
			sets[s].nextway = 0;
			sets[s].lines[w].allocated = false;
			sets[s].lines[w].dirty = false;
			sets[s].lines[w].debug_address = 0;
		}
	last_set = -1;
	last_way = -1;
}

template<int TagBits, int SetBits, int WayBits, int LineBits>
template<int ReadWrite>
u8* cpucache<TagBits, SetBits, WayBits, LineBits>::search(u32 address)
{
	const int addresset = (address & SetMask) >> LineBits;
	const int addrestag = address >> TagShift;

	for (int w = 0; w < Ways; w++)
		if ((sets[addresset].lines[w].allocated) && (sets[addresset].lines[w].tag == addrestag))
		{
			if (ReadWrite != 0)
				sets[addresset].lines[w].dirty = true;
			return sets[addresset].lines[w].data;
		}
	return nullptr;
}

template<int TagBits, int SetBits, int WayBits, int LineBits>
template<int ReadWrite>
bool cpucache<TagBits, SetBits, WayBits, LineBits>::allocate(u32 address, u8 **data)
{
	const int addresset = (address & SetMask) >> LineBits;
	const int addrestag = address >> TagShift;
	const int victimway = sets[addresset].nextway;
	bool old_allocated, old_dirty;
	bool ret;

	sets[addresset].nextway = (victimway + 1) & WayMask; // decide wich way will be allocated next
	old_allocated = sets[addresset].lines[victimway].allocated;
	old_dirty = sets[addresset].lines[victimway].dirty;
	writeback_base = (sets[addresset].lines[victimway].tag << TagShift) | (address & SetMask);
	sets[addresset].lines[victimway].tag = addrestag;
	sets[addresset].lines[victimway].allocated = true;
	if (ReadWrite == 0)
		sets[addresset].lines[victimway].dirty = false; // caller must write back the cacheline if told so
	else
		sets[addresset].lines[victimway].dirty = true; // line is allocated to write into it
	*data = sets[addresset].lines[victimway].data;
	sets[addresset].lines[victimway].debug_address = address;
	ret = old_allocated; // ret = old_allocated && old_dirty
	if (!old_dirty)
		ret = false;
	return ret; // true if caller must write back the cacheline
}

template<int TagBits, int SetBits, int WayBits, int LineBits>
u32 cpucache<TagBits, SetBits, WayBits, LineBits>::old()
{
	return writeback_base;
}

template<int TagBits, int SetBits, int WayBits, int LineBits>
u32 cpucache<TagBits, SetBits, WayBits, LineBits>::base(u32 address)
{
	return address & ~LineMask;
}

template<int TagBits, int SetBits, int WayBits, int LineBits>
u32 cpucache<TagBits, SetBits, WayBits, LineBits>::address(u32 tag, u32 set, u32 offset)
{
	return (tag << TagShift) | (set << LineBits) | offset;
}

template<int TagBits, int SetBits, int WayBits, int LineBits>
u8* cpucache<TagBits, SetBits, WayBits, LineBits>::first_dirty(u32 &base, bool clean)
{
	for (int s = 0; s < Sets; s++)
		for (int w = 0; w < Ways; w++)
			if (sets[s].lines[w].dirty == true)
			{
				if (clean)
					sets[s].lines[w].dirty = false;
				last_set = s;
				last_way = w;
				base = address(sets[s].lines[w].tag, s, 0);
				return sets[s].lines[w].data;
			}
	return nullptr;
}

template<int TagBits, int SetBits, int WayBits, int LineBits>
u8* cpucache<TagBits, SetBits, WayBits, LineBits>::next_dirty(u32 &base, bool clean)
{
	if (last_set < 0)
		return nullptr;
	while (true)
	{
		last_way++;
		if (last_way == Ways)
		{
			last_way = 0;
			last_set++;
			if (last_set == Sets)
			{
				last_set = -1;
				last_way = -1;
				return nullptr;
			}
		}
		if (sets[last_set].lines[last_way].dirty == true)
		{
			if (clean)
				sets[last_set].lines[last_way].dirty = false;
			base = address(sets[last_set].lines[last_way].tag, last_set, 0);
			return sets[last_set].lines[last_way].data;
		}
	}
}

#endif

/* To test it outside of Mame
const int memorysize = 256 * 1024;
u8 memory[memorysize];

void readline(u8 *data, u32 address)
{
    for (int n = 0; n < 64; n++)
        data[n] = memory[address + n];
}

void writeline(u8 *data, u32 address)
{
    for (int n = 0; n < 64; n++)
        memory[address + n] = data[n];
}

void cache_tester()
{
    cpucache<18, 8, 6, 2> cache;
    bool r;
    u8 *data;
    int address;
    u8 value;

    for (int n = 0; n < memorysize; n++)
        memory[n] = 0xaa ^ n;
    address = std::rand() & (memorysize - 1);
    r = cache.search(address, &data);
    if (r == false)
    {
        r = cache.allocate(address, &data);
        if (r == true)
            writeline(data, cache.base(address));
        readline(data, cache.base(address));
    }
    value = data[address & 63];
    if (value != memory[address])
        printf("Error reading address %d\n\r", address);
}
*/
