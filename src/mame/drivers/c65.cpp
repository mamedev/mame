// license:LGPL-2.1+
// copyright-holders: Angelo Salese
/***************************************************************************

C=65 / C=64DX (c) 1991 Commodore

Attempt at rewriting the driver ...

Note:
- VIC-4567 will be eventually be added via compile switch, once that I
  get the hang of the system (and checking where the old code fails
  eventually)

Hardware infos can be found at:
http://www.zimmers.net/cbmpics/cbm/c65/c65manual.txt
http://www.zimmers.net/cbmpics/cbm/c65/c65faq20.txt

Hardware pics:
http://www.zimmers.net/cbmpics/cbm/c65/c65-2b-lhs.JPG
http://www.zimmers.net/cbmpics/cbm/c65/c65-2b-rhs.JPG

Schematics:
http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/C65%20Rev%202A%20Schematic.pdf
http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/C64DX_aka_C65_System_Specifications_Preliminary_(1991_Mar).pdf

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m4510.h"
#include "machine/mos6526.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#define MAIN_CLOCK XTAL(28'375'160)/8

class c65_state : public driver_device
{
public:
	c65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cia0(*this, "cia_0")
		, m_cia1(*this, "cia_1")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_workram(*this, "wram")
		, m_palred(*this, "redpal")
		, m_palgreen(*this, "greenpal")
		, m_palblue(*this, "bluepal")
		, m_dmalist(*this, "dmalist")
		, m_cram(*this, "cram")
		, m_gfxdecode(*this, "gfxdecode")
	{ }

	// devices
	required_device<m4510_device> m_maincpu;
	required_device<mos6526_device> m_cia0;
	required_device<mos6526_device> m_cia1;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_palred;
	required_shared_ptr<uint8_t> m_palgreen;
	required_shared_ptr<uint8_t> m_palblue;
	required_shared_ptr<uint8_t> m_dmalist;
	required_shared_ptr<uint8_t> m_cram;
	required_device<gfxdecode_device> m_gfxdecode;

	uint8_t *m_iplrom = nullptr;
	uint8_t m_keyb_input[10]{};
	uint8_t m_keyb_c0_c7 = 0U;
	uint8_t m_keyb_c8_c9 = 0U;

	uint8_t vic4567_dummy_r(offs_t offset);
	void vic4567_dummy_w(offs_t offset, uint8_t data);
	void PalRed_w(offs_t offset, uint8_t data);
	void PalGreen_w(offs_t offset, uint8_t data);
	void PalBlue_w(offs_t offset, uint8_t data);
	uint8_t uart_r(offs_t offset);
	void uart_w(offs_t offset, uint8_t data);
	void DMAgic_w(address_space &space, offs_t offset, uint8_t data);
	uint8_t CIASelect_r(offs_t offset);
	void CIASelect_w(offs_t offset, uint8_t data);
	uint8_t cia0_porta_r();
	void cia0_porta_w(uint8_t data);
	uint8_t cia0_portb_r();
	void cia0_portb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(cia0_irq);

	uint8_t dummy_r();

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void c65_palette(palette_device &palette);
	void init_c65();
	void init_c65pal();

	INTERRUPT_GEN_MEMBER(vic3_vblank_irq);
	void c65(machine_config &config);
	void c65_map(address_map &map);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
private:
	uint8_t m_VIC2_IRQPend = 0U, m_VIC2_IRQMask = 0U;
	/* 0x20: border color (TODO: different thread?) */
	uint8_t m_VIC2_EXTColor = 0U;
	uint8_t m_VIC2_VS_CB_Base = 0U;
	uint8_t m_VIC2_BK0_Color = 0U;
	/* 0x30: banking + PAL + EXT SYNC */
	uint8_t m_VIC3_ControlA = 0U;
	/* 0x31: video modes */
	uint8_t m_VIC3_ControlB = 0U;
	void PalEntryFlush(uint8_t offset);
	void DMAgicExecute(address_space &space,uint32_t address);
	void IRQCheck(uint8_t irq_cause);
	int inner_x_char(int xoffs);
	int inner_y_char(int yoffs);
};

void c65_state::video_start()
{
}

// TODO: inline?
int c65_state::inner_x_char(int xoffs)
{
	return xoffs>>3;
}

int c65_state::inner_y_char(int yoffs)
{
	return yoffs>>3;
}

uint32_t c65_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int pixel_width = (m_VIC3_ControlB & 0x80) ? 1 : 2;
	int columns = 80 / pixel_width;

	uint8_t *cptr = &m_iplrom[((m_VIC3_ControlA & 0x40) ? 0x9000: 0xd000) + ((m_VIC2_VS_CB_Base & 0x2) << 10)];

	// TODO: border area
	for(int y=cliprect.top();y<=cliprect.bottom();y++)
	{
		for(int x=cliprect.left();x<=cliprect.right();x++)
		{
			//int, xi,yi,xm,ym,dot_x;
			int xi = inner_x_char(x / pixel_width);
			int yi = inner_y_char(y);
			int xm = 7 - ((x / pixel_width) & 7);
			int ym = (y & 7);
			uint8_t tile = m_workram[xi+yi*columns+0x800];
			uint8_t attr = m_cram[xi+yi*columns];
			int foreground_color = attr & 0xf;
			int background_color = m_VIC2_BK0_Color & 0xf;
			int highlight_color = 0;

			int enable_dot = ((cptr[(tile<<3)+ym] >> xm) & 1);

			if (attr & 0x10)
			{
				if ((machine().time().attoseconds() / (ATTOSECONDS_PER_SECOND / 2)) & 1)
					attr &= 0x0f;
				else if ((attr & 0xf0) != 0x10)
					attr &= ~0x10;
			}

			if ((attr & 0x80) && ym == 7) enable_dot = 1;
			if (attr & 0x40) highlight_color = 16;
			if (attr & 0x20) enable_dot = !enable_dot;
			if (attr & 0x10) enable_dot = 0;

			//if(cliprect.contains(x, y))
			bitmap.pix(y, x) = m_palette->pen(highlight_color + ((enable_dot) ? foreground_color : background_color));


			//gfx->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);
		}
	}

	return 0;
}

uint8_t c65_state::vic4567_dummy_r(offs_t offset)
{
	uint8_t res;

	res=0xff;
	switch(offset)
	{
		case 0x11:
			res = (m_screen->vpos() & 0x100) >> 1;
			return res;
		case 0x12:
			res = (m_screen->vpos() & 0xff);
			return res;
		case 0x15:
			return 0xff; // silence log for now

		case 0x18:
			return m_VIC2_VS_CB_Base;

		case 0x19:
			return m_VIC2_IRQPend;

		case 0x1a:
			return m_VIC2_IRQMask;

		case 0x20:
			return m_VIC2_EXTColor;

		case 0x21:
			return m_VIC2_BK0_Color;

		case 0x30:
			return m_VIC3_ControlA;
		case 0x31:
			return m_VIC3_ControlB;
	}

	if(!machine().side_effects_disabled())
		printf("%02x\n",offset); // TODO: PC
	return res;
}

void c65_state::vic4567_dummy_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x18:
			m_VIC2_VS_CB_Base = data;
			break;
		case 0x19:
			m_VIC2_IRQPend &= ~data;
			IRQCheck(0);
			break;
		case 0x1a:
			m_VIC2_IRQMask = data & 0xf;
			IRQCheck(0);
			break;
		case 0x20:
			m_VIC2_EXTColor = data & 0xf;
			break;
		case 0x21:
			m_VIC2_BK0_Color = data & 0xf;
			break;

		/* KEY register, handles vic-iii and vic-ii modes via two consecutive writes
		  0xa5 -> 0x96 vic-iii mode
		  any other write vic-ii mode
		  */
		//case 0x2f: break;
		case 0x30:
			if((data & 0xfe) != 0x64)
				printf("CONTROL A %02x\n",data);
			m_VIC3_ControlA = data;
			break;
		case 0x31:
			printf("CONTROL B %02x\n",data);
			m_VIC3_ControlB = data;
			break;
		default:
			printf("%02x %02x\n", offset, data);
			break;
	}

}

void c65_state::PalEntryFlush(uint8_t offset)
{
	m_palette->set_pen_color(offset, pal4bit(m_palred[offset]), pal4bit(m_palgreen[offset]), pal4bit(m_palblue[offset]));
}

void c65_state::PalRed_w(offs_t offset, uint8_t data)
{
	m_palred[offset] = data;
	PalEntryFlush(offset);
}

void c65_state::PalGreen_w(offs_t offset, uint8_t data)
{
	m_palgreen[offset] = data;
	PalEntryFlush(offset);
}

void c65_state::PalBlue_w(offs_t offset, uint8_t data)
{
	m_palblue[offset] = data;
	PalEntryFlush(offset);
}

uint8_t c65_state::uart_r(offs_t offset)
{
	switch (offset)
	{
	case 7:
		return ioport("CAPS")->read();
	}
	return 0xff;
}

void c65_state::uart_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 7:
		m_keyb_c8_c9 = (~data >> 1) & 3;
		break;
	case 8:
		// ddr?
		break;
	}
}

void c65_state::DMAgicExecute(address_space &space,uint32_t address)
{
	uint8_t cmd;// = space.read_byte(address++);
	uint16_t length; //= space.read_byte(address++);
	uint32_t src, dst;
	static const char *const dma_cmd_string[] =
	{
		"COPY",                 // 0
		"MIX",
		"SWAP",
		"FILL"
	};
	cmd = space.read_byte(address++);
	length = space.read_byte(address++);
	length|=(space.read_byte(address++)<<8);
	src = space.read_byte(address++);
	src|=(space.read_byte(address++)<<8);
	src|=(space.read_byte(address++)<<16);
	dst = space.read_byte(address++);
	dst|=(space.read_byte(address++)<<8);
	dst|=(space.read_byte(address++)<<16);

	if(cmd & 0xfc)
		printf("%02x\n",cmd & 0xfc);
	switch(cmd & 3)
	{
		case 0: // copy - TODO: untested
		{
				if(length != 1)
					printf("DMAgic %s %02x -> %08x %04x (CHAIN=%s)\n",dma_cmd_string[cmd & 3],src,dst,length,cmd & 4 ? "yes" : "no");
				uint32_t SourceIndex;
				uint32_t DestIndex;
				uint16_t SizeIndex;
				SourceIndex = src & 0xfffff;
				DestIndex = dst & 0xfffff;
				SizeIndex = length;
				do
				{
					space.write_byte(DestIndex++,space.read_byte(SourceIndex++));
					SizeIndex--;
				}while(SizeIndex != 0);

			return;
		}
		case 3: // fill
			{
				/* TODO: upper bits of source */
				printf("DMAgic %s %02x -> %08x %04x (CHAIN=%s)\n",dma_cmd_string[cmd & 3],src & 0xff,dst,length,cmd & 4 ? "yes" : "no");
				uint8_t FillValue;
				uint32_t DestIndex;
				uint16_t SizeIndex;
				FillValue = src & 0xff;
				DestIndex = dst & 0xfffff;
				SizeIndex = length;
				do
				{
					space.write_byte(DestIndex++,FillValue);
					SizeIndex--;
				}while(SizeIndex != 0);
			}
			return;
	}
	printf("DMAgic %s %08x %08x %04x (CHAIN=%s)\n",dma_cmd_string[cmd & 3],src,dst,length,cmd & 4 ? "yes" : "no");
}


void c65_state::DMAgic_w(address_space &space, offs_t offset, uint8_t data)
{
	m_dmalist[offset] = data;
	if(offset == 0)
		DMAgicExecute(space,(m_dmalist[0])|(m_dmalist[1]<<8)|(m_dmalist[2]<<16));
}

uint8_t c65_state::CIASelect_r(offs_t offset)
{
	if(m_VIC3_ControlA & 1)
		return m_cram[offset];
	else
	{
		// CIA at 0xdc00
		switch((offset & 0x700) | 0x800)
		{
			case 0xc00:
				return m_cia0->read(offset);
			case 0xd00:
				return m_cia1->read(offset);
			default:
				printf("Unknown I/O access read to offset %04x\n",offset);
				break;
		}

	}

	return 0xff;
}

void c65_state::CIASelect_w(offs_t offset, uint8_t data)
{
	if(m_VIC3_ControlA & 1)
		m_cram[offset] = data;
	else
	{
		// CIA at 0xdc00
		switch((offset & 0x700) | 0x800)
		{
			case 0xc00:
				m_cia0->write(offset, data);
				break;

			case 0xd00:
				m_cia1->write(offset, data);
				break;
			default:
				printf("Unknown I/O access write to offset %04x data = %02x\n", offset, data);
				break;
		}
	}

}

uint8_t c65_state::cia0_porta_r()
{
	return 0xff;
}

uint8_t c65_state::cia0_portb_r()
{
	static const char *const c64ports[] = { "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7" };
	static const char *const c65ports[] = { "C8", "C9" };
	uint8_t res;

	res = 0xff;
	for(int i=0;i<8;i++)
	{
		m_keyb_input[i] = ioport(c64ports[i])->read();

		if(m_keyb_c0_c7 & 1 << (i))
			res &= m_keyb_input[i];
	}

	for(int i=0;i<2;i++)
	{
		m_keyb_input[i+8] = ioport(c65ports[i])->read();

		if(m_keyb_c8_c9 & 1 << (i))
			res &= m_keyb_input[i+8];
	}

	return res;
}

void c65_state::cia0_porta_w(uint8_t data)
{
	m_keyb_c0_c7 = ~data;
//  printf("%02x\n",m_keyb_c0_c7);
}

void c65_state::cia0_portb_w(uint8_t data)
{
}

uint8_t c65_state::dummy_r()
{
	return 0;
}

void c65_state::c65_map(address_map &map)
{
	map(0x00000, 0x07fff).ram().share("wram"); // TODO: bank
	map(0x0c800, 0x0cfff).rom().region("maincpu", 0xc800);
	map(0x0d000, 0x0d07f).rw(FUNC(c65_state::vic4567_dummy_r), FUNC(c65_state::vic4567_dummy_w)); // 0x0d000, 0x0d07f VIC-4567
	map(0x0d080, 0x0d081).r(FUNC(c65_state::dummy_r)); // 0x0d080, 0x0d09f FDC
	// 0x0d0a0, 0x0d0ff Ram Expansion Control (REC)
	map(0x0d100, 0x0d1ff).ram().w(FUNC(c65_state::PalRed_w)).share("redpal");// 0x0d100, 0x0d1ff Red Palette
	map(0x0d200, 0x0d2ff).ram().w(FUNC(c65_state::PalGreen_w)).share("greenpal"); // 0x0d200, 0x0d2ff Green Palette
	map(0x0d300, 0x0d3ff).ram().w(FUNC(c65_state::PalBlue_w)).share("bluepal"); // 0x0d300, 0x0d3ff Blue Palette
	// 0x0d400, 0x0d4*f Right SID
	// 0x0d440, 0x0d4*f Left  SID
	map(0x0d600, 0x0d6ff).rw(FUNC(c65_state::uart_r), FUNC(c65_state::uart_w));
	map(0x0d700, 0x0d702).w(FUNC(c65_state::DMAgic_w)).share("dmalist"); // 0x0d700, 0x0d7** DMAgic
	//map(0x0d703, 0x0d703).r(FUNC(c65_state::DMAgic_r));
	// 0x0d800, 0x0d8** Color matrix
	map(0x0d800, 0x0dfff).rw(FUNC(c65_state::CIASelect_r), FUNC(c65_state::CIASelect_w)).share("cram");
	// 0x0dc00, 0x0dc** CIA-1
	// 0x0dd00, 0x0dd** CIA-2
	// 0x0de00, 0x0de** Ext I/O Select 1
	// 0x0df00, 0x0df** Ext I/O Select 2 (RAM window?)
	map(0x0e000, 0x0ffff).rom().region("maincpu", 0x0e000);
	map(0x10000, 0x1f7ff).ram();
	map(0x1f800, 0x1ffff).ram(); // VRAM attributes
	map(0x20000, 0x3ffff).rom().region("maincpu", 0);
}



static INPUT_PORTS_START( c65 )
	PORT_START( "C0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INST DEL") PORT_CODE(KEYCODE_BACKSPACE)       PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)             PORT_CHAR(13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)                                    PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                                    PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                    PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                                    PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START( "C1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START( "C2" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('T')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('X')

	PORT_START( "C3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('V')

	PORT_START( "C4" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('K')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('O')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('N')

	PORT_START( "C5" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')

	PORT_START( "C6" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR(0xA3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR HOME") PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/') PORT_CHAR('?')

	PORT_START( "C7" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE)   PORT_CHAR(0x2190)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL)                          PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN STOP") PORT_CODE(KEYCODE_HOME)

	PORT_START( "C8" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NO SCROLL") PORT_CODE(KEYCODE_SCRLOCK)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)                                    PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6)                                    PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7)                                    PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)

	PORT_START( "C9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("linefeed?")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("cursor left?")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN8")

	PORT_START( "CAPS" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_F8) PORT_TOGGLE
INPUT_PORTS_END


void c65_state::machine_start()
{
	m_iplrom = memregion("maincpu")->base();

	save_pointer(NAME(m_cram.target()), 0x800);
}

void c65_state::machine_reset()
{
}


void c65_state::c65_palette(palette_device &palette)
{
	for (int i = 0; i < 0x100; i++)
		PalEntryFlush(i);
}

static const gfx_layout charlayout =
{
	8,8,
	0x1000/8,
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_c65 )
	GFXDECODE_ENTRY( "maincpu", 0xd000, charlayout,     0, 16 ) // another identical copy is at 0x9000
GFXDECODE_END

void c65_state::IRQCheck(uint8_t irq_cause)
{
	m_VIC2_IRQPend |= (irq_cause != 0) ? 0x80 : 0x00;
	m_VIC2_IRQPend |= irq_cause;

	m_maincpu->set_input_line(M4510_IRQ_LINE,m_VIC2_IRQMask & m_VIC2_IRQPend ? ASSERT_LINE : CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(c65_state::vic3_vblank_irq)
{
	IRQCheck(1);
	//if(m_VIC2_IRQMask & 1)
	//  m_maincpu->set_input_line(M4510_IRQ_LINE,HOLD_LINE);
}

WRITE_LINE_MEMBER(c65_state::cia0_irq)
{
	printf("%d IRQ\n",state);

#if 0
	if(state)
	{
		static const char *const c64ports[] = { "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7" };
		for(int i=0;i<8;i++)
			m_keyb_input[i] = ioport(c64ports[i])->read();
	}
#endif
//  m_cia0_irq = state;
//  c65_irq(state || m_vicirq);
}

void c65_state::c65(machine_config &config)
{
	/* basic machine hardware */
	M4510(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &c65_state::c65_map);
	m_maincpu->set_vblank_int("screen", FUNC(c65_state::vic3_vblank_irq));

	MOS6526(config, m_cia0, MAIN_CLOCK);
	m_cia0->set_tod_clock(60);
	m_cia0->irq_wr_callback().set(FUNC(c65_state::cia0_irq));
	m_cia0->pa_rd_callback().set(FUNC(c65_state::cia0_porta_r));
	m_cia0->pa_wr_callback().set(FUNC(c65_state::cia0_porta_w));
	m_cia0->pb_rd_callback().set(FUNC(c65_state::cia0_portb_r));
	m_cia0->pb_wr_callback().set(FUNC(c65_state::cia0_portb_w));

	MOS6526(config, m_cia1, MAIN_CLOCK);
	m_cia1->set_tod_clock(60);
//  m_cia1->irq_wr_callback().set(FUNC(c65_state::c65_cia1_interrupt));
//  m_cia1->pa_rd_callback().set(FUNC(c65_state::c65_cia1_port_a_r));
//  m_cia1->pa_wr_callback().set(FUNC(c65_state::c65_cia1_port_a_w));


	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(60);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(c65_state::screen_update));
//  m_screen->set_size(32*8, 32*8);
//  m_screen->set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	m_screen->set_raw(MAIN_CLOCK*4, 910, 0, 640, 262, 0, 200); // mods needed
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_c65);

	PALETTE(config, m_palette, FUNC(c65_state::c65_palette), 0x100);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	// 2x 8580 SID

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("c65_flop");
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( c65 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "910111", "V0.9.910111" ) // sum16 CAFF, this shows up on the picture from a spare, unused rom on the 20171102 c64dx auction as "390488-02 CAFF" with the 02 scratched off on the chip and 03 written in pen, unclear what the "correct" label is.
	ROMX_LOAD( "910111.bin", 0x0000, 0x20000, CRC(c5d8d32e) SHA1(71c05f098eff29d306b0170e2c1cdeadb1a5f206), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "910523", "V0.9.910523" ) // sum16 B96B
	ROMX_LOAD( "910523.bin", 0x0000, 0x20000, CRC(e8235dd4) SHA1(e453a8e7e5b95de65a70952e9d48012191e1b3e7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "910626", "V0.9.910626" ) // sum16 888C
	ROMX_LOAD( "910626.bin", 0x0000, 0x20000, CRC(12527742) SHA1(07c185b3bc58410183422f7ac13a37ddd330881b), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "910828", "V0.9.910828" ) // sum16 C9CD
	ROMX_LOAD( "910828.bin", 0x0000, 0x20000, CRC(3ee40b06) SHA1(b63d970727a2b8da72a0a8e234f3c30a20cbcb26), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "911001", "V0.9.911001" ) // sum16 4BCF
	ROMX_LOAD( "911001.bin", 0x0000, 0x20000, CRC(0888b50f) SHA1(129b9a2611edaebaa028ac3e3f444927c8b1fc5d), ROM_BIOS(4) )
ROM_END

ROM_START( c64dx )
	ROM_REGION( 0x20000, "maincpu", 0 ) // "v0.90.910429", sum16 E96A
	ROM_LOAD( "910429.bin", 0x0000, 0x20000, CRC(b025805c) SHA1(c3b05665684f74adbe33052a2d10170a1063ee7d) )
ROM_END

void c65_state::init_c65()
{
//  m_dma.version = 2;
//  c65_common_driver_init();
}

void c65_state::init_c65pal()
{
//  m_dma.version = 1;
//  c65_common_driver_init();
//  m_pal = 1;
}

COMP( 1991, c65,   0,   0, c65, c65, c65_state, init_c65,    "Commodore Business Machines", "Commodore 65 Development System (Prototype, NTSC)",          MACHINE_NOT_WORKING )
COMP( 1991, c64dx, c65, 0, c65, c65, c65_state, init_c65pal, "Commodore Business Machines", "Commodore 64DX Development System (Prototype, PAL, German)", MACHINE_NOT_WORKING )
