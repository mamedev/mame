// license:BSD-3-Clause
// copyright-holders:Luca Elia, Vas Crabb
#ifndef MAME_IGS_MAHJONG_H
#define MAME_IGS_MAHJONG_H

#define IGS_MAHJONG_MATRIX_CONDITIONAL(port, mask, on)                                                                                     \
		PORT_START("KEY0")                                                                                                                 \
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )          PORT_CONDITION(port, mask, EQUALS,    on) /* 槓           (杠)        */ \
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )               PORT_CONDITION(port, mask, EQUALS,    on) /* 開始         (开始)      */ \
		PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )              PORT_CONDITION(port, mask, NOTEQUALS, on)                                \
		PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )                                                                                       \
		PORT_START("KEY1")                                                                                                                 \
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )        PORT_CONDITION(port, mask, EQUALS,    on) /* 聽           (听)        */ \
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )          PORT_CONDITION(port, mask, EQUALS,    on) /* 押注/押                  */ \
		PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )              PORT_CONDITION(port, mask, NOTEQUALS, on)                                \
		PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )                                                                                       \
		PORT_START("KEY2")                                                                                                                 \
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )          PORT_CONDITION(port, mask, EQUALS,    on) /* 吃                       */ \
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )          PORT_CONDITION(port, mask, EQUALS,    on) /* 胡                       */ \
		PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNKNOWN )              PORT_CONDITION(port, mask, NOTEQUALS, on)                                \
		PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )                                                                                       \
		PORT_START("KEY3")                                                                                                                 \
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )            PORT_CONDITION(port, mask, EQUALS,    on)                                \
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )          PORT_CONDITION(port, mask, EQUALS,    on) /* 碰                       */ \
		PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )              PORT_CONDITION(port, mask, NOTEQUALS, on)                                \
		PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )                                                                                       \
		PORT_START("KEY4")                                                                                                                 \
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )  PORT_CONDITION(port, mask, EQUALS,    on) /* 海底                     */ \
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )        PORT_CONDITION(port, mask, EQUALS,    on) /* 得分                     */ \
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )    PORT_CONDITION(port, mask, EQUALS,    on) /* 比倍/續玩    (比倍/续玩) */ \
		PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )              PORT_CONDITION(port, mask, NOTEQUALS, on)                                \
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )                                                                                       \
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )          PORT_CONDITION(port, mask, EQUALS,    on) /* 大/左                    */ \
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )        PORT_CONDITION(port, mask, EQUALS,    on) /* 小/右                    */ \
		PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )              PORT_CONDITION(port, mask, NOTEQUALS, on)                                \
		PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_EXTERN(igs_mahjong_matrix);

#endif // MAME_IGS_MAHJONG_H
