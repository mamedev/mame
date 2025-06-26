// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Sharp MZ-1E35 (for MZ-2800) & MZ-2500 Dust Box equivalent

TODO:
- Just enough to make it being recognized by Dust Box;
- How to reproduce sound? Are .opn files requiring a TSR sound driver or it's just for
  specific Dust Box games?

**************************************************************************************************/


#include "emu.h"
#include "mz1e35.h"

#include "speaker.h"

DEFINE_DEVICE_TYPE(MZ1E35, mz1e35_device, "mz1e35", "Sharp MZ-1E35 ADPCM")

mz1e35_device::mz1e35_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mz80_exp_device(mconfig, MZ1E35, tag, owner, clock)
	, m_opl(*this, "opl")
{
}

void mz1e35_device::io_map(address_map &map)
{
	map(0x98, 0x99).mirror(0xff00).rw("opl", FUNC(y8950_device::read), FUNC(y8950_device::write));
}

void mz1e35_device::device_add_mconfig(machine_config &config)
{
	// SP OUT
	SPEAKER(config, "mono").front_center();

	// TODO: check how it draws clock from expansion I/F
	Y8950(config, m_opl, XTAL(4'000'000));
	m_opl->add_route(ALL_OUTPUTS, "mono", 1.0, 0);

	// TODO: LINE IN / LINE OUT / MIC IN
}
