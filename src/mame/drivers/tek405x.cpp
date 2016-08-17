// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Tektronix 4051/4052A

    Skeleton driver.

    http://www.electronixandmore.com/articles/teksystem.html

****************************************************************************/

/*

    TODO:

    - bank switch
    - keyboard
    - video (persistent vector display)
    - joystick
    - magnetic tape storage (3M 300)
    - communications backpack
    - 4051E01 ROM expander

*/


#include "includes/tek405x.h"
#include "softlist.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum {
	LBS_RBC = 0,    // ROM Bank Control
	LBS_BSOFL,      // Bank Switch Overflow (Select Overflow ROMs)
	LBS_BSCOM,      // Bank Switch Communication (Select Communication ROMs)
	LBS_INVALID,
	LBS_BS_L,       // Bank Switch Left (Select Left ROM Pack)
	LBS_BS_R,       // Bank Switch Right (Select Right ROM Pack)
	LBS_BSX_L,      // Bank Switch Expander (Select Left ROM Expander Unit)
	LBS_BSX_R       // Bank Switch Expander (Select Right ROM Expander Unit)
};



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  update_irq -
//-------------------------------------------------

void tek4051_state::update_irq()
{
	int state = m_kb_pia_irqa | m_kb_pia_irqb | m_x_pia_irqa | m_x_pia_irqb | m_gpib_pia_irqa | m_gpib_pia_irqb | m_com_pia_irqa | m_com_pia_irqb | m_acia_irq;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}


//-------------------------------------------------
//  update_nmi -
//-------------------------------------------------

void tek4051_state::update_nmi()
{
	int state = m_y_pia_irqa | m_y_pia_irqb | m_tape_pia_irqa | m_tape_pia_irqb;

	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}



//**************************************************************************
//  KEYBOARD
//**************************************************************************

//-------------------------------------------------
//  scan_keyboard - scan keyboard
//-------------------------------------------------

void tek4051_state::scan_keyboard()
{
}


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( keyboard_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(tek4051_state::keyboard_tick)
{
	scan_keyboard();
}



//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void tek4051_state::bankswitch(UINT8 data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	//int d = data & 0x07;
	int lbs = (data >> 3) & 0x07;

	switch (lbs)
	{
	case LBS_RBC:
		program.install_rom(0x8800, 0xa7ff, m_rom->base() + 0x800);
		break;

	case LBS_BSOFL:
		program.install_rom(0x8800, 0xa7ff, m_bsofl_rom->base());
		break;

	case LBS_BSCOM:
		program.install_rom(0x8800, 0xa7ff, m_bscom_rom->base());
		break;

	default:
		program.unmap_readwrite(0x8800, 0xa7ff);
	}
}


WRITE8_MEMBER( tek4051_state::lbs_w )
{
	/*

	    bit     description

	    0       ROM Expander Slot Address
	    1       ROM Expander Slot Address
	    2       ROM Expander Slot Address
	    3       Bank Switch
	    4       Bank Switch
	    5       Bank Switch
	    6
	    7

	*/

	logerror("LBS %02x\n", data);

	bankswitch(data);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( tek4051_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( tek4051_mem, AS_PROGRAM, 8, tek4051_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x7fff) AM_RAM // optional RAM
	AM_RANGE(0x8000, 0x877f) AM_ROM AM_REGION(MC6800_TAG, 0)
	AM_RANGE(0x878c, 0x878f) AM_DEVREADWRITE(MC6820_Y_TAG, pia6821_device, read, write)
	AM_RANGE(0x8794, 0x8797) AM_DEVREADWRITE(MC6820_X_TAG, pia6821_device, read, write)
	AM_RANGE(0x8798, 0x879b) AM_DEVREADWRITE(MC6820_TAPE_TAG, pia6821_device, read, write)
	AM_RANGE(0x87a8, 0x87ab) AM_DEVREADWRITE(MC6820_KB_TAG, pia6821_device, read, write)
	AM_RANGE(0x87b0, 0x87b3) AM_DEVREADWRITE(MC6820_GPIB_TAG, pia6821_device, read, write)
	AM_RANGE(0x87c0, 0x87c0) AM_MIRROR(0x03) AM_WRITE(lbs_w)
//  AM_RANGE(0x87c0, 0x87c3) AM_DEVREADWRITE(MC6820_COM_TAG, pia6821_device, read, write)
//  AM_RANGE(0x87c4, 0x87c4) AM_MIRROR(0x02) AM_DEVREADWRITE(MC6850_TAG, acia6850_device, status_r, control_w)
//  AM_RANGE(0x87c5, 0x87c5) AM_MIRROR(0x02) AM_DEVREADWRITE(MC6850_TAG, acia6850_device, data_r, data_w)
//  AM_RANGE(0x87c8, 0x87cb) XPC2
//  AM_RANGE(0x87cc, 0x87cf) XPC3
//  AM_RANGE(0x87d0, 0x87d3) XPC4
	AM_RANGE(0x8800, 0xa7ff) AM_ROM AM_REGION(MC6800_TAG, 0x800)
	AM_RANGE(0xa800, 0xffff) AM_ROM AM_REGION(MC6800_TAG, 0x2800)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( tek4052_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( tek4052_mem, AS_PROGRAM, 8, tek4052_state )
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( tek4051 )
//-------------------------------------------------

static INPUT_PORTS_START( tek4051 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Right SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TTY LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

void tek4051_state::video_start()
{
}


void tek4052_state::video_start()
{
}



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

READ8_MEMBER( tek4051_state::x_pia_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2     DRBUSY-0
	    PA3     VPULSE-1
	    PA4
	    PA5
	    PA6
	    PA7

	*/

	return 0;
}

WRITE8_MEMBER( tek4051_state::x_pia_pa_w )
{
	/*

	    bit     description

	    PA0     X D/A
	    PA1     X D/A
	    PA2
	    PA3
	    PA4     ERASE-0
	    PA5     COPY-0
	    PA6     VECTOR-0
	    PA7     VEN-1

	*/
}

WRITE8_MEMBER( tek4051_state::x_pia_pb_w )
{
	/*

	    bit     description

	    PB0     X D/A
	    PB1     X D/A
	    PB2     X D/A
	    PB3     X D/A
	    PB4     X D/A
	    PB5     X D/A
	    PB6     X D/A
	    PB7     X D/A

	*/
}

WRITE_LINE_MEMBER( tek4051_state::adot_w )
{
}

WRITE_LINE_MEMBER( tek4051_state::bufclk_w )
{
}

WRITE_LINE_MEMBER( tek4051_state::x_pia_irqa_w )
{
	m_x_pia_irqa = state;
	update_irq();
}

WRITE_LINE_MEMBER( tek4051_state::x_pia_irqb_w )
{
	m_x_pia_irqb = state;
	update_irq();
}

READ8_MEMBER( tek4051_state::sa_r )
{
	/*

	    bit     description

	    PA0     SA0
	    PA1     SA1
	    PA2     SA2
	    PA3     SA3
	    PA4     SA4
	    PA5     SA5
	    PA6     SA6
	    PA7     SA7

	*/

	return 0;
}

WRITE8_MEMBER( tek4051_state::y_pia_pa_w )
{
	/*

	    bit     description

	    PA0     Y D/A
	    PA1     Y D/A
	    PA2     Y CHAR D/A
	    PA3     Y CHAR D/A
	    PA4     Y CHAR D/A
	    PA5     X CHAR D/A
	    PA6     X CHAR D/A
	    PA7     X CHAR D/A

	*/
}

WRITE8_MEMBER( tek4051_state::sb_w )
{
	/*

	    bit     description

	    PB0     SB0
	    PB1     SB1
	    PB2     SB2
	    PB3     SB3
	    PB4     SB4
	    PB5     SB5
	    PB6     SB6
	    PB7     SB7

	*/
}

WRITE_LINE_MEMBER( tek4051_state::sot_w )
{
}

WRITE_LINE_MEMBER( tek4051_state::y_pia_irqa_w )
{
	m_y_pia_irqa = state;
	update_nmi();
}

WRITE_LINE_MEMBER( tek4051_state::y_pia_irqb_w )
{
	m_y_pia_irqb = state;
	update_nmi();
}


READ8_MEMBER( tek4051_state::kb_pia_pa_r )
{
	/*

	    bit     description

	    PA0     KC0-1
	    PA1     KC1-1
	    PA2     KC2-1
	    PA3     KC3-1
	    PA4     KC4-1
	    PA5     KC5-1
	    PA6     KC6-1
	    PA7     TTY-0

	*/

	UINT8 data = 0;
	UINT8 special = m_special->read();

	// keyboard column
	data = m_kc;

	// TTY lock
	data |= BIT(special, 3) << 7;

	return data;
}

READ8_MEMBER( tek4051_state::kb_pia_pb_r )
{
	/*

	    bit     description

	    PB0     SHIFT-0
	    PB1     CTRL-0
	    PB2
	    PB3     LOAD-0
	    PB4
	    PB5
	    PB6
	    PB7

	*/

	UINT8 data = 0;
	UINT8 special = m_special->read();

	// shift
	data |= (BIT(special, 0) & BIT(special, 1));

	// ctrl
	data |= BIT(special, 2) << 1;

	return data;
}

WRITE8_MEMBER( tek4051_state::kb_pia_pb_w )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3
	    PB4     EOI-1
	    PB5     BREAK-1
	    PB6     I/O-1
	    PB7     BUSY-1/REN-0/SPEAKER

	*/

	// lamps
	output().set_led_value(1, !BIT(data, 5));
	output().set_led_value(2, !BIT(data, 6));
	output().set_led_value(3, !BIT(data, 7));

	// end or identify
	m_gpib->eoi_w(!BIT(data, 4));

	// speaker
	m_speaker->level_w(!BIT(data, 7));

	// remote enable
	m_gpib->ren_w(!BIT(data, 7));
}

WRITE_LINE_MEMBER( tek4051_state::kb_halt_w )
{
}

WRITE_LINE_MEMBER( tek4051_state::kb_pia_irqa_w )
{
	m_kb_pia_irqa = state;
	update_irq();
}

WRITE_LINE_MEMBER( tek4051_state::kb_pia_irqb_w )
{
	m_kb_pia_irqb = state;
	update_irq();
}


READ8_MEMBER( tek4051_state::tape_pia_pa_r )
{
	/*

	    bit     description

	    PA0     DELAY OUT-1
	    PA1
	    PA2     TUTS-1
	    PA3     SAFE-1
	    PA4     SAFE-1
	    PA5     JOYSTICK
	    PA6     FILFND-1
	    PA7     JOYSTICK

	*/

	return 0;
}

WRITE8_MEMBER( tek4051_state::tape_pia_pa_w )
{
	/*

	    bit     description

	    PA0
	    PA1     LDCLK-1
	    PA2
	    PA3
	    PA4
	    PA5     XERR-1
	    PA6
	    PA7     YERR-1

	*/
}

WRITE8_MEMBER( tek4051_state::tape_pia_pb_w )
{
	/*

	    bit     description

	    PB0     WR-0
	    PB1     FAST-1
	    PB2     REV-1
	    PB3     DRTAPE-0
	    PB4     GICG-1
	    PB5     FICG-1
	    PB6     WENABLE-1
	    PB7     FSENABLE-0

	*/
}

WRITE_LINE_MEMBER( tek4051_state::tape_pia_irqa_w )
{
	m_tape_pia_irqa = state;
	update_nmi();
}

WRITE_LINE_MEMBER( tek4051_state::tape_pia_irqb_w )
{
	m_tape_pia_irqb = state;
	update_nmi();
}


WRITE8_MEMBER( tek4051_state::dio_w )
{
	/*

	    bit     description

	    PA0     DIO1-1
	    PA1     DIO2-1
	    PA2     DIO3-1
	    PA3     DIO4-1
	    PA4     DIO5-1
	    PA5     DIO6-1
	    PA6     DIO7-1
	    PA7     DIO8-1

	*/

	if (m_talk)
	{
		m_gpib->dio_w(data);
	}
}

READ8_MEMBER( tek4051_state::gpib_pia_pb_r )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3
	    PB4     NRFD
	    PB5     SRQ-1
	    PB6     DAV-1
	    PB7     NDAC

	*/

	UINT8 data = 0;

	// service request
	data |= m_gpib->srq_r() << 5;

	// data valid
	data |= m_gpib->dav_r() << 6;

	if (!m_talk)
	{
		// not ready for data
		data |= m_gpib->nrfd_r() << 4;

		// not data acknowledged
		data |= m_gpib->ndac_r() << 7;
	}

	return data;
}

WRITE8_MEMBER( tek4051_state::gpib_pia_pb_w )
{
	/*

	    bit     description

	    PB0     EOI-1
	    PB1     IFC-1
	    PB2
	    PB3     ATN-0
	    PB4     NRFD
	    PB5
	    PB6
	    PB7     NDAC

	*/

	// end or identify
	m_gpib->eoi_w(!BIT(data, 0));

	// interface clear
	m_gpib->ifc_w(!BIT(data, 1));

	// attention
	m_gpib->atn_w(BIT(data, 3));

	if (m_talk)
	{
		// not ready for data
		m_gpib->nrfd_w(!BIT(data, 4));

		// not data acknowledged
		m_gpib->ndac_w(!BIT(data, 7));
	}
}

WRITE_LINE_MEMBER( tek4051_state::talk_w )
{
	m_talk = state;

	if (!m_talk)
	{
		m_gpib->dio_w(0xff);
		m_gpib->nrfd_w(1);
		m_gpib->ndac_w(1);
	}
}

WRITE_LINE_MEMBER( tek4051_state::gpib_pia_irqa_w )
{
	m_gpib_pia_irqa = state;
	update_irq();
}

WRITE_LINE_MEMBER( tek4051_state::gpib_pia_irqb_w )
{
	m_gpib_pia_irqb = state;
	update_irq();
}


WRITE8_MEMBER( tek4051_state::com_pia_pa_w )
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2
	    PA3     Bank Switch Register
	    PA4     Bank Switch Register
	    PA5     Bank Switch Register
	    PA6
	    PA7

	*/

	bankswitch(data);
}

READ8_MEMBER( tek4051_state::com_pia_pb_r )
{
	/*

	    bit     description

	    PB0     SRX
	    PB1     DTR
	    PB2     RTS
	    PB3
	    PB4
	    PB5
	    PB6
	    PB7

	*/

	UINT8 data = 0;

	// data terminal ready

	// request to send

	return data;
}

WRITE8_MEMBER( tek4051_state::com_pia_pb_w )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3     CTS
	    PB4     STXA-C
	    PB5     Baud Rate Select
	    PB6     Baud Rate Select
	    PB7     Baud Rate Select

	*/

	// clear to send

	// baud rate select
	int osc = BIT(data, 7) ? 28160 : 38400;
	int div = 1;

	switch ((data >> 5) & 0x03)
	{
	case 2: div = 4; break;
	case 3: div = 2; break;
	}

	m_acia_clock->set_unscaled_clock(osc);
	m_acia_clock->set_clock_scale((double) 1 / div);
}

WRITE_LINE_MEMBER( tek4051_state::com_pia_irqa_w )
{
	m_com_pia_irqa = state;
	update_irq();
}

WRITE_LINE_MEMBER( tek4051_state::com_pia_irqb_w )
{
	m_com_pia_irqb = state;
	update_irq();
}


WRITE_LINE_MEMBER( tek4051_state::acia_irq_w )
{
	m_acia_irq = state;
	update_irq();
}

WRITE_LINE_MEMBER( tek4051_state::write_acia_clock )
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( tek4051 )
//-------------------------------------------------

void tek4051_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// configure RAM
	switch (m_ram->size())
	{
	case 8*1024:
		program.unmap_readwrite(0x2000, 0x7fff);
		break;

	case 16*1024:
		program.unmap_readwrite(0x4000, 0x7fff);
		break;

	case 24*1024:
		program.unmap_readwrite(0x6000, 0x7fff);
		break;
	}

	// register for state saving
}


//-------------------------------------------------
//  MACHINE_START( tek4052 )
//-------------------------------------------------

void tek4052_state::machine_start()
{
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( tek4051 )
//-------------------------------------------------

static MACHINE_CONFIG_START( tek4051, tek4051_state )
	// basic machine hardware
	MCFG_CPU_ADD(MC6800_TAG, M6800, XTAL_12_5MHz/15)
	MCFG_CPU_PROGRAM_MAP(tek4051_mem)

	// video hardware
	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, VECTOR, rgb_t::green)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE(1024, 780)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 780-1)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", tek4051_state, keyboard_tick, attotime::from_hz(XTAL_12_5MHz/15/4))

	MCFG_DEVICE_ADD(MC6820_X_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(tek4051_state, x_pia_pa_r))
	// CB1 viewcause
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(tek4051_state, x_pia_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(tek4051_state, x_pia_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(tek4051_state, adot_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(tek4051_state, bufclk_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(tek4051_state, x_pia_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(tek4051_state, x_pia_irqb_w))

	MCFG_DEVICE_ADD(MC6820_Y_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(tek4051_state, sa_r))
	// CA1 rdbyte
	// CB1 mdata
	// CB2 fmark
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(tek4051_state, y_pia_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(tek4051_state, sb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(tek4051_state, sot_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(tek4051_state, y_pia_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(tek4051_state, y_pia_irqb_w))

	MCFG_DEVICE_ADD(MC6820_KB_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(tek4051_state, kb_pia_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(tek4051_state, kb_pia_pb_r))
	// CA1 key
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(tek4051_state, kb_pia_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(tek4051_state, kb_halt_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(tek4051_state, kb_pia_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(tek4051_state, kb_pia_irqb_w))

	MCFG_DEVICE_ADD(MC6820_TAPE_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(tek4051_state, tape_pia_pa_r))
	// CA1 rmark
	// CB1 lohole
	// CA2 filfnd
	// CB2 uphole
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(tek4051_state, tape_pia_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(tek4051_state, tape_pia_pb_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(tek4051_state, tape_pia_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(tek4051_state, tape_pia_irqb_w))

	MCFG_DEVICE_ADD(MC6820_GPIB_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(DEVREAD8(IEEE488_TAG, ieee488_device, dio_r))
	MCFG_PIA_READPB_HANDLER(READ8(tek4051_state, gpib_pia_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(tek4051_state, dio_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(tek4051_state, gpib_pia_pb_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(tek4051_state, talk_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(tek4051_state, gpib_pia_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(tek4051_state, gpib_pia_irqb_w))

	MCFG_DEVICE_ADD(MC6820_COM_TAG, PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(tek4051_state, com_pia_pb_r))
	//CA1 - SRX (RS-232 pin 12)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(tek4051_state, com_pia_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(tek4051_state, com_pia_pb_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(tek4051_state, com_pia_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(tek4051_state, com_pia_irqb_w))

	MCFG_DEVICE_ADD(MC6850_TAG, ACIA6850, 0)
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(tek4051_state, acia_irq_w))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 38400)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(tek4051_state, write_acia_clock))

	MCFG_IEEE488_BUS_ADD()
	MCFG_IEEE488_EOI_CALLBACK(DEVWRITELINE(MC6820_GPIB_TAG, pia6821_device, ca1_w))
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE(MC6820_GPIB_TAG, pia6821_device, cb1_w))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8K")
	MCFG_RAM_EXTRA_OPTIONS("16K,24K,32K")

	// cartridge
	MCFG_GENERIC_CARTSLOT_ADD("cartslot1", generic_plain_slot, "tek4050_cart")
	MCFG_GENERIC_CARTSLOT_ADD("cartslot2", generic_plain_slot, "tek4050_cart")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( tek4052 )
//-------------------------------------------------

static MACHINE_CONFIG_START( tek4052, tek4052_state )
	// basic machine hardware
	MCFG_CPU_ADD(AM2901A_TAG, M6800, 1000000) // should be 4x AM2901A + AM2911
	MCFG_CPU_PROGRAM_MAP(tek4052_mem)

	// video hardware
	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, VECTOR, rgb_t::green)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE(1024, 780)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 780-1)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("64K")

	// cartridge
	MCFG_GENERIC_CARTSLOT_ADD("cartslot1", generic_plain_slot, "tek4050_cart")
	MCFG_GENERIC_CARTSLOT_ADD("cartslot2", generic_plain_slot, "tek4050_cart")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list", "tek4052_cart")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( tek4054 )
//-------------------------------------------------
/*
static MACHINE_CONFIG_START( tek4054, tek4052_state )
    MCFG_SCREEN_SIZE(4096, 3125)
    MCFG_SCREEN_VISIBLE_AREA(0, 4096-1, 0, 3125-1)
MACHINE_CONFIG_END
*/



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( tek4051 )
//-------------------------------------------------

ROM_START( tek4051 )
	ROM_REGION( 0x8000, MC6800_TAG, 0 )
	ROM_LOAD( "156-0659-xx.u585", 0x0000, 0x0800, CRC(0017ba54) SHA1(533bfacb2e698c1df88a00acce6df6a8c536239d) ) // -01 or -02 ?
	ROM_LOAD( "156-0660-01.u581", 0x0800, 0x0800, CRC(c4e302e8) SHA1(a0438cbc70ddc1bab000e30c0f835f0948aae969) )
	ROM_LOAD( "156-0661-01.u487", 0x1000, 0x0800, CRC(edb097c8) SHA1(a07b777b6b20de496089730db8097b65b9a8ef31) )
	ROM_LOAD( "156-0662-01.u485", 0x1800, 0x0800, CRC(0a3af548) SHA1(01bcffa7d55b2585348ea96852b4319c6454d19a) )
	ROM_LOAD( "156-0663-01.u481", 0x2000, 0x0800, CRC(f220d24c) SHA1(fb5dc8a617f4b4d5c094c0cbabb9af158c19c2c2) )
	ROM_LOAD( "156-0664-xx.u385", 0x2800, 0x0800, CRC(aa1ae67d) SHA1(8816654c1b0ad0230bf5027f68c2ec1d315ad188) ) // -01 or -02 ?
	ROM_LOAD( "156-0665-xx.u381", 0x3000, 0x0800, CRC(90497138) SHA1(2d6bbe169a434581a49fffcab0d2ca18107b67da) ) // -01 or -02 ?
	ROM_LOAD( "156-0666-01.u285", 0x3800, 0x0800, CRC(c35ce405) SHA1(10cae122d000fe2886228a178a1a4e4d3ec2c3f5) )
	ROM_LOAD( "156-0667-xx.u595", 0x4000, 0x0800, CRC(e1723a06) SHA1(dde3e64eb6acc77d8e7234235a99ec338cc74717) ) // -01 or -02 ?
	ROM_LOAD( "156-0668-xx.u591", 0x4800, 0x0800, CRC(d8b80d7a) SHA1(6c1f00fa2f9ff6ebd84268c2100637ddbdb3bae6) ) // -01 or -02 ?
	ROM_LOAD( "156-0669-01.u497", 0x5000, 0x0800, CRC(f760ed39) SHA1(2b30d20d880002ce3950ee7abe40f4e1539208cc) )
	ROM_LOAD( "156-0670-01.u495", 0x5800, 0x0800, CRC(d40a303d) SHA1(111b399f6178e2fe50b1368536f290d4df9883fc) )
	ROM_LOAD( "156-0671-xx.u491", 0x6000, 0x0800, CRC(32060b64) SHA1(af254dc068686fcd7584128258724fd5415ea458) ) // -01 or -02 ?
	ROM_LOAD( "156-0672-xx.u395", 0x6800, 0x0800, CRC(93ad68d1) SHA1(8242058bfc89d11cd9ae82d967410714875eface) ) // -01 or -02 ?
	ROM_LOAD( "156-0673-xx.u391", 0x7000, 0x0800, CRC(ee1f5d4c) SHA1(a6fb8347a0dfa94268f091fb7ca091c8bfabeabd) ) // -01 or -02 or -03 ?
	ROM_LOAD( "156-0674-xx.u295", 0x7800, 0x0800, CRC(50582341) SHA1(1a028176e5d5ca83012e2f75e9daad88cd7d8fc3) ) // -01 or -02 ?

	ROM_REGION( 0x2000, "020_0147_00", 0 ) // Firmware Backpack (020-0147-00)
	ROM_LOAD( "156-0747-xx.u101", 0x0000, 0x0800, CRC(9e1facc1) SHA1(7e7a118c3e8c49630f630ee02c3de843dd95d7e1) ) // -00 or -01 ?
	ROM_LOAD( "156-0748-xx.u201", 0x0800, 0x0800, CRC(be42bfbf) SHA1(23575b411bd9dcb7d7116628820096e3064ff93b) ) // -00 or -01 ?

	ROM_REGION( 0x2000, "021_0188_00", 0 ) // Communications Backpack (021-0188-00)
	ROM_LOAD( "156-0712-00.u101", 0x0000, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0712-01.u101", 0x0000, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0713-00.u111", 0x0800, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0713-01.u111", 0x0800, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0714-00.u121", 0x1000, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0714-01.u121", 0x1000, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0715-01.u131", 0x1800, 0x0800, NO_DUMP )
/*
    ROM_REGION( 0x2000, "4051r01", 0 ) // 4051R01 Matrix Functions
    ROM_LOAD( "4051r01", 0x0000, 0x1000, NO_DUMP )

    ROM_REGION( 0x2000, "4051r05", 0 ) // 4051R05 Binary Program Loader
    ROM_LOAD( "156-0856-00.u1",  0x0000, 0x0800, NO_DUMP )
    ROM_LOAD( "156-0857-00.u11", 0x0800, 0x0800, NO_DUMP )

    ROM_REGION( 0x2000, "4051r06", 0 ) // 4051R06 Editor
    ROM_LOAD( "4051r06", 0x0000, 0x1000, NO_DUMP )
*/
ROM_END


//-------------------------------------------------
//  ROM( tek4052a )
//-------------------------------------------------

ROM_START( tek4052a )
	ROM_REGION( 0x3800, AM2901A_TAG, 0 ) // ALU 670-7705-00 microcode
	ROM_LOAD( "160-1689-00.u340", 0x0000, 0x0800, CRC(97ff62d4) SHA1(e25b495fd1b3f8a5bfef5c8f20efacde8366e89c) )
	ROM_LOAD( "160-1688-00.u335", 0x0800, 0x0800, CRC(19033422) SHA1(0f6ea45be5123701940331c4278bcdc5db4f4147) )
	ROM_LOAD( "160-1687-00.u330", 0x1000, 0x0800, CRC(3b2e37dd) SHA1(294b626d0022fbf9bec50680b6b932f21a3cb049) )
	ROM_LOAD( "160-1686-00.u320", 0x1800, 0x0800, CRC(7916cd64) SHA1(d6b3ed783e488667ca203ec605aa27d78261c61b) )
	ROM_LOAD( "160-1695-00.u315", 0x2000, 0x0800, CRC(cdac76a7) SHA1(2a47f34ae63fb7fe96e7c6fc92ca4f629b0e31ec) )
	ROM_LOAD( "160-1694-00.u305", 0x2800, 0x0800, CRC(c33ae212) SHA1(25ed4cc3600391fbc93bb46c92d3f64ca2aca58e) )
	ROM_LOAD( "160-1693-00.u300", 0x3000, 0x0800, CRC(651b7af2) SHA1(7255c682cf74f2e77661d77c8406b36a567eea46) )

	ROM_REGION( 0x14000, "672_0799_08", 0 ) // Memory Access Sequencer 672-0799-08
	ROM_LOAD16_BYTE( "160-1698-00.u810", 0x00000, 0x2000, CRC(fd0b8bc3) SHA1(ea9caa151295024267a467ba636ad41aa0c517d3) )
	ROM_LOAD16_BYTE( "160-1682-00.u893", 0x00001, 0x2000, CRC(d54104ef) SHA1(b796c320c8f96f6e4b845718bfb62ccb5903f2db) )
	ROM_LOAD16_BYTE( "160-1699-00.u820", 0x04000, 0x2000, CRC(59cc6c2e) SHA1(b6d85cdfaaef360af2a4fc44eae55e512bb08b21) )
	ROM_LOAD16_BYTE( "160-1684-00.u870", 0x04001, 0x2000, CRC(b24cdbaf) SHA1(476032624110bc5c7a7f30d10458e8baffb2f2ff) )
	ROM_LOAD16_BYTE( "160-1700-00.u825", 0x08000, 0x2000, CRC(59f4a4e2) SHA1(81b5f74ec14bb13735382abc123bb9b09dfb7a63) )
	ROM_LOAD16_BYTE( "160-1683-00.u880", 0x08001, 0x2000, CRC(f6bb15d1) SHA1(eca06211876c12236cfeae0417131e8f04728d51) )
	ROM_LOAD16_BYTE( "160-1701-00.u835", 0x0c000, 0x2000, CRC(195988b9) SHA1(995a77963e869f1a461addf58db9ecbf2b9222ba) )
	ROM_LOAD16_BYTE( "160-1691-00.u885", 0x0c001, 0x2000, CRC(0670bf9e) SHA1(a8c39bd7f3f61436296c6bc5f734354371ae8123) )
	ROM_LOAD16_BYTE( "160-1702-00.u845", 0x10000, 0x2000, CRC(013344b1) SHA1(4a79654427e15d0fcedd9519914f6448938ecffd) )
	ROM_LOAD16_BYTE( "160-1685-00.u863", 0x10001, 0x2000, CRC(53ddc8f9) SHA1(431d6f329dedebb54232c623a924d5ecddc5e44e) )

	ROM_REGION( 0x2000, "020_0147_00", 0 ) // Firmware Backpack (020-0147-00)
	ROM_LOAD( "156-0747-xx.u101", 0x0000, 0x0800, CRC(9e1facc1) SHA1(7e7a118c3e8c49630f630ee02c3de843dd95d7e1) ) // -00 or -01 ?
	ROM_LOAD( "156-0748-xx.u201", 0x0800, 0x0800, CRC(be42bfbf) SHA1(23575b411bd9dcb7d7116628820096e3064ff93b) ) // -00 or -01 ?

	ROM_REGION( 0x2000, "021_0188_00", 0 ) // Communications Backpack (021-0188-00)
	ROM_LOAD( "156-0712-00.u101", 0x0000, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0712-01.u101", 0x0000, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0713-00.u111", 0x0800, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0713-01.u111", 0x0800, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0714-00.u121", 0x1000, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0714-01.u121", 0x1000, 0x0800, NO_DUMP )
	ROM_LOAD( "156-0715-01.u131", 0x1800, 0x0800, NO_DUMP )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY         FULLNAME            FLAGS */
COMP( 1975, tek4051,    0,          0,      tek4051,    tek4051, driver_device, 0,      "Tektronix",    "Tektronix 4051",   MACHINE_NOT_WORKING )
COMP( 1978, tek4052a,   tek4051,    0,      tek4052,    tek4051, driver_device, 0,      "Tektronix",    "Tektronix 4052A",  MACHINE_NOT_WORKING )
//COMP( 1979, tek4054,  tek4051,    0,      tek4054,    tek4054, driver_device,    0,      "Tektronix",    "Tektronix 4054",   MACHINE_NOT_WORKING )
