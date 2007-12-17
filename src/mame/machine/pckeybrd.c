/**********************************************************************

    pckeybrd.c

    PC-style keyboard emulation

    This emulation is decoupled from the AT 8042 emulation used in the
    IBM ATs and above

**********************************************************************/

/* Todo: (added by KT 22-Jun-2000
    1. Check scancodes I have added are the actual scancodes for set 2 or 3.
    2. Check how codes are changed based on Shift/Control states for those sets
       that require it - info in Help PC!

*/

#include "driver.h"
#include "machine/pckeybrd.h"

#ifdef MESS
#include "inputx.h"
#endif /* MESS */

/* AT keyboard documentation comes from www.beyondlogic.org and HelpPC documentation */

/* to enable logging of keyboard read/writes */
#define LOG_KEYBOARD	0


/*
    The PS/2 models have three make/break scan code sets.  The first
      set matches the PC & XT make/break scan code set and is the one
      listed here.  Scan code sets are selected by writing the value F0
      to the keyboard via the ~8042~ (port 60h).  The following is a brief
      description of the scan code sets (see the PS/2 Technical Reference
      manuals for more information on scan code sets 2 and 3):

    *  set 1, each key has a base scan code.  Some keys generate
       extra scan codes to generate artificial shift states.  This
       is similar to the standard scan code set used on the PC and XT.
    *  set 2, each key sends one make scan code and two break scan
       codes bytes (F0 followed by the make code).  This scan code
       set is available on the IBM AT also.
    *  set 3, each key sends one make scan code and two break scan
       codes bytes (F0 followed by the make code) and no keys are
       altered by Shift/Alt/Ctrl keys.
    *  typematic scan codes are the same as the make scan code

*/


/* using the already existing input port definitions, this table re-maps
to scancode set 3.
I don't have the details for scan-code set 2,3 but they sound very similar
to the scancode set I have here. - KT 22/Jun/2000 */


/* key set 3 */
static int at_keyboard_scancode_set_2_3[]=
{
	0,
	0x076,
	0x016,
	0x01e,
	0x026,
	0x025,
	0x02e,
	0x036,
	0x03d,
	0x03e,
	0x046,
	0x045,
	0x04e,
	0x055,
	0x066,
	0x00d,
	0x015,
	0x01d,
	0x024,
	0x02d,
	0x02c,
	0x035,
	0x03c,
	0x043,
	0x044,
	0x04d,
	0x054,
	0x05b,
	0x05a,
	0x014,
	0x01c,
	0x01b,
	0x023,
	0x02b,
	0x034,
	0x033,
	0x03b,
	0x042,
	0x04b,
	0x04c,
	0x052,
	0x00e,
	0x012,
	0x05d,
	0x01a,
	0x022,
	0x021,
	0x02a,
	0x032,
	0x031,
	0x03a,
	0x041,
	0x049,
	0x04a,
	0x059,
	0x000,
	0x011,
	0x029,
	0x058,
	0x05,
	0x06,
	0x04,
	0x0c,
	0x03,
	0x0b,
	0x083,
	0x0a,
	0x01,
	0x09,
	0x077,
	0x07e,
	0x06c,
	0x075,
	0x07d,
	0x07b,
	0x06b,
	0x073,
	0x074,
	0x079,
	0x069,
	0x072,
	0x07a,
	0x070,
	0x071,
	0x00,
	0x00,
	0x078,
	0x07,
	0x05a,
	0x014,
	0x04a,
	0x000,
	0x011,
	0x06c,
	0x075,
	0x07d,
	0x06b,
	0x074,
	0x069,
	0x072,
	0x07a,
	0x070,
	0x071,
	0x000,
	0x000,
	0x000
};


#define AT_KEYBOARD_QUEUE_MAXSIZE	256

typedef struct at_keyboard
{
	AT_KEYBOARD_TYPE type;
	int on;
	UINT8 delay;   /* 240/60 -> 0,25s */
	UINT8 repeat;   /* 240/ 8 -> 30/s */
	int numlock;
	UINT8 queue[AT_KEYBOARD_QUEUE_MAXSIZE];
	UINT8 head;
	UINT8 tail;
	UINT8 make[128];

	int input_state;
	int scan_code_set;
	int last_code;

	int ports[8];
} at_keyboard;

static at_keyboard keyboard;

typedef struct extended_keyboard_code
{
	const char *pressed;
	const char *released;
} extended_keyboard_code;


static extended_keyboard_code keyboard_mf2_code[0x10][2/*numlock off, on*/]={
	{	{ "\xe0\x1c", "\xe0\x9c" } }, // keypad enter
	{	{ "\xe0\x1d", "\xe0\x9d" } }, // right control
	{	{ "\xe0\x35", "\xe0\xb5" } },
	{	{ "\xe0\x37", "\xe0\xb7" } },
	{	{ "\xe0\x38", "\xe0\xb8" } },
	{	{ "\xe0\x47", "\xe0\xc7" }, { "\xe0\x2a\xe0\x47", "\xe0\xc7\xe0\xaa" } },
	{	{ "\xe0\x48", "\xe0\xc8" }, { "\xe0\x2a\xe0\x48", "\xe0\xc8\xe0\xaa" } },
	{	{ "\xe0\x49", "\xe0\xc9" }, { "\xe0\x2a\xe0\x49", "\xe0\xc9\xe0\xaa" } },
	{	{ "\xe0\x4b", "\xe0\xcb" }, { "\xe0\x2a\xe0\x4b", "\xe0\xcb\xe0\xaa" } },
	{	{ "\xe0\x4d", "\xe0\xcd" },	{ "\xe0\x2a\xe0\x4d", "\xe0\xcd\xe0\xaa" } },
	{	{ "\xe0\x4f", "\xe0\xcf" }, { "\xe0\x2a\xe0\x4f", "\xe0\xcf\xe0\xaa" } },
	{	{ "\xe0\x50", "\xe0\xd0" }, { "\xe0\x2a\xe0\x50", "\xe0\xd0\xe0\xaa" } },
	{	{ "\xe0\x51", "\xe0\xd1" }, { "\xe0\x2a\xe0\x51", "\xe0\xd1\xe0\xaa" } },
	{	{ "\xe0\x52", "\xe0\xd2" }, { "\xe0\x2a\xe0\x52", "\xe0\xd2\xe0\xaa" } },
	{	{ "\xe0\x53", "\xe0\xd3" }, { "\xe0\x2a\xe0\x53", "\xe0\xd3\xe0\xaa" } },
	{	{ "\xe1\x1d\x45\xe1\x9d\xc5" }, { "\xe0\x2a\xe1\x1d\x45\xe1\x9d\xc5" } }
};

/* I don't think these keys change if num-lock is active! */
/* pc-at extended keyboard make/break codes for code set 3 */
static extended_keyboard_code at_keyboard_extended_codes_set_2_3[]=
{
	/*keypad enter */
	{
		"\xe0\x5a",
		"\xe0\xf0\x5a"
	},
	/* right control */
	{
		"\xe0\x14",
		"\xe0\xf0\x14"
	},
	/* keypad slash */
	{
		"\xe0\x4a",
		"\xe0\xf0\x4a"
	},
	/* print screen */
	{
		"\xe0\x12\xe0\x7c",
		0, /* I don't know the break sequence */

	},
	/* right alt */
	{
		"\xe0\x11",
		"\xe0\xf0\x11"
	},
	/* home */
	{
		"\xe0\x6c",
		"\xe0\xf0\x6c"
	},
	/* cursor up */
	{
		"\xe0\x75",
		"\xe0\xf0\x75"
	},
	/* page up */
	{
		"\xe0\x7d",
		"\xe0\xf0\x7d"
	},
	/* cursor left */
	{
		"\xe0\x6b",
		"\xe0\xf0\x6b",
	},
	/* cursor right */
	{
		"\xe0\x74",
		"\xe0\xf0\x74"
	},
	/* end */
	{
		"\xe0\x69",
		"\xe0\xf0\x69",
	},
	/* cursor down */
	{
		"\xe0\x72",
		"\xe0\xf0\x72"
	},
	/* page down */
	{
		"\xe0\x7a",
		"\xe0\xf0\x7a"
	},
	/* insert */
	{
		"\xe0\x70",
		"\xe0\xf0\x70",
	},
	/* delete */
	{
		"\xe0\x71",
		"\xe0\xf0\x71"
	},
	/* pause */
	{
		"\xe1\x14\x77\xe1\xf0\x14\xf0\x77",
		0, /*?? I don't know the break sequence */
	}

};

static void at_keyboard_queue_insert(UINT8 data);

#ifdef MESS
static int at_keyboard_queue_size(void);
static int at_keyboard_queue_chars(const unicode_char *text, size_t text_len);
static int at_keyboard_accept_char(unicode_char ch);
static int at_keyboard_charqueue_empty(void);
#endif



void at_keyboard_init(AT_KEYBOARD_TYPE type)
{
	int i;
	char buf[32];

	memset(&keyboard, 0, sizeof(keyboard));
	keyboard.type = type;
	keyboard.on = 1;
	keyboard.delay = 60;
	keyboard.repeat = 8;
	keyboard.numlock = 0;
	keyboard.head = keyboard.tail = 0;
	keyboard.input_state = 0;
	memset(&keyboard.make[0], 0, sizeof(UINT8)*128);
	/* set default led state */
	set_led_status(2, 0);
	set_led_status(0, 0);
	set_led_status(1, 0);

	keyboard.scan_code_set = 3;

	/* locate the keyboard ports */
	for (i = 0; i < sizeof(keyboard.ports) / sizeof(keyboard.ports[0]); i++)
	{
		sprintf(buf, "pc_keyboard_%d", i);
		keyboard.ports[i] = port_tag_to_index(buf);
	}

#ifdef MESS
	inputx_setup_natural_keyboard(at_keyboard_queue_chars,
		at_keyboard_accept_char,
		at_keyboard_charqueue_empty);
#endif
}



void at_keyboard_reset(void)
{
	keyboard.head = keyboard.tail = 0;
	keyboard.input_state = 0;
	memset(&keyboard.make[0], 0, sizeof(UINT8)*128);
	/* set default led state */
	set_led_status(2, 0);
	set_led_status(0, 0);
	set_led_status(1, 0);

	keyboard.scan_code_set=1;
	at_keyboard_queue_insert(0xaa);
}

/* set initial scan set */
void at_keyboard_set_scan_code_set(int set)
{
	keyboard.scan_code_set = set;
}



/* insert a code into the buffer */
static void at_keyboard_queue_insert(UINT8 data)
{
	if (LOG_KEYBOARD)
		logerror("keyboard queueing %.2x\n",data);

	keyboard.queue[keyboard.head] = data;
	keyboard.head++;
	keyboard.head %= (sizeof(keyboard.queue) / sizeof(keyboard.queue[0]));
}


#ifdef MESS
static int at_keyboard_queue_size(void)
{
	int queue_size;
	queue_size = keyboard.head - keyboard.tail;
	if (queue_size < 0)
		queue_size += sizeof(keyboard.queue) / sizeof(keyboard.queue[0]);
	return queue_size;
}
#endif


/* add a list of codes to the keyboard buffer */
static void at_keyboard_helper(const char *codes)
{
	int i;
	for (i = 0; codes[i]; i++)
		at_keyboard_queue_insert(codes[i]);
}


/* add codes for standard keys */
static void at_keyboard_standard_scancode_insert(int our_code, int pressed)
{
	int scancode = our_code;

	switch (keyboard.scan_code_set)
	{
		case 1:
		{
			/* the original code was designed for this set, and there is
            a 1:1 correspondance for the scancodes */
			scancode = our_code;

			if (!pressed)
			{
				/* adjust code for break code */
				scancode|=0x080;
			}
		}
		break;

		case 2:
		case 3:
		{
			/* lookup scancode */
			scancode = at_keyboard_scancode_set_2_3[our_code];

			if (!pressed)
			{
				/* break code */
				at_keyboard_queue_insert(0x0f0);
			}

		}
		break;
	}

	at_keyboard_queue_insert(scancode);
}

static void at_keyboard_extended_scancode_insert(int code, int pressed)
{
	code = code - 0x060;

	switch (keyboard.scan_code_set)
	{
		case 1:
		{
			if (pressed)
			{
				if (keyboard_mf2_code[code][keyboard.numlock].pressed)
					at_keyboard_helper(keyboard_mf2_code[code][keyboard.numlock].pressed);
				else
					at_keyboard_helper(keyboard_mf2_code[code][0].pressed);
			}
			else
			{
				if (keyboard_mf2_code[code][keyboard.numlock].released)
					at_keyboard_helper(keyboard_mf2_code[code][keyboard.numlock].released);
				else if (keyboard_mf2_code[code][0].released)
					at_keyboard_helper(keyboard_mf2_code[code][0].released);
			}
		}
		break;

		case 2:
		case 3:
		{
			extended_keyboard_code *key = &at_keyboard_extended_codes_set_2_3[code];

			if (pressed)
			{
				if (key->pressed)
				{
					at_keyboard_helper(key->pressed);
				}
			}
			else
			{
				if (key->released)
				{
					at_keyboard_helper(key->released);
				}
			}
		}
		break;
	}

}


/**************************************************************************
 *  scan keys and stuff make/break codes
 **************************************************************************/

static UINT32 at_keyboard_readport(int port)
{
	UINT32 result = 0;
	if (keyboard.ports[port] >= 0)
		result = readinputport(keyboard.ports[port]);
	return result;
}

void at_keyboard_polling(void)
{
	int i;

	if (keyboard.on)
	{
		/* add codes for keys that are set */
		for( i = 0x01; i < 0x80; i++  )
		{
			if (i==0x60) i+=0x10; // keys 0x60..0x6f need special handling

			if( at_keyboard_readport(i/16) & (1 << (i & 15)) )
			{
				if( keyboard.make[i] == 0 )
				{
					keyboard.make[i] = 1;

					if (i==0x45) keyboard.numlock^=1;

					at_keyboard_standard_scancode_insert(i,1);
				}
				else
				{
					keyboard.make[i] += 1;

					if( keyboard.make[i] == keyboard.delay )
					{
						at_keyboard_standard_scancode_insert(i, 1);
					}
					else
					{
						if( keyboard.make[i] == keyboard.delay + keyboard.repeat )
						{
							keyboard.make[i] = keyboard.delay;
							at_keyboard_standard_scancode_insert(i, 1);
						}
					}
				}
			}
			else
			{
				if( keyboard.make[i] )
				{
					keyboard.make[i] = 0;

					at_keyboard_standard_scancode_insert(i, 0);
				}
			}
		}

			/* extended scan-codes */
			for( i = 0x60; i < 0x70; i++  )
			{
				if( at_keyboard_readport(i/16) & (1 << (i & 15)) )
				{
					if( keyboard.make[i] == 0 )
					{
						keyboard.make[i] = 1;

						at_keyboard_extended_scancode_insert(i,1);

					}
					else
					{
						keyboard.make[i] += 1;
						if( keyboard.make[i] == keyboard.delay )
						{
							at_keyboard_extended_scancode_insert(i, 1);
						}
						else
						{
							if( keyboard.make[i] == keyboard.delay + keyboard.repeat )
							{
								keyboard.make[i]=keyboard.delay;

								at_keyboard_extended_scancode_insert(i, 1);
							}
						}
					}
				}
				else
				{
					if( keyboard.make[i] )
					{
						keyboard.make[i] = 0;

						at_keyboard_extended_scancode_insert(i,0);
					}
				}
		}
	}
}

int at_keyboard_read(void)
{
	int data;
	if (keyboard.tail == keyboard.head)
		return -1;

	data = keyboard.queue[keyboard.tail];

	if (LOG_KEYBOARD)
		logerror("at_keyboard_read(): Keyboard Read 0x%02x\n",data);

	keyboard.tail++;
	keyboard.tail %= sizeof(keyboard.queue) / sizeof(keyboard.queue[0]);
	return data;
}

static void at_clear_buffer_and_acknowledge(void)
{
	/* clear output buffer and respond with acknowledge */
	keyboard.head = keyboard.tail = 0;
	at_keyboard_queue_insert(0x0fa);
}

/* From Ralf Browns Interrupt list:

Values for keyboard commands (data also goes to PORT 0060h):
Value   Count   Description
 EDh    double  set/reset mode indicators Caps Num Scrl
        bit 2 = CapsLk, bit 1 = NumLk, bit 0 = ScrlLk
        all other bits must be zero.
 EEh    sngl    diagnostic echo. returns EEh.
 EFh    sngl    NOP (No OPeration). reserved for future use
 EF+26h double  [Cherry MF2 G80-1501HAD] read 256 bytes of chipcard data
        keyboard must be disabled before this and has to
        be enabled after finished.
 F0h    double  get/set scan code set
        00h get current set
        01h scancode set 1 (PCs and PS/2 mod 30, except Type 2 ctrlr)

        02h scancode set 2 (ATs, PS/2, default)
        03h scancode set 3
 F2h    sngl    read keyboard ID (read two ID bytes)
        AT keyboards returns FA (ACK)
        MF2 returns AB 41 (translation) or
                AB 83 (pass through)
 F3h    double  set typematic rate/delay
        format of the second byte:
        bit7=0 : reserved
        bit6-5 : typemativ delay
             00b=250ms     10b= 750ms
             01b=500ms     11b=1000ms
        bit4-0 : typematic rate (see #P050)
 F4h    sngl    enable keyboard
 F5h    sngl    disable keyboard. set default parameters (no keyboard scanning)
 F6h    sngl    set default parameters
 F7h    sngl    [MCA] set all keys to typematic (scancode set 3)

 F8h    sngl    [MCA] set all keys to make/release
 F9h    sngl    [MCA] set all keys to make only
 FAh    sngl    [MCA] set all keys to typematic/make/release
 FBh    sngl    [MCA] set al keys to typematic
 FCh    double  [MCA] set specific key to make/release
 FDh    double  [MCA] set specific key to make only
 FEh    sngl    resend last scancode
 FFh    sngl    perform internal power-on reset function
Note:   each command is acknowledged by FAh (ACK), if not mentioned otherwise.
      See PORT 0060h-R for details.
SeeAlso: #P046
*/

void at_keyboard_write(UINT8 data)
{
	if (LOG_KEYBOARD)
		logerror("keyboard write %.2x\n",data);

	switch (keyboard.input_state)
	{
		case 0:
			switch (data) {
			case 0xed: // leds schalten
				/* acknowledge */
				at_keyboard_queue_insert(0x0fa);
				/* now waiting for  code... */
				keyboard.input_state=1;
				break;
			case 0xee: // echo
				/* echo code with no acknowledge */
				at_keyboard_queue_insert(0xee);
				break;
			case 0xf0: // scancodes adjust
				/* acknowledge */
				at_clear_buffer_and_acknowledge();
				/* waiting for data */
				keyboard.input_state=2;
				break;
			case 0xf2: // identify keyboard
				/* ack and two byte keyboard id */
				at_keyboard_queue_insert(0xfa);

				/* send keyboard code */
				if (keyboard.type == AT_KEYBOARD_TYPE_MF2) {
					at_keyboard_queue_insert(0xab);
					at_keyboard_queue_insert(0x41);
				}
				else
				{
					/* from help-pc docs */
					at_keyboard_queue_insert(0x0ab);
					at_keyboard_queue_insert(0x083);
				}

				break;
			case 0xf3: // adjust rates
				/* acknowledge */
				at_keyboard_queue_insert(0x0fa);

				keyboard.input_state=3;
				break;
			case 0xf4: // activate
				at_clear_buffer_and_acknowledge();

				keyboard.on=1;
				break;
			case 0xf5:
				/* acknowledge */
				at_clear_buffer_and_acknowledge();
				// standardvalues
				keyboard.on=0;
				break;
			case 0xf6:
				at_clear_buffer_and_acknowledge();
				// standardvalues
				keyboard.on=1;
				break;
			case 0xfe: // resend
				// should not happen, for now send 0
				at_keyboard_queue_insert(0);	//keyboard.last_code);
				break;
			case 0xff: // reset
				/* it doesn't state this in the docs I have read, but I assume
                that the keyboard input buffer is cleared. The PCW16 sends &ff,
                and requires that 0x0fa is the first byte to be read */

				at_clear_buffer_and_acknowledge();

	//          /* acknowledge */
	//          at_keyboard_queue_insert(0xfa);
				/* BAT completion code */
				at_keyboard_queue_insert(0xaa);
				break;
			}
			break;
		case 1:
			/* code received */
			keyboard.input_state=0;

			/* command? */
			if (data & 0x080)
			{
				/* command received instead of code - execute command */
				at_keyboard_write(data);
			}
			else
			{
				/* send acknowledge */
				at_keyboard_queue_insert(0x0fa);

				/* led  bits */
				/* bits: 0 scroll lock, 1 num lock, 2 capslock */

				/* led's in same order as my keyboard leds. */
				/* num lock, caps lock, scroll lock */
				set_led_status(2, (data & 0x01));
				set_led_status(0, ((data & 0x02)>>1));
				set_led_status(1, ((data & 0x04)>>2));

			}
			break;
		case 2:
			keyboard.input_state=0;

			/* command? */
			if (data & 0x080)
			{
				/* command received instead of code - execute command */
				at_keyboard_write(data);
			}
			else
			{
				/* 00  return byte indicating scan code set in use
                01  select scan code set 1  (used on PC & XT)
                02  select scan code set 2
                03  select scan code set 3
                */

				if (data == 0x00)
				{
						at_keyboard_queue_insert(keyboard.scan_code_set);
				}
				else
				{
					keyboard.scan_code_set = data;
				}
			}

			break;
		case 3:
			/* 6,5: 250ms, 500ms, 750ms, 1s */
			/* 4..0: 30 26.7 .... 2 chars/s*/

			/* command? */
			keyboard.input_state=0;
			if (data & 0x080)
			{
				/* command received instead of code - execute command */
				at_keyboard_write(data);
			}
			else
			{
				/* received keyboard repeat */

			}

			break;
	}
}

#ifdef MESS
/***************************************************************************
  unicode_char_to_at_keycode
***************************************************************************/

static UINT8 unicode_char_to_at_keycode(unicode_char ch)
{
	UINT8 b;
	switch(ch)
	{
		case '\033':					b = 1;		break;
		case '1':						b = 2;		break;
		case '2':						b = 3;		break;
		case '3':						b = 4;		break;
		case '4':						b = 5;		break;
		case '5':						b = 6;		break;
		case '6':						b = 7;		break;
		case '7':						b = 8;		break;
		case '8':						b = 9;		break;
		case '9':						b = 10;		break;
		case '0':						b = 11;		break;
		case '-':						b = 12;		break;
		case '=':						b = 13;		break;
		case '\010':					b = 14;		break;
		case '\t':						b = 15;		break;
		case 'q':						b = 16;		break;
		case 'w':						b = 17;		break;
		case 'e':						b = 18;		break;
		case 'r':						b = 19;		break;
		case 't':						b = 20;		break;
		case 'y':						b = 21;		break;
		case 'u':						b = 22;		break;
		case 'i':						b = 23;		break;
		case 'o':						b = 24;		break;
		case 'p':						b = 25;		break;
		case '[':						b = 26;		break;
		case ']':						b = 27;		break;
		case '\r':						b = 28;		break;
		case UCHAR_MAMEKEY(CAPSLOCK):	b = 29;		break;
		case 'a':						b = 30;		break;
		case 's':						b = 31;		break;
		case 'd':						b = 32;		break;
		case 'f':						b = 33;		break;
		case 'g':						b = 34;		break;
		case 'h':						b = 35;		break;
		case 'j':						b = 36;		break;
		case 'k':						b = 37;		break;
		case 'l':						b = 38;		break;
		case ';':						b = 39;		break;
		case '\'':						b = 40;		break;
		case '`':						b = 41;		break;
		case '\\':						b = 43;		break;
		case 'z':						b = 44;		break;
		case 'x':						b = 45;		break;
		case 'c':						b = 46;		break;
		case 'v':						b = 47;		break;
		case 'b':						b = 48;		break;
		case 'n':						b = 49;		break;
		case 'm':						b = 50;		break;
		case ',':						b = 51;		break;
		case '.':						b = 52;		break;
		case '/':						b = 53;		break;
		case ' ':						b = 0x39;	break;
		case UCHAR_MAMEKEY(F1):			b = 0x3b;	break;
		case UCHAR_MAMEKEY(F2):			b = 0x3c;	break;
		case UCHAR_MAMEKEY(F3):			b = 0x3d;	break;
		case UCHAR_MAMEKEY(F4):			b = 0x3e;	break;
		case UCHAR_MAMEKEY(F5):			b = 0x3f;	break;
		case UCHAR_MAMEKEY(F6):			b = 0x40;	break;
		case UCHAR_MAMEKEY(F7):			b = 0x41;	break;
		case UCHAR_MAMEKEY(F8):			b = 0x42;	break;
		case UCHAR_MAMEKEY(F9):			b = 0x43;	break;
		case UCHAR_MAMEKEY(F10):		b = 0x44;	break;
		case UCHAR_MAMEKEY(NUMLOCK):	b = 0x45;	break;
		case UCHAR_MAMEKEY(SCRLOCK):	b = 0x46;	break;
		case UCHAR_MAMEKEY(7_PAD):		b = 0x47;	break;
		case UCHAR_MAMEKEY(8_PAD):		b = 0x48;	break;
		case UCHAR_MAMEKEY(9_PAD):		b = 0x49;	break;
		case UCHAR_MAMEKEY(MINUS_PAD):	b = 0x4a;	break;
		case UCHAR_MAMEKEY(4_PAD):		b = 0x4b;	break;
		case UCHAR_MAMEKEY(5_PAD):		b = 0x4c;	break;
		case UCHAR_MAMEKEY(6_PAD):		b = 0x4d;	break;
		case UCHAR_MAMEKEY(PLUS_PAD):	b = 0x4e;	break;
		case UCHAR_MAMEKEY(1_PAD):		b = 0x4f;	break;
		case UCHAR_MAMEKEY(2_PAD):		b = 0x50;	break;
		case UCHAR_MAMEKEY(3_PAD):		b = 0x51;	break;
		case UCHAR_MAMEKEY(0_PAD):		b = 0x52;	break;
		case UCHAR_MAMEKEY(DEL_PAD):	b = 0x53;	break;
		case UCHAR_MAMEKEY(F11):		b = 0x57;	break;
		case UCHAR_MAMEKEY(F12):		b = 0x58;	break;
		case '~':						b = 0x81;	break;
		case '!':						b = 0x82;	break;
		case '@':						b = 0x83;	break;
		case '#':						b = 0x84;	break;
		case '$':						b = 0x85;	break;
		case '%':						b = 0x86;	break;
		case '^':						b = 0x87;	break;
		case '&':						b = 0x88;	break;
		case '*':						b = 0x89;	break;
		case '(':						b = 0x8a;	break;
		case ')':						b = 0x8b;	break;
		case '_':						b = 0x8c;	break;
		case '+':						b = 0x8d;	break;
		case 'Q':						b = 0x90;	break;
		case 'W':						b = 0x91;	break;
		case 'E':						b = 0x92;	break;
		case 'R':						b = 0x93;	break;
		case 'T':						b = 0x94;	break;
		case 'Y':						b = 0x95;	break;
		case 'U':						b = 0x96;	break;
		case 'I':						b = 0x97;	break;
		case 'O':						b = 0x98;	break;
		case 'P':						b = 0x99;	break;
		case '{':						b = 0x9a;	break;
		case '}':						b = 0x9b;	break;
		case 'A':						b = 0x9e;	break;
		case 'S':						b = 0x9f;	break;
		case 'D':						b = 0xa0;	break;
		case 'F':						b = 0xa1;	break;
		case 'G':						b = 0xa2;	break;
		case 'H':						b = 0xa3;	break;
		case 'J':						b = 0xa4;	break;
		case 'K':						b = 0xa5;	break;
		case 'L':						b = 0xa6;	break;
		case ':':						b = 0xa7;	break;
		case '\"':						b = 0xa8;	break;
		case '|':						b = 0xab;	break;
		case 'Z':						b = 0xac;	break;
		case 'X':						b = 0xad;	break;
		case 'C':						b = 0xae;	break;
		case 'V':						b = 0xaf;	break;
		case 'B':						b = 0xb0;	break;
		case 'N':						b = 0xb1;	break;
		case 'M':						b = 0xb2;	break;
		case '<':						b = 0xb3;	break;
		case '>':						b = 0xb4;	break;
		case '?':						b = 0xb5;	break;
		default:						b = 0;		break;
	}
	return b;
}

/***************************************************************************
  at_keyboard_queue_chars
***************************************************************************/

static int at_keyboard_queue_chars(const unicode_char *text, size_t text_len)
{
	int i;
	UINT8 b;

	for (i = 0; (i < text_len) && ((at_keyboard_queue_size()) + 4 < AT_KEYBOARD_QUEUE_MAXSIZE); i++)
	{
		b = unicode_char_to_at_keycode(text[i]);
		if (b)
		{
			if (b & 0x80)
				at_keyboard_standard_scancode_insert(0x36, 1);

			at_keyboard_standard_scancode_insert(b & 0x7f, 1);
			at_keyboard_standard_scancode_insert(b & 0x7f, 0);

			if (b & 0x80)
				at_keyboard_standard_scancode_insert(0x36, 0);
		}
	}
	return i;
}



/***************************************************************************
  Keyboard declaration
***************************************************************************/

INPUT_PORTS_START( pc_keyboard )
	PORT_START_TAG("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED ) 	/* unused scancode 0 */
	PORT_BIT( 0x0002, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)			/* Esc                         01  81 */
	PORT_BIT( 0x0004, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')	/* 1                           02  82 */
	PORT_BIT( 0x0008, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')	/* 2                           03  83 */
	PORT_BIT( 0x0010, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')	/* 3                           04  84 */
	PORT_BIT( 0x0020, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')	/* 4                           05  85 */
	PORT_BIT( 0x0040, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')	/* 5                           06  86 */
	PORT_BIT( 0x0080, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')	/* 6                           07  87 */
	PORT_BIT( 0x0100, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')	/* 7                           08  88 */
	PORT_BIT( 0x0200, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')	/* 8                           09  89 */
	PORT_BIT( 0x0400, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')	/* 9                           0A  8A */
	PORT_BIT( 0x0800, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')	/* 0                           0B  8B */
	PORT_BIT( 0x1000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')	/* -                           0C  8C */
	PORT_BIT( 0x2000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')	/* =                           0D  8D */
	PORT_BIT( 0x4000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)			/* Backspace                   0E  8E */
	PORT_BIT( 0x8000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)			/* Tab                         0F  8F */

	PORT_START_TAG("pc_keyboard_1")
	PORT_BIT( 0x0001, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') /* Q                           10  90 */
	PORT_BIT( 0x0002, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') /* W                           11  91 */
	PORT_BIT( 0x0004, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') /* E                           12  92 */
	PORT_BIT( 0x0008, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') /* R                           13  93 */
	PORT_BIT( 0x0010, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') /* T                           14  94 */
	PORT_BIT( 0x0020, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') /* Y                           15  95 */
	PORT_BIT( 0x0040, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') /* U                           16  96 */
	PORT_BIT( 0x0080, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') /* I                           17  97 */
	PORT_BIT( 0x0100, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') /* O                           18  98 */
	PORT_BIT( 0x0200, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') /* P                           19  99 */
	PORT_BIT( 0x0400, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') /* [                           1A  9A */
	PORT_BIT( 0x0800, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') /* ]                           1B  9B */
	PORT_BIT( 0x1000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) /* Enter                       1C  9C */
	PORT_BIT( 0x2000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))      /* Left Ctrl                   1D  9D */
	PORT_BIT( 0x4000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') /* A                           1E  9E */
	PORT_BIT( 0x8000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') /* S                           1F  9F */

	PORT_START_TAG("pc_keyboard_2")
	PORT_BIT( 0x0001, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') /* D                           20  A0 */
	PORT_BIT( 0x0002, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') /* F                           21  A1 */
	PORT_BIT( 0x0004, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') /* G                           22  A2 */
	PORT_BIT( 0x0008, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') /* H                           23  A3 */
	PORT_BIT( 0x0010, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') /* J                           24  A4 */
	PORT_BIT( 0x0020, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') /* K                           25  A5 */
	PORT_BIT( 0x0040, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') /* L                           26  A6 */
	PORT_BIT( 0x0080, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':') /* ;                           27  A7 */
	PORT_BIT( 0x0100, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"') /* '                           28  A8 */
	PORT_BIT( 0x0200, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~') /* `                           29  A9 */
	PORT_BIT( 0x0400, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_MAMEKEY(LSHIFT)) /* Left Shift                  2A  AA */
	PORT_BIT( 0x0800, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') /* \                           2B  AB */
	PORT_BIT( 0x1000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') /* Z                           2C  AC */
	PORT_BIT( 0x2000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') /* X                           2D  AD */
	PORT_BIT( 0x4000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') /* C                           2E  AE */
	PORT_BIT( 0x8000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') /* V                           2F  AF */

	PORT_START_TAG("pc_keyboard_3")
	PORT_BIT( 0x0001, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') /* B                           30  B0 */
	PORT_BIT( 0x0002, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') /* N                           31  B1 */
	PORT_BIT( 0x0004, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') /* M                           32  B2 */
	PORT_BIT( 0x0008, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') /* ,                           33  B3 */
	PORT_BIT( 0x0010, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') /* .                           34  B4 */
	PORT_BIT( 0x0020, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') /* /                           35  B5 */
	PORT_BIT( 0x0040, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) /* Right Shift                 36  B6 */
	PORT_BIT( 0x0080, 0x0000, IPT_KEYBOARD) PORT_NAME("KP * (PrtScr)") PORT_CODE(KEYCODE_ASTERISK)		/* Keypad *  (PrtSc)           37  B7 */
	PORT_BIT( 0x0100, 0x0000, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT)					/* Left Alt                    38  B8 */
	PORT_BIT( 0x0200, 0x0000, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')	/* Space                       39  B9 */
	PORT_BIT( 0x0400, 0x0000, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE	/* Caps Lock                   3A  BA */
	PORT_BIT( 0x0800, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))			/* F1                          3B  BB */
	PORT_BIT( 0x1000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))			/* F2                          3C  BC */
	PORT_BIT( 0x2000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))			/* F3                          3D  BD */
	PORT_BIT( 0x4000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))			/* F4                          3E  BE */
	PORT_BIT( 0x8000, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))			/* F5                          3F  BF */

	PORT_START_TAG("pc_keyboard_4")
	PORT_BIT( 0x0001, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))			 /* F6                          40  C0 */
	PORT_BIT( 0x0002, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))			 /* F7                          41  C1 */
	PORT_BIT( 0x0004, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))			 /* F8                          42  C2 */
	PORT_BIT( 0x0008, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))		     /* F9                          43  C3 */
	PORT_BIT( 0x0010, 0x0000, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))	     /* F10                         44  C4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK)     /* Num Lock                    45  C5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ScrLock") PORT_CODE(KEYCODE_SCRLOCK)     /* Scroll Lock                 46  C6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 (Home)") PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_HOME )  /* Keypad 7  (Home)            47  C7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 (Up)") PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_UP )    /* Keypad 8  (Up arrow)        48  C8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_PGUP)   /* Keypad 9  (PgUp)            49  C9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -") PORT_CODE(KEYCODE_MINUS_PAD)     /* Keypad -                    4A  CA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 (Left)") PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_LEFT )  /* Keypad 4  (Left arrow)      4B  CB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5") PORT_CODE(KEYCODE_5_PAD)     /* Keypad 5                    4C  CC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6 (Right)") PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_RIGHT ) /* Keypad 6  (Right arrow)     4D  CD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +") PORT_CODE(KEYCODE_PLUS_PAD)     /* Keypad +                    4E  CE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_END )   /* Keypad 1  (End)             4F  CF */

	PORT_START_TAG("pc_keyboard_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 (Down)") PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_DOWN )   /* Keypad 2  (Down arrow)      50  D0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_PGDN )   /* Keypad 3  (PgDn)            51  D1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0 (Ins)") PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_INSERT ) /* Keypad 0  (Ins)             52  D2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP . (Del)") PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(KEYCODE_DEL )    /* Keypad .  (Del)             53  D3 */
	PORT_BIT ( 0x0030, 0x0000, IPT_UNUSED )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(84/102)\\") PORT_CODE(KEYCODE_BACKSLASH2)      /* Backslash 2                 56  D6 */
	PORT_BIT ( 0xff80, 0x0000, IPT_UNUSED )

	PORT_START_TAG("pc_keyboard_6")
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )

	PORT_START_TAG("pc_keyboard_7")
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )
INPUT_PORTS_END



INPUT_PORTS_START( at_keyboard )
	PORT_START_TAG("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED ) 	/* unused scancode 0 */
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

	PORT_START_TAG("pc_keyboard_1")
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

	PORT_START_TAG("pc_keyboard_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) /* D                           20  A0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) /* F                           21  A1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) /* G                           22  A2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) /* H                           23  A3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) /* J                           24  A4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) /* K                           25  A5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) /* L                           26  A6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) /* ;                           27  A7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) /* '                           28  A8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) /* `                           29  A9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Shift") PORT_CODE(KEYCODE_LSHIFT) /* Left Shift                  2A  AA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) /* \                           2B  AB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) /* Z                           2C  AC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) /* X                           2D  AD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) /* C                           2E  AE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) /* V                           2F  AF */

	PORT_START_TAG("pc_keyboard_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) /* B                           30  B0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) /* N                           31  B1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) /* M                           32  B2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) /* ,                           33  B3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) /* .                           34  B4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) /* /                           35  B5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-Shift") PORT_CODE(KEYCODE_RSHIFT) /* Right Shift                 36  B6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP * (PrtScr)") PORT_CODE(KEYCODE_ASTERISK    ) /* Keypad *  (PrtSc)           37  B7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) /* Left Alt                    38  B8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) /* Space                       39  B9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) /* Caps Lock                   3A  BA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) /* F1                          3B  BB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) /* F2                          3C  BC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) /* F3                          3D  BD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) /* F4                          3E  BE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) /* F5                          3F  BF */

	PORT_START_TAG("pc_keyboard_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) /* F6                          40  C0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) /* F7                          41  C1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) /* F8                          42  C2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) /* F9                          43  C3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) /* F10                         44  C4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK) /* Num Lock                    45  C5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ScrLock") PORT_CODE(KEYCODE_SCRLOCK) /* Scroll Lock                 46  C6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 (Home)") PORT_CODE(KEYCODE_7_PAD       ) /* Keypad 7  (Home)            47  C7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 (Up)") PORT_CODE(KEYCODE_8_PAD       ) /* Keypad 8  (Up arrow)        48  C8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD       ) /* Keypad 9  (PgUp)            49  C9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -") PORT_CODE(KEYCODE_MINUS_PAD) /* Keypad -                    4A  CA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 (Left)") PORT_CODE(KEYCODE_4_PAD       ) /* Keypad 4  (Left arrow)      4B  CB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5") PORT_CODE(KEYCODE_5_PAD) /* Keypad 5                    4C  CC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6 (Right)") PORT_CODE(KEYCODE_6_PAD       ) /* Keypad 6  (Right arrow)     4D  CD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +") PORT_CODE(KEYCODE_PLUS_PAD) /* Keypad +                    4E  CE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD       ) /* Keypad 1  (End)             4F  CF */

	PORT_START_TAG("pc_keyboard_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 (Down)") PORT_CODE(KEYCODE_2_PAD       ) /* Keypad 2  (Down arrow)      50  D0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD       ) /* Keypad 3  (PgDn)            51  D1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0 (Ins)") PORT_CODE(KEYCODE_0_PAD       ) /* Keypad 0  (Ins)             52  D2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP . (Del)") PORT_CODE(KEYCODE_DEL_PAD     ) /* Keypad .  (Del)             53  D3 */
	PORT_BIT ( 0x0030, 0x0000, IPT_UNUSED )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(84/102)\\") PORT_CODE(KEYCODE_BACKSLASH2) /* Backslash 2                 56  D6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)F11") PORT_CODE(KEYCODE_F11) /* F11                         57  D7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)F12") PORT_CODE(KEYCODE_F12) /* F12                         58  D8 */
	PORT_BIT ( 0xfe00, 0x0000, IPT_UNUSED )

	PORT_START_TAG("pc_keyboard_6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)KP Enter") PORT_CODE(KEYCODE_ENTER_PAD) /* PAD Enter                   60  e0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Right Control") PORT_CODE(KEYCODE_RCONTROL) /* Right Control               61  e1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)KP /") PORT_CODE(KEYCODE_SLASH_PAD) /* PAD Slash                   62  e2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)PRTSCR") PORT_CODE(KEYCODE_PRTSCR) /* Print Screen                63  e3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)ALTGR") PORT_CODE(KEYCODE_RALT) /* ALTGR                       64  e4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Home") PORT_CODE(KEYCODE_HOME) /* Home                        66  e6 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Cursor Up") PORT_CODE(KEYCODE_UP) /* Up                          67  e7 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Page Up") PORT_CODE(KEYCODE_PGUP) /* Page Up                     68  e8 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Cursor Left") PORT_CODE(KEYCODE_LEFT) /* Left                        69  e9 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Cursor Right") PORT_CODE(KEYCODE_RIGHT) /* Right                       6a  ea */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)End") PORT_CODE(KEYCODE_END) /* End                         6b  eb */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Cursor Down") PORT_CODE(KEYCODE_DOWN) /* Down                        6c  ec */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Page Down") PORT_CODE(KEYCODE_PGDN) /* Page Down                   6d  ed */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Insert") PORT_CODE(KEYCODE_INSERT) /* Insert                      6e  ee */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Delete") PORT_CODE(KEYCODE_DEL) /* Delete                      6f  ef */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(MF2)Pause") PORT_CODE(KEYCODE_PAUSE) /* Pause                       65  e5 */

	PORT_START_TAG("pc_keyboard_7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Print Screen") PORT_CODE(KEYCODE_PRTSCR) /* Print Screen alternate      77  f7 */
	PORT_BIT ( 0xfffe, 0x0000, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************
  Inputx stuff
***************************************************************************/

static int at_keyboard_accept_char(unicode_char ch)
{
	return unicode_char_to_at_keycode(ch) != 0;
}



static int at_keyboard_charqueue_empty(void)
{
	return at_keyboard_queue_size() == 0;
}
#endif /* MESS */



