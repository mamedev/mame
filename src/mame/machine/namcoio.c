/***************************************************************************

The following Namco custom chips are all instances of the same 4-bit MCU,
the Fujitsu MB8843 (42-pin DIP package) and MB8842/MB8844 (28-pin DIP),
differently programmed.

chip  MCU   pins function
---- ------ ---- --------
56XX         42  I/O (coin management built-in)
58XX         42  I/O (coin management built-in)
62XX         28  I/O and explosion (noise) generator

16XX interface:
---------------
Super Pac Man           56XX  56XX  ----  ----
Pac & Pal               56XX  59XX  ----  ----
Mappy                   58XX  58XX  ----  ----
Phozon                  58XX  56XX  ----  ----
The Tower of Druaga     58XX  56XX  ----  ----
Grobda                  58XX  56XX  ----  ----
Dig Dug II              58XX  56XX  ----  ----
Motos                   56XX  56XX  ----  ----
Gaplus                  56XX  58XX  62XX  ----
Gaplus (alt.)           58XX  56XX  62XX  ----
Libble Rabble           58XX  56XX  56XX  ----
Toy Pop                 58XX  56XX  56XX  ----


Pinouts:

        MB8843                   MB8842/MB8844
       +------+                    +------+
  EXTAL|1   42|Vcc            EXTAL|1   28|Vcc
   XTAL|2   41|K3              XTAL|2   27|K3
 /RESET|3   40|K2            /RESET|3   26|K2
   /IRQ|4   39|K1                O0|4   25|K1
     SO|5   38|K0                O1|5   24|K0
     SI|6   37|R15               O2|6   23|R10 /IRQ
/SC /TO|7   36|R14               O3|7   22|R9 /TC
    /TC|8   35|R13               O4|8   21|R8
     P0|9   34|R12               O5|9   20|R7
     P1|10  33|R11               O6|10  19|R6
     P2|11  32|R10               O7|11  18|R5
     P3|12  31|R9                R0|12  17|R4
     O0|13  30|R8                R1|13  16|R3
     O1|14  29|R7               GND|14  15|R2
     O2|15  28|R6                  +------+
     O3|16  27|R5
     O4|17  26|R4
     O5|18  25|R3
     O6|19  24|R2
     O7|20  23|R1
    GND|21  22|R0
       +------+


      O  O  R  R  R  K
62XX  O  O  IO O     I

      P  O  O  R  R  R  R  K
56XX  O  O  O  I  I  I  IO I
58XX  O  O  O  I  I  I  IO I
59XX  O  O  O  I  I  I  IO I


Namco custom I/O chips 56XX, 58XX, 59XX

These chips work together with a 16XX, that interfaces them with the buffer
RAM. Each chip uses 16 nibbles of memory; the 16XX supports up to 4 chips,
but most games use only 2.

The 56XX, 58XX and 59XX are pin-to-pin compatible, but not functionally equivalent:
they provide the same functions, but the command codes and memory addresses
are different, so they cannot be exchanged.

The devices have 42 pins. There are 16 input lines and 8 output lines to be
used for I/O.


pin   description
---   -----------
1     clock (Mappy, Super Pac-Man)
2     clock (Gaplus; equivalent to the above?)
3     reset
4     irq
5-6   (to/from 16XX) (this is probably a normal I/O port used to synchronize with the 16XX)
7-8   ?
9-12  address to r/w from RAM; 12 also goes to the 16XX and acts as r/w line, so
      the chip can only read from addresses 0-7 and only write to addresses 8-F
      (this is probably a normal I/O port used for that purpose)
13-16 out port A
17-20 out port B
21    GND
22-25 in port B
26-29 in port C
30-33 in port D
34-37 (to 16XX) probably data to r/w from RAM
      (this is probably a normal I/O port used for that purpose)
38-41 in port A
42    Vcc

TODO:
- It's likely that the 56XX and 58XX chips, when in "coin mode", also internally
  handle outputs for start lamps, coin counters and coin lockout, like the 51XX.
  Such lines are NOT present in the Mappy and Super Pacman schematics, so they
  were probably not used for those games, but they might have been used in
  others (most likely Gaplus).

***************************************************************************/

#include "driver.h"
#include "machine/namcoio.h"
#include "machine/namco50.h"
#include "machine/namco51.h"
#include "machine/namco53.h"
#include "audio/namco52.h"
#include "audio/namco54.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

struct namcoio
{
	INT32 type;
	const device_config *device;
	read8_space_func in[4];
	write8_space_func out[2];
	INT32 reset;
	INT32 lastcoins,lastbuttons;
	INT32 credits;
	INT32 coins[2];
	INT32 coins_per_cred[2];
	INT32 creds_per_coin[2];
	INT32 in_count;
};

static struct namcoio io[MAX_NAMCOIO];

static READ8_HANDLER( nop_r ) { return 0x0f; }
static WRITE8_HANDLER( nop_w ) { }

#define READ_PORT(m,n)	(io[chip].in[n](m,0) & 0x0f)
#define WRITE_PORT(m,n,d)	io[chip].out[n](m,0,(d) & 0x0f)



/***************************************************************************/


static UINT8 namcoio_ram[MAX_NAMCOIO * 16];

#define IORAM_READ(offset) (namcoio_ram[chip * 0x10 + (offset)] & 0x0f)
#define IORAM_WRITE(offset,data) {namcoio_ram[chip * 0x10 + (offset)] = (data) & 0x0f;}


static void handle_coins(running_machine *machine,int chip,int swap)
{
	int val,toggled;
	int credit_add = 0;
	int credit_sub = 0;
	int button;
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

//popmessage("%x %x %x %x %x %x %x %x",IORAM_READ(8),IORAM_READ(9),IORAM_READ(10),IORAM_READ(11),IORAM_READ(12),IORAM_READ(13),IORAM_READ(14),IORAM_READ(15));

	val = ~READ_PORT(space,0);	// pins 38-41
	toggled = val ^ io[chip].lastcoins;
	io[chip].lastcoins = val;

	/* check coin insertion */
	if (val & toggled & 0x01)
	{
		io[chip].coins[0]++;
		if (io[chip].coins[0] >= (io[chip].coins_per_cred[0] & 7))
		{
			credit_add = io[chip].creds_per_coin[0] - (io[chip].coins_per_cred[0] >> 3);
			io[chip].coins[0] -= io[chip].coins_per_cred[0] & 7;
		}
		else if (io[chip].coins_per_cred[0] & 8)
			credit_add = 1;
	}
	if (val & toggled & 0x02)
	{
		io[chip].coins[1]++;
		if (io[chip].coins[1] >= (io[chip].coins_per_cred[1] & 7))
		{
			credit_add = io[chip].creds_per_coin[1] - (io[chip].coins_per_cred[1] >> 3);
			io[chip].coins[1] -= io[chip].coins_per_cred[1] & 7;
		}
		else if (io[chip].coins_per_cred[1] & 8)
			credit_add = 1;
	}
	if (val & toggled & 0x08)
	{
		credit_add = 1;
	}

	val = ~READ_PORT(space,3);	// pins 30-33
	toggled = val ^ io[chip].lastbuttons;
	io[chip].lastbuttons = val;

	/* check start buttons, only if the game allows */
	if (IORAM_READ(9) == 0)
	// the other argument is IORAM_READ(10) = 1, meaning unknown
	{
		if (val & toggled & 0x04)
		{
			if (io[chip].credits >= 1) credit_sub = 1;
		}
		else if (val & toggled & 0x08)
		{
			if (io[chip].credits >= 2) credit_sub = 2;
		}
	}

	io[chip].credits += credit_add - credit_sub;

	IORAM_WRITE(0 ^ swap, io[chip].credits / 10);	// BCD credits
	IORAM_WRITE(1 ^ swap, io[chip].credits % 10);	// BCD credits
	IORAM_WRITE(2 ^ swap, credit_add);	// credit increment (coin inputs)
	IORAM_WRITE(3 ^ swap, credit_sub);	// credit decrement (start buttons)
	IORAM_WRITE(4, ~READ_PORT(space,1));	// pins 22-25
	button = ((val & 0x05) << 1) | (val & toggled & 0x05);
	IORAM_WRITE(5, button);	// pins 30 & 32 normal and impulse
	IORAM_WRITE(6, ~READ_PORT(space,2));	// pins 26-29
	button = (val & 0x0a) | ((val & toggled & 0x0a) >> 1);
	IORAM_WRITE(7, button);	// pins 31 & 33 normal and impulse
}



static void namco_customio_56XX_run(running_machine *machine, int chip)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	LOG(("execute 56XX %d mode %d\n",chip,IORAM_READ(8)));

	switch (IORAM_READ(8))
	{
		case 0:	// nop?
			break;

		case 1:	// read switch inputs
			IORAM_WRITE(0, ~READ_PORT(space,0));	// pins 38-41
			IORAM_WRITE(1, ~READ_PORT(space,1));	// pins 22-25
			IORAM_WRITE(2, ~READ_PORT(space,2));	// pins 26-29
			IORAM_WRITE(3, ~READ_PORT(space,3));	// pins 30-33

//popmessage("%x %x %x %x %x %x %x %x",IORAM_READ(8),IORAM_READ(9),IORAM_READ(10),IORAM_READ(11),IORAM_READ(12),IORAM_READ(13),IORAM_READ(14),IORAM_READ(15));

			WRITE_PORT(space,0,IORAM_READ(9));	// output to pins 13-16 (motos, pacnpal, gaplus)
			WRITE_PORT(space,1,IORAM_READ(10));	// output to pins 17-20 (gaplus)
			break;

		case 2:	// initialize coinage settings
			io[chip].coins_per_cred[0] = IORAM_READ(9);
			io[chip].creds_per_coin[0] = IORAM_READ(10);
			io[chip].coins_per_cred[1] = IORAM_READ(11);
			io[chip].creds_per_coin[1] = IORAM_READ(12);
			// IORAM_READ(13) = 1; meaning unknown - possibly a 3rd coin input? (there's a IPT_UNUSED bit in port A)
			// IORAM_READ(14) = 1; meaning unknown - possibly a 3rd coin input? (there's a IPT_UNUSED bit in port A)
			// IORAM_READ(15) = 0; meaning unknown
			break;

		case 4:	// druaga, digdug chip #1: read dip switches and inputs
				// superpac chip #0: process coin and start inputs, read switch inputs
			handle_coins(machine,chip,0);
			break;

		case 7:	// bootup check (liblrabl only)
			{
				// liblrabl chip #1: 9-15 = f 1 2 3 4 0 0, expects 2 = e
				// liblrabl chip #2: 9-15 = 0 1 4 5 5 0 0, expects 7 = 6
				IORAM_WRITE(2,0xe);
				IORAM_WRITE(7,0x6);
			}
			break;

		case 8:	// bootup check
			{
				int i,sum;

				// superpac: 9-15 = f f f f f f f, expects 0-1 = 6 9. 0x69 = f+f+f+f+f+f+f.
				// motos:    9-15 = f f f f f f f, expects 0-1 = 6 9. 0x69 = f+f+f+f+f+f+f.
				// phozon:   9-15 = 1 2 3 4 5 6 7, expects 0-1 = 1 c. 0x1c = 1+2+3+4+5+6+7
				sum = 0;
				for (i = 9;i < 16;i++)
					sum += IORAM_READ(i);
				IORAM_WRITE(0,sum >> 4);
				IORAM_WRITE(1,sum & 0xf);
			}
			break;

		case 9:	// read dip switches and inputs
			WRITE_PORT(space,0,0);	// set pin 13 = 0
			IORAM_WRITE(0, ~READ_PORT(space,0));	// pins 38-41, pin 13 = 0
			IORAM_WRITE(2, ~READ_PORT(space,1));	// pins 22-25, pin 13 = 0
			IORAM_WRITE(4, ~READ_PORT(space,2));	// pins 26-29, pin 13 = 0
			IORAM_WRITE(6, ~READ_PORT(space,3));	// pins 30-33, pin 13 = 0
			WRITE_PORT(space,0,1);	// set pin 13 = 1
			IORAM_WRITE(1, ~READ_PORT(space,0));	// pins 38-41, pin 13 = 1
			IORAM_WRITE(3, ~READ_PORT(space,1));	// pins 22-25, pin 13 = 1
			IORAM_WRITE(5, ~READ_PORT(space,2));	// pins 26-29, pin 13 = 1
			IORAM_WRITE(7, ~READ_PORT(space,3));	// pins 30-33, pin 13 = 1
			break;

		default:
			logerror("Namco I/O %d: unknown I/O mode %d\n",chip,IORAM_READ(8));
	}
}



static void namco_customio_59XX_run(running_machine *machine, int chip)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	LOG(("execute 59XX %d mode %d\n",chip,IORAM_READ(8)));

	switch (IORAM_READ(8))
	{
		case 0:	// nop?
			break;

		case 3:	// pacnpal chip #1: read dip switches and inputs
			IORAM_WRITE(4, ~READ_PORT(space,0));	// pins 38-41, pin 13 = 0 ?
			IORAM_WRITE(5, ~READ_PORT(space,2));	// pins 26-29 ?
			IORAM_WRITE(6, ~READ_PORT(space,1));	// pins 22-25 ?
			IORAM_WRITE(7, ~READ_PORT(space,3));	// pins 30-33
			break;

		default:
			logerror("Namco I/O %d: unknown I/O mode %d\n",chip,IORAM_READ(8));
	}
}



static void namco_customio_58XX_run(running_machine *machine, int chip)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	LOG(("execute 58XX %d mode %d\n",chip,IORAM_READ(8)));

	switch (IORAM_READ(8))
	{
		case 0:	// nop?
			break;

		case 1:	// read switch inputs
			IORAM_WRITE(4, ~READ_PORT(space,0));	// pins 38-41
			IORAM_WRITE(5, ~READ_PORT(space,1));	// pins 22-25
			IORAM_WRITE(6, ~READ_PORT(space,2));	// pins 26-29
			IORAM_WRITE(7, ~READ_PORT(space,3));	// pins 30-33

//popmessage("%x %x %x %x %x %x %x %x",IORAM_READ(8),IORAM_READ(9),IORAM_READ(10),IORAM_READ(11),IORAM_READ(12),IORAM_READ(13),IORAM_READ(14),IORAM_READ(15));

			WRITE_PORT(space,0,IORAM_READ(9));	// output to pins 13-16 (toypop)
			WRITE_PORT(space,1,IORAM_READ(10));	// output to pins 17-20 (toypop)
			break;

		case 2:	// initialize coinage settings
			io[chip].coins_per_cred[0] = IORAM_READ(9);
			io[chip].creds_per_coin[0] = IORAM_READ(10);
			io[chip].coins_per_cred[1] = IORAM_READ(11);
			io[chip].creds_per_coin[1] = IORAM_READ(12);
			// IORAM_READ(13) = 1; meaning unknown - possibly a 3rd coin input? (there's a IPT_UNUSED bit in port A)
			// IORAM_READ(14) = 0; meaning unknown - possibly a 3rd coin input? (there's a IPT_UNUSED bit in port A)
			// IORAM_READ(15) = 0; meaning unknown
			break;

		case 3:	// process coin and start inputs, read switch inputs
			handle_coins(machine,chip,2);
			break;

		case 4:	// read dip switches and inputs
			WRITE_PORT(space,0,0);	// set pin 13 = 0
			IORAM_WRITE(0, ~READ_PORT(space,0));	// pins 38-41, pin 13 = 0
			IORAM_WRITE(2, ~READ_PORT(space,1));	// pins 22-25, pin 13 = 0
			IORAM_WRITE(4, ~READ_PORT(space,2));	// pins 26-29, pin 13 = 0
			IORAM_WRITE(6, ~READ_PORT(space,3));	// pins 30-33, pin 13 = 0
			WRITE_PORT(space,0,1);	// set pin 13 = 1
			IORAM_WRITE(1, ~READ_PORT(space,0));	// pins 38-41, pin 13 = 1
			IORAM_WRITE(3, ~READ_PORT(space,1));	// pins 22-25, pin 13 = 1
			IORAM_WRITE(5, ~READ_PORT(space,2));	// pins 26-29, pin 13 = 1
			IORAM_WRITE(7, ~READ_PORT(space,3));	// pins 30-33, pin 13 = 1
			break;

		case 5:	// bootup check
			/* mode 5 values are checked against these numbers during power up
               mappy:  9-15 = 3 6 5 f a c e, expects 1-7 =   8 4 6 e d 9 d
               grobda: 9-15 = 2 3 4 5 6 7 8, expects 2 = f and 6 = c
               phozon: 9-15 = 0 1 2 3 4 5 6, expects 0-7 = 0 2 3 4 5 6 c a
               gaplus: 9-15 = f f f f f f f, expects 0-1 = f f

               This has been determined to be the result of repeated XORs,
               controlled by a 7-bit LFSR. The following algorithm should be
               equivalent to the original one (though probably less efficient).
               The first nibble of the result however is uncertain. It is usually
               0, but in some cases it toggles between 0 and F. We use a kludge
               to give Gaplus the F it expects.
            */
			{
				int i,n,rng,seed;
				#define NEXT(n) ((((n) & 1) ? (n) ^ 0x90 : (n)) >> 1)

				/* initialize the LFSR depending on the first two arguments */
				n = (IORAM_READ(9) * 16 + IORAM_READ(10)) & 0x7f;
				seed = 0x22;
				for (i = 0;i < n;i++)
					seed = NEXT(seed);

				/* calculate the answer */
				for (i = 1;i < 8;i++)
				{
					n = 0;
					rng = seed;
					if (rng & 1) { n ^= ~IORAM_READ(11); }
					rng = NEXT(rng);
					seed = rng;	// save state for next loop
					if (rng & 1) { n ^= ~IORAM_READ(10); }
					rng = NEXT(rng);
					if (rng & 1) { n ^= ~IORAM_READ(9); }
					rng = NEXT(rng);
					if (rng & 1) { n ^= ~IORAM_READ(15); }
					rng = NEXT(rng);
					if (rng & 1) { n ^= ~IORAM_READ(14); }
					rng = NEXT(rng);
					if (rng & 1) { n ^= ~IORAM_READ(13); }
					rng = NEXT(rng);
					if (rng & 1) { n ^= ~IORAM_READ(12); }

					IORAM_WRITE(i,~n);
				}
				IORAM_WRITE(0,0x0);
				/* kludge for gaplus */
				if (IORAM_READ(9) == 0xf) IORAM_WRITE(0,0xf);
			}
			break;

		default:
			logerror("Namco I/O %d: unknown I/O mode %d\n",chip,IORAM_READ(8));
	}
}



READ8_HANDLER( namcoio_r )
{
	// RAM is 4-bit wide; Pac & Pal requires the | 0xf0 otherwise Easter egg doesn't work
	offset &= 0x3f;

	LOG(("%04x: I/O read %d: mode %d, offset %d = %02x\n", cpu_get_pc(space->cpu), offset / 16, namcoio_ram[(offset & 0x30) + 8], offset & 0x0f, namcoio_ram[offset]&0x0f));

	return 0xf0 | namcoio_ram[offset];
}

WRITE8_HANDLER( namcoio_w )
{
	offset &= 0x3f;
	data &= 0x0f;	// RAM is 4-bit wide

	LOG(("%04x: I/O write %d: offset %d = %02x\n", cpu_get_pc(space->cpu), offset / 16, offset & 0x0f, data));

	namcoio_ram[offset] = data;
}

void namcoio_set_reset_line(int chipnum, int state)
{
	io[chipnum].reset = (state == ASSERT_LINE) ? 1 : 0;
	if (state != CLEAR_LINE)
	{
		/* reset internal registers */
		io[chipnum].credits = 0;
		io[chipnum].coins[0] = 0;
		io[chipnum].coins_per_cred[0] = 1;
		io[chipnum].creds_per_coin[0] = 1;
		io[chipnum].coins[1] = 0;
		io[chipnum].coins_per_cred[1] = 1;
		io[chipnum].creds_per_coin[1] = 1;
		io[chipnum].in_count = 0;
	}
}

static TIMER_CALLBACK( namcoio_run )
{
	switch (io[param].type)
	{
		case NAMCOIO_56XX:
			namco_customio_56XX_run(machine, param);
			break;
		case NAMCOIO_58XX:
			namco_customio_58XX_run(machine, param);
			break;
		case NAMCOIO_59XX:
			namco_customio_59XX_run(machine, param);
			break;
	}
}

void namcoio_set_irq_line(running_machine *machine, int chipnum, int state)
{
	if (chipnum < MAX_NAMCOIO && state != CLEAR_LINE && !io[chipnum].reset)
	{
		/* give the cpu a tiny bit of time to write the command before processing it */
		timer_set(machine, ATTOTIME_IN_USEC(50), NULL, chipnum, namcoio_run);
	}
}

static void namcoio_state_save(running_machine *machine, int chipnum)
{
	state_save_register_item_pointer(machine, "namcoio", NULL, chipnum, ((UINT8 *) (&namcoio_ram[chipnum * 16])), 16);
	state_save_register_item(machine, "namcoio", NULL, chipnum, io[chipnum].reset);
	state_save_register_item(machine, "namcoio", NULL, chipnum, io[chipnum].lastcoins);
	state_save_register_item(machine, "namcoio", NULL, chipnum, io[chipnum].lastbuttons);
	state_save_register_item(machine, "namcoio", NULL, chipnum, io[chipnum].credits);
	state_save_register_item_array(machine, "namcoio", NULL, chipnum, io[chipnum].coins);
	state_save_register_item_array(machine, "namcoio", NULL, chipnum, io[chipnum].coins_per_cred);
	state_save_register_item_array(machine, "namcoio", NULL, chipnum, io[chipnum].creds_per_coin);
	state_save_register_item(machine, "namcoio", NULL, chipnum, io[chipnum].in_count);
}

void namcoio_init(running_machine *machine, int chipnum, int type, const struct namcoio_interface *intf, const char *device)
{
	if (chipnum < MAX_NAMCOIO)
	{
		io[chipnum].type = type;
		io[chipnum].device = device ? devtag_get_device(machine, device) : NULL;
		io[chipnum].in[0] = (intf && intf->in[0]) ? intf->in[0] : nop_r;
		io[chipnum].in[1] = (intf && intf->in[1]) ? intf->in[1] : nop_r;
		io[chipnum].in[2] = (intf && intf->in[2]) ? intf->in[2] : nop_r;
		io[chipnum].in[3] = (intf && intf->in[3]) ? intf->in[3] : nop_r;
		io[chipnum].out[0] = (intf && intf->out[0]) ? intf->out[0] : nop_w;
		io[chipnum].out[1] = (intf && intf->out[1]) ? intf->out[1] : nop_w;
		namcoio_state_save(machine, chipnum);
		namcoio_set_reset_line(chipnum,PULSE_LINE);
	}
}
