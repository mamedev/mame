// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Processor Technology Corp. SOL-20

    07/2009 Skeleton driver.

    Info from: http://www.sol20.org/

    The Sol Terminal Computer was the first complete S-100 microcomputer
    with a built-in keyboard and video output. However, its internal
    architecture resembles a video display terminal more than anything else.

    Note that the SOLOS dump comes from the Solace emu. Confirmed as ok.

    The roms DPMON and CONSOL are widely available on the net as ENT files,
    which can be loaded into memory with the Paste option, then exported to
    binary files with the debugger. They are working, however the ENT files
    do not indicate the values of unused rom space. Therefore they are marked
    as BAD_DUMP.

    Note that the CONSOL rom is basically a dumb terminal program and doesn't
    do anything useful unless the MODE key (whatever that is) is pressed.

    CUTER is a relocatable cassette-based alternative to SOLOS. According
    to the manual, it should work if the sense switches are set to on. But,
    it continuously reads port 00 and does nothing.

    Need original dumps of all roms.

    The character roms are built by a script from the Solace source, then the
    characters moved back to the original 7x9 matrix positions, as shown in
    the manual. Although they look exactly the same, I've marked them as
    BAD_DUMP until properly verified.

    Other roms needed:
      - Dump of U18 in the keyboard

    Hardware variants (ever shipped?):
      - SOL-10, i.e. a stripped down version without expansion backplane and
        numeric keypad
      - SOL-PC, i.e. the single board version, basically it consisted of the
        SOL-20 board only

    Similarity to Exidy Sorcerer:
      - Some hardware is the same design (parallel ports, serial ports, 2
        tape players, the cassette circuits)
      - The cassette format is totally identical, in fact tapes from one
        machine can be loaded on the other natively. They won't run of course.
      - Sorcerer monitor commands are almost identical to the SOLOS ones.
        example: to change tape speed to 300 baud:
        SE T=1 (sorcerer)  SE TA 1 (SOLOS)
        Most other commands are identical. The EN command (enter bytes into
        memory) is rather primitive in SOLOS, better on Sorcerer.
      - Sol20 has "personality modules" where you could plug a new OS into
        a special socket on the motherboard. Sorcerer improved upon this by
        being the first computer to use cartridges.
      - Some circuits are completely different... the video and keyboard are
        notable examples, while the addresses of ram and bios on the basic
        machines is another major difference.

    ToDo:
      - connect up half/full duplex dipswitch function
      - connect up the remaining functions of port FE
      - keyboard (using terminal keyboard for now)
      - optional floppy disk support and CP/M
      - parallel i/o and handshake lines
      - connect serial i/o to something
      - support for loading the various file formats

    File Formats:
      - Most files are simple ascii which can be loaded via the Paste handler.
        These are ASC, ENT, BAS, ROM, BS5 and ECB. Most files require that the
        correct version of BASIC be loaded first. Paste works, but it is very
        very slow. Perhaps we need something faster such as what Solace has.
      - SVT (Solace Virtual Tape) files are a representation of a cassette,
        usually holding about 4 games, just like a multifile tape. This format
        is partially supported.
      - HEX files appear to be the standard Intel format, and can be loaded
        by Solace.
      - The remaining formats (OPN, PL, PRN, SMU, SOL, ASM and LIB) appear
        at first glance to be more specialised, and probably not worth being
        supported.

    System Setup (to play games etc).
      - In the Dipswitches (not the Configuration), turn cursor flashing OFF.
      - Loading via wav files works, so load a tape image in the file manager
      - In UPPER CASE, enter XE press enter, cassette will load.
      - At the end, the program will start by itself.
      - When it says use 2,4,6,8 keys, you can use the keyboard arrow keys.

    Monitor Commands:
      - TE - return to terminal mode
      - DU - dump memory
      - EN - modify memory
      - EX - Go (execute)
      - CU - Insert or remove a custom command
      - SE - Set parameters (eg tape speed)
      - SA - Save
      - GE - Load
      - XE - Load and run
      - CA - List the files on a tape

    How to use the music system:
    1. Choose the "music" item from the software list
    2. Type XEQ press Enter
    3. When the program starts, press R, you are back in the monitor
    4. Type GET press Enter
    5. When this finishes loading, type EX 0 press Enter, there's no response
    6. Press F, wait for numbers to appear
    7. Press S, wait for numbers to appear
    8. Press P, music will be heard (it isn't very loud)

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "imagedev/cassette.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/keyboard.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/sol_cas.h"


namespace {

class sol20_state : public driver_device
{
public:
	sol20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_screen(*this, "screen")
		, m_cass1(*this, "cassette")
		, m_cass2(*this, "cassette2")
		, m_uart(*this, "uart")
		, m_uart_s(*this, "uart_s")
		, m_uart_clock(*this, "uart_clock")
		, m_uart_s_clock(*this, "uart_s_clock")
		, m_rs232(*this, "rs232")
		, m_vram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_iop_arrows(*this, "ARROWS")
		, m_iop_config(*this, "CONFIG")
		, m_iop_s1(*this, "S1")
		, m_iop_s2(*this, "S2")
		, m_iop_s3(*this, "S3")
		, m_iop_s4(*this, "S4")
	{ }

	void sol20(machine_config &config);

private:
	enum
	{
		TIMER_SOL20_CASSETTE_TC,
	};

	struct cass_data_t
	{
		struct {
			int length;     /* time cassette level is at input.level */
			int level;      /* cassette level */
			int bit;        /* bit being read */
		} input;
		struct {
			int length;     /* time cassette level is at output.level */
			int level;      /* cassette level */
			int bit;        /* bit to output */
		} output;
	};

	u8 sol20_f8_r();
	u8 sol20_fa_r();
	u8 sol20_fc_r();
	u8 sol20_fd_r();
	void sol20_f8_w(u8 data);
	void sol20_fa_w(u8 data);
	void sol20_fd_w(u8 data);
	void sol20_fe_w(u8 data);
	void kbd_put(u8 data);
	TIMER_CALLBACK_MEMBER(sol20_cassette_tc);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	u8 m_sol20_fa = 0U;
	u8 m_sol20_fc = 0U;
	u8 m_sol20_fe = 0U;
	u8 m_framecnt = 0U;
	cass_data_t m_cass_data;
	emu_timer *m_cassette_timer;
	cassette_image_device *cassette_device_image();
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<i8080a_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cass1;
	required_device<cassette_image_device> m_cass2;
	required_device<ay31015_device> m_uart;
	required_device<ay31015_device> m_uart_s;
	required_device<clock_device> m_uart_clock;
	required_device<clock_device> m_uart_s_clock;
	required_device<rs232_port_device> m_rs232;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_p_chargen;
	required_ioport m_iop_arrows;
	required_ioport m_iop_config;
	required_ioport m_iop_s1;
	required_ioport m_iop_s2;
	required_ioport m_iop_s3;
	required_ioport m_iop_s4;
};


/* timer to read cassette waveforms */


cassette_image_device *sol20_state::cassette_device_image()
{
	if (m_sol20_fa & 0x40)
		return m_cass2;
	else
		return m_cass1;
}


void sol20_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_SOL20_CASSETTE_TC:
		sol20_cassette_tc(param);
		break;
	default:
		throw emu_fatalerror("Unknown id in sol20_state::device_timer");
	}
}


// identical to sorcerer
TIMER_CALLBACK_MEMBER(sol20_state::sol20_cassette_tc)
{
	u8 cass_ws = 0;
	switch (m_sol20_fa & 0x20)
	{
		case 0x20:              /* Cassette 300 baud */

			/* loading a tape - this is basically the same as the super80.
			               We convert the 1200/2400 Hz signal to a 0 or 1, and send it to the uart. */

			m_cass_data.input.length++;

			cass_ws = ((cassette_device_image())->input() > +0.02) ? 1 : 0;

			if (cass_ws != m_cass_data.input.level)
			{
				m_cass_data.input.level = cass_ws;
				m_cass_data.input.bit = ((m_cass_data.input.length < 0x6) || (m_cass_data.input.length > 0x20)) ? 1 : 0;
				m_cass_data.input.length = 0;
				m_uart->write_si(m_cass_data.input.bit);
			}

			/* saving a tape - convert the serial stream from the uart, into 1200 and 2400 Hz frequencies.
			               Synchronisation of the frequency pulses to the uart is extremely important. */

			m_cass_data.output.length++;
			if (!(m_cass_data.output.length & 0x1f))
			{
				cass_ws = m_uart->so_r();
				if (cass_ws != m_cass_data.output.bit)
				{
					m_cass_data.output.bit = cass_ws;
					m_cass_data.output.length = 0;
				}
			}

			if (!(m_cass_data.output.length & 3))
			{
				if (!((m_cass_data.output.bit == 0) && (m_cass_data.output.length & 4)))
				{
					m_cass_data.output.level ^= 1;          // toggle output this, except on 2nd half of low bit
					cassette_device_image()->output(m_cass_data.output.level ? -1.0 : +1.0);
				}
			}
			return;

		case 0x00:          /* Cassette 1200 baud */
			/* loading a tape */
			m_cass_data.input.length++;

			cass_ws = ((cassette_device_image())->input() > +0.02) ? 1 : 0;

			if (cass_ws != m_cass_data.input.level || m_cass_data.input.length == 10)
			{
				m_cass_data.input.bit = ((m_cass_data.input.length < 10) || (m_cass_data.input.length > 0x20)) ? 1 : 0;
				if ( cass_ws != m_cass_data.input.level )
				{
					m_cass_data.input.length = 0;
					m_cass_data.input.level = cass_ws;
				}
				m_uart->write_si(m_cass_data.input.bit);
			}

			/* saving a tape - convert the serial stream from the uart, into 600 and 1200 Hz frequencies. */

			m_cass_data.output.length++;
			if (!(m_cass_data.output.length & 7))
			{
				cass_ws = m_uart->so_r();
				if (cass_ws != m_cass_data.output.bit)
				{
					m_cass_data.output.bit = cass_ws;
					m_cass_data.output.length = 0;
				}
			}

			if (!(m_cass_data.output.length & 7))
			{
				if (!((m_cass_data.output.bit == 0) && (m_cass_data.output.length & 8)))
				{
					m_cass_data.output.level ^= 1;          // toggle output this, except on 2nd half of low bit
					cassette_device_image()->output(m_cass_data.output.level ? -1.0 : +1.0);
				}
			}
			return;
	}
}

u8 sol20_state::sol20_f8_r()
{
// d7 - TBMT; d6 - DAV; d5 - CTS; d4 - OE; d3 - PE; d2 - FE; d1 - DSR; d0 - CD
	u8 data = 0;

	m_uart_s->write_swe(0);
	data |= m_uart_s->tbmt_r() ? 0x80 : 0;
	data |= m_uart_s->dav_r( ) ? 0x40 : 0;
	data |= m_rs232->cts_r(  ) ? 0x20 : 0;
	data |= m_uart_s->or_r(  ) ? 0x10 : 0;
	data |= m_uart_s->pe_r(  ) ? 0x08 : 0;
	data |= m_uart_s->fe_r(  ) ? 0x04 : 0;
	data |= m_rs232->dsr_r(  ) ? 0x02 : 0;
	data |= m_rs232->dcd_r(  ) ? 0x01 : 0;
	m_uart_s->write_swe(1);

	return data;
}

u8 sol20_state::sol20_fa_r()
{
	/* set unused bits high */
	u8 data = 0x26;

	m_uart->write_swe(0);
	data |= m_uart->tbmt_r() ? 0x80 : 0;
	data |= m_uart->dav_r( ) ? 0x40 : 0;
	data |= m_uart->or_r(  ) ? 0x10 : 0;
	data |= m_uart->fe_r(  ) ? 0x08 : 0;
	m_uart->write_swe(1);

	bool arrowkey = m_iop_arrows->read() ? 0 : 1;
	bool keydown = m_sol20_fa & 1;

	return data | (arrowkey & keydown);
}

u8 sol20_state::sol20_fc_r()
{
	u8 data = m_iop_arrows->read();
	if (BIT(data, 0)) return 0x32;
	if (BIT(data, 1)) return 0x34;
	if (BIT(data, 2)) return 0x36;
	if (BIT(data, 3)) return 0x38;

	m_sol20_fa |= 1;
	return m_sol20_fc;
}

u8 sol20_state::sol20_fd_r()
{
// Return a byte from parallel interface
	return 0;
}

void sol20_state::sol20_f8_w(u8 data)
{
// The only function seems to be to send RTS from bit 4
	m_rs232->write_rts(BIT(data, 4));
}

void sol20_state::sol20_fa_w(u8 data)
{
	m_sol20_fa &= 1;
	m_sol20_fa |= (data & 0xf0);

	/* cassette 1 motor */
	m_cass1->change_state(
		(BIT(data,7)) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	/* cassette 2 motor */
	m_cass2->change_state(
		(BIT(data,6)) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (data & 0xc0)
		m_cassette_timer->adjust(attotime::zero, 0, attotime::from_hz(19200));
	else
		m_cassette_timer->adjust(attotime::zero);

	// bit 5 baud rate */
	m_uart_clock->set_unscaled_clock(BIT(data, 5) ? 4800 : 19200);
}

void sol20_state::sol20_fd_w(u8 data)
{
// Output a byte to parallel interface
}

void sol20_state::sol20_fe_w(u8 data)
{
	m_sol20_fe = data;
}

void sol20_state::mem_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share("mainram"); // optional s100 ram
	map(0xc000, 0xc7ff).rom().region("maincpu", 0);
	map(0xc800, 0xcbff).ram(); // system ram
	map(0xcc00, 0xcfff).ram().share("videoram");
	map(0xd000, 0xffff).ram(); // optional s100 ram
}

void sol20_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf8, 0xf8).rw(FUNC(sol20_state::sol20_f8_r), FUNC(sol20_state::sol20_f8_w));
	map(0xf9, 0xf9).rw(m_uart_s, FUNC(ay51013_device::receive), FUNC(ay51013_device::transmit));
	map(0xfa, 0xfa).rw(FUNC(sol20_state::sol20_fa_r), FUNC(sol20_state::sol20_fa_w));
	map(0xfb, 0xfb).rw(m_uart, FUNC(ay51013_device::receive), FUNC(ay51013_device::transmit));
	map(0xfc, 0xfc).r(FUNC(sol20_state::sol20_fc_r));
	map(0xfd, 0xfd).rw(FUNC(sol20_state::sol20_fd_r), FUNC(sol20_state::sol20_fd_w));
	map(0xfe, 0xfe).w(FUNC(sol20_state::sol20_fe_w));
	map(0xff, 0xff).portr("S2");
/*  map(0xf8, 0xf8) serial status in (bit 6=data av, bit 7=tmbe)
    map(0xf9, 0xf9) serial data in, out
    map(0xfa, 0xfa) general status in (bit 0=keyb data av, bit 1=parin data av, bit 2=parout ready)
    map(0xfb, 0xfb) tape
    map(0xfc, 0xfc) keyboard data in
    map(0xfd, 0xfd) parallel data in, out
    map(0xfe, 0xfe) scroll register
    map(0xff, 0xff) sense switches */
}

/* Input ports */
static INPUT_PORTS_START( sol20 )
	PORT_START("ARROWS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("S1")
	PORT_DIPNAME( 0x04, 0x00, "Ctrl Chars")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x00, "Polarity")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x08, "Inverse")
	PORT_DIPNAME( 0x30, 0x10, "Cursor Type")
	PORT_DIPSETTING(    0x10, "Blinking")
	PORT_DIPSETTING(    0x20, "Solid")
	PORT_DIPSETTING(    0x30, DEF_STR(None))

	PORT_START("S2") // Sense Switches
	PORT_DIPNAME( 0x01, 0x01, "FF bit 0")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "FF bit 1")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "FF bit 2")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x08, "FF bit 3")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x10, "FF bit 4")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "FF bit 5")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "FF bit 6")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x80, "FF bit 7")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("S3")
	PORT_DIPNAME( 0xff, 0x08, "Baud Rate")
	PORT_DIPSETTING(    0x01, "75")
	PORT_DIPSETTING(    0x02, "110")
	PORT_DIPSETTING(    0x04, "180")
	PORT_DIPSETTING(    0x08, "300")
	PORT_DIPSETTING(    0x10, "600")
	PORT_DIPSETTING(    0x20, "1200")
	PORT_DIPSETTING(    0x40, "2400")
	PORT_DIPSETTING(    0x80, "4800/9600")

	PORT_START("S4")
	PORT_DIPNAME( 0x11, 0x10, "Parity")
	PORT_DIPSETTING(    0x00, "Even")
	PORT_DIPSETTING(    0x01, "Odd")
	PORT_DIPSETTING(    0x10, DEF_STR(None))
	PORT_DIPNAME( 0x06, 0x06, "Data Bits")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x02, "6")
	PORT_DIPSETTING(    0x04, "7")
	PORT_DIPSETTING(    0x06, "8")
	PORT_DIPNAME( 0x08, 0x08, "Stop Bits")
	PORT_DIPSETTING(    0x00, "1")
	PORT_DIPSETTING(    0x08, "2")
	PORT_DIPNAME( 0x20, 0x00, "Duplex")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x20, "Full")

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "High Baud Rate") // jumper on the board
	PORT_CONFSETTING(    0x00, "4800")
	PORT_CONFSETTING(    0x01, "9600")
	PORT_CONFNAME( 0x02, 0x00, "Character Rom")
	PORT_CONFSETTING(    0x00, "6574")
	PORT_CONFSETTING(    0x02, "6575")
	PORT_CONFNAME( 0x04, 0x04, "Field Rate") // modification described on page III-40 of Sol Systems Manual
	PORT_CONFSETTING(    0x00, "50 Hz")
	PORT_CONFSETTING(    0x04, "60 Hz")
	PORT_CONFNAME( 0x18, 0x18, "CPU Clock")
	PORT_CONFSETTING(    0x18, "2.04 MHz (8080A)")
	PORT_CONFSETTING(    0x10, "2.36 MHz (8080A-2)")
	PORT_CONFSETTING(    0x08, "2.86 MHz (8080A-1)")
INPUT_PORTS_END


void sol20_state::machine_start()
{
	m_cassette_timer = timer_alloc(TIMER_SOL20_CASSETTE_TC);
	save_item(NAME(m_sol20_fa));
	save_item(NAME(m_sol20_fc));
	save_item(NAME(m_sol20_fe));
	save_item(NAME(m_framecnt)); // unimportant
}

void sol20_state::machine_reset()
{
	u8 data = 0, s_count = 0;
	int s_clock;
	const u16 s_bauds[8]={ 75, 110, 180, 300, 600, 1200, 2400, 4800 };
	m_sol20_fe=0;
	m_sol20_fa=1;

	// set hard-wired uart pins
	m_uart->write_cs(0);
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_tsb(1);
	m_uart->write_eps(1);
	m_uart->write_np(1);
	m_uart->write_cs(1);

	// set switched uart pins
	data = m_iop_s4->read();
	m_uart_s->write_cs(0);
	m_uart_s->write_nb1(BIT(data, 1));
	m_uart_s->write_nb2(BIT(data, 2));
	m_uart_s->write_tsb(BIT(data, 3));
	m_uart_s->write_eps(BIT(data, 0));
	m_uart_s->write_np(BIT(data, 4));
	m_uart_s->write_cs(1);

	// set rs232 baud rate
	data = m_iop_s3->read();

	if (data > 1)
		do
		{
			s_count++;
			data >>= 1;
		}
		while (!(data & 1) && (s_count < 7)); // find which switch is used

	if ( (s_count == 7) & BIT(m_iop_config->read(), 0) ) // if highest, look at jumper
		s_clock = 9600 << 4;
	else
		s_clock = s_bauds[s_count] << 4;

	m_uart_s_clock->set_unscaled_clock(s_clock);

	m_rs232->write_dtr(0);
	m_rs232->write_rts(1);

	int lines = BIT(m_iop_config->read(), 2) ? 260 : 312;
	m_screen->configure(918, lines, m_screen->visible_area(), attotime::from_ticks(918 * lines, 14.318181_MHz_XTAL).as_attoseconds());

	// set CPU speed (TODO: also present on bus pin 49)
	double freq = (14.318181_MHz_XTAL / (4 + ((m_iop_config->read() >> 3) & 3))).dvalue();
	m_maincpu->set_unscaled_clock(freq);

	// Boot tap
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xc000, 0xc7ff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}


u32 sol20_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
// Visible screen is 64 x 16, with start position controlled by scroll register.
// Each character is 9 pixels wide (blank ones at the right) and 13 lines deep.
// Note on blinking characters:
// any character with bit 7 set will blink. With DPMON, do DA C000 C2FF to see what happens
	u16 which = (m_iop_config->read() & 2) << 10;
	u8 s1 = m_iop_s1->read();
	u16 sy=0;
	u8 polarity = (s1 & 8) ? 0xff : 0;

	bool cursor_inv = false;
	if (((s1 & 0x30) == 0x20) || (((s1 & 0x30) == 0x10) && (m_framecnt & 0x08)))
		cursor_inv = true;

	m_framecnt++;

	u16 ma = m_sol20_fe << 6; // scroll register

	for (u8 y = 0; y < 16; y++)
	{
		for (u8 ra = 0; ra < 13; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x++)
			{
				u16 inv = polarity;
				u8 chr = m_vram[x & 0x3ff];

				// cursor
				if (BIT(chr, 7) && cursor_inv)
					inv ^= 0xff;

				chr &= 0x7f;

				u8 gfx;
				if ((ra == 0) || ((s1 & 4) && (chr < 0x20)))
					gfx = inv;
				else if ((chr==0x2C) || (chr==0x3B) || (chr==0x67) || (chr==0x6A) || (chr==0x70) || (chr==0x71) || (chr==0x79))
				{
					if (ra < 4)
						gfx = inv;
					else
						gfx = m_p_chargen[which | (chr<<4) | (ra-4) ] ^ inv;
				}
				else
				{
					if (ra < 10)
						gfx = m_p_chargen[which | (chr<<4) | (ra-1) ] ^ inv;
					else
						gfx = inv;
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
				*p++ = BIT(inv, 0);
			}
		}
		ma+=64;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout sol20_charlayout =
{
	7, 9,                   /* 7 x 9 characters */
	128*2,                  /* 128 characters per rom */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_sol20 )
	GFXDECODE_ENTRY( "chargen", 0x0000, sol20_charlayout, 0, 1 )
GFXDECODE_END

void sol20_state::kbd_put(u8 data)
{
	if (data)
	{
		m_sol20_fa &= 0xfe;
		m_sol20_fc = data;
	}
}

void sol20_state::sol20(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 14.318181_MHz_XTAL / 7); // divider selectable as 5, 6 or 7 through jumpers
	m_maincpu->set_addrmap(AS_PROGRAM, &sol20_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sol20_state::io_map);
	m_maincpu->out_inte_func().set("speaker", FUNC(speaker_sound_device::level_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL, 918, 0, 576, 260, 0, 208);
	m_screen->set_screen_update(FUNC(sol20_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_sol20);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 2.00); // music board

	// devices
	CASSETTE(config, m_cass1).set_formats(sol20_cassette_formats);
	m_cass1->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass1->add_route(ALL_OUTPUTS, "mono", 0.05); // cass1 speaker
	m_cass1->set_interface("sol20_cass");

	CASSETTE(config, m_cass2).set_formats(sol20_cassette_formats);
	m_cass2->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass2->add_route(ALL_OUTPUTS, "mono", 0.05); // cass2 speaker
	m_cass2->set_interface("sol20_cass");

	AY51013(config, m_uart); // TMS6011NC
	m_uart->set_auto_rdav(true); // ROD (pin 4) tied to RDD (pin 18)

	CLOCK(config, m_uart_clock, 4800);
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay51013_device::write_rcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay51013_device::write_tcp));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	AY51013(config, m_uart_s); // TMS6011NC
	m_uart_s->read_si_callback().set(m_rs232, FUNC(rs232_port_device::rxd_r));
	m_uart_s->write_so_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_uart_s->set_auto_rdav(true); // ROD (pin 4) tied to RDD (pin 18)

	CLOCK(config, m_uart_s_clock, 4800);
	m_uart_s_clock->signal_handler().set(m_uart_s, FUNC(ay51013_device::write_rcp));
	m_uart_s_clock->signal_handler().append(m_uart_s, FUNC(ay51013_device::write_tcp));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(sol20_state::kbd_put));

	SOFTWARE_LIST(config, "cass_list").set_original("sol20_cass");
}

/* ROM definition */
ROM_START( sol20 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "solos", "SOLOS")
	ROMX_LOAD( "solos.bin", 0x0000, 0x0800, CRC(4d0af383) SHA1(ac4510c3380ed4a31ccf4f538af3cb66b76701ef), ROM_BIOS(0) )    // from solace emu
	ROM_SYSTEM_BIOS(1, "dpmon", "DPMON")
	ROMX_LOAD( "dpmon.bin", 0x0000, 0x0800, BAD_DUMP CRC(2a84f099) SHA1(60ff6e38082c50afcf0f40707ef65668a411008b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "consol", "CONSOL")
	ROMX_LOAD( "consol.bin", 0x0000, 0x0400, BAD_DUMP CRC(80bf6d85) SHA1(84b81c60bb08a3a5435ec1be56a67aa695bce099), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "solos2", "Solos Patched")
	ROMX_LOAD( "solos2.bin", 0x0000, 0x0800, CRC(7776cc7d) SHA1(c4739a9ea7e8146ce7ae3305ed526b6045efa9d6), ROM_BIOS(3) ) // from Nama
	ROM_SYSTEM_BIOS(4, "bootload", "BOOTLOAD")
	ROMX_LOAD( "bootload.bin", 0x0000, 0x0800, BAD_DUMP CRC(4261ac71) SHA1(4752408ac85d88857e8e9171c7f42bd623c9271e), ROM_BIOS(4) ) // from Nama
	// This one doesn't work
	ROM_SYSTEM_BIOS(5, "cuter", "CUTER")
	ROMX_LOAD( "cuter.bin", 0x0000, 0x0800, BAD_DUMP CRC(39cca901) SHA1(33725d6da63e295552ee13f0a735d33aee8f0d17), ROM_BIOS(5) ) // from Nama

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "6574.bin", 0x0000, 0x0800, BAD_DUMP CRC(fd75df4f) SHA1(4d09aae2f933478532b7d3d1a2dee7123d9828ca) )
	ROM_LOAD( "6575.bin", 0x0800, 0x0800, BAD_DUMP CRC(cfdb76c2) SHA1(ab00798161d13f07bee3cf0e0070a2f0a805591f) )

	ROM_REGION( 0x100, "keyboard", 0 )
	ROM_LOAD( "8574.u18", 0x000, 0x100, NO_DUMP ) // 256x4 bipolar PROM or mask ROM; second half unused
ROM_END

} // anonymous namespace

/* Driver */
//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                             FULLNAME                    FLAGS
COMP( 1976, sol20, 0,      0,      sol20,   sol20, sol20_state, empty_init, "Processor Technology Corporation", "Sol-20 Terminal Computer", MACHINE_SUPPORTS_SAVE )
