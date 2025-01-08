// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"
#include "toaplan_coincounter.h"

DEFINE_DEVICE_TYPE(TOAPLAN_COINCOUNTER, toaplan_coincounter_device, "toaplan_coincounter", "Toaplan Coin Counter")

toaplan_coincounter_device::toaplan_coincounter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TOAPLAN_COINCOUNTER, tag, owner, clock)
{
}

void toaplan_coincounter_device::device_start()
{
}

void toaplan_coincounter_device::device_reset()
{
}

void toaplan_coincounter_device::coin_w(u8 data)
{
	/* +----------------+------ Bits 7-5 not used ------+--------------+ */
	/* | Coin Lockout 2 | Coin Lockout 1 | Coin Count 2 | Coin Count 1 | */
	/* |     Bit 3      |     Bit 2      |     Bit 1    |     Bit 0    | */

	if (data & 0x0f)
	{
		machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
		machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3));
		machine().bookkeeping().coin_counter_w(0, BIT( data, 0));
		machine().bookkeeping().coin_counter_w(1, BIT( data, 1));
	}
	else
	{
		machine().bookkeeping().coin_lockout_global_w(1);    // Lock all coin slots
	}
	if (data & 0xf0)
	{
		logerror("Writing unknown upper bits (%02x) to coin control\n",data);
	}
}
