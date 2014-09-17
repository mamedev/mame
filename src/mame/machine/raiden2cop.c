/***************************************************************************

 Seibu Cop (Co-Processor) emulation
  (new implementation, based on Raiden 2 code)

***************************************************************************/

#include "emu.h"
#include "raiden2cop.h"

const device_type RAIDEN2COP = &device_creator<raiden2cop_device>;

raiden2cop_device::raiden2cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RAIDEN2COP, "RAIDEN2COP", tag, owner, clock, "raiden2cop", __FILE__),
	cop_dma_v1(0),
	cop_dma_v2(0),
	cop_dma_mode(0),
	cop_dma_adr_rel(0),
	pal_brightness_val(0),
	pal_brightness_mode(0),
	m_videoramout_cb(*this),
	m_palette(*this, ":palette")
{
	  memset(cop_dma_src, 0, sizeof(UINT16)*(0x200));
	  memset(cop_dma_dst, 0, sizeof(UINT16)*(0x200));
	  memset(cop_dma_size, 0, sizeof(UINT16)*(0x200));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void raiden2cop_device::device_start()
{
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
