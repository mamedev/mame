/**************************************************************************************

    Basic Master Level 3 (MB-6890) (c) 1980 Hitachi

    preliminary driver by Angelo Salese

    TODO:
    - keyboard is actually tied to crtc hsync timer
    - understand how to load a tape
    - some NEWON commands now makes the keyboard to not work anymore (it does if you
      soft reset)
    - video bugs for the interlaced video modes.
    - LINE command doesn't work? It says "type mismatch"

    NOTES:
    - NEWON changes the video mode, they are:
        0: 320 x 200, bit 5 active
        1: 320 x 200, bit 5 unactive
        2: 320 x 375, bit 5 active
        3: 320 x 375, bit 5 unactive
        4: 640 x 200, bit 5 active
        5: 640 x 200, bit 5 unactive
        6: 640 x 375, bit 5 active
        7: 640 x 375, bit 5 unactive
        8-15: same as above plus sets bit 4
        16-31: same as above plus shows the bar at the bottom

**************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
// #$# ? remove if not using proper mc6843?
// #include "machine/mc6843.h"
#include "sound/beep.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "sound/2203intf.h"

//#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
// #$# ? needed
// #include "formats/basicdsk.h"
#include "formats/bml3_dsk.h"
#include "machine/wd17xx.h"

class bml3_state : public driver_device
{
public:
	bml3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		//m_cass(*this, "cassette"),
		m_beep(*this, "beeper"),
		m_ym2203(*this, "ym2203")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	//required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beep;
	required_device<ym2203_device> m_ym2203;
	DECLARE_WRITE8_MEMBER(bml3_6845_w);
	DECLARE_READ8_MEMBER(bml3_keyboard_r);
	DECLARE_WRITE8_MEMBER(bml3_keyboard_w);
	DECLARE_WRITE8_MEMBER(bml3_hres_reg_w);
	DECLARE_WRITE8_MEMBER(bml3_vres_reg_w);
	DECLARE_READ8_MEMBER(bml3_vram_r);
	DECLARE_WRITE8_MEMBER(bml3_vram_w);
	DECLARE_READ8_MEMBER(bml3_wd179x_r);
	DECLARE_WRITE8_MEMBER(bml3_wd179x_w);
	DECLARE_READ8_MEMBER(bml3_mc6843_r);
	DECLARE_WRITE8_MEMBER(bml3_mc6843_w);
	DECLARE_READ8_MEMBER(bml3_fdd_r);
	DECLARE_WRITE8_MEMBER(bml3_fdd_w);
	DECLARE_READ8_MEMBER(bml3_kanji_r);
	DECLARE_WRITE8_MEMBER(bml3_kanji_w);
	DECLARE_READ8_MEMBER(bml3_psg_latch_r);
	DECLARE_WRITE8_MEMBER(bml3_psg_latch_w);
	DECLARE_READ8_MEMBER(bml3_vram_attr_r);
	DECLARE_WRITE8_MEMBER(bml3_vram_attr_w);
	DECLARE_READ8_MEMBER(bml3_beep_r);
	DECLARE_WRITE8_MEMBER(bml3_beep_w);
	DECLARE_WRITE8_MEMBER(bml3_piaA_w);
	DECLARE_READ8_MEMBER(bml3_keyb_nmi_r);
	DECLARE_WRITE8_MEMBER(bml3_firq_mask_w);
	DECLARE_READ8_MEMBER(bml3_firq_status_r);


	DECLARE_READ_LINE_MEMBER( bml3_acia_rx_r );
	DECLARE_WRITE_LINE_MEMBER( bml3_acia_tx_w );
	DECLARE_READ_LINE_MEMBER( bml3_acia_dts_r );
	DECLARE_WRITE_LINE_MEMBER( bml3_acia_rts_w );
	DECLARE_READ_LINE_MEMBER(bml3_acia_dcd_r);
	DECLARE_WRITE_LINE_MEMBER(bml3_acia_irq_w);

	DECLARE_READ8_MEMBER(bml3_a000_r); DECLARE_WRITE8_MEMBER(bml3_a000_w);
	DECLARE_READ8_MEMBER(bml3_c000_r); DECLARE_WRITE8_MEMBER(bml3_c000_w);
	DECLARE_READ8_MEMBER(bml3_e000_r); DECLARE_WRITE8_MEMBER(bml3_e000_w);
	DECLARE_READ8_MEMBER(bml3_f000_r); DECLARE_WRITE8_MEMBER(bml3_f000_w);
	DECLARE_READ8_MEMBER(bml3_fff0_r); DECLARE_WRITE8_MEMBER(bml3_fff0_w);

	UINT8 m_attr_latch;
	UINT8 m_io_latch;
	UINT8 m_hres_reg;
	UINT8 m_vres_reg;
	UINT8 m_keyb_press;
	UINT8 m_keyb_press_flag;
	// #$# change to UINT8?
	int m_keyb_interrupt_disabled;
	int m_keyb_nmi_disabled;
	int m_keyb_counter_operation_disabled;
	int m_keyb_empty_scan;
	int m_keyb_scancode;
	int m_keyb_capslock_led_on;
	int m_keyb_hiragana_led_on;
	int m_keyb_katakana_led_on;
	UINT8 *m_p_chargen;
	UINT8 m_psg_latch;
	void m6845_change_clock(UINT8 setting);
	UINT8 m_crtc_vreg[0x100],m_crtc_index;
	UINT16 m_kanji_addr;
	UINT8 *m_extram;
	UINT8 m_firq_mask,m_firq_status;

protected:
	virtual void machine_reset();

	virtual void machine_start();
	virtual void video_start();
	virtual void palette_init();
public:
	UINT32 screen_update_bml3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(bml3_irq);
	INTERRUPT_GEN_MEMBER(bml3_timer_firq);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	DECLARE_READ8_MEMBER(bml3_ym2203_r);
	DECLARE_WRITE8_MEMBER(bml3_ym2203_w);
};

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
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


void bml3_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 bml3_state::screen_update_bml3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,hf,count;
	int xi,yi;
	int width,interlace,lowres;
	int rawbits,dots[2],color,reverse,graphic;
	UINT8 *vram = memregion("vram")->base();

	count = 0x0000;

	width = (m_hres_reg & 0x80) ? 80 : 40;
	interlace = (m_vres_reg & 0x08) ? 1 : 0;
	lowres = (m_hres_reg & 0x40) ? 1 : 0;

	// redundant initializers to keep compiler happy
	rawbits = dots[0] = dots[1] = color = reverse = graphic = 0;

//  popmessage("%02x %02x",m_hres_reg,m_vres_reg);

	for(y=0;y<25;y++)
	{
		for(x=0;x<width;x++)
		{
			for(yi=0;yi<8;yi++)
			{
				if (!lowres || yi == 0) {
					int offset = count + yi * (width / 40 * 0x400) + mc6845_start_addr - 0x400;
					if (offset >= 0x0000 && offset < 0x4000) {
						rawbits = vram[offset+0x0000];
						color = vram[offset+0x4000] & 7;
						reverse = vram[offset+0x4000] & 8;
						graphic = vram[offset+0x4000] & 0x10;
					}
					else {
						// outside vram - don't know what should happen here
						rawbits = color = reverse = graphic = 0;
					}
				}
				if (graphic) {
					if (lowres) {
						// low-res graphics, each tile has 8 bits arranged as follows:
						// 4 0
						// 5 1
						// 6 2
						// 7 3
						dots[0] = dots[1] = (rawbits >> yi/2 & 0x11) * 0xf;
					}
					else {
						dots[0] = dots[1] = rawbits;
					}
				}
				else {
					// character mode
					int tile = rawbits  & 0x7f;
					int tile_bank = (rawbits & 0x80) >> 7;
					if (interlace) {
						dots[0] = m_p_chargen[(tile_bank<<11)|(tile<<4)|(yi<<1)];
						dots[1] = m_p_chargen[(tile_bank<<11)|(tile<<4)|(yi<<1)|tile_bank];
					}
					else {
						dots[0] = dots[1] = m_p_chargen[(tile<<4)|(yi<<1)|tile_bank];
					}
				}
				for(hf=0;hf<=interlace;hf++)
				{
					for(xi=0;xi<8;xi++)
					{
						int pen;

						if(reverse)
							pen = (dots[hf] >> (7-xi) & 1) ? 0 : color;
						else
							pen = (dots[hf] >> (7-xi) & 1) ? color : 0;

						bitmap.pix16((y*8+yi)*(interlace+1)+hf, x*8+xi) = pen;
					}
				}
			}

			if(mc6845_cursor_addr-mc6845_start_addr == count)
			{
				int xc,yc,cursor_on;

				cursor_on = 0;
				switch(mc6845_cursor_y_start & 0x60)
				{
					case 0x00: cursor_on = 1; break; //always on
					case 0x20: cursor_on = 0; break; //always off
					case 0x40: if(machine().primary_screen->frame_number() & 0x10) { cursor_on = 1; } break; //fast blink
					case 0x60: if(machine().primary_screen->frame_number() & 0x20) { cursor_on = 1; } break; //slow blink
				}

				if(cursor_on)
				{
					for(yc=0;yc<(8-(mc6845_cursor_y_start & 7));yc++)
					{
						for(xc=0;xc<8;xc++)
						{
							if(interlace)
							{
								bitmap.pix16((y*8+yc+7)*2+0, x*8+xc) = 7;
								bitmap.pix16((y*8+yc+7)*2+1, x*8+xc) = 7;
							}
							else
								bitmap.pix16(y*8+yc+7, x*8+xc) = 7;
						}
					}
				}
			}

			count++;
		}
	}

	return 0;
}

WRITE8_MEMBER( bml3_state::bml3_6845_w )
{
	if(offset == 0)
	{
		m_crtc_index = data;
		m_crtc->address_w(space, 0, data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		m_crtc->register_w(space, 0, data);
	}
}

READ8_MEMBER( bml3_state::bml3_keyboard_r )
{
	m_keyb_press_flag = 0;
	logerror("bml_keyboard_r %02x\n", m_keyb_press);
	// #$# not sure what to return if no key pressed, some number from 0-9?  maybe low three bits of scancode?
	return m_keyb_press;
}

WRITE8_MEMBER( bml3_state::bml3_keyboard_w )
{
	logerror("bml_keyboard_w %02x\n", data);
	m_keyb_katakana_led_on = (data & 0x01) != 0;
	m_keyb_hiragana_led_on = (data & 0x02) != 0;
	m_keyb_capslock_led_on = (data & 0x04) != 0;
	m_keyb_counter_operation_disabled = (data & 0x08) != 0;
	m_keyb_interrupt_disabled = (data & 0x40) == 0;
	m_keyb_nmi_disabled = (data & 0x80) == 0;
}

void bml3_state::m6845_change_clock(UINT8 setting)
{
	int m6845_clock = XTAL_1MHz;

	switch(setting & 0x88)
	{
		case 0x00: m6845_clock = XTAL_1MHz; break; //320 x 200
		case 0x08: m6845_clock = XTAL_1MHz*2; break; //320 x 375
		case 0x80: m6845_clock = XTAL_1MHz*2; break; //640 x 200
		case 0x88: m6845_clock = XTAL_1MHz*4; break; //640 x 375
	}

	m_crtc->set_clock(m6845_clock);
}

WRITE8_MEMBER( bml3_state::bml3_hres_reg_w )
{
	/*
	x--- ---- width (1) 80 / (0) 40
	-x-- ---- used in some modes, unknown purpose
	--x- ---- used in some modes, unknown purpose (also wants $ffc4 to be 0xff), color / monochrome switch?
	*/

	m_hres_reg = data;

	m6845_change_clock((m_hres_reg & 0x80) | (m_vres_reg & 0x08));
}

WRITE8_MEMBER( bml3_state::bml3_vres_reg_w )
{
	/*
	---- x--- char height
	*/
	m_vres_reg = data;

	m6845_change_clock((m_hres_reg & 0x80) | (m_vres_reg & 0x08));
}


READ8_MEMBER( bml3_state::bml3_vram_r )
{
	UINT8 *vram = memregion("vram")->base();

	// Bit 7 masks reading back to the latch
	if ((m_attr_latch & 0x80) == 0) {
		m_attr_latch = vram[offset+0x4000];
	}

	return vram[offset];
}

WRITE8_MEMBER( bml3_state::bml3_vram_w )
{
	UINT8 *vram = memregion("vram")->base();

	vram[offset] = data;
	// color ram is 5-bit
	vram[offset+0x4000] = m_attr_latch & 0x1F;
}

READ8_MEMBER( bml3_state::bml3_wd179x_r)
{
	device_t* dev = machine().device("fdc");
	switch(offset)
	{
		case 0:
			return ~wd17xx_status_r(dev,space,offset);
		case 1:
			return ~wd17xx_track_r(dev,space,offset);
		case 2:
			return ~wd17xx_sector_r(dev,space,offset);
		case 3:
			return ~wd17xx_data_r(dev,space,offset);
		case 4:
			return wd17xx_drq_r(dev) ? 0x00 : 0x80;
		default:
			return -1;
	}
}

WRITE8_MEMBER( bml3_state::bml3_wd179x_w)
{
	device_t* dev = machine().device("fdc");
	switch(offset)
	{
		case 0:
			wd17xx_command_w(dev,space,offset,~data);
			break;
		case 1:
			wd17xx_track_w(dev,space,offset,~data);
			break;
		case 2:
			wd17xx_sector_w(dev,space,offset,~data);
			break;
		case 3:
			wd17xx_data_w(dev,space,offset,~data);
			break;
		case 4:
			// #$# support four drives - should be & 0x03 here?
			int drive = data & 0x01;
			int side = BIT(data, 4);
			int motor = BIT(data, 3);
			wd17xx_set_drive(dev,drive);
			floppy_mon_w(floppy_get_device(machine(),drive), !motor);
			floppy_drive_set_ready_state(floppy_get_device(machine(), drive), ASSERT_LINE, 0);
			wd17xx_set_side(dev,side);
			break;
	}
}

READ8_MEMBER( bml3_state::bml3_mc6843_r)
{
	device_t* dev = machine().device("fdc");
	// #$# JME debug
	logerror("bml3_state::bml3_mc6843_r(offset=%04X)\n", offset);
	switch(offset)
	{
		case 0:
			return wd17xx_data_r(dev,space,offset);
		case 1:
			return m_io_latch;
		case 2:
			// b3 is something?
			return 0;
		case 3:
			// ???
			// return 0x04;
			// return wd17xx_status_r(dev,offset);
			return 0x04 | (wd17xx_drq_r(dev) ? 0x01 : 0x00) | (wd17xx_status_r(dev,space,offset) & 0x01 ? 0x80 : 0x00);
		case 4:
			// ???
			return 0;
		default:
			return 0;
	}
}

WRITE8_MEMBER( bml3_state::bml3_mc6843_w)
{
	device_t* dev = machine().device("fdc");
	// #$# JME debug
	logerror("bml3_state::bml3_mc6843_w(offset=%04X, data=%02X)\n", offset, data);
	switch(offset)
	{
		case 0:
			break;
		case 1:
			// ???
			m_io_latch = data;
			break;
		case 2:
			// not sure if this is wd17xx command or something a bit different...
			if (data == 0xE4)
				// read a sector
				wd17xx_command_w(dev,space,offset,0x80);
			else
				// ROM also uses data == 0xC2, not sure what for
				wd17xx_command_w(dev,space,offset,data);
			break;
		case 3:
			// ? something here, gets 0x66 written to it
		{
			// Nasty hack... code in boot sector sets an NMI handler address
			// at 010A, but it assumes a JMP opcode (7E) is already in 0109.
			// But nothing ever writes the opcode 8-(
			// The workaround here is to write the following NMI handler, which
			// simply jumps to an RTI instruction in ROM.
			// JMP $EAD5
			address_space &mem = machine().device("maincpu")->memory().space(AS_PROGRAM);
			mem.write_byte(0x0109, 0x7E);
			mem.write_byte(0x010A, 0xEA);
			mem.write_byte(0x010B, 0xD5);
		}
			break;
		case 4:
			wd17xx_sector_w(dev,space,offset,data);
			break;
		case 7:
			wd17xx_track_w(dev,space,offset,data);
			break;
		default:
			break;
	}
}

READ8_MEMBER( bml3_state::bml3_fdd_r )
{
	//printf("FDD 0xff20 R\n");
	return -1;
}

WRITE8_MEMBER( bml3_state::bml3_fdd_w )
{
	//printf("FDD 0xff20 W %02x\n",data);
	device_t* dev = machine().device("fdc");
	// ? something here, gets 0x81 written to it if latch found at FF19, or 0x00 if not
	// don't know which bits are what
	int drive = 0;
	int side = 0;
	int motor = BIT(data, 7);
	wd17xx_set_drive(dev,drive);
	floppy_mon_w(floppy_get_device(machine(),drive), motor);
	floppy_drive_set_ready_state(floppy_get_device(machine(), drive), ASSERT_LINE, 0);
	wd17xx_set_side(dev,side);
}

READ8_MEMBER( bml3_state::bml3_kanji_r )
{
//  return m_kanji_rom[m_kanji_addr << 1 + offset];
	return machine().rand();
}

WRITE8_MEMBER( bml3_state::bml3_kanji_w )
{
	m_kanji_addr &= (0xff << (offset*8));
	m_kanji_addr |= (data << ((offset^1)*8));
}

READ8_MEMBER( bml3_state::bml3_psg_latch_r)
{
	return 0x7f;
}

WRITE8_MEMBER( bml3_state::bml3_psg_latch_w)
{
	m_psg_latch = data;
}

READ8_MEMBER(bml3_state::bml3_ym2203_r)
{
	UINT8 dev_offs = ((m_psg_latch & 3) != 3);

	return m_ym2203->read(space, dev_offs);
}

WRITE8_MEMBER(bml3_state::bml3_ym2203_w)
{
	UINT8 dev_offs = ((m_psg_latch & 3) != 3);

	m_ym2203->write(space, dev_offs, data);
}

READ8_MEMBER( bml3_state::bml3_vram_attr_r)
{
	return m_attr_latch;
}

WRITE8_MEMBER( bml3_state::bml3_vram_attr_w)
{
	m_attr_latch = data;
}

READ8_MEMBER( bml3_state::bml3_beep_r)
{
	return -1; // BEEP status read?
}

WRITE8_MEMBER( bml3_state::bml3_beep_w)
{
	m_beep->set_state(!BIT(data, 7));
}

READ8_MEMBER( bml3_state::bml3_a000_r) { return m_extram[offset + 0xa000]; }
WRITE8_MEMBER( bml3_state::bml3_a000_w) { m_extram[offset + 0xa000] = data; }
READ8_MEMBER( bml3_state::bml3_c000_r) { return m_extram[offset + 0xc000]; }
WRITE8_MEMBER( bml3_state::bml3_c000_w) { m_extram[offset + 0xc000] = data; }
READ8_MEMBER( bml3_state::bml3_e000_r) { return m_extram[offset + 0xe000]; }
WRITE8_MEMBER( bml3_state::bml3_e000_w) { m_extram[offset + 0xe000] = data; }
READ8_MEMBER( bml3_state::bml3_f000_r) { return m_extram[offset + 0xf000]; }
WRITE8_MEMBER( bml3_state::bml3_f000_w) { m_extram[offset + 0xf000] = data; }
READ8_MEMBER( bml3_state::bml3_fff0_r) { return m_extram[offset + 0xfff0]; }
WRITE8_MEMBER( bml3_state::bml3_fff0_w) { m_extram[offset + 0xfff0] = data; }

READ8_MEMBER( bml3_state::bml3_keyb_nmi_r)
{
	return 0; // bit 7 used to signal a BREAK key pressure
}

WRITE8_MEMBER( bml3_state::bml3_firq_mask_w)
{
	m_firq_mask = data & 0x80;
	if(m_firq_mask)
	{
		m_firq_status = 0; // clear pending firq
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	}
}

READ8_MEMBER( bml3_state::bml3_firq_status_r )
{
	UINT8 res = m_firq_status << 7;
	m_firq_status = 0;
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return res;
}

static ADDRESS_MAP_START(bml3_mem, AS_PROGRAM, 8, bml3_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x0400, 0x43ff) AM_READWRITE(bml3_vram_r,bml3_vram_w)
	AM_RANGE(0x4400, 0x9fff) AM_RAM
	// Floppy boot block code and disk basic use this controller
	AM_RANGE(0xff00, 0xff04) AM_READWRITE(bml3_wd179x_r,bml3_wd179x_w)
	// #$# WD179X floppy controller also occupies this address space... maybe ym2203 can be relocated?
	// AM_RANGE(0xff00, 0xff00) AM_READWRITE(bml3_ym2203_r,bml3_ym2203_w)
	// AM_RANGE(0xff02, 0xff02) AM_READWRITE(bml3_psg_latch_r,bml3_psg_latch_w) // PSG address/data select
	// #$# ? is it possible to use a proper mc6843 driver here? Apparently the wd179x and mc6843 both control the same drive.
	// Disk bootstrap in ROM uses this controller to load boot blocks from disk
	AM_RANGE(0xff18, 0xff1f) AM_READWRITE(bml3_mc6843_r,bml3_mc6843_w)
	// AM_RANGE(0xff18, 0xff1f) AM_DEVREADWRITE_LEGACY("mc6843",mc6843_r,mc6843_w)
	AM_RANGE(0xff20, 0xff20) AM_READWRITE(bml3_fdd_r,bml3_fdd_w) // FDD drive select
	AM_RANGE(0xff75, 0xff76) AM_READWRITE(bml3_kanji_r,bml3_kanji_w)// kanji i/f
	AM_RANGE(0xffc0, 0xffc3) AM_DEVREADWRITE("pia6821", pia6821_device, read, write)
	AM_RANGE(0xffc4, 0xffc4) AM_DEVREADWRITE("acia6850", acia6850_device, status_read, control_write)
	AM_RANGE(0xffc5, 0xffc5) AM_DEVREADWRITE("acia6850", acia6850_device, data_read, data_write)
	AM_RANGE(0xffc6, 0xffc7) AM_WRITE(bml3_6845_w)
	AM_RANGE(0xffc8, 0xffc8) AM_READ(bml3_keyb_nmi_r) // keyboard nmi
	AM_RANGE(0xffc9, 0xffc9) AM_READ_PORT("DSW")
	AM_RANGE(0xffca, 0xffca) AM_READ(bml3_firq_status_r) // timer irq
//  AM_RANGE(0xffcb, 0xffcb) light pen flag
	AM_RANGE(0xffd0, 0xffd0) AM_WRITE(bml3_hres_reg_w) // mode select
//  AM_RANGE(0xffd1, 0xffd1) trace counter
//  AM_RANGE(0xffd2, 0xffd2) remote switch
	AM_RANGE(0xffd3, 0xffd3) AM_READWRITE(bml3_beep_r,bml3_beep_w) // music select
	AM_RANGE(0xffd4, 0xffd4) AM_WRITE(bml3_firq_mask_w)
//  AM_RANGE(0xffd5, 0xffd5) light pen
	AM_RANGE(0xffd6, 0xffd6) AM_WRITE(bml3_vres_reg_w) // interlace select
//  AM_RANGE(0xffd7, 0xffd7) baud select
	AM_RANGE(0xffd8, 0xffd8) AM_READWRITE(bml3_vram_attr_r,bml3_vram_attr_w) // attribute register
	AM_RANGE(0xffe0, 0xffe0) AM_READWRITE(bml3_keyboard_r,bml3_keyboard_w) // keyboard mode register
//  AM_RANGE(0xffe8, 0xffe8) bank register
//  AM_RANGE(0xffe9, 0xffe9) IG mode register
//  AM_RANGE(0xffea, 0xffea) IG enable register
	AM_RANGE(0xa000, 0xfeff) AM_ROM AM_REGION("maincpu", 0xa000)
	AM_RANGE(0xfff0, 0xffff) AM_ROM AM_REGION("maincpu", 0xfff0)
	AM_RANGE(0xa000, 0xbfff) AM_WRITE(bml3_a000_w)
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(bml3_c000_w)
	AM_RANGE(0xe000, 0xefff) AM_WRITE(bml3_e000_w)
	AM_RANGE(0xf000, 0xfeff) AM_WRITE(bml3_f000_w)
	AM_RANGE(0xfff0, 0xffff) AM_WRITE(bml3_fff0_w)
ADDRESS_MAP_END


/* Input ports */

static INPUT_PORTS_START( bml3 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x01, "NewOn" ) // TODO
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Show F help" )
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

	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("? PAD")
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X1") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift")PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X2")
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana Lock") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana Shift")  PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 PAD") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 PAD") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 PAD") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X7") //backspace
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xC2\xA5") //PORT_NAME("?")

	PORT_START("key2") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 PAD") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".")
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) //or cls?
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 PAD") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 PAD") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 PAD") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-')

	PORT_START("key3") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ PAD") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_")
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X8")
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 PAD") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0xffe00000,IP_ACTIVE_HIGH,IPT_UNKNOWN)
INPUT_PORTS_END


static MC6845_INTERFACE( mc6845_intf )
{
	"screen",   /* screen we are acting on */
	false,      /* show border area */
	8,          /* number of pixels per video memory address */
	NULL,       /* before pixel update callback */
	NULL,       /* row update callback */
	NULL,       /* after pixel update callback */
	DEVCB_NULL, /* callback for display state changes */
	DEVCB_NULL, /* callback for cursor state changes */
	DEVCB_NULL, /* HSYNC callback */
	DEVCB_NULL, /* VSYNC callback */
	NULL        /* update address callback */
};

static WRITE_LINE_DEVICE_HANDLER( bml3_fdc_intrq_w )
{
	// #$# ? HOLD_LINE or PULSE_LINE
	// #$#debug
	logerror("bml3_fdc_intrq_w called %d\n", state);
	if (state)
		device->machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(bml3_state::keyboard_callback)
{
	static const char *const portnames[3] = { "key1","key2","key3" };
 	int i,port_i,scancode,trigger = 0;

 	if(!m_keyb_press_flag)
 	{
 		m_keyb_scancode = (m_keyb_scancode + 1) & 0x7F;
 		scancode = m_keyb_scancode;

 		if (scancode == 0x7F)
 		{
 			if (m_keyb_empty_scan == 1)
 			{
 				// full scan completed with no keypress
 				m_keyb_press = 0;
 				if (!m_keyb_counter_operation_disabled)
 					trigger = !0;
	  		}
 			if (m_keyb_empty_scan > 0)
 				m_keyb_empty_scan--;
	  	}
 		else if (scancode < 32*3)
 		{
 			port_i = scancode / 32;
 			i = scancode % 32;
			if((ioport(portnames[port_i])->read()>>i) & 1)
			{
 				m_keyb_empty_scan = 2;
 				trigger = !0;
 			}
 		}
 		if (trigger)
 		{
 			m_keyb_press_flag = 1;
 			m_keyb_press = m_keyb_scancode | 0x80;
 			if (!m_keyb_interrupt_disabled)
				m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
 		}
 		/*
 		else {
 			// #$# ? don't need this
			m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
 		}
 		*/
 	}

}

#if 0
INTERRUPT_GEN_MEMBER(bml3_state::bml3_irq)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
}
#endif


INTERRUPT_GEN_MEMBER(bml3_state::bml3_timer_firq)
{
	if(!m_firq_mask)
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
		m_firq_status = 1;
	}
}

void bml3_state::palette_init()
{
	int i;

	for(i=0;i<8;i++)
		palette_set_color_rgb(machine(), i, pal1bit(i >> 1),pal1bit(i >> 2),pal1bit(i >> 0));
}

void bml3_state::machine_start()
{
	m_beep->set_frequency(1200); //guesswork
	m_beep->set_state(0);
	m_extram = auto_alloc_array(machine(),UINT8,0x10000);
}

void bml3_state::machine_reset()
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	/* defaults */
	mem.install_rom(0xa000, 0xfeff,memregion("maincpu")->base() + 0xa000);
	mem.install_rom(0xfff0, 0xffff,memregion("maincpu")->base() + 0xfff0);
	mem.install_write_handler(0xa000, 0xbfff, 0, 0,write8_delegate(FUNC(bml3_state::bml3_a000_w), this),0);
	mem.install_write_handler(0xc000, 0xdfff, 0, 0,write8_delegate(FUNC(bml3_state::bml3_c000_w), this),0);
	mem.install_write_handler(0xe000, 0xefff, 0, 0,write8_delegate(FUNC(bml3_state::bml3_e000_w), this),0);
	mem.install_write_handler(0xf000, 0xfeff, 0, 0,write8_delegate(FUNC(bml3_state::bml3_f000_w), this),0);
	mem.install_write_handler(0xfff0, 0xffff, 0, 0,write8_delegate(FUNC(bml3_state::bml3_fff0_w), this),0);

	m_firq_mask = -1; // disable firq
}

/* F4 Character Displayer */
static const gfx_layout bml3_charlayout8x8even =
{
	8, 8,                   /* 8 x 8 characters */
	RGN_FRAC(1,1),                  /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 2*8,4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	8*16                    /* every char takes 8 bytes */
};

static const gfx_layout bml3_charlayout8x8odd =
{
	8, 8,                   /* 8 x 8 characters */
	RGN_FRAC(1,1),                  /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 1*8, 3*8, 5*8, 7*8, 9*8, 11*8, 13*8, 15*8 },
	8*16                    /* every char takes 8 bytes */
};

static const gfx_layout bml3_charlayout8x16 =
{
	8, 16,                  /* 8 x 8 characters */
	RGN_FRAC(1,1),                  /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 8 bytes */
};

static GFXDECODE_START( bml3 )
	GFXDECODE_ENTRY( "chargen", 0, bml3_charlayout8x8even, 0, 4 )
	GFXDECODE_ENTRY( "chargen", 0, bml3_charlayout8x8odd, 0, 4 )
	GFXDECODE_ENTRY( "chargen", 0, bml3_charlayout8x16, 0, 4 )
GFXDECODE_END

// #$# ? remove if not using proper mc6843?
// const mc6843_interface bml3_6843_if = { NULL };

WRITE8_MEMBER(bml3_state::bml3_piaA_w)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	/* ROM banking:
	-0-- --0- 0xa000 - 0xbfff ROM R RAM W
	-1-- --0- 0xa000 - 0xbfff RAM R/W
	-x-- --1- 0xa000 - 0xbfff no change
	0--- -0-- 0xc000 - 0xdfff ROM R RAM W
	1--- -0-- 0xc000 - 0xdfff RAM R/W
	x--- -1-- 0xc000 - 0xdfff no change
	0--- 0--- 0xe000 - 0xefff ROM R RAM W
	1--- 0--- 0xe000 - 0xefff RAM R/W
	x--- 1--- 0xe000 - 0xefff no change
	---- ---x 0xf000 - 0xfeff (0) ROM R RAM W (1) RAM R/W
	---- --x- 0xfff0 - 0xffff (0) ROM R RAM W (1) RAM R/W
	*/
	printf("Check banking PIA A -> %02x\n",data);

	if(!(data & 0x2))
	{
		if(data & 0x40)
		{
			mem.install_readwrite_handler(0xa000, 0xbfff, 0, 0,
				read8_delegate(FUNC(bml3_state::bml3_a000_r), this),
				write8_delegate(FUNC(bml3_state::bml3_a000_w), this), 0);
		}
		else
		{
			mem.install_rom(0xa000, 0xbfff,
				memregion("maincpu")->base() + 0xa000);
			mem.install_write_handler(0xa000, 0xbfff, 0, 0,
				write8_delegate(FUNC(bml3_state::bml3_a000_w), this),
				0);
		}
	}

	if(!(data & 0x4))
	{
		if(data & 0x40)
		{
			mem.install_readwrite_handler(0xc000, 0xdfff, 0, 0,
				read8_delegate(FUNC(bml3_state::bml3_c000_r), this),
				write8_delegate(FUNC(bml3_state::bml3_c000_w), this), 0);
		}
		else
		{
			mem.install_rom(0xc000, 0xdfff,
				memregion("maincpu")->base() + 0xc000);
			mem.install_write_handler(0xc000, 0xdfff, 0, 0,
				write8_delegate(FUNC(bml3_state::bml3_c000_w), this),
				0);
		}
	}

	if(!(data & 0x8))
	{
		if(data & 0x80)
		{
			mem.install_readwrite_handler(0xe000, 0xefff, 0, 0,
				read8_delegate(FUNC(bml3_state::bml3_e000_r), this),
				write8_delegate(FUNC(bml3_state::bml3_e000_w), this), 0);
		}
		else
		{
			mem.install_rom(0xe000, 0xefff,
				memregion("maincpu")->base() + 0xe000);
			mem.install_write_handler(0xe000, 0xefff, 0, 0,
				write8_delegate(FUNC(bml3_state::bml3_e000_w), this),
				0);
		}
	}

	if(data & 1)
	{
		mem.install_readwrite_handler(0xf000, 0xfeff, 0, 0,
			read8_delegate(FUNC(bml3_state::bml3_f000_r), this),
			write8_delegate(FUNC(bml3_state::bml3_f000_w), this), 0);
	}
	else
	{
		mem.install_rom(0xf000, 0xfeff,
			memregion("maincpu")->base() + 0xf000);
		mem.install_write_handler(0xf000, 0xfeff, 0, 0,
			write8_delegate(FUNC(bml3_state::bml3_f000_w), this),
			0);
	}

	if(data & 2)
	{
		mem.install_readwrite_handler(0xfff0, 0xffff, 0, 0,
			read8_delegate(FUNC(bml3_state::bml3_fff0_r), this),
			write8_delegate(FUNC(bml3_state::bml3_fff0_w), this), 0);
	}
	else
	{
		mem.install_rom(0xfff0, 0xffff,
			memregion("maincpu")->base() + 0xfff0);
		mem.install_write_handler(0xfff0, 0xffff, 0, 0,
			write8_delegate(FUNC(bml3_state::bml3_fff0_w), this),
			0);
	}
}

static const pia6821_interface bml3_pia_config =
{
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,

	DEVCB_DRIVER_MEMBER(bml3_state, bml3_piaA_w),   /* port A output */
	DEVCB_NULL,
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
};


READ_LINE_MEMBER( bml3_state::bml3_acia_rx_r )
{
	printf("TAPE R\n");
	return 1;
}

WRITE_LINE_MEMBER( bml3_state::bml3_acia_tx_w )
{
	printf("%02x TAPE\n",state);
}


READ_LINE_MEMBER( bml3_state::bml3_acia_dts_r )
{
	printf("TAPE R DTS\n");
	return 1;
}

WRITE_LINE_MEMBER( bml3_state::bml3_acia_rts_w )
{
	printf("%02x TAPE RTS\n",state);
}

READ_LINE_MEMBER( bml3_state::bml3_acia_dcd_r )
{
	printf("TAPE R DCD\n");
	return 1;
}

WRITE_LINE_MEMBER( bml3_state::bml3_acia_irq_w )
{
	printf("%02x TAPE IRQ\n",state);
}


static ACIA6850_INTERFACE( bml3_acia_if )
{
	600,
	600,
	DEVCB_DRIVER_LINE_MEMBER(bml3_state, bml3_acia_rx_r),
	DEVCB_DRIVER_LINE_MEMBER(bml3_state, bml3_acia_tx_w),
	DEVCB_DRIVER_LINE_MEMBER(bml3_state, bml3_acia_dts_r),
	DEVCB_DRIVER_LINE_MEMBER(bml3_state, bml3_acia_rts_w),
	DEVCB_DRIVER_LINE_MEMBER(bml3_state, bml3_acia_dcd_r),
	DEVCB_DRIVER_LINE_MEMBER(bml3_state, bml3_acia_irq_w)
};

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL, // read A
	DEVCB_NULL, // read B
	DEVCB_NULL, // write A
	DEVCB_NULL  // write B
};

static const floppy_interface bml3_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(bml3),
	NULL
};

const wd17xx_interface bml3_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_LINE(bml3_fdc_intrq_w),
	DEVCB_NULL,
	{FLOPPY_0, FLOPPY_1}
};

static MACHINE_CONFIG_START( bml3, bml3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(bml3_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bml3_state,  bml3_timer_firq)
//  MCFG_CPU_PERIODIC_INT_DRIVER(bml3_state, bml3_firq, 45)

//  MCFG_MACHINE_RESET_OVERRIDE(bml3_state,bml3)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(bml3_state, screen_update_bml3)
	MCFG_PALETTE_LENGTH(8)
	MCFG_GFXDECODE(bml3)

	/* Devices */
	MCFG_MC6845_ADD("crtc", H46505, XTAL_1MHz, mc6845_intf)
	// fire once per scan of an individual key
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard_timer", bml3_state, keyboard_callback, attotime::from_hz(15750/2))
	// #$# ? remove if not using proper mc6843?
	// MCFG_MC6843_ADD( "mc6843", bml3_6843_if )
	MCFG_PIA6821_ADD("pia6821", bml3_pia_config)
	MCFG_ACIA6850_ADD("acia6850", bml3_acia_if)

 	/* floppy */
	// #$# not sure what wd17xx model, this is just a guess
 	MCFG_WD1773_ADD("fdc", bml3_wd17xx_interface )
 	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(bml3_floppy_interface)

	/* Audio */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)

	MCFG_SOUND_ADD("ym2203", YM2203, 2000000) //unknown clock / divider
	MCFG_YM2203_AY8910_INTF(&ay8910_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( bml3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "l3bas.rom", 0xa000, 0x6000, BAD_DUMP CRC(d81baa07) SHA1(a8fd6b29d8c505b756dbf5354341c48f9ac1d24d)) //original, 24k isn't a proper rom size!
	/* Handcrafted ROMs, rom labels and contents might not match */
	ROM_LOAD( "598 p16611.ic3", 0xa000, 0x2000, BAD_DUMP CRC(954b9bad) SHA1(047948fac6808717c60a1d0ac9205a5725362430))
	ROM_LOAD( "599 p16561.ic4", 0xc000, 0x2000, BAD_DUMP CRC(b27a48f5) SHA1(94cb616df4caa6415c5076f9acdf675acb7453e2))
	ROM_LOAD( "600 p16681.ic5", 0xe000, 0x2000, BAD_DUMP CRC(fe3988a5) SHA1(edc732f1cd421e0cf45ffcfc71c5589958ceaae7))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("font.rom", 0x00000, 0x1000, BAD_DUMP CRC(0b6f2f10) SHA1(dc411b447ca414e94843636d8b5f910c954581fb) ) // handcrafted

	ROM_REGION( 0x8000, "vram", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD("kanji.rom", 0x00000, 0x20000, NO_DUMP )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY         FULLNAME           FLAGS */
COMP( 1980, bml3,   0,      0,       bml3,      bml3, driver_device,    0,      "Hitachi", "Basic Master Level 3", GAME_NOT_WORKING | GAME_NO_SOUND)
