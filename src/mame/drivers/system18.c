/*  System18 Hardware
**
**  MC68000 + Z80
**  2xYM3438 + Custom PCM
**
**  Alien Storm
**  Bloxeed
**  Clutch Hitter
**  D.D. Crew
**  Laser Ghost
**  Michael Jackson's Moonwalker
**  Shadow Dancer
**  Search Wally
*/

/*

Changes:
04/28/04  Charles MacDonald
- Added MSM5205 sample playback to shdancbl.

03/17/04
- Added Where's Wally? (wwally) It's encrypted and unplayable.
- Moved Ace Attacker to system16.c (it's a not a System 18 game)
- Fixed System 18 sample ROM banking. This doesn't help the current working games, but will support others when/if they are decrypted.
- Fixed RF5C68A clock (7.15 MHz -> 8.00 MHz).
- Fixed Z80 clock (8.192 MHz -> 8.00 MHz).
- Cleaned up shdancbl sound hardware a little.

03/12/04
- Added preliminary VDP emulation to fix shdancer tile banking and VDP memory tests in other games.
- Added I/O chip emulation, fixes blanked screen at end of shdancer memory test, coin meter/lockout implemented.
- Cleaned up shdancer,shdancrj,shdancbl drivers.
- Added Shadow Dancer (Rev.B) driver (shdancrb).
- Added sound emulation to shdancbl.

To do:
- mwalkbl VDP test says IC68 (315-5313) is bad, but the test code seems to print the 'bad' message always, and it
  doesn't look like that VDP layer is supposed to cover the text (indicating success).
- astormbl writes to mirrored sprite RAM at $141000.
- mwalkbl writes to mirrored palette RAM at $841000.

shdancbl:
- Sampled sound needs to be implemented.
- ROM test fails. Maybe the original game code was not patched to fix this, rather than the ROMs really being bad.
- Supporting the screen blanking control at $E4001D causes some problems after coin-up.
- Player 1 inputs map to player 2.
- Misplaced tilemap horizontal scrolling in several locations:
  Level map screen, bonus stage, train level, sewer level, etc.
- Bad sprites in some specific cases:
  - Green shield-throwing man, animation when dog attacks enemy, fade-in when ninjas appear, etc.
- Missing "Shadow Dancer" logo on title screen if you insert a coin instead of watching the intro sequence.

I very carefully checked the sprite ROMs to make sure they are loaded correctly, and it seems that for the bad
sprites listed above, one of the even/odd pair is correct while the other is not. The correct data isn't present
in any of the ROMs I think. Maybe they need to be redumped, or it's an actual problem of the bootleg?

Other notes:
- The sys18_sound_info structure holds the offset to each ROM in the "sound" space as well as a mask to be applied
   to the 8K bank offset into each ROM. For an unused ROM, the mask should be set to zero.

*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "system16.h"
#include "cpu/m68000/m68000.h"
#include "machine/fd1094.h"
#include "sound/msm5205.h"
#include "sound/2612intf.h"
#include "sound/rf5c68.h"

/* video/segac2.c */
extern void update_system18_vdp( bitmap_t *bitmap, const rectangle *cliprect );
extern READ16_HANDLER( segac2_vdp_r );
extern WRITE16_HANDLER( segac2_vdp_w );

/***************************************************************************/

static WRITE16_HANDLER( sys18_refreshenable_w )
{
	if(ACCESSING_BITS_0_7)
	{
		sys16_refreshenable = data & 0x02;
	}
}

static WRITE16_HANDLER( sys18_tilebank_w )
{
	if(ACCESSING_BITS_0_7)
	{
		sys16_tile_bank0 = (data >> 0) & 0x0F;
		sys16_tile_bank1 = (data >> 4) & 0x0F;
	}
}

/***************************************************************************/

static void set_fg_page( int data ){
	sys16_fg_page[0] = data>>12;
	sys16_fg_page[1] = (data>>8)&0xf;
	sys16_fg_page[2] = (data>>4)&0xf;
	sys16_fg_page[3] = data&0xf;
}

static void set_bg_page( int data ){
	sys16_bg_page[0] = data>>12;
	sys16_bg_page[1] = (data>>8)&0xf;
	sys16_bg_page[2] = (data>>4)&0xf;
	sys16_bg_page[3] = data&0xf;
}

static void set_fg2_page( int data ){
	sys16_fg2_page[0] = data>>12;
	sys16_fg2_page[1] = (data>>8)&0xf;
	sys16_fg2_page[2] = (data>>4)&0xf;
	sys16_fg2_page[3] = data&0xf;
}

static void set_bg2_page( int data ){
	sys16_bg2_page[0] = data>>12;
	sys16_bg2_page[1] = (data>>8)&0xf;
	sys16_bg2_page[2] = (data>>4)&0xf;
	sys16_bg2_page[3] = data&0xf;
}

/***************************************************************************/
/*
    Sound hardware for Shadow Dancer (Datsu bootleg)

    Z80 memory map
    0000-7FFF : ROM (fixed)
    8000-BFFF : ROM (banked)
    C000-C007 : ?
    C400      : Sound command (r/o)
    C800      : MSM5205 sample data output (w/o)
    CC00-CC03 : YM3438 #1
    D000-D003 : YM3438 #2
    D400      : ROM bank control (w/o)
    DF00-DFFF : ?
    E000-FFFF : Work RAM

    The unused memory locations and I/O port access seem to be remnants of the original code that were not patched out:

    - Program accesses RF5C68A channel registes at $C000-$C007
    - Program clears RF5C68A wave memory at $DF00-$DFFF
    - Program writes to port $A0 to access sound ROM banking control latch
    - Program reads port $C0 to access sound command

    Interrupts

    IRQ = Triggered when 68000 writes sound command. Z80 reads from $C400.
    NMI = Triggered when second nibble of sample data has been output to the MSM5205.
          Program copies sample data from ROM bank to the MSM5205 sample data buffer at $C800.

    ROM banking seems correct.
    It doesn't look like there's a way to reset the MSM5205, unless that's related to bit 7 of the
    ROM bank control register.
    MSM5205 clock speed hasn't been confirmed.
*/
/***************************************************************************/

static int sample_buffer = 0;
static int sample_select = 0;

static WRITE8_HANDLER( shdancbl_msm5205_data_w )
{
	sample_buffer = data;
}

static void shdancbl_msm5205_callback(running_machine *machine, int data)
{
	msm5205_data_w(0, sample_buffer & 0x0F);
	sample_buffer >>= 4;
	sample_select ^= 1;
	if(sample_select == 0)
		cpu_set_input_line(machine->cpu[1], INPUT_LINE_NMI, PULSE_LINE);
}

static const msm5205_interface shdancbl_msm5205_interface =
{
	shdancbl_msm5205_callback,
	MSM5205_S48_4B
};

static UINT8* shdancbl_soundbank_ptr = NULL;		/* Pointer to currently selected portion of ROM */

static WRITE16_HANDLER( sound_command_irq_w ){
	if( ACCESSING_BITS_0_7 ){
		soundlatch_w( space->machine,0,data&0xff );
		cpu_set_input_line(space->machine->cpu[1], 0, HOLD_LINE );
	}
}

static READ8_HANDLER( shdancbl_soundbank_r )
{
	if(shdancbl_soundbank_ptr) return shdancbl_soundbank_ptr[offset & 0x3FFF];
	return 0xFF;
}

static WRITE8_HANDLER( shdancbl_bankctrl_w )
{
	UINT8 *mem = memory_region(space->machine, "sound");

	switch(data)
	{
		case 0:
			shdancbl_soundbank_ptr = &mem[0x18000]; /* IC45 8000-BFFF */
			break;
		case 1:
			shdancbl_soundbank_ptr = &mem[0x1C000]; /* IC45 C000-FFFF */
			break;
		case 2:
			shdancbl_soundbank_ptr = &mem[0x20000]; /* IC46 0000-3FFF */
			break;
		case 3:
			shdancbl_soundbank_ptr = &mem[0x24000]; /* IC46 4000-7FFF */
			break;
		default:
			shdancbl_soundbank_ptr = NULL;
			logerror("Invalid bank setting %02X (%04X)\n", data, cpu_get_pc(space->cpu));
			break;
	}
}

static ADDRESS_MAP_START( shdancbl_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(shdancbl_soundbank_r)
	AM_RANGE(0xc400, 0xc400) AM_READ(soundlatch_r)
	AM_RANGE(0xcc00, 0xcc00) AM_READ(ym3438_status_port_0_a_r)
	AM_RANGE(0xcc01, 0xcc01) AM_READ(ym3438_status_port_0_b_r)
	AM_RANGE(0xcc02, 0xcc02) AM_READ(ym3438_status_port_0_b_r)
	AM_RANGE(0xcc03, 0xcc03) AM_READ(ym3438_status_port_0_b_r)
	AM_RANGE(0xd000, 0xd000) AM_READ(ym3438_status_port_1_a_r)
	AM_RANGE(0xd001, 0xd001) AM_READ(ym3438_status_port_1_b_r)
	AM_RANGE(0xd002, 0xd002) AM_READ(ym3438_status_port_1_b_r)
	AM_RANGE(0xd003, 0xd003) AM_READ(ym3438_status_port_1_b_r)
	AM_RANGE(0xdf00, 0xdfff) AM_READ(SMH_NOP)
	AM_RANGE(0xe000, 0xffff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START(shdancbl_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(SMH_NOP) /* ROM bank */
	AM_RANGE(0xc000, 0xc00f) AM_WRITE(SMH_NOP)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(shdancbl_msm5205_data_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(ym3438_control_port_0_a_w)
	AM_RANGE(0xcc01, 0xcc01) AM_WRITE(ym3438_data_port_0_a_w)
	AM_RANGE(0xcc02, 0xcc02) AM_WRITE(ym3438_control_port_0_b_w)
	AM_RANGE(0xcc03, 0xcc03) AM_WRITE(ym3438_data_port_0_b_w)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(ym3438_control_port_1_a_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITE(ym3438_data_port_1_a_w)
	AM_RANGE(0xd002, 0xd002) AM_WRITE(ym3438_control_port_1_b_w)
	AM_RANGE(0xd003, 0xd003) AM_WRITE(ym3438_data_port_1_b_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(shdancbl_bankctrl_w)
	AM_RANGE(0xdf00, 0xdfff) AM_WRITE(SMH_NOP)
	AM_RANGE(0xe000, 0xffff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( shdancbl_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xa0, 0xbf) AM_WRITE(SMH_NOP)
	AM_RANGE(0xc0, 0xdf) AM_READ(SMH_NOP)
ADDRESS_MAP_END

/***************************************************************************/

static UINT8 *sys18_SoundMemBank;

static READ8_HANDLER( system18_bank_r )
{
	if(sys18_SoundMemBank)
		return sys18_SoundMemBank[offset];
	return 0xFF;
}

static ADDRESS_MAP_START( sound_readmem_18, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_READ(SMH_ROM)
	AM_RANGE(0xa000, 0xbfff) AM_READ(system18_bank_r)
	/**** D/A register ****/
	AM_RANGE(0xd000, 0xdfff) AM_READ(rf5c68_r)
	AM_RANGE(0xe000, 0xffff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem_18, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(SMH_ROM)
	/**** D/A register ****/
	AM_RANGE(0xc000, 0xc008) AM_WRITE(rf5c68_reg_w)
	AM_RANGE(0xd000, 0xdfff) AM_WRITE(rf5c68_w)
	AM_RANGE(0xe000, 0xffff) AM_WRITE(SMH_RAM)	//??
ADDRESS_MAP_END


static WRITE8_HANDLER( sys18_soundbank_w )
{
	UINT8 *mem = memory_region(space->machine, "sound");
	int rom = (data >> 6) & 3;
	int bank = (data & 0x3f);
	int mask = sys18_sound_info[rom*2+0];
	int offs = sys18_sound_info[rom*2+1];
	if(mask) sys18_SoundMemBank = &mem[0x10000 + offs + ((bank & mask) << 13)];
	else sys18_SoundMemBank = NULL;
}

static ADDRESS_MAP_START( sound_18_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_READWRITE(ym3438_status_port_0_a_r, ym3438_control_port_0_a_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(ym3438_data_port_0_a_w)
//  AM_RANGE(0x82, 0x82) AM_READ(ym3438_status_port_0_b_r)
	AM_RANGE(0x82, 0x82) AM_WRITE(ym3438_control_port_0_b_w)
	AM_RANGE(0x83, 0x83) AM_WRITE(ym3438_data_port_0_b_w)
//  AM_RANGE(0x90, 0x90) AM_READ(ym3438_status_port_1_a_r)
	AM_RANGE(0x90, 0x90) AM_WRITE(ym3438_control_port_1_a_w)
	AM_RANGE(0x91, 0x91) AM_WRITE(ym3438_data_port_1_a_w)
//  AM_RANGE(0x92, 0x92) AM_READ(ym3438_status_port_1_b_r)
	AM_RANGE(0x92, 0x92) AM_WRITE(ym3438_control_port_1_b_w)
	AM_RANGE(0x93, 0x93) AM_WRITE(ym3438_data_port_1_b_w)
	AM_RANGE(0xa0, 0xa0) AM_WRITE(sys18_soundbank_w)
	AM_RANGE(0xc0, 0xc0) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static WRITE16_HANDLER( sound_command_nmi_w ){
	if( ACCESSING_BITS_0_7 ){
		soundlatch_w( space->machine,0,data&0xff );
		cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_NMI, PULSE_LINE);
	}
}

/***************************************************************************/

/*
    315-5313 Video Display Processor (VDP) emulation
*/

static struct {
	UINT16 vram[0x8000], read_buffer;
	UINT16 cram[0x40];
	UINT16 vsram[0x40];
	UINT8 reg[0x20];
	int pending, code, addr, addr_latch;
} vdp;

static WRITE16_HANDLER( vdp_w )
{
	switch(offset)
	{
		case 0: /* Data port */
		case 1: /* Data port */
			vdp.pending = 0;
			switch(vdp.code & 0x0F)
			{
				case 1: /* VRAM */
					vdp.vram[(vdp.addr >> 1) & 0x7FFF] = data;
					break;
				case 3: /* CRAM */
					vdp.cram[(vdp.addr >> 1) & 0x3F] = data;
					break;
				case 5: /* VSRAM */
					vdp.vsram[(vdp.addr >> 1) & 0x3F] = data;
					break;
			}
			vdp.addr = (vdp.addr + vdp.reg[0x0F]) & 0xFFFF;
			break;

		case 2: /* Control port */
		case 3: /* Control port */
			if(vdp.pending == 0)
			{
				if((data & 0xC000) == 0x8000)
				{
					int d = (data >> 0) & 0xFF;
					int r = (data >> 8) & 0x1F;
					vdp.reg[r] = d;
				}
				else
				{
					vdp.pending = 1;
					vdp.code = (vdp.code & 0x3C) | ((data >> 14) & 3);
					vdp.addr = (vdp.addr_latch & 0xC000) | (data & 0x3FFF);
				}
			}
			else
			{
				vdp.pending = 0;
				vdp.code = (vdp.code & 0x03) | ((data >> 2) & 0x3C);
				vdp.addr = (vdp.addr & 0x3FFF) | ((data & 3) << 14);
				vdp.addr_latch = vdp.addr & 0xC000;

				// Read-ahead for VRAM
				if(vdp.code == 0)
				{
					vdp.read_buffer = vdp.vram[(vdp.addr >> 1) & 0x7FFF];
					vdp.addr = (vdp.addr + vdp.reg[0x0F]) & 0xFFFF;
				}

				// Check for DMA request
				if((vdp.reg[1] & 0x20) && (vdp.code & 0x20))
				{
					logerror("vdp: DMA disabled in this system.\n");
				}
			}
			break;

		default: /* Unused */
			logerror("vdp: write %04X to %08X\n", data, offset);
			break;
	}
}

static READ16_HANDLER( vdp_r )
{
	UINT16 temp = -1;

	vdp.pending = 0;

	switch(offset)
	{
		case 0: /* Data port */
		case 1: /* Data port */
			switch(vdp.code & 0x0F)
			{
				case 0: /* VRAM */
					temp = vdp.read_buffer;
					vdp.read_buffer = vdp.vram[(vdp.addr >> 1) & 0x7FFF];
					break;

				case 4: /* VSRAM */
					temp = vdp.vsram[(vdp.addr >> 1) & 0x3F];
					break;

				case 8: /* CRAM */
					temp = vdp.cram[(vdp.addr >> 1) & 0x3F];
					break;
			}
			vdp.addr = (vdp.addr + vdp.reg[0x0F]) & 0xFFFF;
			break;

		case 2: /* Control port */
		case 3: /* Control port */

			/* VRAM write FIFO is always empty */
			temp = 0x0200;

			/* Having screen turned off forces V-Blank flag to be set */
			if((vdp.reg[1] & 0x40) == 0)
				temp |= 0x80;

			break;
	}

	return temp;
}

/***************************************************************************/

/*
    315-5296 I/O chip emulation
*/

static int io_reg[0x10];

static READ16_HANDLER( sys18_io_r )
{
	if(ACCESSING_BITS_0_7)
	{
		switch(offset & 0x3000/2)
		{
			case 0x0000/2: /* I/O chip internal locations */
			case 0x1000/2: /* I/O chip internal locations (mirror) */
				switch(offset & 0x1F)
				{
					case 0x00: /* Port A - 1P controls */
						if(io_reg[0x0F] & 0x01)
							return io_reg[0x00];
						else
							return input_port_read(space->machine, "P1");
						break;

					case 0x01: /* Port B - 2P controls */
						if(io_reg[0x0F] & 0x02)
							return io_reg[0x01];
						else
							return input_port_read(space->machine, "P2");
						break;

					case 0x02: /* Port C - Bidirectional I/O port */
						if(io_reg[0x0F] & 0x04)
							return io_reg[0x02];
						else
							return -1;
						break;

					case 0x03: /* Port D - Miscellaneous outputs */
						if(io_reg[0x0F] & 0x08)
							return io_reg[0x03];
						else
							return -1;
						break;

					case 0x04: /* Port E - Service / Coin inputs */
						if(io_reg[0x0F] & 0x10)
							return io_reg[0x04];
						else
							return input_port_read(space->machine, "SERVICE");
						break;

					case 0x05: /* Port F - DIP switch #1 */
						if(io_reg[0x0F] & 0x20)
							return io_reg[0x05];
						else
							return input_port_read(space->machine, "DSW1");
						break;

					case 0x06: /* Port G - DIP switch #2 */
						if(io_reg[0x0F] & 0x40)
							return io_reg[0x06];
						else
							return input_port_read(space->machine, "P3");
						break;

					case 0x07: /* Port H - Tile banking control */
						if(io_reg[0x0F] & 0x80)
							return io_reg[0x07];
						else
							return -1;
						break;

					case 0x08: /* Protection #1 */
						return 'S';
					case 0x09: /* Protection #2 */
						return 'E';
					case 0x0A: /* Protection #3 */
						return 'G';
					case 0x0B: /* Protection #4 */
						return 'A';

					case 0x0C: /* CNT2-0 pin output control (mirror) */
					case 0x0E: /* CNT2-0 pin output control */
						return io_reg[0x0E];

					case 0x0D: /* Port direction control (mirror) */
					case 0x0F: /* Port direction control */
						return io_reg[0x0F];
				}
				return -1;

			case 0x2000/2: /* Unused */
				logerror("read video control latch %06X (%06X)\n", offset, cpu_get_pc(space->cpu));
				return -1;

			case 0x3000/2: /* Expansion connector */
				logerror("read expansion area %06X (%06X)\n", offset, cpu_get_pc(space->cpu));
				return -1;
		}
	}

	return -1;
}

static WRITE16_HANDLER( sys18_io_w )
{
	if(ACCESSING_BITS_0_7)
	{
		switch(offset & 0x3000/2)
		{
			case 0x0000/2: /* I/O chip internal locations */
			case 0x1000/2: /* I/O chip internal locations (mirror) */
				switch(offset & 0x1F)
				{
					case 0x00: /* Port A - 1P controls */
						io_reg[0x00] = data;
						break;

					case 0x01: /* Port B - 2P controls */
						io_reg[0x01] = data;
						break;

					case 0x02: /* Port C - Bidirectional I/O port */
						io_reg[0x02] = data;
						break;

					case 0x03: /* Port D - Miscellaneous outputs */
						io_reg[0x03] = data;
						coin_lockout_w(1, data & 8);
						coin_lockout_w(0, data & 4);
						coin_counter_w(1, data & 2);
						coin_counter_w(0, data & 1);
						break;

					case 0x04: /* Port E - Service / Coin inputs */
						io_reg[0x04] = data;
						break;

					case 0x05: /* Port F - DIP switch #1 */
						io_reg[0x05] = data;
						break;

					case 0x06: /* Port G - DIP switch #2 */
						io_reg[0x06] = data;
						break;

					case 0x07: /* Port H - Tile banking control */
						io_reg[0x07] = data;
						sys16_tile_bank0 = (data >> 0) & 0x0F;
						sys16_tile_bank1 = (data >> 4) & 0x0F;
						break;

					case 0x0E: /* CNT2-0 pin output control */
						io_reg[0x0E] = data;
						sys16_refreshenable = data & 0x02;
						break;

					case 0x0F: /* Port direction control */
						io_reg[0x0F] = data;
						break;
				}
				break;

			case 0x2000/2: /* Video control latch */
				logerror("write video control latch %06X = %04X (%06X)\n", offset, data, cpu_get_pc(space->cpu));
				break;

			case 0x3000/2: /* Expansion connector */
//              logerror("write expansion area %06X = %04X (%06X)\n", offset, data, cpu_get_pc(space->cpu));
				break;
		}
	}
}


/***************************************************************************/
/*
    Shadow Dancer (Bootleg)

    This seems to be a modified version of shdancer. It has no warning screen, displays English text during the
    attract sequence, and has a 2P input test. The 'Sega' copyright text was changed to 'Datsu', and their
    logo is missing.

    Access to the configuration registers, I/O chip, and VDP are done even though it's likely none of this hardware
    exists in the bootleg. For example:

    - Most I/O port access has been redirected to new addresses.
    - Z80 sound command has been redirected to a new address.
    - The tilebank routine which saves the bank value in VDP VRAM has a form of protection has been modified to store
      the tilebank value directly to $E4001F.
    - Implementing screen blanking control via $E4001D leaves the screen blanked at the wrong times (after coin-up).

    This is probably due to unmodified parts of the original code accessing these components, which would be ignored
    on the bootleg hardware. Both the I/O chip and VDP are supported in this driver, just as I don't know for certain
    how much of either are present on the real board.

    Bootleg specific addresses:

    C40001 = DIP switch #1
    C40003 = DIP switch #2
    C40007 = Z80 sound command
    C41001 = Service input
    C41003 = Player 1 input
    C41005 = Player 2 input
    C44000 = Has 'clr.w' done after setting tile bank in $E4000F.
    C460xx = Extra video hardware controls

    Here are the I/O chip addresses accessed:

    E40001 = Player 1
    E40007 = Miscellaneous outputs (coin control, etc.)
    E4000F = Tile bank
    E4001D = CNT2-0 pin output state
    E4001F = I/O chip port direction
*/
/***************************************************************************/

static ADDRESS_MAP_START( shdancbl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(SMH_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x440000, 0x4407ff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc00000, 0xc0ffff) AM_READ(vdp_r)
	AM_RANGE(0xe40000, 0xe4ffff) AM_READ(sys18_io_r)
	AM_RANGE(0xc40000, 0xc40001) AM_READ_PORT("COINAGE")
	AM_RANGE(0xc40002, 0xc40003) AM_READ_PORT("DSW1")
	AM_RANGE(0xc41002, 0xc41003) AM_READ_PORT("P1")
	AM_RANGE(0xc41004, 0xc41005) AM_READ_PORT("P2")
	AM_RANGE(0xc41000, 0xc41001) AM_READ_PORT("SERVICE")
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( shdancbl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc00000, 0xc0ffff) AM_WRITE(vdp_w)
	AM_RANGE(0xe4001c, 0xe4001d) AM_WRITE(SMH_NOP) // to prevent access to screen blanking control below
	AM_RANGE(0xe40000, 0xe4ffff) AM_WRITE(sys18_io_w)
	AM_RANGE(0xc40006, 0xc40007) AM_WRITE(sound_command_irq_w)
	AM_RANGE(0xc44000, 0xc44001) AM_WRITE(SMH_NOP) // only used via clr.w after tilebank set
	AM_RANGE(0xc46000, 0xc46fff) AM_WRITE(SMH_NOP) // bootleg specific video hardware
	AM_RANGE(0xfe0020, 0xfe003f) AM_WRITE(SMH_NOP) // config regs
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

/***************************************************************************/

static void shdancbl_update_proc( void )
{
	sys16_fg_scrollx = sys16_textram[0x0e98/2];
	sys16_bg_scrollx = sys16_textram[0x0e9a/2];
	sys16_fg_scrolly = sys16_textram[0x0e90/2];
	sys16_bg_scrolly = sys16_textram[0x0e92/2];

	set_fg_page( sys16_textram[0x0e80/2] );
	set_bg_page( sys16_textram[0x0e82/2] );

	sys16_fg2_scrollx = sys16_textram[0x0e9c/2];
	sys16_bg2_scrollx = sys16_textram[0x0e9e/2];
	sys16_fg2_scrolly = sys16_textram[0x0e94/2];
	sys16_bg2_scrolly = sys16_textram[0x0e96/2];

	set_fg2_page( sys16_textram[0x0e84/2] );
	set_bg2_page( sys16_textram[0x0e82/2] );

	sys18_bg2_active=0;
	sys18_fg2_active=0;

	if(sys16_fg2_scrollx | sys16_fg2_scrolly | sys16_textram[0x0e84/2])
		sys18_fg2_active=1;
	if(sys16_bg2_scrollx | sys16_bg2_scrolly | sys16_textram[0x0e86/2])
		sys18_bg2_active=1;
}

static MACHINE_RESET( shdancbl )
{
	sys16_sprxoffset = -0xbc+0x77;
	sys16_update_proc = shdancbl_update_proc;
}

static READ16_HANDLER( shdancbl_skip_r ){
	if (cpu_get_pc(space->cpu)==0x2f76) {cpu_spinuntil_int(space->cpu); return 0xffff;}
	return sys16_workingram[0];
}

static DRIVER_INIT( shdancbl )
{
	int i;
	UINT8 *mem;

	/* Invert tile ROM data*/
	mem = memory_region(machine, "gfx1");
	for(i = 0; i < 0xc0000; i++)
		mem[i] ^= 0xFF;

	MACHINE_RESET_CALL(sys16_onetime);
	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xffc000, 0xffc001, 0, 0, shdancbl_skip_r );

	sys18_splittab_fg_x=&sys16_textram[0x0f80/2];
	sys18_splittab_bg_x=&sys16_textram[0x0fc0/2];
	sys16_MaxShadowColors=0;

	/* Copy first 32K of IC45 to Z80 address space */
	mem = memory_region(machine, "sound");
	memcpy(mem, mem+0x10000, 0x8000);
}

/***************************************************************************/
/*
    Moonwalker (Bootleg)
*/
/***************************************************************************/

static READ16_HANDLER( mwalkbl_skip_r ){
	if (cpu_get_pc(space->cpu)==0x308a) {cpu_spinuntil_int(space->cpu); return 0xffff;}
	return sys16_workingram[0x202c/2];
}


static ADDRESS_MAP_START( mwalkbl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(SMH_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x440000, 0x440fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc00000, 0xc0ffff) AM_READ(vdp_r)
	AM_RANGE(0xc40000, 0xc40001) AM_READ_PORT("COINAGE")
	AM_RANGE(0xc40002, 0xc40003) AM_READ_PORT("DSW1")
	AM_RANGE(0xc41002, 0xc41003) AM_READ_PORT("P1")
	AM_RANGE(0xc41004, 0xc41005) AM_READ_PORT("P2")
	AM_RANGE(0xc41006, 0xc41007) AM_READ_PORT("P3")
	AM_RANGE(0xc41008, 0xc41009) AM_READ(SMH_NOP) // figure this out, extra input for 3p?
	AM_RANGE(0xc41000, 0xc41001) AM_READ_PORT("SERVICE")
	AM_RANGE(0xe40000, 0xe4ffff) AM_READ(SYS16_MRA16_EXTRAM2)
	AM_RANGE(0xffe02c, 0xffe02d) AM_READ(mwalkbl_skip_r)
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mwalkbl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc00000, 0xc0ffff) AM_WRITE(vdp_w)
	AM_RANGE(0xc40006, 0xc40007) AM_WRITE(sound_command_nmi_w)
	AM_RANGE(0xc46600, 0xc46601) AM_WRITE(sys18_refreshenable_w)
	AM_RANGE(0xc46800, 0xc46801) AM_WRITE(sys18_tilebank_w)
	AM_RANGE(0xe40000, 0xe4ffff) AM_WRITE(SYS16_MWA16_EXTRAM2) AM_BASE(&sys16_extraram2)
	AM_RANGE(0xfe0020, 0xfe003f) AM_WRITE(SMH_NOP) // config regs
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

/***************************************************************************/

static void mwalkbl_update_proc( void ){
	sys16_fg_scrollx = sys16_textram[0x0e98/2];
	sys16_bg_scrollx = sys16_textram[0x0e9a/2];
	sys16_fg_scrolly = sys16_textram[0x0e90/2];
	sys16_bg_scrolly = sys16_textram[0x0e92/2];

	set_fg_page( sys16_textram[0x0e80/2] );
	set_bg_page( sys16_textram[0x0e82/2] );

	sys16_fg2_scrollx = sys16_textram[0x0e9c/2];
	sys16_bg2_scrollx = sys16_textram[0x0e9e/2];
	sys16_fg2_scrolly = sys16_textram[0x0e94/2];
	sys16_bg2_scrolly = sys16_textram[0x0e96/2];

	set_fg2_page( sys16_textram[0x0e84/2] );
	set_bg2_page( sys16_textram[0x0e86/2] );

	if(sys16_fg2_scrollx | sys16_fg2_scrolly | sys16_textram[0x0e84/2])
		sys18_fg2_active=1;
	else
		sys18_fg2_active=0;
	if(sys16_bg2_scrollx | sys16_bg2_scrolly | sys16_textram[0x0e86/2])
		sys18_bg2_active=1;
	else
		sys18_bg2_active=0;
}

static MACHINE_RESET( mwalkbl ){
	sys16_bg_priority_value=0x1000;
	sys16_sprxoffset = -0x238;

	sys16_patch_code( 0x70116, 0x4e);
	sys16_patch_code( 0x70117, 0x71);

	sys16_patch_code( 0x314a, 0x46);
	sys16_patch_code( 0x314b, 0x42);

	sys16_patch_code( 0x311b, 0x3f);

	sys16_patch_code( 0x70103, 0x00);
	sys16_patch_code( 0x70109, 0x00);
	sys16_patch_code( 0x07727, 0x00);
	sys16_patch_code( 0x07729, 0x00);
	sys16_patch_code( 0x0780d, 0x00);
	sys16_patch_code( 0x0780f, 0x00);
	sys16_patch_code( 0x07861, 0x00);
	sys16_patch_code( 0x07863, 0x00);
	sys16_patch_code( 0x07d47, 0x00);
	sys16_patch_code( 0x07863, 0x00);
	sys16_patch_code( 0x08533, 0x00);
	sys16_patch_code( 0x08535, 0x00);
	sys16_patch_code( 0x085bd, 0x00);
	sys16_patch_code( 0x085bf, 0x00);
	sys16_patch_code( 0x09a4b, 0x00);
	sys16_patch_code( 0x09a4d, 0x00);
	sys16_patch_code( 0x09b2f, 0x00);
	sys16_patch_code( 0x09b31, 0x00);
	sys16_patch_code( 0x0a05b, 0x00);
	sys16_patch_code( 0x0a05d, 0x00);
	sys16_patch_code( 0x0a23f, 0x00);
	sys16_patch_code( 0x0a241, 0x00);
	sys16_patch_code( 0x10159, 0x00);
	sys16_patch_code( 0x1015b, 0x00);
	sys16_patch_code( 0x109fb, 0x00);
	sys16_patch_code( 0x109fd, 0x00);

	// * SEGA mark
	sys16_patch_code( 0x70212, 0x4e);
	sys16_patch_code( 0x70213, 0x71);

	sys16_update_proc = mwalkbl_update_proc;
}

static DRIVER_INIT( mwalkbl ){
	UINT8 *RAM= memory_region(machine, "sound");
	static const int mwalk_sound_info[] =
	{
		0x0f, 0x00000, // ROM #1 = 128K
		0x1f, 0x20000, // ROM #2 = 256K
		0x1f, 0x60000, // ROM #3 = 256K
		0x1f, 0xA0000  // ROM #4 = 256K
	};

	MACHINE_RESET_CALL(sys16_onetime);
	sys18_splittab_fg_x=&sys16_textram[0x0f80/2];
	sys18_splittab_bg_x=&sys16_textram[0x0fc0/2];

	memcpy(sys18_sound_info, mwalk_sound_info, sizeof(sys18_sound_info));
	memcpy(RAM,&RAM[0x10000],0xa000);
}

/***************************************************************************/

/* bootleg doesn't have real vdp or i/o */

static ADDRESS_MAP_START( astormbl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(SMH_ROM)
	AM_RANGE(0x100000, 0x10ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x110000, 0x110fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x140000, 0x140fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0x200000, 0x200fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0xa00000, 0xa00001) AM_READ_PORT("COINAGE")
	AM_RANGE(0xa00002, 0xa00003) AM_READ_PORT("DSW1")
	AM_RANGE(0xa01002, 0xa01003) AM_READ_PORT("P1")
	AM_RANGE(0xa01004, 0xa01005) AM_READ_PORT("P2")
	AM_RANGE(0xa01006, 0xa01007) AM_READ_PORT("P3")
	AM_RANGE(0xa01000, 0xa01001) AM_READ_PORT("SERVICE")
	AM_RANGE(0xc00000, 0xc0ffff) AM_READ(vdp_r)
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( astormbl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x100000, 0x10ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x110000, 0x110fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x140000, 0x140fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0x200000, 0x200fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0xa00006, 0xa00007) AM_WRITE(sound_command_nmi_w)
	AM_RANGE(0xa0000e, 0xa0000f) AM_WRITE(sys18_tilebank_w)
	AM_RANGE(0xc00000, 0xc0ffff) AM_WRITE(vdp_w)
	AM_RANGE(0xc46600, 0xc46601) AM_WRITE(sys18_refreshenable_w)
	AM_RANGE(0xfe0020, 0xfe003f) AM_WRITE(SMH_NOP)
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END


/***************************************************************************/

static void astormbl_update_proc( void ){
	UINT16 data;
	sys16_fg_scrollx = sys16_textram[0x0e98/2];
	sys16_bg_scrollx = sys16_textram[0x0e9a/2];
	sys16_fg_scrolly = sys16_textram[0x0e90/2];
	sys16_bg_scrolly = sys16_textram[0x0e92/2];

	data = sys16_textram[0x0e80/2];
	sys16_fg_page[1] = data>>12;
	sys16_fg_page[3] = (data>>8)&0xf;
	sys16_fg_page[0] = (data>>4)&0xf;
	sys16_fg_page[2] = data&0xf;

	data = sys16_textram[0x0e82/2];
	sys16_bg_page[1] = data>>12;
	sys16_bg_page[3] = (data>>8)&0xf;
	sys16_bg_page[0] = (data>>4)&0xf;
	sys16_bg_page[2] = data&0xf;

	sys16_fg2_scrollx = sys16_textram[0x0e9c/2];
	sys16_bg2_scrollx = sys16_textram[0x0e9e/2];
	sys16_fg2_scrolly = sys16_textram[0x0e94/2];
	sys16_bg2_scrolly = sys16_textram[0x0e96/2];

	data = sys16_textram[0x0e84/2];
	sys16_fg2_page[1] = data>>12;
	sys16_fg2_page[3] = (data>>8)&0xf;
	sys16_fg2_page[0] = (data>>4)&0xf;
	sys16_fg2_page[2] = data&0xf;

	data = sys16_textram[0x0e86/2];
	sys16_bg2_page[1] = data>>12;
	sys16_bg2_page[3] = (data>>8)&0xf;
	sys16_bg2_page[0] = (data>>4)&0xf;
	sys16_bg2_page[2] = data&0xf;

// enable regs
	if(sys16_fg2_scrollx | sys16_fg2_scrolly | sys16_textram[0x0e84/2])
		sys18_fg2_active=1;
	else
		sys18_fg2_active=0;
	if(sys16_bg2_scrollx | sys16_bg2_scrolly | sys16_textram[0x0e86/2])
		sys18_bg2_active=1;
	else
		sys18_bg2_active=0;
}

static MACHINE_RESET( astormbl ){
	sys16_fgxoffset = sys16_bgxoffset = -9;

	sys16_patch_code( 0x2D6E, 0x32 );
	sys16_patch_code( 0x2D6F, 0x3c );
	sys16_patch_code( 0x2D70, 0x80 );
	sys16_patch_code( 0x2D71, 0x00 );
	sys16_patch_code( 0x2D72, 0x33 );
	sys16_patch_code( 0x2D73, 0xc1 );
	sys16_patch_code( 0x2ea2, 0x30 );
	sys16_patch_code( 0x2ea3, 0x38 );
	sys16_patch_code( 0x2ea4, 0xec );
	sys16_patch_code( 0x2ea5, 0xf6 );
	sys16_patch_code( 0x2ea6, 0x30 );
	sys16_patch_code( 0x2ea7, 0x80 );
	sys16_patch_code( 0x2e5c, 0x30 );
	sys16_patch_code( 0x2e5d, 0x38 );
	sys16_patch_code( 0x2e5e, 0xec );
	sys16_patch_code( 0x2e5f, 0xe2 );
	sys16_patch_code( 0x2e60, 0xc0 );
	sys16_patch_code( 0x2e61, 0x7c );

	sys16_patch_code( 0x4cd8, 0x02 );
	sys16_patch_code( 0x4cec, 0x03 );
	sys16_patch_code( 0x2dc6c, 0xe9 );
	sys16_patch_code( 0x2dc64, 0x10 );
	sys16_patch_code( 0x2dc65, 0x10 );
	sys16_patch_code( 0x3a100, 0x10 );
	sys16_patch_code( 0x3a101, 0x13 );
	sys16_patch_code( 0x3a102, 0x90 );
	sys16_patch_code( 0x3a103, 0x2b );
	sys16_patch_code( 0x3a104, 0x00 );
	sys16_patch_code( 0x3a105, 0x01 );
	sys16_patch_code( 0x3a106, 0x0c );
	sys16_patch_code( 0x3a107, 0x00 );
	sys16_patch_code( 0x3a108, 0x00 );
	sys16_patch_code( 0x3a109, 0x01 );
	sys16_patch_code( 0x3a10a, 0x66 );
	sys16_patch_code( 0x3a10b, 0x06 );
	sys16_patch_code( 0x3a10c, 0x42 );
	sys16_patch_code( 0x3a10d, 0x40 );
	sys16_patch_code( 0x3a10e, 0x54 );
	sys16_patch_code( 0x3a10f, 0x8b );
	sys16_patch_code( 0x3a110, 0x60 );
	sys16_patch_code( 0x3a111, 0x02 );
	sys16_patch_code( 0x3a112, 0x30 );
	sys16_patch_code( 0x3a113, 0x1b );
	sys16_patch_code( 0x3a114, 0x34 );
	sys16_patch_code( 0x3a115, 0xc0 );
	sys16_patch_code( 0x3a116, 0x34 );
	sys16_patch_code( 0x3a117, 0xdb );
	sys16_patch_code( 0x3a118, 0x24 );
	sys16_patch_code( 0x3a119, 0xdb );
	sys16_patch_code( 0x3a11a, 0x24 );
	sys16_patch_code( 0x3a11b, 0xdb );
	sys16_patch_code( 0x3a11c, 0x4e );
	sys16_patch_code( 0x3a11d, 0x75 );
	sys16_patch_code( 0xaf8e, 0x66 );

	/* fix missing credit text */
	sys16_patch_code( 0x3f9a, 0xec );
	sys16_patch_code( 0x3f9b, 0x36 );

	sys16_update_proc = astormbl_update_proc;
}


static DRIVER_INIT( astormbl ){
	UINT8 *RAM= memory_region(machine, "sound");
	static const int astormbl_sound_info[] =
	{
		0x0f, 0x00000, // ROM #1 = 128K
		0x1f, 0x20000, // ROM #2 = 256K
		0x1f, 0x60000, // ROM #3 = 256K
		0x1f, 0xA0000  // ROM #4 = 256K
	};

	MACHINE_RESET_CALL(sys16_onetime);
	sys18_splittab_fg_x=&sys16_textram[0x0f80/2];
	sys18_splittab_bg_x=&sys16_textram[0x0fc0/2];
	sys16_MaxShadowColors = 0; // doesn't seem to use transparent shadows

	memcpy(sys18_sound_info, astormbl_sound_info, sizeof(sys18_sound_info));
	memcpy(RAM,&RAM[0x10000],0xa000);
}


/*****************************************************************************/

static MACHINE_DRIVER_START( system18 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", M68000, 10000000)
	MDRV_CPU_VBLANK_INT("main", irq4_line_hold)

	MDRV_CPU_ADD("sound", Z80, 8000000)
	MDRV_CPU_PROGRAM_MAP(sound_readmem_18,sound_writemem_18)
	MDRV_CPU_IO_MAP(sound_18_io_map,0)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(sys16)
	MDRV_PALETTE_LENGTH((2048+2048)*ShadowColorsMultiplier) // 64 extra colours for vdp (but we use 2048 so shadow mask works)

	MDRV_VIDEO_START(system18old)
	MDRV_VIDEO_UPDATE(system18old)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("3438.0", YM3438, 8000000)
	MDRV_SOUND_ROUTE(0, "left", 0.40)
	MDRV_SOUND_ROUTE(1, "right", 0.40)
	MDRV_SOUND_ROUTE(2, "left", 0.40)
	MDRV_SOUND_ROUTE(3, "right", 0.40)

	MDRV_SOUND_ADD("3438.1", YM3438, 8000000)
	MDRV_SOUND_ROUTE(0, "left", 0.40)
	MDRV_SOUND_ROUTE(1, "right", 0.40)
	MDRV_SOUND_ROUTE(2, "left", 0.40)
	MDRV_SOUND_ROUTE(3, "right", 0.40)

	MDRV_SOUND_ADD("5c68", RF5C68, 8000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( astormbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system18)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(astormbl_readmem,astormbl_writemem)

	MDRV_MACHINE_RESET(astormbl)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mwalkbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system18)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mwalkbl_readmem,mwalkbl_writemem)

	MDRV_MACHINE_RESET(mwalkbl)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( shdancbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system18)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(shdancbl_readmem,shdancbl_writemem)

	MDRV_CPU_MODIFY("sound")
	MDRV_CPU_PROGRAM_MAP(shdancbl_sound_readmem,shdancbl_sound_writemem)
	MDRV_CPU_IO_MAP(shdancbl_sound_io_map,0)
	MDRV_SOUND_REMOVE("5c68")

	MDRV_SOUND_ADD("5205", MSM5205, 200000)
	MDRV_SOUND_CONFIG(shdancbl_msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.80)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.80)

	MDRV_MACHINE_RESET(shdancbl)
MACHINE_DRIVER_END


/***************************************************************************/

static INPUT_PORTS_START( astormbl )
	PORT_START("P1")	/* player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("P2")	/* player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("COINAGE")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START("DSW1")	/* DSW1 */
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Special" )
	PORT_DIPNAME( 0x20, 0x20, "Coin Chutes" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P3")	/* player 3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
INPUT_PORTS_END

static INPUT_PORTS_START( mwalkbl )
	PORT_INCLUDE( astormbl )

	PORT_MODIFY("SERVICE")	/* service */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1")	/* DSW1 */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x08, 0x08, "Player Vitality" )
	PORT_DIPSETTING(    0x08, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, "Play Mode" )
	PORT_DIPSETTING(    0x10, "2 Players" )
	PORT_DIPSETTING(    0x00, "3 Players" )
	PORT_DIPNAME( 0x20, 0x20, "Coin Mode" )
	PORT_DIPSETTING(    0x20, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_MODIFY("P3")	/* player 3 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

/*****************************************************************************/

ROM_START( astormbl )
	ROM_REGION( 0x080000, "main", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "astorm.a6", 0x000000, 0x40000, CRC(7682ed3e) SHA1(b857352ad9c66488e91f60989472638c483e4ae8) )
	ROM_LOAD16_BYTE( "astorm.a5", 0x000001, 0x40000, CRC(efe9711e) SHA1(496fd9e30941fde1658fab7292a669ef7964cecb) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "epr13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "mpr13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "astorm.a11",   0x000000, 0x40000, CRC(7829c4f3) SHA1(3adb7aa7f70163d3848c98316e18b9783c41d663) )
	ROM_LOAD16_BYTE( "mpr13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	ROM_REGION( 0x100000, "sound", 0 ) /* sound CPU */
	ROM_LOAD( "epr13083.bin", 0x10000, 0x20000, CRC(5df3af20) SHA1(e49105fcfd5bf37d14bd760f6adca5ce2412883d) )
	ROM_LOAD( "epr13076.bin", 0x30000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr13077.bin", 0x70000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr13078.bin", 0xb0000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END

/*

CPUs:
on main board:

1x MC68000P10 (main)
1x Z8400BB1-Z80BCPU (sound)
1x OKI M6295 (sound)
1x oscillator 24.000MHz (close to main)
1x oscillator 8.000MHz (close to sound)

ROMs
on main board:

9x M27C512 (1,2,3,4,5,6,7,8,10)
1x TMS27C256 (9)
21x NM27C010Q (11-31)

----------------------

on roms board:
6x NM27C010Q (32-37)
2x N82S123N

*/

ROM_START( astormb2 )
	ROM_REGION( 0x080000, "main", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "1.a4", 0x000000, 0x10000, CRC(cca0d0af) SHA1(26fdbbeb8444d05f0ca2056a7c7fb81b0f1f2b5a) )
	ROM_LOAD16_BYTE( "2.a3", 0x020000, 0x10000, CRC(f95eb883) SHA1(b25d9c0fd46a534e7612f4a3ffa708b73654ae2b) )
	ROM_LOAD16_BYTE( "3.a2", 0x040000, 0x10000, CRC(4206ecd4) SHA1(45c65d7727cfaf215a7081159f6931185e92b39a) ) // epr13182.bin [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "4.a1", 0x060000, 0x10000, CRC(23247c95) SHA1(e4d78c453d2cb77946dd1b5266de823968eade77) ) // epr13182.bin [4/4]      98.648071%
	ROM_LOAD16_BYTE( "5.a9", 0x000001, 0x10000, CRC(6143039e) SHA1(8a5143c1e2c637149e988c423fa30b31e29a1193) )
	ROM_LOAD16_BYTE( "6.a8", 0x020001, 0x10000, CRC(0fd17bec) SHA1(e9a5dd93394fdf1561a925e4111dfce51b717b14) )
	ROM_LOAD16_BYTE( "7.a7", 0x040001, 0x10000, CRC(c901e228) SHA1(f459ba819a4e5f5174ff1b3957fb648c93beed53) ) // epr13181.bin [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "8.a6", 0x060001, 0x10000, CRC(bfb9d607) SHA1(8c3e10c1397fa0807d8df4715c9eb1945c774924) ) // epr13181.bin [4/4]      98.587036%

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "32.01",  0x00000, 0x20000, CRC(d2aeb4ab) SHA1(9338ec5dc48f5d2b20511628a281236fe4646ef4) ) // epr13073.bin [1/2]      IDENTICAL
	ROM_LOAD( "33.011", 0x20000, 0x20000, CRC(2193f0ae) SHA1(84070f74693699c1ffc1a47517a97b5d058d08ec) ) // epr13073.bin [2/2]      IDENTICAL
	ROM_LOAD( "34.02",  0x40000, 0x20000, CRC(849aa725) SHA1(0f949dfe8a6c5796edc86a05339da80a158a95ae) ) // epr13074.bin [1/2]      IDENTICAL
	ROM_LOAD( "35.021", 0x60000, 0x20000, CRC(3f190347) SHA1(131953ccefb95eeef1ea90499ce521c3749f95c1) ) // epr13074.bin [2/2]      IDENTICAL
	ROM_LOAD( "36.03",  0x80000, 0x20000, CRC(c0f9628d) SHA1(aeacf5e409adfa0b9c28c90d4e89eb1f56cd5f4d) ) // epr13075.bin [1/2]      IDENTICAL
	ROM_LOAD( "37.031", 0xa0000, 0x20000, CRC(95af904e) SHA1(6574fa874c355c368109b417aab7d0b05c9d215d) ) // epr13075.bin [2/2]      IDENTICAL

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "17.042", 0x000001, 0x20000, CRC(db08beb5) SHA1(c154d22c69b77637d6a9d0f2bffcfb47e6901ec8) ) // mpr13082.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "16.043", 0x040001, 0x20000, CRC(41f78977) SHA1(9cf9fcf96722d148c4b2cf7aa33425b6efcd0379) ) // mpr13082.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "29.012", 0x000000, 0x20000, CRC(22acf675) SHA1(80fd0d96017bf36d964a79f7e13e73fee7ed370a) ) // mpr13089.bin [1/2]      99.941254%
	ROM_LOAD16_BYTE( "28.013", 0x040000, 0x20000, CRC(32b37a3a) SHA1(70f268aa99a17739fd9d832b5f1d9e37247747e6) ) // mpr13089.bin [2/2]      IDENTICAL
 	ROM_LOAD16_BYTE( "19.040", 0x080001, 0x20000, CRC(10c359ac) SHA1(9087cb824242ce5fc8eba45b61cca8b329c576e5) ) // mpr13081.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "18.041", 0x0c0001, 0x20000, CRC(47146c1d) SHA1(cd5d92136f86128a9f304c4f8850f1efd652dd5c) ) // mpr13081.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "31.010", 0x080000, 0x20000, CRC(e88fc39c) SHA1(f19c55c49771625a76e65b639a3b23969db8031d) ) // mpr13088.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "30.011", 0x0c0000, 0x20000, CRC(6fe7e2a2) SHA1(94e5852377f72fd00daae302db4a5f93301213e4) ) // mpr13088.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "21.032", 0x100001, 0x20000, CRC(c9e5a258) SHA1(809a3a3f88efe9c7a9dd9f6439ccb48fffb84df0) ) // mpr13080.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "20.033", 0x140001, 0x20000, CRC(ddf8d00e) SHA1(0a9031063921bb03e7fd57eea369a1ddcfa85431) ) // mpr13080.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "25.022", 0x100000, 0x20000, CRC(af8f3700) SHA1(3787f732eee5c6c9b6550bd4ce5387aff2c4072e) ) // mpr13087.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "24.023", 0x140000, 0x20000, CRC(a092ecb6) SHA1(d7cc85eaea70c7947c497bc1d9743ab12a6fb43e) ) // mpr13087.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "23.030", 0x180001, 0x20000, CRC(adc1b625) SHA1(496a1e92a833dbde37a0426165ff4250848b6ef4) ) // epr13079.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "22.031", 0x1c0001, 0x20000, CRC(27c27f38) SHA1(439502250da4e376d2aa4bd9122455c6991e334d) ) // epr13079.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "27.020", 0x180000, 0x20000, CRC(6c5312aa) SHA1(94b74c78f318fcc1881a2926cebc98033a7e535d) ) // epr13086.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "26.021", 0x1c0000, 0x20000, CRC(c67fc986) SHA1(5fac826f9dde45201e3b93582dbe29c584a10229) ) // epr13086.bin [2/2]      99.987030%

	/* Sound HW is very different to the originals */
	ROM_REGION( 0x210000, "sound", ROMREGION_ERASEFF ) /* Z80 sound CPU */
	ROM_LOAD( "9.a5", 0x10000, 0x08000, CRC(0a4638e9) SHA1(0470e03a194464ff53c7583637193b585f5fd79f) )

	ROM_REGION( 0x40000, "oki1", ROMREGION_ERASEFF ) /* Oki6295 Samples - fixed? samples */
	ROM_LOAD( "11.a10", 0x00000, 0x20000, CRC(7e0f752c) SHA1(a4070c3fa4848b5be223f9b927de4b6926dbb4e6) ) // contains sample table
	ROM_LOAD( "10.a11", 0x20000, 0x10000, CRC(722e5969) SHA1(9cf891c6533b2e2a5c4741aa4e405038a7bf4e97) )
	/* 0x30000- 0x3ffff banked? (guess) */

	ROM_REGION( 0xc0000, "oki2", ROMREGION_ERASEFF ) /* Oki6295 Samples - banked? samples*/
	ROM_LOAD( "12.a15", 0x00000, 0x20000, CRC(cb4517db) SHA1(4c93376c2b3e70001bbc283d4485bb55514f6ef9) )
	ROM_LOAD( "13.a14", 0x20000, 0x20000, CRC(c60d6f18) SHA1(c9610729f19ae8414efd785948a1e6fb079bfe8d) )
	ROM_LOAD( "14.a13", 0x40000, 0x20000, CRC(07e6b3a5) SHA1(32da2a9aeb840b76e6f0117ac342ff5d612762b4) )
	ROM_LOAD( "15.a12", 0x60000, 0x20000, CRC(dffde929) SHA1(037b32470747d155385e532ee574b1234b3c2b26) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "n82s129n.129",  0x0000, 0x0100, CRC(a7c22d96) SHA1(160deae8053b09c09328325246598b3518c7e20b) )
	ROM_LOAD( "n82s123n.123",  0x0100, 0x0020, CRC(58bcf8bd) SHA1(e4d3d179b08c0f3424a6bec0f15058fb1b56f8d8) )
ROM_END


ROM_START( mwalkbl )
	ROM_REGION( 0x080000, "main", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "mwalkbl.01", 0x000000, 0x10000, CRC(f49cdb16) SHA1(34b7e98d31c3b9db2f0f055d7b249b0e5e5cb746) )
	ROM_LOAD16_BYTE( "mwalkbl.05", 0x000001, 0x10000, CRC(c483f29f) SHA1(8fdfa764d8e49754844a9dc001400d439f9af9f0) )
	ROM_LOAD16_BYTE( "mwalkbl.02", 0x020000, 0x10000, CRC(0bde1896) SHA1(42731ae90d56918dc50c0dcb53d092dcfb957159) )
	ROM_LOAD16_BYTE( "mwalkbl.06", 0x020001, 0x10000, CRC(5b9fc688) SHA1(53d8143c3876548f63b392f0ea16c0e7c30a7917) )
	ROM_LOAD16_BYTE( "mwalkbl.03", 0x040000, 0x10000, CRC(0c5fe15c) SHA1(626e3f37f019448c3c96bf73b2d2b5fe4b3716c0) )
	ROM_LOAD16_BYTE( "mwalkbl.07", 0x040001, 0x10000, CRC(9e600704) SHA1(efd3d450b26f81dc2b74f44b4aaf906fa017e437) )
	ROM_LOAD16_BYTE( "mwalkbl.04", 0x060000, 0x10000, CRC(64692f79) SHA1(ad7f32997b78863e3aa3214018cdd24e3ec9c5cb) )
	ROM_LOAD16_BYTE( "mwalkbl.08", 0x060001, 0x10000, CRC(546ca530) SHA1(51f74878fdc221fee026e2e6a7ca96f290c8947f) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "mpr13216.b1", 0x00000, 0x40000, CRC(862d2c03) SHA1(3c5446d702a639b62a602c6d687f9875d8450218) )
	ROM_LOAD( "mpr13217.b2", 0x40000, 0x40000, CRC(7d1ac3ec) SHA1(8495357304f1df135bba77ef3b96e79a883b8ff0) )
	ROM_LOAD( "mpr13218.b3", 0x80000, 0x40000, CRC(56d3393c) SHA1(50a2d065060692c9ecaa56046a781cb21d93e554) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "mpr13224.b11", 0x000001, 0x40000, CRC(c59f107b) SHA1(10fa60fca6e34eda277c483bb1c0e81bb88c8a47) )
	ROM_LOAD16_BYTE( "mpr13231.a11", 0x000000, 0x40000, CRC(a5e96346) SHA1(a854f4dd5dc16975373255110fdb8ab3d121b1af) )
	ROM_LOAD16_BYTE( "mpr13223.b10", 0x080001, 0x40000, CRC(364f60ff) SHA1(9ac887ec0b2e32b504b7c6a5f3bb1ce3fe41a15a) )
	ROM_LOAD16_BYTE( "mpr13230.a10", 0x080000, 0x40000, CRC(9550091f) SHA1(bb6e898f7b540e130fd338c10f74609a7604cef4) )
	ROM_LOAD16_BYTE( "mpr13222.b9",  0x100001, 0x40000, CRC(523df3ed) SHA1(2e496125e75decd674c3a08404fbdb53791a965d) )
	ROM_LOAD16_BYTE( "mpr13229.a9",  0x100000, 0x40000, CRC(f40dc45d) SHA1(e9468cef428f52ecdf6837c6d9a9fea934e7676c) )
	ROM_LOAD16_BYTE( "epr13221.b8",  0x180001, 0x40000, CRC(9ae7546a) SHA1(5413b0131881b0b32bac8de51da9a299835014bb) )
	ROM_LOAD16_BYTE( "epr13228.a8",  0x180000, 0x40000, CRC(de3786be) SHA1(2279bb390aa3efab9aeee0a643e5cb6a4f5933b6) )

	ROM_REGION( 0x100000, "sound", 0 ) /* sound CPU */
	ROM_LOAD( "epr13225.a4", 0x10000, 0x20000, CRC(56c2e82b) SHA1(d5755a1bb6e889d274dc60e883d4d65f12fdc877) )
	ROM_LOAD( "mpr13219.b4", 0x30000, 0x40000, CRC(19e2061f) SHA1(2dcf1718a43dab4da53b4f67722664e70ddd2169) )
	ROM_LOAD( "mpr13220.b5", 0x70000, 0x40000, CRC(58d4d9ce) SHA1(725e73a656845b02702ef131b4c0aa2a73cdd02e) )
	ROM_LOAD( "mpr13249.b6", 0xb0000, 0x40000, CRC(623edc5d) SHA1(c32d9f818d40f311877fbe6532d9e95b6045c3c4) )
ROM_END


// Shadow Dancer
ROM_START( shdancbl )
	ROM_REGION( 0x080000, "main", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ic39", 0x000000, 0x10000, CRC(adc1781c) SHA1(b2ca2831a48779df7533e6b2a406ee539e1f650c) )
	ROM_LOAD16_BYTE( "ic53", 0x000001, 0x10000, CRC(1c1ac463) SHA1(21075f7afae372daef197f04f5f12d14479a8140) )
	ROM_LOAD16_BYTE( "ic38", 0x020000, 0x10000, CRC(cd6e155b) SHA1(e37b53cc431533091d26b37be9b8e30494de5faf) )
	ROM_LOAD16_BYTE( "ic52", 0x020001, 0x10000, CRC(bb3c49a4) SHA1(ab01a6de1a6d338d30f9cfea7b3bf80dda67f215) )
	ROM_LOAD16_BYTE( "ic37", 0x040000, 0x10000, CRC(1bd8d5c3) SHA1(4d663362c059e112ac6c742d80200be98d50d175) )
	ROM_LOAD16_BYTE( "ic51", 0x040001, 0x10000, CRC(ce2e71b4) SHA1(3e251319cd4c8c63c66e6b92b2eef514d79dba8e) )
	ROM_LOAD16_BYTE( "ic36", 0x060000, 0x10000, CRC(bb861290) SHA1(62ea8eec74c6b1f5530ee86f97ad821daeac26ad) )
	ROM_LOAD16_BYTE( "ic50", 0x060001, 0x10000, CRC(7f7b82b1) SHA1(675020b57ce689b2767ff83773e2b828cda5aeed) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "ic4",  0x00000, 0x20000, CRC(f0a016fe) SHA1(1426f3fbf50a04a8c5e998e071ca0e78d15f37a8) )
	ROM_LOAD( "ic18", 0x20000, 0x20000, CRC(f6bee053) SHA1(39ee5edfcc67bb4855217c7428254f3e8c862ba0) )
	ROM_LOAD( "ic3",  0x40000, 0x20000, CRC(e07e6b5d) SHA1(bdeb1193415049d0c9261ca261073bdd9e251b88) )
	ROM_LOAD( "ic17", 0x60000, 0x20000, CRC(f59deba1) SHA1(21188d22fe607281bb7da1e1f418a33d4a315695) )
	ROM_LOAD( "ic2",  0x80000, 0x20000, CRC(60095070) SHA1(913c2ee51fb6f838f3c6cbd27032bdf754fbadf1) )
	ROM_LOAD( "ic16", 0xa0000, 0x20000, CRC(0f0d5dd3) SHA1(76812e2f831256a8b6598257dd84a7f07443642e) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */

	// 12719
	ROM_LOAD16_BYTE( "ic73", 0x000001, 0x10000, CRC(59e77c96) SHA1(08da058529ac83352a4528d3792a21edda348f7a) )
	ROM_LOAD16_BYTE( "ic74", 0x020001, 0x10000, CRC(90ea5407) SHA1(4bdd93c86cb35822517433d491aa8be6857dd36c) )
	ROM_LOAD16_BYTE( "ic75", 0x040001, 0x10000, CRC(27d2fa61) SHA1(0ba3cd9448e54ce9fc9433f3edd28de9a4e451e9) )
	ROM_LOAD16_BYTE( "ic76", 0x060001, 0x10000, CRC(f36db688) SHA1(a527298ce9ca1d9f5aa7b9eac93985f34ca8119f) )

	// 12726
	ROM_LOAD16_BYTE( "ic58", 0x000000, 0x10000, CRC(9cd5c8c7) SHA1(54c2d0a683bda37eb9a75f90f4ca5e620c09c4cf) )
	ROM_LOAD16_BYTE( "ic59", 0x020000, 0x10000, CRC(ff40e872) SHA1(bd2c4aac427d106a46318f4cb2eb05c34d3c70b6) )
	ROM_LOAD16_BYTE( "ic60", 0x040000, 0x10000, CRC(826d7245) SHA1(bb3394de058bd63b9939cd05f22c925e0cdc840a) )
	ROM_LOAD16_BYTE( "ic61", 0x060000, 0x10000, CRC(dcf8068b) SHA1(9c78de224df76fc90fb90f1bbd9b22dad0874f69) )

	// 12718
	ROM_LOAD16_BYTE( "ic77", 0x080001, 0x10000, CRC(f93470b7) SHA1(1041afa43aa8d0589d6def9743721cdbda617f78) )
//  ROM_LOAD16_BYTE( "ic78", 0x0A0001, 0x10000, CRC(4d523ea3) SHA1(053c30778017127dddeae0783af463aef17bcc9a) ) // corrupt? (bad sprite when dog attacts in attract mode)
	ROM_LOAD16_BYTE( "sdbl.78", 0x0A0001, 0x10000, CRC(e533be5d) SHA1(926d6ba3f7a3ac289b0ae40d7633c70b2819df4d) )
	ROM_LOAD16_BYTE( "ic95", 0x0C0001, 0x10000, CRC(828b8294) SHA1(f2cdb882fb0709a909e6ef98f0315aceeb8bf283) )
//  ROM_LOAD16_BYTE( "ic94", 0x0E0001, 0x10000, CRC(542b2d1e) SHA1(1ce91aea6c49e6e365a91c30ca3049682c2162da) )
	ROM_LOAD16_BYTE( "sdbl.94", 0x0E0001, 0x10000, CRC(e2fa2b41) SHA1(7186107734dac5763dee43addcea7c14fb0d9d74) )

	// 12725
	ROM_LOAD16_BYTE( "ic62", 0x080000, 0x10000, CRC(50ca8065) SHA1(8c0d6ae34b9da6c376df387e8fc8b1068bcb4dcb) )
	ROM_LOAD16_BYTE( "ic63", 0x0A0000, 0x10000, CRC(d1866aa9) SHA1(524c82a12a1c484a246b8d49d9f05a774d008108) )
	ROM_LOAD16_BYTE( "ic90", 0x0C0000, 0x10000, CRC(3602b758) SHA1(d25b6c8420e07d0f2ac3e1d8717f14738466df16) )
	ROM_LOAD16_BYTE( "ic89", 0x0E0000, 0x10000, CRC(1ba4be93) SHA1(6f4fe2016e375be3df477436f5cde7508a24ecd1) )

	// 12717
	ROM_LOAD16_BYTE( "ic79", 0x100001, 0x10000, CRC(f22548ee) SHA1(723cb7604784c6715817daa8c86c18c6bcd1388d) )
	ROM_LOAD16_BYTE( "ic80", 0x120001, 0x10000, CRC(6209f7f9) SHA1(09b33c99d972a62af8ef56dacfa6262f002aba0c) )
	ROM_LOAD16_BYTE( "ic81", 0x140001, 0x10000, CRC(34692f23) SHA1(56126a81ac279662e3e3423da5205f65a62c4600) )
	ROM_LOAD16_BYTE( "ic82", 0x160001, 0x10000, CRC(7ae40237) SHA1(fae97cfcfd3cd557da3330158831e4727c438745) )

	// 12724
	ROM_LOAD16_BYTE( "ic64", 0x100000, 0x10000, CRC(7a8b7bcc) SHA1(00cbbbc4b3db48ca3ac65ff56b02c7d63a1b898a) )
	ROM_LOAD16_BYTE( "ic65", 0x120000, 0x10000, CRC(90ffca14) SHA1(00962e5309a79ce34c6f420036054bc607595dfe) )
	ROM_LOAD16_BYTE( "ic66", 0x140000, 0x10000, CRC(5d655517) SHA1(2a1c197dde62bd7946ca7b5f1c2833bdbc2e2e32) )
	ROM_LOAD16_BYTE( "ic67", 0x160000, 0x10000, CRC(0e5d0855) SHA1(3c15088f7fdda5c2bba9c89d244bbcff022f05fd) )

	// 12716
	ROM_LOAD16_BYTE( "ic83", 0x180001, 0x10000, CRC(a9040a32) SHA1(7b0b375285f528b2833c50892b55b0d4c550506d) )
	ROM_LOAD16_BYTE( "ic84", 0x1A0001, 0x10000, CRC(d6810031) SHA1(a82857a9ac442fbe076cdafcf7390765391ed136) )
	ROM_LOAD16_BYTE( "ic92", 0x1C0001, 0x10000, CRC(b57d5cb5) SHA1(636f1a07a84d37cecbe388a2f585893c4611436c) )
	ROM_LOAD16_BYTE( "ic91", 0x1E0001, 0x10000, CRC(49def6c8) SHA1(d8b2cc1993f0808553f87bf56fdbe47374576c5a) )

	// 12723
	ROM_LOAD16_BYTE( "ic68", 0x180000, 0x10000, CRC(8d684e53) SHA1(00e82ddaf875a7452ff978b7b7eb87a1a5a8fb64) )
	ROM_LOAD16_BYTE( "ic69", 0x1A0000, 0x10000, CRC(c47d32e2) SHA1(92b21f51abdd7950fb09d965b1d71b7bffac31ec) )
	ROM_LOAD16_BYTE( "ic88", 0x1C0000, 0x10000, CRC(9de140e1) SHA1(f1125e056a898a4fa519b49ae866c5c742e36bf7) )
	ROM_LOAD16_BYTE( "ic87", 0x1E0000, 0x10000, CRC(8172a991) SHA1(6d12b1533a19cb02613b473cc8ba73ece1f2a2fc) )

	ROM_REGION( 0x30000, "sound", 0 ) /* sound CPU */
	ROM_LOAD( "ic45", 0x10000, 0x10000, CRC(576b3a81) SHA1(b65356a3837ed3875634ab0cbcd61acce44f2bb9) )
	ROM_LOAD( "ic46", 0x20000, 0x10000, CRC(c84e8c84) SHA1(f57895bedb6152c30733e91e6f4795702a62ac3a) )
ROM_END


/*****************************************************************************/

GAME( 1990, astormbl, astorm,   astormbl, astormbl, astormbl, ROT0, "bootleg", "Alien Storm (bootleg, set 1)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1990, astormb2, astorm,   astormbl, astormbl, astormbl, ROT0, "bootleg", "Alien Storm (bootleg, set 2)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND )

GAME( 1990, mwalkbl,  mwalk,    mwalkbl,  mwalkbl,  mwalkbl,  ROT0, "bootleg", "Michael Jackson's Moonwalker (bootleg)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1989, shdancbl, shdancer, shdancbl, mwalkbl,  shdancbl, ROT0, "bootleg", "Shadow Dancer (bootleg)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

