// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/************************************************************************
Philips P2000T Memory map

    CPU: Z80
        0000-0fff   ROM
        1000-4fff   ROM (appl)
        5000-57ff   RAM (Screen T ver)
        5000-5fff   RAM (Screen M ver)
        6000-9fff   RAM (system)
        a000-ffff   RAM (extension)

    Interrupts:

    Ports:
        00-09       Keyboard input
        10-1f       Output ports   
                        bit
                         0  - data to cassette
                         1  - write command cassette
                         2  - rewind cassette
                         3  - forward cassette
                         4  - reserved
                         5  - reserved
                         6  - keyboard enable (0=disabled)
                         7  - printer data out (pen 3 serial port)
                         
        20-2f       Input ports  
                       bit
                         0  - data from printer port (pen 2 serial port)
                         1  - printer ready (pen 20 serial port)
                         2  - printer type switch (0=letterwheel, 1=matrix printer)
                         3  - cassette write protection
                         4  - cassette pressent
                         5  - start op tape signal
                         6  - cassette read clock
                         7  - read data from cassette

        2c          2c Hires reset port
        30-3f       Scroll reg (T ver) (bit 7 black screen bit 0-6 horizontal offset/srcoll)
        40-4f       Free I/O ports  (M2009 modem/Serial/mouse/centronics etc.)
        50-5f       Beeper (bit 0)
        60-67       Free I/O ports  (Uniface/V.24 etc.)
        68-6b       Hires communication ports
                        68: PIO A DATA status channel 
                        6a: PIO A ctrl status channel 
                        69: PIO B DATA status channel 
                        6b: PIO B ctrl status channel 

        70-7f       DISAS (M ver)
        
        80-83       CTCBSEL - Z80-CTCB (SIO baud control) (channel 0-3)
        84-87       SIO 
                        84: data reg. RS232
                        85: cmd/status RS232
                        86: data RS422
                        87: md/status RS422)
        88-8b       CTCSEL - Z80-CTC (channel 0-3)
        8c-8f       FDCSEL - Floppy ctrl (fdc) uPD765
        90-93       IOSEL - Floppy/DC control port
        94          SWSEL - RAM Bank select
        95-97       RAM disk 
                        95: set track
                        96: set sector (+ reset data cnt)
                        97: data (in/out)
        98-9b       Centronics 
                        98: data reg. [out]
                        99: status reg. [in]
                        9a: strobe on
                        9b: strobe off
        9c-9d       RTC (9c=set register  9d=data in/out)


    Display: SAA5050

************************************************************************/

#include "emu.h"
#include "includes/p2000t.h"
#include "formats/dsk_dsk.h"

#include "screen.h"
#include "speaker.h"

#define VERBOSE (0)
#include "logmacro.h"

/* port i/o functions */
void p2000t_state::p2000t_io(address_map &map)
{
    map.global_mask(0xff);
    map(0x00, 0x0f).r(FUNC(p2000t_state::p2000t_port_000f_r));
    map(0x00, 0x00).w(FUNC(p2000t_state::p2000t_port_00_w));    
    map(0x10, 0x1f).w(FUNC(p2000t_state::p2000t_port_101f_w));
    map(0x20, 0x2f).r(FUNC(p2000t_state::p2000t_port_202f_r));
    map(0x30, 0x3f).w(FUNC(p2000t_state::p2000t_port_303f_w));  
    map(0x50, 0x5f).w(FUNC(p2000t_state::p2000t_port_505f_w));
    map(0x70, 0x7f).rw(FUNC(p2000t_state::p2000t_port_707f_r), FUNC(p2000t_state::p2000t_port_707f_w));
   
    map(0x94, 0x94).w(FUNC(p2000t_state::p2000t_port_9494_w));  
}

/* Memory w/r functions */
void p2000t_state::p2000t_mem(address_map &map)
{
    map(0x0000, 0x0fff).rom();
    map(0x1000, 0x4fff).rom();
    map(0x5000, 0x57ff).ram().share("videoram");
    map(0x5800, 0x5fff).noprw();  // NO RAM on T-model here
    map(0x6000, 0xdfff).ram();
    map(0xe000, 0xffff).bankrw(m_bank);
}

void p2000m_state::p2000m_mem(address_map &map)
{
    map(0x0000, 0x0fff).rom();
    map(0x1000, 0x4fff).rom();
    map(0x5000, 0x5fff).ram().share("videoram");
    map(0x6000, 0xdfff).ram();
    map(0xe000, 0xffff).bankrw(m_bank);
}

DEVICE_IMAGE_LOAD_MEMBER(p2000t_state::card_load)
{
	uint32_t size = m_slot1->common_get_size("rom");

	m_slot1->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_BIG);
	m_slot1->common_load_rom(m_slot1->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

/* graphics output */
static const gfx_layout p2000m_charlayout =
{
    6, 10,
    256,
    1,
    { 0 },
    { 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8,
        5*8, 6*8, 7*8, 8*8, 9*8 },
    8 * 10
};

void p2000m_state::p2000m_palette(palette_device &palette) const
{
    palette.set_pen_color(0, rgb_t::white()); // white
    palette.set_pen_color(1, rgb_t::black()); // black
    palette.set_pen_color(2, rgb_t::black()); // black
    palette.set_pen_color(3, rgb_t::white()); // white
}

static GFXDECODE_START( gfx_p2000m )
    GFXDECODE_ENTRY( "gfx1", 0x0000, p2000m_charlayout, 0, 2 )
GFXDECODE_END

/* Keyboard input */

/* 
Notice that pictures of p2000 units shows slightly different key mappings, suggesting
many different .chr roms could exist

Small note about natural keyboard support: currently,
- "Code" is mapped to 'F1'
- "Clrln" is mapped to 'F2'
*/

static INPUT_PORTS_START (p2000t)
    PORT_START("KEY.0")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_CHAR(UCHAR_MAMEKEY(LEFT))
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q')
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR(0xA3)
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('\'')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')

    PORT_START("KEY.1")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)         PORT_CHAR('\t')
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z')
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S')
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D')
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G')
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F')

    PORT_START("KEY.2")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHAR(UCHAR_MAMEKEY(00_PAD))
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)       PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("#  \xE2\x96\xAA") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('#')
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

    PORT_START("KEY.3")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock")        PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('<') PORT_CHAR('>')
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X')
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C')
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B')
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V')

    PORT_START("KEY.4")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Code") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(F1))
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A')
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W')
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E')
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T')
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R')
 
    PORT_START("KEY.5")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clrln")             PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(15) PORT_CHAR(12)
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)   PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(8)
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR('=')
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR(0xFF0D)

    PORT_START("KEY.6")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)       PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)       PORT_CHAR(13)
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P')
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@  \xE2\x86\x91")   PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('^')

    PORT_START("KEY.7")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)       PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)       PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)       PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x92  \xE2\x86\x90") PORT_CODE(KEYCODE_CLOSEBRACE)
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"')

    PORT_START("KEY.8")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)       PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L')
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)       PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)       PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR(0x00BC) PORT_CHAR(0x00BE)
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+')
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I')
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(':') PORT_CHAR('*')

    PORT_START("KEY.9")
    PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
    PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N/C")
    PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N/C")
    PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N/C")
    PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N/C")
    PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N/C")
    PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N/C")
    PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT)

    PORT_START("jumper")
	PORT_DIPNAME(   0x04, 0x00, "Printer Type")
	PORT_DIPSETTING(0x04, "Daisywheel")
	PORT_DIPSETTING(0x00, "Matrix")
    PORT_DIPNAME(   0x01, 0x00, "Printer PEN 2 and 6")
	PORT_DIPSETTING(0x01, "Open")
	PORT_DIPSETTING(0x00, "Closed")
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( printer )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

INTERRUPT_GEN_MEMBER(p2000t_state::p2000_interrupt)
{
    if (BIT(m_port_101f, 6))
    {
        // Keyboard int - with FDC/M2200 board available use CTC trg3 (conform orginal design)
        if (!m_ext1->dew_r()) 
        {
            m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
        }
    }
}

WRITE_LINE_MEMBER(p2000t_state::p2000_slot_interrupt)
{
    m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t p2000t_state::videoram_r(offs_t offset)
{
    /* The scroll register in only supported 40 char/line mode */
    if (!in_80char_mode()) 
    {
        if (BIT(m_port_303f, 7)) 
        {   // Bit 7 1 = screen disabled (blank) 0 = screen enabled
            return 0;
        }
        /* The scroll register adds an offset in video ram so the text colums have an offset */
        return m_videoram[(offset + m_port_303f) & 0x0fff];
    }
    return m_videoram[offset];    
}

uint32_t p2000t_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    if (m_ext2->vidon_r())
    {
       m_saa5050->screen_update(screen, bitmap, cliprect);
    }
    return m_ext2->screen_update(screen, bitmap, cliprect);
}

/* Machine definition */
void p2000t_state::p2000t(machine_config &config)
{
    /* basic machine hardware */
    Z80(config, m_maincpu, 2.5_MHz_XTAL);
    m_maincpu->set_addrmap(AS_PROGRAM, &p2000t_state::p2000t_mem);
    m_maincpu->set_addrmap(AS_IO, &p2000t_state::p2000t_io);
    m_maincpu->set_vblank_int("screen", FUNC(p2000t_state::p2000_interrupt));
    
	/* video hardware */
    SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
    m_screen->set_refresh_hz(50);
    m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
    m_screen->set_size(80 * 12, 24 * 20);
    m_screen->set_visarea(0, 40 * 12 - 1, 0, 24 * 20 - 1);
    m_screen->set_screen_update(FUNC(p2000t_state::screen_update));
    
    SAA5050(config, m_saa5050, 6_MHz_XTAL);
    m_saa5050->d_cb().set(FUNC(p2000t_state::videoram_r));
    m_saa5050->set_screen_size(80, 24, 80);

    /* sound hardware */
    SPEAKER(config, "mono").front_center();
    SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

    /* the mini cassette driver */
    MDCR(config, m_mdcr, 0);

    /* Serial printer port */
    RS232_PORT(config, m_printer, default_rs232_devices, "printer");
    m_printer->set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(printer));
    m_printer->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(printer));
	m_printer->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(printer));

    /* Extention Slots 1 + 2, ext 1 + 2 */
    GENERIC_CARTSLOT(config, m_slot1, generic_plain_slot, "p2000_cart", "bin,rom");
    m_slot1->set_device_load(FUNC(p2000t_state::card_load));
    
    P2000_EXPANSION_SLOT(config, m_slot2, 0, p2000_slot2_devices, nullptr);
    m_slot2->set_io_space(m_maincpu, AS_IO);
    m_slot2->irq().set(FUNC(p2000_slot_interrupt));

    P2000_EXPANSION_SLOT(config, m_ext1, 0, p2000_ext1_devices, nullptr);
    m_ext1->set_io_space(m_maincpu, AS_IO);
    m_ext1->irq().set(FUNC(p2000_slot_interrupt));
    
    P2000_EXPANSION_SLOT(config, m_ext2, 0, p2000_ext2_devices, nullptr);
    m_ext2->set_io_space(m_maincpu, AS_IO);
    m_ext2->irq().set(FUNC(p2000_slot_interrupt));
    m_ext2->in_mode80().set(FUNC(in_80char_mode));

    /* internal ram */
    RAM(config, m_ram).set_default_size("16K").set_extra_options("16K,32K,48K,64K,80K,104K");

    /* software lists */ 
	SOFTWARE_LIST(config, "cart_list").set_original("p2000_cart");
}

/* Machine definition */
void p2000m_state::p2000m(machine_config &config)
{
    /* basic machine hardware */
    Z80(config, m_maincpu, 2.5_MHz_XTAL);
    m_maincpu->set_addrmap(AS_PROGRAM, &p2000m_state::p2000m_mem);
    m_maincpu->set_addrmap(AS_IO, &p2000m_state::p2000t_io);
    m_maincpu->set_vblank_int("screen", FUNC(p2000m_state::p2000_interrupt));
    config.set_maximum_quantum(attotime::from_hz(60));

    /* video hardware */
    screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
    screen.set_refresh_hz(50);
    screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
    screen.set_size(80 * 12, 24 * 20);
    screen.set_visarea(0, 80 * 12 - 1, 0, 24 * 20 - 1);
    screen.set_screen_update(FUNC(p2000m_state::screen_update_p2000m));
    screen.set_palette(m_palette);
    
    GFXDECODE(config, m_gfxdecode, m_palette, gfx_p2000m);
    PALETTE(config, m_palette, FUNC(p2000m_state::p2000m_palette), 4);

    /* sound hardware */
    SPEAKER(config, "mono").front_center();
    SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

    /* the mini cassette driver */
    MDCR(config, m_mdcr, 0);

    /* Serial printer port */
    RS232_PORT(config, m_printer, default_rs232_devices, "printer");
    m_printer->set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(printer));
    m_printer->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(printer));
	m_printer->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(printer));
	
    /* Extention Slots 1 + 2, ext 1 + 2 */
    GENERIC_CARTSLOT(config, m_slot1, generic_plain_slot, "p2000_cart", "bin,rom");
    m_slot1->set_device_load(FUNC(card_load));

    P2000_EXPANSION_SLOT(config, m_slot2, 0, p2000_slot2_devices, nullptr);
    m_slot2->set_io_space(m_maincpu, AS_IO);
    m_slot2->irq().set(FUNC(p2000_slot_interrupt));

    P2000_EXPANSION_SLOT(config, m_ext1, 0, p2000_ext1_devices, nullptr);
    m_ext1->set_io_space(m_maincpu, AS_IO);
    m_slot2->irq().set(FUNC(p2000_slot_interrupt));

	P2000_EXPANSION_SLOT(config, m_ext2, 0, p2000_ext2_devices, nullptr);
    m_ext2->set_io_space(m_maincpu, AS_IO);
    m_slot2->irq().set(FUNC(p2000_slot_interrupt));

    /* internal ram */
    RAM(config, m_ram).set_default_size("16K").set_extra_options("16K,32K,48K,64K,80K,96K,102K");

    /* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("p2000_cart");
}

ROM_START(p2000t)
    ROM_REGION(0x10000, "maincpu",0)
    ROM_LOAD("p2000.rom", 0x0000, 0x1000, CRC(650784a3) SHA1(4dbb28adad30587f2ea536ba116898d459faccac))
ROM_END

ROM_START(p2000m)
    ROM_REGION(0x10000, "maincpu",0)
    ROM_LOAD("p2000.rom", 0x0000, 0x1000, CRC(650784a3) SHA1(4dbb28adad30587f2ea536ba116898d459faccac))

    ROM_REGION(0x01000, "gfx1",0)
    ROM_LOAD("p2000.chr", 0x0140, 0x08c0, BAD_DUMP CRC(78c17e3e) SHA1(4e1c59dc484505de1dc0b1ba7e5f70a54b0d4ccc))
ROM_END

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY    FULLNAME          FLAGS
COMP( 1980, p2000t, 0,      0,      p2000t,  p2000t, p2000t_state, empty_init, "Philips", "Philips P2000T", 0 )
COMP( 1980, p2000m, p2000t, 0,      p2000m,  p2000t, p2000m_state, empty_init, "Philips", "Philips P2000M", 0 )
