// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/*************************************************************************************

    SMC-777 (c) 1983 Sony

    driver by Angelo Salese

    TODO:
    - Implement SMC-70 specific features;
    - Implement GFX modes other than 160x100x4
    - ROM/RAM bankswitch, it apparently happens after one instruction prefetching.
      We currently use an hackish implementation until the MAME/MESS framework can
      support that ...
    - keyboard input is very sluggish;
    - cursor stuck in Bird Crash;
    - add mc6845 features;
    - many other missing features;

**************************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/74259.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "sound/beep.h"
#include "sound/sn76496.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

#define mc6845_h_char_total     (m_crtc_vreg[0]+1)
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4]+1)
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))

class smc777_state : public driver_device
{
public:
	smc777_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_ioctrl(*this, "ioctrl")
		, m_beeper(*this, "beeper")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void smc777(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void mc6845_w(offs_t offset, uint8_t data);
	uint8_t vram_r(offs_t offset);
	uint8_t attr_r(offs_t offset);
	uint8_t pcg_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	void attr_w(offs_t offset, uint8_t data);
	void pcg_w(offs_t offset, uint8_t data);
	uint8_t fbuf_r(offs_t offset);
	void fbuf_w(offs_t offset, uint8_t data);
	uint8_t key_r(offs_t offset);
	void key_w(offs_t offset, uint8_t data);
	void border_col_w(uint8_t data);
	uint8_t io_status_1c_r();
	uint8_t io_status_1d_r();
	void io_control_w(uint8_t data);
	void raminh_w(int state);
	void vsup_w(int state);
	void screen_lines_w(int state);
	void rgb_select_w(int state);
	void mt_on_w(int state);
	void sound_out_w(int state);
	void printer_strb_w(int state);
	void cas_out_w(int state);
	void color_mode_w(uint8_t data);
	void ramdac_w(offs_t offset, uint8_t data);
	uint8_t gcw_r();
	void gcw_w(uint8_t data);
	uint8_t smc777_mem_r(offs_t offset);
	void smc777_mem_w(offs_t offset, uint8_t data);
	uint8_t vsync_irq_status_r();
	void vsync_irq_enable_w(uint8_t data);
	void smc777_palette(palette_device &palette) const;
	uint32_t screen_update_smc777(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vsync_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);

	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	uint8_t fdc1_fast_status_r();
	void fdc1_select_w(uint8_t data);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void smc777_io(address_map &map) ATTR_COLD;
	void smc777_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_crtc;
	required_device<mb8876_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<ls259_device> m_ioctrl;
	required_device<beep_device> m_beeper;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t *m_ipl_rom = nullptr;
	std::unique_ptr<uint8_t[]> m_work_ram;
	std::unique_ptr<uint8_t[]> m_vram;
	std::unique_ptr<uint8_t[]> m_attr;
	std::unique_ptr<uint8_t[]> m_gvram;
	std::unique_ptr<uint8_t[]> m_pcg;

	uint8_t m_keyb_press = 0;
	uint8_t m_keyb_press_flag = 0;
	uint8_t m_shift_press_flag = 0;
	uint8_t m_backdrop_pen = 0;
	uint8_t m_display_reg = 0;
	uint8_t m_fdc_irq_flag = 0;
	uint8_t m_fdc_drq_flag = 0;
	struct { uint8_t r = 0, g = 0, b = 0; } m_pal;
	uint8_t m_raminh = 0, m_raminh_pending_change = 0; //bankswitch
	uint8_t m_raminh_prefetch = 0;
	uint8_t m_pal_mode = 0;
	uint8_t m_keyb_cmd = 0;
	uint8_t m_crtc_vreg[0x20]{};
	uint8_t m_crtc_addr = 0;
	bool m_vsync_idf = false;
	bool m_vsync_ief = false;
};


/* TODO: correct calculation thru mc6845 regs */
#define CRTC_MIN_X 24*8
#define CRTC_MIN_Y 4*8

void smc777_state::video_start()
{
}

uint32_t smc777_state::screen_update_smc777(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t count;

//  popmessage("%d %d %d %d",mc6845_v_char_total,mc6845_v_total_adj,mc6845_v_display,mc6845_v_sync_pos);

	bitmap.fill(m_palette->pen(m_backdrop_pen), cliprect);

	int x_width = ((m_display_reg & 0x80) >> 7);

	count = 0x0000;

	for(int yi=0;yi<8;yi++)
	{
		for(int y=0;y<200;y+=8)
		{
			for(int x=0;x<160;x++)
			{
				uint16_t color = (m_gvram[count] & 0xf0) >> 4;

				/* todo: clean this up! */
				//if(x_width)
				{
					bitmap.pix(y+yi+CRTC_MIN_Y, x*4+0+CRTC_MIN_X) = m_palette->pen(color);
					bitmap.pix(y+yi+CRTC_MIN_Y, x*4+1+CRTC_MIN_X) = m_palette->pen(color);
				}
				//else
				//{
				//  bitmap.pix(y+yi+CRTC_MIN_Y, x*2+0+CRTC_MIN_X) = m_palette->pen(color);
				//}

				color = (m_gvram[count] & 0x0f) >> 0;
				//if(x_width)
				{
					bitmap.pix(y+yi+CRTC_MIN_Y, x*4+2+CRTC_MIN_X) = m_palette->pen(color);
					bitmap.pix(y+yi+CRTC_MIN_Y, x*4+3+CRTC_MIN_X) = m_palette->pen(color);
				}
				//else
				//{
				//  bitmap.pix(y+yi+CRTC_MIN_Y, x*2+1+CRTC_MIN_X) = m_palette->pen(color);
				//}

				count++;

			}
		}
		count+= 0x60;
	}

	count = 0x0000;

	for(int y=0;y<25;y++)
	{
		for(int x=0;x<80/(x_width+1);x++)
		{
			/*
			-x-- ---- blink
			--xx x--- bg color (00 transparent, 01 white, 10 black, 11 complementary to fg color
			---- -xxx fg color
			*/
			int tile = m_vram[count];
			int color = m_attr[count] & 7;
			int bk_color = (m_attr[count] & 0x18) >> 3;
			int blink = m_attr[count] & 0x40;
			//int bk_struct[4] = { -1, 0x10, 0x11, (color & 7) ^ 8 };

			int bk_pen = -1;
			switch(bk_color & 3)
			{
				case 0: bk_pen = -1; break; //transparent
				case 1: bk_pen = 0x17; break; //white
				case 2: bk_pen = 0x10; break; //black
				case 3: bk_pen = (color ^ 0xf); break; //complementary
			}

			if(blink && m_screen->frame_number() & 0x10) //blinking, used by Dragon's Alphabet
				color = bk_pen;

			for(int yi=0;yi<8;yi++)
			{
				for(int xi=0;xi<8;xi++)
				{
					int pen = ((m_pcg[tile*8+yi]>>(7-xi)) & 1) ? (color+m_pal_mode) : bk_pen;

					if (pen != -1)
					{
						if(x_width)
						{
							bitmap.pix(y*8+CRTC_MIN_Y+yi, (x*8+xi)*2+0+CRTC_MIN_X) = m_palette->pen(pen);
							bitmap.pix(y*8+CRTC_MIN_Y+yi, (x*8+xi)*2+1+CRTC_MIN_X) = m_palette->pen(pen);
						}
						else
							bitmap.pix(y*8+CRTC_MIN_Y+yi, x*8+CRTC_MIN_X+xi) = m_palette->pen(pen);
					}
				}
			}

			// draw cursor
			if(mc6845_cursor_addr == count)
			{
				int cursor_on = 0;
				switch(mc6845_cursor_y_start & 0x60)
				{
					case 0x00: cursor_on = 1; break; //always on
					case 0x20: cursor_on = 0; break; //always off
					case 0x40: if(m_screen->frame_number() & 0x10) { cursor_on = 1; } break; //fast blink
					case 0x60: if(m_screen->frame_number() & 0x20) { cursor_on = 1; } break; //slow blink
				}

				if(cursor_on)
				{
					for(int yc=0;yc<(8-(mc6845_cursor_y_start & 7));yc++)
					{
						for(int xc=0;xc<8;xc++)
						{
							if(x_width)
							{
								bitmap.pix(y*8+CRTC_MIN_Y-yc+7, (x*8+xc)*2+0+CRTC_MIN_X) = m_palette->pen(0x7);
								bitmap.pix(y*8+CRTC_MIN_Y-yc+7, (x*8+xc)*2+1+CRTC_MIN_X) = m_palette->pen(0x7);
							}
							else
								bitmap.pix(y*8+CRTC_MIN_Y-yc+7, x*8+CRTC_MIN_X+xc) = m_palette->pen(0x7);
						}
					}
				}
			}

			(m_display_reg & 0x80) ? count+=2 : count++;
		}
	}

	return 0;
}

void smc777_state::mc6845_w(offs_t offset, uint8_t data)
{
	if(offset == 0)
	{
		m_crtc_addr = data;
		m_crtc->address_w(data);
	}
	else
	{
		m_crtc_vreg[m_crtc_addr] = data;
		m_crtc->register_w(data);
	}
}

uint8_t smc777_state::vram_r(offs_t offset)
{
	uint16_t vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return m_vram[vram_index];
}

uint8_t smc777_state::attr_r(offs_t offset)
{
	uint16_t vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return m_attr[vram_index];
}

uint8_t smc777_state::pcg_r(offs_t offset)
{
	uint16_t vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return m_pcg[vram_index];
}

void smc777_state::vram_w(offs_t offset, uint8_t data)
{
	uint16_t vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	m_vram[vram_index] = data;
}

void smc777_state::attr_w(offs_t offset, uint8_t data)
{
	uint16_t vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	m_attr[vram_index] = data;
}

void smc777_state::pcg_w(offs_t offset, uint8_t data)
{
	uint16_t vram_index;

	vram_index  = ((offset & 0x0007) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	m_pcg[vram_index] = data;

	m_gfxdecode->gfx(0)->mark_dirty(vram_index >> 3);
}

uint8_t smc777_state::fbuf_r(offs_t offset)
{
	uint16_t vram_index;

	vram_index  = ((offset & 0x007f) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	return m_gvram[vram_index];
}

void smc777_state::fbuf_w(offs_t offset, uint8_t data)
{
	uint16_t vram_index;

	vram_index  = ((offset & 0x00ff) << 8);
	vram_index |= ((offset & 0xff00) >> 8);

	m_gvram[vram_index] = data;
}


/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(smc777_state::quickload_cb)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	// Avoid loading a program if CP/M-80 is not in memory
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return std::make_pair(image_error::UNSUPPORTED, "CP/M must already be running");
	}

	// Check for sufficient RAM based on position of CPM
	const int mem_avail = 256 * prog_space.read_byte(7) + prog_space.read_byte(6) - 512;
	if (mem_avail < image.length())
		return std::make_pair(image_error::UNSPECIFIED, "Insufficient memory available");

	// Load image to the TPA (Transient Program Area)
	uint16_t quickload_size = image.length();
	for (uint16_t i = 0; i < quickload_size; i++)
	{
		uint8_t data;
		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, "Problem reading the image at offset " + std::to_string(i));
		prog_space.write_byte(i + 0x100, data);
	}

	// clear out command tail
	prog_space.write_byte(0x80, 0);
	prog_space.write_byte(0x81, 0);

	// Roughly set SP basing on the BDOS position
	m_maincpu->set_state_int(Z80_SP, mem_avail + 384);
	m_maincpu->set_pc(0x100); // start program

	return std::make_pair(std::error_condition(), std::string());
}

uint8_t smc777_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void smc777_state::fdc_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ 0xff);
}

uint8_t smc777_state::fdc1_fast_status_r()
{
	uint8_t data = 0;

	// TODO: inverted wrt documentation?
	data |= !m_fdc_drq_flag << 6;
	data |= m_fdc_irq_flag << 7;

	return data;
}

void smc777_state::fdc1_select_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// x--- ---- SIDE1: [SMC-70] side select
	// ---- --x- EXDS: [SMC-70] external drive select (0=internal, 1=external)
	// ---- ---x DS01: select floppy drive
	floppy = m_floppy[data & 1]->get_device();

	m_fdc->set_floppy(floppy);

	// no idea where the motor on signal is
	if (floppy)
		floppy->mon_w(0);

	if(data & 0xf0)
		printf("floppy access %02x\n", data);
}

void smc777_state::fdc_intrq_w(int state)
{
	m_fdc_irq_flag = state;
}

void smc777_state::fdc_drq_w(int state)
{
	m_fdc_drq_flag = state;
}

uint8_t smc777_state::key_r(offs_t offset)
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
		default:
		{
			//if(offset == 1)
			//  printf("Unknown keyboard command %02x read-back\n",m_keyb_cmd);

			return (offset == 0) ? 0x00 : (machine().rand() & 0x5);
		}
	}

	// never executed
	//return 0x00;
}

/* TODO: the packet commands strikes me as something I've already seen before, don't remember where however ... */
void smc777_state::key_w(offs_t offset, uint8_t data)
{
	if(offset == 1) //keyboard command
		m_keyb_cmd = data;
	else
	{
		// keyboard command param
	}
}

void smc777_state::border_col_w(uint8_t data)
{
	if(data & 0xf0)
		printf("Special border color enabled %02x\n",data);

	m_backdrop_pen = data & 0xf;
}


uint8_t smc777_state::io_status_1c_r()
{
	/*
	 * RES     x--- ---- Power On bit (1=reset switch)
	 * HiZ     -x-- ---- [SMC-70] no drive (always '1'?)
	 * LPH     --x- ---- [SMC-70] light pen H position
	 * CP      ---x ---- [SMC-777] color board (active low)
	 * LPV     ---x x--- [SMC-70] light pen V position
	 * ID      ---- -x-- 0=SMC-777 1=SMC-70
	 * MD      ---- --xx [SMC-70] boot mode (00=DISK; 10=ROM; 11=EXT)
	 */
	printf("System R\n");

	return 0;
}

uint8_t smc777_state::io_status_1d_r()
{
	/*
	 * TCIN    x--- ---- CMT read data
	 * HiZ     -x-- ---- [SMC-70] no drive (always '1'?)
	 * LPIN    --x- ---- [SMC-70] light pen input
	 * PR_BUSY ---x ---- printer busy
	 * PR_ACK  ---- x--- printer ACK
	 * ID      ---- -x-- 0=SMC-777 1=SMC-70
	 * MD      ---- --xx [SMC-70] boot mode (00=DISK; 10=ROM; 11=EXT)
	 */
	return 0;
}


void smc777_state::io_control_w(uint8_t data)
{
	m_ioctrl->write_bit(data & 0x07, BIT(data, 4));
}

void smc777_state::raminh_w(int state)
{
	// "ROM / RAM register change is done at the beginning of the next M1 cycle"
	m_raminh_pending_change = state ^ 1;
	m_raminh_prefetch = (uint8_t)(m_maincpu->state_int(Z80_R)) & 0x7f;
}

void smc777_state::vsup_w(int state)
{
	logerror("%s: Display %sabled\n", machine().describe_context(), state ? "dis" : "en");
}

void smc777_state::screen_lines_w(int state)
{
	logerror("%s: Screen line number %d\n", machine().describe_context(), state ? 625 : 525);
}

void smc777_state::rgb_select_w(int state)
{
	// 0=RGB 1=Component
	logerror("%s: %s video selected\n", machine().describe_context(), state ? "Component" : "RGB");
}

void smc777_state::mt_on_w(int state)
{
	logerror("%s: Cassette monitor %s\n", machine().describe_context(), state ? "on" : "off");
}

void smc777_state::sound_out_w(int state)
{
	m_beeper->set_state(state);
}

void smc777_state::printer_strb_w(int state)
{
	logerror("%s: Printer strobe %d\n", machine().describe_context(), !state);
}

void smc777_state::cas_out_w(int state)
{
	logerror("%s: Cassette write %d\n", machine().describe_context(), state);
}

void smc777_state::color_mode_w(uint8_t data)
{
	/*
	 * ---x -111 gfx palette select
	 * ---x -110 text palette select
	 * ---x -101 joy 2 out
	 * ...
	 * ---x -000 joy 2 out
	 */
	switch(data & 0x07)
	{
		case 0x06: m_pal_mode = (data & 0x10) ^ 0x10; break;
		default: printf("Color FF %02x\n",data); break;
	}
}

void smc777_state::ramdac_w(offs_t offset, uint8_t data)
{
	uint8_t pal_index;
	pal_index = (offset & 0xf00) >> 8;

	if(data & 0x0f)
		printf("RAMdac used with data bits 0-3 set (%02x)\n",data);

	switch((offset & 0x3000) >> 12)
	{
		case 0: m_pal.r = (data & 0xf0) >> 4; m_palette->set_pen_color(pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 1: m_pal.g = (data & 0xf0) >> 4; m_palette->set_pen_color(pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 2: m_pal.b = (data & 0xf0) >> 4; m_palette->set_pen_color(pal_index, pal4bit(m_pal.r), pal4bit(m_pal.g), pal4bit(m_pal.b)); break;
		case 3: printf("RAMdac used with gradient index = 3! pal_index = %02x data = %02x\n",pal_index,data); break;
	}
}

uint8_t smc777_state::gcw_r()
{
	return m_display_reg;
}

/* x */
void smc777_state::gcw_w(uint8_t data)
{
	/*
	 * x--- ---- text mode (0 = 80x25 1 = 40x25)
	 * -x-- ---- text page (in 40x25 mode)
	 * --x- ---- color mode (1=for 2bpp mode, blue is replaced with white)
	 * ---x ---- [SMC-70] interlace
	 * ---- xxyy gfx mode (model dependant)
	 * [SMC-70]
	 * ---- 11-- 640x400x1 bpp
	 * ---- 10-- 640x200x2 bpp
	 * ---- 01-- 320x200x4 bpp
	 * ---- 00yy 160x100x4 bpp, bits 0-1 selects page
	 * [SMC-777]
	 * ---- 1--- 640x200x2 bpp
	 * ---- 0--- 320x200x4 bpp
	 */

	m_display_reg = data;
}

uint8_t smc777_state::smc777_mem_r(offs_t offset)
{
	uint8_t z80_r;

	// TODO: do the bankswitch AFTER that the prefetch instruction is executed (hackish implementation)
	if(m_raminh_prefetch != 0xff && !machine().side_effects_disabled())
	{
		z80_r = (uint8_t)m_maincpu->state_int(Z80_R);

		if(z80_r == ((m_raminh_prefetch+1) & 0x7f))
		{
			m_raminh = m_raminh_pending_change;
			m_raminh_prefetch = 0xff;
		}
	}

	if(m_raminh == 1 && ((offset & 0xc000) == 0))
		return m_ipl_rom[offset];

	return m_work_ram[offset];
}

void smc777_state::smc777_mem_w(offs_t offset, uint8_t data)
{
	m_work_ram[offset] = data;
}

uint8_t smc777_state::vsync_irq_status_r()
{
	if (m_vsync_idf == true)
	{
		m_vsync_idf = false;
		return 1;
	}

	return 0;
}

void smc777_state::vsync_irq_enable_w(uint8_t data)
{
	if(data & 0xfe)
		logerror("Irq mask = %02x\n",data & 0xfe);

	// IRQ mask
	m_vsync_ief = BIT(data,0);
	// TODO: clear idf on 1->0 irq mask transition?
}

void smc777_state::smc777_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(smc777_state::smc777_mem_r), FUNC(smc777_state::smc777_mem_w));
}

void smc777_state::smc777_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x07).select(0xff00).rw(FUNC(smc777_state::vram_r), FUNC(smc777_state::vram_w));
	map(0x08, 0x0f).select(0xff00).rw(FUNC(smc777_state::attr_r), FUNC(smc777_state::attr_w));
	map(0x10, 0x17).select(0xff00).rw(FUNC(smc777_state::pcg_r), FUNC(smc777_state::pcg_w));
	map(0x18, 0x19).mirror(0xff00).w(FUNC(smc777_state::mc6845_w));
	map(0x1a, 0x1b).mirror(0xff00).rw(FUNC(smc777_state::key_r), FUNC(smc777_state::key_w));
	map(0x1c, 0x1c).mirror(0xff00).rw(FUNC(smc777_state::io_status_1c_r), FUNC(smc777_state::io_control_w));
	map(0x1d, 0x1d).mirror(0xff00).rw(FUNC(smc777_state::io_status_1d_r), FUNC(smc777_state::io_control_w));
//  map(0x1e, 0x1f) rs232 irq control
	map(0x20, 0x20).mirror(0xff00).rw(FUNC(smc777_state::gcw_r), FUNC(smc777_state::gcw_w));
	map(0x21, 0x21).mirror(0xff00).rw(FUNC(smc777_state::vsync_irq_status_r), FUNC(smc777_state::vsync_irq_enable_w));
//  map(0x22, 0x22) printer output data
	map(0x23, 0x23).mirror(0xff00).w(FUNC(smc777_state::border_col_w));
//  map(0x24, 0x24) rtc write address (M5M58321RS)
//  map(0x25, 0x25) rtc read
//  map(0x26, 0x26) rs232 #1
//  map(0x28, 0x2c) fdc #2 (8")
//  map(0x2d, 0x2f) rs232 #2
	map(0x30, 0x33).mirror(0xff00).rw(FUNC(smc777_state::fdc_r), FUNC(smc777_state::fdc_w));
	map(0x34, 0x34).mirror(0xff00).rw(FUNC(smc777_state::fdc1_fast_status_r), FUNC(smc777_state::fdc1_select_w));
//  map(0x35, 0x37) rs232 #3
//  map(0x38, 0x3b) cache disk unit
	// 0x38 (R) CDSTS status port (W) CDCMD command port
	// 0x39 (W) track register
	// 0x3a (W) sector register
	// 0x3b (RW) data port
//  map(0x3c, 0x3d) rgb superimposer / genlock control
//  map(0x40, 0x47) ieee-488 / TMS9914A I/F
	map(0x44, 0x44).mirror(0xff00).portr("GPDSW"); // normally unmapped in GPIB interface
//  map(0x48, 0x49) hdd (winchester)
	// TODO: address bit 8 selects joy port 2
	map(0x51, 0x51).mirror(0xff00).portr("JOY_1P").w(FUNC(smc777_state::color_mode_w));
	map(0x52, 0x52).select(0xff00).w(FUNC(smc777_state::ramdac_w));
	map(0x53, 0x53).mirror(0xff00).w("sn1", FUNC(sn76489a_device::write));
//  map(0x54, 0x59) vrt controller
//  map(0x5a, 0x5b) ram banking
//  map(0x70, 0x70) auto-start ROM (ext-ROM)
//  map(0x74, 0x74) ieee-488 GPIB ROM port
//  map(0x75, 0x75) vrt controller ROM
//  map(0x7e, 0x7f) kanji ROM
	map(0x80, 0xff).select(0xff00).rw(FUNC(smc777_state::fbuf_r), FUNC(smc777_state::fbuf_w));
}

/* Input ports */
static INPUT_PORTS_START( smc777 )
	PORT_START("key0") //0x00-0x07
	PORT_BIT (0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT (0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT (0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT (0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT (0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT (0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT (0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT (0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("key1") //0x08-0x0f
	PORT_BIT (0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT (0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
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

	PORT_START("GPDSW")
	PORT_DIPNAME( 0x01, 0x00, "GPDSW" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
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

static const uint8_t smc777_keytable[2][0xa0] =
{
	/* normal*/
	{
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* numpad */
		0x38, 0x39, 0x2f, 0x2a, 0x2d, 0x2b, 0x0d, 0x2e,
		0xff, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* A - G */
		0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, /* H - O */
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, /* P - W */
		0x78, 0x79, 0x7a, 0x2d, 0x5d, 0x60, 0xff, 0xff, /* X - Z */
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 0 - 7*/
		0x38, 0x39, 0x3b, 0x5c, 0x2c, 0x2e, 0x2f, 0xff, /* 8 - 9 */
		0x0d, 0x20, 0x08, 0x09, 0x1b, 0x0f, 0x11, 0xff,
		0x17, 0x1c, 0x16, 0x19, 0x14, 0x0e, 0x12, 0x03,
		0x01, 0x02, 0x04, 0x06, 0x0b, 0xff, 0xff, 0xff,

	},
	/* shift */
	{
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* numpad */
		0x38, 0x39, 0x2f, 0x2a, 0x2d, 0x2b, 0x0d, 0x2e,
		0xff, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
		0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, /* H - O */
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, /* P - W */
		0x58, 0x59, 0x5a, 0x5f, 0x7d, 0x7e, 0xff, 0xff, /* X - Z */
		0x29, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26,
		0x2a, 0x28, 0x3a, 0x7c, 0x3c, 0x3e, 0x3f, 0xff,
		0x0d, 0x20, 0x08, 0x09, 0x1b, 0x0f, 0x11, 0xff,
		0x17, 0x1c, 0x16, 0x19, 0x14, 0x0e, 0x12, 0x03,
		0x15, 0x18, 0x12, 0x05, 0x03, 0xff, 0xff, 0xff, /* F1 - F5 */
	}
};

TIMER_DEVICE_CALLBACK_MEMBER(smc777_state::keyboard_callback)
{
	static const char *const portnames[11] = { "key0","key1","key2","key3","key4","key5","key6","key7", "key8", "key9", "keya" };
	int i,port_i,scancode;
	uint8_t shift_mod = ioport("key_mod")->read() & 1;
	uint8_t kana_mod = ioport("key_mod")->read() & 0x10;
	scancode = 0;

	for(port_i=0;port_i<11;port_i++)
	{
		for(i=0;i<8;i++)
		{
			if((ioport(portnames[port_i])->read()>>i) & 1)
			{
				m_keyb_press = smc777_keytable[shift_mod & 1][scancode];
				if(kana_mod) { m_keyb_press|=0x80; }
				m_keyb_press_flag = 1;
				m_shift_press_flag = shift_mod & 1;
				return;
			}
			scancode++;
		}
	}
}

static const gfx_layout smc777_charlayout =
{
	8, 8,
	0x800 / 8,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

void smc777_state::machine_start()
{
	m_ipl_rom = memregion("ipl")->base();
	m_work_ram = make_unique_clear<uint8_t[]>(0x10000);
	m_vram = make_unique_clear<uint8_t[]>(0x800);
	m_attr = make_unique_clear<uint8_t[]>(0x800);
	m_gvram = make_unique_clear<uint8_t[]>(0x8000);
	m_pcg = make_unique_clear<uint8_t[]>(0x800);

	save_pointer(NAME(m_work_ram), 0x10000);
	save_pointer(NAME(m_vram), 0x800);
	save_pointer(NAME(m_attr), 0x800);
	save_pointer(NAME(m_gvram), 0x8000);
	save_pointer(NAME(m_pcg), 0x800);

	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, smc777_charlayout, m_pcg.get(), 0, 8, 0));
}

void smc777_state::machine_reset()
{
	m_raminh = 1;
	m_raminh_pending_change = 1;
	m_raminh_prefetch = 0xff;
	m_pal_mode = 0x10;
	m_vsync_idf = false;

	m_beeper->set_state(0);
}


/* set-up SMC-70 mode colors */
void smc777_state::smc777_palette(palette_device &palette) const
{
	for(int i = 0x10; i < 0x18; i++)
	{
		uint8_t const r = BIT(i, 2);
		uint8_t const g = BIT(i, 1);
		uint8_t const b = BIT(i, 0);

		palette.set_pen_color(i, pal1bit(r), pal1bit(g), pal1bit(b));
		palette.set_pen_color(i+8, rgb_t::black());
	}
}


void smc777_state::vsync_w(int state)
{
	if (state && m_vsync_ief)
	{
		m_maincpu->set_input_line(0,HOLD_LINE);
		m_vsync_idf = true;
	}
}


static void smc777_floppies(device_slot_interface &device)
{
	device.option_add("ssdd", FLOPPY_35_SSDD);
}


void smc777_state::smc777(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 32.2238_MHz_XTAL;

	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK / 8); // nominally 4.028 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &smc777_state::smc777_mem);
	m_maincpu->set_addrmap(AS_IO, &smc777_state::smc777_io);

	I8041A(config, "mcu", 6_MHz_XTAL).set_disable();

	LS259(config, m_ioctrl);
	m_ioctrl->q_out_cb<0>().set(FUNC(smc777_state::raminh_w));
	m_ioctrl->q_out_cb<1>().set(FUNC(smc777_state::vsup_w));
	m_ioctrl->q_out_cb<2>().set(FUNC(smc777_state::screen_lines_w));
	m_ioctrl->q_out_cb<3>().set(FUNC(smc777_state::rgb_select_w));
	m_ioctrl->q_out_cb<4>().set(FUNC(smc777_state::mt_on_w));
	m_ioctrl->q_out_cb<5>().set(FUNC(smc777_state::sound_out_w));
	m_ioctrl->q_out_cb<6>().set(FUNC(smc777_state::printer_strb_w));
	m_ioctrl->q_out_cb<7>().set(FUNC(smc777_state::cas_out_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(0x400, 400);
	m_screen->set_visarea(0, 660-1, 0, 220-1); //normal 640 x 200 + 20 pixels for border color
	m_screen->set_screen_update(FUNC(smc777_state::screen_update_smc777));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(smc777_state::smc777_palette), 0x20); // 16 + 8 colors (SMC-777 + SMC-70) + 8 empty entries (SMC-70)

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	HD6845S(config, m_crtc, MASTER_CLOCK / 16); // HD68A45SP; unknown clock, hand tuned to get ~60 fps
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set(FUNC(smc777_state::vsync_w));

	// floppy controller
	MB8876(config, m_fdc, MASTER_CLOCK / 32); // divider not confirmed
	m_fdc->intrq_wr_callback().set(FUNC(smc777_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(smc777_state::fdc_drq_w));

	// does it really support 16 of them?
	FLOPPY_CONNECTOR(config, m_floppy[0], smc777_floppies, "ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], smc777_floppies, "ssdd", floppy_image_device::default_mfm_floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("smc777");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(smc777_state::quickload_cb));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SN76489A(config, "sn1", MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown clock / divider

	BEEP(config, m_beeper, 300); // TODO: correct frequency
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.50);

	TIMER(config, "keyboard_timer").configure_periodic(FUNC(smc777_state::keyboard_callback), attotime::from_hz(240/32));
}

/* ROM definition */
ROM_START( smc777 )
	/* shadow ROM */
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "1st", "1st rev.")
	ROMX_LOAD( "smcrom.dat", 0x0000, 0x4000, CRC(b2520d31) SHA1(3c24b742c38bbaac85c0409652ba36e20f4687a1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2nd", "2nd rev.")
	ROMX_LOAD( "smcrom.v2",  0x0000, 0x4000, CRC(c1494b8f) SHA1(a7396f5c292f11639ffbf0b909e8473c5aa63518), ROM_BIOS(1))

	ROM_REGION( 0x400, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "m5l8041a-077p.bin", 0x000, 0x400, NO_DUMP ) // 8041 keyboard mcu, needs decapping
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   STATE         INIT        COMPANY  FULLNAME   FLAGS
COMP( 1983, smc777, 0,      0,      smc777,  smc777, smc777_state, empty_init, "Sony",  "SMC-777", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)
