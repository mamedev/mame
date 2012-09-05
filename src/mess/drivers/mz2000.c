/***************************************************************************

    Sharp MZ-2000

    driver by Angelo Salese
    font conversion by Tomasz Slanina

    Basically a simpler version of Sharp MZ-2500

    TODO:
    - cassette interface;
    - implement remaining video capabilities

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/i8255.h"
#include "machine/wd17xx.h"
#include "machine/pit8253.h"
#include "sound/beep.h"
#include "sound/wave.h"
#include "machine/rp5c15.h"

#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "formats/mz_cas.h"



class mz2000_state : public driver_device
{
public:
	mz2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_cass(*this, CASSETTE_TAG)
		{ }

	required_device<cassette_image_device> m_cass;

	UINT8 m_ipl_enable;
	UINT8 m_tvram_enable;
	UINT8 m_gvram_enable;
	UINT8 m_gvram_bank;

	UINT8 m_key_mux,m_pio_latchb;

	UINT8 m_old_portc;
	UINT8 m_width80;
	UINT8 m_tvram_attr;
	UINT8 m_gvram_mask;

	UINT8 m_color_mode;
	UINT8 m_has_fdc;
	UINT8 m_hi_mode;
	DECLARE_READ8_MEMBER(mz2000_ipl_r);
	DECLARE_READ8_MEMBER(mz2000_wram_r);
	DECLARE_WRITE8_MEMBER(mz2000_wram_w);
	DECLARE_READ8_MEMBER(mz2000_tvram_r);
	DECLARE_WRITE8_MEMBER(mz2000_tvram_w);
	DECLARE_READ8_MEMBER(mz2000_gvram_r);
	DECLARE_WRITE8_MEMBER(mz2000_gvram_w);
	DECLARE_READ8_MEMBER(mz2000_mem_r);
	DECLARE_WRITE8_MEMBER(mz2000_mem_w);
	DECLARE_WRITE8_MEMBER(mz2000_gvram_bank_w);
	DECLARE_WRITE8_MEMBER(mz2000_fdc_w);
	DECLARE_WRITE8_MEMBER(timer_w);
	DECLARE_WRITE8_MEMBER(mz2000_tvram_attr_w);
	DECLARE_WRITE8_MEMBER(mz2000_gvram_mask_w);
};

static VIDEO_START( mz2000 )
{
}

static SCREEN_UPDATE_IND16( mz2000 )
{
	mz2000_state *state = screen.machine().driver_data<mz2000_state>();
	UINT8 *tvram = screen.machine().root_device().memregion("tvram")->base();
	UINT8 *gvram = screen.machine().root_device().memregion("gvram")->base();
	UINT8 *gfx_data = state->memregion("chargen")->base();
	int x,y,xi,yi;
	UINT8 x_size;
	UINT32 count;

	count = 0;

	for(y=0;y<200;y++)
	{
		for(x=0;x<640;x+=8)
		{
			for(xi=0;xi<8;xi++)
			{
				int pen;
				pen  = ((gvram[count+0x4000] >> (xi)) & 1) ? 1 : 0; //B
				pen |= ((gvram[count+0x8000] >> (xi)) & 1) ? 2 : 0; //R
				pen |= ((gvram[count+0xc000] >> (xi)) & 1) ? 4 : 0; //G
				pen &= state->m_gvram_mask;

				bitmap.pix16(y*2+0, x+xi) = screen.machine().pens[pen];
				bitmap.pix16(y*2+1, x+xi) = screen.machine().pens[pen];
			}
			count++;
		}
	}

	x_size = (state->m_width80+1)*40;

	for(y=0;y<25;y++)
	{
		for(x=0;x<x_size;x++)
		{
			UINT8 tile = tvram[y*x_size+x];
			UINT8 color = state->m_tvram_attr & 7;

			for(yi=0;yi<8*(state->m_hi_mode+1);yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					int pen;
					int res_x,res_y;
					UINT16 tile_offset;

					res_x = x * 8 + xi;
					res_y = y * (8 *(state->m_hi_mode+1)) + yi;

					if(res_x > 640-1 || res_y > (200*(state->m_hi_mode+1))-1)
						continue;

					tile_offset = tile*(8*(state->m_hi_mode+1))+yi + (state->m_hi_mode * 0x800);

					pen = ((gfx_data[tile_offset] >> (7-xi)) & 1) ? color : -1;

					/* TODO: clean this up */
					if(pen != -1)
					{
						if(state->m_hi_mode)
						{
							if(state->m_width80 == 0)
							{
								bitmap.pix16(res_y, res_x*2+0) = screen.machine().pens[pen];
								bitmap.pix16(res_y, res_x*2+1) = screen.machine().pens[pen];
							}
							else
							{
								bitmap.pix16(res_y, res_x) = screen.machine().pens[pen];
							}
						}
						else
						{
							if(state->m_width80 == 0)
							{
								bitmap.pix16(res_y*2+0, res_x*2+0) = screen.machine().pens[pen];
								bitmap.pix16(res_y*2+0, res_x*2+1) = screen.machine().pens[pen];
								bitmap.pix16(res_y*2+1, res_x*2+0) = screen.machine().pens[pen];
								bitmap.pix16(res_y*2+1, res_x*2+1) = screen.machine().pens[pen];
							}
							else
							{
								bitmap.pix16(res_y*2+0, res_x) = screen.machine().pens[pen];
								bitmap.pix16(res_y*2+1, res_x) = screen.machine().pens[pen];
							}
						}
					}
				}
			}
		}
	}

    return 0;
}

READ8_MEMBER(mz2000_state::mz2000_ipl_r)
{
	UINT8 *ipl = memregion("ipl")->base();

	return ipl[offset];
}

READ8_MEMBER(mz2000_state::mz2000_wram_r)
{
	UINT8 *wram = memregion("wram")->base();

	return wram[offset];
}

WRITE8_MEMBER(mz2000_state::mz2000_wram_w)
{
	UINT8 *wram = memregion("wram")->base();

	wram[offset] = data;
}

READ8_MEMBER(mz2000_state::mz2000_tvram_r)
{
	UINT8 *tvram = memregion("tvram")->base();

	return tvram[offset];
}

WRITE8_MEMBER(mz2000_state::mz2000_tvram_w)
{
	UINT8 *tvram = memregion("tvram")->base();

	tvram[offset] = data;
}

READ8_MEMBER(mz2000_state::mz2000_gvram_r)
{
	UINT8 *gvram = memregion("gvram")->base();

	return gvram[offset+m_gvram_bank*0x4000];
}

WRITE8_MEMBER(mz2000_state::mz2000_gvram_w)
{
	UINT8 *gvram = memregion("gvram")->base();

	gvram[offset+m_gvram_bank*0x4000] = data;
}


READ8_MEMBER(mz2000_state::mz2000_mem_r)
{
	UINT8 page_mem;

	page_mem = (offset & 0xf000) >> 12;

	if(page_mem == 0 && m_ipl_enable)
		return mz2000_ipl_r(space,offset & 0xfff);

	if(((page_mem & 8) == 0) && m_ipl_enable == 0) // if ipl is enabled, 0x1000 - 0x7fff accesses to dummy region
		return mz2000_wram_r(space,offset);

	if(page_mem & 8)
	{
		if(page_mem == 0xd && m_tvram_enable)
			return mz2000_tvram_r(space,offset & 0xfff);
		else if(page_mem >= 0xc && m_gvram_enable)
			return mz2000_gvram_r(space,offset & 0x3fff);
		else
		{
			UINT16 wram_mask = (m_ipl_enable) ? 0x7fff : 0xffff;
			return mz2000_wram_r(space,offset & wram_mask);
		}
	}

	return 0xff;
}

WRITE8_MEMBER(mz2000_state::mz2000_mem_w)
{
	UINT8 page_mem;

	page_mem = (offset & 0xf000) >> 12;

	if((page_mem & 8) == 0 && m_ipl_enable == 0)
		mz2000_wram_w(space,offset,data);

	if(page_mem & 8)
	{
		if(page_mem == 0xd && m_tvram_enable)
			mz2000_tvram_w(space,offset & 0xfff,data);
		else if(page_mem >= 0xc && m_gvram_enable)
			mz2000_gvram_w(space,offset & 0x3fff,data);
		else
		{
			UINT16 wram_mask = (m_ipl_enable) ? 0x7fff : 0xffff;

			mz2000_wram_w(space,offset & wram_mask,data);
		}
	}
}

WRITE8_MEMBER(mz2000_state::mz2000_gvram_bank_w)
{
	m_gvram_bank = data & 3;
}

static READ8_DEVICE_HANDLER( mz2000_wd17xx_r )
{
	mz2000_state *state = device->machine().driver_data<mz2000_state>();

	if(state->m_has_fdc)
		return wd17xx_r(device, offset) ^ 0xff;

	return 0xff;
}

static WRITE8_DEVICE_HANDLER( mz2000_wd17xx_w )
{
	mz2000_state *state = device->machine().driver_data<mz2000_state>();

	if(state->m_has_fdc)
		wd17xx_w(device, offset, data ^ 0xff);
}

WRITE8_MEMBER(mz2000_state::mz2000_fdc_w)
{
	device_t* dev = machine().device("mb8877a");

	switch(offset+0xdc)
	{
		case 0xdc:
			wd17xx_set_drive(dev,data & 3);
			floppy_mon_w(floppy_get_device(machine(), data & 3), (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
			floppy_drive_set_ready_state(floppy_get_device(machine(), data & 3), 1,0);
			break;
		case 0xdd:
			wd17xx_set_side(dev,(data & 1));
			break;
	}
}

WRITE8_MEMBER(mz2000_state::timer_w)
{
	device_t *pit8253 = machine().device("pit");

	pit8253_gate0_w(pit8253, 1);
	pit8253_gate1_w(pit8253, 1);
	pit8253_gate0_w(pit8253, 0);
	pit8253_gate1_w(pit8253, 0);
	pit8253_gate0_w(pit8253, 1);
	pit8253_gate1_w(pit8253, 1);
}

WRITE8_MEMBER(mz2000_state::mz2000_tvram_attr_w)
{
	m_tvram_attr = data;
}

WRITE8_MEMBER(mz2000_state::mz2000_gvram_mask_w)
{
	m_gvram_mask = data;
}

static ADDRESS_MAP_START(mz2000_map, AS_PROGRAM, 8, mz2000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xffff ) AM_READWRITE(mz2000_mem_r,mz2000_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mz2000_io, AS_IO, 8, mz2000_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xd8, 0xdb) AM_DEVREADWRITE_LEGACY("mb8877a", mz2000_wd17xx_r, mz2000_wd17xx_w)
	AM_RANGE(0xdc, 0xdd) AM_WRITE(mz2000_fdc_w)
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE("i8255_0", i8255_device, read, write)
    AM_RANGE(0xe4, 0xe7) AM_DEVREADWRITE_LEGACY("pit", pit8253_r, pit8253_w)
	AM_RANGE(0xe8, 0xeb) AM_DEVREADWRITE("z80pio_1", z80pio_device, read_alt, write_alt)
    AM_RANGE(0xf0, 0xf3) AM_WRITE(timer_w)
//  AM_RANGE(0xf4, 0xf7) CRTC
	AM_RANGE(0xf5, 0xf5) AM_WRITE(mz2000_tvram_attr_w)
	AM_RANGE(0xf6, 0xf6) AM_WRITE(mz2000_gvram_mask_w)
	AM_RANGE(0xf7, 0xf7) AM_WRITE(mz2000_gvram_bank_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( mz2000 )
	PORT_START("KEY0")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)

	PORT_START("KEY1")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("00 (PAD)") //PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(". (PAD)") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("- (PAD)") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("KEY2")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("KEY3")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("BREAK") //PORT_CODE(KEYCODE_ESC)

	PORT_START("KEY4")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/") //PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("KEY5")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("KEY6")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("KEY7")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("^")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("\\")
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("_")
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(".")
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(",")

	PORT_START("KEY8")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("KEY9")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(":")
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(";")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("@")
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("[")
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYA")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("]")
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYB")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("GRPH")
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SLOCK")
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("RVS")
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_BIT(0xe0,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYC")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYD")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("UNUSED")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Video Board" )
	PORT_CONFSETTING( 0x00, "Monochrome" )
	PORT_CONFSETTING( 0x01, "Color" )
	PORT_CONFNAME( 0x02, 0x02, "Floppy Device" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x02, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x04, 0x04, "High Resolution" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x04, DEF_STR( Yes ) )
INPUT_PORTS_END


static MACHINE_RESET(mz2000)
{
	mz2000_state *state = machine.driver_data<mz2000_state>();

	state->m_ipl_enable = 1;
	state->m_tvram_enable = 0;
	state->m_gvram_enable = 0;

	beep_set_frequency(machine.device(BEEPER_TAG),4096);
	beep_set_state(machine.device(BEEPER_TAG),0);

	state->m_color_mode = machine.root_device().ioport("CONFIG")->read() & 1;
	state->m_has_fdc = (machine.root_device().ioport("CONFIG")->read() & 2) >> 1;
	state->m_hi_mode = (machine.root_device().ioport("CONFIG")->read() & 4) >> 2;

	{
		int i;
		int r,g,b;

		for(i=0;i<8;i++)
		{
			r = (state->m_color_mode) ? (i & 2)>>1 : 0;
			g = (state->m_color_mode) ? (i & 4)>>2 : ((i) ? 1 : 0);
			b = (state->m_color_mode) ? (i & 1)>>0 : 0;

			palette_set_color_rgb(machine, i,pal1bit(r),pal1bit(g),pal1bit(b));
		}
	}
}


static const gfx_layout mz2000_charlayout_8 =
{
	8, 8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout mz2000_charlayout_16 =
{
	8, 16,
	256,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START( mz2000 )
	GFXDECODE_ENTRY( "chargen", 0x0000, mz2000_charlayout_8, 0, 1 )
	GFXDECODE_ENTRY( "chargen", 0x0800, mz2000_charlayout_16, 0, 1 )
GFXDECODE_END

static READ8_DEVICE_HANDLER( mz2000_porta_r )
{
	printf("A R\n");
	return 0xff;
}

static READ8_DEVICE_HANDLER( mz2000_portb_r )
{
	mz2000_state *state = device->machine().driver_data<mz2000_state>();
	/*
    x--- ---- break key
    -x-- ---- read tape data
    --x- ---- no tape signal
    ---x ---- no tape write signal
    ---- x--- end of tape reached
    ---- ---x "blank" control
    */
	UINT8 res = 0x80;

	res |= (state->m_cass->input() > 0.0038) ? 0x40 : 0x00;
	res |= ((state->m_cass->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY) ? 0x00 : 0x20;
	res |= (state->m_cass->get_position() >= state->m_cass->get_length()) ? 0x08 : 0x00;
	res |= (device->machine().primary_screen->vblank()) ? 0x00 : 0x01;

//	popmessage("%02x",res);

	return res;
}

static READ8_DEVICE_HANDLER( mz2000_portc_r )
{
	printf("C R\n");
	return 0xff;
}

static WRITE8_DEVICE_HANDLER( mz2000_porta_w )
{
	/*
	TODO: it's likely to be a 0->1 transition
    x--- ---- tape "APSS"
    -x-- ---- tape "APLAY"
    --x- ---- tape "AREW"
    ---x ---- reverse video
    ---- x--- tape stop
    ---- -x-- tape play
    ---- --x- tape ff
    ---- ---x tape rewind
    */
	mz2000_state *state = device->machine().driver_data<mz2000_state>();

	if((data & 8) == 0) // stop
	{
		state->m_cass->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		state->m_cass->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
	}
	if((data & 4) == 0) // play
	{
		state->m_cass->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
		state->m_cass->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
	}
}

static WRITE8_DEVICE_HANDLER( mz2000_portb_w )
{
	//printf("B W %02x\n",data);

	// ...
}

static WRITE8_DEVICE_HANDLER( mz2000_portc_w )
{
	mz2000_state *state = device->machine().driver_data<mz2000_state>();
	/*
        x--- ---- tape data write
        -x-- ---- tape rec
        --x- ---- tape ?
        ---x ---- tape open
        ---- x--- 0->1 transition = IPL reset
        ---- -x-- beeper state
        ---- --x- 0->1 transition = Work RAM reset
    */
	//printf("C W %02x\n",data);

	if(((state->m_old_portc & 8) == 0) && data & 8)
		state->m_ipl_enable = 1;

	if(((state->m_old_portc & 2) == 0) && data & 2)
	{
		state->m_ipl_enable = 0;
		/* correct? */
		cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_RESET, PULSE_LINE);
	}

	beep_set_state(device->machine().device(BEEPER_TAG),data & 0x04);

	state->m_old_portc = data;
}

static I8255_INTERFACE( ppi8255_intf )
{
	DEVCB_HANDLER(mz2000_porta_r),						/* Port A read */
	DEVCB_HANDLER(mz2000_porta_w),						/* Port A write */
	DEVCB_HANDLER(mz2000_portb_r),						/* Port B read */
	DEVCB_HANDLER(mz2000_portb_w),						/* Port B write */
	DEVCB_HANDLER(mz2000_portc_r),						/* Port C read */
	DEVCB_HANDLER(mz2000_portc_w)						/* Port C write */
};

static WRITE8_DEVICE_HANDLER( mz2000_pio1_porta_w )
{
	mz2000_state *state = device->machine().driver_data<mz2000_state>();
	state->m_tvram_enable = ((data & 0xc0) == 0xc0);
	state->m_gvram_enable = ((data & 0xc0) == 0x80);
	state->m_width80 = ((data & 0x20) >> 5);
	state->m_key_mux = data & 0x1f;
}

static READ8_DEVICE_HANDLER( mz2000_pio1_porta_r )
{
	mz2000_state *state = device->machine().driver_data<mz2000_state>();
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3",
	                                        "KEY4", "KEY5", "KEY6", "KEY7",
	                                        "KEY8", "KEY9", "KEYA", "KEYB",
	                                        "KEYC", "KEYD", "UNUSED", "UNUSED" };

	if(((state->m_key_mux & 0x10) == 0x00) || ((state->m_key_mux & 0x0f) == 0x0f)) //status read
	{
		int res,i;

		res = 0xff;
		for(i=0;i<0xe;i++)
			res &= device->machine().root_device().ioport(keynames[i])->read();

		state->m_pio_latchb = res;

		return res;
	}

	state->m_pio_latchb = device->machine().root_device().ioport(keynames[state->m_key_mux & 0xf])->read();

	return device->machine().root_device().ioport(keynames[state->m_key_mux & 0xf])->read();
}

static Z80PIO_INTERFACE( mz2000_pio1_intf )
{
	DEVCB_NULL,
	DEVCB_HANDLER( mz2000_pio1_porta_r ),
	DEVCB_HANDLER( mz2000_pio1_porta_w ),
	DEVCB_NULL,
	DEVCB_HANDLER( mz2000_pio1_porta_r ),
	DEVCB_NULL,
	DEVCB_NULL
};

static const wd17xx_interface mz2000_mb8877a_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

static LEGACY_FLOPPY_OPTIONS_START( mz2000 )
	LEGACY_FLOPPY_OPTION( img2d, "2d", "2D disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface mz2000_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_3_5_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	NULL,
	NULL
};

/* PIT8253 Interface */

/* TODO: clocks aren't known */
static const struct pit8253_config mz2000_pit8253_intf =
{
	{
		{
			31250,
			DEVCB_NULL,
			DEVCB_NULL
		},
		{
			31250, /* needed by "Art Magic" to boot */
			DEVCB_NULL,
			DEVCB_NULL
		},
		{
			31250,
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};


static const cassette_interface mz2000_cassette_interface =
{
	mz700_cassette_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED),
	"mz_cass",
	NULL
};

static MACHINE_CONFIG_START( mz2000, mz2000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_17_73447MHz/5) /* TODO: was 4 MHz, but otherwise cassette won't work */
	MCFG_CPU_PROGRAM_MAP(mz2000_map)
	MCFG_CPU_IO_MAP(mz2000_io)

	MCFG_MACHINE_RESET(mz2000)

	MCFG_I8255_ADD( "i8255_0", ppi8255_intf )
	MCFG_Z80PIO_ADD( "z80pio_1", 4000000, mz2000_pio1_intf )
	MCFG_PIT8253_ADD("pit", mz2000_pit8253_intf)

	MCFG_MB8877_ADD("mb8877a",mz2000_mb8877a_interface)
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(mz2000_floppy_interface)
	MCFG_SOFTWARE_LIST_ADD("flop_list","mz2000_flop")

	MCFG_CASSETTE_ADD( CASSETTE_TAG, mz2000_cassette_interface )
	MCFG_SOFTWARE_LIST_ADD("cass_list","mz2000_cass")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)

	MCFG_GFXDECODE(mz2000)
	MCFG_PALETTE_LENGTH(8)

	MCFG_VIDEO_START(mz2000)
	MCFG_SCREEN_UPDATE_STATIC(mz2000)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.15)
MACHINE_CONFIG_END



ROM_START( mz2000 )
	ROM_REGION( 0x1000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "mz20ipl.bin",0x0000, 0x0800, CRC(d7ccf37f) SHA1(692814ffc2cf50fa8bf9e30c96ebe4a9ee536a86))

	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )
	//ROM_LOAD( "vosque2000.mzt",0x0000, 0x80, CRC(1) SHA1(1))
	//ROM_CONTINUE( 0x0000, 0x7f80 )

	ROM_REGION( 0x1000, "tvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x10000, "gvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1800, "chargen", 0 )
//  ROM_LOAD( "mzfont.rom", 0x0000, 0x0800, BAD_DUMP CRC(0631efc3) SHA1(99b206af5c9845995733d877e9e93e9681b982a8) ) //original has JP characters
	/* these are hand-crafted roms, converted from bmps floating around the net */
	ROM_LOAD( "font.bin",    0x0000, 0x0800, BAD_DUMP CRC(6ae6ce8e) SHA1(6adcdab9e4647429dd8deb73146264746b5eccda) )
	ROM_LOAD( "font400.bin", 0x0800, 0x1000, BAD_DUMP CRC(56c5d2bc) SHA1(fea655ff5eedacf8978fa3c185485db44376e24d) )
ROM_END

ROM_START( mz2200 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "mz2200ipl.bin", 0x0000, 0x0800, CRC(476801e8) SHA1(6b1f0620945c5492475ea1694bd09a3fcf88549d) )

	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )
	//ROM_LOAD( "vosque2000.mzt",0x0000, 0x80, CRC(1) SHA1(1))
	//ROM_CONTINUE( 0x0000, 0x7f80 )

	ROM_REGION( 0x1000, "tvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x10000, "gvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1800, "chargen", 0 )
//  ROM_LOAD( "mzfont.rom", 0x0000, 0x0800, BAD_DUMP CRC(0631efc3) SHA1(99b206af5c9845995733d877e9e93e9681b982a8) ) //original has JP characters
	/* these are hand-crafted roms, converted from bmps floating around the net */
	ROM_LOAD( "font.bin",    0x0000, 0x0800, BAD_DUMP CRC(6ae6ce8e) SHA1(6adcdab9e4647429dd8deb73146264746b5eccda) )
	ROM_LOAD( "font400.bin", 0x0800, 0x1000, BAD_DUMP CRC(56c5d2bc) SHA1(fea655ff5eedacf8978fa3c185485db44376e24d) )
ROM_END


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE     INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1982, mz2000,   mz80b,    0,   mz2000,   mz2000, driver_device,  0, "Sharp",   "MZ-2000", GAME_NOT_WORKING )
COMP( 1982, mz2200,   mz80b,    0,   mz2000,   mz2000, driver_device,  0, "Sharp",   "MZ-2200", GAME_NOT_WORKING )
