// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

      Common Dynax mahjong DIP switch settings

***************************************************************************/
#ifndef MAME_DYNAX_MJDIPSW_H
#define MAME_DYNAX_MJDIPSW_H

#pragma once


#define MAHJONG_PAYOUT_RATE(shift, loc) \
		PORT_DIPNAME( 0x0f << shift, 0x07 << shift, "Payout Rate" ) PORT_DIPLOCATION(loc) \
		PORT_DIPSETTING(             0x00 << shift, "50%" ) \
		PORT_DIPSETTING(             0x01 << shift, "53%" ) \
		PORT_DIPSETTING(             0x02 << shift, "56%" ) \
		PORT_DIPSETTING(             0x03 << shift, "59%" ) \
		PORT_DIPSETTING(             0x04 << shift, "62%" ) \
		PORT_DIPSETTING(             0x05 << shift, "65%" ) \
		PORT_DIPSETTING(             0x06 << shift, "68%" ) \
		PORT_DIPSETTING(             0x07 << shift, "71%" ) \
		PORT_DIPSETTING(             0x08 << shift, "75%" ) \
		PORT_DIPSETTING(             0x09 << shift, "78%" ) \
		PORT_DIPSETTING(             0x0a << shift, "81%" ) \
		PORT_DIPSETTING(             0x0b << shift, "84%" ) \
		PORT_DIPSETTING(             0x0c << shift, "87%" ) \
		PORT_DIPSETTING(             0x0d << shift, "90%" ) \
		PORT_DIPSETTING(             0x0e << shift, "93%" ) \
		PORT_DIPSETTING(             0x0f << shift, "96%" )

#define MAHJONG_ODDS_RATE(shift, loc) \
		PORT_DIPNAME( 0x03 << shift, 0x00 << shift, "Odds Rate" ) PORT_DIPLOCATION(loc) \
		PORT_DIPSETTING(             0x03 << shift, "1 2 4 8 12 16 24 32" ) \
		PORT_DIPSETTING(             0x00 << shift, "1 2 3 5 8 15 30 50" ) \
		PORT_DIPSETTING(             0x01 << shift, "1 2 3 5 10 25 50 100" ) \
		PORT_DIPSETTING(             0x02 << shift, "1 2 3 5 10 50 100 200" )

#define MAHJONG_COINAGE(shift, loc) \
		PORT_DIPNAME( 0x03 << shift, 0x03 << shift, DEF_STR(Coinage) ) PORT_DIPLOCATION(loc) /* ＣＯＩＮ　ＲＡＴＥ   */ \
		PORT_DIPSETTING(             0x03 << shift, DEF_STR(1C_1C) )                         /* １コイン　　１プレイ */ \
		PORT_DIPSETTING(             0x02 << shift, DEF_STR(1C_2C) )                         /* １コイン　　２プレイ */ \
		PORT_DIPSETTING(             0x01 << shift, DEF_STR(1C_5C) )                         /* １コイン　　５プレイ */ \
		PORT_DIPSETTING(             0x00 << shift, "1 Coin/10 Credits" )                    /* １コイン　１０プレイ */

#define MAHJONG_NOTE_CREDITS(shift, loc, ct, cs) \
		PORT_DIPNAME( 0x01 << shift, 0x00 << shift, "Credits Per Note" ) PORT_DIPLOCATION(loc)                 /* ＮＯＴＥ　ＲＡＴＥ */ \
		PORT_DIPSETTING(             0x01 << shift, "5" )   PORT_CONDITION(ct, 0x03 << cs, EQUALS, 0x03 << cs) /* ＣＯＩＮ×５        */ \
		PORT_DIPSETTING(             0x01 << shift, "10" )  PORT_CONDITION(ct, 0x03 << cs, EQUALS, 0x02 << cs) \
		PORT_DIPSETTING(             0x01 << shift, "25" )  PORT_CONDITION(ct, 0x03 << cs, EQUALS, 0x01 << cs) \
		PORT_DIPSETTING(             0x01 << shift, "50" )  PORT_CONDITION(ct, 0x03 << cs, EQUALS, 0x00 << cs) \
		PORT_DIPSETTING(             0x00 << shift, "10" )  PORT_CONDITION(ct, 0x03 << cs, EQUALS, 0x03 << cs) /* ＣＯＩＮ×１０      */ \
		PORT_DIPSETTING(             0x00 << shift, "20" )  PORT_CONDITION(ct, 0x03 << cs, EQUALS, 0x02 << cs) \
		PORT_DIPSETTING(             0x00 << shift, "50" )  PORT_CONDITION(ct, 0x03 << cs, EQUALS, 0x01 << cs) \
		PORT_DIPSETTING(             0x00 << shift, "100" ) PORT_CONDITION(ct, 0x03 << cs, EQUALS, 0x00 << cs)

#define MAHJONG_YAKUMAN_BONUS(shift, dflt, loc) \
		PORT_DIPNAME( 0x07 << shift, dflt << shift, "Yakuman Bonus Cycle" ) PORT_DIPLOCATION(loc) /* 役満ボーナスの設定周期 */ \
		PORT_DIPSETTING(             0x07 << shift, "None" )                                      /* 無し                   */ \
		PORT_DIPSETTING(             0x06 << shift, "First time only" )                           /* 初回のみ               */ \
		PORT_DIPSETTING(             0x05 << shift, "Every 300 coins" )                           /* ３００コイン毎         */ \
		PORT_DIPSETTING(             0x04 << shift, "Every 500 coins" )                           /* ５００コイン毎         */ \
		PORT_DIPSETTING(             0x03 << shift, "Every 700 coins" )                           /* ７００コイン毎         */ \
		PORT_DIPSETTING(             0x02 << shift, "Every 1000 coins" )                          /* １０００コイン毎       */ \
	/*  PORT_DIPSETTING(             0x01 << shift, "Every 1000 coins" )*/ \
	/*  PORT_DIPSETTING(             0x00 << shift, "Every 1000 coins" )*/

#endif // MAME_DYNAX_MJDIPSW_H
