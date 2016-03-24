// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/namcos1.h"


/*******************************************************************************
*                                                                              *
*   BANK area handling                                                         *
*                                                                              *
*******************************************************************************/

WRITE8_MEMBER( namcos1_state::_3dcs_w )
{
	if (offset & 1) popmessage("LEFT");
	else            popmessage("RIGHT");
}


READ8_MEMBER( namcos1_state::no_key_r )
{
	popmessage("CPU %s PC %08x: keychip read %04x\n", space.device().tag(), space.device().safe_pc(), offset);
	return 0;
}

WRITE8_MEMBER( namcos1_state::no_key_w )
{
	popmessage("CPU %s PC %08x: keychip write %04x=%02x\n", space.device().tag(), space.device().safe_pc(), offset, data);
}


/*****************************************************************************

  Key custom type 1 (CUS136, CUS143, CUS144)

  16/8 bit division + key no.

*****************************************************************************/

/*
the startup checks are the same in the three games using this kind of chip,
apart from the [operating mode?] values written.

dspirit: (CUS136)
CPU #0 PC da06: keychip write 0000=01 [maybe to initialize chip?]
CPU #0 PC da09: keychip read 0003     [return key no.]
CPU #0 PC da14: keychip write 0003=f2 [operating mode?]
CPU #0 PC da1d: keychip write 0000=xx [ \ operands]
CPU #0 PC da22: keychip write 0001=xx [    "    " ]
CPU #0 PC da22: keychip write 0002=xx [    "    " ]
CPU #0 PC da25: keychip read 0001     [   division result (verified to be correct)]
CPU #0 PC da25: keychip read 0002     [    "           "  (verified to be correct)]
CPU #0 PC da28: keychip read 0000     [ /  "           "  (verified to be correct)]
CPU #0 PC daf6: keychip write 0003=c2 [operating mode?]
CPU #0 PC dafa: keychip write 0002=00-ff [???????]
CPU #0 PC db00: keychip read 0003     [???? - stored in 0001]
CPU #0 PC db07: keychip write 0001=82-ff [ \ ???????]
CPU #0 PC db07: keychip write 0002=8d    [ / ???????]
CPU #0 PC db0d: keychip read 0003     [???? - stored in 0002]
CPU #0 PC db14: keychip read 0001     [ \ ???????]
CPU #0 PC db14: keychip read 0002     [   ???????]
CPU #0 PC db17: keychip write 0001=xx [   ???????]
CPU #0 PC db17: keychip write 0002=xx [ / ???????]
CPU #0 PC db1d: keychip read 0001     [???? - stored in 0003]
CPU #0 PC db1d: keychip read 0002     [???? - stored in 0004]
CPU #0 PC db24: keychip write 0003=f2 [operating mode?]
---- end of common checks
CPU #0 PC 8191: keychip write 0003=01 [????]
CPU #0 PC 8196: keychip write 0000=40 [ \ operands]
CPU #0 PC 819c: keychip write 0002=00 [    "    " ]
CPU #0 PC 819f: keychip write 0001=04 [    "    " ]
CPU #0 PC 81a2: keychip read 0001     [   division result (verified to be correct)]
CPU #0 PC 81a2: keychip read 0002     [ /  "           "  (verified to be correct)]
if division result is wrong, reads again key no. from 03, and doesn't crash if it's correct

wldcourt: (CUS143)
CPU #0 PC db0a: keychip write 0000=01 [maybe to initialize chip?]
CPU #0 PC db0d: keychip read 0003     [return key no.]
CPU #0 PC db18: keychip write 0003=d9 [operating mode?]
CPU #0 PC db21: keychip write 0000=xx [ \ operands]
CPU #0 PC db26: keychip write 0001=xx [    "    " ]
CPU #0 PC db26: keychip write 0002=xx [    "    " ]
CPU #0 PC db29: keychip read 0001     [   division result (verified to be correct)]
CPU #0 PC db29: keychip read 0002     [    "           "  (verified to be correct)]
CPU #0 PC db2c: keychip read 0000     [ /  "           "  (verified to be correct)]
CPU #0 PC dbfa: keychip write 0003=db [operating mode?]
CPU #0 PC dbfe: keychip write 0002=00-ff [???????]
CPU #0 PC dc04: keychip read 0003     [???? - stored in 0001]
CPU #0 PC dc0b: keychip write 0001=82-ff [ \ ???????]
CPU #0 PC dc0b: keychip write 0002=8d    [ / ???????]
CPU #0 PC dc11: keychip read 0003     [???? - stored in 0002]
CPU #0 PC dc18: keychip read 0001     [ \ ???????]
CPU #0 PC dc18: keychip read 0002     [   ???????]
CPU #0 PC dc1b: keychip write 0001=xx [   ???????]
CPU #0 PC dc1b: keychip write 0002=xx [ / ???????]
CPU #0 PC dc21: keychip read 0001     [???? - stored in 0003]
CPU #0 PC dc21: keychip read 0002     [???? - stored in 0004]
CPU #0 PC dc28: keychip write 0003=d9 [operating mode?]
---- end of common checks
CPU #0 PC e31c: keychip read 0003     [return key no.]

blazer: (CUS144)
CPU #0 PC da0b: keychip write 0000=01 [maybe to initialize chip?]
CPU #0 PC da0e: keychip read 0003     [return key no.]
CPU #0 PC da19: keychip write 0003=b7 [operating mode?]
CPU #0 PC da22: keychip write 0000=12 [ \ operands]
CPU #0 PC da27: keychip write 0001=0a [    "    " ]
CPU #0 PC da27: keychip write 0002=95 [    "    " ]
CPU #0 PC da2a: keychip read 0001     [   division result (verified to be correct)]
CPU #0 PC da2a: keychip read 0002     [    "           "  (verified to be correct)]
CPU #0 PC da2d: keychip read 0000     [ /  "           "  (verified to be correct)]
CPU #0 PC dafb: keychip write 0003=b6 [operating mode?]
CPU #0 PC daff: keychip write 0002=00-ff [???????]
CPU #0 PC db05: keychip read 0003     [???? - stored in 0001]
CPU #0 PC db0c: keychip write 0001=82-ff [ \ ???????]
CPU #0 PC db0c: keychip write 0002=8d    [ / ???????]
CPU #0 PC db12: keychip read 0003     [???? - stored in 0002]
CPU #0 PC db19: keychip read 0001     [ \ ???????]
CPU #0 PC db19: keychip read 0002     [   ???????]
CPU #0 PC db1c: keychip write 0001=xx [   ???????]
CPU #0 PC db1c: keychip write 0002=xx [ / ???????]
CPU #0 PC db22: keychip read 0001     [???? - stored in 0003]
CPU #0 PC db22: keychip read 0002     [???? - stored in 0004]
CPU #0 PC db29: keychip write 0003=b7 [operating mode?]
---- end of common checks

puzlclub:
CPU #0 PC e017: keychip write 0003=35 [they probably used RAM instead of a key chip for this prototype]
CPU #0 PC e3d4: keychip read 0003     [AND #$37 = key no.]
*/
READ8_MEMBER( namcos1_state::key_type1_r )
{
//  logerror("CPU %s PC %04x: keychip read %04x\n", space.device().tag(), space.device().safe_pc(), offset);

	if (offset < 3)
	{
		int d = m_key[0];
		int n = (m_key[1] << 8) | m_key[2];
		int q,r;

		if (d)
		{
			q = n / d;
			r = n % d;
		}
		else
		{
			q = 0xffff;
			r = 0x00;
		}

		if (offset == 0) return r;
		if (offset == 1) return q >> 8;
		if (offset == 2) return q & 0xff;
	}
	else if (offset == 3)
		return m_key_id;

	return 0;
}

WRITE8_MEMBER( namcos1_state::key_type1_w )
{
//  logerror("CPU %s PC %04x: keychip write %04x=%02x\n", space.device().tag(), space.device().safe_pc(), offset, data);

	if (offset < 4)
		m_key[offset] = data;
}


/*****************************************************************************

  Key custom type 2 (CUS151 - CUS155)

  16/16 or 32/16 bit division + key no.

  it's not entirely understood how the 32/16 division works. Only galaga88
  seems to use it.

*****************************************************************************/

/*
pacmania: (CUS151)
CPU #0 PC dac2: keychip write 0004=4b [operating mode?]
CPU #0 PC dac5: keychip read 0004     [AND #$37 = key no.]
CPU #0 PC dad9: keychip write 0000=xx
CPU #0 PC dad9: keychip write 0001=xx
CPU #0 PC dae0: keychip write 0002=xx
CPU #0 PC dae0: keychip write 0003=xx
CPU #0 PC dae4: keychip read 0000
CPU #0 PC dae4: keychip read 0001
CPU #0 PC daed: keychip read 0002
CPU #0 PC daed: keychip read 0003
CPU #1 PC f188: keychip write 0004=4b [operating mode?]
CPU #1 PC f2d0: keychip write 0000=01
CPU #1 PC f2d0: keychip write 0001=aa
CPU #1 PC f30d: keychip write 0002=xx
CPU #1 PC f30d: keychip write 0003=xx
CPU #1 PC f310: keychip read 0003
CPU #1 PC f55d: keychip write 0000=00
CPU #1 PC f55d: keychip write 0001=10
CPU #1 PC f562: keychip write 0004=4b [operating mode?]
CPU #1 PC f5d3: keychip write 0002=xx
CPU #1 PC f5d3: keychip write 0003=xx
CPU #1 PC f5d6: keychip read 0003
CPU #1 PC f5ef: keychip read 0001
CPU #1 PC cbc6: keychip write 0000=00
CPU #1 PC cbc6: keychip write 0001=02
CPU #1 PC cbdd: keychip write 0002=xx
CPU #1 PC cbdd: keychip write 0003=xx
CPU #1 PC cbe8: keychip read 0002
CPU #1 PC cbe8: keychip read 0003

mmaze: (CUS152)
CPU #0 PC 60f3: keychip write 0004=5b [operating mode?]
CPU #0 PC 60fe: keychip write 0000=00
CPU #0 PC 60fe: keychip write 0001=01
CPU #0 PC 6101: keychip write 0002=00
CPU #0 PC 6101: keychip write 0003=01
CPU #0 PC 6104: keychip read 0004
CPU #0 PC 61ae: keychip write 0000=00
CPU #0 PC 61ae: keychip write 0001=33
CPU #0 PC 61b1: keychip write 0002=02
CPU #0 PC 61b1: keychip write 0003=00
CPU #0 PC 61b4: keychip read 0002
CPU #0 PC 61b4: keychip read 0003
CPU #0 PC 68fa: keychip write 0000=01
CPU #0 PC 68fa: keychip write 0001=00
CPU #0 PC 68fd: keychip write 0002=00
CPU #0 PC 68fd: keychip write 0003=00
CPU #0 PC 6900: keychip read 0002
CPU #0 PC 6900: keychip read 0003
CPU #0 PC bf6c: keychip write 0000=00
CPU #0 PC bf6c: keychip write 0001=20
CPU #0 PC bf6f: keychip write 0002=00
CPU #0 PC bf6f: keychip write 0003=00
CPU #0 PC bf72: keychip read 0002
CPU #0 PC bf72: keychip read 0003
CPU #0 PC 9576: keychip write 0000=00
CPU #0 PC 9576: keychip write 0001=14
CPU #0 PC 9579: keychip write 0002=00
CPU #0 PC 9579: keychip write 0003=00
CPU #0 PC 957c: keychip read 0002
CPU #0 PC 957c: keychip read 0003
CPU #0 PC 6538: keychip write 0000=00
CPU #0 PC 6538: keychip write 0001=15
CPU #0 PC 653b: keychip write 0002=00
CPU #0 PC 653b: keychip write 0003=01
CPU #0 PC 653e: keychip read 0002
CPU #0 PC 653e: keychip read 0003

galaga88: (CUS153)
CPU #0 PC db02: keychip write 0004=2d [operating mode?]
CPU #0 PC db05: keychip read 0004     [AND #$7F = key no.]
CPU #0 PC db17: keychip write 0000=xx
CPU #0 PC db17: keychip write 0001=xx
CPU #0 PC db1e: keychip write 0002=xx
CPU #0 PC db1e: keychip write 0003=xx
CPU #0 PC db22: keychip read 0000
CPU #0 PC db22: keychip read 0001
CPU #0 PC db2b: keychip read 0002
CPU #0 PC db2b: keychip read 0003
CPU #0 PC e136: keychip write 0004=0c [operating mode?]

then a 32/16 bit division is used to calculate the hit ratio at the end of the game:
CPU #0 PC ef6f: keychip write 0004=2d [operating mode?]
CPU #0 PC ef76: keychip write 0000=01 \ denominator
CPU #0 PC ef76: keychip write 0001=37 /
CPU #0 PC ef7c: keychip write 0002=00 \ numerator high word
CPU #0 PC ef7c: keychip write 0003=03 /
CPU #0 PC ef81: keychip write 0004=0c [operating mode?]
CPU #0 PC ef87: keychip write 0002=b1 \ numerator low word
CPU #0 PC ef87: keychip write 0003=50 /
CPU #0 PC ef8a: keychip read 0002
CPU #0 PC ef8a: keychip read 0003

ws: (CUS154)
CPU #0 PC e052: keychip write 0004=d3 [operating mode?]
CPU #0 PC e82a: keychip read 0004     [AND #$3F = key no.]

bakutotu: (CUS155)
CPU #0 PC db38: keychip write 0004=03 [operating mode?]
CPU #0 PC db3b: keychip read 0004     [AND #$37 = key no.]
CPU #0 PC db4f: keychip write 0000=xx
CPU #0 PC db4f: keychip write 0001=xx
CPU #0 PC db56: keychip write 0002=xx
CPU #0 PC db56: keychip write 0003=xx
CPU #0 PC db5a: keychip read 0000
CPU #0 PC db5a: keychip read 0001
CPU #0 PC db63: keychip read 0002
CPU #0 PC db63: keychip read 0003
CPU #0 PC db70: keychip write 0004=0b [operating mode?]
CPU #0 PC db77: keychip write 0000=ff
CPU #0 PC db77: keychip write 0001=ff
CPU #0 PC db7e: keychip write 0002=ff
CPU #0 PC db7e: keychip write 0003=ff
CPU #0 PC db82: keychip read 0000
CPU #0 PC db82: keychip read 0001
CPU #0 PC db8b: keychip read 0002
CPU #0 PC db8b: keychip read 0003
CPU #0 PC db95: keychip write 0004=03 [operating mode?]
CPU #0 PC e552: keychip read 0004     [AND #$3F = key no.]
CPU #0 PC e560: keychip write 0000=00
CPU #0 PC e560: keychip write 0001=73
CPU #0 PC e566: keychip write 0002=a4
CPU #0 PC e566: keychip write 0003=83
CPU #0 PC e569: keychip read 0002
CPU #0 PC e569: keychip read 0003
CPU #0 PC e574: keychip read 0000
CPU #0 PC e574: keychip read 0001

*/

READ8_MEMBER( namcos1_state::key_type2_r )
{
//  logerror("CPU %s PC %04x: keychip read %04x\n", space.device().tag(), space.device().safe_pc(), offset);

	m_key_numerator_high_word = 0;

	if (offset < 4)
	{
		if (offset == 0) return m_key_reminder >> 8;
		if (offset == 1) return m_key_reminder & 0xff;
		if (offset == 2) return m_key_quotient >> 8;
		if (offset == 3) return m_key_quotient & 0xff;
	}
	else if (offset == 4)
		return m_key_id;

	return 0;
}

WRITE8_MEMBER( namcos1_state::key_type2_w )
{
//  logerror("CPU %s PC %04x: keychip write %04x=%02x\n", space.device().tag(), space.device().safe_pc(), offset, data);

	if (offset < 5)
	{
		m_key[offset] = data;

		if (offset == 3)
		{
			unsigned int d = (m_key[0] << 8) | m_key[1];
			unsigned int n = (m_key_numerator_high_word << 16) | (m_key[2] << 8) | m_key[3];

			if (d)
			{
				m_key_quotient = n / d;
				m_key_reminder = n % d;
			}
			else
			{
				m_key_quotient = 0xffff;
				m_key_reminder = 0x0000;
			}

//  logerror("calculating division: %08x / %04x = %04x, %04x\n", n, d, key_quotient, m_key_reminder);

			m_key_numerator_high_word = (m_key[2] << 8) | m_key[3];
		}
	}
}


/*****************************************************************************

  Key custom type 3 (CUS181 - CUS185, CUS308 - CUS311)

  Nibble swapper + rng + key no.

*****************************************************************************/

/*
splatter: (CUS181)
CPU #0 PC cd66: keychip write 003f=01 [maybe to initialize chip?]
CPU #0 PC cd78: keychip read 004f     [rng]
CPU #0 PC cd91: keychip read 003f     [key no.]
CPU #0 PC f90c: keychip read 0036     [key no.]
CPU #0 PC f91f: keychip read 0036     [key no.]

rompers: (CUS182)
CPU #0 PC b891: keychip read 0070     [key no.]

blastoff: (CUS183)
CPU #0 PC db4f: keychip read 0000     [key no.]
CPU #0 PC db55: keychip read 0070     [rng?]
CPU #0 PC db57: keychip write 0030=xx [ARG for 58]
CPU #0 PC db60: keychip write 0858=xx [debug leftover?]
CPU #0 PC db67: keychip read 0858     [debug leftover?]
CPU #0 PC db69: keychip read 0058     [ARG >> 4) | (ARG << 4]
CPU #0 PC e01a: keychip read 0000     [key no.]
CPU #0 PC e021: keychip read 0070     [rng?]
CPU #0 PC e024: keychip write 0030=xx [arg for 58]
CPU #0 PC e037: keychip read 0058     [ARG >> 4) | (ARG << 4]
CPU #0 PC 6989: keychip read 0000     [ignored]
CPU #0 PC 6989: keychip read 0001     [ignored]

ws89: (CUS184)
CPU #0 PC e050: keychip read 0020     [key no.]
CPU #0 PC e835: keychip read 0020     [key no.]

tankfrce: (CUS185)
CPU #0 PC c441: keychip read 0057     [key no.]
CPU #0 PC c444: keychip write 0017=b9 [ARG for 2b]
CPU #0 PC c447: keychip read 002b     [0xb0 | (ARG & 0x0f)]
CPU #0 PC f700: keychip read 0050     [key no.]

dangseed: (CUS308)
CPU #0 PC c628: keychip read 0067     [key no.]
CPU #0 PC c62b: keychip write 0057=34 [ARG for 03]
CPU #0 PC c62e: keychip read 0003     [0x30 | (ARG & 0x0f)]
CPU #1 PC feb8: keychip write 0050=xx [ARG for 41 and 01]
CPU #1 PC febb: keychip read 0041     [0x10 | (ARG >> 4)]   used for stars display
CPU #1 PC fec0: keychip read 0001     [0x10 | (ARG & 0x0f)] used for stars display
CPU #1 PC f74d: keychip write 0050=xx [ARG for 40 and 00]
CPU #1 PC f750: keychip read 0040     [0x00 | (ARG >> 4)]   used for score display
CPU #1 PC f755: keychip read 0000     [0x00 | (ARG & 0x0f)] used for score display
CPU #1 PC c310: keychip write 0050=xx [ARG for 00]
CPU #1 PC c31e: keychip read 0000     [0x00 | (ARG & 0x0f)] used for credits display

pistoldm: (CUS309)
CPU #0 PC c84a: keychip read 0017     [key no.]
CPU #0 PC c84d: keychip write 0007=35 [ARG for 43]
CPU #0 PC c850: keychip read 0043     [0x30 | (ARG & 0x0f)]
CPU #0 PC cdef: keychip read 0020     [rng] used for several random events

ws90: (CUS310)
CPU #0 PC c82f: keychip read 0047     [key no.]
CPU #0 PC c832: keychip write 007f=36 [ARG for 33]
CPU #0 PC c835: keychip read 0033     [0x30 | (ARG & 0x0f)]
CPU #0 PC e828: keychip read 0040     [key no.]

soukobdx: (CUS311)
CPU #0 PC ca90: keychip read 0027     [key no.]
CPU #0 PC ca93: keychip write 0007=37 [ARG for 43]
CPU #0 PC ca96: keychip read 0043     [0x30 | (ARG & 0x0f)]
CPU #0 PC e45a: keychip read 0030     [discarded]
*/

READ8_MEMBER( namcos1_state::key_type3_r )
{
//  logerror("CPU %s PC %04x: keychip read %04x\n", space.device().tag(), space.device().safe_pc(), offset);

	/* I need to handle blastoff's read from 0858. The game previously writes to 0858,
	   using it as temporary storage, so maybe it expects to act as RAM, however
	   it happens to work correctly also using the standard handling for 0058.
	   The schematics don't show A11 being used, so I go for this handling.
	  */

	int op = (offset & 0x70) >> 4;

	if (op == m_key_reg)     return m_key_id;
	if (op == m_key_rng)     return machine().rand();
	if (op == m_key_swap4)   return (m_key[m_key_swap4_arg] << 4) | (m_key[m_key_swap4_arg] >> 4);
	if (op == m_key_bottom4) return (offset << 4) | (m_key[m_key_swap4_arg] & 0x0f);
	if (op == m_key_top4)    return (offset << 4) | (m_key[m_key_swap4_arg] >> 4);

	popmessage("CPU %s PC %08x: keychip read %04x", space.device().tag(), space.device().safe_pc(), offset);

	return 0;
}

WRITE8_MEMBER( namcos1_state::key_type3_w )
{
//  logerror("CPU %s PC %04x: keychip write %04x=%02x\n", space.device().tag(), space.device().safe_pc(), offset, data);

	m_key[(offset & 0x70) >> 4] = data;
}



/*******************************************************************************
*                                                                              *
*   Sound banking emulation (CUS121)                                           *
*                                                                              *
*******************************************************************************/

WRITE8_MEMBER(namcos1_state::sound_bankswitch_w)
{
	m_soundbank->set_entry((data & 0x70) >> 4);
}



/*******************************************************************************
*                                                                              *
*   Banking emulation (CUS117)                                                 *
*                                                                              *
*******************************************************************************/

WRITE_LINE_MEMBER(namcos1_state::subres_w)
{
//  logerror("reset control pc=%04x %02x\n",space.device().safe_pc(),data);
	if (state != m_reset)
	{
		m_mcu_patch_data = 0;
		m_reset = state;
	}

	m_subcpu->set_input_line(INPUT_LINE_RESET, state);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state);
	m_mcu->set_input_line(INPUT_LINE_RESET, state);
}

/*******************************************************************************
*                                                                              *
*   Initialization                                                             *
*                                                                              *
*******************************************************************************/

void namcos1_state::machine_start()
{
	m_soundbank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
	m_mcubank->configure_entries(0, 24, memregion("voice")->base(), 0x8000);

	save_item(NAME(m_dac0_value));
	save_item(NAME(m_dac1_value));
	save_item(NAME(m_dac0_gain));
	save_item(NAME(m_dac1_gain));
	save_item(NAME(m_key));
	save_item(NAME(m_mcu_patch_data));
	save_item(NAME(m_reset));
}

void namcos1_state::machine_reset()
{
	/* mcu patch data clear */
	m_mcu_patch_data = 0;

	init_DACs();
	memset(m_key, 0, sizeof(m_key));
	m_key_quotient = 0;
	m_key_reminder = 0;
	m_key_numerator_high_word = 0;
}



/*******************************************************************************
*                                                                              *
*   63701 MCU emulation (CUS64)                                                *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*                                                                              *
*   MCU banking emulation and patch                                            *
*                                                                              *
*******************************************************************************/

/* mcu banked rom area select */
WRITE8_MEMBER(namcos1_state::mcu_bankswitch_w)
{
	int bank;

	/* bit 2-7 : chip select line of ROM chip */
	switch (data & 0xfc)
	{
		case 0xf8: bank = 0;  data ^= 2; break;    /* bit 2 : ROM 0, A16 is inverted */
		case 0xf4: bank = 4;  break;               /* bit 3 : ROM 1 */
		case 0xec: bank = 8;  break;               /* bit 4 : ROM 2 */
		case 0xdc: bank = 12; break;               /* bit 5 : ROM 3 */
		case 0xbc: bank = 16; break;               /* bit 6 : ROM 4 */
		case 0x7c: bank = 20; break;               /* bit 7 : ROM 5 */
		default:   bank = 0;  break;               /* illegal (selects multiple ROMs at once) */
	}

	/* bit 0-1 : address line A15-A16 */
	bank += (data & 3);

	m_mcubank->set_entry(bank);
}



/* This point is very obscure, but I haven't found any better way yet. */
/* Works with all games so far.                                        */

/* patch points of memory address */
/* CPU0/1 bank[17f][1000] */
/* CPU2   [7000]      */
/* CPU3   [c000]      */

/* This memory point should be set $A6 by anywhere, but         */
/* I found set $A6 only initialize in MCU                       */
/* This patch kill write this data by MCU case $A6 to xx(clear) */

WRITE8_MEMBER(namcos1_state::mcu_patch_w)
{
	//logerror("mcu C000 write pc=%04x data=%02x\n",space.device().safe_pc(),data);
	if (m_mcu_patch_data == 0xa6) return;
	m_mcu_patch_data = data;
	m_triram[0] = data;
}



/*******************************************************************************
*                                                                              *
*   driver specific initialize routine                                         *
*                                                                              *
*******************************************************************************/
void namcos1_state::driver_init()
{
	// bit 16 of the address is inverted for PRG7 (and bits 17,18 just not connected)
	for (int i = 0x380000;i < 0x400000;i++)
	{
		if ((i & 0x010000) == 0)
		{
			UINT8 t = m_rom[i];
			m_rom[i] = m_rom[i + 0x010000];
			m_rom[i + 0x010000] = t;
		}
	}

	// kludge! see notes
	m_mcu->space(AS_PROGRAM).install_write_handler(0xc000, 0xc000, write8_delegate(FUNC(namcos1_state::mcu_patch_w), this));

	// these are overridden as needed in the specific DRIVER_INIT_MEMBERs
	m_key_id        = 0;
	m_key_reg       = 0;
	m_key_rng       = 0;
	m_key_swap4_arg = 0;
	m_key_swap4     = 0;
	m_key_bottom4   = 0;
	m_key_top4      = 0;
}


/*******************************************************************************
*   Shadowland / Youkai Douchuuki specific                                     *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,shadowld)
{
	driver_init();
}

/*******************************************************************************
*   Dragon Spirit specific (CUS136)                                            *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,dspirit)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type1_r),this),
		write8_delegate(FUNC(namcos1_state::key_type1_w),this));
	m_key_id = 0x36;
}

/*******************************************************************************
*   World Court specific (CUS143)                                              *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,wldcourt)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type1_r),this),
		write8_delegate(FUNC(namcos1_state::key_type1_w),this));
	m_key_id = 0x35;
}

/*******************************************************************************
*   Blazer specific (CUS144)                                                   *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,blazer)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type1_r),this),
		write8_delegate(FUNC(namcos1_state::key_type1_w),this));
	m_key_id = 0x13;
}

/*******************************************************************************
*   Puzzle Club specific                                                       *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,puzlclub)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type1_r),this),
		write8_delegate(FUNC(namcos1_state::key_type1_w),this));
	m_key_id = 0x35;
}

/*******************************************************************************
*   Pac-Mania specific (CUS151)                                                *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,pacmania)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type2_r),this),
		write8_delegate(FUNC(namcos1_state::key_type2_w),this));
	m_key_id = 0x12;
	save_item(NAME(m_key_quotient));
	save_item(NAME(m_key_reminder));
	save_item(NAME(m_key_numerator_high_word));
}

/*******************************************************************************
*   Alice in Wonderland / Marchen Maze specific (CUS152)                       *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,alice)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type2_r),this),
		write8_delegate(FUNC(namcos1_state::key_type2_w),this));
	m_key_id = 0x25;
	save_item(NAME(m_key_quotient));
	save_item(NAME(m_key_reminder));
	save_item(NAME(m_key_numerator_high_word));
}

/*******************************************************************************
*   Galaga '88 specific (CUS153)                                               *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,galaga88)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type2_r),this),
		write8_delegate(FUNC(namcos1_state::key_type2_w),this));
	m_key_id = 0x31;
	save_item(NAME(m_key_quotient));
	save_item(NAME(m_key_reminder));
	save_item(NAME(m_key_numerator_high_word));
}

/*******************************************************************************
*   World Stadium specific (CUS154)                                            *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,ws)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type2_r),this),
		write8_delegate(FUNC(namcos1_state::key_type2_w),this));
	m_key_id = 0x07;
	save_item(NAME(m_key_quotient));
	save_item(NAME(m_key_reminder));
	save_item(NAME(m_key_numerator_high_word));
}

/*******************************************************************************
*   Bakutotsu Kijuutei specific (CUS155)                                       *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,bakutotu)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type2_r),this),
		write8_delegate(FUNC(namcos1_state::key_type2_w),this));
	m_key_id = 0x22;
	save_item(NAME(m_key_quotient));
	save_item(NAME(m_key_reminder));
	save_item(NAME(m_key_numerator_high_word));
}

/*******************************************************************************
*   Splatter House specific (CUS181)                                           *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,splatter)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 181;
	m_key_reg       = 3;
	m_key_rng       = 4;
	m_key_swap4_arg = -1;
	m_key_swap4     = -1;
	m_key_bottom4   = -1;
	m_key_top4      = -1;
}

/*******************************************************************************
*   Rompers specific (CUS182)                                                  *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,rompers)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 182;
	m_key_reg       = 7;
	m_key_rng       = -1;
	m_key_swap4_arg = -1;
	m_key_swap4     = -1;
	m_key_bottom4   = -1;
	m_key_top4      = -1;
}

/*******************************************************************************
*   Blast Off specific (CUS183)                                                *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,blastoff)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 183;
	m_key_reg       = 0;
	m_key_rng       = 7;
	m_key_swap4_arg = 3;
	m_key_swap4     = 5;
	m_key_bottom4   = -1;
	m_key_top4      = -1;
}

/*******************************************************************************
*   World Stadium '89 specific (CUS184)                                        *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,ws89)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 184;
	m_key_reg       = 2;
	m_key_rng       = -1;
	m_key_swap4_arg = -1;
	m_key_swap4     = -1;
	m_key_bottom4   = -1;
	m_key_top4      = -1;
}

/*******************************************************************************
*   Tank Force specific (CUS185)                                               *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,tankfrce)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 185;
	m_key_reg       = 5;
	m_key_rng       = -1;
	m_key_swap4_arg = 1;
	m_key_swap4     = -1;
	m_key_bottom4   = 2;
	m_key_top4      = -1;
}

DRIVER_INIT_MEMBER(namcos1_state,tankfrc4)
{
	DRIVER_INIT_CALL(tankfrce);

	m_input_count = 0;
	m_strobe_count = 0;
	m_stored_input[0] = 0;
	m_stored_input[1] = 0;

	m_mcu->space(AS_PROGRAM).install_read_handler(0x1400, 0x1401, read8_delegate(FUNC(namcos1_state::faceoff_inputs_r), this));
	save_item(NAME(m_input_count));
	save_item(NAME(m_stored_input));
}

/*******************************************************************************
*   Dangerous Seed specific (CUS308)                                           *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,dangseed)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 308;
	m_key_reg       = 6;
	m_key_rng       = -1;
	m_key_swap4_arg = 5;
	m_key_swap4     = -1;
	m_key_bottom4   = 0;
	m_key_top4      = 4;
}

/*******************************************************************************
*   Pistol Daimyo no Bouken specific (CUS309)                                  *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,pistoldm)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 309;
	m_key_reg       = 1;
	m_key_rng       = 2;
	m_key_swap4_arg = 0;
	m_key_swap4     = -1;
	m_key_bottom4   = 4;
	m_key_top4      = -1;
}

/*******************************************************************************
*   World Stadium '90 specific (CUS310)                                        *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,ws90)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 310;
	m_key_reg       = 4;
	m_key_rng       = -1;
	m_key_swap4_arg = 7;
	m_key_swap4     = -1;
	m_key_bottom4   = 3;
	m_key_top4      = -1;
}

/*******************************************************************************
*   Souko Ban DX specific (CUS311)                                             *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,soukobdx)
{
	driver_init();
	m_c117->space(AS_PROGRAM).install_readwrite_handler(0x2f8000, 0x2f9fff,
		read8_delegate(FUNC(namcos1_state::key_type3_r),this),
		write8_delegate(FUNC(namcos1_state::key_type3_w),this));
	m_key_id        = 311;
	m_key_reg       = 2;
	m_key_rng       = 3; /*?*/
	m_key_swap4_arg = 0;
	m_key_swap4     = -1;
	m_key_bottom4   = 4;
	m_key_top4      = -1;
}



/*******************************************************************************
*   Quester specific                                                           *
*******************************************************************************/
READ8_MEMBER( namcos1_state::quester_paddle_r )
{
	if (offset == 0)
	{
		int ret;

		if (!(m_strobe & 0x20))
			ret = (ioport("CONTROL0")->read()&0x90) | (m_strobe & 0x40) | (ioport("PADDLE0")->read()&0x0f);
		else
			ret = (ioport("CONTROL0")->read()&0x90) | (m_strobe & 0x40) | (ioport("PADDLE1")->read()&0x0f);

		m_strobe ^= 0x40;

		return ret;
	}
	else
	{
		int ret;

		if (!(m_strobe & 0x20))
			ret = (ioport("CONTROL1")->read()&0x90) | 0x00 | (ioport("PADDLE0")->read()>>4);
		else
			ret = (ioport("CONTROL1")->read()&0x90) | 0x20 | (ioport("PADDLE1")->read()>>4);

		if (!(m_strobe & 0x40)) m_strobe ^= 0x20;

		return ret;
	}
}

DRIVER_INIT_MEMBER(namcos1_state,quester)
{
	m_strobe = 0;
	driver_init();
	m_mcu->space(AS_PROGRAM).install_read_handler(0x1400, 0x1401, read8_delegate(FUNC(namcos1_state::quester_paddle_r), this));
	save_item(NAME(m_strobe));
}



/*******************************************************************************
*   Beraboh Man specific                                                       *
*******************************************************************************/

READ8_MEMBER( namcos1_state::berabohm_buttons_r )
{
	int res;

	if (offset == 0)
	{
		int inp = m_input_count;

		if (inp == 4) res = ioport("CONTROL0")->read();
		else
		{
			char portname[40];

#ifdef PRESSURE_SENSITIVE
			static int counter[4];

			sprintf(portname,"IN%d",inp);   /* IN0-IN3 */
			res = ioport(portname)->read();
			if (res & 0x80)
			{
				if (counter[inp] >= 0)
					res = 0x40 | counter[inp];
				else
				{
					if (res & 0x40) res = 0x40;
					else res = 0x00;
				}
			}
			else if (res & 0x40)
			{
				if (counter[inp] < 0x3f)
				{
					counter[inp] += 4;
					res = 0x00;
				}
				else res = 0x7f;
			}
			else
				counter[inp] = -1;
#else
			sprintf(portname,"IN%d",inp);   /* IN0-IN3 */
			res = ioport(portname)->read();
			if (res & 1) res = 0x7f;        /* weak */
			else if (res & 2) res = 0x48;   /* medium */
			else if (res & 4) res = 0x40;   /* strong */
#endif
		}

		return res;
	}
	else
	{
		res = ioport("CONTROL1")->read() & 0x8f;

		/* the strobe cannot happen too often, otherwise the MCU will waste too
		   much time reading the inputs and won't have enough cycles to play two
		   digital sounds at once. This value is enough to read all inputs at least
		   once per frame */
		if (++m_strobe_count > 4)
		{
			m_strobe_count = 0;
			m_strobe ^= 0x40;
			if (m_strobe == 0)
			{
				m_input_count = (m_input_count + 1) % 5;
				if (m_input_count == 3) res |= 0x10;
			}
		}

		res |= m_strobe;

		return res;
	}
}

DRIVER_INIT_MEMBER(namcos1_state,berabohm)
{
	m_input_count = 0;
	m_strobe = 0;
	m_strobe_count = 0;
	driver_init();
	m_mcu->space(AS_PROGRAM).install_read_handler(0x1400, 0x1401, read8_delegate(FUNC(namcos1_state::berabohm_buttons_r), this));
	save_item(NAME(m_input_count));
	save_item(NAME(m_strobe));
	save_item(NAME(m_strobe_count));
}



/*******************************************************************************
*   Face Off specific                                                          *
*******************************************************************************/

// used by faceoff and tankforce 4 player (input multiplex)

READ8_MEMBER( namcos1_state::faceoff_inputs_r )
{
	int res;

	if (offset == 0)
	{
		res = (ioport("CONTROL0")->read() & 0x80) | m_stored_input[0];

		return res;
	}
	else
	{
		res = ioport("CONTROL1")->read() & 0x80;

		/* the strobe cannot happen too often, otherwise the MCU will waste too
		   much time reading the inputs and won't have enough cycles to play two
		   digital sounds at once. This value is enough to read all inputs at least
		   once per frame */
		if (++m_strobe_count > 8)
		{
			m_strobe_count = 0;

			res |= m_input_count;

			switch (m_input_count)
			{
				case 0:
					m_stored_input[0] = ioport("IN0")->read() & 0x1f;
					m_stored_input[1] = (ioport("IN3")->read() & 0x07) << 3;
					break;

				case 3:
					m_stored_input[0] = ioport("IN2")->read() & 0x1f;
					break;

				case 4:
					m_stored_input[0] = ioport("IN1")->read() & 0x1f;
					m_stored_input[1] = ioport("IN3")->read() & 0x18;
					break;

				default:
					m_stored_input[0] = 0x1f;
					m_stored_input[1] = 0x1f;
					break;
			}

			m_input_count = (m_input_count + 1) & 7;
		}
		else
		{
			res |= 0x40 | m_stored_input[1];
		}

		return res;
	}
}

DRIVER_INIT_MEMBER(namcos1_state,faceoff)
{
	m_input_count = 0;
	m_strobe_count = 0;
	m_stored_input[0] = 0;
	m_stored_input[1] = 0;

	driver_init();
	m_mcu->space(AS_PROGRAM).install_read_handler(0x1400, 0x1401, read8_delegate(FUNC(namcos1_state::faceoff_inputs_r), this));
	save_item(NAME(m_input_count));
	save_item(NAME(m_stored_input));
}
