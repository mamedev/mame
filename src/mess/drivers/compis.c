/******************************************************************************

    drivers/compis.c
    machine driver

    Per Ola Ingvarsson
    Tomas Karlsson

    Hardware:
        - Intel 80186 CPU 8MHz, integrated DMA(8237?), PIC(8259?), PIT(8253?)
                - Intel 80130 OSP Operating system processor (PIC 8259, PIT 8254)
        - Intel 8274 MPSC Multi-protocol serial communications controller (NEC 7201)
        - Intel 8255 PPI Programmable peripheral interface
        - Intel 8253 PIT Programmable interval timer
        - Intel 8251 USART Universal synchronous asynchronous receiver transmitter
        - National 58174 Real-time clock (compatible with 58274)
    Peripheral:
        - Intel 82720 GDC Graphic display processor (NEC uPD 7220)
        - Intel 8272 FDC Floppy disk controller (Intel iSBX-218A)
        - Western Digital WD1002-05 Winchester controller

    Memory map:

    00000-3FFFF RAM LMCS (Low Memory Chip Select)
    40000-4FFFF RAM MMCS 0 (Midrange Memory Chip Select)
    50000-5FFFF RAM MMCS 1 (Midrange Memory Chip Select)
    60000-6FFFF RAM MMCS 2 (Midrange Memory Chip Select)
    70000-7FFFF RAM MMCS 3 (Midrange Memory Chip Select)
    80000-EFFFF NOP
    F0000-FFFFF ROM UMCS (Upper Memory Chip Select)

18/08/2011 -[Robbbert]
- Modernised
- Removed F4 display, as the gfx is in different places per bios.
- Changed to monochrome, it usually had a greenscreen monitor, although some
  were amber.
- Still doesn't work.
- Added a nasty hack to get a display on compis2 (wait 20 seconds)


 ******************************************************************************/

#include "includes/compis.h"
#include "formats/mfi_dsk.h"

#if 0
/* TODO: this is likely to come from a RAMdac ... */
static const unsigned char COMPIS_palette[16*3] =
{
	0, 0, 0,
	0, 0, 0,
	33, 200, 66,
	94, 220, 120,
	84, 85, 237,
	125, 118, 252,
	212, 82, 77,
	66, 235, 245,
	252, 85, 84,
	255, 121, 120,
	212, 193, 84,
	230, 206, 128,
	33, 176, 59,
	201, 91, 186,
	204, 204, 204,
	255, 255, 255
};

PALETTE_INIT_MEMBER(compis_state,compis_gdc)
{
	int i;

	for ( i = 0; i < 16; i++ ) {
		palette_set_color_rgb(machine(), i, COMPIS_palette[i*3], COMPIS_palette[i*3+1], COMPIS_palette[i*3+2]);
	}
}
#endif


void compis_state::palette_init()
{
	palette_set_color(machine(), 0, RGB_BLACK); // black
	palette_set_color(machine(), 1, MAKE_RGB(0x00, 0xc0, 0x00)); // green
	palette_set_color(machine(), 2, MAKE_RGB(0x00, 0xff, 0x00)); // highlight
}

UINT32 compis_state::screen_update_compis2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)// temporary
{
	UINT8 *m_p_chargen;
	m_p_chargen = memregion("maincpu")->base()+0xca70; //bios0
	if (m_p_chargen[0x214] != 0x08) m_p_chargen+= 0x10; //bios1
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 16; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 240; x+=3)
			{
				chr = m_video_ram[x & 0x1ffff];

				if (chr < 0x20)
					gfx = 0;
				else
					gfx = m_p_chargen[(chr<<4) | ra ];

				*p++ = BIT(gfx, 0);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 7);
			}
		}
		ma+=240;
	}

	return 0;
}

static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	compis_state *state = device->machine().driver_data<compis_state>();
	UINT8 i,gfx = state->m_video_ram[address & 0x1ffff];

	for(i=0; i<8; i++)
		bitmap.pix16(y, x + i) = BIT((gfx >> i), 0);
}


static UPD7220_INTERFACE( hgdc_intf )
{
	"screen",
	hgdc_display_pixels,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/* TODO: why it writes to ROM region? */
WRITE8_MEMBER( compis_state::vram_w )
{
	m_video_ram[offset] = data;
}

static ADDRESS_MAP_START( compis_mem , AS_PROGRAM, 16, compis_state )
	AM_RANGE( 0x00000, 0x3ffff) AM_RAM
	AM_RANGE( 0x40000, 0x4ffff) AM_RAM
	AM_RANGE( 0x50000, 0x5ffff) AM_RAM
	AM_RANGE( 0x60000, 0x6ffff) AM_RAM
	AM_RANGE( 0x70000, 0x7ffff) AM_RAM
	AM_RANGE( 0xe8000, 0xeffff) AM_ROM AM_REGION("maincpu",0) AM_WRITE8(vram_w, 0xffff)
	AM_RANGE( 0xf0000, 0xfffff) AM_ROM AM_REGION("maincpu",0) AM_WRITE8(vram_w, 0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( compis_io, AS_IO, 16, compis_state )
	AM_RANGE( 0x0000, 0x0007) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xff00)
	AM_RANGE( 0x0080, 0x0087) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE( 0x0100, 0x011b) AM_DEVREADWRITE8_LEGACY("mm58274c", mm58274c_r, mm58274c_w, 0xffff)
	AM_RANGE( 0x0280, 0x0283) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0xffff) /* 80150/80130 */
  //AM_RANGE( 0x0288, 0x028f) AM_DEVREADWRITE_LEGACY("pit8254", compis_osp_pit_r, compis_osp_pit_w ) /* PIT 8254 (80150/80130)  */
	AM_RANGE( 0x0310, 0x031f) AM_READWRITE( compis_usart_r, compis_usart_w )	/* USART 8251 Keyboard      */
	AM_RANGE( 0x0330, 0x0333) AM_DEVREADWRITE8("upd7220", upd7220_device, read, write, 0x00ff) /* GDC 82720 PCS6:6     */
	AM_RANGE( 0x0340, 0x0343) AM_DEVICE8("i8272a", i8272a_device, map, 0x00ff)	/* iSBX0 (J8) FDC 8272      */
	AM_RANGE( 0x0350, 0x0351) AM_DEVREADWRITE8("i8272a", i8272a_device, mdma_r, mdma_w, 0x00ff)	/* iSBX0 (J8) DMA ACK       */
	AM_RANGE( 0xff00, 0xffff) AM_READWRITE( compis_i186_internal_port_r, compis_i186_internal_port_w)/* CPU 80186         */
//{ 0x0100, 0x017e, compis_null_r },    /* RTC              */
//{ 0x0180, 0x01ff, compis_null_r },    /* PCS3?            */
//{ 0x0200, 0x027f, compis_null_r },    /* Reserved         */
//{ 0x0280, 0x02ff, compis_null_r },    /* 80150 not used?      */
//{ 0x0300, 0x0300, compis_null_r },    /* Cassette  motor      */
//{ 0x0301, 0x030f, compis_null_r},     /* DMA ACK Graphics     */
//{ 0x0310, 0x031e, compis_null_r },    /* SCC 8274 Int Ack     */
//{ 0x0320, 0x0320, compis_null_r },    /* SCC 8274 Serial port     */
//{ 0x0321, 0x032f, compis_null_r },    /* DMA Terminate        */
//{ 0x0331, 0x033f, compis_null_r },    /* DMA Terminate        */
//{ 0x0341, 0x034f, compis_null_r },    /* J8 CS1 (16-bit)      */
//{ 0x0350, 0x035e, compis_null_r },    /* J8 CS1 (8-bit)       */
//{ 0x0360, 0x036e, compis_null_r },    /* J9 CS0 (8/16-bit)        */
//{ 0x0361, 0x036f, compis_null_r },    /* J9 CS1 (16-bit)      */
//{ 0x0370, 0x037e, compis_null_r },    /* J9 CS1 (8-bit)       */
//{ 0x0371, 0x037f, compis_null_r },    /* J9 CS1 (8-bit)       */
//{ 0xff20, 0xffff, compis_null_r },    /* CPU 80186            */
ADDRESS_MAP_END

/* COMPIS Keyboard */

/* 2008-05 FP:
Small note about natural keyboard: currently,
- Both "SShift" keys (left and right) are not mapped
- Keypad '00' and '000' are not mapped
- "Compis !" is mapped to 'F3'
- "Compis ?" is mapped to 'F4'
- "Compis |" is mapped to 'F5'
- "Compis S" is mapped to 'F6'
- "Avbryt" is mapped to 'F7'
- "Inpassa" is mapped to 'Insert'
- "S?k" is mapped to "Print Screen"
- "Utpl?na"is mapped to 'Delete'
- "Start / Stop" is mapped to 'Pause'
- "TabL" is mapped to 'Page Up'
- "TabR" is mapped to 'Page Down'
*/

static INPUT_PORTS_START (compis)
	PORT_START("ROW0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)		PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)			PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)			PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)			PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)			PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)			PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)			PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)			PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)			PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)			PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)			PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)		PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC2\xB4 `") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('`')
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)	PORT_CHAR(8)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)		PORT_CHAR('\t')
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)			PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)			PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)			PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)			PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)			PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)			PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)			PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)			PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)			PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)			PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(a_RING " " A_RING) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u_UMLAUT " " U_UMLAUT) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00FC) PORT_CHAR(0x00DC)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)		PORT_CHAR(13)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)			PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)			PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)			PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)			PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)			PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)			PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)			PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)			PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)			PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(o_UMLAUT " " O_UMLAUT) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(a_UMLAUT " " A_UMLAUT) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'' *") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('*')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)	PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)			PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)			PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)			PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)			PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)			PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)			PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)			PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)		PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)		PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)		PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SShift (Left)") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)	PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)		PORT_CHAR(' ')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)	PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SShift (Right)") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INPASSA") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S" O_UMLAUT "K") PORT_CODE(KEYCODE_PRTSCR) PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UTPL" A_RING "NA") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("START-STOP") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AVBRYT") PORT_CODE(KEYCODE_SCRLOCK) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TABL") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TABR") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMPIS !") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMPIS ?") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMPIS |") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)		PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)		PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMPIS S") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)		PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)		PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)		PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_START("ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)		PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)		PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)		PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)		PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)		PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)		PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)		PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 00") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 000") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Enter") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x18, 0x00, "S8 Test mode")
	PORT_DIPSETTING( 0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x08, "Remote" )
	PORT_DIPSETTING( 0x10, "Stand alone" )
	PORT_DIPSETTING( 0x18, "Reserved" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "iSBX-218A DMA")
	PORT_DIPSETTING( 0x01, "Enabled" )
	PORT_DIPSETTING( 0x00, "Disabled" )
INPUT_PORTS_END


static const unsigned i86_address_mask = 0x000fffff;

static const mm58274c_interface compis_mm58274c_interface =
{
	0,	/*  mode 24*/
	1   /*  first day of week */
};


static const floppy_format_type compis_floppy_formats[] = {
	FLOPPY_MFI_FORMAT,
	NULL
};

static SLOT_INTERFACE_START( compis_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

static ADDRESS_MAP_START( upd7220_map, AS_0, 8, compis_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x1ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END

static MACHINE_CONFIG_START( compis, compis_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80186, 8000000)	/* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(compis_mem)
	MCFG_CPU_IO_MAP(compis_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", compis_state,  compis_vblank_int)
	MCFG_CPU_CONFIG(i86_address_mask)

	//MCFG_QUANTUM_TIME(attotime::from_hz(60))


	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)
#if 0
	MCFG_PALETTE_LENGTH(16)
	MCFG_PALETTE_INIT_OVERRIDE(compis_state,compis_gdc)
#endif
	MCFG_PALETTE_LENGTH(3)

	/* Devices */
	MCFG_PIT8253_ADD( "pit8253", compis_pit8253_config )
	MCFG_PIT8254_ADD( "pit8254", compis_pit8254_config )
	MCFG_PIC8259_ADD( "pic8259_master", compis_pic8259_master_config )
	MCFG_PIC8259_ADD( "pic8259_slave", compis_pic8259_slave_config )
	MCFG_I8255_ADD( "ppi8255", compis_ppi_interface )
	MCFG_UPD7220_ADD("upd7220", XTAL_4MHz, hgdc_intf, upd7220_map) //unknown clock
	MCFG_CENTRONICS_PRINTER_ADD("centronics", standard_centronics)
	MCFG_I8251_ADD("uart", compis_usart_interface)
	MCFG_MM58274C_ADD("mm58274c", compis_mm58274c_interface)
	MCFG_I8272A_ADD("i8272a", true)
	MCFG_FLOPPY_DRIVE_ADD("i8272a:0", compis_floppies, "525qd", 0, compis_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("i8272a:1", compis_floppies, "525qd", 0, compis_floppy_formats)
	MCFG_COMPIS_KEYBOARD_ADD()

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_list", "compis")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( compis2, compis_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80186, 8000000)	/* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(compis_mem)
	MCFG_CPU_IO_MAP(compis_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", compis_state,  compis_vblank_int)
	MCFG_CPU_CONFIG(i86_address_mask)

	//MCFG_QUANTUM_TIME(attotime::from_hz(60))


	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DRIVER(compis_state, screen_update_compis2)
	MCFG_PALETTE_LENGTH(3)

	/* Devices */
	MCFG_PIT8253_ADD( "pit8253", compis_pit8253_config )
	MCFG_PIT8254_ADD( "pit8254", compis_pit8254_config )
	MCFG_PIC8259_ADD( "pic8259_master", compis_pic8259_master_config )
	MCFG_PIC8259_ADD( "pic8259_slave", compis_pic8259_slave_config )
	MCFG_I8255_ADD( "ppi8255", compis_ppi_interface )
	MCFG_UPD7220_ADD("upd7220", XTAL_4MHz, hgdc_intf, upd7220_map) //unknown clock
	MCFG_CENTRONICS_PRINTER_ADD("centronics", standard_centronics)
	MCFG_I8251_ADD("uart", compis_usart_interface)
	MCFG_MM58274C_ADD("mm58274c", compis_mm58274c_interface)
	MCFG_I8272A_ADD("i8272a", true)
	MCFG_FLOPPY_DRIVE_ADD("i8272a:0", compis_floppies, "525qd", 0, compis_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("i8272a:1", compis_floppies, "525qd", 0, compis_floppy_formats)
	MCFG_COMPIS_KEYBOARD_ADD()

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_list", "compis")
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( compis )
	ROM_REGION16_LE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa883003.u40", 0x0000, 0x4000, CRC(195ef6bf) SHA1(eaf8ae897e1a4b62d3038ff23777ce8741b766ef) )
	ROM_LOAD16_BYTE( "sa883003.u36", 0x0001, 0x4000, CRC(7c918f56) SHA1(8ba33d206351c52f44f1aa76cc4d7f292dcef761) )
	ROM_LOAD16_BYTE( "sa883003.u39", 0x8000, 0x4000, CRC(3cca66db) SHA1(cac36c9caa2f5bb42d7a6d5b84f419318628935f) )
	ROM_LOAD16_BYTE( "sa883003.u35", 0x8001, 0x4000, CRC(43c38e76) SHA1(f32e43604107def2c2259898926d090f2ed62104) )
ROM_END

ROM_START( compis2 )
	ROM_REGION16_LE( 0x10000, "maincpu", 0 )
	ROM_DEFAULT_BIOS( "v303" )

	ROM_SYSTEM_BIOS( 0, "v302", "Compis II v3.02 (1986-09-09)" )
	ROMX_LOAD( "comp302.u39", 0x0000, 0x8000, CRC(16a7651e) SHA1(4cbd4ba6c6c915c04dfc913ec49f87c1dd7344e3), ROM_BIOS(1) | ROM_SKIP(1) )
	ROMX_LOAD( "comp302.u35", 0x0001, 0x8000, CRC(ae546bef) SHA1(572e45030de552bb1949a7facbc885b8bf033fc6), ROM_BIOS(1) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 1, "v303", "Compis II v3.03 (1987-03-09)" )
	ROMX_LOAD( "rysa094.u39", 0x0000, 0x8000, CRC(e7302bff) SHA1(44ea20ef4008849af036c1a945bc4f27431048fb), ROM_BIOS(2) | ROM_SKIP(1) )
	ROMX_LOAD( "rysa094.u35", 0x0001, 0x8000, CRC(b0694026) SHA1(eb6b2e3cb0f42fd5ffdf44f70e652ecb9714ce30), ROM_BIOS(2) | ROM_SKIP(1) )
ROM_END

/*   YEAR   NAME        PARENT  COMPAT MACHINE  INPUT   INIT     COMPANY     FULLNAME */
COMP(1985,  compis,     0,      0,     compis,  compis, compis_state, compis, "Telenova", "Compis" , GAME_NOT_WORKING | GAME_NO_SOUND)
COMP(1986,  compis2,    compis, 0,     compis2, compis, compis_state, compis, "Telenova", "Compis II" , GAME_NOT_WORKING | GAME_NO_SOUND)
