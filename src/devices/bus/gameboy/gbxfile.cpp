// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 GBX file format helpers

 ***************************************************************************/

#include "emu.h"
#include "gbxfile.h"

#include <cstring>


namespace bus::gameboy::gbxfile {

bool get_data(
		memory_region *region,
		leader_1_0 &leader,
		u8 const *&extra,
		u32 &extralen)
{
	// no region, no dice
	if (!region)
		return false;

	// needs to contain enough data for the trailer at the very least
	auto const bytes(region->bytes());
	if (bytes < sizeof(trailer))
		return false;

	// check for supported format
	u8 *const base(&region->as_u8());
	trailer t;
	std::memcpy(&t, &base[bytes - sizeof(t)], sizeof(t));
	t.swap();
	if ((MAGIC_GBX != t.magic) || (1 != t.ver_maj))
		return false;

	// check that the footer fits and the leader doesn't overlap the trailer
	if ((bytes < t.size) || ((sizeof(leader) + sizeof(t)) > t.size))
		return false;

	// get leader in host byte order
	std::memcpy(&leader, &base[bytes - t.size], sizeof(leader));
	leader.swap();

	// get pointer to extra data if there's any
	extralen = t.size - sizeof(leader) - sizeof(t);
	if (extralen)
		extra = &base[bytes - t.size + sizeof(leader)];
	else
		extra = nullptr;

	// all good
	return true;
}

} // namespace bus::gameboy::gbxfile
