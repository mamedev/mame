// license:BSD-3-Clause
// copyright-holders:Roberto Lavarone
/******************************************************************************

    Nuova Elettronica Z80NE system driver

    Preliminary driver

    Roberto Lavarone, 2009-01-05

    Thanks go to:
        Roberto Bazzano: http://www.z80ne.com/

    Z80NE memory map

        LX.382 CPU board
            range     short     description
            0000-03FF RAM
            8000-83FF EPROM     firmware

        LX.383 HEX keyboard and display controller for LX.384 hex keyboard and display
            range     short     description
            F0-F7     I/O       7-segment LED dual-port RAM write
            F0        I/O       keyboard read
            F8        I/O       enable single step for next instruction

        LX.385 Cassette interface
            range     short     description
            EE        I/O       UART Data Read/Write
            EF        I/O       UART Status/Control - Cassette Tape Control

        LX.392 32K Memory expansion board
            range     short     description

        LX.389 Printer Interface
            range     short     description
            02-03
            06-07
            0A-0B
            0E-0F
            12-13
            16-17
            1A-1B
            1E-1F

        LX.548 16K Basic eprom
            range     short     description
            0000-3FFF EPROM     firmware

        LX.388 Video Interface
            range     short     description
            EC00-EDFF RAM
            EA        I/O       keyboard
            EB        I/O       video retrace

        LX.529 Graphics Video and Printer Interface
            range     short     description
            80        I/O       PIO 0 port A data (ram 0)
            81        I/O       PIO 0 port A control (ram 0)
            82        I/O       PIO 0 port B data (printer)
            83        I/O       PIO 0 port B control (printer)
            84        I/O       PIO 1 port A data (ram 1)
            85        I/O       PIO 1 port A control (ram 1)
            86        I/O       PIO 1 port B data (keyboard)
            87        I/O       PIO 1 port B control (keyboard)
            88        I/O       PIO 2 port A data (ram 2)
            89        I/O       PIO 2 port A control (ram 2)
            8A        I/O       PIO 2 port B data (printer busy + 40/80 video chars)
            8B        I/O       PIO 2 port B control (printer busy + 40/40 video chars)
            8C        I/O       SY6845 address and status register
            8D        I/O       SY6845 data register
            8E        I/O       RAM 3 character attribute
            8F        I/O       beeper

        LX.390 Floppy Interface
            range     short     description
            F000-F3FF EPROM     firmware
            D0        I/O       command/status register
            D1        I/O       trace register
            D2        I/O       sector register
            D3        I/O       data register (write only if controller idle)
            D6        I/O       drive select / drive side one select
            D7        I/O       data register (write always)

        LX.394-395 EPROM Programmer
            range     short     description
            9000-9FFF EPROM     EPROM to be written
            8400-8FFF EPROM     firmware

Quick Instructions:
  Z80NE:
  - Use 0-F to enter an address (or use mouse in artwork)
  - Hold CTRL press 0 to show data at that address (CTRL cannot be held with mouse)
  - Use 0-F to enter data
  - CTRL 0 to advance to next address
  - CTRL 2 to view/change CPU registers
  - CTRL 0 change register, advance to next
  Z80NET
  - In Machine Configuration, select your preferred baud rate, and reboot machine.
  - CTRL 6 to load a tape, press A or B to choose tape device, choose Play.
  - After this, click any key, enter 1000, CTRL 4 to run.
  - CTRL 5 to save
  - All tapes in software list (except bioritmi & tapebas) are BASIC programs.
  - To use tapebas, load side 1 in the normal way, run it, select side 2 for cas 0, play, the rest loads
  - The Basic is bilingual - ENG for English, ITA for Italian, so enter ENG.
  - Then for any Basic program in software list: CLOAD hit Play RUN
  Z80NETB
  - BASIC-only, English only. A version of TRS80 Level II Basic. Not compatible with software list.
  Z80NETF
  - There is a choice of 5 bioses, via the Machine Configuration menu
  - EP382:  same as Z80NET.
  - EP548:  same as Z80NETB.
  - EP390:  for swlist-item "basic55k". Press B, from the menu select 2, runs. Type ENG for English.
            It can load and run tapes from the swlist, although it seems to often have load errors.
  - EP1390: requires a floppy to boot from. Disks marked as NE-DOS 1.5 should work.
  - EP2390: uses ports 8x, not emulated, not working. For NE-DOS G.1

Natural Keyboard and Paste:
    - The hexpad keys conflict with the keyboard keys, therefore Natural Keyboard is not
      supported. Paste is meant for Z80NE only.

    - Test Paste:
      N1000=11=22=33=44=55=66=77=88=99=N1000=
      Press Ctrl+0 to review the data.

*********************************************************************************************************/

/* Core includes */
#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/kr2376.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "video/mc6847.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/dmk_dsk.h"

//#define VERBOSE 1
#include "logmacro.h"

/* Layout */
#include "z80ne.lh"
#include "z80net.lh"
#include "z80netb.lh"
#include "z80netf.lh"


/******************************************************************************
 Constants
******************************************************************************/

namespace {

#define Z80NE_CPU_SPEED_HZ      1920000 /* 1.92 MHz */

#define LX383_KEYS              16
#define LX383_DOWNSAMPLING      16

#define LX385_TAPE_SAMPLE_FREQ  38400

/* wave duration threshold */
enum z80netape_speed
{
	TAPE_300BPS  = 300, /*  300 bps */
	TAPE_600BPS  = 600, /*  600 bps */
	TAPE_1200BPS = 1200 /* 1200 bps */
};

struct z80ne_cass_data_t {
	struct {
		int length = 0;     /* time cassette level is at input.level */
		int level = 0;      /* cassette level */
		int bit = 0;        /* bit being read */
	} input;
	struct {
		int length = 0;     /* time cassette level is at output.level */
		int level = 0;      /* cassette level */
		int bit = 0;        /* bit to output */
	} output;
	z80netape_speed speed;  /* 300 - 600 - 1200 */
	int wave_filter = 0;
	int wave_length = 0;
	int wave_short = 0;
	int wave_long = 0;
};


class z80ne_state : public driver_device
{
public:
	z80ne_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_uart(*this, "uart"),
		m_uart_clock(*this, "uart_clock"),
		m_maincpu(*this, "maincpu"),
		m_cassette1(*this, "cassette"),
		m_cassette2(*this, "cassette2"),
		m_ram(*this, RAM_TAG),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_ctrl(*this, "CTRL"),
		m_io_rst(*this, "RST"),
		m_io_lx_385(*this, "LX.385"),
		m_lx383_digits(*this, "digit%u", 0U),
		m_rom(*this, "maincpu"),
		m_mram(*this, "mainram")
	{ }

	void z80ne(machine_config &config);
	void init_z80ne();

	DECLARE_INPUT_CHANGED_MEMBER(z80ne_reset);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void base_reset();
	void save_state_vars();

	static void floppy_formats(format_registration &fr);

	uint8_t m_lx383_scan_counter = 0;
	uint8_t m_lx383_key[LX383_KEYS]{};
	int m_lx383_downsampler = 0;
	uint8_t m_lx385_ctrl = 0;
	emu_timer *m_cassette_timer = nullptr;
	emu_timer *m_kbd_timer = nullptr;
	z80ne_cass_data_t m_cass_data;

	uint8_t lx383_r();
	void lx383_w(offs_t offset, uint8_t data);
	uint8_t lx385_ctrl_r();
	void lx385_ctrl_w(uint8_t data);
	void lx385_uart_tx_clock_w(int state);

	TIMER_CALLBACK_MEMBER(cassette_tc);
	TIMER_CALLBACK_MEMBER(kbd_scan);
	TIMER_CALLBACK_MEMBER(pulse_nmi);

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<ay31015_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	optional_device<ram_device> m_ram;
	optional_memory_bank m_bank1;
	optional_memory_bank m_bank2;
	optional_memory_bank m_bank3;
	optional_memory_bank m_bank4;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_ctrl;
	required_ioport m_io_rst;
	required_ioport m_io_lx_385;
	output_finder<8> m_lx383_digits;
	required_region_ptr<u8> m_rom;
	optional_shared_ptr<u8> m_mram;

	emu_timer *m_timer_nmi = nullptr;

	cassette_image_device *cassette_device_image();

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};

class z80net_state : public z80ne_state
{
public:
	z80net_state(const machine_config &mconfig, device_type type, const char *tag) :
		z80ne_state(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_vdg(*this, "mc6847"),
		m_lx387_kr2376(*this, "lx387_kr2376"),
		m_io_lx387_brk(*this, "LX387_BRK"),
		m_io_modifiers(*this, "MODIFIERS")
	{
	}

	void lx387(machine_config &config);
	void z80net(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(z80net_nmi);

protected:
	virtual void machine_reset() override ATTR_COLD;

	int lx387_shift_r();
	int lx387_control_r();
	uint8_t lx387_data_r();
	uint8_t lx388_mc6847_videoram_r(offs_t offset);
	uint8_t lx388_read_field_sync();

	required_shared_ptr<uint8_t> m_videoram;
	required_device<mc6847_base_device> m_vdg;
	required_device<kr2376_device> m_lx387_kr2376;
	required_ioport m_io_lx387_brk;
	required_ioport m_io_modifiers;

	void reset_lx387();

	void io_map(address_map &map) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
};

class z80netb_state : public z80net_state
{
public:
	z80netb_state(const machine_config &mconfig, device_type type, const char *tag) :
		z80net_state(mconfig, type, tag)
	{
	}

	void z80netb(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
};

class z80netf_state : public z80netb_state
{
public:
	z80netf_state(const machine_config &mconfig, device_type type, const char *tag) :
		z80netb_state(mconfig, type, tag),
		m_io_config(*this, "CONFIG"),
		m_floppy(*this, "wd1771:%u", 0U),
		m_wd1771(*this, "wd1771"),
		m_drv_led(*this, "drv%u", 0U)
	{
	}

	void z80netf(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void driver_start() override;

	struct wd17xx_state_t
	{
		int drq = 0;
		int intrq = 0;
		uint8_t drive = 0; /* current drive */
		uint8_t head = 0;  /* current head */
	};

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void lx390_motor_w(uint8_t data);
	uint8_t lx390_fdc_r(offs_t offset);
	void lx390_fdc_w(offs_t offset, uint8_t data);

	void reset_lx390_banking();

	required_ioport m_io_config;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<fd1771_device> m_wd1771;
	wd17xx_state_t m_wd17xx_state;
	output_finder<2> m_drv_led;
};



/******************************************************************************
 I/O
******************************************************************************/

/* timer to read cassette waveforms */

cassette_image_device* z80ne_state::cassette_device_image()
{
	if (m_lx385_ctrl & 0x08)
		return m_cassette2;
	else
		return m_cassette1;
}

TIMER_CALLBACK_MEMBER(z80ne_state::cassette_tc)
{
	uint8_t cass_ws = 0;
	m_cass_data.input.length++;

	cass_ws = ((cassette_device_image())->input() > +0.02) ? 1 : 0;

	if ((cass_ws ^ m_cass_data.input.level) & cass_ws)
	{
		m_cass_data.input.level = cass_ws;
		m_cass_data.input.bit = ((m_cass_data.input.length < m_cass_data.wave_filter) || (m_cass_data.input.length > 0x20)) ? 1 : 0;
		m_cass_data.input.length = 0;
		m_uart->write_si(m_cass_data.input.bit);
	}
	m_cass_data.input.level = cass_ws;

	/* saving a tape - convert the serial stream from the uart */
	m_cass_data.output.length--;

	if (!(m_cass_data.output.length))
	{
		if (m_cass_data.output.level)
			m_cass_data.output.level = 0;
		else
		{
			m_cass_data.output.level=1;
			cass_ws = m_uart->so_r();
			m_cass_data.wave_length = cass_ws ? m_cass_data.wave_short : m_cass_data.wave_long;
		}
		cassette_device_image()->output(m_cass_data.output.level ? -1.0 : +1.0);
		m_cass_data.output.length = m_cass_data.wave_length;
	}
}

void z80ne_state::save_state_vars()
{
	save_item(NAME(m_lx383_scan_counter));
	save_item(NAME(m_lx383_key));
	save_item(NAME(m_lx383_downsampler));
	save_item(NAME(m_lx385_ctrl));
}

void z80ne_state::init_z80ne()
{
	save_state_vars();
}

void z80netf_state::driver_start()
{
	save_state_vars();

	/* first two entries point to rom on reset */
	u8 *r = m_ram->pointer();
	m_bank1->configure_entry(0, r); /* RAM   at 0x0000-0x03FF */
	m_bank1->configure_entries(1, 3, m_rom+0x4400, 0x0400); /* ep390, ep1390, ep2390 at 0x0000-0x03FF */
	m_bank1->configure_entry(4, m_rom+0x4000); /* ep382 at 0x0000-0x03FF */
	m_bank1->configure_entry(5, m_rom); /* ep548 at 0x0000-0x03FF */

	m_bank2->configure_entry(0, r+0x0400); /* RAM   at 0x0400 */
	m_bank2->configure_entry(1, m_rom+0x0400); /* ep548 at 0x0400-0x3FFF */

	m_bank3->configure_entry(0, r+0x4000); /* RAM   at 0x8000 */
	m_bank3->configure_entry(1, m_rom+0x4000); /* ep382 at 0x8000 */

	m_bank4->configure_entry(0, r+0x5000); /* RAM   at 0xF000 */
	m_bank4->configure_entries(1, 3, m_rom+0x4400, 0x0400); /* ep390, ep1390, ep2390 at 0xF000 */

}

TIMER_CALLBACK_MEMBER(z80ne_state::kbd_scan)
{
	/*
	 * NE555 is connected to a 74LS93 binary counter
	 * 74LS93 output:
	 *   QA-QC: column index for LEDs and keyboard
	 *   QD:    keyboard row select
	 *
	 * Port F0 input bit assignment:
	 *   0  QA  bits 0..3 of row counter
	 *   1  QB
	 *   2  QC
	 *   3  QD
	 *   4  Control button pressed, active high
	 *   5  Always low
	 *   6  Always low
	 *   7  Selected button pressed, active low
	 *
	 *
	 */

	uint16_t key_bits;
	uint8_t ctrl; //, rst;
	uint8_t i;

	/* 4-bit counter */
	--m_lx383_scan_counter;
	m_lx383_scan_counter &= 0x0f;

	if ( --m_lx383_downsampler == 0 )
	{
		m_lx383_downsampler = LX383_DOWNSAMPLING;
		key_bits = (m_io_row1->read() << 8) | m_io_row0->read();
//      rst = m_io_rst->read();
		ctrl = m_io_ctrl->read();

		for ( i = 0; i<LX383_KEYS; i++)
		{
			m_lx383_key[i] = ( i | (key_bits & 0x01 ? 0x80 : 0x00) | ~ctrl);
			key_bits >>= 1;
		}
	}
}

TIMER_CALLBACK_MEMBER(z80ne_state::pulse_nmi)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void z80net_state::reset_lx387()
{
	m_lx387_kr2376->set_input_pin( kr2376_device::KR2376_DSII, 0);
	m_lx387_kr2376->set_input_pin( kr2376_device::KR2376_PII, 0);
}

void z80netf_state::reset_lx390_banking()
{
	switch (m_io_config->read() & 0x07)
	{
	case 0x01: /* EP382 Hex Monitor */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep382\n");
		m_bank1->set_entry(4);  /* ep382 at 0x0000 for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400 */
		m_bank3->set_entry(1);  /* ep382 at 0x8000 */
		m_bank4->set_entry(0);  /* RAM   at 0xF000 */
		break;
	case 0x02: /* EP548  16k BASIC */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep548\n");
		m_bank1->set_entry(5);  /* ep548 at 0x0000-0x03FF */
		m_bank2->set_entry(1);  /* ep548 at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(0);  /* RAM   at 0xF000 */
		break;
	case 0x03: /* EP390  Boot Loader for 5.5k floppy BASIC */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep390\n");
		m_bank1->set_entry(1);  /* ep390 at 0x0000-0 x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(1);  /* ep390 at 0xF000 */
		break;
	case 0x04: /* EP1390 Boot Loader for NE DOS 1.0/1.5 */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep1390\n");
		m_bank1->set_entry(2);  /* ep1390 at 0x0000-0x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(2);  /* ep1390 at 0xF000 */
		break;
	case 0x05: /* EP2390 Boot Loader for NE DOS G.1 */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep2390\n");
		m_bank1->set_entry(3);  /* ep2390 at 0x0000-0x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(3);  /* ep2390 at 0xF000 */
		break;
	}

	/* TODO: in real hardware the ENH bus line is pulled down
	 * until a I/O read is performed on a address with A0 address bit low and A1 or A2 address bit high
	 */
}

void z80ne_state::base_reset()
{
	for (int i = 0; i < LX383_KEYS; i++)
		m_lx383_key[i] = 0xf0 | i;
	m_lx383_scan_counter = 0x0f;
	m_lx383_downsampler = LX383_DOWNSAMPLING;

	/* Initialize cassette interface */
	switch(m_io_lx_385->read() & 0x07)
	{
	case 0x01:
		m_cass_data.speed = TAPE_300BPS;
		m_cass_data.wave_filter = LX385_TAPE_SAMPLE_FREQ / 1600;
		m_cass_data.wave_short = LX385_TAPE_SAMPLE_FREQ / (2400 * 2);
		m_cass_data.wave_long = LX385_TAPE_SAMPLE_FREQ / (1200 * 2);
		break;
	case 0x02:
		m_cass_data.speed = TAPE_600BPS;
		m_cass_data.wave_filter = LX385_TAPE_SAMPLE_FREQ / 3200;
		m_cass_data.wave_short = LX385_TAPE_SAMPLE_FREQ / (4800 * 2);
		m_cass_data.wave_long = LX385_TAPE_SAMPLE_FREQ / (2400 * 2);
		break;
	case 0x04:
		m_cass_data.speed = TAPE_1200BPS;
		m_cass_data.wave_filter = LX385_TAPE_SAMPLE_FREQ / 6400;
		m_cass_data.wave_short = LX385_TAPE_SAMPLE_FREQ / (9600 * 2);
		m_cass_data.wave_long = LX385_TAPE_SAMPLE_FREQ / (4800 * 2);
	}
	m_cass_data.wave_length = m_cass_data.wave_short;
	m_cass_data.output.length = m_cass_data.wave_length;
	m_cass_data.output.level = 1;
	m_cass_data.input.length = 0;
	m_cass_data.input.bit = 1;

	m_uart->write_cs(0);
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_tsb(1);
	m_uart->write_eps(1);
	m_uart->write_np(m_io_lx_385->read() & 0x80 ? 1 : 0);
	m_uart->write_cs(1);
	m_uart_clock->set_unscaled_clock(m_cass_data.speed * 16);

	lx385_ctrl_w(0);
}

void z80ne_state::machine_reset()
{
	base_reset();

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x03ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0x8000, 0x83ff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x03ff, m_mram);
				}
			},
			&m_rom_shadow_tap);
}

void z80net_state::machine_reset()
{
	reset_lx387();
	z80ne_state::machine_reset();
}

void z80netb_state::machine_reset()
{
	base_reset();
	reset_lx387();
}

void z80netf_state::machine_reset()
{
	reset_lx390_banking();
	base_reset();
	reset_lx387();

	// basic roms are exempt from memory tap
	if ((m_io_config->read() & 0x07) != 2)
	{
		address_space &program = m_maincpu->space(AS_PROGRAM);
		m_rom_shadow_tap.remove();
		m_rom_shadow_tap = program.install_read_tap(
				0x8000, 0xf3ff,
				"rom_shadow_r",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					if (!machine().side_effects_disabled())
					{
						// delete this tap
						m_rom_shadow_tap.remove();

						// reinstall RAM over the ROM shadow
						m_bank1->set_entry(0);
					}
				},
				&m_rom_shadow_tap);
	}
}

INPUT_CHANGED_MEMBER(z80ne_state::z80ne_reset)
{
	uint8_t rst = m_io_rst->read();

	if ( ! BIT(rst, 0))
		machine().schedule_soft_reset();
}

INPUT_CHANGED_MEMBER(z80net_state::z80net_nmi)
{
	uint8_t nmi = m_io_lx387_brk->read();

	if ( ! BIT(nmi, 0))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void z80ne_state::machine_start()
{
	m_timer_nmi = timer_alloc(FUNC(z80ne_state::pulse_nmi), this);

	m_lx383_digits.resolve();

	m_lx385_ctrl = 0x1f;
	m_cassette_timer = timer_alloc(FUNC(z80ne_state::cassette_tc), this);
	m_kbd_timer = timer_alloc(FUNC(z80ne_state::kbd_scan), this);
	m_kbd_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));
}

void z80netf_state::machine_start()
{
	z80ne_state::machine_start();
	m_drv_led.resolve();
}



/******************************************************************************
 Drivers
******************************************************************************/

/* LX.383 - LX.384 HEX keyboard and display */
uint8_t z80ne_state::lx383_r()
{
	/*
	 * Keyboard scanning
	 *
	 * IC14 NE555 astable oscillator
	 * IC13 74LS93 binary counter
	 * IC5  74LS240 tri-state buffer
	 *
	 * l'oscillatore NE555 alimenta il clock del contatore 74LS93
	 *      D0 - Q(A) --\
	 *      D1 - Q(B)    |-- column
	 *      D2 - Q(C) --/
	 *      D3 - Q(D)        row
	 *      D4 - CTRL
	 *      D5 - 0
	 *      D6 - 0
	 *      D7 - ~KEY Pressed
	 */
	return m_lx383_key[m_lx383_scan_counter];
}

void z80ne_state::lx383_w(offs_t offset, uint8_t data)
{
	/*
	 * First 8 locations (F0-F7) are mapped to a dual-port 8-byte RAM
	 * The 1KHz NE-555 astable oscillator circuit drive
	 * a 4-bit 74LS93 binary counter.
	 * The 3 least significant bits of the counter are connected
	 * both to the read address of the dual-port ram and to
	 * a 74LS156 3 to 8 binary decoder driving the cathode
	 * of 8 7-segments LEDS.
	 * The data output of the dual-port ram drive the anodes
	 * of the LEDS through 74LS07 buffers.
	 * LED segments - dual-port RAM bit:
	 *   A   0x01
	 *   B   0x02
	 *   C   0x04
	 *   D   0x08
	 *   E   0x10
	 *   F   0x20
	 *   G   0x40
	 *   P   0x80 (represented by DP in original schematics)
	 *
	 *   A write in the range F0-FF starts a 74LS90 counter
	 *   that trigger the NMI line of the CPU after 2 instruction
	 *   fetch cycles for single step execution.
	 */

	if ( offset < 8 )
		m_lx383_digits[offset] = data ^ 0xff;
	else
	{
		// after writing to port 0xF8 and the first ~M1 cycles strike a NMI for single step execution
		m_timer_nmi->adjust(m_maincpu->cycles_to_attotime(1));
	}
}


/* LX.385 Cassette tape interface */
/*
 * NE555 is connected to a 74LS93 binary counter
 * 74LS93 output:
 *   QA-QC: column index for LEDs and keyboard
 *   QD:    keyboard row select
 *
 * Port EE: UART Data Read/Write
 * Port EF: Status/Control
 *     read, UART status bits read
 *         0 OR   Overrun
 *         1 FE   Framing Error
 *         2 PE   Parity Error
 *         3 TBMT Transmitter Buffer Empty
 *         4 DAV  Data Available
 *         5 EOC  End Of Character
 *         6 1
 *         7 1
 *     write, UART control bits / Tape Unit select / Modulation control
 *         0 bit1=0, bit0=0   UART Reset pulse
 *         1 bit1=0, bit0=1   UART RDAV (Reset Data Available) pulse
 *         2 Tape modulation enable
 *         3 *TAPEA Enable (active low) (at reset: low)
 *         4 *TAPEB Enable (active low) (at reset: low)
 *  Cassette is connected to the uart data input and output via the cassette
 *  interface hardware.
 *
 *  The cassette interface hardware converts square-wave pulses into bits which the uart receives.
 *
 *  1. the cassette format: "frequency shift" is converted
    into the uart data format "non-return to zero"

    2. on cassette a 1 data bit is stored as 8 2400 Hz pulses
    and a 0 data bit as 4 1200 Hz pulses
    - At 1200 baud, a logic 1 is 1 cycle of 1200 Hz and a logic 0 is 1/2 cycle of 600 Hz.
    - At 300 baud, a logic 1 is 8 cycles of 2400 Hz and a logic 0 is 4 cycles of 1200 Hz.

    Attenuation is applied to the signal and the square wave edges are rounded.

    A manchester encoder is used. A flip-flop synchronises input
    data on the positive-edge of the clock pulse.

    The UART is a RCA CDP1854 CMOS device with pin 2 jumpered to GND to select the
    AY-3-1015 compatibility mode. The jumper at P4 can be switched to place 12 V on
    pin 2 for an old PMOS UART.
 *
 */
uint8_t z80ne_state::lx385_ctrl_r()
{
	/* set unused bits high */
	uint8_t data = 0xc0;

	m_uart->write_swe(0);
	data |= (m_uart->or_r(  ) ? 0x01 : 0);
	data |= (m_uart->fe_r(  ) ? 0x02 : 0);
	data |= (m_uart->pe_r(  ) ? 0x04 : 0);
	data |= (m_uart->tbmt_r() ? 0x08 : 0);
	data |= (m_uart->dav_r( ) ? 0x10 : 0);
	data |= (m_uart->eoc_r( ) ? 0x20 : 0);
	m_uart->write_swe(1);

	return data;
}

#define LX385_CASSETTE_MOTOR_MASK ((1<<3)|(1<<4))

void z80ne_state::lx385_ctrl_w(uint8_t data)
{
	/* Translate data to control signals
	 *     0 bit1=0, bit0=0   UART Reset pulse
	 *     1 bit1=0, bit0=1   UART RDAV (Reset Data Available) pulse
	 *     2 UART Tx Clock Enable (active high)
	 *     3 *TAPEA Enable (active low) (at reset: low)
	 *     4 *TAPEB Enable (active low) (at reset: low)
	 */
	uint8_t uart_reset, uart_rdav;
	uint8_t motor_a, motor_b;
	uint8_t changed_bits = (m_lx385_ctrl ^ data) & 0x1C;
	m_lx385_ctrl = data;

	uart_reset = ((data & 0x03) == 0x00);
	uart_rdav  = ((data & 0x03) == 0x01);
	motor_a = ((data & 0x08) == 0x00);
	motor_b = ((data & 0x10) == 0x00);

	/* UART Reset and RDAV */
	if (uart_reset)
	{
		m_uart->write_xr(1);
		m_uart->write_xr(0);
	}

	if (uart_rdav)
	{
		m_uart->write_rdav(1);
		m_uart->write_rdav(0);
	}

	if (!changed_bits) return;

	/* motors */
	if(changed_bits & 0x18)
	{
		m_cassette1->change_state(
			(motor_a) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

		m_cassette2->change_state(
			(motor_b) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

		if (motor_a || motor_b)
			m_cassette_timer->adjust(attotime::zero, 0, attotime::from_hz(LX385_TAPE_SAMPLE_FREQ));
		else
			m_cassette_timer->adjust(attotime::zero);
	}
}

void z80ne_state::lx385_uart_tx_clock_w(int state)
{
	if (BIT(m_lx385_ctrl, 2))
		m_uart->write_tcp(state);
}

int z80net_state::lx387_shift_r()
{
	return BIT(m_io_modifiers->read(), 0) || BIT(m_io_modifiers->read(), 2);
}

int z80net_state::lx387_control_r()
{
	return BIT(m_io_modifiers->read(), 1);
}

uint8_t z80net_state::lx388_mc6847_videoram_r(offs_t offset)
{
	if (offset == ~0) return 0xff;

	int d6 = BIT(m_videoram[offset], 6);
	int d7 = BIT(m_videoram[offset], 7);

	m_vdg->inv_w(d6 && d7);
	m_vdg->as_w(!d6 && d7);
	m_vdg->intext_w(!d6 && d7);

	return m_videoram[offset];
}

uint8_t z80net_state::lx387_data_r()
{
	uint8_t data = m_lx387_kr2376->data_r() & 0x7f;
	data |= m_lx387_kr2376->get_output_pin(kr2376_device::KR2376_SO) << 7;
	return data;
}

uint8_t z80net_state::lx388_read_field_sync()
{
	return m_vdg->fs_r() << 7;
}

/*
 * DRQ INTRQ IC9B.10 IC8B.*Q
 *  0    0     1       0
 *  0    1     0       x
 *  1    0     0       x
 *  1    1     0       x
 *
 */

void z80netf_state::lx390_motor_w(uint8_t data)
{
	/* Selection of drive and parameters
	 A write also causes the selected drive motor to turn on for about 3 seconds.
	 When the motor turns off, the drive is deselected.
	    d7 Unused             (trs80: 1=MFM, 0=FM)
	    d6 (trs80: 1=Wait)
	    d5 0=Side 0, 1=Side 1 (trs80: 1=Write Precompensation enabled)
	    d4 Unused             (trs80: 0=Side 0, 1=Side 1)
	    d3 1=select drive 3
	    d2 1=select drive 2
	    d1 1=select drive 1
	    d0 1=select drive 0 */

	floppy_image_device *floppy = nullptr;

	for (u8 f = 0; f < 4; f++)
		if (BIT(data, f))
			floppy = m_floppy[f]->get_device();

	m_wd1771->set_floppy(floppy);

	if (floppy)
	{
		floppy->ss_w(BIT(data, 5));
		floppy->mon_w(0);
	}

	m_wd17xx_state.head = (data & 32) ? 1 : 0;
	m_wd17xx_state.drive = data & 0x0F;

	/* no drive selected, turn off all leds */
	if (!m_wd17xx_state.drive)
	{
		m_drv_led[0] = 0;
		m_drv_led[1] = 0;
	}
}

uint8_t z80netf_state::lx390_fdc_r(offs_t offset)
{
	uint8_t d;

	switch(offset)
	{
	case 0:
		d = m_wd1771->status_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx status: %02x\n", d);
		break;
	case 1:
		d = m_wd1771->track_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx track:  %02x\n", d);
		break;
	case 2:
		d = m_wd1771->sector_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx sector: %02x\n", d);
		break;
	case 3:
		d = m_wd1771->data_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx data3:  %02x\n", d);
		break;
	case 6:
		d = 0xff;
		m_bank1->set_entry(0);
		break;
	case 7:
		d = m_wd1771->data_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx data7, force:  %02x\n", d);
		break;
	default:
		d = 0x00;
	}
	return d;
}

void z80netf_state::lx390_fdc_w(offs_t offset, uint8_t data)
{
	uint8_t d = data;
	switch(offset)
	{
	case 0:
		LOG("lx390_fdc_w, WD17xx command: %02x\n", d);
		m_wd1771->cmd_w(d ^ 0xff);
		if (m_wd17xx_state.drive & 1)
			m_drv_led[0] = 2;
		else if (m_wd17xx_state.drive & 2)
			m_drv_led[1] = 2;
		break;
	case 1:
		LOG("lx390_fdc_w, WD17xx track:   %02x\n", d);
		m_wd1771->track_w(d ^ 0xff);
		break;
	case 2:
		LOG("lx390_fdc_w, WD17xx sector:  %02x\n", d);
		m_wd1771->sector_w(d ^ 0xff);
		break;
	case 3:
		m_wd1771->data_w(d ^ 0xff);
		LOG("lx390_fdc_w, WD17xx data3:   %02x\n", d);
		break;
	case 6:
		LOG("lx390_fdc_w, motor_w:   %02x\n", d);
		lx390_motor_w(d);
		break;
	case 7:
		LOG("lx390_fdc_w, WD17xx data7, force:   %02x\n", d);
		m_wd1771->data_w(d ^ 0xff);
		break;
	}
}



/******************************************************************************
 Memory Maps
******************************************************************************/

/* LX.382 CPU Board RAM */
/* LX.382 CPU Board EPROM */
void z80ne_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("mainram");
	map(0x8000, 0x83ff).rom().region("maincpu",0);
}

void z80net_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("mainram");
	map(0x8000, 0x83ff).rom().region("maincpu",0);
	map(0x8400, 0xebff).ram();
	map(0xec00, 0xedff).ram().share("videoram"); /* (6847) */
	map(0xee00, 0xffff).ram();
}

void z80netb_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xebff).ram();
	map(0xec00, 0xedff).ram().share("videoram"); /* (6847) */
	map(0xee00, 0xffff).ram();
}

void z80ne_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xee, 0xee).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xef, 0xef).rw(FUNC(z80ne_state::lx385_ctrl_r), FUNC(z80ne_state::lx385_ctrl_w));
	map(0xf0, 0xff).rw(FUNC(z80ne_state::lx383_r), FUNC(z80ne_state::lx383_w));
}

void z80net_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xea, 0xea).r(FUNC(z80net_state::lx387_data_r));
	map(0xeb, 0xeb).r(FUNC(z80net_state::lx388_read_field_sync));
	map(0xee, 0xee).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xef, 0xef).rw(FUNC(z80net_state::lx385_ctrl_r), FUNC(z80net_state::lx385_ctrl_w));
	map(0xf0, 0xff).rw(FUNC(z80net_state::lx383_r), FUNC(z80net_state::lx383_w));
}

void z80netf_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).bankrw("bank1");
	map(0x0400, 0x3fff).bankrw("bank2");
	map(0x4000, 0x7fff).ram();
	map(0x8000, 0x83ff).bankrw("bank3");
	map(0x8400, 0xdfff).ram();
	map(0xec00, 0xedff).ram().share("videoram"); /* (6847) */
	map(0xf000, 0xf3ff).bankrw("bank4");
}

void z80netf_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xd0, 0xd7).rw(FUNC(z80netf_state::lx390_fdc_r), FUNC(z80netf_state::lx390_fdc_w));
	map(0xea, 0xea).r(FUNC(z80netf_state::lx387_data_r));
	map(0xeb, 0xeb).r(FUNC(z80netf_state::lx388_read_field_sync));
	map(0xee, 0xee).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xef, 0xef).rw(FUNC(z80netf_state::lx385_ctrl_r), FUNC(z80netf_state::lx385_ctrl_w));
	map(0xf0, 0xff).rw(FUNC(z80netf_state::lx383_r), FUNC(z80netf_state::lx383_w));
}



/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( z80ne )
	/* LX.384 Hex Keyboard and Display */
	/*
	 * In natural mode the CTRL key is mapped on shift
	 */
	PORT_START("ROW0")          /* IN0 keys row 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=') // set address, increment
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('-') // ?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('R') // registers
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('T') // single-step
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('X') // go
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('L') // load
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('S') // save
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('K') // reserved for future expansion

	PORT_START("ROW1")          /* IN1 keys row 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("CTRL")          /* CONTROL key */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RST")           /* RESET key */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 Reset")  PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, z80ne_state, z80ne_reset, 0) PORT_CHAR('N')

	/* Settings - need to reboot after altering these */
	PORT_START("LX.385")
	PORT_CONFNAME(0x07, 0x04, "LX.385 Cassette: P1,P3 Data Rate")
	PORT_CONFSETTING( 0x01, "A-B: 300 bps")
	PORT_CONFSETTING( 0x02, "A-C: 600 bps")
	PORT_CONFSETTING( 0x04, "A-D: 1200 bps")
	PORT_CONFNAME( 0x08, 0x00, "LX.385: P4 Parity Check")
	PORT_CONFSETTING( 0x00, "Parity Check Enabled")
	PORT_CONFSETTING( 0x08, "Parity Check Disabled")
INPUT_PORTS_END


static INPUT_PORTS_START( z80net )
	PORT_INCLUDE( z80ne )

	/* LX.387 Keyboard BREAK key */
	PORT_START("LX387_BRK")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_CHANGED_MEMBER(DEVICE_SELF, z80net_state, z80net_nmi, 0)

	/* LX.387 Keyboard (Encoded by KR2376) */
	PORT_START("X0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                               PORT_CHAR('_')

	PORT_START("X3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('`') PORT_CHAR('@')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)                   PORT_NAME("Del")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                  PORT_NAME("CR")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(10) PORT_NAME("LF")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("X5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("X6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("X7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("MODIFIERS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
INPUT_PORTS_END


static INPUT_PORTS_START( z80netf )
	PORT_INCLUDE( z80net )

	/* Settings */
	PORT_START("CONFIG")
	PORT_CONFNAME(0x07, 0x01, "Boot EPROM")
	PORT_CONFSETTING(   0x01, "EP382  Hex Monitor")
	PORT_CONFSETTING(   0x02, "EP548  16k BASIC")
	PORT_CONFSETTING(   0x03, "EP390  Boot Loader for 5.5k floppy BASIC")
	PORT_CONFSETTING(   0x04, "EP1390 Boot Loader for NE DOS 1.0/1.5")
	PORT_CONFSETTING(   0x05, "EP2390 Boot Loader for NE DOS G.1")
	PORT_BIT(0xf8, 0xf8, IPT_UNUSED)
INPUT_PORTS_END



/******************************************************************************
 Machine Drivers
******************************************************************************/

#if 0
static const uint32_t lx388palette[] =
{
	rgb_t(0x00, 0xff, 0x00), /* GREEN */
	rgb_t(0x00, 0xff, 0x00), /* YELLOW in original, here GREEN */
	rgb_t(0x00, 0x00, 0xff), /* BLUE */
	rgb_t(0xff, 0x00, 0x00), /* RED */
	rgb_t(0xff, 0xff, 0xff), /* BUFF */
	rgb_t(0x00, 0xff, 0xff), /* CYAN */
	rgb_t(0xff, 0x00, 0xff), /* MAGENTA */
	rgb_t(0xff, 0x80, 0x00), /* ORANGE */

	rgb_t(0x00, 0x20, 0x00), /* BLACK in original, here DARK green */
	rgb_t(0x00, 0xff, 0x00), /* GREEN */
	rgb_t(0x00, 0x00, 0x00), /* BLACK */
	rgb_t(0xff, 0xff, 0xff), /* BUFF */

	rgb_t(0x00, 0x20, 0x00), /* ALPHANUMERIC DARK GREEN */
	rgb_t(0x00, 0xff, 0x00), /* ALPHANUMERIC BRIGHT GREEN */
	rgb_t(0x40, 0x10, 0x00), /* ALPHANUMERIC DARK ORANGE */
	rgb_t(0xff, 0xc4, 0x18)  /* ALPHANUMERIC BRIGHT ORANGE */
};
#endif

void z80ne_state::floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
	fr.add(FLOPPY_DMK_FORMAT);
}

static void z80ne_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_SSSD);
}

void z80ne_state::z80ne(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, Z80NE_CPU_SPEED_HZ);
	m_maincpu->set_addrmap(AS_PROGRAM, &z80ne_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80ne_state::io_map);

	AY31015(config, m_uart);

	CLOCK(config, m_uart_clock, 4800);
	m_uart_clock->signal_handler().set(FUNC(z80ne_state::lx385_uart_tx_clock_w));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette1->set_interface("z80ne_cass");

	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette2->set_interface("z80ne_cass");

	config.set_default_layout(layout_z80ne);

	// all known tapes require LX.388 expansion
	//SOFTWARE_LIST(config, "cass_list").set_original("z80ne_cass");
}

void z80net_state::lx387(machine_config &config)
{
	KR2376_ST(config, m_lx387_kr2376, 50000);
	m_lx387_kr2376->x<0>().set_ioport("X0");
	m_lx387_kr2376->x<1>().set_ioport("X1");
	m_lx387_kr2376->x<2>().set_ioport("X2");
	m_lx387_kr2376->x<3>().set_ioport("X3");
	m_lx387_kr2376->x<4>().set_ioport("X4");
	m_lx387_kr2376->x<5>().set_ioport("X5");
	m_lx387_kr2376->x<6>().set_ioport("X6");
	m_lx387_kr2376->x<7>().set_ioport("X7");
	m_lx387_kr2376->shift().set(FUNC(z80net_state::lx387_shift_r));
	m_lx387_kr2376->control().set(FUNC(z80net_state::lx387_control_r));
}

void z80net_state::z80net(machine_config &config)
{
	z80ne(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &z80net_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80net_state::io_map);

	lx387(config);

	/* video hardware */
	SCREEN(config, "lx388", SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, 4.433619_MHz_XTAL);
	m_vdg->set_screen("lx388");
	m_vdg->input_callback().set(FUNC(z80net_state::lx388_mc6847_videoram_r));
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	config.set_default_layout(layout_z80net);

	SOFTWARE_LIST(config, "cass_list").set_original("z80ne_cass");
}

void z80netb_state::z80netb(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, Z80NE_CPU_SPEED_HZ);
	m_maincpu->set_addrmap(AS_PROGRAM, &z80netb_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80netb_state::io_map);

	AY31015(config, m_uart);

	CLOCK(config, m_uart_clock, 4800);
	m_uart_clock->signal_handler().set(FUNC(z80netb_state::lx385_uart_tx_clock_w));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette1->set_interface("z80ne_cass");

	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette2->set_interface("z80ne_cass");

	lx387(config);

	/* video hardware */
	SCREEN(config, "lx388", SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, 4.433619_MHz_XTAL);
	m_vdg->set_screen("lx388");
	m_vdg->input_callback().set(FUNC(z80netb_state::lx388_mc6847_videoram_r));
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	config.set_default_layout(layout_z80netb);

	// none of the software is compatible
	//SOFTWARE_LIST(config, "cass_list").set_original("z80ne_cass");
}

void z80netf_state::z80netf(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, Z80NE_CPU_SPEED_HZ);
	m_maincpu->set_addrmap(AS_PROGRAM, &z80netf_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80netf_state::io_map);

	AY31015(config, m_uart);

	CLOCK(config, m_uart_clock, 4800);
	m_uart_clock->signal_handler().set(FUNC(z80netf_state::lx385_uart_tx_clock_w));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette1->set_interface("z80ne_cass");

	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette2->set_interface("z80ne_cass");

	lx387(config);

	/* video hardware */
	SCREEN(config, "lx388", SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, 4.433619_MHz_XTAL);
	m_vdg->set_screen("lx388");
	m_vdg->input_callback().set(FUNC(z80netf_state::lx388_mc6847_videoram_r));
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	FD1771(config, m_wd1771, 2_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, "wd1771:0", z80ne_floppies, "sssd", z80ne_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1771:1", z80ne_floppies, "sssd", z80ne_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1771:2", z80ne_floppies, nullptr,   z80ne_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1771:3", z80ne_floppies, nullptr,   z80ne_state::floppy_formats);

	config.set_default_layout(layout_z80netf);

	/* internal ram */
	RAM(config, m_ram).set_default_size("56K").set_default_value(0x00);

	SOFTWARE_LIST(config, "cass_list").set_original("z80ne_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("z80ne_flop");
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( z80ne )
	ROM_REGION(0x0400, "maincpu", 0)
	ROM_LOAD( "ep382.ic5", 0x0000, 0x0400, CRC(55818366) SHA1(adcac04b83c09265517b7bafbc2f5f665d751bec) )
ROM_END

ROM_START( z80net )
	ROM_REGION(0x0400, "maincpu", 0)
	ROM_LOAD( "ep382.ic5", 0x0000, 0x0400, CRC(55818366) SHA1(adcac04b83c09265517b7bafbc2f5f665d751bec) )
ROM_END

ROM_START( z80netb )
	ROM_REGION(0x4000, "maincpu", 0)
	// 16k Basic
	ROM_LOAD( "548-1.ic1", 0x0000, 0x0800, CRC(868cad39) SHA1(0ea8af010786a080f823a879a4211f5712d260da) )
	ROM_LOAD( "548-2.ic2", 0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0) )
	ROM_LOAD( "548-3.ic3", 0x1000, 0x0800, CRC(9c1fe511) SHA1(ff5b6e49a137c2ff9cb760c39bfd85ce4b52bb7d) )
	ROM_LOAD( "548-4.ic4", 0x1800, 0x0800, CRC(cb5e0de3) SHA1(0beaa8927faaf61f6c3fc0ea1d3d5670f901aae3) )
	ROM_LOAD( "548-5.ic5", 0x2000, 0x0800, CRC(0bd4559c) SHA1(e736a3124819ffb43e96a8114cd188f18d538053) )
	ROM_LOAD( "548-6.ic6", 0x2800, 0x0800, CRC(6d663034) SHA1(57588be4e360658dbb313946d7a608e36c1fdd68) )
	ROM_LOAD( "548-7.ic7", 0x3000, 0x0800, CRC(0bab06c0) SHA1(d52f1519c798e91f25648e996b1db174d90ce0f5) )
	ROM_LOAD( "548-8.ic8", 0x3800, 0x0800, CRC(f381b594) SHA1(2de7a8941ba48d463974c73d62e994d3cbe2868d) )
ROM_END

ROM_START( z80netf )
	ROM_REGION(0x5000, "maincpu", 0)
	// ep548 banked at 0x0000 - 0x3FFF
	ROM_LOAD(  "548-1.ic1", 0x0000, 0x0800, CRC(868cad39) SHA1(0ea8af010786a080f823a879a4211f5712d260da) )
	ROM_LOAD(  "548-2.ic2", 0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0) )
	ROM_LOAD(  "548-3.ic3", 0x1000, 0x0800, CRC(9c1fe511) SHA1(ff5b6e49a137c2ff9cb760c39bfd85ce4b52bb7d) )
	ROM_LOAD(  "548-4.ic4", 0x1800, 0x0800, CRC(cb5e0de3) SHA1(0beaa8927faaf61f6c3fc0ea1d3d5670f901aae3) )
	ROM_LOAD(  "548-5.ic5", 0x2000, 0x0800, CRC(0bd4559c) SHA1(e736a3124819ffb43e96a8114cd188f18d538053) )
	ROM_LOAD(  "548-6.ic6", 0x2800, 0x0800, CRC(6d663034) SHA1(57588be4e360658dbb313946d7a608e36c1fdd68) )
	ROM_LOAD(  "548-7.ic7", 0x3000, 0x0800, CRC(0bab06c0) SHA1(d52f1519c798e91f25648e996b1db174d90ce0f5) )
	ROM_LOAD(  "548-8.ic8", 0x3800, 0x0800, CRC(f381b594) SHA1(2de7a8941ba48d463974c73d62e994d3cbe2868d) )

	// ep382 - 8000
	ROM_LOAD(  "ep382.ic5", 0x4000, 0x0400, CRC(55818366) SHA1(adcac04b83c09265517b7bafbc2f5f665d751bec) )
	// ep390 - F000
	ROM_LOAD(  "ep390.ic6", 0x4400, 0x0400, CRC(e4dd7de9) SHA1(523caa97112a9e67cc078c1a70ceee94ec232093) )
	// ep1390 - F000
	ROM_LOAD( "ep1390.ic6", 0x4800, 0x0400, CRC(dc2cbc1d) SHA1(e23418b8f8261a17892f3a73ec09c72bb02e1d0b) )
	// ep2390 - F000
	ROM_LOAD( "ep2390.ic6", 0x4c00, 0x0400, CRC(28d28eee) SHA1(b80f75c1ac4905ae369ecbc9b9ce120cc85502ed) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY               FULLNAME                      FLAGS
COMP( 1980, z80ne,   0,      0,      z80ne,   z80ne,   z80ne_state,   init_z80ne, "Nuova Elettronica",  "Z80NE",                      MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, z80net,  z80ne,  0,      z80net,  z80net,  z80net_state,  init_z80ne, "Nuova Elettronica",  "Z80NE + LX.388",             MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, z80netb, z80ne,  0,      z80netb, z80net,  z80netb_state, init_z80ne, "Nuova Elettronica",  "Z80NE + LX.388 + Basic 16k", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, z80netf, z80ne,  0,      z80netf, z80netf, z80netf_state, empty_init, "Nuova Elettronica",  "Z80NE + LX.388 + LX.390",    MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
