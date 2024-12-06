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

 */
#include "emu.h"
#include "mahjong.h"


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
