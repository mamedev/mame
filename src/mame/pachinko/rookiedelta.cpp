// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

    Nishijin Digital-Pachinko Series Driver

    Driver by Takahiro Nogi

    Initial driver development started on 2024-01-01.

    Supported Games:
    - Rookie Delta MH  (c) 1991 Nishijin / Sophia

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

    Rookie Delta SEG layout:

        U
       (0)
     L     R
    (0)   (9)

    *: () indicates initial power-on symbol

    Normal game display:
    U = 3rd stop
    L = 1st stop
    R = 2nd stop

    Bonus round display:
    U = Bonus Symbol
    L = Current Round
    R = Round Hit Count

         L   U   R
        --- --- ---
    0    0   0   0
    1    1   1   1
    2    2   2   2
    3    3   3   3
    4    4   4   4
    5    5   5   5
    6    6   6   6
    7    7   7   7
    8    8   8   8
    9    9   9   9
    10   A   A   A
    11   F   F   F
    12   H   H   H
    13   J   J   J
    14   P   P   P

    bit: 7, 6, 5, 4, 3, 2, 1, 0
    SEG: .  f  g  a  b  c  d  e

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#include "speaker.h"


namespace {

// internal artwork
#include "rookiedelta.lh"

class rookiedelta_state : public driver_device
{
public:
	rookiedelta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sn76496(*this, "sn76496")
		, m_ioport_in(*this, "INPUT")
		, m_lay_digits_red(*this, "digit%u_red", 0U)
		, m_lay_digits_green(*this, "digit%u_green", 0U)
		, m_lay_digits_yellow(*this, "digit%u_yellow", 0U)
		, m_lay_digits_back(*this, "digit%u_back", 0U)
		, m_lay_leds1(*this, "leds1_%u", 0U)
		, m_lay_leds2(*this, "leds2_%u", 0U)
		, m_lay_leds3(*this, "leds3_%u", 0U)
		, m_lay_lamps(*this, "lamps%u", 0U)
	{ }

	void rookiedelta(machine_config &config);

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
	output_finder<4> m_lay_leds1;
	output_finder<7> m_lay_leds2;
	output_finder<7> m_lay_leds3;
	output_finder<8> m_lay_lamps;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(interrupt);

	u8 switch_r(offs_t offset);
	void bank_select_w(offs_t offset, u8 data);
	void lamp_latch_w(offs_t offset, u8 data);
	void segment_data_w(offs_t offset, u8 data);
	void aux_control_w(offs_t offset, u8 data);

	u8 m_switch_data;
	u8 m_bank_select;
	u8 m_lamp_data[8];
	u8 m_seg_data[8];
	u8 m_aux_data[8];
	u8 m_lamp_pending;
	u8 m_seg_pending;
	u8 m_aux_pending;
};


void rookiedelta_state::machine_start()
{
	save_item(NAME(m_switch_data));
	save_item(NAME(m_bank_select));
	save_item(NAME(m_lamp_data));
	save_item(NAME(m_seg_data));
	save_item(NAME(m_aux_data));
	save_item(NAME(m_lamp_pending));
	save_item(NAME(m_seg_pending));
	save_item(NAME(m_aux_pending));
}

void rookiedelta_state::machine_reset()
{
	m_switch_data = 0;
	m_bank_select = 0;
	m_lamp_pending = 0;
	m_seg_pending = 0;
	m_aux_pending = 0;
	std::fill(std::begin(m_lamp_data), std::end(m_lamp_data), 0);
	std::fill(std::begin(m_seg_data), std::end(m_seg_data), 0);
	std::fill(std::begin(m_aux_data), std::end(m_aux_data), 0);
}


INTERRUPT_GEN_MEMBER(rookiedelta_state::interrupt)
{
	// Port 0xa000 read:
	// bit0: Start chacker sensor
	// bit1: V-Zone gate sensor
	// bit2: Attacker gate sensor
	// bit3: Unused
	// bit4: Unused
	// bit5: Unused
	// bit6: Unused
	// bit7: Program reset

	static const u8 seg[3]  = { 0, 2, 1 };
	static const u8 num1[3] = { 0, 2, 4 };
	static const u8 num2[3] = { 1, 3, 5 };

	m_switch_data = m_ioport_in->read();

	// Playfield general illumination / lamps
	for (u8 i = 0; i < 6; i++)
		m_lay_lamps[i] = BIT(m_lamp_data[1], i);

	// 7-segment digital display elements (Red and Green can be lit simultaneously to form Yellow)
	for (u8 i = 0; i < 3; i++)
	{
		u8 tmpr = bitswap<8>((m_seg_data[num1[i]] & 0x7f), 7, 5, 6, 0, 1, 2, 3, 4);	// Red
		u8 tmpg = bitswap<8>((m_seg_data[num2[i]] & 0x7f), 7, 5, 6, 0, 1, 2, 3, 4);	// Green
		u8 tmpy = 0;                                                                    // Yellow (Red & Green)

		for (u8 n = 0; n < 8; n++)
		{
			if (BIT(tmpr, n) && BIT(tmpg, n))
				tmpy |= (1 << n);
		}

		tmpr &= tmpy ^ 0x7f;
		tmpg &= tmpy ^ 0x7f;

		m_lay_digits_red[seg[i]]    = tmpr;
		m_lay_digits_green[seg[i]]  = tmpg;
		m_lay_digits_yellow[seg[i]] = tmpy;
	}

	// "Chance Stock" memory LEDs (4-step reserve ball indicator)
	for (u8 i = 0; i < 4; i++)
		m_lay_leds1[i] = BIT(m_aux_data[4], i);

	// Decorative LED arrays
	for (u8 i = 0; i < 7; i++)
	{
		m_lay_leds2[i] = BIT(m_aux_data[6], i);
		m_lay_leds3[i] = BIT(m_aux_data[7], i);
	}

	// 7-segment background backlight illumination (bi-color LED simulation)
	for (u8 i = 0; i < 3; i++)
	{
		const u8 tmpr = BIT(m_aux_data[num1[i]], 7);
		const u8 tmpg = BIT(m_aux_data[num2[i]], 7);
		u8 bg_color;

		if (tmpr && tmpg)
			bg_color = 3;	// Yellow (Red & Green)
		else if (tmpg)
			bg_color = 2;	// Green
		else if (tmpr)
			bg_color = 1;	// Red
		else
			bg_color = 0;	// Off

		m_lay_digits_back[seg[i]] = bg_color;
	}

	// Attacker payout gate solenoid
	m_lay_lamps[6] = BIT(m_seg_data[6], 7);
	// V-Zone gate solenoid
	m_lay_lamps[7] = !BIT(m_bank_select, 3);

	// Pulse Z80 RESET to restart execution cycle from 0x0000
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


u8 rookiedelta_state::switch_r(offs_t offset)
{
	const u8 selected_bank = m_bank_select & 0x07;

	m_lamp_data[selected_bank] = m_lamp_pending;
	m_seg_data[selected_bank]  = m_seg_pending;
	m_aux_data[selected_bank]  = m_aux_pending;

	return m_switch_data;
}

void rookiedelta_state::bank_select_w(offs_t offset, u8 data)
{
	m_bank_select = data;
}

void rookiedelta_state::lamp_latch_w(offs_t offset, u8 data)
{
	m_lamp_pending = data;
}

void rookiedelta_state::segment_data_w(offs_t offset, u8 data)
{
	m_seg_pending = data;
}

void rookiedelta_state::aux_control_w(offs_t offset, u8 data)
{
	m_aux_pending = data;
}


void rookiedelta_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram();
	map(0x4000, 0x4000).w(FUNC(rookiedelta_state::bank_select_w));
	map(0x6000, 0x6000).w(FUNC(rookiedelta_state::lamp_latch_w));
	map(0x8000, 0x8000).w(FUNC(rookiedelta_state::segment_data_w));
	map(0xa000, 0xa000).r(FUNC(rookiedelta_state::switch_r)).w(FUNC(rookiedelta_state::aux_control_w));
}


void rookiedelta_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x00).w(m_sn76496, FUNC(sn76496_device::write));
}


static INPUT_PORTS_START( rookiedelta )

	PORT_START("INPUT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_START1 )  PORT_NAME("Start Chacker")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("V-Zone Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Attacker Sensor")
	PORT_BIT( 0x78, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_OTHER )   PORT_CODE(KEYCODE_R) PORT_NAME("Program Reset")

INPUT_PORTS_END


void rookiedelta_state::rookiedelta(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4.096_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rookiedelta_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &rookiedelta_state::io_map);
	// Interrupt frequency is estimated from observed gameplay pacing
	// and verified audio timing, not measured from original hardware.
	m_maincpu->set_periodic_int(FUNC(rookiedelta_state::interrupt), attotime::from_hz(500));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SN76496(config, m_sn76496, 4.096_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 0.7);
}


ROM_START( pcrkdelta )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_NOERASE )
	ROM_LOAD( "rookiedmh.bin", 0x0000, 0x2000, CRC(703e234d) SHA1(9649f975014029cf51546cc48f08b0be7817ef0d) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,      PARENT, MACHINE,     INPUT,       CLASS,             INIT,       MONITOR, COMPANY,             FULLNAME,          FLAGS,                                      LAYOUT
GAMEL( 1991, pcrkdelta, 0,      rookiedelta, rookiedelta, rookiedelta_state, empty_init, 0,       "Nishijin / Sophia", "Rookie Delta MH", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_rookiedelta )
