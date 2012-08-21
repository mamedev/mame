/******************************************************************************

    Tatung Einstein
    system driver


    TMS9129 VDP Graphics
        16k ram

    Z80 CPU (4 MHz)

    Z80 CTC (4 MHz)
        channel 0 is serial transmit clock
        channel 1 is serial receive clock
        trigger for channel 0,1 and 2 is a 2 MHz clock
        trigger for channel 3 is the terminal count of channel 2

    Intel 8251 Serial (2 MHz clock?)

    WD1770 Floppy Disc controller
        density is fixed, 4 drives and double sided supported

    AY-3-8910 PSG (2 MHz)
        port A and port B are connected to the keyboard. Port A is keyboard
        line select, Port B is data.

    printer connected to port A of PIO. /ACK from printer is connected to /ASTB.
    D7-D0 of PIO port A is printer data lines.
    ARDY of PIO is connected to /STROBE on printer.

    user port is port B of PIO
    keyboard connected to port A and port B of PSG

    TODO:
    - The ADC is not emulated!
    - printer emulation needs checking!

    Many thanks to Chris Coxall for the schematics of the TC-01, the dump of the
    system rom and a dump of a Xtal boot disc.

    Many thanks to Andrew Dunipace for his help with the 80-column card
    and Speculator hardware (Spectrum emulator).

    Kevin Thacker [MESS driver]


    2011-Mar-14, Phill Harvey-Smith.
        Having traced out the circuit of the TK02 80 coumn card, I have changed the
        emulation to match what the hardware does, the emulation was mostly correct,
        just some minor issues with the addressing of the VRAM, and bit 0 of the
        status register is the latched output of the 6845 DE, and not vblank.

        Also added defines to stop the log being flooded with keyboard messages :)

 ******************************************************************************/

#include "emu.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "video/tms9928a.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/wd1772.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/dsk_dsk.h"
#include "machine/ctronics.h"
#include "machine/i8251.h"
#include "imagedev/flopdrv.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "rendlay.h"
#include "machine/ram.h"
#include "includes/einstein.h"

#define VERBOSE_KEYBOARD	0
#define VERBOSE_DISK		0

const floppy_format_type einstein_state::floppy_formats[] = {
	FLOPPY_DSK_FORMAT, FLOPPY_MFM_FORMAT, FLOPPY_MFI_FORMAT,
	NULL
};

/***************************************************************************
    80 COLUMN DEVICE
***************************************************************************/

/* lower 3 bits of address define a 256-byte "row".
 * upper 8 bits define the offset in the row,
 * data bits define data to write
 */
WRITE8_MEMBER(einstein_state::einstein_80col_ram_w)
{
	m_crtc_ram[((offset & 0x07) << 8) | ((offset >> 8) & 0xff)] = data;
}

READ8_MEMBER(einstein_state::einstein_80col_ram_r)
{
	return m_crtc_ram[((offset & 0x07) << 8) | ((offset >> 8) & 0xff)];
}

/* TODO: Verify implementation */
/* From traces of the TK02 board character ROM is addressed by the 6845 as follows
   bit 0..2     ra0..ra2
   bit 3..10    data 0..7 from VRAM
   bit 11       ra3
   bit 12       jumper M004, this could be used to select two different character
                sets.
*/
static MC6845_UPDATE_ROW( einstein_6845_update_row )
{
	einstein_state *einstein = device->machine().driver_data<einstein_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *data = device->machine().root_device().memregion("gfx1")->base();
	UINT8 char_code, data_byte;
	int i, x;

	for (i = 0, x = 0; i < x_count; i++, x += 8)
	{
		char_code = einstein->m_crtc_ram[(ma + i) & 0x07ff];
		data_byte = data[(char_code << 3) + (ra & 0x07) + ((ra & 0x08) << 8)];

		bitmap.pix32(y, x + 0) = palette[TMS9928A_PALETTE_SIZE + BIT(data_byte, 7)];
		bitmap.pix32(y, x + 1) = palette[TMS9928A_PALETTE_SIZE + BIT(data_byte, 6)];
		bitmap.pix32(y, x + 2) = palette[TMS9928A_PALETTE_SIZE + BIT(data_byte, 5)];
		bitmap.pix32(y, x + 3) = palette[TMS9928A_PALETTE_SIZE + BIT(data_byte, 4)];
		bitmap.pix32(y, x + 4) = palette[TMS9928A_PALETTE_SIZE + BIT(data_byte, 3)];
		bitmap.pix32(y, x + 5) = palette[TMS9928A_PALETTE_SIZE + BIT(data_byte, 2)];
		bitmap.pix32(y, x + 6) = palette[TMS9928A_PALETTE_SIZE + BIT(data_byte, 1)];
		bitmap.pix32(y, x + 7) = palette[TMS9928A_PALETTE_SIZE + BIT(data_byte, 0)];
	}
}

static WRITE_LINE_DEVICE_HANDLER( einstein_6845_de_changed )
{
	einstein_state *einstein = device->machine().driver_data<einstein_state>();
	einstein->m_de=state;
}

/* bit 0 - latched display enabled (DE) from 6845
 * bit 1 - Jumper M003 0 = 40 column mode (color monitor), 1 = 80 column mode (b/w monitor)
 * bit 2 - Jumper M002 0 = 50Hz, 1 = 60Hz
 * bit 3 - Jumper M001 0 = ???, 1=??? perminently wired high on both the boards I have seen - PHS.
 */
READ8_MEMBER(einstein_state::einstein_80col_state_r)
{
	UINT8 result = 0;

	result |= m_de;
	result |= ioport("80column_dips")->read() & 0x06;

	return result;
}

/* int priority */
/* keyboard int->ctc/adc->pio */
static const z80_daisy_config einstein_daisy_chain[] =
{
	{ "keyboard_daisy" },
	{ IC_I058 },
	{ "adc_daisy" },
	{ IC_I063 },
	{ "fire_daisy" },
	{ NULL }
};


/***************************************************************************
    KEYBOARD
***************************************************************************/

/* refresh keyboard data. It is refreshed when the keyboard line is written */
static void einstein_scan_keyboard(running_machine &machine)
{
	einstein_state *einstein = machine.driver_data<einstein_state>();
	UINT8 data = 0xff;

	if (!BIT(einstein->m_keyboard_line, 0)) data &= machine.root_device().ioport("LINE0")->read();
	if (!BIT(einstein->m_keyboard_line, 1)) data &= machine.root_device().ioport("LINE1")->read();
	if (!BIT(einstein->m_keyboard_line, 2)) data &= machine.root_device().ioport("LINE2")->read();
	if (!BIT(einstein->m_keyboard_line, 3)) data &= machine.root_device().ioport("LINE3")->read();
	if (!BIT(einstein->m_keyboard_line, 4)) data &= machine.root_device().ioport("LINE4")->read();
	if (!BIT(einstein->m_keyboard_line, 5)) data &= machine.root_device().ioport("LINE5")->read();
	if (!BIT(einstein->m_keyboard_line, 6)) data &= machine.root_device().ioport("LINE6")->read();
	if (!BIT(einstein->m_keyboard_line, 7)) data &= machine.root_device().ioport("LINE7")->read();

	einstein->m_keyboard_data = data;
}

static TIMER_DEVICE_CALLBACK( einstein_keyboard_timer_callback )
{
	einstein_state *einstein = timer.machine().driver_data<einstein_state>();

	/* re-scan keyboard */
	einstein_scan_keyboard(timer.machine());

	/* if /fire1 or /fire2 is 0, signal a fire interrupt */
	if ((timer.machine().root_device().ioport("BUTTONS")->read() & 0x03) != 0)
	{
		einstein->m_interrupt |= EINSTEIN_FIRE_INT;
	}

	/* keyboard data changed? */
	if (einstein->m_keyboard_data != 0xff)
	{
		/* generate interrupt */
		einstein->m_interrupt |= EINSTEIN_KEY_INT;
	}
}

WRITE8_MEMBER(einstein_state::einstein_keyboard_line_write)
{

	if (VERBOSE_KEYBOARD)
		logerror("einstein_keyboard_line_write: %02x\n", data);

	m_keyboard_line = data;

	/* re-scan the keyboard */
	einstein_scan_keyboard(machine());
}

READ8_MEMBER(einstein_state::einstein_keyboard_data_read)
{

	/* re-scan the keyboard */
	einstein_scan_keyboard(machine());

	if (VERBOSE_KEYBOARD)
		logerror("einstein_keyboard_data_read: %02x\n", m_keyboard_data);

	return m_keyboard_data;
}


/***************************************************************************
    FLOPPY DRIVES
***************************************************************************/

static WRITE8_DEVICE_HANDLER( einstein_drsel_w )
{
	einstein_state *einstein = device->machine().driver_data<einstein_state>();
	if(VERBOSE_DISK)
		logerror("%s: einstein_drsel_w %02x\n", device->machine().describe_context(), data);

	/* bit 0 to 3 select the drive */
	static const char *names[] = { "fd0", "fd1", "fd2", "fd3" };
	floppy_image_device *floppy = 0;
	for(int i=0; i<4; i++) {
		if(BIT(data, i)) {
			floppy_connector *con = device->machine().device<floppy_connector>(names[i]);
			if(con)
				floppy = con->get_device();
		}
	}

	/* double sided drive connected? */
	if (device->machine().root_device().ioport("config")->read() & data)
	{
		/* bit 4 selects the side then */
		//floppy->ss_w(BIT(data, 4));
	}
	if (floppy) floppy->ss_w(0);
	einstein->m_fdc->set_floppy(floppy);
}


/***************************************************************************
    CTC
***************************************************************************/

/* channel 0 and 1 have a 2 MHz input clock for triggering */
static TIMER_DEVICE_CALLBACK( einstein_ctc_trigger_callback )
{
	einstein_state *einstein = timer.machine().driver_data<einstein_state>();

	/* toggle line status */
	einstein->m_ctc_trigger ^= 1;

	einstein->m_ctc->trg0(einstein->m_ctc_trigger);
	einstein->m_ctc->trg1(einstein->m_ctc_trigger);
}


/***************************************************************************
    UART
***************************************************************************/

static WRITE_LINE_DEVICE_HANDLER( einstein_serial_transmit_clock )
{
	i8251_device *uart = device->machine().device<i8251_device>(IC_I060);
	uart->transmit_clock();
}

static WRITE_LINE_DEVICE_HANDLER( einstein_serial_receive_clock )
{
	i8251_device *uart = device->machine().device<i8251_device>(IC_I060);
	uart->receive_clock();
}


/***************************************************************************
    MEMORY BANKING
***************************************************************************/

static void einstein_page_rom(running_machine &machine)
{
	einstein_state *state = machine.driver_data<einstein_state>();
	state->membank("bank1")->set_base(state->m_rom_enabled ? machine.root_device().memregion("bios")->base() : machine.device<ram_device>(RAM_TAG)->pointer());
}

/* writing to this port is a simple trigger, and switches between RAM and ROM */
WRITE8_MEMBER(einstein_state::einstein_rom_w)
{
	m_rom_enabled ^= 1;
	einstein_page_rom(machine());
}


/***************************************************************************
    INTERRUPTS
***************************************************************************/

READ8_MEMBER(einstein_state::einstein_kybintmsk_r)
{
	centronics_device *centronics = machine().device<centronics_device>("centronics");
	UINT8 data = 0;

	/* clear key int. a read of this I/O port will do this or a reset */
	m_interrupt &= ~EINSTEIN_KEY_INT;

	/* bit 0 and 1: fire buttons on the joysticks */
	data |= ioport("BUTTONS")->read();

	/* bit 2 to 4: printer status */
	data |= centronics->busy_r() << 2;
	data |= centronics->pe_r() << 3;
	data |= centronics->fault_r() << 4;

	/* bit 5 to 7: graph, control and shift key */
	data |= ioport("EXTRA")->read();

	if(VERBOSE_KEYBOARD)
		logerror("%s: einstein_kybintmsk_r %02x\n", machine().describe_context(), data);

	return data;
}

WRITE8_MEMBER(einstein_state::einstein_kybintmsk_w)
{

	logerror("%s: einstein_kybintmsk_w %02x\n", machine().describe_context(), data);

	/* set mask from bit 0 */
	if (data & 0x01)
	{
		logerror("key int is disabled\n");
		m_interrupt_mask &= ~EINSTEIN_KEY_INT;
	}
	else
	{
		logerror("key int is enabled\n");
		m_interrupt_mask |= EINSTEIN_KEY_INT;
	}
}

/* writing to this I/O port sets the state of the mask; D0 is used */
/* writing 0 enables the /ADC interrupt */
WRITE8_MEMBER(einstein_state::einstein_adcintmsk_w)
{

	logerror("%s: einstein_adcintmsk_w %02x\n", machine().describe_context(), data);

	if (data & 0x01)
	{
		logerror("adc int is disabled\n");
		m_interrupt_mask &= ~EINSTEIN_ADC_INT;
	}
	else
	{
		logerror("adc int is enabled\n");
		m_interrupt_mask |= EINSTEIN_ADC_INT;
	}
}

/* writing to this I/O port sets the state of the mask; D0 is used */
/* writing 0 enables the /FIRE interrupt */
WRITE8_MEMBER(einstein_state::einstein_fire_int_w)
{

	logerror("%s: einstein_fire_int_w %02x\n", machine().describe_context(), data);

	if (data & 0x01)
	{
		logerror("fire int is disabled\n");
		m_interrupt_mask &= ~EINSTEIN_FIRE_INT;
	}
	else
	{
		logerror("fire int is enabled\n");
		m_interrupt_mask |= EINSTEIN_FIRE_INT;
	}
}


/***************************************************************************
    MACHINE EMULATION
***************************************************************************/

static TMS9928A_INTERFACE(einstein_tms9929a_interface)
{
	"screen",
	0x4000, /* 16k RAM, provided by IC i040 and i041 */
	DEVCB_NULL
};

static MACHINE_START( einstein )
{
}

static MACHINE_RESET( einstein )
{
	einstein_state *state = machine.driver_data<einstein_state>();
	//device_t *floppy;
	//UINT8 config = machine.root_device().ioport("config")->read();

	/* save pointers to our devices */
	state->m_color_screen = machine.device("screen");
	state->m_ctc = machine.device<z80ctc_device>(IC_I058);

	/* initialize memory mapping */
	state->membank("bank2")->set_base(machine.device<ram_device>(RAM_TAG)->pointer());
	state->membank("bank3")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0x8000);
	state->m_rom_enabled = 1;
	einstein_page_rom(machine);

	/* a reset causes the fire int, adc int, keyboard int mask
    to be set to 1, which causes all these to be DISABLED */
	state->m_interrupt = 0;
	state->m_interrupt_mask = 0;

	state->m_ctc_trigger = 0;

	/* configure floppy drives */
/*  floppy_type_t type_80 = FLOPPY_STANDARD_5_25_DSHD;
    floppy_type_t type_40 = FLOPPY_STANDARD_5_25_SSDD_40;
    floppy = machine.device("floppy0");
    floppy_drive_set_geometry(floppy, config & 0x01 ? type_80 : type_40);
    floppy = machine.device("floppy1");
    floppy_drive_set_geometry(floppy, config & 0x02 ? type_80 : type_40);
    floppy = machine.device("floppy2");
    floppy_drive_set_geometry(floppy, config & 0x04 ? type_80 : type_40);
    floppy = machine.device("floppy3");
    floppy_drive_set_geometry(floppy, config & 0x08 ? type_80 : type_40);*/
}

static MACHINE_RESET( einstein2 )
{
	einstein_state *einstein = machine.driver_data<einstein_state>();

	/* call standard initialization first */
	MACHINE_RESET_CALL(einstein);

	/* get 80 column specific devices */
	einstein->m_mc6845 = machine.device<mc6845_device>("crtc");
	einstein->m_crtc_screen = machine.device<screen_device>("80column");

	/* 80 column card palette */
	palette_set_color(machine, TMS9928A_PALETTE_SIZE, RGB_BLACK);
	palette_set_color(machine, TMS9928A_PALETTE_SIZE + 1, MAKE_RGB(0, 224, 0));
}

static MACHINE_START( einstein2 )
{
	einstein_state *einstein = machine.driver_data<einstein_state>();
	einstein->m_crtc_ram = auto_alloc_array(machine, UINT8, 2048);
	MACHINE_START_CALL(einstein);
}


/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

static SCREEN_UPDATE_RGB32( einstein2 )
{
	einstein_state *einstein = screen.machine().driver_data<einstein_state>();

	if (&screen == einstein->m_color_screen)
	{
		tms9929a_device *tms9929a = screen.machine().device<tms9929a_device>( "tms9929a" );
		const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
		bitmap_ind16 &src = tms9929a->get_bitmap();
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
				bitmap.pix32(y, x) = palette[src.pix16(y, x)];
	}
	else if (&screen == einstein->m_crtc_screen)
		einstein->m_mc6845->screen_update( screen, bitmap, cliprect);
	else
		fatalerror("Unknown screen '%s'", screen.tag());

	return 0;
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( einstein_mem, AS_PROGRAM, 8, einstein_state )
	AM_RANGE(0x0000, 0x07fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
	AM_RANGE(0x8000, 0x0ffff) AM_RAMBANK("bank3")
ADDRESS_MAP_END

/* The I/O ports are decoded into 8 blocks using address lines A3 to A7 */
static ADDRESS_MAP_START( einstein_io, AS_IO, 8, einstein_state )
	/* block 0, ay8910 psg */
	AM_RANGE(0x02, 0x02) AM_MIRROR(0xff04) AM_DEVREADWRITE_LEGACY(IC_I030, ay8910_r, ay8910_address_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0xff04) AM_DEVWRITE_LEGACY(IC_I030, ay8910_data_w)
	/* block 1, tms9928a vdp */
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff06) AM_DEVREADWRITE("tms9929a", tms9929a_device, vram_read, vram_write)
	AM_RANGE(0x09, 0x09) AM_MIRROR(0xff06) AM_DEVREADWRITE("tms9929a", tms9929a_device, register_read, register_write)
	/* block 2, i8251 uart */
	AM_RANGE(0x10, 0x10) AM_MIRROR(0xff06) AM_DEVREADWRITE(IC_I060, i8251_device, data_r, data_w)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0xff06) AM_DEVREADWRITE(IC_I060, i8251_device, status_r, control_w)
	/* block 3, wd1770 floppy controller */
	AM_RANGE(0x18, 0x1b) AM_MIRROR(0xff04) AM_DEVREADWRITE(IC_I042, wd177x_t, read, write)
	/* block 4, internal controls */
	AM_RANGE(0x20, 0x20) AM_MIRROR(0xff00) AM_READWRITE(einstein_kybintmsk_r, einstein_kybintmsk_w)
	AM_RANGE(0x21, 0x21) AM_MIRROR(0xff00) AM_WRITE(einstein_adcintmsk_w)
	AM_RANGE(0x23, 0x23) AM_MIRROR(0xff00) AM_DEVWRITE_LEGACY(IC_I042, einstein_drsel_w)
	AM_RANGE(0x24, 0x24) AM_MIRROR(0xff00) AM_WRITE(einstein_rom_w)
	AM_RANGE(0x25, 0x25) AM_MIRROR(0xff00) AM_WRITE(einstein_fire_int_w)
	/* block 5, z80ctc */
	AM_RANGE(0x28, 0x2b) AM_MIRROR(0xff04) AM_DEVREADWRITE(IC_I058, z80ctc_device, read, write)
	/* block 6, z80pio */
	AM_RANGE(0x30, 0x33) AM_MIRROR(0xff04) AM_DEVREADWRITE(IC_I063, z80pio_device, read, write)
#if 0
	/* block 7, adc */
	AM_RANGE(0x38, 0x38) AM_MIRROR(0xff07) AM_DEVREADWRITE_LEGACY(IC_I050, adc0844_r, adc0844_w)
#endif
ADDRESS_MAP_END

static ADDRESS_MAP_START( einstein2_io, AS_IO, 8, einstein_state )
	AM_IMPORT_FROM(einstein_io)
	AM_RANGE(0x40, 0x47) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(einstein_80col_ram_r, einstein_80col_ram_w)
	AM_RANGE(0x48, 0x48) AM_MIRROR(0xff00) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x49, 0x49) AM_MIRROR(0xff00) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x4c, 0x4c) AM_MIRROR(0xff00) AM_READ(einstein_80col_state_r)
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( einstein )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('\xA3')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\xBA') PORT_CHAR('\xBD')	// is \xBA correct for double vertical bar || ?
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('@')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("EXTRA")
	PORT_BIT(0x1f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRPH")    PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)

	/* fire buttons for analogue joysticks */
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 1 Button 1") PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 2 Button 1") PORT_PLAYER(2)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	/* analog joystick 1 x axis */
	PORT_START("JOY1_X")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1) PORT_REVERSE

	/* analog joystick 1 y axis */
	PORT_START("JOY1_Y")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1) PORT_REVERSE

	/* analog joystick 2 x axis */
	PORT_START("JOY2_X")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2) PORT_REVERSE

	/* analog joystick 2 Y axis */
	PORT_START("JOY2_Y")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("config")
	PORT_CONFNAME(0x01, 0x00, "Floppy drive #1")
	PORT_CONFSETTING(0x00, "Single sided")
	PORT_CONFSETTING(0x01, "Double sided")
	PORT_CONFNAME(0x02, 0x00, "Floppy drive #2")
	PORT_CONFSETTING(0x00, "Single sided")
	PORT_CONFSETTING(0x02, "Double sided")
	PORT_CONFNAME(0x04, 0x00, "Floppy drive #3")
	PORT_CONFSETTING(0x00, "Single sided")
	PORT_CONFSETTING(0x04, "Double sided")
	PORT_CONFNAME(0x08, 0x00, "Floppy drive #4")
	PORT_CONFSETTING(0x00, "Single sided")
	PORT_CONFSETTING(0x08, "Double sided")
INPUT_PORTS_END

static INPUT_PORTS_START( einstein_80col )
	PORT_INCLUDE(einstein)

	/* dip switches on the 80 column card */
	PORT_START("80column_dips")
	PORT_DIPNAME(0x02, 0x00, "Startup mode")
	PORT_DIPSETTING(0x00, "40 column")
	PORT_DIPSETTING(0x02, "80 column")
	PORT_DIPNAME(0x04, 0x00, "50/60Hz")
	PORT_DIPSETTING(0x00, "50Hz")
	PORT_DIPSETTING(0x04, "60Hz")
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static Z80CTC_INTERFACE( einstein_ctc_intf )
{
	DEVCB_NULL,
	DEVCB_LINE(einstein_serial_transmit_clock),
	DEVCB_LINE(einstein_serial_receive_clock),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF, z80ctc_device, trg3)
};


static Z80PIO_INTERFACE( einstein_pio_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("centronics", centronics_device, write),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("centronics", centronics_device, strobe_w),
	DEVCB_NULL
};

static const ay8910_interface einstein_ay_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(einstein_state, einstein_keyboard_data_read),
	DEVCB_DRIVER_MEMBER(einstein_state, einstein_keyboard_line_write),
	DEVCB_NULL
};

static const centronics_interface einstein_centronics_config =
{
	DEVCB_DEVICE_LINE_MEMBER(IC_I063, z80pio_device, strobe_a),
	DEVCB_NULL,
	DEVCB_NULL
};

static const mc6845_interface einstein_crtc6845_interface =
{
	"80column",
	8,
	NULL,
	einstein_6845_update_row,
	NULL,
	DEVCB_LINE(einstein_6845_de_changed),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

/* F4 Character Displayer */
static const gfx_layout einstei2_charlayout =
{
	8, 10,					/* 8 x 10 characters */
	256,					/* 256*2 characters */
	1,					/* 1 bits per pixel */
	{ 0 },
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0x800*8, 0x801*8 },
	8*8
};

static GFXDECODE_START( einstei2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, einstei2_charlayout, 16, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, einstei2_charlayout, 16, 1 )
GFXDECODE_END

static SLOT_INTERFACE_START( einstein_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( einstein, einstein_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(IC_I001, Z80, XTAL_X002 / 2)
	MCFG_CPU_PROGRAM_MAP(einstein_mem)
	MCFG_CPU_IO_MAP(einstein_io)
	MCFG_CPU_CONFIG(einstein_daisy_chain)

	MCFG_MACHINE_START(einstein)
	MCFG_MACHINE_RESET(einstein)

	/* this is actually clocked at the system clock 4 MHz, but this would be too fast for our
    driver. So we update at 50Hz and hope this is good enough. */
	MCFG_TIMER_ADD_PERIODIC("keyboard", einstein_keyboard_timer_callback, attotime::from_hz(50))

	MCFG_Z80PIO_ADD(IC_I063, XTAL_X002 / 2, einstein_pio_intf)

	MCFG_Z80CTC_ADD(IC_I058, XTAL_X002 / 2, einstein_ctc_intf)
	/* the input to channel 0 and 1 of the ctc is a 2 MHz clock */
	MCFG_TIMER_ADD_PERIODIC("ctc", einstein_ctc_trigger_callback, attotime::from_hz(XTAL_X002 /4))

	/* Einstein daisy chain support for non-Z80 devices */
	MCFG_DEVICE_ADD("keyboard_daisy", EINSTEIN_KEYBOARD_DAISY, 0)
	MCFG_DEVICE_ADD("adc_daisy", EINSTEIN_ADC_DAISY, 0)
	MCFG_DEVICE_ADD("fire_daisy", EINSTEIN_FIRE_DAISY, 0)

	/* video hardware */
	MCFG_TMS9928A_ADD( "tms9929a", TMS9929A, einstein_tms9929a_interface )
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9929a", tms9929a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(IC_I030, AY8910, XTAL_X002 / 4)
	MCFG_SOUND_CONFIG(einstein_ay_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	/* printer */
	MCFG_CENTRONICS_PRINTER_ADD("centronics", einstein_centronics_config)

	/* uart */
	MCFG_I8251_ADD(IC_I060, default_i8251_interface)

	MCFG_WD1770x_ADD(IC_I042, XTAL_X002)

	MCFG_FLOPPY_DRIVE_ADD("fd0", einstein_floppies, "525dd", 0, einstein_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1", einstein_floppies, "525dd", 0, einstein_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd2", einstein_floppies, "525dd", 0, einstein_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd3", einstein_floppies, "525dd", 0, einstein_state::floppy_formats)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","einstein")

	/* RAM is provided by 8k DRAM ICs i009, i010, i011, i012, i013, i014, i015 and i016 */
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( einstei2, einstein )

	MCFG_CPU_MODIFY(IC_I001)
	MCFG_CPU_IO_MAP(einstein2_io)

	MCFG_MACHINE_START(einstein2)
	MCFG_MACHINE_RESET(einstein2)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("80column", RASTER)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_STATIC(einstein2)
	MCFG_GFXDECODE(einstei2)

	/* 2 additional colors for the 80 column screen */
	MCFG_PALETTE_LENGTH(TMS9928A_PALETTE_SIZE + 2)

	MCFG_MC6845_ADD("crtc", MC6845, XTAL_X002 / 4, einstein_crtc6845_interface)
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

/* There are two sockets, i023 and i024, each either a 2764 or 27128
 * only i023 is used by default and fitted with the 8k bios (called MOS).
 *
 * We are missing dumps of version MOS 1.1, possibly of 1.0 if it exists.
 */
ROM_START( einstein )
	ROM_REGION(0x8000, "bios", 0)
	/* i023 */
	ROM_SYSTEM_BIOS(0,  "mos12",  "MOS 1.2")
	ROMX_LOAD("mos12.i023", 0, 0x2000, CRC(ec134953) SHA1(a02125d8ebcda48aa784adbb42a8b2d7ef3a4b77), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1,  "mos121",  "MOS 1.21")
	ROMX_LOAD("mos121.i023", 0, 0x2000, CRC(a746eeb6) SHA1(f75aaaa777d0fd92225acba291f6bf428b341d3e), ROM_BIOS(2))
	ROM_RELOAD(0x2000, 0x2000)
	/* i024 */
	ROM_FILL(0x4000, 0x4000, 0xff)
ROM_END

ROM_START( einstei2 )
	ROM_REGION(0x8000, "bios", 0)
	/* i023 */
	ROM_SYSTEM_BIOS(0,  "mos12",  "MOS 1.2")
	ROMX_LOAD("mos12.i023", 0, 0x2000, CRC(ec134953) SHA1(a02125d8ebcda48aa784adbb42a8b2d7ef3a4b77), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1,  "mos121",  "MOS 1.21")
	ROMX_LOAD("mos121.i023", 0, 0x2000, CRC(a746eeb6) SHA1(f75aaaa777d0fd92225acba291f6bf428b341d3e), ROM_BIOS(2))
	ROM_RELOAD(0x2000, 0x2000)
	/* i024 */
	ROM_FILL(0x4000, 0x4000, 0xff)

	/* character rom from 80 column card */
	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("tk02-v1.00.rom", 0, 0x2000, CRC(ad3c4346) SHA1(cd57e630371b4d0314e3f15693753fb195c7257d))
ROM_END

ROM_START( einst256 )
	ROM_REGION(0x8000, "bios", 0)
	ROM_LOAD("tc256.rom", 0x0000, 0x4000, CRC(ef8dad88) SHA1(eb2102d3bef572db7161c26a7c68a5fcf457b4d0) )
ROM_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT           INIT  COMPANY   FULLNAME                             FLAGS */
COMP( 1984, einstein, 0,        0,		einstein, einstein, driver_device,       0,    "Tatung", "Einstein TC-01",                    0 )
COMP( 1984, einstei2, einstein, 0,		einstei2, einstein_80col, driver_device, 0,    "Tatung", "Einstein TC-01 + 80 column device", 0 )
COMP( 1984, einst256, 0,        0,		einstein, einstein, driver_device,       0,    "Tatung", "Einstein 256",						 GAME_NOT_WORKING )
