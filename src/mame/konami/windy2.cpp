// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
 * Konami Windy2 I/O (JVS)
 *
 * Main PCB Layout
 * ---------------
 * PWB405270B
 * (C)1997 KONAMI CO.,LTD.
 * |------------------------------------------|
 * |    CN1         CN2                 CN3   |
 * |                                          |
 * |                                          |
 * |                                          |
 * |  CN6    CN5             JP3 JP4   CN4    |
 * |                                          |
 * |               DIPSW8P                    |
 * |                                          |
 * |                                          |
 * |                         SP485        B   |
 * |                                          |
 * |                                          |
 * |7.378MHZ H8                           A   |
 * |                                          |
 * |------------------------------------------|
 *
 * CN1 - 12-pin connector (for Player 1)
 * CN2 - 12-pin connector (for Player 2)
 * CN3 - 8-pin connector
 * CN4 - 8-pin connector
 * CN5 - 4-pin connector
 * CN6 - 6-pin Power Connector
 * H8 - Hitachi H8/3644 microcontroller labeled '707 V2 31484'
 * SP485 - 9619 LTC485 CN8 Low Power RS485 Interface Transceiver
 * A - USB-A connector
 * B - USB-B connector
 */

#include "emu.h"
#include "windy2.h"

DEFINE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_2L6B_PANEL,  windy2_2l6b_device,  "windy2_2l6b", "Konami Windy2 I/O (2L6B Panel, Standard)")
DEFINE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_QUIZ_PANEL,  windy2_quiz_device,  "windy2_quiz", "Konami Windy2 I/O (Quiz Panel, GU707-JB)")
DEFINE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_2L8B_PANEL,  windy2_2l8b_device,  "windy2_2l8b", "Konami Windy2 I/O (2L8B Panel, GU707-JC)")
DEFINE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_1L6B_PANEL,  windy2_1l6b_device,  "windy2_1l6b", "Konami Windy2 I/O (1L6B Panel, GU707-JD)")
DEFINE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_2L12B_PANEL, windy2_2l12b_device, "windy2_2l12b", "Konami Windy2 I/O (2L12B Panel, GU707-JG)")

windy2_device::windy2_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t player_count, uint8_t switch_count) :
	jvs_device(mconfig, type, tag, owner, clock),
	m_test_port(*this, "TEST"),
	m_player_ports(*this, "P%u", 1U),
	m_player_count(player_count),
	m_switch_count(switch_count)
{
}

void windy2_device::device_start()
{
	jvs_device::device_start();
	save_item(NAME(m_coin_counter));
}

void windy2_device::device_reset()
{
	jvs_device::device_reset();
	std::fill(std::begin(m_coin_counter), std::end(m_coin_counter), 0);
}

const char *windy2_device::device_id()
{
	// Known device IDs:
	// KONAMI CO.,LTD.;Windy2;Ver1.0;Windy2 I/O Ver1.0 (referenced by all games I've seen using Windy 2 I/O)
	// KONAMI CO.,LTD.;Windy2;Ver1.0;Windy2 I/O Ver1.1 (referenced by System 573 games)
	// The difference between the two is currently unknown.
	return "KONAMI CO.,LTD.;Windy2;Ver1.0;Windy2 I/O Ver1.0";
}

uint8_t windy2_device::command_format_version()
{
	return 0x11;
}

uint8_t windy2_device::jvs_standard_version()
{
	return 0x20;
}

uint8_t windy2_device::comm_method_version()
{
	return 0x10;
}

void windy2_device::function_list(uint8_t*& buf)
{
	// Switch input function
	*buf++ = 0x01;
	*buf++ = m_player_count;
	*buf++ = m_switch_count; // this count includes any levers positions and additional buttons (start, service) per player
	*buf++ = 0;

	// Coin input function
	*buf++ = 0x02;
	*buf++ = std::size(m_coin_counter);
	*buf++ = 0;
	*buf++ = 0;
}

bool windy2_device::switches(uint8_t*& buf, uint8_t count_players, uint8_t bytes_per_switch)
{
	if (count_players > 2 || bytes_per_switch > 2)
		return false;

	*buf++ = m_test_port->read();

	for (int i = 0; i < count_players; i++) {
		const uint32_t pval = m_player_ports[i]->read();
		for (int j = 0; j < bytes_per_switch; j++)
			*buf++ = BIT(pval, ((bytes_per_switch - 1) - j) * 8, 8);
	}

	return true;
}

bool windy2_device::coin_counters(uint8_t*& buf, uint8_t count)
{
	if (count > std::size(m_coin_counter))
		return false;

	for (int i = 0; i < count; i++) {
		*buf++ = (m_coin_counter[i] >> 8) & 0x3f; // top 2 bits are for coin condition, 0 = normal
		*buf++ = m_coin_counter[i];
	}

	return true;
}

INPUT_CHANGED_MEMBER(windy2_device::coin_inserted)
{
	if (newval && param < std::size(m_coin_counter))
		m_coin_counter[param] = (m_coin_counter[param] + 1) & 0x3fff;
}

static INPUT_PORTS_START( windy2 )
	PORT_START( "TEST" )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_HIGH )
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "P1" )
	PORT_BIT( 0xffff3fff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START( "P2" )
	PORT_BIT( 0xffff3fff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_SERVICE2 )

	PORT_START( "COIN" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(windy2_device::coin_inserted), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(windy2_device::coin_inserted), 1)
INPUT_PORTS_END

static INPUT_PORTS_START( windy2_2l6b )
	PORT_INCLUDE( windy2 )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "P2" )
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( windy2_2l8b )
	PORT_INCLUDE( windy2_2l6b )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "P2" )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( windy2_2l12b )
	PORT_INCLUDE( windy2_2l8b )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "P2" )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( windy2_1l6b )
	PORT_INCLUDE( windy2_2l12b )

	// Same button layout as 2L12B but only 1 player
	PORT_MODIFY( "P2" )
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( windy2_quiz )
	PORT_INCLUDE( windy2 )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "P2" )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

/////////////////////////////////////////////

// Stock Windy II cabinet panel for 2 players with 3 buttons + start + service per player
// Corresponds to the dipswitch setting of 01000000 on I/O board
windy2_2l6b_device::windy2_2l6b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	windy2_device(mconfig, KONAMI_WINDY2_JVS_IO_2L6B_PANEL, tag, owner, clock, 2, 9)
{
}

ioport_constructor windy2_2l6b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(windy2_2l6b);
}

/////////////////////////////////////////////

// GU707-JB Quiz panel setup for 2 players with 4 buttons + start + service per player
// Corresponds to the dipswitch setting of 00000000 on I/O board
windy2_quiz_device::windy2_quiz_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	windy2_device(mconfig, KONAMI_WINDY2_JVS_IO_QUIZ_PANEL, tag, owner, clock, 2, 6)
{
}

ioport_constructor windy2_quiz_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(windy2_quiz);
}

/////////////////////////////////////////////

// GU707-JC 2L8B panel setup for 2 players with 4 buttons + 4 levers + start + service per player
// Corresponds to the dipswitch setting of 11000000 on I/O board
windy2_2l8b_device::windy2_2l8b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	windy2_device(mconfig, KONAMI_WINDY2_JVS_IO_2L8B_PANEL, tag, owner, clock, 2, 10)
{
}

ioport_constructor windy2_2l8b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(windy2_2l8b);
}

/////////////////////////////////////////////

// GU707-JD 1L6B panel setup for 1 player with 6 buttons + 4 levers + start + test
// Corresponds to the dipswitch setting of 10101001 on I/O board
windy2_1l6b_device::windy2_1l6b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	windy2_device(mconfig, KONAMI_WINDY2_JVS_IO_1L6B_PANEL, tag, owner, clock, 1, 12)
{
}

ioport_constructor windy2_1l6b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(windy2_1l6b);
}

/////////////////////////////////////////////

// GU707-JG 2L12B panel setup for 2 players with 6 buttons + 4 levers + start + service per player
// Corresponds to the dipswitch setting of 10101000 on I/O board
windy2_2l12b_device::windy2_2l12b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	windy2_device(mconfig, KONAMI_WINDY2_JVS_IO_2L12B_PANEL, tag, owner, clock, 2, 12)
{
}

ioport_constructor windy2_2l12b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(windy2_2l12b);
}
