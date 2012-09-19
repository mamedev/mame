/***************************************************************************
    zx.c

    machine driver
    Juergen Buchmueller <pullmoll@t-online.de>, Dec 1999

****************************************************************************/

#include "includes/zx.h"

#define video_screen_get_refresh(screen)	(((screen_config *)(screen)->inline_config)->refresh)

#define	DEBUG_ZX81_PORTS	1
#define DEBUG_ZX81_VSYNC	1

#define LOG_ZX81_IOR(_comment) do { if (DEBUG_ZX81_PORTS) logerror("ZX81 IOR: %04x, Data: %02x, Scanline: %d (%s)\n", offset, data, space.machine().primary_screen->vpos(), _comment); } while (0)
#define LOG_ZX81_IOW(_comment) do { if (DEBUG_ZX81_PORTS) logerror("ZX81 IOW: %04x, Data: %02x, Scanline: %d (%s)\n", offset, data, space.machine().primary_screen->vpos(), _comment); } while (0)
#define LOG_ZX81_VSYNC do { if (DEBUG_ZX81_VSYNC) logerror("VSYNC starts in scanline: %d\n", space.machine().primary_screen->vpos()); } while (0)


WRITE8_MEMBER(zx_state::zx_ram_w)
{
	UINT8 *RAM = memregion("maincpu")->base();
	RAM[offset + 0x4000] = data;

	if (data & 0x40)
	{
		space.write_byte(offset | 0xc000, data);
		RAM[offset | 0xc000] = data;
	}
	else
	{
		space.write_byte(offset | 0xc000, 0);
		RAM[offset | 0xc000] = 0;
	}
}

/* I know this looks really pointless... but it has to be here */
READ8_MEMBER( zx_state::zx_ram_r )
{
	UINT8 *RAM = memregion("maincpu")->base();
	return RAM[offset | 0xc000];
}

DRIVER_INIT_MEMBER(zx_state,zx)
{
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	space.install_read_bank(0x4000, 0x4000 + machine().device<ram_device>(RAM_TAG)->size() - 1, "bank1");
	space.install_write_handler(0x4000, 0x4000 + machine().device<ram_device>(RAM_TAG)->size() - 1, write8_delegate(FUNC(zx_state::zx_ram_w),this));
	membank("bank1")->set_base(memregion("maincpu")->base() + 0x4000);
}

DIRECT_UPDATE_MEMBER(zx_state::zx_setdirect)
{
	if (address & 0xc000)
		zx_ula_r(machine(), address, "maincpu", 0);
	return address;
}

DIRECT_UPDATE_MEMBER(zx_state::pc8300_setdirect)
{
	if (address & 0xc000)
		zx_ula_r(machine(), address, "gfx1", 0);
	return address;
}

DIRECT_UPDATE_MEMBER(zx_state::pow3000_setdirect)
{
	if (address & 0xc000)
		zx_ula_r(machine(), address, "gfx1", 1);
	return address;
}

void zx_state::machine_reset()
{
	machine().device("maincpu")->memory().space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(zx_state::zx_setdirect), this));
	m_tape_bit = 0x80;
}

MACHINE_RESET_MEMBER(zx_state,pow3000)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(zx_state::pow3000_setdirect), this));
	m_tape_bit = 0x80;
}

MACHINE_RESET_MEMBER(zx_state,pc8300)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(zx_state::pc8300_setdirect), this));
	m_tape_bit = 0x80;
}

static TIMER_CALLBACK(zx_tape_pulse)
{
	zx_state *state = machine.driver_data<zx_state>();
	state->m_tape_bit = 0x80;
}

READ8_MEMBER( zx_state::zx80_io_r )
{
/* port FE = read keyboard, NTSC/PAL diode, and cass bit; turn off HSYNC-generator/cass-out
    The upper 8 bits are used to select a keyboard scan line */

	UINT8 data = 0xff;
	UINT8 offs = offset & 0xff;

	if (offs == 0xfe)
	{
		if ((offset & 0x0100) == 0)
			data &= ioport("ROW0")->read();
		if ((offset & 0x0200) == 0)
			data &= ioport("ROW1")->read();
		if ((offset & 0x0400) == 0)
			data &= ioport("ROW2")->read();
		if ((offset & 0x0800) == 0)
			data &= ioport("ROW3")->read();
		if ((offset & 0x1000) == 0)
			data &= ioport("ROW4")->read();
		if ((offset & 0x2000) == 0)
			data &= ioport("ROW5")->read();
		if ((offset & 0x4000) == 0)
			data &= ioport("ROW6")->read();
		if ((offset & 0x8000) == 0)
			data &= ioport("ROW7")->read();

		if (!ioport("CONFIG")->read())
			data &= ~0x40;

		machine().device<cassette_image_device>(CASSETTE_TAG)->output(+1.0);

		if (m_ula_irq_active)
		{
			zx_ula_bkgnd(0);
			m_ula_irq_active = 0;
		}
//      else
//      {
			if (((machine().device<cassette_image_device>(CASSETTE_TAG))->input() < -0.75) && m_tape_bit)
			{
				m_tape_bit = 0x00;
				machine().scheduler().timer_set(attotime::from_usec(362), FUNC(zx_tape_pulse));
			}

			data &= ~m_tape_bit;
//      }
		if (m_ula_frame_vsync == 3)
		{
			m_ula_frame_vsync = 2;
		}
	}

	return data;
}

READ8_MEMBER( zx_state::zx81_io_r )
{
/* port FB = read printer status, not emulated
    FE = read keyboard, NTSC/PAL diode, and cass bit; turn off HSYNC-generator/cass-out
    The upper 8 bits are used to select a keyboard scan line */

	UINT8 data = 0xff;
	UINT8 offs = offset & 0xff;

	if (offs == 0xfe)
	{
		if ((offset & 0x0100) == 0)
			data &= ioport("ROW0")->read();
		if ((offset & 0x0200) == 0)
			data &= ioport("ROW1")->read();
		if ((offset & 0x0400) == 0)
			data &= ioport("ROW2")->read();
		if ((offset & 0x0800) == 0)
			data &= ioport("ROW3")->read();
		if ((offset & 0x1000) == 0)
			data &= ioport("ROW4")->read();
		if ((offset & 0x2000) == 0)
			data &= ioport("ROW5")->read();
		if ((offset & 0x4000) == 0)
			data &= ioport("ROW6")->read();
		if ((offset & 0x8000) == 0)
			data &= ioport("ROW7")->read();

		if (!ioport("CONFIG")->read())
			data &= ~0x40;

		machine().device<cassette_image_device>(CASSETTE_TAG)->output(+1.0);

		if (m_ula_irq_active)
		{
			zx_ula_bkgnd(0);
			m_ula_irq_active = 0;
		}
		else
		{
			if (((machine().device<cassette_image_device>(CASSETTE_TAG))->input() < -0.75) && m_tape_bit)
			{
				m_tape_bit = 0x00;
				machine().scheduler().timer_set(attotime::from_usec(362), FUNC(zx_tape_pulse));
			}

			data &= ~m_tape_bit;
		}
		if (m_ula_frame_vsync == 3)
		{
			m_ula_frame_vsync = 2;
		}
	}

	return data;
}

READ8_MEMBER( zx_state::pc8300_io_r )
{
/* port F5 = sound
    F6 = unknown
    FB = read printer status, not emulated
    FE = read keyboard and cass bit; turn off HSYNC-generator/cass-out
    The upper 8 bits are used to select a keyboard scan line.
    No TV diode */

	UINT8 data = 0xff;
	UINT8 offs = offset & 0xff;
	device_t *speaker = machine().device(SPEAKER_TAG);

	if (offs == 0xf5)
	{
		m_speaker_state ^= 1;
		speaker_level_w(speaker, m_speaker_state);
	}
	else
	if (offs == 0xfe)
	{
		if ((offset & 0x0100) == 0)
			data &= ioport("ROW0")->read();
		if ((offset & 0x0200) == 0)
			data &= ioport("ROW1")->read();
		if ((offset & 0x0400) == 0)
			data &= ioport("ROW2")->read();
		if ((offset & 0x0800) == 0)
			data &= ioport("ROW3")->read();
		if ((offset & 0x1000) == 0)
			data &= ioport("ROW4")->read();
		if ((offset & 0x2000) == 0)
			data &= ioport("ROW5")->read();
		if ((offset & 0x4000) == 0)
			data &= ioport("ROW6")->read();
		if ((offset & 0x8000) == 0)
			data &= ioport("ROW7")->read();

		machine().device<cassette_image_device>(CASSETTE_TAG)->output(+1.0);

		if (m_ula_irq_active)
		{
			zx_ula_bkgnd(0);
			m_ula_irq_active = 0;
		}
		else
		{
			if (((machine().device<cassette_image_device>(CASSETTE_TAG))->input() < -0.75) && m_tape_bit)
			{
				m_tape_bit = 0x00;
				machine().scheduler().timer_set(attotime::from_usec(362), FUNC(zx_tape_pulse));
			}

			data &= ~m_tape_bit;
		}
		if (m_ula_frame_vsync == 3)
		{
			m_ula_frame_vsync = 2;
		}
	}

	return data;
}

READ8_MEMBER( zx_state::pow3000_io_r )
{
/* port 7E = read NTSC/PAL diode
    F5 = sound
    F6 = unknown
    FB = read printer status, not emulated
    FE = read keyboard and cass bit; turn off HSYNC-generator/cass-out
    The upper 8 bits are used to select a keyboard scan line */

	UINT8 data = 0xff;
	UINT8 offs = offset & 0xff;
	device_t *speaker = machine().device(SPEAKER_TAG);

	if (offs == 0x7e)
	{
		data = (ioport("CONFIG")->read());
	}
	else
	if (offs == 0xf5)
	{
		m_speaker_state ^= 1;
		speaker_level_w(speaker, m_speaker_state);
	}
	else
	if (offs == 0xfe)
	{
		if ((offset & 0x0100) == 0)
			data &= ioport("ROW0")->read();
		if ((offset & 0x0200) == 0)
			data &= ioport("ROW1")->read();
		if ((offset & 0x0400) == 0)
			data &= ioport("ROW2")->read();
		if ((offset & 0x0800) == 0)
			data &= ioport("ROW3")->read();
		if ((offset & 0x1000) == 0)
			data &= ioport("ROW4")->read();
		if ((offset & 0x2000) == 0)
			data &= ioport("ROW5")->read();
		if ((offset & 0x4000) == 0)
			data &= ioport("ROW6")->read();
		if ((offset & 0x8000) == 0)
			data &= ioport("ROW7")->read();

		machine().device<cassette_image_device>(CASSETTE_TAG)->output(+1.0);

		if (m_ula_irq_active)
		{
			zx_ula_bkgnd(0);
			m_ula_irq_active = 0;
		}
		else
		{
			if (((machine().device<cassette_image_device>(CASSETTE_TAG))->input() < -0.75) && m_tape_bit)
			{
				m_tape_bit = 0x00;
				machine().scheduler().timer_set(attotime::from_usec(362), FUNC(zx_tape_pulse));
			}

			data &= ~m_tape_bit;
		}
		if (m_ula_frame_vsync == 3)
		{
			m_ula_frame_vsync = 2;
		}
	}

	return data;
}

WRITE8_MEMBER( zx_state::zx80_io_w )
{
/* port FF = write HSYNC and cass data */

	UINT8 offs = offset & 0xff;

	if (offs == 0xff)
		machine().device<cassette_image_device>(CASSETTE_TAG)->output(-1.0);
}

WRITE8_MEMBER( zx_state::zx81_io_w )
{
	address_space &mem = machine().device("maincpu")->memory().space(AS_PROGRAM);
/* port F5 = unknown, pc8300/pow3000/lambda only
    F6 = unknown, pc8300/pow3000/lambda only
    FB = write data to printer, not emulated
    FD = turn off NMI generator
    FE = turn on NMI generator
    FF = write HSYNC and cass data */

	screen_device *screen = machine().first_screen();
	int height = screen->height();
	UINT8 offs = offset & 0xff;

	if (offs == 0xfd)
	{
		m_ula_nmi->reset();
	}
	else
	if (offs == 0xfe)
	{
		m_ula_nmi->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(207));

		/* remove the IRQ */
		m_ula_irq_active = 0;
	}
	else
	if (offs == 0xff)
	{
		machine().device<cassette_image_device>(CASSETTE_TAG)->output(-1.0);
		zx_ula_bkgnd(1);
		if (m_ula_frame_vsync == 2)
		{
			mem.device().execute().spin_until_time(machine().primary_screen->time_until_pos(height - 1, 0));
			m_ula_scanline_count = height - 1;
			logerror ("S: %d B: %d\n", machine().primary_screen->vpos(), machine().primary_screen->hpos());
		}
	}
}

