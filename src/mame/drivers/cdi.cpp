// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    Philips CD-I-based games
    ------------------------

    Preliminary MAME driver by Ryan Holtz
    Help provided by CD-i Fan


*******************************************************************************

STATUS:

Quizard does not work for unknown reasons.

TODO:

- Proper abstraction of the 68070's internal devices (UART,DMA,Timers etc.)
- Mono-I: Full emulation of the CDIC, SLAVE and/or MCD212 customs
- Mono-II: SERVO and SLAVE I/O device hookup
- Mono-II: DSP56k hookup
- Mono-II: Move 68HC05 I/O device hookup into CPU core

*******************************************************************************/

#include "emu.h"
#include "includes/cdi.h"

#include "cpu/m6805/m6805.h"
#include "imagedev/chd_cd.h"
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

#define LOG_SERVO       (1 << 0)
#define LOG_SLAVE       (1 << 1)
#define LOG_ALL         (LOG_SERVO | LOG_SLAVE)

#define VERBOSE         (LOG_ALL)
#include "logmacro.h"

#define ENABLE_UART_PRINTING (0)

/*************************
*      Memory maps       *
*************************/

void cdi_state::cdimono1_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().share("mcd212:planea");
	map(0x00200000, 0x0027ffff).ram().share("mcd212:planeb");
	map(0x00300000, 0x00303bff).rw(m_cdic, FUNC(cdicdic_device::ram_r), FUNC(cdicdic_device::ram_w));
#if ENABLE_UART_PRINTING
	map(0x00301400, 0x00301403).r(m_maincpu, FUNC(scc68070_device::uart_loopback_enable));
#endif
	map(0x00303c00, 0x00303fff).rw(m_cdic, FUNC(cdicdic_device::regs_r), FUNC(cdicdic_device::regs_w));
	map(0x00310000, 0x00317fff).rw(m_slave_hle, FUNC(cdislave_device::slave_r), FUNC(cdislave_device::slave_w));
	map(0x00318000, 0x0031ffff).noprw();
	map(0x00320000, 0x00323fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);    /* nvram (only low bytes used) */
	map(0x00400000, 0x0047ffff).rom().region("maincpu", 0);
	map(0x004fffe0, 0x004fffff).rw(m_mcd212, FUNC(mcd212_device::regs_r), FUNC(mcd212_device::regs_w));
	map(0x00500000, 0x0057ffff).ram();
	map(0x00580000, 0x00ffffff).noprw();
	map(0x00e00000, 0x00efffff).ram(); // DVC
}

void cdi_state::cdimono2_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().share("mcd212:planea");
	map(0x00200000, 0x0027ffff).ram().share("mcd212:planeb");
#if ENABLE_UART_PRINTING
	map(0x00301400, 0x00301403).r(m_maincpu, FUNC(scc68070_device::uart_loopback_enable));
#endif
	//map(0x00300000, 0x00303bff).rw("cdic", FUNC(cdicdic_device::ram_r), FUNC(cdicdic_device::ram_w));
	//map(0x00303c00, 0x00303fff).rw("cdic", FUNC(cdicdic_device::regs_r), FUNC(cdicdic_device::regs_w));
	//map(0x00310000, 0x00317fff).rw("slave", FUNC(cdislave_device::slave_r), FUNC(cdicdic_device::slave_w));
	//map(0x00318000, 0x0031ffff).noprw();
	map(0x00320000, 0x00323fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);    /* nvram (only low bytes used) */
	map(0x00400000, 0x0047ffff).rom().region("maincpu", 0);
	map(0x004fffe0, 0x004fffff).rw(m_mcd212, FUNC(mcd212_device::regs_r), FUNC(mcd212_device::regs_w));
	//map(0x00500000, 0x0057ffff).ram();
	map(0x00500000, 0x00ffffff).noprw();
	//map(0x00e00000, 0x00efffff).ram(); // DVC
}

void cdi_state::cdi910_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().share("mcd212:planea");
	map(0x00180000, 0x001fffff).rom().region("maincpu", 0); // boot vectors point here

	map(0x00200000, 0x0027ffff).ram().share("mcd212:planeb");
#if ENABLE_UART_PRINTING
	map(0x00301400, 0x00301403).r(m_maincpu, FUNC(scc68070_device::uart_loopback_enable));
#endif
//  map(0x00300000, 0x00303bff).rw("cdic", FUNC(cdicdic_device::ram_r), FUNC(cdicdic_device::ram_w));
//  map(0x00303c00, 0x00303fff).rw("cdic", FUNC(cdicdic_device::regs_r), FUNC(cdicdic_device::regs_w));
//  map(0x00310000, 0x00317fff).rw("slave_hle", FUNC(cdislave_device::slave_r), FUNC(cdicdic_device::slave_w));
//  map(0x00318000, 0x0031ffff).noprw();
	map(0x00320000, 0x00323fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);    /* nvram (only low bytes used) */
	map(0x004fffe0, 0x004fffff).rw(m_mcd212, FUNC(mcd212_device::regs_r), FUNC(mcd212_device::regs_w));
//  map(0x00500000, 0x0057ffff).ram();
	map(0x00500000, 0x00ffffff).noprw();
//  map(0x00e00000, 0x00efffff).ram(); // DVC
}


/*************************
*      Input ports       *
*************************/

INPUT_CHANGED_MEMBER(cdi_state::mcu_input)
{
	bool send = false;

	switch (param)
	{
		case 0x39:
			if (m_input1.read_safe(0) & 0x01) send = true;
			break;
		case 0x37:
			if (m_input1.read_safe(0) & 0x02) send = true;
			break;
		case 0x31:
			if (m_input1.read_safe(0) & 0x04) send = true;
			break;
		case 0x32:
			if (m_input1.read_safe(0) & 0x08) send = true;
			break;
		case 0x33:
			if (m_input1.read_safe(0) & 0x10) send = true;
			break;

		case 0x30:
			if (m_input2.read_safe(0) & 0x01) send = true;
			break;
		case 0x38:
			if (m_input2.read_safe(0) & 0x02) send = true;
			break;
		case 0x34:
			if (m_input2.read_safe(0) & 0x04) send = true;
			break;
		case 0x35:
			if (m_input2.read_safe(0) & 0x08) send = true;
			break;
		case 0x36:
			if (m_input2.read_safe(0) & 0x10) send = true;
			break;
	}

	if(send)
	{
		uint8_t data = uint8_t(param & 0x000000ff);
		quizard_rx(data);
	}
}

static INPUT_PORTS_START( cdi )
	PORT_START("DEBUG")
	PORT_CONFNAME( 0x01, 0x00, "Plane A Disable")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x00, "Plane B Disable")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_CONFNAME( 0x04, 0x00, "Force Backdrop Color")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_CONFNAME( 0xf0, 0x00, "Backdrop Color")
	PORT_CONFSETTING(    0x00, "Black" )
	PORT_CONFSETTING(    0x10, "Half-Bright Blue" )
	PORT_CONFSETTING(    0x20, "Half-Bright Green" )
	PORT_CONFSETTING(    0x30, "Half-Bright Cyan" )
	PORT_CONFSETTING(    0x40, "Half-Bright Red" )
	PORT_CONFSETTING(    0x50, "Half-Bright Magenta" )
	PORT_CONFSETTING(    0x60, "Half-Bright Yellow" )
	PORT_CONFSETTING(    0x70, "Half-Bright White" )
	PORT_CONFSETTING(    0x80, "Black (Alternate)" )
	PORT_CONFSETTING(    0x90, "Blue" )
	PORT_CONFSETTING(    0xa0, "Green" )
	PORT_CONFSETTING(    0xb0, "Cyan" )
	PORT_CONFSETTING(    0xc0, "Red" )
	PORT_CONFSETTING(    0xd0, "Magenta" )
	PORT_CONFSETTING(    0xe0, "Yellow" )
	PORT_CONFSETTING(    0xf0, "White" )
INPUT_PORTS_END

static INPUT_PORTS_START( cdimono2 )
	PORT_START("DEBUG")
	PORT_CONFNAME( 0x01, 0x00, "Plane A Disable")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x00, "Plane B Disable")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_CONFNAME( 0x04, 0x00, "Force Backdrop Color")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_CONFNAME( 0xf0, 0x00, "Backdrop Color")
	PORT_CONFSETTING(    0x00, "Black" )
	PORT_CONFSETTING(    0x10, "Half-Bright Blue" )
	PORT_CONFSETTING(    0x20, "Half-Bright Green" )
	PORT_CONFSETTING(    0x30, "Half-Bright Cyan" )
	PORT_CONFSETTING(    0x40, "Half-Bright Red" )
	PORT_CONFSETTING(    0x50, "Half-Bright Magenta" )
	PORT_CONFSETTING(    0x60, "Half-Bright Yellow" )
	PORT_CONFSETTING(    0x70, "Half-Bright White" )
	PORT_CONFSETTING(    0x80, "Black (Alternate)" )
	PORT_CONFSETTING(    0x90, "Blue" )
	PORT_CONFSETTING(    0xa0, "Green" )
	PORT_CONFSETTING(    0xb0, "Cyan" )
	PORT_CONFSETTING(    0xc0, "Red" )
	PORT_CONFSETTING(    0xd0, "Magenta" )
	PORT_CONFSETTING(    0xe0, "Yellow" )
	PORT_CONFSETTING(    0xf0, "White" )
INPUT_PORTS_END


static INPUT_PORTS_START( quizard )
	PORT_INCLUDE( cdi )

	PORT_START("INPUT1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Coin 1") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x39)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start 1") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x37)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Player 1 A") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x31)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Player 1 B") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x32)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Player 1 C") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x33)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("INPUT2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x30)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Start 2") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x38)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Player 2 A") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x34)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Player 2 B") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x35)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Player 2 C") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, 0x36)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


/***************************
*  Machine Initialization  *
***************************/

MACHINE_RESET_MEMBER( cdi_state, cdimono1 )
{
	uint16_t *src   = (uint16_t*)memregion("maincpu")->base();
	uint16_t *dst   = m_planea;
	memcpy(dst, src, 0x8);

	// Quizard Protection HLE data
	memset(m_seeds, 0, 10 * sizeof(uint16_t));
	memset(m_state, 0, 8 * sizeof(uint8_t));
	m_mcu_value = 0;
	m_mcu_ack = 0;
}

MACHINE_RESET_MEMBER( cdi_state, cdimono2 )
{
	uint16_t *src   = (uint16_t*)memregion("maincpu")->base();
	uint16_t *dst   = m_planea;
	memcpy(dst, src, 0x8);

	m_maincpu->reset();
}

MACHINE_RESET_MEMBER( cdi_state, quizard1 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	set_quizard_mcu_value(0x021f);
	set_quizard_mcu_ack(0x5a);
}

MACHINE_RESET_MEMBER( cdi_state, quizard2 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	// 0x2b1: Italian
	// 0x001: French
	// 0x188: German

	set_quizard_mcu_value(0x188);
	set_quizard_mcu_ack(0x59);
}

MACHINE_RESET_MEMBER( cdi_state, quizard3 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	set_quizard_mcu_value(0x00ae);
	set_quizard_mcu_ack(0x58);
}

MACHINE_RESET_MEMBER( cdi_state, quizard4 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	set_quizard_mcu_value(0x011f);
	set_quizard_mcu_ack(0x57);
}


/***************************
*  Quizard Protection HLE  *
***************************/

void cdi_state::set_quizard_mcu_ack(uint8_t ack)
{
	m_mcu_ack = ack;
}

void cdi_state::set_quizard_mcu_value(uint16_t value)
{
	m_mcu_value = value;
}

void cdi_state::quizard_rx(uint8_t data)
{
	m_maincpu->uart_rx(0x5a);
	m_maincpu->uart_rx(data);
}

void cdi_state::quizard_set_seeds(uint8_t *rx)
{
	m_seeds[0] = (rx[1] << 8) | rx[0];
	m_seeds[1] = (rx[3] << 8) | rx[2];
	m_seeds[2] = (rx[5] << 8) | rx[4];
	m_seeds[3] = (rx[7] << 8) | rx[6];
	m_seeds[4] = (rx[9] << 8) | rx[8];
	m_seeds[5] = (rx[11] << 8) | rx[10];
	m_seeds[6] = (rx[13] << 8) | rx[12];
	m_seeds[7] = (rx[15] << 8) | rx[14];
	m_seeds[8] = (rx[17] << 8) | rx[16];
	m_seeds[9] = (rx[19] << 8) | rx[18];
}

void cdi_state::quizard_calculate_state()
{
	//const uint16_t desired_bitfield = mcu_value;
	const uint16_t field0 = 0x00ff;
	const uint16_t field1 = m_mcu_value ^ 0x00ff;

	uint16_t total0 = 0;
	uint16_t total1 = 0;

	for(int index = 0; index < 10; index++)
	{
		if (field0 & (1 << index))
		{
			total0 += m_seeds[index];
		}
		if (field1 & (1 << index))
		{
			total1 += m_seeds[index];
		}
	}

	uint16_t hi0 = (total0 >> 8) + 0x40;
	m_state[2] = hi0 / 2;
	m_state[3] = hi0 - m_state[2];

	uint16_t lo0 = (total0 & 0x00ff) + 0x40;
	m_state[0] = lo0 / 2;
	m_state[1] = lo0 - m_state[0];

	uint16_t hi1 = (total1 >> 8) + 0x40;
	m_state[6] = hi1 / 2;
	m_state[7] = hi1 - m_state[6];

	uint16_t lo1 = (total1 & 0x00ff) + 0x40;
	m_state[4] = lo1 / 2;
	m_state[5] = lo1 - m_state[4];
}

void cdi_state::quizard_handle_byte_tx(uint8_t data)
{
	static int state = 0;
	static uint8_t rx[0x100];
	static uint8_t rx_ptr = 0xff;

	switch (state)
	{
		case 0: // Waiting for a leadoff byte
			if (data == m_mcu_ack) // Sequence end
			{
				//scc68070_uart_rx(machine, scc68070, 0x5a);
				//scc68070_uart_rx(machine, scc68070, 0x42);
			}
			else
			{
				switch (data)
				{
					case 0x44: // DATABASEPATH = **_DATABASE/
						rx[0] = 0x44;
						rx_ptr = 1;
						state = 3;
						break;
					case 0x2e: // Unknown; ignored
						break;
					case 0x56: // Seed start
						rx_ptr = 0;
						state = 1;
						break;
					default:
						//printf("Unknown leadoff byte: %02x\n", data);
						break;
				}
			}
			break;

		case 1: // Receiving the seed
			rx[rx_ptr] = data;
			rx_ptr++;
			if (rx_ptr == 20)
			{
				//printf("Calculating seeds\n");
				quizard_set_seeds(rx);
				quizard_calculate_state();
				state = 2;
			}
			break;

		case 2: // Receiving the seed acknowledge
		case 4:
			if (data == m_mcu_ack)
			{
				if (state == 2)
				{
					state = 4;
				}
				else
				{
					state = 0;
				}
				//printf("Sending seed ack\n");
				m_maincpu->uart_rx(0x5a);
				m_maincpu->uart_rx(m_state[0]);
				m_maincpu->uart_rx(m_state[1]);
				m_maincpu->uart_rx(m_state[2]);
				m_maincpu->uart_rx(m_state[3]);
				m_maincpu->uart_rx(m_state[4]);
				m_maincpu->uart_rx(m_state[5]);
				m_maincpu->uart_rx(m_state[6]);
				m_maincpu->uart_rx(m_state[7]);
			}
			break;

		case 3: // Receiving the database path
			rx[rx_ptr] = data;
			rx_ptr++;
			if (data == 0x0a)
			{
				/*rx[rx_ptr] = 0;
				//printf("Database path: %s\n", rx);
				scc68070_uart_rx(machine, scc68070, 0x5a);
				scc68070_uart_rx(machine, scc68070, g_state[0]);
				scc68070_uart_rx(machine, scc68070, g_state[1]);
				scc68070_uart_rx(machine, scc68070, g_state[2]);
				scc68070_uart_rx(machine, scc68070, g_state[3]);
				scc68070_uart_rx(machine, scc68070, g_state[4]);
				scc68070_uart_rx(machine, scc68070, g_state[5]);
				scc68070_uart_rx(machine, scc68070, g_state[6]);
				scc68070_uart_rx(machine, scc68070, g_state[7]);*/
				state = 0;
			}
			break;
	}
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

void cdi_state::draw_lcd(int y)
{
	if (y >= 22 || !m_slave_hle.found())
		return;

	uint32_t *scanline = &m_lcdbitmap.pix32(y);

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

void cdi_state::video_start()
{
	if (m_lcd)
		m_lcd->register_screen_bitmap(m_lcdbitmap);
}

uint32_t cdi_state::screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_lcdbitmap, 0, 0, 0, 0, cliprect);
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

	MCD212(config, m_mcd212, CLOCK_A);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));
	m_mcd212->set_scanline_callback(FUNC(cdi_state::draw_lcd));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(384, 302);
	screen.set_visarea(0, 384-1, 22, 302-1); // TODO: dynamic resolution
	screen.set_screen_update("mcd212", FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd, SCREEN_TYPE_RASTER);
	m_lcd->set_refresh_hz(60);
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

	CDI_SLAVE(config, m_slave_hle, 0);
	m_slave_hle->int_callback().set(m_maincpu, FUNC(scc68070_device::in2_w));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMADAC(config, m_dmadac[0]);
	m_dmadac[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	DMADAC(config, m_dmadac[1]);
	m_dmadac[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	CDDA(config, m_cdda);
	m_cdda->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_cdda->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	MK48T08(config, "mk48t08");
}

// CD-i model 220 (Mono-II, NTSC)
void cdi_state::cdimono2(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdimono2_mem);

	MCD212(config, m_mcd212, CLOCK_A);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));
	m_mcd212->set_scanline_callback(FUNC(cdi_state::draw_lcd));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(384, 302);
	screen.set_visarea(0, 384-1, 22, 302-1); // TODO: dynamic resolution
	screen.set_screen_update("mcd212", FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd, SCREEN_TYPE_RASTER);
	m_lcd->set_refresh_hz(60);
	m_lcd->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_lcd->set_size(192, 22);
	m_lcd->set_visarea(0, 192-1, 0, 22-1);
	m_lcd->set_screen_update(FUNC(cdi_state::screen_update_cdimono1_lcd));

	PALETTE(config, "palette").set_entries(0x100);

	config.set_default_layout(layout_cdi);

	MCFG_MACHINE_RESET_OVERRIDE( cdi_state, cdimono2 )

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

	CDDA(config, m_cdda);
	m_cdda->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_cdda->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	MK48T08(config, "mk48t08");
}

void cdi_state::cdi910(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdi910_mem);

	MCD212(config, m_mcd212, CLOCK_A);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));
	m_mcd212->set_scanline_callback(FUNC(cdi_state::draw_lcd));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(384, 302);
	screen.set_visarea(0, 384-1, 22, 302-1); // TODO: dynamic resolution
	screen.set_screen_update("mcd212", FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd, SCREEN_TYPE_RASTER);
	m_lcd->set_refresh_hz(60);
	m_lcd->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_lcd->set_size(192, 22);
	m_lcd->set_visarea(0, 192-1, 0, 22-1);
	m_lcd->set_screen_update(FUNC(cdi_state::screen_update_cdimono1_lcd));

	PALETTE(config, "palette").set_entries(0x100);

	config.set_default_layout(layout_cdi);

	MCFG_MACHINE_RESET_OVERRIDE( cdi_state, cdimono2 )

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

	CDDA(config, m_cdda);
	m_cdda->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_cdda->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	MK48T08(config, "mk48t08");
}

// CD-i Mono-I, with CD-ROM image device (MESS) and Software List (MESS)
void cdi_state::cdimono1(machine_config &config)
{
	cdimono1_base(config);
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, cdimono1)

	CDROM(config, "cdrom").set_interface("cdi_cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("cdi").set_filter("!DVC");
}

void cdi_state::quizard(machine_config &config)
{
	cdimono1_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdimono1_mem);
	m_maincpu->uart_tx_callback().set(FUNC(cdi_state::quizard_handle_byte_tx));
}


READ8_MEMBER( cdi_state::quizard_mcu_p1_r )
{
	LOG("%s: MCU Port 1 Read\n", machine().describe_context());
	return 0;
}

void cdi_state::quizard1(machine_config &config)
{
	quizard(config);
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard1)

	i8751_device &mcu(I8751(config, "mcu", 8000000));
	mcu.port_in_cb<1>().set(FUNC(cdi_state::quizard_mcu_p1_r));
//  mcu.set_vblank_int("screen", FUNC(cdi_state::irq0_line_pulse));
}

void cdi_state::quizard2(machine_config &config)
{
	quizard(config);
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard2)
}

void cdi_state::quizard3(machine_config &config)
{
	quizard(config);
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard3)
}

void cdi_state::quizard4(machine_config &config)
{
	quizard(config);
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard4)

	i8751_device &mcu(I8751(config, "mcu", 8000000));
	mcu.port_in_cb<1>().set(FUNC(cdi_state::quizard_mcu_p1_r));
//  mcu.set_vblank_int("screen", FUNC(cdi_state::irq0_line_pulse));
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

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping
ROM_END



ROM_START( cdi910 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "cdi910", "CD-I 910-17P Mini-MMC" )
	ROMX_LOAD( "philips__cd-i_2.1__mb834200b-15__26b_aa__9224_z01.tc574200.7211", 0x000000, 0x80000, CRC(4ae3bee3) SHA1(9729b4ee3ce0c17172d062339c47b1ab822b222b), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE )
	ROM_SYSTEM_BIOS( 1, "cdi910_alt", "alt" )
	ROMX_LOAD( "cdi910.rom", 0x000000, 0x80000, CRC(2f3048d2) SHA1(11c4c3e602060518b52e77156345fa01f619e793), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE )

	// cdic

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

	ROM_REGION(0x40000, "mpegs", 0) // keep these somewhere
	ROM_LOAD( "impega.rom", 0x0000, 0x40000, CRC(84d6f6aa) SHA1(02526482a0851ea2a7b582d8afaa8ef14a8bd914) )
	ROM_LOAD( "vmpega.rom", 0x0000, 0x40000, CRC(db264e8b) SHA1(be407fbc102f1731a0862554855e963e5a47c17b) )
ROM_END




ROM_START( cdibios ) // for the quizard sets
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "mcdi200", "Magnavox CD-i 200" )
	ROMX_LOAD( "cdi200.rom", 0x000000, 0x80000, CRC(40c4e6b9) SHA1(d961de803c89b3d1902d656ceb9ce7c02dccb40a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pcdi220", "Philips CD-i 220 F2" )
	ROMX_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e), ROM_BIOS(1) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping
ROM_END

/*  Quizard notes

    The MCU controls the protection sequence, which in turn controls the game display language.
    Each Quizard game (1,2,3,4) requires it's own MCU, you can upgrade between revisions by changing
    just the CD, but not between games as a new MCU is required.

    The only dumped MCUs are German region ones for Quizard 1 and 4.
    A Czech Quizard 4 MCU was located but it was an 89c51 type instead

*/


// Quizard (1)

ROM_START( quizard ) /* CD-ROM printed ??/?? */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard18", 0, BAD_DUMP SHA1(ede873b22957f2a707bbd3039e962ef2ca5aedbd) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard1_german_d8751.bin", 0x0000, 0x1000, CRC(95f45b6b) SHA1(51b34956539b1e2cf0306f243a970750f1e18d01) ) // confirmed good on original hardware
ROM_END


ROM_START( quizard_17 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard17", 0, BAD_DUMP SHA1(4bd698f076505b4e17be978481bce027eb47123b) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard1_german_d8751.bin", 0x0000, 0x1000, CRC(95f45b6b) SHA1(51b34956539b1e2cf0306f243a970750f1e18d01) ) // confirmed good on original hardware
ROM_END

ROM_START( quizard_12 ) /* CD-ROM printed 01/95 */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard12", 0, BAD_DUMP SHA1(6e41683b96b74e903040842aeb18437ad7813c82) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard1_german_d8751.bin", 0x0000, 0x1000, CRC(95f45b6b) SHA1(51b34956539b1e2cf0306f243a970750f1e18d01) ) // confirmed good on original hardware
ROM_END

ROM_START( quizard_10 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping


	// software: BurnAtOnce 0.99.5 / CHDMAN 0.163
	// Drive: TS-L633R
	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard10", 0, SHA1(5715db50f0d5ffe06f47c0943f4bf0481ab6048e) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard1_german_d8751.bin", 0x0000, 0x1000, CRC(95f45b6b) SHA1(51b34956539b1e2cf0306f243a970750f1e18d01) ) // confirmed good on original hardware
ROM_END


// Quizard 2

ROM_START( quizard2 ) /* CD-ROM printed ??/?? */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard23", 0, BAD_DUMP SHA1(cd909d9a54275d6f2d36e03e83eea996e781b4d3) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard2_d8751.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END

ROM_START( quizard2_22 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard22", 0, BAD_DUMP SHA1(03c8fdcf27ead6e221691111e8c679b551099543) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard2_d8751.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END

// Quizard 3

ROM_START( quizard3 ) /* CD-ROM printed ??/?? */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard34", 0, BAD_DUMP SHA1(37ad49b72b5175afbb87141d57bc8604347fe032) )

	ROM_REGION(0x1000, "mcu", 0) // d8751h
	ROM_LOAD( "de132d3.bin", 0x0000, 0x1000, CRC(8858251e) SHA1(2c1005a74bb6f0c2918dff4ab6326528eea48e1f) ) // confirmed good on original hardware
ROM_END

ROM_START( quizard3_32 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard32", 0, BAD_DUMP SHA1(31e9fa2169aa44d799c37170b238134ab738e1a1) )

	ROM_REGION(0x1000, "mcu", 0) // d8751h
	ROM_LOAD( "de132d3.bin", 0x0000, 0x1000, CRC(8858251e) SHA1(2c1005a74bb6f0c2918dff4ab6326528eea48e1f) ) // confirmed good on original hardware
ROM_END


ROM_START( quizard4 ) /* CD-ROM printed 09/98 */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r42", 0, BAD_DUMP SHA1(a5d5c8950b4650b8753f9119dc7f1ccaa2aa5442) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard4_german_d8751.bin", 0x0000, 0x1000, CRC(77be0b40) SHA1(113b5c239480a2259f55e411ba8fb3972e6d4301) ) // confirmed good on original hardware
ROM_END

ROM_START( quizard4_41 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r41", 0, BAD_DUMP SHA1(2c0484c6545aac8e00b318328c6edce6f5dde43d) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard4_german_d8751.bin", 0x0000, 0x1000, CRC(77be0b40) SHA1(113b5c239480a2259f55e411ba8fb3972e6d4301) ) // confirmed good on original hardware
ROM_END

ROM_START( quizard4_40 ) /* CD-ROM printed 07/97 */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r40", 0, BAD_DUMP SHA1(288cc37a994e4f1cbd47aa8c92342879c6fc0b87) )

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD( "quizard4_german_d8751.bin", 0x0000, 0x1000, CRC(77be0b40) SHA1(113b5c239480a2259f55e411ba8fb3972e6d4301) ) // confirmed good on original hardware
ROM_END


/*************************
*      Game driver(s)    *
*************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS      INIT        COMPANY         FULLNAME */
// BIOS / System
CONS( 1991, cdimono1, 0,      0,      cdimono1, cdi,      cdi_state, empty_init, "Philips",      "CD-i (Mono-I) (PAL)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE  )
CONS( 1991, cdimono2, 0,      0,      cdimono2, cdimono2, cdi_state, empty_init, "Philips",      "CD-i (Mono-II) (NTSC)",   MACHINE_NOT_WORKING )
CONS( 1991, cdi910,   0,      0,      cdi910,   cdimono2, cdi_state, empty_init, "Philips",      "CD-i 910-17P Mini-MMC (PAL)",   MACHINE_NOT_WORKING  )
CONS( 1991, cdi490a,  0,      0,      cdimono1, cdi,      cdi_state, empty_init, "Philips",      "CD-i 490",   MACHINE_NOT_WORKING  )

// The Quizard games are RETAIL CD-i units, with additional JAMMA adapters & dongles for protection, hence being 'clones' of the system.
/*    YEAR  NAME         PARENT    MACHINE        INPUT     DEVICE     INIT         MONITOR     COMPANY         FULLNAME */
GAME( 1995, cdibios,     0,        cdimono1_base, quizard,  cdi_state, empty_init,  ROT0,       "Philips",      "CD-i (Mono-I) (PAL) BIOS", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IS_BIOS_ROOT )

GAME( 1995, quizard,     cdibios,  quizard1,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard (v1.8)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, quizard_17,  quizard,  quizard1,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard (v1.7)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, quizard_12,  quizard,  quizard1,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard (v1.2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, quizard_10,  quizard,  quizard1,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard (v1.0)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )

GAME( 1995, quizard2,    cdibios,  quizard2,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard 2 (v2.3)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, quizard2_22, quizard2, quizard2,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard 2 (v2.2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )

// Quizard 3 and 4 will hang after inserting a coin (incomplete protection sims?)

GAME( 1995, quizard3,    cdibios,  quizard3,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard 3 (v3.4)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1996, quizard3_32, quizard3, quizard3,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard 3 (v3.2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )

GAME( 1998, quizard4,    cdibios,  quizard4,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard 4 Rainbow (v4.2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1998, quizard4_41, quizard4, quizard4,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard 4 Rainbow (v4.1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1997, quizard4_40, quizard4, quizard4,      quizard,  cdi_state, empty_init,  ROT0,       "TAB Austria",  "Quizard 4 Rainbow (v4.0)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
