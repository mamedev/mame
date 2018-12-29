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
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

static constexpr XTAL MAIN_CLOCK = 6.144_MHz_XTAL;

class fp200_state : public driver_device
{
public:
	fp200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void fp200(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(keyb_irq);

private:
	// devices
	required_device<i8085a_cpu_device> m_maincpu;
	uint8_t m_io_type;
	uint8_t *m_chargen;
	uint8_t m_keyb_mux;

	struct{
		uint8_t x;
		uint8_t y;
		uint8_t status;
		std::unique_ptr<uint8_t[]> vram;
		std::unique_ptr<uint8_t[]> attr;
	}m_lcd;
	uint8_t read_lcd_attr(uint16_t X, uint16_t Y);
	uint8_t read_lcd_vram(uint16_t X, uint16_t Y);
	void write_lcd_attr(uint16_t X, uint16_t Y,uint8_t data);
	void write_lcd_vram(uint16_t X, uint16_t Y,uint8_t data);

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(fp200_io_r);
	DECLARE_WRITE8_MEMBER(fp200_io_w);
	DECLARE_READ8_MEMBER(fp200_lcd_r);
	DECLARE_WRITE8_MEMBER(fp200_lcd_w);
	DECLARE_READ8_MEMBER(fp200_keyb_r);
	DECLARE_WRITE8_MEMBER(fp200_keyb_w);

	DECLARE_WRITE_LINE_MEMBER(sod_w);
	DECLARE_READ_LINE_MEMBER(sid_r);

	void fp200_palette(palette_device &palette) const;
	void fp200_io(address_map &map);
	void fp200_map(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
};

void fp200_state::video_start()
{
	m_lcd.vram = make_unique_clear<uint8_t[]>(20*64);
	m_lcd.attr = make_unique_clear<uint8_t[]>(20*64);
}

/* TODO: Very preliminary, I actually believe that the LCDC writes in a blitter fashion ... */
uint32_t fp200_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint16_t l_offs, r_offs;

	bitmap.fill(0, cliprect);

	l_offs = 0;
	r_offs = 0;
	for(int y=cliprect.top(); y<cliprect.bottom(); y++) // FIXME: off-by-one?
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

	for(int y=cliprect.top(); y<cliprect.bottom(); y++) // FIXME: off-by-one?
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

	for(int y=cliprect.top(); y<cliprect.bottom(); y++) // FIXME: off-by-one?
	{
		for(int x=0;x<80;x++)
		{
			uint16_t yoffs;

			yoffs = y + l_offs;

			if(yoffs >= 64)
				yoffs -= 64;

			if(m_lcd.attr[x/8+yoffs*20] == 0x60 || m_lcd.attr[x/8+yoffs*20] == 0x50)
			{
				uint8_t vram,pix;

				vram = m_lcd.vram[x/8+yoffs*20];
				pix = ((m_chargen[vram*8+(x & 7)]) >> (7-(yoffs & 7))) & 1;
				bitmap.pix16(y,x) = pix;
			}
			/*
			else if(m_lcd.attr[x/8+yoffs*20] == 0x40)
			{
			    uint8_t vram,pix;

			    vram = m_lcd.vram[x/8+yoffs*20];
			    pix = (vram) >> (7-(yoffs & 7)) & 1;
			    bitmap.pix16(y,x) = pix;
			}*/
		}
	}

	for(int y=cliprect.top(); y<cliprect.bottom(); y++) // FIXME: off-by-one?
	{
		for(int x=80;x<160;x++)
		{
			uint16_t yoffs;

			yoffs = y + r_offs;

			if(yoffs >= 64)
				yoffs -= 64;

			if(m_lcd.attr[x/8+yoffs*20] == 0x60 || m_lcd.attr[x/8+yoffs*20] == 0x50)
			{
				uint8_t vram,pix;

				vram = m_lcd.vram[x/8+yoffs*20];
				pix = ((m_chargen[vram*8+(x & 7)]) >> (7-(yoffs & 7))) & 1;
				bitmap.pix16(y,x) = pix;
			}
			/*else if(m_lcd.attr[x/8+yoffs*20] == 0x40)
			{
			    uint8_t vram,pix;

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
uint8_t fp200_state::read_lcd_attr(uint16_t X, uint16_t Y)
{
	uint16_t base_offs;
	uint8_t res = 0;

	for(int yi=0;yi<8;yi++)
	{
		base_offs = X+(Y+yi)*20;

		if(base_offs >= 20*64)
			return 0xff;

		res = m_lcd.attr[base_offs];
	}

	return res;
}

uint8_t fp200_state::read_lcd_vram(uint16_t X, uint16_t Y)
{
	uint16_t base_offs;
	uint8_t res = 0;

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
	uint8_t res;

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

void fp200_state::write_lcd_attr(uint16_t X, uint16_t Y,uint8_t data)
{
	uint16_t base_offs;

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

void fp200_state::write_lcd_vram(uint16_t X, uint16_t Y,uint8_t data)
{
	uint16_t base_offs;

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
	uint8_t res;

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
	uint8_t res;

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

void fp200_state::fp200_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
//  0xa000, 0xffff exp RAM
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xffff).ram();
}

void fp200_state::fp200_io(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(fp200_state::fp200_io_r), FUNC(fp200_state::fp200_io_w));
}

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

static GFXDECODE_START( gfx_fp200 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout,     0, 1 )
GFXDECODE_END


void fp200_state::machine_start()
{
	uint8_t *raw_gfx = memregion("raw_gfx")->base();
	m_chargen = memregion("chargen")->base();

	for(int i=0;i<0x800;i++)
	{
		m_chargen[i] = raw_gfx[bitswap<16>(i,15,14,13,12,11,6,5,4,3,10,9,8,7,2,1,0)];
	}
}

void fp200_state::machine_reset()
{
}


void fp200_state::fp200_palette(palette_device &palette) const
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

void fp200_state::fp200(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &fp200_state::fp200_map);
	m_maincpu->set_addrmap(AS_IO, &fp200_state::fp200_io);
	m_maincpu->in_sid_func().set(FUNC(fp200_state::sid_r));
	m_maincpu->out_sod_func().set(FUNC(fp200_state::sod_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(fp200_state::screen_update));
	screen.set_size(20*8, 8*8);
	screen.set_visarea(0*8, 20*8-1, 0*8, 8*8-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_fp200);

	PALETTE(config, "palette", FUNC(fp200_state::fp200_palette), 2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}


/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( fp200 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fp200rom.bin", 0x0000, 0x8000, CRC(dba6e41b) SHA1(c694fa19172eb56585a9503997655bcf9d369c34) )

	ROM_REGION( 0x800, "raw_gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "chr.bin", 0x0000, 0x800, CRC(2e6501a5) SHA1(6186e25feabe6db851ee7d61dad11e182a6d3a4a) )

	ROM_REGION( 0x800, "chargen", ROMREGION_ERASE00 )
ROM_END

COMP( 1982, fp200, 0, 0, fp200, fp200, fp200_state, empty_init, "Casio", "FP-200 (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
