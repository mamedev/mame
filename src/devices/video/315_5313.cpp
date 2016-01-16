// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega 315-5313 - Megadrive VDP */

#include "emu.h"
#include "video/315_5313.h"

/* still have dependencies on the following external gunk */

#include "sound/sn76496.h"

#define MAX_HPOSITION 480


const device_type SEGA315_5313 = &device_creator<sega315_5313_device>;

sega315_5313_device::sega315_5313_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: sega315_5124_device(mconfig, SEGA315_5313, "Sega 315-5313 Megadrive VDP", tag, owner, clock, SEGA315_5124_CRAM_SIZE, 0, true, "sega315_5313", __FILE__), m_render_bitmap(nullptr),
	m_render_line(nullptr), m_render_line_raw(nullptr), m_megadriv_scanline_timer(nullptr),
	m_sndirqline_callback(*this),
	m_lv6irqline_callback(*this),
	m_lv4irqline_callback(*this), m_command_pending(0), m_command_part1(0), m_command_part2(0), m_vdp_code(0), m_vdp_address(0), m_vram_fill_pending(0), m_vram_fill_length(0), m_irq4counter(0),
	m_imode_odd_frame(0), m_sprite_collision(0), m_irq6_pending(0), m_irq4_pending(0), m_scanline_counter(0), m_vblank_flag(0), m_imode(0), m_visible_scanlines(0), m_irq6_scanline(0),
	m_z80irq_scanline(0), m_total_scanlines(0), m_base_total_scanlines(0), m_framerate(0), m_vdp_pal(0), m_use_cram(0),
	m_dma_delay(0), m_regs(nullptr), m_vram(nullptr), m_cram(nullptr), m_vsram(nullptr), m_internal_sprite_attribute_table(nullptr), m_irq6_on_timer(nullptr), m_irq4_on_timer(nullptr),
	m_render_timer(nullptr), m_sprite_renderline(nullptr), m_highpri_renderline(nullptr), m_video_renderline(nullptr), m_palette_lookup(nullptr), m_palette_lookup_sprite(nullptr),
	m_palette_lookup_shadow(nullptr), m_palette_lookup_highlight(nullptr), m_space68k(nullptr), m_cpu68k(nullptr)
{
	m_use_alt_timing = 0;
	m_palwrite_base = -1;
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void sega315_5313_device::static_set_palette_tag(device_t &device, std::string tag)
{
	downcast<sega315_5313_device &>(device).m_palette.set_tag(tag);
}


static MACHINE_CONFIG_FRAGMENT( sega_genesis_vdp )
	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(sega315_5124_device, sega315_5124)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor sega315_5313_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sega_genesis_vdp );
}

static TIMER_CALLBACK( render_timer_callback )
{
	sega315_5313_device* vdp = (sega315_5313_device*)ptr;
	vdp->render_scanline();
}

void sega315_5313_device::vdp_handle_irq6_on_timer_callback(int param)
{
// m_irq6_pending = 1;
	if (MEGADRIVE_REG01_IRQ6_ENABLE)
		m_lv6irqline_callback(true);
}

static TIMER_CALLBACK( irq6_on_timer_callback )
{
	sega315_5313_device* vdp = (sega315_5313_device*)ptr;
	vdp->vdp_handle_irq6_on_timer_callback(param);
}

void sega315_5313_device::vdp_handle_irq4_on_timer_callback(int param)
{
	m_lv4irqline_callback(true);
}

static TIMER_CALLBACK( irq4_on_timer_callback )
{
	sega315_5313_device* vdp = (sega315_5313_device*)ptr;
	vdp->vdp_handle_irq4_on_timer_callback(param);
}



void sega315_5313_device::set_alt_timing(device_t &device, int use_alt_timing)
{
	sega315_5313_device &dev = downcast<sega315_5313_device &>(device);
	dev.m_use_alt_timing = use_alt_timing;
}

void sega315_5313_device::set_palwrite_base(device_t &device, int palwrite_base)
{
	sega315_5313_device &dev = downcast<sega315_5313_device &>(device);
	dev.m_palwrite_base = palwrite_base;
}




void sega315_5313_device::device_start()
{
	m_sndirqline_callback.resolve_safe();
	m_lv6irqline_callback.resolve_safe();
	m_lv4irqline_callback.resolve_safe();

	m_32x_scanline_func.bind_relative_to(*owner());
	m_32x_interrupt_func.bind_relative_to(*owner());
	m_32x_scanline_helper_func.bind_relative_to(*owner());

	m_vram  = std::make_unique<UINT16[]>(0x10000/2);
	m_cram  = std::make_unique<UINT16[]>(0x80/2);
	m_vsram = std::make_unique<UINT16[]>(0x80/2);
	m_regs = std::make_unique<UINT16[]>(0x40/2);
	m_internal_sprite_attribute_table = std::make_unique<UINT16[]>(0x400/2);

	memset(m_vram.get(), 0x00, 0x10000);
	memset(m_cram.get(), 0x00, 0x80);
	memset(m_vsram.get(), 0x00, 0x80);
	memset(m_regs.get(), 0x00, 0x40);
	memset(m_internal_sprite_attribute_table.get(), 0x00, 0x400);


	save_pointer(NAME(m_vram.get()), 0x10000/2);
	save_pointer(NAME(m_cram.get()), 0x80/2);
	save_pointer(NAME(m_vsram.get()), 0x80/2);
	save_pointer(NAME(m_regs.get()), 0x40/2);
	save_pointer(NAME(m_internal_sprite_attribute_table.get()), 0x400/2);

	save_item(NAME(m_command_pending));
	save_item(NAME(m_command_part1));
	save_item(NAME(m_command_part2));
	save_item(NAME(m_vdp_code));
	save_item(NAME(m_vdp_address));
	save_item(NAME(m_vram_fill_pending));
	save_item(NAME(m_vram_fill_length));
	save_item(NAME(m_irq4counter));
	save_item(NAME(m_imode_odd_frame));
	save_item(NAME(m_sprite_collision));
	save_item(NAME(m_imode));
	save_item(NAME(m_irq6_pending));
	save_item(NAME(m_irq4_pending));
	save_item(NAME(m_visible_scanlines));
	save_item(NAME(m_irq6_scanline));
	save_item(NAME(m_z80irq_scanline));
	save_item(NAME(m_scanline_counter));
	save_item(NAME(m_vblank_flag));
	save_item(NAME(m_total_scanlines));

	m_sprite_renderline = std::make_unique<UINT8[]>(1024);
	m_highpri_renderline = std::make_unique<UINT8[]>(320);
	m_video_renderline = std::make_unique<UINT32[]>(320);

	m_palette_lookup = std::make_unique<UINT16[]>(0x40);
	m_palette_lookup_sprite = std::make_unique<UINT16[]>(0x40);

	m_palette_lookup_shadow = std::make_unique<UINT16[]>(0x40);
	m_palette_lookup_highlight = std::make_unique<UINT16[]>(0x40);

	memset(m_palette_lookup.get(),0x00,0x40*2);
	memset(m_palette_lookup_sprite.get(),0x00,0x40*2);

	memset(m_palette_lookup_shadow.get(),0x00,0x40*2);
	memset(m_palette_lookup_highlight.get(),0x00,0x40*2);


	if (!m_use_alt_timing)
		m_render_bitmap = std::make_unique<bitmap_ind16>(320, 512); // allocate maximum sizes we're going to use, it's safer.
	else
		m_render_line = std::make_unique<UINT16[]>(320);

	m_render_line_raw = std::make_unique<UINT16[]>(320);

	// FIXME: are these all needed? I'm pretty sure some of these (most?) are just helpers which don't need to be saved,
	// but better safe than sorry...
	save_pointer(NAME(m_sprite_renderline.get()), 1024);
	save_pointer(NAME(m_highpri_renderline.get()), 320);
	save_pointer(NAME(m_video_renderline.get()), 320/4);
	save_pointer(NAME(m_palette_lookup.get()), 0x40);
	save_pointer(NAME(m_palette_lookup_sprite.get()), 0x40);
	save_pointer(NAME(m_palette_lookup_shadow.get()), 0x40);
	save_pointer(NAME(m_palette_lookup_highlight.get()), 0x40);
	save_pointer(NAME(m_render_line_raw.get()), 320/2);
	if (m_use_alt_timing)
		save_pointer(NAME(m_render_line.get()), 320/2);

	m_irq6_on_timer = machine().scheduler().timer_alloc(FUNC(irq6_on_timer_callback), (void*)this);
	m_irq4_on_timer = machine().scheduler().timer_alloc(FUNC(irq4_on_timer_callback), (void*)this);
	m_render_timer = machine().scheduler().timer_alloc(FUNC(render_timer_callback), (void*)this);

	m_space68k = &machine().device<m68000_base_device>(":maincpu")->space();
	m_cpu68k = machine().device<m68000_base_device>(":maincpu");

	sega315_5124_device::device_start();
}

void sega315_5313_device::device_reset()
{
	m_command_pending = 0;
	m_command_part1 = 0;
	m_command_part2 = 0;
	m_vdp_code = 0;
	m_vdp_address = 0;
	m_vram_fill_pending = 0;
	m_vram_fill_length = 0;
	m_irq4counter = -1;
	m_imode_odd_frame = 0;
	m_sprite_collision = 0;
	m_imode = 0;
	m_irq6_pending = 0;
	m_irq4_pending = 0;
	m_scanline_counter = 0;
	m_vblank_flag = 0;
	m_total_scanlines = 262;

	sega315_5124_device::device_reset();
}

void sega315_5313_device::device_reset_old()
{
	// other stuff, are we sure we want to set some of these every reset?
	// it's called from machine_reset
	m_total_scanlines = 262;
	m_visible_scanlines = 224;
	m_irq6_scanline = 224;
	m_z80irq_scanline = 226;
}



void sega315_5313_device::vdp_vram_write(UINT16 data)
{
	UINT16 sprite_base_address = MEGADRIVE_REG0C_RS1?((MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9):((MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9);
	int spritetable_size = MEGADRIVE_REG0C_RS1?0x400:0x200;
	int lowlimit = sprite_base_address;
	int highlimit = sprite_base_address+spritetable_size;

	if (m_vdp_address&1)
	{
		data = ((data&0x00ff)<<8)|((data&0xff00)>>8);
	}

	MEGADRIV_VDP_VRAM(m_vdp_address>>1) = data;

	/* The VDP stores an Internal copy of any data written to the Sprite Attribute Table.
	   This data is _NOT_ invalidated when the Sprite Base Address changes, thus allowing
	   for some funky effects, as used by Castlevania Bloodlines Stage 6-3 */
	if (m_vdp_address>=lowlimit && m_vdp_address<highlimit)
	{
//      osd_printf_debug("spritebase is %04x-%04x vram address is %04x, write %04x\n",lowlimit, highlimit-1, m_vdp_address, data);
		m_internal_sprite_attribute_table[(m_vdp_address&(spritetable_size-1))>>1] = data;
	}

	m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
	m_vdp_address &= 0xffff;
}

void sega315_5313_device::vdp_vsram_write(UINT16 data)
{
	m_vsram[(m_vdp_address&0x7e)>>1] = data;

	//logerror("Wrote to VSRAM addr %04x data %04x\n",m_vdp_address&0xfffe,m_vsram[m_vdp_address>>1]);

	m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;

	m_vdp_address &=0xffff;
}

void sega315_5313_device::write_cram_value(int offset, int data)
{
	m_cram[offset] = data;

	//logerror("Wrote to CRAM addr %04x data %04x\n",m_vdp_address&0xfffe,m_cram[m_vdp_address>>1]);
	if (m_use_cram)
	{
		int r,g,b;
		r = ((data >> 1)&0x07);
		g = ((data >> 5)&0x07);
		b = ((data >> 9)&0x07);
		if (m_palwrite_base != -1)
		{
			m_palette->set_pen_color(offset + m_palwrite_base ,pal3bit(r),pal3bit(g),pal3bit(b));
			m_palette->set_pen_color(offset + m_palwrite_base + 0x40 ,pal3bit(r>>1),pal3bit(g>>1),pal3bit(b>>1));
			m_palette->set_pen_color(offset + m_palwrite_base + 0x80 ,pal3bit((r>>1)|0x4),pal3bit((g>>1)|0x4),pal3bit((b>>1)|0x4));
		}
		m_palette_lookup[offset] = (b<<2) | (g<<7) | (r<<12);
		m_palette_lookup_sprite[offset] = (b<<2) | (g<<7) | (r<<12);
		m_palette_lookup_shadow[offset] = (b<<1) | (g<<6) | (r<<11);
		m_palette_lookup_highlight[offset] = ((b|0x08)<<1) | ((g|0x08)<<6) | ((r|0x08)<<11);
	}
}

void sega315_5313_device::vdp_cram_write(UINT16 data)
{
	int offset;
	offset = (m_vdp_address&0x7e)>>1;

	write_cram_value(offset,data);

	m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;

	m_vdp_address &=0xffff;
}


void sega315_5313_device::data_port_w(int data)
{
	m_command_pending = 0;

	/*
	0000b : VRAM read
	0001b : VRAM write
	0011b : CRAM write
	0100b : VSRAM read
	0101b : VSRAM write
	1000b : CRAM read
	*/
//  logerror("write to vdp data port %04x with code %04x, write address %04x\n",data, m_vdp_code, m_vdp_address );

	if (m_vram_fill_pending)
	{
		int count;

		m_vdp_address&=0xffff;

		if (m_vdp_address&1)
		{
			MEGADRIV_VDP_VRAM((m_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((m_vdp_address>>1))&0xff00) | (data&0x00ff);
		}
		else
		{
			MEGADRIV_VDP_VRAM((m_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((m_vdp_address>>1))&0x00ff) | ((data&0x00ff)<<8);
		}


		for (count=0;count<=m_vram_fill_length;count++) // <= for james pond 3
		{
			if (m_vdp_address&1)
			{
				MEGADRIV_VDP_VRAM((m_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((m_vdp_address>>1))&0x00ff) | (data&0xff00);
			}
			else
			{
				MEGADRIV_VDP_VRAM((m_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((m_vdp_address>>1))&0xff00) | ((data&0xff00)>>8);
			}

			m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			m_vdp_address&=0xffff;

		}

		m_regs[0x13] = 0;
		m_regs[0x14] = 0;

	//  m_regs[0x15] = (source>>1) & 0xff;
	//  m_regs[0x16] = (source>>9) & 0xff;
	//  m_regs[0x17] = (source>>17) & 0xff;


	}
	else
	{
		switch (m_vdp_code & 0x000f)
		{
			case 0x0000:
				logerror("Attempting to WRITE to DATA PORT in VRAM READ MODE\n");
				break;

			case 0x0001:
				vdp_vram_write(data);
				break;

			case 0x0003:
				vdp_cram_write(data);
				break;

			case 0x0004:
				logerror("Attempting to WRITE to DATA PORT in VSRAM READ MODE\n");
				break;

			case 0x0005:
				vdp_vsram_write(data);
				break;

			case 0x0008:
				logerror("Attempting to WRITE to DATA PORT in CRAM READ MODE\n");
				break;

			default:
				logerror("Attempting to WRITE to DATA PORT in #UNDEFINED# MODE %1x %04x\n",m_vdp_code&0xf, data);
				break;
		}
	}



}



void sega315_5313_device::vdp_set_register(int regnum, UINT8 value)
{
	m_regs[regnum] = value;

	/* We need special handling for the IRQ enable registers, some games turn
	   off the irqs before they are taken, delaying them until the IRQ is turned
	   back on */

	if (regnum == 0x00)
	{
	//osd_printf_debug("setting reg 0, irq enable is now %d\n",MEGADRIVE_REG0_IRQ4_ENABLE);

		if (m_irq4_pending)
		{
			if (MEGADRIVE_REG0_IRQ4_ENABLE)
				m_lv4irqline_callback(true);
			else
				m_lv4irqline_callback(false);
		}

		/* ??? Fatal Rewind needs this but I'm not sure it's accurate behavior
		   it causes flickering in roadrash */
	//  m_irq6_pending = 0;
	//  m_irq4_pending = 0;

	}

	if (regnum == 0x01)
	{
		if (m_irq6_pending)
		{
			if (MEGADRIVE_REG01_IRQ6_ENABLE )
				m_lv6irqline_callback(true);
			else
				m_lv6irqline_callback(false);

		}

		/* ??? */
	//  m_irq6_pending = 0;
	//  m_irq4_pending = 0;

	}


//  if (regnum == 0x0a)
//      osd_printf_debug("Set HINT Reload Register to %d on scanline %d\n",value, get_scanline_counter());

//  osd_printf_debug("%s: Setting VDP Register #%02x to %02x\n",machine().describe_context(), regnum,value);
}

void sega315_5313_device::update_code_and_address(void)
{
	m_vdp_code = ((m_command_part1 & 0xc000) >> 14) |
							((m_command_part2 & 0x00f0) >> 2);

	m_vdp_address = ((m_command_part1 & 0x3fff) >> 0) |
							((m_command_part2 & 0x0003) << 14);
}

// if either SVP CPU or segaCD is present, there is a 'lag' we have to compensate for
// hence, for segacd and svp we set m_dma_delay to the appropriate value at start
inline UINT16 sega315_5313_device::vdp_get_word_from_68k_mem(UINT32 source)
{
	// should we limit the valid areas here?
	// how does this behave with the segacd etc?
	// note, the RV bit on 32x is important for this to work, because it causes a normal cart mapping - see tempo

	//printf("vdp_get_word_from_68k_mem_default %08x\n", source);

	if (source <= 0x3fffff)
		return m_space68k->read_word(source - m_dma_delay);    // compensate DMA lag
	else if ((source >= 0xe00000) && (source <= 0xffffff))
		return m_space68k->read_word(source);
	else
	{
		printf("DMA Read unmapped %06x\n",source);
		return machine().rand();
	}
}

/*  Table from Charles Macdonald


    DMA Mode      Width       Display      Transfer Count
    -----------------------------------------------------
    68K > VDP     32-cell     Active       16
                              Blanking     167
                  40-cell     Active       18
                              Blanking     205
    VRAM Fill     32-cell     Active       15
                              Blanking     166
                  40-cell     Active       17
                              Blanking     204
    VRAM Copy     32-cell     Active       8
                              Blanking     83
                  40-cell     Active       9
                              Blanking     102

*/


/* Note, In reality this transfer is NOT instant, the 68k isn't paused
   as the 68k address bus isn't accessed */

/* Wani Wani World, James Pond 3, Pirates Gold! */
void sega315_5313_device::insta_vram_copy(UINT32 source, UINT16 length)
{
	int x;

	for (x=0;x<length;x++)
	{
		UINT8 source_byte;

		//osd_printf_debug("vram copy length %04x source %04x dest %04x\n",length, source, m_vdp_address );
		if (source&1) source_byte = MEGADRIV_VDP_VRAM((source&0xffff)>>1)&0x00ff;
		else  source_byte = (MEGADRIV_VDP_VRAM((source&0xffff)>>1)&0xff00)>>8;

		if (m_vdp_address&1)
		{
			MEGADRIV_VDP_VRAM((m_vdp_address&0xffff)>>1) = (MEGADRIV_VDP_VRAM((m_vdp_address&0xffff)>>1)&0xff00) | source_byte;
		}
		else
		{
			MEGADRIV_VDP_VRAM((m_vdp_address&0xffff)>>1) = (MEGADRIV_VDP_VRAM((m_vdp_address&0xffff)>>1)&0x00ff) | (source_byte<<8);
		}

		source++;
		m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		m_vdp_address&=0xffff;
	}
}

/* Instant, but we pause the 68k a bit */
void sega315_5313_device::insta_68k_to_vram_dma(UINT32 source,int length)
{
	int count;

	if (length==0x00) length = 0xffff;

	/* This is a hack until real DMA timings are implemented */
	m_cpu68k->spin_until_time(attotime::from_nsec(length * 1000 / 3500));

	for (count = 0;count<(length>>1);count++)
	{
		vdp_vram_write(vdp_get_word_from_68k_mem(source));
		source+=2;
		if (source>0xffffff) source = 0xe00000;
	}

	m_vdp_address&=0xffff;

	m_regs[0x13] = 0;
	m_regs[0x14] = 0;

	m_regs[0x15] = (source>>1) & 0xff;
	m_regs[0x16] = (source>>9) & 0xff;
	m_regs[0x17] = (source>>17) & 0xff;
}


void sega315_5313_device::insta_68k_to_cram_dma(UINT32 source,UINT16 length)
{
	int count;

	if (length==0x00) length = 0xffff;

	for (count = 0;count<(length>>1);count++)
	{
		//if (m_vdp_address>=0x80) return; // abandon

		write_cram_value((m_vdp_address&0x7e)>>1, vdp_get_word_from_68k_mem(source));
		source+=2;

		if (source>0xffffff) source = 0xfe0000;

		m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		m_vdp_address&=0xffff;
	}

	m_regs[0x13] = 0;
	m_regs[0x14] = 0;

	m_regs[0x15] = (source>>1) & 0xff;
	m_regs[0x16] = (source>>9) & 0xff;
	m_regs[0x17] = (source>>17) & 0xff;

}

void sega315_5313_device::insta_68k_to_vsram_dma(UINT32 source,UINT16 length)
{
	int count;

	if (length==0x00) length = 0xffff;

	for (count = 0;count<(length>>1);count++)
	{
		if (m_vdp_address>=0x80) return; // abandon

		m_vsram[(m_vdp_address&0x7e)>>1] = vdp_get_word_from_68k_mem(source);
		source+=2;

		if (source>0xffffff) source = 0xfe0000;

		m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		m_vdp_address&=0xffff;
	}

	m_regs[0x13] = 0;
	m_regs[0x14] = 0;

	m_regs[0x15] = (source>>1) & 0xff;
	m_regs[0x16] = (source>>9) & 0xff;
	m_regs[0x17] = (source>>17) & 0xff;
}

/* This can be simplified quite a lot.. */
void sega315_5313_device::handle_dma_bits()
{
#if 0
	if (m_vdp_code&0x20)
	{
		UINT32 source;
		UINT16 length;
		source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0xff)<<16))<<1;
		length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;
		osd_printf_debug("%s 68k DMAtran set source %06x length %04x dest %04x enabled %01x code %02x %02x\n", machine().describe_context(), source, length, m_vdp_address,MEGADRIVE_REG01_DMA_ENABLE, m_vdp_code,MEGADRIVE_REG0F_AUTO_INC);
	}
#endif
	if (m_vdp_code==0x20)
	{
		osd_printf_debug("DMA bit set 0x20 but invalid??\n");
	}
	else if (m_vdp_code==0x21 || m_vdp_code==0x31) /* 0x31 used by tecmo cup */
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//osd_printf_debug("68k->VRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, m_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) insta_68k_to_vram_dma(source,length);

		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//osd_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				m_vram_fill_pending = 1;
				m_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8)); // source (byte offset)
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8)); // length in bytes
			//osd_printf_debug("setting vram copy mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);

			if (MEGADRIVE_REG01_DMA_ENABLE) insta_vram_copy(source, length);
		}
	}
	else if (m_vdp_code==0x23)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//osd_printf_debug("68k->CRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, m_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) insta_68k_to_cram_dma(source,length);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//osd_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				m_vram_fill_pending = 1;
				m_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			osd_printf_debug("setting vram copy (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
	}
	else if (m_vdp_code==0x25)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//osd_printf_debug("68k->VSRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, m_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) insta_68k_to_vsram_dma(source,length);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//osd_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				m_vram_fill_pending = 1;
				m_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			osd_printf_debug("setting vram copy (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
	}
	else if (m_vdp_code==0x30)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0)
		{
			osd_printf_debug("setting vram 68k->vram (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x1)
		{
			osd_printf_debug("setting vram 68k->vram (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			osd_printf_debug("setting vram fill (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8)); // source (byte offset)
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8)); // length in bytes
			//osd_printf_debug("setting vram copy mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);

			if (MEGADRIVE_REG01_DMA_ENABLE) insta_vram_copy(source, length);
		}
	}
}

void sega315_5313_device::ctrl_port_w(int data)
{
//  logerror("write to vdp control port %04x\n",data);
	m_vram_fill_pending = 0; // ??

	if (m_command_pending)
	{
		/* 2nd part of 32-bit command */
		m_command_pending = 0;
		m_command_part2 = data;

		update_code_and_address();
		handle_dma_bits();

		//logerror("VDP Write Part 2 setting Code %02x Address %04x\n",m_vdp_code, m_vdp_address);

	}
	else
	{
		if ((data & 0xc000) == 0x8000)
		{   /* Register Setting Command */
			int regnum = (data & 0x3f00) >> 8;
			int value  = (data & 0x00ff);

			if (regnum &0x20) osd_printf_debug("reg error\n");

			vdp_set_register(regnum & 0x1f, value);
			m_vdp_code = 0;
			m_vdp_address = 0;
		}
		else
		{
			m_command_pending = 1;
			m_command_part1 = data;
			update_code_and_address();
			//logerror("VDP Write Part 1 setting Code %02x Address %04x\n",m_vdp_code, m_vdp_address);
		}

	}
}

WRITE16_MEMBER( sega315_5313_device::vdp_w )
{
	switch (offset<<1)
	{
		case 0x00:
		case 0x02:
			if (!ACCESSING_BITS_8_15)
			{
				data = (data&0x00ff) | data<<8;
			//  osd_printf_debug("8-bit write VDP data port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			}
			else if (!ACCESSING_BITS_0_7)
			{
				data = (data&0xff00) | data>>8;
			//  osd_printf_debug("8-bit write VDP data port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			}
			data_port_w(data);
			break;

		case 0x04:
		case 0x06:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) osd_printf_debug("8-bit write VDP control port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			ctrl_port_w(data);
			break;

		case 0x08:
		case 0x0a:
		case 0x0c:
		case 0x0e:
			logerror("Attempt to Write to HV counters!!\n");
			break;

		case 0x10:
		case 0x12:
		case 0x14:
		case 0x16:
		{
			// accessed by either segapsg_device or sn76496_device
			sn76496_base_device *sn = machine().device<sn76496_base_device>(":snsnd");
			if (ACCESSING_BITS_0_7) sn->write(space, 0, data & 0xff);
			//if (ACCESSING_BITS_8_15) sn->write(space, 0, (data>>8) & 0xff);
			break;
		}

		default:
		osd_printf_debug("write to unmapped vdp port\n");
	}
}

UINT16 sega315_5313_device::vdp_vram_r(void)
{
	return MEGADRIV_VDP_VRAM((m_vdp_address&0xfffe)>>1);
}

UINT16 sega315_5313_device::vdp_vsram_r(void)
{
	return m_vsram[(m_vdp_address&0x7e)>>1];
}

UINT16 sega315_5313_device::vdp_cram_r(void)
{
	return m_cram[(m_vdp_address&0x7e)>>1];
}

UINT16 sega315_5313_device::data_port_r()
{
	UINT16 retdata=0;

	//return machine().rand();

	m_command_pending = 0;

	switch (m_vdp_code & 0x000f)
	{
		case 0x0000:
			retdata = vdp_vram_r();
			m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			m_vdp_address&=0xffff;
			break;

		case 0x0001:
			logerror("Attempting to READ from DATA PORT in VRAM WRITE MODE\n");
			retdata = machine().rand();
			break;

		case 0x0003:
			logerror("Attempting to READ from DATA PORT in CRAM WRITE MODE\n");
			retdata = machine().rand();
			break;

		case 0x0004:
			retdata = vdp_vsram_r();
			m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			m_vdp_address&=0xffff;
			break;

		case 0x0005:
			logerror("Attempting to READ from DATA PORT in VSRAM WRITE MODE\n");
			break;

		case 0x0008:
			retdata = vdp_cram_r();
			m_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			m_vdp_address&=0xffff;
			break;

		default:
			logerror("Attempting to READ from DATA PORT in #UNDEFINED# MODE\n");
			retdata = machine().rand();
			break;
	}

//  osd_printf_debug("vdp_data_port_r %04x %04x %04x\n",m_vdp_code, m_vdp_address, retdata);

//  logerror("Read VDP Data Port\n");
	return retdata;
}

/*

 NTSC, 256x224
 -------------

 Lines  Description

 224    Active display
 8      Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 11     Top border

 V counter values
 00-EA, E5-FF

PAL, 256x224
 ------------

 Lines  Description

 224    Active display
 32     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 38     Top border

 V counter values
 00-FF, 00-02, CA-FF

 PAL, 256x240
 ------------

 Lines  Description

 240    Active display
 24     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 30     Top border

 V counter values
 00-FF, 00-0A, D2-FF



 Pixels H.Cnt   Description
  256 : 00-7F : Active display
   15 : 80-87 : Right border
    8 : 87-8B : Right blanking
   26 : 8B-ED : Horizontal sync
    2 : ED-EE : Left blanking
   14 : EE-F5 : Color burst
    8 : F5-F9 : Left blanking
   13 : F9-FF : Left border

*/



UINT16 sega315_5313_device::ctrl_port_r()
{
	/* Battletoads is very fussy about the vblank flag
	   it wants it to be 1. in scanline 224 */

	/* Double Dragon 2 is very sensitive to hblank timing */
	/* xperts is very fussy too */

	/* Game no Kanzume Otokuyou (J) [!] is also fussy
	  - it cares about the bits labeled always 0, always 1.. (!)
	 */

	/* Megalo Mania also fussy - cares about pending flag*/

	int sprite_overflow = 0;
	int odd_frame = m_imode_odd_frame^1;
	int hblank_flag = 0;
	int dma_active = 0;
	int vblank = m_vblank_flag;
	int fifo_empty = 1;
	int fifo_full = 0;

	UINT16 hpos = get_hposition();

	if (hpos>400) hblank_flag = 1;
	if (hpos>460) hblank_flag = 0;

	/* extra case */
	if (MEGADRIVE_REG01_DISP_ENABLE==0) vblank = 1;

/*

// these aren't *always* 0/1 some of them are open bus return
 d15 - Always 0
 d14 - Always 0
 d13 - Always 1
 d12 - Always 1

 d11 - Always 0
 d10 - Always 1
 d9  - FIFO Empty
 d8  - FIFO Full

 d7  - Vertical interrupt pending
 d6  - Sprite overflow on current scan line
 d5  - Sprite collision
 d4  - Odd frame

 d3  - Vertical blanking
 d2  - Horizontal blanking
 d1  - DMA in progress
 d0  - PAL mode flag
*/

	return (1<<13) | // ALWAYS 1
			(1<<12) | // ALWAYS 1
			(1<<10) | // ALWAYS 1
			(fifo_empty<<9 ) | // FIFO EMPTY
			(fifo_full<<8 ) | // FIFO FULL
			(m_irq6_pending << 7) | // exmutants has a tight loop checking this ..
			(sprite_overflow << 6) |
			(m_sprite_collision << 5) |
			(odd_frame << 4) |
			(vblank << 3) |
			(hblank_flag << 2) |
			(dma_active << 1 ) |
			(m_vdp_pal << 0); // PAL MODE FLAG checked by striker for region prot..
}

static const UINT8 vc_ntsc_224[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,/**/0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
	0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static const UINT8 vc_ntsc_240[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

static const UINT8 vc_pal_224[] =
{
	0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2,    0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	0x00, 0x01, 0x02,/**/0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
	0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
	0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static const UINT8 vc_pal_240[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,    0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa,    0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,/**/0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
	0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1,    0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
	0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,    0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};


UINT16 sega315_5313_device::get_hposition()
{
	UINT16 value4;

	if (!m_use_alt_timing)
	{
		attotime time_elapsed_since_megadriv_scanline_timer;

		time_elapsed_since_megadriv_scanline_timer = m_megadriv_scanline_timer->time_elapsed();

		if (time_elapsed_since_megadriv_scanline_timer.attoseconds() < (ATTOSECONDS_PER_SECOND/m_framerate /m_total_scanlines))
		{
			value4 = (UINT16)(MAX_HPOSITION*((double)(time_elapsed_since_megadriv_scanline_timer.attoseconds()) / (double)(ATTOSECONDS_PER_SECOND/m_framerate /m_total_scanlines)));
		}
		else /* in some cases (probably due to rounding errors) we get some stupid results (the odd huge value where the time elapsed is much higher than the scanline time??!).. hopefully by clamping the result to the maximum we limit errors */
		{
			value4 = MAX_HPOSITION;
		}

	}
	else
	{
		value4 = m_screen->hpos();
	}

	return value4;
}

int sega315_5313_device::get_scanline_counter()
{
	if (!m_use_alt_timing)
		return m_scanline_counter;
	else
		return m_screen->vpos();
}


UINT16 sega315_5313_device::megadriv_read_hv_counters()
{
	/* Bubble and Squeek wants vcount=0xe0 */
	/* Dracula is very sensitive to this */
	/* Marvel Land is sensitive to this */

	int vpos = get_scanline_counter();
	UINT16 hpos = get_hposition();

//  if (hpos>424) vpos++; // fixes dracula, breaks road rash
	if (hpos>460) vpos++; // when does vpos increase.. also on sms, check game gear manual..

	/* shouldn't happen.. */
	if (vpos<0)
	{
		vpos = m_total_scanlines;
		osd_printf_debug("negative vpos?!\n");
	}

	if (MEGADRIVE_REG01_240_LINE)
	{
		assert(vpos % m_total_scanlines < (m_vdp_pal ? sizeof(vc_pal_240) : sizeof(vc_ntsc_240)));
		vpos = m_vdp_pal ? vc_pal_240[vpos % m_total_scanlines] : vc_ntsc_240[vpos % m_total_scanlines];
	}
	else
	{
		assert(vpos % m_total_scanlines < (m_vdp_pal ? sizeof(vc_pal_224) : sizeof(vc_ntsc_224)));
		vpos = m_vdp_pal ? vc_pal_224[vpos % m_total_scanlines] : vc_ntsc_224[vpos % m_total_scanlines];
	}

	if (hpos>0xf7) hpos -=0x49;

	return ((vpos&0xff)<<8)|(hpos&0xff);

}

READ16_MEMBER( sega315_5313_device::vdp_r )
{
	UINT16 retvalue = 0;



	switch (offset<<1)
	{
		case 0x00:
		case 0x02:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) osd_printf_debug("8-bit VDP read data port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = data_port_r();
			break;

		case 0x04:
		case 0x06:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) osd_printf_debug("8-bit VDP read control port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = ctrl_port_r();
		//  retvalue = machine().rand();
		//  osd_printf_debug("%06x: Read Control Port at scanline %d hpos %d (return %04x)\n",space.device().safe_pc(),get_scanline_counter(), get_hposition(),retvalue);
			break;

		case 0x08:
		case 0x0a:
		case 0x0c:
		case 0x0e:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) osd_printf_debug("8-bit VDP read HV counter port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_read_hv_counters();
		//  retvalue = machine().rand();
		//  osd_printf_debug("%06x: Read HV counters at scanline %d hpos %d (return %04x)\n",space.device().safe_pc(),get_scanline_counter(), get_hposition(),retvalue);
			break;

		case 0x10:
		case 0x12:
		case 0x14:
		case 0x16:
			logerror("Attempting to read PSG!\n");
			retvalue = 0;
			break;
	}
	return retvalue;
}



// line length = 342

/*
 The V counter counts up from 00h to EAh, then it jumps back to E5h and
 continues counting up to FFh. This allows it to cover the entire 262 line
 display.

 The H counter counts up from 00h to E9h, then it jumps back to 93h and
 continues counting up to FFh. This allows it to cover an entire 342 pixel
 line.
*/

/*

 - The 80th sprite has been drawn in 40-cell mode.
 - The 64th sprite has been drawn in 32-cell mode.
 - Twenty sprites on the same scanline have been drawn in 40 cell mode.
 - Sixteen sprites on the same scanline have been drawn in 32 cell mode.
 - 320 pixels worth of sprite data has been drawn on the same scanline
   in 40 cell mode.
 - 256 pixels worth of sprite data has been drawn on the same scanline
   in 32 cell mode.
 - The currently drawn sprite has a link field of zero.

*/

/*

 $05 - Sprite Attribute Table Base Address
 -----------------------------------------

 Bits 6-0 of this register correspond to bits A15-A09 of the sprite
 attribute table.

 In 40-cell mode, A09 is always forced to zero.

*/

void sega315_5313_device::render_spriteline_to_spritebuffer(int scanline)
{
	int screenwidth;
	int maxsprites=0;
	int maxpixels=0;
	UINT16 base_address=0;



	screenwidth = MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1);

	switch (screenwidth&3)
	{
		case 0: maxsprites = 64; maxpixels = 256; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9; break;
		case 1: maxsprites = 64; maxpixels = 256; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9; break;
		case 2: maxsprites = 80; maxpixels = 320; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9; break;
		case 3: maxsprites = 80; maxpixels = 320; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9; break;
	}


	/* Clear our Render Buffer */
	memset(m_sprite_renderline.get(), 0, 1024);


	{
		int spritenum;
		int ypos,xpos,addr;
		int drawypos;
		int /*drawwidth,*/ drawheight;
		int spritemask = 0;
		UINT8 height,width,link,xflip,yflip,colour,pri;

		/* Get Sprite Attribs */
		spritenum = 0;

		//if (scanline==40) osd_printf_debug("spritelist start base %04x\n",base_address);

		do
		{
			//UINT16 value1,value2,value3,value4;

			//value1 = m_vram[((base_address>>1)+spritenum*4)+0x0];
			//value2 = m_vram[((base_address>>1)+spritenum*4)+0x1];
			//value3 = m_vram[((base_address>>1)+spritenum*4)+0x2];
			//value4 = m_vram[((base_address>>1)+spritenum*4)+0x3];

			ypos  = (m_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x01ff)>>0; /* 0x03ff? */ // puyo puyo requires 0x1ff mask, not 0x3ff, see speech bubble corners
			height= (m_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x0300)>>8;
			width = (m_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x0c00)>>10;
			link  = (m_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x007f)>>0;
			xpos  = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x3) & 0x01ff)>>0; /* 0x03ff? */ // pirates gold has a sprite with co-ord 0x200...

			if(m_imode == 3)
			{
				ypos  = (m_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x03ff)>>0; /* 0x3ff requried in interlace mode (sonic 2 2 player) */
				drawypos = ypos - 256;
				drawheight = (height+1)*16;
			}
			else
			{
				ypos  = (m_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x01ff)>>0; /* 0x03ff? */ // puyo puyo requires 0x1ff mask, not 0x3ff, see speech bubble corners
				drawypos = ypos - 128;
				drawheight = (height+1)*8;
			}



			//if (scanline==40) osd_printf_debug("xpos %04x ypos %04x\n",xpos,ypos);

			if ((drawypos<=scanline) && ((drawypos+drawheight)>scanline))
			{
				addr  = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x07ff)>>0;
				xflip = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x0800)>>11;
				yflip = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x1000)>>12;
				colour= (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x6000)>>13;
				pri   = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x8000)>>15;

				if(m_imode == 3)
				{
					addr<<=1;
					addr &=0x7ff;
				}

				//drawwidth = (width+1)*8;
				if (pri==1) pri = 0x80;
				else pri = 0x40;

				/* todo: fix me, I'm sure this isn't right but sprite 0 + other sprite seem to do something..
				   maybe spritemask|=2 should be set for anything < 0x40 ?*/
				if (xpos==0x00) spritemask|=1;

				//if (xpos==0x01) spritemask|=2;
				//if (xpos==0x04) spritemask|=2;  // sonic 2 title screen
				//if (xpos==0x08) spritemask|=2;  // rocket night adventures
				//if (xpos==0x10) spritemask|=2;  // mercs l1 boss
				//if (xpos==0x0a) spritemask|=2;  // legend of galahad
				//if (xpos==0x21) spritemask|=2;  // shadow of the beast?
				if ((xpos>0) && (xpos<0x40)) spritemask|=2;

				if (spritemask==0x3)
					return;
				/* end todo: */

				{
					//int xdraw;
					int xtile;
					int yline = scanline - drawypos;

					for (xtile=0;xtile<width+1;xtile++)
					{
						int dat;

						if (!xflip)
						{
							UINT16 base_addr;
							int xxx;
							UINT32 gfxdata;
							int loopcount;

							if(m_imode == 3)
							{
								if (!yflip) base_addr = (addr<<4)+(xtile*((height+1)*(2*16)))+(yline*2);
								else base_addr = (addr<<4)+(xtile*((height+1)*(2*16)))+((((height+1)*16)-yline-1)*2);
							}
							else
							{
								if (!yflip) base_addr = (addr<<4)+(xtile*((height+1)*(2*8)))+(yline*2);
								else base_addr = (addr<<4)+(xtile*((height+1)*(2*8)))+((((height+1)*8)-yline-1)*2);
							}

							xxx = (xpos+xtile*8)&0x1ff;

							gfxdata = MEGADRIV_VDP_VRAM(base_addr+1) | (MEGADRIV_VDP_VRAM(base_addr+0)<<16);

							for(loopcount=0;loopcount<8;loopcount++)
							{
								dat = (gfxdata & 0xf0000000)>>28; gfxdata <<=4;
								if (dat) { if (!m_sprite_renderline[xxx]) { m_sprite_renderline[xxx] = dat | (colour<<4)| pri; } else { m_sprite_collision = 1; } }
								xxx++;xxx&=0x1ff;
								if (--maxpixels == 0x00) return;
							}

						}
						else
						{
							UINT16 base_addr;
							int xxx;
							UINT32 gfxdata;

							int loopcount;

							if(m_imode == 3)
							{
								if (!yflip) base_addr = (addr<<4)+(((width-xtile))*((height+1)*(2*16)))+(yline*2);
								else base_addr =      (addr<<4)+(((width-xtile))*((height+1)*(2*16)))+((((height+1)*16)-yline-1)*2);

							}
							else
							{
								if (!yflip) base_addr = (addr<<4)+(((width-xtile))*((height+1)*(2*8)))+(yline*2);
								else base_addr =        (addr<<4)+(((width-xtile))*((height+1)*(2*8)))+((((height+1)*8)-yline-1)*2);
							}

							xxx = (xpos+xtile*8)&0x1ff;

							gfxdata = MEGADRIV_VDP_VRAM(base_addr+1) | (MEGADRIV_VDP_VRAM(base_addr)<<16);

							for(loopcount=0;loopcount<8;loopcount++)
							{
								dat = (gfxdata & 0x0000000f)>>0; gfxdata >>=4;
								if (dat) { if (!m_sprite_renderline[xxx]) { m_sprite_renderline[xxx] = dat | (colour<<4)| pri; } else { m_sprite_collision = 1; } }
								xxx++;xxx&=0x1ff;
								if (--maxpixels == 0x00) return;
							}

						}
					}
				}
			}

			spritenum = link;
			maxsprites--;
		}
		while ((maxsprites>=0) && (link!=0));


	}
}

/* Clean up this function (!) */
void sega315_5313_device::render_videoline_to_videobuffer(int scanline)
{
	UINT16 base_a;
	UINT16 base_w=0;
	UINT16 base_b;

	UINT16 size;
	UINT16 hsize = 64;
	UINT16 vsize = 64;
	UINT16 window_right;
//  UINT16 window_hpos;
	UINT16 window_down;
//  UINT16 window_vpos;
	UINT16 hscroll_base;
//  UINT8  vscroll_mode;
//  UINT8  hscroll_mode;
	int window_firstline;
	int window_lastline;
	int window_firstcol;
	int window_lastcol;
	int screenwidth;
	int numcolumns = 0;
	int hscroll_a = 0;
	int hscroll_b = 0;
	int x;
	int window_hsize=0;
	int window_vsize=0;
	int window_is_bugged;
	int non_window_firstcol;
	int non_window_lastcol;
	int screenheight = MEGADRIVE_REG01_240_LINE?240:224;

	/* Clear our Render Buffer */
	for (x=0;x<320;x++)
	{
		m_video_renderline[x]=MEGADRIVE_REG07_BGCOLOUR | 0x20000; // mark as BG
	}

	memset(m_highpri_renderline.get(), 0, 320);

	/* is this line enabled? */
	if (!MEGADRIVE_REG01_DISP_ENABLE)
	{
		//osd_printf_debug("line disabled %d\n",scanline);
		return;
	}

	/* looks different? */
	if (MEGADRIVE_REG0_DISPLAY_DISABLE)
	{
		return;
	}



	base_a = MEGADRIVE_REG02_PATTERN_ADDR_A << 13;

	base_b = MEGADRIVE_REG04_PATTERN_ADDR_B << 13;
	size  = MEGADRIVE_REG10_HSCROLL_SIZE | (MEGADRIVE_REG10_VSCROLL_SIZE<<4);
	window_right = MEGADRIVE_REG11_WINDOW_RIGHT;
//  window_hpos = MEGADRIVE_REG11_WINDOW_HPOS;
	window_down = MEGADRIVE_REG12_WINDOW_DOWN;
//  window_vpos = MEGADRIVE_REG12_WINDOW_VPOS;

	screenwidth = MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1);

	switch (screenwidth)
	{
		case 0: numcolumns = 32; window_hsize = 32; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1f) << 11; break;
		case 1: numcolumns = 32; window_hsize = 32; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1f) << 11; break;
		case 2: numcolumns = 40; window_hsize = 64; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1e) << 11; break;
		case 3: numcolumns = 40; window_hsize = 64; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1e) << 11; break; // talespin cares about base mask, used for status bar
	}

	//osd_printf_debug("screenwidth %d\n",screenwidth);

	//base_w = machine().rand()&0xff;

	/* Calculate Exactly where we're going to draw the Window, and if the Window Bug applies */
	window_is_bugged = 0;
	if (window_right)
	{
		window_firstcol = MEGADRIVE_REG11_WINDOW_HPOS*16;
		window_lastcol = numcolumns*8;
		if (window_firstcol>window_lastcol) window_firstcol = window_lastcol;

		non_window_firstcol = 0;
		non_window_lastcol = window_firstcol;
	}
	else
	{
		window_firstcol = 0;
		window_lastcol = MEGADRIVE_REG11_WINDOW_HPOS*16;
		if (window_lastcol>numcolumns*8) window_lastcol = numcolumns*8;

		non_window_firstcol = window_lastcol;
		non_window_lastcol = numcolumns*8;

		if (window_lastcol!=0) window_is_bugged=1;
	}

	if (window_down)
	{
		window_firstline = MEGADRIVE_REG12_WINDOW_VPOS*8;
		window_lastline = screenheight; // 240 in PAL?
		if (window_firstline>screenheight) window_firstline = screenheight;
	}
	else
	{
		window_firstline = 0;
		window_lastline = MEGADRIVE_REG12_WINDOW_VPOS*8;
		if (window_lastline>screenheight) window_lastline = screenheight;
	}

	/* if we're on a window scanline between window_firstline and window_lastline the window is the full width of the screen */
	if (scanline>=window_firstline && scanline < window_lastline)
	{
		window_firstcol = 0; window_lastcol = numcolumns*8; // window is full-width of the screen
		non_window_firstcol = 0; non_window_lastcol=0; // disable non-window
	}


//    vscroll_mode = MEGADRIVE_REG0B_VSCROLL_MODE;
//    hscroll_mode = MEGADRIVE_REG0B_HSCROLL_MODE;
	hscroll_base = MEGADRIVE_REG0D_HSCROLL_ADDR<<10;

	switch (size)
	{
		case 0x00: hsize = 32; vsize = 32; break;
		case 0x01: hsize = 64; vsize = 32; break;
		case 0x02: hsize = 64; vsize = 1; /* osd_printf_debug("Invalid HSize! %02x\n",size);*/ break;
		case 0x03: hsize = 128;vsize = 32; break;

		case 0x10: hsize = 32; vsize = 64; break;
		case 0x11: hsize = 64; vsize = 64; break;
		case 0x12: hsize = 64; vsize = 1; /*osd_printf_debug("Invalid HSize! %02x\n",size);*/ break;
		case 0x13: hsize = 128;vsize = 32;/*osd_printf_debug("Invalid Total Size! %02x\n",size);*/break;

		case 0x20: hsize = 32; vsize = 64; osd_printf_debug("Invalid VSize!\n"); break;
		case 0x21: hsize = 64; vsize = 64; osd_printf_debug("Invalid VSize!\n"); break;
		case 0x22: hsize = 64; vsize = 1; /*osd_printf_debug("Invalid HSize & Invalid VSize!\n");*/ break;
		case 0x23: hsize = 128;vsize = 64; osd_printf_debug("Invalid VSize!\n"); break;

		case 0x30: hsize = 32; vsize = 128; break;
		case 0x31: hsize = 64; vsize = 64; /*osd_printf_debug("Invalid Total Size! %02x\n",size);*/break; // super skidmarks attempts this..
		case 0x32: hsize = 64; vsize = 1; /*osd_printf_debug("Invalid HSize & Invalid Total Size!\n");*/ break;
		case 0x33: hsize = 128;vsize = 128; osd_printf_debug("Invalid Total Size! %02x\n",size);break;
	}

	switch (MEGADRIVE_REG0B_HSCROLL_MODE)
	{
		case 0x00: // Full Screen Scroll
			hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0);
			hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1);
			break;

		case 0x01: // 'Broken' Line Scroll
			if(m_imode == 3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+((scanline>>1)&7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+((scanline>>1)&7)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline&7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline&7)*2);
			}
			break;

		case 0x02: // Cell Scroll
			if(m_imode == 3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+((scanline>>1)&~7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+((scanline>>1)&~7)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline&~7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline&~7)*2);
			}
			break;

		case 0x03: // Full Line Scroll
			if(m_imode == 3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline>>1)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline>>1)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+scanline*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+scanline*2);
			}
			break;
	}

	/* Low Priority B Tiles */
	{
		int column;
		int vscroll;

		for (column=0;column<numcolumns/2;column++)
		{   /* 20x 16x1 blocks */
			int vcolumn;
			int dpos;

			/* Get V Scroll Value for this block */

			dpos = column*16;

			{
				/* hscroll is not divisible by 8, this segment will contain 3 tiles, 1 partial, 1 whole, 1 partial */
				int hscroll_part = 8-(hscroll_b%8);
				int hcolumn;
				int tile_base;
				int tile_dat;
				int tile_addr;
				int tile_xflip;
				int tile_yflip;
				int tile_colour;
				int tile_pri;
				int dat;

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_b&0xf) vscroll = m_vsram[((column-1)*2+1)&0x3f];
					else vscroll = m_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = m_vsram[1];
				}

				hcolumn = ((column*2-1)-(hscroll_b>>3))&(hsize-1);

				if(m_imode == 3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;

				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}


				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(m_imode == 3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4); }  else m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;  if (!tile_pri) { if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4); }  else m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_b&0xf) vscroll = m_vsram[((column-1)*2+1)&0x3f];
					else vscroll = m_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = m_vsram[1];
				}

				hcolumn = ((column*2)-(hscroll_b>>3))&(hsize-1);

				if(m_imode == 3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(m_imode == 3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;

					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4); }  else m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4))&0x000f;  if (!tile_pri) { if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4); }  else m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					vscroll = m_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = m_vsram[1];
				}

				hcolumn = ((column*2+1)-(hscroll_b>>3))&(hsize-1);

				if(m_imode == 3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(m_imode == 3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}


				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4); }  else m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;  if (!tile_pri) { if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4); }  else m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
			}
		}
		/* END */
	}
	/* Low Priority A Tiles + Window(!) */

	{
		int column;
		int vscroll;

		for (column=window_firstcol/16;column<window_lastcol/16;column++)
		{
			int vcolumn;
			int dpos;

			int hcolumn;
			int tile_base;
			int tile_dat;
			int tile_addr;
			int tile_xflip;
			int tile_yflip;
			int tile_colour;
			int tile_pri;
			int dat;

			vcolumn = scanline&((window_vsize*8)-1);
			dpos = column*16;
			hcolumn = (column*2)&(window_hsize-1);

			if(m_imode == 3)
			{
				tile_base = (base_w>>1)+((vcolumn>>4)*window_hsize)+hcolumn;
			}
			else
			{
				tile_base = (base_w>>1)+((vcolumn>>3)*window_hsize)+hcolumn;
			}

			tile_base &=0x7fff;
			tile_dat = MEGADRIV_VDP_VRAM(tile_base);
			tile_xflip = (tile_dat&0x0800);
			tile_yflip = (tile_dat&0x1000);
			tile_colour =(tile_dat&0x6000)>>13;
			tile_pri = (tile_dat&0x8000)>>15;
			tile_addr = ((tile_dat&0x07ff)<<4);

			if(m_imode == 3)
			{
				tile_addr <<=1;
				tile_addr &=0x7fff;
			}

			if(m_imode == 3)
			{
				if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
				else tile_addr+=((0xf-vcolumn)&0xf)*2;
			}
			else
			{
				if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
				else tile_addr+=((7-vcolumn)&7)*2;
			}

			if (!tile_xflip)
			{
				/* 8 pixels */
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;

				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(28-(shift*4)))&0x000f;
					if (!tile_pri)
					{
						if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
			else
			{
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;
				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(shift*4) )&0x000f;
					if (!tile_pri)
					{
						if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
					}
					dpos++;

				}
			}


			hcolumn = (column*2+1)&(window_hsize-1);
			if(m_imode == 3)
			{
				tile_base = (base_w>>1)+((vcolumn>>4)*window_hsize)+hcolumn;
			}
			else
			{
				tile_base = (base_w>>1)+((vcolumn>>3)*window_hsize)+hcolumn;
			}
			tile_base &=0x7fff;
			tile_dat = MEGADRIV_VDP_VRAM(tile_base);
			tile_xflip = (tile_dat&0x0800);
			tile_yflip = (tile_dat&0x1000);
			tile_colour =(tile_dat&0x6000)>>13;
			tile_pri = (tile_dat&0x8000)>>15;
			tile_addr = ((tile_dat&0x07ff)<<4);

			if(m_imode == 3)
			{
				tile_addr <<=1;
				tile_addr &=0x7fff;
			}

			if(m_imode == 3)
			{
				if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
				else tile_addr+=((0xf-vcolumn)&0xf)*2;
			}
			else
			{
				if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
				else tile_addr+=((7-vcolumn)&7)*2;
			}

			if (!tile_xflip)
			{
				/* 8 pixels */
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;

				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(28-(shift*4)))&0x000f;
					if (!tile_pri)
					{
						if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
			else
			{
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;
				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(shift*4) )&0x000f;
					if (!tile_pri)
					{
						if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
		}

		/* Non Window Part */

		for (column=non_window_firstcol/16;column<non_window_lastcol/16;column++)
		{   /* 20x 16x1 blocks */
		//  int xx;
			int vcolumn;
			int dpos;

			dpos = column*16;

			{   /* hscroll is not divisible by 8, this segment will contain 3 tiles, 1 partial, 1 whole, 1 partial */
				int hscroll_part = 8-(hscroll_a%8);
				int hcolumn;
				int tile_base;
				int tile_dat;
				int tile_addr;
				int tile_xflip;
				int tile_yflip;
				int tile_colour;
				int tile_pri;
				int dat;

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_a&0xf) vscroll = m_vsram[((column-1)*2+0)&0x3f];
					else vscroll = m_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = m_vsram[0];
				}


				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2-1)-(hscroll_a>>3))&(hsize-1);
				else hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);

				if(m_imode == 3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
				}

				if(m_imode == 3)
				{
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}


				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(m_imode == 3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_a&0xf) vscroll = m_vsram[((column-1)*2+0)&0x3f];
					else vscroll = m_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = m_vsram[0];
				}

				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2)-(hscroll_a>>3))&(hsize-1); // not affected by bug?
				else
				{
					if ((hscroll_a&0xf)<8) hcolumn = ((column*2)-(hscroll_a>>3))&(hsize-1);
					else hcolumn = ((column*2+2)-(hscroll_a>>3))&(hsize-1);
				}


				if(m_imode == 3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);


				if(m_imode == 3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					vscroll = m_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = m_vsram[0];
				}

				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);
				else hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);

				if(m_imode == 3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}
				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(m_imode == 3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
				}

				if(m_imode == 3)
				{
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) m_video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) m_highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else m_highpri_renderline[dpos] = m_highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
			}
		}
	}
		/* END */

/* MEGADRIVE_REG0C_SHADOW_HIGLIGHT */
		/* Low Priority Sprites */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				if (m_sprite_renderline[x+128] & 0x40)
				{
					m_video_renderline[x] = m_sprite_renderline[x+128]&0x3f;
					m_video_renderline[x] |= 0x10000; // mark as sprite pixel
				}
			}
			else
			{   /* Special Shadow / Highlight processing */

				if (m_sprite_renderline[x+128] & 0x40)
				{
					UINT8 spritedata;
					spritedata = m_sprite_renderline[x+128]&0x3f;

					if ((spritedata==0x0e) || (spritedata==0x1e) || (spritedata==0x2e))
					{
						/* BUG in sprite chip, these colours are always normal intensity */
						m_video_renderline[x] = spritedata | 0x4000;
						m_video_renderline[x] |= 0x10000; // mark as sprite pixel
					}
					else if (spritedata==0x3e)
					{
						/* Everything below this is half colour, mark with 0x8000 to mark highlight' */
						m_video_renderline[x] = m_video_renderline[x]|0x8000; // spiderwebs..
					}
					else if (spritedata==0x3f)
					{
						/* This is a Shadow operator, but everything below is already low pri, no effect */
						m_video_renderline[x] = m_video_renderline[x]|0x2000;

					}
					else
					{
						m_video_renderline[x] = spritedata;
						m_video_renderline[x] |= 0x10000; // mark as sprite pixel
					}

				}
			}
		}
		/* High Priority A+B Tiles */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				/* Normal Processing */
				int dat;
				dat = m_highpri_renderline[x];

				if (dat&0x80)
				{
						if (dat&0x0f) m_video_renderline[x] = m_highpri_renderline[x]&0x3f;
				}
			}
			else
			{
				/* Shadow / Highlight Mode */
				int dat;
				dat = m_highpri_renderline[x];

				if (dat&0x80)
				{
						if (dat&0x0f) m_video_renderline[x] = (m_highpri_renderline[x]&0x3f) | 0x4000;
						else m_video_renderline[x] = m_video_renderline[x] | 0x4000; // set 'normal'
				}
			}
		}

		/* High Priority Sprites */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				/* Normal */
				if (m_sprite_renderline[x+128] & 0x80)
				{
					m_video_renderline[x] = m_sprite_renderline[x+128]&0x3f;
					m_video_renderline[x] |= 0x10000; // mark as sprite pixel
				}
			}
			else
			{
				if (m_sprite_renderline[x+128] & 0x80)
				{
					UINT8 spritedata;
					spritedata = m_sprite_renderline[x+128]&0x3f;

					if (spritedata==0x3e)
					{
						/* set flag 0x8000 to indicate highlight */
						m_video_renderline[x] = m_video_renderline[x]|0x8000;
					}
					else if (spritedata==0x3f)
					{
						/* This is a Shadow operator set shadow bit */
						m_video_renderline[x] = m_video_renderline[x]|0x2000;
					}
					else
					{
						m_video_renderline[x] = spritedata | 0x4000;
						m_video_renderline[x] |= 0x10000; // mark as sprite pixel
					}
				}
			}
		}
}


/* This converts our render buffer to real screen colours */
void sega315_5313_device::render_videobuffer_to_screenbuffer(int scanline)
{
	UINT16 *lineptr;



	if (!m_use_alt_timing)
	{
		if (scanline >= m_render_bitmap->height()) // safety, shouldn't happen now we allocate a fixed amount tho
			return;

		lineptr = &m_render_bitmap->pix16(scanline);

	}
	else
		lineptr = m_render_line.get();

	for (int x = 0; x < 320; x++)
	{
		UINT32 dat = m_video_renderline[x];

		if (!(dat & 0x20000))
			m_render_line_raw[x] = 0x100;
		else
			m_render_line_raw[x] = 0x000;


		if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
		{
			if (dat & 0x10000)
			{
				lineptr[x] = m_palette_lookup_sprite[(dat & 0x3f)];
				m_render_line_raw[x] |= (dat & 0x3f) | 0x080;
			}
			else
			{
				lineptr[x] = m_palette_lookup[(dat & 0x3f)];
				m_render_line_raw[x] |= (dat & 0x3f) | 0x040;
			}

		}
		else
		{
			/* Verify my handling.. I'm not sure all cases are correct */
			switch (dat & 0x1e000)
			{
				case 0x00000: // low priority, no shadow sprite, no highlight = shadow
				case 0x02000: // low priority, shadow sprite, no highlight = shadow
				case 0x06000: // normal pri,   shadow sprite, no highlight = shadow?
				case 0x10000: // (sprite) low priority, no shadow sprite, no highlight = shadow
				case 0x12000: // (sprite) low priority, shadow sprite, no highlight = shadow
				case 0x16000: // (sprite) normal pri,   shadow sprite, no highlight = shadow?
					lineptr[x] = m_palette_lookup_shadow[(dat & 0x3f)];
					m_render_line_raw[x] |= (dat & 0x3f) | 0x000;
					break;

				case 0x4000: // normal pri, no shadow sprite, no highlight = normal;
				case 0x8000: // low pri, highlight sprite = normal;
					lineptr[x] = m_palette_lookup[(dat & 0x3f)];
					m_render_line_raw[x] |= (dat & 0x3f) | 0x040;
					break;

				case 0x14000: // (sprite) normal pri, no shadow sprite, no highlight = normal;
				case 0x18000: // (sprite) low pri, highlight sprite = normal;
					lineptr[x] = m_palette_lookup_sprite[(dat & 0x3f)];
					m_render_line_raw[x] |= (dat & 0x3f) | 0x080;
					break;


				case 0x0c000: // normal pri, highlight set = highlight?
				case 0x1c000: // (sprite) normal pri, highlight set = highlight?
					lineptr[x] = m_palette_lookup_highlight[(dat & 0x3f)];
					m_render_line_raw[x] |= (dat & 0x3f) | 0x0c0;
					break;

				case 0x0a000: // shadow set, highlight set - not possible
				case 0x0e000: // shadow set, highlight set, normal set, not possible
				case 0x1a000: // (sprite)shadow set, highlight set - not possible
				case 0x1e000: // (sprite)shadow set, highlight set, normal set, not possible
				default:
					lineptr[x] = m_render_line_raw[x] |= (machine().rand() & 0x3f);
					break;
			}
		}
	}

	if (!m_32x_scanline_helper_func.isnull())
		m_32x_scanline_helper_func(scanline);
	if (!m_32x_scanline_func.isnull())
	{
		for (int x = 0; x < 320; x++)
			m_32x_scanline_func(x, m_video_renderline[x] & 0x20000, lineptr[x]);
	}
}

void sega315_5313_device::render_scanline()
{
	int scanline = get_scanline_counter();

	if (scanline >= 0 && scanline < m_visible_scanlines)
	{
		//if (MEGADRIVE_REG01_DMA_ENABLE==0) osd_printf_debug("off\n");
		render_spriteline_to_spritebuffer(get_scanline_counter());
		render_videoline_to_videobuffer(scanline);
		render_videobuffer_to_screenbuffer(scanline);
	}
}

void sega315_5313_device::vdp_handle_scanline_callback(int scanline)
{
/* Compensate for some rounding errors

       When the counter reaches 261 we should have reached the end of the frame, however due
       to rounding errors in the timer calculation we're not quite there.  Let's assume we are
       still in the previous scanline for now.
    */

	if (get_scanline_counter() != (m_total_scanlines - 1))
	{
		if (!m_use_alt_timing) m_scanline_counter++;
//      osd_printf_debug("scanline %d\n",get_scanline_counter());
		m_render_timer->adjust(attotime::from_usec(1));

		if (get_scanline_counter() == m_irq6_scanline)
		{
		//  osd_printf_debug("x %d",get_scanline_counter());
			m_irq6_on_timer->adjust(attotime::from_usec(6));
			m_irq6_pending = 1;
			m_vblank_flag = 1;

		}

	//  if (get_scanline_counter()==0) m_irq4counter = MEGADRIVE_REG0A_HINT_VALUE;
		// m_irq4counter = MEGADRIVE_REG0A_HINT_VALUE;

		if (get_scanline_counter()<=224)
		{
			m_irq4counter--;

			if (m_irq4counter==-1)
			{
				if (m_imode == 3) m_irq4counter = MEGADRIVE_REG0A_HINT_VALUE*2;
				else m_irq4counter=MEGADRIVE_REG0A_HINT_VALUE;

				m_irq4_pending = 1;

				if (MEGADRIVE_REG0_IRQ4_ENABLE)
				{
					m_irq4_on_timer->adjust(attotime::from_usec(1));
					//osd_printf_debug("irq4 on scanline %d reload %d\n",get_scanline_counter(),MEGADRIVE_REG0A_HINT_VALUE);
				}
			}
		}
		else
		{
			if (m_imode == 3) m_irq4counter = MEGADRIVE_REG0A_HINT_VALUE*2;
			else m_irq4counter=MEGADRIVE_REG0A_HINT_VALUE;
		}

		//if (get_scanline_counter()==0) irq4_on_timer->adjust(attotime::from_usec(2));


		if (get_scanline_counter() == m_z80irq_scanline)
		{
			m_sndirqline_callback(true);
		}
		if (get_scanline_counter() == m_z80irq_scanline + 1)
		{
			m_sndirqline_callback(false);
		}
	}
	else /* pretend we're still on the same scanline to compensate for rounding errors */
	{
		if (!m_use_alt_timing) m_scanline_counter = m_total_scanlines - 1;
	}

	// 32x interrupts!
	if (!m_32x_interrupt_func.isnull())
		m_32x_interrupt_func(get_scanline_counter(), m_irq6_scanline);
}


void sega315_5313_device::vdp_handle_eof()
{
	rectangle visarea;
	int scr_width = 320;

	m_vblank_flag = 0;
	//m_irq6_pending = 0; /* NO! (breaks warlock) */

	/* Set it to -1 here, so it becomes 0 when the first timer kicks in */
	if (!m_use_alt_timing) m_scanline_counter = -1;
	m_sprite_collision=0;//? when to reset this ..
	m_imode = MEGADRIVE_REG0C_INTERLEAVE; // can't change mid-frame..
	m_imode_odd_frame^=1;
//      m_genesis_snd_z80->set_input_line(0, CLEAR_LINE); // if the z80 interrupt hasn't happened by now, clear it..

	if (MEGADRIVE_REG01_240_LINE)
	{
		/* this is invalid in PAL! */
		m_total_scanlines = m_base_total_scanlines;
		m_visible_scanlines = 240;
		m_irq6_scanline = 240;
		m_z80irq_scanline = 240;
	}
	else
	{
		m_total_scanlines = m_base_total_scanlines;
		m_visible_scanlines = 224;
		m_irq6_scanline = 224;
		m_z80irq_scanline = 224;
	}

	if (m_imode == 3)
	{
		m_total_scanlines <<= 1;
		m_visible_scanlines <<= 1;
		m_irq6_scanline <<= 1;
		m_z80irq_scanline <<= 1;
	}


	switch (MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1))
	{
			/* note, add 240 mode + init new timings! */
		case 0:scr_width = 256;break;
		case 1:scr_width = 256;break;
		case 2:scr_width = 320;break;
		case 3:scr_width = 320;break;
	}
//      osd_printf_debug("my mode %02x", m_regs[0x0c]);

	visarea.set(0, scr_width - 1, 0, m_visible_scanlines - 1);

	m_screen->configure(480, m_total_scanlines, visarea, m_screen->frame_period().attoseconds());
}


// called at the start of each scanline
TIMER_DEVICE_CALLBACK_MEMBER( sega315_5313_device::megadriv_scanline_timer_callback )
{
	if (!m_use_alt_timing)
	{
		machine().scheduler().synchronize();
		vdp_handle_scanline_callback(param);

		m_megadriv_scanline_timer->adjust(attotime::from_hz(get_framerate()) / m_total_scanlines);
	}
	else
	{
		vdp_handle_scanline_callback(param);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( sega315_5313_device::megadriv_scanline_timer_callback_alt_timing )
{
	if (m_use_alt_timing)
	{
		if (param==0)
		{
			//printf("where are we? %d %d\n", m_screen->vpos(), screen().hpos());
			vdp_handle_eof();
			//vdp_clear_bitmap();
		}


		vdp_handle_scanline_callback(param);

		int vpos = screen().vpos();
		if (vpos > 0)
			screen().update_partial(vpos-1);
	}
}
