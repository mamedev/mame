// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

/* Senario 6 Player Poker systems

TODO:
improve controller hookup / simulation and remove PC-based hacks!

*/

#include "emu.h"
#include "spg2xx.h"

#include "sentx6p.lh"


namespace {

class sentx6p_state : public spg2xx_game_state
{
public:
	sentx6p_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_porta_data(0x0000),
		m_suite1(*this, "SUITE_LEFT_BZ%u", 0U),
		m_suite2(*this, "SUITE_RIGHT_BZ%u", 0U),
		m_number1(*this, "NUMBER_LEFT_BZ%u", 0U),
		m_number2(*this, "NUMBER_RIGHT_BZ%u", 0U),
		m_select_fold(*this, "SELECT_FOLD_BZ%u", 0U),
		m_select_check(*this, "SELECT_CHECK_BZ%u", 0U),
		m_select_bet(*this, "SELECT_BET_BZ%u", 0U),
		m_select_call(*this, "SELECT_CALL_BZ%u", 0U),
		m_select_raise(*this, "SELECT_RAISE_BZ%u", 0U),
		m_select_allin(*this, "SELECT_ALLIN_BZ%u", 0U),
		m_option_fold(*this, "OPTION_FOLD_BZ%u", 0U),
		m_option_check(*this, "OPTION_CHECK_BZ%u", 0U),
		m_option_bet(*this, "OPTION_BET_BZ%u", 0U),
		m_option_call(*this, "OPTION_CALL_BZ%u", 0U),
		m_option_raise(*this, "OPTION_RAISE_BZ%u", 0U),
		m_option_allin(*this, "OPTION_ALLIN_BZ%u", 0U),

		m_led(*this, "LED_BZ%u", 0U)
	{ }

	void sentx6p(machine_config &config);

	void init_sentx6p();
	void init_sentx6puk();
	void init_sentx6pd();

	void mem_map_2m_texas(address_map &map) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(ok_latch)
	{
		if (newval == 1)
		{
			m_inputlatches[param] |= 0x02;
			logerror("latching OK button for Player %d\n", param+1);
		}
	}

	DECLARE_INPUT_CHANGED_MEMBER(select_latch)
	{
		if (newval == 1)
		{
			m_inputlatches[param] |= 0x01;
			logerror("latching Select button for Player %d\n", param+1);
		}
	}

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint16_t sentx_porta_r();
	uint16_t sentx_portb_r();
	uint16_t sentx_portc_r();

	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	void sentx_tx_w(uint8_t data);

	uint8_t m_lcd_card1[6];
	uint8_t m_lcd_card2[6];
	uint8_t m_lcd_options[6];
	uint8_t m_lcd_options_select[6];
	uint8_t m_lcd_led[6];

	void set_card1(uint8_t value, int select_bits);
	void set_card2(uint8_t value, int select_bits);
	void set_options(uint8_t value, int select_bits);
	void set_options_select(uint8_t value, int select_bits);
	void set_controller_led(uint8_t value, int select_bits);

	void controller_send_data(int which);

	uint8_t m_inputlatches[6];

	uint16_t m_porta_data;

	output_finder<6> m_suite1;
	output_finder<6> m_suite2;
	output_finder<6> m_number1;
	output_finder<6> m_number2;

	output_finder<6> m_select_fold;
	output_finder<6> m_select_check;
	output_finder<6> m_select_bet;
	output_finder<6> m_select_call;
	output_finder<6> m_select_raise;
	output_finder<6> m_select_allin;

	output_finder<6> m_option_fold;
	output_finder<6> m_option_check;
	output_finder<6> m_option_bet;
	output_finder<6> m_option_call;
	output_finder<6> m_option_raise;
	output_finder<6> m_option_allin;

	output_finder<6> m_led;

	// for I/O hacks
	uint32_t m_pchackaddress1 = 0;
	uint32_t m_pchackaddress2 = 0;
	uint16_t m_controller_sense_addr = 0;
};


void sentx6p_state::mem_map_2m_texas(address_map &map)
{
	map(0x000000, 0x3fffff).bankr("cartbank");
	map(0x3f0000, 0x3f7fff).ram();
}

static INPUT_PORTS_START( sentx6p )
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "Port 1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// is this really just the battery sensor, or does it have some other meaning, like data ready?
	PORT_DIPNAME( 0x8000, 0x0000, "Low Battery" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("P2")
	// these must be sense lines
	PORT_DIPNAME( 0x0001, 0x0001, "Player 1 Connected" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Player 2 Connected" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Player 3 Connected" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Player 4 Connected" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Player 5 Connected" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Player 6 Connected" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x0040, 0x0040, "Port 2" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_DIPNAME( 0x0001, 0x0001, "Port 3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_5) PORT_NAME("Console Select") // the Console buttons also work for Player 1
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_1) PORT_NAME("Console Ok")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	// these are presuambly read through the UART as the LCD screens are driven by it, currently not hooked up
	// using PORT_CHANGED_MEMBER because we assume the controller latches them for sending as inputs are only read every few frames
	PORT_START("CTRL1")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,select_latch, 0)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,ok_latch, 0)

	PORT_START("CTRL2")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,select_latch, 1)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,ok_latch, 1)

	PORT_START("CTRL3")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,select_latch, 2)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,ok_latch, 2)

	PORT_START("CTRL4")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,select_latch, 3)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,ok_latch, 3)

	PORT_START("CTRL5")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(5) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,select_latch, 4)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(5) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,ok_latch, 4)

	PORT_START("CTRL6")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(6) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,select_latch, 5)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(6) PORT_CHANGED_MEMBER(DEVICE_SELF, sentx6p_state,ok_latch, 5)

INPUT_PORTS_END


void sentx6p_state::machine_start()
{
	spg2xx_game_state::machine_start();

	m_suite1.resolve();
	m_suite2.resolve();
	m_number1.resolve();
	m_number2.resolve();

	m_select_fold.resolve();
	m_select_check.resolve();
	m_select_bet.resolve();
	m_select_call.resolve();
	m_select_raise.resolve();
	m_select_allin.resolve();

	m_option_fold.resolve();
	m_option_check.resolve();
	m_option_bet.resolve();
	m_option_call.resolve();
	m_option_raise.resolve();
	m_option_allin.resolve();

	m_led.resolve();
}

void sentx6p_state::machine_reset()
{
	spg2xx_game_state::machine_reset();

	for (int i = 0; i < 6; i++)
	{
		m_lcd_card1[i] = 0x00;
		m_lcd_card2[i] = 0x00;
		m_lcd_options[i] = 0x00;
		m_lcd_options_select[i] = 0x00;
		m_lcd_led[i] = 0x00;

		m_inputlatches[i] = 0x00;
	}

	/* HACK: this address needs to contain '1' so that it thinks the first controller
	   is present, otherwise it hangs on boot.  How does this get set?
	   following addresses are for the other controllers, and get set based on Port B
	   status flags */
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	mem.write_word(m_controller_sense_addr,0x0001);
}

void sentx6p_state::controller_send_data(int which)
{
	// 0x78 have to be set, this is probably because there is an unused space in the commands
	// going the other way at 0x78 - 0x7f
	// bit 3 (0x04) can also be set, doesn't care

	uint8_t send = m_inputlatches[which] | 0x78;

	m_maincpu->uart_rx( send );

	if (m_inputlatches[which] & 0x03)
	{
		m_inputlatches[which] = 0x00;
		logerror("clearing input latches for player %d\n", which + 1);
	}
}

uint16_t sentx6p_state::sentx_porta_r()
{
	int select_bits = (m_porta_data >> 8) & 0x3f;
	//logerror("%s: sentx_porta_r (with controller select bits %02x)\n", machine().describe_context(), select_bits);

	/* 0000 = no controller? (system buttons only?)
	   0100 = controller 1?
	   0200 = controller 2?
	   0400 = controller 3?
	   0800 = controller 4?
	   1000 = controller 5?
	   2000 = controller 6?

	   this is an assumption based on startup, where the port is polled after writing those values
	*/

	// the code around 029811 uses a ram value shifted left 8 times as the select bits (select_bits) on write
	// then does a mask with them on the reads from this port, without shifting, comparing with 0

	// the 'select bits' must also be active when the controller wants to send data, the UART read function
	// won't proceed if they're zero, but the port is written with 0 before that

	//logerror("%08x\n", m_maincpu->pc());

	// regular logic for writing to the LCDs
	uint16_t ret = (m_io_p1->read() & 0xffc0) | select_bits;

	// hacks needed for reading from the UART to work

	// the select bit must be high here to read from the controller?
	if (m_maincpu->pc() == m_pchackaddress1)
	{
		ret ^= 0x3f;
	}

	// but must be low again here after writing the select bits in order to not
	// get stuck in a timeout loop
	if (m_maincpu->pc() == m_pchackaddress2)
	{
		ret &= ~0x3f;

		// presumably the inputs can only be sent once the controller is actually selected, otherwise
		// there would be competing UART sources?
		for (int i = 0; i < 6; i++)
		{
			if (select_bits & (1 << i))
			{
				controller_send_data(i);
			}
		}
	}

	return ret;
}

uint16_t sentx6p_state::sentx_portb_r()
{
	return m_io_p2->read();
}

uint16_t sentx6p_state::sentx_portc_r()
{
	return m_io_p3->read();
}

void sentx6p_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: sentx_porta_w %04x\n", machine().describe_context(), data);

	COMBINE_DATA(&m_porta_data);
}

void sentx6p_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: sentx_portb_w %04x\n", machine().describe_context(), data);
}

void sentx6p_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: sentx_portc_w %04x\n", machine().describe_context(), data);
}

/*
    Card Table
    (the controller must contain an MCU under the glob to receive thes commands
     and convert them to actual LCD segments)

         off      00
    | A  diamonds 01 | A  hearts 0e   | A  spades 1b  | A  clubs 28 |
    | 2  diamonds 02 | 2  hearts 0f   | 2  spades 1c  | 2  clubs 29 |
    | 3  diamonds 03 | 3  hearts 10   | 3  spades 1d  | 3  clubs 2a |
    | 4  diamonds 04 | 4  hearts 11   | 4  spades 1e  | 4  clubs 2b |
    | 5  diamonds 05 | 5  hearts 12   | 5  spades 1f  | 5  clubs 2c |
    | 6  diamonds 06 | 6  hearts 13   | 6  spades 20  | 6  clubs 2d |
    | 7  diamonds 07 | 7  hearts 14   | 7  spades 21  | 7  clubs 2e |
    | 8  diamonds 08 | 8  hearts 15   | 8  spades 22  | 8  clubs 2f |
    | 9  diamonds 09 | 9  hearts 16   | 9  spades 23  | 9  clubs 30 |
    | 10 diamonds 0a | 10 hearts 17   | 10 spades 24  | 10 clubs 31 |
    | J  diamonds 0b | J  hearts 18   | J  spades 25  | J  clubs 32 |
    | Q  diamonds 0c | Q  hearts 19   | Q  spades 26  | Q  clubs 33 |
    | K  diamonds 0d | K  hearts 1a   | K  spades 27  | K  clubs 34 |

*/

void sentx6p_state::set_card1(uint8_t value, int select_bits)
{
	for (int i = 0; i < 6; i++)
	{
		if (select_bits & (1 << i))
		{
			m_lcd_card1[i] = value;

			int val = m_lcd_card1[i];
			if (val == 0)
			{
				m_suite1[i] = 0;
				m_number1[i] = 0;
			}
			else
			{
				int suite = (val-1) / 13;
				int number = (val-1) % 13;

				m_suite1[i] = suite+1;
				m_number1[i] = number+1;
			}
		}
	}
}

void sentx6p_state::set_card2(uint8_t value, int select_bits)
{
	for (int i = 0; i < 6; i++)
	{
		if (select_bits & (1 << i))
		{
			m_lcd_card2[i] = value;

			int val = m_lcd_card2[i];
			if (val == 0)
			{
				m_suite2[i] = 0;
				m_number2[i] = 0;
			}
			else
			{
				int suite = (val-1) / 13;
				int number = (val-1) % 13;

				m_suite2[i] = suite+1;
				m_number2[i] = number+1;
			}
		}
	}
}

void sentx6p_state::set_options(uint8_t value, int select_bits)
{
	for (int i = 0; i < 6; i++)
	{
		if (select_bits & (1 << i))
		{
			m_lcd_options[i] = value;

			// assume same mapping as selector bit below
			m_option_fold[i] =  (value & 0x01) ? 1 : 0;
			m_option_check[i] = (value & 0x02) ? 1 : 0;
			m_option_bet[i] =   (value & 0x04) ? 1 : 0;
			m_option_call[i] =  (value & 0x08) ? 1 : 0;
			m_option_raise[i] = (value & 0x10) ? 1 : 0;
			m_option_allin[i] = (value & 0x20) ? 1 : 0;
		}
	}
}

/*
    c0 = no selection highlight (00)
    c1 = fold selected (01)
    c2 = check selected (02)
    c4 = bet selected (04)
    c8 = call selected (08)
    d0 = raise selected (10)
    e0 = all in selected (20)
*/

void sentx6p_state::set_options_select(uint8_t value, int select_bits)
{
	for (int i = 0; i < 6; i++)
	{
		if (select_bits & (1 << i))
		{
			m_lcd_options_select[i] = value;

			m_select_fold[i] =  (value & 0x01) ? 1 : 0;
			m_select_check[i] = (value & 0x02) ? 1 : 0;
			m_select_bet[i] =   (value & 0x04) ? 1 : 0;
			m_select_call[i] =  (value & 0x08) ? 1 : 0;
			m_select_raise[i] = (value & 0x10) ? 1 : 0;
			m_select_allin[i] = (value & 0x20) ? 1 : 0;
		}
	}
}



void sentx6p_state::set_controller_led(uint8_t value, int select_bits)
{
	for (int i = 0; i < 6; i++)
	{
		if (select_bits & (1 << i))
		{
			m_lcd_led[i] = value;

			m_led[i] = value ^ 1;
		}
	}
}

void sentx6p_state::sentx_tx_w(uint8_t data)
{
	int select_bits = (m_porta_data >> 8) & 0x3f;

	// TX function is at 0x029773
	// starts by writing controller select, then checking if controller is selected, then transmits data

	// RX function is at 0x029811
	// similar logic to write controller ID, check if selected, then recieve data

	switch (data)
	{
	case 0x00: // card 1 off
	case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: // card 1 show Diamonds A,2,3,4,5,6,7,8,10,J,Q,K
	case 0x0e: case 0x0f: case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: case 0x1a: // card 1 show Hearts   A,2,3,4,5,6,7,8,10,J,Q,K
	case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // card 1 show Spades   A,2,3,4,5,6,7,8,10,J,Q,K
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: // card 1 show Clubs    A,2,3,4,5,6,7,8,10,J,Q,K
		set_card1(data & 0x3f, select_bits);
		break;

	case 0x40: // card 2 off
	case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: // card 1 show Diamonds A,2,3,4,5,6,7,8,10,J,Q,K
	case 0x4e: case 0x4f: case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: // card 1 show Hearts   A,2,3,4,5,6,7,8,10,J,Q,K
	case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // card 1 show Spades   A,2,3,4,5,6,7,8,10,J,Q,K
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f: case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: // card 1 show Clubs    A,2,3,4,5,6,7,8,10,J,Q,K
		set_card2(data & 0x3f, select_bits);
		break;

	// cases 0x78 - 0x7f are transmissions in the other direction (from controller to main unit, used for sending buttons)

	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f: // show selection options
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7: case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7: case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		set_options(data & 0x3f, select_bits);
		break;

	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: // show selection cursor
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		set_options_select(data & 0x3f, select_bits);
		break;

	case 0x38:
	case 0x39:
		set_controller_led(data & 0x01, select_bits);
		break;

	case 0x3a:
		// reset controller? (the input code also seems to check for 3a in one place coming in the other direction)
		break;

	default:
		logerror("unknown LCD command %02x with controller select %02x\n", data, select_bits);
		break;
	}

}




void sentx6p_state::sentx6p(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &sentx6p_state::mem_map_2m_texas);
	m_maincpu->set_pal(true);

	spg2xx_base(config);

	m_screen->set_refresh_hz(50);

	m_maincpu->porta_in().set(FUNC(sentx6p_state::sentx_porta_r));
	m_maincpu->portb_in().set(FUNC(sentx6p_state::sentx_portb_r));
	m_maincpu->portc_in().set(FUNC(sentx6p_state::sentx_portc_r));

	m_maincpu->porta_out().set(FUNC(sentx6p_state::porta_w));
	m_maincpu->portb_out().set(FUNC(sentx6p_state::portb_w));
	m_maincpu->portc_out().set(FUNC(sentx6p_state::portc_w));

	m_maincpu->uart_tx().set(FUNC(sentx6p_state::sentx_tx_w));

	config.set_default_layout(layout_sentx6p);
}

ROM_START( sentx6p )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "vsmaxxtexasholdem.bin", 0x000000, 0x200000, CRC(d9d7ea1d) SHA1(ae531958d6074ea2034910f53e043d5fa10eaf8c) )
ROM_END

ROM_START( sentx6puk )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	// chksum:05B58350 is @ 0x3000.
	// This is the correct sum of 0x3010 - 0x1fffff (the first half of the ROM area after the checksum string)
	// The 2nd half is not summed (and not used by the game?) but definitely exists in ROM (same data on 2 units)
	ROM_LOAD16_WORD_SWAP( "senario texas holdem.bin", 0x000000, 0x400000, CRC(7c7d2d33) SHA1(71631074ba66e3b0cdeb86ebca3931599f3a911c) )
ROM_END

ROM_START( sentx6pd )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "senariodeluxetvpoker.bin", 0x000000, 0x800000, CRC(e2b6844e) SHA1(2413195dfe7a08d16ca870c070a19034c9fe8c30) )
ROM_END

void sentx6p_state::init_sentx6puk()
{
	m_pchackaddress1 = 0x2981e;
	m_pchackaddress2 = 0x29834;
	m_controller_sense_addr = 0x1e08;
}

void sentx6p_state::init_sentx6p()
{
	m_pchackaddress1 = 0x29754;
	m_pchackaddress2 = 0x2976a;
	m_controller_sense_addr = 0x0804;
}

void sentx6p_state::init_sentx6pd()
{
	m_pchackaddress1 = 0xca20;
	m_pchackaddress2 = 0xca36;
	m_controller_sense_addr = 0x1e3a;
}

} // anonymous namespace


CONS( 2004, sentx6p,    0,           0,        sentx6p,     sentx6p, sentx6p_state, init_sentx6p,   "Senario",               "Vs Maxx Texas Hold'em TV Poker - 6 Player Edition (US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // from a US version, values in USD
CONS( 2004, sentx6puk,  sentx6p,     0,        sentx6p,     sentx6p, sentx6p_state, init_sentx6puk, "Senario / Play Vision", "Vs Maxx Texas Hold'em TV Poker - 6 Player Edition (UK)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // from a UK Play Vision branded box, values in GBP

CONS( 2004, sentx6pd,   0,           0,        sentx6p,     sentx6p, sentx6p_state, init_sentx6pd,  "Senario",               "Deluxe TV Poker - Texas Hold'em, Blackjack & Video Poker (US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// Deluxe version wasn't released outside of US?
