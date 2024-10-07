// license:BSD-3-Clause
// copyright-holders: Grull Osgo.
/********************************************************************

  Super Ball

  6-player Electromechanical Roulette.

  Copyright 199x by Electro Chance.
  Industria Argentina.

  Driver by Grull Osgo.

  2 sets dumped:

  - Super Ball (Version EC-3.50 N322),     1991.
  - Super Ball (Version EC-3.50 N165),     1991.


*********************************************************************

  PCB specs: Hardware not available


*********************************************************************

  Edge Connector:


       Solder side |  | Components
  -----------------|--|-----------------
             +5Vcc |01| GND
             +5Vcc |02| -5Vcc
          Player 2 |03| Player 1
          Player 4 |04| Player 3
          Player 6 |06| Player 5
                   |07|
                   |08| Left
             Right |09| Down
        Credits IN |10| Credits OUT
               Bet |11| Up
                   |12|
                   |13|
                   |14|
                   |15|
                   |16|
                   |17|
                   |18| +5VCC
        Counter IN |19|
                   |20| Counter OUT
      Operator Key |21|
                   |22| Programming
                   |23|
             Green |24| Sync
              Blue |25| Red
            +12Vcc |26| Speaker
             +5Vcc |27| GND
             +5Vcc |28| GND


  DIP Switches:

  .---------------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches                          |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +---------------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +---------------------------------------+-----+-----------------------------------------+
  | Reset total                  No       | OFF |                                         |
  | (Full Reset)                 Si       | ON  |                                         |
  '---------------------------------------+-----+-----+-----------------------------------+
  | Repite                       No       |     | OFF |                                   |
  | (repeat)                     Si       |     | ON  |                                   |
  +---------------------------------------+-----+-----+-----+-----------------------------+
  | Demo                         No       |     |     | OFF |                             |
  |                              Si       |     |     | ON  |                             |
  +---------------------------------------+-----+-----+-----+-----+-----------------------+
  | Acceso a Pagina              No       |                 | OFF |                       |
  | (settings access)            Si       |                 | ON  |                       |
  +---------------------------------------+-----------------+-----+-----+-----------------+
  | Test plato                   No       |                       | OFF |                 |
  | (wheel test)                 Si       |                       | ON  |                 |
  +---------------------------------------+-----------------------+-----+-----+-----------+
  | Inmovilizar                  No       |                             | OFF |           |
  | (freeze)                     Si       |                             | ON  |           |
  +---------------------------------------+-----------------------------+-----+-----+-----+
  | Doble cero (double zero)     No       |                                   | OFF |     |
  | (Jackpot program)            Si       |                                   | ON  |     |
  +---------------------------------------+-----------------------------------+-----+-----+
  | Castellano                   No       |                                         | OFF |
  | (spanish)                    Si       |                                         | ON  |
  +---------------------------------------+-----------------------------------------+-----+


*********************************************************************

  Game Initialization
  -------------------

  (You must enable "LAMPS" layout for this procedure)

  When the game runs by the very first time (no nvram yet), it will
  complain about "DATA ERROR" in a blue message window. After a few
  seconds, it will bring up another red window with the "HARD ERROR 02"
  message.

  So, we must initialize the game following the next instructions:

  1.- Toggle ON "Operator Key" (turn to green).

  2.- Press once "Page Key". It will show a green window titled
      "CONTROL ADMINISTRATIVO" and a message with a security token.

  3.- Press again "Page Key". It will bring up a new window where we
      must type a password. At this time, in the layout, under the
      roulette leds will appear the password we need (six numeric digits).

  4.- Enter the required password using the credits in (IN1....IN6)
      and credits out (OUT1...OUT6) buttons following the key assignment
      indications located under the password field.
      Use the "E" button to finish once all numbers were typed.
      Use the "B" button clear last digit typed, in case of mistake.

  5.- Once finished that, the game will reboot and will be ready
      to play. Also, password showed on layout will disappear.

      In case that (by unknown reason) the game asks for
      "CONTROL ADMINISTRATIVO" again, follow the instructions starting
      from step "1". After that, we will can play again.


*********************************************************************/


/*********************************************************************

  Dev notes:

  Again, this driver was written based on a couple of ROM dumps and a
  lot of reverse engineering, with no hardware available, but guessing
  that this firmware and hardware are similar to others well known
  roulettes like Lucky Ball, Lucky Roulette, Corona, Re900, etc...

  The most exciting part of this work was discover that this game runs
  with an electromechanical roulette, not a typical LED roulette. It
  added an extra challenge to the work, which implies a full develop
  of an electromechanical part simulation, objective that finally
  could be reached.

  Surprisingly, this game firmware includes a full communications
  module, accessible via RS232 serial interface, that let the users
  some useful things like reconfigure hardware and game options or
  get different kinds of reports, like accounting, statistics and
  many other technical items. All this tasks are performed from a
  PC running a D.O.S. program provided by the maker and fortunately
  still available. Even more, there is another standalone D.O.S.
  software provided by the manufacturer (also still available) to
  get the passwords needed when hardware fails or administrative
  tasks are required.

  Another interesting thing found on this game is that it can be
  configured for a single or a double zero roulette, depending on
  what kind of roulette has attached.


*********************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "machine/i8255.h"
#include "video/v9938.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"

#include "rulechan.lh"


namespace {

#define CPU_CLOCK       XTAL(8'000'000)     // guess
#define VID_CLOCK       XTAL(21'477'272)    // guess
#define TMS_CLOCK       (CPU_CLOCK / 4)      // guess
#define VDP_MEM         0x20000  // 4x 4464 (64K x 4 DRAM)

#define BIT2    BIT(m_p30,2)
#define BIT3    BIT(m_p30,3)
#define BIT4    BIT(m_p30,4)
#define BIT5    BIT(m_p30,5)

#define SND_FLG 0xf009
#define RAM_PSW 0xf078

#define BALLIN  (m_ballin >= 2)
#define MOTORON ((m_p32 & 0x0f) != 0x00)
#define LEDNOTNUM (m_led != m_num)


class rulechan_state : public driver_device
{
public:
	rulechan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_v9938(*this, "v9938")
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
		, m_aux(*this, "AUX")
		, m_dsw(*this, "DSW")
		, m_keymx(*this, "IN%u", 0)
		, m_lamps(*this, "lamp%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void rulechan(machine_config &config);
	void rulechan_init();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void port0_w(uint8_t data);
	uint8_t port2_r();
	uint8_t port30_r();
	void port31_w(uint8_t data);
	void port32_w(uint8_t data);
	uint8_t psg_portA_r();
	uint8_t psg_portB_r();

	TIMER_DEVICE_CALLBACK_MEMBER( ball_speed );
	TIMER_DEVICE_CALLBACK_MEMBER( wheel_speed );

	uint8_t m_sline;
	uint8_t m_p30;
	uint8_t m_p31;
	uint8_t m_p32;
	uint8_t m_step;
	uint8_t m_updn2;
	uint8_t m_updn3;
	uint8_t m_updn4;
	uint8_t m_num;
	uint8_t m_spin;
	uint8_t m_tspin;
	uint8_t d_spin;
	uint8_t m_ballin;
	uint8_t m_led;
	uint8_t m_pass[6];
	static constexpr uint8_t s_sndsrt[10] = { 0x0a, 0x0e, 0x06, 0x0a, 0x0b, 0x48, 0x0c, 0x00, 0x0d, 0x01 };

	required_device<v9938_device> m_v9938;
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_ioport m_aux;
	required_ioport m_dsw;
	required_ioport_array<6> m_keymx;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_sort();
	void sound_off();

	output_finder<63> m_lamps;
	output_finder<6> m_digits;
};

constexpr uint8_t rulechan_state::s_sndsrt[10];

void rulechan_state::machine_start()
{
	m_lamps.resolve();
	m_digits.resolve();

	save_item(NAME(m_sline));
	save_item(NAME(m_p30));
	save_item(NAME(m_p31));
	save_item(NAME(m_p32));
	save_item(NAME(m_step));
	save_item(NAME(m_updn2));
	save_item(NAME(m_updn3));
	save_item(NAME(m_updn4));
	save_item(NAME(m_num));
	save_item(NAME(m_spin));
	save_item(NAME(m_tspin));
	save_item(NAME(d_spin));
	save_item(NAME(m_ballin));
	save_item(NAME(m_led));
	save_item(NAME(m_pass));
}

/* BCD to Seven Segment Decoder */

static uint8_t dec_7seg(int data)
{
	uint8_t segment;
	switch (data)
	{
		case 0: segment = 0x3f; break;
		case 1: segment = 0x06; break;
		case 2: segment = 0x5b; break;
		case 3: segment = 0x4f; break;
		case 4: segment = 0x66; break;
		case 5: segment = 0x6d; break;
		case 6: segment = 0x7d; break;
		case 7: segment = 0x07; break;
		case 8: segment = 0x7f; break;
		case 9: segment = 0x6f; break;
		default: segment = 0x00;
	}

	return segment;
}


/**************************************
*             Memory Map              *
**************************************/

void rulechan_state::main_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xffff).ram().share("nvram");
}

void rulechan_state::main_io(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x03).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x11).w("ay8910", FUNC(ay8910_device::address_data_w));                  // sound ok.
	map(0x12, 0x12).r("ay8910", FUNC(ay8910_device::data_r));                          // ports ok.
	map(0x20, 0x23).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));  // video ok.
	map(0x30, 0x33).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));   // wheel control.
	map(0x40, 0x43).nopr().nopw();                                                     // serial communications?
	map(0x60, 0x60).nopw();                                                            // Watchdog.
}


/**************************
*      Read Handlers      *
**************************/

uint8_t rulechan_state::port2_r()
{
	return m_keymx[m_sline]->read();
}

/******************************
   Port 30 - Wheel control    *
*  bit 2 - ball detector      *
*  bit 3 - step detector      *
*  bit 4 - Zero detector      *
*  bit 5 - Ball in shooter    *
******************************/

uint8_t rulechan_state::port30_r()
{
	return m_p30;
}

uint8_t rulechan_state::psg_portA_r()
{
	m_lamps[60] = (BIT(m_aux->read(), 3)) ? 0 : 1;    // Show Operator Key via layout lamp.
	m_lamps[61] = (BIT(m_aux->read(), 7)) ? 0 : 1;    // Show Page Key via layout lamp.
	return m_aux->read();                             // Operator Key read.
}

uint8_t rulechan_state::psg_portB_r()
{
	return m_dsw->read();                   // DIP Switch read.
}


/***********************
*    Write Handlers    *
***********************/

void rulechan_state::port0_w(uint8_t data)
{
	m_sline = data & 0x07;                 // Matrix scan line selector.

	if (m_sline > 5)
		m_sline = 0;
}

/****************************************
   Port 31 - Wheel driver               *
*  bit 1 - Port 32 Data Latch impulse   *
*  bit 4 - ball drop control            *
*  bit 7 - ball shooter                 *
****************************************/

void rulechan_state::port31_w(uint8_t data)
{
	m_p31 = data;

	if (BIT(m_p31, 4))
	{
		m_p30 |= 0x20;
		m_ballin = 0;      // Drop ball....ball in shooter.
	}

	if (BIT(m_p31, 7))                      // Shoot ball.
	{
		m_p30 &= 0xdf;                     // ball out....
		m_num = machine().rand() % 37;     // sort winning number.

		LOG("shooting ball 2d\n", m_num);
	}
}

/****************************************
   Port 32h - Motor Control             *
*                                       *
*  data latch for speed control ...     *
*                                       *
****************************************/

void rulechan_state::port32_w(uint8_t data)
{
	m_p32 = data;
}


/************************************************
*    Extra Sound - Roulette spinning ball       *
************************************************/

void rulechan_state::sound_off()
{
	m_maincpu->space(AS_IO).write_byte(0x10, 0x07);
	m_maincpu->space(AS_IO).write_byte(0x11, m_maincpu->space(AS_PROGRAM).read_byte(SND_FLG) | 0x20);
	m_maincpu->space(AS_IO).write_byte(0x10, 0x0e);
}

void rulechan_state::sound_sort()
{
	for (int i = 0; i < 5; i++)
	{
		m_maincpu->space(AS_IO).write_byte(0x10, s_sndsrt[(2 * i)]);
		m_maincpu->space(AS_IO).write_byte(0x11, s_sndsrt[(2 * i) + 1]);
	}
	m_maincpu->space(AS_IO).write_byte(0x10, 0x07);
	m_maincpu->space(AS_IO).write_byte(0x11, m_maincpu->space(AS_PROGRAM).read_byte(SND_FLG) & 0xdf);
	m_maincpu->space(AS_IO).write_byte(0x10, 0x0e);
}


/***********************
*    Wheel Simulation  *
***********************/

TIMER_DEVICE_CALLBACK_MEMBER(rulechan_state::wheel_speed)
{
	if (m_step == 0)
	{
		if ((BIT4) && (m_updn4 == 0))
		{
			m_p30 &= 0xef;
			m_updn4 = 1;

			LOG("1:port_p30:- Reset bit 4 pulse start -%2x cont_pasos:%2d\n", m_p30, m_step);
			return;
		}

		if ((!BIT4) && (m_updn4 == 1))
		{
			m_p30 |= 0x10;

			LOG("2:port_p30:- Set bit 4 -%2x cont_pasos:%2d\n", m_p30, m_step);
			return;
		}

		if ((BIT4) && (m_updn4 == 1))
		{
			m_updn4 = 0;
			m_step++;

			LOG("3:port_p30:-end mark for reset bit 4 -%2x cont_pasos:%2d\n", m_p30, m_step);
			return;
		}
	}
	else
	{
		if (BIT3 && (m_updn3 == 0))
		{
			m_p30 &= 0xf7;

			LOG("4:port_p30:-reset bit 3 -%2x cont_pasos:%2d\n", m_p30, m_step);
			return;
		}

		if (!BIT3 && (m_updn3 == 0))
		{
			if (!BIT2)
			{
				m_p30 |= 0x04;

				LOG("5:port_p30:-set bit 2 -%2x cont_pasos:%2d\n", m_p30, m_step);
				return;
			}
			else
			{
				if ((m_step - 1 == m_num) && (m_updn2 == 0))
				{
					if (!BIT5)   // ball in pocket?...
					{
						m_p30 &= 0xfb;
						m_updn2 = 1;
						m_ballin++;
						logerror("Ball In Pocket m_num:%2x\n", m_num);
						return;
					}
				}
			}

			m_updn2 = 0;
			m_updn3 = 1;
		}

		if (!BIT3 && (m_updn3 == 1))
		{
			m_p30 |= 0x08;
			m_updn3 = 0;
			m_step++;

			if (m_step == 39)
			{
				m_step = 0;
				m_p30 |= 0x1c;
			}

			LOG("6:port_p30:-set bit 3 -%2x cont_pasos:%2d \n", m_p30, m_step);
			return;
		}
	}
}


/***********************
*    Ball Simulation   *
***********************/

TIMER_DEVICE_CALLBACK_MEMBER(rulechan_state::ball_speed)
{
	if (MOTORON)
	{
		if (d_spin == 0)
		{
			m_tspin++;
			d_spin = m_spin;

			if (BALLIN)
			{
				m_tspin = 37;   // breaking ball once per number step.
			}

			if (m_tspin == 37)
			{
				m_tspin = 0;
				m_spin++;
				d_spin = m_spin;   // breaking ball once per round.
			}

			if (!BALLIN || LEDNOTNUM)
			{
				m_led++;

				if (m_led == 37)
					m_led = 0;

				for (int i = 0; i < 37; i++)
					m_lamps[i + 20] = (m_led == i) ? 1 : 0;   // update roulette led lamps.

				sound_sort();
			}
		}
		else
		{
			d_spin--;     // burn cycles...
			sound_off();
		}
	}
	else
	{
		for (int i = 0; i < 37; i++)
			m_lamps[i + 20] = (m_num == i) ? 1 : 0;

		m_spin = d_spin = m_tspin = m_ballin = 0;
	}

	/* END of Ball simulation */

	/* if needed, get pass and shows it on layout*/
	m_pass[0] = m_maincpu->space(AS_PROGRAM).read_byte(RAM_PSW);

	if ((m_pass[0] <= 0x39) && (m_pass[0] >= 0x30))
	{
		for (int i = 0; i < 6; i++)
		{
			m_pass[i] = m_maincpu->space(AS_PROGRAM).read_byte(RAM_PSW + i);
			m_lamps[10 + i] = dec_7seg(m_pass[i] - 0x30);
		}
	}
	else
	{
		for (int i = 0; i < 6; i++)
			m_lamps[10 + i] = dec_7seg(0xff);
	}
}


/**************************************
*            Input Ports              *
**************************************/

static INPUT_PORTS_START( rulechan )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Key-In")  PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Key-Out") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P2 Key-In")  PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P2 Key-Out") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P3 Key-In")  PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P3 Key-Out") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P4 Key-In")  PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P4 Key-Out") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P5 Key-In")  PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P5 Key-Out") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_NAME("Bet")        PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_NAME("Down")       PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )  PORT_NAME("Up")         PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )  PORT_NAME("Left")       PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 )  PORT_NAME("Right")      PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED  )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P6 Key-In")  PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P6 Key-Out") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AUX")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_NAME("Tilt 1")    PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_NAME("Tilt 2")    PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_TOGGLE            PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM )  PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   // bit 6 is EEPROM data.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CODE(KEYCODE_9)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Borrado Total" )  PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Repite" )         PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Demo" )           PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Acceso" )         PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Test de Plato" )  PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Inmoviliza" )     PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Doble Cero" )     PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Castellano" )     PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)    // bit 3 is data (active high).
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)   // bit 4 is clock (active high).
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)    // bit 5 is cs.

INPUT_PORTS_END


/**************************************
*           Machine Driver            *
**************************************/

void rulechan_state::rulechan(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &rulechan_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &rulechan_state::main_io);
	m_maincpu->set_periodic_int(FUNC(rulechan_state::irq0_line_hold), attotime::from_hz(120));

	/* nvram */
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* eeprom */
	EEPROM_93C46_8BIT(config, "eeprom");

	TIMER(config, "ball_speed").configure_periodic(FUNC(rulechan_state::ball_speed), attotime::from_hz(60));
	TIMER(config, "wheel_speed").configure_periodic(FUNC(rulechan_state::wheel_speed), attotime::from_hz(60));

	i8255_device &ppi0(I8255(config, "ppi0"));
	ppi0.out_pa_callback().set(FUNC(rulechan_state::port0_w));   // Matrix scan line selector - Must read back as 0xf0 at power-up.
	ppi0.out_pb_callback().set_ioport("EEPROMOUT");              // EEPROM ok.
	ppi0.in_pc_callback().set(FUNC(rulechan_state::port2_r));    // Matrix button read.

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pa_callback().set(FUNC(rulechan_state::port30_r));
	ppi1.out_pb_callback().set(FUNC(rulechan_state::port31_w));  // Must read back as 0x00 at power-up.
	ppi1.out_pc_callback().set(FUNC(rulechan_state::port32_w));
	ppi1.tri_pc_callback().set_constant(0xf0);                   // Motor off at startup

	/* video hardware */
	v9938_device &v9938(V9938(config, "v9938", VID_CLOCK));
	v9938.set_screen_ntsc("screen");
	v9938.set_vram_size(VDP_MEM);
	//v9938.int_cb().set_inputline("maincpu", 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware   */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay_re900(AY8910(config, "ay8910", TMS_CLOCK));
	ay_re900.port_a_read_callback().set(FUNC(rulechan_state::psg_portA_r));
	ay_re900.port_b_read_callback().set(FUNC(rulechan_state::psg_portB_r));
	ay_re900.add_route(ALL_OUTPUTS, "mono", 0.5);
}


/**************************************
*              ROM Load               *
**************************************/

ROM_START( rulechan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eprom322", 0x00000, 0x10000, CRC(a78e106f) SHA1(23e25ca60296e8002df9b9cf16ad8fe8a1e6c93b) )

	ROM_REGION(0x80, "eeprom", 0)
	ROM_LOAD( "eeprom.322", 0x0000, 0x0080, CRC(ded905d2) SHA1(999e8e54a31a232092c4345434b93358226d1144) )
ROM_END

ROM_START( rulechab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eprom165", 0x00000, 0x10000, CRC(1669f266) SHA1(14e70b91e260e3bc338870936954e09c182fe591) )

	ROM_REGION(0x80, "eeprom", 0)
	ROM_LOAD( "eeprom.165", 0x0000, 0x0080, CRC(042ac516) SHA1(a32d82acb438cceda0f03b58be6977eeb589836c) )
ROM_END


/************************
*      Driver Init      *
************************/

void rulechan_state::rulechan_init()
{
	m_p30 = 0x3c;
	m_step = 0;
	m_updn2 = 0;
	m_updn3 = 0;
	m_updn4 = 0;
}

} // anonymous namespace


/**************************************
*           Game Driver(s)            *
**************************************/

//    YEAR  NAME      PARENT    MACHINE   INPUT     STATE           INIT           ROT    COMPANY          FULLNAME                            FLAGS                                       LAYOUT
GAMEL(1991, rulechan, 0,        rulechan, rulechan, rulechan_state, rulechan_init, ROT0, "ElectroChance", "Super Ball (Version EC-3.50 N322)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL, layout_rulechan)
GAMEL(1991, rulechab, 0,        rulechan, rulechan, rulechan_state, rulechan_init, ROT0, "ElectroChance", "Super Ball (Version EC-3.50 N165)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL, layout_rulechan)
