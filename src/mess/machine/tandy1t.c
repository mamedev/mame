#include "emu.h"

#include "machine/pckeybrd.h"

#include "machine/pcshare.h"
#include "includes/pc.h"
#include "machine/pit8253.h"
#include "includes/tandy1t.h"


/* tandy 1000 eeprom
  hx and later
  clock, and data out lines at 0x37c
  data in at 0x62 bit 4 (0x10)

  8x read 16 bit word at x
  30 cx 30 4x 16bit 00 write 16bit at x
*/
static struct
{
	int state;
	int clock;
	UINT8 oper;
	UINT16 data;
	struct
	{
		UINT8 low, high;
	} ee[0x40]; /* only 0 to 4 used in hx, addressing seems to allow this */
} eeprom={0};

NVRAM_HANDLER( tandy1000 )
{
	if (file==NULL)
	{
		// init only
	}
	else if (read_or_write)
	{
		file->write(eeprom.ee, sizeof(eeprom.ee));
	}
	else
	{
		file->read(eeprom.ee, sizeof(eeprom.ee));
	}
}

static int tandy1000_read_eeprom(void)
{
	if ((eeprom.state>=100)&&(eeprom.state<=199))
		return eeprom.data&0x8000;
	return 1;
}

static void tandy1000_write_eeprom(UINT8 data)
{
	if (!eeprom.clock && (data&4) )
	{
//              logerror("!!!tandy1000 eeprom %.2x %.2x\n",eeprom.state, data);
		switch (eeprom.state)
		{
		case 0:
			if ((data&3)==0) eeprom.state++;
			break;
		case 1:
			if ((data&3)==2) eeprom.state++;
			break;
		case 2:
			if ((data&3)==3) eeprom.state++;
			break;
		case 3:
			eeprom.oper=data&1;
			eeprom.state++;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			eeprom.oper=(eeprom.oper<<1)|(data&1);
			eeprom.state++;
			break;
		case 10:
			eeprom.oper=(eeprom.oper<<1)|(data&1);
			logerror("!!!tandy1000 eeprom %.2x\n",eeprom.oper);
			if ((eeprom.oper&0xc0)==0x80)
			{
				eeprom.state=100;
				eeprom.data=eeprom.ee[eeprom.oper&0x3f].low
					|(eeprom.ee[eeprom.oper&0x3f].high<<8);
				logerror("!!!tandy1000 eeprom read %.2x,%.4x\n",eeprom.oper,eeprom.data);
			}
			else if ((eeprom.oper&0xc0)==0x40)
			{
				eeprom.state=200;
			}
			else
				eeprom.state=0;
			break;

			/* read 16 bit */
		case 100:
			eeprom.state++;
			break;
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:
		case 112:
		case 113:
		case 114:
		case 115:
			eeprom.data<<=1;
			eeprom.state++;
			break;
		case 116:
			eeprom.data<<=1;
			eeprom.state=0;
			break;

			/* write 16 bit */
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:
		case 208:
		case 209:
		case 210:
		case 211:
		case 212:
		case 213:
		case 214:
			eeprom.data=(eeprom.data<<1)|(data&1);
			eeprom.state++;
			break;
		case 215:
			eeprom.data=(eeprom.data<<1)|(data&1);
			logerror("tandy1000 %.2x %.4x written\n",eeprom.oper,eeprom.data);
			eeprom.ee[eeprom.oper&0x3f].low=eeprom.data&0xff;
			eeprom.ee[eeprom.oper&0x3f].high=eeprom.data>>8;
			eeprom.state=0;
			break;
		}
	}
	eeprom.clock=data&4;
}

static struct
{
	UINT8 data[8];

	UINT8 bios_bank;    /* I/O port FFEAh */
} tandy={ {0}};

WRITE8_HANDLER ( pc_t1t_p37x_w )
{
//  DBG_LOG(2,"T1T_p37x_w",("%.5x #%d $%02x\n", space.device().safe_pc( ),offset, data));
	if (offset!=4)
		logerror("T1T_p37x_w %.5x #%d $%02x\n", space.device().safe_pc( ),offset, data);
	tandy.data[offset]=data;
	switch( offset )
	{
		case 4:
			tandy1000_write_eeprom(data);
			break;
	}
}

	READ8_HANDLER ( pc_t1t_p37x_r )
{
	int data = tandy.data[offset];
//  DBG_LOG(1,"T1T_p37x_r",("%.5x #%d $%02x\n", space.device().safe_pc( ), offset, data));
	return data;
}

/* this is for tandy1000hx
   hopefully this works for all x models
   must not be a ppi8255 chip
   (think custom chip)
   port c:
   bit 4 input eeprom data in
   bit 3 output turbo mode
*/
static struct
{
	UINT8 portb, portc;
} tandy_ppi= { 0 };

WRITE8_HANDLER ( tandy1000_pio_w )
{
	switch (offset)
	{
	case 1:
		tandy_ppi.portb = data;
		pit8253_gate2_w(space.machine().device("pit8253"), BIT(data, 0));
		pc_speaker_set_spkrdata( space.machine(), data & 0x02 );
		// sx enables keyboard from bit 3, others bit 6, hopefully theres no conflict
		pc_keyb_set_clock(data&0x48);
		if ( data & 0x80 )
		{
			pc_keyb_clear();
		}
		break;
	case 2:
		tandy_ppi.portc = data;
		if (data & 8)
			space.machine().device("maincpu")->set_clock_scale(1);
		else
			space.machine().device("maincpu")->set_clock_scale(4.77/8);
		break;
	}
}

	READ8_HANDLER(tandy1000_pio_r)
{
	int data=0xff;
	switch (offset)
	{
	case 0:
		data = pc_keyb_read();
		break;
	case 1:
		data=tandy_ppi.portb;
		break;
	case 2:
//      if (tandy1000hx) {
//      data=tandy_ppi.portc; // causes problems (setuphx)
		if (!tandy1000_read_eeprom()) data&=~0x10;
//  }
		break;
	}
	return data;
}


static void tandy1000_set_bios_bank( running_machine &machine )
{
	machine.root_device().membank( "bank11" )->set_base( machine.root_device().memregion("maincpu")->base() + ( tandy.bios_bank & 0x07 ) * 0x10000 );
}


MACHINE_RESET_MEMBER(pc_state,tandy1000rl)
{
	MACHINE_RESET_CALL_MEMBER( pc );
	tandy.bios_bank = 6;
	tandy1000_set_bios_bank( machine() );
}


READ8_HANDLER( tandy1000_bank_r )
{
	UINT8 data = 0xFF;

	logerror( "%s: tandy1000_bank_r: offset = %x\n", space.machine().describe_context(), offset );

	switch( offset )
	{
	case 0x00:  /* FFEA */
		data = tandy.bios_bank;
		break;
	}

	return data;
}


WRITE8_HANDLER( tandy1000_bank_w )
{
	logerror( "%s: tandy1000_bank_w: offset = %x, data = %02x\n", space.machine().describe_context(), offset, data );

	switch( offset )
	{
	case 0x00:  /* FFEA */
		tandy.bios_bank = data;
		tandy1000_set_bios_bank(space.machine());
		break;
	}
}



INPUT_PORTS_START( t1000_keyboard )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) /* Esc                         01  81 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) /* 1                           02  82 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) /* 2                           03  83 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) /* 3                           04  84 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) /* 4                           05  85 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) /* 5                           06  86 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) /* 6                           07  87 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) /* 7                           08  88 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) /* 8                           09  89 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) /* 9                           0A  8A */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) /* 0                           0B  8B */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) /* -                           0C  8C */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) /* =                           0D  8D */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<--") PORT_CODE(KEYCODE_BACKSPACE) /* Backspace                   0E  8E */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) /* Tab                         0F  8F */

	PORT_START("pc_keyboard_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) /* Q                           10  90 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) /* W                           11  91 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) /* E                           12  92 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) /* R                           13  93 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) /* T                           14  94 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) /* Y                           15  95 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) /* U                           16  96 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) /* I                           17  97 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) /* O                           18  98 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) /* P                           19  99 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) /* [                           1A  9A */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) /* ]                           1B  9B */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) /* Enter                       1C  9C */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Ctrl") PORT_CODE(KEYCODE_LCONTROL) /* Left Ctrl                   1D  9D */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) /* A                           1E  9E */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) /* S                           1F  9F */

	PORT_START("pc_keyboard_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) /* D                           20  A0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) /* F                           21  A1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) /* G                           22  A2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) /* H                           23  A3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) /* J                           24  A4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) /* K                           25  A5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) /* L                           26  A6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) /* ;                           27  A7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) /* '                           28  A8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP) /*                             29  A9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Shift") PORT_CODE(KEYCODE_LSHIFT) /* Left Shift                  2A  AA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT) /*                             2B  AB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) /* Z                           2C  AC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) /* X                           2D  AD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) /* C                           2E  AE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) /* V                           2F  AF */

	PORT_START("pc_keyboard_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) /* B                           30  B0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) /* N                           31  B1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) /* M                           32  B2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) /* ,                           33  B3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) /* .                           34  B4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) /* /                           35  B5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-Shift") PORT_CODE(KEYCODE_RSHIFT) /* Right Shift                 36  B6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Print") /*                             37  B7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) /* Left Alt                    38  B8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) /* Space                       39  B9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE /* Caps Lock                   3A  BA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) /* F1                          3B  BB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) /* F2                          3C  BC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) /* F3                          3D  BD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) /* F4                          3E  BE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) /* F5                          3F  BF */

	PORT_START("pc_keyboard_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) /* F6                          40  C0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) /* F7                          41  C1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) /* F8                          42  C2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) /* F9                          43  C3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) /* F10                         44  C4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK) PORT_TOGGLE /* Num Lock                    45  C5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hold") PORT_CODE(KEYCODE_SCRLOCK) /*                           46  C6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 \\") PORT_CODE(KEYCODE_7_PAD) /* Keypad 7                    47  C7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 ~") PORT_CODE(KEYCODE_8_PAD) /* Keypad 8                    48  C8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD) /* Keypad 9  (PgUp)            49  C9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN) /*                             4A  CA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 |") PORT_CODE(KEYCODE_4_PAD) /* Keypad 4                    4B  CB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5") PORT_CODE(KEYCODE_5_PAD) /* Keypad 5                    4C  CC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6") PORT_CODE(KEYCODE_6_PAD) /* Keypad 6                    4D  CD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT) /*                             4E  CE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD) /* Keypad 1  (End)             4F  CF */

	PORT_START("pc_keyboard_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 `") PORT_CODE(KEYCODE_2_PAD) /* Keypad 2                    50  D0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD) /* Keypad 3  (PgDn)            51  D1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0") PORT_CODE(KEYCODE_0_PAD) /* Keypad 0                    52  D2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP - (Del)") PORT_CODE(KEYCODE_MINUS_PAD) /* - Delete                    53  D3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_STOP) /* Break                       54  D4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ Insert") PORT_CODE(KEYCODE_PLUS_PAD) /* + Insert                    55  D5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_DEL_PAD) /* .                           56  D6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD) /* Enter                       57  D7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME) /* HOME                        58  D8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_F11) /* F11                         59  D9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_F12) /* F12                         5a  Da */

	PORT_START("pc_keyboard_6")
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )

	PORT_START("pc_keyboard_7")
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )
INPUT_PORTS_END
