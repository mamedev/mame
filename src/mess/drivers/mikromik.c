/*

    Nokia Elektroniikka pj

    Controller ILC 9534
    FDC-Interface ILC 9530

    Parts:

    6,144 MHz xtal (CPU clock)
    18,720 MHz xtal (pixel clock)
    16 MHz xtal (FDC clock)
    Intel I8085AP (CPU)
    Intel 8253-5P (PIT)
    Intel 8275P (CRTC)
    Intel 8212P (I/OP)
    Intel 8237A-5P (DMAC)
    NEC uPD7220C (GDC)
    NEC uPD7201P (MPSC=uart)
    NEC uPD765 (FDC)
    TMS4116-15 (16Kx4 DRAM)*4 = 32KB Video RAM for 7220
    2164-6P (64Kx1 DRAM)*8 = 64KB Work RAM

    DMA channels:

    0   CRT
    1   MPSC transmit
    2   MPSC receive
    3   FDC

    Interrupts:

    INTR    MPSC INT
    RST5.5  FDC IRQ
    RST6.5  8212 INT
    RST7.5  DMA EOP

*/

/*

    TODO:

    - add HRTC/VRTC output to i8275
    - NEC uPD7220 GDC
    - accurate video timing
    - floppy DRQ during RECALL = 0
    - PCB layout
    - NEC uPD7201 MPSC

*/

#include "includes/mikromik.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define MMU_IOEN_MEMEN	0x01
#define MMU_RAMEN		0x02
#define MMU_CE4			0x08
#define MMU_CE0			0x10
#define MMU_CE1			0x20
#define MMU_CE2			0x40
#define MMU_CE3			0x80



//**************************************************************************
//  MEMORY MANAGEMENT UNIT
//**************************************************************************

//-------------------------------------------------
//  mmu_r -
//-------------------------------------------------

READ8_MEMBER( mm1_state::mmu_r )
{
	UINT8 data = 0;
	UINT8 mmu = m_mmu_rom[(m_a8 << 8) | (offset >> 8)];

	if (mmu & MMU_IOEN_MEMEN)
	{
		switch ((offset >> 4) & 0x07)
		{
		case 0:
			data = m_dmac->read(space, offset & 0x0f);
			break;

		case 1:
			data = m_mpsc->cd_ba_r(space, offset & 0x03);
			break;

		case 2:
			data = i8275_r(m_crtc, space, offset & 0x01);
			break;

		case 3:
			data = pit8253_r(m_pit, space, offset & 0x03);
			break;

		case 4:
			data = m_iop->data_r(space, 0);
			break;

		case 5:
			if (BIT(offset, 0))
			{
				data = upd765_data_r(m_fdc, space, 0);
			}
			else
			{
				data = upd765_status_r(m_fdc, space, 0);
			}
			break;

		case 7:
			data = m_hgdc->read(space, offset & 0x01);
			break;
		}
	}
	else
	{
		if (mmu & MMU_RAMEN)
		{
			data = m_ram->pointer()[offset];
		}
		else if (!(mmu & MMU_CE0))
		{
			data = memregion(I8085A_TAG)->base()[offset & 0x1fff];
		}
		else if (!(mmu & MMU_CE1))
		{
			data = memregion(I8085A_TAG)->base()[0x2000 + (offset & 0x1fff)];
		}
	}

	return data;
}



//-------------------------------------------------
//  mmu_w -
//-------------------------------------------------

WRITE8_MEMBER( mm1_state::mmu_w )
{
	UINT8 mmu = m_mmu_rom[(m_a8 << 8) | (offset >> 8)];

	if (mmu & MMU_IOEN_MEMEN)
	{
		switch ((offset >> 4) & 0x07)
		{
		case 0:
			m_dmac->write(space, offset & 0x0f, data);
			break;

		case 1:
			m_mpsc->cd_ba_w(space, offset & 0x03, data);
			break;

		case 2:
			i8275_w(m_crtc, space, offset & 0x01, data);
			break;

		case 3:
			pit8253_w(m_pit, space, offset & 0x03, data);
			break;

		case 4:
			m_iop->data_w(space, 0, data);
			break;

		case 5:
			if (BIT(offset, 0))
			{
				upd765_data_w(m_fdc, space, 0, data);
			}
			break;

		case 6:
			ls259_w(space, offset & 0x07, data);
			break;

		case 7:
			m_hgdc->write(space, offset & 0x01, data);
			break;
		}
	}
	else
	{
		if (mmu & MMU_RAMEN)
		{
			m_ram->pointer()[offset] = data;
		}
	}
}


//-------------------------------------------------
//  ls259_w -
//-------------------------------------------------

WRITE8_MEMBER( mm1_state::ls259_w )
{
	int d = BIT(data, 0);

	switch (offset)
	{
	case 0: // IC24 A8
		if (LOG) logerror("IC24 A8 %u\n", d);
		m_a8 = d;
		break;

	case 1: // RECALL
		if (LOG) logerror("RECALL %u\n", d);
		m_recall = d;
		upd765_reset_w(m_fdc, d);
		break;

	case 2: // _RV28/RX21
		m_rx21 = d;
		break;

	case 3: // _TX21
		m_tx21 = d;
		break;

	case 4: // _RCL
		m_rcl = d;
		break;

	case 5: // _INTC
		m_intc = d;
		break;

	case 6: // LLEN
		if (LOG) logerror("LLEN %u\n", d);
		m_llen = d;
		break;

	case 7: // MOTOR ON
		if (LOG) logerror("MOTOR %u\n", d);
		floppy_mon_w(m_floppy0, !d);
		floppy_mon_w(m_floppy1, !d);
		floppy_drive_set_ready_state(m_floppy0, d, 1);
		floppy_drive_set_ready_state(m_floppy1, d, 1);

		if (ioport("T5")->read()) upd765_ready_w(m_fdc, d);
		break;
	}
}



//**************************************************************************
//  KEYBOARD
//**************************************************************************

//-------------------------------------------------
//  scan_keyboard -
//-------------------------------------------------

void mm1_state::scan_keyboard()
{
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8", "ROW9" };

	UINT8 data = ioport(keynames[m_drive])->read();
	UINT8 special = ioport("SPECIAL")->read();
	int ctrl = BIT(special, 0);
	int shift = BIT(special, 2) & BIT(special, 1);
	UINT8 keydata = 0xff;

	if (!BIT(data, m_sense))
	{
		// get key data from PROM
		keydata = m_key_rom[(ctrl << 8) | (shift << 7) | (m_drive << 3) | (m_sense)];
	}

	if (m_keydata != keydata)
	{
		// latch key data
		m_keydata = keydata;

		if (keydata != 0xff)
		{
			// strobe in key data
			m_iop->stb_w(1);
			m_iop->stb_w(0);
		}
	}

	if (keydata == 0xff)
	{
		// increase scan counters
		m_sense++;

		if (m_sense == 8)
		{
			m_sense = 0;
			m_drive++;

			if (m_drive == 10)
			{
				m_drive = 0;
			}
		}
	}

}


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( kbclk_tick )
//-------------------------------------------------

static TIMER_DEVICE_CALLBACK( kbclk_tick )
{
	mm1_state *state = timer.machine().driver_data<mm1_state>();

	state->scan_keyboard();
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( mm1_map )
//-------------------------------------------------

static ADDRESS_MAP_START( mm1_map, AS_PROGRAM, 8, mm1_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mmu_r, mmu_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( mm1 )
//-------------------------------------------------

static INPUT_PORTS_START( mm1 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC2\xB4 '") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(0x00B4) PORT_CHAR('`')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x96")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(a_RING " " A_RING) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('~') PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LF")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('@') PORT_CHAR('*')

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)

	PORT_START("ROW8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(a_UMLAUT " " A_UMLAUT) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("ROW9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(o_UMLAUT " " O_UMLAUT) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Right SHIFT") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("T5")
	PORT_CONFNAME( 0x01, 0x00, "Floppy Drive Type")
	PORT_CONFSETTING( 0x00, "640 KB" )
	PORT_CONFSETTING( 0x01, "160/320 KB" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8212_INTERFACE( iop_intf )
//-------------------------------------------------

READ8_MEMBER( mm1_state::kb_r )
{
	return m_keydata;
}

static I8212_INTERFACE( iop_intf )
{
	DEVCB_CPU_INPUT_LINE(I8085A_TAG, I8085_RST65_LINE),
	DEVCB_DRIVER_MEMBER(mm1_state, kb_r),
	DEVCB_NULL
};


//-------------------------------------------------
//  I8237_INTERFACE( dmac_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( mm1_state::dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// Assert HLDA
	m_dmac->hack_w(state);
}

READ8_MEMBER( mm1_state::mpsc_dack_r )
{
	// clear data request
	m_dmac->dreq2_w(CLEAR_LINE);

	return m_mpsc->dtra_r();
}

WRITE8_MEMBER( mm1_state::mpsc_dack_w )
{
	m_mpsc->hai_w(data);

	// clear data request
	m_dmac->dreq1_w(CLEAR_LINE);
}

WRITE_LINE_MEMBER( mm1_state::tc_w )
{
	if (!m_dack3)
	{
		// floppy terminal count
		upd765_tc_w(m_fdc, !state);
	}

	m_tc = !state;

	m_maincpu->set_input_line(I8085_RST75_LINE, state);
}

WRITE_LINE_MEMBER( mm1_state::dack3_w )
{
	m_dack3 = state;

	if (!m_dack3)
	{
		// floppy terminal count
		upd765_tc_w(m_fdc, m_tc);
	}
}

static UINT8 memory_read_byte(address_space &space, offs_t address) { return space.read_byte(address); }
static void memory_write_byte(address_space &space, offs_t address, UINT8 data) { space.write_byte(address, data); }

static I8237_INTERFACE( dmac_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(mm1_state, dma_hrq_changed),
	DEVCB_DRIVER_LINE_MEMBER(mm1_state, tc_w),
	DEVCB_MEMORY_HANDLER(I8085A_TAG, PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER(I8085A_TAG, PROGRAM, memory_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_MEMBER(mm1_state, mpsc_dack_r), DEVCB_DEVICE_HANDLER(UPD765_TAG, upd765_dack_r) },
	{ DEVCB_DEVICE_HANDLER(I8275_TAG, i8275_dack_w), DEVCB_DRIVER_MEMBER(mm1_state, mpsc_dack_w), DEVCB_NULL, DEVCB_DEVICE_HANDLER(UPD765_TAG, upd765_dack_w) },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_LINE_MEMBER(mm1_state, dack3_w) }
};


//-------------------------------------------------
//  pit8253_config pit_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( mm1_state::itxc_w )
{
	if (!m_intc)
	{
		m_mpsc->txca_w(state);
	}
}

WRITE_LINE_MEMBER( mm1_state::irxc_w )
{
	if (!m_intc)
	{
		m_mpsc->rxca_w(state);
	}
}

WRITE_LINE_MEMBER( mm1_state::auxc_w )
{
	m_mpsc->txcb_w(state);
	m_mpsc->rxcb_w(state);
}

static const struct pit8253_config pit_intf =
{
	{
		{
			XTAL_6_144MHz/2/2,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(mm1_state, itxc_w)
		}, {
			XTAL_6_144MHz/2/2,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(mm1_state, irxc_w)
		}, {
			XTAL_6_144MHz/2/2,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(mm1_state, auxc_w)
		}
	}
};


//-------------------------------------------------
//  UPD7201_INTERFACE( mpsc_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( mm1_state::drq2_w )
{
	if (state)
	{
		m_dmac->dreq2_w(ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( mm1_state::drq1_w )
{
	if (state)
	{
		m_dmac->dreq1_w(ASSERT_LINE);
	}
}

static UPD7201_INTERFACE( mpsc_intf )
{
	DEVCB_NULL,					// interrupt
	{
		{
			0,					// receive clock
			0,					// transmit clock
			DEVCB_DRIVER_LINE_MEMBER(mm1_state, drq2_w),	// receive DRQ
			DEVCB_DRIVER_LINE_MEMBER(mm1_state, drq1_w),	// transmit DRQ
			DEVCB_NULL,			// receive data
			DEVCB_NULL,			// transmit data
			DEVCB_NULL,			// clear to send
			DEVCB_NULL,			// data carrier detect
			DEVCB_NULL,			// ready to send
			DEVCB_NULL,			// data terminal ready
			DEVCB_NULL,			// wait
			DEVCB_NULL			// sync output
		}, {
			0,					// receive clock
			0,					// transmit clock
			DEVCB_NULL,			// receive DRQ
			DEVCB_NULL,			// transmit DRQ
			DEVCB_NULL,			// receive data
			DEVCB_NULL,			// transmit data
			DEVCB_NULL,			// clear to send
			DEVCB_LINE_GND,		// data carrier detect
			DEVCB_NULL,			// ready to send
			DEVCB_NULL,			// data terminal ready
			DEVCB_NULL,			// wait
			DEVCB_NULL			// sync output
		}
	}
};


//-------------------------------------------------
//  I8085_CONFIG( i8085_intf )
//-------------------------------------------------

READ_LINE_MEMBER( mm1_state::dsra_r )
{
	return 1;
}

static I8085_CONFIG( i8085_intf )
{
	DEVCB_NULL,			// STATUS changed callback
	DEVCB_NULL,			// INTE changed callback
	DEVCB_DRIVER_LINE_MEMBER(mm1_state, dsra_r),	// SID changed callback (I8085A only)
	DEVCB_DEVICE_LINE(SPEAKER_TAG, speaker_level_w)	// SOD changed callback (I8085A only)
};


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( mm1 )
	LEGACY_FLOPPY_OPTION( mm1_640kb, "dsk", "Nokia MikroMikko 1 640KB disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([8]) // 3:1 sector skew (1,4,7,2,5,8,3,6)
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static LEGACY_FLOPPY_OPTIONS_START( mm2 )
	LEGACY_FLOPPY_OPTION( mm2_360kb, "dsk", "Nokia MikroMikko 2 360KB disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([40])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))

	LEGACY_FLOPPY_OPTION( mm2_720kb, "dsk", "Nokia MikroMikko 2 720KB disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([40])
		SECTORS([18])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface mm1_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSQD,
	LEGACY_FLOPPY_OPTIONS_NAME(mm1),
	"floppy_5_25",
	NULL
};

static const upd765_interface fdc_intf =
{
	DEVCB_CPU_INPUT_LINE(I8085A_TAG, I8085_RST55_LINE),
	DEVCB_DEVICE_LINE_MEMBER(I8237_TAG, am9517a_device, dreq3_w),
	NULL,
	UPD765_RDY_PIN_NOT_CONNECTED,
	{ FLOPPY_0, FLOPPY_1, NULL, NULL }
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( mm1 )
//-------------------------------------------------

void mm1_state::machine_start()
{
	// find memory regions
	m_mmu_rom = memregion("address")->base();
	m_key_rom = memregion("keyboard")->base();

	// register for state saving
	save_item(NAME(m_sense));
	save_item(NAME(m_drive));
	save_item(NAME(m_llen));
	save_item(NAME(m_intc));
	save_item(NAME(m_rx21));
	save_item(NAME(m_tx21));
	save_item(NAME(m_rcl));
	save_item(NAME(m_recall));
	save_item(NAME(m_dack3));
}


//-------------------------------------------------
//  MACHINE_RESET( mm1 )
//-------------------------------------------------

void mm1_state::machine_reset()
{
	address_space *program = m_maincpu->space(AS_PROGRAM);
	int i;

	// reset LS259
	for (i = 0; i < 8; i++) ls259_w(*program, i, 0);

	// set FDC ready
	if (!ioport("T5")->read()) upd765_ready_w(m_fdc, 1);

	// reset FDC
	upd765_reset_w(m_fdc, 1);
	upd765_reset_w(m_fdc, 0);
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( mm1 )
//-------------------------------------------------

static MACHINE_CONFIG_START( mm1, mm1_state )
	// basic system hardware
	MCFG_CPU_ADD(I8085A_TAG, I8085A, XTAL_6_144MHz)
	MCFG_CPU_PROGRAM_MAP(mm1_map)
	MCFG_CPU_CONFIG(i8085_intf)

	MCFG_TIMER_ADD_PERIODIC("kbclk", kbclk_tick, attotime::from_hz(2500)) //attotime::from_hz(XTAL_6_144MHz/2/8))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// peripheral hardware
	MCFG_I8212_ADD(I8212_TAG, iop_intf)
	MCFG_I8237_ADD(I8237_TAG, XTAL_6_144MHz/2, dmac_intf)
	MCFG_PIT8253_ADD(I8253_TAG, pit_intf)
	MCFG_UPD765A_ADD(UPD765_TAG, /* XTAL_16MHz/2/2 */ fdc_intf)
	MCFG_UPD7201_ADD(UPD7201_TAG, XTAL_6_144MHz/2, mpsc_intf)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(mm1_floppy_interface)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( mm1m6 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( mm1m6, mm1 )
	// video hardware
	MCFG_FRAGMENT_ADD(mm1m6_video)
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( mm1m6 )
//-------------------------------------------------

ROM_START( mm1m6 )
	ROM_REGION( 0x4000, I8085A_TAG, 0 ) // BIOS
	ROM_LOAD( "9081b.ic2", 0x0000, 0x2000, CRC(2955feb3) SHA1(946a6b0b8fb898be3f480c04da33d7aaa781152b) )

	ROM_REGION( 0x200, "address", 0 ) // address decoder
	ROM_LOAD( "720793a.ic24", 0x0000, 0x0200, CRC(deea87a6) SHA1(8f19e43252c9a0b1befd02fc9d34fe1437477f3a) )

	ROM_REGION( 0x200, "keyboard", 0 ) // keyboard encoder
	ROM_LOAD( "mmi6349-1j.bin", 0x0000, 0x0200, CRC(4ab3bf03) SHA1(925c9ee22db13566416cdbc505c03d4116ff8d5f) )

	ROM_REGION( 0x1000, "chargen", 0 ) // character generator
	ROM_LOAD( "6807b.ic61", 0x0000, 0x1000, CRC(32b36220) SHA1(8fe7a181badea3f7e656dfaea21ee9e4c9baf0f1) )
ROM_END


//-------------------------------------------------
//  ROM( mm1m7 )
//-------------------------------------------------

#define rom_mm1m7 rom_mm1m6



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT    COMPANY           FULLNAME                FLAGS
COMP( 1981, mm1m6,		0,		0,		mm1m6,		mm1, driver_device,		0,		"Nokia Data",		"MikroMikko 1 M6",		GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND)
COMP( 1981, mm1m7,		mm1m6,	0,		mm1m6,		mm1, driver_device,		0,		"Nokia Data",		"MikroMikko 1 M7",		GAME_NOT_WORKING)
