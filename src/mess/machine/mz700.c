// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Dirk Best
/******************************************************************************
 *  Sharp MZ700
 *
 *  Reference: http://sharpmz.computingmuseum.com
 *
 *****************************************************************************/

#include "emu.h"
#include "includes/mz700.h"
#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/z80pio.h"
#include "machine/74145.h"
#include "sound/speaker.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#define LOG(N,M,A,mac)  \
	do { \
		if(VERBOSE>=N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s",mac.time().as_double(), (const char*)M ); \
			logerror A; \
		} \
	} while (0)


/***************************************************************************
    INITIALIZATION
***************************************************************************/

DRIVER_INIT_MEMBER(mz_state,mz700)
{
	m_mz700 = TRUE;
	m_mz700_mode = TRUE;

	m_videoram = auto_alloc_array(machine(), UINT8, 0x800);
	memset(m_videoram, 0, sizeof(UINT8) * 0x800);
	m_colorram = auto_alloc_array(machine(), UINT8, 0x800);
	memset(m_colorram, 0, sizeof(UINT8) * 0x800);
}

DRIVER_INIT_MEMBER(mz_state,mz800)
{
	m_mz700 = FALSE;
	m_mz700_mode = FALSE;

	/* video ram */
	m_videoram = auto_alloc_array(machine(), UINT8, 0x4000);
	memset(m_videoram, 0, sizeof(UINT8) * 0x4000);
	m_colorram = m_videoram + 0x800;

	/* character generator ram */
	m_cgram = auto_alloc_array(machine(), UINT8, 0x1000);
	memset(m_cgram, 0, sizeof(UINT8) * 0x1000);
}

void mz_state::machine_start()
{
	/* reset memory map to defaults */
	mz700_bank_4_w(m_maincpu->space(AS_PROGRAM), 0, 0);
}


/***************************************************************************
    MMIO
***************************************************************************/

READ8_MEMBER(mz_state::mz700_e008_r)
{
	UINT8 data = 0;

	data |= m_other_timer;
	data |= ioport("JOY")->read();
	data |= machine().first_screen()->hblank() << 7;

	LOG(1, "mz700_e008_r", ("%02X\n", data), machine());

	return data;
}

WRITE8_MEMBER(mz_state::mz700_e008_w)
{
	m_pit->write_gate0(BIT(data, 0));
}


/***************************************************************************
    BANK SWITCHING
***************************************************************************/

READ8_MEMBER(mz_state::mz800_bank_0_r)
{
	UINT8 *videoram = m_videoram;
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	/* switch in cgrom */
	spc.install_read_bank(0x1000, 0x1fff, "bank2");
	spc.nop_write(0x1000, 0x1fff);
	membank("bank2")->set_base(memregion("monitor")->base() + 0x1000);

	if (m_mz700_mode)
	{
		/* cgram from 0xc000 to 0xcfff */
		spc.install_read_bank(0xc000, 0xcfff, "bank6");
		spc.install_write_handler(0xc000, 0xcfff, write8_delegate(FUNC(mz_state::mz800_cgram_w),this));
		membank("bank6")->set_base(m_cgram);
	}
	else
	{
		if (m_hires_mode)
		{
			/* vram from 0x8000 to 0xbfff */
			spc.install_readwrite_bank(0x8000, 0xbfff, "bank4");
			membank("bank4")->set_base(videoram);
		}
		else
		{
			/* vram from 0x8000 to 0x9fff */
			spc.install_readwrite_bank(0x8000, 0x9fff, "bank4");
			membank("bank4")->set_base(videoram);

			/* ram from 0xa000 to 0xbfff */
			spc.install_readwrite_bank(0xa000, 0xbfff, "bank5");
			membank("bank5")->set_base(m_ram->pointer() + 0xa000);
		}
	}

	return 0xff;
}

WRITE8_MEMBER(mz_state::mz700_bank_0_w)
{
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	spc.install_readwrite_bank(0x0000, 0x0fff, "bank1");
	membank("bank1")->set_base(m_ram->pointer());
}

WRITE8_MEMBER(mz_state::mz800_bank_0_w)
{
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	spc.install_readwrite_bank(0x0000, 0x7fff, "bank1");
	membank("bank1")->set_base(m_ram->pointer());
}

READ8_MEMBER(mz_state::mz800_bank_1_r)
{
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	/* switch in ram from 0x1000 to 0x1fff */
	spc.install_readwrite_bank(0x1000, 0x1fff, "bank2");
	membank("bank2")->set_base(m_ram->pointer() + 0x1000);

	if (m_mz700_mode)
	{
		/* ram from 0xc000 to 0xcfff */
		spc.install_readwrite_bank(0xc000, 0xcfff, "bank6");
		membank("bank6")->set_base(m_ram->pointer() + 0xc000);
	}
	else
	{
		/* ram from 0x8000 to 0xbfff */
		spc.install_readwrite_bank(0x8000, 0xbfff, "bank4");
		membank("bank4")->set_base(m_ram->pointer() + 0x8000);
	}

	return 0xff;
}

WRITE8_MEMBER(mz_state::mz700_bank_1_w)
{
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	if (m_mz700_mode)
	{
		/* switch in ram when not locked */
		if (!m_mz700_ram_lock)
		{
			spc.install_readwrite_bank(0xd000, 0xffff, "bank7");
			membank("bank7")->set_base(m_ram->pointer() + 0xd000);
			m_mz700_ram_vram = FALSE;
		}
	}
	else
	{
		/* switch in ram when not locked */
		if (!m_mz800_ram_lock)
		{
			spc.install_readwrite_bank(0xe000, 0xffff, "bank8");
			membank("bank8")->set_base(m_ram->pointer() + 0xe000);
			m_mz800_ram_monitor = FALSE;
		}
	}
}

WRITE8_MEMBER(mz_state::mz700_bank_2_w)
{
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	spc.install_read_bank(0x0000, 0x0fff, "bank1");
	spc.nop_write(0x0000, 0x0fff);
	membank("bank1")->set_base(memregion("monitor")->base());
}

WRITE8_MEMBER(mz_state::mz700_bank_3_w)
{
	UINT8 *videoram = m_videoram;
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	if (m_mz700_mode)
	{
		if (!m_mz700_ram_lock)
		{
			/* switch in videoram */
			spc.install_readwrite_bank(0xd000, 0xd7ff, "bank7");
			membank("bank7")->set_base(videoram);

			/* switch in colorram */
			spc.install_readwrite_bank(0xd800, 0xdfff, "bank9");
			membank("bank9")->set_base(m_colorram);

			m_mz700_ram_vram = TRUE;

			/* switch in memory mapped i/o devices */
			if (m_mz700)
			{
				spc.install_readwrite_handler(0xe000, 0xfff3, 0, 0x1ff0, read8_delegate(FUNC(i8255_device::read), (i8255_device*)m_ppi), write8_delegate(FUNC(i8255_device::write), (i8255_device*)m_ppi));
				spc.install_readwrite_handler(0xe004, 0xfff7, 0, 0x1ff0, read8_delegate(FUNC(pit8253_device::read), (pit8253_device*)m_pit), write8_delegate(FUNC(pit8253_device::write), (pit8253_device*)m_pit));
				spc.install_readwrite_handler(0xe008, 0xfff8, 0, 0x1ff0, read8_delegate(FUNC(mz_state::mz700_e008_r),this), write8_delegate(FUNC(mz_state::mz700_e008_w),this));
			}
			else
			{
				spc.install_readwrite_handler(0xe000, 0xe003, read8_delegate(FUNC(i8255_device::read), (i8255_device*)m_ppi), write8_delegate(FUNC(i8255_device::write), (i8255_device*)m_ppi));
				spc.install_readwrite_handler(0xe004, 0xe007, read8_delegate(FUNC(pit8253_device::read), (pit8253_device*)m_pit), write8_delegate(FUNC(pit8253_device::write), (pit8253_device*)m_pit));
				spc.install_readwrite_handler(0xe008, 0xe008, read8_delegate(FUNC(mz_state::mz700_e008_r),this), write8_delegate(FUNC(mz_state::mz700_e008_w),this));
			}
		}
	}
	else
	{
		if (!m_mz800_ram_lock)
		{
			/* switch in mz800 monitor rom if not locked */
			spc.install_read_bank(0xe000, 0xffff, "bank8");
			spc.nop_write(0xe000, 0xffff);
			membank("bank8")->set_base(memregion("monitor")->base() + 0x2000);
			m_mz800_ram_monitor = TRUE;
		}
	}
}

WRITE8_MEMBER(mz_state::mz700_bank_4_w)
{
	UINT8 *videoram = m_videoram;
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	if (m_mz700_mode)
	{
		m_mz700_ram_lock = FALSE;       /* reset lock */
		mz700_bank_2_w(space, 0, 0);    /* switch in monitor rom */
		mz700_bank_3_w(space, 0, 0);    /* switch in videoram, colorram, and mmio */

		/* rest is ram is always ram in mz700 mode */
		spc.install_readwrite_bank(0x1000, 0xcfff, "bank2");
		membank("bank2")->set_base(m_ram->pointer() + 0x1000);
	}
	else
	{
		/* monitor rom and cgrom */
		spc.install_read_bank(0x0000, 0x1fff, "bank1");
		spc.nop_write(0x0000, 0x1fff);
		membank("bank1")->set_base(memregion("monitor")->base());

		/* ram from 0x2000 to 0x7fff */
		spc.install_readwrite_bank(0x2000, 0x7fff, "bank3");
		membank("bank3")->set_base(m_ram->pointer());

		if (m_hires_mode)
		{
			/* vram from 0x8000 to 0xbfff */
			spc.install_readwrite_bank(0x8000, 0xbfff, "bank4");
			membank("bank4")->set_base(videoram);
		}
		else
		{
			/* vram from 0x8000 to 0x9fff */
			spc.install_readwrite_bank(0x8000, 0x9fff, "bank4");
			membank("bank4")->set_base(videoram);

			/* ram from 0xa000 to 0xbfff */
			spc.install_readwrite_bank(0xa000, 0xbfff, "bank5");
			membank("bank5")->set_base(m_ram->pointer() + 0xa000);
		}

		/* ram from 0xc000 to 0xdfff */
		spc.install_readwrite_bank(0xc000, 0xdfff, "bank6");
		membank("bank6")->set_base(m_ram->pointer() + 0xc000);

		/* mz800 monitor rom from 0xe000 to 0xffff */
		spc.install_read_bank(0xe000, 0xffff, "bank8");
		spc.nop_write(0xe000, 0xffff);
		membank("bank8")->set_base(memregion("monitor")->base() + 0x2000);
		m_mz800_ram_monitor = TRUE;

		m_mz800_ram_lock = FALSE; /* reset lock? */
	}
}

WRITE8_MEMBER(mz_state::mz700_bank_5_w)
{
	address_space &spc = m_maincpu->space(AS_PROGRAM);

	if (m_mz700_mode)
	{
		/* prevent access from 0xd000 to 0xffff */
		m_mz700_ram_lock = TRUE;
		spc.nop_readwrite(0xd000, 0xffff);
	}
	else
	{
		/* prevent access from 0xe000 to 0xffff */
		m_mz800_ram_lock = TRUE;
		spc.nop_readwrite(0xe000, 0xffff);
	}
}

WRITE8_MEMBER(mz_state::mz700_bank_6_w)
{
	if (m_mz700_mode)
	{
		m_mz700_ram_lock = FALSE;

		/* restore access */
		if (m_mz700_ram_vram)
			mz700_bank_3_w(space, 0, 0);
		else
			mz700_bank_1_w(space, 0, 0);
	}
	else
	{
		m_mz800_ram_lock = FALSE;

		/* restore access from 0xe000 to 0xffff */
		if (m_mz800_ram_monitor)
			mz700_bank_3_w(space, 0, 0);
		else
			mz700_bank_1_w(space, 0, 0);
	}
}


/************************ PIT ************************************************/

/* Timer 0 is the clock for the speaker output */

WRITE_LINE_MEMBER(mz_state::pit_out0_changed)
{
	if((m_prev_state==0) && (state==1)) {
		m_speaker_level ^= 1;
	}
	m_prev_state = state;
	m_speaker->level_w(m_speaker_level);
}

/* timer 2 is the AM/PM (12 hour) interrupt */
WRITE_LINE_MEMBER(mz_state::pit_irq_2)
{
	if (!m_intmsk)
		m_maincpu->set_input_line(0, state);
}


/***************************************************************************
    8255 PPI
***************************************************************************/

READ8_MEMBER(mz_state::pio_port_b_r)
{
	device_t *device = machine().device("ls145");
	int key_line = dynamic_cast<ttl74145_device *>(device)->read();
	const char *const keynames[10] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8", "ROW9" };
	int i;
	UINT8 res = 0;

	for(i=0;i<10;i++)
	{
		if(key_line & (1 << i))
			res |= ioport(keynames[i])->read();
	}

	return res;
}

/*
 * bit 7 in     vertical blank
 * bit 6 in     NE556 output
 * bit 5 in     tape data (RDATA)
 * bit 4 in     motor (1 = on)
 */
READ8_MEMBER(mz_state::pio_port_c_r)
{
	UINT8 data = 0;

	/* note: this is actually connected to Q output of the motor-control flip-flop (see below) */
	if ((m_cassette->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED)
		data |= 0x10;

	if ((m_cassette)->input() > 0.0038)
		data |= 0x20;       /* set the RDATA status */

	data |= m_cursor_timer << 6;
	data |= machine().first_screen()->vblank() << 7;

	LOG(2,"mz700_pio_port_c_r",("%02X\n", data),machine());

	return data;
}


WRITE8_MEMBER(mz_state::pio_port_a_w)
{
	device_t *device = machine().device("ls145");
	timer_device *timer = machine().device<timer_device>("cursor");

	LOG(2,"mz700_pio_port_a_w",("%02X\n", data),machine());

	/* the ls145 is connected to PA0-PA3 */
	dynamic_cast<ttl74145_device *>(device)->write(data & 0x07);

	/* ne556 reset is connected to PA7 */
	timer->enable(BIT(data, 7));
}


WRITE8_MEMBER(mz_state::pio_port_c_w)
{
	/*
	 * bit 3 out    motor control (0 = on)
	 * bit 2 out    INTMSK
	 * bit 1 out    tape data (WDATA)
	 * bit 0 out    unused
	 */

//  UINT8 state = cassette_get_state(m_cassette);
//  UINT8 action = ((~pio_port_c_output & 8) & (data & 8));     /* detect low-to-high transition */

	/* The motor control circuit consists of a resistor, capacitor, invertor, nand-gate, and D flip-flop.
	    The sense input from the cassette player goes low whenever play, rewind or fast-forward is pressed.
	    This connects to most of the above components.
	    The Q output enables the motor, and also connects to Bit 4 (input).
	    Bit 3 outputs a string of pulses to the Clock pin, and therefore cannot be used to control
	    the motor directly.
	    For the moment, the user can use the UI to select play, stop, etc.
	    If you load from the command-line or the software-picker, type in L <enter> immediately. */
#if 0

		m_cassette->change_state(
		((data & 0x08) && mz700_motor_on) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,
		CASSETTE_MOTOR_DISABLED);

#endif

	LOG(2,"mz700_pio_port_c_w",("%02X\n", data),machine());

	m_cassette->output((data & 0x02) ? +1.0 : -1.0);
}


/******************************************************************************
 *  Sharp MZ800
 *
 *
 ******************************************************************************/

//static UINT8 mz800_display_mode = 0;
//static UINT8 mz800_port_e8 = 0;


/***************************************************************************
    Z80 PIO
***************************************************************************/

WRITE_LINE_MEMBER(mz_state::mz800_z80pio_irq)
{
	m_maincpu->set_input_line(0, state);
}

WRITE_LINE_MEMBER(mz_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER(mz_state::write_centronics_perror)
{
	m_centronics_perror = state;
}

READ8_MEMBER(mz_state::mz800_z80pio_port_a_r)
{
	UINT8 result = 0;

	result |= m_centronics_busy;
	result |= m_centronics_perror << 1;
	result |= machine().first_screen()->hblank() << 5;

	return result;
}

WRITE8_MEMBER(mz_state::mz800_z80pio_port_a_w)
{
	m_centronics->write_init(BIT(data, 6));
	m_centronics->write_strobe(BIT(data, 7));
}

/* port CE */
READ8_MEMBER(mz_state::mz800_crtc_r)
{
	UINT8 data = 0x00;
	LOG(1,"mz800_crtc_r",("%02X\n",data),machine());
	return data;
}


/* port EA */
READ8_MEMBER(mz_state::mz800_ramdisk_r)
{
	UINT8 *mem = memregion("user1")->base();
	UINT8 data = mem[m_mz800_ramaddr];
	LOG(2,"mz800_ramdisk_r",("[%04X] -> %02X\n", m_mz800_ramaddr, data),machine());
	if (m_mz800_ramaddr++ == 0)
		LOG(1,"mz800_ramdisk_r",("address wrap 0000\n"),machine());
	return data;
}

/* port CC */
WRITE8_MEMBER(mz_state::mz800_write_format_w)
{
	LOG(1,"mz800_write_format_w",("%02X\n", data),machine());
}

/* port CD */
WRITE8_MEMBER(mz_state::mz800_read_format_w)
{
	LOG(1,"mz800_read_format_w",("%02X\n", data),machine());
}

/* port CE
 * bit 3    1: MZ700 mode       0: MZ800 mode
 * bit 2    1: 640 horizontal   0: 320 horizontal
 * bit 1    1: 4bpp/2bpp        0: 2bpp/1bpp
 * bit 0    ???
 */
WRITE8_MEMBER(mz_state::mz800_display_mode_w)
{
	m_mz700_mode = BIT(data, 3);
	m_hires_mode = BIT(data, 2);
	m_screennum = data & 0x03;

	/* change memory maps if we switched mode */
//  if (BIT(data, 3) != m_mz700_mode)
//  {
//      logerror("mz800_display_mode_w: switching mode to %s\n", (BIT(data, 3) ? "mz700" : "mz800"));
//      m_mz700_mode = BIT(data, 3);
//      mz700_bank_4_w(*m_maincpu->&space(AS_PROGRAM), 0, 0);
//  }
}

/* port CF */
WRITE8_MEMBER(mz_state::mz800_scroll_border_w)
{
	LOG(1,"mz800_scroll_border_w",("%02X\n", data),machine());
}

/* port EA */
WRITE8_MEMBER(mz_state::mz800_ramdisk_w)
{
	UINT8 *mem = memregion("user1")->base();
	LOG(2,"mz800_ramdisk_w",("[%04X] <- %02X\n", m_mz800_ramaddr, data),machine());
	mem[m_mz800_ramaddr] = data;
	if (m_mz800_ramaddr++ == 0)
		LOG(1,"mz800_ramdisk_w",("address wrap 0000\n"),machine());
}

/* port EB */
WRITE8_MEMBER(mz_state::mz800_ramaddr_w)
{
	m_mz800_ramaddr = (m_maincpu->state_int(Z80_BC) & 0xff00) | (data & 0xff);
	LOG(1,"mz800_ramaddr_w",("%04X\n", m_mz800_ramaddr),machine());
}

/* port F0 */
WRITE8_MEMBER(mz_state::mz800_palette_w)
{
	if (data & 0x40)
	{
		m_mz800_palette_bank = data & 3;
		LOG(1,"mz800_palette_w",("bank: %d\n", m_mz800_palette_bank),machine());
	}
	else
	{
		int idx = (data >> 4) & 3;
		int val = data & 15;
		LOG(1,"mz800_palette_w",("palette[%d] <- %d\n", idx, val),machine());
		m_mz800_palette[idx] = val;
	}
}
