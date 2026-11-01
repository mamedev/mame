// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

    Nishijin Digital-Pachinko Series Driver

    Driver by Takahiro Nogi

    Initial driver development started on 2024-01-01.

    Supported Games:
    - Funky 7 P-3  (c) 1989 Nishijin / Sophia

    Hardware:
    - Main CPU: Z80 @ provisional 4.096 MHz
    - Sound: SN76496 @ 4.096 MHz

    TODO:
    - Determine physical crystal frequencies from an authentic PCB.
      (Current CPU/sound clock is provisionally calibrated by synchronizing
      against audio recorded from physical machine gameplay footage).
    - Acquire high-resolution photographic scans of the playfield cell board
      to accurately measure lamp, solenoid, and mechanical fixture coordinates.
      (Surviving vintage units of this model are exceptionally scarce).

    NOTE:
    - The dedicated 7-segment LED fixtures designed for this machine do not
      feature decimal point (dot) elements.
    - Lamp, LED, and 7-segment display outputs have been verified against
      gameplay footage and behave as expected during normal operation.
      However, the exact display update timing is not yet fully understood
      and may differ from real hardware at the logic level.
    - Periodic interrupt frequency is approximated based on gameplay footage
      and may not reflect exact hardware timing.

****************************************************************************

    Funky 7 Digital Color Table

         L   C   R
        --- --- ---
    0   0-R 0-R 0-R
    1   1-R 0-G(*) 1-R
    2   1-G 1-R 1-G
    3   2-R 1-G 2-R
    4   3-R 2-R 3-R
    5   3-G 3-R 3-G
    6   4-R 3-G 4-R
    7   5-R 4-R 5-R
    8   5-G 5-R 5-G
    9   6-R 5-G 6-R
    10  7-R 6-R 7-R
    11  7-G 7-R 7-G
    12  8-R 7-G 8-R
    13  9-R 8-R 9-R
    14  9-G 9-R 9-G
    15      9-G

    *: exists only on center digit

    bit: 7, 6, 5, 4, 3, 2, 1, 0
    SEG: .  a  b  c  d  e  f  g

    0   0x7e    0b01111110
    1   0x30    0b00110000
    2   0x6d    0b01101101
    3   0x79    0b01111001
    4   0x33    0b00110011
    5   0x5b    0b01011011
    6   0x5f    0b01011111
    7   0x70    0b01110000
    8   0x7f    0b01111111
    9   0x7b    0b01111011
    -   0x01    0b00000001

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#include "speaker.h"


namespace {

// internal artwork
#include "funky7.lh"

class funky7_state : public driver_device
{
public:
	funky7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sn76496(*this, "sn76496")
		, m_ioport_in(*this, "INPUT")
		, m_lay_digits_red(*this, "digit%u_red", 0U)
		, m_lay_digits_green(*this, "digit%u_green", 0U)
		, m_lay_digits_yellow(*this, "digit%u_yellow", 0U)
		, m_lay_digits_back(*this, "digit%u_back", 0U)
		, m_lay_leds(*this, "leds%u", 0U)
		, m_lay_lamps(*this, "lamps%u", 0U)
	{ }

	void funky7(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sn76496_device> m_sn76496;
	required_ioport m_ioport_in;
	output_finder<3> m_lay_digits_red;
	output_finder<3> m_lay_digits_green;
	output_finder<3> m_lay_digits_yellow;
	output_finder<3> m_lay_digits_back;
	output_finder<4> m_lay_leds;
	output_finder<7> m_lay_lamps;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(interrupt);

	u8 switch_r(offs_t offset);
	void bank_select_w(offs_t offset, u8 data);
	void lamp_latch_w(offs_t offset, u8 data);
	void segment_data_w(offs_t offset, u8 data);
	void aux_control_w(offs_t offset, u8 data);

	static constexpr u8 kPendingLatch = 0;	// temporary latch (index 0) used before target selection

	u8 m_switch_data;
	u8 m_bank_select;
	u8 m_lamp_data[8];
	u8 m_seg_data[8];
	u8 m_aux_data[8];
	u8 m_bg_color;
};


void funky7_state::machine_start()
{
	save_item(NAME(m_switch_data));
	save_item(NAME(m_bank_select));
	save_item(NAME(m_lamp_data));
	save_item(NAME(m_seg_data));
	save_item(NAME(m_aux_data));
	save_item(NAME(m_bg_color));
}

void funky7_state::machine_reset()
{
	m_switch_data = 0;
	m_bank_select = 0;
	std::fill(std::begin(m_lamp_data), std::end(m_lamp_data), 0);
	std::fill(std::begin(m_seg_data), std::end(m_seg_data), 0);
	std::fill(std::begin(m_aux_data), std::end(m_aux_data), 0);
	m_bg_color = 0;
}


INTERRUPT_GEN_MEMBER(funky7_state::interrupt)
{
	// Port 0xa000 read:
	// bit0: Start chacker 1 switch
	// bit1: Start chacker 2 sensor
	// bit2: Start chacker 3 sensor
	// bit3: V-Zone sensor
	// bit4: Attacker sensor
	// bit5: Unused
	// bit6: Unused
	// bit7: Program reset

	static const u8 num1[3] = { 2, 4, 6 };			// SEG Red bank offset
	static const u8 num2[3] = { 1, 3, 5 };			// SEG Green bank offset

	m_switch_data = m_ioport_in->read();

	// Playfield general illumination / lamps
	for (u8 i = 0; i < 6; i++)
		m_lay_lamps[i] = BIT(m_lamp_data[0], i);

	// 7-segment digital display elements
	for (u8 i = 0; i < 3; i++)
	{
		m_lay_digits_red[i] = bitswap<8>(m_seg_data[num1[i]], 7, 0, 1, 2, 3, 4, 5, 6);
		m_lay_digits_green[i] = bitswap<8>(m_seg_data[num2[i]], 7, 0, 1, 2, 3, 4, 5, 6);
	}

	// "Chance Stock" memory LEDs (4-step reserve ball indicator)
	for (u8 i = 0; i < 4; i++)
		m_lay_leds[i] = BIT((m_aux_data[1] >> 2), i);

	// 7-segment background backlight illumination
	if (BIT(m_aux_data[3], 5))
		m_bg_color = 2;	// Green
	else if (BIT(m_aux_data[3], 2))
		m_bg_color = 1;	// Red
	else
		m_bg_color = 0;	// Off


	for (u8 i = 0; i < 3; i++)
		m_lay_digits_back[i] = m_bg_color;

	// Attacker payout gate solenoid
	m_lay_lamps[6] = BIT(m_lamp_data[0], 6);

	// Pulse Z80 RESET to restart execution cycle from 0x0000
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


u8 funky7_state::switch_r(offs_t offset)
{
	return m_switch_data;
}

void funky7_state::bank_select_w(offs_t offset, u8 data)
{
	u8 target = 0;

	m_bank_select = data;

	// Resolve target display/lamp buffer slot
	if ((m_bank_select & 0x0f) != 0x00)
	{
		// bit0: Left-Red, bit1: Center-Red, bit2: Right-Red, bit3: Left-Green
		for (u8 bit = 0; bit < 4; bit++)
		{
			if (BIT(m_bank_select, bit))
			{
				target = bit + 1;
				break;
			}
		}
	}
	else if ((m_aux_data[kPendingLatch] & 0x03) != 0x00)
	{
		// bit4: Center-Green, bit5: Right-Green
		target = 4 + (m_aux_data[kPendingLatch] & 0x03);
	}

	if (target != 0)
	{
		m_lamp_data[target] = m_lamp_data[kPendingLatch];
		m_seg_data[target]  = m_seg_data[kPendingLatch];
		m_aux_data[target]  = m_aux_data[kPendingLatch];
	}
}

void funky7_state::lamp_latch_w(offs_t offset, u8 data)
{
	m_lamp_data[kPendingLatch] = data;
}

void funky7_state::segment_data_w(offs_t offset, u8 data)
{
	m_seg_data[kPendingLatch] = data;
}

void funky7_state::aux_control_w(offs_t offset, u8 data)
{
	m_aux_data[kPendingLatch] = data;
}


void funky7_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram();
	map(0x4000, 0x4000).w(FUNC(funky7_state::bank_select_w));
	map(0x6000, 0x6000).w(FUNC(funky7_state::lamp_latch_w));
	map(0x8000, 0x8000).w(FUNC(funky7_state::segment_data_w));
	map(0xa000, 0xa000).r(FUNC(funky7_state::switch_r)).w(FUNC(funky7_state::aux_control_w));
}


void funky7_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x00).w(m_sn76496, FUNC(sn76496_device::write));
}


static INPUT_PORTS_START( funky7 )

	PORT_START("INPUT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_START1 )  PORT_NAME("Start Chacker 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )  PORT_NAME("Start Chacker 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START3 )  PORT_NAME("Start Chacker 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("V-Zone Sensor")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Attacker Sensor")
	PORT_BIT( 0x60, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_OTHER )   PORT_CODE(KEYCODE_R) PORT_NAME("Program Reset")

INPUT_PORTS_END


void funky7_state::funky7(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4.096_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &funky7_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &funky7_state::io_map);
	// Interrupt frequency is estimated from observed gameplay pacing
	// and verified audio timing, not measured from original hardware.
	m_maincpu->set_periodic_int(FUNC(funky7_state::interrupt), attotime::from_hz(500));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SN76496(config, m_sn76496, 4.096_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 0.7);
}


ROM_START( pcfunky7 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_NOERASE )
	ROM_LOAD( "funky7p3.bin", 0x0000, 0x2000, CRC(686a06c1) SHA1(2e8411692601a331b476f4e995803081b83a3bbe) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,     PARENT, MACHINE, INPUT,  CLASS,        INIT,       MONITOR, COMPANY,             FULLNAME,          FLAGS,                                      LAYOUT
GAMEL( 1989, pcfunky7, 0,      funky7,  funky7, funky7_state, empty_init, 0,       "Nishijin / Sophia", "Funky 7 P-3",     (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_funky7 )
