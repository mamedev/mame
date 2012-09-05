/*************************************************************************************

    SMC-777 (c) 1983 Sony

    driver by Angelo Salese

    TODO:
    - no real documentation, the entire driver is just a bunch of educated
      guesses ...
    - ROM/RAM bankswitch, it apparently happens after one instruction prefetching.
      We currently use an hackish implementation until the MAME/MESS framework can
      support that ...
    - keyboard input is very sluggish;
    - cursor stuck in Bird Crash;
    - add mc6845 features;
    - many other missing features;

**************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "machine/wd17xx.h"
#include "formats/basicdsk.h"
#include "imagedev/flopdrv.h"


class smc777_state : public driver_device
{
public:
	smc777_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_sn(*this, "sn1")
	{ }
		
	optional_device<sn76489a_new_device> m_sn;
	UINT16 m_cursor_addr;
	UINT16 m_cursor_raster;
	UINT8 m_keyb_press;
	UINT8 m_keyb_press_flag;
	UINT8 m_shift_press_flag;
	UINT8 m_backdrop_pen;
	UINT8 m_display_reg;
	int m_addr_latch;
	UINT8 m_fdc_irq_flag;
	UINT8 m_fdc_drq_flag;
	UINT8 m_system_data;
	struct { UINT8 r,g,b; } m_pal;
	UINT8 m_raminh,m_raminh_pending_change; //bankswitch
	UINT8 m_raminh_prefetch;
	UINT8 m_irq_mask;
	UINT8 m_keyb_direct;
	UINT8 m_pal_mode;
	UINT8 m_keyb_cmd;
	DECLARE_WRITE8_MEMBER(smc777_6845_w);
	DECLARE_READ8_MEMBER(smc777_vram_r);
	DECLARE_READ8_MEMBER(smc777_attr_r);
	DECLARE_READ8_MEMBER(smc777_pcg_r);
	DECLARE_WRITE8_MEMBER(smc777_vram_w);
	DECLARE_WRITE8_MEMBER(smc777_attr_w);
	DECLARE_WRITE8_MEMBER(smc777_pcg_w);
	DECLARE_READ8_MEMBER(smc777_fbuf_r);
	DECLARE_WRITE8_MEMBER(smc777_fbuf_w);
	DECLARE_READ8_MEMBER(smc777_fdc1_r);
	DECLARE_WRITE8_MEMBER(smc777_fdc1_w);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_WRITE8_MEMBER(key_w);
	DECLARE_WRITE8_MEMBER(border_col_w);
	DECLARE_READ8_MEMBER(system_input_r);
	DECLARE_WRITE8_MEMBER(system_output_w);
	DECLARE_READ8_MEMBER(smc777_joystick_r);
	DECLARE_WRITE8_MEMBER(smc777_color_mode_w);
	DECLARE_WRITE8_MEMBER(smc777_ramdac_w);
	DECLARE_READ8_MEMBER(display_reg_r);
	DECLARE_WRITE8_MEMBER(display_reg_w);
	DECLARE_READ8_MEMBER(smc777_mem_r);
	DECLARE_WRITE8_MEMBER(smc777_mem_w);
	DECLARE_READ8_MEMBER(smc777_irq_mask_r);
	DECLARE_WRITE8_MEMBER(smc777_irq_mask_w);
	DECLARE_READ8_MEMBER(smc777_io_r);
	DECLARE_WRITE8_MEMBER(smc777_io_w);
};



#define CRTC_MIN_X 10
#define CRTC_MIN_Y 10

static VIDEO_START( smc777 )
{
}

static SCREEN_UPDATE_IND16( smc777 )
{
	smc777_state *state = screen.machine().driver_data<smc777_state>();
	int x,y,yi;
	UINT16 count;
	UINT8 *vram = screen.machine().root_device().memregion("vram")->base();
	UINT8 *attr = screen.machine().root_device().memregion("attr")->base();
	UINT8 *gram = state->memregion("fbuf")->base();
	int x_width;

	bitmap.fill(screen.machine().pens[state->m_backdrop_pen], cliprect);

	x_width = (state->m_display_reg & 0x80) ? 2 : 4;

	count = 0x0000;

	for(yi=0;yi<8;yi++)
	{
		for(y=0;y<200;y+=8)
		{
			for(x=0;x<160;x++)
			{
				UINT16 color;

				color = (gram[count] & 0xf0) >> 4;
				/* todo: clean this up! */
				if(x_width == 2)
				{
					bitmap.pix16(y+yi+CRTC_MIN_Y, x*2+0+CRTC_MIN_X) = screen.machine().pens[color];
				}
				else
				{
					bitmap.pix16(y+yi+CRTC_MIN_Y, x*4+0+CRTC_MIN_X) = screen.machine().pens[color];
					bitmap.pix16(y+yi+CRTC_MIN_Y, x*4+1+CRTC_MIN_X) = screen.machine().pens[color];
				}

				color = (gram[count] & 0x0f) >> 0;
				if(x_width == 2)
				{
					bitmap.pix16(y+yi+CRTC_MIN_Y, x*2+1+CRTC_MIN_X) = screen.machine().pens[color];
				}
				else
				{
					bitmap.pix16(y+yi+CRTC_MIN_Y, x*4+2+CRTC_MIN_X) = screen.machine().pens[color];
					bitmap.pix16(y+yi+CRTC_MIN_Y, x*4+3+CRTC_MIN_X) = screen.machine().pens[color];
				}

				count++;

			}
		}
		count+= 0x60;
	}

	count = 0x0000;

	x_width = (state->m_display_reg & 0x80) ? 40 : 80;

	for(y=0;y<25;y++)
	{
		for(x=0;x<x_width;x++)
		{
			/*
            -x-- ---- blink
            --xx x--- bg color (00 transparent, 01 white, 10 black, 11 complementary to fg color
            ---- -xxx fg color
            */
			int tile = vram[count];
			int color = attr[count] & 7;
			int bk_color = (attr[count] & 0x18) >> 3;
			int blink = attr[count] & 0x40;
			int xi;
			int bk_pen;
			//int bk_struct[4] = { -1, 0x10, 0x11, (color & 7) ^ 8 };

			bk_pen = -1;
			switch(bk_color & 3)
			{
				case 0: bk_pen = -1; break; //transparent
				case 1: bk_pen = 0x17; break; //white
				case 2: bk_pen = 0x10; break; //black
				case 3: bk_pen = (color ^ 0xf); break; //complementary
			}

			if(blink && screen.machine().primary_screen->frame_number() & 0x10) //blinking, used by Dragon's Alphabet
				color = bk_pen;

			for(yi=0;yi<8;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					UINT8 *gfx_data = screen.machine().root_device().memregion("pcg")->base();
					int pen;

					pen = ((gfx_data[tile*8+yi]>>(7-xi)) & 1) ? (color+state->m_pal_mode) : bk_pen;

					if(pen != -1)
						bitmap.pix16(y*8+CRTC_MIN_Y+yi, x*8+CRTC_MIN_X+xi) = screen.machine().pens[pen];
				}
			}

			// draw cursor
			if(state->m_cursor_addr == count)
			{
				int xc,yc,cursor_on;

				cursor_on = 0;
				switch(state->m_cursor_raster & 0x60)
				{
					case 0x00: cursor_on = 1; break; //always on
					case 0x20: cursor_on = 0; break; //always off
					case 0x40: if(screen.machine().primary_screen->frame_number() & 0x10) { cursor_on = 1; } break; //fast blink
					case 0x60: if(screen.machine().primary_screen->frame_number() & 0x20) { cursor_on = 1; } break; //slow blink
				}

				if(cursor_on)
				{
					for(yc=0;yc<(8-(state->m_cursor_raster & 7));yc++)
					{
						for(xc=0;xc<8;xc++)
						{
							bitmap.pix16(y*8+CRTC_MIN_Y-yc+7, x*8+CRTC_MIN_X+xc) = screen.machine().pens[0x7];
						}
					}
				}
			}

			(state->m_display_reg & 0x80) ? count+=2 : count++;
		}
	}

    return 0;
}

WRITE8_MEMBER(smc777_state::smc777_6845_w)
{
	if(offset == 0)
	{
		m_addr_latch = data;
		//mc6845_address_w(machine().device("crtc"), 0,data);
	}
	else
	{
		if(m_addr_latch == 0x0a)
			m_cursor_raster = data;
		else if(m_addr_latch == 0x0e)
			m_cursor_addr = ((data<<8) & 0x3f00) | (m_cursor_addr & 0xff);
		else if(m_addr_latch == 0x0f)
			m_cursor_addr = (m_cursor_addr & 0x3f00) | (data & 0xff);

		//mc6845_register_w(machine().device("crtc"), 0,data);
	}
}

READ8_MEMBER(smc777_state::smc777_vram_r)
{
	UINT8 *vram = memregion("vram")->base();
	UINT16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return vram[vram_index];
}

READ8_MEMBER(smc777_state::smc777_attr_r)
{
	UINT8 *attr = memregion("attr")->base();
	UINT16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return attr[vram_index];
}

READ8_MEMBER(smc777_state::smc777_pcg_r)
{
	UINT8 *pcg = memregion("pcg")->base();
	UINT16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return pcg[vram_index];
}

WRITE8_MEMBER(smc777_state::smc777_vram_w)
{
	UINT8 *vram = memregion("vram")->base();
	UINT16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	vram[vram_index] = data;
}

WRITE8_MEMBER(smc777_state::smc777_attr_w)
{
	UINT8 *attr = memregion("attr")->base();
	UINT16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	attr[vram_index] = data;
}

WRITE8_MEMBER(smc777_state::smc777_pcg_w)
{
	UINT8 *pcg = memregion("pcg")->base();
	UINT16 vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	pcg[vram_index] = data;

    gfx_element_mark_dirty(machine().gfx[0], vram_index >> 3);
}

READ8_MEMBER(smc777_state::smc777_fbuf_r)
{
	UINT8 *fbuf = memregion("fbuf")->base();
	UINT16 vram_index;

	vram_index  = ((offset & 0x007f) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return fbuf[vram_index];
}

WRITE8_MEMBER(smc777_state::smc777_fbuf_w)
{
	UINT8 *fbuf = memregion("fbuf")->base();
	UINT16 vram_index;

	vram_index  = ((offset & 0x00ff) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	fbuf[vram_index] = data;
}


static void check_floppy_inserted(running_machine &machine)
{
	int f_num;
	floppy_image_legacy *floppy;

	/* check if a floppy is there, automatically disconnect the ready line if so (HW doesn't control the ready line) */
	/* FIXME: floppy drive 1 doesn't work? */
	for(f_num=0;f_num<2;f_num++)
	{
		floppy = flopimg_get_image(floppy_get_device(machine, f_num));
		floppy_mon_w(floppy_get_device(machine, f_num), (floppy != NULL) ? 0 : 1);
		floppy_drive_set_ready_state(floppy_get_device(machine, f_num), (floppy != NULL) ? 1 : 0,0);
	}
}

READ8_MEMBER(smc777_state::smc777_fdc1_r)
{
	device_t* dev = machine().device("fdc");

	check_floppy_inserted(machine());

	switch(offset)
	{
		case 0x00:
			return wd17xx_status_r(dev,offset) ^ 0xff;
		case 0x01:
			return wd17xx_track_r(dev,offset) ^ 0xff;
		case 0x02:
			return wd17xx_sector_r(dev,offset) ^ 0xff;
		case 0x03:
			return wd17xx_data_r(dev,offset) ^ 0xff;
		case 0x04: //irq / drq status
			//popmessage("%02x %02x\n",m_fdc_irq_flag,m_fdc_drq_flag);

			return (m_fdc_irq_flag ? 0x80 : 0x00) | (m_fdc_drq_flag ? 0x00 : 0x40);
	}

	return 0x00;
}

WRITE8_MEMBER(smc777_state::smc777_fdc1_w)
{
	device_t* dev = machine().device("fdc");

	check_floppy_inserted(machine());

	switch(offset)
	{
		case 0x00:
			wd17xx_command_w(dev,offset,data ^ 0xff);
			break;
		case 0x01:
			wd17xx_track_w(dev,offset,data ^ 0xff);
			break;
		case 0x02:
			wd17xx_sector_w(dev,offset,data ^ 0xff);
			break;
		case 0x03:
			wd17xx_data_w(dev,offset,data ^ 0xff);
			break;
		case 0x04:
			// ---- xxxx select floppy drive (yes, 15 of them, A to P)
			wd17xx_set_drive(dev,data & 0x01);
			//  wd17xx_set_side(dev,(data & 0x10)>>4);
			if(data & 0xf0)
				printf("floppy access %02x\n",data);
			break;
	}
}

static WRITE_LINE_DEVICE_HANDLER( smc777_fdc_intrq_w )
{
	smc777_state *drvstate = device->machine().driver_data<smc777_state>();
	drvstate->m_fdc_irq_flag = state;
}

static WRITE_LINE_DEVICE_HANDLER( smc777_fdc_drq_w )
{
	smc777_state *drvstate = device->machine().driver_data<smc777_state>();
	drvstate->m_fdc_drq_flag = state;
}

READ8_MEMBER(smc777_state::key_r)
{
	/*
    -x-- ---- shift key
    ---- -x-- MCU data ready
    ---- ---x handshake bit?
    */

	switch(m_keyb_cmd)
	{
		case 0x00: //poll keyboard input
		{
			if(offset == 0)
				m_keyb_press_flag = 0;

			return (offset == 0) ? m_keyb_press : ((m_shift_press_flag << 6) | (m_keyb_press_flag << 2) | (m_keyb_press_flag));
		}
		break;
		default:
		{
			//if(offset == 1)
			//  printf("Unknown keyboard command %02x read-back\n",m_keyb_cmd);

			return (offset == 0) ? 0x00 : (machine().rand() & 0x5);
		}
	}

	return 0x00;
}

/* TODO: the packet commands strikes me as something I've already seen before, don't remember where however ... */
WRITE8_MEMBER(smc777_state::key_w)
{

	if(offset == 1) //keyboard command
		m_keyb_cmd = data;
	else
	{
		// keyboard command param
	}
}

WRITE8_MEMBER(smc777_state::border_col_w)
{
	if(data & 0xf0)
		printf("Special border color enabled %02x\n",data);

	m_backdrop_pen = data & 0xf;
}


READ8_MEMBER(smc777_state::system_input_r)
{

	printf("System FF R %02x\n",m_system_data & 0x0f);

	switch(m_system_data & 0x0f)
	{
		case 0x00:
			return ((m_raminh & 1) << 4); //unknown bit, Dragon's Alphabet and Bird Crush relies on this for correct colors
	}

	return m_system_data;
}



WRITE8_MEMBER(smc777_state::system_output_w)
{
	/*
    ---x 0000 ram inibit signal
    ---x 1001 beep
    all the rest is unknown at current time
    */
	m_system_data = data;
	switch(m_system_data & 0x0f)
	{
		case 0x00:
			m_raminh_pending_change = ((data & 0x10) >> 4) ^ 1;
			m_raminh_prefetch = (UINT8)(cpu_get_reg(&space.device(), Z80_R)) & 0x7f;
			break;
		case 0x02: printf("Interlace %s\n",data & 0x10 ? "on" : "off"); break;
		case 0x05: beep_set_state(machine().device(BEEPER_TAG),data & 0x10); break;
		default: printf("System FF W %02x\n",data); break;
	}
}

/* presumably SMC-777 specific */
READ8_MEMBER(smc777_state::smc777_joystick_r)
{

	return ioport("JOY_1P")->read();
}

WRITE8_MEMBER(smc777_state::smc777_color_mode_w)
{

	switch(data & 0x0f)
	{
		case 0x06: m_pal_mode = (data & 0x10) ^ 0x10; break;
		default: printf("Color FF %02x\n",data); break;
	}
}

WRITE8_MEMBER(smc777_state::smc777_ramdac_w)
{
	UINT8 pal_index;
	pal_index = (offset & 0xf00) >> 8;

	if(data & 0x0f)
		printf("RAMdac used with data bits 0-3 set (%02x)\n",data);

	switch((offset & 0x3000) >> 12)
	{
		case 0: m_pal.r = (data & 0xf0) >> 4; palette_set_color_rgb(machine(), pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 1: m_pal.g = (data & 0xf0) >> 4; palette_set_color_rgb(machine(), pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 2: m_pal.b = (data & 0xf0) >> 4; palette_set_color_rgb(machine(), pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 3: printf("RAMdac used with gradient index = 3! pal_index = %02x data = %02x\n",pal_index,data); break;
	}
}

READ8_MEMBER(smc777_state::display_reg_r)
{
	return m_display_reg;
}

/* x */
WRITE8_MEMBER(smc777_state::display_reg_w)
{
	/*
    x--- ---- width 80 / 40 switch (0 = 640 x 200 1 = 320 x 200)
    ---- -x-- mode select?
    */

	{
		if((m_display_reg & 0x80) != (data & 0x80))
		{
			rectangle visarea = machine().primary_screen->visible_area();
			int x_width;

			x_width = (data & 0x80) ? 320 : 640;

			visarea.set(0, (x_width+(CRTC_MIN_X*2)) - 1, 0, (200+(CRTC_MIN_Y*2)) - 1);

			machine().primary_screen->configure(660, 220, visarea, machine().primary_screen->frame_period().attoseconds);
		}
	}

	m_display_reg = data;
}

READ8_MEMBER(smc777_state::smc777_mem_r)
{
	UINT8 *wram = memregion("wram")->base();
	UINT8 *bios = memregion("bios")->base();
	UINT8 z80_r;

	if(m_raminh_prefetch != 0xff) //do the bankswitch AFTER that the prefetch instruction is executed (FIXME: this is an hackish implementation)
	{
		z80_r = (UINT8)cpu_get_reg(&space.device(), Z80_R);

		if(z80_r == ((m_raminh_prefetch+2) & 0x7f))
		{
			m_raminh = m_raminh_pending_change;
			m_raminh_prefetch = 0xff;
		}
	}

	if(m_raminh == 1 && ((offset & 0xc000) == 0))
		return bios[offset];

	return wram[offset];
}

WRITE8_MEMBER(smc777_state::smc777_mem_w)
{
	UINT8 *wram = memregion("wram")->base();

	wram[offset] = data;
}

READ8_MEMBER(smc777_state::smc777_irq_mask_r)
{

	return m_irq_mask;
}

WRITE8_MEMBER(smc777_state::smc777_irq_mask_w)
{

	if(data & 0xfe)
		printf("Irq mask = %02x\n",data & 0xfe);

	m_irq_mask = data & 1;
}

static ADDRESS_MAP_START(smc777_mem, AS_PROGRAM, 8, smc777_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(smc777_mem_r,smc777_mem_w)
ADDRESS_MAP_END

READ8_MEMBER(smc777_state::smc777_io_r)
{
	UINT8 low_offs;

	low_offs = offset & 0xff;

	if(low_offs <= 0x07)						  { return smc777_vram_r(space,offset & 0xff07); }
	else if(low_offs >= 0x08 && low_offs <= 0x0f) { return smc777_attr_r(space,offset & 0xff07); }
	else if(low_offs >= 0x10 && low_offs <= 0x17) { return smc777_pcg_r(space,offset & 0xff07); }
	else if(low_offs >= 0x18 && low_offs <= 0x19) { logerror("6845 read %02x",low_offs & 1); }
	else if(low_offs >= 0x1a && low_offs <= 0x1b) { return key_r(space,low_offs & 1); }
	else if(low_offs == 0x1c)					  { return system_input_r(space,0); }
	else if(low_offs == 0x1d)					  { logerror("System and control data R PC=%04x\n",cpu_get_pc(&space.device())); return 0xff; }
	else if(low_offs == 0x20)					  { return display_reg_r(space,0); }
	else if(low_offs == 0x21)					  { return smc777_irq_mask_r(space,0); }
	else if(low_offs == 0x25)					  { logerror("RTC read PC=%04x\n",cpu_get_pc(&space.device())); return 0xff; }
	else if(low_offs == 0x26)					  { logerror("RS-232c RX %04x\n",cpu_get_pc(&space.device())); return 0xff; }
	else if(low_offs >= 0x28 && low_offs <= 0x2c) { logerror("FDC 2 read %02x\n",low_offs & 7); return 0xff; }
	else if(low_offs >= 0x2d && low_offs <= 0x2f) { logerror("RS-232c no. 2 read %02x\n",low_offs & 3); return 0xff; }
	else if(low_offs >= 0x30 && low_offs <= 0x34) { return smc777_fdc1_r(space,low_offs & 7); }
	else if(low_offs >= 0x35 && low_offs <= 0x37) { logerror("RS-232c no. 3 read %02x\n",low_offs & 3); return 0xff; }
	else if(low_offs >= 0x38 && low_offs <= 0x3b) { logerror("Cache disk unit read %02x\n",low_offs & 7); return 0xff; }
	else if(low_offs >= 0x3c && low_offs <= 0x3d) { logerror("RGB superimposer read %02x\n",low_offs & 1); return 0xff; }
	else if(low_offs >= 0x40 && low_offs <= 0x47) { logerror("IEEE-488 interface unit read %02x\n",low_offs & 7); return 0xff; }
	else if(low_offs >= 0x48 && low_offs <= 0x4f) { logerror("HDD (Winchester) read %02x\n",low_offs & 1); return 0xff; } //might be 0x48 - 0x50
	else if(low_offs == 0x51)					  { return smc777_joystick_r(space,0); }
	else if(low_offs >= 0x54 && low_offs <= 0x59) { logerror("VTR Controller read %02x\n",low_offs & 7); return 0xff; }
	else if(low_offs == 0x5a || low_offs == 0x5b) { logerror("RAM Banking %02x\n",low_offs & 1); }
	else if(low_offs == 0x70)					  { logerror("Auto-start ROM read\n"); }
	else if(low_offs == 0x74)					  { logerror("IEEE-488 ROM read\n"); }
	else if(low_offs == 0x75)					  { logerror("VTR Controller ROM read\n"); }
	else if(low_offs == 0x7e || low_offs == 0x7f) { logerror("Kanji ROM read %02x\n",low_offs & 1); }
	else if(low_offs >= 0x80)					  { return smc777_fbuf_r(space,offset & 0xff7f); }

	logerror("Undefined read at %04x offset = %02x\n",cpu_get_pc(&space.device()),low_offs);
	return 0xff;
}

WRITE8_MEMBER(smc777_state::smc777_io_w)
{
	UINT8 low_offs;

	low_offs = offset & 0xff;

	if(low_offs <= 0x07)						  { smc777_vram_w(space,offset & 0xff07,data); }
	else if(low_offs >= 0x08 && low_offs <= 0x0f) { smc777_attr_w(space,offset & 0xff07,data); }
	else if(low_offs >= 0x10 && low_offs <= 0x17) { smc777_pcg_w(space,offset & 0xff07,data); }
	else if(low_offs >= 0x18 && low_offs <= 0x19) { smc777_6845_w(space,low_offs & 1,data); }
	else if(low_offs == 0x1a || low_offs == 0x1b) { key_w(space,low_offs & 1,data); }
	else if(low_offs == 0x1c)					  { system_output_w(space,0,data); }
	else if(low_offs == 0x1d)					  { logerror("Printer status / strobe write %02x\n",data); }
	else if(low_offs == 0x1e || low_offs == 0x1f) { logerror("RS-232C irq control [%02x] %02x\n",low_offs & 1,data); }
	else if(low_offs == 0x20)					  { display_reg_w(space,0,data); }
	else if(low_offs == 0x21)					  { smc777_irq_mask_w(space,0,data); }
	else if(low_offs == 0x22)					  { logerror("Printer output data %02x\n",data); }
	else if(low_offs == 0x23)					  { border_col_w(space,0,data); }
	else if(low_offs == 0x24)					  { logerror("RTC write / specify address %02x\n",data); }
	else if(low_offs == 0x26)					  { logerror("RS-232c TX %02x\n",data); }
	else if(low_offs >= 0x28 && low_offs <= 0x2c) { logerror("FDC 2 write %02x %02x\n",low_offs & 7,data); }
	else if(low_offs >= 0x2d && low_offs <= 0x2f) { logerror("RS-232c no. 2 write %02x %02x\n",low_offs & 3,data); }
	else if(low_offs >= 0x30 && low_offs <= 0x34) { smc777_fdc1_w(space,low_offs & 7,data); }
	else if(low_offs >= 0x35 && low_offs <= 0x37) { logerror("RS-232c no. 3 write %02x %02x\n",low_offs & 3,data); }
	else if(low_offs >= 0x38 && low_offs <= 0x3b) { logerror("Cache disk unit write %02x %02x\n",low_offs & 7,data); }
	else if(low_offs >= 0x3c && low_offs <= 0x3d) { logerror("RGB superimposer write %02x %02x\n",low_offs & 1,data); }
	else if(low_offs >= 0x40 && low_offs <= 0x47) { logerror("IEEE-488 interface unit write %02x %02x\n",low_offs & 7,data); }
	else if(low_offs >= 0x48 && low_offs <= 0x4f) { logerror("HDD (Winchester) write %02x %02x\n",low_offs & 1,data); } //might be 0x48 - 0x50
	else if(low_offs == 0x51)					  { smc777_color_mode_w(space,0,data); }
	else if(low_offs == 0x52)					  { smc777_ramdac_w(space,offset & 0xff00,data); }
	else if(low_offs == 0x53)					  { m_sn->write(space,0,data); }
	else if(low_offs >= 0x54 && low_offs <= 0x59) { logerror("VTR Controller write [%02x] %02x\n",low_offs & 7,data); }
	else if(low_offs == 0x5a || low_offs == 0x5b) { logerror("RAM Banking write [%02x] %02x\n",low_offs & 1,data); }
	else if(low_offs == 0x70)					  { logerror("Auto-start ROM write %02x\n",data); }
	else if(low_offs == 0x74)					  { logerror("IEEE-488 ROM write %02x\n",data); }
	else if(low_offs == 0x75)					  { logerror("VTR Controller ROM write %02x\n",data); }
	else if(low_offs == 0x7e || low_offs == 0x7f) { logerror("Kanji ROM write [%02x] %02x\n",low_offs & 1,data); }
	else if(low_offs >= 0x80)					  { smc777_fbuf_w(space,offset & 0xff7f,data); }
	else										  { logerror("Undefined write at %04x offset = %02x data = %02x\n",cpu_get_pc(&space.device()),low_offs,data); }
}

static ADDRESS_MAP_START( smc777_io , AS_IO, 8, smc777_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(smc777_io_r,smc777_io_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( smc777 )
	PORT_START("key0") //0x00-0x07
	PORT_BIT (0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)		PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT (0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)		PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT (0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)		PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT (0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)		PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT (0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)		PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT (0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)		PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT (0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)		PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT (0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)		PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("key1") //0x08-0x0f
	PORT_BIT (0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)		PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT (0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)		PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT (0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ PAD") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT (0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("* PAD") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT (0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- PAD") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT (0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ PAD") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT (0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER PAD") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)
	PORT_BIT (0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". PAD") PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("key2") //0x10-0x17
	PORT_BIT (0x01, IP_ACTIVE_HIGH,IPT_UNUSED) // PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT (0x02, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT (0x04, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT (0x08, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT (0x10, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT (0x20, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT (0x40, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT (0x80, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("key3") //0x18-0x1f
	PORT_BIT (0x01, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT (0x02, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT (0x04, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT (0x08, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT (0x10, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT (0x20, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT (0x40, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT (0x80, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("key4") //0x20-0x27
	PORT_BIT (0x01, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT (0x02, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT (0x04, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT (0x08, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT (0x10, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT (0x20, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT (0x40, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT (0x80, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("key5") //0x28-0x2f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-1") /* TODO: labels */
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-2")
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-1")
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-2")

	PORT_START("key6") //0x30-0x37
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("key7") //0x38-0x3f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-1") /* TODO: labels */
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-3")
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-1")
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-2")
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-3")

	PORT_START("key8") //0x40-0x47
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CHAR(27)
	PORT_BIT(0x20, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	/* TODO: control inputs */

	PORT_START("key9") //0x40-0x47
	/* TODO: cursor inputs */
	PORT_BIT(0x01, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CURSOR UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CURSOR DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CURSOR LEFT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CURSOR RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x20, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("END") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x40, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PGUP") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x80, IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PGDOWN") PORT_CODE(KEYCODE_PGDN)

	PORT_START("keya") //0x48-0x4f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)

	PORT_START("key_mod")
	PORT_BIT (0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT (0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KANA SHIFT") PORT_CODE(KEYCODE_LALT)


	#if 0
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')

	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_")
	#endif

	PORT_START("JOY_1P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNKNOWN ) //status?
INPUT_PORTS_END

/*
    Keyboard data:
    kana lock |= 0x80
    numpad 0 to 9 = 0x30 - 0x39
    / pad = 0x2f
    * pad = 0x2a
    - pad = 0x2d
    + pad = 0x2b
    ENTER pad = 0x0d
    . pad = 0x2e
    enter = 0x0d
    space = 0x20
    backspace = 0x08
    CTRL (TAB?) = 0x09
    cursor up = 0x17
    cursor down = 0x1c
    cursor left = 0x16
    cursor right = 0x19
    0 to 9 -> 0x30 to 0x39
    S + 0 = 0x29
    S + 1 = 0x21
    S + 2 = 0x40
    S + 3 = 0x23
    S + 4 = 0x24
    S + 5 = 0x25
    S + 6 = 0x5e
    S + 7 = 0x26
    S + 8 = 0x2a
    S + 9 = 0x28
    F1 = 0x01 / 0x15
    F2 = 0x02 / 0x18
    F3 = 0x04 / 0x12
    F4 = 0x06 / 0x05
    F5 = 0x0b / 0x03
    ESC = 0x1b
    INS = 0x0f
    DEL = 0x11
    PG UP = 0x12
    PG DOWN = 0x03
    HOME = 0x14
    END = 0x0e
    ' = 0x2d / 0x5f
    "P" row 1 ] = 0x5d / 0x7d
    "P" row 2 ' / ~ = 0x60 / 0x7e
    "L" row 1 ; / : = 0x3b / 0x3a
    "L" row 2 = (unused)
    "L" row 3 Yen / | = 0x5c / 0x7c
    "M" row 1 , / < = 0x2c / 0x3c
    "M" row 2 . / > = 0x2e / 0x3e
    "M" row 3 / / ? = 0x2f / 0x3f
*/

static const UINT8 smc777_keytable[2][0xa0] =
{
	/* normal*/
	{
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* numpad */
		0x38, 0x39, 0x2f, 0x2a, 0x2d, 0x2b, 0x0d, 0x2e,
		-1,   0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* A - G */
		0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, /* H - O */
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, /* P - W */
		0x78, 0x79, 0x7a, 0x2d, 0x5d, 0x60, -1, -1, /* X - Z */
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 0 - 7*/
		0x38, 0x39, 0x3b, 0x5c, 0x2c, 0x2e, 0x2f, -1, /* 8 - 9 */
		0x0d, 0x20, 0x08, 0x09, 0x1b, 0x0f, 0x11, -1,
		0x17, 0x1c, 0x16, 0x19, 0x14, 0x0e, 0x12, 0x03,
		0x01, 0x02, 0x04, 0x06, 0x0b, -1, -1, -1,

	},
	/* shift */
	{
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* numpad */
		0x38, 0x39, 0x2f, 0x2a, 0x2d, 0x2b, 0x0d, 0x2e,
		-1,   0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
		0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, /* H - O */
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, /* P - W */
		0x58, 0x59, 0x5a, 0x5f, 0x7d, 0x7e,   -1,   -1, /* X - Z */
		0x29, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26,
		0x2a, 0x28, 0x3a, 0x7c, 0x3c, 0x3e, 0x3f,   -1,
		0x0d, 0x20, 0x08, 0x09, 0x1b, 0x0f, 0x11,   -1,
		0x17, 0x1c, 0x16, 0x19, 0x14, 0x0e, 0x12, 0x03,
		0x15, 0x18, 0x12, 0x05, 0x03, -1, -1, -1, /* F1 - F5 */
	}
};

static TIMER_DEVICE_CALLBACK( keyboard_callback )
{
	smc777_state *state = timer.machine().driver_data<smc777_state>();
	static const char *const portnames[11] = { "key0","key1","key2","key3","key4","key5","key6","key7", "key8", "key9", "keya" };
	int i,port_i,scancode;
	UINT8 shift_mod = timer.machine().root_device().ioport("key_mod")->read() & 1;
	UINT8 kana_mod = timer.machine().root_device().ioport("key_mod")->read() & 0x10;
	scancode = 0;

	for(port_i=0;port_i<11;port_i++)
	{
		for(i=0;i<8;i++)
		{
			if((timer.machine().root_device().ioport(portnames[port_i])->read()>>i) & 1)
			{
				state->m_keyb_press = smc777_keytable[shift_mod & 1][scancode];
				if(kana_mod) { state->m_keyb_press|=0x80; }
				state->m_keyb_press_flag = 1;
				state->m_shift_press_flag = shift_mod & 1;
				return;
			}
			scancode++;
		}
	}
}

static MACHINE_START(smc777)
{
	//smc777_state *state = machine.driver_data<smc777_state>();


	beep_set_frequency(machine.device(BEEPER_TAG),300); //guesswork
	beep_set_state(machine.device(BEEPER_TAG),0);
}

static MACHINE_RESET(smc777)
{
	smc777_state *state = machine.driver_data<smc777_state>();

	state->m_raminh = 1;
	state->m_raminh_pending_change = 1;
	state->m_raminh_prefetch = 0xff;
}

static const gfx_layout smc777_charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( smc777 )
	GFXDECODE_ENTRY( "pcg", 0x0000, smc777_charlayout, 0, 8 )
GFXDECODE_END

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};

static PALETTE_INIT( smc777 )
{
	int i;

	for(i=0x10;i<0x18;i++)
	{
		UINT8 r,g,b;

		r = (i & 4) >> 2;
		g = (i & 2) >> 1;
		b = (i & 1) >> 0;

		palette_set_color_rgb(machine, i, pal1bit(r),pal1bit(g),pal1bit(b));
	}
}

static const wd17xx_interface smc777_mb8876_interface =
{
	DEVCB_NULL,
	DEVCB_LINE(smc777_fdc_intrq_w),
	DEVCB_LINE(smc777_fdc_drq_w),
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};

static LEGACY_FLOPPY_OPTIONS_START( smc777 )
	LEGACY_FLOPPY_OPTION( img, "img", "SMC70 disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([70])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface smc777_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(smc777),
	"floppy_5_25",
	NULL
};

static INTERRUPT_GEN( smc777_vblank_irq )
{
	smc777_state *state = device->machine().driver_data<smc777_state>();

	if(state->m_irq_mask)
		device_set_input_line(device,0,HOLD_LINE);
}


/*************************************
 *
 *  Sound interface
 *
 *************************************/
 
 
//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
    DEVCB_NULL
};


#define MASTER_CLOCK XTAL_4_028MHz

static MACHINE_CONFIG_START( smc777, smc777_state )
    /* basic machine hardware */
    MCFG_CPU_ADD("maincpu",Z80, MASTER_CLOCK)
    MCFG_CPU_PROGRAM_MAP(smc777_mem)
    MCFG_CPU_IO_MAP(smc777_io)
	MCFG_CPU_VBLANK_INT("screen",smc777_vblank_irq)

    MCFG_MACHINE_START(smc777)
    MCFG_MACHINE_RESET(smc777)

    /* video hardware */
    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_REFRESH_RATE(60)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
    MCFG_SCREEN_SIZE(0x400, 400)
    MCFG_SCREEN_VISIBLE_AREA(0, 660-1, 0, 220-1) //normal 640 x 200 + 20 pixels for border color
    MCFG_SCREEN_UPDATE_STATIC(smc777)

    MCFG_PALETTE_LENGTH(0x10+8) //16 palette entries + 8 special colors
    MCFG_PALETTE_INIT(smc777)
	MCFG_GFXDECODE(smc777)

	MCFG_MC6845_ADD("crtc", H46505, MASTER_CLOCK/2, mc6845_intf)	/* unknown clock, hand tuned to get ~60 fps */

    MCFG_VIDEO_START(smc777)

	MCFG_MB8876_ADD("fdc",smc777_mb8876_interface)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(smc777_floppy_interface)
	MCFG_SOFTWARE_LIST_ADD("flop_list","smc777")

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489A_NEW, MASTER_CLOCK) // unknown clock / divider
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_CONFIG(psg_intf)

	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)

	MCFG_TIMER_ADD_PERIODIC("keyboard_timer", keyboard_callback, attotime::from_hz(240/32))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( smc777 )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	/* shadow ROM */
    ROM_REGION( 0x10000, "bios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "1st", "1st rev.")
	ROMX_LOAD( "smcrom.dat", 0x0000, 0x4000, CRC(b2520d31) SHA1(3c24b742c38bbaac85c0409652ba36e20f4687a1), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "2nd", "2nd rev.")
	ROMX_LOAD( "smcrom.v2",  0x0000, 0x4000, CRC(c1494b8f) SHA1(a7396f5c292f11639ffbf0b909e8473c5aa63518), ROM_BIOS(2))

	ROM_REGION( 0x800, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i80xx", 0x000, 0x800, NO_DUMP ) // keyboard mcu, needs decapping

    ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )

    ROM_REGION( 0x800, "vram", ROMREGION_ERASE00 )

    ROM_REGION( 0x800, "attr", ROMREGION_ERASE00 )

    ROM_REGION( 0x800, "pcg", ROMREGION_ERASE00 )

    ROM_REGION( 0x8000,"fbuf", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1983, smc777,  0,       0,	smc777, 	smc777, driver_device,	 0,  "Sony",   "SMC-777",		GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND)

