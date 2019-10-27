// license:GPL-2.0+
// copyright-holders:Peter Trauner, Dan Boris, Dirk Best, Robbbert
/******************************************************************************

 AIM65

******************************************************************************/


#include "emu.h"
#include "includes/aim65.h"

//#define VERBOSE 1
#include "logmacro.h"


/******************************************************************************
 Interrupt handling
******************************************************************************/

/* STEP/RUN
 *
 * Switch S2 (STEP/RUN) causes AIM 65 to operate either in the RUN mode or the
 * single STEP mode. In the STEP mode, the NMI interrupt line is driven low
 * when SYNC and O2 go high during instruction execution if the address lines
 * are outside the A000-FFFF range. The NMI interrupt occurs on the high to
 * low transition of the NMI line. The Monitor software will trace instructions
 * and register, outside the Monitor instruction address range if the trace
 * modes are selected and the NMI Interrupt Routine is not bypassed.
 */



/******************************************************************************
 6821 PIA
******************************************************************************/

/* PA0: A0 (Address)
 * PA1: A1 (Address)
 * PA2: CE1 (Chip enable)
 * PA3: CE2 (Chip enable)
 * PA4: CE3 (Chip enable)
 * PA5: CE4 (Chip enable)
 * PA6: CE5 (Chip enable)
 * PA7: W (Write enable)
 * PB0-6: D0-D6 (Data)
 * PB7: CU (Cursor)
 */

void aim65_state::u1_pa_w( u8 data )
{
	LOG("pia a: a=%u /ce=%u,%u,%u,%u,%u /wr=%u\n",
			data & 0x03, BIT(data, 2), BIT(data, 3), BIT(data, 4), BIT(data, 5), BIT(data, 6), BIT(data, 7));
	for (std::size_t index = 0; m_ds.size() > index; ++index)
	{
		m_ds[index]->wr_w(BIT(data, 7));
		m_ds[index]->ce_w(BIT(data, 2 + index));
		m_ds[index]->addr_w(data & 0x03);
	}
}


void aim65_state::u1_pb_w( u8 data )
{
	LOG("pia b: d=%02x /cu=%u\n", data & 0x7f, BIT(data, 7));
	for (required_device<dl1416_device> &ds : m_ds)
	{
		ds->cu_w(BIT(data, 7));
		ds->data_w(data & 0x7f);
	}
}


template <unsigned D> WRITE16_MEMBER( aim65_state::update_ds )
{
	m_digits[((D - 1) << 2) | (offset ^ 3)] = data;
}

template WRITE16_MEMBER( aim65_state::update_ds<1> );
template WRITE16_MEMBER( aim65_state::update_ds<2> );
template WRITE16_MEMBER( aim65_state::update_ds<3> );
template WRITE16_MEMBER( aim65_state::update_ds<4> );
template WRITE16_MEMBER( aim65_state::update_ds<5> );


/******************************************************************************
 6532 RIOT
******************************************************************************/


u8 aim65_state::z33_pb_r()
{
	u8 data = 0xff;

	/* scan keyboard rows */
	for (u8 i = 0; i < 8; i++)
		if (!BIT(m_riot_port_a, i))
			data &= m_io_keyboard[i]->read();

	return data;
}


/***************************************************************************
    DRIVER INIT
***************************************************************************/

void aim65_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	m_digits.resolve();

	// Init ROM sockets
	if (m_z24->exists())
		space.install_read_handler(0xd000, 0xdfff, read8sm_delegate(*m_z24, FUNC(generic_slot_device::read_rom)));
	if (m_z25->exists())
		space.install_read_handler(0xc000, 0xcfff, read8sm_delegate(*m_z25, FUNC(generic_slot_device::read_rom)));
	if (m_z26->exists())
		space.install_read_handler(0xb000, 0xbfff, read8sm_delegate(*m_z26, FUNC(generic_slot_device::read_rom)));

	// Init PROM/ROM module sockets
	if (m_z12->exists())
		space.install_read_handler(0x4000, 0x4fff, read8sm_delegate(*m_z12, FUNC(generic_slot_device::read_rom)));
	if (m_z13->exists())
		space.install_read_handler(0x5000, 0x5fff, read8sm_delegate(*m_z13, FUNC(generic_slot_device::read_rom)));
	if (m_z14->exists())
		space.install_read_handler(0x6000, 0x6fff, read8sm_delegate(*m_z14, FUNC(generic_slot_device::read_rom)));
	if (m_z15->exists())
		space.install_read_handler(0x7000, 0x7fff, read8sm_delegate(*m_z15, FUNC(generic_slot_device::read_rom)));

	// Init RAM
	space.install_ram(0x0000, m_ram->size() - 1, m_ram->pointer());

	// Register save state
	save_item(NAME(m_riot_port_a));
	save_item(NAME(m_pb_save));
	save_item(NAME(m_kb_en));
	save_item(NAME(m_ca2));
	save_item(NAME(m_cb2));
	save_item(NAME(m_printer_x));
	save_item(NAME(m_printer_y));
	save_item(NAME(m_printer_flag));
	save_item(NAME(m_printer_level));
	m_print_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aim65_state::printer_timer),this));
	m_printerRAM = make_unique_clear<uint16_t[]>(64*100);
	save_pointer(NAME(m_printerRAM), 64*100);
}


/* RESET
 *
 * Pushbutton switch S1 initiates RESET of the AIM65 hardware and software.
 * Timer Z4 holds the RES low for at least 15 ms from the time the pushbutton
 * is released. RES is routed to the R6502 CPU, the Monitor R6522 (Z32), the
 * Monitor R6532 RIOT (Z33), the user R6522 VIA (Z1), and the display R6520 PIA
 * (U1). To initiate the device RESET function is also routed to the expansion
 * connector for off-board RESET functions. The Monitor performs a software
 * reset when the RES line goes high.
 */


/******************************************************************************
 Cassette
******************************************************************************/

void aim65_state::z32_pb_w( u8 data )
{
/*
    d7 = cass out (both decks)
    d5 = cass2 motor
    d4 = cass1 motor
    d2 = tty out
    d0/1 = printer data

    There seems to be a mistake in the code. When the cassettes should both be stopped, it turns them on instead.
*/

	if ((data & 0x30)==0x30)
		data &= ~0x30;    // fix bug with motors

	uint8_t bits = data ^ m_pb_save;
	m_pb_save = data;

	if (BIT(bits, 7) && !m_ca2)
	{
		if (BIT(data, 4))
			m_cassette1->output(BIT(data, 7) ? -1.0 : +1.0);
		if (BIT(data, 5))
			m_cassette2->output(BIT(data, 7) ? -1.0 : +1.0);
	}

	if (BIT(bits, 5))
			m_cassette2->change_state(BIT(data, 5) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (BIT(bits, 4))
			m_cassette1->change_state(BIT(data, 4) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (!m_kb_en)
		m_rs232->write_txd(BIT(data, 2));

	// 4 left-most characters for the thermal printer
	data &= 0x03;
	if ((m_printer_flag == 1) && (!m_cb2))
		m_printerRAM[(m_printer_y * 100) + m_printer_x ] |= (data << 8);
}


u8 aim65_state::z32_pb_r ()
{
/*
    d7 = cassette in
    d6 = tty in
    d3 = kb/tty switch
*/

	uint8_t data = ioport("switches")->read() & 8;
	m_kb_en = BIT(data, 3);

	if (m_ca2)
	{
		if (BIT(m_pb_save, 4))
			data |= (m_cassette1->input() > +0.03) ? 0x80 : 0;
		else
		if (BIT(m_pb_save, 5))
			data |= (m_cassette2->input() > +0.03) ? 0x80 : 0;
	}

	if (m_kb_en)
		data |= 0x40; // TTY must be H if not used.
	else
		data |= (m_rs232->rxd_r()) ? 0x40 : 0;

	data |= m_pb_save & 0x37;
	return data;
}


/******************************************************************************
 Printer
******************************************************************************/

/*
The system sends out the pattern of dots to the 'dot-matrix' print-head.

  aim65 thermal printer (20 characters)
  10 heat elements (place on 1 line, space between 2 characters(about 14dots))
  (pa0..pa7,pb0,pb1 1 heat element on)

  cb2 0 motor, heat elements on. Enables printer. Also indicates that a new line is coming.
  cb1 0 input - printer is requesting data, driven by "start" line
  ca1 ^ input - changes polarity on each line, driven by s1 and s2

  normally printer 5x7 characters
  (horizontal movement limits not known, normally 2 dots between characters)

  3 dots space between lines
*/

// to get printer to pass the test at start.
TIMER_CALLBACK_MEMBER(aim65_state::printer_timer)
{
		m_via0->write_cb1(m_printer_level);
		m_via0->write_ca1(m_printer_level);
		m_printer_level ^= 1;
}

// CB2 - enable, & new-line
void aim65_state::z32_cb2_w( bool state )
{
	m_cb2 = state;
	if (!state)
	{
		m_printer_x = 0;
		m_printer_y++;
		m_printer_y &= 63;
		for (u8 i = 0; i < 100; i++)
			m_printerRAM[m_printer_y*100+i] = 0;
		m_printer_flag = 0;
		m_via0->write_cb1(1);
		m_print_timer->adjust(attotime::zero, 0, attotime::from_msec(1));
		m_printer_level = 1;
	}
	else
		m_print_timer->reset();
}

// new pixels spread among multiple thermal heads
void aim65_state::z32_pa_w( u8 data )
{
	if (m_printer_flag == 1)
		m_printerRAM[m_printer_y * 100 + m_printer_x] |= data;

	m_printer_flag++;
	if (m_printer_flag > 2)
	{
		m_printer_flag = 0;
		m_printer_x++;
	}
	// toggle ca1 each 10 bytes
	if ((m_printer_x % 10) == 0)
	{
		m_via0->write_ca1(m_printer_level);
		m_printer_level ^= 1;
	}
	// take cb1 low after a complete line of characters
	if (m_printer_x == 100)
	{
		m_printer_x = 0;
		m_via0->write_cb1(0);
	}
}


// Display printer output
uint32_t aim65_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t data;

	for (u8 y = 0; y<cliprect.max_y; y++)
	{
		u16 sy = m_printer_y*10 - cliprect.max_y + 10 + y;
		if (sy > 0xfe80) // wrap-around correctly
			sy += 0x280;
		for(u8 x = 0; x < 10; x++)
		{
			if (!BIT(sy, 0))
				data = m_printerRAM[sy * 10 + x];
			else
				data = m_printerRAM[sy * 10 + (9 - x)];

			for (u8 b = 0; b < 10; b++)
				bitmap.pix16(y, 120 - (b * 12) - ((x > 4) ? x+1 : x)) = BIT(data, b);
		}
	}

	return 0;
}

