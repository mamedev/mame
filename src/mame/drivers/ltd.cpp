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

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "sound/ay8910.h"
#include "ltd.lh"

class ltd_state : public genpin_class
{
public:
	ltd_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "nvram")
	{ }

	DECLARE_DRIVER_INIT(atla_ltd);
	DECLARE_DRIVER_INIT(bhol_ltd);
	DECLARE_DRIVER_INIT(zephy);
	DECLARE_DRIVER_INIT(ltd);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_WRITE8_MEMBER(count_reset_w);
	DECLARE_INPUT_CHANGED_MEMBER(ficha);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_r);
private:
	bool m_timer_r;
	bool m_clear;
	UINT8 m_counter;
	UINT8 m_digit;
	UINT8 m_game;
	UINT8 m_out_offs;
	UINT8 m_port2;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
};


static ADDRESS_MAP_START( ltd3_map, AS_PROGRAM, 8, ltd_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM AM_SHARE("nvram") // internal to the cpu
	AM_RANGE(0x0080, 0x0087) AM_MIRROR(0x78) AM_READ(io_r)
	AM_RANGE(0x0800, 0x2fff) AM_WRITE(io_w)
	AM_RANGE(0xc000, 0xcfff) AM_ROM AM_MIRROR(0x3000) AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ltd4_map, AS_PROGRAM, 8, ltd_state )
	AM_RANGE(0x0000, 0x001f) AM_RAM // internal to the cpu
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0800, 0x0800) AM_WRITE(count_reset_w)
	AM_RANGE(0x0c00, 0x0c00) AM_DEVWRITE("aysnd_1", ay8910_device, reset_w)
	AM_RANGE(0x1000, 0x1000) AM_DEVWRITE("aysnd_0", ay8910_device, address_w)
	AM_RANGE(0x1400, 0x1400) AM_DEVWRITE("aysnd_0", ay8910_device, reset_w)
	AM_RANGE(0x1800, 0x1800) AM_DEVWRITE("aysnd_1", ay8910_device, address_w)
	//AM_RANGE(0x2800, 0x2800) AM_WRITE(auxlamps_w)
	AM_RANGE(0x3000, 0x3000) AM_DEVWRITE("aysnd_0", ay8910_device, data_w)
	AM_RANGE(0x3800, 0x3800) AM_DEVWRITE("aysnd_1", ay8910_device, data_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROM AM_MIRROR(0x2000) AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ltd4_io, AS_IO, 8, ltd_state )
	AM_RANGE(0x0100, 0x0100) AM_READWRITE(port1_r,port1_w)
	AM_RANGE(0x0101, 0x0101) AM_READWRITE(port2_r,port2_w)
ADDRESS_MAP_END

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
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
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
		UINT8 row = m_digit >> 4;

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
		UINT8 row = m_digit & 15;
		UINT8 segment = BITSWAP8(data, 7, 0, 1, 2, 3, 4, 5, 6);

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
						output_set_digit_value(row+2, segment); // P2
					else
						output_set_digit_value(row, segment); // P1
				}
				break;
			case 8:
				if (m_clear)
				{
					if (row>13)
						output_set_digit_value(row+26, segment); // credits / ball
					else
					if (row>7)
						output_set_digit_value(row+22, segment); // P4
					else
						output_set_digit_value(row+20, segment); // P3
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

DRIVER_INIT_MEMBER( ltd_state, ltd )
{
	m_game = 0;
}

DRIVER_INIT_MEMBER( ltd_state, atla_ltd )
{
	m_game = 1;
	output_set_digit_value(0, 0x3f);
	output_set_digit_value(10, 0x3f);
}

DRIVER_INIT_MEMBER( ltd_state, bhol_ltd )
{
	m_game = 2;
}

DRIVER_INIT_MEMBER( ltd_state, zephy )
{
	m_game = 3;
}

TIMER_DEVICE_CALLBACK_MEMBER( ltd_state::timer_r )
{
	m_timer_r ^= 1;
	m_maincpu->set_input_line(M6800_IRQ_LINE, (m_timer_r) ? CLEAR_LINE : ASSERT_LINE);
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	m_out_offs++;
	if (m_out_offs > 0x7f) m_out_offs = 0x60;

	if ((m_out_offs > 0x5f) && (m_out_offs < 0x6a))
	{
		switch(m_game)
		{
			case 1: // atlantis (2-player, 5-digit)
			{
				switch(m_out_offs-0x60)
				{
					case 0:
						output_set_digit_value(1, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(2, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 1:
						output_set_digit_value(11, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(12, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 2:
						output_set_digit_value(3, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(4, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 3:
						output_set_digit_value(13, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(14, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 8:
						output_set_digit_value(41, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(40, patterns[m_p_ram[m_out_offs]>>4]);
						break;
				}
				break;
			}
			case 2: // black hole (2-player, 6-digit)
			{
				switch(m_out_offs-0x60)
				{
					case 0:
						output_set_digit_value(0, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(1, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 1:
						output_set_digit_value(10, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(11, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 2:
						output_set_digit_value(2, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(3, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 3:
						output_set_digit_value(12, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(13, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 4:
						output_set_digit_value(4, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(5, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 5:
						output_set_digit_value(14, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(15, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 8:
						output_set_digit_value(41, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(40, patterns[m_p_ram[m_out_offs]>>4]);
						break;
				}
				break;
			}
			case 3: // zephy (3-player, 6-digit)
			{
				switch(m_out_offs-0x60)
				{
					case 0:
						output_set_digit_value(0, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(1, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 1:
						output_set_digit_value(2, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(3, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 2:
						output_set_digit_value(4, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(5, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 3:
						output_set_digit_value(10, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(11, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 4:
						output_set_digit_value(12, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(13, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 5:
						output_set_digit_value(14, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(15, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 6:
						output_set_digit_value(20, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(21, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 7:
						output_set_digit_value(22, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(23, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 8:
						output_set_digit_value(24, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(25, patterns[m_p_ram[m_out_offs]>>4]);
						break;
					case 9:
						output_set_digit_value(40, patterns[m_p_ram[m_out_offs]&15]);
						output_set_digit_value(41, patterns[m_p_ram[m_out_offs]>>4]);
						break;
				}
				break;
			}

		}
	}
}

static MACHINE_CONFIG_START( ltd3, ltd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6802, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(ltd3_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_ltd)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_r", ltd_state, timer_r, attotime::from_hz(500))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ltd4, ltd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6803, XTAL_3_579545MHz) // guess, no details available
	MCFG_CPU_PROGRAM_MAP(ltd4_map)
	MCFG_CPU_IO_MAP(ltd4_io)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_ltd)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd_0", AY8910, XTAL_3_579545MHz/2) /* guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.3)
	MCFG_SOUND_ADD("aysnd_1", AY8910, XTAL_3_579545MHz/2) /* guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.3)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Atlantis
/-------------------------------------------------------------------*/
ROM_START(atla_ltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("atlantis.bin", 0x0000, 0x0800, CRC(c61be043) SHA1(e6c4463f59a5743fa34aa55beeb6f536ad9f1b56))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Black Hole
/-------------------------------------------------------------------*/
ROM_START(bhol_ltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("blackhol.bin", 0x0000, 0x0800, CRC(9f6ae35e) SHA1(c17bf08a41c6cf93550671b0724c58e8ac302c33))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Zephy
/-------------------------------------------------------------------*/
ROM_START(zephy)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("zephy.l2", 0x0000, 0x1000, CRC(8dd11287) SHA1(8133d0c797eb0fdb56d83fc55da91bfc3cddc9e3))
ROM_END

/*-------------------------------------------------------------------
/ Cowboy Eight Ball
/-------------------------------------------------------------------*/
ROM_START(cowboy)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("cowboy_l.bin", 0x0000, 0x1000, CRC(87befe2a) SHA1(93fdf40b10e53d7d95e5dc72923b6be887411fc0))
	ROM_LOAD("cowboy_h.bin", 0x1000, 0x1000, CRC(105e5d7b) SHA1(75edeab8c8ba19f334479133802acbc25f405763))
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
/ Columbia
/-------------------------------------------------------------------*/
ROM_START(columbia)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("columb-d.bin", 0x0000, 0x1000, NO_DUMP)
	ROM_LOAD("columb-e.bin", 0x1000, 0x1000, CRC(013abca0) SHA1(269376af92368d214c3d09ec6d3eb653841666f3))
ROM_END

// system 3
GAME(1981, atla_ltd, 0,  ltd3,  ltd3, ltd_state, atla_ltd, ROT0, "LTD", "Atlantis (LTD)",     MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME(1981, bhol_ltd, 0,  ltd3,  ltd3, ltd_state, bhol_ltd, ROT0, "LTD", "Black Hole (LTD)",   MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME(1981, zephy,    0,  ltd3,  ltd3, ltd_state, zephy,    ROT0, "LTD", "Zephy",              MACHINE_IS_SKELETON_MECHANICAL)

// system 4
GAME(1981, cowboy,   0,  ltd4,  ltd4, ltd_state, ltd,      ROT0, "LTD", "Cowboy Eight Ball 2", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, pecmen,   0,  ltd4,  ltd4, ltd_state, ltd,      ROT0, "LTD", "Mr. & Mrs. Pec-Men", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981, alcapone, 0,  ltd4,  ltd4, ltd_state, ltd,      ROT0, "LTD", "Al Capone",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982, columbia, 0,  ltd4,  ltd4, ltd_state, ltd,      ROT0, "LTD", "Columbia",           MACHINE_IS_SKELETON_MECHANICAL)
