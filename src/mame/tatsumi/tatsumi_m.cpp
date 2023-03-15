// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
#include "emu.h"
#include "tatsumi.h"
#include "sound/okim6295.h"


/******************************************************************************/

void tatsumi_state::tatsumi_reset()
{
	m_last_control = 0;
	m_control_word = 0;

	save_item(NAME(m_last_control));
	save_item(NAME(m_control_word));
}

/******************************************************************************/

uint16_t apache3_state::apache3_bank_r()
{
	return m_control_word;
}

void apache3_state::apache3_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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
void apache3_state::apache3_z80_ctrl_w(uint16_t data)
{
	m_subcpu2->set_input_line(INPUT_LINE_HALT, data & 2 ? ASSERT_LINE : CLEAR_LINE);
}

uint16_t apache3_state::apache3_v30_v20_r(offs_t offset)
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
		logerror("%08x: unmapped read z80 rom %08x\n", m_maincpu->pc(), offset);
	return 0xff00 | targetspace.read_byte(offset);
}

void apache3_state::apache3_v30_v20_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	if ((m_control_word & 0xe0) != 0x80)
		logerror("%08x: write unmapped v30 rom %08x\n", m_maincpu->pc(), offset);

	/* Only 8 bits of the V30 data bus are connected - ignore writes to the other half */
	if (ACCESSING_BITS_0_7)
	{
		targetspace.write_byte(offset, data & 0xff);
	}
}

uint16_t apache3_state::apache3_z80_r(offs_t offset)
{
	return m_apache3_z80_ram[offset];
}

void apache3_state::apache3_z80_w(offs_t offset, uint16_t data)
{
	m_apache3_z80_ram[offset] = data & 0xff;
}

uint8_t apache3_state::apache3_vr1_r()
{
	return (uint8_t)((255./100) * (100 - m_vr1->read()));
}

/* Ground/sky rotation control
 *
 * There are 12 16-bit values that are
 * presumably loaded into the 8 TZ2213 custom
 * accumulators and counters.
 */
void apache3_state::apache3_rotate_w(uint16_t data)
{
	m_apache3_rotate_ctrl[m_apache3_rot_idx] = data;
	m_apache3_rot_idx = (m_apache3_rot_idx + 1) % 12;
}

/******************************************************************************/

uint16_t roundup5_state::roundup_v30_z80_r(offs_t offset)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	/* Each Z80 byte maps to a V30 word */
	if (m_control_word & 0x20)
		offset += 0x8000; /* Upper half */

	return 0xff00 | targetspace.read_byte(offset);
}

void roundup5_state::roundup_v30_z80_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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


void roundup5_state::roundup5_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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
//      logerror("%s:  Changed bank to %04x (%d)\n", m_maincpu->pc(), tatsumi_control_w,offset);

//todo - watchdog

	/* Bank:

	    0x0017  :   OBJ banks
	    0x0018  :   68000 RAM       mask 0x0380 used to save bits when writing
	    0x0c10  :   68000 ROM

	    0x0040  :   Z80 rom (lower half) mapped to 0x10000
	    0x0060  :   Z80 rom (upper half) mapped to 0x10000

	    0x0080  :   enabled when showing map screen after a play
	                (switches video priority between text layer and sprites)

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

void roundup5_state::road_vregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	    ---- ---x ---- ---- enabled when there's a road slope of any kind, unknown purpose
	    ---- ---- -xx- ---- enables alternatively in tunnels sometimes, color mods?
	    ---- ---- ---x ---- road bank select
	    ---- ---- ---- xxxx various values written during POST while accessing road pixel ram,
	                        otherwise 0xb at the start of irq service
	*/

	COMBINE_DATA(&m_road_vregs[offset]);

	m_subcpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE); // guess, probably wrong
//  logerror("d_68k_e0000_w %s %04x\n", m_maincpu->pc(), data);
}

/******************************************************************************/

void cyclwarr_state::cyclwarr_control_w(uint8_t data)
{
	m_control_word = data;

//  if ((m_control_word&0xfe) != (m_last_control&0xfe))
//      logerror("%s  control_w %04x\n", m_maincpu->pc(), data);

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

	m_last_control = m_control_word;
}

/******************************************************************************/

uint16_t tatsumi_state::tatsumi_v30_68000_r(offs_t offset)
{
	const uint16_t* rom=(uint16_t*)m_subregion->base();

//logerror("%s:68000_r(%04X),cw=%04X\n", m_maincpu->pc(), offset*2, m_control_word);
	/* Read from 68k RAM */
	if ((m_control_word&0x1f)==0x18)
	{
#if 0
		// hack to make roundup 5 boot
		// doesn't seem necessary anymore, left for reference
		if (m_maincpu->pc()==0xec575)
		{
			uint8_t *dst = m_mainregion->base();
			dst[BYTE_XOR_LE(0xec57a)]=0x46;
			dst[BYTE_XOR_LE(0xec57b)]=0x46;

			dst[BYTE_XOR_LE(0xfc520)]=0x46; //code that stops cpu after coin counter goes mad..
			dst[BYTE_XOR_LE(0xfc521)]=0x46;
			dst[BYTE_XOR_LE(0xfc522)]=0x46;
			dst[BYTE_XOR_LE(0xfc523)]=0x46;
			dst[BYTE_XOR_LE(0xfc524)]=0x46;
			dst[BYTE_XOR_LE(0xfc525)]=0x46;
		}
#endif

		return m_sharedram[offset & 0x1fff];
	}

	/* Read from 68k ROM */
	offset+=(m_control_word&0x7)*0x8000;

	return rom[offset];
}

void tatsumi_state::tatsumi_v30_68000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ((m_control_word&0x1f)!=0x18)
		logerror("68k write in bank %05x\n",m_control_word);

	COMBINE_DATA(&m_sharedram[offset]);
}

/***********************************************************************************/

uint8_t cyclwarr_state::oki_status_xor_r()
{
	int r = m_oki->read();

	// Cycle Warriors and Big Fight access this with reversed activeness.
	// this is particularly noticeable with the "We got em" sample played in CW at stage clear:
	// gets cut too early with the old hack below.
	// fwiw returning normal oki status doesn't work at all, both games don't make any sound.
	// TODO: verify with HW
	return (r ^ 0xff);
#if 0
	// old hack left for reference

	if (m_audiocpu->pc()==0x2b70 || m_audiocpu->pc()==0x2bb5
		|| m_audiocpu->pc()==0x2acc
		|| m_audiocpu->pc()==0x1c79 // BigFight
		|| m_audiocpu->pc()==0x1cbe // BigFight
		|| m_audiocpu->pc()==0xf9881)
		return 0xf;
	if (m_audiocpu->pc()==0x2ba3 || m_audiocpu->pc()==0x2a9b || m_audiocpu->pc()==0x2adc
		|| m_audiocpu->pc()==0x1cac) // BigFight
		return 0;
	return r;
#endif
}
