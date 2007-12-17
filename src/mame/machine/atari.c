/***************************************************************************

    Atari 400/800

    Machine driver

    Juergen Buchmueller, June 1998

***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "includes/atari.h"
#include "sound/pokey.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "video/gtia.h"

#define VERBOSE_POKEY	0
#define VERBOSE_SERIAL	0
#define VERBOSE_TIMERS	0

//#define LOG(x) if (errorlog) fprintf x

static int atari = 0;
#define ATARI_5200	0
#define ATARI_400	1
#define ATARI_800	2
#define ATARI_600XL 3
#define ATARI_800XL 4

#ifdef MESS
static int a800_cart_loaded = 0;
static int a800_cart_is_16k = 0;
#endif

static void a800xl_mmu(UINT8 new_mmu);
static void a600xl_mmu(UINT8 new_mmu);

static void pokey_reset(running_machine *machine);

void atari_interrupt_cb(int mask)
{

#if VERBOSE_POKEY
		if (mask & 0x80)
			logerror("atari interrupt_cb BREAK\n");
		if (mask & 0x40)
			logerror("atari interrupt_cb KBCOD\n");
#endif
#if VERBOSE_SERIAL
		if (mask & 0x20)
			logerror("atari interrupt_cb SERIN\n");
		if (mask & 0x10)
			logerror("atari interrupt_cb SEROR\n");
		if (mask & 0x08)
			logerror("atari interrupt_cb SEROC\n");
#endif
#if VERBOSE_TIMERS
		if (mask & 0x04)
			logerror("atari interrupt_cb TIMR4\n");
		if (mask & 0x02)
			logerror("atari interrupt_cb TIMR2\n");
		if (mask & 0x01)
			logerror("atari interrupt_cb TIMR1\n");
#endif

	cpunum_set_input_line(0, 0, HOLD_LINE);
}

/**************************************************************
 *
 * PIA interface
 *
 **************************************************************/

static READ8_HANDLER(atari_pia_pa_r)
{
	return atari_input_disabled() ? 0xFF : readinputportbytag_safe("djoy_0_1", 0);
}

static READ8_HANDLER(atari_pia_pb_r)
{
	return atari_input_disabled() ? 0xFF : readinputportbytag_safe("djoy_2_3", 0);
}

static WRITE8_HANDLER(a600xl_pia_pb_w) { a600xl_mmu(data); }
static WRITE8_HANDLER(a800xl_pia_pb_w) { a800xl_mmu(data); }

#ifdef MESS
extern WRITE8_HANDLER(atari_pia_cb2_w);
#else
#define atari_pia_cb2_w		(0)
#endif

static const pia6821_interface atari_pia_interface =
{
	/*inputs : A/B,CA/B1,CA/B2 */ atari_pia_pa_r, atari_pia_pb_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, atari_pia_cb2_w,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface a600xl_pia_interface =
{
	/*inputs : A/B,CA/B1,CA/B2 */ atari_pia_pa_r, atari_pia_pb_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, a600xl_pia_pb_w, 0, atari_pia_cb2_w,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface a800xl_pia_interface =
{
	/*inputs : A/B,CA/B1,CA/B2 */ atari_pia_pa_r, atari_pia_pb_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, a800xl_pia_pb_w, 0, atari_pia_cb2_w,
	/*irqs   : A/B             */ 0, 0
};




/**************************************************************
 *
 * Reset hardware
 *
 **************************************************************/

void a600xl_mmu(UINT8 new_mmu)
{
	read8_handler rbank2;
	write8_handler wbank2;

	/* check if self-test ROM changed */
	if ( new_mmu & 0x80 )
	{
		logerror("%s MMU SELFTEST RAM\n", Machine->gamedrv->name);
		rbank2 = MRA8_NOP;
		wbank2 = MWA8_NOP;
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", Machine->gamedrv->name);
		rbank2 = MRA8_BANK2;
		wbank2 = MWA8_ROM;
	}
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5000, 0x57ff, 0, 0, rbank2);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5000, 0x57ff, 0, 0, wbank2);
	if (rbank2 == MRA8_BANK2)
		memory_set_bankptr(2, memory_region(REGION_CPU1)+0x5000);
}

void a800xl_mmu(UINT8 new_mmu)
{
	read8_handler rbank1, rbank2, rbank3, rbank4;
	write8_handler wbank1, wbank2, wbank3, wbank4;
	UINT8 *base1, *base2, *base3, *base4;

	/* check if memory C000-FFFF changed */
	if( new_mmu & 0x01 )
	{
		logerror("%s MMU BIOS ROM\n", Machine->gamedrv->name);
		rbank3 = MRA8_BANK3;
		wbank3 = MWA8_ROM;
		base3 = memory_region(REGION_CPU1)+0x14000;  /* 8K lo BIOS */
		rbank4 = MRA8_BANK4;
		wbank4 = MWA8_ROM;
		base4 = memory_region(REGION_CPU1)+0x15800;  /* 4K FP ROM + 8K hi BIOS */
	}
	else
	{
		logerror("%s MMU BIOS RAM\n", Machine->gamedrv->name);
		rbank3 = MRA8_BANK3;
		wbank3 = MWA8_BANK3;
		base3 = memory_region(REGION_CPU1)+0x0c000;  /* 8K RAM */
		rbank4 = MRA8_BANK4;
		wbank4 = MWA8_BANK4;
		base4 = memory_region(REGION_CPU1)+0x0d800;  /* 4K RAM + 8K RAM */
	}
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xcfff, 0, 0, rbank3);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xcfff, 0, 0, wbank3);
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0xd800, 0xffff, 0, 0, rbank4);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xd800, 0xffff, 0, 0, wbank4);
	memory_set_bankptr(3, base3);
	memory_set_bankptr(4, base4);

	/* check if BASIC changed */
	if( new_mmu & 0x02 )
	{
		logerror("%s MMU BASIC RAM\n", Machine->gamedrv->name);
		rbank1 = MRA8_BANK1;
		wbank1 = MWA8_BANK1;
		base1 = memory_region(REGION_CPU1)+0x0a000;  /* 8K RAM */
	}
	else
	{
		logerror("%s MMU BASIC ROM\n", Machine->gamedrv->name);
		rbank1 = MRA8_BANK1;
		wbank1 = MWA8_ROM;
		base1 = memory_region(REGION_CPU1)+0x10000;  /* 8K BASIC */
	}
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa000, 0xbfff, 0, 0, rbank1);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa000, 0xbfff, 0, 0, wbank1);
	memory_set_bankptr(1, base1);

	/* check if self-test ROM changed */
	if( new_mmu & 0x80 )
	{
		logerror("%s MMU SELFTEST RAM\n", Machine->gamedrv->name);
		rbank2 = MRA8_BANK2;
		wbank2 = MWA8_BANK2;
		base2 = memory_region(REGION_CPU1)+0x05000;  /* 0x0800 bytes */
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", Machine->gamedrv->name);
		rbank2 = MRA8_BANK2;
		wbank2 = MWA8_ROM;
		base2 = memory_region(REGION_CPU1)+0x15000;  /* 0x0800 bytes */
	}
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5000, 0x57ff, 0, 0, rbank2);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5000, 0x57ff, 0, 0, wbank2);
	memory_set_bankptr(2, base2);
}



/**************************************************************
 *
 * Keyboard
 *
 **************************************************************/

#define AKEY_L			0x00
#define AKEY_J			0x01
#define AKEY_SEMICOLON	0x02
#define AKEY_BREAK		0x03	/* this not really a scancode */
#define AKEY_K			0x05
#define AKEY_PLUS		0x06
#define AKEY_ASTERISK	0x07
#define AKEY_O			0x08
#define AKEY_NONE		0x09
#define AKEY_P			0x0a
#define AKEY_U			0x0b
#define AKEY_ENTER		0x0c
#define AKEY_I			0x0d
#define AKEY_MINUS		0x0e
#define AKEY_EQUALS 	0x0f
#define AKEY_V			0x10
#define AKEY_C			0x12
#define AKEY_B			0x15
#define AKEY_X			0x16
#define AKEY_Z			0x17
#define AKEY_4			0x18
#define AKEY_3			0x1a
#define AKEY_6			0x1b
#define AKEY_ESC		0x1c
#define AKEY_5			0x1d
#define AKEY_2			0x1e
#define AKEY_1			0x1f
#define AKEY_COMMA		0x20
#define AKEY_SPACE		0x21
#define AKEY_STOP		0x22
#define AKEY_N			0x23
#define AKEY_M			0x25
#define AKEY_SLASH		0x26
#define AKEY_ATARI		0x27
#define AKEY_R			0x28
#define AKEY_E			0x2a
#define AKEY_Y			0x2b
#define AKEY_TAB		0x2c
#define AKEY_T			0x2d
#define AKEY_W			0x2e
#define AKEY_Q			0x2f
#define AKEY_9			0x30
#define AKEY_0			0x32
#define AKEY_7			0x33
#define AKEY_BSP		0x34
#define AKEY_8			0x35
#define AKEY_LESSER 	0x36
#define AKEY_GREATER	0x37
#define AKEY_F			0x38
#define AKEY_H			0x39
#define AKEY_D			0x3a
#define AKEY_CAPS		0x3c
#define AKEY_G			0x3d
#define AKEY_S			0x3e
#define AKEY_A			0x3f
#define ASHF_L			0x40
#define ASHF_J			0x41
#define ASHF_COLON		0x42
#define ASHF_BREAK		0x43	/* this not really a scancode */
#define ASHF_K			0x45
#define ASHF_BACKSLASH	0x46
#define ASHF_TILDE		0x47
#define ASHF_O			0x48
#define ASHF_SHIFT		0x49
#define ASHF_P			0x4a
#define ASHF_U			0x4b
#define ASHF_ENTER		0x4c
#define ASHF_I			0x4d
#define ASHF_UNDERSCORE 0x4e
#define ASHF_BAR		0x4f
#define ASHF_V			0x50
#define ASHF_C			0x52
#define ASHF_B			0x55
#define ASHF_X			0x56
#define ASHF_Z			0x57
#define ASHF_DOLLAR 	0x58
#define ASHF_HASH		0x5a
#define ASHF_AMPERSAND	0x5b
#define ASHF_ESC		0x5c
#define ASHF_PERCENT	0x5d
#define ASHF_DQUOTE 	0x5e
#define ASHF_EXCLAM 	0x5f
#define ASHF_LBRACE 	0x60
#define ASHF_SPACE		0x61
#define ASHF_RBRACE 	0x62
#define ASHF_N			0x63
#define ASHF_M			0x65
#define ASHF_QUESTION	0x66
#define ASHF_ATARI		0x67
#define ASHF_R			0x68
#define ASHF_E			0x6a
#define ASHF_Y			0x6b
#define ASHF_TAB		0x6c
#define ASHF_T			0x6d
#define ASHF_W			0x6e
#define ASHF_Q			0x6f
#define ASHF_LPAREN 	0x70
#define ASHF_RPAREN 	0x72
#define ASHF_QUOTE		0x73
#define ASHF_BSP		0x74
#define ASHF_AT 		0x75
#define ASHF_CLEAR		0x76
#define ASHF_INSERT 	0x77
#define ASHF_F			0x78
#define ASHF_H			0x79
#define ASHF_D			0x7a
#define ASHF_CAPS		0x7c
#define ASHF_G			0x7d
#define ASHF_S			0x7e
#define ASHF_A			0x7f
#define ACTL_L			0x80
#define ACTL_J			0x81
#define ACTL_SEMICOLON	0x82
#define ACTL_BREAK		0x83	/* this not really a scancode */
#define ACTL_K			0x85
#define ACTL_PLUS		0x86
#define ACTL_ASTERISK	0x87
#define ACTL_O			0x88
#define ACTL_CONTROL	0x89
#define ACTL_P			0x8a
#define ACTL_U			0x8b
#define ACTL_ENTER		0x8c
#define ACTL_I			0x8d
#define ACTL_MINUS		0x8e
#define ACTL_EQUALS 	0x8f
#define ACTL_V			0x90
#define ACTL_C			0x92
#define ACTL_B			0x95
#define ACTL_X			0x96
#define ACTL_Z			0x97
#define ACTL_4			0x98
#define ACTL_3			0x9a
#define ACTL_6			0x9b
#define ACTL_ESC		0x9c
#define ACTL_5			0x9d
#define ACTL_2			0x9e
#define ACTL_1			0x9f
#define ACTL_COMMA		0xa0
#define ACTL_SPACE		0xa1
#define ACTL_STOP		0xa2
#define ACTL_N			0xa3
#define ACTL_M			0xa5
#define ACTL_SLASH		0xa6
#define ACTL_ATARI		0xa7
#define ACTL_R			0xa8
#define ACTL_E			0xaa
#define ACTL_Y			0xab
#define ACTL_TAB		0xac
#define ACTL_T			0xad
#define ACTL_W			0xae
#define ACTL_Q			0xaf
#define ACTL_9			0xb0
#define ACTL_0			0xb2
#define ACTL_7			0xb3
#define ACTL_BSP		0xb4
#define ACTL_8			0xb5
#define ACTL_LESSER 	0xb6
#define ACTL_GREATER	0xb7
#define ACTL_F			0xb8
#define ACTL_H			0xb9
#define ACTL_D			0xba
#define ACTL_CAPS		0xbc
#define ACTL_G			0xbd
#define ACTL_S			0xbe
#define ACTL_A			0xbf
#define ACSH_L			0xc0
#define ACSH_J			0xc1
#define ACSH_COLON		0xc2
#define ACSH_BREAK		0xc3	/* this not really a scancode */
#define ACSH_K			0xc5
#define ACSH_BACKSLASH	0xc6
#define ACSH_TILDE		0xc7
#define ACSH_O			0xc8
#define ACSH_CTRLSHIFT	0xc9
#define ACSH_P			0xca
#define ACSH_U			0xcb
#define ACSH_ENTER		0xcc
#define ACSH_I			0xcd
#define ACSH_UNDERSCORE 0xce
#define ACSH_BAR		0xcf
#define ACSH_V			0xd0
#define ACSH_C			0xd2
#define ACSH_B			0xd5
#define ACSH_X			0xd6
#define ACSH_Z			0xd7
#define ACSH_DOLLAR 	0xd8
#define ACSH_HASH		0xda
#define ACSH_AMPERSAND	0xdb
#define ACSH_ESC		0xdc
#define ACSH_PERCENT	0xdd
#define ACSH_DQUOTE 	0xde
#define ACSH_EXCLAM 	0xdf
#define ACSH_LBRACE 	0xe0
#define ACSH_SPACE		0xe1
#define ACSH_RBRACE 	0xe2
#define ACSH_N			0xe3
#define ACSH_M			0xe5
#define ACSH_QUESTION	0xe6
#define ACSH_ATARI		0xe7
#define ACSH_R			0xe8
#define ACSH_E			0xea
#define ACSH_Y			0xeb
#define ACSH_TAB		0xec
#define ACSH_T			0xed
#define ACSH_W			0xee
#define ACSH_Q			0xef
#define ACSH_LPAREN 	0xf0
#define ACSH_RPAREN 	0xf2
#define ACSH_QUOTE		0xf3
#define ACSH_BSP		0xf4
#define ACSH_AT 		0xf5
#define ACSH_CLEAR		0xf6
#define ACSH_INSERT 	0xf7
#define ACSH_F			0xf8
#define ACSH_H			0xf9
#define ACSH_D			0xfa
#define ACSH_CAPS		0xfc
#define ACSH_G			0xfd
#define ACSH_S			0xfe
#define ACSH_A			0xff

static const UINT8 keys[64][4] = {
{AKEY_NONE		 ,AKEY_NONE 	  ,AKEY_NONE	   ,AKEY_NONE		}, /* ""       CODE_NONE             */
{AKEY_ESC		 ,ASHF_ESC		  ,ACTL_ESC 	   ,ACSH_ESC		}, /*"Escape" KEYCODE_ESC              */
{AKEY_1 		 ,ASHF_EXCLAM	  ,ACTL_1		   ,ACSH_EXCLAM 	}, /* "1 !"    KEYCODE_1               */
{AKEY_2 		 ,ASHF_DQUOTE	  ,ACTL_2		   ,ACSH_DQUOTE 	}, /* "2 \""   KEYCODE_2               */
{AKEY_3 		 ,ASHF_HASH 	  ,ACTL_3		   ,ACSH_HASH		}, /* "3 #"    KEYCODE_3               */
{AKEY_4 		 ,ASHF_DOLLAR	  ,ACTL_4		   ,ACSH_DOLLAR 	}, /* "4 $"    KEYCODE_4               */
{AKEY_5 		 ,ASHF_PERCENT	  ,ACTL_5		   ,ACSH_PERCENT	}, /* "5 %"    KEYCODE_5               */
{AKEY_6 		 ,ASHF_TILDE	  ,ACTL_6		   ,ACSH_AMPERSAND	}, /* "6 ^"    KEYCODE_6               */
{AKEY_7 		 ,ASHF_AMPERSAND  ,ACTL_7		   ,ACSH_N			}, /* "7 &"    KEYCODE_7               */
{AKEY_8 		 ,AKEY_ASTERISK   ,ACTL_8		   ,ACSH_M			}, /* "8 *"    KEYCODE_8               */
{AKEY_9 		 ,ASHF_LPAREN	  ,ACTL_9		   ,ACSH_LBRACE 	}, /* "9 ("    KEYCODE_9               */
{AKEY_0 		 ,ASHF_RPAREN	  ,ACTL_0		   ,ACSH_RBRACE 	}, /* "0 )"    KEYCODE_0               */
{AKEY_MINUS 	 ,ASHF_UNDERSCORE ,ACTL_MINUS	   ,ACSH_UNDERSCORE }, /* "- _"    KEYCODE_MINUS           */
{AKEY_EQUALS	 ,AKEY_PLUS 	  ,ACTL_EQUALS	   ,ACTL_PLUS		}, /* "= +"    KEYCODE_EQUALS          */
{AKEY_BSP		 ,ASHF_BSP		  ,ACTL_BSP 	   ,ACSH_BSP		}, /* "Backsp" KEYCODE_BACKSPACE       */
{AKEY_TAB		 ,ASHF_TAB		  ,ACTL_TAB 	   ,ACSH_TAB		}, /* "Tab"    KEYCODE_TAB             */
{AKEY_Q 		 ,ASHF_Q		  ,ACTL_Q		   ,ACSH_Q			}, /* "q Q"    KEYCODE_Q               */
{AKEY_W 		 ,ASHF_W		  ,ACTL_W		   ,ACSH_W			}, /* "w W"    KEYCODE_W               */
{AKEY_E 		 ,ASHF_E		  ,ACTL_E		   ,ACSH_E			}, /* "e E"    KEYCODE_E               */
{AKEY_R 		 ,ASHF_R		  ,ACTL_R		   ,ACSH_R			}, /* "r R"    KEYCODE_R               */
{AKEY_T 		 ,ASHF_T		  ,ACTL_T		   ,ACSH_T			}, /* "t T"    KEYCODE_T               */
{AKEY_Y 		 ,ASHF_Y		  ,ACTL_Y		   ,ACSH_Y			}, /* "y Y"    KEYCODE_Y               */
{AKEY_U 		 ,ASHF_U		  ,ACTL_U		   ,ACTL_U			}, /* "u U"    KEYCODE_U               */
{AKEY_I 		 ,ASHF_I		  ,ACTL_I		   ,ACTL_I			}, /* "i I"    KEYCODE_I               */
{AKEY_O 		 ,ASHF_O		  ,ACTL_O		   ,ACTL_O			}, /* "o O"    KEYCODE_O               */
{AKEY_P 		 ,ASHF_P		  ,ACTL_P		   ,ACTL_P			}, /* "p P"    KEYCODE_P               */
{ASHF_LBRACE	 ,ACTL_COMMA	  ,ACTL_COMMA	   ,ACSH_LBRACE 	}, /* "[ {"    KEYCODE_LBRACE          */
{ASHF_RBRACE	 ,ACTL_STOP 	  ,ACTL_STOP	   ,ACSH_RBRACE 	}, /* "] }"    KEYCODE_RBRACE          */
{AKEY_ENTER 	 ,ASHF_ENTER	  ,ACTL_ENTER	   ,ACSH_ENTER		}, /* "Enter"  KEYCODE_ENTER           */
{AKEY_A 		 ,ASHF_A		  ,ACTL_A		   ,ACSH_A			}, /* "a A"    KEYCODE_A               */
{AKEY_S 		 ,ASHF_S		  ,ACTL_S		   ,ACSH_S			}, /* "s S"    KEYCODE_S               */
{AKEY_D 		 ,ASHF_D		  ,ACTL_D		   ,ACSH_D			}, /* "d D"    KEYCODE_D               */
{AKEY_F 		 ,ASHF_F		  ,ACTL_F		   ,ACSH_F			}, /* "f F"    KEYCODE_F               */
{AKEY_G 		 ,ASHF_G		  ,ACTL_G		   ,ACSH_G			}, /* "g G"    KEYCODE_G               */
{AKEY_H 		 ,ASHF_H		  ,ACTL_H		   ,ACSH_H			}, /* "h H"    KEYCODE_H               */
{AKEY_J 		 ,ASHF_J		  ,ACTL_J		   ,ACSH_J			}, /* "j J"    KEYCODE_J               */
{AKEY_K 		 ,ASHF_K		  ,ACTL_K		   ,ACSH_K			}, /* "k K"    KEYCODE_K               */
{AKEY_L 		 ,ASHF_L		  ,ACTL_L		   ,ACSH_L			}, /* "l L"    KEYCODE_L               */
{AKEY_SEMICOLON  ,ASHF_COLON	  ,ACTL_SEMICOLON  ,ACSH_COLON		}, /* "; :"    KEYCODE_COLON           */
{ASHF_QUOTE 	 ,ACSH_QUOTE	  ,ASHF_DQUOTE	   ,ACSH_DQUOTE 	}, /* "+ \\"   KEYCODE_QUOTE           */
{ASHF_QUOTE 	 ,ACSH_QUOTE	  ,ACTL_ASTERISK   ,ACSH_TILDE		}, /* "* ^"    KEYCODE_TILDE           */
{ASHF_BACKSLASH  ,ASHF_BAR		  ,ACSH_BACKSLASH  ,ACSH_BAR		}, /* "\ |"    KEYCODE_BACKSLASH       */
{AKEY_Z 		 ,ASHF_Z		  ,ACTL_Z		   ,ACSH_Z			}, /* "z Z"    KEYCODE_Z               */
{AKEY_X 		 ,ASHF_X		  ,ACTL_X		   ,ACTL_X			}, /* "x X"    KEYCODE_X               */
{AKEY_C 		 ,ASHF_C		  ,ACTL_C		   ,ACTL_C			}, /* "c C"    KEYCODE_C               */
{AKEY_V 		 ,ASHF_V		  ,ACTL_V		   ,ACTL_V			}, /* "v V"    KEYCODE_V               */
{AKEY_B 		 ,ASHF_B		  ,ACTL_B		   ,ACTL_B			}, /* "b B"    KEYCODE_B               */
{AKEY_N 		 ,ASHF_N		  ,ACTL_N		   ,ACTL_N			}, /* "n N"    KEYCODE_N               */
{AKEY_M 		 ,ASHF_M		  ,ACTL_M		   ,ACTL_M			}, /* "m M"    KEYCODE_M               */
{AKEY_COMMA 	 ,AKEY_LESSER	  ,ACTL_COMMA	   ,ACTL_LESSER 	}, /* ", ["    KEYCODE_COMMA           */
{AKEY_STOP		 ,AKEY_GREATER	  ,ACTL_STOP	   ,ACTL_GREATER	}, /* ". ]"    KEYCODE_STOP            */
{AKEY_SLASH 	 ,ASHF_QUESTION   ,ACTL_SLASH	   ,ACSH_QUESTION	}, /* "/ ?"    KEYCODE_SLASH           */
{ASHF_BACKSLASH  ,ASHF_BAR		  ,ACSH_BACKSLASH  ,ACSH_BAR		}, /* "\ |"    KEYCODE_BACKSLASH2      */
{AKEY_ATARI 	 ,ASHF_ATARI	  ,ACTL_ATARI	   ,ACSH_ATARI		}, /* "Atari"  KEYCODE_LALT            */
{AKEY_SPACE 	 ,ASHF_SPACE	  ,ACTL_SPACE	   ,ACSH_SPACE		}, /* "Space"  KEYCODE_SPACE           */
{AKEY_CAPS		 ,ASHF_CAPS 	  ,ACTL_CAPS	   ,ACSH_CAPS		}, /* "Caps"   KEYCODE_CAPSLOCK        */
{ASHF_CLEAR 	 ,ASHF_CLEAR	  ,ACSH_CLEAR	   ,ACSH_CLEAR		}, /* "Clear"  KEYCODE_HOME            */
{ASHF_INSERT	 ,ASHF_INSERT	  ,ASHF_INSERT	   ,ASHF_INSERT 	}, /* "Insert" KEYCODE_INSERT          */
{AKEY_BSP		 ,AKEY_BSP		  ,AKEY_BSP 	   ,AKEY_BSP		}, /* "Delete" KEYCODE_DEL             */
{AKEY_BREAK 	 ,ASHF_BREAK	  ,ACTL_BREAK	   ,ACSH_BREAK		}, /* "Break"  KEYCODE_PGUP            */
{ACTL_PLUS		 ,ACTL_PLUS 	  ,ACTL_PLUS	   ,ACTL_PLUS		}, /* "(Left)" KEYCODE_LEFT            */
{ACTL_ASTERISK	 ,ACTL_ASTERISK   ,ACTL_ASTERISK   ,ACTL_ASTERISK	}, /* "(Right)"KEYCODE_RIGHT           */
{ACTL_MINUS 	 ,ACTL_MINUS	  ,ACTL_MINUS	   ,ACTL_MINUS		}, /* "(Up)"   KEYCODE_UP              */
{ACTL_EQUALS	 ,ACTL_EQUALS	  ,ACTL_EQUALS	   ,ACTL_EQUALS 	}  /* "(Down)" KEYCODE_DOWN            */
};


void a800_handle_keyboard(void)
{
	static int atari_last = 0xff;
	int i, modifiers, atari_code;
	char tag[64];

    modifiers = 0;

    /* with shift ? */
	if( input_code_pressed(KEYCODE_LSHIFT) || input_code_pressed(KEYCODE_RSHIFT) )
		modifiers |= 1;

    /* with control ? */
	if( input_code_pressed(KEYCODE_LCONTROL) || input_code_pressed(KEYCODE_RCONTROL) )
		modifiers |= 2;

	for( i = 0; i < 64; i++ )
	{
		sprintf(tag, "keyboard_%d", i / 16);
		if( readinputportbytag_safe(tag, 0) & (1 << (i&15)) )
		{
			atari_code = keys[i][modifiers];
			if( atari_code != AKEY_NONE )
			{
				if( atari_code == atari_last )
					return;
				atari_last = atari_code;
				if( (atari_code & 0x3f) == AKEY_BREAK )
				{
					pokey1_break_w(atari_code & 0x40);
					return;
				}
				pokey1_kbcode_w(atari_code, 1);
				return;
			}
		}
	}
	/* remove key pressed status bit from skstat */
	pokey1_kbcode_w(AKEY_NONE, 0);
	atari_last = AKEY_NONE;
}

#define VKEY_BREAK		0x10

/* absolutely no clue what to do here :((( */
void a5200_handle_keypads(void)
{
	int i, modifiers;
	static int atari_last = 0xff;

    modifiers = 0;

    /* with shift ? */
	if (input_code_pressed(KEYCODE_LSHIFT) || input_code_pressed(KEYCODE_RSHIFT))
		modifiers |= 1;

    /* with control ? */
	if (input_code_pressed(KEYCODE_LCONTROL) || input_code_pressed(KEYCODE_RCONTROL))
		modifiers |= 2;

	/* check keypad */
	for (i = 0; i < 16; i++)
	{
		if( readinputportbytag("keypad") & (1 << i) )
		{
			if( i == atari_last )
				return;
			atari_last = i;
			if( i == 0 )
			{
				pokey1_break_w(i & 0x40);
				return;
			}
			pokey1_kbcode_w((i << 1) | 0x21, 1);
			return;
		}
	}

	/* check top button */
	if ((readinputportbytag("djoy_b") & 0x10) == 0)
	{
		if (atari_last == 0xFE)
			return;
		pokey1_kbcode_w(0x61, 1);
		//pokey1_break_w(0x40);
		atari_last = 0xFE;
		return;
	}
	else if (atari_last == 0xFE)
		pokey1_kbcode_w(0x21, 1);

	/* remove key pressed status bit from skstat */
	pokey1_kbcode_w(0xFF, 0);
	atari_last = 0xff;
}

#ifdef MESS
DRIVER_INIT( atari )
{
	offs_t ram_top;
	offs_t ram_size;

	if (!strcmp(machine->gamedrv->name, "a400")
		|| !strcmp(machine->gamedrv->name, "a400pal")
		|| !strcmp(machine->gamedrv->name, "a800")
		|| !strcmp(machine->gamedrv->name, "a800pal")
		|| !strcmp(machine->gamedrv->name, "a800xl"))
	{
		ram_size = 0xA000;
	}
	else
	{
		ram_size = 0x8000;
	}

	/* install RAM */
	ram_top = MIN(mess_ram_size, ram_size) - 1;
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM,
		0x0000, ram_top, 0, 0, MRA8_BANK2);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM,
		0x0000, ram_top, 0, 0, MWA8_BANK2);
	memory_set_bankptr(2, mess_ram);
}
#endif



/*************************************
 *
 *  Generic Atari Code
 *
 *************************************/

#ifdef MESS
static void a800_setbank(int n)
{
	void *read_addr;
	void *write_addr;
	UINT8 *mem = memory_region(REGION_CPU1);

	switch (n)
	{
		case 1:
			read_addr = &mem[0x10000];
			write_addr = NULL;
			break;
		default:
			if( atari <= ATARI_400 )
			{
				/* Atari 400 has no RAM here, so install the NOP handler */
				read_addr = NULL;
				write_addr = NULL;
			}
			else
			{
				read_addr = &mess_ram[0x08000];
				write_addr = &mess_ram[0x08000];
			}
			break;
	}

	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xbfff, 0, 0,
		read_addr ? MRA8_BANK1 : MRA8_NOP);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xbfff, 0, 0,
		write_addr ? MWA8_BANK1 : MWA8_NOP);
	if (read_addr)
		memory_set_bankptr(1, read_addr);
	if (write_addr)
		memory_set_bankptr(1, write_addr);
}
#endif



static void pokey_reset(running_machine *machine)
{
	pokey1_w(15,0);
}



static void cart_reset(running_machine *machine)
{
#ifdef MESS
	if (a800_cart_loaded)
		a800_setbank(1);
#endif /* MESS */
}



static UINT8 console_read(void)
{
	return readinputportbytag("console");
}



static void console_write(UINT8 data)
{
	if (data & 0x08)
		DAC_data_w(0, -120);
	else
		DAC_data_w(0, +120);
}


static void _pia_reset(running_machine *machine)
{
	pia_reset();
}

static void _antic_reset(running_machine *machine)
{
	antic_reset();
}


static void atari_machine_start(int type, const pia6821_interface *pia_intf, int has_cart)
{
	gtia_interface gtia_intf;

	atari = type;

	/* GTIA */
	memset(&gtia_intf, 0, sizeof(gtia_intf));
	if (port_tag_to_index("console") >= 0)
		gtia_intf.console_read = console_read;
	if (sndti_exists(SOUND_DAC, 0))
		gtia_intf.console_write = console_write;
	gtia_init(&gtia_intf);

	/* pokey */
	add_reset_callback(Machine, pokey_reset);

	/* PIA */
	if (pia_intf)
	{
		pia_config(0, pia_intf);
		add_reset_callback(Machine, _pia_reset);
	}

	/* ANTIC */
	add_reset_callback(Machine, _antic_reset);

	/* cartridge */
	if (has_cart)
		add_reset_callback(Machine, cart_reset);

#ifdef MESS
	{
		offs_t ram_top;
		offs_t ram_size;

		if (!strcmp(Machine->gamedrv->name, "a400")
			|| !strcmp(Machine->gamedrv->name, "a400pal")
			|| !strcmp(Machine->gamedrv->name, "a800")
			|| !strcmp(Machine->gamedrv->name, "a800pal")
			|| !strcmp(Machine->gamedrv->name, "a800xl"))
		{
			ram_size = 0xA000;
		}
		else
		{
			ram_size = 0x8000;
		}

		/* install RAM */
		ram_top = MIN(mess_ram_size, ram_size) - 1;
		memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM,
			0x0000, ram_top, 0, 0, MRA8_BANK2);
		memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM,
			0x0000, ram_top, 0, 0, MWA8_BANK2);
		memory_set_bankptr(2, mess_ram);
	}
#endif /* MESS */

	/* save states */
	state_save_register_global_pointer(((UINT8 *) &antic.r), sizeof(antic.r));
	state_save_register_global_pointer(((UINT8 *) &antic.w), sizeof(antic.w));
}



/*************************************
 *
 *  Atari 400
 *  Atari 600XL
 *
 *************************************/

MACHINE_START( a400 )
{
	atari_machine_start(ATARI_400, &atari_pia_interface, TRUE);
}

MACHINE_START( a600xl )
{
	atari_machine_start(ATARI_600XL, &a600xl_pia_interface, TRUE);
}



/*************************************
 *
 *  Atari 800
 *
 *************************************/

MACHINE_START( a800 )
{
	atari_machine_start(ATARI_800, &atari_pia_interface, TRUE);
}



#ifdef MESS
DEVICE_LOAD( a800_cart )
{
	UINT8 *mem = memory_region(REGION_CPU1);
	int size;

	/* load an optional (dual) cartridge (e.g. basic.rom) */
	if( image_index_in_device(image) > 0 )
	{
		size = image_fread(image, &mem[0x12000], 0x2000);
		a800_cart_is_16k = (size == 0x2000);
		logerror("%s loaded right cartridge '%s' size 16K\n", Machine->gamedrv->name, image_filename(image) );
	}
	else
	{
		size = image_fread(image, &mem[0x10000], 0x2000);
		a800_cart_loaded = size > 0x0000;
		size = image_fread(image, &mem[0x12000], 0x2000);
		a800_cart_is_16k = size > 0x2000;
		logerror("%s loaded left cartridge '%s' size %s\n", Machine->gamedrv->name, image_filename(image) , (a800_cart_is_16k) ? "16K":"8K");
	}
	return INIT_PASS;
}

DEVICE_UNLOAD( a800_cart )
{
	if( image_index_in_device(image) > 0 )
	{
		a800_cart_is_16k = 0;
		a800_setbank(1);
    }
	else
	{
		a800_cart_loaded = 0;
		a800_setbank(0);
    }
}
#endif



/*************************************
 *
 *  Atari 800XL
 *
 *************************************/

MACHINE_START( a800xl )
{
	atari_machine_start(ATARI_800XL, &a800xl_pia_interface, TRUE);
}



#ifdef MESS
DEVICE_LOAD( a800xl_cart )
{
	UINT8 *mem = memory_region(REGION_CPU1);
	astring *fname;
	mame_file *basic_fp;
	file_error filerr;
	unsigned size;

	fname = astring_assemble_3(astring_alloc(), Machine->gamedrv->name, PATH_SEPARATOR, "basic.rom");
	filerr = mame_fopen(SEARCHPATH_ROM, astring_c(fname), OPEN_FLAG_READ, &basic_fp);
	astring_free(fname);

	if (filerr != FILERR_NONE)
	{
		size = mame_fread(basic_fp, &mem[0x14000], 0x2000);
		if( size < 0x2000 )
		{
			logerror("%s image '%s' load failed (less than 8K)\n", Machine->gamedrv->name, astring_c(fname));
			mame_fclose(basic_fp);
			return 2;
		}
	}

	/* load an optional (dual) cartidge (e.g. basic.rom) */
	if (filerr != FILERR_NONE)
	{
		{
			size = image_fread(image, &mem[0x14000], 0x2000);
			a800_cart_loaded = size / 0x2000;
			size = image_fread(image, &mem[0x16000], 0x2000);
			a800_cart_is_16k = size / 0x2000;
			logerror("%s loaded cartridge '%s' size %s\n",
					Machine->gamedrv->name, image_filename(image), (a800_cart_is_16k) ? "16K":"8K");
		}
		mame_fclose(basic_fp);
	}

	return INIT_PASS;
}
#endif



/*************************************
 *
 *  Atari 5200 console
 *
 *************************************/

MACHINE_START( a5200 )
{
	atari_machine_start(ATARI_800XL, NULL, FALSE);
}



#ifdef MESS
DEVICE_LOAD( a5200_cart )
{
	UINT8 *mem = memory_region(REGION_CPU1);
	int size;

	/* load an optional (dual) cartidge */
	size = image_fread(image, &mem[0x4000], 0x8000);
	if (size<0x8000) memmove(mem+0x4000+0x8000-size, mem+0x4000, size);
	// mirroring of smaller cartridges
	if (size <= 0x1000) memcpy(mem+0xa000, mem+0xb000, 0x1000);
	if (size <= 0x2000) memcpy(mem+0x8000, mem+0xa000, 0x2000);
	if (size <= 0x4000)
	{
		const char *info;
		memcpy(&mem[0x4000], &mem[0x8000], 0x4000);
		info = image_extrainfo(image);
		if (info!=NULL && strcmp(info, "A13MIRRORING")==0)
		{
			memcpy(&mem[0x8000], &mem[0xa000], 0x2000);
			memcpy(&mem[0x6000], &mem[0x4000], 0x2000);
		}
	}
	logerror("%s loaded cartridge '%s' size %dK\n",
		Machine->gamedrv->name, image_filename(image) , size/1024);
	return INIT_PASS;
}

DEVICE_UNLOAD( a5200_cart )
{
	UINT8 *mem = memory_region(REGION_CPU1);
    /* zap the cartridge memory (again) */
	memset(&mem[0x4000], 0x00, 0x8000);
}
#endif

