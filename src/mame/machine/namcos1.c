#include "emu.h"
#include "sound/ym2151.h"
#include "includes/namcos1.h"


/*******************************************************************************
*                                                                              *
*   BANK area handling                                                         *
*                                                                              *
*******************************************************************************/

/* hardware elements of 1Mbytes physical memory space */

UINT8 namcos1_state::bank_r(address_space &space, offs_t offset, int bank)
{
	return m_active_bank[bank].bank_handler_r(space, offset + m_active_bank[bank].bank_offset, 0xff);
}

READ8_MEMBER( namcos1_state::bank1_r )  { return bank_r(space, offset, 0); }
READ8_MEMBER( namcos1_state::bank2_r )  { return bank_r(space, offset, 1); }
READ8_MEMBER( namcos1_state::bank3_r )  { return bank_r(space, offset, 2); }
READ8_MEMBER( namcos1_state::bank4_r )  { return bank_r(space, offset, 3); }
READ8_MEMBER( namcos1_state::bank5_r )  { return bank_r(space, offset, 4); }
READ8_MEMBER( namcos1_state::bank6_r )  { return bank_r(space, offset, 5); }
READ8_MEMBER( namcos1_state::bank7_r )  { return bank_r(space, offset, 6); }
READ8_MEMBER( namcos1_state::bank8_r )  { return bank_r(space, offset, 7); }
READ8_MEMBER( namcos1_state::bank9_r )  { return bank_r(space, offset, 8); }
READ8_MEMBER( namcos1_state::bank10_r ) { return bank_r(space, offset, 9); }
READ8_MEMBER( namcos1_state::bank11_r ) { return bank_r(space, offset, 10); }
READ8_MEMBER( namcos1_state::bank12_r ) { return bank_r(space, offset, 11); }
READ8_MEMBER( namcos1_state::bank13_r ) { return bank_r(space, offset, 12); }
READ8_MEMBER( namcos1_state::bank14_r ) { return bank_r(space, offset, 13); }
READ8_MEMBER( namcos1_state::bank15_r ) { return bank_r(space, offset, 14); }
READ8_MEMBER( namcos1_state::bank16_r ) { return bank_r(space, offset, 15); }

void namcos1_state::bank_w(address_space &space, offs_t offset, UINT8 data, int bank)
{
	m_active_bank[bank].bank_handler_w(space, offset + m_active_bank[bank].bank_offset, data, 0xff);
}

WRITE8_MEMBER( namcos1_state::bank1_w )  { bank_w(space, offset, data, 0); }
WRITE8_MEMBER( namcos1_state::bank2_w )  { bank_w(space, offset, data, 1); }
WRITE8_MEMBER( namcos1_state::bank3_w )  { bank_w(space, offset, data, 2); }
WRITE8_MEMBER( namcos1_state::bank4_w )  { bank_w(space, offset, data, 3); }
WRITE8_MEMBER( namcos1_state::bank5_w )  { bank_w(space, offset, data, 4); }
WRITE8_MEMBER( namcos1_state::bank6_w )  { bank_w(space, offset, data, 5); }
WRITE8_MEMBER( namcos1_state::bank7_w )  { bank_w(space, offset, data, 6); }
WRITE8_MEMBER( namcos1_state::bank8_w )  { bank_w(space, offset, data, 7); }
WRITE8_MEMBER( namcos1_state::bank9_w )  { bank_w(space, offset, data, 8); }
WRITE8_MEMBER( namcos1_state::bank10_w ) { bank_w(space, offset, data, 9); }
WRITE8_MEMBER( namcos1_state::bank11_w ) { bank_w(space, offset, data, 10); }
WRITE8_MEMBER( namcos1_state::bank12_w ) { bank_w(space, offset, data, 11); }
WRITE8_MEMBER( namcos1_state::bank13_w ) { bank_w(space, offset, data, 12); }
WRITE8_MEMBER( namcos1_state::bank14_w ) { bank_w(space, offset, data, 13); }
WRITE8_MEMBER( namcos1_state::bank15_w ) { bank_w(space, offset, data, 14); }
WRITE8_MEMBER( namcos1_state::bank16_w ) { bank_w(space, offset, data, 15); }

WRITE8_MEMBER( namcos1_state::namcos1_3dcs_w )
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
	int op;

//  logerror("CPU %s PC %04x: keychip read %04x\n", space.device().tag(), space.device().safe_pc(), offset);

	/* I need to handle blastoff's read from 0858. The game previously writes to 0858,
	   using it as temporary storage, so maybe it expects to act as RAM, however
	   it happens to work correctly also using the standard handling for 0058.
	   The schematics don't show A11 being used, so I go for this handling.
	  */

	op = (offset & 0x70) >> 4;

	if (op == m_key_reg)     return m_key_id;
	if (op == m_key_rng)     return machine().rand();
	if (op == m_key_swap4)   return (m_key[m_key_swap4_arg] << 4) | (m_key[m_key_swap4_arg] >> 4);
	if (op == m_key_bottom4) return (offset << 4) | (m_key[m_key_swap4_arg] & 0x0f);
	if (op == m_key_top4)        return (offset << 4) | (m_key[m_key_swap4_arg] >> 4);

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

WRITE8_MEMBER(namcos1_state::namcos1_sound_bankswitch_w)
{
	UINT8 *rom = memregion("audiocpu")->base() + 0xc000;

	int bank = (data & 0x70) >> 4;
	membank("bank17")->set_base(rom + 0x4000 * bank);
}



/*******************************************************************************
*                                                                              *
*   Banking emulation (CUS117)                                                 *
*                                                                              *
*******************************************************************************/


WRITE8_MEMBER(namcos1_state::namcos1_cpu_control_w)
{
//  logerror("reset control pc=%04x %02x\n",space.device().safe_pc(),data);
	if ((data & 1) ^ m_reset)
	{
		m_mcu_patch_data = 0;
		m_reset = data & 1;
	}

	m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
	m_mcu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
}



WRITE8_MEMBER(namcos1_state::namcos1_watchdog_w)
{
	if (&space.device() == m_maincpu)
		m_wdog |= 1;
	else if (&space.device() == m_subcpu)
		m_wdog |= 2;
	else if (&space.device() == m_audiocpu)
		m_wdog |= 4;

	if (m_wdog == 7 || !m_reset)
	{
		m_wdog = 0;
		watchdog_reset_w(space,0,0);
	}
}



READ8_MEMBER( namcos1_state::soundram_r )
{
	if (offset < 0x1000)
	{
		offset &= 0x3ff;

		/* CUS 30 */

		return machine().device<namco_cus30_device>("namco")->namcos1_cus30_r(space,offset);
	}
	else
	{
		offset &= 0x7ff;

		/* shared ram */
		return m_triram[offset];
	}
}

WRITE8_MEMBER( namcos1_state::soundram_w )
{
	if (offset < 0x1000)
	{
		offset &= 0x3ff;

		/* CUS 30 */

		machine().device<namco_cus30_device>("namco")->namcos1_cus30_w(space,offset,data);
	}
	else
	{
		offset &= 0x7ff;

		/* shared ram */
		m_triram[offset] = data;
		return;
	}
}

/* ROM handlers */

WRITE8_MEMBER( namcos1_state::rom_w )
{
	logerror("CPU %s PC %04x: warning - write %02x to rom address %04x\n", space.device().tag(), space.device().safe_pc(), data, offset);
}

/* error handlers */
READ8_MEMBER( namcos1_state::unknown_r )
{
	logerror("CPU %s PC %04x: warning - read from unknown chip\n", space.device().tag(), space.device().safe_pc() );
//  popmessage("CPU %s PC %04x: read from unknown chip", space.device().tag(), space.device().safe_pc() );
	return 0;
}

WRITE8_MEMBER( namcos1_state::unknown_w )
{
	logerror("CPU %s PC %04x: warning - wrote to unknown chip\n", space.device().tag(), space.device().safe_pc() );
//  popmessage("CPU %s PC %04x: wrote to unknown chip", space.device().tag(), space.device().safe_pc() );
}

/* Main bankswitching routine */
void namcos1_state::set_bank(int banknum, const bankhandler *handler)
{
	static const char *const banktags[] = {
		"bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8",
		"bank9", "bank10", "bank11", "bank12", "bank13", "bank14", "bank15", "bank16"
	};

	static const struct { read8_delegate func; } io_bank_handler_r[16] =
	{
		{ read8_delegate(FUNC(namcos1_state::bank1_r),this) }, { read8_delegate(FUNC(namcos1_state::bank2_r),this) }, { read8_delegate(FUNC(namcos1_state::bank3_r),this) }, { read8_delegate(FUNC(namcos1_state::bank4_r),this) },
		{ read8_delegate(FUNC(namcos1_state::bank5_r),this) }, { read8_delegate(FUNC(namcos1_state::bank6_r),this) }, { read8_delegate(FUNC(namcos1_state::bank7_r),this) }, { read8_delegate(FUNC(namcos1_state::bank8_r),this) },
		{ read8_delegate(FUNC(namcos1_state::bank9_r),this) }, { read8_delegate(FUNC(namcos1_state::bank10_r),this) }, { read8_delegate(FUNC(namcos1_state::bank11_r),this) }, { read8_delegate(FUNC(namcos1_state::bank12_r),this) },
		{ read8_delegate(FUNC(namcos1_state::bank13_r),this) }, { read8_delegate(FUNC(namcos1_state::bank14_r),this) }, { read8_delegate(FUNC(namcos1_state::bank15_r),this) }, { read8_delegate(FUNC(namcos1_state::bank16_r),this) }
	};

	static const struct { write8_delegate func; } io_bank_handler_w[16] =
	{
		{ write8_delegate(FUNC(namcos1_state::bank1_w),this) }, { write8_delegate(FUNC(namcos1_state::bank2_w),this) }, { write8_delegate(FUNC(namcos1_state::bank3_w),this) }, { write8_delegate(FUNC(namcos1_state::bank4_w),this) },
		{ write8_delegate(FUNC(namcos1_state::bank5_w),this) }, { write8_delegate(FUNC(namcos1_state::bank6_w),this) }, { write8_delegate(FUNC(namcos1_state::bank7_w),this) }, { write8_delegate(FUNC(namcos1_state::bank8_w),this) },
		{ write8_delegate(FUNC(namcos1_state::bank9_w),this) }, { write8_delegate(FUNC(namcos1_state::bank10_w),this) }, { write8_delegate(FUNC(namcos1_state::bank11_w),this) }, { write8_delegate(FUNC(namcos1_state::bank12_w),this) },
		{ write8_delegate(FUNC(namcos1_state::bank13_w),this) }, { write8_delegate(FUNC(namcos1_state::bank14_w),this) }, { write8_delegate(FUNC(namcos1_state::bank15_w),this) }, { write8_delegate(FUNC(namcos1_state::bank16_w),this) }
	};


	static const char *const cputags[] = { "maincpu", "sub" };

	address_space &space = machine().device(cputags[(banknum >> 3) & 1])->memory().space(AS_PROGRAM);
	int bankstart = (banknum & 7) * 0x2000;

	/* for BANK handlers , memory direct and OP-code base */
	if (handler->bank_pointer)
		membank(banktags[banknum])->set_base(handler->bank_pointer);

	/* read handlers */
	if (handler->bank_handler_r.isnull())
	{
		if (!m_active_bank[banknum].bank_handler_r.isnull())
			space.install_read_bank(bankstart, bankstart + 0x1fff, banktags[banknum]);
	}
	else
	{
		if (m_active_bank[banknum].bank_handler_r.isnull())
			space.install_read_handler(bankstart, bankstart + 0x1fff, io_bank_handler_r[banknum].func);
	}

	/* write handlers (except for the 0xe000-0xffff range) */
	if (bankstart != 0xe000)
	{
		if (handler->bank_handler_w.isnull())
		{
			if (!m_active_bank[banknum].bank_handler_w.isnull())
				space.install_write_bank(bankstart, bankstart + 0x1fff, banktags[banknum]);
		}
		else
		{
			if (m_active_bank[banknum].bank_handler_r.isnull())
				space.install_write_handler(bankstart, bankstart + 0x1fff, io_bank_handler_w[banknum].func);
		}
	}

	/* Remember this bank handler */
	m_active_bank[banknum] = *handler;
}

void namcos1_state::namcos1_bankswitch(int cpu, offs_t offset, UINT8 data)
{
	int bank = (cpu*8) + (( offset >> 9) & 0x07);
	int old = m_chip[bank];

	if (offset & 1)
	{
		m_chip[bank] &= 0x0300;
		m_chip[bank] |= (data & 0xff);
	}
	else
	{
		m_chip[bank] &= 0x00ff;
		m_chip[bank] |= (data & 0x03) << 8;
	}

	/* set_bank is slow, so only call it if uninitialized(unmapped) or changed */
	if (m_active_bank[bank].bank_handler_r == read8_delegate(FUNC(namcos1_state::unknown_r),this) || m_chip[bank] != old)
		set_bank(bank, &m_bank_element[m_chip[bank]]);

	/* unmapped bank warning */
	if( m_active_bank[bank].bank_handler_r == read8_delegate(FUNC(namcos1_state::unknown_r),this))
	{
		logerror("%s:warning unknown chip selected bank %x=$%04x\n", machine().describe_context(), bank , m_chip[bank] );
//      if (m_chip) popmessage("%s:unknown chip selected bank %x=$%04x", cpu , machine.describe_context(), bank , m_chip[bank] );
	}
}

WRITE8_MEMBER(namcos1_state::namcos1_bankswitch_w)
{
//  logerror("cpu %s: namcos1_bankswitch_w offset %04x data %02x\n", device().tag(), offset, data);

	namcos1_bankswitch((&space.device() == m_maincpu) ? 0 : 1, offset, data);
}

/* Sub cpu set start bank port */
WRITE8_MEMBER(namcos1_state::namcos1_subcpu_bank_w)
{
//  logerror("namcos1_subcpu_bank_w offset %04x data %02x\n",offset,data);

	/* Prepare code for CPU 1 */
	namcos1_bankswitch( 1, 0x0e00, 0x03 );
	namcos1_bankswitch( 1, 0x0e01, data );
}

/*******************************************************************************
*                                                                              *
*   Initialization                                                             *
*                                                                              *
*******************************************************************************/

void namcos1_state::namcos1_install_bank(int start,int end,read8_delegate hr,write8_delegate hw, int offset,UINT8 *pointer)
{
	int i;
	for(i=start;i<=end;i++)
	{
		m_bank_element[i].bank_handler_r = hr;
		m_bank_element[i].bank_handler_w = hw;
		m_bank_element[i].bank_offset    = offset;
		m_bank_element[i].bank_pointer   = pointer;
		offset  += 0x2000;
		if(pointer) pointer += 0x2000;
	}
}



void namcos1_state::namcos1_build_banks(read8_delegate key_r,write8_delegate key_w)
{
	int i;

	/**** kludge alert ****/
	UINT8 *dummyrom = auto_alloc_array(machine(), UINT8, 0x2000);

	/* when the games want to reset because the test switch has been flipped (or
	   because the protection checks failed!) they just set the top bits of bank #7
	   to 0, effectively crashing and waiting for the watchdog to kick in.
	   To avoid crashes in MAME, I prepare a dummy ROM containing just BRA -2 so
	   the program doesn't start executing code in unmapped areas.
	   Conveniently, the opcode for BRA -2 is 20 FE, and FE 20 FE is LDU $20FE,
	   so misaligned entry points get immediatly corrected. */
	for (i = 0;i < 0x2000;i+=2)
	{
		dummyrom[i]   = 0x20;
		dummyrom[i+1] = 0xfe;
	}
	/* also provide a valid IRQ vector */
	dummyrom[0x1ff8] = 0xff;
	dummyrom[0x1ff9] = 0x00;

	/* clear all banks to unknown area */
	for(i = 0;i < NAMCOS1_MAX_BANK;i++)
		namcos1_install_bank(i,i,read8_delegate(),write8_delegate(FUNC(namcos1_state::unknown_w),this),0,dummyrom);
	/**** end of kludge alert ****/


	/* 3D glasses */
	namcos1_install_bank(0x160,0x160,read8_delegate(),write8_delegate(FUNC(namcos1_state::namcos1_3dcs_w),this),0,0);
	/* RAM 6 banks - palette */
	namcos1_install_bank(0x170,0x173,read8_delegate(),write8_delegate(FUNC(namcos1_state::namcos1_paletteram_w),this),0,m_paletteram);
	/* RAM 5 banks - videoram */
	namcos1_install_bank(0x178,0x17b,read8_delegate(FUNC(namcos1_state::namcos1_videoram_r),this),write8_delegate(FUNC(namcos1_state::namcos1_videoram_w),this),0,0);
	/* key chip bank */
	namcos1_install_bank(0x17c,0x17c,key_r,key_w,0,0);
	/* RAM 7 banks - display control, playfields, sprites */
	namcos1_install_bank(0x17e,0x17e,read8_delegate(FUNC(namcos1_state::namcos1_spriteram_r),this),write8_delegate(FUNC(namcos1_state::namcos1_spriteram_w),this),0,0);
	/* RAM 1 shared ram, PSG device */
	namcos1_install_bank(0x17f,0x17f,read8_delegate(FUNC(namcos1_state::soundram_r),this),write8_delegate(FUNC(namcos1_state::soundram_w),this),0,0);
	/* RAM 3 banks */
	namcos1_install_bank(0x180,0x183,read8_delegate(),write8_delegate(),0,m_s1ram);

	/* PRG0-PRG7 */
	{
		UINT8 *rom = machine().root_device().memregion("user1")->base();

		namcos1_install_bank(0x200,0x3ff,read8_delegate(),write8_delegate(FUNC(namcos1_state::rom_w),this),0,rom);

		/* bit 16 of the address is inverted for PRG7 (and bits 17,18 just not connected) */
		for (i = 0x380000;i < 0x400000;i++)
		{
			if ((i & 0x010000) == 0)
			{
				UINT8 t = rom[i];
				rom[i] = rom[i + 0x010000];
				rom[i + 0x010000] = t;
			}
		}
	}
}

void namcos1_state::machine_reset()
{
	static const bankhandler unknown_handler = { read8_delegate(FUNC(namcos1_state::unknown_r),this), write8_delegate(FUNC(namcos1_state::unknown_w),this), 0, NULL };
	int bank;

	/* Point all of our bankhandlers to the error handlers */
	for (bank = 0; bank < 2*8 ; bank++)
		set_bank(bank, &unknown_handler);

	memset(m_chip, 0, sizeof(m_chip));

	/* Default MMU setup for Cpu 0 */
	namcos1_bankswitch(0, 0x0000, 0x01 ); /* bank0 = 0x180(RAM) - evidence: wldcourt */
	namcos1_bankswitch(0, 0x0001, 0x80 );
	namcos1_bankswitch(0, 0x0200, 0x01 ); /* bank1 = 0x180(RAM) - evidence: berabohm */
	namcos1_bankswitch(0, 0x0201, 0x80 );

	namcos1_bankswitch(0, 0x0e00, 0x03 ); /* bank7 = 0x3ff(PRG7) */
	namcos1_bankswitch(0, 0x0e01, 0xff );

	/* Default MMU setup for Cpu 1 */
	namcos1_bankswitch(1, 0x0000, 0x01 ); /* bank0 = 0x180(RAM) - evidence: wldcourt */
	namcos1_bankswitch(1, 0x0001, 0x80 );

	namcos1_bankswitch(1, 0x0e00, 0x03); /* bank7 = 0x3ff(PRG7) */
	namcos1_bankswitch(1, 0x0e01, 0xff);

	/* reset Cpu 0 and stop all other CPUs */
	m_maincpu->reset();
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	/* mcu patch data clear */
	m_mcu_patch_data = 0;
	m_reset = 0;

	namcos1_init_DACs();
	memset(m_key, 0, sizeof(m_key));
	m_wdog = 0;
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
WRITE8_MEMBER(namcos1_state::namcos1_mcu_bankswitch_w)
{
	int addr;

	/* bit 2-7 : chip select line of ROM chip */
	switch (data & 0xfc)
	{
		case 0xf8: addr = 0x10000; data ^= 2; break;    /* bit 2 : ROM 0 (bit 1 is inverted in that case) */
		case 0xf4: addr = 0x30000; break;               /* bit 3 : ROM 1 */
		case 0xec: addr = 0x50000; break;               /* bit 4 : ROM 2 */
		case 0xdc: addr = 0x70000; break;               /* bit 5 : ROM 3 */
		case 0xbc: addr = 0x90000; break;               /* bit 6 : ROM 4 */
		case 0x7c: addr = 0xb0000; break;               /* bit 7 : ROM 5 */
		default:   addr = 0x10000; break;               /* illegal */
	}

	/* bit 0-1 : address line A15-A16 */
	addr += (data & 3) * 0x8000;

	membank("bank20")->set_base(memregion("mcu")->base() + addr);
}



/* This point is very obscure, but i havent found any better way yet. */
/* Works with all games so far.                                       */

/* patch points of memory address */
/* CPU0/1 bank[17f][1000] */
/* CPU2   [7000]      */
/* CPU3   [c000]      */

/* This memory point should be set $A6 by anywhere, but         */
/* I found set $A6 only initialize in MCU                       */
/* This patch kill write this data by MCU case $A6 to xx(clear) */

WRITE8_MEMBER(namcos1_state::namcos1_mcu_patch_w)
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
void namcos1_state::namcos1_driver_init(const struct namcos1_specific *specific )
{
	static const struct namcos1_specific no_key =
	{
		read8_delegate(FUNC(namcos1_state::no_key_r),this), write8_delegate(FUNC(namcos1_state::no_key_w),this)
	};

	if (!specific) specific = &no_key;

	/* keychip id */
	m_key_id        = specific->key_id;
	/* for key type 3 */
	m_key_reg       = specific->key_reg1;
	m_key_rng       = specific->key_reg2;
	m_key_swap4_arg = specific->key_reg3;
	m_key_swap4     = specific->key_reg4;
	m_key_bottom4   = specific->key_reg5;
	m_key_top4      = specific->key_reg6;

	/* S1 RAM pointer set */
	m_s1ram = auto_alloc_array(machine(), UINT8, 0x8000);
	m_triram = auto_alloc_array(machine(), UINT8, 0x800);
	m_paletteram = auto_alloc_array(machine(), UINT8, 0x8000);

	/* Register volatile user memory for save state */
	save_pointer(NAME(m_s1ram), 0x8000);
	save_pointer(NAME(m_triram), 0x800);
	save_pointer(NAME(m_paletteram), 0x8000);

	/* Point mcu & sound shared RAM to destination */
	membank("bank18")->set_base(m_triram );
	membank("bank19")->set_base(m_triram );

	/* build bank elements */
	namcos1_build_banks(specific->key_r,specific->key_w);
}


/*******************************************************************************
*   Shadowland / Youkai Douchuuki specific                                     *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,shadowld)
{
	namcos1_driver_init(NULL);
}

/*******************************************************************************
*   Dragon Spirit specific (CUS136)                                            *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,dspirit)
{
	static const struct namcos1_specific dspirit_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type1_r),this), write8_delegate(FUNC(namcos1_state::key_type1_w),this), 0x36
	};
	namcos1_driver_init(&dspirit_specific);
}

/*******************************************************************************
*   World Court specific (CUS143)                                              *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,wldcourt)
{
	static const struct namcos1_specific worldcourt_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type1_r),this), write8_delegate(FUNC(namcos1_state::key_type1_w),this), 0x35
	};
	namcos1_driver_init(&worldcourt_specific);
}

/*******************************************************************************
*   Blazer specific (CUS144)                                                   *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,blazer)
{
	static const struct namcos1_specific blazer_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type1_r),this), write8_delegate(FUNC(namcos1_state::key_type1_w),this), 0x13
	};
	namcos1_driver_init(&blazer_specific);
}

/*******************************************************************************
*   Puzzle Club specific                                                       *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,puzlclub)
{
	static const struct namcos1_specific puzlclub_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type1_r),this), write8_delegate(FUNC(namcos1_state::key_type1_w),this), 0x35
	};
	namcos1_driver_init(&puzlclub_specific);
}

/*******************************************************************************
*   Pac-Mania specific (CUS151)                                                *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,pacmania)
{
	static const struct namcos1_specific pacmania_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type2_r),this), write8_delegate(FUNC(namcos1_state::key_type2_w),this), 0x12
	};
	namcos1_driver_init(&pacmania_specific);
}

/*******************************************************************************
*   Alice in Wonderland / Marchen Maze specific (CUS152)                       *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,alice)
{
	static const struct namcos1_specific alice_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type2_r),this), write8_delegate(FUNC(namcos1_state::key_type2_w),this), 0x25
	};
	namcos1_driver_init(&alice_specific);
}

/*******************************************************************************
*   Galaga '88 specific (CUS153)                                               *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,galaga88)
{
	static const struct namcos1_specific galaga88_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type2_r),this), write8_delegate(FUNC(namcos1_state::key_type2_w),this), 0x31
	};
	namcos1_driver_init(&galaga88_specific);
}

/*******************************************************************************
*   World Stadium specific (CUS154)                                            *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,ws)
{
	static const struct namcos1_specific ws_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type2_r),this), write8_delegate(FUNC(namcos1_state::key_type2_w),this), 0x07
	};
	namcos1_driver_init(&ws_specific);
}

/*******************************************************************************
*   Bakutotsu Kijuutei specific (CUS155)                                       *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,bakutotu)
{
	static const struct namcos1_specific bakutotu_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type2_r),this), write8_delegate(FUNC(namcos1_state::key_type2_w),this), 0x22
	};
	namcos1_driver_init(&bakutotu_specific);

#if 0
	// resolves CPU deadlocks caused by sloppy coding(see driver\namcos1.c)
	{
		static const UINT8 target[8] = {0x34,0x37,0x35,0x37,0x96,0x00,0x2e,0xed};
		UINT8 *rombase, *srcptr, *endptr, *scanptr;

		rombase = memregion("user1")->base();
		srcptr = rombase + 0x1e000;
		endptr = srcptr + 0xa000;

		while ( (scanptr = memchr(srcptr, 0x34, endptr-srcptr)) )
		{
			if (!memcmp(scanptr, target, 8))
			{
				scanptr[7] = 0xfc;
				srcptr = scanptr + 8;

				logerror ("faulty loop patched at %06x\n", scanptr-rombase+7);
			}
			else
				srcptr = scanptr + 1;
		}
	}
#endif
}

/*******************************************************************************
*   Splatter House specific (CUS181)                                           *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,splatter)
{
	static const struct namcos1_specific splatter_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 181, 3, 4,-1,-1,-1,-1
	};

	namcos1_driver_init(&splatter_specific);
}

/*******************************************************************************
*   Rompers specific (CUS182)                                                  *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,rompers)
{
	static const struct namcos1_specific rompers_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 182, 7,-1,-1,-1,-1,-1
	};
	namcos1_driver_init(&rompers_specific);
}

/*******************************************************************************
*   Blast Off specific (CUS183)                                                *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,blastoff)
{
	static const struct namcos1_specific blastoff_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 183, 0, 7, 3, 5,-1,-1
	};
	namcos1_driver_init(&blastoff_specific);
}

/*******************************************************************************
*   World Stadium '89 specific (CUS184)                                        *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,ws89)
{
	static const struct namcos1_specific ws89_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 184, 2,-1,-1,-1,-1,-1
	};
	namcos1_driver_init(&ws89_specific);
}

/*******************************************************************************
*   Tank Force specific (CUS185)                                               *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,tankfrce)
{
	static const struct namcos1_specific tankfrce_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 185, 5,-1, 1,-1, 2,-1
	};
	namcos1_driver_init(&tankfrce_specific);
}



DRIVER_INIT_MEMBER(namcos1_state,tankfrc4)
{
	static const struct namcos1_specific tankfrce_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 185, 5,-1, 1,-1, 2,-1
	};
	namcos1_driver_init(&tankfrce_specific);

	m_mcu->space(AS_PROGRAM).install_read_handler(0x1400, 0x1401, read8_delegate(FUNC(namcos1_state::faceoff_inputs_r), this));
}

/*******************************************************************************
*   Dangerous Seed specific (CUS308)                                           *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,dangseed)
{
	static const struct namcos1_specific dangseed_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 308, 6,-1, 5,-1, 0, 4
	};
	namcos1_driver_init(&dangseed_specific);
}

/*******************************************************************************
*   Pistol Daimyo no Bouken specific (CUS309)                                  *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,pistoldm)
{
	static const struct namcos1_specific pistoldm_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 309, 1, 2, 0,-1, 4,-1
	};
	namcos1_driver_init(&pistoldm_specific);
}

/*******************************************************************************
*   World Stadium '90 specific (CUS310)                                        *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,ws90)
{
	static const struct namcos1_specific ws90_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 310, 4,-1, 7,-1, 3,-1
	};
	namcos1_driver_init(&ws90_specific);
}

/*******************************************************************************
*   Souko Ban DX specific (CUS311)                                             *
*******************************************************************************/
DRIVER_INIT_MEMBER(namcos1_state,soukobdx)
{
	static const struct namcos1_specific soukobdx_specific=
	{
		read8_delegate(FUNC(namcos1_state::key_type3_r),this), write8_delegate(FUNC(namcos1_state::key_type3_w),this), 311, 2, 3/*?*/, 0,-1, 4,-1
	};
	namcos1_driver_init(&soukobdx_specific);
}



/*******************************************************************************
*   Quester specific                                                           *
*******************************************************************************/
READ8_MEMBER( namcos1_state::quester_paddle_r )
{
	static int qnum=0, qstrobe=0;

	if (offset == 0)
	{
		int ret;

		if (!qnum)
			ret = (ioport("CONTROL0")->read()&0x90) | qstrobe | (ioport("PADDLE0")->read()&0x0f);
		else
			ret = (ioport("CONTROL0")->read()&0x90) | qstrobe | (ioport("PADDLE1")->read()&0x0f);

		qstrobe ^= 0x40;

		return ret;
	}
	else
	{
		int ret;

		if (!qnum)
			ret = (ioport("CONTROL1")->read()&0x90) | qnum | (ioport("PADDLE0")->read()>>4);
		else
			ret = (ioport("CONTROL1")->read()&0x90) | qnum | (ioport("PADDLE1")->read()>>4);

		if (!qstrobe) qnum ^= 0x20;

		return ret;
	}
}

DRIVER_INIT_MEMBER(namcos1_state,quester)
{
	namcos1_driver_init(NULL);
	m_mcu->space(AS_PROGRAM).install_read_handler(0x1400, 0x1401, read8_delegate(FUNC(namcos1_state::quester_paddle_r), this));
}



/*******************************************************************************
*   Beraboh Man specific                                                       *
*******************************************************************************/

READ8_MEMBER( namcos1_state::berabohm_buttons_r )
{
	int res;
	static int input_count, strobe, strobe_count;


	if (offset == 0)
	{
		int inp = input_count;

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
		if (++strobe_count > 4)
		{
			strobe_count = 0;
			strobe ^= 0x40;
			if (strobe == 0)
			{
				input_count = (input_count + 1) % 5;
				if (input_count == 3) res |= 0x10;
			}
		}

		res |= strobe;

		return res;
	}
}

DRIVER_INIT_MEMBER(namcos1_state,berabohm)
{
	namcos1_driver_init(NULL);
	m_mcu->space(AS_PROGRAM).install_read_handler(0x1400, 0x1401, read8_delegate(FUNC(namcos1_state::berabohm_buttons_r), this));
}



/*******************************************************************************
*   Face Off specific                                                          *
*******************************************************************************/

// used by faceoff and tankforce 4 player (input multiplex)

READ8_MEMBER( namcos1_state::faceoff_inputs_r )
{
	int res;
	static int input_count, strobe_count, stored_input[2];

	if (offset == 0)
	{
		res = (ioport("CONTROL0")->read() & 0x80) | stored_input[0];

		return res;
	}
	else
	{
		res = ioport("CONTROL1")->read() & 0x80;

		/* the strobe cannot happen too often, otherwise the MCU will waste too
		   much time reading the inputs and won't have enough cycles to play two
		   digital sounds at once. This value is enough to read all inputs at least
		   once per frame */
		if (++strobe_count > 8)
		{
			strobe_count = 0;

			res |= input_count;

			switch (input_count)
			{
				case 0:
					stored_input[0] = ioport("IN0")->read() & 0x1f;
					stored_input[1] = (ioport("IN3")->read() & 0x07) << 3;
					break;

				case 3:
					stored_input[0] = ioport("IN2")->read() & 0x1f;
					break;

				case 4:
					stored_input[0] = ioport("IN1")->read() & 0x1f;
					stored_input[1] = ioport("IN3")->read() & 0x18;
					break;

				default:
					stored_input[0] = 0x1f;
					stored_input[1] = 0x1f;
					break;
			}

			input_count = (input_count + 1) & 7;
		}
		else
		{
			res |= 0x40 | stored_input[1];
		}

		return res;
	}
}

DRIVER_INIT_MEMBER(namcos1_state,faceoff)
{
	namcos1_driver_init(NULL);
	m_mcu->space(AS_PROGRAM).install_read_handler(0x1400, 0x1401, read8_delegate(FUNC(namcos1_state::faceoff_inputs_r), this));
}
