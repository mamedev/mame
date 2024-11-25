// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    Philips CD-i consoles and games
    -------------------------------

    Preliminary MAME driver by Ryan Holtz
    Help provided by CD-i Fan


*******************************************************************************

STATUS:

  CD-i:
- The SLAVE MCU cannot be low-level emulated until there is proper /DTACK
  support in the 68k core. A2/A1 and D7..D0 are hooked up to Port C bits 1/0
  and Port A respectively, the Read/Write signal is sent to Port D bit 7, and
  /DTACK is received from Port B bit 6. The MCU therefore has the capability to
  pull /DTACK high on a data read in order to tell the 68k to hold off until
  data is ready.

- There is currently a lack of documentation on any of the chips used for
  audio in any of the CD-i models. The CDIC, which was used on Mono-I boards,
  is partially emulated thanks to information provided by CD-i Fan, the author
  of CD-i Emu. Desired documentation includes:
  * GSX38KG307CE46, "ATTEX"
  * Philips IMS66490, "CDIC" ADPCM decoder
  * PC85010 DSP

TODO:

- Screen clocks are a hack right now; they should be exactly CLOCK_A/2. However, the
  MCD-212 documentation states in both tables and timing diagrams that vertical retrace
  has an additional half-line even in non-interlaced mode, which cannot be represented
  in the current screen-timing framework. The input clock has been adjusted downward
  to factor out this half-line, resulting in the expected 50Hz exactly in PAL mode.

- Proper abstraction of the 68070's internal devices (UART, DMA, Timers, etc.)

- Mono-I: Full emulation of the CDIC, as well as the SERVO and SLAVE MCUs

- Mono-II: SERVO and SLAVE I/O device hookup
- Mono-II: DSP56k hookup

*******************************************************************************/

#include "emu.h"
#include "cdi.h"

#include "cpu/m6805/m6805.h"
#include "imagedev/cdromimg.h"
#include "machine/timekpr.h"
#include "sound/cdda.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "cdrom.h"

#include "cdi.lh"

// TODO: NTSC system clock is 30.2098 MHz; additional 4.9152 MHz XTAL provided for UART
#define CLOCK_A 30_MHz_XTAL

#define LOG_DVC             (1U << 1)
#define LOG_QUIZARD_READS   (1U << 2)
#define LOG_QUIZARD_WRITES  (1U << 3)
#define LOG_QUIZARD_OTHER   (1U << 4)
#define LOG_UART            (1U << 5)

#define VERBOSE         (0)
#include "logmacro.h"

#define ENABLE_UART_PRINTING (0)

/*************************
*      Memory maps       *
*************************/

void cdi_state::cdimono1_mem(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(cdi_state::bus_error_r), FUNC(cdi_state::bus_error_w));
	map(0x000000, 0x07ffff).rw(FUNC(cdi_state::plane_r<0>), FUNC(cdi_state::plane_w<0>)).share("plane0");
	map(0x200000, 0x27ffff).rw(FUNC(cdi_state::plane_r<1>), FUNC(cdi_state::plane_w<1>)).share("plane1");
	map(0x300000, 0x303bff).rw(m_cdic, FUNC(cdicdic_device::ram_r), FUNC(cdicdic_device::ram_w));
#if ENABLE_UART_PRINTING
	map(0x301400, 0x301403).r(m_maincpu, FUNC(scc68070_device::uart_loopback_enable));
#endif
	map(0x303c00, 0x303fff).rw(m_cdic, FUNC(cdicdic_device::regs_r), FUNC(cdicdic_device::regs_w));
	map(0x310000, 0x317fff).rw(m_slave_hle, FUNC(cdislave_hle_device::slave_r), FUNC(cdislave_hle_device::slave_w));
	map(0x318000, 0x31ffff).noprw();
	map(0x320000, 0x323fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);    /* nvram (only low bytes used) */
	map(0x400000, 0x47ffff).r(FUNC(cdi_state::main_rom_r));
	map(0x4fffe0, 0x4fffff).m(m_mcd212, FUNC(mcd212_device::map));
	map(0x500000, 0x57ffff).ram();
	map(0xd00000, 0xdfffff).ram(); // DVC RAM block 1
	map(0xe00000, 0xe7ffff).rw(FUNC(cdi_state::dvc_r), FUNC(cdi_state::dvc_w));
	map(0xe80000, 0xefffff).ram(); // DVC RAM block 2
}

void cdi_state::cdimono2_mem(address_map &map)
{
	map(0x000000, 0x07ffff).ram().share("plane0");
	map(0x200000, 0x27ffff).ram().share("plane1");
#if ENABLE_UART_PRINTING
	map(0x301400, 0x301403).r(m_maincpu, FUNC(scc68070_device::uart_loopback_enable));
#endif
	map(0x320000, 0x323fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);    /* nvram (only low bytes used) */
	map(0x400000, 0x47ffff).r(FUNC(cdi_state::main_rom_r));
	map(0x4fffe0, 0x4fffff).m(m_mcd212, FUNC(mcd212_device::map));
}

void cdi_state::cdi910_mem(address_map &map)
{
	map(0x000000, 0x07ffff).ram().share("plane0");
	map(0x180000, 0x1fffff).rom().region("maincpu", 0); // boot vectors point here
	map(0x200000, 0x27ffff).ram().share("plane1");
#if ENABLE_UART_PRINTING
	map(0x301400, 0x301403).r(m_maincpu, FUNC(scc68070_device::uart_loopback_enable));
#endif
	map(0x320000, 0x323fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);    /* nvram (only low bytes used) */
	map(0x4fffe0, 0x4fffff).m(m_mcd212, FUNC(mcd212_device::map));
	map(0x500000, 0xffffff).noprw();
}


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( cdi )
	PORT_START("MOUSEX")
	PORT_BIT(0xffff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(2)

	PORT_START("MOUSEY")
	PORT_BIT(0xffff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2)

	PORT_START("MOUSEBTN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Button 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Button 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Button 3")
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( cdimono2 )
INPUT_PORTS_END

static INPUT_PORTS_START( quizard )
	PORT_START("P0")
	PORT_DIPNAME( 0x07, 0x05, "Settings" )
	PORT_DIPSETTING(    0x00, "1 Coin, 0 Bonus Limit, 0 Bonus Number" )
	PORT_DIPSETTING(    0x01, "2 Coins, 0 Bonus Limit, 0 Bonus Number" )
	PORT_DIPSETTING(    0x02, "1 Coin, 2 Bonus Limit, 1 Bonus Number" )
	PORT_DIPSETTING(    0x03, "1 Coin, 3 Bonus Limit, 1 Bonus Number" )
	PORT_DIPSETTING(    0x04, "1 Coin, 5 Bonus Limit, 1 Bonus Number" )
	PORT_DIPSETTING(    0x05, "1 Coin, 5 Bonus Limit, 2 Bonus Number" )
	PORT_DIPSETTING(    0x06, "1 Coin, 10 Bonus Limit, 2 Bonus Number" )
	PORT_DIPSETTING(    0x07, "2 Coins, 4 Bonus Limit, 1 Bonus Number" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xc8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 1 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Player 1 B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Player 1 C")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Player 2 A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Player 2 B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Player 2 C")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************
*  Machine Initialization  *
***************************/

void cdi_state::machine_reset()
{
	uint16_t *src = &m_main_rom[0];
	uint16_t *dst = &m_plane_ram[0][0];
	memcpy(dst, src, 0x8);
}

void quizard_state::machine_start()
{
	save_item(NAME(m_boot_press));

	m_boot_timer = timer_alloc(FUNC(quizard_state::boot_press_tick), this);

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(9600);
}

void quizard_state::machine_reset()
{
	cdi_state::machine_reset();

	m_boot_press = false;
	m_boot_timer->adjust(attotime::from_seconds(13), 1);
	m_mcu_p3 = 0x05; // RTS|RXD
}


/***************************
*  Wait-State Handling     *
***************************/

template<int Channel>
uint16_t cdi_state::plane_r(offs_t offset, uint16_t mem_mask)
{
	m_maincpu->eat_cycles(m_mcd212->ram_dtack_cycle_count<Channel>());
	return m_plane_ram[Channel][offset];
}

template<int Channel>
void cdi_state::plane_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_maincpu->eat_cycles(m_mcd212->ram_dtack_cycle_count<Channel>());
	COMBINE_DATA(&m_plane_ram[Channel][offset]);
}

uint16_t cdi_state::main_rom_r(offs_t offset)
{
	m_maincpu->eat_cycles(m_mcd212->rom_dtack_cycle_count());
	return m_main_rom[offset];
}


/**********************
*  BERR Handling      *
**********************/

uint16_t cdi_state::bus_error_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(offset*2, true, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xff;
}

void cdi_state::bus_error_w(offs_t offset, uint16_t data)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(offset*2, false, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}


/**********************
*  Quizard Protection *
**********************/

TIMER_CALLBACK_MEMBER(quizard_state::boot_press_tick)
{
	m_boot_press = (bool)param;
	if (m_boot_press)
		m_boot_timer->adjust(attotime::from_msec(250), 0);
}

uint8_t quizard_state::mcu_button_press()
{
	return (uint8_t)m_boot_press;
}

void quizard_state::mcu_rtsn_from_cpu(int state)
{
	LOGMASKED(LOG_UART, "MCU receiving RTSN from CPU: %d\n", state);
}

void quizard_state::mcu_rx_from_cpu(uint8_t data)
{
	LOGMASKED(LOG_UART, "MCU receiving %02x from CPU\n", data);

	transmit_register_setup(data);
}

uint8_t quizard_state::mcu_p0_r()
{
	const uint8_t data = m_inputs[0]->read();
	LOGMASKED(LOG_QUIZARD_READS, "%s: MCU Port 0 Read (%02x)\n", machine().describe_context(), data);
	return data;
}

uint8_t quizard_state::mcu_p1_r()
{
	uint8_t data = m_inputs[1]->read();
	if (BIT(~m_inputs[0]->read(), 4))
		data &= ~(1 << 4);
	LOGMASKED(LOG_QUIZARD_READS, "%s: MCU Port 1 Read (%02x)\n", machine().describe_context(), data);
	return data;
}

uint8_t quizard_state::mcu_p2_r()
{
	const uint8_t data = m_inputs[2]->read();
	LOGMASKED(LOG_QUIZARD_READS, "%s: MCU Port 2 Read (%02x)\n", machine().describe_context(), data);
	return data;
}

uint8_t quizard_state::mcu_p3_r()
{
	LOGMASKED(LOG_QUIZARD_READS, "%s: MCU Port 3 Read (%02x)\n", machine().describe_context(), m_mcu_p3);
	return m_mcu_p3;
}

void quizard_state::mcu_p0_w(uint8_t data)
{
	LOGMASKED(LOG_QUIZARD_WRITES, "%s: MCU Port 0 Write (%02x)\n", machine().describe_context(), data);
}

void quizard_state::mcu_p1_w(uint8_t data)
{
	LOGMASKED(LOG_QUIZARD_WRITES, "%s: MCU Port 1 Write (%02x)\n", machine().describe_context(), data);
}

void quizard_state::mcu_p2_w(uint8_t data)
{
	LOGMASKED(LOG_QUIZARD_WRITES, "%s: MCU Port 2 Write (%02x)\n", machine().describe_context(), data);
}

void quizard_state::mcu_p3_w(uint8_t data)
{
	LOGMASKED(LOG_QUIZARD_WRITES, "%s: MCU Port 3 Write (%02x)\n", machine().describe_context(), data);
	rx_w(BIT(data, 1));
	m_maincpu->uart_ctsn(BIT(data, 6));
}

/*************************
*     DVC cartridge      *
*************************/

uint16_t cdi_state::dvc_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_DVC, "%s: dvc_r: %08x = 0000 & %04x\n", machine().describe_context(), 0xe80000 + (offset << 1), mem_mask);
	return 0;
}

void cdi_state::dvc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DVC, "%s: dvc_w: %08x = %04x & %04x\n", machine().describe_context(), 0xe80000 + (offset << 1), data, mem_mask);
}

/*************************
*       LCD screen       *
*************************/

static const uint16_t cdi220_lcd_char[20*22] =
{
	0x2000, 0x2000, 0x2000, 0x2000, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0002, 0x0002, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0002, 0x0002, 0x0002, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0002, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0000, 0x8000, 0x8000, 0x8000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0000, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0000, 0x0000, 0x8000, 0x8000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0000, 0x0000, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0200, 0x0200, 0x0200, 0x0200,
	0x1000, 0x1000, 0x1000, 0x1000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0000, 0x0000, 0x0010, 0x0010, 0x0001, 0x0001, 0x0001, 0x0001, 0x0008, 0x0008, 0x0000, 0x0000, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0000, 0x0010, 0x0010, 0x0010, 0x0001, 0x0001, 0x0001, 0x0001, 0x0008, 0x0008, 0x0008, 0x0000, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0010, 0x0010, 0x0010, 0x0010, 0x0001, 0x0001, 0x0001, 0x0001, 0x0008, 0x0008, 0x0008, 0x0008, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0010, 0x0010, 0x0010, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0008, 0x0008, 0x0008, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0010, 0x0010, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0008, 0x0008, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0400, 0x0400, 0x0400, 0x0400
};

uint32_t cdi_state::screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_slave_hle.found())
		return 0;

	for (int y = 0; y < 22; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);

		for (int lcd = 0; lcd < 8; lcd++)
		{
			uint16_t data = (m_slave_hle->get_lcd_state()[lcd*2] << 8) |
							m_slave_hle->get_lcd_state()[lcd*2 + 1];
			for (int x = 0; x < 20; x++)
			{
				if (data & cdi220_lcd_char[y*20 + x])
				{
					scanline[(7 - lcd)*24 + x] = rgb_t::white();
				}
				else
				{
					scanline[(7 - lcd)*24 + x] = rgb_t::black();
				}
			}
		}
	}

	return 0;
}

/*************************
*    Machine Drivers     *
*************************/

// CD-i Mono-I system base
void cdi_state::cdimono1_base(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdimono1_mem);
	m_maincpu->iack4_callback().set(m_cdic, FUNC(cdicdic_device::intack_r));

	MCD212(config, m_mcd212, CLOCK_A, m_plane_ram[0], m_plane_ram[1]);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14976000, 960, 0, 768, 312, 32, 312);
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_screen_update(m_mcd212, FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd, SCREEN_TYPE_RASTER);
	m_lcd->set_refresh_hz(50);
	m_lcd->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_lcd->set_size(192, 22);
	m_lcd->set_visarea(0, 192-1, 0, 22-1);
	m_lcd->set_screen_update(FUNC(cdi_state::screen_update_cdimono1_lcd));

	PALETTE(config, "palette").set_entries(0x100);

	config.set_default_layout(layout_cdi);

	// IMS66490 CDIC input clocks are 22.5792 MHz and 19.3536 MHz
	// DSP input clock is 7.5264 MHz
	CDI_CDIC(config, m_cdic, 45.1584_MHz_XTAL / 2);
	m_cdic->set_clock2(45.1584_MHz_XTAL * 3 / 7); // generated by PLL circuit incorporating 19.3575 MHz XTAL
	m_cdic->intreq_callback().set(m_maincpu, FUNC(scc68070_device::in4_w));

	CDI_SLAVE_HLE(config, m_slave_hle, 0);
	m_slave_hle->int_callback().set(m_maincpu, FUNC(scc68070_device::in2_w));

	CDROM(config, m_cdrom);
	m_cdrom->set_interface("cdi_cdrom");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMADAC(config, m_dmadac[0]);
	m_dmadac[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	DMADAC(config, m_dmadac[1]);
	m_dmadac[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	MK48T08(config, "mk48t08");
}

// CD-i model 220 (Mono-II, NTSC)
void cdi_state::cdimono2(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdimono2_mem);

	MCD212(config, m_mcd212, CLOCK_A, m_plane_ram[0], m_plane_ram[1]);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14976000, 960, 0, 768, 312, 32, 312);
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_screen_update(m_mcd212, FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd, SCREEN_TYPE_RASTER);
	m_lcd->set_refresh_hz(60);
	m_lcd->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_lcd->set_size(192, 22);
	m_lcd->set_visarea(0, 192-1, 0, 22-1);
	m_lcd->set_screen_update(FUNC(cdi_state::screen_update_cdimono1_lcd));

	PALETTE(config, "palette").set_entries(0x100);

	config.set_default_layout(layout_cdi);

	M68HC05C8(config, m_servo, 4_MHz_XTAL);
	M68HC05C8(config, m_slave, 4_MHz_XTAL);

	CDROM(config, m_cdrom).set_interface("cdi_cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("cdi").set_filter("!DVC");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMADAC(config, m_dmadac[0]);
	m_dmadac[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	DMADAC(config, m_dmadac[1]);
	m_dmadac[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	MK48T08(config, "mk48t08");
}

void cdi_state::cdi910(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdi910_mem);

	MCD212(config, m_mcd212, CLOCK_A, m_plane_ram[0], m_plane_ram[1]);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14976000, 960, 0, 768, 312, 32, 312);
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_screen_update(m_mcd212, FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd, SCREEN_TYPE_RASTER);
	m_lcd->set_refresh_hz(60);
	m_lcd->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_lcd->set_size(192, 22);
	m_lcd->set_visarea(0, 192-1, 0, 22-1);
	m_lcd->set_screen_update(FUNC(cdi_state::screen_update_cdimono1_lcd));

	PALETTE(config, "palette").set_entries(0x100);

	config.set_default_layout(layout_cdi);

	M68HC05C8(config, m_servo, 4_MHz_XTAL);
	M68HC05C8(config, m_slave, 4_MHz_XTAL);

	CDROM(config, "cdrom").set_interface("cdi_cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("cdi").set_filter("!DVC");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMADAC(config, m_dmadac[0]);
	m_dmadac[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	DMADAC(config, m_dmadac[1]);
	m_dmadac[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	MK48T08(config, "mk48t08");
}

// CD-i Mono-I, with CD-ROM image device (MESS) and Software List (MESS)
void cdi_state::cdimono1(machine_config &config)
{
	cdimono1_base(config);

	m_slave_hle->read_mousex().set_ioport("MOUSEX");
	m_slave_hle->read_mousey().set_ioport("MOUSEY");
	m_slave_hle->read_mousebtn().set_ioport("MOUSEBTN");

	SOFTWARE_LIST(config, "cd_list").set_original("cdi").set_filter("!DVC");
}

void quizard_state::quizard(machine_config &config)
{
	cdimono1_base(config);
	m_cdrom->add_region("cdrom");

	m_maincpu->set_addrmap(AS_PROGRAM, &quizard_state::cdimono1_mem);
	m_maincpu->uart_rtsn_callback().set(FUNC(quizard_state::mcu_rtsn_from_cpu));
	m_maincpu->uart_tx_callback().set(FUNC(quizard_state::mcu_rx_from_cpu));

	I8751(config, m_mcu, 11.0592_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(quizard_state::mcu_p0_r));
	m_mcu->port_in_cb<1>().set(FUNC(quizard_state::mcu_p1_r));
	m_mcu->port_in_cb<2>().set(FUNC(quizard_state::mcu_p2_r));
	m_mcu->port_in_cb<3>().set(FUNC(quizard_state::mcu_p3_r));
	m_mcu->port_out_cb<0>().set(FUNC(quizard_state::mcu_p0_w));
	m_mcu->port_out_cb<1>().set(FUNC(quizard_state::mcu_p1_w));
	m_mcu->port_out_cb<2>().set(FUNC(quizard_state::mcu_p2_w));
	m_mcu->port_out_cb<3>().set(FUNC(quizard_state::mcu_p3_w));

	m_slave_hle->read_mousebtn().set(FUNC(quizard_state::mcu_button_press));
}

void quizard_state::tra_callback()
{
	if (transmit_register_get_data_bit())
		m_mcu_p3 |= 1;
	else
		m_mcu_p3 &= ~1;
}

void quizard_state::rcv_complete()
{
	receive_register_extract();

	const uint8_t data = get_received_char();
	LOGMASKED(LOG_QUIZARD_OTHER, "%s: MCU transmitting %02x\n", machine().describe_context(), data);
	m_maincpu->uart_rx(data);
}

/*************************
*        Rom Load        *
*************************/

ROM_START( cdimono1 )
	ROM_REGION(0x80000, "maincpu", 0) // these roms need byteswapping
	ROM_SYSTEM_BIOS( 0, "mcdi200", "Magnavox CD-i 200" )
	ROMX_LOAD( "cdi200.rom", 0x000000, 0x80000, CRC(40c4e6b9) SHA1(d961de803c89b3d1902d656ceb9ce7c02dccb40a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pcdi220", "Philips CD-i 220 F2" )
	ROMX_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "pcdi220_alt", "Philips CD-i 220?" ) // doesn't boot
	ROMX_LOAD( "cdi220.rom", 0x000000, 0x80000, CRC(584c0af8) SHA1(5d757ab46b8c8fc36361555d978d7af768342d47), ROM_BIOS(2) )

	// The two MCU dumps below are taken from the cdi910. We still need dumps from a Mono-I board in case the revisions are different.
	ROM_REGION(0x2000, "servo", 0)
	ROM_LOAD( "zx405037p__cdi_servo_2.1__b43t__llek9215.mc68hc705c8a_withtestrom.7201", 0x0000, 0x2000, CRC(7a3af407) SHA1(fdf8d78d6a0df4a56b5b963d72eabd39fcec163f) BAD_DUMP )

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zx405042p__cdi_slave_2.0__b43t__zzmk9213.mc68hc705c8a_withtestrom.7206", 0x0000, 0x2000, CRC(688cda63) SHA1(56d0acd7caad51c7de703247cd6d842b36173079) BAD_DUMP )
ROM_END

ROM_START( cdi910 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "cdi910", "CD-I 910-17P Mini-MMC" )
	ROMX_LOAD( "philips__cd-i_2.1__mb834200b-15__26b_aa__9224_z01.tc574200.7211", 0x000000, 0x80000, CRC(4ae3bee3) SHA1(9729b4ee3ce0c17172d062339c47b1ab822b222b), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE )
	ROM_SYSTEM_BIOS( 1, "cdi910_alt", "alt" )
	ROMX_LOAD( "cdi910.rom", 0x000000, 0x80000, CRC(2f3048d2) SHA1(11c4c3e602060518b52e77156345fa01f619e793), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE )

	ROM_REGION(0x2000, "servo", 0)
	ROM_LOAD( "zx405037p__cdi_servo_2.1__b43t__llek9215.mc68hc705c8a_withtestrom.7201", 0x0000, 0x2000, CRC(7a3af407) SHA1(fdf8d78d6a0df4a56b5b963d72eabd39fcec163f) )

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zx405042p__cdi_slave_2.0__b43t__zzmk9213.mc68hc705c8a_withtestrom.7206", 0x0000, 0x2000, CRC(688cda63) SHA1(56d0acd7caad51c7de703247cd6d842b36173079) )

	ROM_REGION(0x2000, "pals", 0)
	ROM_LOAD( "ti_portugal_206xf__tibpal20l8-15cnt__m7205n.7205.bin",      0x0000, 0x144, CRC(dd167e0d) SHA1(2ba82a4619d7a0f19e62e02a2841afd4d45d56ba) )
	ROM_LOAD( "ti_portugal_774_206xf__tibpal16l8-10cn_m7204n.7204.bin",    0x0000, 0x104, CRC(04e6bd37) SHA1(153d1a977291bedb7420484a9f889325dbd3628e) )
ROM_END

ROM_START( cdimono2 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "philips__cdi-220_ph3_r1.2__mb834200b-15__02f_aa__9402_z04.tc574200-le._1.7211", 0x000000, 0x80000, CRC(17d723e7) SHA1(6c317a82e35d60ca5e7a74fc99f665055693169d) )

	ROM_REGION(0x2000, "servo", 0)
	ROM_LOAD( "zc405351p__servo_cdi_4.1__0d67p__lluk9404.mc68hc705c8a.7490", 0x0000, 0x2000, CRC(2bc8e4e9) SHA1(8cd052b532fc052d6b0077261c12f800e8655bb1) )

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zc405352p__slave_cdi_4.1__0d67p__lltr9403.mc68hc705c8a.7206", 0x0000, 0x2000, CRC(5b19da07) SHA1(cf02d84977050c71e87a38f1249e83c43a93949b) )
ROM_END

ROM_START( cdi490a )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "cdi490", "CD-i 490" )
	ROMX_LOAD( "cdi490a.rom", 0x000000, 0x80000, CRC(e2f200f6) SHA1(c9bf3c4c7e4fe5cbec3fe3fc993c77a4522ca547), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE  )

	ROM_REGION(0x60000, "mpegs", 0) // keep these somewhere
	ROM_LOAD( "impega.rom", 0x00000, 0x40000, CRC(84d6f6aa) SHA1(02526482a0851ea2a7b582d8afaa8ef14a8bd914) ) // 1ST AND 2ND HALF IDENTICAL
	// Philips CD-i - DVC card 22ER9141
	ROM_LOAD16_BYTE( "fmv ffd9 p7308 r4.1 vmpeg.bin", 0x40000, 0x10000, CRC(30ba9273) SHA1(d8adca0627b356ced6131b9458ac1175e43e6548) )
	ROM_LOAD16_BYTE( "fmv 4ba9 p7307 r4.1 vmpeg.bin", 0x40001, 0x10000, CRC(623edb1f) SHA1(4c6b11e28ad4c2f5c2e439f7910a783e0a79d1a9) )
ROM_END

ROM_START( cdibios ) // for the quizard sets
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "mcdi200", "Magnavox CD-i 200" )
	ROMX_LOAD( "cdi200.rom", 0x000000, 0x80000, CRC(40c4e6b9) SHA1(d961de803c89b3d1902d656ceb9ce7c02dccb40a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pcdi220", "Philips CD-i 220 F2" )
	ROMX_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e), ROM_BIOS(1) )

	// The MCU dump below is taken from the cdi910. We still need a dump from a Mono-I board SLAVE MCU in case the revisions are different.
	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zx405042p__cdi_slave_2.0__b43t__zzmk9213.mc68hc705c8a_withtestrom.7206", 0x0000, 0x2000, CRC(688cda63) SHA1(56d0acd7caad51c7de703247cd6d842b36173079) BAD_DUMP )
ROM_END

/*  Quizard notes

    The MCU controls the protection sequence, which in turn controls the game display language.
    Each Quizard game (1,2,3,4) requires its own MCU, you can upgrade between revisions by changing
    just the CD, but not between games as a new MCU is required.

    MCU Notes:
    i8751 MCU dumps confirmed good on original hardware
    Italian language MCU for Quizard 1 is dumped
    German language MCUs for Quizard 1 through 4 are dumped
    Czech language MCU for Quizard 4 is dumped
    Alt. German language MCU for Quizard 2 is known to exist (DE 122 D3, not dumped)

*/


#define QUIZARD_BIOS_ROM \
	ROM_REGION(0x80000, "maincpu", 0) \
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

//********************************************************
//                     Quizard (1)
//********************************************************

#define QUIZARD1_CHD_10 \
	DISK_REGION( "cdrom" ) \
	DISK_IMAGE_READONLY( "quizard10", 0, SHA1(5715db50f0d5ffe06f47c0943f4bf0481ab6048e) ) // Dumped via BurnAtOnce 0.99.5, CHDMAN 0.163, TS-L633R drive

// CD-ROM printed 01/95
#define QUIZARD1_CHD_12 \
	DISK_REGION( "cdrom" ) \
	DISK_IMAGE_READONLY( "quizard12", 0, BAD_DUMP SHA1(6e41683b96b74e903040842aeb18437ad7813c82) )

#define QUIZARD1_CHD_17 \
	DISK_REGION( "cdrom" ) \
	DISK_IMAGE_READONLY( "quizard17", 0, BAD_DUMP SHA1(4bd698f076505b4e17be978481bce027eb47123b) )

#define QUIZARD1_CHD_18 \
	DISK_REGION( "cdrom" ) \
	DISK_IMAGE_READONLY( "quizard18", 0, BAD_DUMP SHA1(ede873b22957f2a707bbd3039e962ef2ca5aedbd) )

// MCU Type: Intel D8751H MCU
#define QUIZARD1_MCU_DE \
	ROM_REGION(0x1000, "mcu", 0) \
	ROM_LOAD( "de_11_d3.bin", 0x0000, 0x1000, CRC(95f45b6b) SHA1(51b34956539b1e2cf0306f243a970750f1e18d01) ) // German

#define QUIZARD1_MCU_IT \
	ROM_REGION(0x1000, "mcu", 0) \
	ROM_LOAD( "it_11_i2.bin", 0x0000, 0x1000, CRC(e00dc02c) SHA1(e4ef1ea47c242879a99c9d54cfc008ae99a651cb) ) // Italian

ROM_START( quizard )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_18
	QUIZARD1_MCU_DE
ROM_END

ROM_START( quizard_17 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_17
	QUIZARD1_MCU_DE
ROM_END

ROM_START( quizard_12 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_12
	QUIZARD1_MCU_DE
ROM_END

ROM_START( quizard_10 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_10
	QUIZARD1_MCU_DE
ROM_END

ROM_START( quizardi )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_18
	QUIZARD1_MCU_IT
ROM_END

ROM_START( quizardi_17 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_17
	QUIZARD1_MCU_IT
ROM_END

ROM_START( quizardi_12 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_12
	QUIZARD1_MCU_IT
ROM_END

ROM_START( quizardi_10 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_10
	QUIZARD1_MCU_IT
ROM_END

//********************************************************
//                     Quizard 2
//********************************************************

ROM_START( quizard2 ) /* CD-ROM printed ??/?? */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard23", 0, BAD_DUMP SHA1(cd909d9a54275d6f2d36e03e83eea996e781b4d3) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "dn_122_d3.bin", 0x0000, 0x1000, CRC(d48063ea) SHA1(b512fa5e53f296a180340e09b53613dd1c0d38bc) ) // German language - DE 122 D3 known to exist
ROM_END

ROM_START( quizard2_22 )
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard22", 0, BAD_DUMP SHA1(03c8fdcf27ead6e221691111e8c679b551099543) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "dn_122_d3.bin", 0x0000, 0x1000, CRC(d48063ea) SHA1(b512fa5e53f296a180340e09b53613dd1c0d38bc) ) // German language - DE 122 D3 known to exist
ROM_END


//********************************************************
//                     Quizard 3
//********************************************************

ROM_START( quizard3 ) /* CD-ROM printed ??/?? */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard34", 0, BAD_DUMP SHA1(37ad49b72b5175afbb87141d57bc8604347fe032) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "de_132_d3.bin", 0x0000, 0x1000, CRC(8858251e) SHA1(2c1005a74bb6f0c2918dff4ab6326528eea48e1f) ) // German language
ROM_END

ROM_START( quizard3a ) /* CD-ROM printed ??/?? */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard34", 0, BAD_DUMP SHA1(37ad49b72b5175afbb87141d57bc8604347fe032) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "de_132_a1.bin", 0x0000, 0x1000, CRC(313ac673) SHA1(cb0ee7e9a6eaa5f4d000f5ea99b7ee4c440b31d1) ) // German language - earlier version of MCU code
ROM_END

ROM_START( quizard3_32 )
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard32", 0, BAD_DUMP SHA1(31e9fa2169aa44d799c37170b238134ab738e1a1) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "de_132_d3.bin", 0x0000, 0x1000, CRC(8858251e) SHA1(2c1005a74bb6f0c2918dff4ab6326528eea48e1f) ) // German language
ROM_END


//********************************************************
//                     Quizard 4
//********************************************************

ROM_START( quizard4 ) /* CD-ROM printed 09/98 */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r42", 0, BAD_DUMP SHA1(a5d5c8950b4650b8753f9119dc7f1ccaa2aa5442) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "de_142_d3.bin", 0x0000, 0x1000, CRC(77be0b40) SHA1(113b5c239480a2259f55e411ba8fb3972e6d4301) ) // German language
ROM_END

ROM_START( quizard4cz ) /* CD-ROM printed 09/98 */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r42", 0, BAD_DUMP SHA1(a5d5c8950b4650b8753f9119dc7f1ccaa2aa5442) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "ts142_cz1.bin", 0x0000, 0x1000, CRC(fdc1f457) SHA1(5169c4d2ea4073a854c3f619205161386c9af8af) ) // Czech language - works with all Quizard 4 versions
ROM_END

ROM_START( quizard4_41 )
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r41", 0, BAD_DUMP SHA1(2c0484c6545aac8e00b318328c6edce6f5dde43d) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "de_142_d3.bin", 0x0000, 0x1000, CRC(77be0b40) SHA1(113b5c239480a2259f55e411ba8fb3972e6d4301) ) // German language
ROM_END

ROM_START( quizard4_40 ) /* CD-ROM printed 07/97 */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r40", 0, BAD_DUMP SHA1(288cc37a994e4f1cbd47aa8c92342879c6fc0b87) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "de_142_d3.bin", 0x0000, 0x1000, CRC(77be0b40) SHA1(113b5c239480a2259f55e411ba8fb3972e6d4301) ) // German language
ROM_END

// only the CD was dumped, MCU not available
ROM_START( quizardff ) /* CD-ROM printed 01/96 */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizardff", 0, SHA1(ac533040379c1350066e778e3a86d1beb11c6f71) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "8751.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END


/*************************
*      Game driver(s)    *
*************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS      INIT        COMPANY       FULLNAME */
// BIOS / System
CONS( 1991, cdimono1, 0,      0,      cdimono1, cdi,      cdi_state, empty_init, "Philips",    "CD-i (Mono-I) (PAL)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 1991, cdimono2, 0,      0,      cdimono2, cdimono2, cdi_state, empty_init, "Philips",    "CD-i (Mono-II) (NTSC)",   MACHINE_NOT_WORKING )
CONS( 1991, cdi910,   0,      0,      cdi910,   cdimono2, cdi_state, empty_init, "Philips",    "CD-i 910-17P Mini-MMC (PAL)",   MACHINE_NOT_WORKING )
CONS( 1991, cdi490a,  0,      0,      cdimono1, cdi,      cdi_state, empty_init, "Philips",    "CD-i 490",   MACHINE_NOT_WORKING )

// The Quizard games are retail CD-i units in a cabinet, with an additional JAMMA adapter and dongle for protection, hence being clones of the system.
/*    YEAR  NAME         PARENT    MACHINE        INPUT     DEVICE          INIT         MONITOR     COMPANY         FULLNAME */
GAME( 1995, cdibios,     0,        cdimono1,      quizard,  cdi_state,     empty_init,  ROT0,     "Philips",  "CD-i (Mono-I) (PAL) BIOS", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IS_BIOS_ROOT )

GAME( 1995, quizard,     cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.8, German, i8751 DE 11 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard_17,  quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.7, German, i8751 DE 11 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard_12,  quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.2, German, i8751 DE 11 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard_10,  quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.0, German, i8751 DE 11 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizardi,    quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.8, Italian, i8751 IT 11 I2)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizardi_17, quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.7, Italian, i8751 IT 11 I2)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizardi_12, quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.2, Italian, i8751 IT 11 I2)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizardi_10, quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.0, Italian, i8751 IT 11 I2)", MACHINE_IMPERFECT_SOUND )

GAME( 1995, quizard2,    cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 2 (v2.3, German, i8751 DN 122 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard2_22, quizard2, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 2 (v2.2, German, i8751 DN 122 D3)", MACHINE_IMPERFECT_SOUND )

// Quizard 3 and 4 will hang after starting a game (CDIC issues?)
GAME( 1995, quizard3,    cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 3 (v3.4, German, i8751 DE 132 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard3a,   quizard3, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 3 (v3.4, German, i8751 DE 132 A1)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, quizard3_32, quizard3, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 3 (v3.2, German, i8751 DE 132 D3)", MACHINE_IMPERFECT_SOUND )

GAME( 1998, quizard4,    cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 4 Rainbow (v4.2, German, i8751 DE 142 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, quizard4cz,  quizard4, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 4 Rainbow (v4.2, Czech, i8751 TS142 CZ1)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, quizard4_41, quizard4, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 4 Rainbow (v4.1, German, i8751 DE 142 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1997, quizard4_40, quizard4, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 4 Rainbow (v4.0, German, i8751 DE 142 D3)", MACHINE_IMPERFECT_SOUND )

GAME( 1996, quizardff,   cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard Fun and Fascination (French Edition V1 - 01/96)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
