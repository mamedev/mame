// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    FP-200 (c) 1982 Casio

    preliminary driver by Angelo Salese

    TODO:
    - What's the LCDC type? Custom?
    - Unless I've missed something in the schems, this one shouldn't have any
      sound capability.
    - backup RAM.
    - Rewrite video emulation from scratch.

    Notes:
    - on start-up there's a "memory illegal" warning. Enter "RESET" command
      to initialize it (thanks to Takeda Toshiya for pointing this out).

***************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"

#define MAIN_CLOCK XTAL_6_144MHz

class fp200_state : public driver_device
{
public:
	fp200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	UINT8 m_io_type;
	UINT8 *m_chargen;
	UINT8 m_keyb_mux;

	struct{
		UINT8 x;
		UINT8 y;
		UINT8 status;
		UINT8 *vram;
		UINT8 *attr;
	}m_lcd;
	UINT8 read_lcd_attr(UINT16 X, UINT16 Y);
	UINT8 read_lcd_vram(UINT16 X, UINT16 Y);
	void write_lcd_attr(UINT16 X, UINT16 Y,UINT8 data);
	void write_lcd_vram(UINT16 X, UINT16 Y,UINT8 data);

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(fp200_io_r);
	DECLARE_WRITE8_MEMBER(fp200_io_w);
	DECLARE_READ8_MEMBER(fp200_lcd_r);
	DECLARE_WRITE8_MEMBER(fp200_lcd_w);
	DECLARE_READ8_MEMBER(fp200_keyb_r);
	DECLARE_WRITE8_MEMBER(fp200_keyb_w);
	DECLARE_INPUT_CHANGED_MEMBER(keyb_irq);

	DECLARE_WRITE_LINE_MEMBER(sod_w);
	DECLARE_READ_LINE_MEMBER(sid_r);

	DECLARE_PALETTE_INIT(fp200);
protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};

void fp200_state::video_start()
{
	m_lcd.vram = auto_alloc_array_clear(machine(), UINT8, 20*64);
	m_lcd.attr = auto_alloc_array_clear(machine(), UINT8, 20*64);
}

/* TODO: Very preliminary, I actually believe that the LCDC writes in a blitter fashion ... */
UINT32 fp200_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT16 l_offs, r_offs;

	bitmap.fill(0, cliprect);

	l_offs = 0;
	r_offs = 0;
	for(int y=cliprect.min_y;y<cliprect.max_y;y++)
	{
		for(int x=0;x<80;x++)
		{
			if(m_lcd.attr[x/8+y*20] == 0x50)
			{
				l_offs = (y & ~7);
				break;
			}
		}
	}

	for(int y=cliprect.min_y;y<cliprect.max_y;y++)
	{
		for(int x=80;x<160;x++)
		{
			if(m_lcd.attr[x/8+y*20] == 0x50)
			{
				r_offs = (y & ~7);
				break;
			}
		}
	}

	for(int y=cliprect.min_y;y<cliprect.max_y;y++)
	{
		for(int x=0;x<80;x++)
		{
			UINT16 yoffs;

			yoffs = y + l_offs;

			if(yoffs >= 64)
				yoffs -= 64;

			if(m_lcd.attr[x/8+yoffs*20] == 0x60 || m_lcd.attr[x/8+yoffs*20] == 0x50)
			{
				UINT8 vram,pix;

				vram = m_lcd.vram[x/8+yoffs*20];
				pix = ((m_chargen[vram*8+(x & 7)]) >> (7-(yoffs & 7))) & 1;
				bitmap.pix16(y,x) = pix;
			}
			/*
			else if(m_lcd.attr[x/8+yoffs*20] == 0x40)
			{
			    UINT8 vram,pix;

			    vram = m_lcd.vram[x/8+yoffs*20];
			    pix = (vram) >> (7-(yoffs & 7)) & 1;
			    bitmap.pix16(y,x) = pix;
			}*/
		}
	}

	for(int y=cliprect.min_y;y<cliprect.max_y;y++)
	{
		for(int x=80;x<160;x++)
		{
			UINT16 yoffs;

			yoffs = y + r_offs;

			if(yoffs >= 64)
				yoffs -= 64;

			if(m_lcd.attr[x/8+yoffs*20] == 0x60 || m_lcd.attr[x/8+yoffs*20] == 0x50)
			{
				UINT8 vram,pix;

				vram = m_lcd.vram[x/8+yoffs*20];
				pix = ((m_chargen[vram*8+(x & 7)]) >> (7-(yoffs & 7))) & 1;
				bitmap.pix16(y,x) = pix;
			}
			/*else if(m_lcd.attr[x/8+yoffs*20] == 0x40)
			{
			    UINT8 vram,pix;

			    vram = m_lcd.vram[x/8+yoffs*20];
			    pix = (vram) >> (7-(yoffs & 7)) & 1;
			    bitmap.pix16(y,x) = pix;
			}*/
		}
	}

	return 0;
}


/*
[1] DDDD DDDD vram data/attr (left half)
[2] DDDD DDDD vram data/attr (right half)
[8] SSSS --YY Status code (1=vram type/0xb=attr type) / upper part of Y address
[9] YYYY XXXX lower part of Y address / X address
*/
UINT8 fp200_state::read_lcd_attr(UINT16 X, UINT16 Y)
{
	UINT16 base_offs;
	UINT8 res = 0;

	for(int yi=0;yi<8;yi++)
	{
		base_offs = X+(Y+yi)*20;

		if(base_offs >= 20*64)
			return 0xff;

		res = m_lcd.attr[base_offs];
	}

	return res;
}

UINT8 fp200_state::read_lcd_vram(UINT16 X, UINT16 Y)
{
	UINT16 base_offs;
	UINT8 res = 0;

	for(int yi=0;yi<8;yi++)
	{
		base_offs = X+(Y+yi)*20;

		if(base_offs >= 20*64)
			return 0xff;

		res = m_lcd.vram[base_offs];
	}

	return res;
}

READ8_MEMBER(fp200_state::fp200_lcd_r)
{
	UINT8 res;

	res = 0;

	switch(offset)
	{
		case 1:
			//printf("%d %d -> (L) %02x\n",m_lcd.x,m_lcd.y,m_lcd.status);
			if(m_lcd.status == 0xb)
				res = read_lcd_attr(m_lcd.x,m_lcd.y);
			else if(m_lcd.status == 1)
				res = read_lcd_vram(m_lcd.x,m_lcd.y);
			break;
		case 2:
			//printf("%d %d -> (R) %02x\n",m_lcd.x,m_lcd.y,m_lcd.status);
			if(m_lcd.status == 0xb)
				res = read_lcd_attr(m_lcd.x + 10,m_lcd.y);
			else if(m_lcd.status == 1)
				res = read_lcd_vram(m_lcd.x + 10,m_lcd.y);
			break;
		case 8:
			res =  (m_lcd.status & 0xf) << 4;
			res |= (m_lcd.y & 0x30) >> 4;
			break;
		case 9:
			res =  (m_lcd.y & 0xf) << 4;
			res |= (m_lcd.x & 0xf);
			break;
	}


	return res;
}

void fp200_state::write_lcd_attr(UINT16 X, UINT16 Y,UINT8 data)
{
	UINT16 base_offs;

	for(int yi=0;yi<8;yi++)
	{
		base_offs = X+(Y+yi)*20;

		if(base_offs >= 20*64)
			return;

		//if(data != 0x60)
		//  printf("%d %d %02x\n",X,Y,data);

		m_lcd.attr[base_offs] = data;
	}
}

void fp200_state::write_lcd_vram(UINT16 X, UINT16 Y,UINT8 data)
{
	UINT16 base_offs;

	for(int yi=0;yi<8;yi++)
	{
		base_offs = X+(Y+yi)*20;

		if(base_offs >= 20*64)
			return;

		m_lcd.vram[base_offs] = data;
	}
}

WRITE8_MEMBER(fp200_state::fp200_lcd_w)
{
	switch(offset)
	{
		case 1:
			//printf("%d %d -> %02x (%c) (L %02x)\n",m_lcd.x,m_lcd.y,data,data,m_lcd.status);
			if(m_lcd.status == 0xb)
				write_lcd_attr(m_lcd.x,m_lcd.y,data);
			else if(m_lcd.status == 1)
				write_lcd_vram(m_lcd.x,m_lcd.y,data);
			break;
		case 2:
			//printf("%d %d -> %02x (%c) (R %02x)\n",m_lcd.x + 10,m_lcd.y,data,data,m_lcd.status);
			if(m_lcd.status == 0xb)
				write_lcd_attr(m_lcd.x + 10,m_lcd.y,data);
			else if(m_lcd.status == 1)
				write_lcd_vram(m_lcd.x + 10,m_lcd.y,data);
			break;
		case 8:
			m_lcd.status = (data & 0xf0) >> 4;
			if(m_lcd.status == 0x0b)
				m_lcd.y = (m_lcd.y & 0xf) | ((data & 3) << 4);
			break;
		case 9:
			m_lcd.y = (m_lcd.y & 0x30) | ((data & 0xf0) >> 4);
			m_lcd.x = data & 0xf;
			break;
	}
}

READ8_MEMBER(fp200_state::fp200_keyb_r)
{
	const char *const keynames[16] = { "KEY0", "KEY1", "KEY2", "KEY3",
										"KEY4", "KEY5", "KEY6", "KEY7",
										"KEY8", "KEY9", "UNUSED", "UNUSED",
										"UNUSED", "UNUSED", "UNUSED", "UNUSED"};
	UINT8 res;

	if(offset == 0)
		res = ioport(keynames[m_keyb_mux])->read();
	else
	{
		printf("Unknown keyboard offset read access %02x\n",offset + 0x20);
		res = 0;
	}

	return res;
}

WRITE8_MEMBER(fp200_state::fp200_keyb_w)
{
	if(offset == 1)
		m_keyb_mux = data & 0xf;
	else if(offset == 0)
	{
		// ... ?
	}
	else
		printf("Unknown keyboard offset write access %02x %02x\n",offset + 0x20,data);
}

/*
Annoyingly the i/o map uses the SOD to access different devices, so we need trampolines.
SOD = 0
0x10 - 0x1f Timer control (RPC05 RTC)
0x20 - 0x2f AUTO-POWER OFF
0x40 - 0x4f FDC Device ID Code (5 for "FP-1021FD")
0x80 - 0xff FDD (unknown type)
SOD = 1
0x00 - 0x0f LCD control.
0x10 - 0x1f I/O control
0x20 - 0x2f Keyboard
0x40 - 0x4f MT.RS-232C control
0x80 - 0x8f Printer (Centronics)
*/
READ8_MEMBER(fp200_state::fp200_io_r)
{
	UINT8 res;

	if(m_io_type == 0)
	{
		res = 0;
		logerror("Unemulated I/O read %02x (%02x)\n",offset,m_io_type);
	}
	else
	{
		switch(offset & 0xf0)
		{
			//case 0x00: return;
			case 0x00: res = fp200_lcd_r(space, offset & 0xf); break;
			case 0x20: res = fp200_keyb_r(space, offset & 0xf); break;
			default: res = 0; logerror("Unemulated I/O read %02x (%02x)\n",offset,m_io_type); break;
		}
	}

	return res;
}

WRITE8_MEMBER(fp200_state::fp200_io_w)
{
	if(m_io_type == 0)
	{
		switch(offset & 0xf0)
		{
			default:logerror("Unemulated I/O write %02x (%02x) <- %02x\n",offset,m_io_type,data); break;
		}
	}
	else
	{
		switch(offset & 0xf0)
		{
			case 0x00: fp200_lcd_w(space, offset & 0xf,data); break;
			case 0x20: fp200_keyb_w(space, offset & 0xf,data); break;
			default:logerror("Unemulated I/O write %02x (%02x) <- %02x\n",offset,m_io_type,data); break;
		}
	}
}

static ADDRESS_MAP_START( fp200_map, AS_PROGRAM, 8, fp200_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
//  0xa000, 0xffff exp RAM
	AM_RANGE(0xa000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fp200_io, AS_IO, 8, fp200_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(fp200_io_r,fp200_io_w)
ADDRESS_MAP_END

INPUT_CHANGED_MEMBER(fp200_state::keyb_irq)
{
	/* a keyboard stroke causes a rst7.5 */
	m_maincpu->set_input_line(I8085_RST75_LINE, (newval) ? ASSERT_LINE : CLEAR_LINE);

}

/* TODO: remote SW? */
static INPUT_PORTS_START( fp200 )
	PORT_START("KEY0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 / '") PORT_CODE(KEYCODE_7) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 / (") PORT_CODE(KEYCODE_8) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 / )") PORT_CODE(KEYCODE_9) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". / >") PORT_CODE(KEYCODE_STOP) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY3")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", / <") PORT_CODE(KEYCODE_COMMA) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("; / +") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY4")
//  PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) //PORT_TOGGLE PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INS / DEL") PORT_CODE(KEYCODE_INSERT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HOME / CLS") PORT_CODE(KEYCODE_HOME) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF0 / PF5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 / !") PORT_CODE(KEYCODE_1) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STOP / CONT") PORT_CODE(KEYCODE_RIGHT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\ / |") PORT_CODE(KEYCODE_RIGHT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF1 / PF6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 / \"") PORT_CODE(KEYCODE_2) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("^ / ~") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF2 / PF7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 / #") PORT_CODE(KEYCODE_3) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@ / '") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("- / =") PORT_CODE(KEYCODE_MINUS) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF3 / PF8") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 / $") PORT_CODE(KEYCODE_4) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(": / *") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF4 / PF9") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 / %") PORT_CODE(KEYCODE_5) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("] / }") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 / &") PORT_CODE(KEYCODE_6) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEYMOD")
	PORT_BIT( 0x01f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CETL") PORT_TOGGLE
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPH")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	8*8
};

static GFXDECODE_START( fp200 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout,     0, 1 )
GFXDECODE_END


void fp200_state::machine_start()
{
	UINT8 *raw_gfx = memregion("raw_gfx")->base();
	m_chargen = memregion("chargen")->base();

	for(int i=0;i<0x800;i++)
	{
		m_chargen[i] = raw_gfx[BITSWAP16(i,15,14,13,12,11,6,5,4,3,10,9,8,7,2,1,0)];
	}
}

void fp200_state::machine_reset()
{
}


PALETTE_INIT_MEMBER(fp200_state, fp200)
{
	palette.set_pen_color(0, 0xa0, 0xa8, 0xa0);
	palette.set_pen_color(1, 0x30, 0x38, 0x10);
}

WRITE_LINE_MEMBER( fp200_state::sod_w )
{
	m_io_type = state;
}

READ_LINE_MEMBER( fp200_state::sid_r )
{
	return (ioport("KEYMOD")->read() >> m_keyb_mux) & 1;
}

static MACHINE_CONFIG_START( fp200, fp200_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(fp200_map)
	MCFG_CPU_IO_MAP(fp200_io)
	MCFG_I8085A_SID(READLINE(fp200_state, sid_r))
	MCFG_I8085A_SOD(WRITELINE(fp200_state, sod_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(fp200_state, screen_update)
	MCFG_SCREEN_SIZE(20*8, 8*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 20*8-1, 0*8, 8*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fp200)

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(fp200_state, fp200)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( fp200 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fp200rom.bin", 0x0000, 0x8000, CRC(dba6e41b) SHA1(c694fa19172eb56585a9503997655bcf9d369c34) )

	ROM_REGION( 0x800, "raw_gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "chr.bin", 0x0000, 0x800, CRC(2e6501a5) SHA1(6186e25feabe6db851ee7d61dad11e182a6d3a4a) )

	ROM_REGION( 0x800, "chargen", ROMREGION_ERASE00 )
ROM_END

COMP( 1982, fp200,  0,   0,   fp200,  fp200, driver_device,  0,  "Casio",      "FP-200 (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
