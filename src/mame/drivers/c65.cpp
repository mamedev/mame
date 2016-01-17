// license:LGPL-2.1+
// copyright-holders: Angelo Salese
/***************************************************************************

C=65 / C=64DX (c) 1991 Commodore

Attempt at rewriting the driver ...

TODO:
- I need to subtract border color to -1 in order to get blue color (-> register is 6 and blue color is 5 in palette array).
  Also top-left logo seems to draw wrong palette for entries 4,5,6,7. CPU core bug?

Note:
- VIC-4567 will be eventually be added via compile switch, once that I
  get the hang of the system (and checking where the old code fails
  eventually)

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m4510.h"
#include "machine/mos6526.h"
#include "softlist.h"

#define MAIN_CLOCK XTAL_3_5MHz

class c65_state : public driver_device
{
public:
	c65_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_cia0(*this, "cia_0"),
			m_cia1(*this, "cia_1"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_workram(*this, "wram"),
			m_palred(*this, "redpal"),
			m_palgreen(*this, "greenpal"),
			m_palblue(*this, "bluepal"),
			m_dmalist(*this, "dmalist"),
			m_cram(*this, "cram"),
			m_gfxdecode(*this, "gfxdecode")
	{ }

	// devices
	required_device<m4510_device> m_maincpu;
	required_device<mos6526_device> m_cia0;
	required_device<mos6526_device> m_cia1;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_workram;
	required_shared_ptr<UINT8> m_palred;
	required_shared_ptr<UINT8> m_palgreen;
	required_shared_ptr<UINT8> m_palblue;
	required_shared_ptr<UINT8> m_dmalist;
	required_shared_ptr<UINT8> m_cram;
	required_device<gfxdecode_device> m_gfxdecode;

	UINT8 *m_iplrom;
	UINT8 m_keyb_input[10];
	UINT8 m_keyb_mux;

	DECLARE_READ8_MEMBER(vic4567_dummy_r);
	DECLARE_WRITE8_MEMBER(vic4567_dummy_w);
	DECLARE_WRITE8_MEMBER(PalRed_w);
	DECLARE_WRITE8_MEMBER(PalGreen_w);
	DECLARE_WRITE8_MEMBER(PalBlue_w);
	DECLARE_WRITE8_MEMBER(DMAgic_w);
	DECLARE_READ8_MEMBER(CIASelect_r);
	DECLARE_WRITE8_MEMBER(CIASelect_w);
	DECLARE_READ8_MEMBER(cia0_porta_r);
	DECLARE_WRITE8_MEMBER(cia0_porta_w);
	DECLARE_READ8_MEMBER(cia0_portb_r);
	DECLARE_WRITE8_MEMBER(cia0_portb_w);
	DECLARE_WRITE_LINE_MEMBER(cia0_irq);

	DECLARE_READ8_MEMBER(dummy_r);

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(c65);
	DECLARE_DRIVER_INIT(c65);
	DECLARE_DRIVER_INIT(c65pal);

	INTERRUPT_GEN_MEMBER(vic3_vblank_irq);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
private:
	UINT8 m_VIC2_IRQPend, m_VIC2_IRQMask;
	/* 0x20: border color (TODO: different thread?) */
	UINT8 m_VIC2_EXTColor;
	/* 0x30: banking + PAL + EXT SYNC */
	UINT8 m_VIC3_ControlA;
	/* 0x31: video modes */
	UINT8 m_VIC3_ControlB;
	void PalEntryFlush(UINT8 offset);
	void DMAgicExecute(address_space &space,UINT32 address);
	void IRQCheck(UINT8 irq_cause);
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

UINT32 c65_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int y,x;
	int border_color = m_VIC2_EXTColor & 0xf;

	// TODO: border area
	for(y=0;y<m_screen->height();y++)
	{
		for(x=0;x<m_screen->width();x++)
		{
			//int, xi,yi,xm,ym,dot_x;
			int xi = inner_x_char(x);
			int yi = inner_y_char(y);
			int xm = 7 - (x & 7);
			int ym = (y & 7);
			UINT8 tile = m_workram[xi+yi*80+0x800];
			UINT8 attr = m_cram[xi+yi*80];
			if(attr & 0xf0)
				attr = machine().rand() & 0xf;

			int enable_dot = ((m_iplrom[(tile<<3)+ym+0xd000] >> xm) & 1);

			//if(cliprect.contains(x, y))
			bitmap.pix16(y, x) = m_palette->pen((enable_dot) ? attr & 0xf : border_color);


			//gfx->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);
		}
	}

	return 0;
}

READ8_MEMBER(c65_state::vic4567_dummy_r)
{
	UINT8 res;

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
		case 0x19:
			return m_VIC2_IRQPend;

		case 0x1a:
			return m_VIC2_IRQMask;

		case 0x20:
			return m_VIC2_EXTColor;

		case 0x30:
			return m_VIC3_ControlA;
		case 0x31:
			return m_VIC3_ControlB;
	}

	if(!space.debugger_access())
		printf("%02x\n",offset); // TODO: PC
	return res;
}

WRITE8_MEMBER(c65_state::vic4567_dummy_w)
{
	switch(offset)
	{
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
			if(!space.debugger_access())
				printf("%02x %02x\n",offset,data);
			break;
	}

}

void c65_state::PalEntryFlush(UINT8 offset)
{
	m_palette->set_pen_color(offset, pal4bit(m_palred[offset]), pal4bit(m_palgreen[offset]), pal4bit(m_palblue[offset]));
}

WRITE8_MEMBER(c65_state::PalRed_w)
{
	m_palred[offset] = data;
	PalEntryFlush(offset);
}

WRITE8_MEMBER(c65_state::PalGreen_w)
{
	m_palgreen[offset] = data;
	PalEntryFlush(offset);
}

WRITE8_MEMBER(c65_state::PalBlue_w)
{
	m_palblue[offset] = data;
	PalEntryFlush(offset);
}

void c65_state::DMAgicExecute(address_space &space,UINT32 address)
{
	UINT8 cmd;// = space.read_byte(address++);
	UINT16 length; //= space.read_byte(address++);
	UINT32 src, dst;
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
				UINT32 SourceIndex;
				UINT32 DestIndex;
				UINT16 SizeIndex;
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
				UINT8 FillValue;
				UINT32 DestIndex;
				UINT16 SizeIndex;
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


WRITE8_MEMBER(c65_state::DMAgic_w)
{
	m_dmalist[offset] = data;
	if(offset == 0)
		DMAgicExecute(space,(m_dmalist[0])|(m_dmalist[1]<<8)|(m_dmalist[2]<<16));
}

READ8_MEMBER(c65_state::CIASelect_r)
{
	if(m_VIC3_ControlA & 1)
		return m_cram[offset];
	else
	{
		// CIA at 0xdc00
		switch((offset & 0x700) | 0x800)
		{
			case 0xc00:
				return m_cia0->read(space,offset);
			case 0xd00:
				return m_cia1->read(space,offset);
			default:
				printf("Unknown I/O access read to offset %04x\n",offset);
				break;
		}

	}

	return 0xff;
}

WRITE8_MEMBER(c65_state::CIASelect_w)
{
	if(m_VIC3_ControlA & 1)
		m_cram[offset] = data;
	else
	{
		// CIA at 0xdc00
		switch((offset & 0x700) | 0x800)
		{
			case 0xc00:
				m_cia0->write(space,offset,data);
				break;

			case 0xd00:
				m_cia1->write(space,offset,data);
				break;
			default:
				printf("Unknown I/O access write to offset %04x data = %02x\n",offset,data);
				break;
		}
	}

}

READ8_MEMBER(c65_state::cia0_porta_r)
{
	return 0xff;
}

READ8_MEMBER(c65_state::cia0_portb_r)
{
	static const char *const c64ports[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7" };
	UINT8 res;

	res = 0xff;
	for(int i=0;i<8;i++)
	{
		m_keyb_input[i] = machine().root_device().ioport(c64ports[i])->read();

		if(m_keyb_mux & 1 << (i))
			res &= m_keyb_input[i];
	}

	return res;
}

WRITE8_MEMBER(c65_state::cia0_porta_w)
{
	m_keyb_mux = ~data;
	printf("%02x\n",m_keyb_mux);
}

WRITE8_MEMBER(c65_state::cia0_portb_w)
{
}

READ8_MEMBER(c65_state::dummy_r)
{
	return 0;
}

static ADDRESS_MAP_START( c65_map, AS_PROGRAM, 8, c65_state )
	AM_RANGE(0x00000, 0x07fff) AM_RAM AM_SHARE("wram") // TODO: bank
	AM_RANGE(0x0c800, 0x0cfff) AM_ROM AM_REGION("maincpu", 0xc800)
	AM_RANGE(0x0d000, 0x0d07f) AM_READWRITE(vic4567_dummy_r,vic4567_dummy_w) // 0x0d000, 0x0d07f VIC-4567
	AM_RANGE(0x0d080, 0x0d081) AM_READ(dummy_r) // 0x0d080, 0x0d09f FDC
	// 0x0d0a0, 0x0d0ff Ram Expansion Control (REC)
	AM_RANGE(0x0d100, 0x0d1ff) AM_RAM_WRITE(PalRed_w) AM_SHARE("redpal")// 0x0d100, 0x0d1ff Red Palette
	AM_RANGE(0x0d200, 0x0d2ff) AM_RAM_WRITE(PalGreen_w) AM_SHARE("greenpal") // 0x0d200, 0x0d2ff Green Palette
	AM_RANGE(0x0d300, 0x0d3ff) AM_RAM_WRITE(PalBlue_w) AM_SHARE("bluepal") // 0x0d300, 0x0d3ff Blue Palette
	// 0x0d400, 0x0d4*f Right SID
	// 0x0d440, 0x0d4*f Left  SID
	AM_RANGE(0x0d600, 0x0d6ff) AM_RAM // 0x0d600, 0x0d6** UART
	AM_RANGE(0x0d700, 0x0d702) AM_WRITE(DMAgic_w) AM_SHARE("dmalist") // 0x0d700, 0x0d7** DMAgic
	//AM_RANGE(0x0d703, 0x0d703) AM_READ(DMAgic_r)
	// 0x0d800, 0x0d8** Color matrix
	AM_RANGE(0x0d800, 0x0dfff) AM_READWRITE(CIASelect_r,CIASelect_w) AM_SHARE("cram")
	// 0x0dc00, 0x0dc** CIA-1
	// 0x0dd00, 0x0dd** CIA-2
	// 0x0de00, 0x0de** Ext I/O Select 1
	// 0x0df00, 0x0df** Ext I/O Select 2 (RAM window?)
	AM_RANGE(0x0e000, 0x0ffff) AM_ROM AM_REGION("maincpu",0x0e000)
	AM_RANGE(0x10000, 0x1f7ff) AM_RAM
	AM_RANGE(0x1f800, 0x1ffff) AM_RAM // VRAM attributes
	AM_RANGE(0x20000, 0x3ffff) AM_ROM AM_REGION("maincpu",0)
ADDRESS_MAP_END



static INPUT_PORTS_START( c65 )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                                    PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                    PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                                    PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)                                    PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)             PORT_CHAR(13)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INST DEL") PORT_CODE(KEYCODE_BACKSPACE)       PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('I')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('P')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('+')

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR HOME") PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR('\xA3')

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN STOP") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)                               PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE)   PORT_CHAR(0x2190)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1') PORT_CHAR('!')

INPUT_PORTS_END


void c65_state::machine_start()
{
	m_iplrom = memregion("maincpu")->base();

	save_pointer(NAME(m_cram.target()), 0x800);
}

void c65_state::machine_reset()
{
}


PALETTE_INIT_MEMBER(c65_state, c65)
{
	// TODO: initial state?
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

static GFXDECODE_START( c65 )
	GFXDECODE_ENTRY( "maincpu", 0xd000, charlayout,     0, 16 ) // another identical copy is at 0x9000
GFXDECODE_END

void c65_state::IRQCheck(UINT8 irq_cause)
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
		static const char *const c64ports[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7" };
		for(int i=0;i<8;i++)
			m_keyb_input[i] = machine().root_device().ioport(c64ports[i])->read();
	}
#endif
//  m_cia0_irq = state;
//  c65_irq(state || m_vicirq);
}

static MACHINE_CONFIG_START( c65, c65_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M4510,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(c65_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen",c65_state,vic3_vblank_irq)

	MCFG_DEVICE_ADD("cia_0", MOS6526, MAIN_CLOCK)
	MCFG_MOS6526_TOD(60)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c65_state, cia0_irq))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c65_state, cia0_porta_r))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c65_state, cia0_porta_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c65_state, cia0_portb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c65_state, cia0_portb_w))

	MCFG_DEVICE_ADD("cia_1", MOS6526, MAIN_CLOCK)
	MCFG_MOS6526_TOD(60)
//  MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c65_state, c65_cia1_interrupt))
//  MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(c65_state, c65_cia1_port_a_r))
//  MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(c65_state, c65_cia1_port_a_w))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(c65_state, screen_update)
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK*4, 910, 0, 640, 262, 0, 200) // mods needed
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", c65)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(c65_state, c65)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "c65_flop")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( c65 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "910111", "V0.9.910111" )
	ROMX_LOAD( "910111.bin", 0x0000, 0x20000, CRC(c5d8d32e) SHA1(71c05f098eff29d306b0170e2c1cdeadb1a5f206), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "910523", "V0.9.910523" )
	ROMX_LOAD( "910523.bin", 0x0000, 0x20000, CRC(e8235dd4) SHA1(e453a8e7e5b95de65a70952e9d48012191e1b3e7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "910626", "V0.9.910626" )
	ROMX_LOAD( "910626.bin", 0x0000, 0x20000, CRC(12527742) SHA1(07c185b3bc58410183422f7ac13a37ddd330881b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "910828", "V0.9.910828" )
	ROMX_LOAD( "910828.bin", 0x0000, 0x20000, CRC(3ee40b06) SHA1(b63d970727a2b8da72a0a8e234f3c30a20cbcb26), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "911001", "V0.9.911001" )
	ROMX_LOAD( "911001.bin", 0x0000, 0x20000, CRC(0888b50f) SHA1(129b9a2611edaebaa028ac3e3f444927c8b1fc5d), ROM_BIOS(5) )
ROM_END

ROM_START( c64dx )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "910429.bin", 0x0000, 0x20000, CRC(b025805c) SHA1(c3b05665684f74adbe33052a2d10170a1063ee7d) )
ROM_END

DRIVER_INIT_MEMBER(c65_state,c65)
{
//  m_dma.version = 2;
//  c65_common_driver_init();
}

DRIVER_INIT_MEMBER(c65_state,c65pal)
{
//  m_dma.version = 1;
//  c65_common_driver_init();
//  m_pal = 1;
}

COMP( 1991, c65,    0,      0,      c65,    c65, c65_state, c65,    "Commodore Business Machines",  "Commodore 65 Development System (Prototype, NTSC)", MACHINE_NOT_WORKING )
COMP( 1991, c64dx,  c65,    0,      c65,    c65, c65_state, c65pal, "Commodore Business Machines",  "Commodore 64DX Development System (Prototype, PAL, German)", MACHINE_NOT_WORKING )
