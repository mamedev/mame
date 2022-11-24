// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
 * Konami Windy2 I/O (JVS)
 */

#include "emu.h"
#include "windy2.h"

windy2_device::windy2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jvs_device(mconfig, KONAMI_WINDY2_JVS_IO, tag, owner, clock),
	m_test_port(*this, "TEST"),
	m_player_ports(*this, "P%u", 1U),
	m_panel_type(windy2_device::PANEL_2L6B)
{
}

void windy2_device::device_start()
{
	jvs_device::device_start();
	save_item(NAME(m_panel_type));
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
	int players, switches;

	switch (m_panel_type) {
		case windy2_device::PANEL_QUIZ:
			players = 2;
			switches = 6; // 4 buttons + start + service
			break;

		case windy2_device::PANEL_2L8B:
			players = 2;
			switches = 10; // 4 lever + 4 buttons + start + service
			break;

		case windy2_device::PANEL_1L6B:
			players = 1;
			switches = 12; // 4 lever + 6 buttons + start + service
			break;

		case windy2_device::PANEL_2L12B:
			players = 2;
			switches = 12; // 4 lever + 6 buttons + start + service
			break;

		case windy2_device::PANEL_2L6B: // same panel as 2L12B shown in diagram, but only top 3 buttons are used instead of the full 6 buttons
		default:
			players = 2;
			switches = 9; // 4 lever + 3 buttons + start + service
			break;
	}

	// Switch input function
	*buf++ = 0x01;
	*buf++ = players;
	*buf++ = switches;
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

INPUT_PORTS_START( windy2 )
	PORT_START( "TEST" )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_HIGH )
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "P1" )
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "P2" )
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "COIN" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER( DEVICE_SELF, windy2_device, coin_inserted, 0 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER( DEVICE_SELF, windy2_device, coin_inserted, 1 )
INPUT_PORTS_END

INPUT_PORTS_START( windy2_2l12b )
	PORT_INCLUDE( windy2 )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "P2" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

INPUT_PORTS_START( windy2_2l8b )
	PORT_INCLUDE( windy2 )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "P2" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

INPUT_PORTS_START( windy2_2l6b )
	PORT_INCLUDE( windy2 )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "P2" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

INPUT_PORTS_START( windy2_quiz )
	PORT_INCLUDE( windy2 )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "P2" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

INPUT_PORTS_START( windy2_1l6b )
	PORT_INCLUDE( windy2 )

	PORT_MODIFY( "P1" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER( 1 )
INPUT_PORTS_END

ioport_constructor windy2_device::device_input_ports() const
{
	switch (m_panel_type) {
		case windy2_device::PANEL_2L12B:
			return INPUT_PORTS_NAME(windy2_2l12b);
		case windy2_device::PANEL_2L8B:
			return INPUT_PORTS_NAME(windy2_2l8b);
		case windy2_device::PANEL_QUIZ:
			return INPUT_PORTS_NAME(windy2_quiz);
		case windy2_device::PANEL_1L6B:
			return INPUT_PORTS_NAME(windy2_1l6b);
		case windy2_device::PANEL_2L6B:
		default:
			return INPUT_PORTS_NAME(windy2_2l6b);
	}
}

DEFINE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO, windy2_device, "windy2", "Konami Windy2 I/O")
