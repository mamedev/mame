// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1000C

TMS1000 CMOS versions (3-level stack, HALT pin)
- RAM at top-left, ROM at top-right(rotate CCW)
- ROM ordering is different:
  * row select is linear (0-63)
  * bit select is 7-0 instead of 0-7
  * page select doesn't flip in the middle
- 32-term mpla at bottom-right, different order
- 32-term opla at bottom-left, ordered O7-O0(0 or 1), and A8,4,2,1,S

TODO:
- add TMS1200C (has L input pins like TMS1600)

*/

#include "emu.h"
#include "tms1000c.h"


// device definitions
DEFINE_DEVICE_TYPE(TMS1000C, tms1000c_cpu_device, "tms1000c", "Texas Instruments TMS1000C") // 28-pin SDIP, 10 R pins


tms1000c_cpu_device::tms1000c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, TMS1000C, tag, owner, clock, 8 /* o pins */, 10 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 2 /* x width */, 3 /* stack levels */, 10 /* rom width */, address_map_constructor(FUNC(tms1000c_cpu_device::rom_10bit), this), 6 /* ram width */, address_map_constructor(FUNC(tms1000c_cpu_device::ram_6bit), this))
{ }


// machine configs
void tms1000c_cpu_device::device_add_mconfig(machine_config &config)
{
	// microinstructions PLA, output PLA
	PLA(config, "mpla", 8, 16, 32).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 5, 8, 32).set_format(pla_device::FMT::BERKELEY);
}


// microinstructions decode (different order, no active-negative)
u32 tms1000c_cpu_device::decode_micro(u8 sel)
{
	const u32 md[16] = { M_AUTY, M_AUTA, M_STSL, M_NE, M_C8, M_CIN, M_CKP, M_YTP, M_MTP, M_NATN, M_CKN, M_MTN, M_ATN, M_15TN, M_CKM, M_STO };
	u16 mask = m_mpla->read(sel);
	u32 decode = 0;

	for (int bit = 0; bit < 16; bit++)
		if (mask & (1 << bit))
			decode |= md[bit];

	return decode;
}
