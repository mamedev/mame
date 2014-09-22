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

	m_videoramout_cb.resolve_safe();
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
