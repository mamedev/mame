/***************************************************************************

 Seibu Cop (Co-Processor) emulation
  (new implementation, based on Raiden 2 code)

  (should write new notes to replace those that were in seicop.c)

***************************************************************************/

#include "emu.h"
#include "raiden2cop.h"

// use Z to dump out table info
//#define TABLE_DUMPER

const device_type RAIDEN2COP = &device_creator<raiden2cop_device>;

raiden2cop_device::raiden2cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RAIDEN2COP, "RAIDEN2COP", tag, owner, clock, "raiden2cop", __FILE__),
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
	cop_itoa_digit_count(0),
	m_cop_itoa_unused_digit_value(0x30),

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

	m_cop_rom_addr_lo(0),
	m_cop_rom_addr_hi(0),
	m_cop_rom_addr_unk(0),

	m_cop_sprite_dma_abs_x(0),
	m_cop_sprite_dma_abs_y(0),

	m_LEGACY_r0(0),
	m_LEGACY_r1(0),

	m_cpu_is_68k(0),

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
	save_item(NAME(cop_itoa_digit_count));
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

	save_item(NAME(cop_collision_info[1].pos));
	save_item(NAME(cop_collision_info[1].dx));
	save_item(NAME(cop_collision_info[1].size));
	save_item(NAME(cop_collision_info[1].spradr));
	save_item(NAME(cop_collision_info[1].allow_swap));
	save_item(NAME(cop_collision_info[1].flags_swap));

	save_item(NAME(m_cop_rng_max_value));

	save_item(NAME(m_cop_sprite_dma_param));

	save_item(NAME(m_cop_sprite_dma_size));
	save_item(NAME(m_cop_sprite_dma_src));

	save_item(NAME(m_cop_rom_addr_lo));
	save_item(NAME(m_cop_rom_addr_hi));
	save_item(NAME(m_cop_rom_addr_unk));

	save_item(NAME(m_cop_sprite_dma_abs_x));
	save_item(NAME(m_cop_sprite_dma_abs_y));

	// legacy
	save_item(NAME(m_LEGACY_r0));
	save_item(NAME(m_LEGACY_r1));

	m_videoramout_cb.resolve_safe();

	cop_itoa_digit_count = 4; //TODO: Raiden 2 never inits the BCD register, value here is a guess (8 digits, as WR is 10.000.000 + a)

}

UINT16 raiden2cop_device::cop_read_word(address_space &space, int address)
{
	if (m_cpu_is_68k) return space.read_word(address ^ 2);
	else return space.read_word(address);
}

UINT8 raiden2cop_device::cop_read_byte(address_space &space, int address)
{
	if (m_cpu_is_68k) return space.read_byte(address ^ 3);
	else return space.read_byte(address);
}

void raiden2cop_device::cop_write_word(address_space &space, int address, UINT16 data)
{
	if (m_cpu_is_68k) space.write_word(address ^ 2, data);
	else space.write_word(address, data);
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

	printf("## - trig (masked) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask\n");

	for (command = 0; command < 0x20; command++)
	{
		if (cop_func_trigger[command] != 0x0000)
		{
			int maskout = 0x0080;

			printf("%02x - %04x (%04x  ) :  ", command, cop_func_trigger[command], cop_func_trigger[command] & ~maskout);

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

#define seibu_cop_log logerror
#define LOG_CMDS 1

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
		if ((triggerval & mask) == (cop_func_trigger[i] & mask))
		{
#if LOG_CMDS
			seibu_cop_log("    Cop Command %04x found in slot %02x with other params %04x %04x\n", triggerval, i, cop_func_value[i], cop_func_mask[i]);
#endif
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
		return -1;
	}

	printf("multiple matches found with mask passed in! (bad!)\n");
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
const UINT8 raiden2cop_device::fade_table(int v)
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
		/********************************************************************************************************************/
	case 0x14:
	{ // ALL games use this - tilemap DMA (RAM -> private buffer)
		{
			int src = cop_dma_src[cop_dma_mode] << 6;
			if (src == 0xcfc0) src = 0xd000; // R2, why?? everything else sets the right pointer (it also sets up odd size / dest regs, they probably counteract this)

			for (int i = 0; i < 0x2800 / 2; i++)
			{
				UINT16 tileval = space.read_word(src);
				src += 2;
				m_videoramout_cb(space, i, tileval, 0xffff);
			}

		}

		break;
	}
		/********************************************************************************************************************/
	case 0x15:
	{ // ALL games use this - palette DMA (RAM -> private buffer)
			int src = cop_dma_src[cop_dma_mode] << 6;

			for (int i = 0; i < 0x1000 / 2; i++) // todo, use length register
			{
				UINT16 palval = space.read_word(src);
				src += 2;
				m_palette->set_pen_color(i, pal5bit(palval >> 0), pal5bit(palval >> 5), pal5bit(palval >> 10));
			}

		break;
	}
		/********************************************************************************************************************/
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	{ // these are typically used to transfer palette data from one RAM buffer to another, applying fade values to it prior to the 0x15 transfer
		UINT32 src, dst, size, i;

		/*
		Apparently all of those are just different DMA channels, brightness effects are done through a RAM table and the pal_brightness_val / mode
		0x80 is used by Legionnaire
		0x81 is used by SD Gundam and Godzilla
		0x82 is used by Zero Team and X Se Dae
		0x86 is used by Seibu Cup Soccer
		0x87 is used by Denjin Makai

		TODO:
		- Denjin Makai mode 4 is totally guessworked.
		- SD Gundam doesn't fade colors correctly, it should have the text layer / sprites with normal gradient and the rest dimmed in most cases,
		presumably bad RAM table or bad algorithm
		*/

		//if(dma_trigger != 0x87)
		//printf("SRC: %08x %08x DST:%08x SIZE:%08x TRIGGER: %08x %02x %02x\n",cop_dma_src[cop_dma_mode] << 6,cop_dma_adr_rel * 0x400,cop_dma_dst[cop_dma_mode] << 6,cop_dma_size[cop_dma_mode] << 5,cop_dma_mode,pal_brightness_val,pal_brightness_mode);

		src = (cop_dma_src[cop_dma_mode] << 6);
		dst = (cop_dma_dst[cop_dma_mode] << 6);
		size = ((cop_dma_size[cop_dma_mode] << 5) - (cop_dma_dst[cop_dma_mode] << 6) + 0x20) / 2;

		for (i = 0; i < size; i++)
		{
			UINT16 pal_val;
			int r, g, b;
			int rt, gt, bt;

			if (pal_brightness_mode == 5)
			{
				bt = ((space.read_word(src + (cop_dma_adr_rel * 0x400))) & 0x7c00) >> 5;
				bt = fade_table(bt | (pal_brightness_val ^ 0));
				b = ((space.read_word(src)) & 0x7c00) >> 5;
				b = fade_table(b | (pal_brightness_val ^ 0x1f));
				pal_val = ((b + bt) & 0x1f) << 10;
				gt = ((space.read_word(src + (cop_dma_adr_rel * 0x400))) & 0x03e0);
				gt = fade_table(gt | (pal_brightness_val ^ 0));
				g = ((space.read_word(src)) & 0x03e0);
				g = fade_table(g | (pal_brightness_val ^ 0x1f));
				pal_val |= ((g + gt) & 0x1f) << 5;
				rt = ((space.read_word(src + (cop_dma_adr_rel * 0x400))) & 0x001f) << 5;
				rt = fade_table(rt | (pal_brightness_val ^ 0));
				r = ((space.read_word(src)) & 0x001f) << 5;
				r = fade_table(r | (pal_brightness_val ^ 0x1f));
				pal_val |= ((r + rt) & 0x1f);
			}
			else if (pal_brightness_mode == 4) //Denjin Makai
			{
				UINT16 targetpaldata = space.read_word(src + (cop_dma_adr_rel * 0x400));
				UINT16 paldata = space.read_word(src); // ^1 !!! (why?)

				bt = (targetpaldata & 0x7c00) >> 10;
				b = (paldata & 0x7c00) >> 10;
				gt = (targetpaldata & 0x03e0) >> 5;
				g = (paldata & 0x03e0) >> 5;
				rt = (targetpaldata & 0x001f) >> 0;
				r = (paldata & 0x001f) >> 0;

				if (pal_brightness_val == 0x10)
					pal_val = bt << 10 | gt << 5 | rt << 0;
				else if (pal_brightness_val == 0xff) // TODO: might be the back plane or it still doesn't do any mod, needs PCB tests
					pal_val = 0;
				else
				{
					bt = fade_table(bt << 5 | ((pal_brightness_val * 2) ^ 0));
					b = fade_table(b << 5 | ((pal_brightness_val * 2) ^ 0x1f));
					pal_val = ((b + bt) & 0x1f) << 10;
					gt = fade_table(gt << 5 | ((pal_brightness_val * 2) ^ 0));
					g = fade_table(g << 5 | ((pal_brightness_val * 2) ^ 0x1f));
					pal_val |= ((g + gt) & 0x1f) << 5;
					rt = fade_table(rt << 5 | ((pal_brightness_val * 2) ^ 0));
					r = fade_table(r << 5 | ((pal_brightness_val * 2) ^ 0x1f));
					pal_val |= ((r + rt) & 0x1f);
				}
			}
			else
			{
				printf("Warning: palette DMA used with mode %02x!\n", pal_brightness_mode);
				pal_val = space.read_word(src);
			}

			space.write_word(dst, pal_val);
			src += 2;
			dst += 2;
		}
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
			space.write_word(dst, space.read_word(src));
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
			space.write_word(dst, space.read_word(src));
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
			space.write_dword(i, (cop_dma_v1) | (cop_dma_v2 << 16));
		}
		break;
	}

	/********************************************************************************************************************/
	case 0x118:
	case 0x119:
	case 0x11a:
	case 0x11b:
	case 0x11c:
	case 0x11d:
	case 0x11e:
	case 0x11f: {
		UINT32 length, address;
		int i;
		if (cop_dma_dst[cop_dma_mode] != 0x0000) // Invalid?
			return;

		address = (cop_dma_src[cop_dma_mode] << 6);
		length = (cop_dma_size[cop_dma_mode] + 1) << 5;

		//printf("%08x %08x\n",address,length);

		for (i = address; i < address + length; i += 4)
		{
			space.write_dword(i, (cop_dma_v1) | (cop_dma_v2 << 16));
		}
		/*
		UINT32 length, address;
		int i;
		if(cop_dma_dst[cop_dma_mode] != 0x0000) // Invalid?
		return;

		address = (cop_dma_src[cop_dma_mode] << 6);
		length = (cop_dma_size[cop_dma_mode]+1) << 5;

		//printf("%08x %08x\n",address,length);

		for (i=address;i<address+length;i+=4)
		{
		space.write_dword(i, m_fill_val);
		}
		*/

		break;
	}


	}

}

/* Number Conversion */

// according to score display in  https://www.youtube.com/watch?v=T1M8sxYgt9A 
// we should return 0x30 for unused digits? according to Raiden 2 and Zero
// Team the value should be 0x20, can this be configured?
// grainbow doesn't like this implementation at all (21 credits, 2 digit high scores etc.)
WRITE16_MEMBER(raiden2cop_device::cop_itoa_low_w)
{
	cop_itoa = (cop_itoa & ~UINT32(mem_mask)) | (data & mem_mask);

	int digits = 1 << cop_itoa_digit_count*2;
	UINT32 val = cop_itoa;

	if(digits > 9)
		digits = 9;
	
	for (int i = 0; i < digits; i++)
	{
		if (!val && i)
		{
			cop_itoa_digits[i] = m_cop_itoa_unused_digit_value;
		}
		else
		{
			cop_itoa_digits[i] = 0x30 | (val % 10);
			val = val / 10;
		}
	}
	
	cop_itoa_digits[9] = 0;
}

WRITE16_MEMBER(raiden2cop_device::cop_itoa_high_w)
{
	cop_itoa = (cop_itoa & ~(mem_mask << 16)) | ((data & mem_mask) << 16);
}

WRITE16_MEMBER(raiden2cop_device::cop_itoa_digit_count_w)
{
	COMBINE_DATA(&cop_itoa_digit_count);
}

READ16_MEMBER(raiden2cop_device::cop_itoa_digits_r)
{
	return cop_itoa_digits[offset*2] | (cop_itoa_digits[offset*2+1] << 8);
}

/* Main COP functionality */

/*
(masked) just assumes 0x0080 masked out, actual masking / command selection is more complex, it seems bit 0x0001 is masked too (used for sign in 0904 / 0905) and more

## - trig (masked) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
00 - 0205 (0205  ) :  (188, 282, 082, b8e, 98e, 000, 000, 000)  6     ffeb   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
00 - 0105 (0105  ) :  (180, 2e0, 0a0, 000, 000, 000, 000, 000)  6     fffb   (zeroteamsr)
*/
void raiden2cop_device::execute_0205(address_space &space, int offset, UINT16 data)
{
	int ppos = space.read_dword(cop_regs[0] + 4 + offset * 4);
	int npos = ppos + space.read_dword(cop_regs[0] + 0x10 + offset * 4);
	int delta = (npos >> 16) - (ppos >> 16);
	space.write_dword(cop_regs[0] + 4 + offset * 4, npos);
	cop_write_word(space,cop_regs[0] + 0x1e + offset * 4, cop_read_word(space, cop_regs[0] + 0x1e + offset * 4) + delta);
}

/*
01 - 0905 (0905  ) :  (194, 288, 088, 000, 000, 000, 000, 000)  6     fbfb   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
01 - 0b05 (0b05  ) :  (180, 2e0, 0a0, 182, 2e0, 0c0, 000, 000)  6     ffdb   (zeroteamsr)
*/

// triggered with 0904 0905

void raiden2cop_device::execute_0904(address_space &space, int offset, UINT16 data)
{
	if (data&0x0001)
		space.write_dword(cop_regs[0] + 16 + offset * 4, space.read_dword(cop_regs[0] + 16 + offset * 4) + space.read_dword(cop_regs[0] + 0x28 + offset * 4));
	else /* X Se Dae and Zero Team uses this variant */
		space.write_dword(cop_regs[0] + 16 + offset * 4, space.read_dword(cop_regs[0] + 16 + offset * 4) - space.read_dword(cop_regs[0] + 0x28 + offset * 4));
}




/*
02 - 138e (130e  ) :  (984, aa4, d82, aa2, 39b, b9a, b9a, b9a)  5     bf7f   (heatbrl, legionna)
02 - 138e (130e  ) :  (984, aa4, d82, aa2, 39b, b9a, b9a, a9a)  5     bf7f   (cupsoc, godzilla, grainbow, denjinmk)
02 - 130e (130e  ) :  (984, aa4, d82, aa2, 39b, b9a, b9a, a9a)  5     bf7f   (raiden2, raidendx, zeroteam, xsedae)
*/

// triggered with 130e, 138e
void raiden2cop_device::execute_130e(address_space &space, int offset, UINT16 data)
{
	// this can't be right
	execute_338e(space, offset, data);
}

void raiden2cop_device::LEGACY_execute_130e(address_space &space, int offset, UINT16 data)
{
	int dy = space.read_dword(cop_regs[1] + 4) - space.read_dword(cop_regs[0] + 4);
	int dx = space.read_dword(cop_regs[1] + 8) - space.read_dword(cop_regs[0] + 8);

	cop_status = 7;
	if (!dx) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = atan(double(dy) / double(dx)) * 128.0 / M_PI;
		if (dx < 0)
			cop_angle += 0x80;
	}

	m_LEGACY_r0 = dy;
	m_LEGACY_r1 = dx;

	if (data & 0x80)
		space.write_word(cop_regs[0] + (0x34 ^ 2), cop_angle);
}

void raiden2cop_device::LEGACY_execute_130e_cupsoc(address_space &space, int offset, UINT16 data)
{
	int dy = space.read_dword(cop_regs[1] + 4) - space.read_dword(cop_regs[0] + 4);
	int dx = space.read_dword(cop_regs[1] + 8) - space.read_dword(cop_regs[0] + 8);

	cop_status = 7;
	if (!dx) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = atan(double(dy) / double(dx)) * 128.0 / M_PI;
		if (dx < 0)
			cop_angle += 0x80;
	}

	m_LEGACY_r0 = dy;
	m_LEGACY_r1 = dx;

	//printf("%d %d %f %04x\n",dx,dy,atan(double(dy)/double(dx)) * 128 / M_PI,cop_angle);

	if (data & 0x80)
		space.write_word(cop_regs[0] + (0x34 ^ 2), cop_angle);
}

/*
03 - 1905 (1905  ) :  (994, a88, 088, 000, 000, 000, 000, 000)  6     fbfb   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/

/*
04 - 2288 (2208  ) :  (f8a, b8a, 388, b9c, b9a, a9a, 000, 000)  5     f5df   (heatbrl, legionna)
04 - 2288 (2208  ) :  (f8a, b8a, 388, b9a, b9a, a9a, 000, 000)  5     f5df   (cupsoc, godzilla, grainbow, denjinmk)
04 - 2208 (2208  ) :  (f8a, b8a, 388, b9a, b9a, a9a, 000, 000)  5     f5df   (raiden2, raidendx, zeroteam, xsedae)
*/

// also triggered with 0x2208
void raiden2cop_device::execute_2288(address_space &space, int offset, UINT16 data)
{
	int dx = space.read_word(cop_regs[0] + 0x12);
	int dy = space.read_word(cop_regs[0] + 0x16);

	if (!dy) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = atan(double(dx) / double(dy)) * 128 / M_PI;
		if (dy < 0)
			cop_angle += 0x80;
	}

	if (data & 0x0080) {
		space.write_byte(cop_regs[0] + 0x34, cop_angle);
	}
}

/*
05 - 2a05 (2a05  ) :  (9af, a82, 082, a8f, 18e, 000, 000, 000)  6     ebeb   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/

void raiden2cop_device::execute_2a05(address_space &space, int offset, UINT16 data)
{
	int delta = space.read_word(cop_regs[1] + 0x1e + offset * 4);
	space.write_dword(cop_regs[0] + 4 + 2 + offset * 4, space.read_word(cop_regs[0] + 4 + 2 + offset * 4) + delta);
	space.write_dword(cop_regs[0] + 0x1e + offset * 4, space.read_word(cop_regs[0] + 0x1e + offset * 4) + delta);
}


/*
06 - 338e (330e  ) :  (984, aa4, d82, aa2, 39c, b9c, b9c, a9a)  5     bf7f   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx)
06 - 330e (330e  ) :  (984, aa4, d82, aa2, 39c, b9c, b9c, a9a)  5     bf7f   (zeroteam, xsedae)
*/
void raiden2cop_device::execute_338e(address_space &space, int offset, UINT16 data)
{
	int dx = space.read_dword(cop_regs[1] + 4) - space.read_dword(cop_regs[0] + 4);
	int dy = space.read_dword(cop_regs[1] + 8) - space.read_dword(cop_regs[0] + 8);

	if (!dy) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = atan(double(dx) / double(dy)) * 128 / M_PI;
		if (dy < 0)
			cop_angle += 0x80;
	}

	if (data & 0x0080) {
		space.write_byte(cop_regs[0] + 0x34, cop_angle);
	}
}

/*
07 - 3bb0 (3b30  ) :  (f9c, b9c, b9c, b9c, b9c, b9c, b9c, 99c)  4     007f   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx)
07 - 3b30 (3b30  ) :  (f9c, b9c, b9c, b9c, b9c, b9c, b9c, 99c)  4     007f   (zeroteam, xsedae)
*/

// triggered with 0x39b0, 0x3b30, 0x3bb0

void raiden2cop_device::execute_3b30(address_space &space, int offset, UINT16 data)
{
	/* TODO: these are actually internally loaded via 0x130e command */
	int dx, dy;

	dx = space.read_dword(cop_regs[1] + 4) - space.read_dword(cop_regs[0] + 4);
	dy = space.read_dword(cop_regs[1] + 8) - space.read_dword(cop_regs[0] + 8);

	dx = dx >> 16;
	dy = dy >> 16;
	cop_dist = sqrt((double)(dx*dx + dy*dy));

	if (data & 0x0080)
		space.write_word(cop_regs[0] + (data & 0x200 ? 0x3a : 0x38), cop_dist);
}

void raiden2cop_device::LEGACY_execute_3b30(address_space &space, int offset, UINT16 data)
{
	int dy = m_LEGACY_r0;
	int dx = m_LEGACY_r1;

	dx >>= 16;
	dy >>= 16;
	cop_dist = sqrt((double)(dx*dx + dy*dy));

	if (data & 0x80)
		space.write_word(cop_regs[0] + (0x38), cop_dist);
}

/*
08 - 42c2 (4242  ) :  (f9a, b9a, b9c, b9c, b9c, 29c, 000, 000)  5     fcdd   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/
void raiden2cop_device::execute_42c2(address_space &space, int offset, UINT16 data)
{
	int div = space.read_word(cop_regs[0] + (0x36));
	if (!div)
		div = 1;

	/* TODO: bits 5-6-15 */
	cop_status = 7;

	space.write_word(cop_regs[0] + (0x38), (cop_dist << (5 - cop_scale)) / div);
}

void raiden2cop_device::LEGACY_execute_42c2(address_space &space, int offset, UINT16 data)
{
	int dy = m_LEGACY_r0;
	int dx = m_LEGACY_r1;
	int div = space.read_word(cop_regs[0] + (0x36 ^ 2));
	int res;
	int cop_dist_raw;

	if (!div)
	{
		printf("divide by zero?\n");
		div = 1;
	}

	/* TODO: calculation of this one should occur at 0x3b30/0x3bb0 I *think* */
	/* TODO: recheck if cop_scale still masks at 3 with this command */
	dx >>= 11 + cop_scale;
	dy >>= 11 + cop_scale;
	cop_dist_raw = sqrt((double)(dx*dx + dy*dy));

	res = cop_dist_raw;
	res /= div;

	cop_dist = (1 << (5 - cop_scale)) / div;

	/* TODO: bits 5-6-15 */
	cop_status = 7;

	space.write_word(cop_regs[0] + (0x38 ^ 2), res);

}

/*
09 - 4aa0 (4a20  ) :  (f9a, b9a, b9c, b9c, b9c, 99b, 000, 000)  5     fcdd   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/
void raiden2cop_device::execute_4aa0(address_space &space, int offset, UINT16 data)
{
	int div = space.read_word(cop_regs[0] + (0x38));
	if (!div)
		div = 1;

	/* TODO: bits 5-6-15 */
	cop_status = 7;

	space.write_word(cop_regs[0] + (0x36), (cop_dist << (5 - cop_scale)) / div);
}

/*
0a - 5105 (5105  ) :  (a80, 984, 082, 000, 000, 000, 000, 000)  5     fefb   (cupsoc, grainbow)
0a - 5205 (5205  ) :  (180, 2e0, 3a0, 0a0, 3a0, 000, 000, 000)  6     fff7   (raiden2, raidendx)
0a - 5105 (5105  ) :  (180, 2e0, 0a0, 000, 000, 000, 000, 000)  6     fffb   (zeroteam, xsedae)
*/
void raiden2cop_device::execute_5205(address_space &space, int offset, UINT16 data)
{
	space.write_dword(cop_regs[1], space.read_dword(cop_regs[0]));
}
/*
0b - 5905 (5905  ) :  (9c8, a84, 0a2, 000, 000, 000, 000, 000)  5     fffb   (cupsoc, grainbow)
0b - 5a05 (5a05  ) :  (180, 2e0, 3a0, 0a0, 3a0, 000, 000, 000)  6     fff7   (raiden2, raidendx)
0b - 5a85 (5a05  ) :  (180, 2e0, 0a0, 182, 2e0, 0c0, 3c0, 3c0)  6     ffdb   (zeroteam, xsedae)
*/
void raiden2cop_device::execute_5a05(address_space &space, int offset, UINT16 data)
{
	space.write_dword(cop_regs[1], space.read_dword(cop_regs[0]));
}

/*
0c - 6200 (6200  ) :  (380, 39a, 380, a80, 29a, 000, 000, 000)  8     f3e7   (heatbrl, legionna, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
0c - 6200 (6200  ) :  (3a0, 3a6, 380, aa0, 2a6, 000, 000, 000)  8     f3e7   (cupsoc)

*/
void raiden2cop_device::execute_6200(address_space &space, int offset, UINT16 data)
{
	UINT8 angle = cop_read_byte(space, cop_regs[0] + 0x34);
	UINT16 flags = cop_read_word(space, cop_regs[0]);
	cop_angle_target &= 0xff;
	cop_angle_step &= 0xff;
	flags &= ~0x0004;
	int delta = angle - cop_angle_target;
	if (delta >= 128)
		delta -= 256;
	else if (delta < -128)
		delta += 256;
	if (delta < 0) {
		if (delta >= -cop_angle_step) {
			angle = cop_angle_target;
			flags |= 0x0004;
		}
		else
			angle += cop_angle_step;
	}
	else {
		if (delta <= cop_angle_step) {
			angle = cop_angle_target;
			flags |= 0x0004;
		}
		else
			angle -= cop_angle_step;
	}
	space.write_word(cop_regs[0], flags);
	space.write_byte(cop_regs[0] + 0x34, angle);
}


void raiden2cop_device::LEGACY_execute_6200_grainbow(address_space &space, int offset, UINT16 data)
{
	UINT8 cur_angle;
	UINT16 flags;

	cur_angle = cop_read_byte(space,cop_regs[0] + 0x34);
	flags = cop_read_word(space, cop_regs[0]);
	//space.write_byte(cop_regs[1] + (0^3),space.read_byte(cop_regs[1] + (0^3)) & 0xfb); //correct?

	INT8 tempangle_compare = (INT8)cop_angle_target;
	INT8 tempangle_mod_val = (INT8)cop_angle_step;

	tempangle_compare &= 0xff;
	tempangle_mod_val &= 0xff;

	cop_angle_target = tempangle_compare;
	cop_angle_step = tempangle_mod_val;


	flags &= ~0x0004;

	int delta = cur_angle - tempangle_compare;
	if (delta >= 128)
		delta -= 256;
	else if (delta < -128)
		delta += 256;
	if (delta < 0)
	{
		if (delta >= -tempangle_mod_val)
		{
			cur_angle = tempangle_compare;
			flags |= 0x0004;
		}
		else
			cur_angle += tempangle_mod_val;
	}
	else
	{
		if (delta <= tempangle_mod_val)
		{
			cur_angle = tempangle_compare;
			flags |= 0x0004;
		}
		else
			cur_angle -= tempangle_mod_val;
	}

	space.write_byte(cop_regs[0] + (0 ^ 3), flags); // this is a word in the avoce
	space.write_word(cop_regs[0] + (0x34 ^ 3), cur_angle); // why ^3 on a word? should it be a byte, it is in the above
}


void raiden2cop_device::LEGACY_execute_6200(address_space &space, int offset, UINT16 data) // this is for cupsoc, different sequence, works on different registers
{
	UINT8 cur_angle;
	UINT16 flags;

	/* 0 [1] */
	/* 0xc [1] */
	/* 0 [0] */
	/* 0 [1] */
	/* 0xc [1] */

	cur_angle = space.read_byte(cop_regs[1] + (0xc ^ 3));
	flags = space.read_word(cop_regs[1]);
	//space.write_byte(cop_regs[1] + (0^3),space.read_byte(cop_regs[1] + (0^3)) & 0xfb); //correct?

	INT8 tempangle_compare = (INT8)cop_angle_target;
	INT8 tempangle_mod_val = (INT8)cop_angle_step;

	tempangle_compare &= 0xff;
	tempangle_mod_val &= 0xff;

	cop_angle_target = tempangle_compare;
	cop_angle_step = tempangle_mod_val;

	flags &= ~0x0004;

	int delta = cur_angle - tempangle_compare;
	if (delta >= 128)
		delta -= 256;
	else if (delta < -128)
		delta += 256;
	if (delta < 0)
	{
		if (delta >= -tempangle_mod_val)
		{
			cur_angle = tempangle_compare;
			flags |= 0x0004;
		}
		else
			cur_angle += tempangle_mod_val;
	}
	else
	{
		if (delta <= tempangle_mod_val)
		{
			cur_angle = tempangle_compare;
			flags |= 0x0004;
		}
		else
			cur_angle -= tempangle_mod_val;
	}

	space.write_byte(cop_regs[1] + (0 ^ 2), flags);
	space.write_byte(cop_regs[1] + (0xc ^ 3), cur_angle);
}

/*

0d - 6880 (6800  ) :  (b80, ba0, 000, 000, 000, 000, 000, 000)  a     fff3   (heatbrl, legionna, cupsoc, godzilla, denjinmk)
0d - 6980 (6900  ) :  (b80, ba0, 000, 000, 000, 000, 000, 000)  a     fff3   (grainbow, zeroteam, xsedae)
*/
void raiden2cop_device::LEGACY_execute_6980(address_space &space, int offset, UINT16 data)
{
	UINT8 offs;
	int abs_x, abs_y, rel_xy;

	offs = (offset & 3) * 4;

	/* TODO: I really suspect that following two are actually taken from the 0xa180 macro command then internally loaded */
	abs_x = space.read_word(cop_regs[0] + 8) - m_cop_sprite_dma_abs_x;
	abs_y = space.read_word(cop_regs[0] + 4) - m_cop_sprite_dma_abs_y;
	rel_xy = space.read_word(m_cop_sprite_dma_src + 4 + offs);

	//if(rel_xy & 0x0706)
	//  printf("sprite rel_xy = %04x\n",rel_xy);

	if (rel_xy & 1)
		space.write_word(cop_regs[4] + offs + 4, 0xc0 + abs_x - (rel_xy & 0xf8));
	else
		space.write_word(cop_regs[4] + offs + 4, (((rel_xy & 0x78) + (abs_x)-((rel_xy & 0x80) ? 0x80 : 0))));

	space.write_word(cop_regs[4] + offs + 6, (((rel_xy & 0x7800) >> 8) + (abs_y)-((rel_xy & 0x8000) ? 0x80 : 0)));
}

/*

0e - 7100 (7100  ) :  (b80, a80, b80, 000, 000, 000, 000, 000)  8     fdfd   (zeroteam, xsedae)

0f - 7905 (7905  ) :  (1a2, 2c2, 0a2, 000, 000, 000, 000, 000)  6     fffb   (cupsoc, grainbow)
0f - 7e05 (7e05  ) :  (180, 282, 080, 180, 282, 000, 000, 000)  6     fffb   (raidendx)
*/

void raiden2cop_device::execute_7e05(address_space &space, int offset, UINT16 data) // raidendx
{
	space.write_byte(0x470, space.read_byte(cop_regs[4]));
}

/*
10 - 8100 (8100  ) :  (b9a, b88, 888, 000, 000, 000, 000, 000)  7     fdfb   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/

void raiden2cop_device::execute_8100(address_space &space, int offset, UINT16 data)
{
	int raw_angle = (cop_read_word(space, cop_regs[0] + (0x34)) & 0xff);
	double angle = raw_angle * M_PI / 128;
	double amp = (65536 >> 5)*(cop_read_word(space, cop_regs[0] + (0x36)) & 0xff);
	int res;
	/* TODO: up direction needs double, why? */
	if (raw_angle == 0xc0)
		amp *= 2;
	res = int(amp*sin(angle)) << cop_scale;
	space.write_dword(cop_regs[0] + 16, res);
}

/*
11 - 8900 (8900  ) :  (b9a, b8a, 88a, 000, 000, 000, 000, 000)  7     fdfb   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/
void raiden2cop_device::execute_8900(address_space &space, int offset, UINT16 data)
{
	int raw_angle = (cop_read_word(space, cop_regs[0] + (0x34)) & 0xff);
	double angle = raw_angle * M_PI / 128;
	double amp = (65536 >> 5)*(cop_read_word(space, cop_regs[0] + (0x36)) & 0xff);
	int res;
	/* TODO: up direction needs double, why? */
	if (raw_angle == 0x80)
		amp *= 2;
	res = int(amp*cos(angle)) << cop_scale;
	space.write_dword(cop_regs[0] + 20, res);
}

/*
12 - 9180 (9100  ) :  (b80, b94, b94, 894, 000, 000, 000, 000)  7     f8f7   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk)
12 - 9100 (9100  ) :  (b80, b94, 894, 000, 000, 000, 000, 000)  7     fefb   (raiden2, raidendx)
12 - 9100 (9100  ) :  (b80, b94, b94, 894, 000, 000, 000, 000)  7     f8f7   (zeroteam, xsedae)

13 - 9980 (9900  ) :  (b80, b96, b96, 896, 000, 000, 000, 000)  7     f8f7   (heatbrl, legionna)
13 - 9980 (9900  ) :  (b80, b94, b94, 896, 000, 000, 000, 000)  7     f8f7   (cupsoc, godzilla, grainbow, denjinmk)
13 - 9900 (9900  ) :  (b80, b94, 896, 000, 000, 000, 000, 000)  7     fefb   (raiden2, raidendx)
13 - 9900 (9900  ) :  (b80, b94, b94, 896, 000, 000, 000, 000)  7     f8f7   (zeroteam, xsedae)

// x

*/

/*
14 - a100 (a100  ) :  (b80, b82, b84, b86, 000, 000, 000, 000)  0     ffff   (heatbrl, zeroteam, xsedae)
14 - a180 (a100  ) :  (b80, b82, b84, b86, 000, 000, 000, 000)  0     ffff   (legionna, cupsoc, godzilla, denjinmk)
14 - a180 (a100  ) :  (b80, b82, b84, b86, 000, 000, 000, 000)  0     02ff   (grainbow)
14 - a100 (a100  ) :  (b80, b82, b84, b86, 000, 000, 000, 000)  0     00ff   (raiden2, raidendx)
*/
void raiden2cop_device::execute_a100(address_space &space, int offset, UINT16 data)
{
	cop_collision_read_pos(space, 0, cop_regs[0], data & 0x0080);
}

void raiden2cop_device::LEGACY_execute_a100(address_space &space, int offset, UINT16 data)
{
	m_LEGACY_cop_collision_info[0].y = (space.read_dword(cop_regs[0] + 4));
	m_LEGACY_cop_collision_info[0].x = (space.read_dword(cop_regs[0] + 8));
}

/*
15 - a900 (a900  ) :  (ba0, ba2, ba4, ba6, 000, 000, 000, 000)  f     ffff   (heatbrl, zeroteam, xsedae)
15 - a980 (a900  ) :  (ba0, ba2, ba4, ba6, 000, 000, 000, 000)  f     ffff   (legionna, cupsoc, godzilla, denjinmk)
15 - a980 (a900  ) :  (ba0, ba2, ba4, ba6, 000, 000, 000, 000)  f     02ff   (grainbow)
15 - a900 (a900  ) :  (ba0, ba2, ba4, ba6, 000, 000, 000, 000)  f     00ff   (raiden2, raidendx)
*/
void raiden2cop_device::execute_a900(address_space &space, int offset, UINT16 data)
{
	cop_collision_read_pos(space, 1, cop_regs[1], data & 0x0080);
}

void raiden2cop_device::LEGACY_execute_a900(address_space &space, int offset, UINT16 data)
{
	m_LEGACY_cop_collision_info[1].y = (space.read_dword(cop_regs[1] + 4));
	m_LEGACY_cop_collision_info[1].x = (space.read_dword(cop_regs[1] + 8));
}

/*
16 - b080 (b000  ) :  (b40, bc0, bc2, 000, 000, 000, 000, 000)  9     ffff   (heatbrl)
16 - b100 (b100  ) :  (b40, bc0, bc2, 000, 000, 000, 000, 000)  9     ffff   (legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/
void raiden2cop_device::execute_b100(address_space &space, int offset, UINT16 data)
{
	cop_collision_update_hitbox(space, 0, cop_regs[2]);
}

void raiden2cop_device::LEGACY_execute_b100(address_space &space, int offset, UINT16 data)
{
	m_LEGACY_cop_collision_info[0].hitbox = space.read_word(cop_regs[2]);
	m_LEGACY_cop_collision_info[0].hitbox_y = space.read_word((cop_regs[2] & 0xffff0000) | (m_LEGACY_cop_collision_info[0].hitbox));
	m_LEGACY_cop_collision_info[0].hitbox_x = space.read_word(((cop_regs[2] & 0xffff0000) | (m_LEGACY_cop_collision_info[0].hitbox)) + 2);

	/* do the math */
	LEGACY_cop_take_hit_box_params(0);
	cop_hit_status = LEGACY_cop_calculate_collsion_detection();
}

/*
17 - b880 (b800  ) :  (b60, be0, be2, 000, 000, 000, 000, 000)  6     ffff   (heatbrl)
17 - b900 (b900  ) :  (b60, be0, be2, 000, 000, 000, 000, 000)  6     ffff   (legionna, cupsoc, godzilla, grainbow, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/
void raiden2cop_device::execute_b900(address_space &space, int offset, UINT16 data)
{
	cop_collision_update_hitbox(space, 1, cop_regs[3]);
}

void raiden2cop_device::LEGACY_execute_b900(address_space &space, int offset, UINT16 data)
{
	m_LEGACY_cop_collision_info[1].hitbox = space.read_word(cop_regs[3]);
	m_LEGACY_cop_collision_info[1].hitbox_y = space.read_word((cop_regs[3] & 0xffff0000) | (m_LEGACY_cop_collision_info[1].hitbox));
	m_LEGACY_cop_collision_info[1].hitbox_x = space.read_word(((cop_regs[3] & 0xffff0000) | (m_LEGACY_cop_collision_info[1].hitbox)) + 2);

	/* do the math */
	LEGACY_cop_take_hit_box_params(1);
	cop_hit_status = LEGACY_cop_calculate_collsion_detection();
}

/*
18 - c480 (c400  ) :  (080, 882, 000, 000, 000, 000, 000, 000)  a     ff00   (heatbrl, legionna, cupsoc, godzilla, grainbow, denjinmk)
18 - 7c80 (7c00  ) :  (080, 882, 000, 000, 000, 000, 000, 000)  a     ff00   (zeroteam, xsedae)
*/

void raiden2cop_device::LEGACY_execute_c480(address_space &space, int offset, UINT16 data)
{
	UINT8 offs;

	offs = (offset & 3) * 4;

	space.write_word(cop_regs[4] + offs + 0, space.read_word(m_cop_sprite_dma_src + offs) + (m_cop_sprite_dma_param & 0x3f));
	//space.write_word(cop_regs[4] + offs + 2,space.read_word(m_cop_sprite_dma_src+2 + offs));

}

/*
19 - cb8f (cb0f  ) :  (984, aa4, d82, aa2, 39b, b9a, b9a, a9f)  5     bf7f   (cupsoc, grainbow)

1a - d104 (d104  ) :  (ac2, 9e0, 0a2, 000, 000, 000, 000, 000)  5     fffb   (cupsoc, grainbow)
*/
void raiden2cop_device::LEGACY_execute_d104(address_space &space, int offset, UINT16 data)
{
	UINT8 *ROM = space.machine().root_device().memregion("maincpu")->base();
	UINT32 rom_addr = (m_cop_rom_addr_hi << 16 | m_cop_rom_addr_lo) & ~1;
	UINT16 rom_data = (ROM[rom_addr + 0]) | (ROM[rom_addr + 1] << 8);

	/* writes to some unemulated COP registers, then puts the result in here, adding a parameter taken from ROM */
	//space.write_word(cop_regs[0]+(0x44 + offset * 4), rom_data);

	logerror("%04x%04x %04x %04x\n", m_cop_rom_addr_hi, m_cop_rom_addr_lo, m_cop_rom_addr_unk, rom_data);
}
/*
1b - dde5 (dd65  ) :  (f80, aa2, 984, 0c2, 000, 000, 000, 000)  5     7ff7   (cupsoc, grainbow)
*/

void raiden2cop_device::LEGACY_execute_dde5(address_space &space, int offset, UINT16 data)
{
	UINT8 offs;
	int div;
	INT16 dir_offset;
	//              INT16 offs_val;

	/* TODO: [4-7] could be mirrors of [0-3] (this is the only command so far that uses 4-7 actually)*/
	/* ? 0 + [4] */
	/* sub32 4 + [5] */
	/* write16h 8 + [4] */
	/* addmem16 4 + [6] */

	// these two are obvious ...
	// 0xf x 16 = 240
	// 0x14 x 16 = 320
	// what are these two instead? scale factor? offsets? (edit: offsets to apply from the initial sprite data)
	// 0xfc69 ?
	// 0x7f4 ?
	//printf("%08x %08x %08x %08x %08x %08x %08x\n",cop_regs[0],cop_regs[1],cop_regs[2],cop_regs[3],cop_regs[4],cop_regs[5],cop_regs[6]);

	offs = (offset & 3) * 4;

	div = space.read_word(cop_regs[4] + offs);
	dir_offset = space.read_word(cop_regs[4] + offs + 8);
	//              offs_val = space.read_word(cop_regs[3] + offs);
	//420 / 180 = 500 : 400 = 30 / 50 = 98 / 18

	/* TODO: this probably trips a cop status flag */
	if (div == 0) { div = 1; }


	space.write_word((cop_regs[6] + offs + 4), ((space.read_word(cop_regs[5] + offs + 4) + dir_offset) / div));
}

/*
1c - e38e (e30e  ) :  (984, ac4, d82, ac2, 39b, b9a, b9a, a9a)  5     b07f   (cupsoc, grainbow)
1c - e105 (e105  ) :  (a88, 994, 088, 000, 000, 000, 000, 000)  5     06fb   (zeroteam, xsedae)
*/

void raiden2cop_device::LEGACY_execute_e30e(address_space &space, int offset, UINT16 data)
{
	int dy = space.read_dword(cop_regs[2] + 4) - space.read_dword(cop_regs[0] + 4);
	int dx = space.read_dword(cop_regs[2] + 8) - space.read_dword(cop_regs[0] + 8);

	cop_status = 7;
	if (!dx) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = atan(double(dy) / double(dx)) * 128.0 / M_PI;
		if (dx < 0)
			cop_angle += 0x80;
	}

	m_LEGACY_r0 = dy;
	m_LEGACY_r1 = dx;

	//printf("%d %d %f %04x\n",dx,dy,atan(double(dy)/double(dx)) * 128 / M_PI,cop_angle);

	if (data & 0x80)
		space.write_word(cop_regs[0] + (0x34 ^ 2), cop_angle);
}

/*
1d - eb8e (eb0e  ) :  (984, ac4, d82, ac2, 39b, b9a, b9a, a9f)  5     b07f   (cupsoc, grainbow)
1d - ede5 (ed65  ) :  (f88, a84, 986, 08a, 000, 000, 000, 000)  5     05f7   (zeroteam, xsedae)

1e - f105 (f105  ) :  (a88, 994, 088, 000, 000, 000, 000, 000)  5     fefb   (cupsoc, grainbow)
1e - f205 (f205  ) :  (182, 2e0, 3c0, 0c0, 3c0, 000, 000, 000)  6     fff7   (raiden2, raidendx)
1e - f790 (f710  ) :  (f80, b84, b84, b84, b84, b84, b84, b84)  4     00ff   (zeroteam, xsedae)
*/

void raiden2cop_device::execute_f205(address_space &space, int offset, UINT16 data)
{
	space.write_dword(cop_regs[2], space.read_dword(cop_regs[0]+4));
}

/*
1f - fc84 (fc04  ) :  (182, 280, 000, 000, 000, 000, 000, 000)  6     00ff   (zeroteam, xsedae)
*/

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

void  raiden2cop_device::cop_collision_read_pos(address_space &space, int slot, UINT32 spradr, bool allow_swap)
{
	cop_collision_info[slot].allow_swap = allow_swap;
	cop_collision_info[slot].flags_swap = space.read_word(spradr+2);
	cop_collision_info[slot].spradr = spradr;
	for(int i=0; i<3; i++)
		cop_collision_info[slot].pos[i] = space.read_word(spradr+6+4*i);
}

void  raiden2cop_device::cop_collision_update_hitbox(address_space &space, int slot, UINT32 hitadr)
{
	UINT32 hitadr2 = space.read_word(hitadr) | (cop_hit_baseadr << 16);

	for(int i=0; i<3; i++) {
		cop_collision_info[slot].dx[i] = space.read_byte(hitadr2++);
		cop_collision_info[slot].size[i] = space.read_byte(hitadr2++);
	}

	cop_hit_status = 7;

	for(int i=0; i<3; i++) {
		int min[2], max[2];
		for(int j=0; j<2; j++) {
			if(cop_collision_info[j].allow_swap && (cop_collision_info[j].flags_swap & (1 << i))) {
				max[j] = cop_collision_info[j].pos[i] - cop_collision_info[j].dx[i];
				min[j] = max[j] - cop_collision_info[j].size[i];
			} else {
				min[j] = cop_collision_info[j].pos[i] + cop_collision_info[j].dx[i];
				max[j] = min[j] + cop_collision_info[j].size[i];
			}
		}
		if(max[0] > min[1] && min[0] < max[1])
			cop_hit_status &= ~(1 << i);
		cop_hit_val[i] = cop_collision_info[0].pos[i] - cop_collision_info[1].pos[i];
	}

	cop_hit_val_stat = cop_hit_status ? 0xffff : 0x0000;
}

WRITE16_MEMBER( raiden2cop_device::cop_cmd_w)
{
	cop_status &= 0x7fff;

	switch(data) {
	case 0x0205: {  // 0205 0006 ffeb 0000 - 0188 0282 0082 0b8e 098e 0000 0000 0000
		execute_0205(space, offset, data); // angle from dx/dy
		break;
	}

	case 0x0904: /* X Se Dae and Zero Team uses this variant */
	case 0x0905: //  0905 0006 fbfb 0008 - 0194 0288 0088 0000 0000 0000 0000 0000
		execute_0904(space, offset, data);
		break;

	case 0x130e:   // 130e 0005 bf7f 0010 - 0984 0aa4 0d82 0aa2 039b 0b9a 0b9a 0a9a
	case 0x138e:
		execute_130e(space, offset, data); // angle from dx/dy
		break;

	case 0x338e: { // 338e 0005 bf7f 0030 - 0984 0aa4 0d82 0aa2 039c 0b9c 0b9c 0a9a
		execute_338e(space, offset, data); // angle from dx/dy
		break;
	}

	case 0x2208:
	case 0x2288: { // 2208 0005 f5df 0020 - 0f8a 0b8a 0388 0b9a 0b9a 0a9a 0000 0000
		execute_2288(space, offset, data); // angle from dx/dy
		break;
	}

	case 0x2a05: { // 2a05 0006 ebeb 0028 - 09af 0a82 0082 0a8f 018e 0000 0000 0000
		execute_2a05(space, offset, data);
		break;
	}

	case 0x39b0:
	case 0x3b30:
	case 0x3bb0: { // 3bb0 0004 007f 0038 - 0f9c 0b9c 0b9c 0b9c 0b9c 0b9c 0b9c 099c
		execute_3b30(space, offset, data); 

		break;
	}

	case 0x42c2: { // 42c2 0005 fcdd 0040 - 0f9a 0b9a 0b9c 0b9c 0b9c 029c 0000 0000
		execute_42c2(space, offset, data); // DIVIDE
		break;
	}

	case 0x4aa0: { // 4aa0 0005 fcdd 0048 - 0f9a 0b9a 0b9c 0b9c 0b9c 099b 0000 0000
		execute_4aa0(space, offset, data); // DIVIDE
		break;
	}

	case 0x6200: {
		execute_6200(space, offset, data); // Target Angle calcs
		break;
	}

	case 0x8100: { // 8100 0007 fdfb 0080 - 0b9a 0b88 0888 0000 0000 0000 0000 0000
		execute_8100(space, offset, data); // SIN
		break;
	}

	case 0x8900: { // 8900 0007 fdfb 0088 - 0b9a 0b8a 088a 0000 0000 0000 0000 0000
		execute_8900(space, offset, data); // COS
		break;
	}

	case 0x5205:   // 5205 0006 fff7 0050 - 0180 02e0 03a0 00a0 03a0 0000 0000 0000
		//      fprintf(stderr, "sprcpt 5205 %04x %04x %04x %08x %08x\n", cop_regs[0], cop_regs[1], cop_regs[3], space.read_dword(cop_regs[0]), space.read_dword(cop_regs[3]));
		execute_5205(space, offset, data);
		break;

	case 0x5a05:   // 5a05 0006 fff7 0058 - 0180 02e0 03a0 00a0 03a0 0000 0000 0000
		//      fprintf(stderr, "sprcpt 5a05 %04x %04x %04x %08x %08x\n", cop_regs[0], cop_regs[1], cop_regs[3], space.read_dword(cop_regs[0]), space.read_dword(cop_regs[3]));
		execute_5a05(space, offset, data);
	
		break;

	case 0xf205:   // f205 0006 fff7 00f0 - 0182 02e0 03c0 00c0 03c0 0000 0000 0000
		//      fprintf(stderr, "sprcpt f205 %04x %04x %04x %08x %08x\n", cop_regs[0]+4, cop_regs[1], cop_regs[3], space.read_dword(cop_regs[0]+4), space.read_dword(cop_regs[3]));
		execute_f205(space, offset, data);
		break;

		// raidendx only
	case 0x7e05:
		execute_7e05(space, offset, data);
		break;

	case 0xa100:
	case 0xa180:
		execute_a100(space, offset, data); // collisions
		break;

	case 0xa900:
	case 0xa980:
		execute_a900(space, offset, data); // collisions
		break;

	case 0xb100: {
		execute_b100(space, offset, data); // collisions
		break;
	}

	case 0xb900: {
		execute_b900(space, offset, data); // collisions
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

		/* TODO: use a better algorithm than bubble sort! */
		for(i=2;i<sort_size;i+=2)
		{
			for(j=i-2;j<sort_size;j+=2)
			{
				addri = cop_sort_ram_addr+space.read_word(cop_sort_lookup+i);
				addrj = cop_sort_ram_addr+space.read_word(cop_sort_lookup+j);

				vali = space.read_word(addri);
				valj = space.read_word(addrj);

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

					xch_val = space.read_word(cop_sort_lookup+i);
					space.write_word(cop_sort_lookup+i,space.read_word(cop_sort_lookup+j));
					space.write_word(cop_sort_lookup+j,xch_val);
				}
			}
		}
	}
}

/* Random number generators (only verified on 68k games) */

READ16_MEMBER(raiden2cop_device::cop_prng_r)
{
	return space.machine().firstcpu->total_cycles() % (m_cop_rng_max_value + 1);
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

WRITE16_MEMBER( raiden2cop_device::cop_rom_addr_unk_w)
{
	COMBINE_DATA(&m_cop_rom_addr_unk);
}

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
void raiden2cop_device::LEGACY_cop_take_hit_box_params(UINT8 offs)
{
	INT16 start_x,start_y,height,width;

	{
		height = UINT8(m_LEGACY_cop_collision_info[offs].hitbox_y >> 8);
		start_y = INT8(m_LEGACY_cop_collision_info[offs].hitbox_y);
		width = UINT8(m_LEGACY_cop_collision_info[offs].hitbox_x >> 8);
		start_x = INT8(m_LEGACY_cop_collision_info[offs].hitbox_x);
	}

	m_LEGACY_cop_collision_info[offs].min_x = (m_LEGACY_cop_collision_info[offs].x >> 16) + start_x;
	m_LEGACY_cop_collision_info[offs].max_x = m_LEGACY_cop_collision_info[offs].min_x + width;
	m_LEGACY_cop_collision_info[offs].min_y = (m_LEGACY_cop_collision_info[offs].y >> 16) + start_y;
	m_LEGACY_cop_collision_info[offs].max_y = m_LEGACY_cop_collision_info[offs].min_y + height;
}


UINT8 raiden2cop_device::LEGACY_cop_calculate_collsion_detection()
{
	static UINT8 res;

	res = 3;

	/* outbound X check */
	if(m_LEGACY_cop_collision_info[0].max_x >= m_LEGACY_cop_collision_info[1].min_x && m_LEGACY_cop_collision_info[0].min_x <= m_LEGACY_cop_collision_info[1].max_x)
		res &= ~2;

	if(m_LEGACY_cop_collision_info[1].max_x >= m_LEGACY_cop_collision_info[0].min_x && m_LEGACY_cop_collision_info[1].min_x <= m_LEGACY_cop_collision_info[0].max_x)
		res &= ~2;

	/* outbound Y check */
	if(m_LEGACY_cop_collision_info[0].max_y >= m_LEGACY_cop_collision_info[1].min_y && m_LEGACY_cop_collision_info[0].min_y <= m_LEGACY_cop_collision_info[1].max_y)
		res &= ~1;

	if(m_LEGACY_cop_collision_info[1].max_y >= m_LEGACY_cop_collision_info[0].min_y && m_LEGACY_cop_collision_info[1].min_y <= m_LEGACY_cop_collision_info[0].max_y)
		res &= ~1;

	cop_hit_val[1] = (m_LEGACY_cop_collision_info[0].x - m_LEGACY_cop_collision_info[1].x) >> 16;
	cop_hit_val[0] = (m_LEGACY_cop_collision_info[0].y - m_LEGACY_cop_collision_info[1].y) >> 16;
	cop_hit_val[2] = 1;
	cop_hit_val_stat = res; // TODO: there's also bit 2 and 3 triggered in the tests, no known meaning

	//popmessage("%d %d %04x %04x %04x %04x",cop_hit_val[1],cop_hit_val[0],m_LEGACY_cop_collision_info[0].hitbox_x,m_LEGACY_cop_collision_info[0].hitbox_y,m_LEGACY_cop_collision_info[1].hitbox_x,m_LEGACY_cop_collision_info[1].hitbox_y);

	//if(res == 0)
	//popmessage("0:%08x %08x %08x 1:%08x %08x %08x\n",m_LEGACY_cop_collision_info[0].x,m_LEGACY_cop_collision_info[0].y,m_LEGACY_cop_collision_info[0].hitbox,m_LEGACY_cop_collision_info[1].x,m_LEGACY_cop_collision_info[1].y,m_LEGACY_cop_collision_info[1].hitbox);
//  popmessage("0:%08x %08x %08x %08x 1:%08x %08x %08x %08x\n",m_LEGACY_cop_collision_info[0].min_x,m_LEGACY_cop_collision_info[0].max_x,m_LEGACY_cop_collision_info[0].min_y, m_LEGACY_cop_collision_info[0].max_y,
//                                                   m_LEGACY_cop_collision_info[1].min_x,m_LEGACY_cop_collision_info[1].max_x,m_LEGACY_cop_collision_info[1].min_y, m_LEGACY_cop_collision_info[1].max_y);

	return res;
}


WRITE16_MEMBER(raiden2cop_device::LEGACY_cop_cmd_w)
{
	int command;


	logerror("%06x: COPX execute table macro command %04x | regs %08x %08x %08x %08x %08x\n", space.device().safe_pc(), data,  cop_regs[0], cop_regs[1], cop_regs[2], cop_regs[3], cop_regs[4]);


	command = find_trigger_match(data, 0xff00);


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
		execute_0205(space, offset, data);
		return;
	}

	/* "automatic" movement, for arcs in Legionnaire / Zero Team (expression adjustment) 0905 */
	if (check_command_matches(command, 0x194, 0x288, 0x088, 0x000, 0x000, 0x000, 0x000, 0x000, 6, 0xfbfb))
	{
		executed = 1;
		execute_0904(space, offset, data);
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
		execute_8100(space, offset, data); // SIN
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
		execute_8900(space, offset, data); // COS
		return;
	}

	/* 0x130e / 0x138e */
	if (check_command_matches(command, 0x984, 0xaa4, 0xd82, 0xaa2, 0x39b, 0xb9a, 0xb9a, 0xa9a, 5, 0xbf7f))
	{
		executed = 1;
		LEGACY_execute_130e_cupsoc(space, offset, data);
		return;
	}

	/* Pythagorean theorem, hypotenuse direction - 130e / 138e */
	//(heatbrl)  | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a b9a
	if (check_command_matches(command, 0x984, 0xaa4, 0xd82, 0xaa2, 0x39b, 0xb9a, 0xb9a, 0xb9a, 5, 0xbf7f))
	{
		executed = 1;
		LEGACY_execute_130e(space, offset, data);
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
		LEGACY_execute_3b30(space, offset, data);
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
		LEGACY_execute_42c2(space, offset, data);
		return;
	}

	/*
		collision detection:

		int dy_0 = space.read_dword(cop_regs[0]+4);
		int dx_0 = space.read_dword(cop_regs[0]+8);
		int dy_1 = space.read_dword(cop_regs[1]+4);
		int dx_1 = space.read_dword(cop_regs[1]+8);
		int hitbox_param1 = space.read_dword(cop_regs[2]);
		int hitbox_param2 = space.read_dword(cop_regs[3]);

		TODO: we are ignoring the funcval / funcmask params for now
		*/

	if (check_command_matches(command, 0xb80, 0xb82, 0xb84, 0xb86, 0x000, 0x000, 0x000, 0x000, funcval, funcmask))
	{
		LEGACY_execute_a100(space, offset, data);
		executed = 1;
		return;
	}

	//(heatbrl)  | 9 | ffff | b080 | b40 bc0 bc2
	if (check_command_matches(command, 0xb40, 0xbc0, 0xbc2, 0x000, 0x000, 0x000, 0x000, 0x000, funcval, funcmask))
	{
		executed = 1;
		LEGACY_execute_b100(space, offset, data);
		return;
	}

	if (check_command_matches(command, 0xba0, 0xba2, 0xba4, 0xba6, 0x000, 0x000, 0x000, 0x000, funcval, funcmask))
	{
		executed = 1;
		LEGACY_execute_a900(space, offset, data);
		return;
	}

	//(heatbrl)  | 6 | ffff | b880 | b60 be0 be2
	if (check_command_matches(command, 0xb60, 0xbe0, 0xbe2, 0x000, 0x000, 0x000, 0x000, 0x000, funcval, funcmask))
	{
		executed = 1;
		LEGACY_execute_b900(space, offset, data);
		return;
	}

	// grainbow 0d | a | fff3 | 6980 | b80 ba0
	if (check_command_matches(command, 0xb80, 0xba0, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 10, 0xfff3))
	{
		executed = 1;
		LEGACY_execute_6980(space, offset, data);
		return;
	}

	// grainbow 18 | a | ff00 | c480 | 080 882
	if (check_command_matches(command, 0x080, 0x882, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 10, 0xff00))
	{
		executed = 1;
		LEGACY_execute_c480(space, offset, data);
		return;
	}

	// cupsoc 1b | 5 | 7ff7 | dde5 | f80 aa2 984 0c2
	/* radar x/y positions */
	/* FIXME: x/ys are offsetted */
	/* FIXME: uses 0x10044a for something */
	if (check_command_matches(command, 0xf80, 0xaa2, 0x984, 0x0c2, 0x000, 0x000, 0x000, 0x000, 5, 0x7ff7))
	{
		executed = 1;
		LEGACY_execute_dde5(space, offset, data);
		return;
	}

	//(cupsoc)   | 8 | f3e7 | 6200 | 3a0 3a6 380 aa0 2a6
	if (check_command_matches(command, 0x3a0, 0x3a6, 0x380, 0xaa0, 0x2a6, 0x000, 0x000, 0x000, 8, 0xf3e7))
	{
		executed = 1;
		LEGACY_execute_6200(space, offset, data);
		return;
	}

	//(grainbow) | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
	/* search direction, used on SD Gundam homing weapon */
	/* FIXME: still doesn't work ... */
	if (check_command_matches(command, 0x380, 0x39a, 0x380, 0xa80, 0x29a, 0x000, 0x000, 0x000, 8, 0xf3e7))
	{
		executed = 1;
		LEGACY_execute_6200_grainbow(space, offset, data);	
		return;
	}

	//(cupsoc) 1c | 5 | b07f | e38e | 984 ac4 d82 ac2 39b b9a b9a a9a
	if (check_command_matches(command, 0x984, 0xac4, 0xd82, 0xac2, 0x39b, 0xb9a, 0xb9a, 0xa9a, 5, 0xb07f))
	{
		executed = 1;
		LEGACY_execute_e30e(space, offset, data);
		return;
	}

	//(cupsoc) 1a | 5 | fffb | d104 | ac2 9e0 0a2
	/* controls player vs. player collision detection, 0xf105 controls player vs. ball */
	if (check_command_matches(command, 0xac2, 0x9e0, 0x0a2, 0x000, 0x000, 0x000, 0x000, 0x000, 5, 0xfffb))
	{
		executed = 1;
		LEGACY_execute_d104(space, offset, data);
		return;
	}

	if (executed == 0)
		if (data!=0xf105) printf("did not execute %04x\n", data); // cup soccer triggers this a lot (and others)
}




