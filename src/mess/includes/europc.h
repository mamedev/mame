/*****************************************************************************
 *
 * includes/europc.h
 *
 ****************************************************************************/

#ifndef EUROPC_H_
#define EUROPC_H_


/*
layout of an uk europc

ESC, [SPACE], F1,F2,F3,F4,[SPACE],F5,F6,F7,F8,[SPACE],F9,F0,F11,F12
[SPACE]
\|, 1,2,3,4,5,6,7,8,9,0 -,+, BACKSPACE,[SPACE], NUM LOCK, SCROLL LOCK, PRINT SCREEN, KEYPAD -
TAB,Q,W,E,R,T,Y,U,I,O,P,[,], RETURN, [SPACE], KEYPAD 7, KEYPAD 8, KEYPAD 9, KEYPAD +
CTRL, A,S,D,F,G,H,J,K,L,;,@,~, RETURN, [SPACE],KEYPAD 4,KEYPAD 5,KEYPAD 6, KEYPAD +
LEFT SHIFT, Z,X,C,V,B,N,M,<,>,?,RIGHT SHIFT,[SPACE],KEYPAD 1, KEYPAD 2, KEYPAD 3, KEYPAD ENTER
ALT,[SPACE], SPACE BAR,[SPACE],CAPS LOCK,[SPACE], KEYPAD 0, KEYPAD ., KEYPAD ENTER

\ and ~ had to be swapped
i am not sure if keypad enter delivers the mf2 keycode
 */
#define EUROPC_KEYBOARD \
    PORT_START("pc_keyboard_0")  /* IN4 */\
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED ) 	/* unused scancode 0 */\
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) /* Esc                         01  81 */\
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) /* 1                           02  82 */\
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) /* 2                           03  83 */\
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) /* 3                           04  84 */\
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) /* 4                           05  85 */\
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) /* 5                           06  86 */\
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) /* 6                           07  87 */\
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) /* 7                           08  88 */\
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) /* 8                           09  89 */\
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) /* 9                           0A  8A */\
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) /* 0                           0B  8B */\
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) /* -                           0C  8C */\
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) /* =                           0D  8D */\
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<--") PORT_CODE(KEYCODE_BACKSPACE) /* Backspace                   0E  8E */\
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) /* Tab                         0F  8F */\
		\
	PORT_START("pc_keyboard_1")	/* IN5 */\
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) /* Q                           10  90 */\
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) /* W                           11  91 */\
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) /* E                           12  92 */\
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) /* R                           13  93 */\
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) /* T                           14  94 */\
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) /* Y                           15  95 */\
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) /* U                           16  96 */\
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) /* I                           17  97 */\
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) /* O                           18  98 */\
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) /* P                           19  99 */\
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) /* [                           1A  9A */\
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) /* ]                           1B  9B */\
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) /* Enter                       1C  9C */\
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) /* Ctrl                   1D  9D */\
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) /* A                           1E  9E */\
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) /* S                           1F  9F */\
		\
	PORT_START("pc_keyboard_2")	/* IN6 */\
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) /* D                           20  A0 */\
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) /* F                           21  A1 */\
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) /* G                           22  A2 */\
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) /* H                           23  A3 */\
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) /* J                           24  A4 */\
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) /* K                           25  A5 */\
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) /* L                           26  A6 */\
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) /* ;                           27  A7 */\
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) /* '                           28  A8 */\
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_BACKSLASH) /* `                           29  A9 */\
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Shift") PORT_CODE(KEYCODE_LSHIFT) /* Left Shift                  2A  AA */\
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_TILDE) /* \                           2B  AB */\
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) /* Z                           2C  AC */\
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) /* X                           2D  AD */\
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) /* C                           2E  AE */\
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) /* V                           2F  AF */\
		\
	PORT_START("pc_keyboard_3")	/* IN7 */\
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) /* B                           30  B0 */\
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) /* N                           31  B1 */\
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) /* M                           32  B2 */\
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) /* ,                           33  B3 */\
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) /* .                           34  B4 */\
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) /* /                           35  B5 */\
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-Shift") PORT_CODE(KEYCODE_RSHIFT) /* Right Shift                 36  B6 */\
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PrtScr") PORT_CODE(KEYCODE_PRTSCR) /* Keypad *  (PrtSc)           37  B7 */\
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) /* Alt                    38  B8 */\
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) /* Space                       39  B9 */\
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) /* Caps Lock                   3A  BA */\
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) /* F1                          3B  BB */\
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) /* F2                          3C  BC */\
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) /* F3                          3D  BD */\
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) /* F4                          3E  BE */\
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) /* F5                          3F  BF */\
		\
	PORT_START("pc_keyboard_4")	/* IN8 */\
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)     /* F6                          40  C0 */\
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)     /* F7                          41  C1 */\
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)     /* F8                          42  C2 */\
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)     /* F9                          43  C3 */\
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10)     /* F10                         44  C4 */\
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK)     /* Num Lock                    45  C5 */\
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ScrLock") PORT_CODE(KEYCODE_SCRLOCK)     /* Scroll Lock                 46  C6 */\
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 (Home)") PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_HOME )  /* Keypad 7  (Home)            47  C7 */\
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 (Up)") PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_UP )    /* Keypad 8  (Up arrow)        48  C8 */\
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_PGUP)   /* Keypad 9  (PgUp)            49  C9 */\
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -") PORT_CODE(KEYCODE_MINUS_PAD)     /* Keypad -                    4A  CA */\
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 (Left)") PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_LEFT )  /* Keypad 4  (Left arrow)      4B  CB */\
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5") PORT_CODE(KEYCODE_5_PAD)     /* Keypad 5                    4C  CC */\
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6 (Right)") PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_RIGHT ) /* Keypad 6  (Right arrow)     4D  CD */\
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +") PORT_CODE(KEYCODE_PLUS_PAD)     /* Keypad +                    4E  CE */\
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_END )   /* Keypad 1  (End)             4F  CF */\
		\
	PORT_START("pc_keyboard_5")	/* IN9 */\
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 (Down)") PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_DOWN )   /* Keypad 2  (Down arrow)      50  D0 */\
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_PGDN )   /* Keypad 3  (PgDn)            51  D1 */\
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0 (Ins)") PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_INSERT ) /* Keypad 0  (Ins)             52  D2 */\
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP . (Del)") PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(KEYCODE_DEL )    /* Keypad .  (Del)             53  D3 */\
	PORT_BIT ( 0x0070, 0x0000, IPT_UNUSED )\
	/* 0x40 non us backslash 2 not available */\
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_F11)		/* F11                         57  D7 */\
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_F12)		/* F12                         58  D8 */\
	PORT_BIT ( 0xfe00, 0x0000, IPT_UNUSED )\
		\
	PORT_START("pc_keyboard_6")	/* IN10 */\
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP Enter") PORT_CODE(KEYCODE_ENTER_PAD)		/* PAD Enter                   60  e0 */\
	PORT_BIT ( 0xfffe, 0x0000, IPT_UNUSED )\
		\
	PORT_START("pc_keyboard_7")	/* IN11 */\
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )


/*----------- defined in machine/europc.c -----------*/

WRITE8_HANDLER( europc_pio_w );
 READ8_HANDLER( europc_pio_r );

extern WRITE8_HANDLER ( europc_jim_w );
extern  READ8_HANDLER ( europc_jim_r );
extern  READ8_HANDLER ( europc_jim2_r );

extern  READ8_HANDLER( europc_rtc_r );
extern WRITE8_HANDLER( europc_rtc_w );
extern NVRAM_HANDLER( europc_rtc );

void europc_rtc_set_time(running_machine &machine);
void europc_rtc_init(running_machine &machine);


#endif /* EUROPC_H_ */
