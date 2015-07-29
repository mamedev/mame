// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Multi 8 (c) 1983 Mitsubishi

    preliminary driver by Angelo Salese

    TODO:
    - dunno how to trigger the text color mode in BASIC, I just modify
      $f0b1 to 1 for now
    - bitmap B/W mode is untested
    - keyboard

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "sound/beep.h"


class multi8_state : public driver_device
{
public:
	multi8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_ppi(*this, "ppi8255_0"),
	m_crtc(*this, "crtc"),
	m_beeper(*this, "beeper")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<mc6845_device> m_crtc;
	required_device<beep_device> m_beeper;
	DECLARE_WRITE8_MEMBER(multi8_6845_w);
	DECLARE_READ8_MEMBER(key_input_r);
	DECLARE_READ8_MEMBER(key_status_r);
	DECLARE_READ8_MEMBER(multi8_vram_r);
	DECLARE_WRITE8_MEMBER(multi8_vram_w);
	DECLARE_READ8_MEMBER(pal_r);
	DECLARE_WRITE8_MEMBER(pal_w);
	DECLARE_READ8_MEMBER(multi8_kanji_r);
	DECLARE_WRITE8_MEMBER(multi8_kanji_w);
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE8_MEMBER(ym2203_porta_w);
	UINT8 *m_p_vram;
	UINT8 *m_p_wram;
	UINT8 *m_p_kanji;
	UINT8 *m_p_chargen;
	UINT8 m_mcu_init;
	UINT8 m_keyb_press;
	UINT8 m_keyb_press_flag;
	UINT8 m_shift_press_flag;
	UINT8 m_display_reg;
	UINT8 m_crtc_vreg[0x100],m_crtc_index;
	UINT8 m_vram_bank;
	UINT8 m_pen_clut[8];
	UINT8 m_bw_mode;
	UINT16 m_knj_addr;
	DECLARE_READ8_MEMBER(ay8912_0_r);
	DECLARE_READ8_MEMBER(ay8912_1_r);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_multi8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	void multi8_draw_pixel(bitmap_ind16 &bitmap,int y,int x,UINT8 pen,UINT8 width);
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

void multi8_state::video_start()
{
	m_keyb_press = m_keyb_press_flag = m_shift_press_flag = m_display_reg = 0;

	for (m_bw_mode = 0; m_bw_mode < 8; m_bw_mode++)
		m_pen_clut[m_bw_mode]=0;

	m_vram_bank = 8;
	m_bw_mode = 0;
	m_p_chargen = memregion("chargen")->base();
}

void multi8_state::multi8_draw_pixel(bitmap_ind16 &bitmap,int y,int x,UINT8 pen,UINT8 width)
{
	if(!machine().first_screen()->visible_area().contains(x, y))
		return;

	if(width)
	{
		bitmap.pix16(y, x*2+0) = pen;
		bitmap.pix16(y, x*2+1) = pen;
	}
	else
		bitmap.pix16(y, x) = pen;
}

UINT32 multi8_state::screen_update_multi8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	UINT8 x_width;
	UINT8 xi,yi;

	count = 0x0000;

	x_width = (m_display_reg & 0x40) ? 80 : 40;

	for(y=0; y<200; y++)
	{
		for(x=0; x<80; x++)
		{
			for(xi=0; xi<8; xi++)
			{
				int pen_r,pen_g,pen_b,color;

				pen_b = (m_p_vram[count | 0x0000] >> (7-xi)) & 1;
				pen_r = (m_p_vram[count | 0x4000] >> (7-xi)) & 1;
				pen_g = (m_p_vram[count | 0x8000] >> (7-xi)) & 1;

				if (m_bw_mode)
				{
					pen_b = (m_display_reg & 1) ? pen_b : 0;
					pen_r = (m_display_reg & 2) ? pen_r : 0;
					pen_g = (m_display_reg & 4) ? pen_g : 0;

					color = ((pen_b) | (pen_r) | (pen_g)) ? 7 : 0;
				}
				else
					color = (pen_b) | (pen_r << 1) | (pen_g << 2);

				multi8_draw_pixel(bitmap, y, x*8+xi,m_pen_clut[color], 0);
			}
			count++;
		}
	}

	count = 0xc000;

	for(y=0;y<25;y++)
	{
		for(x=0; x<x_width; x++)
		{
			int tile = m_p_vram[count];
			int attr = m_p_vram[count+0x800];
			int color = (m_display_reg & 0x80) ? 7 : (attr & 0x07);

			for (yi=0; yi<8; yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					int pen;

					if(attr & 0x20)
						pen = (m_p_chargen[tile*8+yi] >> (7-xi) & 1) ? 0 : color;
					else
						pen = (m_p_chargen[tile*8+yi] >> (7-xi) & 1) ? color : 0;

					if(pen)
						multi8_draw_pixel(bitmap, y*mc6845_tile_height+yi, x*8+xi, pen, (m_display_reg & 0x40) == 0x00);
				}
			}

			// m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, tile,color >> 5, 0, 0, x*8, y*8);

			// draw cursor
			if(mc6845_cursor_addr+0xc000 == count)
			{
				int xc,yc,cursor_on;

				cursor_on = 0;
				switch(mc6845_cursor_y_start & 0x60)
				{
					case 0x00: cursor_on = 1; break; //always on
					case 0x20: cursor_on = 0; break; //always off
					case 0x40: if(machine().first_screen()->frame_number() & 0x10) { cursor_on = 1; } break; //fast blink
					case 0x60: if(machine().first_screen()->frame_number() & 0x20) { cursor_on = 1; } break; //slow blink
				}

				if(cursor_on)
				{
					for (yc=0; yc<(mc6845_tile_height-(mc6845_cursor_y_start & 7)); yc++)
					{
						for (xc=0; xc<8; xc++)
							multi8_draw_pixel(bitmap, y*mc6845_tile_height+yc, x*8+xc,0x07,(m_display_reg & 0x40) == 0x00);

					}
				}
			}

			(m_display_reg & 0x40) ? count++ : count+=2;
		}
	}
	return 0;
}

WRITE8_MEMBER( multi8_state::multi8_6845_w )
{
	if (!offset)
	{
		m_crtc_index = data;
		m_crtc->address_w(space, offset, data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		m_crtc->register_w(space, offset, data);
	}
}

READ8_MEMBER( multi8_state::key_input_r )
{
	if (m_mcu_init == 0)
	{
		m_mcu_init++;
		return 3;
	}

	m_keyb_press_flag &= 0xfe;

	return m_keyb_press;
}

READ8_MEMBER( multi8_state::key_status_r )
{
	if (m_mcu_init == 0)
		return 1;
	else
	if (m_mcu_init == 1)
	{
		m_mcu_init++;
		return 1;
	}
	else
	if (m_mcu_init == 2)
	{
		m_mcu_init++;
		return 0;
	}

	return m_keyb_press_flag | (m_shift_press_flag << 7);
}

READ8_MEMBER( multi8_state::multi8_vram_r )
{
	UINT8 res;

	if (!BIT(m_vram_bank, 4)) //select plain work ram
		return m_p_wram[offset];

	res = 0xff;

	if (!BIT(m_vram_bank, 0))
		res &= m_p_vram[offset | 0x0000];

	if (!BIT(m_vram_bank, 1))
		res &= m_p_vram[offset | 0x4000];

	if (!BIT(m_vram_bank, 2))
		res &= m_p_vram[offset | 0x8000];

	if (!BIT(m_vram_bank, 3))
		res &= m_p_vram[offset | 0xc000];

	return res;
}

WRITE8_MEMBER( multi8_state::multi8_vram_w )
{
	if (!BIT(m_vram_bank, 4)) //select plain work ram
	{
		m_p_wram[offset] = data;
		return;
	}

	if (!BIT(m_vram_bank, 0))
		m_p_vram[offset | 0x0000] = data;

	if (!BIT(m_vram_bank, 1))
		m_p_vram[offset | 0x4000] = data;

	if (!BIT(m_vram_bank, 2))
		m_p_vram[offset | 0x8000] = data;

	if (!BIT(m_vram_bank, 3))
		m_p_vram[offset | 0xc000] = data;
}

READ8_MEMBER( multi8_state::pal_r )
{
	return m_pen_clut[offset];
}

WRITE8_MEMBER( multi8_state::pal_w )
{
	m_pen_clut[offset] = data;

	UINT8 i;
	for(i=0;i<8;i++)
	{
		if (m_pen_clut[i])
		{
			m_bw_mode = 0;
			return;
		}
		m_bw_mode = 1;
	}
}

READ8_MEMBER(multi8_state::ay8912_0_r){ return machine().device<ay8910_device>("aysnd")->data_r(space, 0); }
READ8_MEMBER(multi8_state::ay8912_1_r){ return machine().device<ay8910_device>("aysnd")->data_r(space, 1); }

READ8_MEMBER( multi8_state::multi8_kanji_r )
{
	return m_p_kanji[(m_knj_addr << 1) | (offset & 1)];
}

WRITE8_MEMBER( multi8_state::multi8_kanji_w )
{
	m_knj_addr = (offset == 0) ? (m_knj_addr & 0xff00) | (data & 0xff) : (m_knj_addr & 0x00ff) | (data << 8);
}

static ADDRESS_MAP_START(multi8_mem, AS_PROGRAM, 8, multi8_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE( multi8_vram_r, multi8_vram_w )
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(multi8_io, AS_IO, 8, multi8_state)
//  ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(key_input_r) AM_WRITENOP//keyboard
	AM_RANGE(0x01, 0x01) AM_READ(key_status_r) AM_WRITENOP//keyboard
	AM_RANGE(0x18, 0x19) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x18, 0x18) AM_READ(ay8912_0_r)
	AM_RANGE(0x1a, 0x1a) AM_READ(ay8912_1_r)
	AM_RANGE(0x1c, 0x1d) AM_WRITE(multi8_6845_w)
//  AM_RANGE(0x20, 0x21) //sio, cmt
//  AM_RANGE(0x24, 0x27) //pit
	AM_RANGE(0x28, 0x2b) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
//  AM_RANGE(0x2c, 0x2d) //i8259
	AM_RANGE(0x30, 0x37) AM_READWRITE(pal_r,pal_w)
	AM_RANGE(0x40, 0x41) AM_READWRITE(multi8_kanji_r,multi8_kanji_w) //kanji regs
//  AM_RANGE(0x70, 0x74) //upd765a fdc
//  AM_RANGE(0x78, 0x78) //memory banking
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( multi8 )
	PORT_START("VBLANK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")

	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED) //0x00 null
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x01 soh
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x02 stx
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x03 etx
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x04 etx
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x05 eot
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x06 enq
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x07 ack
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0a
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0b lf
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0c vt
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0e cr
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0f so

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x10 si
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x11 dle
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x12 dc1
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x13 dc2
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x15 dc4
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x17 syn
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x18 etb
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1a em
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CHAR(27)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1c ???
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1d fs
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1e gs
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1f us

	PORT_START("key2") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x21 !
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x22 "
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x23 #
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x24 $
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x25 %
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x26 &
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x27 '
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED) //0x28 (
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED) //0x29 )
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2a *
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2c ,
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2e .
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2f /

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3d =
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3e >
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3f ?

	PORT_START("key3") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_")

	PORT_START("key_modifiers")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("GRPH") PORT_CODE(KEYCODE_LALT)
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(multi8_state::keyboard_callback)
{
	static const char *const portnames[3] = { "key1","key2","key3" };
	int i,port_i,scancode;
	UINT8 keymod = ioport("key_modifiers")->read() & 0x1f;
	scancode = 0;

	m_shift_press_flag = ((keymod & 0x02) >> 1);

	for(port_i=0;port_i<3;port_i++)
	{
		for(i=0;i<32;i++)
		{
			if((ioport(portnames[port_i])->read()>>i) & 1)
			{
				//key_flag = 1;
				if(!m_shift_press_flag)  // shift not pressed
				{
					if(scancode >= 0x41 && scancode < 0x5b)
						scancode += 0x20;  // lowercase
				}
				else
				{
					if(scancode >= 0x31 && scancode < 0x3a)
						scancode -= 0x10;
					if(scancode == 0x30)
						scancode = 0x3d;

					if(scancode == 0x3b)
						scancode = 0x2c;

					if(scancode == 0x3a)
						scancode = 0x2e;
					if(scancode == 0x5b)
						scancode = 0x2b;
					if(scancode == 0x3c)
						scancode = 0x3e;
				}
				m_keyb_press = scancode;
				m_keyb_press_flag = 1;
				return;
			}
			scancode++;
		}
	}
}

static const gfx_layout multi8_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout multi8_kanjilayout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( multi8 )
	GFXDECODE_ENTRY( "chargen", 0x0000, multi8_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "kanji",   0x0000, multi8_kanjilayout, 0, 1 )
GFXDECODE_END


READ8_MEMBER( multi8_state::porta_r )
{
	int vsync = (ioport("VBLANK")->read() & 0x1) << 5;
	/*
	-x-- ---- kanji rom is present (0) yes
	--x- ---- vsync
	---- --x- fdc rom is present (0) yes
	*/

	return 0x9f | vsync | 0x00;
}


WRITE8_MEMBER( multi8_state::portb_w )
{
	/*
	    x--- ---- color mode
	    -x-- ---- screen width (80 / 40)
	    ---- x--- memory bank status
	    ---- -xxx page screen graphics in B/W mode
	*/

	m_display_reg = data;
}

WRITE8_MEMBER( multi8_state::portc_w )
{
//  printf("Port C w = %02x\n",data);
	m_vram_bank = data & 0x1f;

	if (data & 0x20 && data != 0xff)
		printf("Work RAM bank selected!\n");
//      fatalerror("Work RAM bank selected\n");
}


WRITE8_MEMBER( multi8_state::ym2203_porta_w )
{
	m_beeper->set_state((data & 0x08));
}

void multi8_state::machine_start()
{
	m_p_vram = memregion("vram")->base();
	m_p_wram = memregion("wram")->base();
	m_p_kanji = memregion("kanji")->base();
}

void multi8_state::machine_reset()
{
	m_beeper->set_frequency(1200); //guesswork
	m_beeper->set_state(0);
	m_mcu_init = 0;
}

static MACHINE_CONFIG_START( multi8, multi8_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(multi8_mem)
	MCFG_CPU_IO_MAP(multi8_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(multi8_state, screen_update_multi8)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_3BIT_BRG("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", multi8)

	/* Audio */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8912, 1500000) //unknown clock / divider
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(multi8_state, ym2203_porta_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)

	/* devices */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard_timer", multi8_state, keyboard_callback, attotime::from_hz(240/32))

	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_3_579545MHz/2)    /* unknown clock, hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_DEVICE_ADD("ppi8255_0", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(multi8_state, porta_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(multi8_state, portb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(multi8_state, portc_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( multi8 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x8000, CRC(2131a3a8) SHA1(0f5a565ecfbf79237badbf9011dcb374abe74a57))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "font.rom",  0x0000, 0x0800, BAD_DUMP CRC(08f9ed0e) SHA1(57480510fb30af1372df5a44b23066ca61c6f0d9))

	ROM_REGION( 0x20000, "kanji", 0 )
	ROM_LOAD( "kanji.rom",  0x0000, 0x20000, BAD_DUMP CRC(c3cb3ff9) SHA1(7173cc5053a281d73cfecfacd31442e21ee7474a))

	ROM_REGION( 0x0100, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "kbd.rom",  0x0000, 0x0100, NO_DUMP )

	ROM_REGION( 0x1000, "fdc_bios", 0 )
	ROM_LOAD( "disk.rom",  0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x4000, "wram", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY      FULLNAME       FLAGS */
COMP( 1983, multi8, 0,      0,       multi8,    multi8, driver_device,  0,     "Mitsubishi", "Multi 8", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
