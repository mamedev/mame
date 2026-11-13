// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/****************************************************************************

    TourVisión
    Driver by Mariusz Wojcieszek and Stephh

    Bootleg PC-Engine based arcade board from the Spanish company TourVisión.
    Pay-for-time system with four cartridge slots: the player inserts coins
    to buy play time, chooses one of the four games with the joystick and
    the system starts it, keeping track of the remaining time.

    The cartridges are PC Engine HuCards sealed in a bigger case, with eight
    extra edge connector pins giving each game an ID. The 8085 BIOS uses the
    ID to look up how to handle the game: number of players, the button
    sequence needed to start it (skipping option menus and intros), what the
    start button does, whether Select is allowed and how to stop the game when
    the time runs out.

    Known hardware revisions:
    - V1: a stripped PC Engine board on a motherboard with four plain HuCard
      slots (no game ID), with the display and lamp drivers on the motherboard.
      Runs BIOS VT-1.1 / VT-2.0.
    - V4: a stripped PC Engine board on a sub-board, cartridges with game ID,
      display and lamp drivers moved to external boards (marquee and control
      panel). BIOS 4.0 to 6.0.
    - V4 integrated: same as V4 with the PC Engine chipset on the motherboard.

    Known games list can be found in hash/pce_tourvision.xml.

    More information:
    https://blog.system11.org/?p=1785
    https://blog.system11.org/?p=1943
    https://blog.system11.org/?p=2918
    https://blog.system11.org/?p=2960

    TODO:
    - V1: controls lockout and Run press deduced from the BIOS and the
      system11 description, JAMMA inputs at $b000 bits 4-7 unknown (they
      start the selected game), single joypad?
    - 8155 timer clock is derived from the BIOS timings (the play time counts
      seconds at 100 ticks per second), not measured
    - power-fail TRAP and battery-backed RAM (the BIOS only keeps the RAM
      contents if the TRAP handler ran before powering off)
    - watchdog at $d000

 _______________________________________________________________________________________________________________________________________________
|                                                                                                                                               |
|                                           ____________               ____________               ____________               ____________       |
|                                          |T74LS125AB1|  ____        |T74LS125AB1|  ____        |T74LS125AB1|  ____        |T74LS125AB1|  ____ |
|                                          -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|                                           ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|                                          | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|                                          -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|                                           ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|       ___________   ____________         | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|      |4116R-001  | |X74LS32B1  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|      ------------- -------------                        |JP|                       |JP|                       |JP|                       |JP| |
|       ___________                                       | 4|                       | 3|                       | 2|                       | 1| |
|__     ___________                         ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
   |    ____________  ____________         | 74LS244N  |  |  |        | 74LS244N  |  |  |        | 74LS244N  |  |  |        | 74LS244N  |  |  | |
 __|   |SN74LS257SN| |4116R-001  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|__    ------------- -------------          ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|__     ____________  ____________         | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|__    | 74LS157N  | | 74LS157N  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|__    ------------- -------------          ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|__     ____________  ____________         | 74LS244N  |  ----        | 74LS244N  |  ----        | 74LS244N  |  ----        | 74LS244N  |  ---- |
|__    | SN74LS08N | | SN74LS08N |         -------------              -------------              -------------              -------------       |
|__    ------------- -------------                                                                                                              |
|__     ____________  ____________                                                                                            ____________      |
|__    | SN74LS08N | | SN74LS08N |                                                                                            |HSRM2264LM10     |
|__    ------------- -------------                                                                                            |__________|      |
|__     ____________  ____________                                                                                                              |
|__    | 74LS138N  | | 74LS138N  |                                                                                                              |
|__    ------------- -------------                                                                                                              |
|__     ____________  ____________  ____________   ____________   ____________  ____________  ____________                                      |
|__    |  74LS244N | |  74LS244N |  |T74LS32B1 |   |MC14017BCP|  | T74LS14B1 | | GD74LS393 | |T74LS125AB1|                                      |
|__    ------------- -------------  ------------   ------------  ------------- ------------  -------------                                      |
|__      .........     .........    ____________                                                                                                |
|__      .........     .........    | 74LS138N |                     _________                                                                  |
|__     ___________   ___________   ------------                     |       |                                      ______                      |
|__    |4116R-001  | |4116R-001  |  _________________                |       |                                     | HU  |        HSRM20256LM12 |
|__    ------------- ------------- |                |                |  BT1  |                                     |C6270|          ___         |
|__      __________  ____________  | NEC D4465C     |                |       |                                     |     |         |  |         |
|__     | TC4011BP|  |SN74LS373N | |________________|                |_______|                                     |_____|         |  |         |
|__     -----------  -------------  _________________                                                                              |__|         |
|__    ..... ___________     ____  | TOURVISION BIOS|                                                                                           |
|__         | 74LS138N |     |XT1| |                |                                                     ______     ______          ___        |
   |        ------------     |___| |________________|               ____                                 | HU  |    | HU  |         |  |        |
 __|    IC_  _____________                   ________              |    |                                |C6280A    |C6260A         |  |        |
|      |36| |  74LS244N  |  _________________________              | PT |                                |     |    |     |         |__|        |
|      ---- -------------  |                        |              |____|                                |_____|    |_____|                     |
|  ________  _____________ | NEC D8085AHC           |    ___________   ___________   _______                                       HSRM20256LM12|
| | DIP 2 | |  74LS244N  | |________________________|   | T74LS14B1|  |MC14001BCP|  |LM393N|                          ________                  |
| --------- --------------  _________________________   ------------  ------------  --------                         |D74HCU04C                 |
|  ________  _____________ |                        |    ___________                                                 ---------                  |
|  ________ |  74LS244N  | | NEC D8155HC            |   |  7407N   |                                             ____                           |
| | DIP 1 | -------------- |________________________|   ------------                                             |XT2|                          |
| ---------          ____                _____________         _____________                                     |___|                          |
|                    JP107              | JP 106     |        | JP 105     |                                                                    |
|                    ----               --------------        --------------                                                                    |
|_______________________________________________________________________________________________________________________________________________|

IC36  = ST NE 555N 99201
XT1   = 6144 KSS1H
JP107 = 2-pin connector
JP106 = 14-pin connector to the control panel play time display
JP105 = 16-pin connector to the marquee board (game selection lamps, score counters and play time display)
PT    = Push-type switch
BT1   = 3.6 V battery
XT2   = 21.32825 MHz UNI 90-H
JP1-4 = Carts slots

Games are dumped directly from the cartridge edge connector using the following adapter:

 ----------------------------------------------------------------------------
 Cartridge pinout
 ----------------------------------------------------------------------------

                       +----------+
                (N.C.) |50      49| +5V
                   +5V |48      47| +5V
                   A18 |46      45| +5V
                   A14 |44      43| A17
                    A8 |42      41| A13
                   A11 |40      39| A9
                   A10 |38      37| OE#
                    D7 |36      35| CE#
(front of           D5 |34      33| D6               (rear of
 cartridge)         D3 |32      31| D4                cartridge)
                    D2 |30      29| GND
                    D0 |28      27| D1
                    A1 |26      25| A0
                    A3 |24      23| A2
                    A5 |22      21| A4
                    A7 |20      19| A6
                   A15 |18      17| A12
                   A19 |16      15| A16
                   GND |14      13| (N.C.)
                   GND |12      11| GND)
                 (KEY) |10------09| (KEY)
                   ID7 |08      07| ID6
                   ID5 |06      05| ID4
                   ID3 |04      03| ID2
                   ID1 |02      01| ID0
                       +----------+

 ----------------------------------------------------------------------------
 27C080 pinout
 ----------------------------------------------------------------------------
                        +----v----+
                    A19 | 1     32| +5V
                    A16 | 2     31| A18
                    A15 | 3     30| A17
                    A12 | 4     29| A14
                     A7 | 5     28| A13
                     A6 | 6     27| A8
                     A5 | 7     26| A9
                     A4 | 8     25| A11
                     A3 | 9     24| OE#
                     A2 |10     23| A10
                     A1 |11     22| CE#
                     A0 |12     21| D7
                     D0 |13     20| D6
                     D1 |14     19| D5
                     D2 |15     18| D4
                    GND |16     17| D3
                        +---------+


Stephh notes for 8085 code (6.0 BIOS):
0xe01d : game slot number (range 0-3) - sometimes inc'ed/dec'ed/zeroed, but also filled based on games slot status (code at 0x01e8) :
0xe005 : play time in seconds, converted to BCD at 0x8009 and 0x800a (LSB first) for the displays
0xe030 to 0xe037 : score (bonus time) of each slot

0xe054 to 0xe057 : display timer (main, LSdigit first, stored in 4 lower bits)
0xe058 to 0xe05b : display score (game slot 1, LSdigit first, stored in 4 upper bits)
0xe05c to 0xe05f : display score (game slot 2, LSdigit first, stored in 4 lower bits)
0xe060 to 0xe063 : display score (game slot 3, LSdigit first, stored in 4 upper bits)
0xe064 to 0xe067 : display score (game slot 4, LSdigit first, stored in 4 lower bits)

display timer (main) "filled" with code at 0x054e
display score (game slot n) "filled" with code at 0x04e3

coin insertion routine at 0x0273
coin 1 triggers code at 0x02d7
coin 2 triggers code at 0x028f

in each coin insertion routine, you need to insert n coins (based on DSW settings) then you are awarded u seconds of play time (also based on DSW settings)

****************************************************************************

    V4 hardware, as used by the BIOS:

    8085 memory map:
    0000-7fff  BIOS
    8000-80ff  8155 RAM
    8100-8107  8155 I/O
    9000       DIP switches A
    a000       DIP switches B
    b000       JAMMA inputs (active low): P1 start, P2 start, P1 left, P1 right
    c000       coins (active low), slots 1 to 4 empty (bits 4 to 7)
    d000       watchdog?
    e000-e1ff  RAM (battery backed)
    f000       game ID of the selected cartridge

    8155 port A:
    0-2  button pressed by the BIOS on the PC Engine joypads:
         Run, I, II, Select, Up, Right, Down, Left
    3    set when pressing a button on joypad 2
    4    press the button on joypad 1
    5    press the button on joypad 2 (active low)
    6    player 1 controls lockout
    7    JAMMA button 3 (Select) lockout

    8155 port B:
    0-1  selected slot
    2    always set (cartridge buffers enable?)
    3    player 2 controls lockout
    4    PC Engine audio enable (muted during attract mode by DIP switch)
    5    PC Engine reset (active low)
    6-7  coin counters

    8155 port C drives the external displays through shift registers:
    0    serial data, score counters (8155 RAM $8002/$8003, MSB first)
    1    shift clock
    3    serial data, play time and lamps (8155 RAM $8000/$8001, LSB first)
    4    latch
    Every 8155 timer interrupt sends one digit of each display:
    - time display: BCD digit and digit strobe
    - score counters: one BCD digit for each slot, using a second digit strobe
    - game selection lamps
    The displays use 74LS247 decoders (BCD 15 is blank).

    8085 interrupts:
    TRAP     power fail (saves the RAM checksum)
    RST 5.5  8155 timer output (the BIOS restarts the timer on each interrupt)
    RST 6.5  bookkeeping (only polled through RIM)
    RST 7.5  clears the play time and the score of the selected slot (JP107?)
    SID      'PT' push switch, clears the RAM on power up when held

    The PC Engine Run button isn't wired to the JAMMA connector: the start
    buttons are read by the BIOS, which then presses whatever the game needs.
    There are two joypad ports, read like a PC Engine multitap.

    BIOS game data tables (6.0 offsets, indexed by game ID):
    1184  players: 00 = one, 55 = two alternating, ff = two simultaneous
    1284  start button: 00 = resets the game, ff = runs the P1/P2 start sequence
    1384  Select (JAMMA button 3): 00 = locked out, ff = allowed
    1484  time out: 00 = press Select, 55 = only lock the controls, ff = press Run
    1584  one player start sequence, 32 bytes
    3584  two players start sequence, 40 bytes
    5d84  P1 start button sequence, 8 bytes
    6584  P2 start button sequence, 8 bytes
    Sequences: 10-17 P1 Run, I, II, Select, Up, Right, Down, Left,
               28-2f same for P2, 80-fe wait, ff end

****************************************************************************

    V1 hardware, as used by the BIOS:

    Same 8085 memory map, without the RAM at $e000 (it only uses the 8155
    RAM) and without the game ID at $f000. Only RST 5.5 is used.

    The attract mode cycles through the slots every minute. The player
    chooses a game with the joystick and confirms it with start (or any of
    the inputs at $b000 bits 4-7), then the game runs freely: the start
    button is wired to Run. With 5 seconds left, the BIOS locks the controls
    and holds Run for 5 seconds (pausing the game) while the time display
    stops, then presses Run again and gives back the controls for the last
    5 seconds before resetting the game.

    8155 port A: two BCD digits of the time display
    8155 port B:
    0-1  time display digit strobes (units and hundreds, tens and thousands)
    2    controls lockout
    3    Run (while the controls are locked out)
    4    PC Engine audio enable
    5    PC Engine reset (active low)
    6-7  coin counters
    8155 port C:
    0-1  selected slot
    2-5  game selection lamps

****************************************************************************/


#include "emu.h"
#include "pcecommn.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include <algorithm>
#include <cstdlib>

#include "tourvis.lh"
#include "tourvisv1.lh"


namespace {

class tourvision_base_state : public pce_common_state
{
public:
	ioport_value slots_empty_r();

protected:
	// 74LS247 BCD to 7-segment decoder
	static constexpr u8 SEGMENTS[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00 };

	tourvision_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce_common_state(mconfig, type, tag)
		, m_subcpu(*this, "subcpu")
		, m_i8155(*this, "i8155")
		, m_cart(*this, "slot%u", 1U)
		, m_io_pad(*this, "P%u", 1U)
		, m_io_start(*this, "START")
		, m_time_digits(*this, "time_digit%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		, m_cart_rom(*this, "carts")
		, m_cart_bank(*this, "cart_bank")
	{ }

	virtual void machine_start() override ATTR_COLD;

	void tourvision_base(machine_config &config) ATTR_COLD;
	void base_8085_map(address_map &map) ATTR_COLD;

	void select_slot(unsigned slot);
	void pce_reset_w(int state);
	void pce_audio_w(int state);
	void coin_counters_w(u8 data);

	required_device<i8085a_cpu_device> m_subcpu;
	required_device<i8155_device> m_i8155;
	required_device_array<generic_slot_device, 4> m_cart;
	required_ioport_array<2> m_io_pad;
	required_ioport m_io_start;
	output_finder<4> m_time_digits;
	output_finder<4> m_lamps;

	u8 m_cart_id[4] = { 0xff, 0xff, 0xff, 0xff };
	u8 m_slot = 0;

private:
	void pce_mem(address_map &map) ATTR_COLD;

	template <unsigned N> DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_region_ptr<u8> m_cart_rom;
	required_memory_bank m_cart_bank;
};


class tourvision_state : public tourvision_base_state
{
public:
	tourvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: tourvision_base_state(mconfig, type, tag)
		, m_io_service(*this, "SERVICE")
		, m_score_digits(*this, "score%u_digit%u", 0U, 0U)
	{ }

	void tourvision(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(bookkeeping_changed);
	DECLARE_INPUT_CHANGED_MEMBER(clear_time_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual u8 joy_read() override;

private:
	void tourvision_8085_map(address_map &map) ATTR_COLD;

	u8 jamma_r();
	u8 cart_id_r();
	void i8155_a_w(u8 data);
	void i8155_b_w(u8 data);
	void i8155_c_w(u8 data);
	void update_displays();

	required_ioport m_io_service;
	output_finder<4, 4> m_score_digits;

	u8 m_port_a = 0;
	u8 m_port_b = 0;
	u8 m_port_c = 0;
	u16 m_time_shift = 0;
	u16 m_score_shift = 0;
};


class tourvision_v1_state : public tourvision_base_state
{
public:
	tourvision_v1_state(const machine_config &mconfig, device_type type, const char *tag)
		: tourvision_base_state(mconfig, type, tag)
	{ }

	void tourvision_v1(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual u8 joy_read() override;

private:
	void tourvision_v1_8085_map(address_map &map) ATTR_COLD;

	u8 jamma_r();
	void i8155_a_w(u8 data);
	void i8155_b_w(u8 data);
	void i8155_c_w(u8 data);

	u8 m_port_a = 0;
	u8 m_port_b = 0;
};


/***************************************************************************
    Common hardware
***************************************************************************/

template <unsigned N>
DEVICE_IMAGE_LOAD_MEMBER(tourvision_base_state::cart_load)
{
	u32 size = m_cart[N]->common_get_size("rom");
	u32 offset = 0;

	// skip the header of loose HuCard dumps
	if (!image.loaded_through_softlist() && ((size & 0x1fff) == 0x200))
	{
		offset = 0x200;
		size -= 0x200;
	}

	if (!size || (size > 0x100000))
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be no larger than 1M)");

	m_cart[N]->rom_alloc(size + offset, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart[N]->common_load_rom(m_cart[N]->get_rom_base(), size + offset, "rom");

	u8 *const rgn = &m_cart_rom[N * 0x100000];
	u8 const *const base = m_cart[N]->get_rom_base() + offset;

	if (size == 0x0c0000)
	{
		std::copy_n(base + 0x000000, 0x0c0000, rgn + 0x000000);
		std::copy_n(base + 0x080000, 0x040000, rgn + 0x0c0000);
	}
	else if (size == 0x060000)
	{
		std::copy_n(base + 0x000000, 0x040000, rgn + 0x000000);
		std::copy_n(base + 0x000000, 0x040000, rgn + 0x040000);
		std::copy_n(base + 0x040000, 0x020000, rgn + 0x080000);
		std::copy_n(base + 0x040000, 0x020000, rgn + 0x0a0000);
		std::copy_n(base + 0x040000, 0x020000, rgn + 0x0c0000);
		std::copy_n(base + 0x040000, 0x020000, rgn + 0x0e0000);
	}
	else
	{
		for (u32 i = 0; i < 0x100000; i += size)
			std::copy_n(base, std::min<u32>(size, 0x100000 - i), rgn + i);
	}

	// game ID set by the extra pins on the V4 cartridge edge connector
	char const *const id = image.get_feature("id");
	m_cart_id[N] = id ? strtoul(id, nullptr, 0) : 0xff;

	return std::make_pair(std::error_condition(), std::string());
}

ioport_value tourvision_base_state::slots_empty_r()
{
	ioport_value data = 0;
	for (unsigned i = 0; i < 4; i++)
	{
		if (!m_cart[i]->exists())
			data |= 1 << i;
	}

	return data;
}

void tourvision_base_state::select_slot(unsigned slot)
{
	m_slot = slot;
	m_cart_bank->set_entry(slot);
}

void tourvision_base_state::pce_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}

void tourvision_base_state::pce_audio_w(int state)
{
	m_maincpu->set_output_gain(ALL_OUTPUTS, state ? 1.0 : 0.0);
}

void tourvision_base_state::coin_counters_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}

void tourvision_base_state::machine_start()
{
	pce_common_state::machine_start();

	m_cart_bank->configure_entries(0, 4, &m_cart_rom[0], 0x100000);
	select_slot(0);

	save_item(NAME(m_cart_id));
	save_item(NAME(m_slot));
}

void tourvision_base_state::pce_mem(address_map &map)
{
	map(0x000000, 0x0fffff).bankr(m_cart_bank);
	common_mem_map(map);
}

void tourvision_base_state::base_8085_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x80ff).rw(m_i8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x8100, 0x8107).rw(m_i8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x9000, 0x9000).portr("DSW1");
	map(0xa000, 0xa000).portr("DSW2");
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xd000, 0xd000).nopw(); // watchdog?
}

void tourvision_base_state::tourvision_base(machine_config &config)
{
	// Basic machine hardware
	common_cpu(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &tourvision_base_state::pce_mem);
	m_maincpu->add_route(0, "speaker", 1.00, 0);
	m_maincpu->add_route(1, "speaker", 1.00, 1);

	config.set_maximum_quantum(attotime::from_hz(60));

	I8085A(config, m_subcpu, 6.144_MHz_XTAL);

	// timer clock unverified: both hardware revisions count 100 ticks per second with it
	I8155(config, m_i8155, 6.144_MHz_XTAL / 32);
	m_i8155->out_to_callback().set_inputline(m_subcpu, I8085_RST55_LINE).invert();

	// Video hardware
	common_video(config);

	SPEAKER(config, "speaker", 2).front(); // TODO: correct?

	for (unsigned i = 0; i < 4; i++)
		GENERIC_CARTSLOT(config, m_cart[i], generic_plain_slot, "tourvision_cart", "bin,pce");
	m_cart[0]->set_device_load(FUNC(tourvision_base_state::cart_load<0>));
	m_cart[1]->set_device_load(FUNC(tourvision_base_state::cart_load<1>));
	m_cart[2]->set_device_load(FUNC(tourvision_base_state::cart_load<2>));
	m_cart[3]->set_device_load(FUNC(tourvision_base_state::cart_load<3>));

	SOFTWARE_LIST(config, "tv_list").set_original("pce_tourvision");
}


/***************************************************************************
    V4 hardware
***************************************************************************/

u8 tourvision_state::cart_id_r()
{
	return m_cart[m_slot]->exists() ? m_cart_id[m_slot] : 0xff;
}

u8 tourvision_state::joy_read()
{
	// two joypad ports, read like a multitap
	unsigned const port = joystick_port_select();
	if (port > 1)
		return 0xff;

	// the JAMMA controls are cut off while the BIOS is in control
	u8 data = 0xff;
	if (!(port ? BIT(m_port_b, 3) : BIT(m_port_a, 6)))
	{
		data = m_io_pad[port]->read();
		if (BIT(m_port_a, 7))
			data |= 0x04; // Select
	}

	// buttons pressed by the BIOS: Run, I, II, Select, Up, Right, Down, Left
	if (port ? !BIT(m_port_a, 5) : BIT(m_port_a, 4))
	{
		static constexpr u8 BUTTONS[8] = { 0x08, 0x01, 0x02, 0x04, 0x10, 0x20, 0x40, 0x80 };
		data &= ~BUTTONS[m_port_a & 0x07];
	}

	return data;
}

u8 tourvision_state::jamma_r()
{
	// the BIOS reads the start buttons and P1 left/right for the game selection
	u8 const p1 = m_io_pad[0]->read();
	return (m_io_start->read() & 0xf3) | (BIT(p1, 7) << 2) | (BIT(p1, 5) << 3);
}

INPUT_CHANGED_MEMBER(tourvision_state::bookkeeping_changed)
{
	m_subcpu->set_input_line(I8085_RST65_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(tourvision_state::clear_time_changed)
{
	m_subcpu->set_input_line(I8085_RST75_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
}

void tourvision_state::i8155_a_w(u8 data)
{
	// bits 0-2: button, 4: press on P1 joypad, 5: press on P2 joypad (active low), 6: P1 lockout, 7: Select lockout
	m_port_a = data;
}

void tourvision_state::i8155_b_w(u8 data)
{
	// bits 0-1: slot select
	select_slot(data & 0x03);

	// bit 2: always set (cartridge buffers enable?)
	// bit 3: P2 lockout

	// bit 4: audio enable
	pce_audio_w(BIT(data, 4));

	// bit 5: PC Engine reset
	pce_reset_w(BIT(data, 5));

	// bits 6-7: coin counters
	coin_counters_w(data >> 6);

	m_port_b = data;
}

void tourvision_state::i8155_c_w(u8 data)
{
	// shift clock
	if (!BIT(m_port_c, 1) && BIT(data, 1))
	{
		// play time and lamps are sent LSB first, score counters MSB first
		m_time_shift = (m_time_shift >> 1) | (BIT(data, 3) << 15);
		m_score_shift = (m_score_shift << 1) | BIT(data, 0);
	}

	// latch
	if (!BIT(m_port_c, 4) && BIT(data, 4))
		update_displays();

	m_port_c = data;
}

void tourvision_state::update_displays()
{
	// time display: BCD digit in bits 0-3, digit strobe in bits 4-7
	for (unsigned i = 0; i < 4; i++)
	{
		if (BIT(m_time_shift, 4 + i))
			m_time_digits[i] = SEGMENTS[m_time_shift & 0x0f];
	}

	// score counters: one BCD digit per slot, digit strobe in bits 8-11
	for (unsigned i = 0; i < 4; i++)
	{
		if (BIT(m_time_shift, 8 + i))
		{
			for (unsigned slot = 0; slot < 4; slot++)
				m_score_digits[slot][i] = SEGMENTS[(m_score_shift >> (12 - (slot * 4))) & 0x0f];
		}
	}

	// game selection lamps in bits 12-15
	for (unsigned slot = 0; slot < 4; slot++)
		m_lamps[slot] = BIT(m_time_shift, 12 + slot);
}

void tourvision_state::machine_start()
{
	tourvision_base_state::machine_start();

	save_item(NAME(m_port_a));
	save_item(NAME(m_port_b));
	save_item(NAME(m_port_c));
	save_item(NAME(m_time_shift));
	save_item(NAME(m_score_shift));
}

void tourvision_state::tourvision_8085_map(address_map &map)
{
	base_8085_map(map);
	map(0xb000, 0xb000).r(FUNC(tourvision_state::jamma_r));
	map(0xe000, 0xe1ff).ram();
	map(0xf000, 0xf000).r(FUNC(tourvision_state::cart_id_r));
}


/***************************************************************************
    V1 hardware
***************************************************************************/

u8 tourvision_v1_state::joy_read()
{
	// single joypad, start button wired to Run
	if (!BIT(m_port_b, 2))
		return m_io_pad[0]->read();

	// the BIOS presses Run to pause the game when the time runs out
	return BIT(m_port_b, 3) ? 0xf7 : 0xff;
}

u8 tourvision_v1_state::jamma_r()
{
	// the BIOS reads the start buttons and P1 left/right for the game selection
	u8 const p1 = m_io_pad[0]->read();
	return (m_io_start->read() & 0xf2) | BIT(p1, 3) | (BIT(p1, 7) << 2) | (BIT(p1, 5) << 3);
}

void tourvision_v1_state::i8155_a_w(u8 data)
{
	// two BCD digits of the time display
	m_port_a = data;
}

void tourvision_v1_state::i8155_b_w(u8 data)
{
	// bits 0-1: time display digit strobes (units and hundreds, tens and thousands)
	for (unsigned i = 0; i < 2; i++)
	{
		if (BIT(data, i))
		{
			m_time_digits[i] = SEGMENTS[m_port_a & 0x0f];
			m_time_digits[i + 2] = SEGMENTS[m_port_a >> 4];
		}
	}

	// bit 2: controls lockout
	// bit 3: Run (while the controls are locked out)

	// bit 4: audio enable
	pce_audio_w(BIT(data, 4));

	// bit 5: PC Engine reset
	pce_reset_w(BIT(data, 5));

	// bits 6-7: coin counters
	coin_counters_w(data >> 6);

	m_port_b = data;
}

void tourvision_v1_state::i8155_c_w(u8 data)
{
	// bits 0-1: slot select
	select_slot(data & 0x03);

	// bits 2-5: game selection lamps
	for (unsigned slot = 0; slot < 4; slot++)
		m_lamps[slot] = BIT(data, 2 + slot);
}

void tourvision_v1_state::machine_start()
{
	tourvision_base_state::machine_start();

	save_item(NAME(m_port_a));
	save_item(NAME(m_port_b));
}

void tourvision_v1_state::tourvision_v1_8085_map(address_map &map)
{
	base_8085_map(map);
	map(0xb000, 0xb000).r(FUNC(tourvision_v1_state::jamma_r));
}


/***************************************************************************
    Inputs
***************************************************************************/

INPUT_PORTS_START( tourvision_common )
	// Button II is on the left of a standard PCE joypad
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button I") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button II") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Select") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // Run, only pressed by the BIOS
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Button I") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Button II") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Select") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // Run, only pressed by the BIOS
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_CUSTOM ) // P1 left and right
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(tourvision_base_state::slots_empty_r))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Coin A Coins" ) PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPNAME( 0x78, 0x78, "Coin A Play Time" ) PORT_DIPLOCATION("SWA:4,5,6,7")
	PORT_DIPSETTING(    0x78, "60 seconds" )
	PORT_DIPSETTING(    0x70, "90 seconds" )
	PORT_DIPSETTING(    0x68, "120 seconds" )
	PORT_DIPSETTING(    0x60, "150 seconds" )
	PORT_DIPSETTING(    0x58, "180 seconds" )
	PORT_DIPSETTING(    0x50, "210 seconds" )
	PORT_DIPSETTING(    0x48, "240 seconds" )
	PORT_DIPSETTING(    0x40, "270 seconds" )
	PORT_DIPSETTING(    0x38, "300 seconds" )
	PORT_DIPSETTING(    0x30, "360 seconds" )
	PORT_DIPSETTING(    0x28, "420 seconds" )
	PORT_DIPSETTING(    0x20, "480 seconds" )
	PORT_DIPSETTING(    0x18, "540 seconds" )
	PORT_DIPSETTING(    0x10, "600 seconds" )
	PORT_DIPSETTING(    0x08, "720 seconds" )
	PORT_DIPSETTING(    0x00, "900 seconds" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( tourvision )
	PORT_INCLUDE( tourvision_common )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Bookkeeping") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tourvision_state::bookkeeping_changed), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Clear Time") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tourvision_state::clear_time_changed), 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_NAME("RAM Clear (hold on power up)")

	PORT_START("DSW2")
	// with SWB:1, SWB:2 and SWB:8 on, the system shows each game for the purchased time, without letting anyone play
	PORT_DIPNAME( 0x03, 0x03, "Coin B Coins" ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "4" ) PORT_CONDITION("DSW2", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(    0x00, "4 (Exhibition Mode)" ) PORT_CONDITION("DSW2", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPNAME( 0x04, 0x04, "Score (Bonus Time)" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x78, 0x78, "Coin B Play Time" ) PORT_DIPLOCATION("SWB:4,5,6,7")
	PORT_DIPSETTING(    0x78, "240 seconds" )
	PORT_DIPSETTING(    0x70, "300 seconds" )
	PORT_DIPSETTING(    0x68, "360 seconds" )
	PORT_DIPSETTING(    0x60, "420 seconds" )
	PORT_DIPSETTING(    0x58, "480 seconds" )
	PORT_DIPSETTING(    0x50, "540 seconds" )
	PORT_DIPSETTING(    0x48, "600 seconds" )
	PORT_DIPSETTING(    0x40, "660 seconds" )
	PORT_DIPSETTING(    0x38, "720 seconds" )
	PORT_DIPSETTING(    0x30, "780 seconds" )
	PORT_DIPSETTING(    0x28, "840 seconds" )
	PORT_DIPSETTING(    0x20, "900 seconds" )
	PORT_DIPSETTING(    0x18, "960 seconds" )
	PORT_DIPSETTING(    0x10, "1080 seconds" )
	PORT_DIPSETTING(    0x08, "1200 seconds" )
	PORT_DIPSETTING(    0x00, "1500 seconds" )
	PORT_DIPNAME( 0x80, 0x80, "Slot on Power Up" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "First" )
	PORT_DIPSETTING(    0x80, "Last Played" )
INPUT_PORTS_END

INPUT_PORTS_START( tourvision_v1 )
	PORT_INCLUDE( tourvision_common )

	// single joypad, the start button is wired to Run
	PORT_MODIFY("P1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start / Run")

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) // P1 start

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Coin B Coins" ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPNAME( 0x04, 0x04, "P1+P2 Start Returns to Game Selection" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x78, 0x78, "Coin B Play Time" ) PORT_DIPLOCATION("SWB:4,5,6,7")
	PORT_DIPSETTING(    0x78, "240 seconds" )
	PORT_DIPSETTING(    0x70, "300 seconds" )
	PORT_DIPSETTING(    0x68, "360 seconds" )
	PORT_DIPSETTING(    0x60, "420 seconds" )
	PORT_DIPSETTING(    0x58, "480 seconds" )
	PORT_DIPSETTING(    0x50, "540 seconds" )
	PORT_DIPSETTING(    0x48, "600 seconds" )
	PORT_DIPSETTING(    0x40, "660 seconds" )
	PORT_DIPSETTING(    0x38, "720 seconds" )
	PORT_DIPSETTING(    0x30, "780 seconds" )
	PORT_DIPSETTING(    0x28, "840 seconds" )
	PORT_DIPSETTING(    0x20, "900 seconds" )
	PORT_DIPSETTING(    0x18, "960 seconds" )
	PORT_DIPSETTING(    0x10, "1080 seconds" )
	PORT_DIPSETTING(    0x08, "1200 seconds" )
	PORT_DIPSETTING(    0x00, "1500 seconds" )
	PORT_DIPNAME( 0x80, 0x80, "Coin B Counter" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "Counter A, 4 pulses" )
	PORT_DIPSETTING(    0x80, "Counter B" )
INPUT_PORTS_END


/***************************************************************************
    Machine configs
***************************************************************************/

void tourvision_state::tourvision(machine_config &config)
{
	tourvision_base(config);

	m_subcpu->set_addrmap(AS_PROGRAM, &tourvision_state::tourvision_8085_map);
	m_subcpu->in_sid_func().set_ioport(m_io_service).bit(2);

	m_i8155->out_pa_callback().set(FUNC(tourvision_state::i8155_a_w));
	m_i8155->out_pb_callback().set(FUNC(tourvision_state::i8155_b_w));
	m_i8155->out_pc_callback().set(FUNC(tourvision_state::i8155_c_w));

	config.set_default_layout(layout_tourvis);
}

void tourvision_v1_state::tourvision_v1(machine_config &config)
{
	tourvision_base(config);

	m_subcpu->set_addrmap(AS_PROGRAM, &tourvision_v1_state::tourvision_v1_8085_map);

	m_i8155->out_pa_callback().set(FUNC(tourvision_v1_state::i8155_a_w));
	m_i8155->out_pb_callback().set(FUNC(tourvision_v1_state::i8155_b_w));
	m_i8155->out_pc_callback().set(FUNC(tourvision_v1_state::i8155_c_w));

	config.set_default_layout(layout_tourvisv1);
}


/***************************************************************************
    ROM definitions
***************************************************************************/

ROM_START( tourvis )
	ROM_REGION( 0x400000, "carts", ROMREGION_ERASEFF ) // four 1M cartridge slots

	ROM_REGION( 0x8000, "subcpu", 0 ) // 27C256
	ROM_SYSTEM_BIOS( 0, "60", "V4-60" )
	ROMX_LOAD( "v4-60.ic29", 0x0000, 0x8000, CRC(1fd27e22) SHA1(b103d365eac3fa447c2e9addddf6974b4403ed41), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "55", "V4-55" )
	ROMX_LOAD( "v4-55.ic29", 0x0000, 0x8000, CRC(87cf66c1) SHA1(d6b42137be7a07a0e299c2d922328a6a9a2b7b8f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "54", "V4-54" )
	ROMX_LOAD( "v4-54.ic29", 0x0000, 0x8000, CRC(125e7a6e) SHA1(6f03c49acaad4feb9c2187f84fc1cb5451d0eb2b), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "53", "V4-53" )
	ROMX_LOAD( "v4-53.ic29", 0x0000, 0x8000, CRC(bccb53c9) SHA1(a27113d70cf348c7eafa39fc7a76f55f63723ad7), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "52", "V4-52" )
	ROMX_LOAD( "v4-52.ic29", 0x0000, 0x8000, CRC(ffd7b0fe) SHA1(d1804865c91e925a01b05cf441e8458a3db23f50), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "51", "V4-51" )
	ROMX_LOAD( "v4-51.ic29", 0x0000, 0x8000, CRC(2de0b946) SHA1(c13051453aff3a2329f18ebade75c69aa3b1c28d), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "43", "V4-43" )
	ROMX_LOAD( "v4-43.ic29", 0x0000, 0x8000, CRC(88da23f3) SHA1(9d24faa116129783e55c7f79a4a08902a236d5a6), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "42", "V4-42" )
	ROMX_LOAD( "v4-42.ic29", 0x0000, 0x8000, CRC(0372606d) SHA1(8c6a63865b0b4951ea032725cf022d353115b93b), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "40", "V4-40" )
	ROMX_LOAD( "v4-40.ic29", 0x0000, 0x8000, CRC(ba6290cc) SHA1(92b0e9f55791e892ec209de4fadd80faef370622), ROM_BIOS(8) )
ROM_END

ROM_START( tourvisv1 )
	ROM_REGION( 0x400000, "carts", ROMREGION_ERASEFF ) // four 1M cartridge slots

	// the code ends at 0x077e, the rest of the EPROMs is leftover data from another program
	ROM_REGION( 0x8000, "subcpu", 0 ) // 27C256
	ROM_SYSTEM_BIOS( 0, "20", "VT-20" )
	ROMX_LOAD( "vt_2.0.bin", 0x0000, 0x8000, CRC(36012f88) SHA1(5bd42fb51aa48ff65e704ea06a9181bb87ed2137), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "11", "VT-11" )
	ROMX_LOAD( "vt_1.1.bin", 0x0000, 0x8000, CRC(27abbc36) SHA1(881ea7802b9e241473bc8ced0472e0f1851c9886), ROM_BIOS(1) )
ROM_END

} // anonymous namespace

//     YEAR  NAME       PARENT   MACHINE        INPUT          CLASS                INIT             ROT   COMPANY                  FULLNAME                                FLAGS
GAMEL( 19??, tourvis,   0,       tourvision,    tourvision,    tourvision_state,    init_pce_common, ROT0, u8"bootleg (TourVisión)", u8"TourVisión V4 (PC Engine bootleg)", MACHINE_IS_BIOS_ROOT, layout_tourvis )
GAMEL( 19??, tourvisv1, tourvis, tourvision_v1, tourvision_v1, tourvision_v1_state, init_pce_common, ROT0, u8"bootleg (TourVisión)", u8"TourVisión V1 (PC Engine bootleg)", MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING, layout_tourvisv1 )
