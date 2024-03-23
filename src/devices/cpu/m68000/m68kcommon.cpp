// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"

#include "m68kcommon.h"

void m68000_base_device::autovectors_map(address_map &map)
{
	// Eventually add the sync to E due to vpa
	// 8-bit handlers are used here to be compatible with all bus widths
	map(0x3, 0x3).lr8(NAME([] () -> u8 { return autovector(1); }));
	map(0x5, 0x5).lr8(NAME([] () -> u8 { return autovector(2); }));
	map(0x7, 0x7).lr8(NAME([] () -> u8 { return autovector(3); }));
	map(0x9, 0x9).lr8(NAME([] () -> u8 { return autovector(4); }));
	map(0xb, 0xb).lr8(NAME([] () -> u8 { return autovector(5); }));
	map(0xd, 0xd).lr8(NAME([] () -> u8 { return autovector(6); }));
	map(0xf, 0xf).lr8(NAME([] () -> u8 { return autovector(7); }));
}
