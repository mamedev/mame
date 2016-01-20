// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Entertainment Sciences Real-Time Image Processor (RIP) hardware

    driver by Phil Bennett

    Games supported:
        * Turbo Sub [3 sets]

    ROMs wanted:
        * Bouncer
        * Turbo Sub [later version] (improved gameplay, uses 27512 ROMs)

    Notes:
        * 'turbosub' executes a series of hardware tests on startup.
        To skip, hold down keypad '*' on reset.
        * Hold '*' during the game to access the operator menu.

    BTANB:
        * Missing lines occur on real hardware.

    To do:
        * Implement collision detection hardware (unused by Turbo Sub).

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/esrip/esrip.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "includes/esripsys.h"


/*************************************
 *
 *  6840 PTM
 *
 *************************************/

WRITE_LINE_MEMBER(esripsys_state::ptm_irq)
{
	m_soundcpu->set_input_line(M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

/*************************************
 *
 *  i8251A UART
 *
 *************************************/

/* Note: Game CPU /FIRQ is connected to RXRDY */
WRITE8_MEMBER(esripsys_state::uart_w)
{
	if ((offset & 1) == 0)
		osd_printf_debug("%c",data);
}

READ8_MEMBER(esripsys_state::uart_r)
{
	return 0;
}


/*************************************
 *
 *  Game CPU Status Ports
 *
 *************************************/

/*
    Write                   Read
    =====                   ====
    0: ROM bank bit 0       0: Frame CPU status D0
    1: ROM bank bit 1       1: Frame CPU status D1
    2: -                    2: Frame CPU status D2
    3: Bank sel enable?     3: Frame CPU status D3
    4: Frame CPU /FIRQ      4: -
    5: /INTACK              5: Frame CPU status D5
    6: -                    6: RIP BANK 4
    7: Frame CPU /NMI       7: /VBLANK
*/

READ8_MEMBER(esripsys_state::g_status_r)
{
	int bank4 = BIT(m_videocpu->get_rip_status(), 2);
	int vblank = m_screen->vblank();

	return (!vblank << 7) | (bank4 << 6) | (m_f_status & 0x2f);
}

WRITE8_MEMBER(esripsys_state::g_status_w)
{
	int bankaddress;
	UINT8 *rom = memregion("game_cpu")->base();

	m_g_status = data;

	bankaddress = 0x10000 + (data & 0x03) * 0x10000;
	membank("bank1")->set_base(&rom[bankaddress]);

	m_framecpu->set_input_line(M6809_FIRQ_LINE, data & 0x10 ? CLEAR_LINE : ASSERT_LINE);
	m_framecpu->set_input_line(INPUT_LINE_NMI,  data & 0x80 ? CLEAR_LINE : ASSERT_LINE);

	m_videocpu->set_input_line(INPUT_LINE_RESET, data & 0x40 ? CLEAR_LINE : ASSERT_LINE);

	/* /VBLANK IRQ acknowledge */
	if (!(data & 0x20))
		m_gamecpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}


/*************************************
 *
 *  Frame CPU Status Ports
 *
 *************************************/

/*
    Write                       Read
    =====                       ====
    0: Game CPU status in D0    0: /ERROR (AM29116)
    1: Game CPU status in D1    1: /IPT UPLOAD (AM29116)
    2: Game CPU status in D2    2: -
    3: Game CPU status in D3    3: RER0 (AM29116)
    4: -                        4: RER1 (AM29116)
    5: Game CPU status in D5    5: VBLANK flag (cleared on FRAME write)
    6: -                        6: /FBSEL
    7: /FRDONE                  7: /VBLANK
*/

READ8_MEMBER(esripsys_state::f_status_r)
{
	int vblank = m_screen->vblank();
	UINT8 rip_status = m_videocpu->get_rip_status();

	rip_status = (rip_status & 0x18) | (BIT(rip_status, 6) << 1) |  BIT(rip_status, 7);

	return (!vblank << 7) | (m_fbsel << 6) | (m_frame_vbl << 5) | rip_status;
}

WRITE8_MEMBER(esripsys_state::f_status_w)
{
	m_f_status = data;
}


/*************************************
 *
 *  Frame CPU Functions
 *
 *************************************/

TIMER_CALLBACK_MEMBER(esripsys_state::delayed_bank_swap)
{
	m_fasel ^= 1;
	m_fbsel ^= 1;
}

WRITE8_MEMBER(esripsys_state::frame_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(esripsys_state::delayed_bank_swap),this));
	m_frame_vbl = 1;
}

READ8_MEMBER(esripsys_state::fdt_r)
{
	if (!m_fasel)
		return m_fdt_b[offset];
	else
		return m_fdt_a[offset];
}

WRITE8_MEMBER(esripsys_state::fdt_w)
{
	if (!m_fasel)
		m_fdt_b[offset] = data;
	else
		m_fdt_a[offset] = data;
}


/*************************************
 *
 *  Video CPU Functions
 *
 *************************************/

READ16_MEMBER( esripsys_state::fdt_rip_r )
{
	offset = (offset & 0x7ff) << 1;

	if (!m_fasel)
		return (m_fdt_a[offset] << 8) | m_fdt_a[offset + 1];
	else
		return (m_fdt_b[offset] << 8) | m_fdt_b[offset + 1];
}

WRITE16_MEMBER( esripsys_state::fdt_rip_w )
{
	offset = (offset & 0x7ff) << 1;

	if (!m_fasel)
	{
		m_fdt_a[offset + 0] = data >> 8;
		m_fdt_a[offset + 1] = data & 0xff;
	}
	else
	{
		m_fdt_b[offset + 0] = data >> 8;
		m_fdt_b[offset + 1] = data & 0xff;
	}
}

/*
   D0 = /VBLANK
   D1 = /HBLANK
   D2 = 1/2SEL
   D3 = /FIG
   D4 = /FBSEL
   D5 = VO
   D6 =
   D7 = /FDONE
*/

READ8_MEMBER(esripsys_state::rip_status_in)
{
	int vpos =  m_screen->vpos();
	UINT8 _vblank = !(vpos >= ESRIPSYS_VBLANK_START);
//  UINT8 _hblank = !m_screen->hblank();

	return  _vblank
			| (m_hblank << 1)
			| (m_12sel << 2)
			| (m_fbsel << 4)
			| ((vpos & 1) << 5)
			| (m_f_status & 0x80);
}

/*************************************
 *
 *  I/O
 *
 *************************************/

WRITE8_MEMBER(esripsys_state::g_iobus_w)
{
	m_g_iodata = data;
}

READ8_MEMBER(esripsys_state::g_iobus_r)
{
	switch (m_g_ioaddr & 0x7f)
	{
		case 0:
			return m_s_to_g_latch2 & 0x3f;
		case 3:
			return m_s_to_g_latch1;
		case 5:
			return m_cmos_ram[(m_cmos_ram_a10_3 << 3) | (m_cmos_ram_a2_0 & 3)];
		case 8:
		{
			int keypad = ioport("KEYPAD_B")->read() | m_keypad_status;
			m_keypad_status = 0;
			m_io_firq_status = 0;
			return keypad;
		}
		case 9:
		{
			return ioport("KEYPAD_A")->read();
		}
		case 0xa:
		{
			int coins =  m_coin_latch | (ioport("COINS")->read() & 0x30);
			m_coin_latch = 0;
			m_io_firq_status = 0;
			return coins;
		}
		case 0x10:
			return ioport("IO_1")->read();
		case 0x11:
			return ioport("JOYSTICK_X")->read();
		case 0x12:
			return ioport("JOYSTICK_Y")->read();
		case 0x16:
			return m_io_firq_status;
		case 0x18:
			return ioport("IO_2")->read();
			/* Unused I/O */
		case 0x19:
		case 0x1a:
			return 0xff;
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
			/* MSM5832 real-time clock/calendar */
			return 0xff;
		default:
		{
			logerror("Unknown I/O read (%x)\n", m_g_ioaddr & 0x7f);
			return 0xff;
		}
	}
}

WRITE8_MEMBER(esripsys_state::g_ioadd_w)
{
	m_g_ioaddr = data;

	/* Bit 7 is connected to /OE of LS374 containing I/O data */
	if ((data & 0x80) == 0)
	{
		switch (m_g_ioaddr & 0x7f)
		{
			case 0x00:
			{
				m_g_to_s_latch1 = m_g_iodata;
				break;
			}
			case 0x02:
			{
				m_soundcpu->set_input_line(INPUT_LINE_NMI, m_g_iodata & 4 ? CLEAR_LINE : ASSERT_LINE);

				if (!(m_g_to_s_latch2 & 1) && (m_g_iodata & 1))
				{
					/* Rising D0 will clock in 1 to FF1... */
					m_u56a = 1;

					/*...causing a sound CPU /IRQ */
					m_soundcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
				}

				if (m_g_iodata & 2)
					m_u56b = 0;

				m_g_to_s_latch2 = m_g_iodata;

				break;
			}
			case 0x04:
			{
				m_cmos_ram[(m_cmos_ram_a10_3 << 3) | (m_cmos_ram_a2_0 & 3)] = m_g_iodata;
				break;
			}
			case 0x06:
			{
				m_cmos_ram_a10_3 = m_g_iodata;
				break;
			}
			case 0x07:
			{
				m_cmos_ram_a2_0 = m_g_iodata;
				break;
			}
			case 0x0b:
			{
				/* Possibly I/O acknowledge; see FIRQ */
				break;
			}
			case 0x14:
			{
				break;
			}
			case 0x15:
			{
				m_video_firq_en = m_g_iodata & 1;
				break;
			}
			default:
			{
				logerror("Unknown I/O write to %x with %x\n", m_g_ioaddr, m_g_iodata);
			}
		}
	}
}

INPUT_CHANGED_MEMBER(esripsys_state::keypad_interrupt)
{
	if (newval == 0)
	{
		m_io_firq_status |= 2;
		m_keypad_status |= 0x20;
		m_gamecpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}
}

INPUT_CHANGED_MEMBER(esripsys_state::coin_interrupt)
{
	if (newval == 1)
	{
		m_io_firq_status |= 2;
		m_coin_latch = ioport("COINS")->read() << 2;
		m_gamecpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( turbosub )
	PORT_START("KEYPAD_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)

	PORT_START("KEYPAD_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON8 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 )  PORT_PLAYER(3) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(3) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_PLAYER(3) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("Keypad *") PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,keypad_interrupt, 0)

	PORT_START("COINS")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,coin_interrupt, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, esripsys_state,coin_interrupt, 0)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IO_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Neutralizer")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Fire")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Mirror of IO_1 (unused) */
	PORT_START("IO_2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOYSTICK_X")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_X ) PORT_MINMAX(0xff, 0x00) PORT_SENSITIVITY(25) PORT_KEYDELTA(200)

	PORT_START("JOYSTICK_Y")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_MINMAX(0xff, 0x00) PORT_SENSITIVITY(25) PORT_KEYDELTA(200)
INPUT_PORTS_END


/*************************************
 *
 *  Sound
 *
 *************************************/

/* Game/Sound CPU communications */
READ8_MEMBER(esripsys_state::s_200e_r)
{
	return m_g_to_s_latch1;
}

WRITE8_MEMBER(esripsys_state::s_200e_w)
{
	m_s_to_g_latch1 = data;
}

WRITE8_MEMBER(esripsys_state::s_200f_w)
{
	UINT8 *rom = memregion("sound_data")->base();
	int rombank = data & 0x20 ? 0x2000 : 0;

	/* Bit 6 -> Reset latch U56A */
	/* Bit 7 -> Clock latch U56B */
	if (m_s_to_g_latch2 & 0x40)
	{
		m_u56a = 0;
		m_soundcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	}

	if (!(m_s_to_g_latch2 & 0x80) && (data & 0x80))
		m_u56b = 1;

	/* Speech data resides in the upper 8kB of the ROMs */
	membank("bank2")->set_base(&rom[0x0000 + rombank]);
	membank("bank3")->set_base(&rom[0x4000 + rombank]);
	membank("bank4")->set_base(&rom[0x8000 + rombank]);

	m_s_to_g_latch2 = data;
}

READ8_MEMBER(esripsys_state::s_200f_r)
{
	return (m_g_to_s_latch2 & 0xfc) | (m_u56b << 1) | m_u56a;
}

READ8_MEMBER(esripsys_state::tms5220_r)
{
	if (offset == 0)
	{
		/* TMS5220 core returns status bits in D7-D6 */
		UINT8 status = m_tms->status_r(space, 0);

		status = ((status & 0x80) >> 5) | ((status & 0x40) >> 5) | ((status & 0x20) >> 5);
		return (m_tms->readyq_r() << 7) | (m_tms->intq_r() << 6) | status;
	}

	return 0xff;
}

/* TODO: Implement correctly using the state PROM */
WRITE8_MEMBER(esripsys_state::tms5220_w)
{
	if (offset == 0)
	{
		m_tms_data = data;
		m_tms->data_w(space, 0, m_tms_data);
	}
#if 0
	if (offset == 1)
	{
		m_tms->data_w(space, 0, m_tms_data);
	}
#endif
}

/* Not used in later revisions */
WRITE8_MEMBER(esripsys_state::control_w)
{
	logerror("Sound control write: %.2x (PC:0x%.4x)\n", data, space.device().safe_pcbase());
}


/* 10-bit MC3410CL DAC */
WRITE8_MEMBER(esripsys_state::esripsys_dac_w)
{
	if (offset == 0)
	{
		m_dac_msb = data & 3;
	}
	else
	{
		UINT16 dac_data = (m_dac_msb << 8) | data;

		/*
		    The 8-bit DAC modulates the 10-bit DAC.
		    Shift down to prevent clipping.
		*/
		m_dac->write_signed16((m_dac_vol * dac_data) >> 1);
	}
}

/* 8-bit MC3408 DAC */
WRITE8_MEMBER(esripsys_state::volume_dac_w)
{
	m_dac_vol = data;
}


/*************************************
 *
 *  Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( game_cpu_map, AS_PROGRAM, 8, esripsys_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x4000, 0x42ff) AM_RAM AM_SHARE("pal_ram")
	AM_RANGE(0x4300, 0x4300) AM_WRITE(esripsys_bg_intensity_w)
	AM_RANGE(0x4400, 0x47ff) AM_NOP // Collision detection RAM
	AM_RANGE(0x4800, 0x4bff) AM_READWRITE(g_status_r, g_status_w)
	AM_RANGE(0x4c00, 0x4fff) AM_READWRITE(g_iobus_r, g_iobus_w)
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(g_ioadd_w)
	AM_RANGE(0x5400, 0x57ff) AM_NOP
	AM_RANGE(0x5c00, 0x5fff) AM_READWRITE(uart_r, uart_w)
	AM_RANGE(0x6000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( frame_cpu_map, AS_PROGRAM, 8, esripsys_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x4000, 0x4fff) AM_READWRITE(fdt_r, fdt_w)
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(f_status_r, f_status_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(frame_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_cpu_map, AS_PROGRAM, 8, esripsys_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0fff) AM_RAM // Not installed on later PCBs
	AM_RANGE(0x2008, 0x2009) AM_READWRITE(tms5220_r, tms5220_w)
	AM_RANGE(0x200a, 0x200b) AM_WRITE(esripsys_dac_w)
	AM_RANGE(0x200c, 0x200c) AM_WRITE(volume_dac_w)
	AM_RANGE(0x200d, 0x200d) AM_WRITE(control_w)
	AM_RANGE(0x200e, 0x200e) AM_READWRITE(s_200e_r, s_200e_w)
	AM_RANGE(0x200f, 0x200f) AM_READWRITE(s_200f_r, s_200f_w)
	AM_RANGE(0x2020, 0x2027) AM_DEVREADWRITE("6840ptm", ptm6840_device, read, write)
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank2")
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank3")
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("bank4")
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( video_cpu_map, AS_PROGRAM, 64, esripsys_state )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

DRIVER_INIT_MEMBER(esripsys_state,esripsys)
{
	UINT8 *rom = memregion("sound_data")->base();

	m_fdt_a = std::make_unique<UINT8[]>(FDT_RAM_SIZE);
	m_fdt_b = std::make_unique<UINT8[]>(FDT_RAM_SIZE);
	m_cmos_ram = std::make_unique<UINT8[]>(CMOS_RAM_SIZE);

	machine().device<nvram_device>("nvram")->set_base(m_cmos_ram.get(), CMOS_RAM_SIZE);

	membank("bank2")->set_base(&rom[0x0000]);
	membank("bank3")->set_base(&rom[0x4000]);
	membank("bank4")->set_base(&rom[0x8000]);

	/* Register stuff for state saving */
	save_pointer(NAME(m_fdt_a.get()), FDT_RAM_SIZE);
	save_pointer(NAME(m_fdt_b.get()), FDT_RAM_SIZE);
	save_pointer(NAME(m_cmos_ram.get()), CMOS_RAM_SIZE);

	save_item(NAME(m_g_iodata));
	save_item(NAME(m_g_ioaddr));
	save_item(NAME(m_coin_latch));
	save_item(NAME(m_keypad_status));
	save_item(NAME(m_g_status));
	save_item(NAME(m_f_status));
	save_item(NAME(m_io_firq_status));
	save_item(NAME(m_cmos_ram_a2_0));
	save_item(NAME(m_cmos_ram_a10_3));

	save_item(NAME(m_u56a));
	save_item(NAME(m_u56b));
	save_item(NAME(m_g_to_s_latch1));
	save_item(NAME(m_g_to_s_latch2));
	save_item(NAME(m_s_to_g_latch1));
	save_item(NAME(m_s_to_g_latch2));
	save_item(NAME(m_dac_msb));
	save_item(NAME(m_dac_vol));
	save_item(NAME(m_tms_data));

	m_fasel = 0;
	m_fbsel = 1;
	save_item(NAME(m_fasel));
	save_item(NAME(m_fbsel));
}

static MACHINE_CONFIG_START( esripsys, esripsys_state )
	MCFG_CPU_ADD("game_cpu", M6809E, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(game_cpu_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", esripsys_state,  esripsys_vblank_irq)
	MCFG_QUANTUM_PERFECT_CPU("game_cpu")

	MCFG_CPU_ADD("frame_cpu", M6809E, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(frame_cpu_map)

	MCFG_CPU_ADD("video_cpu", ESRIP, XTAL_40MHz / 4)
	MCFG_CPU_PROGRAM_MAP(video_cpu_map)
	MCFG_ESRIP_FDT_R_CALLBACK(READ16(esripsys_state, fdt_rip_r))
	MCFG_ESRIP_FDT_W_CALLBACK(WRITE16(esripsys_state, fdt_rip_w))
	MCFG_ESRIP_STATUS_IN_CALLBACK(READ8(esripsys_state, rip_status_in))
	MCFG_ESRIP_DRAW_CALLBACK_OWNER(esripsys_state, esripsys_draw)
	MCFG_ESRIP_LBRM_PROM("proms")

	MCFG_CPU_ADD("sound_cpu", M6809E, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(sound_cpu_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(ESRIPSYS_PIXEL_CLOCK, ESRIPSYS_HTOTAL, ESRIPSYS_HBLANK_END, ESRIPSYS_HBLANK_START, ESRIPSYS_VTOTAL, ESRIPSYS_VBLANK_END, ESRIPSYS_VBLANK_START)
	MCFG_SCREEN_UPDATE_DRIVER(esripsys_state, screen_update_esripsys)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)

	/* Sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("tms5220nl", TMS5220, 640000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* 6840 PTM */
	MCFG_DEVICE_ADD("6840ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(XTAL_8MHz / 4)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_IRQ_CB(WRITELINE(esripsys_state, ptm_irq))
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( turbosub )
	ROM_REGION( 0xc0000, "main_code", 0) /* Non-bankswitched, 6809 #0 code */
	ROM_LOAD( "turbosub.u85",    0x18000, 0x4000, CRC(eabb9509) SHA1(cbfb6c5becb3fe1b4ed729e92a0f4029a5df7d67) )

	ROM_REGION( 0x48000, "game_cpu", 0 ) /* Bankswitched 6809 code */
	ROM_LOAD( "turbosub.u82",    0x10000, 0x2000, CRC(de32eb6f) SHA1(90bf31a5adf261d47b4f52e93b5e97f343b7ebf0) )
	ROM_CONTINUE(                0x20000, 0x2000 )
	ROM_LOAD( "turbosub.u81",    0x12000, 0x2000, CRC(9ae09613) SHA1(9b5ada4a21473b30be98bcc461129b6ed4e0bb11) )
	ROM_CONTINUE(                0x22000, 0x2000 )
	ROM_LOAD( "turbosub.u87",    0x14000, 0x2000, CRC(ad2284f7) SHA1(8e11b8ad0a98dd1fe6ec8f7ea9e6e4f4a45d8a1b) )
	ROM_CONTINUE(                0x24000, 0x2000 )
	ROM_LOAD( "turbosub.u86",    0x16000, 0x2000, CRC(4f51e6fd) SHA1(8f51ac6412aace29279ce7b02cad45ed681c2065) )
	ROM_CONTINUE(                0x26000, 0x2000 )

	ROM_LOAD( "turbosub.u80",    0x30000, 0x2000, CRC(ff2e2870) SHA1(45f91d63ad91585482c9dd05290b204b007e3f44) )
	ROM_CONTINUE(                0x40000, 0x2000 )
	ROM_LOAD( "turbosub.u79",    0x32000, 0x2000, CRC(13680923) SHA1(14e3daa2178853cef1fd96a68305420c11fceb96) )
	ROM_CONTINUE(                0x42000, 0x2000 )
	ROM_LOAD( "turbosub.u84",    0x34000, 0x2000, CRC(7059842d) SHA1(c20a8accd3fc23bc4476e1d08798d7a80915d37c) )
	ROM_CONTINUE(                0x44000, 0x2000 )
	ROM_LOAD( "turbosub.u83",    0x36000, 0x2000, CRC(31b86fc6) SHA1(8e56e8a75f653c3c4da2c9f31f739894beb194db) )
	ROM_CONTINUE(                0x46000, 0x2000 )

	/* e000 - ffff = Upper half of U85 (lower half is blank) */
	ROM_COPY( "main_code", 0x18000 + 0x2000, 0xe000, 0x2000 )

	ROM_REGION( 0x10000, "frame_cpu", 0 )
	ROM_LOAD( "turbosub.u63", 0xc000, 0x4000, CRC(35701532) SHA1(77d957682aab10ee902c1e47c468b9ab8fe6a512) )

	ROM_REGION( 0x1000, "video_cpu", 0 )
	ROMX_LOAD( "27s29.u29", 0x0, 0x200, CRC(d580672b) SHA1(b56295a5b780ab5e8ff6817ebb084a8dfad8c281), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u28", 0x1, 0x200, CRC(f7976b87) SHA1(c19a1d375c497f1671170c7833952979819c3812), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u27", 0x2, 0x200, CRC(03ebd3ea) SHA1(109f5369bd36bcf0da5928b96566655c6895c737), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u21", 0x3, 0x200, CRC(e232384b) SHA1(cfc3acc86add06b4cb6addb3455d71123fb359ce), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u20", 0x4, 0x200, CRC(0a8e44d8) SHA1(2df46316510b2dbfd4c9913a1460c00d5572d586), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u19", 0x5, 0x200, CRC(de17e5f0) SHA1(3e14768374e1bda25183aee86a82d220b7f58ff9), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u18", 0x6, 0x200, CRC(e33ed0a4) SHA1(41edbdc7c022971ce14bd2f419c92714b796fad7), ROM_SKIP(7))

	ROM_REGION( 0x10000, "sound_cpu", 0 )
	ROM_LOAD( "turbosub.u66",   0xe000, 0x2000, CRC(8db3bcdb) SHA1(e6ae324ba9dad4884e1cb3d67ce099a6f4739456) )

	ROM_REGION( 0xc000, "sound_data", 0)
	ROM_LOAD( "turbosub.u69", 0x0000, 0x4000, CRC(ad04193b) SHA1(2f660302e60a7e68e079a8dd13266a77c077f939) )
	ROM_LOAD( "turbosub.u68", 0x4000, 0x4000, CRC(72e3d09b) SHA1(eefdfcd0c4c32e465f18d40f46cb5bc022c22bfd) )
	ROM_LOAD( "turbosub.u67", 0x8000, 0x4000, CRC(f8ae82e9) SHA1(fd27b9fe7872c3c680a1f71a4a5d5eeaa12e4a19) )

	ROM_REGION( 0x40000, "4bpp", 0)
	ROM_LOAD( "turbosub.u44", 0x00000, 0x4000, CRC(eaa05860) SHA1(f649891dae9354b7f2e46e6a380b52a569229d64) )
	ROM_LOAD( "turbosub.u49", 0x10000, 0x4000, CRC(b4170ac2) SHA1(bdbfc43c891c8d525dcc46fb9d05602263ab69cd) )
	ROM_LOAD( "turbosub.u54", 0x20000, 0x4000, CRC(bebf98d8) SHA1(170502bb44fc6d6bf14d8dac4778b37888c14a7b) )
	ROM_LOAD( "turbosub.u59", 0x30000, 0x4000, CRC(9c1f4397) SHA1(94335f2db2650f8b7e24fc3f92a04b73325ab164) )

	ROM_LOAD( "turbosub.u43", 0x04000, 0x4000, CRC(5d76237c) SHA1(3d50347856039e43290497348447b1c4581f3a33) )
	ROM_LOAD( "turbosub.u48", 0x14000, 0x4000, CRC(cea4e036) SHA1(4afce4f2a09adf9c83ab7188c05cd7236dea16a3) )
	ROM_LOAD( "turbosub.u53", 0x24000, 0x4000, CRC(1352d58a) SHA1(76ae86c365dd4c9e1a6c5af91c01d31e7ee35f0f) )
	ROM_LOAD( "turbosub.u58", 0x34000, 0x4000, CRC(5024d83f) SHA1(a293d92a0ae01901b5618b0250d48e3ba631dfcb) )

	ROM_LOAD( "turbosub.u42", 0x08000, 0x4000, CRC(057a1c72) SHA1(5af89b128b7818550572d02e5ff724c415fa8b8b) )
	ROM_LOAD( "turbosub.u47", 0x18000, 0x4000, CRC(10def494) SHA1(a3ba691eb2b0d782162ffc6c081761965844a3a9) )
	ROM_LOAD( "turbosub.u52", 0x28000, 0x4000, CRC(070d07d6) SHA1(4c81310cd646641a380817fedffab66e76529c97) )
	ROM_LOAD( "turbosub.u57", 0x38000, 0x4000, CRC(5ddb0458) SHA1(d1169882397f364ca38fbd563250b33d13b1a7c6) )

	ROM_LOAD( "turbosub.u41", 0x0c000, 0x4000, CRC(014bb06b) SHA1(97276ba26b60c2907e59b92cc9de5251298579cf) )
	ROM_LOAD( "turbosub.u46", 0x1c000, 0x4000, CRC(3b866e2c) SHA1(c0dd4827a18eb9f4b1055d92544beed10f01fd86) )
	ROM_LOAD( "turbosub.u51", 0x2c000, 0x4000, CRC(43cdcb5c) SHA1(3dd966daa904d3be7be63c584ba033c0e7904d5c) )
	ROM_LOAD( "turbosub.u56", 0x3c000, 0x4000, CRC(6d116adf) SHA1(f808e28cef41dc86e43d8c12966037213da87c87) )

	ROM_REGION( 0x40000, "8bpp_l", 0)
	ROM_LOAD( "turbosub.u4",  0x00000, 0x4000, CRC(08303604) SHA1(f075b645d89a2d91bd9b621748906a9f9890ee60) )
	ROM_LOAD( "turbosub.u14", 0x10000, 0x4000, CRC(83b26c8d) SHA1(2dfa3b45c44652d255c402511bb3810fffb0731d) )
	ROM_LOAD( "turbosub.u24", 0x20000, 0x4000, CRC(6bbb6cb3) SHA1(d513e547a05b34076bb8261abd51301ac5f3f5d4) )
	ROM_LOAD( "turbosub.u34", 0x30000, 0x4000, CRC(7b844f4a) SHA1(82467eb7e116f9f225711a1698c151945e1de6e4) )

	ROM_LOAD( "turbosub.u3",  0x04000, 0x4000, CRC(825ef29c) SHA1(affadd0976f793b8bdbcbc4768b7de27121e7b11) )
	ROM_LOAD( "turbosub.u13", 0x14000, 0x4000, CRC(350cc17a) SHA1(b98d16be997fc0576d3206f51f29ce3e257492d3) )
	ROM_LOAD( "turbosub.u23", 0x24000, 0x4000, CRC(b1531916) SHA1(805a23f40aa875f431e835fdaceba87261c14155) )
	ROM_LOAD( "turbosub.u33", 0x34000, 0x4000, CRC(0d5130cb) SHA1(7e4e4e5ea50c581a60d15964571464029515c720) )

	ROM_LOAD( "turbosub.u2",  0x08000, 0x4000, CRC(a8b8c032) SHA1(20512a3a1f8b9c0361e6f5a7e9a50605be3ae650) )
	ROM_LOAD( "turbosub.u12", 0x18000, 0x4000, CRC(a2c4badf) SHA1(267af1be6261833211270af25045e306efffee80) )
	ROM_LOAD( "turbosub.u22", 0x28000, 0x4000, CRC(97b7cf0e) SHA1(888fb2f384a5cba8a6f7569886eb6dc27e2b024f) )
	ROM_LOAD( "turbosub.u32", 0x38000, 0x4000, CRC(b286710e) SHA1(5082db13630ba0967006619027c39ee3607b838d) )

	ROM_LOAD( "turbosub.u1",  0x0c000, 0x4000, CRC(88b0a7a9) SHA1(9012c8059cf60131efa6a0432accd87813187206) )
	ROM_LOAD( "turbosub.u11", 0x1c000, 0x4000, CRC(9f0ff723) SHA1(54b52b4ebc32f10aa32c799ac819928290e70455) )
	ROM_LOAD( "turbosub.u21", 0x2c000, 0x4000, CRC(b4122fe2) SHA1(50e8b488a7b7f739336b60a3fd8a5b14f5010b75) )
	ROM_LOAD( "turbosub.u31", 0x3c000, 0x4000, CRC(3fa15c78) SHA1(bf5cb85fc26b5045ad5acc944c917b068ace2c49) )

	ROM_REGION( 0x40000, "8bpp_r", 0)
	ROM_LOAD( "turbosub.u9",  0x00000, 0x4000, CRC(9a03eadf) SHA1(25ee1ebe52f030b2fa09d76161e46540c91cbc4c) )
	ROM_LOAD( "turbosub.u19", 0x10000, 0x4000, CRC(498253b8) SHA1(dd74d4f9f19d8a746415baea604116faedb4fb31) )
	ROM_LOAD( "turbosub.u29", 0x20000, 0x4000, CRC(809c374f) SHA1(d3849eed8441e4641ffcbca7c83ee3bb16681a0b) )
	ROM_LOAD( "turbosub.u39", 0x30000, 0x4000, CRC(3e4e0681) SHA1(ac834f6823ffe835d6f149e79c1d31ae2b89e85d) )

	ROM_LOAD( "turbosub.u8",  0x04000, 0x4000, CRC(01118737) SHA1(3a8e998b80dffe82296170273dcbbe9870c5b695) )
	ROM_LOAD( "turbosub.u18", 0x14000, 0x4000, CRC(39fd8e57) SHA1(392f8a8cf58fc4813de840775d9c53561488152d) )
	ROM_LOAD( "turbosub.u28", 0x24000, 0x4000, CRC(0628586d) SHA1(e37508c2812e1c98659aaba9c495e7396842614e) )
	ROM_LOAD( "turbosub.u38", 0x34000, 0x4000, CRC(7d597a7e) SHA1(2f48faf75406ab3ff0b954040b74e68b7ca6f7a5) )

	ROM_LOAD( "turbosub.u7",  0x08000, 0x4000, CRC(50eea315) SHA1(567dbb3cb3a75a7507f4cb4748c7dd878e69d6b7) )
	ROM_LOAD( "turbosub.u17", 0x18000, 0x4000, CRC(8a9e19e6) SHA1(19067e153c0002edfd4a756f92ad75d9a0cbc3dd) )
	ROM_LOAD( "turbosub.u27", 0x28000, 0x4000, CRC(1c81a8d9) SHA1(3d13d1ccd7ec3dddf2a27600eb64b5be386e868c) )
	ROM_LOAD( "turbosub.u37", 0x38000, 0x4000, CRC(59f978cb) SHA1(e99d6378de941cad92e9702fcb18aea87acd371f) )

	ROM_LOAD( "turbosub.u6",  0x0c000, 0x4000, CRC(841e00bd) SHA1(f777cc8dd8dd7c8baa2007355a76db782a218efc) )
	ROM_LOAD( "turbosub.u16", 0x1c000, 0x4000, CRC(d3b63d81) SHA1(e86dd64825f6d9e7bebc26413f524a8962f68f2d) )
	ROM_LOAD( "turbosub.u26", 0x2c000, 0x4000, CRC(867cfe32) SHA1(549e4e557d63dfab8e8c463916512a1b422ce425) )
	ROM_LOAD( "turbosub.u36", 0x3c000, 0x4000, CRC(0d8ebc21) SHA1(7ae65edae05869376caa975ff2c778a08e8ad8a2) )

	ROM_REGION( 0x260, "proms", 0)
	ROM_LOAD( "27s29.u123",    0x0000, 0x0200, CRC(b2e8770e) SHA1(849292a6b30bb0e6547ce3232438136897a651b0) )
	ROM_LOAD( "6331_snd.u2",   0x0200, 0x0020, CRC(f1328a5e) SHA1(44d4e802988415d24a0b9eaa38300f5add3a2727) )
	ROM_LOAD( "6331_rom.u74",  0x0220, 0x0020, CRC(7b72b34e) SHA1(bc4d67a6993beb36a161368428e648d0492ac436) )
	ROM_LOAD( "6331_vid.u155", 0x0240, 0x0020, CRC(63371737) SHA1(f08c03c81322c0de9ee64b4a9f11a1422c5bd463) )
ROM_END

ROM_START( turbosub7 )
	ROM_REGION( 0xc0000, "main_code", 0) /* Non-bankswitched, 6809 #0 code */
	ROM_LOAD( "mem6u85.bin",    0x18000, 0x4000, CRC(30016c8b) SHA1(0cd2dd7052de0eaa451ff8b0b2224180764c26de) )

	ROM_REGION( 0x48000, "game_cpu", 0 ) /* Bankswitched 6809 code */
	ROM_LOAD( "mem6u82.bin",    0x10000, 0x2000, CRC(ecb01643) SHA1(32571ed9f2289b7943beb3e518e460c6552bbde7) )
	ROM_CONTINUE(               0x20000, 0x2000 )
	ROM_LOAD( "mem6u81.bin",    0x12000, 0x2000, CRC(3938bc3d) SHA1(0b6d770bdad3d40051d214efa38a8900dcd506dd) )
	ROM_CONTINUE(               0x22000, 0x2000 )
	ROM_LOAD( "mem6u87.bin",    0x14000, 0x2000, CRC(3398ddfe) SHA1(c2339440931d994f4aecf7943ba46c4e337d5bce) )
	ROM_CONTINUE(               0x24000, 0x2000 )
	ROM_LOAD( "mem6u86.bin",    0x16000, 0x2000, CRC(e4835206) SHA1(727a758a1810a1f97d75f063aac98393a5473c72) )
	ROM_CONTINUE(               0x26000, 0x2000 )

	ROM_LOAD( "mem6u80.bin",    0x30000, 0x2000, CRC(02cffdce) SHA1(18483921274eb1963ad7a64daea1d4190e5c141d) )
	ROM_CONTINUE(               0x40000, 0x2000 )
	ROM_LOAD( "mem6u79.bin",    0x32000, 0x2000, CRC(2a756db2) SHA1(c530c9a2f41de331d0d32928303c05c3312037b4) )
	ROM_CONTINUE(               0x42000, 0x2000 )
	ROM_LOAD( "mem6u84.bin",    0x34000, 0x2000, CRC(51a7f19b) SHA1(7a174b11b6f84768e3d4c14ce39974bbb3aea02d) )
	ROM_CONTINUE(               0x44000, 0x2000 )
	ROM_LOAD( "mem6u83.bin",    0x36000, 0x2000, CRC(eef7963a) SHA1(1f2f7f8fb1d68abd91f94967bb7e283004661d6d) )
	ROM_CONTINUE(               0x46000, 0x2000 )

	/* e000 - ffff = Upper half of U85 (lower half is blank) */
	ROM_COPY( "main_code", 0x18000 + 0x2000, 0xe000, 0x2000 )

	ROM_REGION( 0x10000, "frame_cpu", 0 )
	ROM_LOAD( "pcb4u63.bin", 0xc000, 0x4000, CRC(35701532) SHA1(77d957682aab10ee902c1e47c468b9ab8fe6a512) )

	ROM_REGION( 0x1000, "video_cpu", 0 )
	ROMX_LOAD( "27s29.u29", 0x0, 0x200, CRC(d580672b) SHA1(b56295a5b780ab5e8ff6817ebb084a8dfad8c281), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u28", 0x1, 0x200, CRC(f7976b87) SHA1(c19a1d375c497f1671170c7833952979819c3812), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u27", 0x2, 0x200, CRC(03ebd3ea) SHA1(109f5369bd36bcf0da5928b96566655c6895c737), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u21", 0x3, 0x200, CRC(e232384b) SHA1(cfc3acc86add06b4cb6addb3455d71123fb359ce), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u20", 0x4, 0x200, CRC(0a8e44d8) SHA1(2df46316510b2dbfd4c9913a1460c00d5572d586), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u19", 0x5, 0x200, CRC(de17e5f0) SHA1(3e14768374e1bda25183aee86a82d220b7f58ff9), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u18", 0x6, 0x200, CRC(e33ed0a4) SHA1(41edbdc7c022971ce14bd2f419c92714b796fad7), ROM_SKIP(7))

	ROM_REGION( 0x10000, "sound_cpu", 0 )
	ROM_LOAD( "mem6u66.bin", 0xc000, 0x4000, CRC(5091bf3d) SHA1(7ab872cef1562a45f7533c16bbbae8772673465b) )

	ROM_REGION( 0xc000, "sound_data", 0)
	ROM_LOAD( "mem6u69.bin", 0x0000, 0x4000, CRC(ad04193b) SHA1(2f660302e60a7e68e079a8dd13266a77c077f939) )
	ROM_LOAD( "mem6u68.bin", 0x4000, 0x4000, CRC(72e3d09b) SHA1(eefdfcd0c4c32e465f18d40f46cb5bc022c22bfd) )
	ROM_LOAD( "mem6u67.bin", 0x8000, 0x4000, CRC(f8ae82e9) SHA1(fd27b9fe7872c3c680a1f71a4a5d5eeaa12e4a19) )

	ROM_REGION( 0x40000, "4bpp", 0)
	ROM_LOAD( "mem6u44.bin", 0x00000, 0x4000, CRC(0dbcf4a8) SHA1(aa104aa9c9a6182e46663c69193c1f414b7e2270) )
	ROM_CONTINUE(            0x08000, 0x4000 )
	ROM_LOAD( "mem6u49.bin", 0x10000, 0x4000, CRC(68cf6096) SHA1(557ac00bf06878856b1e79f709d401e7a7ae50b9) )
	ROM_CONTINUE(            0x18000, 0x4000 )
	ROM_LOAD( "mem6u54.bin", 0x20000, 0x4000, CRC(561ed51e) SHA1(db4d1bb834216e6c235bc3e91f60e1cab7883769) )
	ROM_CONTINUE(            0x28000, 0x4000 )
	ROM_LOAD( "mem6u59.bin", 0x30000, 0x4000, CRC(fff98687) SHA1(f64e2c4b2fb7b2c85e7be81168169d5d5111382a) )
	ROM_CONTINUE(            0x38000, 0x4000 )

	ROM_LOAD( "mem6u43.bin", 0x04000, 0x4000, CRC(420b5bcb) SHA1(74e25f022d5ad3fdda58af5530182bd0a6db6c0c) )
	ROM_CONTINUE(            0x0c000, 0x4000 )
	ROM_LOAD( "mem6u48.bin", 0x14000, 0x4000, CRC(03c67463) SHA1(e1d8b43588948a76d48f4882be522cdcb1254bad) )
	ROM_CONTINUE(            0x1c000, 0x4000 )
	ROM_LOAD( "mem6u53.bin", 0x24000, 0x4000, CRC(5b5c4fc8) SHA1(f222631fcd515772a21af41badb3aead2043e484) )
	ROM_CONTINUE(            0x2c000, 0x4000 )
	ROM_LOAD( "mem6u58.bin", 0x34000, 0x4000, CRC(3e02ef5b) SHA1(1bd7ac2d5340198d7142c03501a6718995f28a67) )
	ROM_CONTINUE(            0x3c000, 0x4000 )

	ROM_REGION( 0x40000, "8bpp_l", 0)
	ROM_LOAD( "mem6u04.bin", 0x00000, 0x4000, CRC(a42581e8) SHA1(ffab2ae5a36095ba1a71b4d1fc88589c27f819bb) )
	ROM_CONTINUE(            0x08000, 0x4000 )
	ROM_LOAD( "mem6u14.bin", 0x10000, 0x4000, CRC(52b53a20) SHA1(add08ea5cb47cdcc7e8db5e94bb97aedccbc0be6) )
	ROM_CONTINUE(            0x18000, 0x4000 )
	ROM_LOAD( "mem6u24.bin", 0x20000, 0x4000, CRC(6642da40) SHA1(6ded7c04d2d57db92c243cc5af6861cb21b782b6) )
	ROM_CONTINUE(            0x28000, 0x4000 )
	ROM_LOAD( "mem6u34.bin", 0x30000, 0x4000, CRC(6e230a0a) SHA1(6855ce817feb9bda777c2d07a362722a03288a7b) )
	ROM_CONTINUE(            0x38000, 0x4000 )

	ROM_LOAD( "mem6u03.bin", 0x04000, 0x4000, CRC(ed5193ce) SHA1(00544213f604a1e7562f407c3e7ac79cba358942) )
	ROM_CONTINUE(            0x0c000, 0x4000 )
	ROM_LOAD( "mem6u13.bin", 0x14000, 0x4000, CRC(26e71525) SHA1(ba820aeb7e113439764c254e91ca83023eaf751e) )
	ROM_CONTINUE(            0x1c000, 0x4000 )
	ROM_LOAD( "mem6u23.bin", 0x24000, 0x4000, CRC(8ce207c5) SHA1(d3148f27c8285a05a77d222246208161c95a4cde) )
	ROM_CONTINUE(            0x2c000, 0x4000 )
	ROM_LOAD( "mem6u33.bin", 0x34000, 0x4000, CRC(ad12a7ae) SHA1(3f39d039c56cb96d065de4fecca98b17ab4cce3d) )
	ROM_CONTINUE(            0x3c000, 0x4000 )

	ROM_REGION( 0x40000, "8bpp_r", 0)
	ROM_LOAD( "mem6u09.bin", 0x00000, 0x4000, CRC(117811ec) SHA1(9b6bef611f265e54bbc120726c3b99149cb3ca37) )
	ROM_CONTINUE(            0x08000, 0x4000 )
	ROM_LOAD( "mem6u19.bin", 0x10000, 0x4000, CRC(5c9f6c06) SHA1(50973ea0675a037747ef9bb1360ec741d43a0743) )
	ROM_CONTINUE(            0x18000, 0x4000 )
	ROM_LOAD( "mem6u29.bin", 0x20000, 0x4000, CRC(e6414c30) SHA1(ec13ae40d0ad7f702c5a41bfca57b3dfef000c13) )
	ROM_CONTINUE(            0x28000, 0x4000 )
	ROM_LOAD( "mem6u39.bin", 0x30000, 0x4000, CRC(f61c0b65) SHA1(6872a775212ca36283e517ba7247f2b380fc8dd5) )
	ROM_CONTINUE(            0x38000, 0x4000 )

	ROM_LOAD( "mem6u08.bin", 0x04000, 0x4000, CRC(b3fb8861) SHA1(de0ebba8ad82dae88f934f91c745e10538e399c7) )
	ROM_CONTINUE(            0x0c000, 0x4000 )
	ROM_LOAD( "mem6u18.bin", 0x14000, 0x4000, CRC(4adff11d) SHA1(7217490fa7c1c339e0b4a865007fad44b3f026c3) )
	ROM_CONTINUE(            0x1c000, 0x4000 )
	ROM_LOAD( "mem6u28.bin", 0x24000, 0x4000, CRC(7702b849) SHA1(ba1e73a51d855c360fb5501b686f5c168246e18d) )
	ROM_CONTINUE(            0x2c000, 0x4000 )
	ROM_LOAD( "mem6u38.bin", 0x34000, 0x4000, CRC(138dbe03) SHA1(338a6ec2e0072f81a70d99ef4ddeb8410e3cdea6) )
	ROM_CONTINUE(            0x3c000, 0x4000 )

	ROM_REGION( 0x260, "proms", 0)
	ROM_LOAD( "27s29.u123",    0x0000, 0x0200, CRC(b2e8770e) SHA1(849292a6b30bb0e6547ce3232438136897a651b0) )
	ROM_LOAD( "6331_snd.u2",   0x0200, 0x0020, CRC(f1328a5e) SHA1(44d4e802988415d24a0b9eaa38300f5add3a2727) )
	ROM_LOAD( "6331_rom.u74",  0x0220, 0x0020, CRC(7b72b34e) SHA1(bc4d67a6993beb36a161368428e648d0492ac436) )
	ROM_LOAD( "6331_vid.u155", 0x0240, 0x0020, CRC(63371737) SHA1(f08c03c81322c0de9ee64b4a9f11a1422c5bd463) )
ROM_END

ROM_START( turbosub6 )
	ROM_REGION( 0xc0000, "main_code", 0) /* Non-bankswitched, 6809 #0 code */
	ROM_LOAD( "u85",    0x18000, 0x4000, CRC(d37ccb06) SHA1(445df1caa4dd4901e474bb0903bf28e536edf493) )

	ROM_REGION( 0x48000, "game_cpu", 0 ) /* Bankswitched 6809 code */
	ROM_LOAD( "u82",    0x10000, 0x2000, CRC(1596a100) SHA1(0a53d4b79245f2a51de87ec4de6db525aa342f2c) )
	ROM_CONTINUE(                0x20000, 0x2000 )
	ROM_LOAD( "u81",    0x12000, 0x2000, CRC(1c8e053d) SHA1(397f04fdf7c5dfaa33a396b1b41c015a86537ef6) )
	ROM_CONTINUE(                0x22000, 0x2000 )
	ROM_LOAD( "u87",    0x14000, 0x2000, CRC(c80d0512) SHA1(aedd829edd2cb214fa30ae2fe25ad7590c86971b) )
	ROM_CONTINUE(                0x24000, 0x2000 )
	ROM_LOAD( "u86",    0x16000, 0x2000, CRC(8af137d3) SHA1(12768d14b18401d07a793de3412da059b5a33699) )
	ROM_CONTINUE(                0x26000, 0x2000 )

	ROM_LOAD( "u80",    0x30000, 0x2000, CRC(6f53b658) SHA1(39841e4b0a6809ad061a07adcdb8d92fd7652959) )
	ROM_CONTINUE(                0x40000, 0x2000 )
	ROM_LOAD( "u79",    0x32000, 0x2000, CRC(aa6f1db6) SHA1(70cacedb57f3c5646181e26c355f87f1cea1d651) )
	ROM_CONTINUE(                0x42000, 0x2000 )
	ROM_LOAD( "u84",    0x34000, 0x2000, CRC(e856323f) SHA1(d973f8efa3a1f5907b8c09b58043d7b41ff3f0c1) )
	ROM_CONTINUE(                0x44000, 0x2000 )
	ROM_LOAD( "u83",    0x36000, 0x2000, CRC(056fc173) SHA1(426bcea3c2420b8df036122ebb6fc80af89e63d2) )
	ROM_CONTINUE(                0x46000, 0x2000 )

	/* e000 - ffff = Upper half of U85 (lower half is blank) */
	ROM_COPY( "main_code", 0x18000 + 0x2000, 0xe000, 0x2000 )

	ROM_REGION( 0x10000, "frame_cpu", 0 )
	ROM_LOAD( "u63",    0xe000, 0x2000, CRC(e85216d4) SHA1(7f61a93c52a31782116e9825d0aefa58ca3720b9) )

	ROM_REGION( 0x1000, "video_cpu", 0 )
	ROMX_LOAD( "27s29.u29", 0x0, 0x200, CRC(d580672b) SHA1(b56295a5b780ab5e8ff6817ebb084a8dfad8c281), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u28", 0x1, 0x200, CRC(f7976b87) SHA1(c19a1d375c497f1671170c7833952979819c3812), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u27", 0x2, 0x200, CRC(03ebd3ea) SHA1(109f5369bd36bcf0da5928b96566655c6895c737), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u21", 0x3, 0x200, CRC(e232384b) SHA1(cfc3acc86add06b4cb6addb3455d71123fb359ce), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u20", 0x4, 0x200, CRC(0a8e44d8) SHA1(2df46316510b2dbfd4c9913a1460c00d5572d586), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u19", 0x5, 0x200, CRC(de17e5f0) SHA1(3e14768374e1bda25183aee86a82d220b7f58ff9), ROM_SKIP(7))
	ROMX_LOAD( "27s29.u18", 0x6, 0x200, CRC(e33ed0a4) SHA1(41edbdc7c022971ce14bd2f419c92714b796fad7), ROM_SKIP(7))

	ROM_REGION( 0x10000, "sound_cpu", 0 )
	ROM_LOAD( "turbosub.u66", 0xe000, 0x2000, CRC(8db3bcdb) SHA1(e6ae324ba9dad4884e1cb3d67ce099a6f4739456) )

	ROM_REGION( 0xc000, "sound_data", 0)
	ROM_LOAD( "turbosub.u69", 0x0000, 0x4000, CRC(ad04193b) SHA1(2f660302e60a7e68e079a8dd13266a77c077f939) )
	ROM_LOAD( "turbosub.u68", 0x4000, 0x4000, CRC(72e3d09b) SHA1(eefdfcd0c4c32e465f18d40f46cb5bc022c22bfd) )
	ROM_LOAD( "turbosub.u67", 0x8000, 0x4000, CRC(f8ae82e9) SHA1(fd27b9fe7872c3c680a1f71a4a5d5eeaa12e4a19) )

	ROM_REGION( 0x40000, "4bpp", 0)
	ROM_LOAD( "turbosub.u44", 0x00000, 0x4000, CRC(eaa05860) SHA1(f649891dae9354b7f2e46e6a380b52a569229d64) )
	ROM_LOAD( "turbosub.u49", 0x10000, 0x4000, CRC(b4170ac2) SHA1(bdbfc43c891c8d525dcc46fb9d05602263ab69cd) )
	ROM_LOAD( "turbosub.u54", 0x20000, 0x4000, CRC(bebf98d8) SHA1(170502bb44fc6d6bf14d8dac4778b37888c14a7b) )
	ROM_LOAD( "turbosub.u59", 0x30000, 0x4000, CRC(9c1f4397) SHA1(94335f2db2650f8b7e24fc3f92a04b73325ab164) )

	ROM_LOAD( "turbosub.u43", 0x04000, 0x4000, CRC(5d76237c) SHA1(3d50347856039e43290497348447b1c4581f3a33) )
	ROM_LOAD( "turbosub.u48", 0x14000, 0x4000, CRC(cea4e036) SHA1(4afce4f2a09adf9c83ab7188c05cd7236dea16a3) )
	ROM_LOAD( "turbosub.u53", 0x24000, 0x4000, CRC(1352d58a) SHA1(76ae86c365dd4c9e1a6c5af91c01d31e7ee35f0f) )
	ROM_LOAD( "turbosub.u58", 0x34000, 0x4000, CRC(5024d83f) SHA1(a293d92a0ae01901b5618b0250d48e3ba631dfcb) )

	ROM_LOAD( "turbosub.u42", 0x08000, 0x4000, CRC(057a1c72) SHA1(5af89b128b7818550572d02e5ff724c415fa8b8b) )
	ROM_LOAD( "turbosub.u47", 0x18000, 0x4000, CRC(10def494) SHA1(a3ba691eb2b0d782162ffc6c081761965844a3a9) )
	ROM_LOAD( "turbosub.u52", 0x28000, 0x4000, CRC(070d07d6) SHA1(4c81310cd646641a380817fedffab66e76529c97) )
	ROM_LOAD( "turbosub.u57", 0x38000, 0x4000, CRC(5ddb0458) SHA1(d1169882397f364ca38fbd563250b33d13b1a7c6) )

	ROM_LOAD( "turbosub.u41", 0x0c000, 0x4000, CRC(014bb06b) SHA1(97276ba26b60c2907e59b92cc9de5251298579cf) )
	ROM_LOAD( "turbosub.u46", 0x1c000, 0x4000, CRC(3b866e2c) SHA1(c0dd4827a18eb9f4b1055d92544beed10f01fd86) )
	ROM_LOAD( "turbosub.u51", 0x2c000, 0x4000, CRC(43cdcb5c) SHA1(3dd966daa904d3be7be63c584ba033c0e7904d5c) )
	ROM_LOAD( "turbosub.u56", 0x3c000, 0x4000, CRC(6d116adf) SHA1(f808e28cef41dc86e43d8c12966037213da87c87) )

	ROM_REGION( 0x40000, "8bpp_l", 0)
	ROM_LOAD( "turbosub.u4",  0x00000, 0x4000, CRC(08303604) SHA1(f075b645d89a2d91bd9b621748906a9f9890ee60) )
	ROM_LOAD( "turbosub.u14", 0x10000, 0x4000, CRC(83b26c8d) SHA1(2dfa3b45c44652d255c402511bb3810fffb0731d) )
	ROM_LOAD( "turbosub.u24", 0x20000, 0x4000, CRC(6bbb6cb3) SHA1(d513e547a05b34076bb8261abd51301ac5f3f5d4) )
	ROM_LOAD( "turbosub.u34", 0x30000, 0x4000, CRC(7b844f4a) SHA1(82467eb7e116f9f225711a1698c151945e1de6e4) )

	ROM_LOAD( "turbosub.u3",  0x04000, 0x4000, CRC(825ef29c) SHA1(affadd0976f793b8bdbcbc4768b7de27121e7b11) )
	ROM_LOAD( "turbosub.u13", 0x14000, 0x4000, CRC(350cc17a) SHA1(b98d16be997fc0576d3206f51f29ce3e257492d3) )
	ROM_LOAD( "turbosub.u23", 0x24000, 0x4000, CRC(b1531916) SHA1(805a23f40aa875f431e835fdaceba87261c14155) )
	ROM_LOAD( "turbosub.u33", 0x34000, 0x4000, CRC(0d5130cb) SHA1(7e4e4e5ea50c581a60d15964571464029515c720) )

	ROM_LOAD( "turbosub.u2",  0x08000, 0x4000, CRC(a8b8c032) SHA1(20512a3a1f8b9c0361e6f5a7e9a50605be3ae650) )
	ROM_LOAD( "turbosub.u12", 0x18000, 0x4000, CRC(a2c4badf) SHA1(267af1be6261833211270af25045e306efffee80) )
	ROM_LOAD( "turbosub.u22", 0x28000, 0x4000, CRC(97b7cf0e) SHA1(888fb2f384a5cba8a6f7569886eb6dc27e2b024f) )
	ROM_LOAD( "turbosub.u32", 0x38000, 0x4000, CRC(b286710e) SHA1(5082db13630ba0967006619027c39ee3607b838d) )

	ROM_LOAD( "turbosub.u1",  0x0c000, 0x4000, CRC(88b0a7a9) SHA1(9012c8059cf60131efa6a0432accd87813187206) )
	ROM_LOAD( "turbosub.u11", 0x1c000, 0x4000, CRC(9f0ff723) SHA1(54b52b4ebc32f10aa32c799ac819928290e70455) )
	ROM_LOAD( "turbosub.u21", 0x2c000, 0x4000, CRC(b4122fe2) SHA1(50e8b488a7b7f739336b60a3fd8a5b14f5010b75) )
	ROM_LOAD( "turbosub.u31", 0x3c000, 0x4000, CRC(3fa15c78) SHA1(bf5cb85fc26b5045ad5acc944c917b068ace2c49) )

	ROM_REGION( 0x40000, "8bpp_r", 0)
	ROM_LOAD( "turbosub.u9",  0x00000, 0x4000, CRC(9a03eadf) SHA1(25ee1ebe52f030b2fa09d76161e46540c91cbc4c) )
	ROM_LOAD( "turbosub.u19", 0x10000, 0x4000, CRC(498253b8) SHA1(dd74d4f9f19d8a746415baea604116faedb4fb31) )
	ROM_LOAD( "turbosub.u29", 0x20000, 0x4000, CRC(809c374f) SHA1(d3849eed8441e4641ffcbca7c83ee3bb16681a0b) )
	ROM_LOAD( "turbosub.u39", 0x30000, 0x4000, CRC(3e4e0681) SHA1(ac834f6823ffe835d6f149e79c1d31ae2b89e85d) )

	ROM_LOAD( "turbosub.u8",  0x04000, 0x4000, CRC(01118737) SHA1(3a8e998b80dffe82296170273dcbbe9870c5b695) )
	ROM_LOAD( "turbosub.u18", 0x14000, 0x4000, CRC(39fd8e57) SHA1(392f8a8cf58fc4813de840775d9c53561488152d) )
	ROM_LOAD( "turbosub.u28", 0x24000, 0x4000, CRC(0628586d) SHA1(e37508c2812e1c98659aaba9c495e7396842614e) )
	ROM_LOAD( "turbosub.u38", 0x34000, 0x4000, CRC(7d597a7e) SHA1(2f48faf75406ab3ff0b954040b74e68b7ca6f7a5) )

	ROM_LOAD( "turbosub.u7",  0x08000, 0x4000, CRC(50eea315) SHA1(567dbb3cb3a75a7507f4cb4748c7dd878e69d6b7) )
	ROM_LOAD( "turbosub.u17", 0x18000, 0x4000, CRC(8a9e19e6) SHA1(19067e153c0002edfd4a756f92ad75d9a0cbc3dd) )
	ROM_LOAD( "turbosub.u27", 0x28000, 0x4000, CRC(1c81a8d9) SHA1(3d13d1ccd7ec3dddf2a27600eb64b5be386e868c) )
	ROM_LOAD( "turbosub.u37", 0x38000, 0x4000, CRC(59f978cb) SHA1(e99d6378de941cad92e9702fcb18aea87acd371f) )

	ROM_LOAD( "turbosub.u6",  0x0c000, 0x4000, CRC(841e00bd) SHA1(f777cc8dd8dd7c8baa2007355a76db782a218efc) )
	ROM_LOAD( "turbosub.u16", 0x1c000, 0x4000, CRC(d3b63d81) SHA1(e86dd64825f6d9e7bebc26413f524a8962f68f2d) )
	ROM_LOAD( "turbosub.u26", 0x2c000, 0x4000, CRC(867cfe32) SHA1(549e4e557d63dfab8e8c463916512a1b422ce425) )
	ROM_LOAD( "turbosub.u36", 0x3c000, 0x4000, CRC(0d8ebc21) SHA1(7ae65edae05869376caa975ff2c778a08e8ad8a2) )

	ROM_REGION( 0x260, "proms", 0)
	ROM_LOAD( "27s29.u123",    0x0000, 0x0200, CRC(b2e8770e) SHA1(849292a6b30bb0e6547ce3232438136897a651b0) )
	ROM_LOAD( "6331_snd.u2",   0x0200, 0x0020, CRC(f1328a5e) SHA1(44d4e802988415d24a0b9eaa38300f5add3a2727) )
	ROM_LOAD( "6331_rom.u74",  0x0220, 0x0020, CRC(7b72b34e) SHA1(bc4d67a6993beb36a161368428e648d0492ac436) )
	ROM_LOAD( "6331_vid.u155", 0x0240, 0x0020, CRC(63371737) SHA1(f08c03c81322c0de9ee64b4a9f11a1422c5bd463) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1985, turbosub,  0,        esripsys, turbosub, esripsys_state, esripsys, ROT0, "Entertainment Sciences", "Turbo Sub (prototype rev. TSCA)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, turbosub7, turbosub, esripsys, turbosub, esripsys_state, esripsys, ROT0, "Entertainment Sciences", "Turbo Sub (prototype rev. TSC7)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, turbosub6, turbosub, esripsys, turbosub, esripsys_state, esripsys, ROT0, "Entertainment Sciences", "Turbo Sub (prototype rev. TSC6)", MACHINE_SUPPORTS_SAVE )
