// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*******************************************************************************

    Input port macros used by many games in multiple Sega drivers

*******************************************************************************/


/**************************** Coinage Dip Swicthes ****************************/

// [standard]
//                  |  COIN SWITCH 1  |  COIN SWITCH 2
// -----------------+-----------------+-----------------
//                  |   1   2   3   4 |   5   6   7   8
// -----------------+-----------------+-----------------
//  1COIN  1CREDIT  | OFF OFF OFF OFF | OFF OFF OFF OFF
//  1COIN  2CREDITS |  ON OFF OFF OFF |  ON OFF OFF OFF
//  1COIN  3CREDITS | OFF  ON OFF OFF | OFF  ON OFF OFF
//  1COIN  4CREDITS |  ON  ON OFF OFF |  ON  ON OFF OFF
//  1COIN  5CREDITS | OFF OFF  ON OFF | OFF OFF  ON OFF
//  1COIN  6CREDITS |  ON OFF  ON OFF |  ON OFF  ON OFF
//  2COINS 1CREDIT  | OFF  ON  ON OFF | OFF  ON  ON OFF
//  3COINS 1CREDIT  |  ON  ON  ON OFF |  ON  ON  ON OFF
//  4COINS 1CREDIT  | OFF OFF OFF  ON | OFF OFF OFF  ON
//  2COINS 3CREDITS |  ON OFF OFF  ON |  ON OFF OFF  ON
// -----------------+-----------------+-----------------
//  2COINS 1CREDIT  | OFF  ON OFF  ON | OFF  ON OFF  ON
//  4COINS 2CREDITS |                 |
//  5COINS 3CREDITS |                 |
//  6COINS 4CREDITS |                 |
// -----------------+-----------------+-----------------
//  2COINS 1CREDIT  |  ON  ON OFF  ON |  ON  ON OFF  ON
//  4COINS 3CREDITS |                 |
// -----------------+-----------------+-----------------
//  1COIN  1CREDIT  | OFF OFF  ON  ON | OFF OFF  ON  ON
//  2COINS 2CREDITS |                 |
//  3COINS 3CREDITS |                 |
//  4COINS 4CREDITS |                 |
//  5COINS 6CREDITS |                 |
// -----------------+-----------------+-----------------
//  1COIN  1CREDIT  |  ON OFF  ON  ON |  ON OFF  ON  ON
//  2COINS 2CREDITS |                 |
//  3COINS 3CREDITS |                 |
//  4COINS 5CREDITS |                 |
// -----------------+-----------------+-----------------
//  1COIN  1CREDIT  | OFF  ON  ON  ON | OFF  ON  ON  ON
//  2COINS 3CREDITS |                 |
// -----------------+-----------------+-----------------
//  FREE PLAY       |  ON  ON  ON  ON &  ON  ON  ON  ON
// -----------------+-----------------+-----------------
//  COIN SWITCH 1   |  ON  ON  ON  ON |  (anywhare OFF)
//  1COIN  1CREDIT  |                 |
// -----------------+-----------------+-----------------
//  COIN SWITCH 2   |  (anywhare OFF) |  ON  ON  ON  ON
//  1COIN  1CREDIT  |                 |

// [exception 1] not have free play
// A few games don't have "FREE PLAY"
//    example "Hang-On Jr."
// -----------------+-----------------+-----------------
//  1COIN  1CREDIT  |  ON  ON  ON  ON |  ON  ON  ON  ON

// [exception 2] easy to free play
// A few games accept "FREE PLAY" with one-side 4bits ON
//    example "Riddle of Pythagoras"
// -----------------+-----------------+-----------------
//  FREE PLAY       |  ON  ON  ON  ON |  ON  ON  ON  ON
//                  |  ON  ON  ON  ON |  (anywhare OFF)
//                  |  (anywhare OFF) |  ON  ON  ON  ON

#define SEGA_COINAGE_A_PART_H \
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit, 5/3, 6/4" ) \
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit, 4/3" ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )

#define SEGA_COINAGE_A_PART_L \
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit, 5/6" ) \
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit, 4/5" ) \
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit, 2/3" ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )

#define SEGA_COINAGE_B_PART_H \
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit, 5/3, 6/4" ) \
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit, 4/3" ) \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
#define SEGA_COINAGE_B_PART_L \
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit, 5/6" ) \
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit, 4/5" ) \
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit, 2/3" ) \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )

// [standard]
#define SEGA_COINAGE_LOC(DIPBANK) \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(#DIPBANK":1,2,3,4") \
	SEGA_COINAGE_A_PART_H \
	SEGA_COINAGE_A_PART_L \
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" ) \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(#DIPBANK":5,6,7,8") \
	SEGA_COINAGE_B_PART_H \
	SEGA_COINAGE_B_PART_L \
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

// [exception 1] not have free play (allow duplicated settins, show 0x00)
#define SEGA_COINAGE_NO_FREE_LOC(DIPBANK) \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(#DIPBANK":1,2,3,4") \
	SEGA_COINAGE_A_PART_H \
	PORT_DIPSETTING(    0x00, " 1 Coin/1 Credit" ) \
	SEGA_COINAGE_A_PART_L \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(#DIPBANK":5,6,7,8") \
	SEGA_COINAGE_B_PART_H \
	PORT_DIPSETTING(    0x00, " 1 Coin/1 Credit" ) \
	SEGA_COINAGE_B_PART_L

// [exception 1.1] not have free play (forbid duplicated settins, hide 0x00)
#define SEGA_COINAGE_NO_FREE_NO_DUP_LOC(DIPBANK) \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(#DIPBANK":1,2,3,4") \
	SEGA_COINAGE_A_PART_H \
	SEGA_COINAGE_A_PART_L \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(#DIPBANK":5,6,7,8") \
	SEGA_COINAGE_B_PART_H \
	SEGA_COINAGE_B_PART_L

// [exception 2] easy to free play
#define SEGA_COINAGE_EASY_FREE_LOC(DIPBANK) \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(#DIPBANK":1,2,3,4") \
	SEGA_COINAGE_A_PART_H \
	SEGA_COINAGE_A_PART_L \
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(#DIPBANK":5,6,7,8") \
	SEGA_COINAGE_B_PART_H \
	SEGA_COINAGE_B_PART_L \
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
