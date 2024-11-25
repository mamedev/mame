// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo.
/********************************************************************

  Lucky Ball 96.
  6-player LEDs Roulette.

  Copyright 1991/96 by Sielcon Games.
  Industria Argentina.

  Driver by Roberto Fresca & Grull Osgo.

  Special thanks to Daniel Gallimberti (CEO of Sielcon Games, programmer
  and owner of this roulette machine) for allow the emulation, the inclu-
  sion in MAME, and the release of this game for non-profit purposes. :-)


  6 sets dumped:
  Version: 4.01 - Final?
  Version: 3.50 - 616
  Version: 3.50 - 623
  Version: 3.50 - 626
  Version: 3.50 - 627
  Version: 3.01

  Each set has:
  1x 64K program ROM (unknown CPU).
  1x 512K sound ROM (unsigned 8-bit PCM @ 11025 KHz).


*********************************************************************

  PCB specs:

  Silkscreened: 'PCB Ver 1.5'
  Silkscreened: 'SIELCON GAMES'
  Silkscreened: 'Copyright 1996'
  Silkscreened: 'MADE IN ARGENTINA'

  1x Unknown sanded PLCC68 IC (identified as Zilog Z180).
  1x unknown sanded DIL24 IC labeled PLD-01.
  1x unknown sanded DIL28 IC labeled PLD-02 (identified as ST6265 MCU).
  1x unknown sanded DIL20 IC labeled PLD-03 @U21.
  1x unknown sanded DIL64 IC @U12 (identified as Yamaha 9938/58 VDP).

  1x PPI 8255 (@U10)
  3x 4099 (8-Bit Addressable Latch) @U1-U3-U5.
  3x 4512 (8-Channel Data Selector) @U2-U4-U7.

  1x INMOS B IMS1630K-70M (8K x 8 SRAM, DIL28)... NOTE: Replaced by UT6264CPC-70LL in another board.
  4x sanded IC's identified as 4464 (64K x 4 DRAM) @U16-U17-U18-U19, near the Yamaha 9938 for video RAM.

  1x 27C512   (program ROM).
  1x M27C4001 (GFX + sound samples).
  1x 24C04 serial EEPROM (missing).

  1x unknown DAC (DIL16) @U13.
  1x LM324 (Single Supply Quad Operational Amplifier)

  1x Maxim MAX691CPE (battery supervisor).

  1x 21.47727 MHz. crystal, near the Yamaha 9938/58 VDP.
  1x 12.288 MHz. crystal, near the Zilog Z180.
  1x 8.000 MHz. sanded crystal near the ST6265 MCU (measured).

  1x 8 DIP Switches bank.
  1x pushbutton silkscreened 'PAGE'.
  1x pushbutton silkscreened 'RESET'.

  1x 3.6 V. battery.

  1x 2x10 blind connector silkscreened 'EXPANSION PORT'.
  1x 2x17 blind connector silkscreened 'EXPANSION SLOT'.
  1x 6-pin connector silkscreened 'NET'.
  1x RS232 connector silkscreened 'RS-232'.
  1x 2x28 JAMMA type connector.


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
  | Acceso a programación        No       | OFF |                                         |
  | (settings access)            Si (yes) | ON  |                                         |
  +---------------------------------------+-----+-----+-----------------------------------+
  | Seteo del Pozo               No       |     | OFF |                                   |
  | (Jackpot settings)           Si (yes) |     | ON  |                                   |
  +---------------------------------------+-----+-----+-----+-----------------------------+
  | Reset de Audit. Parciales    No       |           | OFF |                             |
  | (Reset short term meters)    Si (yes) |           | ON  |                             |
  +---------------------------------------+-----------+-----+-----+-----------------------+
  | Prog. de Pozo acumulado      No       |                 | OFF |                       |
  | (Jackpot program)            Si (yes) |                 | ON  |                       |
  +---------------------------------------+-----------------+-----+-----+-----------------+
  | Prog. de multiplicadores     No       |                       | OFF |                 |
  | (Multipliers program)        Si (yes) |                       | ON  |                 |
  +---------------------------------------+-----------------------+-----+-----+-----+-----+
  | Not Used                              |                             | OFF | OFF |     |
  |                                       |                             | ON  | ON  |     |
  +---------------------------------------+-----------------------------+-----+-----+-----+
  | Reset total                  No       |                                         | OFF |
  | (Full Reset)                 Si (yes) |                                         | ON  |
  '---------------------------------------+-----------------------------------------+-----'


*********************************************************************

  Program ROM
  Data Lines Scrambling.

  BUS    ROM

  D0 ---> D1
  D1 ---> D0
  D2 ---> D3
  D3 ---> D2
  D4 ---> D5
  D5 ---> D4
  D6 ---> D7
  D7 ---> D6


*********************************************************************

  Notes:

  - Programming mode is driven through POS#5 controls (joystick + bet button)

  - The roulette numbers lamps multiplexion & animation seems to be drive from
    the DIL28 IC (labeled PLD-02).

  - Identified the PLCC68 CPU as Zilog Z180.
  - Identified the DIL28 IC as ST6265 MCU.
  - Identified the DIL64 IC as Yamaha 9938/58 VDP.


*********************************************************************

  Media files (27c4001)

  00000-085ff    GFX.
  08600-0ffff    Blank.
  10000-7ffff    Samples.

  Samples are 8-bit unsigned PCM.


*********************************************************************

  I/O:

  1x PPI 8255.
  3x 4099 (8-Bit Addressable Latch).
  3x 4512 (8-Channel Data Selector).


                  PPI 8255
                .-----v-----.
  DAC (D7) <----| PA0   PB0 |----> I/O 4099 (A0) & 4512 (A)
  DAC (D6) <----| PA1   PB1 |----> I/O 4099 (A1) & 4512 (B)
  DAC (D5) <----| PA2   PB2 |----> I/O 4099 (A2) & 4512 (C)
  DAC (D4) <----| PA3   PB3 |----> I/O 4099 (WD 4099 #0, Players)
  DAC (D3) <----| PA4   PB4 |----> I/O 4099 (WD 4099 #1, Counters)
  DAC (D2) <----| PA5   PB5 |----> I/O 4099 (WD 4099 #2, unknown)
  DAC (D1) <----| PA6   PB6 |----> I/O 4099 (D)
  DAC (D0) <----| PA7   PB7 |----> PB7
                |           |
                |       PC0 |----> EEPROM (DI)
                |       PC1 |----> EEPROM (CS)
                |       PC2 |----> EEPROM (SK)
                |       PC3 |----\ tied together
                |       PC4 |----/ to PC3.
                |       PC5 |----> I/O 4512 #0 (SO, inputs #0)
                |       PC6 |----> I/O 4512 #1 (SO, inputs #1)
                |       PC7 |----> I/O 4512 #2 (SO, DIP switches)
                '-----------'


  Inputs...

    4512 #0                     4512 #1                           4512 #2
  .----v----.                 .----v----.                       .----v----.
  |      D0 |----> AUX IN     |      D0 |----> CREDITS IN       |      D0 |----> DSW #1
  |      D1 |----> N/C        |      D1 |----> PLAYER UP        |      D1 |----> DSW #2
  |      D2 |----> N/C        |      D2 |----> PLAYER BET       |      D2 |----> DSW #3
  |      D3 |----> N/C        |      D3 |----> CREDITS OUT      |      D3 |----> DSW #4
  |      D4 |----> TOUCH      |      D4 |----> PLAYER RIGHT     |      D4 |----> DSW #5
  |      D5 |----> N/C        |      D5 |----> PLAYER DOWN      |      D5 |----> DSW #6
  |      D6 |----> CREDIT     |      D6 |----> COIN (???)       |      D6 |----> DSW #7
  |      D7 |----> PAGE       |      D7 |----> PLAYER LEFT      |      D7 |----> DSW #8
  '---------'                 '---------'                       '---------'


  Outputs...

    4099 #0                     4099 #1                                   4099 #2
  .----v----.                 .----v----.                               .----v----.
  |      Q0 |----> PL. 1      |      Q0 |----> AUX OUT1                 |      Q0 |----> N/C
  |      Q1 |----> PL. 2      |      Q1 |----> AUX OUT2  (ULN2004)      |      Q1 |----> N/C
  |      Q2 |----> PL. 3      |      Q2 |----> AUX OUT3  (ULN2004)      |      Q2 |----> N/C
  |      Q3 |----> PL. 4      |      Q3 |----> AUX OUT4  (ULN2004)      |      Q3 |----> N/C
  |      Q4 |----> PL. 5      |      Q4 |----> COUNTER AUX1 (ULN2004)   |      Q4 |----> TOUCH (through 1N60)
  |      Q5 |----> PL. 6      |      Q5 |----> COUNTER IN   (ULN2004)   |      Q5 |----> N/C
  |      Q6 |----> PL. 7      |      Q6 |----> COUNTER AUX2 (ULN2004)   |      Q6 |----> N/C
  |      Q7 |----> PL. 8      |      Q7 |----> COUNTER OUT  (ULN2004)   |      Q7 |----> N/C
  '---------'                 '---------'                               '---------'


  TOUCH = For iButton implementation. Not present in the current PCBs.


*********************************************************************

  Dev notes:

  Not just the ROM, but most external read/write accesses may have
  even and odd data lines swapped. The frequently-called subroutine
  at $4A0D performs this swapping, and the PPI needs it for
  initialization (otherwise the invalid control word $44 gets
  written and the program keeps resetting when it can't read the
  outputs back).


*********************************************************************/


#define CPU_CLOCK       XTAL(12'000'000)    // 12MHz. from schematics.
#define MCU_CLOCK       XTAL(8'000'000)
#define VID_CLOCK       XTAL(21'477'272)

#define VDP_MEM         0x20000  // 4x 4464 (64K x 4 DRAM)


#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/nvram.h"
#include "machine/74259.h"
#include "machine/i8255.h"
#include "video/v9938.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"
#include "luckybal.lh"


namespace {

class luckybal_state : public driver_device
{
public:
	luckybal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_v9938(*this, "v9938")
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_dac(*this, "dac")
		, m_aux(*this, "AUX")
		, m_dsw(*this, "DSW")
		, m_latch(*this, "latch%u", 1)
		, m_keymx(*this, "IN%u", 0)
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void luckybal(machine_config &config);

	void init_luckybal();
	void init_luckybala();
	void init_luckybald();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void csio_cks_w(int state);
	void port90_bitswap_w(uint8_t data);
	uint8_t ppi_bitswap_r(offs_t offset);
	void ppi_bitswap_w(offs_t offset, uint8_t data);
	void output_port_a_w(uint8_t data);
	void output_port_b_w(uint8_t data);
	uint8_t input_port_c_r();
	void output_port_c_w(uint8_t data);

	uint8_t m_csio_in = 0;
	uint16_t m_csio_out = 0;
	uint8_t m_csio_txs = 0;
	uint8_t m_csio_cnt = 0;
	uint8_t m_led_on = 0;

	required_device<v9938_device> m_v9938;
	required_device<z180_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<dac_byte_interface> m_dac;
	required_ioport m_aux;
	required_ioport m_dsw;
	required_device_array<cd4099_device, 3> m_latch;
	required_ioport_array<6> m_keymx;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	output_finder<38> m_lamps;
};


void luckybal_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_csio_in));
	save_item(NAME(m_csio_out));
	save_item(NAME(m_csio_txs));
	save_item(NAME(m_csio_cnt));
	save_item(NAME(m_led_on));
}

void luckybal_state::machine_reset()
{
	m_csio_in = 0;
	m_csio_out = 0;
	m_csio_txs = 0;
	m_csio_cnt = 0;
}


/**************************************
*             Memory Map              *
**************************************/

void luckybal_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).bankr("bank1");  // Banked ROM.
	map(0xe000, 0xffff).ram().share("nvram");  // 6264 SRAM
}

void luckybal_state::main_io(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x3f).nopr().nopw();  // Z180 Internal registers.

	map(0x90, 0x90).w(FUNC(luckybal_state::port90_bitswap_w));
	map(0xc0, 0xc3).rw(FUNC(luckybal_state::ppi_bitswap_r), FUNC(luckybal_state::ppi_bitswap_w));
	map(0xe0, 0xe3).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));
}
/*
;*********** PPI 8255 *******
PORTCN    EQU  C0H    ;80H C
P_AUDIO   EQU  C0H    ;Port A ---> DAC
PORTIN    EQU  C2H    ;Port C (High nibble) Inputs. (4=EE, 5=PLATE, 6=KEY, 7=DIP)
PORT_EE   EQU  C2H    ;Port C (Low nibble) EEPROM.  (0=DI, 1=CS, 2=SK)
PORTIO    EQU  C1H    ;Port B I/O 4099 (0=A0, 1=A1, 2=A2, 3=WJ, 4=WC, 5=WP, 6=D, 7=LED)
PPI_CTRL  EQU  C3H    ;Mode.

;---VDP
PORT0     EQU  E0H
PORT1     EQU  E1H
PORT2     EQU  E2H
PORT3     EQU  E3H

;******* 74LS273 MEMORY MAPPED **********
M_MAP     EQU  90H    ; [A]= Bank to select (BIT6=MEM, BIT7=EN_NMI)

*/


/**************************************
*            R/W handlers             *
**************************************/

void luckybal_state::csio_cks_w(int state)
{
	if (!state)
	{
		m_maincpu->rxs_cts1_w(BIT(m_csio_out, 0));
		m_csio_out >>= 1;
	}
	else
	{
		m_csio_in >>= 1;
		m_csio_in |= m_csio_txs << 7;

		if (++m_csio_cnt == 8)
		{
			m_csio_cnt = 0;
			const uint8_t csio_data = m_csio_in & 0x7f;
			if (csio_data <= 36)
			{
				m_lamps[m_led_on] = 0;
				m_lamps[csio_data] = 1;
				m_led_on = csio_data;
			}

			// read back the previously written value
			m_csio_out |= m_csio_in << 8;
		}
	}
}

void luckybal_state::port90_bitswap_w(uint8_t data)
{
	data = bitswap<8>(data, 6, 7, 4, 5, 2, 3, 0, 1);
	membank("bank1")->set_entry(data & 0x3f);
}

uint8_t luckybal_state::ppi_bitswap_r(offs_t offset)
{
	return bitswap<8>(m_ppi->read(offset), 6, 7, 4, 5, 2, 3, 0, 1);
}

void luckybal_state::ppi_bitswap_w(offs_t offset, uint8_t data)
{
	m_ppi->write(offset, bitswap<8>(data, 6, 7, 4, 5, 2, 3, 0, 1));
}

void luckybal_state::output_port_a_w(uint8_t data)
{
	m_dac->write(data);
}

void luckybal_state::output_port_b_w(uint8_t data)
{
	for (int n = 0; n < 3; n++)
		if (!BIT(data, n + 3))
			m_latch[n]->write_bit(data & 7, BIT(data, 6));
}

uint8_t luckybal_state::input_port_c_r()
{
	uint8_t mux_player, sel_line, bit5, bit6, bit7, ret;
	sel_line = m_ppi->pb_r() & 0x7f;
	mux_player = m_latch[0]->output_state();

	bit5 = BIT(m_aux->read(), sel_line & 0x07) ? 0xff : 0xdf;                   // Operator & Page.

	bit6 = 0xff;
	for (int i = 0; 6 > i; ++i)
	{
		if (!BIT(mux_player, i) && !BIT(m_keymx[i]->read(), sel_line & 0x07))   // Player buttons.
			bit6 &= 0xbf;
	}

	bit7 = BIT(m_dsw->read(), sel_line & 0x07) ? 0xff : 0x7f;                   // Dip Switch.

	if ((sel_line & 0x07) == 6)  m_lamps[37] = (bit5 == 0xff) ? 0 : 1;          // Operator lamp.
	ret = bit7 & bit6 & bit5;

	return ret;
}

void luckybal_state::output_port_c_w(uint8_t data)
{
/*  Writes 0xF0/0xF1 constantly at the beginning... like a watchdog.
    After a while, just stop (when roulette LEDs are transmitted).
*/
}


/**************************************
*            Input Ports              *
**************************************/

static INPUT_PORTS_START( luckybal )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_1)        PORT_NAME("Player 1 - Credits IN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)  PORT_CODE(KEYCODE_W)        PORT_NAME("Player 1 - Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )                        PORT_CODE(KEYCODE_Q)        PORT_NAME("Player 1 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_Z)        PORT_NAME("Player 1 - Credits OUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)  PORT_CODE(KEYCODE_D)        PORT_NAME("Player 1 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)  PORT_CODE(KEYCODE_S)        PORT_NAME("Player 1 - Down")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // 'Player 1 - Coins' in the schematics. Maybe for another game.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)  PORT_CODE(KEYCODE_A)        PORT_NAME("Player 1 - Left")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_2)        PORT_NAME("Player 2 - Credits IN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)  PORT_CODE(KEYCODE_T)        PORT_NAME("Player 2 - Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )                        PORT_CODE(KEYCODE_R)        PORT_NAME("Player 2 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_X)        PORT_NAME("Player 2 - Credits OUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)  PORT_CODE(KEYCODE_H)        PORT_NAME("Player 2 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)  PORT_CODE(KEYCODE_G)        PORT_NAME("Player 2 - Down")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // 'Player 2 - Coins' in the schematics. Maybe for another game.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)  PORT_CODE(KEYCODE_F)        PORT_NAME("Player 2 - Left")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_3)        PORT_NAME("Player 3 - Credits IN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(3)  PORT_CODE(KEYCODE_S)        PORT_NAME("Player 3 - Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )                        PORT_CODE(KEYCODE_U)        PORT_NAME("Player 3 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_C)        PORT_NAME("Player 3 - Credits OUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)  PORT_CODE(KEYCODE_L)        PORT_NAME("Player 3 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(3)  PORT_CODE(KEYCODE_K)        PORT_NAME("Player 3 - Down")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // 'Player 3 - Coins' in the schematics. Maybe for another game.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(3)  PORT_CODE(KEYCODE_J)        PORT_NAME("Player 3 - Left")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_4)        PORT_NAME("Player 4 - Credits IN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(4)  PORT_CODE(KEYCODE_PGUP)     PORT_NAME("Player 4 - Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 )                        PORT_CODE(KEYCODE_LALT)     PORT_NAME("Player 4 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_V)        PORT_NAME("Player 4 - Credits OUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)  PORT_CODE(KEYCODE_7)        PORT_NAME("Player 4 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(4)  PORT_CODE(KEYCODE_PGDN)     PORT_NAME("Player 4 - Down")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // 'Player 4 - Coins' in the schematics. Maybe for another game.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(4)  PORT_CODE(KEYCODE_8)        PORT_NAME("Player 4 - Left")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_5)        PORT_NAME("Player 5 - Credits IN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(5)  PORT_CODE(KEYCODE_DOWN)     PORT_NAME("Player 5 - Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 )                        PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Player 5 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_B)        PORT_NAME("Player 5 - Credits OUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(5)  PORT_CODE(KEYCODE_LEFT)     PORT_NAME("Player 5 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(5)  PORT_CODE(KEYCODE_UP)       PORT_NAME("Player 5 - Down")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // 'Player 5 - Coins' in the schematics. Maybe for another game.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(5)  PORT_CODE(KEYCODE_RIGHT)    PORT_NAME("Player 5 - Left")

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_6)        PORT_NAME("Player 6 - Credits IN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(6)  PORT_CODE(KEYCODE_2_PAD)    PORT_NAME("Player 6 - Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 )                        PORT_CODE(KEYCODE_7_PAD)    PORT_NAME("Player 6 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )                        PORT_CODE(KEYCODE_N)        PORT_NAME("Player 6 - Credits OUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(6)  PORT_CODE(KEYCODE_4_PAD)    PORT_NAME("Player 6 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(6)  PORT_CODE(KEYCODE_8_PAD)    PORT_NAME("Player 6 - Down")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // 'Player 6 - Coins' in the schematics. Maybe for another game.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(6)  PORT_CODE(KEYCODE_6_PAD)    PORT_NAME("Player 6 - Left")

	PORT_START("AUX")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0)  PORT_NAME("Operator Key")  PORT_TOGGLE  // Allow to credit in/out for all players.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9)  PORT_NAME("Test Mode / Books / Page")   // Need the Oper Key active to work.

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Setting Access" )        PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Jackpot Enable" )        PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Partial Books Clear" )   PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Jackpot Mode" )          PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage / Bet Mode" )    PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown - 0x20" )        PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown - 0x40" )        PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Books Clear" )      PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/**************************************
*           Machine Driver            *
**************************************/

void luckybal_state::luckybal(machine_config &config)
{
	// basic machine hardware
	Z80180(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &luckybal_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &luckybal_state::main_io);
	m_maincpu->cks_wr_callback().set(FUNC(luckybal_state::csio_cks_w));
	m_maincpu->txs_wr_callback().set([this] (int state) { m_csio_txs = state; });

	I8255A(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(luckybal_state::output_port_a_w));
	m_ppi->out_pb_callback().set(FUNC(luckybal_state::output_port_b_w));
	m_ppi->in_pc_callback().set(FUNC(luckybal_state::input_port_c_r));
	m_ppi->out_pc_callback().set(FUNC(luckybal_state::output_port_c_w));

	CD4099(config, "latch1", 0);

	CD4099(config, "latch2", 0);

	CD4099(config, "latch3", 0);

	// nvram
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	v9938_device &v9938(V9938(config, "v9938", VID_CLOCK));
	v9938.set_screen_ntsc("screen");
	v9938.set_vram_size(VDP_MEM);
	v9938.int_cb().set_inputline("maincpu", 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC08(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
}


/**************************************
*              ROM Load               *
**************************************/

ROM_START( luckybal )  // luckyball96 v4.01
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "lb97_27c512_d2a1.u15", 0x00000, 0x10000, CRC(9bc6d417) SHA1(d414b389dca8dbebaac23c7f7c1338f57aa95c40) )
	ROM_LOAD( "lb97_27c4001_af7c.u20", 0x10000, 0x80000, CRC(de796370) SHA1(6edee4de330a65ad6b2dbe59c7ac0ddecb3ed0f1) )
ROM_END

ROM_START( luckybala )  // luckyball96 v350-627
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "lb97_627_m27c512.u15", 0x00000, 0x10000, CRC(c1bcffef) SHA1(da5db0ab0555cd98ff8e0206c1ee4ebd3d7447ef) )
	ROM_LOAD( "lb97_627_m27c4001.u20", 0x10000, 0x80000, CRC(dbc45c4a) SHA1(720c6861fa2bfa9c9dad69d687f12bd1e0a71afb) )
ROM_END

ROM_START( luckybalb )  // luckyball96 v350-626
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "lb97_626_m27c512.u15", 0x00000, 0x10000, CRC(d25588c1) SHA1(fc24b1e869d726d2e2ab8cd38ab6304fdca6dfa9) )
	ROM_LOAD( "lb96_sounds_001_m27c4001.u20", 0x10000, 0x80000, CRC(dbc45c4a) SHA1(720c6861fa2bfa9c9dad69d687f12bd1e0a71afb) )
ROM_END

ROM_START( luckybalc )  // luckyball96 v350-623
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "lb96_625_m27c512.u15", 0x00000, 0x10000, CRC(2017edf7) SHA1(0208423b116aeb139c5db193b567bc79fd2a21ac) )
	ROM_LOAD( "lb96_625_m27c4001.u20", 0x10000, 0x80000, CRC(dbc45c4a) SHA1(720c6861fa2bfa9c9dad69d687f12bd1e0a71afb) )
ROM_END

ROM_START( luckybald )  // luckyball96 v350-616
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "nosticker_a18e_m27c512.u15", 0x00000, 0x10000, CRC(9bdf0243) SHA1(e353a86c4b020784d084c4fa12feb6ccd8ebd77b) )
	ROM_LOAD( "nosticker_af7c_m27c4001.u20", 0x10000, 0x80000, CRC(de796370) SHA1(6edee4de330a65ad6b2dbe59c7ac0ddecb3ed0f1) )
ROM_END

ROM_START( luckybale )  // luckyball96 v3.01
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "lb97_6235_f12b.u15", 0x00000, 0x10000, CRC(2d2837c0) SHA1(d42718f81fc46ed753665ba12bd4e0a8a75bff93) )
	ROM_LOAD( "lb97_6235_ac9c.u20", 0x10000, 0x80000, CRC(dbc45c4a) SHA1(720c6861fa2bfa9c9dad69d687f12bd1e0a71afb) )
ROM_END


/************************************
*            Driver Init            *
************************************/

void luckybal_state::init_luckybal()
{
	uint8_t *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0;
	for (int i = start; i < size; i++)
	{
		rom[i] = bitswap<8>(rom[i], 6, 7, 4, 5, 2, 3, 0, 1);
	}

	membank("bank1")->configure_entries(0, 0x40, &rom[0x10000], 0x2000);
}

void luckybal_state::init_luckybala()
{
	uint8_t *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0;

	for (int i = start; i < size; i++)
	{
		rom[i] = bitswap<8>(rom[i], 6, 7, 4, 5, 2, 3, 0, 1);
	}

	/* The following patches are to avoid hardware verifications through
	   the unemulated synchronic serial comm of the z180...
	*/
	rom[0x571] = 0x68;  //31
	rom[0x572] = 0xE8;  //4C
	rom[0x573] = 0x18;  //42
	rom[0x574] = 0x98;  //39
	rom[0x575] = 0x58;  //36
	rom[0x16E1] = 0x0D; //OC
	rom[0x1D65] = 0x0E; //0C
	rom[0x4499] = 0x00; //FF    <------- Checksum.
	rom[0x4AB6] = 0xAF; //B9

	membank("bank1")->configure_entries(0, 0x40, &rom[0x10000], 0x2000);
}

void luckybal_state::init_luckybald()
{
	uint8_t *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0;

	for (int i = start; i < size; i++)
	{
		rom[i] = bitswap<8>(rom[i], 6, 7, 4, 5, 2, 3, 0, 1);
	}

	/* The following patches are to avoid hardware verifications through
	   the unemulated synchronic serial comm of the z180...
	*/
	rom[0x571] = 0x68;  //31
	rom[0x572] = 0xE8;  //4C
	rom[0x573] = 0x18;  //42
	rom[0x574] = 0x98;  //39
	rom[0x575] = 0x58;  //36
	rom[0x16C1] = 0x0D; //OC
	rom[0x1D45] = 0x0E; //0C
	rom[0x44A9] = 0x00; //FF    <------- Checksum.
	rom[0x4AC6] = 0xAF; //B9

	membank("bank1")->configure_entries(0, 0x40, &rom[0x10000], 0x2000);
}

} // anonymous namespace


/**************************************
*           Game Driver(s)            *
**************************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT            ROT    COMPANY          FULLNAME                         FLAGS                  LAYOUT
GAMEL( 1999, luckybal,  0,        luckybal, luckybal, luckybal_state, init_luckybal,  ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 4.01)"      , MACHINE_SUPPORTS_SAVE, layout_luckybal)
GAMEL( 1996, luckybala, luckybal, luckybal, luckybal, luckybal_state, init_luckybala, ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.50 - 627)", MACHINE_SUPPORTS_SAVE, layout_luckybal)
GAMEL( 1996, luckybalb, luckybal, luckybal, luckybal, luckybal_state, init_luckybala, ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.50 - 626)", MACHINE_SUPPORTS_SAVE, layout_luckybal)
GAMEL( 1996, luckybalc, luckybal, luckybal, luckybal, luckybal_state, init_luckybala, ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.50 - 623)", MACHINE_SUPPORTS_SAVE, layout_luckybal)
GAMEL( 1996, luckybald, luckybal, luckybal, luckybal, luckybal_state, init_luckybald, ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.50 - 616)", MACHINE_SUPPORTS_SAVE, layout_luckybal)
GAMEL( 1996, luckybale, luckybal, luckybal, luckybal, luckybal_state, init_luckybal,  ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.01)"      , MACHINE_SUPPORTS_SAVE, layout_luckybal)
