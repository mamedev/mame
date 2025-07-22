// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*******************************************************************************

    Input port macros used by many games in multiple KONAMI drivers

*******************************************************************************/


/***************************** 8bit Players Inputs ****************************/

/*********************** Prototypes 8bit ***********************/

#define KONAMI8_MONO_4WAY( direction1, direction2, button1, button2, button3 )  \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction1 ) PORT_4WAY    \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction2 ) PORT_4WAY    \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY              \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY            \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, button1 )    \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, button2 )    \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, button3 )

#define KONAMI8_COCKTAIL_4WAY( direction1, direction2, button1, button2, button3 )  \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction1 ) PORT_4WAY PORT_COCKTAIL  \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction2 ) PORT_4WAY PORT_COCKTAIL  \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL            \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL          \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, button1 ) PORT_COCKTAIL  \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, button2 ) PORT_COCKTAIL  \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, button3 ) PORT_COCKTAIL

#define KONAMI8_MONO_8WAY( direction1, direction2, button1, button2, button3 )  \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction1 ) PORT_8WAY    \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction2 ) PORT_8WAY    \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY              \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY            \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, button1 )    \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, button2 )    \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, button3 )

#define KONAMI8_COCKTAIL_8WAY( direction1, direction2, button1, button2, button3 )  \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction1 ) PORT_8WAY PORT_COCKTAIL  \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction2 ) PORT_8WAY PORT_COCKTAIL  \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL            \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL          \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, button1 ) PORT_COCKTAIL      \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, button2 ) PORT_COCKTAIL      \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, button3 ) PORT_COCKTAIL

#define KONAMI8_MULTI_8WAY( player, direction1, direction2, button1, button2, button3 ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction1 ) PORT_8WAY PORT_PLAYER(player)    \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_##direction2 ) PORT_8WAY PORT_PLAYER(player)    \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(player)      \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(player)    \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, button1 ) PORT_PLAYER(player)    \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, button2 ) PORT_PLAYER(player)    \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, button3 ) PORT_PLAYER(player)

#define KONAMI8_LR_40( player, button1, button2, button3 )              \
	KONAMI8_MULTI_8WAY( player, LEFT, RIGHT, button1, button2, button3 )

#define KONAMI8_RL_40( player, button1, button2, button3 )              \
	KONAMI8_MULTI_8WAY( player, RIGHT, LEFT, button1, button2, button3 )


/*********************** Actual Inputs 8bit ***********************/

/* Cocktail Cabinet 4Way Inputs */
#define KONAMI8_MONO_4WAY_B12_UNK   \
	KONAMI8_MONO_4WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_COCKTAIL_4WAY_B12_UNK   \
	KONAMI8_COCKTAIL_4WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_MONO_4WAY_B123_UNK  \
	KONAMI8_MONO_4WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_COCKTAIL_4WAY_B123_UNK  \
	KONAMI8_COCKTAIL_4WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )


/* Cocktail Cabinet 8Way Inputs */
/* 1 Button */
#define KONAMI8_MONO_B1_UNK \
	KONAMI8_MONO_8WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_UNKNOWN, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_COCKTAIL_B1_UNK \
	KONAMI8_COCKTAIL_8WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_UNKNOWN, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* 2 Buttons */
#define KONAMI8_MONO_B12_UNK    \
	KONAMI8_MONO_8WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_COCKTAIL_B12_UNK    \
	KONAMI8_COCKTAIL_8WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* 3 Buttons */
#define KONAMI8_MONO_B123_UNK   \
	KONAMI8_MONO_8WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_COCKTAIL_B123_UNK   \
	KONAMI8_COCKTAIL_8WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_MONO_B213_UNK   \
	KONAMI8_MONO_8WAY( LEFT, RIGHT, IPT_BUTTON2, IPT_BUTTON1, IPT_BUTTON3 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_COCKTAIL_B213_UNK   \
	KONAMI8_COCKTAIL_8WAY( LEFT, RIGHT, IPT_BUTTON2, IPT_BUTTON1, IPT_BUTTON3 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* 2 Buttons + Start */
#define KONAMI8_MONO_B12_START  \
	KONAMI8_MONO_8WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

#define KONAMI8_COCKTAIL_B12_START  \
	KONAMI8_COCKTAIL_8WAY( LEFT, RIGHT, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )


/* Upright Multiplayer Cabinet Inputs */
/* 1 Button */
#define KONAMI8_B1( player )    \
	KONAMI8_LR_40( player, IPT_BUTTON1, IPT_UNKNOWN, IPT_UNKNOWN )

#define KONAMI8_B1_UNK( player )    \
	KONAMI8_B1( player )    \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* 2 Buttons */
#define KONAMI8_B12( player )   \
	KONAMI8_LR_40( player, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN )

#define KONAMI8_B12_UNK( player )   \
	KONAMI8_B12( player )   \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define KONAMI8_B12_START( player ) \
	KONAMI8_B12( player )   \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START##player )

#define KONAMI8_B12_COIN_START( player )    \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(player)    \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(player)   \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(player)      \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(player)    \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player)    \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player)    \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN##player )       \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START##player )

#define KONAMI8_B21_UNK( player )   \
	KONAMI8_LR_40( player, IPT_BUTTON2, IPT_BUTTON1, IPT_UNKNOWN )  \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* 3 Buttons */
#define KONAMI8_B123( player )  \
	KONAMI8_LR_40( player, IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3 )

#define KONAMI8_B123_UNK( player )  \
	KONAMI8_B123( player )  \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

#define KONAMI8_B123_START( player )    \
	KONAMI8_B123( player )  \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START##player )

#define KONAMI8_B132( player )  \
	KONAMI8_LR_40( player, IPT_BUTTON1, IPT_BUTTON3, IPT_BUTTON2 )

#define KONAMI8_B321( player )  \
	KONAMI8_LR_40( player, IPT_BUTTON3, IPT_BUTTON2, IPT_BUTTON1 )


/* vendetta.c uses inputs with switched Left/Right directions. We add these inputs here as well
because they just need a few lines of code */
#define KONAMI8_RL_B12_COIN( player )   \
	KONAMI8_RL_40( player, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN )  \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN##player )


/* A few drivers uses bit0 for Start and shuffled joystick inputs */
#define KONAMI8_ALT( player, button1, button2, button3 )    \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START##player )  \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(player)      \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(player)    \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(player)    \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(player)   \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, button1 ) PORT_PLAYER(player)    \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, button2 ) PORT_PLAYER(player)    \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, button3 ) PORT_PLAYER(player)

#define KONAMI8_ALT_B12( player )   \
	KONAMI8_ALT( player, IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN )

#define KONAMI8_ALT_B21( player )   \
	KONAMI8_ALT( player, IPT_BUTTON2, IPT_BUTTON1, IPT_UNKNOWN )

#define KONAMI8_ALT_B123( player )  \
	KONAMI8_ALT( player, IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3 )


/**************************** 16bit Players Inputs ****************************/

/* Upright Multiplayer Cabinet Inputs */
#define KONAMI16_LSB_40( player, button3 )  \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, button3            ) PORT_PLAYER(player)

#define KONAMI16_MSB_40( player, button3 )  \
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, button3            ) PORT_PLAYER(player)

#define KONAMI16_LSB_40_UDLR( player, button3 ) \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, button3            ) PORT_PLAYER(player)

#define KONAMI16_MSB_40_UDLR( player, button3 ) \
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, button3            ) PORT_PLAYER(player)

#define KONAMI16_LSB( player, button3, start )  \
	KONAMI16_LSB_40( player, button3 )          \
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, start )

#define KONAMI16_MSB( player, button3, start )  \
	KONAMI16_MSB_40( player, button3 )          \
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, start )

#define KONAMI16_LSB_UDLR( player, button3, start ) \
	KONAMI16_LSB_40_UDLR( player, button3 )         \
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, start )

#define KONAMI16_MSB_UDLR( player, button3, start ) \
	KONAMI16_MSB_40_UDLR( player, button3 )         \
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, start )

/* Cocktail Cabinet 4Way Inputs */
#define KONAMI16_LSB_40_MONO_4WAY( button3 )  \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) \
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, button3            )

#define KONAMI16_LSB_40_COCKTAIL_4WAY( button3 )  \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY PORT_COCKTAIL \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY PORT_COCKTAIL \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY PORT_COCKTAIL \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_COCKTAIL \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_COCKTAIL \
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, button3            ) PORT_COCKTAIL

#define KONAMI16_LSB_MONO_4WAY( button3, start ) \
	KONAMI16_LSB_40_MONO_4WAY( button3 )         \
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, start )

#define KONAMI16_LSB_COCKTAIL_4WAY( button3, start ) \
	KONAMI16_LSB_40_COCKTAIL_4WAY( button3 )         \
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, start )


/******************** System Inputs (Coin, Start & Service) *******************/

#define KONAMI8_SYSTEM_10   \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )      \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )      \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )   \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )     \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )

#define KONAMI8_SYSTEM_UNK  \
	KONAMI8_SYSTEM_10       \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )


/**************************** Coinage Dip Switches ****************************/

/* Konami games from 80s-90s basically use only two kind of coinage dips. The only
difference is in the settings corresponding to 0x00, which could be either 4C_5C
or a "Free_Play"-related option. Actually, in the latter case the behavior may change
depending on the game code:
Coin A - 0x00 could produce the following effects
    Free_Play = nomen omen (for both players)
    Invalid = both coin slots disabled
Coin B - 0x00 could produce the following effects
    Free_Play = nomen omen (for both players)
    No Coin B = coin slot B open (coins produce sound), but no effect on coin counter
    None = coin slot B disabled
    No Credits = both coin slots open, but no effect on coin counters
    Invalid = both coin slots disabled
Accordingly, we pass below different strings for different games */

#define KONAMI_COINAGE_LOC( STRING_A, STRING_B, DIPBANK )   \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION(#DIPBANK":1,2,3,4")    \
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )    \
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )    \
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )    \
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )    \
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )    \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )    \
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )    \
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )    \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )    \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )    \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )    \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )    \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )    \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )    \
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )    \
	PORT_DIPSETTING(    0x00, STRING_A )            \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION(#DIPBANK":5,6,7,8")    \
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )    \
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )    \
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    \
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )    \
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )    \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )    \
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )    \
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )    \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )    \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )    \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )    \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )    \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )    \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )    \
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )    \
	PORT_DIPSETTING(    0x00, STRING_B )

#define KONAMI_COINAGE( STRING_A, STRING_B )    \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   \
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )    \
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )    \
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )    \
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )    \
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )    \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )    \
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )    \
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )    \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )    \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )    \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )    \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )    \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )    \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )    \
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )    \
	PORT_DIPSETTING(    0x00, STRING_A )            \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   \
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )    \
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )    \
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    \
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )    \
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )    \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )    \
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )    \
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )    \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )    \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )    \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )    \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )    \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )    \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )    \
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )    \
	PORT_DIPSETTING(    0x00, STRING_B )

#define KONAMI_COINAGE_ALT_LOC( DIPBANK )   \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION(#DIPBANK":1,2,3,4")    \
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )    \
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )    \
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )    \
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )    \
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )    \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )    \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )    \
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )    \
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )    \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )    \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )    \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )    \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )    \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )    \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )    \
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )    \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION(#DIPBANK":5,6,7,8")    \
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )    \
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )    \
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    \
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )    \
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )    \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )    \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )    \
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )    \
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )    \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )    \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )    \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )    \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )    \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )    \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )    \
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )

#define KONAMI_COINAGE_ALT  \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   \
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )    \
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )    \
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )    \
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )    \
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )    \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )    \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )    \
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )    \
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )    \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )    \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )    \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )    \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )    \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )    \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )    \
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )    \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   \
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )    \
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )    \
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    \
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )    \
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )    \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )    \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )    \
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )    \
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )    \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )    \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )    \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )    \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )    \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )    \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )    \
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
