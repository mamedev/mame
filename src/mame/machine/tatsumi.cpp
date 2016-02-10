// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "emu.h"
#include "includes/tatsumi.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"


/******************************************************************************/

void tatsumi_state::tatsumi_reset()
{
	m_last_control = 0;
	m_control_word = 0;
	m_apache3_adc = 0;
	m_apache3_rot_idx = 0;

	save_item(NAME(m_last_control));
	save_item(NAME(m_control_word));
	save_item(NAME(m_apache3_adc));
}

/******************************************************************************/

READ16_MEMBER(tatsumi_state::apache3_bank_r)
{
	return m_control_word;
}

WRITE16_MEMBER(tatsumi_state::apache3_bank_w)
{
	/*
	    0x8000  - Set when accessing palette ram (not implemented, perhaps blank screen?)
	    0x0080  - Set when accessing IO cpu RAM/ROM (implemented as halt cpu)
	    0x0060  - IOP bank to access from main cpu (0x0 = RAM, 0x20 = lower ROM, 0x60 = upper ROM)
	    0x0010  - Set when accessing OBJ cpu RAM/ROM (implemented as halt cpu)
	    0x000f  - OBJ bank to access from main cpu (0x8 = RAM, 0x0 to 0x7 = ROM)
	*/

	COMBINE_DATA(&m_control_word);

	if (m_control_word & 0x7f00)
	{
		logerror("Unknown control Word: %04x\n",m_control_word);
		m_subcpu2->set_input_line(INPUT_LINE_HALT, CLEAR_LINE); // ?
	}

	if (m_control_word & 0x10)
		m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	else
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	if (m_control_word & 0x80)
		m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	else
		m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	m_last_control=m_control_word;
}

// D1 = /ZBREQ  - Z80 bus request
// D0 = /GRDACC - Allow 68000 access to road pattern RAM
WRITE16_MEMBER(tatsumi_state::apache3_z80_ctrl_w)
{
	m_subcpu2->set_input_line(INPUT_LINE_HALT, data & 2 ? ASSERT_LINE : CLEAR_LINE);
}

READ16_MEMBER(tatsumi_state::apache3_v30_v20_r)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	/* Each V20 byte maps to a V30 word */
	if ((m_control_word & 0xe0) == 0xe0)
		offset += 0xf8000; /* Upper half */
	else if ((m_control_word & 0xe0) == 0xc0)
		offset += 0xf0000;
	else if ((m_control_word & 0xe0) == 0x80)
		offset += 0x00000; // main ram
	else
		logerror("%08x: unmapped read z80 rom %08x\n", space.device().safe_pc(), offset);
	return 0xff00 | targetspace.read_byte(offset);
}

WRITE16_MEMBER(tatsumi_state::apache3_v30_v20_w)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	if ((m_control_word & 0xe0) != 0x80)
		logerror("%08x: write unmapped v30 rom %08x\n", space.device().safe_pc(), offset);

	/* Only 8 bits of the V30 data bus are connected - ignore writes to the other half */
	if (ACCESSING_BITS_0_7)
	{
		targetspace.write_byte(offset, data & 0xff);
	}
}

READ16_MEMBER(tatsumi_state::apache3_z80_r)
{
	return m_apache3_z80_ram[offset];
}

WRITE16_MEMBER(tatsumi_state::apache3_z80_w)
{
	m_apache3_z80_ram[offset] = data & 0xff;
}

READ8_MEMBER(tatsumi_state::apache3_adc_r)
{
	switch (m_apache3_adc)
	{
		case 0: return ioport("STICK_X")->read();
		case 1: return ioport("STICK_Y")->read();
		case 2: return 0; // VSP1
		case 3: return 0;
		case 4: return (UINT8)((255./100) * (100 - ioport("VR1")->read()));
		case 5: return ioport("THROTTLE")->read();
		case 6: return 0; // RPSNC
		case 7: return 0; // LPSNC
	}

	return 0;
}

WRITE8_MEMBER(tatsumi_state::apache3_adc_w)
{
	m_apache3_adc = offset;
}

/* Ground/sky rotation control
 *
 * There are 12 16-bit values that are
 * presumably loaded into the 8 TZ2213 custom
 * accumulators and counters.
 */
WRITE16_MEMBER(tatsumi_state::apache3_rotate_w)
{
	m_apache3_rotate_ctrl[m_apache3_rot_idx] = data;
	m_apache3_rot_idx = (m_apache3_rot_idx + 1) % 12;
}

/******************************************************************************/

READ16_MEMBER(tatsumi_state::roundup_v30_z80_r)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	/* Each Z80 byte maps to a V30 word */
	if (m_control_word & 0x20)
		offset += 0x8000; /* Upper half */

	return 0xff00 | targetspace.read_byte(offset);
}

WRITE16_MEMBER(tatsumi_state::roundup_v30_z80_w)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	/* Only 8 bits of the V30 data bus are connected - ignore writes to the other half */
	if (ACCESSING_BITS_0_7)
	{
		if (m_control_word & 0x20)
			offset += 0x8000; /* Upper half of Z80 address space */

		targetspace.write_byte(offset, data & 0xff);
	}
}


WRITE16_MEMBER(tatsumi_state::roundup5_control_w)
{
	COMBINE_DATA(&m_control_word);

	if (m_control_word & 0x10)
		m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	else
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	if (m_control_word & 0x4)
		m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	else
		m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

//  if (offset == 1 && (tatsumi_control_w & 0xfeff) != (last_bank & 0xfeff))
//      logerror("%08x:  Changed bank to %04x (%d)\n", space.device().safe_pc(), tatsumi_control_w,offset);

//todo - watchdog

	/* Bank:

	    0x0017  :   OBJ banks
	    0x0018  :   68000 RAM       mask 0x0380 used to save bits when writing
	    0x0c10  :   68000 ROM

	    0x0040  :   Z80 rom (lower half) mapped to 0x10000
	    0x0060  :   Z80 rom (upper half) mapped to 0x10000

	    0x0100  :   watchdog.

	    0x0c00  :   vram bank (bits 0x7000 also set when writing vram)

	    0x8000  :   set whenever writing to palette ram?

	    Changed bank to 0060 (0)
	*/

	if ((m_control_word & 0x8) == 0 && !(m_last_control & 0x8))
		m_subcpu->set_input_line(INPUT_LINE_IRQ4, ASSERT_LINE);
//  if (tatsumi_control_w&0x200)
//      cpu_set_reset_line(1, CLEAR_LINE);
//  else
//      cpu_set_reset_line(1, ASSERT_LINE);

//  if ((tatsumi_control_w&0x200) && (last_bank&0x200)==0)
//      logerror("68k irq\n");
//  if ((tatsumi_control_w&0x200)==0 && (last_bank&0x200)==0x200)
//      logerror("68k reset\n");

	if (m_control_word == 0x3a00)
	{
//      cpu_set_reset_line(1, CLEAR_LINE);
//      logerror("68k on\n");
	}

	m_last_control = m_control_word;
}

WRITE16_MEMBER(tatsumi_state::roundup5_d0000_w)
{
	COMBINE_DATA(&m_roundup5_d0000_ram[offset]);
//  logerror("d_68k_d0000_w %06x %04x\n", space.device().safe_pc(), data);
}

WRITE16_MEMBER(tatsumi_state::roundup5_e0000_w)
{
	/*  Bit 0x10 is road bank select,
	    Bit 0x100 is used, but unknown
	*/

	COMBINE_DATA(&m_roundup5_e0000_ram[offset]);
	m_subcpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE); // guess, probably wrong

//  logerror("d_68k_e0000_w %06x %04x\n", space.device().safe_pc(), data);
}

/******************************************************************************/

READ16_MEMBER(tatsumi_state::cyclwarr_control_r)
{
//  logerror("%08x:  control_r\n", space.device().safe_pc());
	return m_control_word;
}

WRITE16_MEMBER(tatsumi_state::cyclwarr_control_w)
{
	COMBINE_DATA(&m_control_word);

//  if ((m_control_word&0xfe) != (m_last_control&0xfe))
//      logerror("%08x:  control_w %04x\n", space.device().safe_pc(), data);

/*

0x1 - watchdog
0x4 - cpu bus lock



*/

	if ((m_control_word & 4) == 4 && (m_last_control & 4) == 0)
	{
//      logerror("68k 2 halt\n");
		m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	if ((m_control_word & 4) == 0 && (m_last_control & 4) == 4)
	{
//      logerror("68k 2 irq go\n");
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}


	// hack
	if (space.device().safe_pc() == 0x2c3c34)
	{
//      cpu_set_reset_line(1, CLEAR_LINE);
//      logerror("hack 68k2 on\n");
	}

	m_last_control = m_control_word;
}

/******************************************************************************/

READ16_MEMBER(tatsumi_state::tatsumi_v30_68000_r)
{
	const UINT16* rom=(UINT16*)memregion("sub")->base();

logerror("%05X:68000_r(%04X),cw=%04X\n", space.device().safe_pc(), offset*2, m_control_word);
	/* Read from 68k RAM */
	if ((m_control_word&0x1f)==0x18)
	{
		// hack to make roundup 5 boot
		if (space.device().safe_pc()==0xec575)
		{
			UINT8 *dst = memregion("maincpu")->base();
			dst[BYTE_XOR_LE(0xec57a)]=0x46;
			dst[BYTE_XOR_LE(0xec57b)]=0x46;

			dst[BYTE_XOR_LE(0xfc520)]=0x46; //code that stops cpu after coin counter goes mad..
			dst[BYTE_XOR_LE(0xfc521)]=0x46;
			dst[BYTE_XOR_LE(0xfc522)]=0x46;
			dst[BYTE_XOR_LE(0xfc523)]=0x46;
			dst[BYTE_XOR_LE(0xfc524)]=0x46;
			dst[BYTE_XOR_LE(0xfc525)]=0x46;
		}

		return m_68k_ram[offset & 0x1fff];
	}

	/* Read from 68k ROM */
	offset+=(m_control_word&0x7)*0x8000;

	return rom[offset];
}

WRITE16_MEMBER(tatsumi_state::tatsumi_v30_68000_w)
{
	if ((m_control_word&0x1f)!=0x18)
		logerror("68k write in bank %05x\n",m_control_word);

	COMBINE_DATA(&m_68k_ram[offset]);
}

/***********************************************************************************/

// Todo:  Current YM2151 doesn't seem to raise the busy flag quickly enough for the
// self-test in Tatsumi games.  Needs fixed, but hack it here for now.
READ8_MEMBER(tatsumi_state::tatsumi_hack_ym2151_r)
{
	int r=machine().device<ym2151_device>("ymsnd")->status_r(space,0);

	if (space.device().safe_pc()==0x2aca || space.device().safe_pc()==0x29fe
		|| space.device().safe_pc()==0xf9721
		|| space.device().safe_pc()==0x1b96 || space.device().safe_pc()==0x1c65) // BigFight
		return 0x80;
	return r;
}

READ8_MEMBER(tatsumi_state::tatsumi_hack_oki_r)
{
	int r=m_oki->read(space,0);

	if (space.device().safe_pc()==0x2b70 || space.device().safe_pc()==0x2bb5
		|| space.device().safe_pc()==0x2acc
		|| space.device().safe_pc()==0x1c79 // BigFight
		|| space.device().safe_pc()==0x1cbe // BigFight
		|| space.device().safe_pc()==0xf9881)
		return 0xf;
	if (space.device().safe_pc()==0x2ba3 || space.device().safe_pc()==0x2a9b || space.device().safe_pc()==0x2adc
		|| space.device().safe_pc()==0x1cac) // BigFight
		return 0;
	return r;
}
