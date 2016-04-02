// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Karnov (USA version)                   (c) 1987 Data East USA
    Karnov (Japanese version)              (c) 1987 Data East Corporation
    Wonder Planet (Japanese version)       (c) 1987 Data East Corporation
    Chelnov (World version)                (c) 1987 Data East Corporation
    Chelnov (USA version)                  (c) 1988 Data East USA
    Chelnov (Japanese version)             (c) 1987 Data East Corporation


    Emulation by Bryan McPhail, mish@tendril.co.uk


    NOTE!  Karnov USA & Karnov Japan sets have different gameplay!
      and Chelnov USA & Chelnov Japan sets have different gameplay!

    These games use a 68000 main processor with a 6502, YM2203C and YM3526 for
    sound.  Karnov was a major pain to get going because of the
    'protection' on the main player sprite, probably connected to the Intel
    microcontroller on the board.  The game is very sensitive to the wrong values
    at the input ports...

    There is another Karnov rom set - a bootleg version of the Japanese roms with
    the Data East copyright removed - not supported because the original Japanese
    roms work fine.
    ^^ This should be added (DH, 30/03/11)

    One of the two color PROMs for chelnov and chelnoj is different; one is most
    likely a bad read, but I don't know which one.

    Thanks to Oliver Stabel <stabel@rhein-neckar.netsurf.de> for confirming some
    of the sprite & control information :)

    Cheats:

    Karnov - put 0x30 at 0x60201 to skip a level
    Chelnov - level number at 0x60189 - enter a value at cartoon intro


Stephh's notes (based on the games M68000 code and some tests) :

1) 'karnov' and its clones :

  - DSW1 bit 7 is called "No Die Mode" in the manual. It used to give invulnerability
    to shots (but not to falls), but it has no effect due to the "bra" instruction
    at 0x001334 ('karnov') or 0x00131a ('karnovj').

2) 'wndrplnt'

  - There is code at 0x01c000 which tests DSW2 bit 6 which seems to act as a "Freeze"
    Dip Switch, but this address doesn't seem to be reached. Leftover from another game ?
  - DSW2 bit 7 used to give invulnerability, but it has no effect due to
    the "andi.w  #$7fff, D5" instruction at 0x0011a2.

3) 'chelnov' and its clones :

3a) 'chelnov'

  - DSW2 bit 6 isn't tested in this set.
  - DSW2 bit 7 used to give invulnerability, but it has no effect due to
    the "andi.w  #$3fff, D5" instruction at 0x000ed0.

3b) 'chelnovu'

  - DSW2 bit 6 freezes the game (code at 0x000654), but when you turn
    the Dip Swicth back to "Off", it adds credits as if COIN1 was pressed.
    Is that the correct behaviour ?
  - Even if there is a "andi.w  #$ffff, D5" instruction at 0x000ef0,
    DSW2 bit 7 isn't tested in this set.

3c) 'chelnovj'

  - DSW2 bit 6 isn't tested in this set.
  - DSW2 bit 7 used to give invulnerability, but it has no effect due to
    the "andi.w  #$3fff, D5" instruction at 0x000ed8.

*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "includes/karnov.h"
#include "cpu/mcs51/mcs51.h"

/*************************************
 *
 *  Microcontroller emulation
 *
 *************************************/

/* Emulation of the protected microcontroller - for coins & general protection */
void karnov_state::karnov_i8751_w( int data )
{
	/* Pending coin operations may cause protection commands to be queued */
	if (m_i8751_needs_ack)
	{
		m_i8751_command_queue = data;
		return;
	}

	m_i8751_return = 0;

	if (data == 0x100 && m_microcontroller_id == KARNOV) /* USA version */
		m_i8751_return = 0x56b;

	if (data == 0x100 && m_microcontroller_id == KARNOVJ)    /* Japan version */
		m_i8751_return = 0x56a;

	if ((data & 0xf00) == 0x300)
		m_i8751_return = (data & 0xff) * 0x12; /* Player sprite mapping */

	/* I'm not sure the ones marked ^ appear in the right order */
	if (data == 0x400) m_i8751_return = 0x4000; /* Get The Map... */
	if (data == 0x402) m_i8751_return = 0x40a6; /* Ancient Ruins */
	if (data == 0x403) m_i8751_return = 0x4054; /* Forest... */
	if (data == 0x404) m_i8751_return = 0x40de; /* ^Rocky hills */
	if (data == 0x405) m_i8751_return = 0x4182; /* Sea */
	if (data == 0x406) m_i8751_return = 0x41ca; /* Town */
	if (data == 0x407) m_i8751_return = 0x421e; /* Desert */
	if (data == 0x401) m_i8751_return = 0x4138; /* ^Whistling wind */
	if (data == 0x408) m_i8751_return = 0x4276; /* ^Heavy Gates */

//  if (!m_i8751_return && data != 0x300) logerror("%s - Unknown Write %02x intel\n", machine().describe_context(), data);

	m_maincpu->set_input_line(6, HOLD_LINE); /* Signal main cpu task is complete */
	m_i8751_needs_ack = 1;
}

void karnov_state::wndrplnt_i8751_w( int data )
{
	/* The last command hasn't been ACK'd (probably a conflict with coin command) */
	if (m_i8751_needs_ack)
	{
		m_i8751_command_queue = data;
		return;
	}

	m_i8751_return=0;

	if (data == 0x100) m_i8751_return = 0x67a;
	if (data == 0x200) m_i8751_return = 0x214;
	if (data == 0x300) m_i8751_return = 0x17; /* Copyright text on title screen */
//  if (data == 0x300) m_i8751_return = 0x1; /* (USA) Copyright text on title screen */

	/* The game writes many values in the 0x600 range, but only a specific mask
	matters for the return value */
	if ((data & 0x600) == 0x600)
	{
		switch (data & 0x18)
		{
			case 0x00:  m_i8751_return = 0x4d53; break;
			case 0x08:  m_i8751_return = 0x4b54; break;
			case 0x10:  m_i8751_return = 0x5453; break;
			case 0x18:  m_i8751_return = 0x5341; break;
		}
	}
//  else logerror("%s - Unknown Write %02x intel\n", machine().describe_context(), data);

	/* These are 68k function call addresses - different address for each power-up */
	if (data == 0x400) m_i8751_return = 0x594;
	if (data == 0x401) m_i8751_return = 0x5ea;
	if (data == 0x402) m_i8751_return = 0x628;
	if (data == 0x403) m_i8751_return = 0x66c;
	if (data == 0x404) m_i8751_return = 0x6a4;
	if (data == 0x405) m_i8751_return = 0x6a4;
	if (data == 0x406) m_i8751_return = 0x6a4;

	/* This is 68k program code which is executed every frame */
	if (data == 0x50c) m_i8751_return = 0x13fc;
	if (data == 0x50b) m_i8751_return = 0x00ff;
	if (data == 0x50a) m_i8751_return = 0x0006;
	if (data == 0x509) m_i8751_return = 0x0000;
	if (data == 0x508) m_i8751_return = 0x4a39;
	if (data == 0x507) m_i8751_return = 0x0006;
	if (data == 0x506) m_i8751_return = 0x0000;
	if (data == 0x505) m_i8751_return = 0x66f8;
	if (data == 0x504) m_i8751_return = 0x4a39;
	if (data == 0x503) m_i8751_return = 0x000c;
	if (data == 0x502) m_i8751_return = 0x0003;
	if (data == 0x501) m_i8751_return = 0x6bf8;
	if (data == 0x500) m_i8751_return = 0x4e75;

	m_maincpu->set_input_line(6, HOLD_LINE); /* Signal main cpu task is complete */
	m_i8751_needs_ack = 1;
}

void karnov_state::chelnov_i8751_w( int data )
{
	/* Pending coin operations may cause protection commands to be queued */
	if (m_i8751_needs_ack)
	{
		m_i8751_command_queue = data;
		return;
	}

	m_i8751_return = 0;

	if (data == 0x200 && m_microcontroller_id == CHELNOV)    /* World version */
		m_i8751_return = 0x7736;

	if (data == 0x200 && m_microcontroller_id == CHELNOVU)   /* USA version */
		m_i8751_return = 0x783e;

	if (data == 0x200 && m_microcontroller_id == CHELNOVJ)   /* Japan version */
		m_i8751_return = 0x7734;

	if (data == 0x100 && m_microcontroller_id == CHELNOV)    /* World version */
		m_i8751_return = 0x71c;

	if (data == 0x100 && m_microcontroller_id == CHELNOVU)   /* USA version */
		m_i8751_return = 0x71b;

	if (data == 0x100 && m_microcontroller_id == CHELNOVJ)   /* Japan version */
		m_i8751_return = 0x71a;

	if ((data & 0xe000) == 0x6000) {
		if (data & 0x1000) {
			m_i8751_return = ((data & 0x0f) + ((data >> 4) & 0x0f)) * ((data >> 8) & 0x0f);
		} else {
			m_i8751_return = (data & 0x0f) * (((data >> 8) & 0x0f) + ((data >> 4) & 0x0f));
		}
	}

	if ((data & 0xf000) == 0x1000) m_i8751_level = 1;        /* Level 1 */
	if ((data & 0xf000) == 0x2000) m_i8751_level++;      /* Level Increment */

	if ((data & 0xf000) == 0x3000)
	{
		/* Sprite table mapping */
		int b = data & 0xff;
		switch (m_i8751_level)
		{
			case 1: /* Level 1, Sprite mapping tables */
				if (m_microcontroller_id == CHELNOVU) /* USA */
				{
					if (b < 2) m_i8751_return = 0;
					else if (b < 6) m_i8751_return = 1;
					else if (b < 0xb) m_i8751_return = 2;
					else if (b < 0xf) m_i8751_return = 3;
					else if (b < 0x13) m_i8751_return = 4;
					else m_i8751_return = 5;
				}
				else    /* Japan, World */
				{
					if (b < 3) m_i8751_return = 0;
					else if (b < 8) m_i8751_return = 1;
					else if (b < 0xc) m_i8751_return = 2;
					else if (b < 0x10) m_i8751_return = 3;
					else if (b < 0x19) m_i8751_return = 4;
					else if (b < 0x1b) m_i8751_return = 5;
					else if (b < 0x22) m_i8751_return = 6;
					else if (b < 0x28) m_i8751_return = 7;
					else m_i8751_return = 8;
				}
				break;
			case 2: /* Level 2, Sprite mapping tables, all sets are the same */
				if (b < 3) m_i8751_return = 0;
				else if (b < 9) m_i8751_return = 1;
				else if (b < 0x11) m_i8751_return = 2;
				else if (b < 0x1b) m_i8751_return = 3;
				else if (b < 0x21) m_i8751_return = 4;
				else if (b < 0x28) m_i8751_return = 5;
				else m_i8751_return = 6;
				break;
			case 3: /* Level 3, Sprite mapping tables, all sets are the same */
				if (b < 5) m_i8751_return = 0;
				else if (b < 9) m_i8751_return = 1;
				else if (b < 0xd) m_i8751_return = 2;
				else if (b < 0x11) m_i8751_return = 3;
				else if (b < 0x1b) m_i8751_return = 4;
				else if (b < 0x1c) m_i8751_return = 5;
				else if (b < 0x22) m_i8751_return = 6;
				else if (b < 0x27) m_i8751_return = 7;
				else m_i8751_return = 8;
				break;
			case 4: /* Level 4, Sprite mapping tables, all sets are the same */
				if (b < 4) m_i8751_return = 0;
				else if (b < 0xc) m_i8751_return = 1;
				else if (b < 0xf) m_i8751_return = 2;
				else if (b < 0x19) m_i8751_return = 3;
				else if (b < 0x1c) m_i8751_return = 4;
				else if (b < 0x22) m_i8751_return = 5;
				else if (b < 0x29) m_i8751_return = 6;
				else m_i8751_return = 7;
				break;
			case 5: /* Level 5, Sprite mapping tables, all sets are the same  */
				if (b < 7) m_i8751_return = 0;
				else if (b < 0xe) m_i8751_return = 1;
				else if (b < 0x14) m_i8751_return = 2;
				else if (b < 0x1a) m_i8751_return = 3;
				else if (b < 0x23) m_i8751_return = 4;
				else if (b < 0x27) m_i8751_return = 5;
				else m_i8751_return = 6;
				break;
			case 6: /* Level 6, Sprite mapping tables, all sets are the same  */
				if (b < 3) m_i8751_return = 0;
				else if (b < 0xb) m_i8751_return = 1;
				else if (b < 0x11) m_i8751_return = 2;
				else if (b < 0x17) m_i8751_return = 3;
				else if (b < 0x1d) m_i8751_return = 4;
				else if (b < 0x24) m_i8751_return = 5;
				else m_i8751_return = 6;
				break;
			case 7: /* Level 7, Sprite mapping tables, all sets are the same  */
				if (b < 5) m_i8751_return = 0;
				else if (b < 0xb) m_i8751_return = 1;
				else if (b < 0x11) m_i8751_return = 2;
				else if (b < 0x1a) m_i8751_return = 3;
				else if (b < 0x21) m_i8751_return = 4;
				else if (b < 0x27) m_i8751_return = 5;
				else m_i8751_return = 6;
				break;
		}
	}

	//  logerror("%s - Unknown Write %02x intel\n", machine().describe_context(), data);

	m_maincpu->set_input_line(6, HOLD_LINE); /* Signal main cpu task is complete */
	m_i8751_needs_ack = 1;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE16_MEMBER(karnov_state::karnov_control_w)
{
	/* Mnemonics filled in from the schematics, brackets are my comments */
	switch (offset << 1)
	{
		case 0: /* SECLR (Interrupt ack for Level 6 i8751 interrupt) */
			m_maincpu->set_input_line(6, CLEAR_LINE);

			if (m_i8751_needs_ack)
			{
				/* If a command and coin insert happen at once, then the i8751 will queue the coin command until the previous command is ACK'd */
				if (m_i8751_coin_pending)
				{
					m_i8751_return = m_i8751_coin_pending;
					m_maincpu->set_input_line(6, HOLD_LINE);
					m_i8751_coin_pending = 0;
				}
				else if (m_i8751_command_queue)
				{
					/* Pending control command - just write it back as SECREQ */
					m_i8751_needs_ack = 0;
					karnov_control_w(space, 3, m_i8751_command_queue, 0xffff);
					m_i8751_command_queue = 0;
				}
				else
				{
					m_i8751_needs_ack = 0;
				}
			}
			return;

		case 2: /* SONREQ (Sound CPU byte) */
			soundlatch_byte_w(space, 0, data & 0xff);
			m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
			break;

		case 4: /* DM (DMA to buffer spriteram) */
			m_spriteram->copy();
			break;

		case 6: /* SECREQ (Interrupt & Data to i8751) */
			if (m_microcontroller_id == KARNOV || m_microcontroller_id == KARNOVJ)
				karnov_i8751_w(data);
			if (m_microcontroller_id == CHELNOV || m_microcontroller_id == CHELNOVU || m_microcontroller_id == CHELNOVJ)
				chelnov_i8751_w(data);
			if (m_microcontroller_id == WNDRPLNT)
				wndrplnt_i8751_w(data);
			break;

		case 8: /* HSHIFT (9 bits) - Top bit indicates video flip */
			COMBINE_DATA(&m_scroll[0]);
			karnov_flipscreen_w(data >> 15);
			break;

		case 0xa: /* VSHIFT */
			COMBINE_DATA(&m_scroll[1]);
			break;

		case 0xc: /* SECR (Reset i8751) */
			logerror("Reset i8751\n");
			m_i8751_needs_ack = 0;
			m_i8751_coin_pending = 0;
			m_i8751_command_queue = 0;
			m_i8751_return = 0;
			break;

		case 0xe: /* INTCLR (Interrupt ack for Level 7 vbl interrupt) */
			m_maincpu->set_input_line(7, CLEAR_LINE);
			break;
	}
}

READ16_MEMBER(karnov_state::karnov_control_r)
{
	switch (offset << 1)
	{
		case 0:
			return ioport("P1_P2")->read();
		case 2: /* Start buttons & VBL */
			return ioport("SYSTEM")->read();
		case 4:
			return ioport("DSW")->read();
		case 6: /* i8751 return values */
			return m_i8751_return;
	}

	return ~0;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( karnov_map, AS_PROGRAM, 16, karnov_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x063fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x080000, 0x080fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x0a0000, 0x0a07ff) AM_RAM_WRITE(karnov_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0a0800, 0x0a0fff) AM_WRITE(karnov_videoram_w) /* Wndrplnt Mirror */
	AM_RANGE(0x0a1000, 0x0a17ff) AM_WRITEONLY AM_SHARE("pf_data")
	AM_RANGE(0x0a1800, 0x0a1fff) AM_WRITE(karnov_playfield_swap_w)
	AM_RANGE(0x0c0000, 0x0c0007) AM_READ(karnov_control_r)
	AM_RANGE(0x0c0000, 0x0c000f) AM_WRITE(karnov_control_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( karnov_sound_map, AS_PROGRAM, 8, karnov_state )
	AM_RANGE(0x0000, 0x05ff) AM_RAM
	AM_RANGE(0x0800, 0x0800) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0x1800, 0x1801) AM_DEVWRITE("ym2", ym3526_device, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( common )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED ) /* Button 4 on karnov schematics */

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED ) /* Button 4 on karnov schematics */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) /* PL1 Button 5 on karnov schematics */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* PL2 Button 5 on karnov schematics */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


/* verified from M68000 code */
static INPUT_PORTS_START( karnov )
	PORT_INCLUDE( common )

	PORT_START("FAKE")  /* Dummy input for i8751 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )    /* see notes */

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "50 'K'" )
	PORT_DIPSETTING(      0x0800, "70 'K'" )
	PORT_DIPSETTING(      0x0400, "90 'K'" )
	PORT_DIPSETTING(      0x0000, "100 'K'" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Timer Speed" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
INPUT_PORTS_END


/* verified from M68000 code */
static INPUT_PORTS_START( wndrplnt )
	PORT_INCLUDE( common )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_COCKTAIL */

	PORT_START("FAKE")  /* Dummy input for i8751 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )    /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )    /* see notes */
INPUT_PORTS_END


/* verified from M68000 code */
static INPUT_PORTS_START( chelnov )
	PORT_INCLUDE( common )

	PORT_START("FAKE")  /* Dummy input for i8751 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")   /* also determines "Bonus Life" settings */
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )         /* bonus life at 30k 60k 100k 150k 250k 100k+ */
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )       /* bonus life at 50k 120k 200k 300k 100k+ */
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )         /* bonus life at 80k 160k 260k 100k+ */
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      /* bonus life at every 100k */
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )    /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )    /* see notes */
INPUT_PORTS_END

/* verified from M68000 code */
static INPUT_PORTS_START( chelnovj )
	PORT_INCLUDE( chelnov )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

/* verified from M68000 code */
static INPUT_PORTS_START( chelnovu )
	PORT_INCLUDE( chelnovj )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x4000, 0x4000, "Freeze" )        PORT_DIPLOCATION("SW2:7") /* see notes */
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout chars =
{
	8,8,
	1024,
	3,
	{ 0x6000*8,0x4000*8,0x2000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every sprite takes 8 consecutive bytes */
};

static const gfx_layout sprites =
{
	16,16,
	4096,
	4,
	{ 0x60000*8,0x00000*8,0x20000*8,0x40000*8 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
	0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};


/* 16x16 tiles, 4 Planes, each plane is 0x10000 bytes */
static const gfx_layout tiles =
{
	16,16,
	2048,
	4,
	{ 0x30000*8,0x00000*8,0x10000*8,0x20000*8 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
	0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

static GFXDECODE_START( karnov )
	GFXDECODE_ENTRY( "gfx1", 0, chars,     0,  4 )  /* colors 0-31 */
	GFXDECODE_ENTRY( "gfx2", 0, tiles,   512, 16 )  /* colors 512-767 */
	GFXDECODE_ENTRY( "gfx3", 0, sprites, 256, 16 )  /* colors 256-511 */
GFXDECODE_END


/*************************************
 *
 *  Interrupt generator
 *
 *************************************/

INTERRUPT_GEN_MEMBER(karnov_state::karnov_interrupt)
{
	UINT8 port = ioport("FAKE")->read();

	/* Coin input to the i8751 generates an interrupt to the main cpu */
	if (port == m_coin_mask)
		m_latch = 1;

	if (port != m_coin_mask && m_latch)
	{
		if (m_i8751_needs_ack)
		{
			/* i8751 is busy - queue the command */
			m_i8751_coin_pending = port | 0x8000;
		}
		else
		{
			m_i8751_return = port | 0x8000;
			device.execute().set_input_line(6, HOLD_LINE);
			m_i8751_needs_ack = 1;
		}

		m_latch = 0;
	}

	device.execute().set_input_line(7, HOLD_LINE);  /* VBL */
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void karnov_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_scroll));

	save_item(NAME(m_i8751_return));
	save_item(NAME(m_i8751_needs_ack));
	save_item(NAME(m_i8751_coin_pending));
	save_item(NAME(m_i8751_command_queue));
	save_item(NAME(m_i8751_level));
	save_item(NAME(m_latch));

}

void karnov_state::machine_reset()
{
	memset(m_ram, 0, 0x4000 / 2); /* Chelnov likes ram clear on reset.. */

	m_i8751_return = 0;
	m_i8751_needs_ack = 0;
	m_i8751_coin_pending = 0;
	m_i8751_command_queue = 0;
	m_i8751_level = 0;
//  m_latch = 0;

	m_flipscreen = 0;
	m_scroll[0] = 0;
	m_scroll[1] = 0;
}


static MACHINE_CONFIG_START( karnov, karnov_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)   /* 10 MHz */
	MCFG_CPU_PROGRAM_MAP(karnov_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", karnov_state,  karnov_interrupt)

	MCFG_CPU_ADD("audiocpu", M6502, 1500000)    /* Accurate */
	MCFG_CPU_PROGRAM_MAP(karnov_sound_map)


	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(karnov_state, screen_update_karnov)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", karnov)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_INIT_OWNER(karnov_state, karnov)

	MCFG_DEVICE_ADD("spritegen", DECO_KARNOVSPRITES, 0)
	MCFG_DECO_KARNOVSPRITES_GFX_REGION(2)
	MCFG_DECO_KARNOVSPRITES_GFXDECODE("gfxdecode")

	MCFG_VIDEO_START_OVERRIDE(karnov_state,karnov)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ym2", YM3526, 3000000)
	MCFG_YM3526_IRQ_HANDLER(DEVWRITELINE("audiocpu", m6502_device, irq_line))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static ADDRESS_MAP_START( chelnovjbl_mcu_map, AS_PROGRAM, 8, karnov_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( chelnovjbl_mcu_io_map, AS_IO, 8, karnov_state )
	//internal port
//  AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READWRITE(p1_r, p1_w)
//  AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(p3_r, p3_w)
ADDRESS_MAP_END


static MACHINE_CONFIG_DERIVED( chelnovjbl, karnov )
	MCFG_CPU_ADD("mcu", I8031, 2000000) // ??mhz
	MCFG_CPU_PROGRAM_MAP(chelnovjbl_mcu_map)
	MCFG_CPU_IO_MAP(chelnovjbl_mcu_io_map)

MACHINE_CONFIG_END



static MACHINE_CONFIG_START( wndrplnt, karnov_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)   /* 10 MHz */
	MCFG_CPU_PROGRAM_MAP(karnov_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", karnov_state,  karnov_interrupt)

	MCFG_CPU_ADD("audiocpu", M6502, 1500000)    /* Accurate */
	MCFG_CPU_PROGRAM_MAP(karnov_sound_map)


	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(karnov_state, screen_update_karnov)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", karnov)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_INIT_OWNER(karnov_state, karnov)

	MCFG_DEVICE_ADD("spritegen", DECO_KARNOVSPRITES, 0)
	MCFG_DECO_KARNOVSPRITES_GFX_REGION(2)
	MCFG_DECO_KARNOVSPRITES_GFXDECODE("gfxdecode")

	MCFG_VIDEO_START_OVERRIDE(karnov_state,wndrplnt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ym2", YM3526, 3000000)
	MCFG_YM3526_IRQ_HANDLER(DEVWRITELINE("audiocpu", m6502_device, irq_line))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( karnov )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "dn08-6",       0x00000, 0x10000, CRC(4c60837f) SHA1(6886e6ee1d1563c3011b8fea79e7435f983a3ee0) )
	ROM_LOAD16_BYTE( "dn11-6",       0x00001, 0x10000, CRC(cd4abb99) SHA1(b4482175f5d90941ad3aec6c2269a50f57a465ed) )
	ROM_LOAD16_BYTE( "dn07-",        0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "dn10-",        0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "dn06-5",       0x40000, 0x10000, CRC(29d64e42) SHA1(c07ff5f29b7ccd5fc97b5086bcae57ab6eb29330) )
	ROM_LOAD16_BYTE( "dn09-5",       0x40001, 0x10000, CRC(072d7c49) SHA1(92195b89274d066a9c1f87dd810683ea66edaff4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "dn05-5",       0x8000, 0x8000, CRC(fa1a31a8) SHA1(5007a625be03c546d2a78444d72c28761b10cdb0) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* i8751 MCU */
	ROM_LOAD( "karnov_i8751", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "dn00-",        0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )  /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "dn04-",        0x00000, 0x10000, CRC(a9121653) SHA1(04a67ba6fcf551719734ba2b86ee49c37ee1b842) )  /* Backgrounds */
	ROM_LOAD( "dn01-",        0x10000, 0x10000, CRC(18697c9e) SHA1(b454af7922c4b1a651d303a3d8d89e5cc102f9ca) )
	ROM_LOAD( "dn03-",        0x20000, 0x10000, CRC(90d9dd9c) SHA1(00a3bed276927f099d57e90f28fd77bd41a3c360) )
	ROM_LOAD( "dn02-",        0x30000, 0x10000, CRC(1e04d7b9) SHA1(a2c6fde42569a52cc6d9a86715dea4a8bea80092) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "dn12-",        0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )  /* Sprites - 2 sets of 4, interleaved here */
	ROM_LOAD( "dn14-5",       0x10000, 0x08000, CRC(ac9e6732) SHA1(6f61344eb8a13349471145dee252a01aadb8cdf0) )
	ROM_LOAD( "dn13-",        0x20000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "dn15-5",       0x30000, 0x08000, CRC(8933fcb8) SHA1(0dbda4b032ed3776d7633264f39e6f00ace7a238) )
	ROM_LOAD( "dn16-",        0x40000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "dn17-5",       0x50000, 0x08000, CRC(b70ae950) SHA1(1ec833bdad12710ea846ef48dddbe2e1ae6b8ce1) )
	ROM_LOAD( "dn18-",        0x60000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "dn19-5",       0x70000, 0x08000, CRC(8fd4fa40) SHA1(1870fb0c5c64fbc53a10115f0f3c7624cf2465db) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "karnprom.21",  0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) )
	ROM_LOAD( "karnprom.20",  0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) )
ROM_END

ROM_START( karnova )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "dn08-5",       0x00000, 0x10000, CRC(db92c264) SHA1(bd4bcd984a3455eedd2b78dc2090c9d625025671) )
	ROM_LOAD16_BYTE( "dn11-5",       0x00001, 0x10000, CRC(05669b4b) SHA1(c78d0da5afc66750dd9841a7d4f8f244d878c081) )
	ROM_LOAD16_BYTE( "dn07-",        0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "dn10-",        0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "dn06-5",       0x40000, 0x10000, CRC(29d64e42) SHA1(c07ff5f29b7ccd5fc97b5086bcae57ab6eb29330) )
	ROM_LOAD16_BYTE( "dn09-5",       0x40001, 0x10000, CRC(072d7c49) SHA1(92195b89274d066a9c1f87dd810683ea66edaff4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "dn05-5",       0x8000, 0x8000, CRC(fa1a31a8) SHA1(5007a625be03c546d2a78444d72c28761b10cdb0) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* i8751 MCU */
	ROM_LOAD( "karnov_i8751", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "dn00-",        0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )  /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "dn04-",        0x00000, 0x10000, CRC(a9121653) SHA1(04a67ba6fcf551719734ba2b86ee49c37ee1b842) )  /* Backgrounds */
	ROM_LOAD( "dn01-",        0x10000, 0x10000, CRC(18697c9e) SHA1(b454af7922c4b1a651d303a3d8d89e5cc102f9ca) )
	ROM_LOAD( "dn03-",        0x20000, 0x10000, CRC(90d9dd9c) SHA1(00a3bed276927f099d57e90f28fd77bd41a3c360) )
	ROM_LOAD( "dn02-",        0x30000, 0x10000, CRC(1e04d7b9) SHA1(a2c6fde42569a52cc6d9a86715dea4a8bea80092) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "dn12-",        0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )  /* Sprites - 2 sets of 4, interleaved here */
	ROM_LOAD( "dn14-5",       0x10000, 0x08000, CRC(ac9e6732) SHA1(6f61344eb8a13349471145dee252a01aadb8cdf0) )
	ROM_LOAD( "dn13-",        0x20000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "dn15-5",       0x30000, 0x08000, CRC(8933fcb8) SHA1(0dbda4b032ed3776d7633264f39e6f00ace7a238) )
	ROM_LOAD( "dn16-",        0x40000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "dn17-5",       0x50000, 0x08000, CRC(b70ae950) SHA1(1ec833bdad12710ea846ef48dddbe2e1ae6b8ce1) )
	ROM_LOAD( "dn18-",        0x60000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "dn19-5",       0x70000, 0x08000, CRC(8fd4fa40) SHA1(1870fb0c5c64fbc53a10115f0f3c7624cf2465db) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "karnprom.21",  0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) )
	ROM_LOAD( "karnprom.20",  0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) )
ROM_END

ROM_START( karnovj )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "kar8",         0x00000, 0x10000, CRC(3e17e268) SHA1(3a63928bb0148175519540f9d891b03590094dfb) )
	ROM_LOAD16_BYTE( "kar11",        0x00001, 0x10000, CRC(417c936d) SHA1(d31f9291f18c3d5e3c4430768396e1ac10fd9ea3) )
	ROM_LOAD16_BYTE( "dn07-",        0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "dn10-",        0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "kar6",         0x40000, 0x10000, CRC(c641e195) SHA1(fa7a2eba70e730f72a8d868160af9c41f9b2e5b0) )
	ROM_LOAD16_BYTE( "kar9",         0x40001, 0x10000, CRC(d420658d) SHA1(4c7e67a80e419b8b94eb015f7f0af0a01f00c28e) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "kar5",         0x8000, 0x8000, CRC(7c9158f1) SHA1(dfba7b3abd6b8d6991f0207cd252ee652a6050c2) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* i8751 MCU */
	ROM_LOAD( "karnovj_i8751", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "dn00-",        0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )  /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "dn04-",        0x00000, 0x10000, CRC(a9121653) SHA1(04a67ba6fcf551719734ba2b86ee49c37ee1b842) )  /* Backgrounds */
	ROM_LOAD( "dn01-",        0x10000, 0x10000, CRC(18697c9e) SHA1(b454af7922c4b1a651d303a3d8d89e5cc102f9ca) )
	ROM_LOAD( "dn03-",        0x20000, 0x10000, CRC(90d9dd9c) SHA1(00a3bed276927f099d57e90f28fd77bd41a3c360) )
	ROM_LOAD( "dn02-",        0x30000, 0x10000, CRC(1e04d7b9) SHA1(a2c6fde42569a52cc6d9a86715dea4a8bea80092) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "dn12-",        0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )  /* Sprites - 2 sets of 4, interleaved here */
	ROM_LOAD( "kar14",        0x10000, 0x08000, CRC(c6b39595) SHA1(3bc2d0a613cc1b5d255cccc3b26e21ea1c23e75b) )
	ROM_LOAD( "dn13-",        0x20000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "kar15",        0x30000, 0x08000, CRC(2f72cac0) SHA1(a71e61eea77ecd3240c5217ae84e7aa3ef21288a) )
	ROM_LOAD( "dn16-",        0x40000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "kar17",        0x50000, 0x08000, CRC(7851c70f) SHA1(47b7a64dd8230e95cd7ae7f661c7586c7598c356) )
	ROM_LOAD( "dn18-",        0x60000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "kar19",        0x70000, 0x08000, CRC(7bc174bb) SHA1(d8bc320169fc3a9cdd3f271ea523fb0486abae2c) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "karnprom.21",  0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) )
	ROM_LOAD( "karnprom.20",  0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) )
ROM_END

ROM_START( wndrplnt )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ea08.bin",   0x00000, 0x10000, CRC(b0578a14) SHA1(a420d1e8f80405161c86a123610ddf17c7ff07ff) )
	ROM_LOAD16_BYTE( "ea11.bin",   0x00001, 0x10000, CRC(271edc6c) SHA1(6aa411fa4a3613018e7d971c5675f54d5765904d) )
	ROM_LOAD16_BYTE( "ea07.bin",   0x20000, 0x10000, CRC(7095a7d5) SHA1(a7ee88cad03690a72a52b8ea2310416aa53febdd) )
	ROM_LOAD16_BYTE( "ea10.bin",   0x20001, 0x10000, CRC(81a96475) SHA1(2d2e647ed7867b1a7f0dc24544e241e4b1c9fa92) )
	ROM_LOAD16_BYTE( "ea06.bin",   0x40000, 0x10000, CRC(5951add3) SHA1(394552c29a6266becbdb36c3bd65fc1f56701d11) )
	ROM_LOAD16_BYTE( "ea09.bin",   0x40001, 0x10000, CRC(c4b3cb1e) SHA1(006becbcdbbb3e666382e59e8fa5a5ebe06e5724) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ea05.bin",     0x8000, 0x8000, CRC(8dbb6231) SHA1(342faa020448ce916e820b3df18d44191983f7a6) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* i8751 MCU */
	ROM_LOAD( "wndrplnt_i8751", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ea00.bin",    0x00000, 0x08000, CRC(9f3cac4c) SHA1(af8a275ff531029dbada3c820c9f660fef383100) )   /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ea04.bin",    0x00000, 0x10000, CRC(7d701344) SHA1(4efaa73a4b2534078ee25111a2f5143c7c7e846f) )   /* Backgrounds */
	ROM_LOAD( "ea01.bin",    0x10000, 0x10000, CRC(18df55fb) SHA1(406ea47365ff8372bb2588c97c438ea02aa17538) )
	ROM_LOAD( "ea03.bin",    0x20000, 0x10000, CRC(922ef050) SHA1(e33aea6df2e1a14bd371ed0a2b172f58edcc0e8e) )
	ROM_LOAD( "ea02.bin",    0x30000, 0x10000, CRC(700fde70) SHA1(9b5b59aaffac091622329dc6ebedb24806b69964) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ea12.bin",    0x00000, 0x10000, CRC(a6d4e99d) SHA1(a85dbb23d05d1e386d8a66f505fa9dfcc554327b) )   /* Sprites - 2 sets of 4, interleaved here */
	ROM_LOAD( "ea14.bin",    0x10000, 0x10000, CRC(915ffdc9) SHA1(b65cdc8ee953494f2b69e06cd6c97ee142d83c3e) )
	ROM_LOAD( "ea13.bin",    0x20000, 0x10000, CRC(cd839f3a) SHA1(7eae3a1e080b7db22968d556e80b620cb07976b0) )
	ROM_LOAD( "ea15.bin",    0x30000, 0x10000, CRC(a1f14f16) SHA1(5beb2b8967aa34271f734865704c6bab07d76a8c) )
	ROM_LOAD( "ea16.bin",    0x40000, 0x10000, CRC(7a1d8a9c) SHA1(2b924a7e5a2490a7144b981155f2503d3737875d) )
	ROM_LOAD( "ea17.bin",    0x50000, 0x10000, CRC(21a3223d) SHA1(7754ed9cbe4eed94b49130af6108e919be18e5b3) )
	ROM_LOAD( "ea18.bin",    0x60000, 0x10000, CRC(3fb2cec7) SHA1(7231bb728f1009186d41e177402e84b63f25a44f) )
	ROM_LOAD( "ea19.bin",    0x70000, 0x10000, CRC(87cf03b5) SHA1(29bc25642be1dd7e25f13e96dae90572f7a09d21) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ea21.prm",      0x0000, 0x0400, CRC(c8beab49) SHA1(970c2bad3cbf2d7fc313997ae0fe11dd04383b40) )
	ROM_LOAD( "ea20.prm",      0x0400, 0x0400, CRC(619f9d1e) SHA1(17fe49b6c9ce17be4a03e3400229e3ef4998a46f) )
ROM_END

ROM_START( chelnov )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ee08-e.j16",   0x00000, 0x10000, CRC(8275cc3a) SHA1(961166226b68744eef15fed6a306010757b83556) )
	ROM_LOAD16_BYTE( "ee11-e.j19",   0x00001, 0x10000, CRC(889e40a0) SHA1(e927f32d9bc448a331fb7b3478b2d07154f5013b) )
	ROM_LOAD16_BYTE( "a-j14.bin",    0x20000, 0x10000, CRC(51465486) SHA1(e165e754eb756db3abc1f8477171ab817d03a890) )
	ROM_LOAD16_BYTE( "a-j18.bin",    0x20001, 0x10000, CRC(d09dda33) SHA1(1764215606eec61e4fe30c0fc82ea2faf17821dc) )
	ROM_LOAD16_BYTE( "ee06-e.j13",   0x40000, 0x10000, CRC(55acafdb) SHA1(9dc0528c888dd73617f8cab76690b9296715680a) )
	ROM_LOAD16_BYTE( "ee09-e.j17",   0x40001, 0x10000, CRC(303e252c) SHA1(d5d2570e42aa1e1b3600d14cc694677248e12750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* i8751 MCU */
	ROM_LOAD( "chelnov_i8751", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ee00-e.c5",    0x00000, 0x08000, CRC(e06e5c6b) SHA1(70166257da5be428cb8404d8e1063c59c7722365) )  /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )  /* Backgrounds */
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )  /* Sprites */
	ROM_LOAD( "ee13-.f9",     0x20000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x40000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x60000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee21.k8",      0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) )    /* different from the other set; */
															/* might be bad */
	ROM_LOAD( "ee20.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END

ROM_START( chelnovu )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ee08-a.j15",   0x00000, 0x10000, CRC(2f2fb37b) SHA1(f89b424099097a95cf184d20a15b876c5b639552) )
	ROM_LOAD16_BYTE( "ee11-a.j20",   0x00001, 0x10000, CRC(f306d05f) SHA1(e523ffd17fb0104fe28eac288b6ebf7fc0ea2908) )
	ROM_LOAD16_BYTE( "ee07-a.j14",   0x20000, 0x10000, CRC(9c69ed56) SHA1(23606d2fc7c550eaddf0fd4b0da1a4e2c9263e14) )
	ROM_LOAD16_BYTE( "ee10-a.j18",   0x20001, 0x10000, CRC(d5c5fe4b) SHA1(183b2f5dfa4e0a9067674a29abab2744a887fd19) )
	ROM_LOAD16_BYTE( "ee06-e.j13",   0x40000, 0x10000, CRC(55acafdb) SHA1(9dc0528c888dd73617f8cab76690b9296715680a) )
	ROM_LOAD16_BYTE( "ee09-e.j17",   0x40001, 0x10000, CRC(303e252c) SHA1(d5d2570e42aa1e1b3600d14cc694677248e12750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* i8751 MCU */
	ROM_LOAD( "chelnovu_i8751", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ee00-e.c5",    0x00000, 0x08000, CRC(e06e5c6b) SHA1(70166257da5be428cb8404d8e1063c59c7722365) )  /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )  /* Backgrounds */
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )  /* Sprites */
	ROM_LOAD( "ee13-.f9",     0x20000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x40000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x60000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee21.k8",      0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) )    /* different from the other set; */
															/* might be bad */
	ROM_LOAD( "ee20.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END

ROM_START( chelnovj )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "a-j15.bin",    0x00000, 0x10000, CRC(1978cb52) SHA1(833b8e80445ec2384e0479afb7430b32d6a14441) )
	ROM_LOAD16_BYTE( "a-j20.bin",    0x00001, 0x10000, CRC(e0ed3d99) SHA1(f47aaec5c72ecc308c32cdcf117ef4965ac5ea61) )
	ROM_LOAD16_BYTE( "a-j14.bin",    0x20000, 0x10000, CRC(51465486) SHA1(e165e754eb756db3abc1f8477171ab817d03a890) ) // 1xxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "a-j18.bin",    0x20001, 0x10000, CRC(d09dda33) SHA1(1764215606eec61e4fe30c0fc82ea2faf17821dc) ) // 1xxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "a-j13.bin",    0x40000, 0x10000, CRC(cd991507) SHA1(9da858ea41bfbce78496c086e3b462ea9f3722e8) )
	ROM_LOAD16_BYTE( "a-j17.bin",    0x40001, 0x10000, CRC(977f601c) SHA1(b40a37160b493dcb614922c2a9b4b5f140b62aca) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* i8751 MCU */
	ROM_LOAD( "chelnovj_i8751", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "a-c5.bin",     0x00000, 0x08000, CRC(1abf2c6d) SHA1(86d625ae94cd9ea69e4e613895410640efb175b3) )  /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )  /* Backgrounds */
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )  /* Sprites */
	ROM_LOAD( "ee13-.f9",     0x20000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x40000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x60000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "a-k7.bin",     0x0000, 0x0400, CRC(309c49d8) SHA1(7220002f6ef97514b4e6f61706fc16061120dafa) )    /* different from the parent set; - might be bad */
	ROM_LOAD( "ee20.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END

// bootleg of chelnovj, only interesting because it uses a SCM8031HCCN40 instead of the usual 8051 for protection
// matching roms had been stripped out so chip labels and locations unknown :-(
ROM_START( chelnovjbl ) // code is the same as the regular chelnovj set
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "13.bin",       0x00000, 0x10000, CRC(1978cb52) SHA1(833b8e80445ec2384e0479afb7430b32d6a14441) )
	ROM_LOAD16_BYTE( "16.bin",       0x00001, 0x10000, CRC(e0ed3d99) SHA1(f47aaec5c72ecc308c32cdcf117ef4965ac5ea61) )
	ROM_LOAD16_BYTE( "12.bin",       0x20000, 0x08000, CRC(dcb65089) SHA1(1f63044073b429f5f750e170036d5d8763972051) ) // same content but without FF filled 2nd half
	ROM_LOAD16_BYTE( "15.bin",       0x20001, 0x08000, CRC(2aed4c90) SHA1(74d2a03872f75c731c2472fc8cd497a17b2d590d) ) // ^^
	ROM_LOAD16_BYTE( "11.bin",       0x40000, 0x10000, CRC(cd991507) SHA1(9da858ea41bfbce78496c086e3b462ea9f3722e8) )
	ROM_LOAD16_BYTE( "14.bin",       0x40001, 0x10000, CRC(977f601c) SHA1(b40a37160b493dcb614922c2a9b4b5f140b62aca) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x2000, "mcu", 0 )    /* SCM8031HCCN40  */ // unique to the bootlegs (rewritten, or an exact copy of original Data East internal rom?)
	ROM_LOAD( "17o.bin", 0x0000, 0x2000, CRC(9af64150) SHA1(0f478d9f79baebd2ad90615c98c6bc2d73c0056a) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "a-c5.bin",     0x00000, 0x08000, CRC(1abf2c6d) SHA1(86d625ae94cd9ea69e4e613895410640efb175b3) )  /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* Backgrounds */ // same content split into more roms
	ROM_LOAD( "8.bin",    0x00000, 0x08000, CRC(a78b174a) SHA1(e0d82b600a154b81d7e1a787f0e20eb1a341894f) )
	ROM_LOAD( "9.bin",    0x08000, 0x08000, CRC(97d2c146) SHA1(075bb9afc4f0623cd413883ec2bca574d7ff88d4) )
	ROM_LOAD( "2.bin",    0x10000, 0x08000, CRC(8c45e7de) SHA1(d843b7dcc64ed3a5b8717af172a1f22c4c599480) )
	ROM_LOAD( "3.bin",    0x18000, 0x08000, CRC(504cc95c) SHA1(97e5e9f8cd8ebf5e0c18f27f2988a45c4d3809b3) )
	ROM_LOAD( "6.bin",    0x20000, 0x08000, CRC(8f146815) SHA1(c0330b0ced8d12234d71a9d4cdb8a73f4caa61af) )
	ROM_LOAD( "7.bin",    0x28000, 0x08000, CRC(97bf8061) SHA1(16abb3f65bee2ab93b0adfc1558b5c4ceec726a4) )
	ROM_LOAD( "4.bin",    0x30000, 0x08000, CRC(276a46de) SHA1(5b8932dec0e10be128f5ed41798a8928c0aa506b) )
	ROM_LOAD( "5.bin",    0x38000, 0x08000, CRC(99cee6cd) SHA1(b2cd0a1aef04fd63ad27ac8a61d17a6bb4c8b600) )

	ROM_REGION( 0x80000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD( "17.bin",       0x00000, 0x10000, CRC(47c857f8) SHA1(59f50365cee266c0e4075c989dc7fde50e43667a) ) // probably bad (1st half is 99.996948% match)
	ROM_LOAD( "ee13-.f9",     0x20000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x40000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x60000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "a-k7.bin",     0x0000, 0x0400, CRC(309c49d8) SHA1(7220002f6ef97514b4e6f61706fc16061120dafa) )    /* different from the parent set; - might be bad (comment from chelnovj, not dumped here) */
	ROM_LOAD( "ee20.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END

/*

Main cpu 68000
Sound cpu 6502
Sound ics: ym2203 and ym3812
Other ic: Philips MAB8031AH MCU
OSC: 20 mhz, 12 mhz,8 mhz (for mcu)

*/


// same pcb as above?
// this is a further hacked set of the above, with the copyright messages removed etc. (black screens for several seconds instead)
ROM_START( chelnovjbla )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "cheljb13.bin", 0x00000, 0x10000, CRC(1a099586) SHA1(d37d482190b0c1ad38fab5a0351cbf12ad543221) )
	ROM_LOAD16_BYTE( "cheljb16.bin", 0x00001, 0x10000, CRC(a798e7b2) SHA1(35d610f5f86f09981cd6e4120ed3604d87aceba7) )
	ROM_LOAD16_BYTE( "12.bin",       0x20000, 0x08000, CRC(dcb65089) SHA1(1f63044073b429f5f750e170036d5d8763972051) ) // same content but without FF filled 2nd half
	ROM_LOAD16_BYTE( "15.bin",       0x20001, 0x08000, CRC(2aed4c90) SHA1(74d2a03872f75c731c2472fc8cd497a17b2d590d) ) // ^^
	ROM_LOAD16_BYTE( "11.bin",       0x40000, 0x10000, CRC(cd991507) SHA1(9da858ea41bfbce78496c086e3b462ea9f3722e8) )
	ROM_LOAD16_BYTE( "14.bin",       0x40001, 0x10000, CRC(977f601c) SHA1(b40a37160b493dcb614922c2a9b4b5f140b62aca) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x2000, "mcu", 0 )    /* MAB8031AH */ // unique to the bootlegs (rewritten, or an exact copy of original Data East internal rom?)
	ROM_LOAD( "17o.bin", 0x0000, 0x2000, CRC(9af64150) SHA1(0f478d9f79baebd2ad90615c98c6bc2d73c0056a) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "a-c5.bin",     0x00000, 0x08000, CRC(1abf2c6d) SHA1(86d625ae94cd9ea69e4e613895410640efb175b3) )  /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* Backgrounds */ // same content split into more roms
	ROM_LOAD( "8.bin",    0x00000, 0x08000, CRC(a78b174a) SHA1(e0d82b600a154b81d7e1a787f0e20eb1a341894f) )
	ROM_LOAD( "9.bin",    0x08000, 0x08000, CRC(97d2c146) SHA1(075bb9afc4f0623cd413883ec2bca574d7ff88d4) )
	ROM_LOAD( "2.bin",    0x10000, 0x08000, CRC(8c45e7de) SHA1(d843b7dcc64ed3a5b8717af172a1f22c4c599480) )
	ROM_LOAD( "3.bin",    0x18000, 0x08000, CRC(504cc95c) SHA1(97e5e9f8cd8ebf5e0c18f27f2988a45c4d3809b3) )
	ROM_LOAD( "6.bin",    0x20000, 0x08000, CRC(8f146815) SHA1(c0330b0ced8d12234d71a9d4cdb8a73f4caa61af) )
	ROM_LOAD( "7.bin",    0x28000, 0x08000, CRC(97bf8061) SHA1(16abb3f65bee2ab93b0adfc1558b5c4ceec726a4) )
	ROM_LOAD( "4.bin",    0x30000, 0x08000, CRC(276a46de) SHA1(5b8932dec0e10be128f5ed41798a8928c0aa506b) )
	ROM_LOAD( "5.bin",    0x38000, 0x08000, CRC(99cee6cd) SHA1(b2cd0a1aef04fd63ad27ac8a61d17a6bb4c8b600) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )  /* Sprites */
	ROM_LOAD( "ee13-.f9",     0x20000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x40000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x60000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "a-k7.bin",     0x0000, 0x0400, CRC(309c49d8) SHA1(7220002f6ef97514b4e6f61706fc16061120dafa) )    /* different from the parent set; - might be bad (comment from chelnovj, not dumped here) */
	ROM_LOAD( "ee20.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(karnov_state,karnov)
{
	m_microcontroller_id = KARNOV;
	m_coin_mask = 0x07;
}

DRIVER_INIT_MEMBER(karnov_state,karnovj)
{
	m_microcontroller_id = KARNOVJ;
	m_coin_mask = 0x07;
}

DRIVER_INIT_MEMBER(karnov_state,wndrplnt)
{
	m_microcontroller_id = WNDRPLNT;
	m_coin_mask = 0x00;
}

DRIVER_INIT_MEMBER(karnov_state,chelnov)
{
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();

	m_microcontroller_id = CHELNOV;
	m_coin_mask = 0xe0;
	RAM[0x062a/2] = 0x4e71;  /* hangs waiting on i8751 int */
}

DRIVER_INIT_MEMBER(karnov_state,chelnovu)
{
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();

	m_microcontroller_id = CHELNOVU;
	m_coin_mask = 0xe0;
	RAM[0x062a/2] = 0x4e71;  /* hangs waiting on i8751 int */
}

DRIVER_INIT_MEMBER(karnov_state,chelnovj)
{
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();

	m_microcontroller_id = CHELNOVJ;
	m_coin_mask = 0xe0;
	RAM[0x062a/2] = 0x4e71;  /* hangs waiting on i8751 int */
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, karnov,   0,       karnov,   karnov, karnov_state,   karnov,   ROT0,   "Data East USA",         "Karnov (US, rev 6)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, karnova,  karnov,  karnov,   karnov, karnov_state,   karnov,   ROT0,   "Data East USA",         "Karnov (US, rev 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, karnovj,  karnov,  karnov,   karnov, karnov_state,   karnovj,  ROT0,   "Data East Corporation", "Karnov (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, wndrplnt, 0,       wndrplnt, wndrplnt, karnov_state, wndrplnt, ROT270, "Data East Corporation", "Wonder Planet (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnov,  0,       karnov,   chelnov, karnov_state,  chelnov,  ROT0,   "Data East Corporation", "Chelnov - Atomic Runner (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnovu, chelnov, karnov,   chelnovu, karnov_state, chelnovu, ROT0,   "Data East USA",         "Chelnov - Atomic Runner (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnovj, chelnov, karnov,   chelnovj, karnov_state, chelnovj, ROT0,   "Data East Corporation", "Chelnov - Atomic Runner (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnovjbl,chelnov,chelnovjbl,chelnovj,karnov_state, chelnovj, ROT0,   "bootleg",               "Chelnov - Atomic Runner (Japan, bootleg with I8031, set 1)", MACHINE_SUPPORTS_SAVE ) // todo: hook up MCU instead of using simulation code
GAME( 1988, chelnovjbla,chelnov,chelnovjbl,chelnovj,karnov_state,chelnovj, ROT0,   "bootleg",               "Chelnov - Atomic Runner (Japan, bootleg with I8031, set 2)", MACHINE_SUPPORTS_SAVE ) // ^^
