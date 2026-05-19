// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Thermal Printer
    Model PHP1900

    Typical sidecar setup:
                             +-- Thermal Printer
                             v
     +----------------+---+------+----------
     |   TI-99/4(A)   |   |      |    (PEB connection cable)
     +------------+---+   |+----+|----------
     | oooooooooo |   |---||    ||
     | oooooooooo |   | ^ |+----+|
     +----------------+ | |    o |
                        | +------+
                        |
              (Speech Synthesizer)

    The Thermal Printer is a sidecar device for the TI-99/4(A) console. It
    enabled the user to create printouts of BASIC programs via the LIST "TP"
    command, and to create other text-based output using OPEN / PRINT in
    BASIC or the usual DSR calls in assembly language.

    Speed is limited from its technology, and special thermal paper is needed.
    Beside the standard ASCII set, user-defined patterns may be printed, but
    there will always be some empty pixel colums due to the construction of
    the print heads.

    Some games like "A-Maze-Ing" have a built-in screen dump for the
    Thermal Printer (press FCTN-P when the game is over).

    The button on the printer chassis is used to advance the paper.

    Technology:

    The Thermal Printer uses two fixed print heads with 16*5 dots each.

    |***** ***** ***** ... *****|***** ***** ***** ... *****|
     01234 01234 01234     01234 56789 56789 56789     56789   dot number
       15    14    13        0     15    14    13        0     char column number

    |------------------- width of paper --------------------|

    Columns are powered one after another; the selected dots cause the heating
    element to get hot at that location. It is possible to activate multiple
    columns at once, if the selected dot is supposed to be active in all these
    columns.

    Each character column is surrounded by one blank pixel column before and
    one blank pixel column after it, so we get 7 dots width per character
    column.

    The burning is done while the retriggered monoflop LS122 is active. The
    monoflop is a safety precaution to avoid burning the paper during a paper
    jam or a crash of the printer driver.

    Pressing the paper feed button causes an interrupt which runs the paper
    advance routine in the built-in driver. By default, this is bound to the
    END key on the keyboard.

    Michael Zapf
    May 2026

*****************************************************************************/

#include "emu.h"
#include "thermal.h"
#include "machine/rescap.h"

#define LOG_WARN          (1U << 1)   // Warnings
#define LOG_INTERRUPT     (1U << 2)
#define LOG_PRINT         (1U << 3)
#define LOG_CRU           (1U << 4)

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_THERMAL, bus::ti99::sidecar::ti_thermal_printer_device, "ti99_thermal", "TI-99 Thermal Printer")

namespace bus::ti99::sidecar {

#define PORT "extport"
#define BITMAPPR "bitmap_print"

#define TI99_DSRROM "dsr"

#define LATCH_U4 "latch_u4"
#define LATCH_U5 "latch_u5"
#define LATCH_U6 "latch_u6"
#define LATCH_U7 "latch_u7"
#define HEATMF_U8 "heatmf_u8"

static constexpr unsigned LEFTPAD = 12;
static constexpr unsigned PAPER_WIDTH = 242;
static constexpr unsigned PAPER_HEIGHT = (11*72);

/*
    Constructor called from subclasses.
*/
ti_thermal_printer_device::ti_thermal_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	:   bus::ti99::internal::ioport_attached_device(mconfig, TI99_THERMAL, tag, owner, clock),
	m_bitmap_printer(*this, BITMAPPR),
	m_port(*this, PORT),
	m_latch_u4(*this, LATCH_U4),
	m_latch_u5(*this, LATCH_U5),
	m_latch_u6(*this, LATCH_U6),
	m_latch_u7(*this, LATCH_U7),
	m_heatmf_u8(*this, HEATMF_U8)
{
}

void ti_thermal_printer_device::write_line()
{
	// Get the active columns and dots from the latches
	int actcolumns = (m_latch_u6->output_state() << 8) | m_latch_u7->output_state();
	int actdots    = (m_latch_u4->output_state() << 2) | ((m_latch_u5->output_state() & 0xc0) >> 6);

	LOGMASKED(LOG_PRINT, "Printing col=%04x dots=%02x\n", actcolumns, actdots);
	for (int col = 0; col < 16; col++)  // multiple columns can be energized
	{
		if (BIT(actcolumns, col))
		{
			for (int dot = 0; dot < 10; dot++)
			{
				if (BIT(actdots, dot))
				{
					int physcol = (dot >= 5)? (31 - col) : (15 - col);
					m_bitmap_printer->draw_pixel(m_bitmap_printer->m_xpos
						+ 7*physcol + (dot%5) + LEFTPAD,
						m_bitmap_printer->m_ypos,
						0x000000);  //rrggbb
				}
			}
		}
	}
}

/*
    The heat monoflop is a safety feature in case of a paper jam.
    It turns off the current of the write heads after 15 ms when not retriggered.
*/
void ti_thermal_printer_device::enable_column_latches(int state)
{
	m_latch_u7->clear_w(state);
	m_latch_u6->clear_w(state);
}

/*
    Memory read
*/

void ti_thermal_printer_device::readz(offs_t offset, uint8_t *value)
{
	if (((offset & 0xe000) == 0x4000) && m_latch_u5->q0_r())
	{
		*value = m_dsrrom[offset & 0x0fff];
	}

	// Pass through to the external port
	if (m_port != nullptr)
		m_port->readz(offset, value);
}

/*
    Memory write
*/
void ti_thermal_printer_device::write(offs_t offset, uint8_t data)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->write(offset, data);
}

void ti_thermal_printer_device::setaddress_dbin(offs_t offset, int state)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->setaddress_dbin(offset, state);
}

void ti_thermal_printer_device::crureadz(offs_t offset, uint8_t *value)
{
	if ((offset & 0xff00) == 0x1800)
	{
		// Only delivers a 0 when pressed, else Z
		if (m_feed_pressed) *value = 0;
	}

	// Pass through to the external port
	if (m_port != nullptr)
		m_port->crureadz(offset, value);
}

void ti_thermal_printer_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00) == 0x1800)
	{
		LOGMASKED(LOG_CRU, "CRU address = %04x, data = %x\n", offset, data);
		// LOGMASKED(LOG_CRU, "addr %d, bit %d = %d\n", (offset & 0xf0)>>4, (offset & 0x0e)>>1, data);

		int bitaddr = (offset & 0x0e)>>1;
		switch (offset & 0xf0)
		{
		case 0x00:
			m_latch_u5->write_bit(bitaddr, data);
			if (bitaddr == 5) line_feed();
			break;
		case 0x10:
			m_latch_u4->write_bit(bitaddr, data);
			break;
		case 0x20:
			m_latch_u7->write_bit(bitaddr, data);
			break;
		case 0x30:
			m_latch_u6->write_bit(bitaddr, data);
			if (bitaddr == 7) write_line();
			break;
		default:
			break;
		}
	}
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->cruwrite(offset, data);
}

void ti_thermal_printer_device::line_feed()
{
	// Run the stepper
	int pattern = bitswap<4>(m_latch_u5->output_state()>>2, 0, 1, 2, 3);
	m_bitmap_printer->update_pf_stepper(pattern);
	m_bitmap_printer->update_cr_stepper(1);  // have to move the cr stepper to get the page to change
	m_bitmap_printer->update_cr_stepper(5);
	m_bitmap_printer->update_cr_stepper(1);
}

/*
    Just forwarding to the external port
*/
void ti_thermal_printer_device::memen_in(int state)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->memen_in(state);
}

void ti_thermal_printer_device::msast_in(int state)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->msast_in(state);
}

void ti_thermal_printer_device::clock_in(int state)
{
	// Pass through to the external port
	if (m_port != nullptr)
		m_port->clock_in(state);
}

void ti_thermal_printer_device::reset_in(int state)
{
	clear_latches(state);

	// Pass through to the external port
	if (m_port != nullptr)
		m_port->reset_in(state);
}

/*
    Forward external interrupts to the console.
*/
void ti_thermal_printer_device::extint(int state)
{
	LOGMASKED(LOG_INTERRUPT, "Incoming EXTINT=%d from external port\n", state);
	m_ext_int = (line_state)state;
	set_extint(((m_ext_int == ASSERT_LINE) || m_feed_pressed)? ASSERT_LINE : CLEAR_LINE);
}

/*
    Forward the incoming READY to the console
*/
void ti_thermal_printer_device::extready(int state)
{
	set_ready(state);
}

void ti_thermal_printer_device::clear_latches(int state)
{
	int value = (state==ASSERT_LINE)? 0 : 1;
	m_latch_u5->clear_w(value);
	m_latch_u4->clear_w(value);
	m_heatmf_u8->clear_w(value);
}

void ti_thermal_printer_device::device_start()
{
	m_bitmap_printer->set_pf_stepper_ratio(1,8);  // pf stepper moves
	m_bitmap_printer->set_cr_stepper_ratio(1,6);  // this doesn't use the cr stepper
	m_dsrrom = memregion(TI99_DSRROM)->base();
}

void ti_thermal_printer_device::device_reset()
{
	clear_latches(ASSERT_LINE);
	clear_latches(CLEAR_LINE);
}

INPUT_CHANGED_MEMBER( ti_thermal_printer_device::feed_button )
{
	bool pressed = (newval==0);
	if (m_feed_pressed != pressed)
	{
		LOGMASKED(LOG_INTERRUPT, "Feed button %s\n", pressed? "pressed" : "released");
		m_feed_pressed = pressed;
		set_extint(((m_ext_int == ASSERT_LINE) || m_feed_pressed)? ASSERT_LINE : CLEAR_LINE);
	}
}

ROM_START( ti99_thermal )
	ROM_REGION(0x1000, TI99_DSRROM, 0)
	ROM_LOAD("thermal.u12", 0x0000, 0x1000, CRC(83376edb) SHA1(2f923e59a3a52b3645e8d26e11a697f320c6d4cb))
ROM_END

void ti_thermal_printer_device::device_add_mconfig(machine_config& config)
{
	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 120, 72);  // do 72 dpi

	TTL74123(config, m_heatmf_u8, 0);
	m_heatmf_u8->set_connection_type(TTL74123_GROUNDED);
	m_heatmf_u8->set_resistor_value(RES_K(10)); // actually, the 74122 is used (Rint = 10k)
	m_heatmf_u8->set_capacitor_value(CAP_U(4.7f));
	m_heatmf_u8->set_clear_pin_value(1);
	m_heatmf_u8->set_a_pin_value(0);

	LS259(config, m_latch_u4);     // Dots 2-9
	LS259(config, m_latch_u5);     // ROM / Heat trigger / stepper / dots 0 and 1
	LS259(config, m_latch_u6);     // columns 8-15 and 24-31
	LS259(config, m_latch_u7);     // columns 0-7 and 16-23

	m_latch_u5->q_out_cb<1>().set(m_heatmf_u8, FUNC(ttl74123_device::b_w));

	m_heatmf_u8->out_cb().set(FUNC(ti_thermal_printer_device::enable_column_latches));

	TI99_IOPORT(config, m_port, 0, ti99_ioport_options_evpc1, nullptr);
	m_port->extint_cb().set(FUNC(ti_thermal_printer_device::extint));
	m_port->ready_cb().set(FUNC(ti_thermal_printer_device::extready));
}

INPUT_PORTS_START(ti99_thermal)
	PORT_START("PAPERFEED")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Paper feed") PORT_CODE(KEYCODE_END) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ti_thermal_printer_device::feed_button), 1)
INPUT_PORTS_END

ioport_constructor ti_thermal_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ti99_thermal );
}

const tiny_rom_entry *ti_thermal_printer_device::device_rom_region() const
{
	return ROM_NAME( ti99_thermal );
}

} // end namespace bus::ti99::sidecar
