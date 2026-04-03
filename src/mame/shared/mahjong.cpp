// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 "Standard" 5*6 mahjong keyboard matrix used for numerous arcade games.

 Key names
 MAME          Japanese           (alt)   Dynax   IGS TW   (alt)   IGS CN   (alt)   IGS EN
 --------------------------------------------------------------------------------------------
 Flip Flop     フリップフロップ   F.F     F
 Start         スタート                   S       開始             开始             START
 Bet           ベット                     B       押注     押      押注     押      BET
 Take Score    テイクスコアー             T       得分             得分             TAKE
 Double Up     ダブルアップ               W       比倍     續玩    比倍     续玩    DUP
 Big           ビッグ                     B       大       左      大       左      BIG
 Small         スモール                   S       小       右      小       右      SMALL
 Last Chance   ラストチャンス             L       海底             海底
 Kan           カン                       K       槓               杠               GUN
 Pon           ポン                       P       碰               碰               PON
 Chi           チー                       T       吃               吃               EAT
 Reach         リーチ                     R       聽               听               LISTEN
 Ron           ロン                       N       胡               胡               WHO

 Keys present
 Name          Jaleco  Sega  Dynax  IGS
 -----------------------------------------
 Flip Flop     *       *     *
 Start         *       *     *      *
 Bet                   *     *      *
 Take Score                  *      *
 Double Up                   *      *
 Big                         *      *
 Small                       *      *
 Last Chance           *     *      *
 Kan           *       *     *      *
 Pon           *       *     *      *
 Chi           *       *     *      *
 Reach         *       *     *      *
 Ron           *       *     *      *


 "Standard" mahjong keyboards use a 4*6 matrix.  Gambling controls add an additional row.

 A         E         I         M         Kan       Start
 B         F         J         N         Reach     Bet
 C         G         K         Chi       Ron       -
 D         H         L         Pon       -         -
 Last      Take      W-Up      F.Flop    Big       Small

 Hanafuda keyboards use a subset of the matrix.
 These are rare.  A mahjong panel with additional labels would usually be used.

 1         5         -         Yes       -         Start
 2         6         -         No        -         Bet
 3         7         -         -         -         -
 4         8         -         -         -         -
 -         Take      W-Up      F.Flop    Big       Small

 Rarest of all is the 6-button hanafuda (or "hanaroku") keyboard, using a 6*6 matrix.
 This has controls for two player positions in one matrix.

 1P 1      1P 2      1P 3      1P 4      Payout    F.Flop
 1P 5      1P No     1P Yes    1P 6      -         -
 -         1P Start  1P Bet    -         -         -
 2P 1      2P 2      2P 3      2P 4      -         -
 2P 5      2P No     2P Yes    2P 6      -         -
 -         2P Start  2P Bet    -         -         -

 Columns are usually wired from left to right from least significant to most significant bit.
 Nichibutsu wires the bits in the opposite order.
 The Jaleco MegaSystem 32 has the columns rotated by one position so the Start column is the least significant bit.

 Note that non-standard mahjong/hanafuda keyboards exist:
 * Some Nichibutsu hanafuda games use the Reach/Ron positions for Yes/No (rather than M/N).
 * SNK Neo Geo mahjong keyboards use a non-standard 3*7 matrix.

 */
#include "emu.h"
#include "mahjong.h"


namespace {

INPUT_PORTS_START(mahjong_matrix_bet)
	PORT_INCLUDE(mahjong_matrix_1p_bet)

	PORT_MODIFY("KEY0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START)                PORT_PLAYER(1)

	PORT_START("KEY5")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


INPUT_PORTS_START(mahjong_matrix_bet_wup)
	PORT_INCLUDE(mahjong_matrix_1p_bet_wup)

	PORT_MODIFY("KEY0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START)                PORT_PLAYER(1)

	PORT_START("KEY5")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


INPUT_PORTS_START(hanafuda_matrix_bet)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A)           PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E)           PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES)         PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START)                PORT_PLAYER(1)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B)           PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F)           PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO)          PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET)          PORT_PLAYER(1)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C)           PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G)           PORT_PLAYER(1)
	PORT_BIT(0x3c, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D)           PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H)           PORT_PLAYER(1)
	PORT_BIT(0x3c, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY4")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP)    PORT_PLAYER(1)
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


INPUT_PORTS_START(hanafuda_matrix_bet_wup)
	PORT_INCLUDE(hanafuda_matrix_bet)

	PORT_MODIFY("KEY4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE)        PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP)    PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG)          PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL)        PORT_PLAYER(1)
INPUT_PORTS_END


INPUT_PORTS_START(hanaroku_panel)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A)           PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_B)           PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_C)           PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_D)           PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP)    // only a single Flip Flop

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_E)           PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_NO)          PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_YES)         PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_F)           PORT_PLAYER(1)
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START)                PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_BET)          PORT_PLAYER(1)
	PORT_BIT(0x38, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A)           PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_B)           PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_C)           PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_D)           PORT_PLAYER(2)
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_E)           PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_NO)          PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_YES)         PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_F)           PORT_PLAYER(2)
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START)                PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_BET)          PORT_PLAYER(2)
	PORT_BIT(0x38, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END



class mahjong_panel_device_base : public device_t, public device_mahjong_panel_interface
{
public:
	virtual u8 read(u8 select) override
	{
		u8 result = 0x3f;
		for (unsigned i = 0; m_keys.size() > i; ++i)
		{
			if (!BIT(select, i))
				result &= m_keys[i]->read();
		}
		return result;
	}

protected:
	mahjong_panel_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_mahjong_panel_interface(mconfig, *this),
		m_keys(*this, "KEY%u", 0U)
	{
	}

	virtual void device_start() override ATTR_COLD { }

private:
	required_ioport_array<6> m_keys;
};


class mahjong_panel_device : public mahjong_panel_device_base
{
public:
	mahjong_panel_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock) :
		mahjong_panel_device_base(mconfig, MAHJONG_PANEL, tag, owner, clock)
	{
	}

protected:
	ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return INPUT_PORTS_NAME(mahjong_matrix_bet);
	}
};


class mahjong_medal_panel_device : public mahjong_panel_device_base
{
public:
	mahjong_medal_panel_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock) :
		mahjong_panel_device_base(mconfig, MAHJONG_MEDAL_PANEL, tag, owner, clock)
	{
	}

protected:
	ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return INPUT_PORTS_NAME(mahjong_matrix_bet_wup);
	}
};


class hanafuda_panel_device : public mahjong_panel_device_base
{
public:
	hanafuda_panel_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock) :
		mahjong_panel_device_base(mconfig, HANAFUDA_PANEL, tag, owner, clock)
	{
	}

protected:
	ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return INPUT_PORTS_NAME(hanafuda_matrix_bet);
	}
};


class hanafuda_medal_panel_device : public mahjong_panel_device_base
{
public:
	hanafuda_medal_panel_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock) :
		mahjong_panel_device_base(mconfig, HANAFUDA_MEDAL_PANEL, tag, owner, clock)
	{
	}

protected:
	ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return INPUT_PORTS_NAME(hanafuda_matrix_bet_wup);
	}
};


class hanaroku_panel_device : public mahjong_panel_device_base
{
public:
	hanaroku_panel_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock) :
		mahjong_panel_device_base(mconfig, HANAROKU_PANEL, tag, owner, clock)
	{
	}

protected:
	ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return INPUT_PORTS_NAME(hanaroku_panel);
	}
};

} // anonymous namespace



DEFINE_DEVICE_TYPE(MAHJONG_PANEL_CONNECTOR, mahjong_panel_connector_device, "mahjong_panel_connector", "Mahjong panel connector")

DEFINE_DEVICE_TYPE_PRIVATE(MAHJONG_PANEL,        device_mahjong_panel_interface, mahjong_panel_device,        "mahjong_panel",        "Mahjong panel")
DEFINE_DEVICE_TYPE_PRIVATE(MAHJONG_MEDAL_PANEL,  device_mahjong_panel_interface, mahjong_medal_panel_device,  "mahjong_medal_panel",  "Mahjong panel with double-up controls")
DEFINE_DEVICE_TYPE_PRIVATE(HANAFUDA_PANEL,       device_mahjong_panel_interface, hanafuda_panel_device,       "hanafuda_panel",       "Hanafuda panel")
DEFINE_DEVICE_TYPE_PRIVATE(HANAFUDA_MEDAL_PANEL, device_mahjong_panel_interface, hanafuda_medal_panel_device, "hanafuda_medal_panel", "Hanafuda panel with double-up controls")
DEFINE_DEVICE_TYPE_PRIVATE(HANAROKU_PANEL,       device_mahjong_panel_interface, hanaroku_panel_device,       "hanaroku_panel",       "Hanaroku panel")


device_mahjong_panel_interface::device_mahjong_panel_interface(
		machine_config const &mconfig,
		device_t &device) :
	device_interface(device, "mahjongpanel")
{
}

device_mahjong_panel_interface::~device_mahjong_panel_interface()
{
}


mahjong_panel_connector_device::mahjong_panel_connector_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	device_t(mconfig, MAHJONG_PANEL_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_mahjong_panel_interface>(mconfig, *this),
	m_panel(nullptr)
{
}

mahjong_panel_connector_device::~mahjong_panel_connector_device()
{
}

void mahjong_panel_connector_device::device_start()
{
	m_panel = get_card_device();
}



INPUT_PORTS_START(mahjong_matrix_1p)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON)
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY4")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


INPUT_PORTS_START(mahjong_matrix_1p_ff)
	PORT_INCLUDE(mahjong_matrix_1p)

	PORT_MODIFY("KEY4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP)
INPUT_PORTS_END


INPUT_PORTS_START(mahjong_matrix_1p_bet)
	PORT_INCLUDE(mahjong_matrix_1p_ff)

	PORT_MODIFY("KEY1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET)

	PORT_MODIFY("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE)
INPUT_PORTS_END


INPUT_PORTS_START(mahjong_matrix_1p_bet_wup)
	PORT_INCLUDE(mahjong_matrix_1p_bet)

	PORT_MODIFY("KEY4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL)
INPUT_PORTS_END


INPUT_PORTS_START(mahjong_matrix_2p)
	PORT_INCLUDE(mahjong_matrix_1p)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A)            PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E)            PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I)            PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M)            PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN)          PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B)            PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F)            PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J)            PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N)            PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH)        PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C)            PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G)            PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K)            PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI)          PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON)          PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D)            PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H)            PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L)            PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON)          PORT_PLAYER(2)
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY9")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


INPUT_PORTS_START(mahjong_matrix_2p_ff)
	PORT_INCLUDE(mahjong_matrix_2p)

	PORT_MODIFY("KEY4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP)

	PORT_MODIFY("KEY9")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP)    PORT_PLAYER(2)
INPUT_PORTS_END


INPUT_PORTS_START(mahjong_matrix_2p_bet)
	PORT_INCLUDE(mahjong_matrix_2p_ff)

	PORT_MODIFY("KEY1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET)

	PORT_MODIFY("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE)

	PORT_MODIFY("KEY6")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET)          PORT_PLAYER(2)

	PORT_MODIFY("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE)  PORT_PLAYER(2)
INPUT_PORTS_END


INPUT_PORTS_START(mahjong_matrix_2p_bet_wup)
	PORT_INCLUDE(mahjong_matrix_2p_bet)

	PORT_MODIFY("KEY4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL)

	PORT_MODIFY("KEY9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE)        PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP)    PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG)          PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL)        PORT_PLAYER(2)
INPUT_PORTS_END
