// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
/***************************************************************************

 Seibu Cop (Co-Processor) device emulation
  a.k.a. known as Toshiba gate array TC25SC rebadged as:
  SEI300 - Legionnaire PCB
  There's also a ROM labeled COP-Dx, which is probably used for some in-game maths: 
  COP-D1 - Seibu Cup Soccer PCBs
  COP-D2 - legionna.cpp and raiden2.cpp, latter might use another component too
  COP-D3 - New Zero Team / Raiden 2 V33 HWs
  Raiden 2 first boss arms is known to behave strangely without this ROM on a real PCB
  
  (new implementation, based on Raiden 2 code)
  
 TODO:
 - improve documentation, ffs!
 - split commands into own file, 2000+ lines is excessive;
 - improve class OO public/protected/private;
 - give this one own folder;
 - nuke legacy command implementations;
 - assert for something that needs actual playtesting is bad.
 - add better debug facilities in a new sub-class, including but not limited to:
   - disable collision detection;
   - printing facilities;
   - debugger break on pre-setted commands;
   - ...
  
  Tech notes:
  -----------
  [0x6fc] DMA mode bit scheme:
  ---1 ---1 ---- ---- fill op if true, else transfer
  ---- ---- x--- ---- palette brightness
  ---- ---- ---x ---- internal buffer selector
  ---- ---- ---- x--- size modifier? Bus transfer size actually?
  ---- ---- ---- -xxx select channel
  
***************************************************************************/

#include "emu.h"
#include "seibucop.h"
#include "debugger.h"

// use Z to dump out table info
//#define TABLE_DUMPER

#include "seibucop_dma.hxx"
#include "seibucop_cmd.hxx"

#define seibu_cop_log \
	if (LOG_Commands) logerror


const device_type RAIDEN2COP = &device_creator<raiden2cop_device>;

raiden2cop_device::raiden2cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RAIDEN2COP, "Seibu COP (Raiden 2)", tag, owner, clock, "raiden2cop", __FILE__),
	cop_latch_addr(0),
	cop_latch_trigger(0),
	cop_latch_value(0),
	cop_latch_mask(0),
	cop_dma_v1(0),
	cop_dma_v2(0),
	cop_dma_mode(0),
	cop_dma_adr_rel(0),
	pal_brightness_val(0),
	pal_brightness_mode(0),
	cop_itoa(0),
	cop_itoa_mode(0),

	cop_status(0),
	cop_scale(0),

	cop_angle(0),
	cop_dist(0),

	cop_angle_target(0),
	cop_angle_step(0),

	cop_hit_status(0),
	cop_hit_baseadr(0),
	cop_sort_ram_addr(0),
	cop_sort_lookup(0),
	cop_sort_param(0),

	m_cop_rng_max_value(0),

	m_cop_sprite_dma_param(0),
	m_cop_sprite_dma_src(0),
	m_cop_sprite_dma_size(0),

	m_cop_unk_param_a(0),
	m_cop_unk_param_b(0),

	m_cop_rom_addr_lo(0),
	m_cop_rom_addr_hi(0),
	m_cop_precmd(0),

	m_cop_sprite_dma_abs_x(0),
	m_cop_sprite_dma_abs_y(0),

	m_LEGACY_r0(0),
	m_LEGACY_r1(0),

	m_videoramout_cb(*this),
	m_palette(*this, ":palette")
{
	memset(cop_func_trigger, 0, sizeof(UINT16)*(0x100/8));
	memset(cop_func_value, 0, sizeof(UINT16)*(0x100/8));
	memset(cop_func_mask, 0, sizeof(UINT16)*(0x100/8));
	memset(cop_program, 0, sizeof(UINT16)*(0x100));

	memset(cop_dma_src, 0, sizeof(UINT16)*(0x200));
	memset(cop_dma_dst, 0, sizeof(UINT16)*(0x200));
	memset(cop_dma_size, 0, sizeof(UINT16)*(0x200));

	memset(cop_itoa_digits, 0, sizeof(UINT8)*10);

	memset(cop_regs, 0, sizeof(UINT32)*8);


	memset(cop_collision_info, 0, sizeof(colinfo)*2);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void raiden2cop_device::device_start()
{
	save_item(NAME(cop_func_trigger));
	save_item(NAME(cop_func_value));
	save_item(NAME(cop_func_mask));
	save_item(NAME(cop_program));

	save_item(NAME(cop_latch_addr));
	save_item(NAME(cop_latch_trigger));
	save_item(NAME(cop_latch_value));
	save_item(NAME(cop_latch_mask));

	save_item(NAME(cop_dma_v1));
	save_item(NAME(cop_dma_v2));
	save_item(NAME(cop_dma_mode));
	save_item(NAME(cop_dma_adr_rel));
	save_item(NAME(pal_brightness_val));
	save_item(NAME(pal_brightness_mode));

	save_item(NAME(cop_dma_src));
	save_item(NAME(cop_dma_dst));
	save_item(NAME(cop_dma_size));

	save_item(NAME(cop_itoa));
	save_item(NAME(cop_itoa_mode));
	save_item(NAME(cop_itoa_digits));

	save_item(NAME(cop_status));
	save_item(NAME(cop_scale));

	save_item(NAME(cop_angle));
	save_item(NAME(cop_dist));

	save_item(NAME(cop_angle_target));
	save_item(NAME(cop_angle_step));

	save_item(NAME(cop_hit_status));
	save_item(NAME(cop_hit_baseadr));
	save_item(NAME(cop_hit_val));
	save_item(NAME(cop_hit_val_stat));
	save_item(NAME(cop_sort_ram_addr));
	save_item(NAME(cop_sort_lookup));
	save_item(NAME(cop_sort_param));

	save_item(NAME(cop_regs));

	save_item(NAME(cop_collision_info[0].pos));
	save_item(NAME(cop_collision_info[0].dx));
	save_item(NAME(cop_collision_info[0].size));
	save_item(NAME(cop_collision_info[0].spradr));
	save_item(NAME(cop_collision_info[0].allow_swap));
	save_item(NAME(cop_collision_info[0].flags_swap));
	save_item(NAME(cop_collision_info[0].min));
	save_item(NAME(cop_collision_info[0].max));

	save_item(NAME(cop_collision_info[1].pos));
	save_item(NAME(cop_collision_info[1].dx));
	save_item(NAME(cop_collision_info[1].size));
	save_item(NAME(cop_collision_info[1].spradr));
	save_item(NAME(cop_collision_info[1].allow_swap));
	save_item(NAME(cop_collision_info[1].flags_swap));
	save_item(NAME(cop_collision_info[1].min));
	save_item(NAME(cop_collision_info[1].max));

	save_item(NAME(m_cop_rng_max_value));

	save_item(NAME(m_cop_sprite_dma_param));

	save_item(NAME(m_cop_sprite_dma_size));
	save_item(NAME(m_cop_sprite_dma_src));

	save_item(NAME(m_cop_unk_param_a));
	save_item(NAME(m_cop_unk_param_b));

	save_item(NAME(m_cop_rom_addr_lo));
	save_item(NAME(m_cop_rom_addr_hi));
	save_item(NAME(m_cop_precmd));

	save_item(NAME(m_cop_sprite_dma_abs_x));
	save_item(NAME(m_cop_sprite_dma_abs_y));

	// legacy
	save_item(NAME(m_LEGACY_r0));
	save_item(NAME(m_LEGACY_r1));

	m_videoramout_cb.resolve_safe();
	// TODO: tag parameter in device
	m_host_cpu = machine().device<cpu_device>("maincpu");
	m_host_space = &m_host_cpu->space(AS_PROGRAM);
	m_host_endian = m_host_space->endianness() == ENDIANNESS_BIG; // m_cpu_is_68k
	
	m_byte_endian_val = m_host_endian ? 3 : 0;
	m_word_endian_val = m_host_endian ? 2 : 0;
}

UINT16 raiden2cop_device::cop_read_word(int address)
{
	return m_host_space->read_word(address ^ m_word_endian_val);
}

UINT8 raiden2cop_device::cop_read_byte(int address)
{
	return m_host_space->read_byte(address ^ m_byte_endian_val);
}

void raiden2cop_device::cop_write_word(int address, UINT16 data)
{
	m_host_space->write_word(address ^ m_word_endian_val, data);
}

void raiden2cop_device::cop_write_byte(int address, UINT8 data)
{
	m_host_space->write_byte(address ^ m_byte_endian_val, data);
}


/*** Command Table uploads ***/



WRITE16_MEMBER(raiden2cop_device::cop_pgm_data_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	cop_program[cop_latch_addr] = data;
	int idx = cop_latch_addr >> 3;
	cop_func_trigger[idx] = cop_latch_trigger;
	cop_func_value[idx]   = cop_latch_value;
	cop_func_mask[idx]    = cop_latch_mask;

	if(data) {
		int off = data & 31;
		int reg = (data >> 5) & 3;
		int op = (data >> 7) & 31;

		logerror("COPDIS: %04x s=%02x f1=%x l=%x f2=%02x %x %04x %02x %03x %02x.%x.%02x ", cop_latch_trigger,  (cop_latch_trigger >> 11) << 3, (cop_latch_trigger >> 10) & 1, ((cop_latch_trigger >> 7) & 7)+1, cop_latch_trigger & 0x7f, cop_latch_value, cop_latch_mask, cop_latch_addr, data, op, reg, off);

		off *= 2;

		// COPDIS: 0205 s=00 f1=0 l=5 f2=05 6 ffeb 00 188 03.0.08 read32 10(r0)
		// COPDIS: 0205 s=00 f1=0 l=5 f2=05 6 ffeb 01 282 05.0.02 add32 4(r0)
		// COPDIS: 0205 s=00 f1=0 l=5 f2=05 6 ffeb 02 082 01.0.02 write32 4(r0)
		// COPDIS: 0205 s=00 f1=0 l=5 f2=05 6 ffeb 03 b8e 17.0.0e add16h 1c(r0)
		// COPDIS: 0205 s=00 f1=0 l=5 f2=05 6 ffeb 04 98e 13.0.0e write16h 1c(r0)

		// 188 182 082 b8e 98e -> 04  = 04+04    1ch = 1c+04
		// 188 188 082 b8e 98e -> 04  = 04+10    1ch = 1c+10
		// 188 18e 082 b8e 98e -> 04  = 04+1c    1ch = 1c+1c
		// 188 282 082 b8e 98e -> 04  = 04+10    1ch = 1c+10
		// 188 288 082 b8e 98e -> 04  = 10+10    1ch = 1c+10
		// 188 28e 082 b8e 98e -> 04  = 1c+10    1ch = 1c+10
		// 188 282 282 282 082 -> 04  = 04+04+10 10h = 04+10
		// 188 188 188 188 082 -> 04h = 04+10    04l = 04+10+10
		// 188 188 188 188 082 -> 04  = 04+10    04l = 04+10+10  10h = 04+10 (same, but trigger = 020b)

		switch(op) {
		case 0x01:
			if(off)
				logerror("addmem32 %x(r%x)\n", off, reg);
			else
				logerror("addmem32 (r%x)\n", reg);
			break;
		case 0x03:
			if(off)
				logerror("read32 %x(r%x)\n", off, reg);
			else
				logerror("read32 (r%x)\n", reg);
			break;
		case 0x05:
			if(off)
				logerror("add32 %x(r%x)\n", off, reg);
			else
				logerror("add32 (r%x)\n", reg);
			break;
		case 0x13:
			if(off)
				logerror("write16h %x(r%x)\n", off, reg);
			else
				logerror("write16h (r%x)\n", reg);
			break;
		case 0x15:
			if(off)
				logerror("sub32 %x(r%x)\n", off, reg);
			else
				logerror("sub32 (r%x)\n", reg);
			break;
		case 0x17:
			if(off)
				logerror("addmem16 %x(r%x)\n", off, reg);
			else
				logerror("addmem16 (r%x)\n", reg);
			break;
		default:
			logerror("?\n");
			break;
		}
	}
}

void raiden2cop_device::dump_table()
{
#ifdef TABLE_DUMPER
	printf("table dump\n");

	int command;

	printf("## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask\n");

	for (command = 0; command < 0x20; command++)
	{
		if (cop_func_trigger[command] != 0x0000)
		{
			int maskout = 0x07ff;

			printf("%02x - %04x ( %02x) (  %03x) :  ", command, cop_func_trigger[command], (cop_func_trigger[command] & ~maskout)>>11, (cop_func_trigger[command] & maskout));

			printf("(");
			int seqpos;
			for (seqpos = 0; seqpos < 8; seqpos++)
			{
				printf("%03x", cop_program[command * 8 + seqpos]);
				if (seqpos < 7)
					printf(", ");
			}
			printf(")  ");

			printf("%01x     ", cop_func_value[command]);
			printf("%04x ", cop_func_mask[command]);


			printf("  (%s)\n", machine().system().name);
		}
	}
#endif
}


WRITE16_MEMBER(raiden2cop_device::cop_pgm_addr_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	assert(data < 0x100);
	cop_latch_addr = data;
}

WRITE16_MEMBER(raiden2cop_device::cop_pgm_value_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	cop_latch_value = data;
}

WRITE16_MEMBER(raiden2cop_device::cop_pgm_mask_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	cop_latch_mask = data;
}

WRITE16_MEMBER(raiden2cop_device::cop_pgm_trigger_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	cop_latch_trigger = data;
}




// currently only used by legionna.c implementation
int raiden2cop_device::find_trigger_match(UINT16 triggerval, UINT16 mask)
{
	/* search the uploaded 'trigger' table for a matching trigger*/
	/* note, I don't know what the 'mask' or 'value' tables are... probably important, might determine what actually gets executed! */
	/* note: Zero Team triggers macro 0x904 instead of 0x905, Seibu Cup Soccer triggers 0xe30e instead of 0xe38e. I highly doubt that AT LEAST
	    it isn't supposed to do anything, especially in the former case (it definitely NEEDS that sprites have an arc movement when they are knocked down). */
	// we currently pass in mask 0xff00 to look at only match the top bits, but this is wrong, only specific bits are ignored (maybe depends on the 'mask' value uploaded with each trigger?)
	int matched = 0;
	int command = -1;

	for (int i = 0; i < 32; i++)
	{
		// technically 'command' could equal cop_func_trigger[i] >> 11, but there is that odd entry in zero team where that doesn't appear to be true (search 'TABLENOTE1')

		if ((triggerval & mask) == (cop_func_trigger[i] & mask) && cop_func_trigger[i] != 0) /* cop_func_trigger[i] != 0 is just being used to prevent matching against empty / unused slots */
		{
			int otherlog = 1;

			// just some per-game debug code so that we have a record of exactly which triggers each game is known to use


			if (!strcmp(machine().system().name, "legionna"))
			{
				// enemies often walk on the spot, bosses often walk above / below playable game area (the >>16 in the sqrt commands seems responsible)
				// player doesn't walk off screen after end of level (walks on spot, different cause?)
				if (triggerval == 0x0205 || triggerval == 0x0905 ||
					triggerval == 0x8100 || triggerval == 0x8900 || /* sin / cos */
					triggerval == 0x138e || // atan?
					triggerval == 0x3bb0 || // distance?
					triggerval == 0x42c2 || // distance?
					triggerval == 0xa180 || triggerval == 0xa980 || triggerval == 0xb100 || triggerval == 0xb900) /* collisions */
					otherlog = 0;
				// An unused routine writes to 0x9180 and 0x9980 (arguments written to 0x100442 and 0x100440 first)
			}
			else if (!strcmp(machine().system().name, "cupsoc"))
			{
				if (triggerval == 0x0204 || triggerval == 0x0205 || triggerval == 0x0905 ||
					triggerval == 0x130e || triggerval == 0x138e || triggerval == 0x118e ||
					triggerval == 0x3bb0 ||
					triggerval == 0x42c2 ||
					triggerval == 0x5105 || triggerval == 0x5905 ||
					triggerval == 0x6200 ||
					triggerval == 0xd104 ||
					triggerval == 0xdde5 ||
					triggerval == 0xe30e || triggerval == 0xe18e ||
					triggerval == 0xf105 ||
					triggerval == 0x8100 || triggerval == 0x8900) /* sin / cos */
					otherlog = 0;
				// Unused M68000 code in some (earlier?) sets also writes these trigger values:
				// - 0x9180, 0x9980 (arguments written to 0x100442 and 0x100440 first)
				// - 0xa180, 0xa980, 0x1905 (twice at successive offsets)
				// - 0x2a05 (twice or three times at successive offsets)
				// - 0x2288 (may set bit 15 of status), 0x138e, 0x3bb0
				// - 0x338e (may set bit 15 of status), 0x3bb0, 0x4aa0
				// - 0xa180, 0xb100, 0xa980, 0xb100
				// - 0xa180, 0x6880 (to second offset), 0xc480 (to second offset; one of these may set bit 1 of status)
			}
			else if (!strcmp(machine().system().name, "heatbrl"))
			{
				// note, stage 2 end boss (fire breather) will sometimes glitch (and homing missile stg3 - they don't home in properly either?)
				// stage 2+3 priority of boaters is wrong
				// game eventually crashes with address error (happened in stage 4 for me)

				if (triggerval == 0x0205 ||
					triggerval == 0x8100 || triggerval == 0x8900 || /* sin / cos */
					triggerval == 0x138e ||
					triggerval == 0x3bb0 ||
					triggerval == 0x42c2 ||
					triggerval == 0xa100 || triggerval == 0xa900 || triggerval == 0xb080 || triggerval == 0xb880) /* collisions */
					otherlog = 0;
				// Program code also includes the unused routine from Legionnaire
			}
			else if (!strcmp(machine().system().name, "godzilla"))
			{
				// only uses collisions? - possible this one already 'works' apart from prio problems, haven't managed to test beyond 1 level tho

				if (triggerval == 0xa180 || triggerval == 0xa980 || triggerval == 0xb100 || triggerval == 0xb900) /* collisions */
					otherlog = 0;
			}
			else if (!strcmp(machine().system().name, "grainbow"))
			{
				// path 3 (caves) midboss has wrong tiles
				// stage 4 (after 3 selectable stages) has sprite glitches bottom left
				// fade logic is wrong (palettes for some layers shouldn't fade) - DMA operation related, not command related

				if (triggerval == 0x0205 ||
					triggerval == 0x8100 || triggerval == 0x8900 || /* sin / cos */
					triggerval == 0x138e ||
					triggerval == 0x3bb0 ||

					triggerval == 0xa180 || triggerval == 0xa980 || triggerval == 0xb100 || triggerval == 0xb900 || /* collisions */
					triggerval == 0xc480 ||
					triggerval == 0x6200 ||
					triggerval == 0x6980)
					otherlog = 0;
				// Program code also includes many of the unused routines from Olympic Soccer '92
			}
			else if (!strcmp(machine().system().name, "denjinmk"))
			{
				// never calls any programs (M68000 code handles sine and cosine lookups on its own)
			}
			else if (!strcmp(machine().system().name, "zeroteam"))
			{
				// got stuck in lying in corner with tiny bit of health left on first boss (couldn't do anything, had to let 2nd player join)
				// birdman boss is wrong (unemulated commands) (final boss behavior is similar)
				// sprite priority is wrong (not command related - sort DMA)
				// 3rd stage mid-boss does not enter properly (have to use special attack to trigger them into motion)
				// 5th stage, does not punch door for speedboat section
				// bats all fly in a single straight line during ending (maybe PRNG, it isn't hooked up on R2 driver?)

				if (triggerval == 0x0205 || triggerval == 0x0904 ||
					triggerval == 0x8100 || triggerval == 0x8900 || /* sin / cos */
					triggerval == 0x130e || triggerval == 0x138e ||
					triggerval == 0x3b30 ||
					triggerval == 0x42c2 || // throwing
					triggerval == 0x6200 || // emeny throwing crates to the left?
					triggerval == 0xa180 || triggerval == 0xa980 || triggerval == 0xb100 || triggerval == 0xb900 || /* collisions */

					// 2nd level 'bird man' boss uses these
					triggerval == 0xfc84 ||
					triggerval == 0xf790 ||
					triggerval == 0xede5 ||
					triggerval == 0x330e ||
					triggerval == 0x4aa0)

					otherlog = 0;
				// An unused routine in the program code triggers 0x6980 and 0x7100 after writing zero to the word at 0x426
			}
			else if (!strcmp(machine().system().name, "xsedae"))
			{
				// not really sure what's right / wrong with this one..
				if (triggerval == 0x0205 || triggerval == 0x0904 ||
					triggerval == 0x8100 || triggerval == 0x8900 || /* sin / cos */
					triggerval == 0x5a85 ||
					triggerval == 0x5105 ||
					triggerval == 0x130e ||
					triggerval == 0x3b30
					)
					otherlog = 0;
			}
			else if (!strcmp(machine().system().name, "raiden2"))
			{
				if (triggerval == 0x0205 || triggerval == 0x0905 ||
					triggerval == 0x8100 || triggerval == 0x8900 || /* sin / cos */
					triggerval == 0x130e || triggerval == 0x138e ||
					triggerval == 0x2a05 ||
					triggerval == 0x2208 || triggerval == 0x2288 ||
					triggerval == 0x338e ||
					triggerval == 0x39b0 || triggerval == 0x3bb0 ||
					triggerval == 0x4aa0 ||
					triggerval == 0x42c2 ||
					triggerval == 0x5205 ||
					triggerval == 0x5a05 ||
					triggerval == 0x6200 ||
					triggerval == 0xf205 ||
					triggerval == 0xa100 || triggerval == 0xa900 || triggerval == 0xb100 || triggerval == 0xb900 /* collisions */
					)
					otherlog = 0;
				// An unused routine in the program code triggers 0x9100 and 0x9900
			}
			else
			{
				otherlog = 0;
			}

			seibu_cop_log("    Cop Command %04x found in slot %02x with other params %04x %04x\n", triggerval, i, cop_func_value[i], cop_func_mask[i]);

			if (otherlog == 1) printf("used command %04x\n", triggerval);

			command = i;
			matched++;
		}
	}

	if (matched == 1)
	{
		int j;
		seibu_cop_log("     Sequence: ");
		for (j=0;j<0x8;j++)
		{
			seibu_cop_log("%04x ", cop_program[command*8+j]);
		}
		seibu_cop_log("\n");

		return command;
	}
	else if (matched == 0)
	{
		seibu_cop_log("    Cop Command %04x NOT IN TABLE!\n", triggerval);
		printf("Command Not Found!\n");
		return -1;
	}

	printf("multiple matches found with mask passed in! (bad!) (%04x %04x)\n", triggerval, mask); // this should never happen with the uploaded tables
	return -1;

}

//  only used by legionna.c implementation
int raiden2cop_device::check_command_matches(int command, UINT16 seq0, UINT16 seq1, UINT16 seq2, UINT16 seq3, UINT16 seq4, UINT16 seq5, UINT16 seq6, UINT16 seq7, UINT16 _funcval_, UINT16 _funcmask_)
{
	command *= 8;

	if (cop_program[command+0] == seq0 && cop_program[command+1] == seq1 && cop_program[command+2] == seq2 && cop_program[command+3] == seq3 &&
		cop_program[command+4] == seq4 && cop_program[command+5] == seq5 && cop_program[command+6] == seq6 && cop_program[command+7] == seq7 &&
		cop_func_value[command/8] == _funcval_ &&
		cop_func_mask[command/8] == _funcmask_)
		return 1;
	else
		return 0;
}

/*** Regular DMA ***/

WRITE16_MEMBER(raiden2cop_device::cop_dma_adr_rel_w)
{
	COMBINE_DATA(&cop_dma_adr_rel);
}

WRITE16_MEMBER(raiden2cop_device::cop_dma_v1_w)
{
	COMBINE_DATA(&cop_dma_v1);
}

WRITE16_MEMBER(raiden2cop_device::cop_dma_v2_w)
{
	COMBINE_DATA(&cop_dma_v2);
}

WRITE16_MEMBER(raiden2cop_device::cop_dma_dst_w)
{
	COMBINE_DATA(&cop_dma_dst[cop_dma_mode]);
}

READ16_MEMBER(raiden2cop_device::cop_dma_mode_r)
{
	return cop_dma_mode;
}

WRITE16_MEMBER(raiden2cop_device::cop_dma_mode_w)
{
	COMBINE_DATA(&cop_dma_mode);
}

WRITE16_MEMBER(raiden2cop_device::cop_dma_src_w)
{
	COMBINE_DATA(&cop_dma_src[cop_dma_mode]);
}

WRITE16_MEMBER(raiden2cop_device::cop_dma_size_w)
{
	COMBINE_DATA(&cop_dma_size[cop_dma_mode]);
}

WRITE16_MEMBER(raiden2cop_device::cop_pal_brightness_val_w)
{
	COMBINE_DATA(&pal_brightness_val);
}

WRITE16_MEMBER(raiden2cop_device::cop_pal_brightness_mode_w)
{
	COMBINE_DATA(&pal_brightness_mode);
}

/* RE from Seibu Cup Soccer bootleg */
UINT8 raiden2cop_device::fade_table(int v)
{
	int low  = v & 0x001f;
	int high = v & 0x03e0;

	return (low * (high | (high >> 5)) + 0x210) >> 10;
}

WRITE16_MEMBER(raiden2cop_device::cop_dma_trigger_w)
{
#if 0
	if (cop_dma_mode != 0x14 && cop_dma_mode != 0x15)
	{
		printf("COP DMA mode=%x adr=%x size=%x vals=%x %x %x\n",
			cop_dma_mode,
			cop_dma_src[cop_dma_mode] << 6,
			cop_dma_size[cop_dma_mode] << 4,
			cop_dma_v1,
			cop_dma_v2,
			cop_dma_dst[cop_dma_mode]);
	}
#endif

	switch (cop_dma_mode)
	{
		case 0x14:
		{ 
			dma_tilemap_buffer();
			break;
		}
		
		case 0x15:
		{
			dma_palette_buffer();
			break;
		}

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		{ 	dma_palette_brightness();
			break;
		}
		
	/********************************************************************************************************************/
	case 0x09: {
		UINT32 src, dst, size;
		int i;

		src = (cop_dma_src[cop_dma_mode] << 6);
		dst = (cop_dma_dst[cop_dma_mode] << 6);
		size = ((cop_dma_size[cop_dma_mode] << 5) - (cop_dma_dst[cop_dma_mode] << 6) + 0x20) / 2;

		//      printf("%08x %08x %08x\n",src,dst,size);

		for (i = 0; i < size; i++)
		{
			m_host_space->write_word(dst, m_host_space->read_word(src));
			src += 2;
			dst += 2;
		}

		break;
	}
	/********************************************************************************************************************/
	case 0x0e:  // Godzilla / Seibu Cup Soccer
	{
		UINT32 src, dst, size, i;

		src = (cop_dma_src[cop_dma_mode] << 6);
		dst = (cop_dma_dst[cop_dma_mode] << 6);
		size = ((cop_dma_size[cop_dma_mode] << 5) - (cop_dma_dst[cop_dma_mode] << 6) + 0x20) / 2;

		for (i = 0; i < size; i++)
		{
			m_host_space->write_word(dst, m_host_space->read_word(src));
			src += 2;
			dst += 2;
		}

		break;
	}
	/********************************************************************************************************************/
	case 0x116: // Godzilla
	{
		UINT32 length, address;
		int i;

		//if(cop_dma_dst[cop_dma_mode] != 0x0000) // Invalid?
		//  return;

		address = (cop_dma_src[cop_dma_mode] << 6);
		length = ((cop_dma_size[cop_dma_mode] + 1) << 4);

		for (i = address; i < address + length; i += 4)
		{
			m_host_space->write_dword(i, (cop_dma_v1) | (cop_dma_v2 << 16));
		}
		break;
	}

		case 0x118:
		case 0x119:
		case 0x11a:
		case 0x11b:
		case 0x11c:
		case 0x11d:
		case 0x11e:
		case 0x11f: 
		{
			dma_fill();
			break;
		}
	}

}

/* Number Conversion */
void raiden2cop_device::bcd_update()
{
		//int digits = 1 << cop_itoa_mode*2;
	UINT32 val = cop_itoa;

	//if(digits > 9)
		int digits = 9;

	for (int i = 0; i < digits; i++)
	{
		if (!val && i)
		{
			// according to score display in  https://www.youtube.com/watch?v=T1M8sxYgt9A
			// we should return 0x30 for unused digits in Godzilla
			// however, Raiden II, Zero Team and SD Gundam all want 0x20 (SD Gundam even corrects 0x20 to 0x30 in the credits counter)
			// this is guesswork based on comparing M68000 code, using the most likely parameter that Godzilla configures differently
			cop_itoa_digits[i] = (cop_itoa_mode == 3) ? 0x30 : 0x20;
		}
		else
		{
			cop_itoa_digits[i] = 0x30 | (val % 10);
			val = val / 10;
		}
	}

	cop_itoa_digits[9] = 0;
}

WRITE16_MEMBER(raiden2cop_device::cop_itoa_low_w)
{
	cop_itoa = (cop_itoa & ~UINT32(mem_mask)) | (data & mem_mask);

	bcd_update();
}

WRITE16_MEMBER(raiden2cop_device::cop_itoa_high_w)
{
	cop_itoa = (cop_itoa & ~(mem_mask << 16)) | ((data & mem_mask) << 16);
	
	// Godzilla cares, otherwise you get 2p score overflow in 1p vs 2p, TODO: might actually be HW endianness dependant?
	bcd_update();
}

WRITE16_MEMBER(raiden2cop_device::cop_itoa_mode_w)
{
	// BCD / Number conversion related parameter
	// The former working hypothesis that this value is some sort of digit count is almost certainly incorrect
	// SD Gundam writes 0 here at startup, whereas Godzilla and Heated Barrel write 3
	// Denjin Makai writes either 2 or 3 at various times
	// Raiden II and Zero Team also write 2 here, but only in unused routines
	COMBINE_DATA(&cop_itoa_mode);
}

READ16_MEMBER(raiden2cop_device::cop_itoa_digits_r)
{
	return cop_itoa_digits[offset*2] | (cop_itoa_digits[offset*2+1] << 8);
}

READ16_MEMBER( raiden2cop_device::cop_status_r)
{
	return cop_status;
}

READ16_MEMBER( raiden2cop_device::cop_angle_r)
{
	return cop_angle;
}

READ16_MEMBER( raiden2cop_device::cop_dist_r)
{
	return cop_dist;
}

WRITE16_MEMBER( raiden2cop_device::cop_scale_w)
{
	COMBINE_DATA(&cop_scale);
	cop_scale &= 3;
}

WRITE16_MEMBER( raiden2cop_device::cop_angle_target_w)
{
	COMBINE_DATA(&cop_angle_target);
}

WRITE16_MEMBER( raiden2cop_device::cop_angle_step_w)
{
	COMBINE_DATA(&cop_angle_step);
}

READ16_MEMBER( raiden2cop_device::cop_reg_high_r)
{
	return cop_regs[offset] >> 16;
}

WRITE16_MEMBER( raiden2cop_device::cop_reg_high_w)
{
	cop_regs[offset] = (cop_regs[offset] & ~(mem_mask << 16)) | ((data & mem_mask) << 16);
}

READ16_MEMBER( raiden2cop_device::cop_reg_low_r)
{
	return cop_regs[offset];
}

WRITE16_MEMBER( raiden2cop_device::cop_reg_low_w)
{
	cop_regs[offset] = (cop_regs[offset] & ~UINT32(mem_mask)) | (data & mem_mask);
}

WRITE16_MEMBER( raiden2cop_device::cop_hitbox_baseadr_w)
{
	COMBINE_DATA(&cop_hit_baseadr);
}

void  raiden2cop_device::cop_collision_read_pos(int slot, UINT32 spradr, bool allow_swap)
{
	cop_collision_info[slot].allow_swap = allow_swap;
	cop_collision_info[slot].flags_swap = cop_read_word(spradr+2);
	cop_collision_info[slot].spradr = spradr;
	for(int i=0; i<3; i++)
		cop_collision_info[slot].pos[i] = cop_read_word(spradr+6+4*i);
}



/*
Godzilla 0x12c0 X = 0x21ed Y = 0x57da
Megaron  0x12d0 X = 0x1ef1 Y = 0x55db
King Ghidorah 0x12c8 X = 0x26eb Y = 0x55dc
Mecha Ghidorah 0x12dc X = 0x24ec Y = 0x55dc
Mecha Godzilla 0x12d4 X = 0x1cf1 Y = 0x52dc
Gigan 0x12cc X = 0x23e8 Y = 0x55db

(DC.W $1020, $F0C0, $0000, $0000)
X = collides at the same spot
Y = collides between 0xd0 and 0x20
0x588 bits 2 & 3 = 0
(DC.W $F0C0, $1020, $0000, $0000)
X = collides between 0xb0 and 0x50 (inclusive)
Y = collides between 0xd0 and 0x30 (not inclusive)
0x588 bits 2 & 3 = 0x580 bits 0 & 1
*/

void  raiden2cop_device::cop_collision_update_hitbox(UINT16 data, int slot, UINT32 hitadr)
{
	UINT32 hitadr2 = m_host_space->read_word(hitadr) | (cop_hit_baseadr << 16); // DON'T use cop_read_word here, doesn't need endian fixing?!
	int num_axis = 2;
	int extraxor = 0;
	if (m_host_endian) extraxor = 1;

	// guess, heatbrl doesn't have this set and clearly only wants 2 axis to be checked (otherwise it reads bad params into the 3rd)
	// everything else has it set, and legionna clearly wants 3 axis for jumping attacks to work
	if (data & 0x0100) num_axis = 3;

	int i;

	for(i=0; i<3; i++) {
		cop_collision_info[slot].dx[i] = 0;
		cop_collision_info[slot].size[i] = 0;
	}

	for(i=0; i<num_axis; i++) {
		cop_collision_info[slot].dx[i] = m_host_space->read_byte(extraxor^ (hitadr2++));
		cop_collision_info[slot].size[i] = m_host_space->read_byte(extraxor^ (hitadr2++));
	}

	INT16 dx[3],size[3];

	for (i = 0; i < num_axis; i++)
	{
		size[i] = UINT8(cop_collision_info[slot].size[i]);
		dx[i] = INT8(cop_collision_info[slot].dx[i]);
	}

	//printf("%02x %02x %02x %02x %02x %02x\n", (UINT8)size[i], (UINT8)dx[i], (UINT8)size[1], (UINT8)dx[1], (UINT8)size[2], (UINT8)dx[2]);

	int j = slot;

	UINT8 res;

	if (num_axis==3) res = 7;
	else res = 3;

	//for (j = 0; j < 2; j++)
	for (i = 0; i < num_axis;i++)
	{
		if (cop_collision_info[j].allow_swap && (cop_collision_info[j].flags_swap & (1 << i)))
		{
			cop_collision_info[j].max[i] = (cop_collision_info[j].pos[i]) - dx[i];
			cop_collision_info[j].min[i] = cop_collision_info[j].max[i] - size[i];
		}
		else
		{
			cop_collision_info[j].min[i] = (cop_collision_info[j].pos[i]) + dx[i];
			cop_collision_info[j].max[i] = cop_collision_info[j].min[i] + size[i];
		}

		if(cop_collision_info[0].max[i] > cop_collision_info[1].min[i] && cop_collision_info[0].min[i] < cop_collision_info[1].max[i])
			res &= ~(1 << i);

		if(cop_collision_info[1].max[i] > cop_collision_info[0].min[i] && cop_collision_info[1].min[i] < cop_collision_info[0].max[i])
			res &= ~(1 << i);

		cop_hit_val[i] = (cop_collision_info[0].pos[i] - cop_collision_info[1].pos[i]);
	}

	cop_hit_val_stat = res; // TODO: there's also bit 2 and 3 triggered in the tests, no known meaning
	cop_hit_status = res;
}


WRITE16_MEMBER( raiden2cop_device::cop_cmd_w)
{
	find_trigger_match(data, 0xf800);

	cop_status &= 0x7fff;

	switch(data) {
	case 0x0205: {  // 0205 0006 ffeb 0000 - 0188 0282 0082 0b8e 098e 0000 0000 0000
		execute_0205(offset, data); // angle from dx/dy
		break;
	}

	case 0x0904: /* X Se Dae and Zero Team uses this variant */
	case 0x0905: //  0905 0006 fbfb 0008 - 0194 0288 0088 0000 0000 0000 0000 0000
		execute_0904(offset, data);
		break;

	case 0x130e:   // 130e 0005 bf7f 0010 - 0984 0aa4 0d82 0aa2 039b 0b9a 0b9a 0a9a
	case 0x138e:
		execute_130e(offset, data); // angle from dx/dy
		break;

	case 0x338e: { // 338e 0005 bf7f 0030 - 0984 0aa4 0d82 0aa2 039c 0b9c 0b9c 0a9a
		execute_338e(offset, data); // angle from dx/dy
		break;
	}

	case 0x2208:
	case 0x2288: { // 2208 0005 f5df 0020 - 0f8a 0b8a 0388 0b9a 0b9a 0a9a 0000 0000
		execute_2288(offset, data); // angle from dx/dy
		break;
	}

	case 0x2a05: { // 2a05 0006 ebeb 0028 - 09af 0a82 0082 0a8f 018e 0000 0000 0000
		execute_2a05(offset, data);
		break;
	}

	case 0x39b0:
	case 0x3b30:
	case 0x3bb0: { // 3bb0 0004 007f 0038 - 0f9c 0b9c 0b9c 0b9c 0b9c 0b9c 0b9c 099c
		execute_3b30(offset, data);

		break;
	}

	case 0x42c2: { // 42c2 0005 fcdd 0040 - 0f9a 0b9a 0b9c 0b9c 0b9c 029c 0000 0000
		execute_42c2(offset, data); // DIVIDE
		break;
	}

	case 0x4aa0: { // 4aa0 0005 fcdd 0048 - 0f9a 0b9a 0b9c 0b9c 0b9c 099b 0000 0000
		execute_4aa0(offset, data); // DIVIDE
		break;
	}

	case 0x6200: {
		execute_6200(offset, data); // Target Angle calcs
		break;
	}

	case 0x8100: { // 8100 0007 fdfb 0080 - 0b9a 0b88 0888 0000 0000 0000 0000 0000
		execute_8100(offset, data); // SIN
		break;
	}

	case 0x8900: { // 8900 0007 fdfb 0088 - 0b9a 0b8a 088a 0000 0000 0000 0000 0000
		execute_8900(offset, data); // COS
		break;
	}

	case 0x5205:   // 5205 0006 fff7 0050 - 0180 02e0 03a0 00a0 03a0 0000 0000 0000
		//      fprintf(stderr, "sprcpt 5205 %04x %04x %04x %08x %08x\n", cop_regs[0], cop_regs[1], cop_regs[3], m_host_space.read_dword(cop_regs[0]), m_host_space.read_dword(cop_regs[3]));
		execute_5205(offset, data);
		break;

	case 0x5a05:   // 5a05 0006 fff7 0058 - 0180 02e0 03a0 00a0 03a0 0000 0000 0000
		//      fprintf(stderr, "sprcpt 5a05 %04x %04x %04x %08x %08x\n", cop_regs[0], cop_regs[1], cop_regs[3], m_host_space.read_dword(cop_regs[0]), m_host_space.read_dword(cop_regs[3]));
		execute_5a05(offset, data);

		break;

	case 0xf205:   // f205 0006 fff7 00f0 - 0182 02e0 03c0 00c0 03c0 0000 0000 0000
		//      fprintf(stderr, "sprcpt f205 %04x %04x %04x %08x %08x\n", cop_regs[0]+4, cop_regs[1], cop_regs[3], m_host_space.read_dword(cop_regs[0]+4), m_host_space.read_dword(cop_regs[3]));
		execute_f205(offset, data);
		break;

		// raidendx only
	case 0x7e05:
		execute_7e05(offset, data);
		break;

	case 0xa100:
	case 0xa180:
		execute_a100(offset, data); // collisions
		break;

	case 0xa900:
	case 0xa980:
		execute_a900(offset, data); // collisions
		break;

	case 0xb100: {
		execute_b100(offset, data);// collisions
		break;
	}

	case 0xb900: {
		execute_b900(offset, data); // collisions
		break;
	}

	default:
		logerror("pcall %04x [%x %x %x %x]\n", data, /*rps(), rpc(),*/ cop_regs[0], cop_regs[1], cop_regs[2], cop_regs[3]);
	}
}

READ16_MEMBER( raiden2cop_device::cop_collision_status_r)
{
	return cop_hit_status;
}


READ16_MEMBER( raiden2cop_device::cop_collision_status_val_r)
{
	return cop_hit_val[offset];
}

READ16_MEMBER( raiden2cop_device::cop_collision_status_stat_r)
{
	return cop_hit_val_stat;
}

WRITE16_MEMBER( raiden2cop_device::cop_sort_lookup_hi_w)
{
	cop_sort_lookup = (cop_sort_lookup&0x0000ffff)|(data<<16);
}

WRITE16_MEMBER( raiden2cop_device::cop_sort_lookup_lo_w)
{
	cop_sort_lookup = (cop_sort_lookup&0xffff0000)|(data&0xffff);
}

WRITE16_MEMBER( raiden2cop_device::cop_sort_ram_addr_hi_w)
{
	cop_sort_ram_addr = (cop_sort_ram_addr&0x0000ffff)|(data<<16);
}

WRITE16_MEMBER( raiden2cop_device::cop_sort_ram_addr_lo_w)
{
	cop_sort_ram_addr = (cop_sort_ram_addr&0xffff0000)|(data&0xffff);
}

WRITE16_MEMBER( raiden2cop_device::cop_sort_param_w)
{
	cop_sort_param = data;
}

WRITE16_MEMBER( raiden2cop_device::cop_sort_dma_trig_w)
{
	UINT16 sort_size;

	sort_size = data;

	//printf("%04x %04x %04x %04x\n",cop_sort_ram_addr,cop_sort_lookup,cop_sort_param,data);

	{
		int i,j;
		UINT8 xchg_flag;
		UINT32 addri,addrj;
		UINT16 vali,valj;

		// TODO: use a better algorithm than bubble sort!
		for(i=2;i<sort_size;i+=2)
		{
			for(j=i-2;j<sort_size;j+=2)
			{
				addri = cop_sort_ram_addr + m_host_space->read_word(cop_sort_lookup+i);
				addrj = cop_sort_ram_addr + m_host_space->read_word(cop_sort_lookup+j);

				vali = m_host_space->read_word(addri);
				valj = m_host_space->read_word(addrj);

				//printf("%08x %08x %04x %04x\n",addri,addrj,vali,valj);

				switch(cop_sort_param)
				{
					case 2: xchg_flag = (vali > valj); break;
					case 1: xchg_flag = (vali < valj); break;
					default: xchg_flag = 0; /* printf("Warning: sort-DMA used with param %02x\n",cop_sort_param); */ break;
				}

				if(xchg_flag)
				{
					UINT16 xch_val;

					xch_val = m_host_space->read_word(cop_sort_lookup+i);
					m_host_space->write_word(cop_sort_lookup+i,m_host_space->read_word(cop_sort_lookup+j));
					m_host_space->write_word(cop_sort_lookup+j,xch_val);
				}
			}
		}
	}
}

/* Random number generators (only verified on 68k games) */

READ16_MEMBER(raiden2cop_device::cop_prng_r)
{
	return m_host_cpu->total_cycles() % (m_cop_rng_max_value + 1);
}

/* max possible value returned by the RNG at 0x5a*, trusted */
WRITE16_MEMBER(raiden2cop_device::cop_prng_maxvalue_w)
{
	COMBINE_DATA(&m_cop_rng_max_value);
}

READ16_MEMBER(raiden2cop_device::cop_prng_maxvalue_r)
{
	return m_cop_rng_max_value;
}

// misc used by 68k games (mostly grainbow?)

WRITE16_MEMBER( raiden2cop_device::cop_sprite_dma_param_hi_w)
{
	m_cop_sprite_dma_param = (m_cop_sprite_dma_param&0x0000ffff)|(data<<16);
}

WRITE16_MEMBER( raiden2cop_device::cop_sprite_dma_param_lo_w)
{
	m_cop_sprite_dma_param = (m_cop_sprite_dma_param&0xffff0000)|(data&0xffff);
}

WRITE16_MEMBER(raiden2cop_device::cop_sprite_dma_size_w)
{
	m_cop_sprite_dma_size = data;
}


WRITE16_MEMBER( raiden2cop_device::cop_sprite_dma_src_hi_w)
{
	m_cop_sprite_dma_src = (m_cop_sprite_dma_src&0x0000ffff)|(data<<16);
}

WRITE16_MEMBER( raiden2cop_device::cop_sprite_dma_src_lo_w)
{
	m_cop_sprite_dma_src = (m_cop_sprite_dma_src&0xffff0000)|(data&0xffff);
}

// guess
WRITE16_MEMBER(raiden2cop_device::cop_sprite_dma_inc_w)
{
	if (data)
		printf("Warning: COP RAM 0x410 used with %04x\n", data);
	else
	{
		/* guess */
		cop_regs[4] += 8;
		m_cop_sprite_dma_src += 6;

		m_cop_sprite_dma_size--;

		if (m_cop_sprite_dma_size > 0)
			cop_status &= ~2;
		else
			cop_status |= 2;
	}
}

// more misc

// Involved with 0x9100/0x9180 and 0x9900/0x9980
// Some angle results seem to be stored here as well; legionna writes here when 0x138e raises the divide by zero flag
WRITE16_MEMBER( raiden2cop_device::cop_unk_param_a_w)
{
	COMBINE_DATA(&m_cop_unk_param_a);
}

// Involved with 0x9100/0x9180 and 0x9900/0x9980
WRITE16_MEMBER( raiden2cop_device::cop_unk_param_b_w)
{
	COMBINE_DATA(&m_cop_unk_param_b);
}

// cupsoc always writes 0xF before commands 0x5105, 0x5905, 0xD104 and 0xF105 and 0xE before 0xD104, then resets this to zero
// zeroteam writes 0xE here before 0xEDE5, then resets it to zero
WRITE16_MEMBER( raiden2cop_device::cop_precmd_w)
{
	COMBINE_DATA(&m_cop_precmd);
}

// cupsoc writes a longword before 0x5105 or 0xF105 (always 0xC000 for the latter)
WRITE16_MEMBER( raiden2cop_device::cop_rom_addr_lo_w)
{
	COMBINE_DATA(&m_cop_rom_addr_lo);
}

WRITE16_MEMBER( raiden2cop_device::cop_rom_addr_hi_w)
{
	COMBINE_DATA(&m_cop_rom_addr_hi);
}

WRITE16_MEMBER(raiden2cop_device::cop_sprite_dma_abs_y_w)
{
	m_cop_sprite_dma_abs_y = data;
}

WRITE16_MEMBER(raiden2cop_device::cop_sprite_dma_abs_x_w)
{
	m_cop_sprite_dma_abs_x = data;
}

/*------------------------------------------------------------------------------------------------------------------------------------------------*/
/* LEGACY CODE -----------------------------------------------------------------------------------------------------------------------------------*/
/* this is all old code that hasn't been refactored yet, it will go away                                                                          */
/*------------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------------------*/







WRITE16_MEMBER(raiden2cop_device::LEGACY_cop_cmd_w)
{
	int command;


	//seibu_cop_log("%06x: COPX execute table macro command %04x | regs %08x %08x %08x %08x %08x\n", m_host_space.device().safe_pc(), data,  cop_regs[0], cop_regs[1], cop_regs[2], cop_regs[3], cop_regs[4]);


	command = find_trigger_match(data, 0xf800);


	if (command == -1)
	{
		return;
	}
	UINT16 funcval, funcmask;
	// this is pointless.. all we use it for is comparing against the same value
	funcval = get_func_value(command);
	funcmask = get_func_mask(command);
	//printf("%04x %04x %04x\n",m_cop_mcu_ram[offset],funcval,funcmask);

	/*
	Macro notes:
	- endianess changes from/to Raiden 2:
	dword ^= 0
	word ^= 2
	byte ^= 3
	- some macro commands here have a commented algorithm, it's how Seibu Cup **BOOTLEG** version handles maths inside the 14/15 roms.
	The ROMs map tables in the following arrangement:
	0x00000 - 0x1ffff Sine math results
	0x20000 - 0x3ffff Cosine math results
	0x40000 - 0x7ffff Division math results
	0x80000 - 0xfffff Pythagorean theorem, hypotenuse length math results
	Surprisingly atan maths are nowhere to be found from the roms.
	*/

	int executed = 0;

	/* "automatic" movement, 0205 */
	if (check_command_matches(command, 0x188, 0x282, 0x082, 0xb8e, 0x98e, 0x000, 0x000, 0x000, 6, 0xffeb))
	{
		executed = 1;
		execute_0205(offset, data);
		return;
	}

	/* "automatic" movement, for arcs in Legionnaire / Zero Team (expression adjustment) 0905 */
	if (check_command_matches(command, 0x194, 0x288, 0x088, 0x000, 0x000, 0x000, 0x000, 0x000, 6, 0xfbfb))
	{
		executed = 1;
		execute_0904(offset, data);
		return;
	}

	/* SINE math - 0x8100 */
	/*
	        00000-0ffff:
	        amp = x/256
	        ang = x & 255
	        s = sin(ang*2*pi/256)
	        val = trunc(s*amp)
	        if(s<0)
	        val--
	        if(s == 192)
	        val = -2*amp
	        */
	if (check_command_matches(command, 0xb9a, 0xb88, 0x888, 0x000, 0x000, 0x000, 0x000, 0x000, 7, 0xfdfb))
	{
		executed = 1;
		execute_8100(offset, data); // SIN
		return;
	}

	/* COSINE math - 0x8900 */
	/*
	    10000-1ffff:
	    amp = x/256
	    ang = x & 255
	    s = cos(ang*2*pi/256)
	    val = trunc(s*amp)
	    if(s<0)
	    val--
	    if(s == 128)
	    val = -2*amp
	    */
	if (check_command_matches(command, 0xb9a, 0xb8a, 0x88a, 0x000, 0x000, 0x000, 0x000, 0x000, 7, 0xfdfb))
	{
		executed = 1;
		execute_8900(offset, data); // COS
		return;
	}

	/* 0x130e / 0x138e */
	if (check_command_matches(command, 0x984, 0xaa4, 0xd82, 0xaa2, 0x39b, 0xb9a, 0xb9a, 0xa9a, 5, 0xbf7f))
	{
		executed = 1;
		LEGACY_execute_130e_cupsoc(offset, data);
		return;
	}

	/* Pythagorean theorem, hypotenuse direction - 130e / 138e */
	//(heatbrl)  | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a b9a
	if (check_command_matches(command, 0x984, 0xaa4, 0xd82, 0xaa2, 0x39b, 0xb9a, 0xb9a, 0xb9a, 5, 0xbf7f))
	{
		executed = 1;
		LEGACY_execute_130e(offset, data);
		return;
	}

	/* Pythagorean theorem, hypotenuse length - 0x3bb0 */
	//(grainbow) | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
	/*
	    40000-7ffff:
	    v1 = (x / 32768)*64
	    v2 = (x & 255)*32767/255
	    val = sqrt(v1*v1+v2*v2) (unsigned)
	    */
	if (check_command_matches(command, 0xf9c, 0xb9c, 0xb9c, 0xb9c, 0xb9c, 0xb9c, 0xb9c, 0x99c, 4, 0x007f))
	{
		executed = 1;
		LEGACY_execute_3b30(offset, data);
		return;
	}

	/* Division - 0x42c2 */
	/*
	    20000-2ffff:
	    v1 = x / 1024
	    v2 = x & 1023
	    val = !v1 ? 32767 : trunc(v2/v1+0.5)
	    30000-3ffff:
	    v1 = x / 1024
	    v2 = (x & 1023)*32
	    val = !v1 ? 32767 : trunc(v2/v1+0.5)
	    */
	if (check_command_matches(command, 0xf9a, 0xb9a, 0xb9c, 0xb9c, 0xb9c, 0x29c, 0x000, 0x000, 5, 0xfcdd))
	{
		executed = 1;
		LEGACY_execute_42c2(offset, data);
		return;
	}

	/*
	    collision detection:

	    int dy_0 = m_host_space.read_dword(cop_regs[0]+4);
	    int dx_0 = m_host_space.read_dword(cop_regs[0]+8);
	    int dy_1 = m_host_space.read_dword(cop_regs[1]+4);
	    int dx_1 = m_host_space.read_dword(cop_regs[1]+8);
	    int hitbox_param1 = m_host_space.read_dword(cop_regs[2]);
	    int hitbox_param2 = m_host_space.read_dword(cop_regs[3]);

	    TODO: we are ignoring the funcval / funcmask params for now
	    */

	if (check_command_matches(command, 0xb80, 0xb82, 0xb84, 0xb86, 0x000, 0x000, 0x000, 0x000, funcval, funcmask))
	{
		execute_a100(offset, data);
		executed = 1;
		return;
	}

	//(heatbrl)  | 9 | ffff | b080 | b40 bc0 bc2
	if (check_command_matches(command, 0xb40, 0xbc0, 0xbc2, 0x000, 0x000, 0x000, 0x000, 0x000, funcval, funcmask))
	{
		executed = 1;
		execute_b100(offset, data);
		return;
	}

	if (check_command_matches(command, 0xba0, 0xba2, 0xba4, 0xba6, 0x000, 0x000, 0x000, 0x000, funcval, funcmask))
	{
		executed = 1;
		execute_a900(offset, data);
		return;
	}

	//(heatbrl)  | 6 | ffff | b880 | b60 be0 be2
	if (check_command_matches(command, 0xb60, 0xbe0, 0xbe2, 0x000, 0x000, 0x000, 0x000, 0x000, funcval, funcmask))
	{
		executed = 1;
		execute_b900(offset, data);
		return;
	}

	// grainbow 0d | a | fff3 | 6980 | b80 ba0
	if (check_command_matches(command, 0xb80, 0xba0, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 10, 0xfff3))
	{
		executed = 1;
		LEGACY_execute_6980(offset, data);
		return;
	}

	// grainbow 18 | a | ff00 | c480 | 080 882
	if (check_command_matches(command, 0x080, 0x882, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 10, 0xff00))
	{
		executed = 1;
		LEGACY_execute_c480(offset, data);
		return;
	}

	// cupsoc 1b | 5 | 7ff7 | dde5 | f80 aa2 984 0c2
	/* radar x/y positions */
	/* FIXME: x/ys are offsetted */
	/* FIXME: uses 0x10044a for something */
	if (check_command_matches(command, 0xf80, 0xaa2, 0x984, 0x0c2, 0x000, 0x000, 0x000, 0x000, 5, 0x7ff7))
	{
		executed = 1;
		LEGACY_execute_dde5(offset, data);
		return;
	}

	//(cupsoc)   | 8 | f3e7 | 6200 | 3a0 3a6 380 aa0 2a6
	if (check_command_matches(command, 0x3a0, 0x3a6, 0x380, 0xaa0, 0x2a6, 0x000, 0x000, 0x000, 8, 0xf3e7))
	{
		executed = 1;
		LEGACY_execute_6200(offset, data);
		return;
	}

	//(grainbow) | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
	/* search direction, used on SD Gundam homing weapon */
	/* FIXME: still doesn't work ... */
	if (check_command_matches(command, 0x380, 0x39a, 0x380, 0xa80, 0x29a, 0x000, 0x000, 0x000, 8, 0xf3e7))
	{
		executed = 1;
		execute_6200(offset, data);
		return;
	}

	//(cupsoc) 1c | 5 | b07f | e38e | 984 ac4 d82 ac2 39b b9a b9a a9a
	if (check_command_matches(command, 0x984, 0xac4, 0xd82, 0xac2, 0x39b, 0xb9a, 0xb9a, 0xa9a, 5, 0xb07f))
	{
		executed = 1;
		LEGACY_execute_e30e(offset, data);
		return;
	}

	//(cupsoc) 1a | 5 | fffb | d104 | ac2 9e0 0a2
	/* controls player vs. player collision detection, 0xf105 controls player vs. ball */
	if (check_command_matches(command, 0xac2, 0x9e0, 0x0a2, 0x000, 0x000, 0x000, 0x000, 0x000, 5, 0xfffb))
	{
		executed = 1;
		LEGACY_execute_d104(offset, data);
		return;
	}

	if (executed == 0)
	{
		if(data == 0xf105) // cupsoc transition from presentation to kick off
			return;
			
		printf("did not execute %04x\n", data); // cup soccer triggers this a lot (and others)
	}
}
