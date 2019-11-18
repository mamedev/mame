// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

  PINBALL
  LTD (Brazil)

  Not much info available for these machines. System 3 has a homebrew partial
  schematic available (with some obvious mistakes). There's also a manual
  (looks like Portuguese?), but no schematic. No info on system 4 has been found.

  Used PinMAME as a reference.

  System 3: NMI is connected to "FICHA" (coin slot). RST is connected to "TILT".

  The manual mentions these machines:
  Arizona, Atlantis, Galaxia, Hustler, Martian Queen.

  PinMAME has a large list of games, these are:
  1977: O Gaucho, Samba
  1978: Grand Prix
  1981: Al Capone, Amazon, Arizona, Atlantis, Black Hole, Carnaval no Rio,
        Cowboy Eight Ball, Disco Dancing, Force, Galaxia, Haunted Hotel,
        Hustler, King Kong, Kung Fu, Mr. & Mrs. Pec-Men, Martian Queen,
        Space Poker, Time Machine, Zephy
  1982: Alien Warrior, Columbia, Cowboy 2, Trick Shooter
  (unknown year): Viking King

ToDo:
- No mechanical sounds
- Although nvram is fitted, everything is lost at powerup
- System 3, no sound
- System 3, slam tilt to connect to reset line
- Zephy, no playfield inputs
- System 4, no playfield inputs
- System 4, can randomly freeze MAME for no reason
- Alcapone, display needs fixing
- Outputs
- Mechanical

********************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6801.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "ltd.lh"


class ltd_state : public genpin_class
{
public:
	ltd_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "nvram")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void ltd4(machine_config &config);
	void ltd3(machine_config &config);

	void init_atla_ltd();
	void init_bhol_ltd();
	void init_zephy();
	void init_ltd();

	DECLARE_INPUT_CHANGED_MEMBER(ficha);

private:
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_WRITE8_MEMBER(count_reset_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_r);
	void ltd3_map(address_map &map);
	void ltd4_map(address_map &map);

	bool m_timer_r;
	bool m_clear;
	uint8_t m_counter;
	uint8_t m_digit;
	uint8_t m_game;
	uint8_t m_out_offs;
	uint8_t m_port2;
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_ram;
	output_finder<50> m_digits;
};


void ltd_state::ltd3_map(address_map &map)
{
	map(0x0000, 0x007f).ram().share("nvram"); // internal to the cpu
	map(0x0080, 0x0087).mirror(0x78).r(FUNC(ltd_state::io_r));
	map(0x0800, 0x2fff).w(FUNC(ltd_state::io_w));
	map(0xc000, 0xcfff).rom().mirror(0x3000).region("roms", 0);
}

void ltd_state::ltd4_map(address_map &map)
{
	map(0x0100, 0x01ff).ram().share("nvram");
	map(0x0800, 0x0800).w(FUNC(ltd_state::count_reset_w));
	map(0x0c00, 0x0c00).w("aysnd_1", FUNC(ay8910_device::reset_w));
	map(0x1000, 0x1000).w("aysnd_0", FUNC(ay8910_device::address_w));
	map(0x1400, 0x1400).w("aysnd_0", FUNC(ay8910_device::reset_w));
	map(0x1800, 0x1800).w("aysnd_1", FUNC(ay8910_device::address_w));
	//map(0x2800, 0x2800).w(FUNC(ltd_state::auxlamps_w));
	map(0x3000, 0x3000).w("aysnd_0", FUNC(ay8910_device::data_w));
	map(0x3800, 0x3800).w("aysnd_1", FUNC(ay8910_device::data_w));
	map(0xc000, 0xdfff).rom().mirror(0x2000).region("roms", 0);
}

// bits 6,7 not connected to data bus
// 1=does something in Atlantis; 2=does something in Black Hole; note that sometimes pressing G or H will reboot the machine.
static INPUT_PORTS_START( ltd3 )
	PORT_START("FICHA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, ltd_state, ficha, 0)

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD) //tilt
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) //1,2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) //1,2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) //1,2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) //1,2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) //1,2
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH) //2
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON) //1,2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE) //1,2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH) //1,2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE) //1,2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE) //1
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) //1,2
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) //1,2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) //1,2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) //1,2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) //2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) //2
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) //2
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) //2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) //2
	PORT_BIT( 0x18, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole") // 1,2
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// this needs to be redone once inputs start to work
static INPUT_PORTS_START( ltd4 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT ) // tilt all
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE) // start pecmen
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) //coin all
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 ) //coin cowboy,alcapone; start pecmen
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) //start cowboy,alcapone
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS_PAD) // credit pecmen
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( ltd_state::ficha )
{
	if(newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

// switches
READ8_MEMBER( ltd_state::io_r )
{
	if (offset==0)
		return ioport("X0")->read();
	else
	if (offset==1)
		return ioport("X1")->read();
	else
	if (offset==2)
		return ioport("X2")->read();
	else
	if (offset==3)
		return ioport("X3")->read();
	else
	if (offset==7)
		return ioport("X7")->read();

	return 0xff;
}

// Lamps only used by Zephy
WRITE8_MEMBER( ltd_state::io_w )
{
	offset >>= 10; // reduces offsets to 1 per bank
}

READ8_MEMBER( ltd_state:: port1_r )
{
	if (~m_port2 & 0x10)
	{
		uint8_t row = m_digit >> 4;

		if (row==0)
			return ioport("X0")->read();
		else
		if (row==1)
			return ioport("X1")->read();
		else
		if (row==2)
			return ioport("X2")->read();
		else
		if (row==3)
			return ioport("X3")->read();
		else
		if (row==4)
			return ioport("X4")->read();
		else
		if (row==5)
			return ioport("X5")->read();
		else
		if (row==6)
			return ioport("X6")->read();
		else
		//if (row==7)
			return ioport("X7")->read();
	}
	return 0xff;
}

WRITE8_MEMBER( ltd_state::port1_w )
{
	if (m_port2 & 0x10)
	{
		uint8_t row = m_digit & 15;
		uint8_t segment = bitswap<8>(data, 7, 0, 1, 2, 3, 4, 5, 6);

		switch (m_counter)
		{
			case 0:
				m_clear = (data < 0xff);
				break;
			case 6:
				if (m_clear)
					m_digit = data;
				break;
			case 7:
				if (m_clear)
				{
					if (row>7)
						m_digits[row+2] = segment; // P2
					else
						m_digits[row] = segment; // P1
				}
				break;
			case 8:
				if (m_clear)
				{
					if (row>13)
						m_digits[row+26] = segment; // credits / ball
					else
					if (row>7)
						m_digits[row+22] = segment; // P4
					else
						m_digits[row+20] = segment; // P3
				}
				break;
		}
	}
}

READ8_MEMBER( ltd_state:: port2_r )
{
	return m_port2;
}

WRITE8_MEMBER( ltd_state::port2_w )
{
	if (~m_port2 & data & 0x10)
		m_counter++;

	m_port2 = data;
}

WRITE8_MEMBER( ltd_state::count_reset_w )
{
	m_counter = 0;
}

void ltd_state::machine_reset()
{
	m_clear = 0;
	m_counter = 0;
	m_out_offs = 0;
	m_timer_r = 0;
}

void ltd_state::init_ltd()
{
	m_game = 0;
}

void ltd_state::init_atla_ltd()
{
	m_game = 1;
}

void ltd_state::init_bhol_ltd()
{
	m_game = 2;
}

void ltd_state::init_zephy()
{
	m_game = 3;
}

TIMER_DEVICE_CALLBACK_MEMBER( ltd_state::timer_r )
{
	m_timer_r ^= 1;
	m_maincpu->set_input_line(M6802_IRQ_LINE, (m_timer_r) ? CLEAR_LINE : ASSERT_LINE);
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	m_out_offs++;
	if (m_out_offs > 0x7f) m_out_offs = 0x60;

	if ((m_out_offs > 0x5f) && (m_out_offs < 0x6a))
	{
		switch(m_game)
		{
			case 1: // atlantis (2-player, 5-digit)
			{
				m_digits[0] = 0x3f;
				m_digits[10] = 0x3f;
				switch(m_out_offs-0x60)
				{
					case 0:
						m_digits[1] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[2] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 1:
						m_digits[11] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[12] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 2:
						m_digits[3] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[4] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 3:
						m_digits[13] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[14] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 8:
						m_digits[41] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[40] = patterns[m_p_ram[m_out_offs]>>4];
						break;
				}
				break;
			}
			case 2: // black hole (2-player, 6-digit)
			{
				switch(m_out_offs-0x60)
				{
					case 0:
						m_digits[0] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[1] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 1:
						m_digits[10] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[11] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 2:
						m_digits[2] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[3] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 3:
						m_digits[12] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[13] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 4:
						m_digits[4] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[5] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 5:
						m_digits[14] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[15] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 8:
						m_digits[41] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[40] = patterns[m_p_ram[m_out_offs]>>4];
						break;
				}
				break;
			}
			case 3: // zephy (3-player, 6-digit)
			{
				switch(m_out_offs-0x60)
				{
					case 0:
						m_digits[0] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[1] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 1:
						m_digits[2] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[3] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 2:
						m_digits[4] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[5] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 3:
						m_digits[10] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[11] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 4:
						m_digits[12] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[13] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 5:
						m_digits[14] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[15] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 6:
						m_digits[20] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[21] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 7:
						m_digits[22] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[23] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 8:
						m_digits[24] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[25] = patterns[m_p_ram[m_out_offs]>>4];
						break;
					case 9:
						m_digits[40] = patterns[m_p_ram[m_out_offs]&15];
						m_digits[41] = patterns[m_p_ram[m_out_offs]>>4];
						break;
				}
				break;
			}
		}
	}
}

void ltd_state::ltd3(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &ltd_state::ltd3_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_ltd);

	/* Sound */
	genpin_audio(config);

	TIMER(config, "timer_r").configure_periodic(FUNC(ltd_state::timer_r), attotime::from_hz(500));
}

void ltd_state::ltd4(machine_config &config)
{
	/* basic machine hardware */
	m6803_cpu_device &maincpu(M6803(config, "maincpu", XTAL(3'579'545))); // guess, no details available
	maincpu.set_addrmap(AS_PROGRAM, &ltd_state::ltd4_map);
	maincpu.in_p1_cb().set(FUNC(ltd_state::port1_r));
	maincpu.out_p1_cb().set(FUNC(ltd_state::port1_w));
	maincpu.in_p2_cb().set(FUNC(ltd_state::port2_r));
	maincpu.out_p2_cb().set(FUNC(ltd_state::port2_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_ltd);

	/* Sound */
	genpin_audio(config);

	SPEAKER(config, "mono").front_center();
	AY8910(config, "aysnd_0", XTAL(3'579'545)/2).add_route(ALL_OUTPUTS, "mono", 0.3); /* guess */
	AY8910(config, "aysnd_1", XTAL(3'579'545)/2).add_route(ALL_OUTPUTS, "mono", 0.3); /* guess */
}

/*-------------------------------------------------------------------
/ Arizona
/-------------------------------------------------------------------*/
ROM_START(arizona)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("arizltd.bin", 0x0000, 0x0400, CRC(908f00d8) SHA1(98f28f1aedbad43e0e096959fdef45e038405473))
	ROM_RELOAD(0x0400, 0x0400)
	ROM_RELOAD(0x0800, 0x0400)
	ROM_RELOAD(0x0c00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Atlantis
/-------------------------------------------------------------------*/
ROM_START(atla_ltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("atlantis.bin", 0x0000, 0x0800, CRC(c61be043) SHA1(e6c4463f59a5743fa34aa55beeb6f536ad9f1b56))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Disco Dancing
/-------------------------------------------------------------------*/
ROM_START(discodan)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("disco.bin", 0x0000, 0x0800, CRC(83c79157) SHA1(286fd0c984870639fcd7d7b8f6a5a5ddabcddcf5))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Hustler
/-------------------------------------------------------------------*/
ROM_START(hustlerp)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("hustler_1.bin", 0x0000, 0x0800, CRC(43f323f5) SHA1(086b81699bea08b10b4231e398f4f689395355b0))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Martian Queen
/-------------------------------------------------------------------*/

// No good dump available

/*-------------------------------------------------------------------
/ King Kong
/-------------------------------------------------------------------*/

ROM_START(kkongltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("kong.bin", 0x0000, 0x0800, CRC(5b2a3123) SHA1(eee417d17d3272ee63c728915af84da33f1f73a2))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Viking King
/-------------------------------------------------------------------*/

ROM_START(vikngkng)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("vikking.bin", 0x0000, 0x0800, CRC(aa32d158) SHA1(b24294ae4ecb2ab3119ad7fe79ef567b19ac792a))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Force
/-------------------------------------------------------------------*/

ROM_START(force)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("forceltd.bin", 0x0000, 0x0800, CRC(48f9ebbe) SHA1(8aaab352fb21263b1b93ffefd9b5169284083beb))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Space Poker
/-------------------------------------------------------------------*/

// No good dump available

/*-------------------------------------------------------------------
/ Black Hole
/-------------------------------------------------------------------*/
ROM_START(bhol_ltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("blackhol.bin", 0x0000, 0x0800, CRC(9f6ae35e) SHA1(c17bf08a41c6cf93550671b0724c58e8ac302c33))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Cowboy Eight Ball
/-------------------------------------------------------------------*/

ROM_START(cowboy)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("cowboy3p.bin", 0x0000, 0x1000, CRC(5afa29af) SHA1(a5ccf5cd17c63d4292222b792535187b1bcfa786))
ROM_END

/*-------------------------------------------------------------------
/ Zephy
/-------------------------------------------------------------------*/
ROM_START(zephy)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("zephy.l2", 0x0000, 0x1000, CRC(8dd11287) SHA1(8133d0c797eb0fdb56d83fc55da91bfc3cddc9e3))
ROM_END

ROM_START(zephya)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("zephy1.bin", 0x0000, 0x1000, CRC(ae189c8a) SHA1(c309b436ef94cd5c266c88fe5f222261e083e4eb))
ROM_END

/*-------------------------------------------------------------------
/ Cowboy Eight Ball 2
/-------------------------------------------------------------------*/
ROM_START(cowboy2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("cowboy_l.bin", 0x0000, 0x1000, CRC(87befe2a) SHA1(93fdf40b10e53d7d95e5dc72923b6be887411fc0))
	ROM_LOAD("cowboy_h.bin", 0x1000, 0x1000, CRC(105e5d7b) SHA1(75edeab8c8ba19f334479133802acbc25f405763))
ROM_END

/*-------------------------------------------------------------------
/ Haunted Hotel
/-------------------------------------------------------------------*/
ROM_START(hhotel)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("hh1.bin", 0x0000, 0x1000, CRC(a107a683) SHA1(5bb79d9a0a6b33f067cdd54942784c67ab557909))
	ROM_LOAD("hh2.bin", 0x1000, 0x1000, CRC(e0c2ebc1) SHA1(131240589162c7b3f44a2bb951945c7d64f89c8d))
ROM_END

/*-------------------------------------------------------------------
/ Mr. & Mrs. Pec-Men
/-------------------------------------------------------------------*/
ROM_START(pecmen)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("pecmen_l.bin", 0x0000, 0x1000, CRC(f86c724e) SHA1(635ec94a1c6e77800ef9774102cc639be86c4261))
	ROM_LOAD("pecmen_h.bin", 0x1000, 0x1000, CRC(013abca0) SHA1(269376af92368d214c3d09ec6d3eb653841666f3))
ROM_END

/*-------------------------------------------------------------------
/ Al Capone
/-------------------------------------------------------------------*/
ROM_START(alcapone)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("alcapo_l.bin", 0x0000, 0x1000, CRC(c4270ba8) SHA1(f3d80af9900c94df2d43f2755341a346a0b64c87))
	ROM_LOAD("alcapo_h.bin", 0x1000, 0x1000, CRC(279f766d) SHA1(453c58e44c4ef8f1f9eb752b6163c61ebed70b27))
ROM_END

/*-------------------------------------------------------------------
/ Alien Warrior
/-------------------------------------------------------------------*/

// No good dump available

/*-------------------------------------------------------------------
/ Columbia
/-------------------------------------------------------------------*/
ROM_START(columbia)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("columb-l.bin", 0x0000, 0x1000, CRC(ac345dee) SHA1(14f03fa8059de5cd69cc83638aa6533fbcead37e))
	ROM_LOAD("columb-h.bin", 0x1000, 0x1000, CRC(acd2a85b) SHA1(30889ee4230ce05f6060f926b2137bbf5939db2d))
ROM_END

/*-------------------------------------------------------------------
/ Time Machine
/-------------------------------------------------------------------*/
ROM_START(tmacltd4)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("tm4-l.bin", 0x0000, 0x1000, NO_DUMP)
	ROM_LOAD("tm4-h.bin", 0x1000, 0x1000, CRC(f5f97992) SHA1(ba31f71a600e7061b500e0750f50643503e52a80))
ROM_END

ROM_START(tmacltd2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("tm4-l.bin", 0x0000, 0x1000, NO_DUMP)
	ROM_LOAD("tm4-h.bin", 0x1000, 0x1000, CRC(f717c9db) SHA1(9ca5819b707fa20edfc289734e1aa189ae242aa3))
ROM_END

/*-------------------------------------------------------------------
/ Trick Shooter
/-------------------------------------------------------------------*/
ROM_START(tricksht)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("tricks-l.bin", 0x0000, 0x1000, CRC(951413ff) SHA1(f4a28f7b41cb077377433dc7bfb6647e5d392481))
	ROM_LOAD("tricks-h.bin", 0x1000, 0x1000, CRC(2e4efb51) SHA1(3dd20addecf4b47bd68b05d557c378d1dbbbd892))
ROM_END

// system 3
GAME(1981, arizona,  0,        ltd3, ltd3, ltd_state, init_atla_ltd, ROT0, "LTD", "Arizona",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, atla_ltd, 0,        ltd3, ltd3, ltd_state, init_atla_ltd, ROT0, "LTD", "Atlantis (LTD)",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME(1981, discodan, 0,        ltd3, ltd3, ltd_state, init_atla_ltd, ROT0, "LTD", "Disco Dancing",                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, hustlerp, 0,        ltd3, ltd3, ltd_state, init_atla_ltd, ROT0, "LTD", "Hustler",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, kkongltd, 0,        ltd3, ltd3, ltd_state, init_atla_ltd, ROT0, "LTD", "King Kong",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(198?, vikngkng, 0,        ltd3, ltd3, ltd_state, init_atla_ltd, ROT0, "LTD", "Viking King",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, force,    0,        ltd3, ltd3, ltd_state, init_atla_ltd, ROT0, "LTD", "Force",                             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, bhol_ltd, 0,        ltd3, ltd3, ltd_state, init_bhol_ltd, ROT0, "LTD", "Black Hole (LTD)",                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME(1981, cowboy,   0,        ltd3, ltd3, ltd_state, init_zephy,    ROT0, "LTD", "Cowboy Eight Ball",                 MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, zephy,    0,        ltd3, ltd3, ltd_state, init_zephy,    ROT0, "LTD", "Zephy",                             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, zephya,   zephy,    ltd3, ltd3, ltd_state, init_zephy,    ROT0, "LTD", "Zephy (alternate set)",             MACHINE_IS_SKELETON_MECHANICAL)

// system 4
GAME(1982, cowboy2,  0,        ltd4, ltd4, ltd_state, init_ltd,      ROT0, "LTD", "Cowboy Eight Ball 2",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, hhotel,   0,        ltd4, ltd4, ltd_state, init_ltd,      ROT0, "LTD", "Haunted Hotel",                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, pecmen,   0,        ltd4, ltd4, ltd_state, init_ltd,      ROT0, "LTD", "Mr. & Mrs. Pec-Men",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, alcapone, 0,        ltd4, ltd4, ltd_state, init_ltd,      ROT0, "LTD", "Al Capone",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982, columbia, 0,        ltd4, ltd4, ltd_state, init_ltd,      ROT0, "LTD", "Columbia",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, tmacltd4, 0,        ltd4, ltd4, ltd_state, init_ltd,      ROT0, "LTD", "Time Machine (LTD, 4 players)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, tmacltd2, tmacltd4, ltd4, ltd4, ltd_state, init_ltd,      ROT0, "LTD", "Time Machine (LTD, 2 players)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982, tricksht, 0,        ltd4, ltd4, ltd_state, init_ltd,      ROT0, "LTD", "Trick Shooter",                     MACHINE_IS_SKELETON_MECHANICAL)
