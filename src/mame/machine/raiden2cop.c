/***************************************************************************

 Seibu Cop (Co-Processor) emulation
  (new implementation, based on Raiden 2 code)

***************************************************************************/

#include "emu.h"
#include "raiden2cop.h"

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


	m_videoramout_cb.resolve_safe();

	cop_itoa_digit_count = 4; //TODO: Raiden 2 never inits the BCD register, value here is a guess (8 digits, as WR is 10.000.000 + a)

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
		int ppos = space.read_dword(cop_regs[0] + 4 + offset*4);
		int npos = ppos + space.read_dword(cop_regs[0] + 0x10 + offset*4);
		int delta = (npos >> 16) - (ppos >> 16);
		space.write_dword(cop_regs[0] + 4 + offset*4, npos);
		space.write_word(cop_regs[0] + 0x1e + offset*4, space.read_word(cop_regs[0] + 0x1e + offset*4) + delta);
		break;
	}

	case 0x0904: { /* X Se Dae and Zero Team uses this variant */
		space.write_dword(cop_regs[0] + 16 + offset*4, space.read_dword(cop_regs[0] + 16 + offset*4) - space.read_dword(cop_regs[0] + 0x28 + offset*4));
		break;
	}
	case 0x0905: //  0905 0006 fbfb 0008 - 0194 0288 0088 0000 0000 0000 0000 0000
		space.write_dword(cop_regs[0] + 16 + offset*4, space.read_dword(cop_regs[0] + 16 + offset*4) + space.read_dword(cop_regs[0] + 0x28 + offset*4));
		break;

	case 0x130e:   // 130e 0005 bf7f 0010 - 0984 0aa4 0d82 0aa2 039b 0b9a 0b9a 0a9a
	case 0x138e:
	case 0x338e: { // 338e 0005 bf7f 0030 - 0984 0aa4 0d82 0aa2 039c 0b9c 0b9c 0a9a
		int dx = space.read_dword(cop_regs[1]+4) - space.read_dword(cop_regs[0]+4);
		int dy = space.read_dword(cop_regs[1]+8) - space.read_dword(cop_regs[0]+8);

		if(!dy) {
			cop_status |= 0x8000;
			cop_angle = 0;
		} else {
			cop_angle = atan(double(dx)/double(dy)) * 128 / M_PI;
			if(dy<0)
				cop_angle += 0x80;
		}

		if(data & 0x0080) {
			space.write_byte(cop_regs[0]+0x34, cop_angle);
		}
		break;
	}

	case 0x2208:
	case 0x2288: { // 2208 0005 f5df 0020 - 0f8a 0b8a 0388 0b9a 0b9a 0a9a 0000 0000
		int dx = space.read_word(cop_regs[0]+0x12);
		int dy = space.read_word(cop_regs[0]+0x16);

		if(!dy) {
			cop_status |= 0x8000;
			cop_angle = 0;
		} else {
			cop_angle = atan(double(dx)/double(dy)) * 128 / M_PI;
			if(dy<0)
				cop_angle += 0x80;
		}

		if(data & 0x0080) {
			space.write_byte(cop_regs[0]+0x34, cop_angle);
		}
		break;
	}

	case 0x2a05: { // 2a05 0006 ebeb 0028 - 09af 0a82 0082 0a8f 018e 0000 0000 0000
		int delta = space.read_word(cop_regs[1] + 0x1e + offset*4);
		space.write_dword(cop_regs[0] + 4+2  + offset*4, space.read_word(cop_regs[0] + 4+2  + offset*4) + delta);
		space.write_dword(cop_regs[0] + 0x1e + offset*4, space.read_word(cop_regs[0] + 0x1e + offset*4) + delta);
		break;
	}

	case 0x39b0:
	case 0x3b30:
	case 0x3bb0: { // 3bb0 0004 007f 0038 - 0f9c 0b9c 0b9c 0b9c 0b9c 0b9c 0b9c 099c
		/* TODO: these are actually internally loaded via 0x130e command */
		int dx,dy;

		dx = space.read_dword(cop_regs[1]+4) - space.read_dword(cop_regs[0]+4);
		dy = space.read_dword(cop_regs[1]+8) - space.read_dword(cop_regs[0]+8);
		
		dx = dx >> 16;
		dy = dy >> 16;
		cop_dist = sqrt((double)(dx*dx+dy*dy));
		
		if(data & 0x0080)
			space.write_word(cop_regs[0]+(data & 0x200 ? 0x3a : 0x38), cop_dist);
		break;
	}

	case 0x42c2: { // 42c2 0005 fcdd 0040 - 0f9a 0b9a 0b9c 0b9c 0b9c 029c 0000 0000
		int div = space.read_word(cop_regs[0]+(0x36));
		if(!div)
			div = 1;

		/* TODO: bits 5-6-15 */
		cop_status = 7;

		space.write_word(cop_regs[0]+(0x38), (cop_dist << (5 - cop_scale)) / div);
		break;
	}

	case 0x4aa0: { // 4aa0 0005 fcdd 0048 - 0f9a 0b9a 0b9c 0b9c 0b9c 099b 0000 0000
		int div = space.read_word(cop_regs[0]+(0x38));
		if(!div)
			div = 1;

		/* TODO: bits 5-6-15 */
		cop_status = 7;

		space.write_word(cop_regs[0]+(0x36), (cop_dist << (5 - cop_scale)) / div);
		break;
	}

	case 0x6200: {
		UINT8 angle = space.read_byte(cop_regs[0]+0x34);
		UINT16 flags = space.read_word(cop_regs[0]);
		cop_angle_target &= 0xff;
		cop_angle_step &= 0xff;
		flags &= ~0x0004;
		int delta = angle - cop_angle_target;
		if(delta >= 128)
			delta -= 256;
		else if(delta < -128)
			delta += 256;
		if(delta < 0) {
			if(delta >= -cop_angle_step) {
				angle = cop_angle_target;
				flags |= 0x0004;
			} else
				angle += cop_angle_step;
		} else {
			if(delta <= cop_angle_step) {
				angle = cop_angle_target;
				flags |= 0x0004;
			} else
				angle -= cop_angle_step;
		}
		space.write_word(cop_regs[0], flags);
		space.write_byte(cop_regs[0]+0x34, angle);
		break;
	}

	case 0x8100: { // 8100 0007 fdfb 0080 - 0b9a 0b88 0888 0000 0000 0000 0000 0000
		int raw_angle = (space.read_word(cop_regs[0]+(0x34)) & 0xff);
		double angle = raw_angle * M_PI / 128;
		double amp = (65536 >> 5)*(space.read_word(cop_regs[0]+(0x36)) & 0xff);
		int res;
		/* TODO: up direction, why? (check machine/seicop.c) */
		if(raw_angle == 0xc0)
			amp*=2;
		res = int(amp*sin(angle)) << cop_scale;
		space.write_dword(cop_regs[0] + 16, res);
		break;
	}

	case 0x8900: { // 8900 0007 fdfb 0088 - 0b9a 0b8a 088a 0000 0000 0000 0000 0000
		int raw_angle = (space.read_word(cop_regs[0]+(0x34)) & 0xff);
		double angle = raw_angle * M_PI / 128;
		double amp = (65536 >> 5)*(space.read_word(cop_regs[0]+(0x36)) & 0xff);
		int res;
		/* TODO: left direction, why? (check machine/seicop.c) */
		if(raw_angle == 0x80)
			amp*=2;
		res = int(amp*cos(angle)) << cop_scale;
		space.write_dword(cop_regs[0] + 20, res);
		break;
	}

	case 0x5205:   // 5205 0006 fff7 0050 - 0180 02e0 03a0 00a0 03a0 0000 0000 0000
		//      fprintf(stderr, "sprcpt 5205 %04x %04x %04x %08x %08x\n", cop_regs[0], cop_regs[1], cop_regs[3], space.read_dword(cop_regs[0]), space.read_dword(cop_regs[3]));
		space.write_dword(cop_regs[1], space.read_dword(cop_regs[0]));
		break;

	case 0x5a05:   // 5a05 0006 fff7 0058 - 0180 02e0 03a0 00a0 03a0 0000 0000 0000
		//      fprintf(stderr, "sprcpt 5a05 %04x %04x %04x %08x %08x\n", cop_regs[0], cop_regs[1], cop_regs[3], space.read_dword(cop_regs[0]), space.read_dword(cop_regs[3]));
		space.write_dword(cop_regs[1], space.read_dword(cop_regs[0]));
		break;

	case 0xf205:   // f205 0006 fff7 00f0 - 0182 02e0 03c0 00c0 03c0 0000 0000 0000
		//      fprintf(stderr, "sprcpt f205 %04x %04x %04x %08x %08x\n", cop_regs[0]+4, cop_regs[1], cop_regs[3], space.read_dword(cop_regs[0]+4), space.read_dword(cop_regs[3]));
		space.write_dword(cop_regs[2], space.read_dword(cop_regs[0]+4));
		break;

		// raidendx only
	case 0x7e05:
		space.write_byte(0x470, space.read_byte(cop_regs[4]));
		break;

	case 0xa100:
	case 0xa180:
		cop_collision_read_pos(space, 0, cop_regs[0], data & 0x0080);
		break;

	case 0xa900:
	case 0xa980:
		cop_collision_read_pos(space, 1, cop_regs[1], data & 0x0080);
		break;

	case 0xb100: {
		cop_collision_update_hitbox(space, 0, cop_regs[2]);
		break;
	}

	case 0xb900: {
		cop_collision_update_hitbox(space, 1, cop_regs[3]);
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
