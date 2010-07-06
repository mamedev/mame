/*******************************************************************************

Data East machine functions - Bryan McPhail, mish@tendril.co.uk

* Control reads, protection chip emulations & cycle skipping patches

*******************************************************************************/

#include "emu.h"
#include "includes/dec0.h"
#include "cpu/h6280/h6280.h"
#include "cpu/mcs51/mcs51.h"

static int GAME,i8751_return,i8751_command,slyspy_state;

/******************************************************************************/

READ16_HANDLER( dec0_controls_r )
{
	switch (offset<<1)
	{
		case 0: /* Player 1 & 2 joystick & buttons */
			return input_port_read(space->machine, "INPUTS");

		case 2: /* Credits, start buttons */
			return input_port_read(space->machine, "SYSTEM");

		case 4: /* Byte 4: Dipswitch bank 2, Byte 5: Dipswitch Bank 1 */
			return input_port_read(space->machine, "DSW");

		case 8: /* Intel 8751 mc, Bad Dudes & Heavy Barrel only */
			//logerror("CPU #0 PC %06x: warning - read i8751 %06x - %04x\n", cpu_get_pc(space->cpu), 0x30c000+offset, i8751_return);
			return i8751_return;
	}

	logerror("CPU #0 PC %06x: warning - read unmapped memory address %06x\n", cpu_get_pc(space->cpu), 0x30c000+offset);
	return ~0;
}

/******************************************************************************/

READ16_HANDLER( dec0_rotary_r )
{
	switch (offset<<1)
	{
		case 0: /* Player 1 rotary */
			return ~(1 << input_port_read(space->machine, "AN0"));

		case 8: /* Player 2 rotary */
			return ~(1 << input_port_read(space->machine, "AN1"));

		default:
			logerror("Unknown rotary read at 300000 %02x\n", offset);
	}

	return 0;
}

/******************************************************************************/

READ16_HANDLER( midres_controls_r )
{
	switch (offset<<1)
	{
		case 0: /* Player 1 Joystick + start, Player 2 Joystick + start */
			return input_port_read(space->machine, "INPUTS");

		case 2: /* Dipswitches */
			return input_port_read(space->machine, "DSW");

		case 4: /* Player 1 rotary */
			return ~(1 << input_port_read(space->machine, "AN0"));

		case 6: /* Player 2 rotary */
			return ~(1 << input_port_read(space->machine, "AN1"));

		case 8: /* Credits, start buttons */
			return input_port_read(space->machine, "SYSTEM");

		case 12:
			return 0;	/* ?? watchdog ?? */
	}

	logerror("PC %06x unknown control read at %02x\n", cpu_get_pc(space->cpu), 0x180000+offset);
	return ~0;
}

/******************************************************************************/

READ16_HANDLER( slyspy_controls_r )
{
	switch (offset<<1)
	{
		case 0: /* Dip Switches */
			return input_port_read(space->machine, "DSW");

		case 2: /* Player 1 & Player 2 joysticks & fire buttons */
			return input_port_read(space->machine, "INPUTS");

		case 4: /* Credits */
			return input_port_read(space->machine, "SYSTEM");
	}

	logerror("Unknown control read at 30c000 %d\n", offset);
	return ~0;
}

READ16_HANDLER( slyspy_protection_r )
{
	/* These values are for Boulderdash, I have no idea what they do in Slyspy */
	switch (offset<<1) {
		case 0: 	return 0;
		case 2: 	return 0x13;
		case 4:		return 0;
		case 6:		return 0x2;
	}

	logerror("%04x, Unknown protection read at 30c000 %d\n", cpu_get_pc(space->cpu), offset);
	return 0;
}

/*
    The memory map in Sly Spy can change between 4 states according to the protection!

    Default state (called by Traps 1, 3, 4, 7, C)

    240000 - 24001f = control   (Playfield 2 area)
    242000 - 24207f = colscroll
    242400 - 2425ff = rowscroll
    246000 - 2467ff = data

    248000 - 24801f = control  (Playfield 1 area)
    24c000 - 24c07f = colscroll
    24c400 - 24c4ff = rowscroll
    24e000 - 24e7ff = data

    State 1 (Called by Trap 9) uses this memory map:

    248000 = pf1 data
    24c000 = pf2 data

    State 2 (Called by Trap A) uses this memory map:

    240000 = pf2 data
    242000 = pf1 data
    24e000 = pf1 data

    State 3 (Called by Trap B) uses this memory map:

    240000 = pf1 data
    248000 = pf2 data

*/

WRITE16_HANDLER( slyspy_state_w )
{
	slyspy_state=0;
}

READ16_HANDLER( slyspy_state_r )
{
	slyspy_state++;
	slyspy_state=slyspy_state%4;
	return 0; /* Value doesn't mater */
}

WRITE16_HANDLER( slyspy_240000_w )
{
	switch (slyspy_state) {
		case 0x3:
			dec0_pf1_data_w(space,offset,data,mem_mask);
			return;
		case 0x2:
			dec0_pf2_data_w(space,offset,data,mem_mask);
			return;
		case 0x0:
			if (offset<0x8) dec0_pf2_control_0_w(space,offset,data,mem_mask);
			else if (offset<0x10) dec0_pf2_control_1_w(space,offset-0x8,data,mem_mask);
			return;
	}
	logerror("Wrote to 240000 %02x at %04x %04x (Trap %02x)\n",offset,cpu_get_pc(space->cpu),data,slyspy_state);
}

WRITE16_HANDLER( slyspy_242000_w )
{
	switch (slyspy_state) {
		case 0x2: /* Trap A */
			dec0_pf1_data_w(space,offset,data,mem_mask);
			return;
		case 0x0: /* Trap C */
			if (offset<0x40) COMBINE_DATA(&dec0_pf2_colscroll[offset]);
			else if (offset<0x300) COMBINE_DATA(&dec0_pf2_rowscroll[offset-0x200]);
			return;
	}
	logerror("Wrote to 242000 %02x at %04x %04x (Trap %02x)\n",offset,cpu_get_pc(space->cpu),data,slyspy_state);
}

WRITE16_HANDLER( slyspy_246000_w )
{
	switch (slyspy_state) {
		case 0x0:
			dec0_pf2_data_w(space,offset,data,mem_mask);
			return;
	}
	logerror("Wrote to 246000 %02x at %04x %04x (Trap %02x)\n",offset,cpu_get_pc(space->cpu),data,slyspy_state);
}

WRITE16_HANDLER( slyspy_248000_w )
{
	switch (slyspy_state) {
		case 0x1:
			dec0_pf1_data_w(space,offset,data,mem_mask);
			return;
		case 0x3:
			dec0_pf2_data_w(space,offset,data,mem_mask);
			return;
		case 0x0:
			if (offset<0x8) dec0_pf1_control_0_w(space,offset,data,mem_mask);
			else if (offset<0x10) dec0_pf1_control_1_w(space,offset-0x8,data,mem_mask);
			return;
	}
	logerror("Wrote to 248000 %02x at %04x %04x (Trap %02x)\n",offset,cpu_get_pc(space->cpu),data,slyspy_state);
}

WRITE16_HANDLER( slyspy_24c000_w )
{
	switch (slyspy_state) {
		case 0x1: /* Trap 9 */
			dec0_pf2_data_w(space,offset,data,mem_mask);
			return;
		case 0x0: /* Trap C */
			if (offset<0x40) COMBINE_DATA(&dec0_pf1_colscroll[offset]);
			else if (offset<0x300) COMBINE_DATA(&dec0_pf1_rowscroll[offset-0x200]);
			return;
	}
	logerror("Wrote to 24c000 %02x at %04x %04x (Trap %02x)\n",offset,cpu_get_pc(space->cpu),data,slyspy_state);
}

WRITE16_HANDLER( slyspy_24e000_w )
{
	switch (slyspy_state) {
		case 0x2:
		case 0x0:
			dec0_pf1_data_w(space,offset,data,mem_mask);
			return;
	}
	logerror("Wrote to 24e000 %02x at %04x %04x (Trap %02x)\n",offset,cpu_get_pc(space->cpu),data,slyspy_state);
}

/******************************************************************************/

static int share[0xff];
static int hippodrm_msb,hippodrm_lsb;

READ8_HANDLER( hippodrm_prot_r )
{
//logerror("6280 PC %06x - Read %06x\n",cpu_getpc(),offset+0x1d0000);
	if (hippodrm_lsb==0x45) return 0x4e;
	if (hippodrm_lsb==0x92) return 0x15;
	return 0;
}

WRITE8_HANDLER( hippodrm_prot_w )
{
	switch (offset) {
		case 4:	hippodrm_msb=data; break;
		case 5:	hippodrm_lsb=data; break;
	}
//logerror("6280 PC %06x - Wrote %06x to %04x\n",cpu_getpc(),data,offset+0x1d0000);
}

READ8_HANDLER( hippodrm_shared_r )
{
	return share[offset];
}

WRITE8_HANDLER( hippodrm_shared_w )
{
	share[offset]=data;
}

static READ16_HANDLER( hippodrm_68000_share_r )
{
	if (offset==0) cpu_yield(space->cpu); /* A wee helper */
	return share[offset]&0xff;
}

static WRITE16_HANDLER( hippodrm_68000_share_w )
{
	share[offset]=data&0xff;
}

/******************************************************************************/

/*
    Heavy Barrel I8751 connections

    P0.0 - P0.7 <-> 4 * LS374 latches 8B,8C,7B,7C

    P1.0    -> MIXFLG1
    P1.1    -> MIXFLG2
    P1.2    -> B0FLG
    P1.3    -> B1FLG1
    P1.4    -> B1FLG2
    P1.5    -> SOUNDFLG1
    P1.6    -> SOUNDFLG2

    P2.0    -> B2FLG 0
    P2.1    -> B2FLG 1
    P2.2    <- SEL2
    P2.3    -> acknowledge INT1
    P2.4    -> Enable latch 7B
    P2.5    -> Enable latch 8B
    P2.6    -> Enable latch 8C
    P2.7    -> Enable latch 7C

    P3.0    -> CRBACK0
    P3.1    -> CRBACK1
    P3.2    -> CRBACK2
    P3.3    <- /INT1
    P3.5    <- SEL3
    P3.6    <- SEL4
    P3.7    <- SEL5

    The outputs to the graphics & audio hardware are not directly emulated, but the
    values are not known to change after bootup.
*/

static UINT8 i8751_ports[4];

READ8_HANDLER(dec0_mcu_port_r )
{
	int latchEnable=i8751_ports[2]>>4;

	// P0 connected to 4 latches
	if (offset==0)
	{
		if ((latchEnable&1)==0)
			return i8751_command>>8;
		else if ((latchEnable&2)==0)
			return i8751_command&0xff;
		else if ((latchEnable&4)==0)
			return i8751_return>>8;
		else if ((latchEnable&8)==0)
			return i8751_return&0xff;
	}

	return 0xff;
}

WRITE8_HANDLER(dec0_mcu_port_w )
{
	i8751_ports[offset]=data;

	if (offset==2)
	{
		if ((data&0x4)==0)
			cputag_set_input_line(space->machine, "maincpu", 5, HOLD_LINE);
		if ((data&0x8)==0)
			cputag_set_input_line(space->machine, "mcu", MCS51_INT1_LINE, CLEAR_LINE);
		if ((data&0x40)==0)
			i8751_return=(i8751_return&0xff00)|(i8751_ports[0]);
		if ((data&0x80)==0)
			i8751_return=(i8751_return&0xff)|(i8751_ports[0]<<8);
	}
}

static void baddudes_i8751_write(running_machine *machine, int data)
{
	i8751_return=0;

	switch (data&0xffff) {
		case 0x714: i8751_return=0x700; break;
		case 0x73b: i8751_return=0x701; break;
		case 0x72c: i8751_return=0x702; break;
		case 0x73f: i8751_return=0x703; break;
		case 0x755: i8751_return=0x704; break;
		case 0x722: i8751_return=0x705; break;
		case 0x72b: i8751_return=0x706; break;
		case 0x724: i8751_return=0x707; break;
		case 0x728: i8751_return=0x708; break;
		case 0x735: i8751_return=0x709; break;
		case 0x71d: i8751_return=0x70a; break;
		case 0x721: i8751_return=0x70b; break;
		case 0x73e: i8751_return=0x70c; break;
		case 0x761: i8751_return=0x70d; break;
		case 0x753: i8751_return=0x70e; break;
		case 0x75b: i8751_return=0x70f; break;
	}

	if (!i8751_return) logerror("%s: warning - write unknown command %02x to 8571\n",cpuexec_describe_context(machine),data);
	cputag_set_input_line(machine, "maincpu", 5, HOLD_LINE);
}

static void birdtry_i8751_write(running_machine *machine, int data)
{
	static int	pwr,
				hgt;

	i8751_return=0;

	switch(data&0xffff) {
		/*"Sprite control"*/
		case 0x22a:	i8751_return = 0x200;	  break;

		/* Gives an O.B. otherwise (it must be > 0xb0 )*/
		case 0x3c7:	i8751_return = 0x7ff;	  break;

		/*Enables shot checks*/
		case 0x33c: i8751_return = 0x200;     break;

		/*Used on the title screen only(???)*/
		case 0x31e: i8751_return = 0x200;     break;

/*  0x100-0x10d values are for club power meters(1W=0x100<<-->>PT=0x10d).    *
 *  Returned value to i8751 doesn't matter,but send the result to 0x481.     *
 *  Lower the value,stronger is the power.                                   */
		case 0x100: pwr = 0x30; 			break; /*1W*/
		case 0x101: pwr = 0x34; 			break; /*3W*/
		case 0x102: pwr = 0x38; 			break; /*4W*/
		case 0x103: pwr = 0x3c; 			break; /*1I*/
		case 0x104: pwr = 0x40; 			break; /*3I*/
		case 0x105: pwr = 0x44; 			break; /*4I*/
		case 0x106: pwr = 0x48; 			break; /*5I*/
		case 0x107: pwr = 0x4c; 			break; /*6I*/
		case 0x108: pwr = 0x50; 			break; /*7I*/
		case 0x109: pwr = 0x54; 			break; /*8I*/
		case 0x10a: pwr = 0x58; 			break; /*9I*/
		case 0x10b: pwr = 0x5c; 			break; /*PW*/
		case 0x10c: pwr = 0x60; 			break; /*SW*/
		case 0x10d: pwr = 0x80; 			break; /*PT*/
		case 0x481: i8751_return = pwr;     break; /*Power meter*/

/*  0x200-0x20f values are for shot height(STRONG=0x200<<-->>WEAK=0x20f).    *
 *  Returned value to i8751 doesn't matter,but send the result to 0x534.     *
 *  Higher the value,stronger is the height.                                 */
		case 0x200: hgt = 0x5c0;			break; /*H*/
		case 0x201: hgt = 0x580;			break; /*|*/
		case 0x202: hgt = 0x540;			break; /*|*/
		case 0x203: hgt = 0x500;			break; /*|*/
		case 0x204: hgt = 0x4c0;			break; /*|*/
		case 0x205: hgt = 0x480;			break; /*|*/
		case 0x206: hgt = 0x440;			break; /*|*/
		case 0x207: hgt = 0x400;			break; /*M*/
		case 0x208: hgt = 0x3c0;			break; /*|*/
		case 0x209: hgt = 0x380;			break; /*|*/
		case 0x20a: hgt = 0x340;			break; /*|*/
		case 0x20b: hgt = 0x300;			break; /*|*/
		case 0x20c: hgt = 0x2c0;			break; /*|*/
		case 0x20d: hgt = 0x280;			break; /*|*/
		case 0x20e: hgt = 0x240;			break; /*|*/
		case 0x20f: hgt = 0x200;			break; /*L*/
		case 0x534: i8751_return = hgt; 	break; /*Shot height*/

		/*At the ending screen(???)*/
		//case 0x3b4: i8751_return = 0;       break;

		/*These are activated after a shot (???)*/
		case 0x6ca: i8751_return = 0xff;      break;
		case 0x7ff: i8751_return = 0x200;     break;
		default: logerror("%s: warning - write unknown command %02x to 8571\n",cpuexec_describe_context(machine),data);
	}
	cputag_set_input_line(machine, "maincpu", 5, HOLD_LINE);
}

void dec0_i8751_write(running_machine *machine, int data)
{
	i8751_command=data;

	/* Writes to this address cause an IRQ to the i8751 microcontroller */
	if (GAME == 1) cputag_set_input_line(machine, "mcu", MCS51_INT1_LINE, ASSERT_LINE);
	if (GAME == 2) baddudes_i8751_write(machine, data);
	if (GAME == 3) birdtry_i8751_write(machine, data);

	//logerror("%s: warning - write %02x to i8751\n",cpuexec_describe_context(machine),data);
}

void dec0_i8751_reset(void)
{
	i8751_return=i8751_command=0;
}

/******************************************************************************/

static WRITE16_HANDLER( sprite_mirror_w )
{
	COMBINE_DATA(&space->machine->generic.spriteram.u16[offset]);
}

/******************************************************************************/

static READ16_HANDLER( robocop_68000_share_r )
{
//logerror("%08x: Share read %04x\n",cpu_get_pc(space->cpu),offset);

	return robocop_shared_ram[offset];
}

static WRITE16_HANDLER( robocop_68000_share_w )
{
//  logerror("%08x: Share write %04x %04x\n",cpu_get_pc(space->cpu),offset,data);

	robocop_shared_ram[offset]=data&0xff;

	if (offset == 0x7ff) /* A control address - not standard ram */
		cputag_set_input_line(space->machine, "sub", 0, HOLD_LINE);
}

/******************************************************************************/

static void h6280_decrypt(running_machine *machine, const char *cputag)
{
	int i;
	UINT8 *RAM = memory_region(machine, cputag);

	/* Read each byte, decrypt it */
	for (i = 0x00000; i < 0x10000; i++)
		RAM[i] = (RAM[i] & 0x7e) | ((RAM[i] & 0x1) << 7) | ((RAM[i] & 0x80) >> 7);
}

DRIVER_INIT( hippodrm )
{
	UINT8 *RAM = memory_region(machine, "sub");

	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x180000, 0x180fff, 0, 0, hippodrm_68000_share_r, hippodrm_68000_share_w);
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xffc800, 0xffcfff, 0, 0, sprite_mirror_w);

	h6280_decrypt(machine, "sub");

	/* The protection cpu has additional memory mapped protection! */
	RAM[0x189] = 0x60; /* RTS prot area */
	RAM[0x1af] = 0x60; /* RTS prot area */
	RAM[0x1db] = 0x60; /* RTS prot area */
	RAM[0x21a] = 0x60; /* RTS prot area */
}

DRIVER_INIT( slyspy )
{
	UINT8 *RAM = memory_region(machine, "audiocpu");

	h6280_decrypt(machine, "audiocpu");

	/* Slyspy sound cpu has some protection */
	RAM[0xf2d] = 0xea;
	RAM[0xf2e] = 0xea;
}

DRIVER_INIT( robocop )
{
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x180000, 0x180fff, 0, 0, robocop_68000_share_r, robocop_68000_share_w);
}

DRIVER_INIT( baddudes )
{
	GAME = 2;
}

DRIVER_INIT( hbarrel )
{
	GAME = 1;
}

DRIVER_INIT( birdtry )
{
	UINT8 *src, tmp;
	int i, j, k;

	GAME=3;

	src = memory_region(machine, "gfx4");

	/* some parts of the graphic have bytes swapped */
	for (k = 0;k < 0x70000;k += 0x20000)
	{
		for (i = 0x2000;i < 0x10000;i += 32)
		{
			for (j = 0;j < 16;j++)
			{
				tmp = src[k+i+j+16];
				src[k+i+j+16] = src[k+i+j];
				src[k+i+j] = tmp;
			}
		}
	}
}
