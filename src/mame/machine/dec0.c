// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

Data East machine functions - Bryan McPhail, mish@tendril.co.uk

* Control reads, protection chip emulations & cycle skipping patches

*******************************************************************************/

#include "emu.h"
#include "includes/dec0.h"
#include "cpu/h6280/h6280.h"
#include "cpu/mcs51/mcs51.h"


/******************************************************************************/

void dec0_state::machine_start()
{
	save_item(NAME(m_i8751_return));
	save_item(NAME(m_i8751_command));
	save_item(NAME(m_i8751_ports));
}

READ16_MEMBER(dec0_state::dec0_controls_r)
{
	switch (offset<<1)
	{
		case 0: /* Player 1 & 2 joystick & buttons */
			return ioport("INPUTS")->read();

		case 2: /* Credits, start buttons */
			return ioport("SYSTEM")->read();

		case 4: /* Byte 4: Dipswitch bank 2, Byte 5: Dipswitch Bank 1 */
			return ioport("DSW")->read();

		case 8: /* Intel 8751 mc, Bad Dudes & Heavy Barrel only */
			//logerror("CPU #0 PC %06x: warning - read i8751 %06x - %04x\n", space.device().safe_pc(), 0x30c000+offset, m_i8751_return);
			return m_i8751_return;
	}

	logerror("CPU #0 PC %06x: warning - read unmapped memory address %06x\n", space.device().safe_pc(), 0x30c000+offset);
	return ~0;
}

/******************************************************************************/

READ16_MEMBER(dec0_state::dec0_rotary_r)
{
	switch (offset<<1)
	{
		case 0: /* Player 1 rotary */
			return ~(1 << ioport("AN0")->read());

		case 8: /* Player 2 rotary */
			return ~(1 << ioport("AN1")->read());

		default:
			logerror("Unknown rotary read at 300000 %02x\n", offset);
	}

	return 0;
}

/******************************************************************************/

READ16_MEMBER(dec0_state::midres_controls_r)
{
	switch (offset<<1)
	{
		case 0: /* Player 1 Joystick + start, Player 2 Joystick + start */
			return ioport("INPUTS")->read();

		case 2: /* Dipswitches */
			return ioport("DSW")->read();

		case 4: /* Player 1 rotary */
			return ~(1 << ioport("AN0")->read());

		case 6: /* Player 2 rotary */
			return ~(1 << ioport("AN1")->read());

		case 8: /* Credits, start buttons */
			return ioport("SYSTEM")->read();

		case 12:
			return 0;   /* ?? watchdog ?? */
	}

	logerror("PC %06x unknown control read at %02x\n", space.device().safe_pc(), 0x180000+offset);
	return ~0;
}

/******************************************************************************/

/******************************************************************************/


READ8_MEMBER(dec0_state::hippodrm_prot_r)
{
//logerror("6280 PC %06x - Read %06x\n",cpu_getpc(),offset+0x1d0000);
	if (m_hippodrm_lsb==0x45) return 0x4e;
	if (m_hippodrm_lsb==0x92) return 0x15;
	return 0;
}

WRITE8_MEMBER(dec0_state::hippodrm_prot_w)
{
	switch (offset) {
		case 4: m_hippodrm_msb=data; break;
		case 5: m_hippodrm_lsb=data; break;
	}
//logerror("6280 PC %06x - Wrote %06x to %04x\n",cpu_getpc(),data,offset+0x1d0000);
}

READ16_MEMBER(dec0_state::hippodrm_68000_share_r)
{
	if (offset==0) space.device().execute().yield(); /* A wee helper */
	return m_hippodrm_shared_ram[offset]&0xff;
}

WRITE16_MEMBER(dec0_state::hippodrm_68000_share_w)
{
	m_hippodrm_shared_ram[offset]=data&0xff;
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


READ8_MEMBER(dec0_state::dec0_mcu_port_r)
{
	int latchEnable=m_i8751_ports[2]>>4;

	// P0 connected to 4 latches
	if (offset==0)
	{
		if ((latchEnable&1)==0)
			return m_i8751_command>>8;
		else if ((latchEnable&2)==0)
			return m_i8751_command&0xff;
		else if ((latchEnable&4)==0)
			return m_i8751_return>>8;
		else if ((latchEnable&8)==0)
			return m_i8751_return&0xff;
	}

	return 0xff;
}

WRITE8_MEMBER(dec0_state::dec0_mcu_port_w)
{
	m_i8751_ports[offset]=data;

	if (offset==2)
	{
		if ((data&0x4)==0)
			m_maincpu->set_input_line(5, HOLD_LINE);
		if ((data&0x8)==0)
			m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
		if ((data&0x40)==0)
			m_i8751_return=(m_i8751_return&0xff00)|(m_i8751_ports[0]);
		if ((data&0x80)==0)
			m_i8751_return=(m_i8751_return&0xff)|(m_i8751_ports[0]<<8);
	}
}

void dec0_state::baddudes_i8751_write(int data)
{
	m_i8751_return=0;

	switch (data&0xffff) {
		case 0x714: m_i8751_return=0x700; break;
		case 0x73b: m_i8751_return=0x701; break;
		case 0x72c: m_i8751_return=0x702; break;
		case 0x73f: m_i8751_return=0x703; break;
		case 0x755: m_i8751_return=0x704; break;
		case 0x722: m_i8751_return=0x705; break;
		case 0x72b: m_i8751_return=0x706; break;
		case 0x724: m_i8751_return=0x707; break;
		case 0x728: m_i8751_return=0x708; break;
		case 0x735: m_i8751_return=0x709; break;
		case 0x71d: m_i8751_return=0x70a; break;
		case 0x721: m_i8751_return=0x70b; break;
		case 0x73e: m_i8751_return=0x70c; break;
		case 0x761: m_i8751_return=0x70d; break;
		case 0x753: m_i8751_return=0x70e; break;
		case 0x75b: m_i8751_return=0x70f; break;
	}

	if (!m_i8751_return) logerror("%s: warning - write unknown command %02x to 8571\n",machine().describe_context(),data);
	m_maincpu->set_input_line(5, HOLD_LINE);
}

void dec0_state::birdtry_i8751_write(int data)
{
	static int  pwr,
				hgt;

	m_i8751_return=0;

	switch(data&0xffff) {
		/*"Sprite control"*/
		case 0x22a: m_i8751_return = 0x200;    break;

		/* Gives an O.B. otherwise (it must be > 0xb0 )*/
		case 0x3c7: m_i8751_return = 0x7ff;    break;

		/*Enables shot checks*/
		case 0x33c: m_i8751_return = 0x200;     break;

		/*Used on the title screen only(???)*/
		case 0x31e: m_i8751_return = 0x200;     break;

/*  0x100-0x10d values are for club power meters(1W=0x100<<-->>PT=0x10d).    *
 *  Returned value to i8751 doesn't matter,but send the result to 0x481.     *
 *  Lower the value,stronger is the power.                                   */
		case 0x100: pwr = 0x30;             break; /*1W*/
		case 0x101: pwr = 0x34;             break; /*3W*/
		case 0x102: pwr = 0x38;             break; /*4W*/
		case 0x103: pwr = 0x3c;             break; /*1I*/
		case 0x104: pwr = 0x40;             break; /*3I*/
		case 0x105: pwr = 0x44;             break; /*4I*/
		case 0x106: pwr = 0x48;             break; /*5I*/
		case 0x107: pwr = 0x4c;             break; /*6I*/
		case 0x108: pwr = 0x50;             break; /*7I*/
		case 0x109: pwr = 0x54;             break; /*8I*/
		case 0x10a: pwr = 0x58;             break; /*9I*/
		case 0x10b: pwr = 0x5c;             break; /*PW*/
		case 0x10c: pwr = 0x60;             break; /*SW*/
		case 0x10d: pwr = 0x80;             break; /*PT*/
		case 0x481: m_i8751_return = pwr;     break; /*Power meter*/

/*  0x200-0x20f values are for shot height(STRONG=0x200<<-->>WEAK=0x20f).    *
 *  Returned value to i8751 doesn't matter,but send the result to 0x534.     *
 *  Higher the value,stronger is the height.                                 */
		case 0x200: hgt = 0x5c0;            break; /*H*/
		case 0x201: hgt = 0x580;            break; /*|*/
		case 0x202: hgt = 0x540;            break; /*|*/
		case 0x203: hgt = 0x500;            break; /*|*/
		case 0x204: hgt = 0x4c0;            break; /*|*/
		case 0x205: hgt = 0x480;            break; /*|*/
		case 0x206: hgt = 0x440;            break; /*|*/
		case 0x207: hgt = 0x400;            break; /*M*/
		case 0x208: hgt = 0x3c0;            break; /*|*/
		case 0x209: hgt = 0x380;            break; /*|*/
		case 0x20a: hgt = 0x340;            break; /*|*/
		case 0x20b: hgt = 0x300;            break; /*|*/
		case 0x20c: hgt = 0x2c0;            break; /*|*/
		case 0x20d: hgt = 0x280;            break; /*|*/
		case 0x20e: hgt = 0x240;            break; /*|*/
		case 0x20f: hgt = 0x200;            break; /*L*/
		case 0x534: m_i8751_return = hgt;    break; /*Shot height*/

		/*At the ending screen(???)*/
		//case 0x3b4: m_i8751_return = 0;       break;

		/*These are activated after a shot (???)*/
		case 0x6ca: m_i8751_return = 0xff;      break;
		case 0x7ff: m_i8751_return = 0x200;     break;
		default: logerror("%s: warning - write unknown command %02x to 8571\n",machine().describe_context(),data);
	}
	m_maincpu->set_input_line(5, HOLD_LINE);
}

void dec0_state::dec0_i8751_write(int data)
{
	m_i8751_command=data;

	/* Writes to this address cause an IRQ to the i8751 microcontroller */
	if (m_game == 1) m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	if (m_game == 2) baddudes_i8751_write(data);
	if (m_game == 3) birdtry_i8751_write(data);

	//logerror("%s: warning - write %02x to i8751\n",machine().describe_context(),data);
}

void dec0_state::dec0_i8751_reset()
{
	m_i8751_return=m_i8751_command=0;
}

/******************************************************************************/

WRITE16_MEMBER(dec0_state::sprite_mirror_w)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

/******************************************************************************/

READ16_MEMBER(dec0_state::robocop_68000_share_r)
{
//logerror("%08x: Share read %04x\n",space.device().safe_pc(),offset);

	return m_robocop_shared_ram[offset];
}

WRITE16_MEMBER(dec0_state::robocop_68000_share_w)
{
//  logerror("%08x: Share write %04x %04x\n",space.device().safe_pc(),offset,data);

	m_robocop_shared_ram[offset]=data&0xff;

	if (offset == 0x7ff) /* A control address - not standard ram */
		m_subcpu->set_input_line(0, HOLD_LINE);
}

/******************************************************************************/

void dec0_state::h6280_decrypt(const char *cputag)
{
	int i;
	UINT8 *RAM = memregion(cputag)->base();

	/* Read each byte, decrypt it */
	for (i = 0x00000; i < 0x10000; i++)
		RAM[i] = (RAM[i] & 0x7e) | ((RAM[i] & 0x1) << 7) | ((RAM[i] & 0x80) >> 7);
}

DRIVER_INIT_MEMBER(dec0_state,hippodrm)
{
	UINT8 *RAM = memregion("sub")->base();
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x180000, 0x18003f, read16_delegate(FUNC(dec0_state::hippodrm_68000_share_r),this), write16_delegate(FUNC(dec0_state::hippodrm_68000_share_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xffc800, 0xffcfff, write16_delegate(FUNC(dec0_state::sprite_mirror_w),this));

	h6280_decrypt("sub");

	/* The protection cpu has additional memory mapped protection! */
	RAM[0x189] = 0x60; /* RTS prot area */
	RAM[0x1af] = 0x60; /* RTS prot area */
	RAM[0x1db] = 0x60; /* RTS prot area */
	RAM[0x21a] = 0x60; /* RTS prot area */

	save_item(NAME(m_hippodrm_msb));
	save_item(NAME(m_hippodrm_lsb));
}

DRIVER_INIT_MEMBER(dec0_state,slyspy)
{
	UINT8 *RAM = memregion("audiocpu")->base();
	h6280_decrypt("audiocpu");

	/* Slyspy sound cpu has some protection */
	RAM[0xf2d] = 0xea;
	RAM[0xf2e] = 0xea;

	save_item(NAME(m_slyspy_state));
}

DRIVER_INIT_MEMBER(dec0_state,robocop)
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x180000, 0x180fff, read16_delegate(FUNC(dec0_state::robocop_68000_share_r),this), write16_delegate(FUNC(dec0_state::robocop_68000_share_w),this));
}

DRIVER_INIT_MEMBER(dec0_state,baddudes)
{
	m_game = 2;
}

DRIVER_INIT_MEMBER(dec0_state,hbarrel)
{
	m_game = 1;
}

DRIVER_INIT_MEMBER(dec0_state,birdtry)
{
	m_game=3;
}
