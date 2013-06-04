/*

Enterprise Sixty Four / Enterprise One Two Eight
Enterprise Computers Ltd. 1985

MAIN PCB Layout
---------------  
                                        DUAL
|-----------| 9V                        TAPE_IN/OUT                             RES
|HEATSINK   | DC_IN             MON1    AND REMOTE     SR1    PR1   CN2A CN1A   |-|                      
|           |--||---|--||--|--| |--| |-||-||-||-||-| |----| |-----| |--| |--| |-|-|------|
|-----------|7805   | MOD  |  |-|  |-|             |-|    |-|     |-|  |-|  |-|   EXP2 --|EXTERNAL            
        |---+7805   |      |                         74LS06 74LS273 74LS86 74LS32      --|EXPANSION
        |   |       |------|             74LS244                                       --|CONNECTOR
        |   |78L12 POT1  LM386                               |-----|   |-----|   EXP1  --|
   CART |   |          LM1889 LM1886                   74LS04|NICK |   |DAVE |         --|
   CONN |   |   KEYB_8     POT2     KEYB_10                  |     |   |     |         --|
        |   |            4.433619MHz     74LS145 POT3 LED    |-----|   |-----|         --|
        |---+                                                                          --|
            |                            74LS74      74LS244  74LS244  74LS245 |---------|
            |                                                                  |            
            | 74LS373                    4164  4164  74F157                    |
            |                                                                  |
            | 16-2-103   LM339    LM324  4164  4164  74F157           EXOS.IC2 |
            |                                                 8MHz             |
            | 74LS373                    4164  4164                            |
            |                                                74LS04    Z80A    |
            | 74LS273                    4164  4164                            |
            |------------------------------------------------------------------|
Notes: (all IC's shown)
           Z80A - Z80A CPU, clock input 4MHz [8/2]
       EXOS.IC2 - 32k x8-bit mask ROM usually 23256 manufactured by GI (DIP24). Contains EXOS operating 
                  system and built-in word processor software. A few official revisions were made and 
                  there were a few unofficial revision made with additional capabilities and bug-fixes
                  ROM labelling of some official versions found....
                  9256DS-0019 (C)1984 INTELLIGENT SOFTWARE LTD ENTER 08-45-A GI
                  9256DS-0036 (C)1984 INTELLIGENT SOFTWARE LTD ENTER 05-23-A GI
           4164 - 64k x1-bit Dynamic RAM with Page Mode (DIP16)
           NICK - Custom graphics chip (QFP72)
           DAVE - Custom sound chip (QFP72)
          LM386 - National Semiconductor LM386 Low Voltage Audio Power Amplifier (DIP8)
         LM1889 - National Semiconductor LM1889 TV Video Modulator (DIP18)
         LM1886 - National Semiconductor LM1886 TV Video Matrix DAC (DIP20)
          LM339 - SGS LM339 Low Power Low Offset Voltage Quad Comparator (DIP8)
          LM324 - SGS LM324 Quad Operational Amplifier (DIP8)
         74LS04 - Hex Inverter (DIP14)
         74LS06 - Hex Inverter/Buffer with Open-Collector Outputs (DIP14)
         74LS32 - Quad 2-Input Positive OR Gate (DIP14)
         74LS74 - Dual Positive-Edge-Triggered D Flip-Flops with Preset, Clear and Complementary Outputs (DIP14)
         74LS86 - Quad 2-Input Exclusive OR Gate (DIP14)
        74LS145 - BCD to Deccimal Decoder/Driver (DIP16)
         74F157 - Quad 2-Line to 1-Line Data Selector/Multiplexer (DIP16). Early rev mainboards use 74LS158 instead
        74LS244 - Octal 3-State Noninverting Buffer/Line Driver/Line Receiver (DIP20)
        74LS245 - Octal Bus Tranceiver with Tri-State Outputs (DIP20)
        74LS273 - Octal D-Type Flip-Flop With Clear (DIP20)
        74LS373 - Octal D-Type Transparent Latches and Edge-Triggered Flip-Flops (DIP20)
           7805 - Voltage regulator. +9V DC input from DC power pack, +5V DC output
          78L12 - Voltage regulator. Voltage input via small transformer on PCB. +12V DC output
           POT1 - Potentiometer located near modulator and video output circuitry
           POT2 - Potentiometer located near video output circuitry. Probably used to fine-tune the video clock
           POT3 - Potentiometer. Possibly for video/NICK-related adjustments
            LED - LED to show +5V is present
       16-2-103 - Resistor Array (DIP16)
         KEYB_8 - 8 pin keyboard connector
        KEYB_10 - 10 pin keyboard connector
           EXP1 - 28 pin internal expansion connector (solder holes only) used for internal 64k memory expansion card
           EXP2 - 10 pin internal expansion connector (solder holes only) used for internal 64k memory expansion card
            MOD - Astec UM1233 TV modulator
            RES - Reset push button switch
           MON1 - Monitor output connector. Pinout is....
           
                            Green  A1 B1 NC
                            Ground A2 B2 Ground
                  Mono Comp. Video A3 B3 Blue
                             HSync A4 B4 Red
                             VSync A5 B5 Composite Sync
                                NC A6 B6 Mode Switch
                        Left Audio A7 B7 Right Audio
           
            SR1 - Serial/Network connector. Pinout is....
            
                         Reference A1 B1 Ground
                                 - A2 B2 -
                               RTS A3 B3 Data Out
                               CTS A4 B4 Data In

            PR1 - Printer connector. Pinout is....
                  
                            Ground A1 B1 Ground
                            Strobe A2 B2 Ready
                            Data 3 A3 B3 Data 4
                                 - A4 B4 -
                            Data 2 A5 B5 Data 5
                            Data 1 A6 B6 Data 6
                            Data 0 A7 B7 Data 7

           CN2A - Joystick 2 connector
           CN1A - Joystick 1 connector 
                  Pinout is....
                  
                          Keyboard J A1 B1 Ground
                          Keyboard L A2 B2 Keyboard K
                                   - A3 B3 -
                               Right A4 B4 +5V
                                Down A5 B5 Left
                                Fire A6 B6 Up


Floppy Drive Controller PCB Layout
----------------------------------

INTELLIGENT SOFTWARE LTD DISK CONTROLLER 
ISS1
 |--------------------------------------------|
 |                                            |
 |                                            |
|-|  7438  74LS273             WD1770         |
|I|                                           |
|D|                                           |
|C|                                  EPROM.IC2|
|3|           74LS32  74LS02  74LS266         |
|4|  7438                                     |
|-|      74LS126  74LS10  74LS245  74LS266    |
 |                                            |
 |                                            |
 |----------------------------|||||||||||||||||
                              |---------------|
Notes: (All IC's shown)

This PCB plugs into the external expansion connector on the right side of the mainboard

      EPROM.IC2 - 16k x8-bit EPROM labelled 'EXDOS V1.0 P/N 08-60' (DIP28)
         WD1770 - Western Digital WD1770 Floppy Drive Controller (DIP28)
         74LS02 - Quad 2-Input NOR Gate (DIP14)
         74LS10 - Triple 3-input NAND Gate (DIP14)
         74LS32 - Quad 2-Input Positive OR Gate (DIP14)
           7438 - Quad 2-input NAND Buffer (DIP14)
        74LS126 - Quad Bus Buffer (DIP14)
        74LS245 - Octal Bus Tranceiver with Tri-State Outputs (DIP20)
        74LS266 - Quad EXCLUSIVE-NOR Gate (DIP14)
        74LS273 - Octal D-Type Flip-Flop With Clear (DIP20)
          IDC34 - IDC 34-way flat cable connector for floppy drive data cable


Internal Memory Expansion PCB Layout
------------------------------------     

|---------------------|
|  4164 74LS00 PL3 PL2|
|4164 74LS30 74F157   |
|4164 4164            |
|4164 4164   74F157   |
|4164 4164 74LS32  PL1|
|---------------------|
Notes: (All IC's shown)
          PL1 - 28-pin connector (solder pads only) hard-wired to solder pads EXP1 on mainboard
      PL2/PL3 - 5-pin connectors (solder pads only) hard-wired to solder pads EXP2 on mainboard
         4164 - 64k x1-bit Dynamic RAM with Page Mode (DIP16)
       74LS00 - Quad 2-Input NAND Gate (DIP14)
       74LS30 - 8-input NAND Gate (DIP14)
       74LS32 - Quad 2-Input Positive OR Gate (DIP14)
       74F157 - Quad 2-Line to 1-Line Data Selector/Multiplexer (DIP16). Early rev memory boards use 74LS158 instead

*/

/******************************************************************************
 * Enterprise 128k driver
 *
 * Kevin Thacker 1999.
 *
 * James Boulton [EP help]
 * Jean-Pierre Malisse [EP help]
 *
 * EP Hardware: Z80 (CPU), Dave (Sound Chip + I/O)
 * Nick (Graphics), WD1772 (FDC). 128k ram.
 *
 * For an 8-bit machine, this kicks ass! A sound
 * Chip which is as powerful, or more powerful than
 * the C64 SID, and a graphics chip capable of some
 * really nice graphics. Pity it doesn't have HW sprites!
 ******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/wd17xx.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "includes/enterp.h"

#define ENTERPRISE_XTAL_X1  XTAL_8MHz


/***************************************************************************
    MEMORY / I/O
***************************************************************************/

void ep_state::enterprise_update_memory_page(address_space &space, offs_t page, int index)
{
	int start = (page - 1) * 0x4000;
	int end = (page - 1) * 0x4000 + 0x3fff;
	char page_num[10];
	sprintf(page_num,"bank%d",page);

	switch (index)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		space.install_read_bank(start, end, page_num);
		space.nop_write(start, end);
		membank(page_num)->set_base(memregion("exos")->base() + (index * 0x4000));
		break;

	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		space.install_read_bank(start, end, page_num);
		space.nop_write(start, end);
		membank(page_num)->set_base(memregion("cartridges")->base() + ((index - 0x04) * 0x4000));
		break;

	case 0x20:
	case 0x21:
		space.install_read_bank(start, end, page_num);
		space.nop_write(start, end);
		membank(page_num)->set_base(memregion("exdos")->base() + ((index - 0x20) * 0x4000));
		break;

	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfb:
		/* additional 64k ram */
		if (m_ram->size() == 128*1024)
		{
			space.install_readwrite_bank(start, end, page_num);
			membank(page_num)->set_base(m_ram->pointer() + (index - 0xf4) * 0x4000);
		}
		else
		{
			space.unmap_readwrite(start, end);
		}
		break;

	case 0xfc:
	case 0xfd:
	case 0xfe:
	case 0xff:
		/* basic 64k ram */
		space.install_readwrite_bank(start, end, page_num);
		membank(page_num)->set_base(m_ram->pointer() + (index - 0xfc) * 0x4000);
		break;

	default:
		space.unmap_readwrite(start, end);
	}
}


/* EP specific handling of dave register write */
WRITE8_MEMBER(ep_state::enterprise_dave_reg_write)
{
	switch (offset)
	{
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		enterprise_update_memory_page(m_maincpu->space(AS_PROGRAM), offset - 0x0f, data);
		break;

	case 0x15:
		keyboard_line = data & 15;
		break;
	}
}


READ8_MEMBER(ep_state::enterprise_dave_reg_read)
{
	switch (offset)
	{
		case 0x015:
			/* read keyboard line */
			m_dave->set_reg(0x015, m_key[keyboard_line]->read());
			break;

		case 0x016:
		{
			int ExternalJoystickInputs;
			int ExternalJoystickPortInput = m_joy->read();

			if (keyboard_line <= 4)
			{
				ExternalJoystickInputs = ExternalJoystickPortInput>>(4-(keyboard_line));
			}
			else
			{
				ExternalJoystickInputs = 1;
			}

			m_dave->set_reg(0x016, (0x0fe | (ExternalJoystickInputs & 0x01)));
		}
		break;

		default:
			break;
	}
	return 0;
}

/* enterprise interface to dave - ok, so Dave chip is unique
to Enterprise. But these functions make it nice and easy to see
whats going on. */
static const dave_interface enterprise_dave_interface =
{
	DEVCB_DRIVER_MEMBER(ep_state,enterprise_dave_reg_read),
	DEVCB_DRIVER_MEMBER(ep_state,enterprise_dave_reg_write),
	DEVCB_CPU_INPUT_LINE("maincpu", 0)
};


void ep_state::machine_start()
{
	m_maincpu->set_input_line_vector(0, 0xff);
	m_nick->set_vram(m_ram->pointer());
	
	for (int i = 0; i < 10; i++)
	{
		char str[6];
		sprintf(str, "LINE%i", i);
		m_key[i] = ioport(str);
	}
}

void ep_state::machine_reset()
{
	m_maincpu->set_input_line_vector(0, 0xff);
}


UINT32 ep_state::screen_update_enterp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_nick->screen_update_epnick(screen, bitmap, cliprect);
	return 0;
}

// FIXME: Should this be here or in the Nick device?!?

/* Enterprise has 256 colours, all may be on the screen at once!
 the NICK_GET_RED8, NICK_GET_GREEN8, NICK_GET_BLUE8 macros
 return a 8-bit colour value for the index specified.  */

/* given a colour index in range 0..255 gives the Red component */
#define NICK_GET_RED8(x) \
	((      \
		(BIT(x, 0) << 2) | \
		(BIT(x, 3) << 1) | \
		(BIT(x, 6) << 0) \
	) << 5)

/* given a colour index in range 0..255 gives the Red component */
#define NICK_GET_GREEN8(x) \
	((  \
		(BIT(x, 1) << 2) | \
		(BIT(x, 4) << 1) | \
		(BIT(x, 7) << 0) \
	) << 5)

/* given a colour index in range 0..255 gives the Red component */
#define NICK_GET_BLUE8(x) \
	(( \
		(BIT(x, 2) << 1) | \
		(BIT(x, 5) << 0) \
	) << 6)


/* initial the palette */
void ep_state::palette_init()
{
	for (int i = 0; i < 256; i++)
		palette_set_color_rgb(machine(), i, NICK_GET_RED8(i), NICK_GET_GREEN8(i), NICK_GET_BLUE8(i));
}


/***************************************************************************
    FLOPPY/EXDOS
***************************************************************************/

WRITE_LINE_MEMBER(ep_state::enterp_wd1770_intrq_w)
{
	if (state)
		exdos_card_value |= 0x02;
	else
		exdos_card_value &= ~0x02;
}

WRITE_LINE_MEMBER(ep_state::enterp_wd1770_drq_w)
{
	if (state)
		exdos_card_value |= 0x80;
	else
		exdos_card_value &= ~0x80;
}


/* bit 0 - ??
   bit 1 - IRQ from WD1770
   bit 2 - ??
   bit 3 - ??
   bit 4 - ??
   bit 5 - ??
   bit 6 - Disk change signal from disk drive
   bit 7 - DRQ from WD1770
*/
READ8_MEMBER(ep_state::exdos_card_r)
{
	return exdos_card_value;
}


/* bit 0 - select drive 0,
   bit 1 - select drive 1,
   bit 2 - select drive 2,
   bit 3 - select drive 3
   bit 4 - side
   bit 5 - mfm/fm select
   bit 6 - disk change reset
   bit 7 - in use
*/
WRITE8_MEMBER(ep_state::exdos_card_w)
{
	device_t *fdc = machine().device("wd1770");

	/* drive */
	if (BIT(data, 0)) wd17xx_set_drive(fdc, 0);
	if (BIT(data, 1)) wd17xx_set_drive(fdc, 1);
	if (BIT(data, 2)) wd17xx_set_drive(fdc, 2);
	if (BIT(data, 3)) wd17xx_set_drive(fdc, 3);

	wd17xx_set_side(fdc, BIT(data, 4));
	wd17xx_dden_w(fdc, BIT(data, 5));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( enterprise_mem, AS_PROGRAM, 8, ep_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAMBANK("bank1")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank2")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank3")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank4")
ADDRESS_MAP_END


static ADDRESS_MAP_START( enterprise_io, AS_IO, 8, ep_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_MIRROR(0x04) AM_DEVREADWRITE_LEGACY("wd1770", wd17xx_r, wd17xx_w)
	AM_RANGE(0x18, 0x18) AM_MIRROR(0x04) AM_READWRITE(exdos_card_r, exdos_card_w)
	AM_RANGE(0x80, 0x8f) AM_DEVWRITE("nick", nick_device, reg_w)
	AM_RANGE(0xa0, 0xbf) AM_DEVREADWRITE("custom", dave_sound_device, reg_r, reg_w)
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*
Enterprise Keyboard Matrix

        Bit
Line    0    1    2    3    4    5    6    7
0       n    \    b    c    v    x    z    SHFT
1       h    N/C  g    d    f    s    a    CTRL
2       u    q    y    r    t    e    w    TAB
3       7    1    6    4    5    3    2    N/C
4       F4   F8   F3   F6   F5   F7   F2   F1
5       8    N/C  9    -    0    ^    DEL  N/C
6       j    N/C  k    ;    l    :    ]    N/C
7       STOP DOWN RGHT UP   HOLD LEFT RETN ALT
8       m    ERSE ,    /    .    SHFT SPCE INS
9       i    N/C  o    @    p    [    N/C  N/C

N/C - Not connected or just dont know!

2008-05 FP:
Notice that I updated the matrix above with the following new info:
l1:b1 is LOCK: you press it with CTRL to switch to Caps, you press it again to switch back
    (you can also use it with ALT)
l3:b7 is ESC: you use it to exit from nested programs (e.g. if you start to write a program in BASIC,
    then start EXDOS, you can use ESC to go back to BASIC without losing the program you were writing)

According to pictures and manuals, there seem to be no more keys connected, so I label the remaining N/C
as IPT_UNUSED.

Small note about natural keyboard support: currently
- "Lock" is mapped to 'F9' (it acts like a CAPSLOCK, but no IPT_TOGGLE)
- "Stop" is mapped to 'F10'
- "Hold" is mapped to 'F11'
*/


static INPUT_PORTS_START( ep64 )
	PORT_START("LINE0")     /* keyboard line 0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("LINE1")     /* keyboard line 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOCK") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("LINE2")     /* keyboard line 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')

	PORT_START("LINE3")     /* keyboard line 3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 \xC2\xA3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('\xA3')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("LINE4")     /* keyboard line 4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("LINE5")     /* keyboard line 5 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ERASE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE6")     /* keyboard line 6 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	/* Notice that, in fact, ep128 only had the built-in joystick and no cursor arrow keys on the keyboard */
	PORT_START("LINE7")     /* keyboard line 7 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STOP")           PORT_CODE(KEYCODE_END)              PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)     PORT_CODE(JOYCODE_Y_DOWN_SWITCH)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)    PORT_CODE(JOYCODE_X_RIGHT_SWITCH)   PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)       PORT_CODE(JOYCODE_Y_UP_SWITCH)      PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HOLD")           PORT_CODE(KEYCODE_HOME)             PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)     PORT_CODE(JOYCODE_X_LEFT_SWITCH)    PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)                                        PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALT")            PORT_CODE(KEYCODE_LALT)             PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START("LINE8")     /* keyboard line 8 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)          PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT (right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)       PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("LINE9")     /* keyboard line 9 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JOY1")      /* external joystick 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXTERNAL JOYSTICK 1 RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXTERNAL JOYSTICK 1 LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(JOYCODE_X_LEFT_SWITCH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXTERNAL JOYSTICK 1 DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXTERNAL JOYSTICK 1 UP") PORT_CODE(KEYCODE_UP) PORT_CODE(JOYCODE_Y_UP_SWITCH)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXTERNAL JOYSTICK 1 FIRE") PORT_CODE(KEYCODE_SPACE) PORT_CODE(JOYCODE_BUTTON1)
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static const wd17xx_interface enterp_wd1770_interface =
{
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(ep_state,enterp_wd1770_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(ep_state,enterp_wd1770_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

static LEGACY_FLOPPY_OPTIONS_START(enterprise)
	LEGACY_FLOPPY_OPTION(enterprise, "dsk,img", "Enterprise disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface enterprise_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_3_5_DSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(enterprise),
	"floppy_3_5",
	NULL
};

static MACHINE_CONFIG_START( ep64, ep_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, ENTERPRISE_XTAL_X1/2) /* 4 MHz, generated by DAVE */
	MCFG_CPU_PROGRAM_MAP(enterprise_mem)
	MCFG_CPU_IO_MAP(enterprise_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(ENTERPRISE_SCREEN_WIDTH, ENTERPRISE_SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, ENTERPRISE_SCREEN_WIDTH-1, 0, ENTERPRISE_SCREEN_HEIGHT-1)
	MCFG_SCREEN_UPDATE_DRIVER(ep_state, screen_update_enterp)

	MCFG_PALETTE_LENGTH(NICK_PALETTE_SIZE)

	MCFG_NICK_ADD("nick")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", DAVE, 0)
	MCFG_DEVICE_CONFIG( enterprise_dave_interface )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_WD1770_ADD("wd1770", enterp_wd1770_interface )

	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(enterprise_floppy_interface)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "ep64_flop")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ep128, ep64 )
	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( ep64 )
	ROM_REGION(0x8000, "exos", 0)
	ROM_LOAD("9256ds-0038_enter05-23-a.u2", 0x0000, 0x8000, CRC(d421795f) SHA1(6033a0535136c40c47137e4d1cd9273c06d5fdff))

	/* 4 cartridge slots */
	ROM_REGION(0x10000, "cartridges", 0)
	ROM_LOAD("basic20.rom", 0x0000, 0x4000, CRC(d62e4fb7) SHA1(36e12c4ea782ca769225178f61b55bc9a9afb927))
	ROM_FILL(0x4000, 0xc000, 0xff)

	ROM_REGION(0x8000, "exdos", 0)
	ROM_LOAD("exdos13.rom", 0x0000, 0x8000, CRC(d1d7e157) SHA1(31c8be089526aa8aa019c380cdf51ddd3ee76454))
ROM_END

ROM_START( ep128 )
	ROM_REGION(0x10000, "exos", 0)
	ROM_SYSTEM_BIOS(0, "exos21", "EXOS 2.1")
	ROMX_LOAD("9256ds-0019_enter08-45-a.u2", 0x0000, 0x8000, CRC(982a3b44) SHA1(55315b20fecb4441a07ee4bc5dc7153f396e0a2e), ROM_BIOS(1))
	ROM_FILL(0x8000, 0x8000, 0xff)
	ROM_SYSTEM_BIOS(1, "exos22", "EXOS 2.2 (unofficial)")
	ROMX_LOAD("exos22.rom", 0x0000, 0x10000, CRC(c82e699f) SHA1(40cda9573e0c20e6287d27105759e23b9025fa52), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "exos23", "EXOS 2.3 (unofficial)")
	ROMX_LOAD("exos23.rom", 0x0000, 0x10000, CRC(24838410) SHA1(c6241e1c248193108ce38b9a8e9dd33972cf47ba), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "exos231", "EXOS 2.31 (unofficial)")
	ROMX_LOAD("exos231.rom", 0x0000, 0x10000, CRC(d0ecee0d) SHA1(bd0ff3c46f57c88b82b71b0d94a8bda18ea9bafe), ROM_BIOS(4))

	/* 4 cartridge slots */
	ROM_REGION(0x10000, "cartridges", 0)
	ROM_LOAD("9128ds-0237_enter08-46-a.u1", 0x0000, 0x4000, CRC(683cf455) SHA1(50a548d1df3ea86f9b5fa669afd8ff124050e776)) /* BASIC V2.1 */
	ROM_FILL(0x4000, 0xc000, 0xff)

	ROM_REGION(0x8000, "exdos", 0)
	ROM_LOAD("exdos13.rom", 0x0000, 0x8000, CRC(d1d7e157) SHA1(31c8be089526aa8aa019c380cdf51ddd3ee76454))
ROM_END

ROM_START( phc64 )
	ROM_REGION(0x8000, "exos", 0)
	ROM_LOAD("9256ds-0038_enter05-23-a.u2", 0x0000, 0x8000, CRC(d421795f) SHA1(6033a0535136c40c47137e4d1cd9273c06d5fdff))

	/* 4 cartridge slots */
	ROM_REGION(0x10000, "cartridges", 0)
	ROM_LOAD("basic20.rom", 0x0000, 0x4000, CRC(d62e4fb7) SHA1(36e12c4ea782ca769225178f61b55bc9a9afb927))
	ROM_LOAD("brd.rom", 0x4000, 0x4000, CRC(f45a7454) SHA1(096c91fad6a4d10323cd67e133b3ebc5c50e2bb2))
	ROM_FILL(0x8000, 0x8000, 0xff)

	ROM_REGION(0x8000, "exdos", 0)
	ROM_LOAD("exdos13.rom", 0x0000, 0x8000, CRC(d1d7e157) SHA1(31c8be089526aa8aa019c380cdf51ddd3ee76454))
ROM_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  INIT  COMPANY                 FULLNAME */
COMP( 1985, ep64,  0,      0,      ep64,    ep64, driver_device, 0,     "Enterprise Computers", "Enterprise Sixty Four", GAME_IMPERFECT_SOUND )
COMP( 1985, ep128, ep64,   0,      ep128,   ep64, driver_device, 0,     "Enterprise Computers", "Enterprise One Two Eight", GAME_IMPERFECT_SOUND )
COMP( 1985, phc64, ep64,   0,      ep64,    ep64, driver_device, 0,     "Hegener & Glaser",     "Mephisto PHC 64", GAME_IMPERFECT_SOUND )