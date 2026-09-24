// license:GPL-2.0+
// copyright-holders:Curt Coder,Dirk Best
/***************************************************************************

    Epson PX-8

***************************************************************************/

/*

    TODO:

    - dumps of the internal ROMs
    - uPD7508 CPU core
    - cassette
    - jumpers
    - uPD7001 (controlled by uPD7508)
    - RAM disk (64K/128K RAM, Z80, 4K ROM)
    - modem (82C55)
    - Multi-Unit (60K RAM, ROM capsule, modem, 82C55)
    - attach PF-10s

*/

#include "emu.h"
#include "px8.h"

#include "machine/rescap.h"
#include "machine/upd7001.h"
#include "video/sed1330.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "px8.lh"

#include "utf8.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define XTAL_CR1        XTAL(9'830'400)
#define XTAL_CR2        XTAL(32'768)

enum
{
	GAH40M_CTLR1 = 0,
	GAH40M_CMDR,
	GAH40M_CTLR2,
	GAH40M_IER = 4
};

enum
{
	GAH40M_ICRL_C = 0,
	GAH40M_ICRH_C,
	GAH40M_ICRL_B,
	GAH40M_ICRH_B,
	GAH40M_ISR,
	GAH40M_STR,
	GAH40M_SIOR,
	GAH40M_IVR
};

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    bankswitch - memory bankswitching
-------------------------------------------------*/

void px8_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();
	uint8_t *ipl_rom = memregion(UPD70008_TAG)->base();

	if (!m_bank0)
	{
		/* IPL ROM */
		program.install_rom(0x0000, 0x7fff, ipl_rom);
	}
	else
	{
		if (m_bk2)
		{
			/* D-RAM (L) */
			program.install_ram(0x0000, 0x7fff, ram);
		}
		else
		{
			/* OPTION ROM (L) */
			program.unmap_readwrite(0x0000, 0x7fff);
		}
	}

	if (m_bk2)
	{
		/* D-RAM (H) */
		program.install_ram(0x8000, 0xffff, ram + 0x8000);
	}
	else
	{
		/* OPTION ROM (H) */
		program.unmap_readwrite(0x8000, 0xffff);
	}
}

/*-------------------------------------------------
    gah40m_r - GAH40M read
-------------------------------------------------*/

uint8_t px8_state::gah40m_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case GAH40M_ICRL_C:
		/*

		    bit     signal      description

		    0       ICR0
		    1       ICR1
		    2       ICR2
		    3       ICR3
		    4       ICR4
		    5       ICR5
		    6       ICR6
		    7       ICR7

		*/

		m_icr = (machine().time() - m_frc_start).as_ticks(XTAL_CR1 / 16);
		data = m_icr & 0xff;
		break;

	case GAH40M_ICRH_C:
		/*

		    bit     signal      description

		    0       ICR8
		    1       ICR9
		    2       ICR10
		    3       ICR11
		    4       ICR12
		    5       ICR13
		    6       ICR14
		    7       ICR15

		*/

		data = m_icr >> 8;
		break;

	case GAH40M_ICRL_B:
		/*

		    bit     signal      description

		    0       ICR0
		    1       ICR1
		    2       ICR2
		    3       ICR3
		    4       ICR4
		    5       ICR5
		    6       ICR6
		    7       ICR7

		*/
		break;

	case GAH40M_ICRH_B:
		/*

		    bit     signal      description

		    0       ICR8
		    1       ICR9
		    2       ICR10
		    3       ICR11
		    4       ICR12
		    5       ICR13
		    6       ICR14
		    7       ICR15

		*/
		break;

	case GAH40M_ISR:
		/*

		    bit     signal      description

		    0       INT0        7508 interrupt (INT 7508)
		    1       INT1        82C51 interrupt (INT 82C51)
		    2       INT2        6303 interrupt (INT 6303)
		    3       INT3        input capture flag timer (CF)
		    4       INT4        overflow flag timer (OVF)
		    5       INT5        external interrupt (INTEXT)
		    6
		    7

		*/

		data = m_isr;
		break;

	case GAH40M_STR:
		/*

		    bit     signal      description

		    0       _BANK0
		    1       BRDT        barcode reader data timer
		    2       RDY         ready (SIO)
		    3       RDYSIO      SIO ready (SIO)
		    4
		    5
		    6
		    7

		*/

		data = (m_rdysio ? 0x08 : 0) | (m_bank0 ? 0x01 : 0);
		break;

	case GAH40M_SIOR:
		/*

		    bit     signal      description

		    0       SIO0
		    1       SIO1
		    2       SIO2
		    3       SIO3
		    4       SIO4
		    5       SIO5
		    6       SIO6
		    7       SIO7

		*/

		data = m_sio;
		break;

	case GAH40M_IVR:
		/*

		    bit     description

		    0       0
		    1       vect 1
		    2       vect 2
		    3       vect 3
		    4       1
		    5       1
		    6       1
		    7       1

		*/
		break;
	}

	return data;
}

/*-------------------------------------------------
    gah40m_w - GAH40M write
-------------------------------------------------*/

void px8_state::gah40m_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case GAH40M_CTLR1:
		/*

		    bit     signal      description

		    0       _BANK0      bank switching
		    1       BCK0        barcode mode select 0 timer (down)
		    2       BCK1        barcode mode select 1 timer (up)
		    3       SWBCD       barcode reader switch timer
		    4       BRG0        bad rate generator select 0 timer
		    5       BRG1        bad rate generator select 1 timer
		    6       BRG2        bad rate generator select 2 timer
		    7       BRG3        bad rate generator select 3 timer

		*/

		m_bank0 = BIT(data, 0);
		bankswitch();
		break;

	case GAH40M_CMDR:
		/*

		    bit     description

		    0       set RDYSIOFF (pulse)
		    1       reset RDYSIOFF (pulse)
		    2       reset OVF (pulse)
		    3
		    4
		    5
		    6
		    7

		*/

		if (BIT(data, 0))
			m_rdysio = true;

		if (BIT(data, 1))
		{
			m_rdysio = false;
			sub_handshake();
		}

		if (BIT(data, 2))
		{
			m_isr &= ~INT4_FRC;
			update_interrupt();
		}
		break;

	case GAH40M_CTLR2:
		/*

		    bit     signal      description

		    0       LED0        LED
		    1       LED1        LED
		    2       LED2        LED
		    3       SWRS        RS-232C switch
		    4       INHRS       inhibit RS-232C
		    5       AUX         external auxiliary output
		    6
		    7

		*/

		for (int i = 0; i < 3; i++)
			m_leds[i] = BIT(data, i);
		break;

	case GAH40M_IER:
		/*

		    bit     signal      description

		    0       IER0        INT 7508 enable
		    1       IER1        INT 82C51 enable
		    2       IER2        INT 6303 enable
		    3       IER3        INTICF enable
		    4       IER4        INTOVF enable
		    5       IER5        INTEXT enable
		    6
		    7

		*/

		m_ier = data;
		update_interrupt();
		break;

	case GAH40M_SIOR:
		/*

		    bit     signal      description

		    0       SIO0
		    1       SIO1
		    2       SIO2
		    3       SIO3
		    4       SIO4
		    5       SIO5
		    6       SIO6
		    7       SIO7

		*/

		m_sio = data;
		break;
	}
}

/*-------------------------------------------------
    gah40s_r - GAH40S read
-------------------------------------------------*/

uint8_t px8_state::gah40s_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0: /* counter (upper byte) input */
		data = (m_cnt >> 8) & 0x1f;
		break;

	case 1: /* counter (lower byte) */
		data = m_cnt & 0xff;
		break;

	case 3: /* P-ROM read data */
		data = m_prd;
		break;
	}

	return data;
}

/*-------------------------------------------------
    gah40s_w - GAH40S write
-------------------------------------------------*/

void px8_state::gah40s_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: /* counter reset */
		m_cnt = 0;
		break;

	case 1: /* command register */
		/*

		    bit     signal      description

		    0       SW PR       PROM power switch
		    1       SW MCT      microcassette power switch
		    2       MTA         microcassette drive motor control signal A
		    3       MTB         microcassette drive motor control signal B
		    4       MTC         microcassette drive motor control signal C
		    5
		    6       _FAST
		    7       _STOP CNT

		*/

		m_swpr = BIT(data, 0);
		break;

	case 2: /* P-ROM address (upper 8 byte) */
		m_pra = (data << 8) | (m_pra & 0xff);
		m_prd = 0; // TODO read prom!
		break;

	case 3: /* P-ROM address (lower 8 byte) */
		m_pra = (m_pra & 0xff00) | data;
		m_prd = 0; // TODO read prom!
		break;
	}
}

/*-------------------------------------------------
    gah40s_ier_w - interrupt enable register write
-------------------------------------------------*/

void px8_state::gah40s_ier_w(uint8_t data)
{
	m_ier = data;
}

/*-------------------------------------------------
   krtn_read - read keyboard return
-------------------------------------------------*/

uint8_t px8_state::krtn_read()
{
	uint8_t data = 0xff;

	switch (m_ksc)
	{
	case 0: data = ioport("KSC0")->read(); break;
	case 1: data = ioport("KSC1")->read(); break;
	case 2: data = ioport("KSC2")->read(); break;
	case 3: data = ioport("KSC3")->read(); break;
	case 4: data = ioport("KSC4")->read(); break;
	case 5: data = ioport("KSC5")->read(); break;
	case 6: data = ioport("KSC6")->read(); break;
	case 7: data = ioport("KSC7")->read(); break;
	case 8: data = ioport("KSC8")->read(); break;
	case 9: data = ioport("SW4")->read();  break;
	}

	return data;
}

/*-------------------------------------------------
   krtn_0_3_r - keyboard return 0..3 read
-------------------------------------------------*/

uint8_t px8_state::krtn_0_3_r()
{
	return krtn_read() & 0x0f;
}

/*-------------------------------------------------
   krtn_4_7_r - keyboard return 4..7 read
-------------------------------------------------*/

uint8_t px8_state::krtn_4_7_r()
{
	return krtn_read() >> 4;
}

/*-------------------------------------------------
   ksc_w - keyboard scan write
-------------------------------------------------*/

void px8_state::ksc_w(uint8_t data)
{
	m_ksc = data;
}

/*-------------------------------------------------
    update_interrupt - update Z80 interrupt line
-------------------------------------------------*/

void px8_state::update_interrupt()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_isr & m_ier & 0x3f) ? ASSERT_LINE : CLEAR_LINE);
}

/*-------------------------------------------------
    irq_ack - interrupt acknowledge
-------------------------------------------------*/

int px8_state::irq_ack(device_t &device, int irqline)
{
	uint8_t pending = m_isr & m_ier & 0x3f;

	for (int i = 0; i < 6; i++)
	{
		if (BIT(pending, i))
			return 0xf0 | (i << 1);
	}

	return 0xff;
}

TIMER_CALLBACK_MEMBER(px8_state::frc_tick)
{
	m_frc_start = machine().time();
	m_isr |= INT4_FRC;
	update_interrupt();
}

/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( px8_mem )
-------------------------------------------------*/

void px8_state::px8_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankrw("bank0");
	map(0x8000, 0xffff).bankrw("bank1");
}

/*-------------------------------------------------
    ADDRESS_MAP( px8_io )
-------------------------------------------------*/

void px8_state::px8_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x0f);
	map(0x00, 0x07).rw(FUNC(px8_state::gah40m_r), FUNC(px8_state::gah40m_w));
	map(0x0c, 0x0d).rw(I8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0e, 0x0e).rw(FUNC(px8_state::slave_status_r), FUNC(px8_state::slave_data_w));
	map(0x0f, 0x0f).rw(FUNC(px8_state::slave_data_r), FUNC(px8_state::slave_cmd_w));
}

/*-------------------------------------------------
    ADDRESS_MAP( px8_slave_mem )
-------------------------------------------------*/

void px8_state::px8_slave_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0020, 0x0023).rw(FUNC(px8_state::gah40s_r), FUNC(px8_state::gah40s_w));
//  map(0x0024, 0x0027).rw(SED1320_TAG, FUNC(sed1330_device::), FUNC(sed1330_device::));
	map(0x0028, 0x0028).w(FUNC(px8_state::gah40s_ier_w));
	map(0x8000, 0x97ff).ram();
	map(0x9800, 0xefff).noprw();
	map(0xf000, 0xffff).rom().region(HD6303_TAG, 0); /* internal mask rom */
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( px8 )
	PORT_START("KSC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SCRN INS") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Right CTRL") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Right SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NUM GRPH") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))

	PORT_START("KSC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PAUSE") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF5") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("KSC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KSC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("KSC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("KSC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("KSC6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')

	PORT_START("KSC7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR(96)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("KSC8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("SW4")
	PORT_DIPNAME( 0xef, 0x2f, "Character Set" ) PORT_DIPLOCATION("SW4:1,2,3,4,6,7,8")
	PORT_DIPSETTING(    0x2f, "ASCII" )
	PORT_DIPSETTING(    0x2e, DEF_STR( French ) )
	PORT_DIPSETTING(    0x2d, DEF_STR( German ) )
	PORT_DIPSETTING(    0x2c, DEF_STR( English ) )
	PORT_DIPSETTING(    0x2b, "Danish" )
	PORT_DIPSETTING(    0x2a, "Swedish" )
	PORT_DIPSETTING(    0x26, "Norwegian" )
	PORT_DIPSETTING(    0x29, "Italy" )
	PORT_DIPSETTING(    0x28, "Spain" )
	PORT_DIPSETTING(    0x60, "HASCI" )
	PORT_DIPSETTING(    0x21, "Japanese (Japanese)" )
	PORT_DIPSETTING(    0x00, "Japanese (JIS)" )
	PORT_DIPSETTING(    0x01, "Japanese (touch 16)" )
	PORT_DIPNAME( 0x10, 0x00, "RAM disk check" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
    VIDEO
***************************************************************************/

void px8_state::px8_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xa5, 0xad, 0xa5);
	palette.set_pen_color(1, 0x31, 0x39, 0x10);
}

/*-------------------------------------------------
    gfx_layout px8_charlayout
-------------------------------------------------*/

static const gfx_layout px8_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

/*-------------------------------------------------
    GFXDECODE( px8 )
-------------------------------------------------*/

static GFXDECODE_START( gfx_px8 )
	GFXDECODE_ENTRY( "font", 0x0000, px8_charlayout, 0, 1 )
GFXDECODE_END

/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    MACHINE_START( px8 )
-------------------------------------------------*/

void px8_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_ier));
	save_item(NAME(m_isr));
	save_item(NAME(m_bank0));
	save_item(NAME(m_bk2));
	save_item(NAME(m_sio));
	save_item(NAME(m_ksc));

	m_sram = std::make_unique<uint8_t[]>(0x10000);
	std::fill_n(m_sram.get(), 0x10000, 0);
	std::fill(std::begin(m_key_state), std::end(m_key_state), 0xff);
	std::fill(std::begin(m_alarm), std::end(m_alarm), 0);

	system_time systime;
	machine().base_datetime(systime);
	m_rtc[0] = systime.local_time.year % 100;
	m_rtc[1] = systime.local_time.month + 1;
	m_rtc[2] = systime.local_time.mday;
	m_rtc[3] = systime.local_time.hour;
	m_rtc[4] = systime.local_time.minute;
	m_rtc[5] = systime.local_time.second;
	m_rtc[6] = systime.local_time.weekday;

	sub_init();

	m_frc_timer = timer_alloc(FUNC(px8_state::frc_tick), this);
	m_sub_second_timer = timer_alloc(FUNC(px8_state::sub_second_tick), this);
	m_sub_keyboard_timer = timer_alloc(FUNC(px8_state::sub_keyboard_scan), this);

	m_sub_second_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
	m_sub_keyboard_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

void px8_state::machine_reset()
{
	m_bank0 = 0;
	m_bk2 = 1;

	bankswitch();

	m_isr = 0;
	m_ier = 0;
	update_interrupt();

	attotime period = attotime::from_ticks(0x10000, XTAL_CR1 / 16);
	m_frc_start = machine().time();
	m_frc_timer->adjust(period, 0, period);

	m_rdysio = true;
	m_sub_cmd.clear();
	m_sub_rsp.clear();
	m_sub_status = m_sub_cold ? 0x08 : 0x10;
	m_sub_cold = false;

	m_slave_buf.clear();
	m_slave_rsp.clear();
	m_gudc.clear();
	m_cs_addr = 0x8100;
	m_gs_addr = 0x8380;
	m_scr_ptr = m_cs_addr;
	m_udc_start = 0xe0;
	m_lcd_on = true;
	m_char_mode = true;
	m_seven_lines = false;
	m_curs_mode = 0;
	m_curs_x = 0;
	m_curs_y = 0;
	m_wnd_x = 0;
	m_wnd_y = 0;
	m_flash = 0;
	m_mct_protect = false;
	m_mct_counter = 0;
	m_prom_power = false;

	lcdc_init();
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void px8_state::px8(machine_config &config)
{
	/* main cpu (uPD70008) */
	Z80(config, m_maincpu, XTAL_CR1 / 4); /* 2.45 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &px8_state::px8_mem);
	m_maincpu->set_addrmap(AS_IO, &px8_state::px8_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(px8_state::irq_ack));

	/* slave cpu (HD6303CA) */
	hd6301_cpu_device &slave(HD6301V1(config, HD6303_TAG, XTAL_CR1 / 4)); /* 614 kHz */
	slave.set_addrmap(AS_PROGRAM, &px8_state::px8_slave_mem);
	slave.set_disable();

	/* sub CPU (uPD7508) */
//  upd7508_device &sub(UPD7508(config, UPD7508_TAG, 200000)); /* 200 kHz */
//  sub.set_addrmap(AS_IO, &px8_state::px8_sub_io);
//  sub.set_disable();

	/* video hardware */
	config.set_default_layout(layout_px8);

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(72);
	m_screen->set_screen_update(m_lcdc, FUNC(sed1330_device::screen_update));
	m_screen->set_size(480, 64);
	m_screen->set_visarea(0, 479, 0, 63);
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(FUNC(px8_state::lcdc_update));

	SED1330(config, m_lcdc, XTAL_CR1 / 4);
	m_lcdc->set_screen(m_screen);

	GFXDECODE(config, "gfxdecode", "palette", gfx_px8);
	PALETTE(config, "palette", FUNC(px8_state::px8_palette), 2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* cartridge */
	GENERIC_CARTSLOT(config, "capsule1", generic_plain_slot, "px8_cart", "bin,rom");

	GENERIC_CARTSLOT(config, "capsule2", generic_plain_slot, "px8_cart", "bin,rom");

	/* devices */
	I8251(config, I8251_TAG, XTAL_CR1 / 4);

	UPD7001(config, UPD7001_TAG, RES_K(27), CAP_P(47));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(0, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");

	// software
	SOFTWARE_LIST(config, "cart_list").set_original("px8_cart");
	SOFTWARE_LIST(config, "epson_cpm_list").set_original("epson_cpm");
}

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( px8 )
	ROM_REGION( 0x10000, UPD70008_TAG, 0 )
	ROM_DEFAULT_BIOS("052884")
	ROM_SYSTEM_BIOS( 0, "091383", "9/13/83" )
	ROMX_LOAD( "m25030aa.2a", 0x0000, 0x8000, CRC(bd3e4938) SHA1(5bd48abd2a563a1ae31ff137280f40c8f756e969), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "052884", "5/28/84" )
	ROMX_LOAD( "px060688.2a", 0x0000, 0x8000, CRC(44308bdf) SHA1(5c4545fcf1af9931b4699436294d9b6298052a7b), ROM_BIOS(1) )

	ROM_REGION( 0x0800, "font", 0 )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(5b52edbd) SHA1(38197edf301bb2843bea040536af545f76b3d44f) )

	ROM_REGION( 0x1000, HD6303_TAG, 0 )
	ROM_LOAD( "hd6303 slave cpu internal rom.13d", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, UPD7508_TAG, 0 )
	ROM_LOAD( "upd7508 sub cpu internal rom.2e", 0x0000, 0x1000, NO_DUMP )

	// Possibly cartridges
	ROM_REGION( 0x8000, "carts", 0 )
	ROM_LOAD( "px8-util.rom",           0x00000, 0x8000, CRC(4430a271) SHA1(58c23a5f25ad9cdb70ada44dc773e6899e9bd8bf) ) // various utilities
ROM_END

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME  FLAGS */
COMP( 1984, px8,  0,      0,      px8,     px8,   px8_state, empty_init, "Epson", "PX-8",   MACHINE_SUPPORTS_SAVE )
