// license:BSD-3-Clause
// copyright-holders:Luca Elia, Vas Crabb
#include "emu.h"
#include "igsmahjong.h"

#include "mahjong.h"


INPUT_PORTS_START( igs_mahjong_matrix )
	PORT_INCLUDE(mahjong_matrix_1p_bet)

	PORT_MODIFY("KEY0")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_MODIFY("KEY1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_MODIFY("KEY2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_MODIFY("KEY3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_MODIFY("KEY4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED) // no Flip Flop key
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END
