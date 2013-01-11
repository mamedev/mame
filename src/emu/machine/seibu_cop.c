/***************************************************************************

    Seibu COP protection device

    (this header needs expanding!)

***************************************************************************/

#include "emu.h"
#include "machine/seibu_cop.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SEIBU_COP = &device_creator<seibu_cop_device>;


static ADDRESS_MAP_START( seibu_cop_io, AS_0, 16, seibu_cop_device )
	AM_RANGE(0x0428, 0x0429) AM_WRITE(fill_val_lo_w)
	AM_RANGE(0x042a, 0x042b) AM_WRITE(fill_val_hi_w)

	AM_RANGE(0x045a, 0x045b) AM_WRITE(pal_brightness_val_w)
	AM_RANGE(0x045c, 0x045d) AM_WRITE(pal_brightness_mode_w)

	AM_RANGE(0x0474, 0x0475) AM_WRITE(dma_unk_param_w)
	AM_RANGE(0x0476, 0x0477) AM_WRITE(dma_pal_fade_table_w)
	AM_RANGE(0x0478, 0x0479) AM_WRITE(dma_src_w)
	AM_RANGE(0x047a, 0x047b) AM_WRITE(dma_size_w)
	AM_RANGE(0x047c, 0x047d) AM_WRITE(dma_dst_w)
	AM_RANGE(0x047e, 0x047f) AM_READWRITE(dma_trigger_r, dma_trigger_w)
ADDRESS_MAP_END


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT16 seibu_cop_device::read_word(offs_t address)
{
	return space().read_word(address);
}

//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void seibu_cop_device::write_word(offs_t address, UINT16 data)
{
	space().write_word(address, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  seibu_cop_device - constructor
//-------------------------------------------------

seibu_cop_device::seibu_cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEIBU_COP, "seibu_cop", tag, owner, clock),
		device_memory_interface(mconfig, *this),
		m_space_config("io", ENDIANNESS_LITTLE, 16, 16, 0, NULL, *ADDRESS_MAP_NAME(seibu_cop_io))
{

}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void seibu_cop_device::device_config_complete()
{
	// inherit a copy of the static data
	const seibu_cop_interface *intf = reinterpret_cast<const seibu_cop_interface *>(static_config());
	if (intf != NULL)
		*static_cast<seibu_cop_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_byte_cb,   0, sizeof(m_in_byte_cb));
		memset(&m_in_word_cb,   0, sizeof(m_in_word_cb));
		memset(&m_in_dword_cb,  0, sizeof(m_in_dword_cb));
		memset(&m_out_byte_cb,  0, sizeof(m_out_byte_cb));
		memset(&m_out_word_cb,  0, sizeof(m_out_word_cb));
		memset(&m_out_dword_cb, 0, sizeof(m_out_dword_cb));
	}
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void seibu_cop_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void seibu_cop_device::device_start()
{
	// resolve callbacks
	m_in_byte_func.resolve(m_in_byte_cb, *this);
	m_in_word_func.resolve(m_in_word_cb, *this);
	m_in_word_func.resolve(m_in_dword_cb, *this);
	m_out_byte_func.resolve(m_out_byte_cb, *this);
	m_out_word_func.resolve(m_out_word_cb, *this);
	m_out_dword_func.resolve(m_out_dword_cb, *this);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void seibu_cop_device::device_reset()
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *seibu_cop_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE16_MEMBER(seibu_cop_device::fill_val_lo_w)
{
	COMBINE_DATA(&m_fill_val_lo);
	m_fill_val = (m_fill_val_lo) | (m_fill_val_hi << 16);
}

WRITE16_MEMBER(seibu_cop_device::fill_val_hi_w)
{
	COMBINE_DATA(&m_fill_val_hi);
	m_fill_val = (m_fill_val_lo) | (m_fill_val_hi << 16);
}

WRITE16_MEMBER(seibu_cop_device::pal_brightness_val_w)
{
	COMBINE_DATA(&m_pal_brightness_val);

	/* TODO: add checks for bits 15-6 */
}

WRITE16_MEMBER(seibu_cop_device::pal_brightness_mode_w)
{
	COMBINE_DATA(&m_pal_brightness_mode);

	/* TODO: add checks for anything that isn't 4 or 5 */
}

WRITE16_MEMBER(seibu_cop_device::dma_unk_param_w)
{
	/*
	    This sets up a DMA mode of some sort
	        0x0e00: grainbow, cupsoc
	        0x0a00: legionna, godzilla, denjinmk
	        0x0600: heatbrl
	        0x1e00: zeroteam, xsedae
	    raiden2 and raidendx doesn't set this up, this could indicate that this is related to the non-private buffer DMAs
	    (both only uses 0x14 and 0x15 as DMAs afaik)
	*/
	COMBINE_DATA(&m_dma_unk_param);
}

WRITE16_MEMBER(seibu_cop_device::dma_pal_fade_table_w)
{
	COMBINE_DATA(&m_dma_pal_fade_table);
}

WRITE16_MEMBER(seibu_cop_device::dma_src_w)
{
	COMBINE_DATA(&m_dma_src[m_dma_trigger]);
}

WRITE16_MEMBER(seibu_cop_device::dma_size_w)
{
	COMBINE_DATA(&m_dma_size[m_dma_trigger]);
}

WRITE16_MEMBER(seibu_cop_device::dma_dst_w)
{
	COMBINE_DATA(&m_dma_dst[m_dma_trigger]);
}

READ16_MEMBER(seibu_cop_device::dma_trigger_r)
{
	return m_dma_exec_param;
}

WRITE16_MEMBER(seibu_cop_device::dma_trigger_w)
{
	COMBINE_DATA(&m_dma_exec_param);
	m_dma_trigger = m_dma_exec_param & 7;
}

//**************************************************************************
//  READ/WRITE HANDLERS (device to CPU / CPU to device)
//**************************************************************************

READ16_MEMBER( seibu_cop_device::read )
{
	return read_word(offset + (0x400/2));
}

WRITE16_MEMBER( seibu_cop_device::write )
{
	write_word(offset + (0x400/2),data);
}

void seibu_cop_device::normal_dma_transfer(void)
{
	UINT32 src,dst,size,i;

	src = (m_dma_src[m_dma_trigger] << 6);
	dst = (m_dma_dst[m_dma_trigger] << 6);
	size = ((m_dma_size[m_dma_trigger] << 5) - (m_dma_dst[m_dma_trigger] << 6) + 0x20)/2;

	for(i = 0;i < size;i++)
	{
		m_out_word_func(dst, m_in_word_func(src));
		src+=2;
		dst+=2;
	}
}

/* RE from Seibu Cup Soccer bootleg */
const UINT8 seibu_cop_device::fade_table(int v)
{
	int low  = v & 0x001f;
	int high = v & 0x03e0;

	return (low * (high | (high >> 5)) + 0x210) >> 10;
}

void seibu_cop_device::palette_dma_transfer(void)
{
	UINT32 src,dst,size,i;

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

	src = (m_dma_src[m_dma_trigger] << 6);
	dst = (m_dma_dst[m_dma_trigger] << 6);
	size = ((m_dma_size[m_dma_trigger] << 5) - (m_dma_dst[m_dma_trigger] << 6) + 0x20)/2;

	//printf("SRC: %08x %08x DST:%08x SIZE:%08x TRIGGER: %08x %02x %02x\n",cop_dma_src[cop_dma_trigger] << 6,cop_dma_fade_table * 0x400,cop_dma_dst[cop_dma_trigger] << 6,cop_dma_size[cop_dma_trigger] << 5,cop_dma_trigger,pal_brightness_val,pal_brightness_mode);

	for(i = 0;i < size;i++)
	{
		UINT16 pal_val;
		int r,g,b;
		int rt,gt,bt;

		if(m_pal_brightness_mode == 5)
		{
			bt = ((m_in_word_func(src + (m_dma_pal_fade_table * 0x400))) & 0x7c00) >> 5;
			bt = fade_table(bt|(m_pal_brightness_val ^ 0));
			b = ((m_in_word_func(src)) & 0x7c00) >> 5;
			b = fade_table(b|(m_pal_brightness_val ^ 0x1f));
			pal_val = ((b + bt) & 0x1f) << 10;
			gt = ((m_in_word_func(src + (m_dma_pal_fade_table * 0x400))) & 0x03e0);
			gt = fade_table(gt|(m_pal_brightness_val ^ 0));
			g = ((m_in_word_func(src)) & 0x03e0);
			g = fade_table(g|(m_pal_brightness_val ^ 0x1f));
			pal_val |= ((g + gt) & 0x1f) << 5;
			rt = ((m_in_word_func(src + (m_dma_pal_fade_table * 0x400))) & 0x001f) << 5;
			rt = fade_table(rt|(m_pal_brightness_val ^ 0));
			r = ((m_in_word_func(src)) & 0x001f) << 5;
			r = fade_table(r|(m_pal_brightness_val ^ 0x1f));
			pal_val |= ((r + rt) & 0x1f);
		}
		else if(m_pal_brightness_mode == 4) //Denjin Makai
		{
			bt =(m_in_word_func(src + (m_dma_pal_fade_table * 0x400)) & 0x7c00) >> 10;
			b = (m_in_word_func(src) & 0x7c00) >> 10;
			gt =(m_in_word_func(src + (m_dma_pal_fade_table * 0x400)) & 0x03e0) >> 5;
			g = (m_in_word_func(src) & 0x03e0) >> 5;
			rt =(m_in_word_func(src + (m_dma_pal_fade_table * 0x400)) & 0x001f) >> 0;
			r = (m_in_word_func(src) & 0x001f) >> 0;

			if(m_pal_brightness_val == 0x10)
				pal_val = bt << 10 | gt << 5 | rt << 0;
			else if(m_pal_brightness_val == 0xff) // TODO: might be the back plane or it still doesn't do any mod, needs PCB tests
				pal_val = 0;
			else
			{
				bt = fade_table(bt<<5|((m_pal_brightness_val*2) ^ 0));
				b =  fade_table(b<<5|((m_pal_brightness_val*2) ^ 0x1f));
				pal_val = ((b + bt) & 0x1f) << 10;
				gt = fade_table(gt<<5|((m_pal_brightness_val*2) ^ 0));
				g =  fade_table(g<<5|((m_pal_brightness_val*2) ^ 0x1f));
				pal_val |= ((g + gt) & 0x1f) << 5;
				rt = fade_table(rt<<5|((m_pal_brightness_val*2) ^ 0));
				r =  fade_table(r<<5|((m_pal_brightness_val*2) ^ 0x1f));
				pal_val |= ((r + rt) & 0x1f);
			}
		}
		else
		{
			printf("Seibu COP: palette DMA used with mode %02x!\n",m_pal_brightness_mode);
			pal_val = m_in_word_func(src);
		}

		m_out_word_func(dst, pal_val);
		src+=2;
		dst+=2;
	}
}

void seibu_cop_device::fill_word_transfer(void)
{
	UINT32 length, address;
	int i;

	//if(cop_dma_dst[cop_dma_trigger] != 0x0000) // Invalid?
	//  return;

	address = (m_dma_src[m_dma_trigger] << 6);
	length = ((m_dma_size[m_dma_trigger]+1) << 4);

	for (i=address;i<address+length;i+=4)
	{
		m_out_dword_func(i, m_fill_val);
	}
}

void seibu_cop_device::fill_dword_transfer(void)
{
	UINT32 length, address;
	int i;
	if(m_dma_dst[m_dma_trigger] != 0x0000) // Invalid? TODO: log & check this
		return;

	address = (m_dma_src[m_dma_trigger] << 6);
	length = (m_dma_size[m_dma_trigger]+1) << 5;

	//printf("%08x %08x\n",address,length);

	for (i=address;i<address+length;i+=4)
	{
		m_out_dword_func(i, m_fill_val);
	}
}

WRITE16_MEMBER( seibu_cop_device::dma_write_trigger_w )
{
	switch(m_dma_exec_param & 0xfff8)
	{
		case 0x008: normal_dma_transfer(); break;
		case 0x010: break; // private buffer copy, TODO
		case 0x080: palette_dma_transfer(); break;
		case 0x110: fill_word_transfer(); break; // Godzilla uses this
		case 0x118: fill_dword_transfer(); break;
		default:
			logerror("Seibu COP: used unemulated DMA type %04x\n",m_dma_exec_param);
	}
}
