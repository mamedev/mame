#include "emu.h"
#include "includes/tatsumi.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"


/******************************************************************************/

void tatsumi_reset(running_machine &machine)
{
	tatsumi_state *state = machine.driver_data<tatsumi_state>();
	state->m_last_control = 0;
	state->m_control_word = 0;
	state->m_apache3_adc = 0;
	state->m_apache3_rot_idx = 0;

	state_save_register_global(machine, state->m_last_control);
	state_save_register_global(machine, state->m_control_word);
	state_save_register_global(machine, state->m_apache3_adc);
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
		cputag_set_input_line(machine(), "sub2", INPUT_LINE_HALT, CLEAR_LINE); // ?
	}

	if (m_control_word & 0x10)
		cputag_set_input_line(machine(), "sub", INPUT_LINE_HALT, ASSERT_LINE);
	else
		cputag_set_input_line(machine(), "sub", INPUT_LINE_HALT, CLEAR_LINE);

	if (m_control_word & 0x80)
		cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_HALT, ASSERT_LINE);
	else
		cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_HALT, CLEAR_LINE);

	m_last_control=m_control_word;
}

// D1 = /ZBREQ  - Z80 bus request
// D0 = /GRDACC - Allow 68000 access to road pattern RAM
WRITE16_MEMBER(tatsumi_state::apache3_z80_ctrl_w)
{
	cputag_set_input_line(machine(), "sub2", INPUT_LINE_HALT, data & 2 ? ASSERT_LINE : CLEAR_LINE);
}

READ16_MEMBER(tatsumi_state::apache3_v30_v20_r)
{
	address_space *targetspace = machine().device("audiocpu")->memory().space(AS_PROGRAM);

	/* Each V20 byte maps to a V30 word */
	if ((m_control_word & 0xe0) == 0xe0)
		offset += 0xf8000; /* Upper half */
	else if ((m_control_word & 0xe0) == 0xc0)
		offset += 0xf0000;
	else if ((m_control_word & 0xe0) == 0x80)
		offset += 0x00000; // main ram
	else
		logerror("%08x: unmapped read z80 rom %08x\n", cpu_get_pc(&space.device()), offset);
	return 0xff00 | targetspace->read_byte(offset);
}

WRITE16_MEMBER(tatsumi_state::apache3_v30_v20_w)
{
	address_space *targetspace = machine().device("audiocpu")->memory().space(AS_PROGRAM);

	if ((m_control_word & 0xe0) != 0x80)
		logerror("%08x: write unmapped v30 rom %08x\n", cpu_get_pc(&space.device()), offset);

	/* Only 8 bits of the V30 data bus are connected - ignore writes to the other half */
	if (ACCESSING_BITS_0_7)
	{
		targetspace->write_byte(offset, data & 0xff);
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
		case 0: return input_port_read(machine(), "STICK_X");
		case 1: return input_port_read(machine(), "STICK_Y");
		case 2: return 0; // VSP1
		case 3: return 0;
		case 4: return (UINT8)((255./100) * (100 - input_port_read(machine(), "VR1")));
		case 5: return input_port_read(machine(), "THROTTLE");
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
	address_space *targetspace = machine().device("audiocpu")->memory().space(AS_PROGRAM);

	/* Each Z80 byte maps to a V30 word */
	if (m_control_word & 0x20)
		offset += 0x8000; /* Upper half */

	return 0xff00 | targetspace->read_byte(offset);
}

WRITE16_MEMBER(tatsumi_state::roundup_v30_z80_w)
{
	address_space *targetspace = machine().device("audiocpu")->memory().space(AS_PROGRAM);

	/* Only 8 bits of the V30 data bus are connected - ignore writes to the other half */
	if (ACCESSING_BITS_0_7)
	{
		if (m_control_word & 0x20)
			offset += 0x8000; /* Upper half of Z80 address &space */

		targetspace->write_byte(offset, data & 0xff);
	}
}


WRITE16_MEMBER(tatsumi_state::roundup5_control_w)
{
	COMBINE_DATA(&m_control_word);

	if (m_control_word & 0x10)
		cputag_set_input_line(machine(), "sub", INPUT_LINE_HALT, ASSERT_LINE);
	else
		cputag_set_input_line(machine(), "sub", INPUT_LINE_HALT, CLEAR_LINE);

	if (m_control_word & 0x4)
		cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_HALT, ASSERT_LINE);
	else
		cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_HALT, CLEAR_LINE);

//  if (offset == 1 && (tatsumi_control_w & 0xfeff) != (last_bank & 0xfeff))
//      logerror("%08x:  Changed bank to %04x (%d)\n", cpu_get_pc(&space.device()), tatsumi_control_w,offset);

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
		cputag_set_input_line(machine(), "sub", INPUT_LINE_IRQ4, ASSERT_LINE);
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
//  logerror("d_68k_d0000_w %06x %04x\n", cpu_get_pc(&space.device()), data);
}

WRITE16_MEMBER(tatsumi_state::roundup5_e0000_w)
{
	/*  Bit 0x10 is road bank select,
        Bit 0x100 is used, but unknown
    */

	COMBINE_DATA(&m_roundup5_e0000_ram[offset]);
	cputag_set_input_line(machine(), "sub", INPUT_LINE_IRQ4, CLEAR_LINE); // guess, probably wrong

//  logerror("d_68k_e0000_w %06x %04x\n", cpu_get_pc(&space.device()), data);
}

/******************************************************************************/

READ16_MEMBER(tatsumi_state::cyclwarr_control_r)
{
//  logerror("%08x:  control_r\n", cpu_get_pc(&space.device()));
	return m_control_word;
}

WRITE16_MEMBER(tatsumi_state::cyclwarr_control_w)
{
	COMBINE_DATA(&m_control_word);

//  if ((m_control_word&0xfe) != (m_last_control&0xfe))
//      logerror("%08x:  control_w %04x\n", cpu_get_pc(&space.device()), data);

/*

0x1 - watchdog
0x4 - cpu bus lock



*/

	if ((m_control_word & 4) == 4 && (m_last_control & 4) == 0)
	{
//      logerror("68k 2 halt\n");
		cputag_set_input_line(machine(), "sub", INPUT_LINE_HALT, ASSERT_LINE);
	}

	if ((m_control_word & 4) == 0 && (m_last_control & 4) == 4)
	{
//      logerror("68k 2 irq go\n");
		cputag_set_input_line(machine(), "sub", INPUT_LINE_HALT, CLEAR_LINE);
	}


	// hack
	if (cpu_get_pc(&space.device()) == 0x2c3c34)
	{
//      cpu_set_reset_line(1, CLEAR_LINE);
//      logerror("hack 68k2 on\n");
	}

	m_last_control = m_control_word;
}

/******************************************************************************/

READ16_MEMBER(tatsumi_state::tatsumi_v30_68000_r)
{
	const UINT16* rom=(UINT16*)machine().region("sub")->base();

logerror("%05X:68000_r(%04X),cw=%04X\n", cpu_get_pc(&space.device()), offset*2, m_control_word);
	/* Read from 68k RAM */
	if ((m_control_word&0x1f)==0x18)
	{
		// hack to make roundup 5 boot
		if (cpu_get_pc(&space.device())==0xec575)
		{
			UINT8 *dst = machine().region("maincpu")->base();
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
READ8_DEVICE_HANDLER(tatsumi_hack_ym2151_r)
{
	address_space *space = device->machine().device("audiocpu")->memory().space(AS_PROGRAM);
	int r=ym2151_status_port_r(device,0);

	if (cpu_get_pc(&space->device())==0x2aca || cpu_get_pc(&space->device())==0x29fe
		|| cpu_get_pc(&space->device())==0xf9721
		|| cpu_get_pc(&space->device())==0x1b96 || cpu_get_pc(&space->device())==0x1c65) // BigFight
		return 0x80;
	return r;
}

// Todo:  Tatsumi self-test fails if OKI doesn't respond (when sound off).
// Mame really should emulate the OKI status reads even with Mame sound off.
READ8_DEVICE_HANDLER(tatsumi_hack_oki_r)
{
	address_space *space = device->machine().device("audiocpu")->memory().space(AS_PROGRAM);
	int r=downcast<okim6295_device *>(device)->read(*space,0);

	if (cpu_get_pc(&space->device())==0x2b70 || cpu_get_pc(&space->device())==0x2bb5
		|| cpu_get_pc(&space->device())==0x2acc
		|| cpu_get_pc(&space->device())==0x1c79 // BigFight
		|| cpu_get_pc(&space->device())==0x1cbe // BigFight
		|| cpu_get_pc(&space->device())==0xf9881)
		return 0xf;
	if (cpu_get_pc(&space->device())==0x2ba3 || cpu_get_pc(&space->device())==0x2a9b || cpu_get_pc(&space->device())==0x2adc
		|| cpu_get_pc(&space->device())==0x1cac) // BigFight
		return 0;
	return r;
}
